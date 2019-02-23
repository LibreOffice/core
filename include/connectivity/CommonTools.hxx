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
#include <config_java.h>

#include <map>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <vector>
#include <cppuhelper/weakref.hxx>
#include <comphelper/stl_types.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <connectivity/dbtoolsdllapi.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <salhelper/simplereferenceobject.hxx>

namespace com::sun::star::uno { class XComponentContext; }

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
    typedef std::vector< css::uno::WeakReferenceHelper >           OWeakRefArray;
    typedef css::uno::Reference< css::sdbcx::XColumnsSupplier>     OSQLTable;

    typedef std::map<OUString,OSQLTable,comphelper::UStringMixLess> OSQLTables;

    // class ORefVector allows reference counting on a std::vector
    template< class VectorVal > class ORefVector : public salhelper::SimpleReferenceObject
    {
        std::vector< VectorVal > m_vector;

    protected:
        virtual ~ORefVector() override {}
    public:
        typedef std::vector< VectorVal > Vector;

        ORefVector() {}
        ORefVector(size_t _st) : m_vector(_st) {}
        ORefVector(const ORefVector& rOther)
            : salhelper::SimpleReferenceObject()
            , m_vector(rOther.m_vector)
        {}

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

    };

    // class ORowVector includes refcounting and initialize himself
    // with at least one element. This first element is reserved for
    // the bookmark
    template< class VectorVal > class ORowVector : public  ORefVector< VectorVal >
    {
    public:
        ORowVector() : ORefVector< VectorVal >(1){}
        ORowVector(size_t _st) : ORefVector< VectorVal >(_st+1)
            {}
    };

    typedef ORefVector< css::uno::Reference< css::beans::XPropertySet> > OSQLColumns;

    // search from first to last the column with the name _rVal
    // when no such column exist last is returned
    OOO_DLLPUBLIC_DBTOOLS
    OSQLColumns::Vector::const_iterator find(   const OSQLColumns::Vector::const_iterator& first,
                                        const OSQLColumns::Vector::const_iterator& last,
                                        const OUString& _rVal,
                                        const ::comphelper::UStringMixEqual& _rCase);

    // search from first to last the column with the realname _rVal
    // when no such column exist last is returned
    OOO_DLLPUBLIC_DBTOOLS
    OSQLColumns::Vector::const_iterator findRealName(   const OSQLColumns::Vector::const_iterator& first,
                                                const OSQLColumns::Vector::const_iterator& last,
                                                const OUString& _rVal,
                                                const ::comphelper::UStringMixEqual& _rCase);

    // the first two find methods are much faster than the one below
    // search from first to last the column with the property _rProp equals the value _rVal
    // when no such column exist last is returned
    OOO_DLLPUBLIC_DBTOOLS
    OSQLColumns::Vector::const_iterator find(   OSQLColumns::Vector::const_iterator first,
                                        const OSQLColumns::Vector::const_iterator& last,
                                        const OUString& _rProp,
                                        const OUString& _rVal,
                                        const ::comphelper::UStringMixEqual& _rCase);

    /// @throws css::lang::DisposedException
    OOO_DLLPUBLIC_DBTOOLS void checkDisposed(bool _bThrow);

#if HAVE_FEATURE_JAVA
    /** creates a java virtual machine
        @param  _rxContext
            The ORB.
        @return
            The JavaVM.
    */
    OOO_DLLPUBLIC_DBTOOLS ::rtl::Reference< jvmaccess::VirtualMachine > getJavaVM(const css::uno::Reference< css::uno::XComponentContext >& _rxContext);

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
    virtual OUString SAL_CALL getImplementationName(  ) override; \
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) override; \
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override \

#define IMPLEMENT_SERVICE_INFO(classname, implasciiname, serviceasciiname)  \
    OUString SAL_CALL classname::getImplementationName(  )   \
    {   \
        return OUString(implasciiname); \
    }   \
    css::uno::Sequence< OUString > SAL_CALL classname::getSupportedServiceNames(  )  \
    {   \
        css::uno::Sequence< OUString > aSupported { serviceasciiname }; \
        return aSupported;  \
    }   \
    sal_Bool SAL_CALL classname::supportsService( const OUString& rServiceName ) \
    {   \
        return cppu::supportsService(this, rServiceName); \
    }   \

#endif // INCLUDED_CONNECTIVITY_COMMONTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
