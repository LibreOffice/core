/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: uno3.hxx,v $
 * $Revision: 1.9 $
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

#ifndef _COMPHELPER_UNO3_HXX_
#define _COMPHELPER_UNO3_HXX_

#include <osl/interlck.h>
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
    inline oslInterlockedCount increment(oslInterlockedCount& _counter) { return osl_incrementInterlockedCount(&_counter); }
    inline oslInterlockedCount decrement(oslInterlockedCount& _counter) { return osl_decrementInterlockedCount(&_counter); }

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
        ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL classname::getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException) \
        { \
            static ::cppu::OImplementationId* pId = NULL; \
            if (!pId) \
            { \
                ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() ); \
                if (!pId) \
                { \
                    static ::cppu::OImplementationId aId; \
                    pId = &aId; \
                } \
            } \
            return pId->getImplementationId(); \
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
    #define FORWARD_DECLARE_INTERFACE(NAME,XFACE) \
        namespace com \
        {   \
            namespace sun \
            {\
                namespace star \
                {\
                    namespace NAME \
                    {\
                        class XFACE; \
                    }\
                }\
            }\
        }\


//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_UNO3_HXX_

