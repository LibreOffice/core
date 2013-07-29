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

#include "oox/core/xmlfilterbase.hxx"
#include "oox/export/drawingml.hxx"
#include "oox/export/utils.hxx"
#include <oox/token/tokens.hxx>

#include <cstdio>
#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <unotools/fontdefs.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/graph.hxx>
#include <svtools/grfmgr.hxx>
#include <rtl/strbuf.hxx>
#include <sfx2/app.hxx>
#include <svl/languageoptions.hxx>
#include <filter/msfilter/escherex.hxx>
#include <filter/msfilter/util.hxx>
#include <editeng/svxenum.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdoashp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using ::com::sun::star::beans::PropertyState;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertyState;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::geometry::IntegerRectangle2D;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::style::LineSpacing;
using ::com::sun::star::text::XText;
using ::com::sun::star::text::XTextContent;
using ::com::sun::star::text::XTextField;
using ::com::sun::star::text::XTextRange;
using ::sax_fastparser::FSHelperPtr;

DBG(extern void dump_pset(Reference< XPropertySet > rXPropSet));

namespace oox {
namespace drawingml {

#define GETA(propName) \
    GetProperty( rXPropSet, String( #propName ) )

#define GETAD(propName) \
    ( GetPropertyAndState( rXPropSet, rXPropState, String( #propName ), eState ) && eState == beans::PropertyState_DIRECT_VALUE )

#define GET(variable, propName) \
    if ( GETA(propName) ) \
        mAny >>= variable;

// not thread safe
int DrawingML::mnImageCounter = 1;

void DrawingML::ResetCounters()
{
    mnImageCounter = 1;
}

bool DrawingML::GetProperty( Reference< XPropertySet > rXPropSet, OUString aName )
{
    bool bRetValue = false;

    try {
        mAny = rXPropSet->getPropertyValue( aName );
        if ( mAny.hasValue() )
            bRetValue = true;
    } catch( const Exception& ) { /* printf ("exception when trying to get value of property: %s\n", USS(aName)); */ }

    return bRetValue;
}

bool DrawingML::GetPropertyAndState( Reference< XPropertySet > rXPropSet, Reference< XPropertyState > rXPropState, String aName, PropertyState& eState )
{
    bool bRetValue = false;

    try {
        mAny = rXPropSet->getPropertyValue( aName );
        if ( mAny.hasValue() ) {
            bRetValue = true;
            eState = rXPropState->getPropertyState( aName );
        }
    } catch( const Exception& ) { /* printf ("exception when trying to get value of property: %s\n", USS(aName)); */ }

    return bRetValue;
}

void DrawingML::WriteColor( sal_uInt32 nColor )
{
    OString sColor = OString::valueOf( ( sal_Int64 ) nColor, 16 );
    if( sColor.getLength() < 6 ) {
        OStringBuffer sBuf( "0" );
        int remains = 5 - sColor.getLength();

        while( remains > 0 ) {
            sBuf.append( "0" );
            remains--;
        }

        sBuf.append( sColor );

        sColor = sBuf.getStr();
    }
    mpFS->singleElementNS( XML_a, XML_srgbClr, XML_val, sColor.getStr(), FSEND );
}

void DrawingML::WriteSolidFill( sal_uInt32 nColor )
{
    mpFS->startElementNS( XML_a, XML_solidFill, FSEND );
    WriteColor( nColor );
    mpFS->endElementNS( XML_a, XML_solidFill );
}

void DrawingML::WriteSolidFill( Reference< XPropertySet > rXPropSet )
{
    if ( GetProperty( rXPropSet, "FillColor" ) )
        WriteSolidFill( *((sal_uInt32*) mAny.getValue()) & 0xffffff );
}

void DrawingML::WriteGradientStop( sal_uInt16 nStop, sal_uInt32 nColor )
{
    mpFS->startElementNS( XML_a, XML_gs,
                          XML_pos, I32S( nStop * 1000 ),
                          FSEND );
    WriteColor( nColor );
    mpFS->endElementNS( XML_a, XML_gs );
}

sal_uInt32 DrawingML::ColorWithIntensity( sal_uInt32 nColor, sal_uInt32 nIntensity )
{
    return ( ( ( nColor & 0xff ) * nIntensity ) / 100 )
        | ( ( ( ( ( nColor & 0xff00 ) >> 8 ) * nIntensity ) / 100 ) << 8 )
        | ( ( ( ( ( nColor & 0xff0000 ) >> 8 ) * nIntensity ) / 100 ) << 8 );
}

void DrawingML::WriteGradientFill( Reference< XPropertySet > rXPropSet )
{
    awt::Gradient aGradient;
    if( GETA( FillGradient ) ) {
        aGradient = *static_cast< const awt::Gradient* >( mAny.getValue() );

        mpFS->startElementNS( XML_a, XML_gradFill, FSEND );

        switch( aGradient.Style ) {
            default:
            case GradientStyle_LINEAR:
                mpFS->startElementNS( XML_a, XML_gsLst, FSEND );
                WriteGradientStop( 0, ColorWithIntensity( aGradient.StartColor, aGradient.StartIntensity ) );
                WriteGradientStop( 100, ColorWithIntensity( aGradient.EndColor, aGradient.EndIntensity ) );
                mpFS->endElementNS( XML_a, XML_gsLst );
                mpFS->singleElementNS( XML_a, XML_lin,
                                       XML_ang, I32S( ( ( ( 3600 - aGradient.Angle + 900 ) * 6000 ) % 21600000 ) ),
                                       FSEND );
                break;

            case GradientStyle_AXIAL:
                mpFS->startElementNS( XML_a, XML_gsLst, FSEND );
                WriteGradientStop( 0, ColorWithIntensity( aGradient.EndColor, aGradient.EndIntensity ) );
                WriteGradientStop( 50, ColorWithIntensity( aGradient.StartColor, aGradient.StartIntensity ) );
                WriteGradientStop( 100, ColorWithIntensity( aGradient.EndColor, aGradient.EndIntensity ) );
                mpFS->endElementNS( XML_a, XML_gsLst );
                mpFS->singleElementNS( XML_a, XML_lin,
                                       XML_ang, I32S( ( ( ( 3600 - aGradient.Angle + 900 ) * 6000 ) % 21600000 ) ),
                                       FSEND );
                break;

                /* I don't see how to apply transformation to gradients, so
                 * elliptical will end as radial and square as
                 * rectangular. also position offsets are not applied */
            case GradientStyle_RADIAL:
            case GradientStyle_ELLIPTICAL:
            case GradientStyle_RECT:
            case GradientStyle_SQUARE:
                mpFS->startElementNS( XML_a, XML_gsLst, FSEND );
                WriteGradientStop( 0, ColorWithIntensity( aGradient.EndColor, aGradient.EndIntensity ) );
                WriteGradientStop( 100, ColorWithIntensity( aGradient.StartColor, aGradient.StartIntensity ) );
                mpFS->endElementNS( XML_a, XML_gsLst );
                mpFS->singleElementNS( XML_a, XML_path,
                                       XML_path, ( aGradient.Style == awt::GradientStyle_RADIAL || aGradient.Style == awt::GradientStyle_ELLIPTICAL ) ? "circle" : "rect",
                                       FSEND );
                break;
        }

        mpFS->endElementNS( XML_a, XML_gradFill );
    }

}

void DrawingML::WriteLineArrow( Reference< XPropertySet > rXPropSet, sal_Bool bLineStart )
{
    ESCHER_LineEnd eLineEnd;
    sal_Int32 nArrowLength;
    sal_Int32 nArrowWidth;

    if ( EscherPropertyContainer::GetLineArrow( bLineStart, rXPropSet, eLineEnd, nArrowLength, nArrowWidth ) ) {
        const char* len;
        const char* type;
        const char* width;

        switch( nArrowLength ) {
            case ESCHER_LineShortArrow:
                len = "sm";
                break;
            default:
            case ESCHER_LineMediumLenArrow:
                len = "med";
                break;
            case ESCHER_LineLongArrow:
                len = "lg";
                break;
        }

        switch( eLineEnd ) {
            default:
            case ESCHER_LineNoEnd:
                type = "none";
                break;
            case ESCHER_LineArrowEnd:
                type = "triangle";
                break;
            case ESCHER_LineArrowStealthEnd:
                type = "stealth";
                break;
            case ESCHER_LineArrowDiamondEnd:
                type = "diamond";
                break;
            case ESCHER_LineArrowOvalEnd:
                type = "oval";
                break;
            case ESCHER_LineArrowOpenEnd:
                type = "arrow";
                break;
        }

        switch( nArrowWidth ) {
            case ESCHER_LineNarrowArrow:
                width = "sm";
                break;
            default:
            case ESCHER_LineMediumWidthArrow:
                width = "med";
                break;
            case ESCHER_LineWideArrow:
                width = "lg";
                break;
        }

        mpFS->singleElementNS( XML_a, bLineStart ? XML_headEnd : XML_tailEnd,
                               XML_len, len,
                               XML_type, type,
                               XML_w, width,
                               FSEND );
    }
}

void DrawingML::WriteOutline( Reference< XPropertySet > rXPropSet )
{
    drawing::LineStyle aLineStyle( drawing::LineStyle_NONE );

    GET( aLineStyle, LineStyle );

    sal_uInt32 nLineWidth = 0;
    sal_uInt32 nColor = 0;
    sal_Bool bColorSet = sal_False;
    const char* cap = NULL;
    drawing::LineDash aLineDash;
    sal_Bool bDashSet = sal_False;
    bool bNoFill = false;

    GET( nLineWidth, LineWidth );

    switch( aLineStyle ) {
        case drawing::LineStyle_NONE:
            bNoFill = true;
            break;
        case drawing::LineStyle_DASH:
            if( GETA( LineDash ) ) {
                aLineDash = *(drawing::LineDash*) mAny.getValue();
                bDashSet = sal_True;
                if( aLineDash.Style == DashStyle_ROUND || aLineDash.Style == DashStyle_ROUNDRELATIVE )
                    cap = "rnd";

                DBG(printf("dash dots: %d dashes: %d dotlen: %d dashlen: %d distance: %d\n",
                            int( aLineDash.Dots ), int( aLineDash.Dashes ), int( aLineDash.DotLen ), int( aLineDash.DashLen ), int( aLineDash.Distance )));
            }
            /* fallthru intended */
        case drawing::LineStyle_SOLID:
        default:
            if ( GETA( LineColor ) ) {
                nColor = *((sal_uInt32*) mAny.getValue()) & 0xffffff;
                bColorSet = sal_True;
            }
            break;
    }

    mpFS->startElementNS( XML_a, XML_ln,
                          XML_cap, cap,
                          XML_w, nLineWidth > 1 ? I64S( MM100toEMU( nLineWidth ) ) : NULL,
                          FSEND );
    if( bColorSet )
        WriteSolidFill( nColor );

    if( bDashSet ) {
        mpFS->startElementNS( XML_a, XML_custDash, FSEND );
        int i;
        for( i = 0; i < aLineDash.Dots; i ++ )
            mpFS->singleElementNS( XML_a, XML_ds,
                                   XML_d, aLineDash.DotLen ? I64S( aLineDash.DotLen*1000 ) : "100000",
                                   XML_sp, I64S( aLineDash.Distance*1000 ),
                                   FSEND );
        for( i = 0; i < aLineDash.Dashes; i ++ )
            mpFS->singleElementNS( XML_a, XML_ds,
                                   XML_d, aLineDash.DashLen ? I64S( aLineDash.DashLen*1000 ) : "100000",
                                   XML_sp, I64S( aLineDash.Distance*1000 ),
                                   FSEND );
        mpFS->endElementNS( XML_a, XML_custDash );
    }

    if( !bNoFill && nLineWidth > 1 && GETA( LineJoint ) ) {
        LineJoint eLineJoint;

        mAny >>= eLineJoint;
        switch( eLineJoint ) {
            case LineJoint_NONE:
            case LineJoint_MIDDLE:
            case LineJoint_BEVEL:
                mpFS->singleElementNS( XML_a, XML_bevel, FSEND );
                break;
            default:
            case LineJoint_MITER:
                mpFS->singleElementNS( XML_a, XML_miter, FSEND );
                break;
            case LineJoint_ROUND:
                mpFS->singleElementNS( XML_a, XML_round, FSEND );
                break;
        }
    }

    if( !bNoFill )
    {
        WriteLineArrow( rXPropSet, sal_True );
        WriteLineArrow( rXPropSet, sal_False );
    }
    else
    {
        mpFS->singleElementNS( XML_a, XML_noFill, FSEND );
    }

    mpFS->endElementNS( XML_a, XML_ln );
}

OUString DrawingML::WriteImage( const OUString& rURL )
{
    OString aURLBS(OUStringToOString(rURL, RTL_TEXTENCODING_UTF8));

    const char aURLBegin[] = "vnd.sun.star.GraphicObject:";
    sal_Int32 index = aURLBS.indexOf(aURLBegin);

    if ( index != -1 )
    {
        DBG(printf ("begin: %ld %s\n", long( sizeof( aURLBegin ) ), USS( rURL ) + RTL_CONSTASCII_LENGTH( aURLBegin ) ));
        Graphic aGraphic = GraphicObject( aURLBS.copy(RTL_CONSTASCII_LENGTH(aURLBegin)) ).GetTransformedGraphic ();

        return WriteImage( aGraphic );
    } else {
        // add link to relations
    }

    return OUString();
}

const char* DrawingML::GetComponentDir()
{
    switch ( meDocumentType )
    {
        case DOCUMENT_DOCX: return "word";
        case DOCUMENT_PPTX: return "ppt";
        case DOCUMENT_XLSX: return "xl";
    }

    return "unknown";
}

const char* DrawingML::GetRelationCompPrefix()
{
    switch ( meDocumentType )
    {
        case DOCUMENT_DOCX: return "";
        case DOCUMENT_PPTX:
        case DOCUMENT_XLSX: return "../";
    }

    return "unknown";
}

OUString DrawingML::WriteImage( const Graphic& rGraphic )
{
    GfxLink aLink = rGraphic.GetLink ();
    OUString sMediaType;
    const char* pExtension = "";
    OUString sRelId;

    SvMemoryStream aStream;
    const void* aData = aLink.GetData();
    sal_Size nDataSize = aLink.GetDataSize();

    switch ( aLink.GetType() ) {
        case GFX_LINK_TYPE_NATIVE_GIF:
            sMediaType = "image/gif";
            pExtension = ".gif";
            break;
        case GFX_LINK_TYPE_NATIVE_JPG:
            sMediaType = "image/jpeg";
            pExtension = ".jpeg";
            break;
        case GFX_LINK_TYPE_NATIVE_PNG:
            sMediaType = "image/png";
            pExtension = ".png";
            break;
        case GFX_LINK_TYPE_NATIVE_TIF:
            sMediaType = "image/tiff";
            pExtension = ".tiff";
            break;
        case GFX_LINK_TYPE_NATIVE_WMF:
            sMediaType = "image/x-wmf";
            pExtension = ".wmf";
            break;
        case GFX_LINK_TYPE_NATIVE_MET:
            sMediaType = "image/x-met";
            pExtension = ".met";
            break;
        case GFX_LINK_TYPE_NATIVE_PCT:
            sMediaType = "image/x-pict";
            pExtension = ".pct";
            break;
        default: {
            GraphicType aType = rGraphic.GetType();
            if ( aType == GRAPHIC_BITMAP ) {
                GraphicConverter::Export( aStream, rGraphic, CVT_PNG );
                sMediaType = "image/png";
                pExtension = ".png";
            } else if ( aType == GRAPHIC_GDIMETAFILE ) {
                GraphicConverter::Export( aStream, rGraphic, CVT_EMF );
                sMediaType = "image/x-emf";
                pExtension = ".emf";
            } else {
                OSL_TRACE( "unhandled graphic type" );
                break;
            }

            aData = aStream.GetData();
            nDataSize = aStream.GetEndOfData();
            break;
            }
    }

    Reference< XOutputStream > xOutStream = mpFB->openFragmentStream( OUStringBuffer()
                                                                      .appendAscii( GetComponentDir() )
                                                                      .appendAscii( "/media/image" )
                                                                      .append( (sal_Int32) mnImageCounter )
                                                                      .appendAscii( pExtension )
                                                                      .makeStringAndClear(),
                                                                      sMediaType );
    xOutStream->writeBytes( Sequence< sal_Int8 >( (const sal_Int8*) aData, nDataSize ) );
    xOutStream->closeOutput();

    sRelId = mpFB->addRelation( mpFS->getOutputStream(),
                                "http://schemas.openxmlformats.org/officeDocument/2006/relationships/image",
                                OUStringBuffer()
                                .appendAscii( GetRelationCompPrefix() )
                                .appendAscii( "media/image" )
                                .append( (sal_Int32) mnImageCounter ++ )
                                .appendAscii( pExtension )
                                .makeStringAndClear() );

    return sRelId;
}

OUString DrawingML::WriteBlip( Reference< XPropertySet > rXPropSet, OUString& rURL, const Graphic *pGraphic )
{
    OUString sRelId = pGraphic ? WriteImage( *pGraphic ) : WriteImage( rURL );
    sal_Int16 nBright = 0;
    sal_Int32 nContrast = 0;

    GET( nBright, AdjustLuminance );
    GET( nContrast, AdjustContrast );

        mpFS->startElementNS( XML_a, XML_blip,
                  FSNS( XML_r, XML_embed), OUStringToOString( sRelId, RTL_TEXTENCODING_UTF8 ).getStr(),
                  FSEND );
    if( nBright || nContrast )
        mpFS->singleElementNS( XML_a, XML_lum,
                   XML_bright, nBright ? I32S( nBright*1000 ) : NULL,
                   XML_contrast, nContrast ? I32S( nContrast*1000 ) : NULL,
                   FSEND );

    mpFS->endElementNS( XML_a, XML_blip );

    return sRelId;
}

void DrawingML::WriteBlipMode( Reference< XPropertySet > rXPropSet )
{
    BitmapMode eBitmapMode( BitmapMode_NO_REPEAT );
    if (GetProperty( rXPropSet, "FillBitmapMode" ) )
        mAny >>= eBitmapMode;

    DBG(printf("fill bitmap mode: %d\n", eBitmapMode));

    switch (eBitmapMode) {
    case BitmapMode_REPEAT:
        mpFS->singleElementNS( XML_a, XML_tile, FSEND );
        break;
    case BitmapMode_STRETCH:
        mpFS->startElementNS( XML_a, XML_stretch, FSEND );
        mpFS->singleElementNS( XML_a, XML_fillRect, FSEND );
        mpFS->endElementNS( XML_a, XML_stretch );
        break;
    default:
        ;
    }
}

void DrawingML::WriteBlipFill( Reference< XPropertySet > rXPropSet, OUString sURLPropName )
{
    WriteBlipFill( rXPropSet, sURLPropName, XML_a );
}

void DrawingML::WriteBlipFill( Reference< XPropertySet > rXPropSet, OUString sURLPropName, sal_Int32 nXmlNamespace )
{
    if ( GetProperty( rXPropSet, sURLPropName ) ) {
        OUString aURL;
        mAny >>= aURL;

        DBG(printf ("URL: %s\n", OUStringToOString( aURL, RTL_TEXTENCODING_UTF8 ).getStr() ));

        if( aURL.isEmpty() )
            return;

        mpFS->startElementNS( nXmlNamespace , XML_blipFill, FSEND );

        WriteBlip( rXPropSet, aURL );

        if( sURLPropName == "FillBitmapURL" )
            WriteBlipMode( rXPropSet );
        else if( GetProperty( rXPropSet, "FillBitmapStretch" ) ) {
                bool bStretch = false;
                mAny >>= bStretch;

                if( bStretch )
                    WriteStretch();
        }

        mpFS->endElementNS( nXmlNamespace, XML_blipFill );
    }
}

void DrawingML::WriteSrcRect( Reference< XPropertySet > rXPropSet, const OUString& rURL )
{
    Size aOriginalSize( GraphicObject::CreateGraphicObjectFromURL( rURL ).GetPrefSize() );

    if ( GetProperty( rXPropSet, "GraphicCrop" ) )
    {
        ::com::sun::star::text::GraphicCrop aGraphicCropStruct;
        mAny >>= aGraphicCropStruct;

        if ( (0 != aGraphicCropStruct.Left) || (0 != aGraphicCropStruct.Top) || (0 != aGraphicCropStruct.Right) || (0 != aGraphicCropStruct.Bottom) )
        {
            mpFS->singleElementNS( XML_a, XML_srcRect,
                          XML_l, I32S(((aGraphicCropStruct.Left) * 100000)/aOriginalSize.Width()),
                          XML_t, I32S(((aGraphicCropStruct.Top) * 100000)/aOriginalSize.Height()),
                          XML_r, I32S(((aGraphicCropStruct.Right) * 100000)/aOriginalSize.Width()),
                          XML_b, I32S(((aGraphicCropStruct.Bottom) * 100000)/aOriginalSize.Height()),
                          FSEND );
        }
    }
}

void DrawingML::WriteStretch()
{
    mpFS->startElementNS( XML_a, XML_stretch, FSEND );
    mpFS->singleElementNS( XML_a, XML_fillRect, FSEND );
    mpFS->endElementNS( XML_a, XML_stretch );
}

void DrawingML::WriteTransformation( const Rectangle& rRect,
        sal_Int32 nXmlNamespace, sal_Bool bFlipH, sal_Bool bFlipV, sal_Int32 nRotation )
{
    mpFS->startElementNS( nXmlNamespace, XML_xfrm,
                          XML_flipH, bFlipH ? "1" : NULL,
                          XML_flipV, bFlipV ? "1" : NULL,
                          XML_rot, (nRotation % 21600000) ? I32S( nRotation ) : NULL,
                          FSEND );

    mpFS->singleElementNS( XML_a, XML_off, XML_x, IS( MM100toEMU( rRect.Left() ) ), XML_y, IS( MM100toEMU( rRect.Top() ) ), FSEND );
    mpFS->singleElementNS( XML_a, XML_ext, XML_cx, IS( MM100toEMU( rRect.GetWidth() ) ), XML_cy, IS( MM100toEMU( rRect.GetHeight() ) ), FSEND );

    mpFS->endElementNS( nXmlNamespace, XML_xfrm );
}

void DrawingML::WriteShapeTransformation( Reference< XShape > rXShape, sal_Int32 nXmlNamespace, sal_Bool bFlipH, sal_Bool bFlipV, sal_Bool bSuppressRotation  )
{
    DBG(printf( "write shape transformation\n" ));

    sal_Int32 nRotation=0;
    awt::Point aPos = rXShape->getPosition();
    awt::Size aSize = rXShape->getSize();

    if ( aSize.Width < 0 )
        aSize.Width = 1000;
    if ( aSize.Height < 0 )
        aSize.Height = 1000;
    if (!bSuppressRotation)
    {
        SdrObject* pShape = (SdrObject*) GetSdrObjectFromXShape( rXShape );
        nRotation=pShape->GetRotateAngle();
        int faccos=bFlipV ? -1 : 1;
        int facsin=bFlipH ? -1 : 1;
        aPos.X-=(1-faccos*cos(nRotation*F_PI18000))*aSize.Width/2-facsin*sin(nRotation*F_PI18000)*aSize.Height/2;
        aPos.Y-=(1-faccos*cos(nRotation*F_PI18000))*aSize.Height/2+facsin*sin(nRotation*F_PI18000)*aSize.Width/2;
    }
    if (!bSuppressRotation)
    {
        if (bFlipV) {nRotation=(nRotation+18000)%36000;}
    }
    WriteTransformation( Rectangle( Point( aPos.X, aPos.Y ), Size( aSize.Width, aSize.Height ) ), nXmlNamespace, bFlipH, bFlipV, PPTX_EXPORT_ROTATE_CLOCKWISIFY(nRotation) );
}

void DrawingML::WriteRunProperties( Reference< XPropertySet > rRun, sal_Bool bIsField )
{
    Reference< XPropertySet > rXPropSet( rRun, UNO_QUERY );
    Reference< XPropertyState > rXPropState( rRun, UNO_QUERY );
    OUString usLanguage;
    PropertyState eState;
    sal_Int16 nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( Application::GetSettings().GetLanguageTag().getLanguageType() );
    sal_Bool bComplex = ( nScriptType == ScriptType::COMPLEX );
    const char* bold = NULL;
    const char* italic = NULL;
    const char* underline = NULL;
    sal_Int32 nSize = 1800;

    if( GETAD( CharHeight ) )
        nSize = (sal_Int32) (100*(*((float*) mAny.getValue())));

    if ( ( bComplex && GETAD( CharWeightComplex ) ) || GETAD( CharWeight ) )
        if ( *((float*) mAny.getValue()) >= awt::FontWeight::SEMIBOLD )
            bold = "1";

    if ( ( bComplex && GETAD( CharPostureComplex ) ) || GETAD( CharPosture ) )
        switch ( *((awt::FontSlant*) mAny.getValue()) )
        {
            case awt::FontSlant_OBLIQUE :
            case awt::FontSlant_ITALIC :
                italic = "1";
                break;
            default:
                break;
        }

    if ( GETAD( CharUnderline ) )
        switch ( *((sal_Int16*) mAny.getValue()) )
        {
            case awt::FontUnderline::SINGLE :
                underline = "sng";
                break;
            case awt::FontUnderline::DOUBLE :
                underline = "dbl";
                break;
            case awt::FontUnderline::DOTTED :
                underline = "dotted";
                break;
            case awt::FontUnderline::DASH :
                underline = "dash";
                break;
            case awt::FontUnderline::LONGDASH :
                underline = "dashLong";
                break;
            case awt::FontUnderline::DASHDOT :
                underline = "dotDash";
                break;
            case awt::FontUnderline::DASHDOTDOT :
                underline = "dotDotDash";
                break;
            case awt::FontUnderline::WAVE :
                underline = "wavy";
                break;
            case awt::FontUnderline::DOUBLEWAVE :
                underline = "wavyDbl";
                break;
            case awt::FontUnderline::BOLD :
                underline = "heavy";
                break;
            case awt::FontUnderline::BOLDDOTTED :
                underline = "dottedHeavy";
                break;
            case awt::FontUnderline::BOLDDASH :
                underline = "dashHeavy";
                break;
            case awt::FontUnderline::BOLDLONGDASH :
                underline = "dashLongHeavy";
                break;
            case awt::FontUnderline::BOLDDASHDOT :
                underline = "dotDashHeavy";
                break;
            case awt::FontUnderline::BOLDDASHDOTDOT :
                underline = "dotDotDashHeavy";
                break;
            case awt::FontUnderline::BOLDWAVE :
                underline = "wavyHeavy";
                break;
        }

    if( GETA( CharLocale ) ) {
        com::sun::star::lang::Locale aLocale;
        mAny >>= aLocale;
        LanguageTag aLanguageTag( aLocale);
        if (!aLanguageTag.isSystemLocale())
            usLanguage = aLanguageTag.getBcp47();
    }

    mpFS->startElementNS( XML_a, XML_rPr,
                          XML_b, bold,
                          XML_i, italic,
                          XML_lang, usLanguage.isEmpty() ? NULL : USS( usLanguage ),
                          XML_sz, nSize == 1800 ? NULL : IS( nSize ),
                          XML_u, underline,
                          FSEND );

    // mso doesn't like text color to be placed after typeface
    if( GETAD( CharColor ) ) {
        sal_uInt32 color = *((sal_uInt32*) mAny.getValue());
        DBG(printf("run color: %x auto: %x\n", static_cast<unsigned int>( color ), static_cast<unsigned int>( COL_AUTO )));

        if( color == COL_AUTO ) { // nCharColor depends to the background color
            sal_Bool bIsDark = sal_False;
            GET( bIsDark, IsBackgroundDark );
            color = bIsDark ? 0xffffff : 0x000000;
        }
        color &= 0xffffff;

        // TODO: special handle embossed/engraved

        WriteSolidFill( color );
    }

    if( GETAD( CharFontName ) ) {
        const char* pitch = NULL;
        const char* charset = NULL;
        OUString usTypeface;

        mAny >>= usTypeface;
        OUString aSubstName( GetSubsFontName( usTypeface, SUBSFONT_ONLYONE | SUBSFONT_MS ) );

        mpFS->singleElementNS( XML_a, XML_latin,
                               XML_typeface, USS(aSubstName.getLength() ? aSubstName : usTypeface),
                               XML_pitchFamily, pitch,
                               XML_charset, charset,
                               FSEND );
    }

    if( ( bComplex && GETAD( CharFontNameComplex ) ) || ( !bComplex && GETAD( CharFontNameAsian ) ) ) {
        const char* pitch = NULL;
        const char* charset = NULL;
        OUString usTypeface;

        mAny >>= usTypeface;
        OUString aSubstName( GetSubsFontName( usTypeface, SUBSFONT_ONLYONE | SUBSFONT_MS ) );

        mpFS->singleElementNS( XML_a, bComplex ? XML_cs : XML_ea,
                               XML_typeface, USS(aSubstName.getLength() ? aSubstName : usTypeface),
                               XML_pitchFamily, pitch,
                               XML_charset, charset,
                               FSEND );
    }

    if( bIsField ) {
        Reference< XTextField > rXTextField;
        GET( rXTextField, TextField );
        if( rXTextField.is() )
            rRun.set( rXTextField, UNO_QUERY );
    }

    // field properties starts here
    if( GETA( URL ) ) {
    OUString sURL;

    mAny >>= sURL;
    if( !sURL.isEmpty() ) {
        OUString sRelId = mpFB->addRelation( mpFS->getOutputStream(),
                              "http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink",
                              sURL, true );

        mpFS->singleElementNS( XML_a, XML_hlinkClick,
                   FSNS( XML_r,XML_id ), USS( sRelId ),
                   FSEND );
    }
    }

    mpFS->endElementNS( XML_a, XML_rPr );
}

const char* DrawingML::GetFieldType( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > rRun, sal_Bool& bIsField )
{
    const char* sType = NULL;
    Reference< XPropertySet > rXPropSet( rRun, UNO_QUERY );
    OUString aFieldType;

    if( GETA( TextPortionType ) ) {
        aFieldType = OUString( *(OUString*)mAny.getValue() );
        DBG(printf ("field type: %s\n", USS(aFieldType) ));
    }

    if( aFieldType == "TextField" ) {
        Reference< XTextField > rXTextField;
        GET( rXTextField, TextField );
        if( rXTextField.is() ) {
        bIsField = sal_True;
            rXPropSet.set( rXTextField, UNO_QUERY );
            if( rXPropSet.is() ) {
                OUString aFieldKind( rXTextField->getPresentation( sal_True ) );
                DBG(printf ("field kind: %s\n", USS(aFieldKind) ));
                if( aFieldKind == "Page" ) {
                    return "slidenum";
                }
        // else if( aFieldKind == "URL" ) {
        // do not return here
        // and make URL field text run with hyperlink property later
        // }
            }
        }
    }

    return sType;
}

void DrawingML::GetUUID( OStringBuffer& rBuffer )
{
    Sequence< sal_uInt8 > aSeq( 16 );
    static const char cDigits[17] = "0123456789ABCDEF";
    rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
    int i;

    rBuffer.append( '{' );
    for( i = 0; i < 4; i++ ) {
        rBuffer.append( cDigits[ aSeq[i] >> 4 ] );
        rBuffer.append( cDigits[ aSeq[i] & 0xf ] );
    }
    rBuffer.append( '-' );
    for( ; i < 6; i++ ) {
        rBuffer.append( cDigits[ aSeq[i] >> 4 ] );
        rBuffer.append( cDigits[ aSeq[i] & 0xf ] );
    }
    rBuffer.append( '-' );
    for( ; i < 8; i++ ) {
        rBuffer.append( cDigits[ aSeq[i] >> 4 ] );
        rBuffer.append( cDigits[ aSeq[i] & 0xf ] );
    }
    rBuffer.append( '-' );
    for( ; i < 10; i++ ) {
        rBuffer.append( cDigits[ aSeq[i] >> 4 ] );
        rBuffer.append( cDigits[ aSeq[i] & 0xf ] );
    }
    rBuffer.append( '-' );
    for( ; i < 16; i++ ) {
        rBuffer.append( cDigits[ aSeq[i] >> 4 ] );
        rBuffer.append( cDigits[ aSeq[i] & 0xf ] );
    }
    rBuffer.append( '}' );
}

void DrawingML::WriteRun( Reference< XTextRange > rRun )
{
    const char* sFieldType;
    sal_Bool bIsField = sal_False;
    OUString sText = rRun->getString();

    if( sText.isEmpty()) {
        Reference< XPropertySet > xPropSet( rRun, UNO_QUERY );

        try {
        if( !xPropSet.is() || !( xPropSet->getPropertyValue( "PlaceholderText" ) >>= sText ) )
            return;
        if( sText.isEmpty() )
            return;
        }
        catch (const Exception &) {
            return;
        }
    }

    sFieldType = GetFieldType( rRun, bIsField );
    if( ( sFieldType != NULL ) ) {
        OStringBuffer sUUID(39);

        GetUUID( sUUID );
        mpFS->startElementNS( XML_a, XML_fld,
                              XML_id, sUUID.getStr(),
                              XML_type, sFieldType,
                              FSEND );
    } else
        mpFS->startElementNS( XML_a, XML_r, FSEND );

    Reference< XPropertySet > xPropSet( rRun, uno::UNO_QUERY );
    WriteRunProperties( xPropSet, bIsField );

    mpFS->startElementNS( XML_a, XML_t, FSEND );
    mpFS->writeEscaped( sText );
    mpFS->endElementNS( XML_a, XML_t );

    if( sFieldType )
        mpFS->endElementNS( XML_a, XML_fld );
    else
        mpFS->endElementNS( XML_a, XML_r );
}

#define AUTONUM(x) \
                        if( bPBoth ) \
                            pAutoNumType = #x "ParenBoth"; \
                        else if( bPBehind ) \
                            pAutoNumType = #x "ParenR"; \
                        else if( bSDot ) \
                            pAutoNumType = #x "Period";

inline static const char* GetAutoNumType( sal_Int16 nNumberingType, bool bSDot, bool bPBehind, bool bPBoth )
{
    const char* pAutoNumType = NULL;

    switch( (SvxExtNumType)nNumberingType )
        {
        case SVX_NUM_CHARS_UPPER_LETTER_N :
        case SVX_NUM_CHARS_UPPER_LETTER :
            AUTONUM( alphaUc );
            break;
        case SVX_NUM_CHARS_LOWER_LETTER_N :
        case SVX_NUM_CHARS_LOWER_LETTER :
            AUTONUM( alphaLc );
            break;
        case SVX_NUM_ROMAN_UPPER :
            AUTONUM( romanUc );
            break;
        case SVX_NUM_ROMAN_LOWER :
            AUTONUM( romanLc );
            break;
        case SVX_NUM_ARABIC :
            AUTONUM( arabic )
            else
                pAutoNumType = "arabicPlain";
                        break;
        default:
            break;
        }

    return pAutoNumType;
}

void DrawingML::WriteParagraphNumbering( Reference< XPropertySet > rXPropSet, sal_Int16 nLevel )
{
    if( nLevel >= 0 && GETA( NumberingRules ) )
    {
        Reference< XIndexAccess > rXIndexAccess;

        if ( ( mAny >>= rXIndexAccess ) && nLevel < rXIndexAccess->getCount() )
        {
            DBG(printf ("numbering rules\n"));

            Sequence< PropertyValue > aPropertySequence;
            rXIndexAccess->getByIndex( nLevel ) >>= aPropertySequence;

            const PropertyValue* pPropValue = aPropertySequence.getArray();

            sal_Int32 nPropertyCount = aPropertySequence.getLength();

            if ( nPropertyCount ) {

                sal_Int16 nNumberingType = -1;
                bool bSDot = false;
                bool bPBehind = false;
                bool bPBoth = false;
                sal_Unicode aBulletChar = 0x2022; // a bullet
                awt::FontDescriptor aFontDesc;
                bool bHasFontDesc = false;
                OUString aGraphicURL;
                sal_Int16 nBulletRelSize = 0;

                for ( sal_Int32 i = 0; i < nPropertyCount; i++ ) {
                    const void* pValue = pPropValue[ i ].Value.getValue();
                    if ( pValue ) {
                        OUString aPropName( pPropValue[ i ].Name );
                        DBG(printf ("pro name: %s\n", OUStringToOString( aPropName, RTL_TEXTENCODING_UTF8 ).getStr()));
                        if ( aPropName == "NumberingType" )
                            nNumberingType = *( (sal_Int16*)pValue );
                        else if ( aPropName == "Prefix" ) {
                            if( *(OUString*)pValue == ")")
                                bPBoth = true;
                        } else if ( aPropName == "Suffix" ) {
                            if( *(OUString*)pValue == ".")
                                bSDot = true;
                            else if( *(OUString*)pValue == ")")
                                bPBehind = true;
                        } else if ( aPropName == "BulletChar" )
                        {
                            aBulletChar = String ( *( (String*)pValue ) ).GetChar( 0 );
                            //printf ("bullet char: %d\n", aBulletChar.getStr());
                        }
                        else if ( aPropName == "BulletFont" )
                        {
                            aFontDesc = *( (awt::FontDescriptor*)pValue );
                            bHasFontDesc = true;

                            // Our numbullet dialog has set the wrong textencoding for our "StarSymbol" font,
                            // instead of a Unicode encoding the encoding RTL_TEXTENCODING_SYMBOL was used.
                            // Because there might exist a lot of damaged documemts I added this two lines
                            // which fixes the bullet problem for the export.
                            if ( aFontDesc.Name.equalsIgnoreAsciiCase("StarSymbol") )
                                aFontDesc.CharSet = RTL_TEXTENCODING_MS_1252;

                        } else if ( aPropName == "BulletRelSize" ) {
                            nBulletRelSize = *( (sal_Int16*)pValue );
                        } else if ( aPropName == "GraphicURL" ) {
                            aGraphicURL = ( *(OUString*)pValue );
                            DBG(printf ("graphic url: %s\n", OUStringToOString( aGraphicURL, RTL_TEXTENCODING_UTF8 ).getStr()));
                        } else if ( aPropName == "GraphicSize" )
                        {
                            if ( pPropValue[ i ].Value.getValueType() == ::getCppuType( (awt::Size*)0) )
                            {
                                // don't cast awt::Size to Size as on 64-bits they are not the same.
                                ::com::sun::star::awt::Size aSize;
                                pPropValue[ i ].Value >>= aSize;
                                //aBuGraSize.nA = aSize.Width;
                                //aBuGraSize.nB = aSize.Height;
                                DBG(printf("graphic size: %dx%d\n", int( aSize.Width ), int( aSize.Height )));
                            }
                        }
                    }
                }

                const char* pAutoNumType = GetAutoNumType( nNumberingType, bSDot, bPBehind, bPBoth );

                if( nLevel >= 0 ) {
                    if( !aGraphicURL.isEmpty() ) {
                        OUString sRelId = WriteImage( aGraphicURL );

                        mpFS->startElementNS( XML_a, XML_buBlip, FSEND );
                        mpFS->singleElementNS( XML_a, XML_blip, FSNS( XML_r, XML_embed ), USS( sRelId ), FSEND );
                        mpFS->endElementNS( XML_a, XML_buBlip );
                    } else {
                        if( nBulletRelSize && nBulletRelSize != 100 )
                            mpFS->singleElementNS( XML_a, XML_buSzPct,
                                                   XML_val, IS( std::min( (sal_Int32)25000, std::max( (sal_Int32)400000, 1000*( (sal_Int32)nBulletRelSize ) ) ) ), FSEND );
                        if( bHasFontDesc )
                            mpFS->singleElementNS( XML_a, XML_buFont,
                                                   XML_typeface, OUStringToOString( aFontDesc.Name, RTL_TEXTENCODING_UTF8 ).getStr(),
                                                   XML_charset, (aFontDesc.CharSet == awt::CharSet::SYMBOL) ? "2" : NULL,
                                                   FSEND );

                        if( pAutoNumType )
                            mpFS->singleElementNS( XML_a, XML_buAutoNum, XML_type, pAutoNumType, FSEND );
                        else {
                            aBulletChar = SubstituteBullet( aBulletChar, aFontDesc );
                            mpFS->singleElementNS( XML_a, XML_buChar, XML_char, USS( OUString( aBulletChar ) ), FSEND );
                        }
                    }
                }
            }
        }
    }
}

const char* DrawingML::GetAlignment( sal_Int32 nAlignment )
{
    const char* sAlignment = NULL;

    switch( nAlignment ) {
        case style::ParagraphAdjust_CENTER:
            sAlignment = "ctr";
            break;
        case style::ParagraphAdjust_RIGHT:
            sAlignment = "r";
            break;
        case style::ParagraphAdjust_BLOCK:
            sAlignment = "just";
            break;
        default:
            ;
    }

    return sAlignment;
}

void DrawingML::WriteLinespacing( LineSpacing& rSpacing )
{
    if( rSpacing.Mode == LineSpacingMode::PROP )
        mpFS->singleElementNS( XML_a, XML_spcPct,
                               XML_val, I32S( ((sal_Int32)rSpacing.Height)*1000 ),
                               FSEND );
    else
        mpFS->singleElementNS( XML_a, XML_spcPts,
                               XML_val, I32S( rSpacing.Height ),
                               FSEND );
}

void DrawingML::WriteParagraphProperties( Reference< XTextContent > rParagraph )
{
    Reference< XPropertySet > rXPropSet( rParagraph, UNO_QUERY );
    Reference< XPropertyState > rXPropState( rParagraph, UNO_QUERY );
    PropertyState eState;

    if( !rXPropSet.is() || !rXPropState.is() )
        return;

    sal_Int16 nLevel = -1;
    GET( nLevel, NumberingLevel );

    sal_Int16 nAlignment( style::ParagraphAdjust_LEFT );
    GET( nAlignment, ParaAdjust );

    sal_Bool bHasLinespacing = sal_False;
    LineSpacing aLineSpacing;
    if( GETAD( ParaLineSpacing ) )
        bHasLinespacing = ( mAny >>= aLineSpacing );

    if( nLevel != -1
        || nAlignment != style::ParagraphAdjust_LEFT
        || bHasLinespacing ) {
        mpFS->startElementNS( XML_a, XML_pPr,
                              XML_lvl, nLevel > 0 ? I32S( nLevel ) : NULL,
                              XML_marL, NULL,
                              XML_algn, GetAlignment( nAlignment ),
                              FSEND );

        if( bHasLinespacing ) {
            mpFS->startElementNS( XML_a, XML_lnSpc, FSEND );
            WriteLinespacing( aLineSpacing );
            mpFS->endElementNS( XML_a, XML_lnSpc );
        }

        WriteParagraphNumbering( rXPropSet, nLevel );

        mpFS->endElementNS( XML_a, XML_pPr );
    }
}

void DrawingML::WriteParagraph( Reference< XTextContent > rParagraph )
{
    Reference< XEnumerationAccess > access( rParagraph, UNO_QUERY );
    if( !access.is() )
        return;

    Reference< XEnumeration > enumeration( access->createEnumeration() );
    if( !enumeration.is() )
        return;

    mpFS->startElementNS( XML_a, XML_p, FSEND );

    sal_Bool bPropertiesWritten = sal_False;
    while( enumeration->hasMoreElements() ) {
        Reference< XTextRange > run;
        Any any ( enumeration->nextElement() );

        if (any >>= run) {
            if( !bPropertiesWritten ) {
                WriteParagraphProperties( rParagraph );
                bPropertiesWritten = sal_True;
            }
            WriteRun( run );
        }
    }
    mpFS->singleElementNS( XML_a, XML_endParaRPr, FSEND );

    mpFS->endElementNS( XML_a, XML_p );
}

void DrawingML::WriteText( Reference< XInterface > rXIface  )
{
    Reference< XText > xXText( rXIface, UNO_QUERY );
    Reference< XPropertySet > rXPropSet( rXIface, UNO_QUERY );

    if( !xXText.is() )
        return;

#define DEFLRINS 254
#define DEFTBINS 127
    sal_Int32 nLeft, nRight, nTop, nBottom;
    nLeft = nRight = DEFLRINS;
    nTop = nBottom = DEFTBINS;

    // top inset looks a bit different compared to ppt export
    // check if something related doesn't work as expected
    GET( nLeft, TextLeftDistance );
    GET( nRight, TextRightDistance );
    GET( nTop, TextUpperDistance );
    GET( nBottom, TextLowerDistance );

    TextVerticalAdjust eVerticalAlignment( TextVerticalAdjust_TOP );
    const char* sVerticalAlignment = NULL;
    GET( eVerticalAlignment, TextVerticalAdjust );
    switch( eVerticalAlignment ) {
        case TextVerticalAdjust_BOTTOM:
            sVerticalAlignment = "b";
            break;
        case TextVerticalAdjust_CENTER:
            sVerticalAlignment = "ctr";
            break;
        case TextVerticalAdjust_TOP:
        default:
            ;
    }

    const char* sWritingMode = NULL;
    sal_Bool bVertical = sal_False;
    if( GETA( TextWritingMode ) ) {
        WritingMode eMode;

        if( ( mAny >>= eMode ) && eMode == WritingMode_TB_RL ) {
            sWritingMode = "vert";
            bVertical = sal_True;
        }
    }

    if ( GETA( CustomShapeGeometry ) )
    {
        Sequence< PropertyValue > aProps;
        if ( mAny >>= aProps )
        {
            for ( sal_Int32 i = 0, nElems = aProps.getLength(); i < nElems; ++i )
            {
                sal_Int32 nTextRotateAngle = 0;
                if ( aProps[ i ].Name.equals( "TextPreRotateAngle" ) && ( aProps[ i ].Value >>= nTextRotateAngle ) )
                {
                    if ( nTextRotateAngle == -90 )
                    {
                        sWritingMode = "vert";
                        bVertical = sal_True;
                    }
                    break;
                }
            }
        }
    }

    TextHorizontalAdjust eHorizontalAlignment( TextHorizontalAdjust_CENTER );
    bool bHorizontalCenter = false;
    GET( eHorizontalAlignment, TextHorizontalAdjust );
    if( eHorizontalAlignment == TextHorizontalAdjust_CENTER )
        bHorizontalCenter = true;
    else if( bVertical && eHorizontalAlignment == TextHorizontalAdjust_LEFT )
        sVerticalAlignment = "b";

    sal_Bool bHasWrap = sal_False;
    sal_Bool bWrap = sal_False;
    if( GETA( TextWordWrap ) ) {
        mAny >>= bWrap;
        bHasWrap = sal_True;
    }

    mpFS->singleElementNS( XML_a, XML_bodyPr,
                           XML_wrap, bHasWrap && !bWrap ? "none" : NULL,
                           XML_lIns, (nLeft != DEFLRINS) ? IS( MM100toEMU( nLeft ) ) : NULL,
                           XML_rIns, (nRight != DEFLRINS) ? IS( MM100toEMU( nRight ) ) : NULL,
                           XML_tIns, (nTop != DEFTBINS) ? IS( MM100toEMU( nTop ) ) : NULL,
                           XML_bIns, (nBottom != DEFTBINS) ? IS( MM100toEMU( nBottom ) ) : NULL,
                           XML_anchor, sVerticalAlignment,
                           XML_anchorCtr, bHorizontalCenter ? "1" : NULL,
                           XML_vert, sWritingMode,
                           FSEND );

    Reference< XEnumerationAccess > access( xXText, UNO_QUERY );
    if( !access.is() )
        return;

    Reference< XEnumeration > enumeration( access->createEnumeration() );
    if( !enumeration.is() )
        return;

    while( enumeration->hasMoreElements() ) {
        Reference< XTextContent > paragraph;
        Any any ( enumeration->nextElement() );

        if( any >>= paragraph)
            WriteParagraph( paragraph );
    }

}

void DrawingML::WritePresetShape( const char* pShape )
{
    mpFS->startElementNS( XML_a, XML_prstGeom,
                          XML_prst, pShape,
                          FSEND );
    mpFS->singleElementNS( XML_a, XML_avLst, FSEND );
    mpFS->endElementNS(  XML_a, XML_prstGeom );
}

void DrawingML::WritePresetShape( const char* pShape, MSO_SPT eShapeType, sal_Bool bPredefinedHandlesUsed, sal_Int32 nAdjustmentsWhichNeedsToBeConverted, const PropertyValue& rProp )
{
    mpFS->startElementNS( XML_a, XML_prstGeom,
                          XML_prst, pShape,
                          FSEND );
    mpFS->startElementNS( XML_a, XML_avLst, FSEND );

    Sequence< drawing::EnhancedCustomShapeAdjustmentValue > aAdjustmentSeq;
    if ( ( rProp.Value >>= aAdjustmentSeq )
         && eShapeType != mso_sptActionButtonForwardNext  // we have adjustments values for these type of shape, but MSO doesn't like them
         && eShapeType != mso_sptActionButtonBackPrevious // so they are now disabled
        ) {
        DBG(printf("adj seq len: %d\n", int( aAdjustmentSeq.getLength() )));
        if ( bPredefinedHandlesUsed )
            EscherPropertyContainer::LookForPolarHandles( eShapeType, nAdjustmentsWhichNeedsToBeConverted );

        sal_Int32 nValue, nLength = aAdjustmentSeq.getLength();
        for( sal_Int32 i=0; i < nLength; i++ )
            if( EscherPropertyContainer::GetAdjustmentValue( aAdjustmentSeq[ i ], i, nAdjustmentsWhichNeedsToBeConverted, nValue ) )
                mpFS->singleElementNS( XML_a, XML_gd,
                                       XML_name, aAdjustmentSeq[ i ].Name.getLength() > 0 ? USS(aAdjustmentSeq[ i ].Name) : (nLength > 1 ? OString( "adj" + OString::valueOf( i + 1 ) ).getStr() : "adj"),
                                       XML_fmla, OString("val " + OString::valueOf( nValue )).getStr(),
                                       FSEND );
    }

    mpFS->endElementNS( XML_a, XML_avLst );
    mpFS->endElementNS(  XML_a, XML_prstGeom );
}

void DrawingML::WritePolyPolygon( const PolyPolygon& rPolyPolygon )
{
    if( rPolyPolygon.Count() < 1 )
        return;

    mpFS->startElementNS( XML_a, XML_custGeom, FSEND );
    mpFS->singleElementNS( XML_a, XML_avLst, FSEND );
    mpFS->singleElementNS( XML_a, XML_gdLst, FSEND );
    mpFS->singleElementNS( XML_a, XML_ahLst, FSEND );
    mpFS->singleElementNS( XML_a, XML_rect,
                           XML_l, "0",
                           XML_t, "0",
                           XML_r, "r",
                           XML_b, "b",
                           FSEND );

    mpFS->startElementNS( XML_a, XML_pathLst, FSEND );

    for( sal_uInt16 i = 0; i < rPolyPolygon.Count(); i ++ ) {

        const Polygon& rPoly = rPolyPolygon[ i ];
        Rectangle aRect( rPoly.GetBoundRect() );
        sal_Bool bBezier = sal_False;

        mpFS->startElementNS( XML_a, XML_path,
                              XML_w, I64S( aRect.GetWidth() ),
                              XML_h, I64S( aRect.GetHeight() ),
                              FSEND );

        if( rPoly.GetSize() > 0 )
        {
            mpFS->startElementNS( XML_a, XML_moveTo, FSEND );

            mpFS->singleElementNS( XML_a, XML_pt,
                                   XML_x, I64S( rPoly[ 0 ].X() - aRect.Left() ),
                                   XML_y, I64S( rPoly[ 0 ].Y() - aRect.Top() ),
                                   FSEND );

            mpFS->endElementNS( XML_a, XML_moveTo );
        }

        for( sal_uInt16 j = 1; j < rPoly.GetSize(); j ++ )
        {
            enum PolyFlags flags = rPoly.GetFlags(j);
            if( flags == POLY_CONTROL && !bBezier )
            {
                mpFS->startElementNS( XML_a, XML_cubicBezTo, FSEND );
                bBezier = sal_True;
            }
            else if( flags == POLY_NORMAL && !bBezier )
                mpFS->startElementNS( XML_a, XML_lnTo, FSEND );

            mpFS->singleElementNS( XML_a, XML_pt,
                                   XML_x, I64S( rPoly[j].X() - aRect.Left() ),
                                   XML_y, I64S( rPoly[j].Y() - aRect.Top() ),
                                   FSEND );

            if( ( flags == POLY_NORMAL || flags == POLY_SYMMTR ) && bBezier )
            {
                mpFS->endElementNS( XML_a, XML_cubicBezTo );
                bBezier = sal_False;
            }
            else if( flags == POLY_NORMAL && !bBezier )
                mpFS->endElementNS( XML_a, XML_lnTo );
            else if( bBezier && ( j % 3 ) == 0 )
            {
                // //a:cubicBezTo can only contain 3 //a:pt elements, so we
                // need to break things up...
                mpFS->endElementNS( XML_a, XML_cubicBezTo );
                mpFS->startElementNS( XML_a, XML_cubicBezTo, FSEND );
            }
        }

        mpFS->endElementNS( XML_a, XML_path );
    }

    mpFS->endElementNS( XML_a, XML_pathLst );

    mpFS->endElementNS(  XML_a, XML_custGeom );
}

void DrawingML::WriteConnectorConnections( EscherConnectorListEntry& rConnectorEntry, sal_Int32 nStartID, sal_Int32 nEndID )
{
    if( nStartID != -1 )
        mpFS->singleElementNS( XML_a, XML_stCxn,
                               XML_id, I32S( nStartID ),
                               XML_idx, I64S( rConnectorEntry.GetConnectorRule( sal_True ) ),
                               FSEND );
    if( nEndID != -1 )
        mpFS->singleElementNS( XML_a, XML_endCxn,
                               XML_id, I32S( nEndID ),
                               XML_idx, I64S( rConnectorEntry.GetConnectorRule( sal_False ) ),
                               FSEND );
}

sal_Unicode DrawingML::SubstituteBullet( sal_Unicode cBulletId, ::com::sun::star::awt::FontDescriptor& rFontDesc )
{
    if ( IsStarSymbol(rFontDesc.Name) )
    {
        rtl_TextEncoding eCharSet = rFontDesc.CharSet;
        cBulletId = msfilter::util::bestFitOpenSymbolToMSFont(cBulletId, eCharSet, rFontDesc.Name);
        rFontDesc.CharSet = eCharSet;
    }

    return cBulletId;
}

sax_fastparser::FSHelperPtr DrawingML::CreateOutputStream (
    const OUString& sFullStream,
    const OUString& sRelativeStream,
    const Reference< XOutputStream >& xParentRelation,
    const char* sContentType,
    const char* sRelationshipType,
    OUString* pRelationshipId )
{
    OUString sRelationshipId;
    if (xParentRelation.is())
        sRelationshipId = GetFB()->addRelation( xParentRelation, OUString::createFromAscii( sRelationshipType), sRelativeStream );
    else
        sRelationshipId = GetFB()->addRelation( OUString::createFromAscii( sRelationshipType ), sRelativeStream );

    if( pRelationshipId )
        *pRelationshipId = sRelationshipId;

    sax_fastparser::FSHelperPtr p = GetFB()->openFragmentStreamWithSerializer( sFullStream, OUString::createFromAscii( sContentType ) );

    return p;
}

void DrawingML::WriteFill( Reference< XPropertySet > xPropSet )
{
    if ( !GetProperty( xPropSet, "FillStyle" ) )
        return;
    FillStyle aFillStyle( FillStyle_NONE );
    xPropSet->getPropertyValue( "FillStyle" ) >>= aFillStyle;

    if( aFillStyle == FillStyle_HATCH )
        return;

    if ( aFillStyle == FillStyle_SOLID && GetProperty( xPropSet, "FillTransparence" ) )
    {
        // map full transparent background to no fill
        sal_Int16 nVal = 0;
        xPropSet->getPropertyValue( "FillTransparence" ) >>= nVal;
        if ( nVal == 100 )
            aFillStyle = FillStyle_NONE;
    }

    switch( aFillStyle )
    {
    case FillStyle_SOLID :
        WriteSolidFill( xPropSet );
        break;
    case FillStyle_GRADIENT :
        WriteGradientFill( xPropSet );
        break;
    case FillStyle_BITMAP :
        WriteBlipFill( xPropSet, "FillBitmapURL" );
        break;
    case FillStyle_NONE:
        mpFS->singleElementNS( XML_a, XML_noFill, FSEND );
        break;
    default:
        ;
    }

    return;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
