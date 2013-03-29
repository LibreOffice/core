/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#ifdef AIX
#define _LINUX_SOURCE_COMPAT
#include <sys/timer.h>
#undef _LINUX_SOURCE_COMPAT
#endif

#ifdef WNT
#include <prewin.h>
#include <postwin.h>
#undef OPTIONAL
#endif

#ifdef SOLARIS
#include <limits>
#endif

#include <cstdarg>

#include <osl/file.hxx>
#include <plugin/impl.hxx>

PluginComm::PluginComm( const ::rtl::OString& rLibName, bool bReusable ) :
        m_nRefCount( 0 ),
        m_aLibName( rLibName )
{
    if( bReusable )
        PluginManager::get().getPluginComms().push_back( this );
}

PluginComm::~PluginComm()
{
    PluginManager::get().getPluginComms().remove( this );
    while( m_aFilesToDelete.size() )
    {
        OUString aFile( m_aFilesToDelete.front() );
        m_aFilesToDelete.pop_front();
        osl::FileBase::getFileURLFromSystemPath( aFile, aFile );
        osl::File::remove( aFile );
    }
}

NPError PluginComm::NPP_SetWindow( XPlugin_Impl* i_pImpl )
{
    return NPP_SetWindow( i_pImpl->getNPPInstance(), i_pImpl->getNPWindow() );
}

NPError PluginComm::NPP_Destroy( XPlugin_Impl* i_pImpl, NPSavedData** save )
{
    return NPP_Destroy( i_pImpl->getNPPInstance(), save );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
