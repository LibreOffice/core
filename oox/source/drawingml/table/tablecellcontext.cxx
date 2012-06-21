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

#include <osl/diagnose.h>

#include "oox/drawingml/table/tablecellcontext.hxx"
#include "oox/drawingml/textbodycontext.hxx"
#include "oox/drawingml/linepropertiescontext.hxx"
#include "oox/drawingml/fillpropertiesgroupcontext.hxx"
#include "oox/helper/attributelist.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star;
using ::rtl::OUString;

namespace oox { namespace drawingml { namespace table {

TableCellContext::TableCellContext( ContextHandler& rParent, const uno::Reference< xml::sax::XFastAttributeList >& xAttribs, TableCell& rTableCell )
: ContextHandler( rParent )
, mrTableCell( rTableCell )
{
    if ( xAttribs->hasAttribute( XML_rowSpan ) )
        mrTableCell.setRowSpan( xAttribs->getOptionalValue( XML_rowSpan ).toInt32() );
    if ( xAttribs->hasAttribute( XML_gridSpan ) )
        mrTableCell.setGridSpan( xAttribs->getOptionalValue( XML_gridSpan ).toInt32() );

    AttributeList aAttribs( xAttribs );
    mrTableCell.sethMerge( aAttribs.getBool( XML_hMerge, sal_False ) );
    mrTableCell.setvMerge( aAttribs.getBool( XML_vMerge, sal_False ) );
}

TableCellContext::~TableCellContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
TableCellContext::createFastChildContext( ::sal_Int32 aElementToken, const uno::Reference< xml::sax::XFastAttributeList >& xAttribs )
    throw ( xml::sax::SAXException, uno::RuntimeException)
{
    uno::Reference< xml::sax::XFastContextHandler > xRet;

    switch( aElementToken )
    {
    case A_TOKEN( txBody ):     // CT_TextBody
        {
            oox::drawingml::TextBodyPtr xTextBody( new oox::drawingml::TextBody );
            mrTableCell.setTextBody( xTextBody );
            xRet = new oox::drawingml::TextBodyContext( *this, *xTextBody );
        }
        break;

    case A_TOKEN( tcPr ):       // CT_TableCellProperties
        {
            AttributeList aAttribs( xAttribs );
            mrTableCell.setLeftMargin( aAttribs.getInteger( XML_marL, 91440 ) );
            mrTableCell.setRightMargin( aAttribs.getInteger( XML_marR, 91440 ) );
            mrTableCell.setTopMargin( aAttribs.getInteger( XML_marT, 45720 ) );
            mrTableCell.setBottomMargin( aAttribs.getInteger( XML_marB, 45720 ) );
            mrTableCell.setVertToken( xAttribs->getOptionalValueToken( XML_vert, XML_horz ) );                  // ST_TextVerticalType
            mrTableCell.setAnchorToken( xAttribs->getOptionalValueToken( XML_anchor, XML_t ) );                 // ST_TextAnchoringType
            mrTableCell.setAnchorCtr( aAttribs.getBool( XML_anchorCtr, sal_False ) );
            mrTableCell.setHorzOverflowToken( xAttribs->getOptionalValueToken( XML_horzOverflow, XML_clip ) );  // ST_TextHorzOverflowType
        }
        break;
        case A_TOKEN( lnL ):
                xRet.set( new oox::drawingml::LinePropertiesContext( *this, xAttribs, mrTableCell.maLinePropertiesLeft ) );
            break;
        case A_TOKEN( lnR ):
                xRet.set( new oox::drawingml::LinePropertiesContext( *this, xAttribs, mrTableCell.maLinePropertiesRight ) );
            break;
        case A_TOKEN( lnT ):
                xRet.set( new oox::drawingml::LinePropertiesContext( *this, xAttribs, mrTableCell.maLinePropertiesTop ) );
            break;
        case A_TOKEN( lnB ):
                xRet.set( new oox::drawingml::LinePropertiesContext( *this, xAttribs, mrTableCell.maLinePropertiesBottom ) );
            break;
        case A_TOKEN( lnTlToBr ):
                xRet.set( new oox::drawingml::LinePropertiesContext( *this, xAttribs, mrTableCell.maLinePropertiesTopLeftToBottomRight ) );
            break;
        case A_TOKEN( lnBlToTr ):
                xRet.set( new oox::drawingml::LinePropertiesContext( *this, xAttribs, mrTableCell.maLinePropertiesBottomLeftToTopRight ) );
            break;
        case A_TOKEN( cell3D ): // CT_Cell3D
        break;

    case A_TOKEN( extLst ):     // CT_OfficeArtExtensionList
    break;

    default:
        xRet.set( FillPropertiesContext::createFillContext( *this, aElementToken, xAttribs, mrTableCell.maFillProperties ) );
    break;

    }
    if ( !xRet.is() )
    {
        uno::Reference< XFastContextHandler > xTmp( this );
        xRet.set( xTmp );
    }
    return xRet;
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
