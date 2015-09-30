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
#endif

#include <cstdarg>

#include <plugin/impl.hxx>

PluginEventListener::PluginEventListener( XPlugin_Impl* pPlugin,
                                          const char* url,
                                          const char* normurl,
                                          void* notifyData ) :
        m_pPlugin( pPlugin ),
        m_xPlugin( pPlugin ),
        m_pUrl( strdup( url ) ),
        m_pNormalizedUrl( strdup( normurl ) ),
        m_pNotifyData( notifyData )
{
}

PluginEventListener::~PluginEventListener()
{
    if( m_pUrl )
        free( m_pUrl );
    if( m_pNormalizedUrl )
        free( m_pNormalizedUrl );
}

void PluginEventListener::disposing( const css::lang::EventObject& /*rEvt*/ ) throw(std::exception)
{
    m_pPlugin->getPluginComm()->
        NPP_URLNotify( &m_pPlugin->getNPPInstance(),
                       m_pUrl,
                       NPRES_DONE,
                       m_pNotifyData );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
