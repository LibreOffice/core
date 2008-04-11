/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salogl.cxx,v $
 * $Revision: 1.10 $
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

#include "vcl/salgdi.hxx"
#include "salgdi.h"
#include "vcl/salogl.hxx"
#include "salogl.h"


// ========================================================================

// Initialize static data members
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
