/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salogl.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-05 08:28:49 $
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

#ifndef _SV_SALGDI_HXX
#include <vcl/salgdi.hxx>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_SALOGL_HXX
#include <vcl/salogl.hxx>
#endif
#ifndef _SV_SALOGL_H
#include <salogl.h>
#endif


// ========================================================================

// Initialize static data members
CarbonViewRef           AquaSalOpenGL::mhOGLLastDC = 0;
ULONG               AquaSalOpenGL::mnOGLState = OGL_STATE_UNLOADED;

// ========================================================================

AquaSalOpenGL::AquaSalOpenGL( SalGraphics* pGraphics )
{
}

// ------------------------------------------------------------------------

AquaSalOpenGL::~AquaSalOpenGL()
{
}

// ------------------------------------------------------------------------

BOOL AquaSalOpenGL::Create()
{
    return FALSE;
}

// ------------------------------------------------------------------------

void AquaSalOpenGL::Release()
{
}

// ------------------------------------------------------------------------

oglFunction AquaSalOpenGL::GetOGLFnc( const char* pFncName )
{
    return NULL;
}

// ------------------------------------------------------------------------

void AquaSalOpenGL::OGLEntry( SalGraphics* pGraphics )
{
}

// ------------------------------------------------------------------------

void AquaSalOpenGL::OGLExit( SalGraphics* pGraphics )
{
}








void AquaSalOpenGL::StartScene( SalGraphics* pGraphics )
{

}

bool AquaSalOpenGL::IsValid()
{
    return FALSE;
}

void AquaSalOpenGL::StopScene()
{

}
