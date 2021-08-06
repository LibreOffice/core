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

#include "element.hxx"
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmltoken.hxx>

class SfxMedium;
namespace com::sun::star
{
namespace io
{
class XOutputStream;
}
namespace beans
{
class XPropertySet;
}
}

class SmMlExportWrapper
{
private:
    // Model
    css::uno::Reference<css::frame::XModel> m_xModel;
    // Save as a flat document ( mml, fodf ... )
    bool m_bFlat;
    // Use html / mathml entities
    bool m_bUseHTMLMLEntities;
    // Mathmml tree to parse
    SmMlElement* m_pElementTree;

public:
    void setFlat(bool bFlat) { m_bFlat = bFlat; }

    bool getFlat() const { return m_bFlat; }

    void setUseHTMLMLEntities(bool bUseHTMLMLEntities)
    {
        m_bUseHTMLMLEntities = bUseHTMLMLEntities;
    }

    bool getUseHTMLMLEntities() const { return m_bUseHTMLMLEntities; }

public:
    explicit SmMlExportWrapper(css::uno::Reference<css::frame::XModel> const& rRef)
        : m_xModel(rRef)
        , m_bFlat(true)
        , m_bUseHTMLMLEntities(false)
        , m_pElementTree(nullptr)
    {
    }

    /** Export to an archive
      */
    bool Export(SfxMedium& rMedium);

    /** Just export a mathml tree
      */
    OUString Export(SmMlElement* pElementTree);

    // Making this protected we can keep it from get trash as input
public:
    /** export through an XML exporter component (output stream version)
        */
    bool WriteThroughComponent(const css::uno::Reference<css::io::XOutputStream>& xOutputStream,
                               const css::uno::Reference<css::lang::XComponent>& xComponent,
                               css::uno::Reference<css::uno::XComponentContext> const& rxContext,
                               css::uno::Reference<css::beans::XPropertySet> const& rPropSet,
                               const char* pComponentName);

    /** export through an XML exporter component (storage version)
      */
    bool WriteThroughComponent(const css::uno::Reference<css::embed::XStorage>& xStor,
                               const css::uno::Reference<css::lang::XComponent>& xComponent,
                               const char* pStreamName,
                               css::uno::Reference<css::uno::XComponentContext> const& rxContext,
                               css::uno::Reference<css::beans::XPropertySet> const& rPropSet,
                               const char* pComponentName);

    /** export through an XML exporter component (storage version)
      */
    OUString WriteThroughComponent(
        const css::uno::Reference<css::lang::XComponent>& xComponent, const char* pStreamName,
        css::uno::Reference<css::uno::XComponentContext> const& rxContext,
        css::uno::Reference<css::beans::XPropertySet> const& rPropSet, const char* pComponentName);
};

class SmMlExport final : public SvXMLExport
{
private:
    SmMlElement* m_pTree;
    bool m_bSuccess;
    bool m_bUseExportTag;

public:
    /** Everything was allright
     */
    bool getSuccess() const { return m_bSuccess; }

    /** Everything was allright
     */
    bool getUseExportFlag() const { return m_bUseExportTag; }

    /** Everything was allright
     */
    void setUseExportFlag(bool bUseExportTag) { m_bUseExportTag = bUseExportTag; }

private:
    /** Adds an element
      */
    SvXMLElementExport* createElementExport(xmloff::token::XMLTokenEnum nElement)
    {
        return new SvXMLElementExport(*this, XML_NAMESPACE_MATH, nElement, true, false);
    }

    /** Adds an attribute
      */
    void addAttribute(xmloff::token::XMLTokenEnum pAttribute,
                      xmloff::token::XMLTokenEnum pAttributeValue)
    {
        AddAttribute(XML_NAMESPACE_MATH, pAttribute, pAttributeValue);
    }

    /** Adds an attribute
      */
    void addAttribute(xmloff::token::XMLTokenEnum pAttribute, const OUString& pAttributeValue)
    {
        AddAttribute(XML_NAMESPACE_MATH, pAttribute, pAttributeValue);
    }

private:
    /** Exports an attribute of type "length"
     */
    void exportMlAttributteLength(xmloff::token::XMLTokenEnum pAttribute,
                                  const SmLengthValue& aLengthValue);

    /** Exports attributes of an element
      */
    void exportMlAttributtes(const SmMlElement* pMlElement);

    /** Exports an element and all it's attributes
      */
    void exportMlElement(const SmMlElement* pMlElement);

    /** Handles an error on the mathml structure
     */
    void declareMlError();

public:
    /** Constructor
     */
    SmMlExport(const css::uno::Reference<css::uno::XComponentContext>& rContext,
               OUString const& implementationName, SvXMLExportFlags nExportFlags);

public:
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
