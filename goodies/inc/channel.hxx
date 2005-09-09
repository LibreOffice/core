/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: channel.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:15:40 $
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

#ifndef _CHANNEL_HXX
#define _CHANNEL_HXX

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

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

