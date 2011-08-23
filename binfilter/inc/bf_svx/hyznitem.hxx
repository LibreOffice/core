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
#ifndef _SVX_HYZNITEM_HXX
#define _SVX_HYZNITEM_HXX

// include ---------------------------------------------------------------

#ifndef _SFXPOOLITEM_HXX //autogen
#include <bf_svtools/poolitem.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif

namespace rtl
{
    class OUString;
}
namespace binfilter {
class SvXMLUnitConverter;
// class SvxHyphenZoneItem -----------------------------------------------

/*
[Beschreibung]
Dieses Item beschreibt ein Silbentrennungsattribut (Automatisch?, Anzahl der
Zeichen am Zeilenende und -anfang).
*/

class SvxHyphenZoneItem : public SfxPoolItem
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
    virtual int 			 operator==( const SfxPoolItem& ) const;
    virtual	sal_Bool        	 QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	sal_Bool			 PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );


    virtual SfxPoolItem*	 Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*	 Create(SvStream &, USHORT) const;
    virtual SvStream&		 Store(SvStream &, USHORT nItemVersion ) const;

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

}//end of namespace binfilter
#endif

