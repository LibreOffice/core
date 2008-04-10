/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: channel.hxx,v $
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

#ifndef _CHANNEL_HXX
#define _CHANNEL_HXX

#include <tools/list.hxx>
#include <tools/urlobj.hxx>

class ChannelItem;
DECLARE_LIST(ChannelItemList,ChannelItem*)

class ChannelItem
{
    INetURLObject       aTransmitter;
    String              aChannel;
    String              aChannelName;
    String              aChannelAgentName;
    ULONG               nUpdatePeriode;
    ULONG               nLastTimerTick;
    ULONG               nLastSuccUpdDate;
    long                nLastSuccUpdTime;

    BOOL                bAgentUpdChannel;

public:
    ChannelItem();
    ~ChannelItem();

    // Set / Get Transmitter
    void                    SetTransmitter(const INetURLObject& rNew)
                            { aTransmitter = rNew; }
    const INetURLObject&    GetTransmitter() const
                            { return aTransmitter; }

    // Set / Get Channel
    void                    SetChannel(const String& rNew)
                            { aChannel = rNew; }
    const String&           GetChannel() const
                            { return aChannel; }

    // Set / Get ChannelName
    void                    SetChannelName(const String& rNew)
                            { aChannelName = rNew; }
    const String&           GetChannelName() const
                            { return aChannelName; }

    // Set / Get ChannelAgentName
    void                    SetChannelAgentName(const String& rNew)
                            { aChannelAgentName = rNew; }
    const String&           GetChannelAgentName() const
                            { return aChannelAgentName; }

    // Set / Get UpdatePeriode
    void                    SetUpdatePeriode(ULONG nNew)
                            { nUpdatePeriode = nNew; }
    ULONG                   GetUpdatePeriode() const
                            { return nUpdatePeriode; }

    // Set / Get LastTimerTick
    void                    SetLastTimerTick(ULONG nNew)
                            { nLastTimerTick = nNew; }
    ULONG                   GetLastTimerTick() const
                            { return nLastTimerTick; }

    // Set / Get LastSuccUpdDate
    void                    SetLastSuccUpdDate(ULONG nNew)
                            { nLastSuccUpdDate = nNew; }
    ULONG                   GetLastSuccUpdDate() const
                            { return nLastSuccUpdDate; }

    // Set / Get LastSuccUpdTime
    void                    SetLastSuccUpdTime(long nNew)
                            { nLastSuccUpdTime = nNew; }
    long                    GetLastSuccUpdTime() const
                            { return nLastSuccUpdTime; }

    // Set / Get AgentUpdChannel
    void                    SetAgentUpdChannel(BOOL bNew)
                            { bAgentUpdChannel = bNew; }
    BOOL                    GetAgentUpdChannel() const
                            { return bAgentUpdChannel; }
};

#endif // _CHANNEL_HXX

