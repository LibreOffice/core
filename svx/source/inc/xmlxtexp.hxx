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
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext,
        const OUString& rFileName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler > & xHandler,
        const com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > & xTable,
        com::sun::star::uno::Reference< com::sun::star::document::XGraphicObjectResolver >& xGrfResolver);

    virtual ~SvxXMLXTableExportComponent();

    static bool save( const OUString& rURL,
                      const com::sun::star::uno::Reference<
                          ::com::sun::star::container::XNameContainer >& xTable,
                      const ::com::sun::star::uno::Reference <
                          ::com::sun::star::embed::XStorage > &xStorage,
                      OUString *pOptName )
        throw (css::uno::RuntimeException, std::exception);

    // methods without content:
    virtual void _ExportAutoStyles() SAL_OVERRIDE;
    virtual void _ExportMasterStyles() SAL_OVERRIDE;
    virtual void _ExportContent() SAL_OVERRIDE;

private:
    bool exportTable() throw();
    const com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > & mxTable;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
