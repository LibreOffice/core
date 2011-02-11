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

#include "svpdummies.hxx"
#include "svpinst.hxx"
#include <rtl/ustrbuf.hxx>

// SalObject
SvpSalObject::SvpSalObject()
{
    m_aSystemChildData.nSize        = sizeof( SystemChildData );
    m_aSystemChildData.pDisplay     = NULL;
    m_aSystemChildData.aWindow      = 0;
    m_aSystemChildData.pSalFrame    = 0;
    m_aSystemChildData.pWidget      = 0;
    m_aSystemChildData.pVisual      = 0;
    m_aSystemChildData.nDepth       = 0;
    m_aSystemChildData.aColormap    = 0;
    m_aSystemChildData.pAppContext  = NULL;
    m_aSystemChildData.aShellWindow = 0;
    m_aSystemChildData.pShellWidget = NULL;
}

SvpSalObject::~SvpSalObject()
{
}

void SvpSalObject::ResetClipRegion() {}
sal_uInt16 SvpSalObject::GetClipRegionType() { return 0; }
void SvpSalObject::BeginSetClipRegion( sal_uLong ) {}
void SvpSalObject::UnionClipRegion( long, long, long, long ) {}
void SvpSalObject::EndSetClipRegion() {}
void SvpSalObject::SetPosSize( long, long, long, long ) {}
void SvpSalObject::Show( sal_Bool ) {}
void SvpSalObject::Enable( sal_Bool ) {}
void SvpSalObject::GrabFocus() {}
void SvpSalObject::SetBackground() {}
void SvpSalObject::SetBackground( SalColor ) {}
const SystemEnvData* SvpSalObject::GetSystemData() const { return &m_aSystemChildData; }
void SvpSalObject::InterceptChildWindowKeyDown( sal_Bool ) {}

// SalI18NImeStatus
SvpImeStatus::~SvpImeStatus() {}
bool SvpImeStatus::canToggle() { return false; }
void SvpImeStatus::toggle() {}

// SalSystem
SvpSalSystem::~SvpSalSystem() {}

unsigned int SvpSalSystem::GetDisplayScreenCount()
{
    return 1;
}

unsigned int SvpSalSystem::GetDefaultDisplayNumber()
{
    return 0;
}

bool SvpSalSystem::IsMultiDisplay()
{
    return false;
}

Rectangle SvpSalSystem::GetDisplayScreenPosSizePixel( unsigned int nScreen )
{
    Rectangle aRect;
    if( nScreen == 0 )
        aRect = Rectangle( Point(0,0), Size(VIRTUAL_DESKTOP_WIDTH,VIRTUAL_DESKTOP_HEIGHT) );
    return aRect;
}

Rectangle SvpSalSystem::GetDisplayWorkAreaPosSizePixel( unsigned int nScreen )
{
    return GetDisplayScreenPosSizePixel( nScreen );
}

rtl::OUString SvpSalSystem::GetScreenName( unsigned int nScreen )
{
   rtl::OUStringBuffer aBuf( 32 );
   aBuf.appendAscii( "VirtualScreen " );
   aBuf.append( sal_Int32(nScreen) );
   return aBuf.makeStringAndClear();
}

int SvpSalSystem::ShowNativeMessageBox( const String&,
                                        const String&,
                                        int,
                                        int )
{
    return 0;
}

