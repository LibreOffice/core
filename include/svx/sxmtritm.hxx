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
#ifndef INCLUDED_SVX_SXMTRITM_HXX
#define INCLUDED_SVX_SXMTRITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdynitm.hxx>

// text across the dimension line (90deg counter-clockwise rotation)
class SVX_DLLPUBLIC SdrMeasureTextRota90Item: public SdrYesNoItem {
public:
    SdrMeasureTextRota90Item(bool bOn=false): SdrYesNoItem(SDRATTR_MEASURETEXTROTA90,bOn) {}
    virtual ~SdrMeasureTextRota90Item() override;
    virtual SfxPoolItem* Clone(SfxItemPool* pPool=nullptr) const override;

    SdrMeasureTextRota90Item(SdrMeasureTextRota90Item const &) = default;
    SdrMeasureTextRota90Item(SdrMeasureTextRota90Item &&) = default;
    SdrMeasureTextRota90Item & operator =(SdrMeasureTextRota90Item const &) = default;
    SdrMeasureTextRota90Item & operator =(SdrMeasureTextRota90Item &&) = default;
};

// Turn the calculated TextRect through 180 deg
// Text is also switched to the other side of the dimension line, if not Rota90
class SVX_DLLPUBLIC SdrMeasureTextUpsideDownItem: public SdrYesNoItem {
public:
    SdrMeasureTextUpsideDownItem(bool bOn=false): SdrYesNoItem(SDRATTR_MEASURETEXTUPSIDEDOWN,bOn) {}
    virtual ~SdrMeasureTextUpsideDownItem() override;
    virtual SfxPoolItem* Clone(SfxItemPool* pPool=nullptr) const override;

    SdrMeasureTextUpsideDownItem(SdrMeasureTextUpsideDownItem const &) = default;
    SdrMeasureTextUpsideDownItem(SdrMeasureTextUpsideDownItem &&) = default;
    SdrMeasureTextUpsideDownItem & operator =(SdrMeasureTextUpsideDownItem const &) = default;
    SdrMeasureTextUpsideDownItem & operator =(SdrMeasureTextUpsideDownItem &&) = default;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
