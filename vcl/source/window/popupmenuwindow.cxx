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
#include "precompiled_vcl.hxx"

#include "vcl/popupmenuwindow.hxx"

#include <limits>

struct PopupMenuFloatingWindow::ImplData
{
    sal_uInt16      mnMenuStackLevel;  // Store the stack level of a popup menu.  0 = top-level menu.

    ImplData();
    ~ImplData();
};

PopupMenuFloatingWindow::ImplData::ImplData() :
    mnMenuStackLevel( ::std::numeric_limits<sal_uInt16>::max() )
{
}

PopupMenuFloatingWindow::ImplData::~ImplData()
{
}

// ============================================================================

PopupMenuFloatingWindow::PopupMenuFloatingWindow( Window* pParent, WinBits nStyle ) :
    FloatingWindow(pParent, nStyle),
    mpImplData(new ImplData)
{
}

PopupMenuFloatingWindow::~PopupMenuFloatingWindow()
{
    delete mpImplData;
}

sal_uInt16 PopupMenuFloatingWindow::GetMenuStackLevel() const
{
    return mpImplData->mnMenuStackLevel;
}

void PopupMenuFloatingWindow::SetMenuStackLevel( sal_uInt16 nLevel )
{
    mpImplData->mnMenuStackLevel = nLevel;
}

bool PopupMenuFloatingWindow::IsPopupMenu() const
{
    return mpImplData->mnMenuStackLevel != ::std::numeric_limits<sal_uInt16>::max();
}

