/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <ooo/vba/word/XListTemplate.hpp>


typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XStyle > SwVbaStyle_BASE;

class SwVbaStyle : public SwVbaStyle_BASE
{
private:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::beans::XPropertySet > mxStyleProps;
    css::uno::Reference< css::style::XStyle > mxStyle;
public:
    SwVbaStyle( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::beans::XPropertySet >& _xPropertySet ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
    virtual ~SwVbaStyle(){}

    static void setStyle( const css::uno::Reference< css::beans::XPropertySet >& xParaProps, const css::uno::Any& xStyle ) throw (css::uno::RuntimeException);
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
    virtual void SAL_CALL LinkToListTemplate( const css::uno::Reference< ooo::vba::word::XListTemplate >& ListTemplate, const css::uno::Any& ListLevelNumber ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getNameLocal() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setNameLocal( const ::rtl::OUString& _namelocal ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ::ooo::vba::word::XParagraphFormat > SAL_CALL getParagraphFormat() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getAutomaticallyUpdate() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAutomaticallyUpdate( ::sal_Bool _automaticallyupdate ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getBaseStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setBaseStyle( const css::uno::Any& _basestyle ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getNextParagraphStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setNextParagraphStyle( const css::uno::Any& _nextparagraphstyle ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getListLevelNumber() throw (css::uno::RuntimeException);

    //XDefaultProperty
    virtual ::rtl::OUString SAL_CALL getDefaultPropertyName(  ) throw (css::uno::RuntimeException) { return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name")); }

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif //SW_VBA_AXIS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
