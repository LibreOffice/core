/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svpdummies.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 10:26:21 $
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

#include "svpdummies.hxx"
#include "svpinst.hxx"
#include <rtl/ustrbuf.hxx>

// SalOpenGL
SvpSalOpenGL::~SvpSalOpenGL() {}
bool SvpSalOpenGL::IsValid() { return false; }
oglFunction SvpSalOpenGL::GetOGLFnc( const char * ) { return NULL; }
void SvpSalOpenGL::OGLEntry( SalGraphics* ) {}
void SvpSalOpenGL::OGLExit( SalGraphics* ) {}
void SvpSalOpenGL::StartScene( SalGraphics* ) {}
void SvpSalOpenGL::StopScene() {}

// SalSound
SvpSalSound::~SvpSalSound() {}
bool SvpSalSound::IsValid() { return true; }
bool SvpSalSound::Init( const String&, ULONG& rSoundLen )
{
    rSoundLen = 0;
    return true;
}
void SvpSalSound::Play( ULONG, ULONG, bool bLoop )
{
    m_bPlaying = true;
    m_bLooping = bLoop;
}
void SvpSalSound::Stop()
{
    m_bPlaying = m_bLooping = false;
}
void SvpSalSound::Pause()
{
    m_bPaused = true;
}
void SvpSalSound::Continue()
{
    m_bPaused = false;
}
bool SvpSalSound::IsLoopMode() const
{
    return m_bLooping;
}
bool SvpSalSound::IsPlaying() const
{
    return m_bPlaying;
}
bool SvpSalSound::IsPaused() const
{
    return m_bPaused;
}

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
USHORT SvpSalObject::GetClipRegionType() { return 0; }
void SvpSalObject::BeginSetClipRegion( ULONG ) {}
void SvpSalObject::UnionClipRegion( long, long, long, long ) {}
void SvpSalObject::EndSetClipRegion() {}
void SvpSalObject::SetPosSize( long, long, long, long ) {}
void SvpSalObject::Show( BOOL ) {}
void SvpSalObject::Enable( BOOL ) {}
void SvpSalObject::GrabFocus() {}
void SvpSalObject::SetBackground() {}
void SvpSalObject::SetBackground( SalColor ) {}
const SystemEnvData* SvpSalObject::GetSystemData() const { return &m_aSystemChildData; }

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

