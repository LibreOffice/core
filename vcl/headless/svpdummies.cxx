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

#include <headless/svpdummies.hxx>

SvpSalObject::~SvpSalObject()
{
}

void SvpSalObject::ResetClipRegion() {}
void SvpSalObject::BeginSetClipRegion( sal_uInt32 ) {}
void SvpSalObject::UnionClipRegion( tools::Long, tools::Long, tools::Long, tools::Long ) {}
void SvpSalObject::EndSetClipRegion() {}
void SvpSalObject::SetPosSize( tools::Long, tools::Long, tools::Long, tools::Long ) {}
void SvpSalObject::Show( bool ) {}
const SystemEnvData* SvpSalObject::GetSystemData() const { return &m_aSystemChildData; }

// SalSystem
SvpSalSystem::~SvpSalSystem() {}

unsigned int SvpSalSystem::GetDisplayScreenCount()
{
    return 1;
}

AbsoluteScreenPixelRectangle SvpSalSystem::GetDisplayScreenPosSizePixel( unsigned int nScreen )
{
    AbsoluteScreenPixelRectangle aRect;
    if( nScreen == 0 )
        aRect = AbsoluteScreenPixelRectangle( AbsoluteScreenPixelPoint(0,0), AbsoluteScreenPixelSize(VIRTUAL_DESKTOP_WIDTH,VIRTUAL_DESKTOP_HEIGHT) );
    return aRect;
}

int SvpSalSystem::ShowNativeDialog( const OUString&, const OUString&,
                                    const std::vector< OUString >& )
{
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
