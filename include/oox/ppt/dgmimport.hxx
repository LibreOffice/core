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

#ifndef INCLUDED_OOX_PPT_DGMIMPORT_HXX
#define INCLUDED_OOX_PPT_DGMIMPORT_HXX

#include <oox/core/xmlfilterbase.hxx>

#include <com/sun/star/animations/XAnimationNode.hpp>
#include <oox/drawingml/theme.hxx>
#include <oox/ppt/presentationfragmenthandler.hxx>
#include <oox/ppt/slidepersist.hxx>
#include <vector>
#include <map>

namespace oox { namespace ppt {



class QuickDiagrammingImport : public oox::core::XmlFilterBase
{
public:

    QuickDiagrammingImport( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    // from FilterBase
    virtual bool importDocument() throw (css::uno::RuntimeException, std::exception) override;
    virtual bool exportDocument() throw() override;

    virtual const ::oox::drawingml::Theme* getCurrentTheme() const override;
    virtual const oox::drawingml::table::TableStyleListPtr getTableStyles() override;

    virtual oox::vml::Drawing* getVmlDrawing() override;
    virtual oox::drawingml::chart::ChartConverter* getChartConverter() override;

private:
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) override;
    virtual ::oox::ole::VbaProject* implCreateVbaProject() const override;
};

} }

#endif // INCLUDED_OOX_PPT_DGMIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
