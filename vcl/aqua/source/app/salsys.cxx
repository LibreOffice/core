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
#include "precompiled_vcl.hxx"

#include "tools/rc.hxx"
#include "vcl/svids.hrc"

#include "salsys.h"
#include "saldata.hxx"
#include "rtl/ustrbuf.hxx"

using ::rtl::OUString;

// =======================================================================

AquaSalSystem::~AquaSalSystem()
{
}

unsigned int AquaSalSystem::GetDisplayScreenCount()
{
    NSArray* pScreens = [NSScreen screens];
    return pScreens ? [pScreens count] : 1;
}

bool AquaSalSystem::IsMultiDisplay()
{
    return false;
}

unsigned int AquaSalSystem::GetDefaultDisplayNumber()
{
    return 0;
}

Rectangle AquaSalSystem::GetDisplayScreenPosSizePixel( unsigned int nScreen )
{
    NSArray* pScreens = [NSScreen screens];
    Rectangle aRet;
    NSScreen* pScreen = nil;
    if( pScreens && nScreen < [pScreens count] )
        pScreen = [pScreens objectAtIndex: nScreen];
    else
        pScreen = [NSScreen mainScreen];

    if( pScreen )
    {
        NSRect aFrame = [pScreen frame];
        aRet = Rectangle( Point( static_cast<long int>(aFrame.origin.x), static_cast<long int>(aFrame.origin.y) ),
                          Size( static_cast<long int>(aFrame.size.width), static_cast<long int>(aFrame.size.height) ) );
    }
    return aRet;
}

Rectangle AquaSalSystem::GetDisplayWorkAreaPosSizePixel( unsigned int nScreen )
{
    NSArray* pScreens = [NSScreen screens];
    Rectangle aRet;
    NSScreen* pScreen = nil;
    if( pScreens && nScreen < [pScreens count] )
        pScreen = [pScreens objectAtIndex: nScreen];
    else
        pScreen = [NSScreen mainScreen];

    if( pScreen )
    {
        NSRect aFrame = [pScreen visibleFrame];
        aRet = Rectangle( Point( static_cast<long int>(aFrame.origin.x), static_cast<long int>(aFrame.origin.y) ),
                          Size( static_cast<long int>(aFrame.size.width), static_cast<long int>(aFrame.size.height) ) );
    }
    return aRet;
}

rtl::OUString AquaSalSystem::GetScreenName( unsigned int nScreen )
{
   NSArray* pScreens = [NSScreen screens];
   OUString aRet;
   if( nScreen < [pScreens count] )
   {
        ResMgr* pMgr = ImplGetResMgr();
        if( pMgr )
        {
            String aScreenName( ResId( SV_MAC_SCREENNNAME, *pMgr ) );
            aScreenName.SearchAndReplaceAllAscii( "%d", String::CreateFromInt32( nScreen ) );
            aRet = aScreenName;
        }
   }
   return aRet;
}

int AquaSalSystem::ShowNativeDialog( const String& rTitle,
                                    const String& rMessage,
                                    const std::list< String >& rButtons,
                                    int nDefButton )
{
    return 0;
}

int AquaSalSystem::ShowNativeMessageBox( const String& rTitle,
                                        const String& rMessage,
                                        int nButtonCombination,
                                        int nDefaultButton)
{
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
