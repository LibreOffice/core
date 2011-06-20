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

#ifndef _DOCXEXPORTFILTER_HXX_
#define _DOCXEXPORTFILTER_HXX_

#include <oox/core/xmlfilterbase.hxx>
#include <oox/drawingml/chart/chartconverter.hxx>
#include <oox/vml/vmldrawing.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

/// The physical access to the DOCX document (for writing).
class DocxExportFilter : public oox::core::XmlFilterBase
{
public:
    DocxExportFilter( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& xContext );

    // FIXME these should not even exist for the export-only filter!
    // For now, let's just do empty implementations of those.
    virtual bool        importDocument() { return false; }
    virtual const ::oox::drawingml::Theme* getCurrentTheme() const { return NULL; }
    virtual sal_Int32   getSchemeClr( sal_Int32 ) const { return 0; }
    virtual ::oox::vml::Drawing* getVmlDrawing() { return NULL; }
    virtual ::oox::drawingml::chart::ChartConverter& getChartConverter() { static ::oox::drawingml::chart::ChartConverter aConverter; return aConverter; }
    virtual const ::oox::drawingml::table::TableStyleListPtr getTableStyles() { return ::oox::drawingml::table::TableStyleListPtr(); }

    // Actual export of the DOCX document
    virtual bool        exportDocument();

private:

    /// Implementatio of the filter abstract method.
    virtual ::rtl::OUString implGetImplementationName() const;

    virtual ::oox::ole::VbaProject* implCreateVbaProject() const
    {
        return NULL; // FIXME: implement me !
    }
};

#endif // _DOCXEXPORTFILTER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
