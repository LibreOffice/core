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
#ifndef INCLUDED_CONNECTIVITY_COMMONTOOLS_HXX
#define INCLUDED_CONNECTIVITY_COMMONTOOLS_HXX

#include <sal/config.h>
#include <config_features.h>

#include <map>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <vector>
#include <cppuhelper/weakref.hxx>
#include <comphelper/stl_types.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <osl/interlck.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <connectivity/dbtoolsdllapi.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace com { namespace sun { namespace star { namespace util {
    struct Date;
    struct DateTime;
    struct Time;
}
}}}

#if HAVE_FEATURE_JAVA
namespace jvmaccess { class VirtualMachine; }
#endif

namespace connectivity
{
    OOO_DLLPUBLIC_DBTOOLS bool match(const sal_Unicode* pWild, const sal_Unicode* pStr, const sal_Unicode cEscape);
    inline bool match(const OUString &rWild, const OUString &rStr, const sal_Unicode cEscape)
    {
        return match(rWild.getStr(), rStr.getStr(), cEscape);
    }
    // typedefs
    typedef std::vector< ::com::sun::star::uno::WeakReferenceHelper > OWeakRefArray;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>    OSQLTable;

    typedef std::map<OUString,OSQLTable,comphelper::UStringMixLess> OSQLTables;

    // class ORefVector allows reference counting on a std::vector
    template< class VectorVal > class ORefVector
    {
        std::vector< VectorVal > m_vector;
        oslInterlockedCount         m_refCount;

    protected:
        virtual ~ORefVector(){}
    public:
        typedef std::vector< VectorVal > Vector;

        ORefVector() : m_refCount(0) {}
        ORefVector(size_t _st) : m_vector(_st) , m_refCount(0) {}
        ORefVector(const ORefVector& _rRH) : m_vector(_rRH.m_vector),m_refCount(0)
        {
        }
        ORefVector& operator=(const ORefVector& _rRH)
        {
            if ( &_rRH != this )
            {
                m_vector = _rRH.m_vector;
            }
            return *this;
        }

        std::vector< VectorVal > & get() { return m_vector; }
        std::vector< VectorVal > const & get() const { return m_vector; }

        inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW(())
            { return ::rtl_allocateMemory( nSize ); }
        inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW(())
            { ::rtl_freeMemory( pMem ); }
        inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW(())
            { return pMem; }
        inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW(())
            {}

        void acquire()
        {
            osl_atomic_increment( &m_refCount );
        }
        void release()
        {
            if (! osl_atomic_decrement( &m_refCount ))
                delete this;
        }

    };

    // class ORowVector incudes refcounting and initialze himself
    // with at least one element. This first element is reserved for
    // the bookmark
    template< class VectorVal > class ORowVector : public  ORefVector< VectorVal >
    {
    public:
        ORowVector() : ORefVector< VectorVal >(1){}
        ORowVector(size_t _st) : ORefVector< VectorVal >(_st+1)
            {}
    };

    typedef ORefVector< ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> > OSQLColumns;

    // search from __first to __last the column with the name _rVal
    // when no such column exist __last is returned
    OOO_DLLPUBLIC_DBTOOLS
    OSQLColumns::Vector::const_iterator find(   OSQLColumns::Vector::const_iterator __first,
                                        OSQLColumns::Vector::const_iterator __last,
                                        const OUString& _rVal,
                                        const ::comphelper::UStringMixEqual& _rCase);

    // search from __first to __last the column with the realname _rVal
    // when no such column exist __last is returned
    OOO_DLLPUBLIC_DBTOOLS
    OSQLColumns::Vector::const_iterator findRealName(   OSQLColumns::Vector::const_iterator __first,
                                                OSQLColumns::Vector::const_iterator __last,
                                                const OUString& _rVal,
                                                const ::comphelper::UStringMixEqual& _rCase);

    // the first two find methods are much faster than the one below
    // search from __first to __last the column with the property _rProp equals the value _rVal
    // when no such column exist __last is returned
    OOO_DLLPUBLIC_DBTOOLS
    OSQLColumns::Vector::const_iterator find(   OSQLColumns::Vector::const_iterator __first,
                                        OSQLColumns::Vector::const_iterator __last,
                                        const OUString& _rProp,
                                        const OUString& _rVal,
                                        const ::comphelper::UStringMixEqual& _rCase);

    OOO_DLLPUBLIC_DBTOOLS void checkDisposed(bool _bThrow) throw ( ::com::sun::star::lang::DisposedException );

#if HAVE_FEATURE_JAVA
    /** creates a java virtual machine
        @param  _rxContext
            The ORB.
        @return
            The JavaVM.
    */
    OOO_DLLPUBLIC_DBTOOLS ::rtl::Reference< jvmaccess::VirtualMachine > getJavaVM(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext);

    /** return <TRUE/> if the java class exists, otherwise <FALSE/>.
        @param  _pJVM
            The JavaVM.
        @param  _sClassName
            The class name to look for.
    */
    OOO_DLLPUBLIC_DBTOOLS bool existsJavaClassByName( const ::rtl::Reference< jvmaccess::VirtualMachine >& _pJVM,const OUString& _sClassName );
#endif
}

#define DECLARE_SERVICE_INFO()  \
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE; \
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE; \
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE \

#define IMPLEMENT_SERVICE_INFO(classname, implasciiname, serviceasciiname)  \
    OUString SAL_CALL classname::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception)   \
    {   \
        return OUString::createFromAscii(implasciiname); \
    }   \
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL classname::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)  \
    {   \
        ::com::sun::star::uno::Sequence< OUString > aSupported(1);   \
        aSupported[0] = OUString::createFromAscii(serviceasciiname); \
        return aSupported;  \
    }   \
    sal_Bool SAL_CALL classname::supportsService( const OUString& _rServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) \
    {   \
        return cppu::supportsService(this, _rServiceName); \
    }   \

#endif // INCLUDED_CONNECTIVITY_COMMONTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
