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

#ifndef INCLUDED_COMPHELPER_UNO3_HXX
#define INCLUDED_COMPHELPER_UNO3_HXX

#include <rtl/instance.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/uno/XAggregation.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>


namespace comphelper
{
    /** used for declaring UNO3-Defaults, i.e. acquire/release
    */
    #define DECLARE_UNO3_DEFAULTS(classname, baseclass) \
        virtual void    SAL_CALL acquire() SAL_THROW_IfNotObjectiveC () override { baseclass::acquire(); }    \
        virtual void    SAL_CALL release() SAL_THROW_IfNotObjectiveC () override { baseclass::release(); }

    /** used for declaring UNO3-Defaults, i.e. acquire/release if you want to forward all queryInterfaces to the base class,
        (e.g. if you override queryAggregation)
    */
    #define DECLARE_UNO3_AGG_DEFAULTS(classname, baseclass) \
        virtual void           SAL_CALL acquire() SAL_THROW_IfNotObjectiveC () override { baseclass::acquire(); } \
        virtual void           SAL_CALL release() SAL_THROW_IfNotObjectiveC () override { baseclass::release(); }    \
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) SAL_THROW_IfNotObjectiveC (css::uno::RuntimeException, std::exception) override \
            { return baseclass::queryInterface(_rType); }

    /** Use this macro to forward XComponent methods to base class

        When using the ::cppu::WeakComponentImplHelper base classes to
        implement a UNO interface, a problem occurs when the interface
        itself already derives from XComponent (like e.g. awt::XWindow
        or awt::XControl): ::cppu::WeakComponentImplHelper is then
        still abstract. Using this macro in the most derived class
        definition provides overrides for the XComponent methods,
        forwarding them to the given baseclass.

        @param classname
        Name of the class this macro is issued within

        @param baseclass
        Name of the baseclass that should have the XInterface methods
        forwarded to - that's usually the WeakComponentImplHelperN base

        @param implhelper
        Name of the baseclass that should have the XComponent methods
        forwarded to - in the case of the WeakComponentImplHelper,
        that would be ::cppu::WeakComponentImplHelperBase
    */
    #define DECLARE_UNO3_XCOMPONENT_AGG_DEFAULTS(classname, baseclass, implhelper) \
        virtual void SAL_CALL acquire() SAL_THROW_IfNotObjectiveC () override { baseclass::acquire(); }   \
        virtual void SAL_CALL release() SAL_THROW_IfNotObjectiveC () override { baseclass::release(); }   \
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) SAL_THROW_IfNotObjectiveC (css::uno::RuntimeException, std::exception) override \
            { return baseclass::queryInterface(_rType); }                               \
        virtual void SAL_CALL dispose() SAL_THROW_IfNotObjectiveC (css::uno::RuntimeException, std::exception) override \
        {                                                                               \
            implhelper::dispose();                                                      \
        }                                                                               \
        virtual void SAL_CALL addEventListener(                                         \
            css::uno::Reference< css::lang::XEventListener > const & xListener ) SAL_THROW_IfNotObjectiveC (css::uno::RuntimeException, std::exception) override \
        {                                                                               \
            implhelper::addEventListener(xListener);                                        \
        }                                                                               \
        virtual void SAL_CALL removeEventListener(                                      \
            css::uno::Reference< css::lang::XEventListener > const & xListener ) SAL_THROW_IfNotObjectiveC (css::uno::RuntimeException, std::exception) override \
        {                                                                               \
            implhelper::removeEventListener(xListener);                                 \
        }

    //= deriving from multiple XInterface-derived classes

    //= forwarding/merging XInterface funtionality

    #define DECLARE_XINTERFACE( )   \
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) SAL_THROW_IfNotObjectiveC (css::uno::RuntimeException, std::exception) override; \
        virtual void SAL_CALL acquire() SAL_THROW_IfNotObjectiveC () override; \
        virtual void SAL_CALL release() SAL_THROW_IfNotObjectiveC () override;

    #define IMPLEMENT_FORWARD_REFCOUNT( classname, refcountbase ) \
        void SAL_CALL classname::acquire() SAL_THROW_IfNotObjectiveC () { refcountbase::acquire(); } \
        void SAL_CALL classname::release() SAL_THROW_IfNotObjectiveC () { refcountbase::release(); }

    #define IMPLEMENT_FORWARD_XINTERFACE2( classname, refcountbase, baseclass2 ) \
        IMPLEMENT_FORWARD_REFCOUNT( classname, refcountbase ) \
        css::uno::Any SAL_CALL classname::queryInterface( const css::uno::Type& _rType ) SAL_THROW_IfNotObjectiveC (css::uno::RuntimeException, std::exception) \
        { \
            css::uno::Any aReturn = refcountbase::queryInterface( _rType ); \
            if ( !aReturn.hasValue() ) \
                aReturn = baseclass2::queryInterface( _rType ); \
            return aReturn; \
        }

    #define IMPLEMENT_FORWARD_XINTERFACE3( classname, refcountbase, baseclass2, baseclass3 ) \
        IMPLEMENT_FORWARD_REFCOUNT( classname, refcountbase ) \
        css::uno::Any SAL_CALL classname::queryInterface( const css::uno::Type& _rType ) SAL_THROW_IfNotObjectiveC (css::uno::RuntimeException, std::exception) \
        { \
            css::uno::Any aReturn = refcountbase::queryInterface( _rType ); \
            if ( !aReturn.hasValue() ) \
            { \
                aReturn = baseclass2::queryInterface( _rType ); \
                if ( !aReturn.hasValue() ) \
                    aReturn = baseclass3::queryInterface( _rType ); \
            } \
            return aReturn; \
        }


    //= forwarding/merging XTypeProvider funtionality

    #define DECLARE_XTYPEPROVIDER( )    \
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) SAL_THROW_IfNotObjectiveC (css::uno::RuntimeException, std::exception) override; \
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) SAL_THROW_IfNotObjectiveC (css::uno::RuntimeException, std::exception) override;

    #define IMPLEMENT_GET_IMPLEMENTATION_ID( classname ) \
        css::uno::Sequence< sal_Int8 > SAL_CALL classname::getImplementationId(  ) SAL_THROW_IfNotObjectiveC (css::uno::RuntimeException, std::exception) \
        { \
            return css::uno::Sequence<sal_Int8>(); \
        }

    #define IMPLEMENT_FORWARD_XTYPEPROVIDER2( classname, baseclass1, baseclass2 ) \
        css::uno::Sequence< css::uno::Type > SAL_CALL classname::getTypes(  ) SAL_THROW_IfNotObjectiveC (css::uno::RuntimeException, std::exception) \
        { \
            return ::comphelper::concatSequences( \
                baseclass1::getTypes(), \
                baseclass2::getTypes() \
            ); \
        } \
        \
        IMPLEMENT_GET_IMPLEMENTATION_ID( classname )

    #define IMPLEMENT_FORWARD_XTYPEPROVIDER3( classname, baseclass1, baseclass2, baseclass3 ) \
        css::uno::Sequence< css::uno::Type > SAL_CALL classname::getTypes(  ) SAL_THROW_IfNotObjectiveC (css::uno::RuntimeException, std::exception) \
        { \
            return ::comphelper::concatSequences( \
                baseclass1::getTypes(), \
                baseclass2::getTypes(), \
                baseclass3::getTypes() \
            ); \
        } \
        \
        IMPLEMENT_GET_IMPLEMENTATION_ID( classname )


    /** ask for an iface of an aggregated object
        usage:<br/>
            Reference<XFoo> xFoo;<br/>
            if (query_aggregation(xAggregatedObject, xFoo))<br/>
                ....
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
}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_UNO3_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
