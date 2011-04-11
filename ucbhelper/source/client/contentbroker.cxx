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
#include "precompiled_ucbhelper.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <ucbhelper/contentbroker.hxx>

using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

using ::rtl::OUString;

namespace
{
    osl::Mutex globalContentBrokerMutex;
    osl::Mutex & getGlobalContentBrokerMutex() { return globalContentBrokerMutex; }

} // namespace

namespace ucbhelper
{

//=========================================================================
//=========================================================================
//
// class ContentBroker_Impl.
//
//=========================================================================
//=========================================================================

class ContentBroker_Impl
{
    Reference< XMultiServiceFactory >       m_xSMgr;
    Reference< XContentIdentifierFactory >  m_xIdFac;
    Reference< XContentProvider >           m_xProvider;
    Reference< XContentProviderManager >    m_xProviderMgr;
    Reference< XCommandProcessor >          m_xCommandProc;
    osl::Mutex                              m_aMutex;
    Sequence< Any >                         m_aArguments;
    ContentProviderDataList                 m_aProvData;
    bool                                    m_bInitDone;

public:
    ContentBroker_Impl( const Reference< XMultiServiceFactory >& rSMgr,
                        const Sequence< Any >& rArguments )
    : m_xSMgr( rSMgr ), m_aArguments( rArguments ), m_bInitDone( sal_False )
    {}

    ContentBroker_Impl( const Reference< XMultiServiceFactory >& rSMgr,
                        const ContentProviderDataList & rData )
    : m_xSMgr( rSMgr ), m_aProvData( rData ), m_bInitDone( sal_False )
    {}

    ~ContentBroker_Impl();

    bool initialize();

    const Reference< XMultiServiceFactory >& getServiceManager() const
    { return m_xSMgr; }

    const Reference< XContentIdentifierFactory >& getIdFactory() const
    { return m_xIdFac; }

    const Reference< XContentProvider >& getProvider() const
    { return m_xProvider; }

    const Reference< XContentProviderManager >& getProviderManager() const
    { return m_xProviderMgr; }

    const Reference< XCommandProcessor >& getCommandProcessor() const
    { return m_xCommandProc; }
};

//=========================================================================
//=========================================================================
//
// ContentBroker Implementation.
//
//=========================================================================
//=========================================================================

// static member!
ContentBroker* ContentBroker::m_pTheBroker = 0;

//=========================================================================
ContentBroker::ContentBroker( const Reference< XMultiServiceFactory >& rSMgr,
                              const Sequence< Any >& rArguments )
{
    m_pImpl = new ContentBroker_Impl( rSMgr, rArguments );
}

//=========================================================================
ContentBroker::ContentBroker( const Reference< XMultiServiceFactory >& rSMgr,
                              const ContentProviderDataList & rData )
{
    m_pImpl = new ContentBroker_Impl( rSMgr, rData );
}

//=========================================================================
ContentBroker::~ContentBroker()
{
    delete m_pImpl;
}

//=========================================================================
Reference< XMultiServiceFactory > ContentBroker::getServiceManager() const
{
    return m_pImpl->getServiceManager();
}

//=========================================================================
Reference< XContentIdentifierFactory >
                ContentBroker::getContentIdentifierFactoryInterface() const
{
    return m_pImpl->getIdFactory();
}

//=========================================================================
Reference< XContentProvider >
                ContentBroker::getContentProviderInterface() const
{
    return m_pImpl->getProvider();
}

//=========================================================================
Reference< XContentProviderManager >
                ContentBroker::getContentProviderManagerInterface() const
{
    return m_pImpl->getProviderManager();
}

//=========================================================================
Reference< XCommandProcessor >
                ContentBroker::getCommandProcessorInterface() const
{
    return m_pImpl->getCommandProcessor();
}

//=========================================================================
// static
sal_Bool ContentBroker::initialize(
                        const Reference< XMultiServiceFactory >& rSMgr,
                        const Sequence< Any >& rArguments )
{
    OSL_ENSURE( !m_pTheBroker,
                "ContentBroker::initialize - already initialized!" );

    if ( !m_pTheBroker )
    {
        osl::Guard< osl::Mutex > aGuard( getGlobalContentBrokerMutex() );

        if ( !m_pTheBroker )
        {
            ContentBroker * pBroker = new ContentBroker( rSMgr, rArguments );

            // Force init to be able to detect UCB init trouble immediately.
            if ( pBroker->m_pImpl->initialize() )
                m_pTheBroker = pBroker;
            else
                delete pBroker;
        }
    }

    return m_pTheBroker != 0;
}

//=========================================================================
// static
sal_Bool ContentBroker::initialize(
                        const Reference< XMultiServiceFactory >& rSMgr,
                        const ContentProviderDataList & rData )
{
    OSL_ENSURE( !m_pTheBroker,
                "ContentBroker::initialize - already initialized!" );

    if ( !m_pTheBroker )
    {
        osl::Guard< osl::Mutex > aGuard( getGlobalContentBrokerMutex() );

        if ( !m_pTheBroker )
        {
            ContentBroker * pBroker = new ContentBroker( rSMgr, rData );

            // Force init to be able to detect UCB init trouble immediately.
            if ( pBroker->m_pImpl->initialize() )
                m_pTheBroker = pBroker;
            else
                delete pBroker;
        }
    }

    return m_pTheBroker != 0;
}

//=========================================================================
// static
void ContentBroker::deinitialize()
{
    osl::MutexGuard aGuard( getGlobalContentBrokerMutex() );

    delete m_pTheBroker;
    m_pTheBroker = 0;
}

//=========================================================================
// static
ContentBroker* ContentBroker::get()
{
    return m_pTheBroker;
}

//=========================================================================
//=========================================================================
//
// ContentBroker_Impl Implementation.
//
//=========================================================================
//=========================================================================

ContentBroker_Impl::~ContentBroker_Impl()
{
    Reference< XComponent > xComponent( m_xProvider, UNO_QUERY );
    if ( xComponent.is() )
    {
        m_xIdFac       = 0;
        m_xProvider    = 0;
        m_xProviderMgr = 0;

        xComponent->dispose();
    }
}

//=========================================================================
bool ContentBroker_Impl::initialize()
{
    if ( !m_bInitDone )
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( !m_bInitDone )
        {
            Reference< XInterface > xIfc;

            if ( m_aProvData.size() > 0 )
            {
                try
                {
                    xIfc = m_xSMgr->createInstance(
                            OUString(RTL_CONSTASCII_USTRINGPARAM(
                                "com.sun.star.ucb.UniversalContentBroker" )) );
                }
                catch ( Exception const & )
                {
                }

                if ( xIfc.is() )
                {
                    m_xProviderMgr
                        = Reference< XContentProviderManager >( xIfc, UNO_QUERY );

                    if ( m_xProviderMgr.is() )
                    {
                        ContentProviderDataList::const_iterator aEnd(m_aProvData.end());
                        for (ContentProviderDataList::const_iterator aIt(m_aProvData.begin());
                            aIt != aEnd; ++aIt)
                        {
                            registerAtUcb(m_xProviderMgr,
                                      m_xSMgr,
                                      aIt->ServiceName,
                                      aIt->Arguments,
                                      aIt->URLTemplate,
                                      0);
                        }

                    }
                }
            }
            else
            {
                try
                {
                    xIfc = m_xSMgr->createInstanceWithArguments(
                            OUString(RTL_CONSTASCII_USTRINGPARAM(
                                "com.sun.star.ucb.UniversalContentBroker" )),
                            m_aArguments );
                }
                catch ( Exception const & )
                {
                }
            }

            OSL_ENSURE( xIfc.is(), "Error creating UCB service!" );

            if ( !xIfc.is() )
                return false;


            m_xIdFac
                = Reference< XContentIdentifierFactory >( xIfc, UNO_QUERY );

            OSL_ENSURE( m_xIdFac.is(),
                "UCB without required interface XContentIdentifierFactory!" );

            if ( !m_xIdFac.is() )
                return false;

            m_xProvider = Reference< XContentProvider >( xIfc, UNO_QUERY );

            OSL_ENSURE( m_xProvider.is(),
                "UCB without required interface XContentProvider!" );

            if ( !m_xProvider.is() )
                return false;

            if ( !m_xProviderMgr.is() )
                m_xProviderMgr
                    = Reference< XContentProviderManager >( xIfc, UNO_QUERY );

            OSL_ENSURE( m_xProviderMgr.is(),
                "UCB without required interface XContentProviderManager!" );

            if ( !m_xProviderMgr.is() )
                return false;

            m_xCommandProc = Reference< XCommandProcessor >( xIfc, UNO_QUERY );

            OSL_ENSURE( m_xCommandProc.is(),
                "UCB without required interface XCommandProcessor!" );

            if ( !m_xCommandProc.is() )
                return false;

            // Everything okay.
            m_bInitDone = sal_True;
        }
    }

    return true;
}

} /* namespace ucbhelper */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
