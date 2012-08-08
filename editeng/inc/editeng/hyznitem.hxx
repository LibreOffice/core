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
#ifndef _SVX_HYZNITEM_HXX
#define _SVX_HYZNITEM_HXX

#include <svl/poolitem.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxHyphenZoneItem -----------------------------------------------

/*  [Description]

    This item describes a hyphenation attribute  (automatic?, number of
    characters at the end of the line and start).
*/

class EDITENG_DLLPUBLIC SvxHyphenZoneItem : public SfxPoolItem
{
    sal_Bool bHyphen:  1;
    sal_Bool bPageEnd: 1;
    sal_uInt8 nMinLead;
    sal_uInt8 nMinTrail;
    sal_uInt8 nMaxHyphens;

    friend SvStream & operator<<( SvStream & aS, SvxHyphenZoneItem & );

public:
    TYPEINFO();

    SvxHyphenZoneItem( const sal_Bool bHyph /*= sal_False*/,
                       const sal_uInt16 nId  );

    // "pure virtual Methods" from SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion ) const;

    inline void SetHyphen( const sal_Bool bNew ) { bHyphen = bNew; }
    inline sal_Bool IsHyphen() const { return bHyphen; }

    inline void SetPageEnd( const sal_Bool bNew ) { bPageEnd = bNew; }
    inline sal_Bool IsPageEnd() const { return bPageEnd; }

    inline sal_uInt8 &GetMinLead() { return nMinLead; }
    inline sal_uInt8 GetMinLead() const { return nMinLead; }

    inline sal_uInt8 &GetMinTrail() { return nMinTrail; }
    inline sal_uInt8 GetMinTrail() const { return nMinTrail; }

    inline sal_uInt8 &GetMaxHyphens() { return nMaxHyphens; }
    inline sal_uInt8 GetMaxHyphens() const { return nMaxHyphens; }

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
