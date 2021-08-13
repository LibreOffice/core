/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

// Our mathml
#include "element.hxx"

// XML tools
#include <vcl/errcode.hxx>
#include <xmloff/xmlimp.hxx>

// Extras
#include <com/sun/star/beans/PropertyAttribute.hpp>

class SfxMedium;
class SmDocShell;
class SmMLImport;

class SmMLImportWrapper
{
    css::uno::Reference<css::frame::XModel> m_xModel;
    SmDocShell* m_pDocShell;
    SmMLImport* m_pMlImport;

private:
    // Use customized entities

public:
    /** Get the element tree when parsed from text
    */
    SmMlElement* getElementTree();

public:
    /** Constructor
     */
    explicit SmMLImportWrapper(css::uno::Reference<css::frame::XModel> const& rRef)
        : m_xModel(rRef)
        , m_pDocShell(nullptr)
        , m_pMlImport(nullptr)
    {
    }

    /** Imports the mathml
    */
    ErrCode Import(SfxMedium& rMedium);

    /** Imports the mathml
    */
    ErrCode Import(const OUString& aSource);

    /** read a component from input stream
     */
    ErrCode ReadThroughComponent(const css::uno::Reference<css::io::XInputStream>& xInputStream,
                                 const css::uno::Reference<css::lang::XComponent>& xModelComponent,
                                 css::uno::Reference<css::uno::XComponentContext> const& rxContext,
                                 css::uno::Reference<css::beans::XPropertySet> const& rPropSet,
                                 const char16_t* pFilterName, bool bEncrypted);

    /** read a component from storage
     */
    ErrCode ReadThroughComponent(const css::uno::Reference<css::embed::XStorage>& xStorage,
                                 const css::uno::Reference<css::lang::XComponent>& xModelComponent,
                                 const char16_t* pStreamName,
                                 css::uno::Reference<css::uno::XComponentContext> const& rxContext,
                                 css::uno::Reference<css::beans::XPropertySet> const& rPropSet,
                                 const char16_t* pFilterName);

    /** read a component from text
     */
    ErrCode ReadThroughComponent(std::u16string_view aText,
                                 const css::uno::Reference<css::lang::XComponent>& xModelComponent,
                                 css::uno::Reference<css::uno::XComponentContext> const& rxContext,
                                 css::uno::Reference<css::beans::XPropertySet> const& rPropSet);
};

class SmMLImport : public SvXMLImport
{
private:
    SmMlElement* m_pElementTree = new SmMlElement(SmMlElementType::NMlEmpty);
    bool m_bSuccess;
    size_t m_nSmSyntaxVersion;

public:
    /** Get's parsed element tree
    */
    SmMlElement* getElementTree() { return m_pElementTree; }

    /** Checks out if parse was a success
     */
    bool getSuccess() { return m_bSuccess; }

public:
    /** Handles an error on the mathml structure
     */
    void declareMlError();

public:
    /** Constructor
    */
    SmMLImport(const css::uno::Reference<css::uno::XComponentContext>& rContext,
               OUString const& implementationName, SvXMLImportFlags nImportFlags);

    /** Destructor
    */
    virtual ~SmMLImport() noexcept override { cleanup(); };

public:
    // XUnoTunnel
    sal_Int64 SAL_CALL getSomething(const css::uno::Sequence<sal_Int8>& rId) override;
    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId() noexcept;

    /** End the document
    */
    void SAL_CALL endDocument() override;

    /** Create a fast context
    */
    SvXMLImportContext* CreateFastContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;

    /** Imports view settings formula
    */
    virtual void
    SetViewSettings(const css::uno::Sequence<css::beans::PropertyValue>& aViewProps) override;

    /** Imports configurations settings formula
    */
    virtual void SetConfigurationSettings(
        const css::uno::Sequence<css::beans::PropertyValue>& aViewProps) override;

    /** Set syntax version
    */
    void SetSmSyntaxVersion(sal_uInt16 nSmSyntaxVersion) { m_nSmSyntaxVersion = nSmSyntaxVersion; }

    /** Get syntax version
    */
    sal_uInt16 GetSmSyntaxVersion() const { return m_nSmSyntaxVersion; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
