/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "AccessibilityHints.hxx"

using namespace ::com::sun::star;



TYPEINIT1(ScAccWinFocusLostHint, SfxHint);





ScAccWinFocusLostHint::ScAccWinFocusLostHint(
        const uno::Reference< uno::XInterface >& xOld )
    :
    xOldAccessible(xOld)
{
}

ScAccWinFocusLostHint::~ScAccWinFocusLostHint()
{
}



TYPEINIT1(ScAccWinFocusGotHint, SfxHint);





ScAccWinFocusGotHint::ScAccWinFocusGotHint(
        const uno::Reference< uno::XInterface >& xNew )
    :
    xNewAccessible(xNew)
{
}

ScAccWinFocusGotHint::~ScAccWinFocusGotHint()
{
}


TYPEINIT1(ScAccGridWinFocusLostHint, SfxHint);





ScAccGridWinFocusLostHint::ScAccGridWinFocusLostHint(ScSplitPos eOld,
        const uno::Reference< uno::XInterface >& xOld )
    :
    ScAccWinFocusLostHint(xOld),
    eOldGridWin(eOld)
{
}

ScAccGridWinFocusLostHint::~ScAccGridWinFocusLostHint()
{
}



TYPEINIT1(ScAccGridWinFocusGotHint, SfxHint);





ScAccGridWinFocusGotHint::ScAccGridWinFocusGotHint(ScSplitPos eNew,
        const uno::Reference< uno::XInterface >& xNew )
    :
    ScAccWinFocusGotHint(xNew),
    eNewGridWin(eNew)
{
}

ScAccGridWinFocusGotHint::~ScAccGridWinFocusGotHint()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
