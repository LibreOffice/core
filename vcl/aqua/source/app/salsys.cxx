/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salsys.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2007-10-09 15:13:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "vcl/salsys.hxx"
#include "salsys.h"
#include "premac.h"
#include <ApplicationServices/ApplicationServices.h>
#include "postmac.h"
#include "saldata.hxx"

// =======================================================================

//AquaSalSystem::AquaSalSystem()
//{
//}

// -----------------------------------------------------------------------

AquaSalSystem::~AquaSalSystem()
{
}

// -----------------------------------------------------------------------

//bool AquaSalSystem::StartProcess( SalFrame* pFrame,
//                            const XubString& rFileName,
//                            const XubString& rParam,
//                            const XubString& rWorkDir )
//{
//  return FALSE;
//}

// -----------------------------------------------------------------------

//BOOL AquaSalSystem::AddRecentDoc( SalFrame*, const XubString& rFileName )
//{
//  return FALSE;
//}

unsigned int AquaSalSystem::GetDisplayScreenCount()
{
/*
    CGDirectDisplayID displays[64]; // 64 displays are enough for everyone
    CGDisplayCount displayCount;
    if( noErr == CGGetActiveDisplayList( 64, displays, &displayCount ) )
        return displayCount;
*/
    return 1;
}

bool AquaSalSystem::IsMultiDisplay()
{
    /* FIXME: add support for multiple displays. */
    return false;
}

unsigned int AquaSalSystem::GetDefaultDisplayNumber()
{
    return 0;
}

Rectangle AquaSalSystem::GetDisplayScreenPosSizePixel( unsigned int nScreen )
{
    CGRect aRect( CGDisplayBounds ( CGMainDisplayID() ) );
    Rectangle aRet( aRect.origin.x, aRect.origin.y, aRect.origin.x + aRect.size.width, aRect.origin.y + aRect.size.height );
    AquaLog("AquaSalSystem::GetDisplayScreenPosSizePixel(%d) (%ld,%ld,%ld,%ld)\n", nScreen, aRet.nLeft, aRet.nTop, aRet.nRight, aRet.nBottom );
    return aRet;
}

Rectangle AquaSalSystem::GetDisplayWorkAreaPosSizePixel( unsigned int nScreen )
{
    return Rectangle();
}

rtl::OUString AquaSalSystem::GetScreenName( unsigned int nScreen )
{
    // FIXME
    return rtl::OUString();
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
