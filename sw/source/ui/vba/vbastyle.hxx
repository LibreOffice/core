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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBASTYLE_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBASTYLE_HXX
#include <ooo/vba/word/XStyle.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <ooo/vba/word/XFont.hpp>
#include <ooo/vba/word/XListTemplate.hpp>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XStyle > SwVbaStyle_BASE;

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
    static sal_Int32 getLanguageID( const css::uno::Reference< css::beans::XPropertySet >& xTCProps ) throw (css::uno::RuntimeException, std::exception);
    static void setLanguageID( const css::uno::Reference< css::beans::XPropertySet >& xTCProps, sal_Int32 _languageid ) throw (css::uno::RuntimeException);

    // Attributes
    virtual OUString SAL_CALL getName() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setName( const OUString& Name ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getLanguageID( ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setLanguageID( ::sal_Int32 _languageid ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getType() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< ooo::vba::word::XFont > SAL_CALL getFont() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getNameLocal() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setNameLocal( const OUString& _namelocal ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< ::ooo::vba::word::XParagraphFormat > SAL_CALL getParagraphFormat() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getAutomaticallyUpdate() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setAutomaticallyUpdate( sal_Bool _automaticallyupdate ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Any SAL_CALL getBaseStyle() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setBaseStyle( const css::uno::Any& _basestyle ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Any SAL_CALL getNextParagraphStyle() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setNextParagraphStyle( const css::uno::Any& _nextparagraphstyle ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getListLevelNumber() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XDefaultProperty
    virtual OUString SAL_CALL getDefaultPropertyName(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE { return OUString("Name"); }

    // XHelperInterface
    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;
};

#endif //SW_VBA_AXIS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
