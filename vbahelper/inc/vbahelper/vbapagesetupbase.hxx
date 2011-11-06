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


#ifndef VBA_PAGESETUPBASE_HXX
#define VBA_PAGESETUPBASE_HXX

#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/XPageSetupBase.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ooo::vba::XPageSetupBase > VbaPageSetupBase_BASE;

class VBAHELPER_DLLPUBLIC VbaPageSetupBase :  public VbaPageSetupBase_BASE
{
protected:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::beans::XPropertySet > mxPageProps;
    sal_Int32 mnOrientLandscape;
    sal_Int32 mnOrientPortrait;

    VbaPageSetupBase( const css::uno::Reference< ov::XHelperInterface >& xParent,
                    const css::uno::Reference< css::uno::XComponentContext >& xContext ) throw (css::uno::RuntimeException);
public:
    virtual ~VbaPageSetupBase(){}

    // Attribute
    virtual double SAL_CALL getTopMargin() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTopMargin( double margin ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getBottomMargin() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setBottomMargin( double margin ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getRightMargin() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setRightMargin( double margin ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getLeftMargin() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLeftMargin( double margin ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getHeaderMargin() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setHeaderMargin( double margin ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getFooterMargin() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setFooterMargin( double margin ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getOrientation() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setOrientation( sal_Int32 orientation ) throw (css::uno::RuntimeException);
};
#endif
