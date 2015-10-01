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

#ifndef INCLUDED_FORMS_SOURCE_XFORMS_XPATHLIB_EXTENSION_HXX
#define INCLUDED_FORMS_SOURCE_XFORMS_XPATHLIB_EXTENSION_HXX

#include <rtl/ustring.h>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/xml/xpath/XXPathExtension.hpp>
#include <com/sun/star/xforms/XModel.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>




class CLibxml2XFormsExtension : public cppu::WeakImplHelper<
    css::xml::xpath::XXPathExtension, css::lang::XInitialization>
{
private:
    css::uno::Reference <css::xforms::XModel>  m_aModel;
    css::uno::Reference <css::xml::dom::XNode> m_aContextNode;

public:
    CLibxml2XFormsExtension() {}

    css::uno::Reference< css::xforms::XModel > getModel() { return m_aModel;}
    css::uno::Reference< css::xml::dom::XNode > getContextNode() { return m_aContextNode;}

    virtual css::xml::xpath::Libxml2ExtensionHandle SAL_CALL getLibxml2ExtensionHandle() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any >& aSequence) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

};

#endif // INCLUDED_FORMS_SOURCE_XFORMS_XPATHLIB_EXTENSION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
