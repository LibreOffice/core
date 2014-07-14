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

#include <salframe.hxx>
#include <salinst.hxx>
#include <salvd.hxx>
#include <salprn.hxx>
#include <saltimer.hxx>
#include <salimestatus.hxx>
#include <salsys.hxx>
#include <salbmp.hxx>
#include <salobj.hxx>
#include <salmenu.hxx>
#include <vcl/apptypes.hxx>

// this file contains the virtual destructors of the sal interface
// compilers usually put their vtables where the destructor is

ApplicationProperty::~ApplicationProperty()
{
}

// -----------------------------------------------------------------------

SalFrame::~SalFrame()
{
}

// -----------------------------------------------------------------------

// default to full-frame flushes
// on ports where partial-flushes are much cheaper this method should be overridden
void SalFrame::Flush( const Rectangle& )
{
    Flush();
}

// -----------------------------------------------------------------------

void SalFrame::SetRepresentedURL( const rtl::OUString& )
{
    // currently this is Mac only functionality
}

// -----------------------------------------------------------------------

SalInstance::~SalInstance()
{
}

void SalInstance::FillFontPathList( std::list< rtl::OString >& )
{
    // do nothing
}

SalMenu* SalInstance::CreateMenu( sal_Bool, Menu* )
{
    // default: no native menus
    return NULL;
}

void SalInstance::DestroyMenu( SalMenu* pMenu )
{
    (void)pMenu;
    OSL_ENSURE( pMenu == 0, "DestroyMenu called with non-native menus" );
}

SalMenuItem* SalInstance::CreateMenuItem( const SalItemParams* )
{
    return NULL;
}

void SalInstance::DestroyMenuItem( SalMenuItem* pItem )
{
    (void)pItem;
    OSL_ENSURE( pItem == 0, "DestroyMenu called with non-native menus" );
}

SalTimer::~SalTimer()
{
}

SalBitmap::~SalBitmap()
{
}

SalI18NImeStatus::~SalI18NImeStatus()
{
}

SalSystem::~SalSystem()
{
}

SalPrinter::~SalPrinter()
{
}

sal_Bool SalPrinter::StartJob( const String*, const String&, const String&,
                           ImplJobSetup*, vcl::PrinterController& )
{
    return sal_False;
}

SalInfoPrinter::~SalInfoPrinter()
{
}

SalVirtualDevice::~SalVirtualDevice()
{
}

SalObject::~SalObject()
{
}

SalMenu::~SalMenu()
{
}

bool SalMenu::ShowNativePopupMenu(FloatingWindow *, const Rectangle&, sal_uLong )
{
    return false;
}

bool SalMenu::AddMenuBarButton( const SalMenuButtonItem& )
{
    return false;
}

void SalMenu::RemoveMenuBarButton( sal_uInt16 )
{
}

Rectangle SalMenu::GetMenuBarButtonRectPixel( sal_uInt16, SalFrame* )
{
    return Rectangle();
}

SalMenuItem::~SalMenuItem()
{
}

