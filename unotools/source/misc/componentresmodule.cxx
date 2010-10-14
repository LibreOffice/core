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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
