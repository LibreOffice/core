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
#include <oox/helper/refvector.hxx>

namespace oox { namespace drawingml {

typedef RefVector< TextParagraphProperties > TextParagraphPropertiesVector;

class TextListStyle
{
public:

    TextListStyle();
    ~TextListStyle();

    TextListStyle(const TextListStyle& rStyle);
    TextListStyle& operator=(const TextListStyle& rStyle);

    void apply( const TextListStyle& rTextListStyle );

    const TextParagraphPropertiesVector& getListStyle() const { return maListStyle; };
    TextParagraphPropertiesVector&       getListStyle() { return maListStyle; };

    const TextParagraphPropertiesVector& getAggregationListStyle() const { return maAggregationListStyle; };
    TextParagraphPropertiesVector&       getAggregationListStyle() { return maAggregationListStyle; };

#ifdef DBG_UTIL
    void dump() const;
#endif

private:

    TextParagraphPropertiesVector maListStyle;
    TextParagraphPropertiesVector maAggregationListStyle;
};

} }

#endif // INCLUDED_OOX_DRAWINGML_TEXTLISTSTYLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
