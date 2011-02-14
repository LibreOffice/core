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

#ifndef _SVX_XFLGRIT_HXX
#define _SVX_XFLGRIT_HXX

#include <svx/xit.hxx>
#include <svx/xgrad.hxx>
#include "svx/svxdllapi.h"

class SdrModel;

//------------------------
// class XFillGradientItem
//------------------------
class SVX_DLLPUBLIC XFillGradientItem : public NameOrIndex
{
    XGradient   aGradient;

public:
            TYPEINFO();
            XFillGradientItem() : NameOrIndex(XATTR_FILLGRADIENT, -1) {}
            XFillGradientItem(sal_Int32 nIndex, const XGradient& rTheGradient);
            XFillGradientItem(const UniString& rName, const XGradient& rTheGradient);
            XFillGradientItem(SfxItemPool* pPool, const XGradient& rTheGradient);
            XFillGradientItem(SfxItemPool* pPool );
            XFillGradientItem(const XFillGradientItem& rItem);
            XFillGradientItem(SvStream& rIn, sal_uInt16 nVer = 0);

    virtual int             operator==(const SfxPoolItem& rItem) const;
    virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem*    Create(SvStream& rIn, sal_uInt16 nVer) const;
    virtual SvStream&       Store(SvStream& rOut, sal_uInt16 nItemVersion ) const;
    virtual sal_uInt16          GetVersion( sal_uInt16 nFileFormatVersion ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;
    const XGradient&        GetGradientValue(const XGradientTable* pTable = 0) const; // GetValue -> GetGradientValue
    void                    SetGradientValue(const XGradient& rNew) { aGradient = rNew; Detach(); } // SetValue -> SetGradientValue

    static sal_Bool CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 );
    XFillGradientItem* checkForUniqueItem( SdrModel* pModel ) const;
};

#endif

