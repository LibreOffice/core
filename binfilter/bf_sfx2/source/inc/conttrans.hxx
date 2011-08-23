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

#ifndef _CONTTRANS_HXX
#define _CONTTRANS_HXX

// interfaces and structs
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_XCONTENTTRANSMITTER_HPP_
#include <com/sun/star/ucb/XContentTransmitter.hpp>
#endif

// helper classes and functions
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _UNO_DISPATCHER_H_
#include <uno/dispatcher.h>
#endif
#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif


#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <bf_svtools/lstner.hxx>
#endif
namespace binfilter {

//=========================================================================

#define CT_SERVICE_NAME L"com.sun.star.ucb.ContentTransmitter"
#define FLAG_SET_READONLY	0x01
#define FLAG_NOTIFY_ANCHOR	0x02

//=========================================================================

class ContentTransmitter : public SfxListener,
                           public cppu::WeakImplHelper2< 
                                ::com::sun::star::ucb::XContentTransmitter,
                                ::com::sun::star::lang::XServiceInfo >
{
    ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > m_xFact;
    ::rtl::OUString	m_aSource;
    ::rtl::OUString	m_aDest;
    long			m_nFlags;

public:

    ContentTransmitter( const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >& rFact )
        : m_xFact( rFact )
    {}

    // SfxListener
    virtual void
    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // XContentTransmitter
    virtual void SAL_CALL
    transmit( const ::rtl::OUString& Source,
              const ::rtl::OUString& Destination,
              long				   Flags )
        throw( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL
    getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
    supportsService( const ::rtl::OUString& ServiceName )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
    getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException );

    // static Helper functions
    static ::com::sun::star::uno::Sequence< ::rtl::OUString >
    getSupportedServiceNames_Static();

    static ::rtl::OUString
    getImplementationName_Static() { return CT_SERVICE_NAME; }

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >
    createServiceFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxServiceMgr );
};


}//end of namespace binfilter
#endif
