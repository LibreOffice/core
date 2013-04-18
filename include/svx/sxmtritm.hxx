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
#ifndef _SXMTRITM_HXX
#define _SXMTRITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdynitm.hxx>

// Den Text quer zur Masslinie (90deg Drehung nach links)
class SdrMeasureTextRota90Item: public SdrYesNoItem {
public:
    SdrMeasureTextRota90Item(bool bOn=false): SdrYesNoItem(SDRATTR_MEASURETEXTROTA90,bOn) {}
    SdrMeasureTextRota90Item(SvStream& rIn): SdrYesNoItem(SDRATTR_MEASURETEXTROTA90,rIn) {}
};

// Das von mir berechnete TextRect um 180 deg drehen
// Der Text wird dann aber auch auf die andere Seite der
// Masslinie gebracht (wenn nicht Rota90)
class SdrMeasureTextUpsideDownItem: public SdrYesNoItem {
public:
    SdrMeasureTextUpsideDownItem(bool bOn=false): SdrYesNoItem(SDRATTR_MEASURETEXTUPSIDEDOWN,bOn) {}
    SdrMeasureTextUpsideDownItem(SvStream& rIn): SdrYesNoItem(SDRATTR_MEASURETEXTUPSIDEDOWN,rIn) {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
