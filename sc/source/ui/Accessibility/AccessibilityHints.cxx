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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include "AccessibilityHints.hxx"

using namespace ::com::sun::star;

// -----------------------------------------------------------------------

TYPEINIT1(ScAccWinFocusLostHint, SfxHint);

// -----------------------------------------------------------------------
//      ScAccWinFocusLostHint - the current window lost its focus (to another application, view or document)
// -----------------------------------------------------------------------

ScAccWinFocusLostHint::ScAccWinFocusLostHint(
        const uno::Reference< uno::XInterface >& xOld )
    :
    xOldAccessible(xOld)
{
}

ScAccWinFocusLostHint::~ScAccWinFocusLostHint()
{
}

// -----------------------------------------------------------------------

TYPEINIT1(ScAccWinFocusGotHint, SfxHint);

// -----------------------------------------------------------------------
//      ScAccWinFocusGotHint - the window got the focus (from another application, view or document)
// -----------------------------------------------------------------------

ScAccWinFocusGotHint::ScAccWinFocusGotHint(
        const uno::Reference< uno::XInterface >& xNew )
    :
    xNewAccessible(xNew)
{
}

ScAccWinFocusGotHint::~ScAccWinFocusGotHint()
{
}
// -----------------------------------------------------------------------

TYPEINIT1(ScAccGridWinFocusLostHint, SfxHint);

// -----------------------------------------------------------------------
//      ScAccGridWinFocusLostHint - the current grid window lost its focus (to another application, view or document)
// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

TYPEINIT1(ScAccGridWinFocusGotHint, SfxHint);

// -----------------------------------------------------------------------
//      ScAccGridWinFocusGotHint - the grid window got the focus (from another application, view or document)
// -----------------------------------------------------------------------

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
