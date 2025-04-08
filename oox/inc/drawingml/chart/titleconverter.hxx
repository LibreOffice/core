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

#ifndef INCLUDED_OOX_DRAWINGML_CHART_TITLECONVERTER_HXX
#define INCLUDED_OOX_DRAWINGML_CHART_TITLECONVERTER_HXX

#include <drawingml/chart/converterbase.hxx>

namespace com::sun::star {
    namespace chart2 { class XDiagram; }
    namespace chart2 { class XFormattedString; }
    namespace chart2 { class XTitled; }
    namespace chart2::data { class XDataSequence; }
}

namespace oox::drawingml::chart {


struct TextModel;

class TextConverter final : public ConverterBase< TextModel >
{
public:
    explicit            TextConverter( const ConverterRoot& rParent, TextModel& rModel );
    virtual             ~TextConverter() override;

    /** Creates a data sequence object from the contained text data. */
    css::uno::Reference< css::chart2::data::XDataSequence >
                        createDataSequence( const OUString& rRole );
    /** Creates a sequence of formatted string objects. */
    css::uno::Sequence< css::uno::Reference< css::chart2::XFormattedString > >
                        createStringSequence(
                            const OUString& rDefaultText,
                            const ModelRef< TextBody >& rxTextProp,
                            ObjectType eObjType );

private:
    css::uno::Reference< css::chart2::XFormattedString >
                        appendFormattedString(
                            ::std::vector< css::uno::Reference< css::chart2::XFormattedString > >& orStringVec,
                            const OUString& rString,
                            bool bAddNewLine ) const;
};


struct TitleModel;

class TitleConverter final : public ConverterBase< TitleModel >
{
public:
    explicit            TitleConverter( const ConverterRoot& rParent, TitleModel& rModel );
    virtual             ~TitleConverter() override;

    /** Creates a title text object and attaches it at the passed interface. */
    void                convertFromModel(
                            const css::uno::Reference< css::chart2::XTitled >& rxTitled,
                            const OUString& rAutoTitle, ObjectType eObjType,
                            sal_Int32 nMainIdx = -1, sal_Int32 nSubIdx = -1 );
};


struct LegendModel;

class LegendConverter final : public ConverterBase< LegendModel >
{
public:
    explicit            LegendConverter( const ConverterRoot& rParent, LegendModel& rModel );
    virtual             ~LegendConverter() override;

    /** Creates a legend object and attaches it at the passed diagram. */
    void                convertFromModel(
                            const css::uno::Reference< css::chart2::XDiagram >& rxDiagram );

private:
    void                legendEntriesFormatting(const css::uno::Reference<css::chart2::XDiagram>& rxDiagram);
};


} // namespace oox::drawingml::chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
