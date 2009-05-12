/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fwkresid.cxx,v $
 * $Revision: 1.8 $
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
#include "precompiled_framework.hxx"

#include "classes/fwkresid.hxx"
#include <tools/string.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

#include <rtl/strbuf.hxx>

namespace framework
{

ResMgr* FwkResId::GetResManager()
{
    static ResMgr*  pResMgr = NULL;

    if ( !pResMgr )
    {
        rtl::OStringBuffer aBuf( 32 );
        aBuf.append( "fwe" );

        vos::OGuard aSolarGuard( Application::GetSolarMutex() );
        pResMgr = ResMgr::CreateResMgr( aBuf.getStr() );
    }

    return pResMgr;
}

// -----------------------------------------------------------------------

FwkResId::FwkResId( USHORT nId ) :
    ResId( nId, *FwkResId::GetResManager() )
{
}

}

