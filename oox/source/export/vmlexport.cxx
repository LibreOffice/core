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

#include <oox/export/vmlexport.hxx>

#include <oox/token/tokens.hxx>

#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>

#include <tools/stream.hxx>
#include <svx/svdotext.hxx>
#include <vcl/cvtgrf.hxx>
#include <filter/msfilter/msdffimp.hxx>

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>

#include <cstdio>


using namespace sax_fastparser;
using namespace oox::vml;
using namespace com::sun::star;

VMLExport::VMLExport( ::sax_fastparser::FSHelperPtr pSerializer, VMLTextExport* pTextExport )
    : EscherEx( EscherExGlobalRef(new EscherExGlobal(0)), 0 )
    , m_pSerializer( pSerializer )
    , m_pTextExport( pTextExport )
    , m_eHOri( 0 )
    , m_eVOri( 0 )
    , m_eHRel( 0 )
    , m_eVRel( 0 )
    , m_pNdTopLeft( 0 )
    , m_pSdrObject( 0 )
    , m_pShapeAttrList( NULL )
    , m_nShapeType( ESCHER_ShpInst_Nil )
    , m_nShapeFlags(0)
    , m_pShapeStyle( new OStringBuffer( 200 ) )
    , m_pShapeTypeWritten( new bool[ ESCHER_ShpInst_COUNT ] )
{
    mnGroupLevel = 1;
    memset( m_pShapeTypeWritten, 0, ESCHER_ShpInst_COUNT * sizeof( bool ) );
}

void VMLExport::SetFS( ::sax_fastparser::FSHelperPtr pSerializer )
{
    m_pSerializer = pSerializer;
}

VMLExport::~VMLExport()
{
    delete mpOutStrm, mpOutStrm = NULL;
    delete m_pShapeStyle, m_pShapeStyle = NULL;
    delete[] m_pShapeTypeWritten, m_pShapeTypeWritten = NULL;
}

void VMLExport::OpenContainer( sal_uInt16 nEscherContainer, int nRecInstance )
{
    EscherEx::OpenContainer( nEscherContainer, nRecInstance );

    if ( nEscherContainer == ESCHER_SpContainer )
    {
        // opening a shape container
#if OSL_DEBUG_LEVEL > 0
        if ( m_nShapeType != ESCHER_ShpInst_Nil )
            fprintf( stderr, "Warning!  VMLExport::OpenContainer(): opening shape inside a shape.\n" );
#endif
        m_nShapeType = ESCHER_ShpInst_Nil;
        m_pShapeAttrList = m_pSerializer->createAttrList();

        if ( !m_pShapeStyle->isEmpty() )
            m_pShapeStyle->makeStringAndClear();

        m_pShapeStyle->ensureCapacity( 200 );

        // postpone the output so that we are able to write even the elements
        // that we learn inside Commit()
        m_pSerializer->mark();
    }
}

void VMLExport::CloseContainer()
{
    if ( mRecTypes.back() == ESCHER_SpContainer )
    {
        // write the shape now when we have all the info
        sal_Int32 nShapeElement = StartShape();

        m_pSerializer->mergeTopMarks();

        EndShape( nShapeElement );

        // cleanup
        m_nShapeType = ESCHER_ShpInst_Nil;
        m_pShapeAttrList = NULL;
    }

    EscherEx::CloseContainer();
}

sal_uInt32 VMLExport::EnterGroup( const OUString& rShapeName, const Rectangle* pRect )
{
    sal_uInt32 nShapeId = GenerateShapeId();

    OStringBuffer aStyle( 200 );
    FastAttributeList *pAttrList = m_pSerializer->createAttrList();

    pAttrList->add( XML_id, ShapeIdString( nShapeId ) );

    if ( rShapeName.getLength() )
        pAttrList->add( XML_alt, OUStringToOString( rShapeName, RTL_TEXTENCODING_UTF8 ) );

    // style
    if ( pRect )
        AddRectangleDimensions( aStyle, *pRect );

    if ( !aStyle.isEmpty() )
        pAttrList->add( XML_style, aStyle.makeStringAndClear() );

    // coordorigin/coordsize
    if ( pRect && ( mnGroupLevel == 1 ) )
    {
        pAttrList->add( XML_coordorigin,
                OStringBuffer( 20 ).append( sal_Int32( pRect->Left() ) )
                .append( "," ).append( sal_Int32( pRect->Top() ) )
                .makeStringAndClear() );

        pAttrList->add( XML_coordsize,
                OStringBuffer( 20 ).append( sal_Int32( pRect->Right() ) - sal_Int32( pRect->Left() ) )
                .append( "," ).append( sal_Int32( pRect->Bottom() ) - sal_Int32( pRect->Top() ) )
                .makeStringAndClear() );
    }

    m_pSerializer->startElementNS( XML_v, XML_group, XFastAttributeListRef( pAttrList ) );

    mnGroupLevel++;
    return nShapeId;
}

void VMLExport::LeaveGroup()
{
    --mnGroupLevel;
    m_pSerializer->endElementNS( XML_v, XML_group );
}

void VMLExport::AddShape( sal_uInt32 nShapeType, sal_uInt32 nShapeFlags, sal_uInt32 nShapeId )
{
    m_nShapeType = nShapeType;
    m_nShapeFlags = nShapeFlags;

    m_pShapeAttrList->add( XML_id, ShapeIdString( nShapeId ) );
}

static void impl_AddArrowHead( sax_fastparser::FastAttributeList *pAttrList, sal_Int32 nElement, sal_uInt32 nValue )
{
    if ( !pAttrList )
        return;

    const char *pArrowHead = NULL;
    switch ( nValue )
    {
        case ESCHER_LineNoEnd:           pArrowHead = "none"; break;
        case ESCHER_LineArrowEnd:        pArrowHead = "block"; break;
        case ESCHER_LineArrowStealthEnd: pArrowHead = "classic"; break;
        case ESCHER_LineArrowDiamondEnd: pArrowHead = "diamond"; break;
        case ESCHER_LineArrowOvalEnd:    pArrowHead = "oval"; break;
        case ESCHER_LineArrowOpenEnd:    pArrowHead = "open"; break;
    }

    if ( pArrowHead )
        pAttrList->add( nElement, pArrowHead );
}

static void impl_AddArrowLength( sax_fastparser::FastAttributeList *pAttrList, sal_Int32 nElement, sal_uInt32 nValue )
{
    if ( !pAttrList )
        return;

    const char *pArrowLength = NULL;
    switch ( nValue )
    {
        case ESCHER_LineShortArrow:     pArrowLength = "short"; break;
        case ESCHER_LineMediumLenArrow: pArrowLength = "medium"; break;
        case ESCHER_LineLongArrow:      pArrowLength = "long"; break;
    }

    if ( pArrowLength )
        pAttrList->add( nElement, pArrowLength );
}

static void impl_AddArrowWidth( sax_fastparser::FastAttributeList *pAttrList, sal_Int32 nElement, sal_uInt32 nValue )
{
    if ( !pAttrList )
        return;

    const char *pArrowWidth = NULL;
    switch ( nValue )
    {
        case ESCHER_LineNarrowArrow:      pArrowWidth = "narrow"; break;
        case ESCHER_LineMediumWidthArrow: pArrowWidth = "medium"; break;
        case ESCHER_LineWideArrow:        pArrowWidth = "wide"; break;
    }

    if ( pArrowWidth )
        pAttrList->add( nElement, pArrowWidth );
}

static void impl_AddBool( sax_fastparser::FastAttributeList *pAttrList, sal_Int32 nElement, bool bValue )
{
    if ( !pAttrList )
        return;

    pAttrList->add( nElement, bValue? "t": "f" );
}

static void impl_AddColor( sax_fastparser::FastAttributeList *pAttrList, sal_Int32 nElement, sal_uInt32 nColor )
{
#if OSL_DEBUG_LEVEL > 0
    if ( nColor & 0xFF000000 )
        fprintf( stderr, "TODO: this is not a RGB value!\n" );
#endif

    if ( !pAttrList || ( nColor & 0xFF000000 ) )
        return;

    nColor = ( ( nColor & 0xFF ) << 16 ) + ( nColor & 0xFF00 ) + ( ( nColor & 0xFF0000 ) >> 16 );

    const char *pColor = NULL;
    char pRgbColor[10];
    switch ( nColor )
    {
        case 0x000000: pColor = "black"; break;
        case 0xC0C0C0: pColor = "silver"; break;
        case 0x808080: pColor = "gray"; break;
        case 0xFFFFFF: pColor = "white"; break;
        case 0x800000: pColor = "maroon"; break;
        case 0xFF0000: pColor = "red"; break;
        case 0x800080: pColor = "purple"; break;
        case 0xFF00FF: pColor = "fuchsia"; break;
        case 0x008000: pColor = "green"; break;
        case 0x00FF00: pColor = "lime"; break;
        case 0x808000: pColor = "olive"; break;
        case 0xFFFF00: pColor = "yellow"; break;
        case 0x000080: pColor = "navy"; break;
        case 0x0000FF: pColor = "blue"; break;
        case 0x008080: pColor = "teal"; break;
        case 0x00FFFF: pColor = "aqua"; break;
        default:
            {
                snprintf( pRgbColor, sizeof( pRgbColor ), "#%06x", static_cast< unsigned int >( nColor ) ); // not too handy to use OString::valueOf() here :-(
                pColor = pRgbColor;
            }
            break;
    }

    pAttrList->add( nElement, pColor );
}

static void impl_AddInt( sax_fastparser::FastAttributeList *pAttrList, sal_Int32 nElement, sal_uInt32 nValue )
{
    if ( !pAttrList )
        return;

    pAttrList->add( nElement, OString::number( nValue ).getStr() );
}

inline sal_uInt16 impl_GetUInt16( const sal_uInt8* &pVal )
{
    sal_uInt16 nRet = *pVal++;
    nRet += ( *pVal++ ) << 8;
    return nRet;
}

inline sal_Int32 impl_GetPointComponent( const sal_uInt8* &pVal, sal_uInt16 nPointSize )
{
    sal_Int32 nRet = 0;
    if ( ( nPointSize == 0xfff0 ) || ( nPointSize == 4 ) )
    {
        sal_uInt16 nUnsigned = *pVal++;
        nUnsigned += ( *pVal++ ) << 8;

        nRet = sal_Int16( nUnsigned );
    }
    else if ( nPointSize == 8 )
    {
        sal_uInt32 nUnsigned = *pVal++;
        nUnsigned += ( *pVal++ ) << 8;
        nUnsigned += ( *pVal++ ) << 16;
        nUnsigned += ( *pVal++ ) << 24;

        nRet = nUnsigned;
    }

    return nRet;
}

void VMLExport::Commit( EscherPropertyContainer& rProps, const Rectangle& rRect )
{
    if ( m_nShapeType == ESCHER_ShpInst_Nil )
        return;

    // postpone the output of the embedded elements so that they are written
    // inside the shapes
    m_pSerializer->mark();

    // dimensions
    if ( m_nShapeType == ESCHER_ShpInst_Line )
        AddLineDimensions( rRect );
    else
    {
        Rectangle aRect(rRect);
        if (m_pNdTopLeft)
        {
            aRect = m_pSdrObject->GetSnapRect();
            aRect -= *m_pNdTopLeft;
        }
        AddRectangleDimensions( *m_pShapeStyle, aRect );
    }

    // properties
    bool bAlreadyWritten[ 0xFFF ];
    memset( bAlreadyWritten, 0, sizeof( bAlreadyWritten ) );
    const EscherProperties &rOpts = rProps.GetOpts();
    for ( EscherProperties::const_iterator it = rOpts.begin(); it != rOpts.end(); ++it )
    {
        sal_uInt16 nId = ( it->nPropId & 0x0FFF );

        if ( bAlreadyWritten[ nId ] )
            continue;

        switch ( nId )
        {
            case ESCHER_Prop_WrapText: // 133
                {
                    const char *pWrapType = NULL;
                    switch ( it->nPropValue )
                    {
                        case ESCHER_WrapSquare:
                        case ESCHER_WrapByPoints:  pWrapType = "square"; break; // these two are equivalent according to the docu
                        case ESCHER_WrapNone:      pWrapType = "none"; break;
                        case ESCHER_WrapTopBottom: pWrapType = "topAndBottom"; break;
                        case ESCHER_WrapThrough:   pWrapType = "through"; break;
                    }
                    if ( pWrapType )
                        m_pSerializer->singleElementNS( XML_v, XML_wrap,
                                FSNS( XML_v, XML_type ), pWrapType,
                                FSEND );
                }
                bAlreadyWritten[ ESCHER_Prop_WrapText ] = true;
                break;

            // coordorigin
            case ESCHER_Prop_geoLeft: // 320
            case ESCHER_Prop_geoTop: // 321
                {
                    sal_uInt32 nLeft = 0, nTop = 0;

                    if ( nId == ESCHER_Prop_geoLeft )
                    {
                        nLeft = it->nPropValue;
                        rProps.GetOpt( ESCHER_Prop_geoTop, nTop );
                    }
                    else
                    {
                        nTop = it->nPropValue;
                        rProps.GetOpt( ESCHER_Prop_geoLeft, nLeft );
                    }

                    m_pShapeAttrList->add( XML_coordorigin,
                            OStringBuffer( 20 ).append( sal_Int32( nLeft ) )
                            .append( "," ).append( sal_Int32( nTop ) )
                            .makeStringAndClear() );
                }
                bAlreadyWritten[ ESCHER_Prop_geoLeft ] = true;
                bAlreadyWritten[ ESCHER_Prop_geoTop ] = true;
                break;

            // coordsize
            case ESCHER_Prop_geoRight: // 322
            case ESCHER_Prop_geoBottom: // 323
                {
                    sal_uInt32 nLeft = 0, nRight = 0, nTop = 0, nBottom = 0;
                    rProps.GetOpt( ESCHER_Prop_geoLeft, nLeft );
                    rProps.GetOpt( ESCHER_Prop_geoTop, nTop );

                    if ( nId == ESCHER_Prop_geoRight )
                    {
                        nRight = it->nPropValue;
                        rProps.GetOpt( ESCHER_Prop_geoBottom, nBottom );
                    }
                    else
                    {
                        nBottom = it->nPropValue;
                        rProps.GetOpt( ESCHER_Prop_geoRight, nRight );
                    }

                    m_pShapeAttrList->add( XML_coordsize,
                            OStringBuffer( 20 ).append( sal_Int32( nRight ) - sal_Int32( nLeft ) )
                            .append( "," ).append( sal_Int32( nBottom ) - sal_Int32( nTop ) )
                            .makeStringAndClear() );
                }
                bAlreadyWritten[ ESCHER_Prop_geoRight ] = true;
                bAlreadyWritten[ ESCHER_Prop_geoBottom ] = true;
                break;

            case ESCHER_Prop_pVertices: // 325
            case ESCHER_Prop_pSegmentInfo: // 326
                {
                    EscherPropSortStruct aVertices;
                    EscherPropSortStruct aSegments;

                    if ( rProps.GetOpt( ESCHER_Prop_pVertices, aVertices ) &&
                         rProps.GetOpt( ESCHER_Prop_pSegmentInfo, aSegments ) )
                    {
                        const sal_uInt8 *pVerticesIt = aVertices.pBuf + 6;
                        const sal_uInt8 *pSegmentIt = aSegments.pBuf;
                        OStringBuffer aPath( 512 );

                        sal_uInt16 nPointSize = aVertices.pBuf[4] + ( aVertices.pBuf[5] << 8 );

                        // number of segments
                        sal_uInt16 nSegments = impl_GetUInt16( pSegmentIt );
                        pSegmentIt += 4;

                        for ( ; nSegments; --nSegments )
                        {
                            sal_uInt16 nSeg = impl_GetUInt16( pSegmentIt );
                            switch ( nSeg )
                            {
                                case 0x4000: // moveto
                                    {
                                        sal_Int32 nX = impl_GetPointComponent( pVerticesIt, nPointSize );
                                        sal_Int32 nY = impl_GetPointComponent( pVerticesIt, nPointSize );
                                        aPath.append( "m" ).append( nX ).append( "," ).append( nY );
                                    }
                                    break;
                                case 0xb300:
                                case 0xac00:
                                    break;
                                case 0x0001: // lineto
                                    {
                                        sal_Int32 nX = impl_GetPointComponent( pVerticesIt, nPointSize );
                                        sal_Int32 nY = impl_GetPointComponent( pVerticesIt, nPointSize );
                                        aPath.append( "l" ).append( nX ).append( "," ).append( nY );
                                    }
                                    break;
                                case 0x2001: // curveto
                                    {
                                        sal_Int32 nX1 = impl_GetPointComponent( pVerticesIt, nPointSize );
                                        sal_Int32 nY1 = impl_GetPointComponent( pVerticesIt, nPointSize );
                                        sal_Int32 nX2 = impl_GetPointComponent( pVerticesIt, nPointSize );
                                        sal_Int32 nY2 = impl_GetPointComponent( pVerticesIt, nPointSize );
                                        sal_Int32 nX3 = impl_GetPointComponent( pVerticesIt, nPointSize );
                                        sal_Int32 nY3 = impl_GetPointComponent( pVerticesIt, nPointSize );
                                        aPath.append( "c" ).append( nX1 ).append( "," ).append( nY1 ).append( "," )
                                            .append( nX2 ).append( "," ).append( nY2 ).append( "," )
                                            .append( nX3 ).append( "," ).append( nY3 );
                                    }
                                    break;
                                case 0xaa00: // nofill
                                    aPath.append( "nf" );
                                    break;
                                case 0xab00: // nostroke
                                    aPath.append( "ns" );
                                    break;
                                case 0x6001: // close
                                    aPath.append( "x" );
                                    break;
                                case 0x8000: // end
                                    aPath.append( "e" );
                                    break;
                                default:
                                    // See EscherPropertyContainer::CreateCustomShapeProperties, by default nSeg is simply the number of points.
                                    for (int i = 0; i < nSeg; ++i)
                                    {
                                        sal_Int32 nX = impl_GetPointComponent(pVerticesIt, nPointSize);
                                        sal_Int32 nY = impl_GetPointComponent(pVerticesIt, nPointSize);
                                        aPath.append("l").append(nX).append(",").append(nY);
                                    }
                                    break;
                            }
                        }

                        if ( !aPath.isEmpty() )
                            m_pShapeAttrList->add( XML_path, aPath.getStr() );
                    }
#if OSL_DEBUG_LEVEL > 0
                    else
                        fprintf( stderr, "TODO: unhandled shape path, missing either pVertices or pSegmentInfo.\n" );
#endif
                }
                bAlreadyWritten[ ESCHER_Prop_pVertices ] = true;
                bAlreadyWritten[ ESCHER_Prop_pSegmentInfo ] = true;
                break;

            case ESCHER_Prop_fillType: // 384
            case ESCHER_Prop_fillColor: // 385
            case ESCHER_Prop_fillBackColor: // 387
            case ESCHER_Prop_fillBlip: // 390
            case ESCHER_Prop_fNoFillHitTest: // 447
            case ESCHER_Prop_fillOpacity: // 386
                {
                    sal_uInt32 nValue;
                    sax_fastparser::FastAttributeList *pAttrList = m_pSerializer->createAttrList();

                    if ( rProps.GetOpt( ESCHER_Prop_fillType, nValue ) )
                    {
                        const char *pFillType = NULL;
                        switch ( nValue )
                        {
                            case ESCHER_FillSolid:       pFillType = "solid"; break;
                            // TODO case ESCHER_FillPattern:     pFillType = ""; break;
                            case ESCHER_FillTexture:     pFillType = "tile"; break;
                            // TODO case ESCHER_FillPicture:     pFillType = ""; break;
                            // TODO case ESCHER_FillShade:       pFillType = ""; break;
                            // TODO case ESCHER_FillShadeCenter: pFillType = ""; break;
                            // TODO case ESCHER_FillShadeShape:  pFillType = ""; break;
                            // TODO case ESCHER_FillShadeScale:  pFillType = ""; break;
                            // TODO case ESCHER_FillShadeTitle:  pFillType = ""; break;
                            // TODO case ESCHER_FillBackground:  pFillType = ""; break;
                            default:
#if OSL_DEBUG_LEVEL > 0
                                fprintf( stderr, "TODO: unhandled fill type\n" );
#endif
                                break;
                        }
                        if ( pFillType )
                            pAttrList->add( XML_type, pFillType );
                    }
                    else
                        pAttrList->add( XML_on, "false" );

                    if ( rProps.GetOpt( ESCHER_Prop_fillColor, nValue ) )
                        impl_AddColor( m_pShapeAttrList, XML_fillcolor, nValue );

                    if ( rProps.GetOpt( ESCHER_Prop_fillBackColor, nValue ) )
                        impl_AddColor( pAttrList, XML_color2, nValue );

                    EscherPropSortStruct aStruct;
                    if ( rProps.GetOpt( ESCHER_Prop_fillBlip, aStruct ) && m_pTextExport)
                    {
                        SvMemoryStream aStream;
                        int nHeaderSize = 25; // The first bytes are WW8-specific, we're only interested in the PNG
                        aStream.Write(aStruct.pBuf + nHeaderSize, aStruct.nPropSize - nHeaderSize);
                        aStream.Seek(0);
                        Graphic aGraphic;
                        GraphicConverter::Import(aStream, aGraphic, CVT_PNG);
                        OUString aImageId = m_pTextExport->GetDrawingML().WriteImage( aGraphic );
                        pAttrList->add(FSNS(XML_r, XML_id), OUStringToOString(aImageId, RTL_TEXTENCODING_UTF8));
                    }

                    if ( rProps.GetOpt( ESCHER_Prop_fNoFillHitTest, nValue ) )
                        impl_AddBool( pAttrList, XML_detectmouseclick, nValue );

                    if (rProps.GetOpt(ESCHER_Prop_fillOpacity, nValue))
                        // Partly undo the transformation at the end of EscherPropertyContainer::CreateFillProperties(): VML opacity is 0..1.
                        pAttrList->add(XML_opacity, OString::number(double((nValue * 100) >> 16) / 100));

                    m_pSerializer->singleElementNS( XML_v, XML_fill, XFastAttributeListRef( pAttrList ) );
                }
                bAlreadyWritten[ ESCHER_Prop_fillType ] = true;
                bAlreadyWritten[ ESCHER_Prop_fillColor ] = true;
                bAlreadyWritten[ ESCHER_Prop_fillBackColor ] = true;
                bAlreadyWritten[ ESCHER_Prop_fillBlip ] = true;
                bAlreadyWritten[ ESCHER_Prop_fNoFillHitTest ] = true;
                bAlreadyWritten[ ESCHER_Prop_fillOpacity ] = true;
                break;

            case ESCHER_Prop_lineColor: // 448
            case ESCHER_Prop_lineWidth: // 459
            case ESCHER_Prop_lineDashing: // 462
            case ESCHER_Prop_lineStartArrowhead: // 464
            case ESCHER_Prop_lineEndArrowhead: // 465
            case ESCHER_Prop_lineStartArrowWidth: // 466
            case ESCHER_Prop_lineStartArrowLength: // 467
            case ESCHER_Prop_lineEndArrowWidth: // 468
            case ESCHER_Prop_lineEndArrowLength: // 469
            case ESCHER_Prop_lineJoinStyle: // 470
            case ESCHER_Prop_lineEndCapStyle: // 471
                {
                    sal_uInt32 nValue;
                    sax_fastparser::FastAttributeList *pAttrList = m_pSerializer->createAttrList();

                    if ( rProps.GetOpt( ESCHER_Prop_lineColor, nValue ) )
                        impl_AddColor( pAttrList, XML_color, nValue );

                    if ( rProps.GetOpt( ESCHER_Prop_lineWidth, nValue ) )
                        impl_AddInt( pAttrList, XML_weight, nValue );

                    if ( rProps.GetOpt( ESCHER_Prop_lineDashing, nValue ) )
                    {
                        const char *pDashStyle = NULL;
                        switch ( nValue )
                        {
                            case ESCHER_LineSolid:             pDashStyle = "solid"; break;
                            case ESCHER_LineDashSys:           pDashStyle = "shortdash"; break;
                            case ESCHER_LineDotSys:            pDashStyle = "shortdot"; break;
                            case ESCHER_LineDashDotSys:        pDashStyle = "shortdashdot"; break;
                            case ESCHER_LineDashDotDotSys:     pDashStyle = "shortdashdotdot"; break;
                            case ESCHER_LineDotGEL:            pDashStyle = "dot"; break;
                            case ESCHER_LineDashGEL:           pDashStyle = "dash"; break;
                            case ESCHER_LineLongDashGEL:       pDashStyle = "longdash"; break;
                            case ESCHER_LineDashDotGEL:        pDashStyle = "dashdot"; break;
                            case ESCHER_LineLongDashDotGEL:    pDashStyle = "longdashdot"; break;
                            case ESCHER_LineLongDashDotDotGEL: pDashStyle = "longdashdotdot"; break;
                        }
                        if ( pDashStyle )
                            pAttrList->add( XML_dashstyle, pDashStyle );
                    }

                    if ( rProps.GetOpt( ESCHER_Prop_lineStartArrowhead, nValue ) )
                        impl_AddArrowHead( pAttrList, XML_startarrow, nValue );

                    if ( rProps.GetOpt( ESCHER_Prop_lineEndArrowhead, nValue ) )
                        impl_AddArrowHead( pAttrList, XML_endarrow, nValue );

                    if ( rProps.GetOpt( ESCHER_Prop_lineStartArrowWidth, nValue ) )
                        impl_AddArrowWidth( pAttrList, XML_startarrowwidth, nValue );

                    if ( rProps.GetOpt( ESCHER_Prop_lineStartArrowLength, nValue ) )
                        impl_AddArrowLength( pAttrList, XML_startarrowlength, nValue );

                    if ( rProps.GetOpt( ESCHER_Prop_lineEndArrowWidth, nValue ) )
                        impl_AddArrowWidth( pAttrList, XML_endarrowwidth, nValue );

                    if ( rProps.GetOpt( ESCHER_Prop_lineEndArrowLength, nValue ) )
                        impl_AddArrowLength( pAttrList, XML_endarrowlength, nValue );

                    if ( rProps.GetOpt( ESCHER_Prop_lineJoinStyle, nValue ) )
                    {
                        const char *pJoinStyle = NULL;
                        switch ( nValue )
                        {
                            case ESCHER_LineJoinBevel: pJoinStyle = "bevel"; break;
                            case ESCHER_LineJoinMiter: pJoinStyle = "miter"; break;
                            case ESCHER_LineJoinRound: pJoinStyle = "round"; break;
                        }
                        if ( pJoinStyle )
                            pAttrList->add( XML_joinstyle, pJoinStyle );
                    }

                    if ( rProps.GetOpt( ESCHER_Prop_lineEndCapStyle, nValue ) )
                    {
                        const char *pEndCap = NULL;
                        switch ( nValue )
                        {
                            case ESCHER_LineEndCapRound:  pEndCap = "round"; break;
                            case ESCHER_LineEndCapSquare: pEndCap = "square"; break;
                            case ESCHER_LineEndCapFlat:   pEndCap = "flat"; break;
                        }
                        if ( pEndCap )
                            pAttrList->add( XML_endcap, pEndCap );
                    }

                    m_pSerializer->singleElementNS( XML_v, XML_stroke, XFastAttributeListRef( pAttrList ) );
                }
                bAlreadyWritten[ ESCHER_Prop_lineColor ] = true;
                bAlreadyWritten[ ESCHER_Prop_lineWidth ] = true;
                bAlreadyWritten[ ESCHER_Prop_lineDashing ] = true;
                bAlreadyWritten[ ESCHER_Prop_lineStartArrowhead ] = true;
                bAlreadyWritten[ ESCHER_Prop_lineEndArrowhead ] = true;
                bAlreadyWritten[ ESCHER_Prop_lineStartArrowWidth ] = true;
                bAlreadyWritten[ ESCHER_Prop_lineStartArrowLength ] = true;
                bAlreadyWritten[ ESCHER_Prop_lineEndArrowWidth ] = true;
                bAlreadyWritten[ ESCHER_Prop_lineEndArrowLength ] = true;
                bAlreadyWritten[ ESCHER_Prop_lineJoinStyle ] = true;
                bAlreadyWritten[ ESCHER_Prop_lineEndCapStyle ] = true;
                break;

            case ESCHER_Prop_fHidden:
                if ( !it->nPropValue )
                    m_pShapeStyle->append( ";visibility:hidden" );
                break;
            case ESCHER_Prop_shadowColor:
            case ESCHER_Prop_fshadowObscured:
                {
                    sal_uInt32 nValue = 0;
                    bool bShadow = false;
                    bool bObscured = false;
                    if ( rProps.GetOpt( ESCHER_Prop_fshadowObscured, nValue ) )
                    {
                        bShadow = (( nValue & 0x20002 ) == 0x20002 );
                        bObscured = (( nValue & 0x10001 ) == 0x10001 );
                    }
                    if ( bShadow )
                    {
                        sax_fastparser::FastAttributeList *pAttrList = m_pSerializer->createAttrList();
                        impl_AddBool( pAttrList, XML_on, bShadow );
                        impl_AddBool( pAttrList, XML_obscured, bObscured );

                        if ( rProps.GetOpt( ESCHER_Prop_shadowColor, nValue ) )
                            impl_AddColor( pAttrList, XML_color, nValue );

                        m_pSerializer->singleElementNS( XML_v, XML_shadow, XFastAttributeListRef( pAttrList ) );
                        bAlreadyWritten[ ESCHER_Prop_fshadowObscured ] = true;
                        bAlreadyWritten[ ESCHER_Prop_shadowColor ] = true;
                    }
                }
                break;
            case ESCHER_Prop_gtextUNICODE:
            case ESCHER_Prop_gtextFont:
                {
                    EscherPropSortStruct aUnicode;
                    if (rProps.GetOpt(ESCHER_Prop_gtextUNICODE, aUnicode))
                    {
                        SvMemoryStream aStream;
                        aStream.Write(it->pBuf, it->nPropSize);
                        aStream.Seek(0);
                        OUString aTextPathString = SvxMSDffManager::MSDFFReadZString(aStream, it->nPropSize, true);
                        aStream.Seek(0);

                        m_pSerializer->singleElementNS( XML_v, XML_path,
                                XML_textpathok, "t",
                                FSEND );

                        sax_fastparser::FastAttributeList* pAttrList = m_pSerializer->createAttrList();
                        pAttrList->add(XML_on, "t");
                        pAttrList->add(XML_fitshape, "t");
                        pAttrList->add(XML_string, OUStringToOString(aTextPathString, RTL_TEXTENCODING_UTF8));
                        EscherPropSortStruct aFont;
                        OUString aStyle;
                        if (rProps.GetOpt(ESCHER_Prop_gtextFont, aFont))
                        {
                            aStream.Write(aFont.pBuf, aFont.nPropSize);
                            aStream.Seek(0);
                            OUString aTextPathFont = SvxMSDffManager::MSDFFReadZString(aStream, aFont.nPropSize, true);
                            aStyle += "font-family:\"" + aTextPathFont + "\"";
                        }
                        if (!aStyle.isEmpty())
                            pAttrList->add(XML_style, OUStringToOString(aStyle, RTL_TEXTENCODING_UTF8));
                        m_pSerializer->singleElementNS(XML_v, XML_textpath, XFastAttributeListRef(pAttrList));
                    }

                    bAlreadyWritten[ESCHER_Prop_gtextUNICODE] = true;
                    bAlreadyWritten[ESCHER_Prop_gtextFont] = true;
                }
                break;
            case ESCHER_Prop_Rotation:
                {
                    // The higher half of the variable contains the angle.
                    m_pShapeStyle->append(";rotation:").append(double(it->nPropValue >> 16));
                    bAlreadyWritten[ESCHER_Prop_Rotation] = true;
                }
                break;
            case ESCHER_Prop_fNoLineDrawDash:
                {
                    // See DffPropertyReader::ApplyLineAttributes().
                    impl_AddBool( m_pShapeAttrList, XML_stroked, it->nPropValue & 8 );
                    bAlreadyWritten[ESCHER_Prop_fNoLineDrawDash] = true;
                }
                break;
            default:
#if OSL_DEBUG_LEVEL > 0
                fprintf( stderr, "TODO VMLExport::Commit(), unimplemented id: %d, value: %" SAL_PRIuUINT32 ", data: [%" SAL_PRIuUINT32 ", %p]\n",
                        nId, it->nPropValue, it->nPropSize, it->pBuf );
                if ( it->nPropSize )
                {
                    const sal_uInt8 *pIt = it->pBuf;
                    fprintf( stderr, "    ( " );
                    for ( int nCount = it->nPropSize; nCount; --nCount )
                    {
                        fprintf( stderr, "%02x ", *pIt );
                        ++pIt;
                    }
                    fprintf( stderr, ")\n" );
                }
#endif
                break;
        }
    }

    m_pSerializer->mergeTopMarks( sax_fastparser::MERGE_MARKS_POSTPONE );
}

OString VMLExport::ShapeIdString( sal_uInt32 nId )
{
    return OStringBuffer( 20 ).append( "shape_" ).append( sal_Int64( nId ) ).makeStringAndClear();
}

void VMLExport::AddLineDimensions( const Rectangle& rRectangle )
{
    // style
    if ( !m_pShapeStyle->isEmpty() )
        m_pShapeStyle->append( ";" );

    m_pShapeStyle->append( "position:absolute" );

    switch ( m_nShapeFlags & 0xC0 )
    {
        case 0x40: m_pShapeStyle->append( ";flip:y" ); break;
        case 0x80: m_pShapeStyle->append( ";flip:x" ); break;
        case 0xC0: m_pShapeStyle->append( ";flip:xy" ); break;
    }

    // the actual dimensions
    OString aLeft, aTop, aRight, aBottom;

    if ( mnGroupLevel == 1 )
    {
        const OString aPt( "pt" );
        aLeft = OString::valueOf( double( rRectangle.Left() ) / 20 ) + aPt;
        aTop = OString::valueOf( double( rRectangle.Top() ) / 20 ) + aPt;
        aRight = OString::valueOf( double( rRectangle.Right() ) / 20 ) + aPt;
        aBottom = OString::valueOf( double( rRectangle.Bottom() ) / 20 ) + aPt;
    }
    else
    {
        aLeft = OString::valueOf( rRectangle.Left() );
        aTop = OString::valueOf( rRectangle.Top() );
        aRight = OString::valueOf( rRectangle.Right() );
        aBottom = OString::valueOf( rRectangle.Bottom() );
    }

    m_pShapeAttrList->add( XML_from,
            OStringBuffer( 20 ).append( aLeft )
            .append( "," ).append( aTop )
            .makeStringAndClear() );

    m_pShapeAttrList->add( XML_to,
            OStringBuffer( 20 ).append( aRight )
            .append( "," ).append( aBottom )
            .makeStringAndClear() );
}

void VMLExport::AddRectangleDimensions( OStringBuffer& rBuffer, const Rectangle& rRectangle )
{
    if ( !rBuffer.isEmpty() )
        rBuffer.append( ";" );

    rBuffer.append( "position:absolute;" );

    if ( mnGroupLevel == 1 )
    {
        rBuffer.append( "margin-left:" ).append( double( rRectangle.Left() ) / 20 )
            .append( "pt;margin-top:" ).append( double( rRectangle.Top() ) / 20 )
            .append( "pt;width:" ).append( double( rRectangle.Right() - rRectangle.Left() ) / 20 )
            .append( "pt;height:" ).append( double( rRectangle.Bottom() - rRectangle.Top() ) / 20 )
            .append( "pt" );
    }
    else
    {
        rBuffer.append( "left:" ).append( rRectangle.Left() )
            .append( ";top:" ).append( rRectangle.Top() )
            .append( ";width:" ).append( rRectangle.Right() - rRectangle.Left() )
            .append( ";height:" ).append( rRectangle.Bottom() - rRectangle.Top() );
    }
}

void VMLExport::AddShapeAttribute( sal_Int32 nAttribute, const OString& rValue )
{
    m_pShapeAttrList->add( nAttribute, rValue );
}

extern const char* pShapeTypes[];

sal_Int32 VMLExport::StartShape()
{
    if ( m_nShapeType == ESCHER_ShpInst_Nil )
        return -1;

    // some of the shapes have their own name ;-)
    sal_Int32 nShapeElement = -1;
    bool bReferToShapeType = false;
    switch ( m_nShapeType )
    {
        case ESCHER_ShpInst_NotPrimitive:   nShapeElement = XML_shape;     break;
        case ESCHER_ShpInst_Rectangle:      nShapeElement = XML_rect;      break;
        case ESCHER_ShpInst_RoundRectangle: nShapeElement = XML_roundrect; break;
        case ESCHER_ShpInst_Ellipse:        nShapeElement = XML_oval;      break;
        case ESCHER_ShpInst_Arc:            nShapeElement = XML_arc;       break;
        case ESCHER_ShpInst_Line:           nShapeElement = XML_line;      break;
        default:
            if ( m_nShapeType < ESCHER_ShpInst_COUNT )
            {
                nShapeElement = XML_shape;

                // a predefined shape?
                const char* pShapeType = pShapeTypes[ m_nShapeType ];
                if ( pShapeType )
                {
                    bReferToShapeType = true;
                    if ( !m_pShapeTypeWritten[ m_nShapeType ] )
                    {
                        m_pSerializer->write( pShapeType );
                        m_pShapeTypeWritten[ m_nShapeType ] = true;
                    }
                }
                else
                {
                    // rectangle is probably the best fallback...
                    nShapeElement = XML_rect;
                }
            }
            break;
    }

    // anchoring
    switch (m_eHOri)
    {
        case text::HoriOrientation::LEFT:
            m_pShapeStyle->append(";mso-position-horizontal:left");
            break;
        case text::HoriOrientation::CENTER:
            m_pShapeStyle->append(";mso-position-horizontal:center");
            break;
        case text::HoriOrientation::RIGHT:
            m_pShapeStyle->append(";mso-position-horizontal:right");
            break;
        case text::HoriOrientation::INSIDE:
            m_pShapeStyle->append(";mso-position-horizontal:inside");
            break;
        case text::HoriOrientation::OUTSIDE:
            m_pShapeStyle->append(";mso-position-horizontal:outside");
            break;
        default:
        case text::HoriOrientation::NONE:
            break;
    }
    switch (m_eHRel)
    {
        case text::RelOrientation::PAGE_PRINT_AREA:
            m_pShapeStyle->append(";mso-position-horizontal-relative:margin");
            break;
        case text::RelOrientation::PAGE_FRAME:
        case text::RelOrientation::PAGE_LEFT:
        case text::RelOrientation::PAGE_RIGHT:
            m_pShapeStyle->append(";mso-position-horizontal-relative:page");
            break;
        case text::RelOrientation::CHAR:
            m_pShapeStyle->append(";mso-position-horizontal-relative:char");
            break;
        default:
            break;
    }

    switch (m_eVOri)
    {
        case text::VertOrientation::TOP:
        case text::VertOrientation::LINE_TOP:
        case text::VertOrientation::CHAR_TOP:
            m_pShapeStyle->append(";mso-position-vertical:top");
            break;
        case text::VertOrientation::CENTER:
        case text::VertOrientation::LINE_CENTER:
            m_pShapeStyle->append(";mso-position-vertical:center");
            break;
        case text::VertOrientation::BOTTOM:
        case text::VertOrientation::LINE_BOTTOM:
        case text::VertOrientation::CHAR_BOTTOM:
            m_pShapeStyle->append(";mso-position-vertical:bottom");
            break;
        default:
        case text::VertOrientation::NONE:
            break;
    }
    switch (m_eVRel)
    {
        case text::RelOrientation::PAGE_PRINT_AREA:
            m_pShapeStyle->append(";mso-position-vertical-relative:margin");
            break;
        case text::RelOrientation::PAGE_FRAME:
            m_pShapeStyle->append(";mso-position-vertical-relative:page");
            break;
        default:
            break;
    }

    // add style
    m_pShapeAttrList->add( XML_style, m_pShapeStyle->makeStringAndClear() );

    if ( nShapeElement >= 0 )
    {
        if ( bReferToShapeType )
        {
            m_pShapeAttrList->add( XML_type, OStringBuffer( 20 )
                    .append( "shapetype_" ).append( sal_Int32( m_nShapeType ) )
                    .makeStringAndClear() );
        }

        // start of the shape
        m_pSerializer->startElementNS( XML_v, nShapeElement, XFastAttributeListRef( m_pShapeAttrList ) );
    }

    // now check if we have some text and we have a text exporter registered
    const SdrTextObj* pTxtObj = PTR_CAST(SdrTextObj, m_pSdrObject);
    if (pTxtObj && m_pTextExport && m_nShapeType != ESCHER_ShpInst_TextPlainText)
    {
        const OutlinerParaObject* pParaObj = 0;
        bool bOwnParaObj = false;

        /*
        #i13885#
        When the object is actively being edited, that text is not set into
        the objects normal text object, but lives in a separate object.
        */
        if (pTxtObj->IsTextEditActive())
        {
            pParaObj = pTxtObj->GetEditOutlinerParaObject();
            bOwnParaObj = true;
        }
        else
        {
            pParaObj = pTxtObj->GetOutlinerParaObject();
        }

        if( pParaObj )
        {
            // this is reached only in case some text is attached to the shape
            m_pTextExport->WriteOutliner(*pParaObj);
            if( bOwnParaObj )
                delete pParaObj;
        }
    }

    return nShapeElement;
}

void VMLExport::EndShape( sal_Int32 nShapeElement )
{
    if ( nShapeElement >= 0 )
    {
        // end of the shape
        m_pSerializer->endElementNS( XML_v, nShapeElement );
    }
}

sal_uInt32 VMLExport::AddSdrObject( const SdrObject& rObj, sal_Int16 eHOri, sal_Int16 eVOri, sal_Int16 eHRel, sal_Int16 eVRel, const Point* pNdTopLeft )
{
    m_pSdrObject = &rObj;
    m_eHOri = eHOri;
    m_eVOri = eVOri;
    m_eHRel = eHRel;
    m_eVRel = eVRel;
    m_pNdTopLeft = pNdTopLeft;
    return EscherEx::AddSdrObject(rObj);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
