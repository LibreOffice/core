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

#include <AccessibilityHints.hxx>

//      ScAccWinFocusLostHint - the current window lost its focus (to another application, view or document)

ScAccWinFocusLostHint::~ScAccWinFocusLostHint()
{
}

//      ScAccWinFocusGotHint - the window got the focus (from another application, view or document)

ScAccWinFocusGotHint::~ScAccWinFocusGotHint()
{
}

//      ScAccGridWinFocusLostHint - the current grid window lost its focus (to another application, view or document)

ScAccGridWinFocusLostHint::ScAccGridWinFocusLostHint(ScSplitPos eOld )
    :
    ScAccWinFocusLostHint(),
    eOldGridWin(eOld)
{
}

ScAccGridWinFocusLostHint::~ScAccGridWinFocusLostHint()
{
}

//      ScAccGridWinFocusGotHint - the grid window got the focus (from another application, view or document)

ScAccGridWinFocusGotHint::ScAccGridWinFocusGotHint(ScSplitPos eNew )
    :
    ScAccWinFocusGotHint(),
    eNewGridWin(eNew)
{
}

ScAccGridWinFocusGotHint::~ScAccGridWinFocusGotHint()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
