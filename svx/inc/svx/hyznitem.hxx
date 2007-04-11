/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hyznitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:57:24 $
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
#ifndef _SVX_HYZNITEM_HXX
#define _SVX_HYZNITEM_HXX

// include ---------------------------------------------------------------

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxHyphenZoneItem -----------------------------------------------

/*
[Beschreibung]
Dieses Item beschreibt ein Silbentrennungsattribut (Automatisch?, Anzahl der
Zeichen am Zeilenende und -anfang).
*/

class SVX_DLLPUBLIC SvxHyphenZoneItem : public SfxPoolItem
{
    BOOL bHyphen:  1;
    BOOL bPageEnd: 1;
    BYTE nMinLead;
    BYTE nMinTrail;
    BYTE nMaxHyphens;

    friend SvStream & operator<<( SvStream & aS, SvxHyphenZoneItem & );

public:
    TYPEINFO();

    SvxHyphenZoneItem( const BOOL bHyph = FALSE,
                       const USHORT nId = ITEMID_HYPHENZONE );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, USHORT) const;
    virtual SvStream&        Store(SvStream &, USHORT nItemVersion ) const;

    inline void SetHyphen( const BOOL bNew ) { bHyphen = bNew; }
    inline BOOL IsHyphen() const { return bHyphen; }

    inline void SetPageEnd( const BOOL bNew ) { bPageEnd = bNew; }
    inline BOOL IsPageEnd() const { return bPageEnd; }

    inline BYTE &GetMinLead() { return nMinLead; }
    inline BYTE GetMinLead() const { return nMinLead; }

    inline BYTE &GetMinTrail() { return nMinTrail; }
    inline BYTE GetMinTrail() const { return nMinTrail; }

    inline BYTE &GetMaxHyphens() { return nMaxHyphens; }
    inline BYTE GetMaxHyphens() const { return nMaxHyphens; }

    inline SvxHyphenZoneItem &operator=( const SvxHyphenZoneItem &rNew )
    {
        bHyphen = rNew.IsHyphen();
        bPageEnd = rNew.IsPageEnd();
        nMinLead = rNew.GetMinLead();
        nMinTrail = rNew.GetMinTrail();
        nMaxHyphens = rNew.GetMaxHyphens();
        return *this;
    }
};

#endif

