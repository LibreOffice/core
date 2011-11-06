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



#ifndef OOX_XLS_EXCELFILTER_HXX
#define OOX_XLS_EXCELFILTER_HXX

#include "oox/core/binaryfilterbase.hxx"
#include "oox/core/xmlfilterbase.hxx"

namespace oox {
namespace xls {

class WorkbookGlobals;

// ============================================================================

class ExcelFilterBase
{
public:
    void                registerWorkbookGlobals( WorkbookGlobals& rBookGlob );
    WorkbookGlobals&    getWorkbookGlobals() const;
    void                unregisterWorkbookGlobals();

protected:
    explicit            ExcelFilterBase();
    virtual             ~ExcelFilterBase();

private:
    WorkbookGlobals*    mpBookGlob;
};

// ============================================================================

class ExcelFilter : public ::oox::core::XmlFilterBase, public ExcelFilterBase
{
public:
    explicit            ExcelFilter(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext )
                            throw( ::com::sun::star::uno::RuntimeException );
    virtual             ~ExcelFilter();

    virtual bool        importDocument() throw();
    virtual bool        exportDocument() throw();

    virtual const ::oox::drawingml::Theme* getCurrentTheme() const;
    virtual ::oox::vml::Drawing* getVmlDrawing();
    virtual const ::oox::drawingml::table::TableStyleListPtr getTableStyles();
    virtual ::oox::drawingml::chart::ChartConverter& getChartConverter();

private:
    virtual GraphicHelper* implCreateGraphicHelper() const;
    virtual ::oox::ole::VbaProject* implCreateVbaProject() const;
    virtual ::rtl::OUString implGetImplementationName() const;
};

// ============================================================================

class ExcelBiffFilter : public ::oox::core::BinaryFilterBase, public ExcelFilterBase
{
public:
    explicit            ExcelBiffFilter(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext )
                            throw( ::com::sun::star::uno::RuntimeException );
    virtual             ~ExcelBiffFilter();

    virtual bool        importDocument() throw();
    virtual bool        exportDocument() throw();

private:
    virtual GraphicHelper* implCreateGraphicHelper() const;
    virtual ::oox::ole::VbaProject* implCreateVbaProject() const;
    virtual ::rtl::OUString implGetImplementationName() const;
};

// ============================================================================

class ExcelVbaProjectFilter : public ExcelBiffFilter
{
public:
    explicit            ExcelVbaProjectFilter(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext )
                            throw( ::com::sun::star::uno::RuntimeException );

    virtual bool        importDocument() throw();
    virtual bool        exportDocument() throw();

private:
    virtual ::rtl::OUString implGetImplementationName() const;
};
 // ============================================================================

} // namespace xls
} // namespace oox

#endif
