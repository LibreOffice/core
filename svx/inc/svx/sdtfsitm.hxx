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
#ifndef _SDTFSITM_HXX
#define _SDTFSITM_HXX

#include <svl/eitem.hxx>
#include <svx/svddef.hxx>
#include "svx/svxdllapi.h"

enum SdrFitToSizeType {
    SDRTEXTFIT_NONE,         // - no fit-to-size
    SDRTEXTFIT_PROPORTIONAL, // - resize all glyhs proportionally
                             //   (might scale anisotrophically)
    SDRTEXTFIT_ALLLINES,     // - like SDRTEXTFIT_PROPORTIONAL, but
                             //   scales each line separately
    SDRTEXTFIT_AUTOFIT};     // - mimics PPT's automatic adaption of
                             //   font size to text rect - comparable
                             //   to SDRTEXTFIT_PROPORTIONAL, but
                             //   scales isotrophically

// No AutoGrow and no automatic line breaks for
// SDRTEXTFIT_PROPORTIONAL and SDRTEXTFIT_ALLLINES.
// No automatic line breaks for AutoGrowingWidth as well (only if
// TextMaxFrameWidth is reached).

//--------------------------------
// class SdrTextFitToSizeTypeItem
//--------------------------------
class SVX_DLLPUBLIC SdrTextFitToSizeTypeItem: public SfxEnumItem {
public:
    TYPEINFO();
    SdrTextFitToSizeTypeItem(SdrFitToSizeType eFit=SDRTEXTFIT_NONE): SfxEnumItem(SDRATTR_TEXT_FITTOSIZE,(sal_uInt16)eFit) {}
    SdrTextFitToSizeTypeItem(SvStream& rIn)                        : SfxEnumItem(SDRATTR_TEXT_FITTOSIZE,rIn)  {}
    virtual SfxPoolItem*     Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem*     Create(SvStream& rIn, sal_uInt16 nVer) const;
    virtual sal_uInt16           GetValueCount() const; // { return 4; }
            SdrFitToSizeType GetValue() const      { return (SdrFitToSizeType)SfxEnumItem::GetValue(); }

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual String  GetValueTextByPos(sal_uInt16 nPos) const;
    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String& rText, const IntlWrapper * = 0) const;
    virtual int              HasBoolValue() const;
    virtual sal_Bool             GetBoolValue() const;
    virtual void             SetBoolValue(sal_Bool bVal);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
