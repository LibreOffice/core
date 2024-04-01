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

#include <oox/token/tokens.hxx>

#include <commentsbuffer.hxx>

#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/text/XText.hpp>
#include <osl/diagnose.h>
#include <oox/drawingml/shapepropertymap.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/vml/vmlshape.hxx>
#include <addressconverter.hxx>
#include <drawingfragment.hxx>
#include <svx/sdtaitm.hxx>
#include <svx/svdocapt.hxx>
#include <svx/unoshape.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <document.hxx>
#include <drwlayer.hxx>
#include <cellsuno.hxx>
#include <docfunc.hxx>
#include <docuno.hxx>
#include <docsh.hxx>
#include <postit.hxx>

namespace oox::xls {

using namespace ::com::sun::star::drawing;
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
    maModel.mxText = std::make_shared<RichString>();
    return maModel.mxText;
}

namespace
{
    struct OOXGenerateNoteCaption : public GenerateNoteCaption
    {
        css::uno::Sequence<OUString> maPropertyNames;  /// import filter Caption object formatting property names
        css::uno::Sequence<css::uno::Any> maPropertyValues; /// import filter Caption object formatting property values
        std::shared_ptr<RichString> mxText;

        OOXGenerateNoteCaption(std::shared_ptr<RichString>& rText)
            : mxText(rText)
        {
        }

        virtual void Generate(SdrCaptionObj& rCaptionObj) override
        {
            rtl::Reference<SvxShapeText> xAnnoShape(dynamic_cast<SvxShapeText*>(rCaptionObj.getUnoShape().get())); // SvxShapeText
            assert(xAnnoShape && "will not be null");

            if (maPropertyNames.getLength())
            {
                // setting a property triggers expensive process, so set them all at once
                static_cast<SvxShape*>(xAnnoShape.get())->setPropertyValues(maPropertyNames, maPropertyValues);
            }

            // insert text and convert text formatting
            Reference< XText > xAnnoText( xAnnoShape );
            xAnnoShape->addActionLock();
            mxText->convert( xAnnoText );
            xAnnoShape->removeActionLock();
        }

        virtual OUString GetSimpleText() const override
        {
            return mxText->getStringContent();
        }
    };
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

        auto xGenerator = std::make_unique<OOXGenerateNoteCaption>(maModel.mxText);

        // Add shape formatting properties (autoFill, colHidden and rowHidden are dropped)
        // vvv TODO vvv TextFitToSize should be a drawing::TextFitToSizeType not bool
        xGenerator->maPropertyNames =
            css::uno::Sequence<OUString>{ "TextFitToSize", "MoveProtect", "TextHorizontalAdjust", "TextVerticalAdjust" };
        xGenerator->maPropertyValues =
            css::uno::Sequence<css::uno::Any>{ Any(maModel.mbAutoScale), Any(maModel.mbLocked),
                                               Any(lcl_ToHorizAlign( maModel.mnTHA )), Any(lcl_ToVertAlign( maModel.mnTVA )) };

        tools::Rectangle aCaptionRect;
        if( maModel.maAnchor.Width > 0 && maModel.maAnchor.Height > 0 )
        {
            aCaptionRect = tools::Rectangle(Point(maModel.maAnchor.X, maModel.maAnchor.Y),
                                            Size(maModel.maAnchor.Width, maModel.maAnchor.Height));
        }

        // convert shape formatting and visibility
        bool bVisible = true;
        if( const ::oox::vml::ShapeBase* pVmlNoteShape = getVmlDrawing().getNoteShape( maModel.maRange.aStart ) )
        {
            // position and formatting
            css::awt::Rectangle aShapeRect = pVmlNoteShape->getShapeRectangle();
            if (aShapeRect.Width > 0 || aShapeRect.Height > 0)
            {
                aCaptionRect = tools::Rectangle(Point(aShapeRect.X, aShapeRect.Y),
                                                Size(aShapeRect.Width, aShapeRect.Height));

                ::oox::drawingml::ShapePropertyMap aPropMap(pVmlNoteShape->makeShapePropertyMap());

                Sequence<OUString> aVMLPropNames;
                Sequence<Any> aVMLPropValues;
                aPropMap.fillSequences(aVMLPropNames, aVMLPropValues);

                sal_uInt32 nOldPropLen = xGenerator->maPropertyNames.getLength();
                sal_uInt32 nVMLPropLen = aVMLPropNames.getLength();
                xGenerator->maPropertyNames.realloc(nOldPropLen + nVMLPropLen);
                xGenerator->maPropertyValues.realloc(nOldPropLen + nVMLPropLen);
                OUString* pNames = xGenerator->maPropertyNames.getArray();
                Any* pValues = xGenerator->maPropertyValues.getArray();
                for (sal_uInt32 i = 0; i < nVMLPropLen; ++i)
                {
                    pNames[nOldPropLen + i] = aVMLPropNames[i];
                    pValues[nOldPropLen + i] = aVMLPropValues[i];
                }
            }

            // visibility
            bVisible = pVmlNoteShape->getTypeModel().mbVisible;

            // Setting comment text alignment
            const ::oox::vml::ClientData* xClientData = pVmlNoteShape->getClientData();
            sal_uInt32 nOldPropLen = xGenerator->maPropertyNames.getLength();
            xGenerator->maPropertyNames.realloc(nOldPropLen + 2);
            xGenerator->maPropertyValues.realloc(nOldPropLen + 2);
            OUString* pNames = xGenerator->maPropertyNames.getArray();
            Any* pValues = xGenerator->maPropertyValues.getArray();
            pNames[nOldPropLen] = "TextVerticalAdjust";
            pValues[nOldPropLen] <<= lcl_ToVertAlign(xClientData->mnTextVAlign);
            pNames[nOldPropLen + 1] = "ParaAdjust";
            pValues[nOldPropLen + 1] <<= lcl_ToParaAlign( xClientData->mnTextHAlign);
        }

        xGenerator->mxText->finalizeImport(*this);

        pDocShell->GetDocFunc().ImportNote(maModel.maRange.aStart, std::move(xGenerator),
                                           aCaptionRect, bVisible);

        if (bVisible)
            pDocShell->GetDocFunc().ShowNote( maModel.maRange.aStart, bVisible );
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
