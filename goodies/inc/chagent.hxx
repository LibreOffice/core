/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: chagent.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _CHAGENT_HXX
#define _CHAGENT_HXX

#include <tools/list.hxx>
#include <tools/urlobj.hxx>

#include <tools/agitem.hxx>

class ChannelAgentItem;
class ChannelItem;

DECLARE_LIST(ChannelAgentItemList,ChannelAgentItem*)

enum AgentType
{
    AG_TYP_DONTKNOW =   0,
    AG_TYP_DLL      =   1,
    AG_TYP_BASIC    =   2,
    AG_TYP_JAVA     =   3
};

#define     STR_AG_TYP_DLL      "DLL"
#define     STR_AG_TYP_BASIC    "BASIC"
#define     STR_AG_TYP_JAVA     "JAVA"

class ChannelAgentItem : public AgentItem
{
    AgentType           eAgentType;
    String              aChAgentName;
    INetURLObject       aLocation;
    ULONG               nLastSuccUpdDate;
    long                nLastSuccUpdTime;
    INetURLObject       aSource;
    BOOL                bEnabled;
    DateTime            aLastCheckDate;

    // runtime
    BOOL                bIsActive;
    BOOL                bIsInUpdate;
    BOOL                bRestartAfterUpdate;

    ULONG               nChObjDate;
    long                nChObjTime;
    ChannelItem*        pActChannel;
    AgentApi*           pAgentApi;
    ChannelApi*         pChannelApi;
    INetURLObject       aSourceURL;

    Link                aBTXShutdownCallback;

protected:
    friend class ChannelApi;
    void                DeleteApis();

public:
    ChannelAgentItem();
    ~ChannelAgentItem();

    // Set / Get AgentType
    void                    SetChannelAgentType(AgentType eNew)
                            { eAgentType = eNew; }
    AgentType               GetChannelAgentType() const
                            { return eAgentType; }

    // Set / Get ChAgentName
    void                    SetChAgentName(const String& rNew)
                            { aChAgentName = rNew; }
    virtual const String&   GetChAgentName() const
                            { return aChAgentName; }

    // Set / Get Location
    void                    SetLocation(const INetURLObject& rNew)
                            { aLocation = rNew; }
    virtual const INetURLObject& GetLocation() const
                            { return aLocation; }

    // Set / Get LastSuccUpdDate
    void                    SetLastSuccUpdDate(ULONG nNew)
                            { nLastSuccUpdDate = nNew; }
    ULONG                   GetLastSuccUpdDate() const
                            { return nLastSuccUpdDate; }

    // Set / Get LastSuccUpdTime
    void                    SetLastSuccUpdTime(ULONG nNew)
                            { nLastSuccUpdTime = nNew; }
    ULONG                   GetLastSuccUpdTime() const
                            { return nLastSuccUpdTime; }

    // Set / Get LastCheckDate
    void                    SetLastCheckDate( const DateTime& rNew )
                            { aLastCheckDate = rNew; }
    const DateTime&         GetLastCheckDate()
                            { return aLastCheckDate; }

    // Set / Get Source
    void                    SetSource(const INetURLObject& rNew)
                            { aSource = rNew; }
    const INetURLObject&    GetSource() const
                            { return aSource; }

    // Set / Get Enabled
    virtual void            SetEnabled(BOOL bNew = TRUE)
                            { bEnabled = bNew; }
    virtual BOOL            IsEnabled() const
                            { return bEnabled; }

    // Set / Get IsActive
    virtual void            SetIsActive(BOOL bNew)
                            { bIsActive = bNew; }
    virtual BOOL            IsActive() const
                            { return bIsActive; }

    // runtime ////////////////////////////////////////////////////////////////

    const INetURLObject&    GetAgentSourceURL() const
                            { return aSourceURL; }
    void                    SetAgentSourceURL(const INetURLObject& rNew)
                            { aSourceURL = rNew; }

    // Set / Get ActiveChannel
    void                    SetActiveChannel(ChannelItem* pChItem)
                            { pActChannel = pChItem; }
    ChannelItem*            GetActiveChannel() const
                            { return pActChannel; }

    // Set / Get IsInUpdate
    void                    SetIsInUpdate( BOOL bNew )
                            { bIsInUpdate = bNew; }
    BOOL                    IsInUpdate() const
                            { return bIsInUpdate; }

    // Set / Get RestartAfterUpdate
    void                    SetRestartAfterUpdate( BOOL bNew )
                            { bRestartAfterUpdate = bNew; }
    BOOL                    RestartAfterUpdate() const
                            { return bRestartAfterUpdate; }

    // AgentAPI
    void                    SetApi(AgentApi* pNew )
                            { pAgentApi = pNew; }
    virtual AgentApi*       GetApi() const
                            { return pAgentApi; }

    // ChannelAPI
    void                    SetChApi(ChannelApi* pNew )
                            { pChannelApi = pNew; }
    virtual ChApi*          GetChApi() const
                            { return (ChApi*)pChannelApi; }

    // Set / Get ChObjDate
    void                    SetChObjDate(ULONG nNew)
                            { nChObjDate = nNew; }
    ULONG                   GetChObjDate() const
                            { return nChObjDate; }

    // Set / Get ChObjTime
    void                    SetChObjTime(long nNew)
                            { nChObjTime = nNew; }
    long                    GetChObjTime() const
                            { return nChObjTime; }

    void                    SetBTXShutDownCallBack(const Link& rLnk )
                            { aBTXShutdownCallback = rLnk; }
    const Link&             GetBTXShutDownCallBack()
                            { return aBTXShutdownCallback; }

};

#endif // _CHAGENT_HXX
