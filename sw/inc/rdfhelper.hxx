/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_INC_RDFHELPER_HXX
#define INCLUDED_SW_INC_RDFHELPER_HXX

#include <map>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>

#include "swdllapi.h"

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

class SwTextNode;
class SwXTextDocument;
namespace com::sun::star {
    namespace frame {
        class XModel;
    }
    namespace rdf {
        class XDocumentMetadataAccess;
        class XResource;
        class XURI;
    }
}

/// Provides access to RDF metadata on core objects.
class SW_DLLPUBLIC SwRDFHelper
{
public:
    /// Gets all graph-names in RDF of a given type.
    static css::uno::Sequence<css::uno::Reference<css::rdf::XURI>>
    getGraphNames(const rtl::Reference<SwXTextDocument>& xModel,
                  const css::uno::Reference<css::rdf::XURI>& xType);

    /// Gets all graph-names in RDF of a given type.
    static css::uno::Sequence<css::uno::Reference<css::rdf::XURI>>
    getGraphNames(const rtl::Reference<SwXTextDocument>& xModel, const OUString& rType);

    /// Gets all (XResource, key, value) statements in RDF graphs given the graph-names.
    static std::map<OUString, OUString>
    getStatements(const rtl::Reference<SwXTextDocument>& xModel,
                  const css::uno::Sequence<css::uno::Reference<css::rdf::XURI>>& rGraphNames,
                  const css::uno::Reference<css::rdf::XResource>& xSubject);

    /// Gets all (XResource, key, value) statements in RDF graphs of type rType.
    static std::map<OUString, OUString>
    getStatements(const rtl::Reference<SwXTextDocument>& xModel, const OUString& rType,
                  const css::uno::Reference<css::rdf::XResource>& xSubject);

    /// Add an (XResource, key, value) statement in the graph of type rType -- or if it does not exist, create a graph at rPath first.
    static void addStatement(const rtl::Reference<SwXTextDocument>& xModel,
                             const OUString& rType, const OUString& rPath,
                             const css::uno::Reference<css::rdf::XResource>& xSubject,
                             const OUString& rKey, const OUString& rValue);

    /// Check if a graph of type rType exists.
    static bool hasMetadataGraph(const rtl::Reference<SwXTextDocument>& xModel,
                                 const OUString& rType);

    /// Remove an (XResource, key, value) statement in the graph of type rType, if it exists.
    static void removeStatement(const rtl::Reference<SwXTextDocument>& xModel,
                                const OUString& rType,
                                const css::uno::Reference<css::rdf::XResource>& xSubject,
                                const OUString& rKey, const OUString& rValue);

    /// Remove all statements in the graph of type rType, if any exists.
    static void clearStatements(const rtl::Reference<SwXTextDocument>& xModel,
                                const OUString& rType,
                                const css::uno::Reference<css::rdf::XResource>& xSubject);

    /// Gets all (rTextNode, key, value) statements in RDF graphs of type rType.
    static std::map<OUString, OUString> getTextNodeStatements(const OUString& rType, SwTextNode& rTextNode);

    /// Add an (rTextNode, key, value) statement in the graph of type rType -- or if it does not exist, create a graph at rPath first.
    static void addTextNodeStatement(const OUString& rType, const OUString& rPath, SwTextNode& rTextNode, const OUString& rKey, const OUString& rValue);

    /// Remove an (rTextNode, key, value) statement in the graph of type rType.
    static void removeTextNodeStatement(const OUString& rType, SwTextNode& rTextNode, const OUString& rKey, const OUString& rValue);

    /// Update an (rTextNode, key, value) statement in the graph of type rType from old value to new. Creates the graph at rPath if doesn't exist.
    static void updateTextNodeStatement(const OUString& rType, const OUString& rPath, SwTextNode& rTextNode, const OUString& rKey, const OUString& rOldValue, const OUString& rNewValue);

};

#endif // INCLUDED_SW_INC_RDFHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
