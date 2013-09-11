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

#ifndef OOX_DRAWINGML_TEXTPARAGRAPH_HXX
#define OOX_DRAWINGML_TEXTPARAGRAPH_HXX

#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XText.hpp>

#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/textrun.hxx"
#include "oox/drawingml/textliststyle.hxx"
#include "oox/drawingml/textparagraphproperties.hxx"

namespace oox { namespace drawingml {

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

    void                        insertAt(
                                    const ::oox::core::XmlFilterBase& rFilterBase,
                                    const ::com::sun::star::uno::Reference < ::com::sun::star::text::XText > & xText,
                                    const ::com::sun::star::uno::Reference < ::com::sun::star::text::XTextCursor > &xAt,
                                    const TextCharacterProperties& rTextStyleProperties,
                                    const TextListStyle& rTextListStyle,
                                    bool bFirst = false ) const;

private:
    TextParagraphProperties     maProperties;
    TextCharacterProperties     maEndProperties;
    TextRunVector               maRuns;
};

typedef boost::shared_ptr< TextParagraph > TextParagraphPtr;

} }

#endif  //  OOX_DRAWINGML_TEXTPARAGRAPH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
