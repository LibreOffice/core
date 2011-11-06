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

