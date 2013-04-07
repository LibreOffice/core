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

#ifndef OOX_XLS_EXCELFILTER_HXX
#define OOX_XLS_EXCELFILTER_HXX

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
    virtual ::oox::drawingml::chart::ChartConverter* getChartConverter();

    virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rDescriptor ) throw( ::com::sun::star::uno::RuntimeException );

private:
    virtual GraphicHelper* implCreateGraphicHelper() const;
    virtual ::oox::ole::VbaProject* implCreateVbaProject() const;
    virtual OUString implGetImplementationName() const;
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
