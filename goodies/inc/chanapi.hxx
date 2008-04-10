/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: chanapi.hxx,v $
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

#ifndef _CHANAPI_HXX
#define _CHANAPI_HXX


#include <tools/list.hxx>
#include <tools/chapi.hxx>

#ifndef _INETSES_HXX //autogen
#include <inet/inetsess.hxx>
#endif

class ChannelAgentItem;
class ChannelList;
class INetURLObject;
class GetChannelObj;

DECLARE_LIST(GetChObjList, GetChannelObj*);

class ChannelApi : public ChApi
{
    INetSessionRef          xINetSession;

    ChannelAgentItem*       pChAgent;
    ChannelList*            pChannelList;

    GetChObjList*           pGetChObjList;

    DECL_LINK(CloseAgentEvt, void*);
    DECL_LINK(CloseChannelObjEvt, void*);
    DECL_LINK(GetChannelObjNotify, GetChannelObj*);
public:
    ChannelApi( ChannelAgentItem* pAgent, INetSession* pISess,
                ChannelList* pChLst );
    ~ChannelApi();

    void    ShutDownAgent();
    void    SetLastSuccUpd();

    void    GetChannelObject( const INetURLObject& rURL, RequestType eStreamType,
                            const String& rFileName );
    void    AddChannelItem( const String& aChName, const INetURLObject& aTransmitter,
                            const String& aChannel, USHORT nUpdPeriode,
                            const String& rChAgentName );
    void    DelChannelItem( const String& aChName );
    void    SetChTransmitter( const String& aChName, const String& rNewVal );
    void    SetChannel( const String& aChName, const String& rNewVal );
    void    SetChannelName( const String& aChName, const String& rNewVal );
    void    SetChUpdPeriode( const String& aChName, USHORT nUpdPeriode );
    void    SetChannelAgentName( const String& aChName, const String& rNewVal );

    void    SetUpdateTransmitter(ChannelAgentItem* pAgent, const INetURLObject& rTransmitter);
};

#endif //_CHANAPI_HXX
