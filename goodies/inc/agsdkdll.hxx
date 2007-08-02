/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: agsdkdll.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-02 18:24:39 $
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

#ifndef _AGSDKDLL_HXX
#define _AGSDKDLL_HXX

#ifndef _SV_CALL
#if defined(WIN) || defined(WNT) || ( defined(OS2) && !defined( ICC ))
#define _SV_CALL    _cdecl
#elif defined( ICC ) && defined( OS2 )
#define _SV_CALL    __cdecl
#else
#define _SV_CALL
#endif
#endif

#define AGSDKDLL_VERSION        2

typedef void* AgentInst;

////////////////////////////////////////////////////////////////////////////////
//  Channel API Functions
//

enum AgentStreamType {
    AS_LOCAL_FILE   = 1,
    AS_MEMORY       = 2
};

typedef void (_SV_CALL *FncChShutDownAgent)(AgentInst pAg);

typedef void (_SV_CALL *FncChSetLastSuccUpd)(AgentInst pAg);

typedef void (_SV_CALL *FncChGetChannelObj)(AgentInst pAg, const char* pURL,
    AgentStreamType eStreamType, const char* pFileName);

typedef void (_SV_CALL *FncChAddChannelItem)(AgentInst pAg, const char* pName,
    const char* pTransmitter, const char* pChannel, unsigned short nUpdPeriode,
    const char* pAgentName );

typedef void (_SV_CALL *FncChDelChannelItem)(AgentInst pAg, const char* pChName);

typedef void (_SV_CALL *FncChSetTransmitter)(AgentInst pAg, const char* pChName,
    const char* pURL);

typedef void (_SV_CALL *FncChSetChannel)(AgentInst pAg, const char* pChName,
    const char* pRelURL);

typedef void (_SV_CALL *FncChSetChannelName)(AgentInst pAg, const char* pChName,
    const char* pNewChName);

typedef void (_SV_CALL *FncChSetUpdPeriode)(AgentInst pAg, const char* pChName,
    unsigned short nUpdPeriode);

typedef void (_SV_CALL *FncChSetChannelAgentName)(AgentInst pAg, const char* pChName,
    const char* pAgName);

typedef void (_SV_CALL *FncChSetUpdateTransmitter)(AgentInst pAg,
    const char* pTransmitter);

typedef const char* (_SV_CALL *FncChGetAgentSourceURL)(AgentInst pAg);

struct ChannelApiFncs
{
    unsigned short              nVersion;
    FncChShutDownAgent          fncShutDownAgent;
    FncChSetLastSuccUpd         fncSetLastSuccUpd;
    FncChGetChannelObj          fncGetChannelObj;
    FncChAddChannelItem         fncAddChannelItem;
    FncChDelChannelItem         fncDelChannelItem;
    FncChSetTransmitter         fncSetTransmitter;
    FncChSetChannel             fncSetChannel;
    FncChSetChannelName         fncSetChannelName;
    FncChSetUpdPeriode          fncSetUpdPeriode;
    FncChSetChannelAgentName    fncSetChannelAgentName;
    FncChSetUpdateTransmitter   fncSetUpdateTransmitter;
    FncChGetAgentSourceURL      fncGetAgentSourceURL;
};

////////////////////////////////////////////////////////////////////////////////
//  Agent API Functions
//

#if defined(WIN) || defined(WNT) || defined(OS2) || defined(UNX)
extern "C" {
#endif

typedef void (_SV_CALL *FncInitAgent)(AgentInst, ChannelApiFncs*);
void _SV_CALL InitAgent(AgentInst, ChannelApiFncs*);

typedef void (_SV_CALL *FncShutDownAgent)();
void _SV_CALL ShutDownAgent();

// NewDataPermission
// ChannelName
typedef unsigned char (_SV_CALL *FncNewDataPermission)(const char*);
unsigned char  _SV_CALL NewDataPermission(const char*);

// NewData
// ChannelName, URL
typedef void (_SV_CALL *FncNewData)(const char*, const char*);
void  _SV_CALL NewData(const char*, const char*);

//  NotifyChannelObjFile
//  URL, abs. Filename
typedef void (_SV_CALL *FncNotifyChannelObjFile)(const char*, const char*);
void  _SV_CALL NotifyChannelObjFile(const char*, const char*);

//  NotifyChannelObjData
//  URL, Buffer, Offset, Len, TotalLen
typedef void (_SV_CALL *FncNotifyChannelObjData)(const char*, void*, long, long, long);
void  _SV_CALL NotifyChannelObjData(const char*, void*, long, long, long);

typedef void (_SV_CALL *FncRegisterChannels)(AgentInst, ChannelApiFncs*);
void  _SV_CALL RegisterChannels(AgentInst, ChannelApiFncs*);

typedef void (_SV_CALL *FncRegisterUpdateTransmitter)(AgentInst, ChannelApiFncs*);
void  _SV_CALL RegisterUpdateTransmitter(AgentInst, ChannelApiFncs*);

#if defined(WIN) || defined(WNT) || defined(OS2) || defined(UNX)
}
#endif

#endif //_AGSDKDLL_HXX


