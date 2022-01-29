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

#ifndef INCLUDED_OOX_DRAWINGML_TEXTLISTSTYLE_HXX
#define INCLUDED_OOX_DRAWINGML_TEXTLISTSTYLE_HXX

#include <drawingml/textparagraphproperties.hxx>
#include <array>

namespace oox::drawingml
{
constexpr int NUM_TEXT_LIST_STYLE_ENTRIES = 9;
typedef std::array<TextParagraphProperties, NUM_TEXT_LIST_STYLE_ENTRIES>
    TextParagraphPropertiesArray;

class TextListStyle
{
public:
    TextListStyle();
    ~TextListStyle();

    TextListStyle(const TextListStyle& rStyle);
    TextListStyle& operator=(const TextListStyle& rStyle);

    void apply(const TextListStyle& rTextListStyle);

    const TextParagraphPropertiesArray& getListStyle() const { return maListStyle; };
    TextParagraphPropertiesArray& getListStyle() { return maListStyle; };

    const TextParagraphPropertiesArray& getAggregationListStyle() const
    {
        return maAggregationListStyle;
    };
    TextParagraphPropertiesArray& getAggregationListStyle() { return maAggregationListStyle; };

    /// Flags ListStyle as having a non-empty lstStyle tag on import
    void setHasListStyleOnImport() { mbHasListStyleOnImport = true; }
    /** Returns whether the lstStyle tag was non-empty on import
     *
     *  @return true if list style has its own noninherited properties.
     */
    bool hasListStyleOnImport() const { return mbHasListStyleOnImport; }

#ifdef DBG_UTIL
    void dump() const;
#endif

private:
    TextParagraphPropertiesArray maListStyle;
    TextParagraphPropertiesArray maAggregationListStyle;
    /// Set if ListStyle has a non-empty lstStyle tag on import
    bool mbHasListStyleOnImport;
};
}

#endif // INCLUDED_OOX_DRAWINGML_TEXTLISTSTYLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
