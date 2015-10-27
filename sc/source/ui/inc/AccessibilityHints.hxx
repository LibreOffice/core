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

#ifndef INCLUDED_SC_SOURCE_UI_INC_ACCESSIBILITYHINTS_HXX
#define INCLUDED_SC_SOURCE_UI_INC_ACCESSIBILITYHINTS_HXX

#include "viewdata.hxx"
#include <com/sun/star/uno/XInterface.hpp>
#include <svl/smplhint.hxx>
#include <svl/hint.hxx>

#define SC_HINT_ACC_SIMPLE_START    SFX_HINT_USER00
#define SC_HINT_ACC_TABLECHANGED    SC_HINT_ACC_SIMPLE_START + 1
#define SC_HINT_ACC_CURSORCHANGED   SC_HINT_ACC_SIMPLE_START + 2
#define SC_HINT_ACC_VISAREACHANGED  SC_HINT_ACC_SIMPLE_START + 3
#define SC_HINT_ACC_ENTEREDITMODE   SC_HINT_ACC_SIMPLE_START + 4
#define SC_HINT_ACC_LEAVEEDITMODE   SC_HINT_ACC_SIMPLE_START + 5
#define SC_HINT_ACC_MAKEDRAWLAYER   SC_HINT_ACC_SIMPLE_START + 6
#define SC_HINT_ACC_WINDOWRESIZED   SC_HINT_ACC_SIMPLE_START + 7

class ScAccWinFocusLostHint : public SfxHint
{
    css::uno::Reference< css::uno::XInterface >
                xOldAccessible;
public:
                ScAccWinFocusLostHint(
                    const css::uno::Reference< css::uno::XInterface >& xOld );
                virtual ~ScAccWinFocusLostHint();
};

class ScAccWinFocusGotHint : public SfxHint
{
    css::uno::Reference< css::uno::XInterface >
                xNewAccessible;
public:
                ScAccWinFocusGotHint(
                    const css::uno::Reference< css::uno::XInterface >& xNew );
                virtual ~ScAccWinFocusGotHint();
};

class ScAccGridWinFocusLostHint : public ScAccWinFocusLostHint
{
    ScSplitPos  eOldGridWin;
public:
                ScAccGridWinFocusLostHint( ScSplitPos eOldGridWin,
                    const css::uno::Reference< css::uno::XInterface >& xOld );
                virtual ~ScAccGridWinFocusLostHint();

    ScSplitPos  GetOldGridWin() const { return eOldGridWin; }
};

class ScAccGridWinFocusGotHint : public ScAccWinFocusGotHint
{
    ScSplitPos  eNewGridWin;
public:
                ScAccGridWinFocusGotHint( ScSplitPos eNewGridWin,
                    const css::uno::Reference< css::uno::XInterface >& xNew );
                virtual ~ScAccGridWinFocusGotHint();

    ScSplitPos  GetNewGridWin() const { return eNewGridWin; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
