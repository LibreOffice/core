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

#ifndef _PROTOCOL_HXX
#define _PROTOCOL_HXX

#ifndef _ERRCODE_HXX //autogen
#include <tools/errcode.hxx>
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _TOOLS_STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef INCLUDED_SO3DLLAPI_H
#include "bf_so3/so3dllapi.h"
#endif

/*************************************************************************
*************************************************************************/
class  String;

namespace binfilter {

class  SvEmbeddedObject;
class  SvInPlaceObject;
class  SvEmbeddedClient;
class  SvInPlaceClient;
class  SoDll;
class  ImplSvEditObjectProtocol;

class SO3_DLLPUBLIC SvEditObjectProtocol
{
friend class SoDll;
private:
    ImplSvEditObjectProtocol * pImp;
    SO3_DLLPRIVATE static void         Imp_DeleteDefault();
    SO3_DLLPRIVATE BOOL                Release();
public:
    SvEditObjectProtocol();
    SvEditObjectProtocol( const SvEditObjectProtocol & );
    SO3_DLLPRIVATE SvEditObjectProtocol & operator = ( const SvEditObjectProtocol & );
    ~SvEditObjectProtocol();

    // Objekte
    SvEmbeddedObject * GetObj() const;
    SvInPlaceObject *  GetIPObj() const;
    SvEmbeddedClient * GetClient() const;
    SvInPlaceClient *  GetIPClient() const;

    // Zustandsabfragen
    BOOL IsConnect() const;
    BOOL IsOpen() const;
    BOOL IsEmbed() const;
    BOOL IsPlugIn() const;
    BOOL IsInPlaceActive() const;
    BOOL IsUIActive() const;

    // Protokolle
    ErrCode EmbedProtocol();
    ErrCode	PlugInProtocol();
    ErrCode	IPProtocol();
    ErrCode UIProtocol();

    // Protokoll zurueckfahren
    BOOL Reset();
    BOOL Reset2Connect();
    BOOL Reset2Open();
    BOOL Reset2Embed();
    BOOL Reset2PlugIn();
    BOOL Reset2InPlaceActive();
    BOOL Reset2UIActive();

    // Einzelschritte der Protokolle
    void Opened( BOOL bOpen );
    void Embedded( BOOL bEmbed );
    void PlugIn( BOOL bPlugIn );
    void InPlaceActivate( BOOL bActivate );
    void UIActivate( BOOL bActivate );
    void SetInClosed( BOOL bInClosed );
};

}

#endif // _PROTOCOL_HXX
