/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: evtlstnr.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:50:15 $
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
#include "precompiled_extensions.hxx"

#if STLPORT_VERSION>=321
#include <cstdarg>
#endif

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

void PluginEventListener::disposing( const ::com::sun::star::lang::EventObject& /*rEvt*/ ) throw()
{
    m_pPlugin->getPluginComm()->
        NPP_URLNotify( m_pPlugin->getNPPInstance(),
                       m_pUrl,
                       NPRES_DONE,
                       m_pNotifyData );
}
