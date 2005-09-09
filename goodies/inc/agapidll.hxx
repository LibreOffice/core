/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: agapidll.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:10:33 $
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

#ifndef _AGAPIDLL_HXX
#define _AGAPIDLL_HXX

#include "tools/agapi.hxx"
#include "agsdkdll.hxx"

class ChannelList;
class ChannelAgentItem;
class INetURLObject;
class Library;

class AgentApiDll : public AgentApi
{
    Library*                pAgentDll;
    ChannelApiFncs          aChannelApiFncs;

    FncInitAgent                    fncInitAgent;
    FncShutDownAgent                fncShutDownAgent;
    FncNewDataPermission            fncNewDataPermission;
    FncNewData                      fncNewData;
    FncNotifyChannelObjFile         fncNotifyChannelObjFile;
    FncNotifyChannelObjData         fncNotifyChannelObjData;
    FncRegisterChannels             fncRegisterChannels;
    FncRegisterUpdateTransmitter    fncRegisterUpdateTransmitter;

protected:
    friend class ChannelList;
    virtual BOOL            StartAgent();

public:
    AgentApiDll(ChannelAgentItem* pAgent);
    ~AgentApiDll();

    virtual void    InitAgent();
    virtual void    ShutDownAgent();

    virtual BOOL    NewDataPermission(const String& rChannelName);
    virtual void    NewData(const String& rChannelName, const INetURLObject& rURL);

    virtual void    NotifyChannelObjFile(const INetURLObject& rURL,
                        const String& rFileName);
    virtual void    NotifyChannelObjData(const INetURLObject& rURL,
                        void* pBuffer, long nOffset, long nLen, long nTotalLen);

    virtual void    RegisterChannels();
    virtual void    RegisterUpdateTransmitter();
};

#endif //_AGAPIDLL_HXX

