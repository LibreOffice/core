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


#ifndef SW_VBA_STYLE_HXX
#define SW_VBA_STYLE_HXX
#include <ooo/vba/word/XStyle.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <ooo/vba/word/XFont.hpp>


typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XStyle > SwVbaStyle_BASE;

class SwVbaStyle : public SwVbaStyle_BASE
{
private:
    css::uno::Reference< css::beans::XPropertySet > mxStyleProps;
    css::uno::Reference< css::style::XStyle > mxStyle;
public:
    SwVbaStyle( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::beans::XPropertySet >& _xPropertySet ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
    virtual ~SwVbaStyle(){}

    static void setStyle( const css::uno::Reference< css::beans::XPropertySet >& xTCProps, const css::uno::Reference< ooo::vba::word::XStyle >& xStyle ) throw (css::uno::RuntimeException);
    static rtl::OUString getOOoStyleTypeFromMSWord( sal_Int32 _wdStyleType );
    static sal_Int32 getLanguageID( const css::uno::Reference< css::beans::XPropertySet >& xTCProps ) throw (css::uno::RuntimeException);
    static void setLanguageID( const css::uno::Reference< css::beans::XPropertySet >& xTCProps, sal_Int32 _languageid ) throw (css::uno::RuntimeException);

    // Attributes
    virtual rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const rtl::OUString& Name ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getLanguageID( ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLanguageID( ::sal_Int32 _languageid ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ooo::vba::word::XFont > SAL_CALL getFont() throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif //SW_VBA_AXIS_HXX
