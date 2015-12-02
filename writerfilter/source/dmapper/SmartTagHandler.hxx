/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_SMARTTAGHANDLER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_SMARTTAGHANDLER_HXX

#include <vector>

#include <com/sun/star/rdf/XDocumentMetadataAccess.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "LoggedResources.hxx"

namespace writerfilter
{
namespace dmapper
{

/// Handler for smart tags, i.e. <w:smartTag> and below.
class SmartTagHandler
    : public LoggedProperties
{
    css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;
    css::uno::Reference<css::rdf::XDocumentMetadataAccess> m_xDocumentMetadataAccess;
    OUString m_aURI;
    OUString m_aElement;
    std::vector< std::pair<OUString, OUString> > m_aAttributes;

public:
    SmartTagHandler(const css::uno::Reference<css::uno::XComponentContext>& xComponentContext, const css::uno::Reference<css::text::XTextDocument>& xTextDocument);
    virtual ~SmartTagHandler();

    virtual void lcl_attribute(Id Name, Value& val) override;
    virtual void lcl_sprm(Sprm& sprm) override;

    void setURI(const OUString& rURI);
    void setElement(const OUString& rElement);

    /// Set m_aAttributes as RDF statements on xParagraph.
    void handle(const css::uno::Reference<css::text::XTextRange>& xParagraph);
};

} // namespace dmapper
} // namespace writerfilter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
