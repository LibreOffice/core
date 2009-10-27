/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tablebuffer.cxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "oox/xls/commentsbuffer.hxx"
#include <com/sun/star/sheet/XSheetAnnotationAnchor.hpp>
#include <com/sun/star/sheet/XSheetAnnotationShapeSupplier.hpp>
#include <com/sun/star/sheet/XSheetAnnotations.hpp>
#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/vml/vmlshape.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/drawingfragment.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::sheet::XSheetAnnotation;
using ::com::sun::star::sheet::XSheetAnnotationAnchor;
using ::com::sun::star::sheet::XSheetAnnotationShapeSupplier;
using ::com::sun::star::sheet::XSheetAnnotations;
using ::com::sun::star::sheet::XSheetAnnotationsSupplier;

namespace oox {
namespace xls {

// ============================================================================

CommentModel::CommentModel() :
    mnAuthorId( -1 )
{
}

// ----------------------------------------------------------------------------

Comment::Comment( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper )
{
}

void Comment::importComment( const AttributeList& rAttribs )
{
    maModel.mnAuthorId = rAttribs.getInteger( XML_authorId, -1 );
    // cell range will be checked while inserting the comment into the document
    getAddressConverter().convertToCellRangeUnchecked( maModel.maRange, rAttribs.getString( XML_ref, OUString() ), getSheetIndex() );
}

void Comment::importComment( RecordInputStream& rStrm )
{
    BinRange aBinRange;
    rStrm >> maModel.mnAuthorId >> aBinRange;
    // cell range will be checked while inserting the comment into the document
    getAddressConverter().convertToCellRangeUnchecked( maModel.maRange, aBinRange, getSheetIndex() );
}

RichStringRef Comment::createText()
{
    maModel.mxText.reset( new RichString( *this ) );
    return maModel.mxText;
}

void Comment::finalizeImport()
{
    // OOBIN format stores cell range instead of cell address, use first cell of this range
    OSL_ENSURE( (maModel.maRange.StartColumn == maModel.maRange.EndColumn) &&
        (maModel.maRange.StartRow == maModel.maRange.EndRow),
        "Comment::finalizeImport - comment anchor should be a single cell" );
    CellAddress aNotePos( maModel.maRange.Sheet, maModel.maRange.StartColumn, maModel.maRange.StartRow );
    if( getAddressConverter().checkCellAddress( aNotePos, true ) && maModel.mxText.get() ) try
    {
        maModel.mxText->finalizeImport();
        OUString aNoteText = maModel.mxText->getPlainText();
        // non-empty string required by note implementation
        if( aNoteText.getLength() > 0 )
        {
            Reference< XSheetAnnotationsSupplier > xAnnosSupp( getSheet(), UNO_QUERY_THROW );
            Reference< XSheetAnnotations > xAnnos( xAnnosSupp->getAnnotations(), UNO_SET_THROW );
            xAnnos->insertNew( aNotePos, aNoteText );
            // receive craeted note from cell (insertNew does not return the note)
            Reference< XSheetAnnotationAnchor > xAnnoAnchor( getCell( aNotePos ), UNO_QUERY_THROW );
            Reference< XSheetAnnotation > xAnno( xAnnoAnchor->getAnnotation(), UNO_SET_THROW );
            Reference< XSheetAnnotationShapeSupplier > xAnnoShapeSupp( xAnno, UNO_QUERY_THROW );
            Reference< XShape > xAnnoShape( xAnnoShapeSupp->getAnnotationShape(), UNO_SET_THROW );
            // convert shape formatting
            if( const ::oox::vml::ShapeBase* pNoteShape = getVmlDrawing().getNoteShape( aNotePos ) )
            {
                // position and formatting
                pNoteShape->convertFormatting( xAnnoShape );
                // visibility
                const ::oox::vml::ShapeModel::ShapeClientDataPtr& rxClientData = pNoteShape->getShapeModel().mxClientData;
                bool bVisible = rxClientData.get() && rxClientData->mbVisible;
                xAnno->setIsVisible( bVisible );
            }
        }
    }
    catch( Exception& )
    {
    }
}

// ============================================================================

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

// ============================================================================

} // namespace xls
} // namespace oox

