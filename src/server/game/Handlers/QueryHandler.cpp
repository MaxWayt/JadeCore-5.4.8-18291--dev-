/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Common.h"
#include "Language.h"
#include "DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"
#include "NPCHandler.h"
#include "Pet.h"
#include "MapManager.h"

void WorldSession::SendRealmNameQueryOpcode(uint32 realmId)
{
    uint32 RealmId = realmId;

    if(realmId != realmID)
        return; // Cheater ?

    WorldPacket data(SMSG_REALM_NAME_QUERY_RESPONSE);
    data << uint8(0); // ok, realmId exist server-side
    data << uint32(RealmId);
	
    data.WriteBits(sWorld->GetRealmName().size(), 8);
    data.WriteBit(1); // unk, if it's main realm ?
    data.WriteBits(sWorld->GetRealmName().size(), 8);
    data.WriteString(sWorld->GetRealmName());
    data.WriteString(sWorld->GetRealmName());

    SendPacket(&data);
}

void WorldSession::HandleRealmNameQueryOpcode(WorldPacket& recvPacket)
{
    uint32 realmId;
    recvPacket >> realmId;
    SendRealmNameQueryOpcode(realmId);
}

void WorldSession::SendNameQueryOpcode(ObjectGuid guid)
{
    ObjectGuid guid2 = 0;
    ObjectGuid guid3 = guid;

    Player* player = ObjectAccessor::FindPlayer(guid);
    CharacterNameData const* nameData = sWorld->GetCharacterNameData(GUID_LOPART(guid));

    WorldPacket data(SMSG_NAME_QUERY_RESPONSE, 500);
    data.WriteBit(guid[3]);
    data.WriteBit(guid[6]);
    data.WriteBit(guid[7]);
    data.WriteBit(guid[2]);
    data.WriteBit(guid[5]);
    data.WriteBit(guid[4]);
    data.WriteBit(guid[0]);
    data.WriteBit(guid[1]);

    data.WriteByteSeq(guid[5]);
    data.WriteByteSeq(guid[4]);
    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid[2]);

    data << uint8(!nameData);

    if (nameData)
    {
        data << uint32(realmID); // realmIdSecond
        data << uint32(1); // AccID
        data << uint8(nameData->m_class);
        data << uint8(nameData->m_race);
        data << uint8(nameData->m_level);
        data << uint8(nameData->m_gender);
    }

    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(guid[3]);

    if (!nameData)
    {
        SendPacket(&data);
        return;
    }

    data.WriteBit(guid2[2]);
    data.WriteBit(guid2[7]);
    data.WriteBit(guid3[7]);
    data.WriteBit(guid3[2]);
    data.WriteBit(guid3[0]);
    data.WriteBit(0); //isDeleted ? Wod ?
    data.WriteBit(guid2[4]);
    data.WriteBit(guid3[5]);
    data.WriteBit(guid2[1]);
    data.WriteBit(guid2[3]);
    data.WriteBit(guid2[0]);

    DeclinedName const* names = (player ? player->GetDeclinedNames() : NULL);
    for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
        data.WriteBits(names ? names->name[i].size() : 0, 7);

    data.WriteBit(guid3[6]);
    data.WriteBit(guid3[3]);
    data.WriteBit(guid2[5]);
    data.WriteBit(guid3[1]);
    data.WriteBit(guid3[4]);
    data.WriteBits(nameData->m_name.size(), 6);
    data.WriteBit(guid2[2]);

    data.FlushBits();

    data.WriteByteSeq(guid3[6]);
    data.WriteByteSeq(guid3[0]);
    data.WriteString(nameData->m_name);
    data.WriteByteSeq(guid2[5]);
    data.WriteByteSeq(guid2[2]);
    data.WriteByteSeq(guid3[3]);
    data.WriteByteSeq(guid2[4]);
    data.WriteByteSeq(guid2[3]);
    data.WriteByteSeq(guid3[4]);
    data.WriteByteSeq(guid3[2]);
    data.WriteByteSeq(guid2[7]);

    if (names)
        for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            data.WriteString(names->name[i]);

    data.WriteByteSeq(guid2[6]);
    data.WriteByteSeq(guid3[7]);
    data.WriteByteSeq(guid3[1]);
    data.WriteByteSeq(guid2[1]);
    data.WriteByteSeq(guid3[5]);
    data.WriteByteSeq(guid2[0]);

    SendPacket(&data);
}

void WorldSession::HandleNameQueryOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;

    uint8 bit14, bit1C;
    uint32 unk, unk1;

    guid[4] = recvData.ReadBit();
    bit14 = recvData.ReadBit();
    guid[6] = recvData.ReadBit();
    guid[0] = recvData.ReadBit();
    guid[7] = recvData.ReadBit();
    guid[1] = recvData.ReadBit();
    bit1C = recvData.ReadBit();
    guid[5] = recvData.ReadBit();
    guid[2] = recvData.ReadBit();
    guid[3] = recvData.ReadBit();

    recvData.ReadByteSeq(guid[7]);
    recvData.ReadByteSeq(guid[5]);
    recvData.ReadByteSeq(guid[1]);
    recvData.ReadByteSeq(guid[2]);
    recvData.ReadByteSeq(guid[6]);
    recvData.ReadByteSeq(guid[3]);
    recvData.ReadByteSeq(guid[0]);
    recvData.ReadByteSeq(guid[4]);

    // virtual and native realm Addresses

    if (bit14)
        recvData >> unk;

    if (bit1C)
        recvData >> unk1;

    // This is disable by default to prevent lots of console spam
    // TC_LOG_INFO("network", "HandleNameQueryOpcode %u", guid);

    SendNameQueryOpcode(guid);
}

void WorldSession::HandleQueryTimeOpcode(WorldPacket & /*recvData*/)
{
    SendQueryTimeResponse();
}

void WorldSession::SendQueryTimeResponse()
{
    WorldPacket data(SMSG_QUERY_TIME_RESPONSE, 4+4);
    data << uint32(time(NULL));
    data << uint32(sWorld->GetNextDailyQuestsResetTime() - time(NULL));
    SendPacket(&data);
}

/// Only _static_ data is sent in this packet !!!
void WorldSession::HandleCreatureQueryOpcode(WorldPacket & recvData)
{
    uint32 entry;
    recvData >> entry;
	
    WorldPacket data(SMSG_CREATURE_QUERY_RESPONSE, 500);
    CreatureTemplate const* info = sObjectMgr->GetCreatureTemplate(entry);

    data << uint32(entry);
    data.WriteBit(info != 0);                                    // Has data

    if (info)
    {
        std::string Name, SubName;
        Name = info->Name;
        SubName = info->SubName;

        int loc_idx = GetSessionDbLocaleIndex();
        if (loc_idx >= 0)
        {
            if (CreatureLocale const* cl = sObjectMgr->GetCreatureLocale(entry))
            {
                ObjectMgr::GetLocaleString(cl->Name, loc_idx, Name);
                ObjectMgr::GetLocaleString(cl->SubName, loc_idx, SubName);
            }
        }
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_CREATURE_QUERY '%s' - Entry: %u.", info->Name.c_str(), entry);
		
        data.WriteBits(SubName.length() ? SubName.length() + 1 : 0, 11);
        data.WriteBits(MAX_CREATURE_QUEST_ITEMS, 22);        // Quest items
        data.WriteBits(0, 11);

        for (int i = 0; i < 8; i++)
        {
            if (i == 0)
                data.WriteBits(Name.length() + 1, 11);
            else
                data.WriteBits(0, 11);                       // Name2, ..., name8
        }

        data.WriteBit(info->RacialLeader);
        data.WriteBits(info->IconName.length() + 1, 6);
        data.FlushBits();

        data << uint32(info->KillCredit[0]);                  // New in 3.1, kill credit
        data << uint32(info->Modelid4);                       // Modelid4
        data << uint32(info->Modelid2);                       // Modelid2
        data << uint32(info->expansion);                      // Expansion Required
        data << uint32(info->type);                           // CreatureType.dbc
        data << float(info->ModHealth);                       // Hp modifier
        data << uint32(info->type_flags2);                    // Flags2
        data << uint32(info->type_flags);                     // Flags
        data << uint32(info->rank);                           // Creature Rank (elite, boss, etc)
        data << uint32(info->movementId);                     // CreatureMovementInfo.dbc
        data << Name;

        if (SubName != "")
            data << SubName;                                // Subname

        data << uint32(info->Modelid1);                       // Modelid1
        data << uint32(info->Modelid3);                       // Modelid3

        if (info->IconName != "")
            data << info->IconName;                           // "Directions" for guard, string for Icons 2.3.0

        for (uint32 i = 0; i < MAX_CREATURE_QUEST_ITEMS; ++i)
            data << uint32(info->questItems[i]);              // ItemId[6], quest drop

        data << uint32(info->KillCredit[1]);                  // New in 3.1, kill credit
        data << float(info->ModMana);                         // Mana modifier
        data << uint32(info->family);                         // CreatureFamily.dbc
        SendPacket(&data);

        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Sent SMSG_CREATURE_QUERY_RESPONSE");
    }
    else
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_CREATURE_QUERY - NO CREATURE INFO! (ENTRY: %u)", entry);
}

/// Only _static_ data is sent in this packet !!!
void WorldSession::HandleGameObjectQueryOpcode(WorldPacket & recvData)
{
    uint32 entry;
    ObjectGuid guid;

    recvData >> entry;

    guid[5] = recvData.ReadBit();
    guid[3] = recvData.ReadBit();
    guid[6] = recvData.ReadBit();
    guid[2] = recvData.ReadBit();
    guid[7] = recvData.ReadBit();
    guid[1] = recvData.ReadBit();
    guid[0] = recvData.ReadBit();
    guid[4] = recvData.ReadBit();

    recvData.ReadByteSeq(guid[1]);
    recvData.ReadByteSeq(guid[5]);
    recvData.ReadByteSeq(guid[3]);
    recvData.ReadByteSeq(guid[4]);
    recvData.ReadByteSeq(guid[6]);
    recvData.ReadByteSeq(guid[2]);
    recvData.ReadByteSeq(guid[7]);
    recvData.ReadByteSeq(guid[0]);

	
    const GameObjectTemplate* info = sObjectMgr->GetGameObjectTemplate(entry);

    WorldPacket data (SMSG_GAMEOBJECT_QUERY_RESPONSE, 150);
    data.WriteBit(info != NULL);
    data << uint32(entry);

    size_t pos = data.wpos();
    data << uint32(0);

    if (info)
    {
        std::string Name;
        std::string IconName;
        std::string CastBarCaption;

        Name = info->name;
        IconName = info->IconName;
        CastBarCaption = info->castBarCaption;

        int loc_idx = GetSessionDbLocaleIndex();
        if (loc_idx >= 0)
        {
            if (GameObjectLocale const* gl = sObjectMgr->GetGameObjectLocale(entry))
            {
                ObjectMgr::GetLocaleString(gl->Name, loc_idx, Name);
                ObjectMgr::GetLocaleString(gl->CastBarCaption, loc_idx, CastBarCaption);
            }
        }

        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_GAMEOBJECT_QUERY '%s' - Entry: %u. ", info->name.c_str(), entry);
		
        data << uint32(info->type);
        data << uint32(info->displayId);
        data << Name;
        data << uint8(0) << uint8(0) << uint8(0);           // name2, name3, name4
        data << IconName;                                   // 2.0.3, string. Icon name to use instead of default icon for go's (ex: "Attack" makes sword)
        data << CastBarCaption;                             // 2.0.3, string. Text will appear in Cast Bar when using GO (ex: "Collecting")
        data << info->unk1;                                 // 2.0.3, string

        data.append(info->raw.data, MAX_GAMEOBJECT_DATA);
        data << float(info->size);                          // go size

        data << uint8(MAX_GAMEOBJECT_QUEST_ITEMS);

        for (uint32 i = 0; i < MAX_GAMEOBJECT_QUEST_ITEMS; ++i)
            data << uint32(info->questItems[i]);            // itemId[6], quest drop

        data << int32(info->unkInt32);                      // 4.x, unknown

        data.put(pos, uint32(data.wpos() - (pos + 4)));
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Sent SMSG_GAMEOBJECT_QUERY_RESPONSE");
    }
    else
    {
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_GAMEOBJECT_QUERY - Missing gameobject info for (GUID: %u, ENTRY: %u)",
            GUID_LOPART(guid), entry);
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Sent SMSG_GAMEOBJECT_QUERY_RESPONSE");
    }

    SendPacket(&data);
}

void WorldSession::HandleCorpseQueryOpcode(WorldPacket& /*recvData*/)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_CORPSE_QUERY");

    Corpse* corpse = GetPlayer()->GetCorpse();

    if (!corpse)
    {
        WorldPacket data(SMSG_CORPSE_QUERY);

        data << uint32(0);
        data << float(0);
        data << uint32(0);
        data << float(0);
        data << float(0);

        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);
        data.WriteBit(0);

        SendPacket(&data);
        return;
    }

    uint32 mapid = corpse->GetMapId();
    float x = corpse->GetPositionX();
    float y = corpse->GetPositionY();
    float z = corpse->GetPositionZ();
    uint32 corpsemapid = mapid;

    // if corpse at different map
    if (mapid != _player->GetMapId())
    {
        // search entrance map for proper show entrance
        if (MapEntry const* corpseMapEntry = sMapStore.LookupEntry(mapid))
        {
            if (corpseMapEntry->IsDungeon() && corpseMapEntry->entrance_map >= 0)
            {
                // if corpse map have entrance
                if (Map const* entranceMap = sMapMgr->CreateBaseMap(corpseMapEntry->entrance_map))
                {
                    mapid = corpseMapEntry->entrance_map;
                    x = corpseMapEntry->entrance_x;
                    y = corpseMapEntry->entrance_y;
                    z = entranceMap->GetHeight(GetPlayer()->GetPhaseMask(), x, y, MAX_HEIGHT);
                }
            }
        }
    }
    ObjectGuid guid = corpse->GetGUID();

	WorldPacket data(SMSG_CORPSE_QUERY, 9 + 1 + (4 * 5));


	data.WriteBit(guid[0]);
	data.WriteBit(guid[3]);
	data.WriteBit(guid[2]);
	data.WriteBit(1); // Corpse Found
	data.WriteBit(guid[5]);
	data.WriteBit(guid[4]);
	data.WriteBit(guid[1]);
	data.WriteBit(guid[7]);
	data.WriteBit(guid[6]);

	data.WriteByteSeq(guid[5]);
	data << float(z);
	data.WriteByteSeq(guid[1]);
	data << uint32(corpsemapid);
	data.WriteByteSeq(guid[6]);
	data.WriteByteSeq(guid[4]);
	data << float(x);
	data.WriteByteSeq(guid[3]);
	data.WriteByteSeq(guid[7]);
	data.WriteByteSeq(guid[2]);
	data.WriteByteSeq(guid[0]);
	data << int32(mapid);
	data << float(y);

    SendPacket(&data);
}

void WorldSession::HandleNpcTextQueryOpcode(WorldPacket & recvData)
{
    uint32 textID;
    ObjectGuid guid;

    recvData >> textID;
	
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: CMSG_NPC_TEXT_QUERY ID '%u'", textID);
	
    guid[4] = recvData.ReadBit();
    guid[5] = recvData.ReadBit();
    guid[1] = recvData.ReadBit();
    guid[7] = recvData.ReadBit();
    guid[0] = recvData.ReadBit();
    guid[2] = recvData.ReadBit();
    guid[6] = recvData.ReadBit();
    guid[3] = recvData.ReadBit();

    recvData.ReadByteSeq(guid[4]);
    recvData.ReadByteSeq(guid[0]);
    recvData.ReadByteSeq(guid[2]);
    recvData.ReadByteSeq(guid[5]);
    recvData.ReadByteSeq(guid[1]);
    recvData.ReadByteSeq(guid[7]);
    recvData.ReadByteSeq(guid[3]);
    recvData.ReadByteSeq(guid[6]);

    GetPlayer()->SetSelection(guid);

    GossipText const* pGossip = sObjectMgr->GetGossipText(textID);

    WorldPacket data(SMSG_NPC_TEXT_UPDATE, 1 + 4 + 64);
    data << textID;
    data << uint32(64);	/// Data size
	
    for (int i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; i++)
        data << float(pGossip ? pGossip->Options[i].Probability : 0);

    data << textID;                                     // should be a broadcast id

    for (int i = 0; i < MAX_GOSSIP_TEXT_OPTIONS - 1; i++)
        data << uint32(0);

    data.WriteBit(1);                                   // has data
    data.FlushBits();

    SendPacket(&data);

    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Sent SMSG_NPC_TEXT_UPDATE");
}

#define DEFAULT_GREETINGS_GOSSIP      68

void WorldSession::SendBroadcastTextDb2Reply(uint32 entry)
{
    ByteBuffer buffer;
    std::string defaultText = "Greetings, $n, how are you?";

    GossipText const* pGossip = sObjectMgr->GetGossipText(entry);
	
    uint16 nrmTextLength = pGossip ? pGossip->Options[0].Text_0.length() : defaultText.length();
    uint16 altTextLength = pGossip ? pGossip->Options[0].Text_1.length() : defaultText.length();

    buffer << uint32(entry);
    buffer << uint32(pGossip ? pGossip->Options[0].Language : 0);
    buffer << uint16(nrmTextLength);

    if (nrmTextLength)
        buffer << std::string(pGossip ? pGossip->Options[0].Text_0 : defaultText);

    buffer << uint16(altTextLength);

    if (altTextLength)
        buffer << std::string(pGossip ? pGossip->Options[0].Text_1 : defaultText);

    for (int i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; i++)
        buffer << uint32(0);

    buffer << uint32(1);

    WorldPacket data(SMSG_DB_REPLY);
    data << uint32(entry);
    data << uint32(0);
    data << uint32(DB2_REPLY_BROADCAST_TEXT);
    data << uint32(buffer.size());
    data.append(buffer);

    SendPacket(&data);
}

/// Only _static_ data is sent in this packet !!!
void WorldSession::HandlePageTextQueryOpcode(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_PAGE_TEXT_QUERY");
	ObjectGuid Guid;
    uint32 pageID;

    recvData >> pageID;

	uint8 bitOrder[8] = { 2, 1, 3, 7, 6, 4, 0, 5 };
	recvData.ReadBitInOrder(Guid, bitOrder);

	uint8 byteOrder[8] = { 0, 6, 3, 5, 1, 7, 4, 2 };
	recvData.ReadBytesSeq(Guid, byteOrder);

    while (pageID)
    {
        PageText const* pageText = sObjectMgr->GetPageText(pageID);
                                                            // guess size
        WorldPacket data(SMSG_PAGE_TEXT_QUERY_RESPONSE, 50);
        data << pageID;

        if (!pageText)
        {
            data << "Item page missing.";
            data << uint32(0);
            pageID = 0;
        }
        else
        {
            std::string Text = pageText->Text;

            int loc_idx = GetSessionDbLocaleIndex();
            if (loc_idx >= 0)
                if (PageTextLocale const* player = sObjectMgr->GetPageTextLocale(pageID))
                    ObjectMgr::GetLocaleString(player->Text, loc_idx, Text);

            data << Text;
            data << uint32(pageText->NextPage);
            pageID = pageText->NextPage;
        }
        SendPacket(&data);

        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Sent SMSG_PAGE_TEXT_QUERY_RESPONSE");
    }
}

void WorldSession::HandleCorpseMapPositionQuery(WorldPacket& recvData)
{
    sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Recv CMSG_CORPSE_MAP_POSITION_QUERY");

	uint32 transportGuidLow;
	recvData >> transportGuidLow;

    WorldPacket data(SMSG_CORPSE_MAP_POSITION_QUERY_RESPONSE, 4+4+4+4);
    data << float(0);
    data << float(0);
    data << float(0);
    data << float(0);
    SendPacket(&data);
}

void WorldSession::HandleQuestPOIQuery(WorldPacket& recvData)
{
    uint32 count = recvData.ReadBits(22); // quest count, max=25

    if (count >= MAX_QUEST_LOG_SIZE)
    {
        recvData.rfinish();
        return;
    }
	
    
    ByteBuffer poiData;

    WorldPacket data(SMSG_QUEST_POI_QUERY_RESPONSE, 4+(4+4)*count);
    data.WriteBits(count, 20);

    for (uint32 i = 0; i < count; ++i)
    {
        uint32 questId;
        recvData >> questId;

        bool questOk = false;

        uint16 questSlot = _player->FindQuestSlot(questId);

        if (questSlot != MAX_QUEST_LOG_SIZE)
            questOk =_player->GetQuestSlotQuestId(questSlot) == questId;

        if (questOk)
        {
            QuestPOIVector const* POI = sObjectMgr->GetQuestPOIVector(questId);

            if (POI)
            {
                data.WriteBits(POI->size(), 18);                // POI count bits

                for (QuestPOIVector::const_iterator itr = POI->begin(); itr != POI->end(); ++itr)
                {
                    data.WriteBits(itr->points.size(), 21);     // POI points count bits

                    poiData << uint32(itr->FloorId);            // floor id

                    for (std::vector<QuestPOIPoint>::const_iterator itr2 = itr->points.begin(); itr2 != itr->points.end(); ++itr2)
                    {
                        poiData << int32(itr2->x);              // POI point x
                        poiData << int32(itr2->y);              // POI point y
                    }

                    poiData << int32(itr->ObjectiveIndex);      // objective index 
                    poiData << uint32(itr->Id);                 // POI index
                    poiData << uint32(0);                       // unknown (new 5.x.x)
                    poiData << uint32(0);                       // unknown (new 5.x.x)
                    poiData << uint32(itr->MapId);              // mapid
                    poiData << uint32(itr->points.size());      // POI points count
                    poiData << uint32(itr->AreaId);             // areaid
                    poiData << uint32(0);                       // unknown (new 5.x.x)
                    poiData << uint32(itr->Unk4);               // unknown
                    poiData << uint32(itr->Unk3);               // unknown
                }

                poiData << uint32(questId);                     // quest ID
                poiData << uint32(POI->size());                 // POI count
            }
            else
            {
                poiData << uint32(questId);
                poiData << uint32(0);

                data.WriteBits(0, 18);
            }
        }
        else
        {
            poiData << uint32(questId);
            poiData << uint32(0);

            data.WriteBits(0, 18);
        }
    }

    poiData << uint32(count);

    data.FlushBits();
    data.append(poiData);

    SendPacket(&data);
}
