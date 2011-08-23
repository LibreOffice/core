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
#ifndef _SVX_BOXOBJ_HXX
#define _SVX_BOXOBJ_HXX

#include <bf_sfx2/shobj.hxx>
namespace binfilter {
class SvxBoxItem;
class SvxBoxInfoItem;
class SvxBorderLine;

//-------------------------------------------------------------------------

class SvxBoxObject: public SfxShellSubObject

/*	[Beschreibung]

    Diese SbxObject-Subklasse kann das <SvxBoxItem> und <SvxBoxInfoItem>
    im SBX-System recorden und wieder abspielen. Hier werden keine
    G"ultigkeitspr"ufungen durchgef"uhrt, diese m"ussen in abgeleiteten
    Klassen spezifisch implementiert werden; dazu sind virtuelle Methoden
    vorhanden.
*/

{
friend class SvxBoxLineObject_Impl;

    SvxBoxItem* 			pBox;
    SvxBoxInfoItem* 		pBoxInfo;

#ifdef _SVX_BOXOBJ_CXX
private:
    const SvxBorderLine*	GetLine_Impl( USHORT nSID ) const;
    BOOL					SetLine_Impl( USHORT nSID, const SvxBorderLine *pLine );
    void					Update_Impl() const;
#endif

protected:
    // Schnittstelle zum SbxObject/SvDispatch/SfxShell..Object
    void					SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                    const SfxHint& rHint, const TypeId& rHintType );
    virtual ULONG			SetProperty( USHORT nSID, SbxVariable &rRet );
    virtual ULONG			GetProperty( USHORT nSID, SbxVariable &rRet );

    // bei den folgenden Methoden sind die Items bereits besorgt worden
    virtual SbxError    	GetLineColor( USHORT nSID, Color &rColor ) const;
    virtual SbxError        GetLineStyle( USHORT nSID,
                                          USHORT &rOutWidth, USHORT &rDist, USHORT &rInWidth ) const;
    virtual SbxError		GetDistance( USHORT &rDist ) const;

    // nach den folgenden Methoden werden die Items wieder in die Shell gesetzt
    virtual SbxError		SetLineColor( USHORT nSID, const Color &rColor );
    virtual SbxError		SetLineStyle( USHORT nSID,
                                          USHORT nOutWidth, USHORT nDist, USHORT nInWidth );
    virtual SbxError        SetDistance( USHORT nDist );

public:
                            TYPEINFO();
                            SvxBoxObject( SfxShell *pShell = 0 );

                            ~SvxBoxObject();

    const SvxBoxItem&		GetBoxItem() const { return *pBox; }
    const SvxBoxInfoItem&	GetBoxInfoItem() const { return *pBoxInfo; }

    virtual String 			GenerateSource( const String &rLinePrefix,
                                            const SbxObject *pRelativeTo );
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
