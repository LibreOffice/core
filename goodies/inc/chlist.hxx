/*************************************************************************
 *
 *  $RCSfile: chlist.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CHLIST_HXX
#define _CHLIST_HXX

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _DATETIME_HXX //autogen
#include <tools/datetime.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
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
#if SUPD >= 380
#define _TOOLS_STRINGLIST
#ifndef _SSTRING_HXX
#include <tools/sstring.hxx>
#endif
#else
DECLARE_LIST(StringList, String*)
#endif
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

