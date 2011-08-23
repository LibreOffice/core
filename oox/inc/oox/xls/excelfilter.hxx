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

#ifndef OOX_XLS_EXCELFILTER_HXX
#define OOX_XLS_EXCELFILTER_HXX

#include "oox/core/xmlfilterbase.hxx"
#include "oox/core/binaryfilterbase.hxx"

namespace oox {
namespace xls {

class WorkbookData;

// ============================================================================

class ExcelFilterBase
{
public:
    void                registerWorkbookData( WorkbookData& rData );
    WorkbookData&       getWorkbookData() const;
    void                unregisterWorkbookData();

protected:
    explicit            ExcelFilterBase();
    virtual             ~ExcelFilterBase();

private:
    WorkbookData*       mpData;
};

// ============================================================================

class ExcelFilter : public ::oox::core::XmlFilterBase, public ExcelFilterBase
{
public:
    explicit            ExcelFilter(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxGlobalFactory );
    virtual             ~ExcelFilter();

    virtual bool        importDocument() throw();
    virtual bool        exportDocument() throw();

    virtual const ::oox::drawingml::Theme* getCurrentTheme() const;
    virtual ::oox::vml::Drawing* getVmlDrawing();
    virtual const ::oox::drawingml::table::TableStyleListPtr getTableStyles();
    virtual ::oox::drawingml::chart::ChartConverter& getChartConverter();

    virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rDescriptor ) throw( ::com::sun::star::uno::RuntimeException );

private:
    virtual GraphicHelper* implCreateGraphicHelper() const;
    virtual ::rtl::OUString implGetImplementationName() const;
};

// ============================================================================

class ExcelBiffFilter : public ::oox::core::BinaryFilterBase, public ExcelFilterBase
{
public:
    explicit            ExcelBiffFilter(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxGlobalFactory );
    virtual             ~ExcelBiffFilter();

    virtual bool        importDocument() throw();
    virtual bool        exportDocument() throw();

private:
    virtual GraphicHelper* implCreateGraphicHelper() const;
    virtual ::rtl::OUString implGetImplementationName() const;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
