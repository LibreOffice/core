/*************************************************************************
 *
 *  $RCSfile: contentbroker.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:37 $
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

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
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
    Sequence< Any >                         m_aArguments;
    vos::OMutex                             m_aMutex;
    sal_Bool                                m_bInitDone;

private:
    void init() const;
    void init();

public:
    ContentBroker_Impl( const Reference< XMultiServiceFactory >& rSMgr,
                        const Sequence< Any >& rArguments )
    : m_xSMgr( rSMgr ), m_aArguments( rArguments ), m_bInitDone( sal_False )
    {}

    ~ContentBroker_Impl();

    const Reference< XMultiServiceFactory >& getServiceManager() const
    { return m_xSMgr; }

    const Reference< XContentIdentifierFactory >& getIdFactory() const
    { init(); return m_xIdFac; }

    const Reference< XContentProvider >& getProvider() const
    { init(); return m_xProvider; }

    const Reference< XContentProviderManager >& getProviderManager() const
    { init(); return m_xProviderMgr; }
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
// static
sal_Bool ContentBroker::initialize(
                        const Reference< XMultiServiceFactory >& rSMgr,
                        const Sequence< Any >& rArguments )
{
    vos::OGuard aGuard( vos::OMutex::getGlobalMutex() );

    VOS_ENSURE( !m_pTheBroker,
                "ContentBroker::create - already created!" );

    if ( !m_pTheBroker )
        m_pTheBroker = new ContentBroker( rSMgr, rArguments );

    return m_pTheBroker != 0;
}

//=========================================================================
// static
void ContentBroker::deinitialize()
{
    vos::OGuard aGuard( vos::OMutex::getGlobalMutex() );

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
void ContentBroker_Impl::init() const
{
    const_cast< ContentBroker_Impl * >( this )->init();
}

//=========================================================================
void ContentBroker_Impl::init()
{
    vos::OGuard aGuard( m_aMutex );

    if ( !m_bInitDone )
    {
        m_bInitDone = sal_True;

        Reference< XInterface > xIfc
            = m_xSMgr->createInstanceWithArguments(
                OUString::createFromAscii(
                    "com.sun.star.ucb.UniversalContentBroker" ),
                m_aArguments );

        VOS_ENSURE( xIfc.is(), "Error creating UCB service!" );

        if ( xIfc.is() )
        {
            m_xIdFac
                 = Reference< XContentIdentifierFactory >( xIfc, UNO_QUERY );

            VOS_ENSURE( m_xIdFac.is(),
                        "UCB without XContentIdentifierFactory!" );

            m_xProvider
                 = Reference< XContentProvider >( xIfc, UNO_QUERY );

            VOS_ENSURE( m_xProvider.is(),
                        "UCB without XContentProvider!" );

            m_xProviderMgr
                 = Reference< XContentProviderManager >( xIfc, UNO_QUERY );

            VOS_ENSURE( m_xProviderMgr.is(),
                        "UCB without XContentProviderManager!" );
        }
    }
}

} /* namespace ucb */

