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
#ifndef _SXEKITM_HXX
#define _SXEKITM_HXX

#include <svl/eitem.hxx>
#include <svx/svddef.hxx>
#include "svx/svxdllapi.h"

enum SdrEdgeKind {SDREDGE_ORTHOLINES,
                  SDREDGE_THREELINES,
                  SDREDGE_ONELINE,
                  SDREDGE_BEZIER,
                  SDREDGE_ARC};

//------------------------------
// class SdrEdgeKindItem
//------------------------------
class SVX_DLLPUBLIC SdrEdgeKindItem: public SfxEnumItem {
public:
    TYPEINFO();
    SdrEdgeKindItem(SdrEdgeKind eStyle=SDREDGE_ORTHOLINES): SfxEnumItem(SDRATTR_EDGEKIND,sal::static_int_cast< sal_uInt16 >(eStyle)) {}
    SdrEdgeKindItem(SvStream& rIn)                        : SfxEnumItem(SDRATTR_EDGEKIND,rIn)    {}
    virtual SfxPoolItem* Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem* Create(SvStream& rIn, sal_uInt16 nVer) const;
    virtual sal_uInt16       GetValueCount() const; // { return 5; }
            SdrEdgeKind  GetValue() const      { return (SdrEdgeKind)SfxEnumItem::GetValue(); }
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
    virtual String  GetValueTextByPos(sal_uInt16 nPos) const;
    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String& rText, const IntlWrapper * = 0) const;
};

#endif
