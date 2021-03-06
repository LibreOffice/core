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

#include <xmloff/xmlexp.hxx>


namespace com::sun::star {
    namespace container { class XNameAccess; }
    namespace frame { class XModel; }
    namespace lang { class XMultiServiceFactory; }
    namespace uno { template<class X> class Reference; }
    namespace uno { template<class X> class Sequence; }
    namespace uno { class XInterface; }
    namespace uno { class Exception; }
    namespace xml::sax { class XDocumentHandler; }
}


/**
 * Component for the export of events attached to autotext blocks.
 * Via the XInitialization interface it expects up to two strings, the
 * first giving the file name (URL) of the autotext group, and the second
 * identifying the autotext. If one of the strings is not given, it
 * will export the whole group / all groups.
 */
class XMLAutoTextEventExport : public SvXMLExport
{
    css::uno::Reference<css::container::XNameAccess> xEvents;

public:

    XMLAutoTextEventExport(
        const css::uno::Reference< css::uno::XComponentContext >& xContext, OUString const & implementationName, SvXMLExportFlags nFlags
        );

    virtual ~XMLAutoTextEventExport() override;

    // XInitialization
    virtual void SAL_CALL initialize(
        const css::uno::Sequence<css::uno::Any> & rArguments ) override;

private:

    /// export the events off all autotexts
    virtual ErrCode exportDoc(
        enum ::xmloff::token::XMLTokenEnum eClass = xmloff::token::XML_TOKEN_INVALID ) override;

    /// does the document have any events ?
    bool hasEvents() const;

    /// export the events element
    void exportEvents();


    /// add the namespaces used by events
    /// (to be called for the document element)
    void addNamespaces();


    // methods without content:
    virtual void ExportMeta_() override;
    virtual void ExportScripts_() override;
    virtual void ExportFontDecls_() override;
    virtual void ExportStyles_( bool bUsed ) override ;
    virtual void ExportAutoStyles_() override;
    virtual void ExportMasterStyles_() override;
    virtual void ExportContent_() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
