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

#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>

#include "commentsbuffer.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSheetAnnotationAnchor.hpp>
#include <com/sun/star/sheet/XSheetAnnotationShapeSupplier.hpp>
#include <com/sun/star/sheet/XSheetAnnotations.hpp>
#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>
#include <osl/diagnose.h>
#include <oox/helper/attributelist.hxx>
#include <oox/vml/vmlshape.hxx>
#include "addressconverter.hxx"
#include "biffinputstream.hxx"
#include "drawingfragment.hxx"
#include <svx/sdtaitm.hxx>
#include "unitconverter.hxx"
#include "drawingmanager.hxx"

#include <com/sun/star/text/XText.hpp>

using ::com::sun::star::text::XText;

namespace oox {
namespace xls {

using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;

static sal_Int32 lcl_ToHorizAlign( sal_Int32 nAlign )
{
    switch( nAlign )
    {
        case XML_left:
            return SDRTEXTHORZADJUST_LEFT;
        case XML_right:
            return SDRTEXTHORZADJUST_RIGHT;
        case XML_center:
            return SDRTEXTHORZADJUST_CENTER;
        default:
            return SDRTEXTHORZADJUST_BLOCK;
    }
}

static sal_Int32 lcl_ToVertAlign( sal_Int32 nAlign )
{
    switch( nAlign )
    {
        case XML_top:
            return SDRTEXTVERTADJUST_TOP;
        case XML_center:
            return SDRTEXTVERTADJUST_CENTER;
        case XML_bottom:
            return SDRTEXTVERTADJUST_BOTTOM;
        default:
            return SDRTEXTVERTADJUST_BLOCK;
    }
}

CommentModel::CommentModel()
    : mnAuthorId(-1)
    , mbAutoFill(false)
    , mbAutoScale(false)
    , mbColHidden(false)
    , mbLocked(false)
    , mbRowHidden(false)
    , mnTHA(0)
    , mnTVA(0)
{
}

Comment::Comment( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper )
{
}

void Comment::importComment( const AttributeList& rAttribs )
{
    maModel.mnAuthorId = rAttribs.getInteger( XML_authorId, -1 );
    // cell range will be checked while inserting the comment into the document
    AddressConverter::convertToCellRangeUnchecked( maModel.maRange, rAttribs.getString( XML_ref, OUString() ), getSheetIndex() );
}

void Comment::importCommentPr( const AttributeList& rAttribs )
{
    maModel.mbAutoFill  = rAttribs.getBool( XML_autoFill, true );
    maModel.mbAutoScale = rAttribs.getBool( XML_autoScale, false );
    maModel.mbColHidden = rAttribs.getBool( XML_colHidden, false );
    maModel.mbLocked    = rAttribs.getBool( XML_locked, false );
    maModel.mbRowHidden = rAttribs.getBool( XML_rowHidden, false );
    maModel.mnTHA       = rAttribs.getToken( XML_textHAlign, XML_left );
    maModel.mnTVA       = rAttribs.getToken( XML_textVAlign, XML_top );
}

void Comment::importComment( SequenceInputStream& rStrm )
{
    BinRange aBinRange;
    maModel.mnAuthorId = rStrm.readInt32();
    rStrm >> aBinRange;
    // cell range will be checked while inserting the comment into the document
    AddressConverter::convertToCellRangeUnchecked( maModel.maRange, aBinRange, getSheetIndex() );
}

RichStringRef const & Comment::createText()
{
    maModel.mxText.reset( new RichString( *this ) );
    return maModel.mxText;
}

void Comment::finalizeImport()
{
    // BIFF12 stores cell range instead of cell address, use first cell of this range
    OSL_ENSURE( (maModel.maRange.StartColumn == maModel.maRange.EndColumn) &&
        (maModel.maRange.StartRow == maModel.maRange.EndRow),
        "Comment::finalizeImport - comment anchor should be a single cell" );
    CellAddress aNotePos( maModel.maRange.Sheet, maModel.maRange.StartColumn, maModel.maRange.StartRow );
    if( getAddressConverter().checkCellAddress( aNotePos, true ) && maModel.mxText.get() ) try
    {
        Reference< XSheetAnnotationsSupplier > xAnnosSupp( getSheet(), UNO_QUERY_THROW );
        Reference< XSheetAnnotations > xAnnos( xAnnosSupp->getAnnotations(), UNO_SET_THROW );
        // non-empty string required by note implementation (real text will be added below)
        xAnnos->insertNew( aNotePos, OUString( ' ' ) );

        // receive created note from cell (insertNew does not return the note)
        Reference< XSheetAnnotationAnchor > xAnnoAnchor( getCell( aNotePos ), UNO_QUERY_THROW );
        Reference< XSheetAnnotation > xAnno( xAnnoAnchor->getAnnotation(), UNO_SET_THROW );
        Reference< XSheetAnnotationShapeSupplier > xAnnoShapeSupp( xAnno, UNO_QUERY_THROW );
        Reference< XShape > xAnnoShape( xAnnoShapeSupp->getAnnotationShape(), UNO_SET_THROW );

        // convert shape formatting and visibility
        bool bVisible = true;
        // Add shape formatting properties (autoFill, colHidden and rowHidden are dropped)
        PropertySet aCommentPr( xAnnoShape );
        aCommentPr.setProperty( PROP_TextFitToSize, maModel.mbAutoScale );
        aCommentPr.setProperty( PROP_MoveProtect, maModel.mbLocked );
        aCommentPr.setProperty( PROP_TextHorizontalAdjust, lcl_ToHorizAlign( maModel.mnTHA ) );
        aCommentPr.setProperty( PROP_TextVerticalAdjust, lcl_ToVertAlign( maModel.mnTVA ) );
        if( maModel.maAnchor.Width > 0 && maModel.maAnchor.Height > 0 )
        {
            xAnnoShape->setPosition( css::awt::Point( maModel.maAnchor.X, maModel.maAnchor.Y ) );
            xAnnoShape->setSize( css::awt::Size( maModel.maAnchor.Width, maModel.maAnchor.Height ) );
        }

        // convert shape formatting and visibility
        if( const ::oox::vml::ShapeBase* pNoteShape = getVmlDrawing().getNoteShape( aNotePos ) )
        {
            // position and formatting
            pNoteShape->convertFormatting( xAnnoShape );
            // visibility
            bVisible = pNoteShape->getTypeModel().mbVisible;
        }
        xAnno->setIsVisible( bVisible );

        // insert text and convert text formatting
        maModel.mxText->finalizeImport();
        Reference< XText > xAnnoText( xAnnoShape, UNO_QUERY_THROW );
        maModel.mxText->convert( xAnnoText, true );
    }
    catch( Exception& )
    {
    }
}

// private --------------------------------------------------------------------

CommentsBuffer::CommentsBuffer( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper )
{
}

void CommentsBuffer::appendAuthor( const OUString& rAuthor )
{
    maAuthors.push_back( rAuthor );
}

CommentRef CommentsBuffer::createComment()
{
    CommentRef xComment( new Comment( *this ) );
    maComments.push_back( xComment );
    return xComment;
}

void CommentsBuffer::finalizeImport()
{
    maComments.forEachMem( &Comment::finalizeImport );
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
