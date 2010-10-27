/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVX_XMLXTEXP_HXX
#define _SVX_XMLXTEXP_HXX

#include "xmloff/xmlexp.hxx"

namespace rtl { class OUString; }
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
    // #110680#
    SvxXMLXTableExportComponent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        const rtl::OUString& rFileName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler > & xHandler,
        const com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > & xTable,
        com::sun::star::uno::Reference< com::sun::star::document::XGraphicObjectResolver >& xGrfResolver);

    ~SvxXMLXTableExportComponent();

    static sal_Bool save( const rtl::OUString& rURL, const com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xTable ) throw();

    sal_Bool exportTable() throw();

    // methods without content:
    virtual void _ExportAutoStyles();
    virtual void _ExportMasterStyles();
    virtual void _ExportContent();

private:
    const com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > & mxTable;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
