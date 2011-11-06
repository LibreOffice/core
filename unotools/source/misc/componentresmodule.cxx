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
#include "precompiled_unotools.hxx"
#include <unotools/componentresmodule.hxx>

/** === begin UNO includes === **/
/** === end UNO includes === **/
#include <tools/resmgr.hxx>
#include <osl/diagnose.h>

//........................................................................
namespace utl
{
//........................................................................

    //====================================================================
    //= OComponentResModuleImpl
    //====================================================================
    /** PIMPL-class for OComponentResourceModule

        not threadsafe!
    */
    class OComponentResModuleImpl
    {
    private:
        ResMgr*         m_pRessources;
        bool            m_bInitialized;
        ::rtl::OString  m_sResFilePrefix;

    public:
        OComponentResModuleImpl( const ::rtl::OString& _rResFilePrefix )
            :m_pRessources( NULL )
            ,m_bInitialized( false )
            ,m_sResFilePrefix( _rResFilePrefix )
        {
        }

        ~OComponentResModuleImpl()
        {
            freeResManager();
        }

        /** releases our resource manager
        */
        void freeResManager();

        /** retrieves our resource manager
        */
        ResMgr* getResManager();

    private:
        OComponentResModuleImpl();                                              // never implemented
        OComponentResModuleImpl( const OComponentResModuleImpl& );              // never implemented
        OComponentResModuleImpl& operator=( const OComponentResModuleImpl& );   // never implemented
    };

    //--------------------------------------------------------------------
    void OComponentResModuleImpl::freeResManager()
    {
        delete m_pRessources, m_pRessources = NULL;
        m_bInitialized = false;
    }

    //--------------------------------------------------------------------
    ResMgr* OComponentResModuleImpl::getResManager()
    {
        if ( !m_pRessources && !m_bInitialized )
        {
            // create a manager with a fixed prefix
            ByteString aMgrName = m_sResFilePrefix;

            m_pRessources = ResMgr::CreateResMgr( aMgrName.GetBuffer() );
            OSL_ENSURE( m_pRessources,
                    ( ByteString( "OModuleImpl::getResManager: could not create the resource manager (file name: " )
                +=  aMgrName
                +=  ByteString( ")!" ) ).GetBuffer() );

            m_bInitialized = sal_True;
        }
        return m_pRessources;
    }

    //====================================================================
    //= OComponentResourceModule
    //====================================================================
    //--------------------------------------------------------------------
    OComponentResourceModule::OComponentResourceModule( const ::rtl::OString& _rResFilePrefix )
        :BaseClass()
        ,m_pImpl( new OComponentResModuleImpl( _rResFilePrefix ) )
    {
    }

    //--------------------------------------------------------------------
    OComponentResourceModule::~OComponentResourceModule()
    {
    }

    //-------------------------------------------------------------------------
    ResMgr* OComponentResourceModule::getResManager()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_pImpl->getResManager();
    }

    //--------------------------------------------------------------------------
    void OComponentResourceModule::onFirstClient()
    {
        BaseClass::onFirstClient();
    }

    //--------------------------------------------------------------------------
    void OComponentResourceModule::onLastClient()
    {
        m_pImpl->freeResManager();
        BaseClass::onLastClient();
    }

//........................................................................
} // namespace utl
//........................................................................
