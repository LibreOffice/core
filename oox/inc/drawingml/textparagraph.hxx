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

#ifndef INCLUDED_OOX_DRAWINGML_TEXTPARAGRAPH_HXX
#define INCLUDED_OOX_DRAWINGML_TEXTPARAGRAPH_HXX

#include <memory>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XText.hpp>

#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/refvector.hxx>
#include <drawingml/textrun.hxx>
#include <drawingml/textliststyle.hxx>
#include <drawingml/textparagraphproperties.hxx>

// The height the bullet is relative to is different in OOXML
#define OOX_BULLET_LIST_SCALE_FACTOR 0.7f

namespace oox::formulaimport {
    class XmlStreamBuilder;
}

namespace oox::drawingml {

typedef RefVector< TextRun > TextRunVector;

class TextParagraph
{
public:
    TextParagraph();
    ~TextParagraph();

    TextRunVector&       getRuns() { return maRuns; }
    const TextRunVector& getRuns() const { return maRuns; }
    void                 addRun( const TextRunPtr & pRun ) { maRuns.push_back( pRun ); }

    TextParagraphProperties&         getProperties() { return maProperties; }
    const TextParagraphProperties&   getProperties() const { return maProperties; }

    TextCharacterProperties&         getEndProperties() { return maEndProperties; }
    const TextCharacterProperties&   getEndProperties() const { return maEndProperties; }

    TextCharacterProperties          getCharacterStyle(
        const TextCharacterProperties& rTextStyleProperties,
        const TextListStyle& rTextListStyle) const;

    TextParagraphProperties*      getParagraphStyle(
        const TextListStyle& rTextListStyle) const;

    void                        insertAt(
                                    const ::oox::core::XmlFilterBase& rFilterBase,
                                    const css::uno::Reference < css::text::XText > & xText,
                                    const css::uno::Reference < css::text::XTextCursor > &xAt,
                                    const TextCharacterProperties& rTextStyleProperties,
                                    const TextListStyle& rTextListStyle,
                                    bool bFirst,
                                    float nDefaultCharHeight) const;

    bool HasMathXml() const
    {
        return m_pMathXml != nullptr;
    }
    formulaimport::XmlStreamBuilder & GetMathXml();

private:
    TextParagraphProperties     maProperties;
    TextCharacterProperties     maEndProperties;
    TextRunVector               maRuns;
    // temporarily store this here
    std::unique_ptr<formulaimport::XmlStreamBuilder> m_pMathXml;
};

}

#endif // INCLUDED_OOX_DRAWINGML_TEXTPARAGRAPH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
