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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_DOCXEXPORTFILTER_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_DOCXEXPORTFILTER_HXX

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
    virtual bool        importDocument() SAL_OVERRIDE { return false; }
    virtual const ::oox::drawingml::Theme* getCurrentTheme() const SAL_OVERRIDE { return NULL; }
    sal_Int32   getSchemeClr( sal_Int32 ) const { return 0; }
    virtual ::oox::vml::Drawing* getVmlDrawing() SAL_OVERRIDE { return NULL; }
    virtual ::oox::drawingml::chart::ChartConverter* getChartConverter() SAL_OVERRIDE { return NULL; }
    virtual const ::oox::drawingml::table::TableStyleListPtr getTableStyles() SAL_OVERRIDE { return ::oox::drawingml::table::TableStyleListPtr(); }

    // Actual export of the DOCX document
    virtual bool        exportDocument() SAL_OVERRIDE;

private:

    /// Implementatio of the filter abstract method.
    virtual OUString implGetImplementationName() const SAL_OVERRIDE;

    virtual ::oox::ole::VbaProject* implCreateVbaProject() const SAL_OVERRIDE
    {
        return NULL; // FIXME: implement me !
    }
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_DOCXEXPORTFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
