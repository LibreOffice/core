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
#ifndef INCLUDED_SVX_INC_SXMTAITM_HXX
#define INCLUDED_SVX_INC_SXMTAITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdynitm.hxx>
#include <svx/sdangitm.hxx>

// Turn text automatically in wright position (automatic UpsideDown).
// TextUpsideDown stays active and turns the text again if sal_True.
class SdrMeasureTextAutoAngleItem: public SdrYesNoItem {
public:
    SdrMeasureTextAutoAngleItem(bool bOn=true): SdrYesNoItem(SDRATTR_MEASURETEXTAUTOANGLE,bOn) {}
    SdrMeasureTextAutoAngleItem(SvStream& rIn): SdrYesNoItem(SDRATTR_MEASURETEXTAUTOANGLE,rIn) {}
    virtual SfxPoolItem* Clone(SfxItemPool*) const override
    {
        return new SdrMeasureTextAutoAngleItem(*this);
    }
};

// Preferred perspective for reading text is only evaluated if TextAutoAngle=TRUE.
// Angle in 1/100deg from viewpoint of the user.
class SdrMeasureTextAutoAngleViewItem: public SdrAngleItem {
public:
    SdrMeasureTextAutoAngleViewItem(long nVal=31500): SdrAngleItem(SDRATTR_MEASURETEXTAUTOANGLEVIEW,nVal)  {}
    SdrMeasureTextAutoAngleViewItem(SvStream& rIn): SdrAngleItem(SDRATTR_MEASURETEXTAUTOANGLEVIEW,rIn) {}
    virtual SfxPoolItem* Clone(SfxItemPool*) const override
    {
        return new SdrMeasureTextAutoAngleViewItem(*this);
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
