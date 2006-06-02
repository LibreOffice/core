/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: protitem.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2006-06-02 12:52:51 $
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
#ifndef _SVX_PROTITEM_HXX
#define _SVX_PROTITEM_HXX

// include ---------------------------------------------------------------


#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxProtectItem --------------------------------------------------


/*
[Beschreibung]
Dieses Item beschreibt, ob Inhalt, Groesse oder Position geschuetzt werden
sollen.
*/

class SVX_DLLPUBLIC SvxProtectItem : public SfxPoolItem
{
    BOOL bCntnt :1;     //Inhalt geschuetzt
    BOOL bSize  :1;     //Groesse geschuetzt
    BOOL bPos   :1;     //Position geschuetzt

public:
    TYPEINFO();

    inline SvxProtectItem( const USHORT nId = ITEMID_PROTECT );
    inline SvxProtectItem &operator=( const SvxProtectItem &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;


    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, USHORT) const;
    virtual SvStream&        Store(SvStream &, USHORT nItemVersion ) const;

    BOOL IsCntntProtected() const { return bCntnt; }
    BOOL IsSizeProtected()  const { return bSize;  }
    BOOL IsPosProtected()   const { return bPos;   }
    void SetCntntProtect( BOOL bNew ) { bCntnt = bNew; }
    void SetSizeProtect ( BOOL bNew ) { bSize  = bNew; }
    void SetPosProtect  ( BOOL bNew ) { bPos   = bNew; }

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
};

inline SvxProtectItem::SvxProtectItem( const USHORT nId )
    : SfxPoolItem( nId )
{
    bCntnt = bSize = bPos = FALSE;
}

inline SvxProtectItem &SvxProtectItem::operator=( const SvxProtectItem &rCpy )
{
    bCntnt = rCpy.IsCntntProtected();
    bSize  = rCpy.IsSizeProtected();
    bPos   = rCpy.IsPosProtected();
    return *this;
}




#endif

