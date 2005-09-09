/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chanapi.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:15:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CHANAPI_HXX
#define _CHANAPI_HXX


#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
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
