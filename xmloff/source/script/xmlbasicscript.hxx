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

#pragma once

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/script/XLibraryContainer2.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ref.hxx>
#include <xmloff/xmlimp.hxx>

namespace xmloff
{
class BasicElementBase : public SvXMLImportContext
{
protected:
    static bool
    getBoolAttr(bool* pRet, sal_Int32 nToken,
                const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttributes);

public:
    BasicElementBase(SvXMLImport& rImport);
};

class BasicLibrariesElement : public BasicElementBase
{
private:
    css::uno::Reference<css::script::XLibraryContainer2> m_xLibContainer;

public:
    BasicLibrariesElement(SvXMLImport& rImport,
                          const css::uno::Reference<css::frame::XModel>& rxModel);

    virtual css::uno::Reference<XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 Element,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& Attribs) override;
};

class BasicEmbeddedLibraryElement : public BasicElementBase
{
private:
    css::uno::Reference<css::script::XLibraryContainer2> m_xLibContainer;
    css::uno::Reference<css::container::XNameContainer> m_xLib;
    OUString const m_aLibName;
    bool const m_bReadOnly;

public:
    BasicEmbeddedLibraryElement(
        SvXMLImport& rImport,
        const css::uno::Reference<css::script::XLibraryContainer2>& rxLibContainer,
        const OUString& rLibName, bool bReadOnly);

    virtual css::uno::Reference<XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 Element,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& Attribs) override;
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

class BasicModuleElement : public BasicElementBase
{
private:
    css::uno::Reference<css::container::XNameContainer> m_xLib;
    OUString const m_aName;

public:
    BasicModuleElement(SvXMLImport& rImport,
                       const css::uno::Reference<css::container::XNameContainer>& rxLib,
                       const OUString& rName);

    virtual css::uno::Reference<XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 Element,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& Attribs) override;
};

class BasicSourceCodeElement : public BasicElementBase
{
private:
    css::uno::Reference<css::container::XNameContainer> m_xLib;
    OUString const m_aName;
    OUStringBuffer m_aBuffer;

public:
    BasicSourceCodeElement(SvXMLImport& rImport,
                           const css::uno::Reference<css::container::XNameContainer>& rxLib,
                           const OUString& rName);

    virtual void SAL_CALL characters(const OUString& rChars) override;
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

} // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */