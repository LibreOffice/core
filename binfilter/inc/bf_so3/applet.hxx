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

#ifndef _APPLET_HXX
#define _APPLET_HXX

#include <bf_so3/ipobj.hxx>
#include <bf_svtools/ownlist.hxx>

#include "bf_so3/so3dllapi.h"
//=========================================================================
class INetURLObject;
class SjApplet;
class RmAppletObject;

namespace binfilter {

struct SvAppletData_Impl;

class SO3_DLLPUBLIC SvAppletObject : public SvInPlaceObject
/*	[Beschreibung]

    Die Klasse AppletObject ist die Abbildung eines InPlace-Objektes
    auf einen Applet-Server. Dieses Objekt kann nur InPlace- und
    nicht UI-Aktiviert werden.

    [Probleme]
    Es handelt sich bei diesem Objekt immer um einen Link. Dieser Link
    kann allerdings InPlace aktiviert werden.
*/
{
friend class SvAppletEnvironment;

private:
    SvAppletData_Impl *	pImpl;

    SO3_DLLPRIVATE void			DataChanged_Impl( BOOL bOnlyEmbedSource );

protected:
    SO3_DLLPRIVATE BOOL			StartApplet();
    SO3_DLLPRIVATE virtual void    FillClass( SvGlobalName * pClassName,
                               ULONG * pFormat,
                               String * pAppName,
                               String * pFullTypeName,
                               String * pShortTypeName,
                               long nFileFormat = SOFFICE_FILEFORMAT_CURRENT ) const;
                    // Protokoll
    SO3_DLLPRIVATE virtual void    Open( BOOL bOpen );
    SO3_DLLPRIVATE virtual void    InPlaceActivate( BOOL );
    SO3_DLLPRIVATE virtual ErrCode Verb( long, SvEmbeddedClient * pCallerClient,
                        Window * pWin, const Rectangle * pWorkRectPixel );

                    // Datenaustausch
    SO3_DLLPRIVATE virtual void    SetVisArea( const Rectangle & rVisArea );
    SO3_DLLPRIVATE virtual void    Draw( OutputDevice *,
                          const JobSetup & rSetup,
                          USHORT nAspect = ASPECT_CONTENT );

                    // Laden speichern
    SO3_DLLPRIVATE virtual BOOL    InitNew( SvStorage * );
    SO3_DLLPRIVATE virtual BOOL    Load( SvStorage * );
    SO3_DLLPRIVATE virtual BOOL    Save();
    SO3_DLLPRIVATE virtual BOOL    SaveAs( SvStorage * );
    SO3_DLLPRIVATE virtual void    HandsOff();
    SO3_DLLPRIVATE virtual BOOL    SaveCompleted( SvStorage * );

    SO3_DLLPRIVATE 				~SvAppletObject();
public:
                    SO2_DECL_BASIC_CLASS_DLL(SvAppletObject,SOAPP)
                    SvAppletObject();

    virtual ULONG	GetMiscStatus() const;
    virtual BOOL	IsLink() const;

                    // spezielle Internet Methoden
    void			SetCommandList( const SvCommandList & rList );
    const SvCommandList & GetCommandList() const;
    void			SetClass( const String & rClass );
    const String &	GetClass()const;
    void			SetName( const XubString & rClass );
    const XubString &GetName() const;
    void			SetCodeBase( const XubString & );
    const XubString & GetCodeBase() const;
    void			SetMayScript( BOOL bMayScript );
    BOOL			IsMayScript() const;
};

SO2_DECL_IMPL_REF(SvAppletObject)

}

#endif // _Applet_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
