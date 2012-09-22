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

#ifndef _COMPHELPER_UNO3_HXX_
#define _COMPHELPER_UNO3_HXX_

#include <osl/interlck.h>
#include <rtl/instance.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/uno/XAggregation.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>

//.........................................................................
namespace comphelper
{
//.........................................................................

//=========================================================================

    /// manipulate ref counts without calling acquire/release
    inline oslInterlockedCount increment(oslInterlockedCount& _counter) { return osl_atomic_increment(&_counter); }
    inline oslInterlockedCount decrement(oslInterlockedCount& _counter) { return osl_atomic_decrement(&_counter); }

//=========================================================================

    /** used for declaring UNO3-Defaults, i.e. acquire/release
    */
    #define DECLARE_UNO3_DEFAULTS(classname, baseclass) \
        virtual void    SAL_CALL acquire() throw() { baseclass::acquire(); }    \
        virtual void    SAL_CALL release() throw() { baseclass::release(); }    \
        void            SAL_CALL PUT_SEMICOLON_AT_THE_END()

    /** used for declaring UNO3-Defaults, i.e. acquire/release if you want to forward all queryInterfaces to the base class,
        (e.g. if you overload queryAggregation)
    */
    #define DECLARE_UNO3_AGG_DEFAULTS(classname, baseclass) \
        virtual void            SAL_CALL acquire() throw() { baseclass::acquire(); } \
        virtual void            SAL_CALL release() throw() { baseclass::release(); }    \
        virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException) \
            { return baseclass::queryInterface(_rType); } \
        void                    SAL_CALL PUT_SEMICOLON_AT_THE_END()

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
    #define DECLARE_UNO3_XCOMPONENT_DEFAULTS(classname, baseclass, implhelper) \
        virtual void SAL_CALL acquire() throw() { baseclass::acquire(); }   \
        virtual void SAL_CALL release() throw() { baseclass::release(); }   \
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException) \
        {                                                                               \
            implhelper::dispose();                                                      \
        }                                                                               \
        virtual void SAL_CALL addEventListener(                                         \
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > const & xListener ) throw (::com::sun::star::uno::RuntimeException) \
        {                                                                               \
            implhelper::addEventListener(xListener);                                        \
        }                                                                               \
        virtual void SAL_CALL removeEventListener(                                      \
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > const & xListener ) throw (::com::sun::star::uno::RuntimeException) \
        {                                                                               \
            implhelper::removeEventListener(xListener);                                 \
        }                                                                               \
        void         SAL_CALL PUT_SEMICOLON_AT_THE_END()


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
        virtual void SAL_CALL acquire() throw() { baseclass::acquire(); }   \
        virtual void SAL_CALL release() throw() { baseclass::release(); }   \
        virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException) \
            { return baseclass::queryInterface(_rType); }                               \
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException) \
        {                                                                               \
            implhelper::dispose();                                                      \
        }                                                                               \
        virtual void SAL_CALL addEventListener(                                         \
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > const & xListener ) throw (::com::sun::star::uno::RuntimeException) \
        {                                                                               \
            implhelper::addEventListener(xListener);                                        \
        }                                                                               \
        virtual void SAL_CALL removeEventListener(                                      \
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > const & xListener ) throw (::com::sun::star::uno::RuntimeException) \
        {                                                                               \
            implhelper::removeEventListener(xListener);                                 \
        }                                                                               \
        void         SAL_CALL PUT_SEMICOLON_AT_THE_END()


    //=====================================================================
    //= deriving from multiple XInterface-derived classes
    //=====================================================================
    //= forwarding/merging XInterface funtionality
    //=====================================================================
    #define DECLARE_XINTERFACE( )   \
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException); \
        virtual void SAL_CALL acquire() throw(); \
        virtual void SAL_CALL release() throw();

    #define IMPLEMENT_FORWARD_REFCOUNT( classname, refcountbase ) \
        void SAL_CALL classname::acquire() throw() { refcountbase::acquire(); } \
        void SAL_CALL classname::release() throw() { refcountbase::release(); }

    #define IMPLEMENT_FORWARD_XINTERFACE2( classname, refcountbase, baseclass2 ) \
        IMPLEMENT_FORWARD_REFCOUNT( classname, refcountbase ) \
        ::com::sun::star::uno::Any SAL_CALL classname::queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException) \
        { \
            ::com::sun::star::uno::Any aReturn = refcountbase::queryInterface( _rType ); \
            if ( !aReturn.hasValue() ) \
                aReturn = baseclass2::queryInterface( _rType ); \
            return aReturn; \
        }

    #define IMPLEMENT_FORWARD_XINTERFACE3( classname, refcountbase, baseclass2, baseclass3 ) \
        IMPLEMENT_FORWARD_REFCOUNT( classname, refcountbase ) \
        ::com::sun::star::uno::Any SAL_CALL classname::queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException) \
        { \
            ::com::sun::star::uno::Any aReturn = refcountbase::queryInterface( _rType ); \
            if ( !aReturn.hasValue() ) \
            { \
                aReturn = baseclass2::queryInterface( _rType ); \
                if ( !aReturn.hasValue() ) \
                    aReturn = baseclass3::queryInterface( _rType ); \
            } \
            return aReturn; \
        }

    //=====================================================================
    //= forwarding/merging XTypeProvider funtionality
    //=====================================================================
    #define DECLARE_XTYPEPROVIDER( )    \
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException); \
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException);

    #define IMPLEMENT_GET_IMPLEMENTATION_ID( classname ) \
        namespace \
        { \
            class the##classname##ImplementationId : public rtl::Static< ::cppu::OImplementationId, the##classname##ImplementationId> {}; \
        } \
        ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL classname::getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException) \
        { \
            return the##classname##ImplementationId::get().getImplementationId(); \
        }

    #define IMPLEMENT_FORWARD_XTYPEPROVIDER2( classname, baseclass1, baseclass2 ) \
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL classname::getTypes(  ) throw (::com::sun::star::uno::RuntimeException) \
        { \
            return ::comphelper::concatSequences( \
                baseclass1::getTypes(), \
                baseclass2::getTypes() \
            ); \
        } \
        \
        IMPLEMENT_GET_IMPLEMENTATION_ID( classname )

    #define IMPLEMENT_FORWARD_XTYPEPROVIDER3( classname, baseclass1, baseclass2, baseclass3 ) \
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL classname::getTypes(  ) throw (::com::sun::star::uno::RuntimeException) \
        { \
            return ::comphelper::concatSequences( \
                baseclass1::getTypes(), \
                baseclass2::getTypes(), \
                baseclass3::getTypes() \
            ); \
        } \
        \
        IMPLEMENT_GET_IMPLEMENTATION_ID( classname )

//=========================================================================

    /** ask for an iface of an aggregated object
        usage:<br/>
            Reference<XFoo> xFoo;<br/>
            if (query_aggregation(xAggregatedObject, xFoo))<br/>
                ....
    */
    template <class iface>
    sal_Bool query_aggregation(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >& _rxAggregate, ::com::sun::star::uno::Reference<iface>& _rxOut)
    {
        _rxOut = static_cast<iface*>(NULL);
        if (_rxAggregate.is())
        {
            ::com::sun::star::uno::Any aCheck = _rxAggregate->queryAggregation(
                iface::static_type());
            if (aCheck.hasValue())
                _rxOut = *(::com::sun::star::uno::Reference<iface>*)aCheck.getValue();
        }
        return _rxOut.is();
    }

    /** ask for an iface of an object
        usage:<br/>
            Reference<XFoo> xFoo;<br/>
            if (query_interface(xAnything, xFoo))<br/>
                ....
    */
    template <class iface>
    sal_Bool query_interface(const InterfaceRef& _rxObject, ::com::sun::star::uno::Reference<iface>& _rxOut)
    {
        _rxOut = static_cast<iface*>(NULL);
        if (_rxObject.is())
        {
            ::com::sun::star::uno::Any aCheck = _rxObject->queryInterface(
                iface::static_type());
            if(aCheck.hasValue())
            {
                _rxOut = *(::com::sun::star::uno::Reference<iface>*)aCheck.getValue();
                return _rxOut.is();
            }
        }
        return sal_False;
    }

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_UNO3_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
