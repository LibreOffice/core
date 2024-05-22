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
#include <i18nlangtag/lang.h>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <ooo/vba/word/XFont.hpp>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XStyle > SwVbaStyle_BASE;

class SwVbaStyle : public SwVbaStyle_BASE
{
private:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::beans::XPropertySet > mxStyleProps;
    css::uno::Reference< css::style::XStyle > mxStyle;
public:
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    SwVbaStyle( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, css::uno::Reference< css::frame::XModel >  xModel, const css::uno::Reference< css::beans::XPropertySet >& _xPropertySet );

    /// @throws css::uno::RuntimeException
    static void setStyle( const css::uno::Reference< css::beans::XPropertySet >& xParaProps, const css::uno::Any& xStyle );
    /// @throws css::uno::RuntimeException
    static LanguageType getLanguageID( const css::uno::Reference< css::beans::XPropertySet >& xTCProps );
    /// @throws css::uno::RuntimeException
    static void setLanguageID( const css::uno::Reference< css::beans::XPropertySet >& xTCProps, LanguageType _languageid );

    // Attributes
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName( const OUString& Name ) override;
    virtual ::sal_Int32 SAL_CALL getLanguageID( ) override;
    virtual void SAL_CALL setLanguageID( ::sal_Int32 _languageid ) override;
    virtual ::sal_Int32 SAL_CALL getType() override;
    virtual css::uno::Reference< ooo::vba::word::XFont > SAL_CALL getFont() override;
    virtual OUString SAL_CALL getNameLocal() override;
    virtual void SAL_CALL setNameLocal( const OUString& _namelocal ) override;
    virtual css::uno::Reference< ::ooo::vba::word::XParagraphFormat > SAL_CALL getParagraphFormat() override;
    virtual sal_Bool SAL_CALL getAutomaticallyUpdate() override;
    virtual void SAL_CALL setAutomaticallyUpdate( sal_Bool _automaticallyupdate ) override;
    virtual css::uno::Any SAL_CALL getBaseStyle() override;
    virtual void SAL_CALL setBaseStyle( const css::uno::Any& _basestyle ) override;
    virtual css::uno::Any SAL_CALL getNextParagraphStyle() override;
    virtual void SAL_CALL setNextParagraphStyle( const css::uno::Any& _nextparagraphstyle ) override;
    virtual ::sal_Int32 SAL_CALL getListLevelNumber() override;

    //XDefaultProperty
    virtual OUString SAL_CALL getDefaultPropertyName(  ) override { return u"Name"_ustr; }

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif //SW_VBA_AXIS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
