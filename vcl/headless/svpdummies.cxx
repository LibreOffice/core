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

#include <string.h>
#include <rtl/ustrbuf.hxx>
#include "headless/svpdummies.hxx"
#include "headless/svpinst.hxx"

// SalObject
SvpSalObject::SvpSalObject()
{
    // fast and easy cross-platform wiping of the data
    memset( (void *)&m_aSystemChildData, 0, sizeof( SystemChildData ) );
    m_aSystemChildData.nSize = sizeof( SystemChildData );
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

Rectangle SvpSalSystem::GetDisplayScreenPosSizePixel( unsigned int nScreen )
{
    Rectangle aRect;
    if( nScreen == 0 )
        aRect = Rectangle( Point(0,0), Size(VIRTUAL_DESKTOP_WIDTH,VIRTUAL_DESKTOP_HEIGHT) );
    return aRect;
}

Rectangle SvpSalSystem::GetDisplayScreenWorkAreaPosSizePixel( unsigned int nScreen )
{
    return GetDisplayScreenPosSizePixel( nScreen );
}

rtl::OUString SvpSalSystem::GetDisplayScreenName( unsigned int nScreen )
{
   rtl::OUStringBuffer aBuf( 32 );
   aBuf.appendAscii( "VirtualScreen " );
   aBuf.append( sal_Int32(nScreen) );
   return aBuf.makeStringAndClear();
}

int SvpSalSystem::ShowNativeDialog( const rtl::OUString&, const rtl::OUString&,
                                    const std::list< rtl::OUString >&, int )
{
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
