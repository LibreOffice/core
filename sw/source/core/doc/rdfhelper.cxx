/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rdfhelper.hxx>

#include <com/sun/star/rdf/Literal.hpp>
#include <com/sun/star/rdf/Statement.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/XDocumentMetadataAccess.hpp>

#include <comphelper/processfactory.hxx>

#include <doc.hxx>
#include <docsh.hxx>
#include <ndtxt.hxx>
#include <unoparagraph.hxx>

using namespace com::sun::star;

std::map<OUString, OUString> SwRDFHelper::getTextNodeStatements(const OUString& rType, const SwTextNode& rNode)
{
    std::map<OUString, OUString> aRet;

    // We only read the node, but CreateXParagraph() needs a non-cost one.
    SwTextNode& rTextNode = const_cast<SwTextNode&>(rNode);

    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    uno::Reference<rdf::XURI> xType = rdf::URI::create(xComponentContext, rType);
    uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(rTextNode.GetDoc()->GetDocShell()->GetBaseModel(), uno::UNO_QUERY);
    uno::Sequence< uno::Reference<rdf::XURI> > aGraphNames = xDocumentMetadataAccess->getMetadataGraphsWithType(xType);
    if (!aGraphNames.hasElements())
        return aRet;

    uno::Reference<rdf::XResource> xTextNode(SwXParagraph::CreateXParagraph(*rTextNode.GetDoc(), &rTextNode), uno::UNO_QUERY);
    for (const uno::Reference<rdf::XURI>& xGraphName : aGraphNames)
    {
        uno::Reference<rdf::XNamedGraph> xGraph = xDocumentMetadataAccess->getRDFRepository()->getGraph(xGraphName);
        uno::Reference<container::XEnumeration> xStatements = xGraph->getStatements(xTextNode, uno::Reference<rdf::XURI>(), uno::Reference<rdf::XURI>());
        while (xStatements->hasMoreElements())
        {
            rdf::Statement aStatement = xStatements->nextElement().get<rdf::Statement>();
            aRet[aStatement.Predicate->getStringValue()] = aStatement.Object->getStringValue();
        }
    }

    return aRet;
}

void SwRDFHelper::addTextNodeStatement(const OUString& rType, const OUString& rPath, SwTextNode& rTextNode, const OUString& rKey, const OUString& rValue)
{
    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    uno::Reference<rdf::XURI> xType = rdf::URI::create(xComponentContext, rType);
    uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(rTextNode.GetDoc()->GetDocShell()->GetBaseModel(), uno::UNO_QUERY);
    uno::Sequence< uno::Reference<rdf::XURI> > aGraphNames = xDocumentMetadataAccess->getMetadataGraphsWithType(xType);
    uno::Reference<rdf::XURI> xGraphName;
    if (aGraphNames.hasElements())
        xGraphName = aGraphNames[0];
    else
    {
        uno::Sequence< uno::Reference<rdf::XURI> > xTypes = { xType };
        xGraphName = xDocumentMetadataAccess->addMetadataFile(rPath, xTypes);
    }
    uno::Reference<rdf::XNamedGraph> xGraph = xDocumentMetadataAccess->getRDFRepository()->getGraph(xGraphName);
    uno::Reference<rdf::XResource> xSubject(SwXParagraph::CreateXParagraph(*rTextNode.GetDoc(), &rTextNode), uno::UNO_QUERY);
    uno::Reference<rdf::XURI> xKey = rdf::URI::create(xComponentContext, rKey);
    uno::Reference<rdf::XLiteral> xValue = rdf::Literal::create(xComponentContext, rValue);
    xGraph->addStatement(xSubject, xKey, xValue);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
