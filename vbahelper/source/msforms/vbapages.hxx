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


#ifndef SC_VBA_PAGES_HXX
#define SC_VBA_PAGES_HXX

#include <ooo/vba/office/MsoShapeType.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/msforms/XPages.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase1.hxx>

#include <vbahelper/vbacollectionimpl.hxx>
typedef CollTestImplHelper<
ov::msforms::XPages > ScVbaPages_BASE;

class ScVbaPages : public ScVbaPages_BASE
{
protected:
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
public:
    ScVbaPages( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xPages ) throw ( css::lang::IllegalArgumentException );
    virtual ~ScVbaPages() {}
        // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
     // ScVbaPages_BASE
     virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );

};
#endif//SC_VBA_SHAPE_HXX
