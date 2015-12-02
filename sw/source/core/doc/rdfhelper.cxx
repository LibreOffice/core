/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rdfhelper.hxx>

#include <com/sun/star/rdf/Statement.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/XDocumentMetadataAccess.hpp>

#include <comphelper/processfactory.hxx>

#include <doc.hxx>
#include <docsh.hxx>
#include <ndtxt.hxx>
#include <unoparagraph.hxx>

using namespace com::sun::star;

std::map<OUString, OUString> SwRDFHelper::getTextNodeStatements(const OUString& rType, SwTextNode& rTextNode)
{
    std::map<OUString, OUString> aRet;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
