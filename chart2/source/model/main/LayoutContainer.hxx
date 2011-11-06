/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef CHART_LAYOUTCONTAINER_HXX
#define CHART_LAYOUTCONTAINER_HXX

#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/layout/XLayoutContainer.hpp>

#include "ServiceMacros.hxx"

#include <vector>
#include <map>

namespace chart
{

class LayoutContainer : public
    ::cppu::WeakImplHelper2<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::layout::XLayoutContainer >
{
public:
    LayoutContainer();
    virtual ~LayoutContainer();

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

protected:
    // ____ XLayoutContainer ____
    virtual void SAL_CALL addConstrainedElementByIdentifier( const ::rtl::OUString& aIdentifier, const ::com::sun::star::layout::Constraint& Constraint )
        throw (::com::sun::star::layout::IllegalConstraintException,
               ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addElementByIdentifier( const ::rtl::OUString& aIdentifier )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeElementByIdentifier( const ::rtl::OUString& aIdentifier )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setConstraintByIdentifier( const ::rtl::OUString& aIdentifier, const ::com::sun::star::layout::Constraint& Constraint )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::layout::Constraint SAL_CALL getConstraintByIdentifier( const ::rtl::OUString& aIdentifier )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
        ::rtl::OUString > SAL_CALL getElementIdentifiers()
        throw (::com::sun::star::uno::RuntimeException);

private:
    typedef ::std::vector< ::rtl::OUString > tLayoutElements;

    typedef ::std::map<
        ::rtl::OUString,
        ::com::sun::star::layout::Constraint > tConstraintsMap;

    tLayoutElements           m_aLayoutElements;
    tConstraintsMap           m_aConstraints;
};

} //  namespace chart

// CHART_LAYOUTCONTAINER_HXX
#endif
