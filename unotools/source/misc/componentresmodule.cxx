/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: componentresmodule.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-25 16:01:06 $
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
#include "precompiled_unotools.hxx"

#ifndef UNOTOOLS_INC_UNOTOOLS_COMPONENTRESMODULE_HXX
#include <unotools/componentresmodule.hxx>
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

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
