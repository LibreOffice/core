/*************************************************************************
 *
 *  $RCSfile: eschesdo.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: sj $ $Date: 2001-03-09 13:53:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <math.h>

#ifndef _ESCHESDO_HXX
#include "eschesdo.hxx"
#endif

#ifndef _SVDOBJ_HXX //autogen wg. SdrObject
#include "svdobj.hxx"
#endif

#ifndef _SVX_UNOSHAPE_HXX //autogen wg. SvxShape
#include "unoshape.hxx"
#endif

#ifndef _SV_OUTDEV_HXX //autogen wg. OutputDevice
#include <vcl/outdev.hxx>
#endif
#ifndef _SV_POLY_HXX //autogen wg. PolyFlags
#include <vcl/poly.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX //autogen wg. BitmapEx
#include <vcl/bitmapex.hxx>
#endif
#ifndef _SV_GRAPH_HXX //autogen wg. Graphic
#include <vcl/graph.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif

#ifndef _SVX_FMDPAGE_HXX //autogen wg. SvxFmDrawPage
#include <fmdpage.hxx>
#endif

#ifndef _TOOLKIT_UNOHLP_HXX //autogen wg. VCLXBitmap
#include <toolkit/unohlp.hxx>
#endif

#ifndef _COM_SUN_STAR_STYLE_VERTICALALIGNMENT_HPP_
#include <com/sun/star/style/VerticalAlignment.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCESEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONBEZIERCOORDS_HPP_
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FLAGSEQUENCE_HPP_
#include <com/sun/star/drawing/FlagSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTADJUST_HPP_
#include <com/sun/star/drawing/TextAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_CIRCLEKIND_HPP_
#include <com/sun/star/drawing/CircleKind.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTVERTICALADJUST_HPP_
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTHORIZONTALADJUST_HPP_
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#endif

#include <cppuhelper/extract.hxx>

#ifndef _XCEPTION_HXX_ //autogen wg. TRY
#include <vos/xception.hxx>
#endif

#ifndef _FLTCALL_HXX
#include <svtools/fltcall.hxx>
#endif
#ifndef _SV_CVTGRF_HXX
#include <vcl/cvtgrf.hxx>
#endif

using namespace ::vos;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::style;

BOOL ConvertGDIMetaFileToEMF( const GDIMetaFile & rMTF, SvStream & rTargetStream,
                              PFilterCallback pCallback, void * pCallerData );

#define EES_MAP_FRACTION 1440   // 1440 dpi

// ===================================================================

ImplEESdrWriter::ImplEESdrWriter( EscherEx& rEx )
        :
        mpEscherEx              ( &rEx ),
        maMapModeSrc            ( MAP_100TH_MM ),
        // PowerPoint: 576 dpi, WinWord: 1440 dpi, Excel: 1440 dpi
        maMapModeDest( MAP_INCH, Point(), Fraction( 1, EES_MAP_FRACTION ), Fraction( 1, EES_MAP_FRACTION ) ),
        mbStatus                ( FALSE ),
//      mXModel                 ( rXModel ),
//      mXStatusIndicator       ( rXStatInd ),
        mbStatusIndicator       ( FALSE ),
        mnPagesWritten          ( 0 ),
//      mnTxId                  ( 0x7a2f64 ),
        mnShapeMasterTitle      ( 0 ),
        mnShapeMasterBody       ( 0 ),
        mpPicStrm               ( NULL ),
        mpHostAppData           ( NULL )
{
}


// -------------------------------------------------------------------

Point ImplEESdrWriter::ImplMapPoint( const Point& rPoint )
{
    return OutputDevice::LogicToLogic( rPoint, maMapModeSrc, maMapModeDest );
}


// -------------------------------------------------------------------

Size ImplEESdrWriter::ImplMapSize( const Size& rSize )
{
    Size aRetSize( OutputDevice::LogicToLogic( rSize, maMapModeSrc, maMapModeDest ) );

    if ( !aRetSize.Width() )
        aRetSize.Width()++;
    if ( !aRetSize.Height() )
        aRetSize.Height()++;
    return aRetSize;
}


// -------------------------------------------------------------------

void ImplEESdrWriter::ImplWriteTextBundle( ImplEESdrObject& rObj, EscherPropertyContainer& rPropOpt )
{
    if( rObj.ImplGetText() )
    {
        ESCHER_AnchorText   eAnchor = ESCHER_AnchorTop;
        UINT32              nTextAttr = 0x40004;    // rotate text with shape


        if ( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("TextVerticalAdjust") ) )
        {
            ::com::sun::star::drawing::TextVerticalAdjust eVA;
            rObj.GetUsrAny() >>= eVA;
            switch ( eVA )
            {
                case 1 :    // ::com::sun::star::drawing::TextVerticalAdjust_CENTER :
                    eAnchor = ESCHER_AnchorMiddle;
                break;

                case 2 :    // ::com::sun::star::drawing::TextVerticalAdjust_BOTTOM :
                    eAnchor = ESCHER_AnchorBottom;
                break;

                default :
                case 0 :    // ::com::sun::star::drawing::TextVerticalAdjust_TOP :
                    eAnchor = ESCHER_AnchorTop;
                break;
            }
        }
        if( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("TextHorizontalAdjust") ) )
        {
            ::com::sun::star::drawing::TextHorizontalAdjust eTA;
            rObj.GetUsrAny() >>= eTA;
            switch ( eTA )
            {
                case 1 :    // ::com::sun::star::drawing::TextHorizontalAdjust_CENTER :
                case 2 :    // ::com::sun::star::drawing::TextHorizontalAdjust_RIGHT :
                case 0 :    // ::com::sun::star::drawing::TextHorizontalAdjust_LEFT :
                {
                    switch( eAnchor )
                    {
                        case ESCHER_AnchorMiddle :
                            eAnchor = ESCHER_AnchorMiddleCentered;
                        break;
                        case ESCHER_AnchorBottom :
                            eAnchor = ESCHER_AnchorBottomCentered;
                        break;
                        case ESCHER_AnchorTop :
                            eAnchor = ESCHER_AnchorTopCentered;
                        break;
                    }
                }
                break;
                case 3 :    // ::com::sun::star::drawing::TextHorizontalAdjust_BLOCK :
                 break;
            }
        }
/*
        if ( ImplGetPropertyValue( L"TextFitToSize" ) )
        {
            if ( *( (INT16*)mAny.get() ) == 1 )
            {
                nTextAttr |= 0x10001;
                rPropOpt.AddOpt( ESCHER_Prop_scaleText, ? );
            }
        }
*/

        INT32 nLeft = rObj.ImplGetInt32PropertyValue( ::rtl::OUString::createFromAscii("TextLeftDistance"));
        INT32 nTop = rObj.ImplGetInt32PropertyValue( ::rtl::OUString::createFromAscii("TextUpperDistance"));
        INT32 nRight = rObj.ImplGetInt32PropertyValue( ::rtl::OUString::createFromAscii("TextRightDistance"));
        INT32 nBottom = rObj.ImplGetInt32PropertyValue( ::rtl::OUString::createFromAscii("TextLowerDistance"));

        if ( nLeft >= 200 )
            nLeft -= 200;
        if ( nRight >= 200 )
            nRight -=200;

        rPropOpt.AddOpt( ESCHER_Prop_dxTextLeft, nLeft * 360 );
        rPropOpt.AddOpt( ESCHER_Prop_dxTextRight, nRight * 360 );
        rPropOpt.AddOpt( ESCHER_Prop_dyTextTop, nTop * 360 );
        rPropOpt.AddOpt( ESCHER_Prop_dyTextBottom, nBottom * 360 );

        rPropOpt.AddOpt( ESCHER_Prop_WrapText, ESCHER_WrapSquare );
        rPropOpt.AddOpt( ESCHER_Prop_AnchorText, eAnchor );
        rPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, nTextAttr );
        UINT32 nTxtBxId = mpEscherEx->QueryTextID( rObj.GetShapeRef(),
                                                    rObj.GetShapeId() );
        rPropOpt.AddOpt( ESCHER_Prop_lTxid, nTxtBxId );
    }
}


// -------------------------------------------------------------------

void ImplEESdrWriter::ImplFlipBoundingBox( ImplEESdrObject& rObj, EscherPropertyContainer& rPropOpt,
                                            const Point& rRefPoint )
{
    INT32 nAngle = rObj.GetAngle();
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
    nShapeID = mpEscherEx->GetShapeID();                        \
    rObj.SetShapeId( nShapeID );                                \
    mpEscherEx->AddShape( (UINT32)nType, (UINT32)nFlags, nShapeID );    \
    rSolverContainer.AddShape( rObj.GetShapeRef(), nShapeID );  \
}

#define SHAPE_TEXT( bFill )                                         \
{                                                                   \
    mpEscherEx->OpenContainer( ESCHER_SpContainer );                \
    ADD_SHAPE( ESCHER_ShpInst_TextBox, 0xa00 );                     \
    if ( bFill )                                                    \
        aPropOpt.CreateFillProperties( rObj.mXPropSet, sal_True );  \
    ImplWriteTextBundle( rObj, aPropOpt );                          \
}

UINT32 ImplEESdrWriter::ImplWriteShape( ImplEESdrObject& rObj,
                                EscherSolverContainer& rSolverContainer,
                                ImplEESdrPageType ePageType )
{
    UINT32 nShapeID = 0;
    UINT16 nShapeType = 0;
    BOOL bAdditionalText = FALSE;
    UINT32 nGrpShapeID = 0;

    do {
        mpHostAppData = mpEscherEx->StartShape( rObj.GetShapeRef() );
        if ( mpHostAppData && mpHostAppData->DontWriteShape() )
            break;

        Point aTextRefPoint;

        if( rObj.GetType().EqualsAscii( "drawing.Group" ))
        {
            Reference< XIndexAccess > xXIndexAccess( rObj.GetShapeRef(), UNO_QUERY );

            if( xXIndexAccess.is() && 0 != xXIndexAccess->getCount() )
            {
                nShapeID = mpEscherEx->EnterGroup( &rObj.GetRect() );
                nShapeType = ESCHER_ShpInst_Min;

                for( UINT32 n = 0, nCnt = xXIndexAccess->getCount();
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
        rObj.SetAngle( rObj.ImplGetInt32PropertyValue( ::rtl::OUString::createFromAscii("RotateAngle") ));

        if( ( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("IsFontwork") ) &&
            ::cppu::any2bool( rObj.GetUsrAny() ) ) ||
            rObj.GetType().EqualsAscii( "drawing.Measure" ))
        {
            if( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("BoundRect") ) )
            {
                Rectangle aRect( *(Rectangle*)rObj.GetUsrAny().getValue() );
                rObj.SetRect( ImplMapPoint( aRect.TopLeft() ),
                              ImplMapSize( aRect.GetSize() ) );
            }
            rObj.SetType( String( RTL_CONSTASCII_STRINGPARAM(
                                "drawing.dontknow" ),
                                RTL_TEXTENCODING_MS_1252 ));
        }

        const ::com::sun::star::awt::Size   aSize100thmm( rObj.GetShapeRef()->getSize() );
        const ::com::sun::star::awt::Point  aPoint100thmm( rObj.GetShapeRef()->getPosition() );
        Rectangle   aRect100thmm( Point( aPoint100thmm.X, aPoint100thmm.Y ), Size( aSize100thmm.Width, aSize100thmm.Height ) );
        if ( !mpPicStrm )
            mpPicStrm = mpEscherEx->QueryPicStream();
        EscherPropertyContainer aPropOpt( (EscherGraphicProvider&)*mpEscherEx, mpPicStrm, aRect100thmm );
        if ( rObj.GetType().EqualsAscii( "drawing.Rectangle" ))
        {
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            sal_Int32 nRadius = (sal_Int32)rObj.ImplGetInt32PropertyValue(
                                            ::rtl::OUString::createFromAscii("CornerRadius"));
            if( nRadius )
            {
                nRadius = ImplMapSize( Size( nRadius, 0 )).Width();
                ADD_SHAPE( ESCHER_ShpInst_RoundRectangle, 0xa00 );  // Flags: Connector | HasSpt
                INT32 nLenght = rObj.GetRect().GetWidth();
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
            ImplWriteTextBundle( rObj, aPropOpt );
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.Ellipse" ))
        {
            CircleKind  eCircleKind = CircleKind_FULL;
            PolyStyle   ePolyKind;
            if ( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("CircleKind") ) )
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
                aPropOpt.CreateFillProperties( rObj.mXPropSet, sal_True );;
            }
            else
            {
                INT32 nStartAngle, nEndAngle;
                if ( !rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("CircleStartAngle") ) )
                    break;
                nStartAngle = *( (INT32*)rObj.GetUsrAny().getValue() );
                if( !rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("CircleEndAngle") ) )
                    break;
                nEndAngle = *( (INT32*)rObj.GetUsrAny().getValue() );

                Point aStart, aEnd, aCenter;
                aStart.X() = (INT32)( ( cos( (double)( nStartAngle *
                                                F_PI18000 ) ) * 100.0 ) );
                aStart.Y() = - (INT32)( ( sin( (double)( nStartAngle *
                                                F_PI18000 ) ) * 100.0 ) );
                aEnd.X() = (INT32)( ( cos( (double)( nEndAngle *
                                                F_PI18000 ) ) * 100.0 ) );
                aEnd.Y() = - (INT32)( ( sin( (double)( nEndAngle *
                                                F_PI18000 ) ) * 100.0 ) );
                const Rectangle& rRect = rObj.GetRect();
                aCenter.X() = rRect.Left() + ( rRect.GetWidth() / 2 );
                aCenter.Y() = rRect.Top() + ( rRect.GetHeight() / 2 );
                aStart.X() += aCenter.X();
                aStart.Y() += aCenter.Y();
                aEnd.X() += aCenter.X();
                aEnd.Y() += aCenter.Y();
                Polygon aPolygon( rRect, aStart, aEnd, ePolyKind );
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
            ImplWriteTextBundle( rObj, aPropOpt );
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
/*
            if ( ImplGetPropertyValue( L"MeasureKind" ) )
            {
                mpEscherEx->EnterGroup( &maRect );
                mpEscherEx->OpenContainer( ESCHER_SpContainer );
                ImplWriteAny( ANY_FLAGS_LINE, FALSE );
                UINT32 nFlags = 0xa00;                                          // Flags: Connector | HasSpt
                if ( maRect.Top() > maRect.Bottom() )
                    nFlags |= 0x80;                                             // Flags: VertMirror
                if ( maRect.Left() > maRect.Right() )
                    nFlags |= 0x40;                                             // Flags: HorzMirror

                ADD_SHAPE( ESCHER_ShpInst_Line, nFlags );
                aPropOpt.AddOpt( ESCHER_Prop_shapePath, ESCHER_ShapeComplex );
                aPropOpt.CreateLineProperties( rObj.mXPropSet, sal_False );
                mpEscherEx->EndCount( ESCHER_OPT, 3 );
                maRect.Justify();
                mpEscherEx->AddClientAnchor( maRect );
                mpEscherEx->CloseContainer();           // ESCHER_SpContainer

                if ( ImplGetPropertyValue( L"MeasureTextHorizontalPosition" ) )
                {
                }
                if ( ImplGetPropertyValue( L"MeasureTextVerticalPosition" ) )
                {
                }
                if ( ImplGetPropertyValue( L"MeasureLineDistance" ) )
                {
                }
                if ( ImplGetPropertyValue( L"MeasureHelpLineOverhang" ) )
                {
                }
                if ( ImplGetPropertyValue( L"MeasureHelpLineDistance" ) )
                {
                }
                if ( ImplGetPropertyValue( L"MeasureHelpLine1Length" ) )
                {
                }
                if ( ImplGetPropertyValue( L"MeasureHelpLine2Length" ) )
                {
                }
                if ( ImplGetPropertyValue( L"MeasureBelowReferenceEdge" ) )
                {
                }
                if ( ImplGetPropertyValue( L"MeasureTextRotate90" ) )
                {
                }
                if ( ImplGetPropertyValue( L"MeasureTextUpsideDown" ) )
                {
                }
                if ( ImplGetPropertyValue( L"MeasureOverhang" ) )
                {
                }
                if ( ImplGetPropertyValue( L"MeasureUnit" ) )
                {
                }
                if ( ImplGetPropertyValue( L"MeasureScale" ) )
                {
                }
                if ( ImplGetPropertyValue( L"MeasureShowUnit" ) )
                {
                }
                if ( ImplGetPropertyValue( L"MeasureFormatString" ) )
                {
                }
                if ( ImplGetPropertyValue( L"MeasureTextAutoAngle" ) )
                {
                }
                if ( ImplGetPropertyValue( L"MeasureTextAutoAngleView" ) )
                {
                }
                if ( ImplGetPropertyValue( L"MeasureTextIsFixedAngle" ) )
                {
                }
                if ( ImplGetPropertyValue( L"MeasureTextFixedAngle" ) )
                {
                }
                mpEscherEx->LeaveGroup();
            }
*/
            break;
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.Line" ))
        {
            ::com::sun::star::awt::Rectangle aNewRect;
            aPropOpt.CreatePolygonProperties( rObj.mXPropSet, ESCHER_CREATEPOLYGON_LINE, sal_False, aNewRect, NULL );
            rObj.SetRect( Rectangle( ImplMapPoint( Point( aNewRect.X, aNewRect.Y ) ),
                                        ImplMapSize( Size( aNewRect.Width, aNewRect.Height ) ) ) );
            if( rObj.ImplHasText() )
            {
                aTextRefPoint = rObj.GetRect().TopLeft();
                bAdditionalText = TRUE;
                nGrpShapeID = ImplEnterAdditionalTextGroup( rObj.GetShapeRef(), &rObj.GetRect() );
            }
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            const Rectangle& rRect = rObj.GetRect();
            UINT32 nFlags = 0xa00;          // Flags: Connector | HasSpt
            if( rRect.Top() > rRect.Bottom() )
                nFlags |= 0x80;             // Flags: VertMirror
            if( rRect.Left() > rRect.Right() )
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
                bAdditionalText = TRUE;
            }
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            ADD_SHAPE( ESCHER_ShpInst_NotPrimitive, 0xa00 );        // Flags: Connector | HasSpt
            ::com::sun::star::awt::Rectangle aNewRect;
            aPropOpt.CreatePolygonProperties( rObj.mXPropSet, ESCHER_CREATEPOLYGON_POLYPOLYGON, sal_False, aNewRect, NULL );
            rObj.SetRect( Rectangle( ImplMapPoint( Point( aNewRect.X, aNewRect.Y ) ),
                                        ImplMapSize( Size( aNewRect.Width, aNewRect.Height ) ) ) );
            aPropOpt.CreateFillProperties( rObj.mXPropSet, sal_True );
            rObj.SetAngle( 0 );
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.PolyLine" ))
        {
            if( rObj.ImplHasText() )
            {
                nGrpShapeID = ImplEnterAdditionalTextGroup( rObj.GetShapeRef(), &rObj.GetRect() );
                bAdditionalText = TRUE;
            }
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            ADD_SHAPE( ESCHER_ShpInst_NotPrimitive, 0xa00 );        // Flags: Connector | HasSpt
            ::com::sun::star::awt::Rectangle aNewRect;
            aPropOpt.CreatePolygonProperties( rObj.mXPropSet, ESCHER_CREATEPOLYGON_POLYLINE, sal_False, aNewRect, NULL );
            rObj.SetRect( Rectangle( ImplMapPoint( Point( aNewRect.X, aNewRect.Y ) ),
                                        ImplMapSize( Size( aNewRect.Width, aNewRect.Height ) ) ) );
            aPropOpt.CreateLineProperties( rObj.mXPropSet, sal_False );
            rObj.SetAngle( 0 );
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.OpenBezier" ) ||
                  rObj.GetType().EqualsAscii( "drawing.OpenFreeHand" ) ||
                  rObj.GetType().EqualsAscii( "drawing.PolyLinePath" ) )
        {
            if ( rObj.ImplHasText() )
            {
                nGrpShapeID = ImplEnterAdditionalTextGroup( rObj.GetShapeRef(), &rObj.GetRect() );
                bAdditionalText = TRUE;
            }
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            ADD_SHAPE( ESCHER_ShpInst_NotPrimitive, 0xa00 );        // Flags: Connector | HasSpt
            ::com::sun::star::awt::Rectangle aNewRect;
            aPropOpt.CreatePolygonProperties( rObj.mXPropSet, ESCHER_CREATEPOLYGON_POLYLINE, sal_True, aNewRect, NULL );
            rObj.SetRect( Rectangle( ImplMapPoint( Point( aNewRect.X, aNewRect.Y ) ),
                                        ImplMapSize( Size( aNewRect.Width, aNewRect.Height ) ) ) );
            aPropOpt.CreateLineProperties( rObj.mXPropSet, sal_False );
            rObj.SetAngle( 0 );
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.ClosedBezier" ) ||
                  rObj.GetType().EqualsAscii( "drawing.ClosedFreeHand" ) ||
                  rObj.GetType().EqualsAscii( "drawing.PolyPolygonPath" ) )
        {
            if ( rObj.ImplHasText() )
            {
                nGrpShapeID = ImplEnterAdditionalTextGroup( rObj.GetShapeRef(), &rObj.GetRect() );
                bAdditionalText = TRUE;
            }
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            ADD_SHAPE( ESCHER_ShpInst_NotPrimitive, 0xa00 );        // Flags: Connector | HasSpt
            ::com::sun::star::awt::Rectangle aNewRect;
            aPropOpt.CreatePolygonProperties( rObj.mXPropSet, ESCHER_CREATEPOLYGON_POLYPOLYGON, sal_True, aNewRect, NULL );
            rObj.SetRect( Rectangle( ImplMapPoint( Point( aNewRect.X, aNewRect.Y ) ),
                                        ImplMapSize( Size( aNewRect.Width, aNewRect.Height ) ) ) );
            aPropOpt.CreateFillProperties( rObj.mXPropSet, sal_True );
            rObj.SetAngle( 0 );
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.GraphicObject" ))
        {
            mpEscherEx->OpenContainer( ESCHER_SpContainer );

            // ein GraphicObject kann auch ein ClickMe Element sein
            if( rObj.IsEmptyPresObj() && ( ePageType == NORMAL ) )
            {
                ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0x220 );               // Flags: HaveAnchor | HaveMaster
                UINT32 nTxtBxId = mpEscherEx->QueryTextID( rObj.GetShapeRef(),
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
                    ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0xa00 );           // Flags: Connector | HasSpt
                    if ( aPropOpt.CreateGraphicProperties( rObj.mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "GraphicURL" ) ), sal_True ) )
                    {
                        aPropOpt.AddOpt( ESCHER_Prop_WrapText, ESCHER_WrapNone );
                        aPropOpt.AddOpt( ESCHER_Prop_AnchorText, ESCHER_AnchorMiddle );
                        aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x140014 );
                        aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0x8000000 );
                        aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x80000 );
                        ImplWriteTextBundle( rObj, aPropOpt );
                    }
                }
                else
                {
                    ADD_SHAPE( ESCHER_ShpInst_PictureFrame, 0xa00 );
                    if ( aPropOpt.CreateGraphicProperties( rObj.mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "GraphicURL" ) ), sal_False ) )
                        aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
                }
            }
        }
        else if ( rObj.GetType().EqualsAscii(  "drawing.Text" ))
        {
            SHAPE_TEXT( TRUE );
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
                UINT32 nTxtBxId = mpEscherEx->QueryTextID( rObj.GetShapeRef(),
                                                        rObj.GetShapeId() );
                aPropOpt.AddOpt( ESCHER_Prop_lTxid, nTxtBxId );
                aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x10001 );
                aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x10001 );
                aPropOpt.AddOpt( ESCHER_Prop_hspMaster, mnShapeMasterBody );
            }
            else
            {
                //2do: could be made an option in HostAppData whether OLE object should be written or not
                BOOL bAppOLE = TRUE;
                ADD_SHAPE( ESCHER_ShpInst_PictureFrame,
                    0xa00 | (bAppOLE ? SHAPEFLAG_OLESHAPE : 0) );
                if ( aPropOpt.CreateGraphicProperties( rObj.mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "MetaFile" ) ), sal_False ) )
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
//                      aPropOpt.AddOpt( ESCHER_Prop_fshadowObscured,0x00020000 );
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
            if ( !rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("Bitmap") ) )
                break;

            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            ADD_SHAPE( ESCHER_ShpInst_PictureFrame, 0xa00 );

                if ( aPropOpt.CreateGraphicProperties( rObj.mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Bitmap" ) ), sal_False ) )
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
            rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("LayerID") ) &&
            (*((UINT16*)rObj.GetUsrAny().getValue()) ) == mpEscherEx->GetHellLayerId() )
        {
            aPropOpt.AddOpt( ESCHER_Prop_fPrint, 0x200020 );
        }

        {
            Rectangle aRect( rObj.GetRect() );
            aRect.Justify();
            rObj.SetRect( aRect );
        }

        if( rObj.GetAngle() )
        {
            if( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("RotationPointX") ))
            {
                Point aRefPoint( *( (INT32*)rObj.GetUsrAny().getValue() ), 0 );
                if( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("RotationPointY") ))
                {
                    aRefPoint.Y() = *( (INT32*)rObj.GetUsrAny().getValue() );
                    ImplFlipBoundingBox( rObj, aPropOpt, ImplMapPoint( aRefPoint ) );
                }
            }
        }
        aPropOpt.Commit( mpEscherEx->GetStream() );
        if( mpEscherEx->GetGroupLevel() > 1 )
        {
            mpEscherEx->AddAtom( 16, ESCHER_ChildAnchor );
            const Rectangle& rRect = rObj.GetRect();
            mpEscherEx->GetStream() << (INT32)rRect.Left()
                                    << (INT32)rRect.Top()
                                       << (INT32)rRect.Right()
                                    << (INT32)rRect.Bottom();
        }
        if ( mpHostAppData )
        {   //! with AdditionalText the App has to control whether these are written or not
            mpHostAppData->WriteClientAnchor( *mpEscherEx, rObj.GetRect() );
            mpHostAppData->WriteClientData( *mpEscherEx );
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
    UINT32 nShapeID = 0;
    UINT16 nShapeType = 0;
    do
    {
        mpHostAppData = mpEscherEx->StartShape( rObj.GetShapeRef() );
        if ( mpHostAppData && mpHostAppData->DontWriteShape() )
            break;

        const ::com::sun::star::awt::Size   aSize100thmm( rObj.GetShapeRef()->getSize() );
        const ::com::sun::star::awt::Point  aPoint100thmm( rObj.GetShapeRef()->getPosition() );
        Rectangle   aRect100thmm( Point( aPoint100thmm.X, aPoint100thmm.Y ), Size( aSize100thmm.Width, aSize100thmm.Height ) );
        if ( !mpPicStrm )
            mpPicStrm = mpEscherEx->QueryPicStream();
        EscherPropertyContainer aPropOpt( (EscherGraphicProvider&)*mpEscherEx, mpPicStrm, aRect100thmm );
        rObj.SetAngle( rObj.ImplGetInt32PropertyValue( ::rtl::OUString::createFromAscii("RotateAngle")));
        INT32 nAngle = rObj.GetAngle();
        if( rObj.GetType().EqualsAscii( "drawing.Line" ))
        {
//2do: this does not work right
            double fDist = hypot( rObj.GetRect().GetWidth(),
                                    rObj.GetRect().GetHeight() );
            rObj.SetRect( Rectangle( rTextRefPoint,
                            Point( (sal_Int32)( rTextRefPoint.X() + fDist ), rTextRefPoint.Y() - 1 ) ) );

            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            mpEscherEx->AddShape( ESCHER_ShpInst_TextBox, 0xa00 );
            ImplWriteTextBundle( rObj, aPropOpt );

            aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90000 );
            aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100000 );
            aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, 0x60006 );     // Size Shape To Fit Text
            if ( nAngle < 0 )
                nAngle = ( 36000 + nAngle ) % 36000;
            if ( nAngle )
                ImplFlipBoundingBox( rObj, aPropOpt, rObj.GetRect().TopLeft() );
        }
        else
        {
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            nShapeID = mpEscherEx->GetShapeID();
            mpEscherEx->AddShape( nShapeType = ESCHER_ShpInst_TextBox, 0xa00, nShapeID );
            ImplWriteTextBundle( rObj, aPropOpt );

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
        aPropOpt.Commit( mpEscherEx->GetStream() );

        // write the childanchor
        mpEscherEx->AddAtom( 16, ESCHER_ChildAnchor );
        const Rectangle& rRect = rObj.GetRect();
        mpEscherEx->GetStream() << (INT32)rRect.Left()
                                << (INT32)rRect.Top()
                                   << (INT32)rRect.Right()
                                << (INT32)rRect.Bottom();

#if EES_WRITE_EPP
        // ClientAnchor
        mpEscherEx->AddClientAnchor( maRect );
        // ClientTextbox
        mpEscherEx->OpenContainer( ESCHER_ClientTextbox );
        mpEscherEx->AddAtom( 4, EPP_TextHeaderAtom );
        *mpStrm << (UINT32)EPP_TEXTTYPE_Other;                              // Text in a Shape
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


// -------------------------------------------------------------------

UINT32 ImplEESdrWriter::ImplEnterAdditionalTextGroup( const Reference< XShape >& rShape,
            const Rectangle* pBoundRect )
{
    mpHostAppData = mpEscherEx->EnterAdditionalTextGroup();
    UINT32 nGrpId = mpEscherEx->EnterGroup( pBoundRect );
    mpHostAppData = mpEscherEx->StartShape( rShape );
    return nGrpId;
}


// -------------------------------------------------------------------

BOOL ImplEESdrWriter::ImplInitPageValues()
{
    mnIndices = 0;
    mnOutlinerCount = 0;                // die gliederungsobjekte muessen dem layout entsprechen,
    mnEffectCount = 0;
    mbIsTitlePossible = TRUE;           // bei mehr als einem title geht powerpoint in die knie

    return TRUE;
}


// -------------------------------------------------------------------

void ImplEESdrWriter::ImplWritePage(
            EscherSolverContainer& rSolverContainer,
            ImplEESdrPageType ePageType, BOOL bBackGround )
{
    ImplInitPageValues();

    UINT32 nLastPer = 0, nShapes = mXShapes->getCount();
    for( UINT32 n = 0; n < nShapes; ++n )
    {
        UINT32 nPer = ( 5 * n ) / nShapes;
        if( nPer != nLastPer )
        {
            nLastPer = nPer;
            UINT32 nValue = mnPagesWritten * 5 + nPer;
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

// ===================================================================

ImplEscherExSdr::ImplEscherExSdr( EscherEx& rEx )
        :
        ImplEESdrWriter( rEx ),
        mpSdrPage( NULL ),
        mpSolverContainer( NULL )
{
}


// -------------------------------------------------------------------

ImplEscherExSdr::~ImplEscherExSdr()
{
    DBG_ASSERT( !mpSolverContainer, "ImplEscherExSdr::~ImplEscherExSdr: unwritten SolverContainer" );
    delete mpSolverContainer;
}


// -------------------------------------------------------------------

SvxDrawPage* ImplEscherExSdr::ImplInitPage( const SdrPage& rPage )
{
    do
    {
        SvxDrawPage* pSvxDrawPage;
        if ( mpSdrPage != &rPage || !mXDrawPage.is() )
        {
            // eventually write SolverContainer of current page, deletes the Solver
            ImplFlushSolverContainer();

            mpSdrPage = NULL;
            // why not declare a const parameter if the object will not be modified?
//          mXDrawPage = pSvxDrawPage = new SvxDrawPage( (SdrPage*) &rPage );
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

        return pSvxDrawPage;
    } while ( 0 );

    return NULL;
}


// -------------------------------------------------------------------

void ImplEscherExSdr::ImplExitPage()
{
    ImplFlushSolverContainer();
    mpSdrPage = NULL;   // reset page for next init
}


// -------------------------------------------------------------------

void ImplEscherExSdr::ImplFlushSolverContainer()
{
    if ( mpSolverContainer )
    {
        mpSolverContainer->WriteSolver( mpEscherEx->GetStream() );
        delete mpSolverContainer;
        mpSolverContainer = NULL;
    }
}


// -------------------------------------------------------------------

void ImplEscherExSdr::ImplWriteCurrentPage()
{
    DBG_ASSERT( mpSolverContainer, "ImplEscherExSdr::ImplWriteCurrentPage: no SolverContainer" );
    ImplWritePage( *mpSolverContainer, NORMAL );
    ImplExitPage();
}


// -------------------------------------------------------------------

UINT32 ImplEscherExSdr::ImplWriteTheShape( ImplEESdrObject& rObj )
{
    DBG_ASSERT( mpSolverContainer, "ImplEscherExSdr::ImplWriteShape: no SolverContainer" );
    return ImplWriteShape( rObj, *mpSolverContainer, NORMAL );
}


// ===================================================================

void EscherEx::AddSdrPage( const SdrPage& rPage )
{
    if ( mpImplEscherExSdr->ImplInitPage( rPage ) )
        mpImplEscherExSdr->ImplWriteCurrentPage();
}


// -------------------------------------------------------------------

UINT32 EscherEx::AddSdrObject( const SdrObject& rObj )
{
    ImplEESdrObject aObj( *mpImplEscherExSdr, rObj );
    if( aObj.IsValid() )
        return mpImplEscherExSdr->ImplWriteTheShape( aObj );
    return 0;
}


// -------------------------------------------------------------------

void EscherEx::EndSdrObjectPage()
{
    mpImplEscherExSdr->ImplExitPage();
}

// -------------------------------------------------------------------

EscherExHostAppData* EscherEx::StartShape( const Reference< XShape >& rShape )
{
    return NULL;
}

// -------------------------------------------------------------------

void EscherEx::EndShape( UINT16 nShapeType, UINT32 nShapeID )
{
}

// -------------------------------------------------------------------

SvStream* EscherEx::QueryPicStream()
{
    return NULL;
}

// -------------------------------------------------------------------

UINT32 EscherEx::QueryTextID( const Reference< XShape >&, UINT32 )
{
    return 0;
}

// -------------------------------------------------------------------
// add an dummy rectangle shape into the escher stream
UINT32 EscherEx::AddDummyShape( const SdrObject& rObj )
{
    OpenContainer( ESCHER_SpContainer );
    UINT32 nShapeID = GetShapeID();
    AddShape( ESCHER_ShpInst_Rectangle, 0xa00, nShapeID );
//??    aSolverContainer.AddShape( mXShape, nShapeID );
    CloseContainer();

    return nShapeID;
}

// -------------------------------------------------------------------

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


// -------------------------------------------------------------------

ImplEESdrObject::ImplEESdrObject( ImplEscherExSdr& rEx,
                                    const SdrObject& rObj )
    : mbValid( FALSE ), mbPresObj( FALSE ), mbEmptyPresObj( FALSE ),
    mnShapeId( 0 ), mnAngle( 0 ), mnTextSize( 0 )
{
    SdrPage* pPage = rObj.GetPage();
    DBG_ASSERT( pPage, "ImplEESdrObject::ImplEESdrObject: no SdrPage" );
    if( pPage )
    {
        SvxDrawPage* pSvxDrawPage = rEx.ImplInitPage( *pPage );
        if( pSvxDrawPage )
        {
            // why not declare a const parameter if the object will
            // not be modified?
            mXShape = uno::Reference< drawing::XShape >::query( ((SdrObject*)&rObj)->getUnoShape() );;
            Init( rEx );
        }
    }
}

ImplEESdrObject::ImplEESdrObject( ImplEESdrWriter& rEx,
                                    const Reference< XShape >& rShape )
    : mbValid( FALSE ), mbPresObj( FALSE ), mbEmptyPresObj( FALSE ),
    mnShapeId( 0 ), mnAngle( 0 ), mXShape( rShape ),
    mnTextSize( 0 )
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

        static const OUString sPresStr(rtl::OUString::createFromAscii("IsPresentationObject"));
        static const OUString sEmptyPresStr(rtl::OUString::createFromAscii("IsEmptyPresentationObject"));

        if( ImplGetPropertyValue( sPresStr ) )
            mbPresObj = ::cppu::any2bool( mAny );

        if( mbPresObj && ImplGetPropertyValue( sEmptyPresStr ) )
            mbEmptyPresObj = ::cppu::any2bool( mAny );

        mbValid = TRUE;
    }
}

//BOOL ImplEESdrObject::ImplGetPropertyValue( const OUString& rString )
BOOL ImplEESdrObject::ImplGetPropertyValue( const sal_Unicode* rString )
{
    BOOL bRetValue = FALSE;
    if( mbValid )
    {
        try
        {
            mAny = mXPropSet->getPropertyValue( rString );
            if( mAny.hasValue() )
                bRetValue = TRUE;
        }
        catch( ::com::sun::star::uno::Exception& )
        {
            bRetValue = FALSE;
        }
    }
    return bRetValue;
}

#ifdef USED
BOOL ImplEESdrObject::ImplGetPropertyValue( const Reference< XPropertySet >& rXPropSet,
                                            const OUString& rString )
{
    BOOL bRetValue = FALSE;
    if( mbValid )
    {
        try
        {
            mAny = rXPropSet->getPropertyValue( rString );
            if( 0 != mAny.get() )
                bRetValue = TRUE;
        }
        catch( ::com::sun::star::uno::Exception& )
        {
            bRetValue = FALSE;
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

//  laedt und konvertiert text aus shape, ergebnis ist mnTextSize gespeichert
UINT32 ImplEESdrObject::ImplGetText()
{
    Reference< XText > xXText( mXShape, UNO_QUERY );
    mnTextSize = 0;
    if( xXText.is() )
        mnTextSize = xXText->getString().getLength();
    return mnTextSize;
}

BOOL ImplEESdrObject::ImplHasText() const
{
    Reference< XText > xXText( mXShape, UNO_QUERY );
    return xXText.is() && xXText->getString().getLength();
}

