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
