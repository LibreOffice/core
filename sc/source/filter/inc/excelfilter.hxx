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

#pragma once

#include <oox/core/xmlfilterbase.hxx>

namespace oox::xls {

class WorkbookGlobals;

class ExcelFilter : public ::oox::core::XmlFilterBase
{
public:
    /// @throws css::uno::RuntimeException
    explicit            ExcelFilter(
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual             ~ExcelFilter() override;

    void                registerWorkbookGlobals( WorkbookGlobals& rBookGlob );
    WorkbookGlobals&    getWorkbookGlobals() const;
    void                unregisterWorkbookGlobals();

    virtual bool        importDocument() override;
    virtual bool        exportDocument() throw() override;

    virtual const ::oox::drawingml::Theme* getCurrentTheme() const override;
    virtual ::oox::vml::Drawing* getVmlDrawing() override;
    virtual ::oox::drawingml::table::TableStyleListPtr getTableStyles() override;
    virtual ::oox::drawingml::chart::ChartConverter* getChartConverter() override;
    virtual void useInternalChartDataTable( bool bInternal ) override;

    virtual sal_Bool SAL_CALL filter( const css::uno::Sequence< css::beans::PropertyValue >& rDescriptor ) override;

private:
    virtual GraphicHelper* implCreateGraphicHelper() const override;
    virtual ::oox::ole::VbaProject* implCreateVbaProject() const override;
    virtual OUString SAL_CALL getImplementationName() override;

    WorkbookGlobals*    mpBookGlob;
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
