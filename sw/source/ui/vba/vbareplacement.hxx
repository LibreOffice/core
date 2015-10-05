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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAREPLACEMENT_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAREPLACEMENT_HXX

#include <ooo/vba/word/XReplacement.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/util/XPropertyReplace.hpp>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XReplacement > SwVbaReplacement_BASE;

class SwVbaReplacement : public SwVbaReplacement_BASE
{
private:
    css::uno::Reference< css::util::XPropertyReplace> mxPropertyReplace;

public:
    SwVbaReplacement( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::util::XPropertyReplace >& xPropertyReplace ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaReplacement();

    // Attributes
    virtual OUString SAL_CALL getText() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setText( const OUString& _text ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //Methods
    virtual void SAL_CALL ClearFormatting() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XHelperInterface
    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAREPLACEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
