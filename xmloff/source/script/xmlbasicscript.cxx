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

#include <sal/config.h>

#include "xmlbasicscript.hxx"
#include <sal/log.hxx>
#include <xmlscript/xmlns.h>
#include <xmloff/xmlnamespace.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

namespace xmloff
{
// BasicElementBase

BasicElementBase::BasicElementBase(SvXMLImport& rImport)
    : SvXMLImportContext(rImport)
{
}

bool BasicElementBase::getBoolAttr(bool* pRet, sal_Int32 nToken,
                                   const Reference<XFastAttributeList>& xAttributes)
{
    OUString aValue = xAttributes->getOptionalValue(nToken);
    if (!aValue.isEmpty())
    {
        if (aValue == "true")
        {
            *pRet = true;
            return true;
        }
        else if (aValue == "false")
        {
            *pRet = false;
            return true;
        }
        else
        {
            throw xml::sax::SAXException(SvXMLImport::getNameFromToken(nToken)
                                             + ": no boolean value (true|false)!",
                                         Reference<XInterface>(), Any());
        }
    }
    return false;
}

// BasicLibrariesElement

BasicLibrariesElement::BasicLibrariesElement(SvXMLImport& rImport,
                                             const css::uno::Reference<css::frame::XModel>& rxModel)
    : BasicElementBase(rImport)
{
    // try the XEmbeddedScripts interface
    Reference<document::XEmbeddedScripts> xDocumentScripts(rxModel, UNO_QUERY_THROW);
    m_xLibContainer = xDocumentScripts->getBasicLibraries();

    if (!m_xLibContainer.is())
    {
        // try the "BasicLibraries" property (old-style, for compatibility)
        Reference<beans::XPropertySet> xPSet(rxModel, UNO_QUERY);
        if (xPSet.is())
            xPSet->getPropertyValue("BasicLibraries") >>= m_xLibContainer;
    }

    SAL_WARN_IF(!m_xLibContainer.is(), "xmlscript.xmlflat",
                "BasicImport::startRootElement: nowhere to import to!");

    if (!m_xLibContainer.is())
    {
        throw xml::sax::SAXException("nowhere to import to", Reference<XInterface>(), Any());
    }
}

// XElement

Reference<XFastContextHandler>
BasicLibrariesElement::createFastChildContext(sal_Int32 nElement,
                                              const Reference<XFastAttributeList>& xAttributes)
{
    if (!IsTokenInNamespace(nElement, XML_NAMESPACE_OOO))
    {
        throw xml::sax::SAXException("illegal namespace!", Reference<XInterface>(), Any());
    }
    else if ((nElement & TOKEN_MASK) == XML_LIBRARY_LINKED)
    {
        OUString aName = xAttributes->getValue(NAMESPACE_TOKEN(XML_NAMESPACE_OOO) | XML_NAME);

        OUString aStorageURL = xAttributes->getValue(XML_ELEMENT(XLINK, XML_HREF));

        bool bReadOnly = false;
        getBoolAttr(&bReadOnly, NAMESPACE_TOKEN(XML_NAMESPACE_OOO) | XML_READONLY, xAttributes);

        if (m_xLibContainer.is())
        {
            try
            {
                Reference<container::XNameAccess> xLib(
                    m_xLibContainer->createLibraryLink(aName, aStorageURL, bReadOnly));
                if (xLib.is())
                    return new BasicElementBase(GetImport());
            }
            catch (const container::ElementExistException&)
            {
                TOOLS_INFO_EXCEPTION("xmlscript.xmlflat",
                                     "BasicLibrariesElement::startChildElement");
            }
            catch (const lang::IllegalArgumentException&)
            {
                TOOLS_INFO_EXCEPTION("xmlscript.xmlflat",
                                     "BasicLibrariesElement::startChildElement");
            }
        }
    }
    else if ((nElement & TOKEN_MASK) == XML_LIBRARY_EMBEDDED)
    {
        // TODO: create password protected libraries

        OUString aName = xAttributes->getValue(NAMESPACE_TOKEN(XML_NAMESPACE_OOO) | XML_NAME);

        bool bReadOnly = false;
        getBoolAttr(&bReadOnly, NAMESPACE_TOKEN(XML_NAMESPACE_OOO) | XML_READONLY, xAttributes);

        if (m_xLibContainer.is())
        {
            try
            {
                Reference<container::XNameContainer> xLib;
                if (m_xLibContainer->hasByName(aName))
                {
                    // Standard library
                    m_xLibContainer->getByName(aName) >>= xLib;
                }
                else
                {
                    xLib.set(m_xLibContainer->createLibrary(aName));
                }

                if (xLib.is())
                    return new BasicEmbeddedLibraryElement(GetImport(), m_xLibContainer, aName,
                                                           bReadOnly);
            }
            catch (const lang::IllegalArgumentException&)
            {
                TOOLS_INFO_EXCEPTION("xmlscript.xmlflat",
                                     "BasicLibrariesElement::startChildElement");
            }
        }
    }
    else
    {
        throw xml::sax::SAXException("expected library-linked or library-embedded element!",
                                     Reference<XInterface>(), Any());
    }

    return nullptr;
}

// BasicEmbeddedLibraryElement

BasicEmbeddedLibraryElement::BasicEmbeddedLibraryElement(
    SvXMLImport& rImport, const Reference<script::XLibraryContainer2>& rxLibContainer,
    const OUString& rLibName, bool bReadOnly)
    : BasicElementBase(rImport)
    , m_xLibContainer(rxLibContainer)
    , m_aLibName(rLibName)
    , m_bReadOnly(bReadOnly)
{
    try
    {
        if (m_xLibContainer.is() && m_xLibContainer->hasByName(m_aLibName))
            m_xLibContainer->getByName(m_aLibName) >>= m_xLib;
    }
    catch (const lang::WrappedTargetException&)
    {
        TOOLS_INFO_EXCEPTION("xmlscript.xmlflat", "BasicEmbeddedLibraryElement::CTOR:");
    }
}

Reference<XFastContextHandler> BasicEmbeddedLibraryElement::createFastChildContext(
    sal_Int32 nElement, const Reference<XFastAttributeList>& xAttributes)
{
    if (!IsTokenInNamespace(nElement, XML_NAMESPACE_OOO))
    {
        throw xml::sax::SAXException("illegal namespace!", Reference<XInterface>(), Any());
    }
    else if ((nElement & TOKEN_MASK) == XML_MODULE)
    {
        OUString aName = xAttributes->getValue(NAMESPACE_TOKEN(XML_NAMESPACE_OOO) | XML_NAME);

        if (m_xLib.is() && !aName.isEmpty())
            return new BasicModuleElement(GetImport(), m_xLib, aName);
    }
    else
    {
        throw xml::sax::SAXException("expected module element!", Reference<XInterface>(), Any());
    }

    return nullptr;
}

void BasicEmbeddedLibraryElement::endFastElement(sal_Int32)
{
    if (m_xLibContainer.is() && m_xLibContainer->hasByName(m_aLibName) && m_bReadOnly)
        m_xLibContainer->setLibraryReadOnly(m_aLibName, m_bReadOnly);
}

// BasicModuleElement

BasicModuleElement::BasicModuleElement(SvXMLImport& rImport,
                                       const Reference<container::XNameContainer>& rxLib,
                                       const OUString& rName)
    : BasicElementBase(rImport)
    , m_xLib(rxLib)
    , m_aName(rName)
{
}

Reference<XFastContextHandler>
BasicModuleElement::createFastChildContext(sal_Int32 nElement,
                                           const Reference<XFastAttributeList>& xAttributes)
{
    // TODO: <byte-code>

    if (!IsTokenInNamespace(nElement, XML_NAMESPACE_OOO))
    {
        throw xml::sax::SAXException("illegal namespace!", Reference<XInterface>(), Any());
    }
    else if ((nElement & TOKEN_MASK) == XML_SOURCE_CODE)
    {
        // TODO: password protected libraries

        if (xAttributes.is())
        {
            if (m_xLib.is() && !m_aName.isEmpty())
                return new BasicSourceCodeElement(GetImport(), m_xLib, m_aName);
        }
    }
    else
    {
        throw xml::sax::SAXException("expected source-code element!", Reference<XInterface>(),
                                     Any());
    }

    return nullptr;
}

// BasicSourceCodeElement

BasicSourceCodeElement::BasicSourceCodeElement(SvXMLImport& rImport,
                                               const Reference<container::XNameContainer>& rxLib,
                                               const OUString& rName)
    : BasicElementBase(rImport)
    , m_xLib(rxLib)
    , m_aName(rName)
{
}

// XElement

void BasicSourceCodeElement::characters(const OUString& rChars) { m_aBuffer.append(rChars); }

void BasicSourceCodeElement::endFastElement(sal_Int32)
{
    try
    {
        if (m_xLib.is() && !m_aName.isEmpty())
        {
            Any aElement;
            aElement <<= m_aBuffer.makeStringAndClear();
            m_xLib->insertByName(m_aName, aElement);
        }
    }
    catch (const container::ElementExistException&)
    {
        TOOLS_INFO_EXCEPTION("xmlscript.xmlflat", "BasicSourceCodeElement::endElement");
    }
    catch (const lang::IllegalArgumentException&)
    {
        TOOLS_INFO_EXCEPTION("xmlscript.xmlflat", "BasicSourceCodeElement::endElement");
    }
    catch (const lang::WrappedTargetException&)
    {
        TOOLS_INFO_EXCEPTION("xmlscript.xmlflat", "BasicSourceCodeElement::endElement");
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
