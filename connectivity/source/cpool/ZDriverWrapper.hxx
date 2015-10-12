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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_CPOOL_ZDRIVERWRAPPER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_CPOOL_ZDRIVERWRAPPER_HXX

#include <com/sun/star/sdbc/XDriver.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/XAggregation.hpp>


namespace connectivity
{


    class OConnectionPool;

    typedef ::cppu::WeakImplHelper<   ::com::sun::star::sdbc::XDriver
                                  >   ODriverWrapper_BASE;

    class ODriverWrapper : public ODriverWrapper_BASE
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >
                            m_xDriverAggregate;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >
                            m_xDriver;
        OConnectionPool*    m_pConnectionPool;

    public:
        /** creates a new wrapper for a driver
            @param _rxAggregateDriver
                the driver to aggregate. The object will be reset to <NULL/> when returning from the ctor.
        */
        ODriverWrapper(
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >& _rxAggregateDriver,
            OConnectionPool* _pPool
            );



        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    protected:
        /// dtor
        virtual ~ODriverWrapper();
        // XDriver
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connect( const OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL acceptsURL( const OUString& url ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getMajorVersion(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getMinorVersion(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    };


}   // namespace connectivity


#endif // INCLUDED_CONNECTIVITY_SOURCE_CPOOL_ZDRIVERWRAPPER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
