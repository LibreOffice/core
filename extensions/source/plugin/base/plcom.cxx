/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: plcom.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:08:29 $
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

#ifdef SOLARIS
#include <limits>
#endif

#if STLPORT_VERSION>=321
#include <cstdarg>
#endif

#include <tools/fsys.hxx>
#include <plugin/impl.hxx>

PluginComm::PluginComm( const ::rtl::OString& rLibName ) :
        m_nRefCount( 0 ),
        m_aLibName( rLibName )
{
    PluginManager::get().getPluginComms().push_back( this );
}

PluginComm::~PluginComm()
{
    PluginManager::get().getPluginComms().remove( this );
    while( m_aFilesToDelete.size() )
    {
        String aFile = m_aFilesToDelete.front();
        m_aFilesToDelete.pop_front();
        DirEntry aEntry( aFile );
        aEntry.Kill();
    }
}
