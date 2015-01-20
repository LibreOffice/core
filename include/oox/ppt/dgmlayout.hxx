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

#include <oox/core/xmlfilterbase.hxx>

#include <com/sun/star/animations/XAnimationNode.hpp>
#include <oox/drawingml/theme.hxx>
#include <oox/ppt/presentationfragmenthandler.hxx>
#include <oox/ppt/slidepersist.hxx>
#include <vector>
#include <map>

namespace oox { namespace ppt {



class QuickDiagrammingLayout : public oox::core::XmlFilterBase
{
public:

    QuickDiagrammingLayout( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

    // from FilterBase
    virtual bool importDocument() throw (css::uno::RuntimeException) SAL_OVERRIDE;
    virtual bool exportDocument() throw() SAL_OVERRIDE;

    virtual const ::oox::drawingml::Theme* getCurrentTheme() const SAL_OVERRIDE;
    sal_Int32 getSchemeClr( sal_Int32 nColorSchemeToken ) const;
    virtual const oox::drawingml::table::TableStyleListPtr getTableStyles() SAL_OVERRIDE;

    virtual ::oox::vml::Drawing* getVmlDrawing() SAL_OVERRIDE;
    virtual ::oox::drawingml::chart::ChartConverter* getChartConverter() SAL_OVERRIDE;

private:
    virtual OUString implGetImplementationName() const SAL_OVERRIDE;
    virtual ::oox::ole::VbaProject* implCreateVbaProject() const SAL_OVERRIDE;
    drawingml::ThemePtr mpThemePtr;
};

} }

#endif // INCLUDED_OOX_PPT_DGMLAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
