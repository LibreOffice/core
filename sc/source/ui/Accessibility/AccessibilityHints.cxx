/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include "AccessibilityHints.hxx"

using namespace ::com::sun::star;

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
