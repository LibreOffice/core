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

#include <drawingml/textliststylecontext.hxx>
#include <drawingml/textparagraphpropertiescontext.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox::drawingml {

// CT_TextListStyle
TextListStyleContext::TextListStyleContext( ContextHandler2Helper const & rParent, TextListStyle& rTextListStyle )
: ContextHandler2( rParent )
, mrTextListStyle( rTextListStyle )
{
}

TextListStyleContext::~TextListStyleContext()
{
}

ContextHandlerRef TextListStyleContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
        case A_TOKEN( defPPr ):     // CT_TextParagraphProperties
            return new TextParagraphPropertiesContext( *this, rAttribs, mrTextListStyle.getListStyle()[ 0 ] );
        case A_TOKEN( outline1pPr ):
            return new TextParagraphPropertiesContext( *this, rAttribs, mrTextListStyle.getAggregationListStyle()[ 0 ] );
        case A_TOKEN( outline2pPr ):
            return new TextParagraphPropertiesContext( *this, rAttribs, mrTextListStyle.getAggregationListStyle()[ 1 ] );
        case A_TOKEN( lvl1pPr ):
            return new TextParagraphPropertiesContext( *this, rAttribs, mrTextListStyle.getListStyle()[ 0 ] );
        case A_TOKEN( lvl2pPr ):
            return new TextParagraphPropertiesContext( *this, rAttribs, mrTextListStyle.getListStyle()[ 1 ] );
        case A_TOKEN( lvl3pPr ):
            return new TextParagraphPropertiesContext( *this, rAttribs, mrTextListStyle.getListStyle()[ 2 ] );
        case A_TOKEN( lvl4pPr ):
            return new TextParagraphPropertiesContext( *this, rAttribs, mrTextListStyle.getListStyle()[ 3 ] );
        case A_TOKEN( lvl5pPr ):
            return new TextParagraphPropertiesContext( *this, rAttribs, mrTextListStyle.getListStyle()[ 4 ] );
        case A_TOKEN( lvl6pPr ):
            return new TextParagraphPropertiesContext( *this, rAttribs, mrTextListStyle.getListStyle()[ 5 ] );
        case A_TOKEN( lvl7pPr ):
            return new TextParagraphPropertiesContext( *this, rAttribs, mrTextListStyle.getListStyle()[ 6 ] );
        case A_TOKEN( lvl8pPr ):
            return new TextParagraphPropertiesContext( *this, rAttribs, mrTextListStyle.getListStyle()[ 7 ] );
        case A_TOKEN( lvl9pPr ):
            return new TextParagraphPropertiesContext( *this, rAttribs, mrTextListStyle.getListStyle()[ 8 ] );
    }

    return this;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
