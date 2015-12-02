/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SmartTagHandler.hxx>

#include <com/sun/star/rdf/URI.hpp>

#include <ooxml/resourceids.hxx>

namespace
{
OUString lcl_getTypePath(const OUString& rType)
{
    OUString aRet;
    if (rType == "urn:tscp:names:baf:1.1")
        aRet = "tscp/baf.rdf";
    return aRet;
}
}

namespace writerfilter
{
namespace dmapper
{

using namespace ::com::sun::star;

SmartTagHandler::SmartTagHandler(const uno::Reference<uno::XComponentContext>& xComponentContext, const uno::Reference<text::XTextDocument>& xTextDocument)
    : LoggedProperties("SmartTagHandler"),
      m_xComponentContext(xComponentContext),
      m_xDocumentMetadataAccess(xTextDocument, uno::UNO_QUERY)
{
}

SmartTagHandler::~SmartTagHandler()
{
}

void SmartTagHandler::lcl_attribute(Id nName, Value& rValue)
{
    switch (nName)
    {
    case NS_ooxml::LN_CT_Attr_name:
        m_aAttributes.emplace_back(rValue.getString(), OUString());
        break;
    case NS_ooxml::LN_CT_Attr_val:
        if (!m_aAttributes.empty())
            m_aAttributes.back().second = rValue.getString();
        break;
    default:
        SAL_WARN("writerfilter", "SmartTagHandler::lcl_attribute: unhandled attribute " << nName << " (string value: '"<<rValue.getString()<<"')");
        break;
    }
}

void SmartTagHandler::lcl_sprm(Sprm& rSprm)
{
    switch (rSprm.getId())
    {
    case NS_ooxml::LN_CT_SmartTagPr_attr:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties.get())
            pProperties->resolve(*this);
        break;
    }
    }
}

void SmartTagHandler::setURI(const OUString& rURI)
{
    m_aURI = rURI;
}

void SmartTagHandler::setElement(const OUString& rElement)
{
    m_aElement = rElement;
}

void SmartTagHandler::handle(const uno::Reference<text::XTextRange>& xParagraph)
{
    if (!m_aURI.isEmpty() && !m_aElement.isEmpty() && !m_aAttributes.empty())
    {
        uno::Reference<rdf::XResource> xSubject(xParagraph, uno::UNO_QUERY);

        for (const std::pair<OUString, OUString>& rAttribute : m_aAttributes)
        {
            sal_Int32 nIndex = rAttribute.first.indexOf('#');
            if (nIndex == -1)
                continue;

            OUString aTypeNS = rAttribute.first.copy(0, nIndex);
            OUString aMetadataFilePath = lcl_getTypePath(aTypeNS);
            if (aMetadataFilePath.isEmpty())
                continue;

            uno::Reference<rdf::XURI> xType = rdf::URI::create(m_xComponentContext, aTypeNS);
            uno::Sequence< uno::Reference<rdf::XURI> > aGraphNames = m_xDocumentMetadataAccess->getMetadataGraphsWithType(xType);
            uno::Reference<rdf::XURI> xGraphName;
            if (aGraphNames.hasElements())
                xGraphName = aGraphNames[0];
            else
            {
                uno::Sequence< uno::Reference<rdf::XURI> > xTypes = { xType };
                xGraphName = m_xDocumentMetadataAccess->addMetadataFile(aMetadataFilePath, xTypes);
            }
            uno::Reference<rdf::XNamedGraph> xGraph = m_xDocumentMetadataAccess->getRDFRepository()->getGraph(xGraphName);
            uno::Reference<rdf::XURI> xKey = rdf::URI::create(m_xComponentContext, rAttribute.first);
            uno::Reference<rdf::XURI> xValue = rdf::URI::create(m_xComponentContext, rAttribute.second);
            xGraph->addStatement(xSubject, xKey, xValue);
        }

        m_aURI.clear();
        m_aElement.clear();
        m_aAttributes.clear();
    }
}

} // namespace dmapper
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
