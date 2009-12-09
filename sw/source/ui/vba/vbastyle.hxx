/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:  $
 * $Revision:  $
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
