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

// Our mathml
#include "element.hxx"

#include <unomodel.hxx>

// Xml tools
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmltoken.hxx>

// Extras
#include <com/sun/star/io/XOutputStream.hpp>

class SfxMedium;
class SmDocShell;
class SmModel;

class SmMLExport final : public SvXMLExport
{
private:
    SmMlElement* m_pElementTree;
    bool m_bSuccess;
    bool m_bUseExportTag;

private:
    /** Adds an element
      */
    SvXMLElementExport* createElementExport(xmloff::token::XMLTokenEnum nElement)
    {
        // We can't afford to ignore white spaces. They are part of the code.
        return new SvXMLElementExport(*this, XML_NAMESPACE_MATH, nElement, false, false);
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

public:
    /** Exports an attribute of type "length"
     */
    void exportMlAttributeLength(xmloff::token::XMLTokenEnum pAttribute,
                                 const SmLengthValue& aLengthValue);

    /** Exports attributes of an element
      */
    void exportMlAttributes(const SmMlElement* pMlElement);

    /** Exports an element and all it's attributes
      */
    SvXMLElementExport* exportMlElement(const SmMlElement* pMlElement);

    /** Exports an element tree
      */
    void exportMlElementTree();

    /** Handles an error on the mathml structure
     */
    void declareMlError();

public:
    /** Constructor
     */
    SmMLExport(const css::uno::Reference<css::uno::XComponentContext>& rContext,
               OUString const& implementationName, SvXMLExportFlags nExportFlags);

private:
    /** Get's document shell
     */
    SmDocShell* getSmDocShell();

public:
    /** Exports auto styles
     * However math doesn't have any
     */
    void ExportAutoStyles_() override {}

    /** Exports master styles
     * However math doesn't have any
     */
    void ExportMasterStyles_() override {}

    /** Exports formula
     * Handler used from exportDoc
     */
    void ExportContent_() override { exportMlElementTree(); };

    /** Exports the document
     * If m_pElementTree isn't null then exports m_pElementTree
    */
    ErrCode exportDoc(enum ::xmloff::token::XMLTokenEnum eClass
                      = ::xmloff::token::XML_TOKEN_INVALID) override;

    /** Get's view settings and prepares them to export
     */
    virtual void GetViewSettings(css::uno::Sequence<css::beans::PropertyValue>& aProps) override;

    /** Get's configuration settings and prepares them to export
     */
    virtual void
    GetConfigurationSettings(css::uno::Sequence<css::beans::PropertyValue>& aProps) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
