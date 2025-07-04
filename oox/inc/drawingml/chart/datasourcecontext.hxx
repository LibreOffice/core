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

#ifndef INCLUDED_OOX_DRAWINGML_CHART_DATASOURCECONTEXT_HXX
#define INCLUDED_OOX_DRAWINGML_CHART_DATASOURCECONTEXT_HXX

#include <memory>
#include <oox/drawingml/chart/datasourcemodel.hxx>
#include <drawingml/chart/chartcontextbase.hxx>

class SvNumberFormatter;

namespace oox::drawingml::chart {


struct DataSequenceModel;

typedef ContextBase< DataSequenceModel > DataSequenceContextBase;


/** Handler for a double sequence context (c:numLit, c:numRef elements).
 */
class DoubleSequenceContext final : public DataSequenceContextBase
{
public:
    explicit            DoubleSequenceContext( ::oox::core::ContextHandler2Helper& rParent, DataSequenceModel& rModel );
    virtual             ~DoubleSequenceContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onCharacters( const OUString& rChars ) override;

private:
    SvNumberFormatter*  getNumberFormatter();

private:
    sal_Int32           mnPtIndex;          /// Current data point index.
    std::unique_ptr<SvNumberFormatter>
                        mpNumberFormatter;
};


/** Handler for a string sequence context (c:multiLvlStrRef, c:strLit,
    c:strRef elements).
 */
class StringSequenceContext final : public DataSequenceContextBase
{
public:
    explicit            StringSequenceContext( ::oox::core::ContextHandler2Helper& rParent, DataSequenceModel& rModel );
    virtual             ~StringSequenceContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onCharacters( const OUString& rChars ) override;

private:
    sal_Int32           mnPtIndex;          /// Current data point index.
    bool                mbReadC15;          /// Allow reading extension tags data under c15 namespace.
};


struct DataSourceModel;

/** Handler for a data source context (c:bubbleSize, c:cat, c:minus, c:plus,
    c:val, c:xVal, c:yVal elements).
 */
class DataSourceContext final : public ContextBase< DataSourceModel >
{
public:
    explicit            DataSourceContext( ::oox::core::ContextHandler2Helper& rParent, DataSourceModel& rModel );
    virtual             ~DataSourceContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};

struct DataSourceCxModel;

/** Handler for a chartex data source context (cx:chartData, cx:data elements).
 */
class DataSourceCxContext final : public ContextBase< DataSourceCxModel >
{
public:
    explicit            DataSourceCxContext( ::oox::core::ContextHandler2Helper& rParent,
            DataSourceCxModel& rModel);
    virtual             ~DataSourceCxContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;

    DataSourceCxModel::DataSourceMap *paCurSource;
};


} // namespace oox::drawingml::chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
