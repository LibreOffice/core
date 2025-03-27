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


#include <drawingml/table/tablecellcontext.hxx>
#include <drawingml/textbodycontext.hxx>
#include <drawingml/linepropertiescontext.hxx>
#include <drawingml/misccontexts.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star;

namespace oox::drawingml::table {

TableCellContext::TableCellContext( ContextHandler2Helper const & rParent, const AttributeList& rAttribs, TableCell& rTableCell )
: ContextHandler2( rParent )
, mrTableCell( rTableCell )
{
    if ( rAttribs.hasAttribute( XML_rowSpan ) )
        mrTableCell.setRowSpan( rAttribs.getInteger( XML_rowSpan, 0 ) );
    if ( rAttribs.hasAttribute( XML_gridSpan ) )
        mrTableCell.setGridSpan( rAttribs.getInteger( XML_gridSpan, 0 ) );

    mrTableCell.sethMerge( rAttribs.getBool( XML_hMerge, false ) );
    mrTableCell.setvMerge( rAttribs.getBool( XML_vMerge, false ) );
}

TableCellContext::~TableCellContext()
{
}

ContextHandlerRef
TableCellContext::onCreateContext( ::sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
    case A_TOKEN( txBody ):     // CT_TextBody
        {
            oox::drawingml::TextBodyPtr xTextBody = std::make_shared<oox::drawingml::TextBody>();
            xTextBody->getTextProperties().maPropertyMap.setProperty(
                PROP_FontIndependentLineSpacing, true);
            mrTableCell.setTextBody( xTextBody );
            return new oox::drawingml::TextBodyContext( *this, *xTextBody );
        }

    case A_TOKEN( tcPr ):       // CT_TableCellProperties
        {
            mrTableCell.setLeftMargin( rAttribs.getInteger( XML_marL, 91440 ) );
            mrTableCell.setRightMargin( rAttribs.getInteger( XML_marR, 91440 ) );
            mrTableCell.setTopMargin( rAttribs.getInteger( XML_marT, 45720 ) );
            mrTableCell.setBottomMargin( rAttribs.getInteger( XML_marB, 45720 ) );
            mrTableCell.setVertToken( rAttribs.getToken( XML_vert, XML_horz ) );                  // ST_TextVerticalType
            mrTableCell.setAnchorToken( rAttribs.getToken( XML_anchor, XML_t ) );                 // ST_TextAnchoringType
            mrTableCell.setAnchorCtr( rAttribs.getBool( XML_anchorCtr, false ) );
            mrTableCell.setHorzOverflowToken( rAttribs.getToken( XML_horzOverflow, XML_clip ) );  // ST_TextHorzOverflowType
        }
        break;
        case A_TOKEN( lnL ):
                return new oox::drawingml::LinePropertiesContext( *this, rAttribs, mrTableCell.maLinePropertiesLeft );
        case A_TOKEN( lnR ):
                return new oox::drawingml::LinePropertiesContext( *this, rAttribs, mrTableCell.maLinePropertiesRight );
        case A_TOKEN( lnT ):
                return new oox::drawingml::LinePropertiesContext( *this, rAttribs, mrTableCell.maLinePropertiesTop );
        case A_TOKEN( lnB ):
                return new oox::drawingml::LinePropertiesContext( *this, rAttribs, mrTableCell.maLinePropertiesBottom );
        case A_TOKEN( lnTlToBr ):
                return new oox::drawingml::LinePropertiesContext( *this, rAttribs, mrTableCell.maLinePropertiesTopLeftToBottomRight );
        case A_TOKEN( lnBlToTr ):
                return new oox::drawingml::LinePropertiesContext( *this, rAttribs, mrTableCell.maLinePropertiesBottomLeftToTopRight );
        case A_TOKEN( cell3D ): // CT_Cell3D
        break;

    case A_TOKEN( extLst ):     // CT_OfficeArtExtensionList
    break;

    default:
        return FillPropertiesContext::createFillContext(*this, aElementToken, rAttribs, mrTableCell.maFillProperties, nullptr);
    }

    return this;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
