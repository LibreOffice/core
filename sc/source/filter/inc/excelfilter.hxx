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

class ExcelFilter : public ::oox::core::XmlFilterBase
{
public:
    explicit            ExcelFilter(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext )
                            throw( ::com::sun::star::uno::RuntimeException );
    virtual             ~ExcelFilter();

    void                registerWorkbookGlobals( WorkbookGlobals& rBookGlob );
    WorkbookGlobals&    getWorkbookGlobals() const;
    void                unregisterWorkbookGlobals();

    virtual bool        importDocument() throw() SAL_OVERRIDE;
    virtual bool        exportDocument() throw() SAL_OVERRIDE;

    virtual const ::oox::drawingml::Theme* getCurrentTheme() const SAL_OVERRIDE;
    virtual ::oox::vml::Drawing* getVmlDrawing() SAL_OVERRIDE;
    virtual const ::oox::drawingml::table::TableStyleListPtr getTableStyles() SAL_OVERRIDE;
    virtual ::oox::drawingml::chart::ChartConverter* getChartConverter() SAL_OVERRIDE;
    virtual void useInternalChartDataTable( bool bInternal ) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rDescriptor ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

private:
    virtual GraphicHelper* implCreateGraphicHelper() const SAL_OVERRIDE;
    virtual ::oox::ole::VbaProject* implCreateVbaProject() const SAL_OVERRIDE;
    virtual OUString implGetImplementationName() const SAL_OVERRIDE;

    WorkbookGlobals*    mpBookGlob;
};

css::uno::Reference< css::uno::XInterface > SAL_CALL ExcelFilter_create(
    css::uno::Reference< css::uno::XComponentContext > const & context);

OUString SAL_CALL ExcelFilter_getImplementationName();

css::uno::Sequence< OUString > SAL_CALL ExcelFilter_getSupportedServiceNames();

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
