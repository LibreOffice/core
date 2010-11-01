/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _CHAPI_HXX
#define _CHAPI_HXX

#include <tools/solar.h>

class ChannelAgentItem;
class INetURLObject;
class String;

enum RequestType {
    REQTYP_LOCAL_FILE   = 1,
    REQTYP_MEMORY       = 2
};

class ChApi
{
public:
    virtual ~ChApi() {}

    virtual void    ShutDownAgent() = 0;
    virtual void    SetLastSuccUpd() = 0;

    virtual void    GetChannelObject( const INetURLObject& rURL, RequestType eStreamType,
                            const String& rFileName ) = 0;
    virtual void    AddChannelItem( const String& aChName, const INetURLObject& aTransmitter,
                            const String& aChannel, USHORT nUpdPeriode,
                            const String& rChAgentName ) = 0;
    virtual void    DelChannelItem( const String& aChName ) = 0;
    virtual void    SetChTransmitter( const String& aChName, const String& rNewVal ) = 0;
    virtual void    SetChannel( const String& aChName, const String& rNewVal ) = 0;
    virtual void    SetChannelName( const String& aChName, const String& rNewVal ) = 0;
    virtual void    SetChUpdPeriode( const String& aChName, USHORT nUpdPeriode ) = 0;
    virtual void    SetChannelAgentName( const String& aChName, const String& rNewVal ) = 0;

    virtual void    SetUpdateTransmitter(ChannelAgentItem* pAgent, const INetURLObject& rTransmitter) = 0;
};

////////////////////////////////////////////////////////////////////////////////
//

#endif //_CHAPI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
