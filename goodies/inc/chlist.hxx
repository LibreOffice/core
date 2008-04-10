/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: chlist.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _CHLIST_HXX
#define _CHLIST_HXX

#include <tools/urlobj.hxx>
#include <tools/datetime.hxx>
#include <tools/string.hxx>
#include <vcl/timer.hxx>
#include <tools/list.hxx>
#ifndef _INETSES_HXX //autogen
#include <inet/inetsess.hxx>
#endif

class Downloader;
class ChannelAgentItem;
class ChannelItem;
class ChannelItemList;
class ChannelAgentItemList;
class ChannelTuner;
class ChannelApi;
class GetChannelObj;
class GetChObjList;
class NotifyItem;
class Date;
class Time;
class ResMgr;

struct UiAgentItem
{
    String      aName;
    ULONG       aUpdPeriode;
    DateTime    aLastDataDate;
    DateTime    aLastUpdDate;
    BOOL        bUnregister;
    BOOL        bIsEnabled;
};

DECLARE_LIST(UiAgentList, UiAgentItem*)
DECLARE_LIST(NotifyList, NotifyItem*)

#ifndef STRING_LIST
#define STRING_LIST
#define _TOOLS_STRINGLIST
#include <tools/sstring.hxx>
#endif

///////////////////////////////////////////////////////////////////////////////
//
//
#define JOBID_DOWNLOAD_AGENT        1
#define JOBID_UPDATE_AGENT          2
#define JOBID_CHECK_TRANSMITTER     3

struct BTXResponse
{
    USHORT  nJobId;
    BOOL    bFinish;
    ULONG   nTransferStatus;
    String  aLocalFileName;
};

///////////////////////////////////////////////////////////////////////////////
//
//
class ChannelList
{
    ResMgr*                     pResMgr;
    ChannelAgentItemList*       pAgentList;
    ChannelItemList*            pChannelList;
    NotifyList*                 pNotifyList;
    GetChObjList*               pGetChObjList;
    UiAgentList*                pUiAgentList;

    ChannelTuner*               pChannelTuner;

    String                      aAgentPath;
    AutoTimer                   aTimer;
    INetSessionRef              xINetSession;

    Link                        aNotifyPermissionHdl;
    Link                        aCheckFinishHdl;
    // wird von der ChannelList gehandelt; sollte ausgebaut werden; auch im SFX
    Link                        aAgentUpdateHdl;
    Downloader*                 pBTXDecoder;

    void                        MakeGodChannel(ChannelAgentItem* pAgent);
    void                        DeleteUiAgentList();

    BOOL                        StartAgent(ChannelAgentItem* pAgent);

    void                        DoNotifyAgents();
    void                        NotifyAgent(ChannelAgentItem* pAgent, ULONG nNotifyItemIdx);

    void                        UpdateAgent(ChannelAgentItem* pAgent, ULONG nNotifyItemIdx);
    void                        SendItemToTuner(ChannelItem* pItem);

    BOOL                        ParseDateTime(const String& rSrc, Date& rDate, Time& rTime);

    void                        UnZipAgent(ChannelAgentItem* pAgent);
    static void                 EnumFilesCallBack(char *pFile, void *pObject);

    DECL_LINK(CheckChannels, void*);
    DECL_LINK(CloseChannelObjEvt, void*);
    DECL_LINK(GetChannelObjNotify, GetChannelObj*);
protected:
    friend class ChannelApi;
    friend class ChannelTuner;

    ChannelItem*                GetChannelItemByName(const String& rName);
    ChannelAgentItem*           GetAgentItemByName(const String& rName);

    void                        ReadChannelList();
    void                        ReadAgentList();
    void                        WriteChannelItem(ChannelItem* pItem);
    void                        WriteAgentItem(ChannelAgentItem* pChAgent);
    void                        SetLastSuccUpdChannel(ChannelAgentItem* pChAgent);
    void                        SetLastSuccUpdAgent(ChannelAgentItem* pChAgent);
    void                        SetEnabledAgent(ChannelAgentItem* pChAgent);

    void                        DeleteAgentItem(ChannelAgentItem* pItem);
    void                        DeleteChannelItem(ChannelItem* pItem);
    void                        RegisterNewChannelItem(ChannelItem* pItem);

    void                        CancelBTX();
    Downloader*                 GetBTXDecoder() const { return pBTXDecoder; }
    ResMgr*                     GetResMgr() const { return pResMgr; }

    DECL_LINK(CheckBTXFinishHdl, void*);

    void                        NotifyTransmitterMsg(const String& rStr);

public:
    ChannelList(const String& rAgentPath);
    ~ChannelList();

    void                SubscribeNewAgent(const INetURLObject& rURL);

    void                SetINetSession(INetSession* pNewSess);
    void                SetBTXDecoder(Downloader* pDecoder);

    void                SetNotifyPermissionHdl(const Link& rHdl) { aNotifyPermissionHdl = rHdl; }
    void                SetAgentUpdateHdl(const Link& rHdl) { aAgentUpdateHdl = rHdl; }

    void                SetTimerInterval(ULONG nNew);
    void                StartCheckChannels();
    void                StopCheckChannels();

    void                CheckAgentNow(const String& rAgName, BOOL bCheckGOD = FALSE);
    void                SetCheckFinishHdl(const Link& rHdl) { aCheckFinishHdl = rHdl; }
    const Link&         GetCheckFinishHdl() const { return aCheckFinishHdl;};

    UiAgentList*        GetUiAgentList();
    void                JoinUiAgentList();
};

///////////////////////////////////////////////////////////////////////////////
//
//
class NotifyItem
{
    ChannelItem*    pChItem;

    USHORT          nGMTOffset;
    ULONG           nDateLastUpd;
    long            nTimeLastUpd;
    INetURLObject   aChObjURL;
    String          aChName;
public:
    NotifyItem( ChannelItem* pItem, short nGMTOff, ULONG nLastDate,
                ULONG nLastTime, const INetURLObject& rChObjURL,
                const String& rChName);
    ~NotifyItem();

    ChannelItem*        GetChannelItem() const          { return pChItem;       }

    const String&       GetChannelName() const          { return aChName;       }
    const INetURLObject GetChannelObjURL() const        { return aChObjURL;     }

    void                SetDateLastUpd(ULONG nNew)      { nDateLastUpd = nNew;  }
    ULONG               GetDateLastUpd() const          { return nDateLastUpd;  }

    void                SetTimeLastUpd(long nNew)       { nTimeLastUpd = nNew;  }
    long                GetTimeLastUpd() const          { return nTimeLastUpd;  }

    const String&       GetChAgentName() const;

};

#endif // _CHLIST_HXX

