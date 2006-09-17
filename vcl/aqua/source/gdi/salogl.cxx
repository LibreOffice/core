/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salogl.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:45:02 $
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
#include <salgdi.hxx>
#endif
#ifndef _SV_SALOGL_HXX
#include <salogl.hxx>
#endif


// ========================================================================

// Initialize static data members
VCLVIEW             SalOpenGL::mhOGLLastDC = 0;
ULONG               SalOpenGL::mnOGLState = OGL_STATE_UNLOADED;

// ========================================================================

SalOpenGL::SalOpenGL( SalGraphics* pGraphics )
{
}

// ------------------------------------------------------------------------

SalOpenGL::~SalOpenGL()
{
}

// ------------------------------------------------------------------------

BOOL SalOpenGL::Create()
{
    return FALSE;
}

// ------------------------------------------------------------------------

void SalOpenGL::Release()
{
}

// ------------------------------------------------------------------------

void *SalOpenGL::GetOGLFnc( const char* pFncName )
{
    return NULL;
}

// ------------------------------------------------------------------------

void SalOpenGL::OGLEntry( SalGraphics* pGraphics )
{
}

// ------------------------------------------------------------------------

void SalOpenGL::OGLExit( SalGraphics* pGraphics )
{
}
