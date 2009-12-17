/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sxmkitm.hxx,v $
 * $Revision: 1.3 $
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
#ifndef _SXMKITM_HXX
#define _SXMKITM_HXX

#include <svx/svddef.hxx>
#include <svl/eitem.hxx>

enum SdrMeasureKind     {SDRMEASURE_STD,SDRMEASURE_RADIUS}; // n.i.

//------------------------------
// class SdrMeasureKindItem (n.i.)
//------------------------------
class SdrMeasureKindItem: public SfxEnumItem {
public:
    TYPEINFO();
    SdrMeasureKindItem(SdrMeasureKind eKind=SDRMEASURE_STD): SfxEnumItem(SDRATTR_MEASUREKIND,sal::static_int_cast< USHORT >(eKind)) {}
    SdrMeasureKindItem(SvStream& rIn)                      : SfxEnumItem(SDRATTR_MEASUREKIND,rIn)    {}
    virtual SfxPoolItem*   Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem*   Create(SvStream& rIn, USHORT nVer) const;
    virtual USHORT         GetValueCount() const; // { return 2; }
            SdrMeasureKind GetValue() const { return (SdrMeasureKind)SfxEnumItem::GetValue(); }

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual String  GetValueTextByPos(USHORT nPos) const;
    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String& rText, const IntlWrapper * = 0) const;
};

#endif
