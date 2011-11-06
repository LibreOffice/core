/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
        String aFile = m_aFilesToDelete.front();
        m_aFilesToDelete.pop_front();
        DirEntry aEntry( aFile );
        aEntry.Kill();
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

