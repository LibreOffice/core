/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVX_SDTAKITM_HXX
#define INCLUDED_SVX_SDTAKITM_HXX

#include <svl/eitem.hxx>
#include <svx/svddef.hxx>
#include <svx/svxdllapi.h>

/**
 * Animation type for text frame.
 */
enum SdrTextAniKind {
    SDRTEXTANI_NONE,      /// no animation
    SDRTEXTANI_BLINK,     /// blinking
    SDRTEXTANI_SCROLL,    /// scroll through
    SDRTEXTANI_ALTERNATE, /// scroll back and forth
    SDRTEXTANI_SLIDE      /// scroll in
};

// - SDRTEXTANI_BLINK:
//   Just blink.  Direction and Amount don't effect things.
//   Frequency delay: 0 = 0.5Hz (Delay = 250).
//   Count = number of blinks. (0 = forever)
//   start inside: sal_False = begin with the blinker off, sal_True  = begin with the blinker on
//   stop inside: sal_False = end with the blinker off, sal_True = end with the blinker on
//                (Only if count! = 0)
// - SDRTEXTANI_SCROLL:
//   Text just scrolls all the way through, and starts over when it is done.

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
    TYPEINFO_OVERRIDE();
    SdrTextAniKindItem(SdrTextAniKind eKind=SDRTEXTANI_NONE): SfxEnumItem(SDRATTR_TEXT_ANIKIND,(sal_uInt16)eKind) {}
    SdrTextAniKindItem(SvStream& rIn)                       : SfxEnumItem(SDRATTR_TEXT_ANIKIND,rIn)  {}
    virtual SfxPoolItem*      Clone(SfxItemPool* pPool=NULL) const override;
    virtual SfxPoolItem*      Create(SvStream& rIn, sal_uInt16 nVer) const override;
    virtual sal_uInt16            GetValueCount() const override; // { return 5; }
            SdrTextAniKind GetValue() const      { return (SdrTextAniKind)SfxEnumItem::GetValue(); }

    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual OUString GetValueTextByPos(sal_uInt16 nPos) const override;
    virtual bool GetPresentation(SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, OUString& rText, const IntlWrapper * = 0) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
