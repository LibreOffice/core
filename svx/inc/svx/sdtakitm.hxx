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
#ifndef SDTAKITM_HXX
#define SDTAKITM_HXX

#include <svl/eitem.hxx>
#include <svx/svddef.hxx>
#include "svx/svxdllapi.h"

//------------------------------
// class SdrTextAniKindItem
//------------------------------

enum SdrTextAniKind {SDRTEXTANI_NONE,
                     SDRTEXTANI_BLINK,
                     SDRTEXTANI_SCROLL,
                     SDRTEXTANI_ALTERNATE,
                     SDRTEXTANI_SLIDE};

// - SDRTEXTANI_BLINK:
//   Just blink.  Direction and Amount don't effect things.
//   Frequency delay: 0 = 0.5Hz (Delay = 250).
//   Count = number of blinks. (0 = forever)
//   start inside: sal_False = begin with the blinker off, sal_True  = begin with the blinker on
//   stop inside: sal_False = end with the blinker off, sal_True = end with the blinker on
//                (Only if count! = 0)
// - SDRTEXTANI_SCROLL:
//   Text just scrolls all the way through, and starts over when it is done.
//
//   Delay in ms: (except delay = 0 means 50ms (20Hz) default)
//   Count = number of passes (0 = go forever)
//   Direction: the direction to scroll
//   StartInside: sal_False = on the first pass, scroll the text in (not initially visible)
//                sal_True = on the first pass, the left part of the text is already visible
//                           in the scroll area before starting.
//   StopInside: sal_False = on the last pass, scroll the text all the way out.
//                sal_True = on the last pass, keep the right part of the text visible.
//                (Only if count! = 0)
//   Amount: Step size in logical units. Negative values = use Pixels instead
//           of logical units. If Amount = 0, then default to 1 Pixel.
// - SDRTEXTANI_ALTERNATE:
//   Like SDRTEXTANI_SCROLL, except don't scroll until all of the text is out,
//        but just until the last part is visible.  Then, change direction and
//        scroll the text the other way.
//   Count = number of passes (number of direction changes minus 1)
//        If count = 1, this is the same as SDRTEXTANI_SCROLL
//   Direction = the starting direction.
//   All Other Parameters: just like SDRTEXTANI_SCROLL
// - SDRTEXTANI_SLIDE:
//   Text will slide in to the original position.
//   -> same as SCROLL with StartInside = sal_False, StopInside = sal_True
//   and Count = 1 (Count = 0 is interpreted as count = 1).
//   For each count > 1, the text, will be slid out backwards
//   (much like ALTERNATE) and then slid back in again.
//   StopInside is not evaluated, because inside is always stopped.
//   StartInside is not evaluated, because outside is always started.
//   All other parameters are like SDRTEXTANI_SCROLL
// StartInside, StopInside: When sal_True, the starting / final position
//   of the text depends on the anchor of the drawing object.  This
//   corresponds to the position of the text in normal Paint (without scrolling).

class SVX_DLLPUBLIC SdrTextAniKindItem: public SfxEnumItem {
public:
    TYPEINFO();
    SdrTextAniKindItem(SdrTextAniKind eKind=SDRTEXTANI_NONE): SfxEnumItem(SDRATTR_TEXT_ANIKIND,(sal_uInt16)eKind) {}
    SdrTextAniKindItem(SvStream& rIn)                       : SfxEnumItem(SDRATTR_TEXT_ANIKIND,rIn)  {}
    virtual SfxPoolItem*      Clone(SfxItemPool* pPool=NULL) const;
    virtual SfxPoolItem*      Create(SvStream& rIn, sal_uInt16 nVer) const;
    virtual sal_uInt16            GetValueCount() const; // { return 5; }
            SdrTextAniKind GetValue() const      { return (SdrTextAniKind)SfxEnumItem::GetValue(); }

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual rtl::OUString GetValueTextByPos(sal_uInt16 nPos) const;
    virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, String& rText, const IntlWrapper * = 0) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
