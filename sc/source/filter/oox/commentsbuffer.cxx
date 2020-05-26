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

#include <commentsbuffer.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/sheet/XSheetAnnotationAnchor.hpp>
#include <com/sun/star/sheet/XSheetAnnotationShapeSupplier.hpp>
#include <com/sun/star/sheet/XSheetAnnotations.hpp>
#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>
#include <com/sun/star/text/XText.hpp>
#include <osl/diagnose.h>
#include <oox/helper/attributelist.hxx>
#include <oox/vml/vmlshape.hxx>
#include <addressconverter.hxx>
#include <drawingfragment.hxx>
#include <svx/sdtaitm.hxx>
#include <svx/svdocapt.hxx>
#include <svx/unoshape.hxx>
#include <tools/diagnose_ex.h>
#include <document.hxx>
#include <drwlayer.hxx>
#include <cellsuno.hxx>
#include <docfunc.hxx>
#include <docuno.hxx>
#include <docsh.hxx>
#include <postit.hxx>

namespace oox::xls {

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
        case XML_Top:
            return SDRTEXTVERTADJUST_TOP;
        case XML_center:
        case XML_Center:
            return SDRTEXTVERTADJUST_CENTER;
        case XML_bottom:
        case XML_Bottom:
            return SDRTEXTVERTADJUST_BOTTOM;
        default:
            return SDRTEXTVERTADJUST_BLOCK;
    }
}

static sal_Int16 lcl_ToParaAlign(sal_Int32 nAlign)
{
    switch ( nAlign )
    {
        case XML_Left:
            return sal_Int16(css::style::ParagraphAdjust_LEFT);
        case XML_Right:
            return sal_Int16(css::style::ParagraphAdjust_RIGHT);
        case XML_Center:
            return sal_Int16(css::style::ParagraphAdjust_CENTER);
        default:
            return sal_Int16(css::style::ParagraphAdjust_BLOCK);
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
    maModel.mxText = std::make_shared<RichString>( *this );
    return maModel.mxText;
}

void Comment::finalizeImport()
{
    // BIFF12 stores cell range instead of cell address, use first cell of this range
    OSL_ENSURE( maModel.maRange.aStart == maModel.maRange.aEnd,
        "Comment::finalizeImport - comment anchor should be a single cell" );
    if( !getAddressConverter().checkCellAddress( maModel.maRange.aStart, true ) || !maModel.mxText )
        return;

    try
    {
        ScTableSheetObj* pAnnosSupp = static_cast<ScTableSheetObj*>(getSheet().get());
        rtl::Reference<ScAnnotationsObj> xAnnos = static_cast<ScAnnotationsObj*>(pAnnosSupp->getAnnotations().get());
        ScDocShell* pDocShell = xAnnos->GetDocShell();
        // non-empty string required by note implementation (real text will be added below)
        ScPostIt* pPostIt = pDocShell->GetDocFunc().ImportNote( maModel.maRange.aStart, OUString( ' ' ) );
        SdrCaptionObj* pCaption = pPostIt->GetOrCreateCaption( maModel.maRange.aStart );

        Reference< XShape > xAnnoShape( pCaption->getUnoShape(), UNO_QUERY_THROW ); // SvxShapeText
        // setting a property triggers expensive process, so set them all at once
        Reference< css::beans::XMultiPropertySet > xAnnoShapeMultiPropSet(xAnnoShape, UNO_QUERY_THROW);

        // Add shape formatting properties (autoFill, colHidden and rowHidden are dropped)
        xAnnoShapeMultiPropSet->setPropertyValues(
            Sequence<OUString> { "TextFitToSize", "MoveProtect", "TextHorizontalAdjust", "TextVerticalAdjust" },
            Sequence<Any> { Any(maModel.mbAutoScale), Any(maModel.mbLocked),
                Any(lcl_ToHorizAlign( maModel.mnTHA )), Any(lcl_ToVertAlign( maModel.mnTVA )) });
        if( maModel.maAnchor.Width > 0 && maModel.maAnchor.Height > 0 )
        {
            xAnnoShape->setPosition( css::awt::Point( maModel.maAnchor.X, maModel.maAnchor.Y ) );
            xAnnoShape->setSize( css::awt::Size( maModel.maAnchor.Width, maModel.maAnchor.Height ) );
        }

        // convert shape formatting and visibility
        bool bVisible = true;
        if( const ::oox::vml::ShapeBase* pVmlNoteShape = getVmlDrawing().getNoteShape( maModel.maRange.aStart ) )
        {
            // position and formatting
            pVmlNoteShape->convertFormatting( xAnnoShape );
            // visibility
            bVisible = pVmlNoteShape->getTypeModel().mbVisible;

            // Setting comment text alignment
            const ::oox::vml::ClientData* xClientData = pVmlNoteShape->getClientData();
            xAnnoShapeMultiPropSet->setPropertyValues(
                Sequence<OUString> { "TextVerticalAdjust", "ParaAdjust" },
                Sequence<Any> { Any(lcl_ToVertAlign( xClientData->mnTextVAlign )), Any(lcl_ToParaAlign( xClientData->mnTextHAlign )) });
        }
        if (bVisible)
            pDocShell->GetDocFunc().ShowNote( maModel.maRange.aStart, bVisible );

        // insert text and convert text formatting
        maModel.mxText->finalizeImport();
        Reference< XText > xAnnoText( xAnnoShape, UNO_QUERY_THROW );
        maModel.mxText->convert( xAnnoText );
    }
    catch( Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("sc");
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
    CommentRef xComment = std::make_shared<Comment>( *this );
    maComments.push_back( xComment );
    return xComment;
}

void CommentsBuffer::finalizeImport()
{
    // keep the model locked to avoid repeated reformatting in the model
    auto pModel = getScDocument().GetDrawLayer();
    bool bWasLocked = pModel->isLocked();
    pModel->setLock(true);
    maComments.forEachMem( &Comment::finalizeImport );
    pModel->setLock(bWasLocked);
}

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
