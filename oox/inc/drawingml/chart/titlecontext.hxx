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

#ifndef INCLUDED_OOX_DRAWINGML_CHART_TITLECONTEXT_HXX
#define INCLUDED_OOX_DRAWINGML_CHART_TITLECONTEXT_HXX

#include <drawingml/chart/chartcontextbase.hxx>

namespace oox {
namespace drawingml {
namespace chart {



struct TextModel;

/** Handler for a chart text context (c:tx element).
 */
class TextContext : public ContextBase< TextModel >
{
public:
    explicit            TextContext(  ::oox::core::ContextHandler2Helper& rParent, TextModel& rModel );
    virtual             ~TextContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onCharacters( const OUString& rChars ) override;
};



struct TitleModel;

/** Handler for a chart title context (c:title element).
 */
class TitleContext : public ContextBase< TitleModel >
{
public:
    explicit            TitleContext( ::oox::core::ContextHandler2Helper& rParent, TitleModel& rModel );
    virtual             ~TitleContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



struct LegendModel;

/** Handler for a chart legend context (c:legend element).
 */
class LegendContext : public ContextBase< LegendModel >
{
public:
    explicit            LegendContext( ::oox::core::ContextHandler2Helper& rParent, LegendModel& rModel );
    virtual             ~LegendContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
