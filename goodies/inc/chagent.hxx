/*************************************************************************
 *
 *  $RCSfile: chagent.hxx,v $
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

#ifndef _CHAGENT_HXX
#define _CHAGENT_HXX

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

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
