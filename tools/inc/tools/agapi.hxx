/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: agapi.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:07:51 $
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

#ifndef _AGAPI_HXX
#define _AGAPI_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class INetURLObject;
class ChannelList;
class AgentItem;
class String;

class AgentApi
{
protected:
    friend class ChannelList;

    AgentItem*          pChannelAgent;
    virtual BOOL        StartAgent() = 0;

    AgentApi(AgentItem* pAgent) { pChannelAgent = pAgent; }

public:
    virtual ~AgentApi() {}

    virtual void    InitAgent() = 0;
    virtual void    ShutDownAgent() = 0;

    virtual BOOL    NewDataPermission(const String& rChannelName) = 0;
    virtual void    NewData(const String& rChannelName,
                        const INetURLObject& rURL) = 0;
    virtual void    NotifyChannelObjFile(const INetURLObject& rURL,
                        const String& rFileName) = 0;
    virtual void    NotifyChannelObjData(const INetURLObject& rURL,
                        void* pBuffer, long nOffset, long nLen, long nTotalLen) = 0;

    virtual void    RegisterChannels() = 0;
    virtual void    RegisterUpdateTransmitter() = 0;
};

#endif //_AGAPI_HXX

