/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sfxresid.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 16:25:22 $
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
#include "precompiled_sfx2.hxx"

#ifndef GCC
#endif

#ifndef _TOOLS_SIMPLERESMGR_HXX_
#include <tools/simplerm.hxx>
#endif

#include "sfxresid.hxx"
#include "app.hxx"

// -----------------------------------------------------------------------

static ResMgr* pMgr=NULL;

SfxResId::SfxResId( USHORT nId ) :

    ResId( nId, GetResMgr() )
{
}

//============================================================================
//
// SfxSimpleResId Implementation.
//
//============================================================================

SfxSimpleResId::SfxSimpleResId(USHORT nID):
    m_sText( SFX_APP()->GetSimpleResManager()->ReadString(nID) )
{}

ResMgr* SfxResId::GetResMgr()
{
    if ( !pMgr )
    {
        pMgr = SfxApplication::CreateResManager("sfx");
    }

    return pMgr;
}

void SfxResId::DeleteResMgr()
{
    DELETEZ( pMgr );
}


