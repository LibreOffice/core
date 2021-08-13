/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "element.hxx"

#include <xmloff/xmlimp.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>

class SmMlImport : public SvXMLImport
{
private:
    SmMlElement* m_pElementTree;
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
    SmMlImport(const css::uno::Reference<css::uno::XComponentContext>& rContext,
               OUString const& implementationName, SvXMLImportFlags nImportFlags);

    /** Destructor
    */
    virtual ~SmMlImport() noexcept override { cleanup(); };

public:
    // XUnoTunnel
    sal_Int64 SAL_CALL getSomething(const css::uno::Sequence<sal_Int8>& rId) override;
    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId() noexcept;

    void SAL_CALL endDocument() override;

    SvXMLImportContext* CreateFastContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;

    virtual void
    SetViewSettings(const css::uno::Sequence<css::beans::PropertyValue>& aViewProps) override;
    virtual void SetConfigurationSettings(
        const css::uno::Sequence<css::beans::PropertyValue>& aViewProps) override;

    void SetSmSyntaxVersion(sal_uInt16 nSmSyntaxVersion) { m_nSmSyntaxVersion = nSmSyntaxVersion; }
    sal_uInt16 GetSmSyntaxVersion() const { return m_nSmSyntaxVersion; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
