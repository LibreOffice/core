/*************************************************************************
 *
 *  $RCSfile: contentbroker.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kso $ $Date: 2002-08-29 12:53:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIERFACTORY_HPP_
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDERMANAGER_HPP_
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#endif

#ifndef _UCBHELPER_CONTENTBROKER_HXX
#include <ucbhelper/contentbroker.hxx>
#endif

using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace rtl;

namespace ucb
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

    bool init() const;
    bool init();

    const Reference< XMultiServiceFactory >& getServiceManager() const
    { return m_xSMgr; }

    const Reference< XContentIdentifierFactory >& getIdFactory() const
    { init(); return m_xIdFac; }

    const Reference< XContentProvider >& getProvider() const
    { init(); return m_xProvider; }

    const Reference< XContentProviderManager >& getProviderManager() const
    { init(); return m_xProviderMgr; }

    const Reference< XCommandProcessor >& getCommandProcessor() const
    { init(); return m_xCommandProc; }
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
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );

        if ( !m_pTheBroker )
        {
            ContentBroker * pBroker = new ContentBroker( rSMgr, rArguments );

            // Force init to be able to detect UCB init trouble immediately.
            if ( pBroker->m_pImpl->init() )
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
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );

        if ( !m_pTheBroker )
        {
            ContentBroker * pBroker = new ContentBroker( rSMgr, rData );

            // Force init to be able to detect UCB init trouble immediately.
            if ( pBroker->m_pImpl->init() )
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
    osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() );

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
    if ( xComponent.is() ) // must not exist, if init() was never called.
    {
        m_xIdFac       = 0;
        m_xProvider    = 0;
        m_xProviderMgr = 0;

        xComponent->dispose();
    }
}

//=========================================================================
bool ContentBroker_Impl::init() const
{
    return const_cast< ContentBroker_Impl * >( this )->init();
}

//=========================================================================
bool ContentBroker_Impl::init()
{
    if ( !m_bInitDone )
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( !m_bInitDone )
        {
            Reference< XInterface > xIfc;

            if ( m_aProvData.size() > 0 )
            {
                xIfc
                    = m_xSMgr->createInstance(
                        OUString::createFromAscii(
                            "com.sun.star.ucb.UniversalContentBroker" ) );

                if ( xIfc.is() )
                {
                    m_xProviderMgr
                        = Reference< XContentProviderManager >( xIfc, UNO_QUERY );

                    if ( m_xProviderMgr.is() )
                    {
                        if ( !::ucb::configureUcb( m_xProviderMgr,
                                                   m_xSMgr,
                                                   m_aProvData,
                                                   0 ) )
                            OSL_ENSURE( false, "Failed to configure UCB!" );
                            return false;
                    }
                }
            }
            else
            {
                xIfc
                    = m_xSMgr->createInstanceWithArguments(
                        OUString::createFromAscii(
                            "com.sun.star.ucb.UniversalContentBroker" ),
                        m_aArguments );
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

} /* namespace ucb */

