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
class SmDocShell;
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
    // Export xmlns tag
    bool m_bUseExportTag;

public:
    /** Set's the writer to export to flat document
     */
    void setFlat(bool bFlat) { m_bFlat = bFlat; }

    /** Checks if the writer is set to export to flat document
     */
    bool getFlat() const { return m_bFlat; }

    /** Checks the use of HTML / MathML entities such as &infinity;
     */
    void setUseHTMLMLEntities(bool bUseHTMLMLEntities)
    {
        m_bUseHTMLMLEntities = bUseHTMLMLEntities;
    }

    /** Activates the use of HTML / MathML entities such as &infinity;
     */
    bool getUseHTMLMLEntities() const { return m_bUseHTMLMLEntities; }

    /** Get's if xmlns field is added
     */
    bool getUseExportFlag() const { return m_bUseExportTag; }

    /** Set's if xmlns field is added
     */
    void setUseExportFlag(bool bUseExportTag) { m_bUseExportTag = bUseExportTag; }

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

    // Making this protected we can keep it from getting trash as input
protected:
    /** export through an XML exporter component (output stream version)
        */
    bool WriteThroughComponentOS(const css::uno::Reference<css::io::XOutputStream>& xOutputStream,
                                 const css::uno::Reference<css::lang::XComponent>& xComponent,
                                 css::uno::Reference<css::uno::XComponentContext> const& rxContext,
                                 css::uno::Reference<css::beans::XPropertySet> const& rPropSet,
                                 const char16_t* pComponentName);

    /** export through an XML exporter component (storage version)
      */
    bool WriteThroughComponentS(const css::uno::Reference<css::embed::XStorage>& xStor,
                                const css::uno::Reference<css::lang::XComponent>& xComponent,
                                const char16_t* pStreamName,
                                css::uno::Reference<css::uno::XComponentContext> const& rxContext,
                                css::uno::Reference<css::beans::XPropertySet> const& rPropSet,
                                const char16_t* pComponentName);

    /** export through an XML exporter component (memory stream version)
      */
    OUString
    WriteThroughComponentMS(const css::uno::Reference<css::lang::XComponent>& xComponent,
                            css::uno::Reference<css::uno::XComponentContext> const& rxContext,
                            css::uno::Reference<css::beans::XPropertySet> const& rPropSet);
};

class SmMlExport final : public SvXMLExport
{
private:
    SmMlElement* m_pElementTree;
    bool m_bSuccess;
    bool m_bUseExportTag;

public:
    /** Everything was allright
     */
    bool getSuccess() const { return m_bSuccess; }

    /** Get's if xmlns field is added
     */
    bool getUseExportFlag() const { return m_bUseExportTag; }

    /** Set's if xmlns field is added
     */
    void setUseExportFlag(bool bUseExportTag) { m_bUseExportTag = bUseExportTag; }

    /** Set's the element tree to be exported.
      * If it isn't nullptr the this will be exported instead of the document
     */
    void setElementTree(SmMlElement* pElementTree) { m_pElementTree = pElementTree; }

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

    /** Exports an element tree
      */
    void exportMlElementTree();

    /** Handles an error on the mathml structure
     */
    void declareMlError();

public:
    /** Constructor
     */
    SmMlExport(const css::uno::Reference<css::uno::XComponentContext>& rContext,
               OUString const& implementationName, SvXMLExportFlags nExportFlags);

private:
    /** Get's document shell
     */
    SmDocShell* getSmDocShell();

public:
    // XUnoTunnel
    sal_Int64 SAL_CALL getSomething(const css::uno::Sequence<sal_Int8>& rId) override;
    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId() noexcept;

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

    virtual void GetViewSettings(css::uno::Sequence<css::beans::PropertyValue>& aProps) override;
    virtual void
    GetConfigurationSettings(css::uno::Sequence<css::beans::PropertyValue>& aProps) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
