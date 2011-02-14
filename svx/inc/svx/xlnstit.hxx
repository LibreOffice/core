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

#ifndef _SVX_XLNSTIT_HXX
#define _SVX_XLNSTIT_HXX

#include <svx/xit.hxx>
#include "svx/svxdllapi.h"
#include <basegfx/polygon/b2dpolypolygon.hxx>

class SdrModel;

//------------------------
// class XLineStartItem
//------------------------
class SVX_DLLPUBLIC XLineStartItem : public NameOrIndex
{
    basegfx::B2DPolyPolygon     maPolyPolygon;

public:
            TYPEINFO();
            XLineStartItem(sal_Int32 nIndex = -1);
            XLineStartItem(const String& rName, const basegfx::B2DPolyPolygon& rPolyPolygon);
            XLineStartItem(SfxItemPool* pPool, const basegfx::B2DPolyPolygon& rPolyPolygon);
            XLineStartItem(SfxItemPool* pPool );
            XLineStartItem(const XLineStartItem& rItem);
            XLineStartItem(SvStream& rIn);

    virtual int             operator==(const SfxPoolItem& rItem) const;
    virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem*    Create(SvStream& rIn, sal_uInt16 nVer) const;
    virtual SvStream&       Store(SvStream& rOut, sal_uInt16 nItemVersion ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    basegfx::B2DPolyPolygon GetLineStartValue(const XLineEndTable* pTable = 0) const;
    void SetLineStartValue(const basegfx::B2DPolyPolygon& rPolyPolygon) { maPolyPolygon = rPolyPolygon; Detach(); }

    XLineStartItem* checkForUniqueItem( SdrModel* pModel ) const;
};

#endif
