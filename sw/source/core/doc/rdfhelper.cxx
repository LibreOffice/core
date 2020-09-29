/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rdfhelper.hxx>

#include <com/sun/star/frame/XModel.hpp>
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

css::uno::Sequence<css::uno::Reference<css::rdf::XURI>> SwRDFHelper::getGraphNames(
    const css::uno::Reference<rdf::XDocumentMetadataAccess>& xDocumentMetadataAccess,
    const css::uno::Reference<rdf::XURI>& xType)
{
    try
    {
        return xDocumentMetadataAccess->getMetadataGraphsWithType(xType);
    }
    catch (const uno::RuntimeException&)
    {
        return uno::Sequence<uno::Reference<rdf::XURI>>();
    }
}

css::uno::Sequence<uno::Reference<css::rdf::XURI>>
SwRDFHelper::getGraphNames(const css::uno::Reference<css::frame::XModel>& xModel,
                           const OUString& rType)
{
    try
    {
        uno::Reference<uno::XComponentContext> xComponentContext(
            comphelper::getProcessComponentContext());
        // rdf::URI::create may fail with type: com.sun.star.uno.DeploymentException
        // message: component context fails to supply service com.sun.star.rdf.URI of type com.sun.star.rdf.XURI
        // context: cppu::ComponentContext
        uno::Reference<rdf::XURI> xType = rdf::URI::create(xComponentContext, rType);
        uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(xModel,
                                                                             uno::UNO_QUERY);
        return getGraphNames(xDocumentMetadataAccess, xType);
    }
    catch (const ::css::uno::Exception&)
    {
        return uno::Sequence<uno::Reference<rdf::XURI>>();
    }
}

std::map<OUString, OUString>
SwRDFHelper::getStatements(const css::uno::Reference<css::frame::XModel>& xModel,
                           const uno::Sequence<uno::Reference<css::rdf::XURI>>& rGraphNames,
                           const css::uno::Reference<css::rdf::XResource>& xSubject)
{
    std::map<OUString, OUString> aRet;
    if (!rGraphNames.hasElements())
        return aRet;

    uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(xModel, uno::UNO_QUERY);
    const uno::Reference<rdf::XRepository>& xRepo = xDocumentMetadataAccess->getRDFRepository();
    for (const uno::Reference<rdf::XURI>& xGraphName : rGraphNames)
    {
        uno::Reference<rdf::XNamedGraph> xGraph = xRepo->getGraph(xGraphName);
        if (!xGraph.is())
            continue;

        uno::Reference<container::XEnumeration> xStatements = xGraph->getStatements(
            xSubject, uno::Reference<rdf::XURI>(), uno::Reference<rdf::XURI>());
        while (xStatements->hasMoreElements())
        {
            const rdf::Statement aStatement = xStatements->nextElement().get<rdf::Statement>();
            aRet[aStatement.Predicate->getStringValue()] = aStatement.Object->getStringValue();
        }
    }

    return aRet;
}

std::map<OUString, OUString>
SwRDFHelper::getStatements(const css::uno::Reference<css::frame::XModel>& xModel,
                           const OUString& rType,
                           const css::uno::Reference<css::rdf::XResource>& xSubject)
{
    return getStatements(xModel, getGraphNames(xModel, rType), xSubject);
}

void SwRDFHelper::addStatement(const css::uno::Reference<css::frame::XModel>& xModel,
                               const OUString& rType, const OUString& rPath,
                               const css::uno::Reference<css::rdf::XResource>& xSubject,
                               const OUString& rKey, const OUString& rValue)
{
    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    uno::Reference<rdf::XURI> xType = rdf::URI::create(xComponentContext, rType);
    uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(xModel, uno::UNO_QUERY);
    const uno::Sequence< uno::Reference<rdf::XURI> > aGraphNames = getGraphNames(xDocumentMetadataAccess, xType);
    uno::Reference<rdf::XURI> xGraphName;
    if (aGraphNames.hasElements())
        xGraphName = aGraphNames[0];
    else
    {
        uno::Sequence< uno::Reference<rdf::XURI> > xTypes = { xType };
        xGraphName = xDocumentMetadataAccess->addMetadataFile(rPath, xTypes);
    }
    uno::Reference<rdf::XNamedGraph> xGraph = xDocumentMetadataAccess->getRDFRepository()->getGraph(xGraphName);
    uno::Reference<rdf::XURI> xKey = rdf::URI::create(xComponentContext, rKey);
    uno::Reference<rdf::XLiteral> xValue = rdf::Literal::create(xComponentContext, rValue);
    xGraph->addStatement(xSubject, xKey, xValue);
}

bool SwRDFHelper::hasMetadataGraph(const css::uno::Reference<css::frame::XModel>& xModel, const OUString& rType)
{
    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    uno::Reference<rdf::XURI> xType = rdf::URI::create(xComponentContext, rType);
    uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(xModel, uno::UNO_QUERY);
    return getGraphNames(xDocumentMetadataAccess, xType).hasElements();
}

void SwRDFHelper::removeStatement(const css::uno::Reference<css::frame::XModel>& xModel,
                                  const OUString& rType,
                                  const css::uno::Reference<css::rdf::XResource>& xSubject,
                                  const OUString& rKey, const OUString& rValue)
{
    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    uno::Reference<rdf::XURI> xType = rdf::URI::create(xComponentContext, rType);
    uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(xModel, uno::UNO_QUERY);
    const uno::Sequence< uno::Reference<rdf::XURI> > aGraphNames = getGraphNames(xDocumentMetadataAccess, xType);
    if (!aGraphNames.hasElements())
        return;

    uno::Reference<rdf::XNamedGraph> xGraph = xDocumentMetadataAccess->getRDFRepository()->getGraph(aGraphNames[0]);
    uno::Reference<rdf::XURI> xKey = rdf::URI::create(xComponentContext, rKey);
    uno::Reference<rdf::XLiteral> xValue = rdf::Literal::create(xComponentContext, rValue);
    xGraph->removeStatements(xSubject, xKey, xValue);
}

void SwRDFHelper::clearStatements(const css::uno::Reference<css::frame::XModel>& xModel,
                                  const OUString& rType,
                                  const css::uno::Reference<css::rdf::XResource>& xSubject)
{
    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    uno::Reference<rdf::XURI> xType = rdf::URI::create(xComponentContext, rType);
    uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(xModel, uno::UNO_QUERY);
    const uno::Sequence< uno::Reference<rdf::XURI> > aGraphNames = getGraphNames(xDocumentMetadataAccess, xType);
    if (!aGraphNames.hasElements())
        return;

    for (const uno::Reference<rdf::XURI>& xGraphName : aGraphNames)
    {
        uno::Reference<rdf::XNamedGraph> xGraph = xDocumentMetadataAccess->getRDFRepository()->getGraph(xGraphName);
        uno::Reference<container::XEnumeration> xStatements = xGraph->getStatements(xSubject, uno::Reference<rdf::XURI>(), uno::Reference<rdf::XURI>());
        while (xStatements->hasMoreElements())
        {
            rdf::Statement aStatement = xStatements->nextElement().get<rdf::Statement>();
            uno::Reference<rdf::XURI> xKey = rdf::URI::create(xComponentContext, aStatement.Predicate->getStringValue());
            uno::Reference<rdf::XLiteral> xValue = rdf::Literal::create(xComponentContext, aStatement.Object->getStringValue());
            xGraph->removeStatements(xSubject, xKey, xValue);
        }
    }
}

void SwRDFHelper::cloneStatements(const css::uno::Reference<css::frame::XModel>& xSrcModel,
                                  const css::uno::Reference<css::frame::XModel>& xDstModel,
                                  const OUString& rType,
                                  const css::uno::Reference<css::rdf::XResource>& xSrcSubject,
                                  const css::uno::Reference<css::rdf::XResource>& xDstSubject)
{
    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    uno::Reference<rdf::XURI> xType = rdf::URI::create(xComponentContext, rType);
    uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(xSrcModel, uno::UNO_QUERY);
    const uno::Sequence< uno::Reference<rdf::XURI> > aGraphNames = getGraphNames(xDocumentMetadataAccess, xType);
    if (!aGraphNames.hasElements())
        return;

    for (const uno::Reference<rdf::XURI>& xGraphName : aGraphNames)
    {
        uno::Reference<rdf::XNamedGraph> xGraph = xDocumentMetadataAccess->getRDFRepository()->getGraph(xGraphName);
        uno::Reference<container::XEnumeration> xStatements = xGraph->getStatements(xSrcSubject, uno::Reference<rdf::XURI>(), uno::Reference<rdf::XURI>());
        while (xStatements->hasMoreElements())
        {
            const rdf::Statement aStatement = xStatements->nextElement().get<rdf::Statement>();

            const OUString sKey = aStatement.Predicate->getStringValue();
            const OUString sValue = aStatement.Object->getStringValue();
            addStatement(xDstModel, rType, xGraphName->getLocalName(), xDstSubject, sKey, sValue);
        }
    }
}

std::map<OUString, OUString> SwRDFHelper::getTextNodeStatements(const OUString& rType, SwTextNode& rTextNode)
{
    uno::Reference<rdf::XResource> xTextNode(SwXParagraph::CreateXParagraph(rTextNode.GetDoc(), &rTextNode), uno::UNO_QUERY);
    return getStatements(rTextNode.GetDoc().GetDocShell()->GetBaseModel(), rType, xTextNode);
}

void SwRDFHelper::addTextNodeStatement(const OUString& rType, const OUString& rPath, SwTextNode& rTextNode, const OUString& rKey, const OUString& rValue)
{
    uno::Reference<rdf::XResource> xSubject(SwXParagraph::CreateXParagraph(rTextNode.GetDoc(), &rTextNode), uno::UNO_QUERY);
    addStatement(rTextNode.GetDoc().GetDocShell()->GetBaseModel(), rType, rPath, xSubject, rKey, rValue);
}

void SwRDFHelper::removeTextNodeStatement(const OUString& rType, SwTextNode& rTextNode, const OUString& rKey, const OUString& rValue)
{
    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    uno::Reference<rdf::XURI> xType = rdf::URI::create(xComponentContext, rType);
    uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(rTextNode.GetDoc().GetDocShell()->GetBaseModel(), uno::UNO_QUERY);
    const uno::Sequence< uno::Reference<rdf::XURI> > aGraphNames = getGraphNames(xDocumentMetadataAccess, xType);
    if (!aGraphNames.hasElements())
        return;

    uno::Reference<rdf::XURI> xGraphName = aGraphNames[0];
    uno::Reference<rdf::XNamedGraph> xGraph = xDocumentMetadataAccess->getRDFRepository()->getGraph(xGraphName);
    uno::Reference<rdf::XResource> xSubject(SwXParagraph::CreateXParagraph(rTextNode.GetDoc(), &rTextNode), uno::UNO_QUERY);
    uno::Reference<rdf::XURI> xKey = rdf::URI::create(xComponentContext, rKey);
    uno::Reference<rdf::XLiteral> xValue = rdf::Literal::create(xComponentContext, rValue);
    xGraph->removeStatements(xSubject, xKey, xValue);
}

void SwRDFHelper::updateTextNodeStatement(const OUString& rType, const OUString& rPath, SwTextNode& rTextNode, const OUString& rKey, const OUString& rOldValue, const OUString& rNewValue)
{
    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    uno::Reference<rdf::XURI> xType = rdf::URI::create(xComponentContext, rType);
    uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(rTextNode.GetDoc().GetDocShell()->GetBaseModel(), uno::UNO_QUERY);
    const uno::Sequence< uno::Reference<rdf::XURI> > aGraphNames = getGraphNames(xDocumentMetadataAccess, xType);
    uno::Reference<rdf::XURI> xGraphName;
    if (aGraphNames.hasElements())
    {
        xGraphName = aGraphNames[0];
    }
    else
    {
        uno::Sequence< uno::Reference<rdf::XURI> > xTypes = { xType };
        xGraphName = xDocumentMetadataAccess->addMetadataFile(rPath, xTypes);
    }

    uno::Reference<rdf::XNamedGraph> xGraph = xDocumentMetadataAccess->getRDFRepository()->getGraph(xGraphName);
    uno::Reference<rdf::XResource> xSubject(SwXParagraph::CreateXParagraph(rTextNode.GetDoc(), &rTextNode), uno::UNO_QUERY);
    uno::Reference<rdf::XURI> xKey = rdf::URI::create(xComponentContext, rKey);

    if (aGraphNames.hasElements())
    {
        // Remove the old value.
        uno::Reference<rdf::XLiteral> xOldValue = rdf::Literal::create(xComponentContext, rOldValue);
        xGraph->removeStatements(xSubject, xKey, xOldValue);
    }

    // Now add it with new value.
    uno::Reference<rdf::XLiteral> xNewValue = rdf::Literal::create(xComponentContext, rNewValue);
    xGraph->addStatement(xSubject, xKey, xNewValue);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
