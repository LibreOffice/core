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
#ifndef _SDTADITM_HXX
#define _SDTADITM_HXX

#include <svl/eitem.hxx>
#include <svx/svddef.hxx>
#include "svx/svxdllapi.h"

//------------------------------
// class SdrTextAniDirectionItem
//------------------------------

enum SdrTextAniDirection {SDRTEXTANI_LEFT,
                          SDRTEXTANI_RIGHT,
                          SDRTEXTANI_UP,
                          SDRTEXTANI_DOWN};

class SVX_DLLPUBLIC SdrTextAniDirectionItem: public SfxEnumItem {
public:
    TYPEINFO();
    SdrTextAniDirectionItem(SdrTextAniDirection eDir=SDRTEXTANI_LEFT): SfxEnumItem(SDRATTR_TEXT_ANIDIRECTION,(USHORT)eDir) {}
    SdrTextAniDirectionItem(SvStream& rIn)                           : SfxEnumItem(SDRATTR_TEXT_ANIDIRECTION,rIn)  {}
    virtual SfxPoolItem*   Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem*   Create(SvStream& rIn, USHORT nVer) const;
    virtual USHORT         GetValueCount() const;
    SdrTextAniDirection GetValue() const      { return (SdrTextAniDirection)SfxEnumItem::GetValue(); }

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual String  GetValueTextByPos(USHORT nPos) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String& rText, const IntlWrapper * = 0) const;
};

#endif
