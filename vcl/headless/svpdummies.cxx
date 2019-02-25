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

#include <string.h>
#include <rtl/ustrbuf.hxx>
#include <headless/svpdummies.hxx>
#include <headless/svpinst.hxx>

// SalObject
SvpSalObject::SvpSalObject()
{
    // fast and easy cross-platform wiping of the data
    memset( static_cast<void *>(&m_aSystemChildData), 0, sizeof( SystemEnvData ) );
    m_aSystemChildData.nSize = sizeof( SystemEnvData );
}

SvpSalObject::~SvpSalObject()
{
}

void SvpSalObject::ResetClipRegion() {}
void SvpSalObject::BeginSetClipRegion( sal_uInt32 ) {}
void SvpSalObject::UnionClipRegion( long, long, long, long ) {}
void SvpSalObject::EndSetClipRegion() {}
void SvpSalObject::SetPosSize( long, long, long, long ) {}
void SvpSalObject::Show( bool ) {}
const SystemEnvData* SvpSalObject::GetSystemData() const { return &m_aSystemChildData; }

// SalSystem
SvpSalSystem::~SvpSalSystem() {}

unsigned int SvpSalSystem::GetDisplayScreenCount()
{
    return 1;
}

tools::Rectangle SvpSalSystem::GetDisplayScreenPosSizePixel( unsigned int nScreen )
{
    tools::Rectangle aRect;
    if( nScreen == 0 )
        aRect = tools::Rectangle( Point(0,0), Size(VIRTUAL_DESKTOP_WIDTH,VIRTUAL_DESKTOP_HEIGHT) );
    return aRect;
}

int SvpSalSystem::ShowNativeDialog( const OUString&, const OUString&,
                                    const std::vector< OUString >& )
{
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
