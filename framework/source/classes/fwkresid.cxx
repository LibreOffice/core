/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fwkresid.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:47:03 $
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
#include "precompiled_framework.hxx"

#include "classes/fwkresid.hxx"

#ifndef _TOOLS_STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#define U2S(STRING)     ::rtl::OUStringToOString(STRING, RTL_TEXTENCODING_UTF8)

namespace framework
{

ResMgr* FwkResId::GetResManager()
{
    static ResMgr*  pResMgr = NULL;

    String aMgrName = String::CreateFromAscii( "fwe" );
    aMgrName += String::CreateFromInt32(SUPD); // current version number

    if ( !pResMgr )
    {
        vos::OGuard aSolarGuard( Application::GetSolarMutex() );
        pResMgr = ResMgr::CreateResMgr( U2S( aMgrName ));
    }

    return pResMgr;
}

// -----------------------------------------------------------------------

FwkResId::FwkResId( USHORT nId ) :
    ResId( nId, FwkResId::GetResManager() )
{
}

}

