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

#ifndef INCLUDED_OOX_PPT_DGMLAYOUT_HXX
#define INCLUDED_OOX_PPT_DGMLAYOUT_HXX

#include <exception>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <oox/core/filterbase.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/drawingml/theme.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace com { namespace sun { namespace star {
    namespace uno { class XComponentContext; }
} } }

namespace oox {
    namespace drawingml { namespace chart { class ChartConverter; } }
    namespace ole { class VbaProject; }
    namespace vml { class Drawing; }
}

namespace oox { namespace ppt {


class QuickDiagrammingLayout : public oox::core::XmlFilterBase
{
public:

    QuickDiagrammingLayout( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    // from FilterBase
    virtual bool importDocument() override;
    virtual bool exportDocument() throw() override;

    virtual const ::oox::drawingml::Theme* getCurrentTheme() const override;
    virtual const oox::drawingml::table::TableStyleListPtr getTableStyles() override;

    virtual ::oox::vml::Drawing* getVmlDrawing() override;
    virtual ::oox::drawingml::chart::ChartConverter* getChartConverter() override;

private:
    virtual OUString SAL_CALL getImplementationName() override;
    virtual ::oox::ole::VbaProject* implCreateVbaProject() const override;
    drawingml::ThemePtr mpThemePtr;
};

} }

#endif // INCLUDED_OOX_PPT_DGMLAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
