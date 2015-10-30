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

#ifndef INCLUDED_SVX_SOURCE_INC_XMLXTEXP_HXX
#define INCLUDED_SVX_SOURCE_INC_XMLXTEXP_HXX

#include "xmloff/xmlexp.hxx"

namespace com { namespace sun { namespace star {
    namespace frame { class XModel; }
    namespace uno { template<class X> class Reference; }
    namespace uno { class XInterface; }
    namespace container { class XNameContainer; }
    namespace document { class XGraphicObjectResolver; }
    namespace xml { namespace sax { class XDocumentHandler; } }
} } }

class SvxXMLXTableExportComponent : public SvXMLExport
{
public:
    SvxXMLXTableExportComponent(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const OUString& rFileName,
        const css::uno::Reference< css::xml::sax::XDocumentHandler > & xHandler,
        const css::uno::Reference< css::container::XNameContainer > & xTable,
        css::uno::Reference< css::document::XGraphicObjectResolver >& xGrfResolver);

    virtual ~SvxXMLXTableExportComponent();

    static bool save( const OUString& rURL,
                      const css::uno::Reference< css::container::XNameContainer >& xTable,
                      const css::uno::Reference< css::embed::XStorage > &xStorage,
                      OUString *pOptName )
        throw (css::uno::RuntimeException, std::exception);

    // methods without content:
    virtual void _ExportAutoStyles() override;
    virtual void _ExportMasterStyles() override;
    virtual void _ExportContent() override;

private:
    bool exportTable() throw();
    const css::uno::Reference< css::container::XNameContainer > & mxTable;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
