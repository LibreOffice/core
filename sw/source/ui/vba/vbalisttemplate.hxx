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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBALISTTEMPLATE_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBALISTTEMPLATE_HXX

#include <ooo/vba/word/XListTemplate.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include "vbalisthelper.hxx"

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XListTemplate > SwVbaListTemplate_BASE;

class SwVbaListTemplate : public SwVbaListTemplate_BASE
{
private:
    SwVbaListHelperRef pListHelper;

public:
    SwVbaListTemplate( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextDocument >& xTextDoc, sal_Int32 nGalleryType, sal_Int32 nTemplateType ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaListTemplate();

    void applyListTemplate( css::uno::Reference< css::beans::XPropertySet >& xProps ) throw ( css::uno::RuntimeException );

    // Methods
    virtual css::uno::Any SAL_CALL ListLevels( const css::uno::Any& index ) throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XHelperInterface
    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBALISTTEMPLATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
