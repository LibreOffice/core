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

#include "eschesdo.hxx"
#include <svx/svdobj.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdoashp.hxx>
#include <svx/unoshape.hxx>
#include <vcl/outdev.hxx>
#include <tools/poly.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/graph.hxx>
#include <tools/debug.hxx>
#include <svx/fmdpage.hxx>
#include <toolkit/unohlp.hxx>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/FlagSequence.hpp>
#include <com/sun/star/drawing/TextAdjust.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <comphelper/extract.hxx>
#include <svtools/fltcall.hxx>
#include <vcl/cvtgrf.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::style;

#define EES_MAP_FRACTION 1440   // 1440 dpi

ImplEESdrWriter::ImplEESdrWriter( EscherEx& rEx )
        :
        mpEscherEx              ( &rEx ),
        maMapModeSrc            ( MAP_100TH_MM ),
        // PowerPoint: 576 dpi, WinWord: 1440 dpi, Excel: 1440 dpi
        maMapModeDest( MAP_INCH, Point(), Fraction( 1, EES_MAP_FRACTION ), Fraction( 1, EES_MAP_FRACTION ) ),
        mpPicStrm               ( NULL ),
        mpHostAppData           ( NULL ),
        mnPagesWritten          ( 0 ),
        mnShapeMasterTitle      ( 0 ),
        mnShapeMasterBody       ( 0 ),
        mbStatusIndicator       ( sal_False ),
        mbStatus                ( sal_False )
{
}


Point ImplEESdrWriter::ImplMapPoint( const Point& rPoint )
{
    return OutputDevice::LogicToLogic( rPoint, maMapModeSrc, maMapModeDest );
}

Size ImplEESdrWriter::ImplMapSize( const Size& rSize )
{
    Size aRetSize( OutputDevice::LogicToLogic( rSize, maMapModeSrc, maMapModeDest ) );

    if ( !aRetSize.Width() )
        aRetSize.Width()++;
    if ( !aRetSize.Height() )
        aRetSize.Height()++;
    return aRetSize;
}

void ImplEESdrWriter::ImplFlipBoundingBox( ImplEESdrObject& rObj, EscherPropertyContainer& rPropOpt )
{
    sal_Int32 nAngle = rObj.GetAngle();
    Rectangle aRect( rObj.GetRect() );

    if ( nAngle < 0 )
        nAngle = ( 36000 + nAngle ) % 36000;
    else
        nAngle = ( 36000 - ( nAngle % 36000 ) );

    double fVal = (double)nAngle * F_PI18000;
    double  fCos = cos( fVal );
    double  fSin = sin( fVal );

    double  nWidthHalf = (double) aRect.GetWidth() / 2;
    double  nHeightHalf = (double) aRect.GetHeight() / 2;

    double nXDiff = fCos * nWidthHalf + fSin * (-nHeightHalf);
    double nYDiff = - ( fSin * nWidthHalf - fCos * ( -nHeightHalf ) );

    aRect.Move( (sal_Int32)( -( nWidthHalf - nXDiff ) ), (sal_Int32)( - ( nHeightHalf + nYDiff ) ) );

    nAngle *= 655;
    nAngle += 0x8000;
    nAngle &=~0xffff;                                   // nAngle auf volle Gradzahl runden
    rPropOpt.AddOpt( ESCHER_Prop_Rotation, nAngle );

    rObj.SetAngle( nAngle );
    rObj.SetRect( aRect );
}

//  -----------------------------------------------------------------------

#define ADD_SHAPE( nType, nFlags )                              \
{                                                               \
    nShapeType = nType;                                         \
    nShapeID = mpEscherEx->GenerateShapeId();                   \
    rObj.SetShapeId( nShapeID );                                \
    mpEscherEx->AddShape( (sal_uInt32)nType, (sal_uInt32)nFlags, nShapeID );    \
    rSolverContainer.AddShape( rObj.GetShapeRef(), nShapeID );  \
}

#define SHAPE_TEXT( bFill )                                         \
{                                                                   \
    mpEscherEx->OpenContainer( ESCHER_SpContainer );                \
    ADD_SHAPE( ESCHER_ShpInst_TextBox, 0xa00 );                     \
    if ( bFill )                                                    \
        aPropOpt.CreateFillProperties( rObj.mXPropSet, sal_True );  \
    if( rObj.ImplGetText() )                                        \
        aPropOpt.CreateTextProperties( rObj.mXPropSet,              \
            mpEscherEx->QueryTextID( rObj.GetShapeRef(),            \
                rObj.GetShapeId() ) );                              \
}

//Map from twips to export units, generally twips as well, only excel and word
//export is happening here, so native units are export units, leave as
//placeholder if required in future
void ImplEESdrWriter::MapRect(ImplEESdrObject& /* rObj */ )
{
}

sal_uInt32 ImplEESdrWriter::ImplWriteShape( ImplEESdrObject& rObj,
                                EscherSolverContainer& rSolverContainer,
                                ImplEESdrPageType ePageType )
{
    sal_uInt32 nShapeID = 0;
    sal_uInt16 nShapeType = 0;
    sal_Bool bDontWriteText = sal_False;        // if a metafile is written as shape replacement, then the text is already part of the metafile
    sal_Bool bAdditionalText = sal_False;
    sal_uInt32 nGrpShapeID = 0;

    do {
        mpHostAppData = mpEscherEx->StartShape( rObj.GetShapeRef(), (mpEscherEx->GetGroupLevel() > 1) ? &rObj.GetRect() : 0 );
        if ( mpHostAppData && mpHostAppData->DontWriteShape() )
            break;

        // #i51348# get shape name
        rtl::OUString aShapeName;
        if( const SdrObject* pSdrObj = rObj.GetSdrObject() )
            if (!pSdrObj->GetName().isEmpty())
                aShapeName = pSdrObj->GetName();

        Point aTextRefPoint;

        if( rObj.GetType().EqualsAscii( "drawing.Group" ))
        {
            Reference< XIndexAccess > xXIndexAccess( rObj.GetShapeRef(), UNO_QUERY );

            if( xXIndexAccess.is() && 0 != xXIndexAccess->getCount() )
            {
                nShapeID = mpEscherEx->EnterGroup( aShapeName, &rObj.GetRect() );
                nShapeType = ESCHER_ShpInst_Min;

                for( sal_uInt32 n = 0, nCnt = xXIndexAccess->getCount();
                        n < nCnt; ++n )
                {
                    ImplEESdrObject aObj( *this, *(Reference< XShape >*)
                                    xXIndexAccess->getByIndex( n ).getValue() );
                    if( aObj.IsValid() )
                        ImplWriteShape( aObj, rSolverContainer, ePageType );
                }
                mpEscherEx->LeaveGroup();
            }
            break;
        }
        rObj.SetAngle( rObj.ImplGetInt32PropertyValue( ::rtl::OUString( "RotateAngle" ) ));

        if( ( rObj.ImplGetPropertyValue( ::rtl::OUString( "IsFontwork" ) ) &&
            ::cppu::any2bool( rObj.GetUsrAny() ) ) ||
            rObj.GetType().EqualsAscii( "drawing.Measure" ) )
        {
            rObj.SetType( String( RTL_CONSTASCII_STRINGPARAM(
                                "drawing.dontknow" ),
                                RTL_TEXTENCODING_MS_1252 ));
        }

        const ::com::sun::star::awt::Size   aSize100thmm( rObj.GetShapeRef()->getSize() );
        const ::com::sun::star::awt::Point  aPoint100thmm( rObj.GetShapeRef()->getPosition() );
        Rectangle   aRect100thmm( Point( aPoint100thmm.X, aPoint100thmm.Y ), Size( aSize100thmm.Width, aSize100thmm.Height ) );
        if ( !mpPicStrm )
            mpPicStrm = mpEscherEx->QueryPictureStream();
        EscherPropertyContainer aPropOpt( mpEscherEx->GetGraphicProvider(), mpPicStrm, aRect100thmm );

        // #i51348# shape name
        if (!aShapeName.isEmpty())
            aPropOpt.AddOpt( ESCHER_Prop_wzName, aShapeName );
        if ( InteractionInfo* pInteraction = mpHostAppData ? mpHostAppData->GetInteractionInfo():NULL )
        {
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            const std::auto_ptr< SvMemoryStream >& pMemStrm = pInteraction->getHyperlinkRecord();
            SAL_WNODEPRECATED_DECLARATIONS_POP
            if ( pMemStrm.get() )
            {
                pMemStrm->ObjectOwnsMemory( sal_False );
                sal_uInt8* pBuf = (sal_uInt8*) pMemStrm->GetData();
                sal_uInt32 nSize = pMemStrm->Seek( STREAM_SEEK_TO_END );
                aPropOpt.AddOpt( ESCHER_Prop_pihlShape, sal_False, nSize, pBuf, nSize );
            }
            if ( pInteraction->hasInteraction() )
                aPropOpt.AddOpt( ESCHER_Prop_fPrint, 0x00080008 );
        }

        if ( rObj.GetType().EqualsAscii( "drawing.Custom" ) )
        {
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            sal_uInt32 nMirrorFlags;

            rtl::OUString sCustomShapeType;
            MSO_SPT eShapeType = aPropOpt.GetCustomShapeType( rObj.GetShapeRef(), nMirrorFlags, sCustomShapeType );
            if ( sCustomShapeType == "col-502ad400" || sCustomShapeType == "col-60da8460" )
            {
                ADD_SHAPE( ESCHER_ShpInst_PictureFrame, 0xa00 );
                if ( aPropOpt.CreateGraphicProperties( rObj.mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "MetaFile" ) ), sal_False ) )
                {
                    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
                    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100000 );        // no fill
                    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90000 );        // no linestyle
                        SdrObject* pObj = GetSdrObjectFromXShape( rObj.GetShapeRef() );
                    if ( pObj )
                    {
                        Rectangle aBound = pObj->GetCurrentBoundRect();
                        Point aPosition( ImplMapPoint( aBound.TopLeft() ) );
                        Size aSize( ImplMapSize( aBound.GetSize() ) );
                        rObj.SetRect( Rectangle( aPosition, aSize ) );
                        rObj.SetAngle( 0 );
                        bDontWriteText = sal_True;
                    }
                }
            }
            else
            {
                ADD_SHAPE(
                    sal::static_int_cast< sal_uInt16 >(eShapeType),
                    nMirrorFlags | 0xa00 );
                aPropOpt.CreateCustomShapeProperties( eShapeType, rObj.GetShapeRef() );
                aPropOpt.CreateFillProperties( rObj.mXPropSet, sal_True );
                if ( rObj.ImplGetText() )
                {
                    if ( !aPropOpt.IsFontWork() )
                        aPropOpt.CreateTextProperties( rObj.mXPropSet, mpEscherEx->QueryTextID(
                            rObj.GetShapeRef(), rObj.GetShapeId() ), sal_True, sal_False );
                }
            }
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.Rectangle" ))
        {
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            sal_Int32 nRadius = (sal_Int32)rObj.ImplGetInt32PropertyValue(
                                            ::rtl::OUString( "CornerRadius" ));
            if( nRadius )
            {
                nRadius = ImplMapSize( Size( nRadius, 0 )).Width();
                ADD_SHAPE( ESCHER_ShpInst_RoundRectangle, 0xa00 );  // Flags: Connector | HasSpt
                sal_Int32 nLenght = rObj.GetRect().GetWidth();
                if ( nLenght > rObj.GetRect().GetHeight() )
                    nLenght = rObj.GetRect().GetHeight();
                nLenght >>= 1;
                if ( nRadius >= nLenght )
                    nRadius = 0x2a30;                           // 0x2a30 ist PPTs maximum radius
                else
                    nRadius = ( 0x2a30 * nRadius ) / nLenght;
                aPropOpt.AddOpt( ESCHER_Prop_adjustValue, nRadius );
            }
            else
            {
                ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0xa00 );           // Flags: Connector | HasSpt
            }
            aPropOpt.CreateFillProperties( rObj.mXPropSet, sal_True );
            if( rObj.ImplGetText() )
                aPropOpt.CreateTextProperties( rObj.mXPropSet,
                    mpEscherEx->QueryTextID( rObj.GetShapeRef(),
                        rObj.GetShapeId() ), sal_False, sal_False );
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.Ellipse" ))
        {
            CircleKind  eCircleKind = CircleKind_FULL;
            PolyStyle   ePolyKind = PolyStyle();
            if ( rObj.ImplGetPropertyValue( ::rtl::OUString( "CircleKind" ) ) )
            {
                eCircleKind = *( (CircleKind*)rObj.GetUsrAny().getValue() );
                switch ( eCircleKind )
                {
                    case CircleKind_SECTION :
                    {
                        ePolyKind = POLY_PIE;
                    }
                    break;
                    case CircleKind_ARC :
                    {
                        ePolyKind = POLY_ARC;
                    }
                    break;

                    case CircleKind_CUT :
                    {
                        ePolyKind = POLY_CHORD;
                    }
                    break;

                    default:
                        eCircleKind = CircleKind_FULL;
                }
            }
            if ( eCircleKind == CircleKind_FULL )
            {
                mpEscherEx->OpenContainer( ESCHER_SpContainer );
                ADD_SHAPE( ESCHER_ShpInst_Ellipse, 0xa00 );         // Flags: Connector | HasSpt
                aPropOpt.CreateFillProperties( rObj.mXPropSet, sal_True );
            }
            else
            {
                sal_Int32 nStartAngle, nEndAngle;
                if ( !rObj.ImplGetPropertyValue( ::rtl::OUString( "CircleStartAngle" ) ) )
                    break;
                nStartAngle = *( (sal_Int32*)rObj.GetUsrAny().getValue() );
                if( !rObj.ImplGetPropertyValue( ::rtl::OUString( "CircleEndAngle" ) ) )
                    break;
                nEndAngle = *( (sal_Int32*)rObj.GetUsrAny().getValue() );

                Point aStart, aEnd, aCenter;
                aStart.X() = (sal_Int32)( ( cos( (double)( nStartAngle *
                                                F_PI18000 ) ) * 100.0 ) );
                aStart.Y() = - (sal_Int32)( ( sin( (double)( nStartAngle *
                                                F_PI18000 ) ) * 100.0 ) );
                aEnd.X() = (sal_Int32)( ( cos( (double)( nEndAngle *
                                                F_PI18000 ) ) * 100.0 ) );
                aEnd.Y() = - (sal_Int32)( ( sin( (double)( nEndAngle *
                                                F_PI18000 ) ) * 100.0 ) );
                const Rectangle& rRect = aRect100thmm;
                aCenter.X() = rRect.Left() + ( rRect.GetWidth() / 2 );
                aCenter.Y() = rRect.Top() + ( rRect.GetHeight() / 2 );
                aStart.X() += aCenter.X();
                aStart.Y() += aCenter.Y();
                aEnd.X() += aCenter.X();
                aEnd.Y() += aCenter.Y();
                Polygon aPolygon( rRect, aStart, aEnd, ePolyKind );
                if( rObj.GetAngle() )
                {
                    aPolygon.Rotate( rRect.TopLeft(), (sal_uInt16)( rObj.GetAngle() / 10 ) );
                    rObj.SetAngle( 0 );
                }
                mpEscherEx->OpenContainer( ESCHER_SpContainer );
                ADD_SHAPE( ESCHER_ShpInst_NotPrimitive, 0xa00 );        // Flags: Connector | HasSpt
                ::com::sun::star::awt::Rectangle aNewRect;
                switch ( ePolyKind )
                {
                    case POLY_PIE :
                    case POLY_CHORD :
                    {
                        aPropOpt.CreatePolygonProperties( rObj.mXPropSet, ESCHER_CREATEPOLYGON_POLYPOLYGON, sal_False, aNewRect, &aPolygon );
                        aPropOpt.CreateFillProperties( rObj.mXPropSet, sal_True  );
                    }
                    break;

                    case POLY_ARC :
                    {
                        aPropOpt.CreatePolygonProperties( rObj.mXPropSet, ESCHER_CREATEPOLYGON_POLYLINE, sal_False, aNewRect, &aPolygon );
                        aPropOpt.CreateLineProperties( rObj.mXPropSet, sal_False );
                    }
                    break;
                }
                rObj.SetRect( Rectangle( ImplMapPoint( Point( aNewRect.X, aNewRect.Y ) ),
                                            ImplMapSize( Size( aNewRect.Width, aNewRect.Height ) ) ) );
            }
            if ( rObj.ImplGetText() )
                aPropOpt.CreateTextProperties( rObj.mXPropSet,
                    mpEscherEx->QueryTextID( rObj.GetShapeRef(),
                        rObj.GetShapeId() ), sal_False, sal_False );

        }
        else if ( rObj.GetType().EqualsAscii( "drawing.Control" ))
        {
            break;
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.Connector" ))
        {
            sal_uInt16 nSpType, nSpFlags;
            ::com::sun::star::awt::Rectangle aNewRect;
            if ( aPropOpt.CreateConnectorProperties( rObj.GetShapeRef(),
                            rSolverContainer, aNewRect, nSpType, nSpFlags ) == sal_False )
                break;
            rObj.SetRect( Rectangle( ImplMapPoint( Point( aNewRect.X, aNewRect.Y ) ),
                                        ImplMapSize( Size( aNewRect.Width, aNewRect.Height ) ) ) );

            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            ADD_SHAPE( nSpType, nSpFlags );
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.Measure" ))
        {
            break;
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.Line" ))
        {
            ::com::sun::star::awt::Rectangle aNewRect;
            aPropOpt.CreatePolygonProperties( rObj.mXPropSet, ESCHER_CREATEPOLYGON_LINE, sal_False, aNewRect, NULL );
            MapRect(rObj);
            //i27942: Poly/Lines/Bezier do not support text.

            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            sal_uInt32 nFlags = 0xa00;      // Flags: Connector | HasSpt
            if( aNewRect.Height < 0 )
                nFlags |= 0x80;             // Flags: VertMirror
            if( aNewRect.Width < 0 )
                nFlags |= 0x40;             // Flags: HorzMirror

            ADD_SHAPE( ESCHER_ShpInst_Line, nFlags );
            aPropOpt.AddOpt( ESCHER_Prop_shapePath, ESCHER_ShapeComplex );
            aPropOpt.CreateLineProperties( rObj.mXPropSet, sal_False );
            rObj.SetAngle( 0 );
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.PolyPolygon" ))
        {
            if( rObj.ImplHasText() )
            {
                nGrpShapeID = ImplEnterAdditionalTextGroup( rObj.GetShapeRef(), &rObj.GetRect() );
                bAdditionalText = sal_True;
            }
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            ADD_SHAPE( ESCHER_ShpInst_NotPrimitive, 0xa00 );        // Flags: Connector | HasSpt
            ::com::sun::star::awt::Rectangle aNewRect;
            aPropOpt.CreatePolygonProperties( rObj.mXPropSet, ESCHER_CREATEPOLYGON_POLYPOLYGON, sal_False, aNewRect, NULL );
            MapRect(rObj);
            aPropOpt.CreateFillProperties( rObj.mXPropSet, sal_True );
            rObj.SetAngle( 0 );
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.PolyLine" ))
        {
            //i27942: Poly/Lines/Bezier do not support text.

            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            ADD_SHAPE( ESCHER_ShpInst_NotPrimitive, 0xa00 );        // Flags: Connector | HasSpt
            ::com::sun::star::awt::Rectangle aNewRect;
            aPropOpt.CreatePolygonProperties( rObj.mXPropSet, ESCHER_CREATEPOLYGON_POLYLINE, sal_False, aNewRect, NULL );
            MapRect(rObj);
            aPropOpt.CreateLineProperties( rObj.mXPropSet, sal_False );
            rObj.SetAngle( 0 );
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.OpenBezier" ) )
        {
            //i27942: Poly/Lines/Bezier do not support text.

            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            ADD_SHAPE( ESCHER_ShpInst_NotPrimitive, 0xa00 );        // Flags: Connector | HasSpt
            ::com::sun::star::awt::Rectangle aNewRect;
            aPropOpt.CreatePolygonProperties( rObj.mXPropSet, ESCHER_CREATEPOLYGON_POLYLINE, sal_True, aNewRect, NULL );
            MapRect(rObj);
            aPropOpt.CreateLineProperties( rObj.mXPropSet, sal_False );
            rObj.SetAngle( 0 );
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.ClosedBezier" ) )
        {
            if ( rObj.ImplHasText() )
            {
                nGrpShapeID = ImplEnterAdditionalTextGroup( rObj.GetShapeRef(), &rObj.GetRect() );
                bAdditionalText = sal_True;
            }
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            ADD_SHAPE( ESCHER_ShpInst_NotPrimitive, 0xa00 );        // Flags: Connector | HasSpt
            ::com::sun::star::awt::Rectangle aNewRect;
            aPropOpt.CreatePolygonProperties( rObj.mXPropSet, ESCHER_CREATEPOLYGON_POLYPOLYGON, sal_True, aNewRect, NULL );
            MapRect(rObj);
            aPropOpt.CreateFillProperties( rObj.mXPropSet, sal_True );
            rObj.SetAngle( 0 );
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.GraphicObject" ))
        {
            mpEscherEx->OpenContainer( ESCHER_SpContainer );

            // a GraphicObject can also be a ClickMe element
            if( rObj.IsEmptyPresObj() && ( ePageType == NORMAL ) )
            {
                ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0x220 );               // Flags: HaveAnchor | HaveMaster
                sal_uInt32 nTxtBxId = mpEscherEx->QueryTextID( rObj.GetShapeRef(),
                                                        rObj.GetShapeId() );
                aPropOpt.AddOpt( ESCHER_Prop_lTxid, nTxtBxId );
                aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x10001 );
                aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x10001 );
                aPropOpt.AddOpt( ESCHER_Prop_hspMaster, mnShapeMasterBody );
            }
            else
            {
                if( rObj.ImplGetText() )
                {
                    /* SJ #i34951#: because M. documents are not allowing GraphicObjects containing text, we
                       have to create a simpe Rectangle with fill bitmap instead (while not allowing BitmapMode_Repeat).
                    */
                    ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0xa00 );           // Flags: Connector | HasSpt
                    if ( aPropOpt.CreateGraphicProperties( rObj.mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "GraphicURL" ) ), sal_True,  sal_True, sal_False ) )
                    {
                        aPropOpt.AddOpt( ESCHER_Prop_WrapText, ESCHER_WrapNone );
                        aPropOpt.AddOpt( ESCHER_Prop_AnchorText, ESCHER_AnchorMiddle );
                        aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x140014 );
                        aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0x8000000 );
                        aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x80000 );
                        if ( rObj.ImplGetText() )
                            aPropOpt.CreateTextProperties( rObj.mXPropSet,
                                mpEscherEx->QueryTextID( rObj.GetShapeRef(),
                                    rObj.GetShapeId() ), sal_False, sal_False );
                    }
                }
                else
                {
                    ADD_SHAPE( ESCHER_ShpInst_PictureFrame, 0xa00 );
                    if ( aPropOpt.CreateGraphicProperties( rObj.mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "GraphicURL" ) ), sal_False, sal_True ) )
                        aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
                }
            }
        }
        else if ( rObj.GetType().EqualsAscii(  "drawing.Text" ))
        {
            SHAPE_TEXT( sal_True );
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.Page" ))
        {
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0xa00 );
            aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x40004 );
            aPropOpt.AddOpt( ESCHER_Prop_fFillOK, 0x100001 );
            aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x110011 );
            aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90008 );
            aPropOpt.AddOpt( ESCHER_Prop_fshadowObscured, 0x10001 );
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.Frame" ))
        {
            break;
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.OLE2" ))
        {
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            if( rObj.IsEmptyPresObj() && ( ePageType == NORMAL ) )
            {
                ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0x220 );               // Flags: HaveAnchor | HaveMaster
                sal_uInt32 nTxtBxId = mpEscherEx->QueryTextID( rObj.GetShapeRef(),
                                                        rObj.GetShapeId() );
                aPropOpt.AddOpt( ESCHER_Prop_lTxid, nTxtBxId );
                aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x10001 );
                aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x10001 );
                aPropOpt.AddOpt( ESCHER_Prop_hspMaster, mnShapeMasterBody );
            }
            else
            {
                //2do: could be made an option in HostAppData whether OLE object should be written or not
                sal_Bool bAppOLE = sal_True;
                ADD_SHAPE( ESCHER_ShpInst_PictureFrame,
                    0xa00 | (bAppOLE ? SHAPEFLAG_OLESHAPE : 0) );
                if ( aPropOpt.CreateOLEGraphicProperties( rObj.GetShapeRef() ) )
                {
                    if ( bAppOLE )
                    {   // snooped from Xcl hex dump, nobody knows the trouble I have seen
                        aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape,    0x00080008 );
                        aPropOpt.AddOpt( ESCHER_Prop_pictureId,     0x00000001 );
                        aPropOpt.AddOpt( ESCHER_Prop_fillColor,     0x08000041 );
                        aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0x08000041 );
                        aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest,    0x00110010 );
                        aPropOpt.AddOpt( ESCHER_Prop_lineColor,     0x08000040 );
                        aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash,0x00080008 );
                        aPropOpt.AddOpt( ESCHER_Prop_fPrint,            0x00080000 );
                    }
                    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
                }
            }
        }
        else if( '3' == rObj.GetType().GetChar(8 ) &&
                 'D' == rObj.GetType().GetChar( 9 ) )   // drawing.3D
        {
            // SceneObject, CubeObject, SphereObject, LatheObject, ExtrudeObject, PolygonObject
            if ( !rObj.ImplGetPropertyValue( ::rtl::OUString( "Bitmap" ) ) )
                break;

            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            ADD_SHAPE( ESCHER_ShpInst_PictureFrame, 0xa00 );

                if ( aPropOpt.CreateGraphicProperties( rObj.mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Bitmap" ) ), sal_False ) )
                aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.Caption" ))
        {
            rObj.SetAngle( 0 );
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            ADD_SHAPE( ESCHER_ShpInst_TextBox, 0xa00 );
            if ( aPropOpt.CreateGraphicProperties( rObj.mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "MetaFile" ) ), sal_False ) )
                aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.dontknow" ))
        {
            rObj.SetAngle( 0 );
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            ADD_SHAPE( ESCHER_ShpInst_PictureFrame, 0xa00 );
            if ( aPropOpt.CreateGraphicProperties( rObj.mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "MetaFile" ) ), sal_False ) )
                aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
        }
        else
        {
            break;
        }
        aPropOpt.CreateShadowProperties( rObj.mXPropSet );

        if( USHRT_MAX != mpEscherEx->GetHellLayerId() &&
            rObj.ImplGetPropertyValue( ::rtl::OUString( "LayerID" ) ) &&
            (*((sal_uInt16*)rObj.GetUsrAny().getValue()) ) == mpEscherEx->GetHellLayerId() )
        {
            aPropOpt.AddOpt( ESCHER_Prop_fPrint, 0x200020 );
        }

        {
            Rectangle aRect( rObj.GetRect() );
            aRect.Justify();
            rObj.SetRect( aRect );
        }

        if( rObj.GetAngle() )
            ImplFlipBoundingBox( rObj, aPropOpt );

        aPropOpt.CreateShapeProperties( rObj.GetShapeRef() );
        mpEscherEx->Commit( aPropOpt, rObj.GetRect() );
        if( mpEscherEx->GetGroupLevel() > 1 )
            mpEscherEx->AddChildAnchor( rObj.GetRect() );

        if ( mpHostAppData )
        {   //! with AdditionalText the App has to control whether these are written or not
            mpHostAppData->WriteClientAnchor( *mpEscherEx, rObj.GetRect() );
            mpHostAppData->WriteClientData( *mpEscherEx );
            if ( !bDontWriteText )
                mpHostAppData->WriteClientTextbox( *mpEscherEx );
        }
        mpEscherEx->CloseContainer();       // ESCHER_SpContainer

        if( bAdditionalText )
        {
            mpEscherEx->EndShape( nShapeType, nShapeID );
            ImplWriteAdditionalText( rObj, aTextRefPoint );
        }

    } while ( 0 );

    if ( bAdditionalText )
        mpEscherEx->EndShape( ESCHER_ShpInst_Min, nGrpShapeID );
    else
        mpEscherEx->EndShape( nShapeType, nShapeID );
    return nShapeID;
}

void ImplEESdrWriter::ImplWriteAdditionalText( ImplEESdrObject& rObj,
                                                const Point& rTextRefPoint )
{
    sal_uInt32 nShapeID = 0;
    sal_uInt16 nShapeType = 0;
    do
    {
        mpHostAppData = mpEscherEx->StartShape( rObj.GetShapeRef(), (mpEscherEx->GetGroupLevel() > 1) ? &rObj.GetRect() : 0 );
        if ( mpHostAppData && mpHostAppData->DontWriteShape() )
            break;

        const ::com::sun::star::awt::Size   aSize100thmm( rObj.GetShapeRef()->getSize() );
        const ::com::sun::star::awt::Point  aPoint100thmm( rObj.GetShapeRef()->getPosition() );
        Rectangle   aRect100thmm( Point( aPoint100thmm.X, aPoint100thmm.Y ), Size( aSize100thmm.Width, aSize100thmm.Height ) );
        if ( !mpPicStrm )
            mpPicStrm = mpEscherEx->QueryPictureStream();
        EscherPropertyContainer aPropOpt( mpEscherEx->GetGraphicProvider(), mpPicStrm, aRect100thmm );
        rObj.SetAngle( rObj.ImplGetInt32PropertyValue( ::rtl::OUString( "RotateAngle" )));
        sal_Int32 nAngle = rObj.GetAngle();
        if( rObj.GetType().EqualsAscii( "drawing.Line" ))
        {
//2do: this does not work right
            double fDist = hypot( rObj.GetRect().GetWidth(),
                                    rObj.GetRect().GetHeight() );
            rObj.SetRect( Rectangle( rTextRefPoint,
                            Point( (sal_Int32)( rTextRefPoint.X() + fDist ), rTextRefPoint.Y() - 1 ) ) );

            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            mpEscherEx->AddShape( ESCHER_ShpInst_TextBox, 0xa00 );
            if ( rObj.ImplGetText() )
                aPropOpt.CreateTextProperties( rObj.mXPropSet,
                    mpEscherEx->QueryTextID( rObj.GetShapeRef(),
                        rObj.GetShapeId() ) );

            aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90000 );
            aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100000 );
            aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, 0x60006 );     // Size Shape To Fit Text
            if ( nAngle < 0 )
                nAngle = ( 36000 + nAngle ) % 36000;
            if ( nAngle )
                ImplFlipBoundingBox( rObj, aPropOpt );
        }
        else
        {
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            nShapeID = mpEscherEx->GenerateShapeId();
            mpEscherEx->AddShape( nShapeType = ESCHER_ShpInst_TextBox, 0xa00, nShapeID );
            if ( rObj.ImplGetText() )
                aPropOpt.CreateTextProperties( rObj.mXPropSet,
                    mpEscherEx->QueryTextID( rObj.GetShapeRef(),
                        rObj.GetShapeId() ) );
            aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90000 );
            aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100000 );

            if( nAngle < 0 )
                nAngle = ( 36000 + nAngle ) % 36000;
            else
                nAngle = ( 36000 - ( nAngle % 36000 ) );

            nAngle *= 655;
            nAngle += 0x8000;
            nAngle &=~0xffff;   // nAngle auf volle Gradzahl runden
            aPropOpt.AddOpt( ESCHER_Prop_Rotation, nAngle );
            mpEscherEx->SetGroupSnapRect( mpEscherEx->GetGroupLevel(),
                                            rObj.GetRect() );
            mpEscherEx->SetGroupLogicRect( mpEscherEx->GetGroupLevel(),
                                            rObj.GetRect() );
        }
        rObj.SetAngle( nAngle );
        aPropOpt.CreateShapeProperties( rObj.GetShapeRef() );
        mpEscherEx->Commit( aPropOpt, rObj.GetRect() );

        // write the childanchor
        mpEscherEx->AddChildAnchor( rObj.GetRect() );

#if defined EES_WRITE_EPP
        // ClientAnchor
        mpEscherEx->AddClientAnchor( maRect );
        // ClientTextbox
        mpEscherEx->OpenContainer( ESCHER_ClientTextbox );
        mpEscherEx->AddAtom( 4, EPP_TextHeaderAtom );
        *mpStrm << (sal_uInt32)EPP_TEXTTYPE_Other;                              // Text in a Shape
        ImplWriteTextStyleAtom();
        mpEscherEx->CloseContainer();   // ESCHER_ClientTextBox
#else // !EES_WRITE_EPP
        if ( mpHostAppData )
        {   //! the App has to control whether these are written or not
            mpHostAppData->WriteClientAnchor( *mpEscherEx, rObj.GetRect() );
            mpHostAppData->WriteClientData( *mpEscherEx );
            mpHostAppData->WriteClientTextbox( *mpEscherEx );
        }
#endif // EES_WRITE_EPP
        mpEscherEx->CloseContainer();   // ESCHER_SpContainer
    } while ( 0 );
    mpEscherEx->LeaveGroup();
    mpEscherEx->EndShape( nShapeType, nShapeID );
}


sal_uInt32 ImplEESdrWriter::ImplEnterAdditionalTextGroup( const Reference< XShape >& rShape,
            const Rectangle* pBoundRect )
{
    mpHostAppData = mpEscherEx->EnterAdditionalTextGroup();
    sal_uInt32 nGrpId = mpEscherEx->EnterGroup( pBoundRect );
    mpHostAppData = mpEscherEx->StartShape( rShape, pBoundRect );
    return nGrpId;
}


sal_Bool ImplEESdrWriter::ImplInitPageValues()
{
    mnIndices = 0;
    mnOutlinerCount = 0;                // die gliederungsobjekte muessen dem layout entsprechen,
    mnEffectCount = 0;
    mbIsTitlePossible = sal_True;           // bei mehr als einem title geht powerpoint in die knie

    return sal_True;
}

void ImplEESdrWriter::ImplWritePage(
            EscherSolverContainer& rSolverContainer,
            ImplEESdrPageType ePageType, sal_Bool /* bBackGround */ )
{
    ImplInitPageValues();

    sal_uInt32 nLastPer = 0, nShapes = mXShapes->getCount();
    for( sal_uInt32 n = 0; n < nShapes; ++n )
    {
        sal_uInt32 nPer = ( 5 * n ) / nShapes;
        if( nPer != nLastPer )
        {
            nLastPer = nPer;
            sal_uInt32 nValue = mnPagesWritten * 5 + nPer;
            if( nValue > mnStatMaxValue )
                nValue = mnStatMaxValue;
            if( mbStatusIndicator )
                mXStatusIndicator->setValue( nValue );
        }

        ImplEESdrObject aObj( *this, *(Reference< XShape >*)
                                    mXShapes->getByIndex( n ).getValue() );
        if( aObj.IsValid() )
        {
            ImplWriteShape( aObj, rSolverContainer, ePageType );
        }
    }
    mnPagesWritten++;
}

ImplEscherExSdr::ImplEscherExSdr( EscherEx& rEx )
        :
        ImplEESdrWriter( rEx ),
        mpSdrPage( NULL ),
        mpSolverContainer( NULL )
{
}


ImplEscherExSdr::~ImplEscherExSdr()
{
    DBG_ASSERT( !mpSolverContainer, "ImplEscherExSdr::~ImplEscherExSdr: unwritten SolverContainer" );
    delete mpSolverContainer;
}


bool ImplEscherExSdr::ImplInitPage( const SdrPage& rPage )
{
    do
    {
        SvxDrawPage* pSvxDrawPage;
        if ( mpSdrPage != &rPage || !mXDrawPage.is() )
        {
            // eventually write SolverContainer of current page, deletes the Solver
            ImplFlushSolverContainer();

            mpSdrPage = NULL;
            mXDrawPage = pSvxDrawPage = new SvxFmDrawPage( (SdrPage*) &rPage );
            mXShapes = Reference< XShapes >::query( mXDrawPage );
            if ( !mXShapes.is() )
                break;
            if ( !ImplInitPageValues() )    // ImplEESdrWriter
                break;
            mpSdrPage = &rPage;

            mpSolverContainer = new EscherSolverContainer;
        }
        else
            pSvxDrawPage = SvxDrawPage::getImplementation(mXDrawPage);

        return pSvxDrawPage != 0;
    } while ( 0 );

    return false;
}

bool ImplEscherExSdr::ImplInitUnoShapes( const Reference< XShapes >& rxShapes )
{
    // eventually write SolverContainer of current page, deletes the Solver
    ImplFlushSolverContainer();

    if( !rxShapes.is() )
        return false;

    mpSdrPage = 0;
    mXDrawPage.clear();
    mXShapes = rxShapes;

    if( !ImplInitPageValues() )    // ImplEESdrWriter
        return false;

    mpSolverContainer = new EscherSolverContainer;
    return true;
}

void ImplEscherExSdr::ImplExitPage()
{
    // close all groups before the solver container is written
    while( mpEscherEx->GetGroupLevel() )
        mpEscherEx->LeaveGroup();

    ImplFlushSolverContainer();
    mpSdrPage = NULL;   // reset page for next init
}


void ImplEscherExSdr::ImplFlushSolverContainer()
{
    if ( mpSolverContainer )
    {
        mpSolverContainer->WriteSolver( mpEscherEx->GetStream() );
        delete mpSolverContainer;
        mpSolverContainer = NULL;
    }
}


void ImplEscherExSdr::ImplWriteCurrentPage()
{
    DBG_ASSERT( mpSolverContainer, "ImplEscherExSdr::ImplWriteCurrentPage: no SolverContainer" );
    ImplWritePage( *mpSolverContainer, NORMAL );
    ImplExitPage();
}


sal_uInt32 ImplEscherExSdr::ImplWriteTheShape( ImplEESdrObject& rObj )
{
    DBG_ASSERT( mpSolverContainer, "ImplEscherExSdr::ImplWriteShape: no SolverContainer" );
    return ImplWriteShape( rObj, *mpSolverContainer, NORMAL );
}


void EscherEx::AddSdrPage( const SdrPage& rPage )
{
    if ( mpImplEscherExSdr->ImplInitPage( rPage ) )
        mpImplEscherExSdr->ImplWriteCurrentPage();
}

void EscherEx::AddUnoShapes( const Reference< XShapes >& rxShapes )
{
    if ( mpImplEscherExSdr->ImplInitUnoShapes( rxShapes ) )
        mpImplEscherExSdr->ImplWriteCurrentPage();
}

sal_uInt32 EscherEx::AddSdrObject( const SdrObject& rObj )
{
    ImplEESdrObject aObj( *mpImplEscherExSdr, rObj );
    if( aObj.IsValid() )
        return mpImplEscherExSdr->ImplWriteTheShape( aObj );
    return 0;
}


void EscherEx::EndSdrObjectPage()
{
    mpImplEscherExSdr->ImplExitPage();
}

EscherExHostAppData* EscherEx::StartShape( const Reference< XShape >& /* rShape */, const Rectangle* /*pChildAnchor*/ )
{
    return NULL;
}

void EscherEx::EndShape( sal_uInt16 /* nShapeType */, sal_uInt32 /* nShapeID */ )
{
}

sal_uInt32 EscherEx::QueryTextID( const Reference< XShape >&, sal_uInt32 )
{
    return 0;
}

// add an dummy rectangle shape into the escher stream
sal_uInt32 EscherEx::AddDummyShape()
{
    OpenContainer( ESCHER_SpContainer );
    sal_uInt32 nShapeID = GenerateShapeId();
    AddShape( ESCHER_ShpInst_Rectangle, 0xa00, nShapeID );
    CloseContainer();

    return nShapeID;
}

// static
const SdrObject* EscherEx::GetSdrObject( const Reference< XShape >& rShape )
{
    const SdrObject* pRet = 0;
    const SvxShape* pSvxShape = SvxShape::getImplementation( rShape );
    DBG_ASSERT( pSvxShape, "EscherEx::GetSdrObject: no SvxShape" );
    if( pSvxShape )
    {
        pRet = pSvxShape->GetSdrObject();
        DBG_ASSERT( pRet, "EscherEx::GetSdrObject: no SdrObj" );
    }
    return pRet;
}


ImplEESdrObject::ImplEESdrObject( ImplEscherExSdr& rEx,
                                    const SdrObject& rObj ) :
    mnShapeId( 0 ),
    mnTextSize( 0 ),
    mnAngle( 0 ),
    mbValid( sal_False ),
    mbPresObj( sal_False ),
    mbEmptyPresObj( sal_False )
{
    SdrPage* pPage = rObj.GetPage();
    DBG_ASSERT( pPage, "ImplEESdrObject::ImplEESdrObject: no SdrPage" );
    if( pPage && rEx.ImplInitPage( *pPage ) )
    {
        // why not declare a const parameter if the object will
        // not be modified?
        mXShape = uno::Reference< drawing::XShape >::query( ((SdrObject*)&rObj)->getUnoShape() );
        Init( rEx );
    }
}

ImplEESdrObject::ImplEESdrObject( ImplEESdrWriter& rEx,
                                    const Reference< XShape >& rShape ) :
    mXShape( rShape ),
    mnShapeId( 0 ),
    mnTextSize( 0 ),
    mnAngle( 0 ),
    mbValid( sal_False ),
    mbPresObj( sal_False ),
    mbEmptyPresObj( sal_False )
{
    Init( rEx );
}


ImplEESdrObject::~ImplEESdrObject()
{
}

void ImplEESdrObject::Init( ImplEESdrWriter& rEx )
{
    mXPropSet = Reference< XPropertySet >::query( mXShape );
    if( mXPropSet.is() )
    {
        static const sal_Char aPrefix[] = "com.sun.star.";
        static const xub_StrLen nPrefix = sizeof(aPrefix)-1;
        SetRect( rEx.ImplMapPoint( Point( mXShape->getPosition().X, mXShape->getPosition().Y ) ),
                 rEx.ImplMapSize( Size( mXShape->getSize().Width, mXShape->getSize().Height ) ) );
        mType = String( mXShape->getShapeType() );
        mType.Erase( 0, nPrefix );  // strip "com.sun.star."
        xub_StrLen nPos = mType.SearchAscii( "Shape" );
        mType.Erase( nPos, 5 );

        static const OUString sPresStr("IsPresentationObject" );
        static const OUString sEmptyPresStr("IsEmptyPresentationObject" );

        if( ImplGetPropertyValue( sPresStr ) )
            mbPresObj = ::cppu::any2bool( mAny );

        if( mbPresObj && ImplGetPropertyValue( sEmptyPresStr ) )
            mbEmptyPresObj = ::cppu::any2bool( mAny );

        mbValid = sal_True;
    }
}

sal_Bool ImplEESdrObject::ImplGetPropertyValue( const sal_Unicode* rString )
{
    sal_Bool bRetValue = sal_False;
    if( mbValid )
    {
        try
        {
            mAny = mXPropSet->getPropertyValue( rString );
            if( mAny.hasValue() )
                bRetValue = sal_True;
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
            bRetValue = sal_False;
        }
    }
    return bRetValue;
}

#ifdef USED
sal_Bool ImplEESdrObject::ImplGetPropertyValue( const Reference< XPropertySet >& rXPropSet,
                                            const OUString& rString )
{
    sal_Bool bRetValue = sal_False;
    if( mbValid )
    {
        try
        {
            mAny = rXPropSet->getPropertyValue( rString );
            if( 0 != mAny.get() )
                bRetValue = sal_True;
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
            bRetValue = sal_False;
        }
    }
    return bRetValue;
}
#endif

void ImplEESdrObject::SetRect( const Point& rPos, const Size& rSz )
{
    maRect = Rectangle( rPos, rSz );
}

const SdrObject* ImplEESdrObject::GetSdrObject() const
{
    return EscherEx::GetSdrObject( mXShape );
}

//  loads and  converts text from shape, result is being saved in mnTextSize respeichert
sal_uInt32 ImplEESdrObject::ImplGetText()
{
    Reference< XText > xXText( mXShape, UNO_QUERY );
    mnTextSize = 0;
    if( xXText.is() )
        mnTextSize = xXText->getString().getLength();
    return mnTextSize;
}

sal_Bool ImplEESdrObject::ImplHasText() const
{
    Reference< XText > xXText( mXShape, UNO_QUERY );
    return xXText.is() && !xXText->getString().isEmpty();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
