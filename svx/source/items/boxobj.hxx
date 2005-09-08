/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: boxobj.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:32:31 $
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
#ifndef _SVX_BOXOBJ_HXX
#define _SVX_BOXOBJ_HXX


#ifndef _SFX_SHOBJ_HXX //autogen
#include <sfx2/shobj.hxx>
#endif
class SvxBoxItem;
class SvxBoxInfoItem;
class SvxBorderLine;

//-------------------------------------------------------------------------

class SvxBoxObject: public SfxShellSubObject

/*  [Beschreibung]

    Diese SbxObject-Subklasse kann das <SvxBoxItem> und <SvxBoxInfoItem>
    im SBX-System recorden und wieder abspielen. Hier werden keine
    G"ultigkeitspr"ufungen durchgef"uhrt, diese m"ussen in abgeleiteten
    Klassen spezifisch implementiert werden; dazu sind virtuelle Methoden
    vorhanden.
*/

{
friend class SvxBoxLineObject_Impl;

    SvxBoxItem*             pBox;
    SvxBoxInfoItem*         pBoxInfo;

#ifdef _SVX_BOXOBJ_CXX
private:
    const SvxBorderLine*    GetLine_Impl( USHORT nSID ) const;
    BOOL                    SetLine_Impl( USHORT nSID, const SvxBorderLine *pLine );
    void                    Update_Impl() const;
#endif

protected:
    // Schnittstelle zum SbxObject/SvDispatch/SfxShell..Object
    void                    SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                    const SfxHint& rHint, const TypeId& rHintType );
    virtual ULONG           SetProperty( USHORT nSID, SbxVariable &rRet );
    virtual ULONG           GetProperty( USHORT nSID, SbxVariable &rRet );

    // bei den folgenden Methoden sind die Items bereits besorgt worden
    virtual SbxError        GetLineColor( USHORT nSID, Color &rColor ) const;
    virtual SbxError        GetLineStyle( USHORT nSID,
                                          USHORT &rOutWidth, USHORT &rDist, USHORT &rInWidth ) const;
    virtual SbxError        GetDistance( USHORT &rDist ) const;

    // nach den folgenden Methoden werden die Items wieder in die Shell gesetzt
    virtual SbxError        SetLineColor( USHORT nSID, const Color &rColor );
    virtual SbxError        SetLineStyle( USHORT nSID,
                                          USHORT nOutWidth, USHORT nDist, USHORT nInWidth );
    virtual SbxError        SetDistance( USHORT nDist );

public:
                            TYPEINFO();
                            SvxBoxObject( SfxShell *pShell = 0 );

                            ~SvxBoxObject();

    const SvxBoxItem&       GetBoxItem() const { return *pBox; }
    const SvxBoxInfoItem&   GetBoxInfoItem() const { return *pBoxInfo; }

    virtual String          GenerateSource( const String &rLinePrefix,
                                            const SbxObject *pRelativeTo );
};


#endif

