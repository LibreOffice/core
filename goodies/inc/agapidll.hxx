/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: agapidll.hxx,v $
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

