/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <unotools/componentresmodule.hxx>

#include <tools/resmgr.hxx>
#include <osl/diagnose.h>
#include <rtl/strbuf.hxx>

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
        ResMgr*         m_pResources;
        bool            m_bInitialized;
        ::rtl::OString  m_sResFilePrefix;

    public:
        OComponentResModuleImpl( const ::rtl::OString& _rResFilePrefix )
            :m_pResources( NULL )
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
        delete m_pResources, m_pResources = NULL;
        m_bInitialized = false;
    }

    //--------------------------------------------------------------------
    ResMgr* OComponentResModuleImpl::getResManager()
    {
        if ( !m_pResources && !m_bInitialized )
        {
            // create a manager with a fixed prefix
            rtl::OString aMgrName = m_sResFilePrefix;

            m_pResources = ResMgr::CreateResMgr( aMgrName.getStr() );
            OSL_ENSURE( m_pResources,
                    rtl::OStringBuffer( "OModuleImpl::getResManager: could not create the resource manager (file name: " )
                .append(aMgrName)
                .append(")!").getStr() );

            m_bInitialized = sal_True;
        }
        return m_pResources;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
