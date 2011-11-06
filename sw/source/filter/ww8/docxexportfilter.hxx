/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _DOCXEXPORTFILTER_HXX_
#define _DOCXEXPORTFILTER_HXX_

#include <oox/core/xmlfilterbase.hxx>
#include <oox/drawingml/chart/chartconverter.hxx>
#include <oox/vml/drawing.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

/// The physical access to the DOCX document (for writing).
class DocxExportFilter : public oox::core::XmlFilterBase
{
public:
    DocxExportFilter( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rMSF );

    // FIXME these should not even exist for the export-only filter!
    // For now, let's just do empty implementations of those.
    virtual bool        importDocument() { return false; }
    virtual const ::oox::drawingml::Theme* getCurrentTheme() const { return NULL; }
    virtual sal_Int32   getSchemeClr( sal_Int32 ) const { return 0; }
    virtual const ::oox::vml::DrawingPtr getDrawings() { return ::oox::vml::DrawingPtr(); }
    virtual ::oox::drawingml::chart::ChartConverter& getChartConverter() { static ::oox::drawingml::chart::ChartConverter aConverter; return aConverter; }
    virtual const ::oox::drawingml::table::TableStyleListPtr getTableStyles() { return ::oox::drawingml::table::TableStyleListPtr(); }

    // Actual export of the DOCX document
    virtual bool        exportDocument();

private:

    /// Implementatio of the filter abstract method.
    virtual ::rtl::OUString implGetImplementationName() const;
};

#endif // _DOCXEXPORTFILTER_HXX_
/* vi:set tabstop=4 shiftwidth=4 expandtab: */
