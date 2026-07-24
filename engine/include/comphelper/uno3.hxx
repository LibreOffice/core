/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#ifndef INCLUDED_COMPHELPER_UNO3_HXX
#define INCLUDED_COMPHELPER_UNO3_HXX

#include <com/sun/star/uno/XAggregation.hpp>
#include <comphelper/sequence.hxx>


namespace comphelper
{
    /** used for declaring UNO3-Defaults, i.e. acquire/release
    */
    #define DECLARE_UNO3_DEFAULTS(classname, baseclass) \
        virtual void    acquire() noexcept override { baseclass::acquire(); }    \
        virtual void    release() noexcept override { baseclass::release(); }

    /** used for declaring UNO3-Defaults, i.e. acquire/release if you want to forward all queryInterfaces to the base class,
        (e.g. if you override queryAggregation)
    */
    #define DECLARE_UNO3_AGG_DEFAULTS(classname, baseclass) \
        virtual void            acquire() noexcept override { baseclass::acquire(); } \
        virtual void            release() noexcept override { baseclass::release(); }    \
        virtual cpo::uno::Any  queryInterface(const cpo::uno::Type& _rType) override \
            { return baseclass::queryInterface(_rType); }

    //= deriving from multiple XInterface-derived classes

    //= forwarding/merging XInterface functionality

    #define DECLARE_XINTERFACE( )   \
        virtual cpo::uno::Any queryInterface( const cpo::uno::Type& aType ) override; \
        virtual void acquire() noexcept override; \
        virtual void release() noexcept override;

    #define IMPLEMENT_FORWARD_REFCOUNT( classname, refcountbase ) \
        void classname::acquire() noexcept { refcountbase::acquire(); } \
        void classname::release() noexcept { refcountbase::release(); }

    #define IMPLEMENT_FORWARD_XINTERFACE2( classname, refcountbase, baseclass2 ) \
        IMPLEMENT_FORWARD_REFCOUNT( classname, refcountbase ) \
        cpo::uno::Any classname::queryInterface( const cpo::uno::Type& _rType ) \
        { \
            cpo::uno::Any aReturn = refcountbase::queryInterface( _rType ); \
            if ( !aReturn.hasValue() ) \
                aReturn = baseclass2::queryInterface( _rType ); \
            return aReturn; \
        }

    #define IMPLEMENT_FORWARD_XINTERFACE3( classname, refcountbase, baseclass2, baseclass3 ) \
        IMPLEMENT_FORWARD_REFCOUNT( classname, refcountbase ) \
        cpo::uno::Any classname::queryInterface( const cpo::uno::Type& _rType ) \
        { \
            cpo::uno::Any aReturn = refcountbase::queryInterface( _rType ); \
            if ( !aReturn.hasValue() ) \
            { \
                aReturn = baseclass2::queryInterface( _rType ); \
                if ( !aReturn.hasValue() ) \
                    aReturn = baseclass3::queryInterface( _rType ); \
            } \
            return aReturn; \
        }


    //= forwarding/merging XTypeProvider functionality

    #define DECLARE_XTYPEPROVIDER( )    \
        virtual cpo::uno::Sequence< cpo::uno::Type > getTypes(  ) override; \
        virtual cpo::uno::Sequence< sal_Int8 > getImplementationId(  ) override;

    #define IMPLEMENT_GET_IMPLEMENTATION_ID( classname ) \
        cpo::uno::Sequence< sal_Int8 > classname::getImplementationId(  ) \
        { \
            return cpo::uno::Sequence<sal_Int8>(); \
        }

    #define IMPLEMENT_FORWARD_XTYPEPROVIDER2( classname, baseclass1, baseclass2 ) \
        cpo::uno::Sequence< cpo::uno::Type > classname::getTypes(  ) \
        { \
            return ::comphelper::concatSequences( \
                baseclass1::getTypes(), \
                baseclass2::getTypes() \
            ); \
        } \
        \
        IMPLEMENT_GET_IMPLEMENTATION_ID( classname )

    /** ask for an iface of an aggregated object
        usage:<br/>
            Reference<XFoo> xFoo;<br/>
            if (query_aggregation(xAggregatedObject, xFoo))<br/>
                ...
    */
    template <class iface>
    bool query_aggregation(const css::uno::Reference< css::uno::XAggregation >& _rxAggregate, css::uno::Reference<iface>& _rxOut)
    {
        _rxOut.clear();
        if (_rxAggregate.is())
        {
            _rxAggregate->queryAggregation(cppu::UnoType<iface>::get())
                >>= _rxOut;
        }
        return _rxOut.is();
    }

    /** ask for an iface of an aggregated object
        usage:<br/>
            if (auto xFoo = query_aggregation<XFoo>(xAggregatedObject))<br/>
                ...
    */
    template <class iface>
    css::uno::Reference<iface> query_aggregation(const css::uno::Reference< css::uno::XAggregation >& _rxAggregate)
    {
        css::uno::Reference<iface> _rxOut;
        query_aggregation(_rxAggregate, _rxOut);
        return _rxOut;
    }
}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_UNO3_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
