/*************************************************************************
 *
 *  $RCSfile: eschesdo.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: er $ $Date: 2000-10-11 12:17:30 $
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
#ifndef _SVX_ESCHEREX_HXX
#include "escherex.hxx"
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
#ifndef _COM_SUN_STAR_DRAWING_CONNECTORTYPE_HPP_
#include <com/sun/star/drawing/ConnectorType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
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

#define ANY_FLAGS_LINE          0x01
#define ANY_FLAGS_POLYLINE      0x02
#define ANY_FLAGS_POLYPOLYGON   0x04

// we don't write any PowerPoint specific data, implementations left out,
// for example all the text stuff
#define EES_WRITE_EPP 0

// PowerPoint TextStyles
#define EPP_TEXTSTYLE_NORMAL            0x00000001
#define EPP_TEXTSTYLE_TITLE             0x00000010
#define EPP_TEXTSTYLE_BODY              0x00000100
#define EPP_TEXTSTYLE_TEXT              0x00001000

// PowerPoint PLACEHOLDER_ID
#define EPP_PLACEHOLDER_NONE                  0 /*  0 None                        */
#define EPP_PLACEHOLDER_MASTERTITLE           1 /*  1 Master title                */
#define EPP_PLACEHOLDER_MASTERBODY            2 /*  2 Master body                 */
#define EPP_PLACEHOLDER_MASTERCENTEREDTITLE   3 /*  3 Master centered title       */
#define EPP_PLACEHOLDER_MASTERNOTESSLIDEIMAGE 4 /*  4 Master notes slide image    */
#define EPP_PLACEHOLDER_MASTERNOTESBODYIMAGE  5 /*  5 Master notes body image     */
#define EPP_PLACEHOLDER_MASTERDATE            6 /*  6 Master date                 */
#define EPP_PLACEHOLDER_MASTERSLIDENUMBER     7 /*  7 Master slide number         */
#define EPP_PLACEHOLDER_MASTERFOOTER          8 /*  8 Master footer               */
#define EPP_PLACEHOLDER_MASTERHEADER          9 /*  9 Master header               */
#define EPP_PLACEHOLDER_MASTERSUBTITLE       10 /* 10 Master subtitle             */
#define EPP_PLACEHOLDER_GENERICTEXTOBJECT    11 /* 11 Generic text object         */
#define EPP_PLACEHOLDER_TITLE                13 /* 12 Title                       */
#define EPP_PLACEHOLDER_BODY                 14 /* 13 Body                        */
//#define EPP_PLACEHOLDER_NOTESBODY          14 /* 14 Notes body                  */
#define EPP_PLACEHOLDER_CENTEREDTITLE        15 /* 15 Centered title              */
#define EPP_PLACEHOLDER_SUBTITLE             16 /* 16 Subtitle                    */
#define EPP_PLACEHOLDER_VERTICALTEXTTITLE    17 /* 17 Vertical text title         */
#define EPP_PLACEHOLDER_VERTICALTEXTBODY     18 /* 18 Vertical text body          */
#define EPP_PLACEHOLDER_NOTESSLIDEIMAGE      19 /* 19 Notes slide image           */
#define EPP_PLACEHOLDER_OBJECT               20 /* 20 Object (no matter the size) */
#define EPP_PLACEHOLDER_GRAPH                21 /* 21 Graph                       */
#define EPP_PLACEHOLDER_TABLE                22 /* 22 Table                       */
#define EPP_PLACEHOLDER_CLIPART              23 /* 23 Clip Art                    */
#define EPP_PLACEHOLDER_ORGANISZATIONCHART   24 /* 24 Organization Chart          */
#define EPP_PLACEHOLDER_MEDIACLIP            25 /* 25 Media Clip                  */


// ===================================================================

// ---------------------------------------------------------------------------------------------
// bei Rechtecken           bei Ellipsen    bei Polygonen
//
// nRule =  0 ->Top         0 ->Top         nRule = Index auf ein (Poly)Polygon Punkt
//          1 ->Left        2 ->Left
//          2 ->Bottom      4 ->Bottom
//          3 ->Right       6 ->Right

UINT32 ImplEESdrConnectorListEntry::GetConnectorRule( BOOL bFirst )
{
    UINT32 nRule = 0;

    Point aRefPoint( ( bFirst ) ? maPointA : maPointB );
    Reference< XShape > aXShape( ( bFirst ) ? mXConnectToA : mXConnectToB );

    String aType( aXShape->getShapeType() );
    aType.Erase( 0, 12 );   // "stardiv.one.drawing." entfernen
    xub_StrLen nPos = aType.SearchAscii( "Shape" );
    aType.Erase( nPos, 5 );

    if( aType.EqualsAscii( "drawing.PolyPolygon" ) ||
        aType.EqualsAscii( "drawing.PolyLine" ) )
    {
        Reference< XPropertySet > aPropertySet( aXShape, uno::UNO_QUERY );
        if ( aPropertySet.is() )
        {
            try
            {
                Any aAny( aPropertySet->getPropertyValue( rtl::OUString::createFromAscii("PolyPolygon") ) );
                if ( aAny.getValue() )
                {
                    PointSequenceSequence* pSourcePolyPolygon = (PointSequenceSequence*)aAny.getValue();
                    INT32 nOuterSequenceCount = pSourcePolyPolygon->getLength();
                    PointSequence* pOuterSequence = pSourcePolyPolygon->getArray();

                    if ( pOuterSequence )
                    {
                        INT32 a, b, nIndex = 0;
                        UINT32 nDistance = 0xffffffff;
                        for( a = 0; a < nOuterSequenceCount; a++ )
                        {
                            PointSequence* pInnerSequence = pOuterSequence++;
                            if ( pInnerSequence )
                            {
                                awt::Point* pArray = pInnerSequence->getArray();
                                if ( pArray )
                                {
                                    for ( b = 0; b < pInnerSequence->getLength(); b++, nIndex++, pArray++ )
                                    {
                                        UINT32 nDist = (UINT32)hypot( aRefPoint.X() - pArray->X, aRefPoint.Y() - pArray->Y );
                                        if ( nDist < nDistance )
                                        {
                                            nRule = nIndex;
                                            nDistance = nDist;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch(...)
            {
            }
        }
    }
    else if ( aType.EqualsAscii( "drawing.OpenBezier" ) ||
              aType.EqualsAscii( "drawing.OpenFreeHand" ) ||
              aType.EqualsAscii( "drawing.PolyLinePath" ) ||
              aType.EqualsAscii( "drawing.ClosedBezier" ) ||
              aType.EqualsAscii("drawing.ClosedFreeHand" ) ||
              aType.EqualsAscii("drawing.PolyPolygonPath" ) )
    {
        Reference< XPropertySet > aPropertySet( aXShape, uno::UNO_QUERY );
        if( aPropertySet.is() )
        {
            try
            {
                Any aAny( aPropertySet->getPropertyValue( rtl::OUString::createFromAscii("PolyPolygonBezier") ) );
                if ( aAny.getValue() )
                {
                    PolyPolygonBezierCoords* pSourcePolyPolygon = (PolyPolygonBezierCoords*)aAny.getValue();
                    INT32 nOuterSequenceCount = pSourcePolyPolygon->Coordinates.getLength();

                    // Zeiger auf innere sequences holen
                    PointSequence* pOuterSequence = pSourcePolyPolygon->Coordinates.getArray();
                    FlagSequence*  pOuterFlags = pSourcePolyPolygon->Flags.getArray();

                    if ( pOuterSequence && pOuterFlags )
                    {
                        INT32 a, b, nIndex = 0;
                        UINT32 nDistance = 0xffffffff;

                        for ( a = 0; a < nOuterSequenceCount; a++ )
                        {
                            PointSequence* pInnerSequence = pOuterSequence++;
                            FlagSequence*  pInnerFlags = pOuterFlags++;
                            if ( pInnerSequence && pInnerFlags )
                            {
                                awt::Point* pArray = pInnerSequence->getArray();
                                PolygonFlags* pFlags = pInnerFlags->getArray();
                                if ( pArray && pFlags )
                                {
                                    for ( b = 0; b < pInnerSequence->getLength(); b++, pArray++ )
                                    {
                                        PolyFlags ePolyFlags = *( (PolyFlags*)pFlags++ );
                                        if ( ePolyFlags == POLY_CONTROL )
                                            continue;
                                        UINT32 nDist = (UINT32)hypot( aRefPoint.X() - pArray->X, aRefPoint.Y() - pArray->Y );
                                        if ( nDist < nDistance )
                                        {
                                            nRule = nIndex;
                                            nDistance = nDist;
                                        }
                                        nIndex++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch(...)
            {
            }
        }
    }
    else
    {
        Rectangle aRect( aXShape->getPosition().X, aXShape->getPosition().Y, aXShape->getSize().Width, aXShape->getSize().Height );
        nRule = 0;
        if ( aRefPoint.X() == aRect.Left() )
            nRule++;
        else if ( aRefPoint.Y() == aRect.Bottom() )
            nRule = 2;
        else if ( aRefPoint.X() == aRect.Right() )
            nRule = 3;
    }
    if( aType.EqualsAscii( "drawing.Ellipse" ))
        nRule <<= 1;    // In PPT hat eine Ellipse 8 Möglichkeiten sich zu connecten

    return nRule;
}

// ---------------------------------------------------------------------------------------------

ImplEESdrSolverContainer::~ImplEESdrSolverContainer()
{
    void *pP;
    for( pP = maShapeList.First(); pP; pP = maShapeList.Next() )
        delete (ImplEESdrShapeListEntry*)pP;
    for( pP = maConnectorList.First(); pP; pP = maConnectorList.Next() )
        delete (ImplEESdrConnectorListEntry*)pP;
}

// ---------------------------------------------------------------------------------------------

void ImplEESdrSolverContainer::AddShape( const Reference< XShape >& rXShape, UINT32 nId )
{
    maShapeList.Insert( new ImplEESdrShapeListEntry( rXShape, nId ),
                        LIST_APPEND );
}

// ---------------------------------------------------------------------------------------------

void ImplEESdrSolverContainer::AddConnector( const Reference< XShape >& rConnector,
                            const Point& rPA, const Reference< XShape >& rConA,
                            const Point& rPB, const Reference< XShape >& rConB )
{
    maConnectorList.Insert( new ImplEESdrConnectorListEntry(
                        rConnector, rPA, rConA, rPB, rConB ), LIST_APPEND );
}

// ---------------------------------------------------------------------------------------------

UINT32 ImplEESdrSolverContainer::ImplGetId( const Reference< XShape >& rXShape )
{
    for ( ImplEESdrShapeListEntry* pPtr = (ImplEESdrShapeListEntry*)maShapeList.First(); pPtr; pPtr = (ImplEESdrShapeListEntry*)maShapeList.Next() )
    {
        if ( rXShape == pPtr->aXShape )
            return ( pPtr->nEscherId );
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------

void ImplEESdrSolverContainer::WriteSolver( EscherEx& rEscherEx )
{
    UINT32 nCount = maConnectorList.Count();
    if ( nCount )
    {
        SvStream& rStrm = rEscherEx.GetStream();
        rEscherEx.OpenContainer( ESCHER_SolverContainer, nCount );

        ImplEESdrConnectorRule aConnectorRule;
        aConnectorRule.nRuleId = 2;
        for ( ImplEESdrConnectorListEntry* pPtr = (ImplEESdrConnectorListEntry*)maConnectorList.First(); pPtr; pPtr = (ImplEESdrConnectorListEntry*)maConnectorList.Next() )
        {
            aConnectorRule.ncptiA = aConnectorRule.ncptiB = 0xffffffff;
            aConnectorRule.nShapeC = ImplGetId( pPtr->mXConnector );
            aConnectorRule.nShapeA = ImplGetId( pPtr->mXConnectToA );
            aConnectorRule.nShapeB = ImplGetId( pPtr->mXConnectToB );

            if ( aConnectorRule.nShapeC )
            {
                if ( aConnectorRule.nShapeA )
                    aConnectorRule.ncptiA = pPtr->GetConnectorRule( TRUE );
                if ( aConnectorRule.nShapeB )
                    aConnectorRule.ncptiB = pPtr->GetConnectorRule( FALSE );
            }
            rEscherEx.AddAtom( 24, ESCHER_ConnectorRule, 1 );
            rStrm   << aConnectorRule.nRuleId
                    << aConnectorRule.nShapeA
                    << aConnectorRule.nShapeB
                    << aConnectorRule.nShapeC
                    << aConnectorRule.ncptiA
                    << aConnectorRule.ncptiB;

            aConnectorRule.nRuleId += 2;
        }
        rEscherEx.CloseContainer(); // ESCHER_SolverContainer
    }
}


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
#if 0
BOOL ImplEESdrWriter::ImplGetShapeByIndex( UINT32 nIndex, BOOL bGroup )
{
    while(TRUE)
    {
        if (  ( bGroup == FALSE ) || ( GetCurrentGroupLevel() == 0 ) )
            mXShape = *(Reference< XShape >*)mXShapes->getByIndex( nIndex ).get();
        else
            mXShape = *(Reference< XShape >*)GetCurrentGroupAccess()->getByIndex( GetCurrentGroupIndex() ).get();

        if ( !ImplInitCurrentShape() )
            break;

        return TRUE;
    }
    return FALSE;
}
#endif

// -------------------------------------------------------------------

void ImplEESdrWriter::ImplWriteTextBundle( ImplEESdrObject& rObj )
{
    if( rObj.ImplGetText() )
    {
        ESCHER_AnchorText   eAnchor = ESCHER_AnchorTop;
        UINT32              nTextAttr = 0x40004;    // rotate text with shape

        VerticalAlignment* pVAlign =
            ( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("TextVerticalAdjust") ) )
                ? (VerticalAlignment*)rObj.GetUsrAny().getValue()
                : NULL;
        if ( pVAlign )
        {
            switch ( *pVAlign )
            {
                case VerticalAlignment_MIDDLE :
                    eAnchor = ESCHER_AnchorMiddle;
                break;

                case VerticalAlignment_BOTTOM :
                    eAnchor = ESCHER_AnchorBottom;
                break;

                default :
                case VerticalAlignment_TOP :
                    eAnchor = ESCHER_AnchorTop;
                break;
            }
        }
        if( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("TextHorizontalAdjust") ) )
        {
            switch ( *(TextAdjust*)rObj.GetUsrAny().getValue() )
            {
                case TextAdjust_CENTER :
                case TextAdjust_RIGHT :
                case TextAdjust_LEFT :
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
                case TextAdjust_STRETCH :
                case TextAdjust_BLOCK :
                break;
            }
        }
/*
        if ( ImplGetPropertyValue( L"TextFitToSize" ) )
        {
            if ( *( (INT16*)mAny.get() ) == 1 )
            {
                nTextAttr |= 0x10001;
                mpEscherEx->AddOpt( ESCHER_Prop_scaleText, ? );
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

        mpEscherEx->AddOpt( ESCHER_Prop_dxTextLeft, nLeft * 360 );
        mpEscherEx->AddOpt( ESCHER_Prop_dxTextRight, nRight * 360 );
        mpEscherEx->AddOpt( ESCHER_Prop_dyTextTop, nTop * 360 );
        mpEscherEx->AddOpt( ESCHER_Prop_dyTextBottom, nBottom * 360 );

        mpEscherEx->AddOpt( ESCHER_Prop_WrapText, ESCHER_WrapSquare );
        mpEscherEx->AddOpt( ESCHER_Prop_AnchorText, eAnchor );
        mpEscherEx->AddOpt( ESCHER_Prop_FitTextToShape, nTextAttr );
        UINT32 nTxtBxId = mpEscherEx->QueryTextID( rObj.GetShapeRef(),
                                                    rObj.GetShapeId() );
        mpEscherEx->AddOpt( ESCHER_Prop_lTxid, nTxtBxId );
    }
}


// -------------------------------------------------------------------

void ImplEESdrWriter::ImplFlipBoundingBox( ImplEESdrObject& rObj,
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

    INT32 nXDiff = fCos * nWidthHalf + fSin * (-nHeightHalf);
    INT32 nYDiff = - ( fSin * nWidthHalf - fCos * ( -nHeightHalf ) );

    aRect.Move( -( nWidthHalf - nXDiff ), - ( nHeightHalf + nYDiff ) );

#if EES_WRITE_EPP
// this is not true for Excel, what about Word?
    if ( ( nAngle > 4500 && nAngle <= 13500 ) ||
        ( nAngle > 22500 && nAngle <= 31500 ) )
    {
        // In diesen beiden Bereichen steht in PPT gemeinerweise die
        // BoundingBox bereits senkrecht. Daher muss diese VOR
        // DER ROTATION flachgelegt werden.
        Point   aTopLeft( aRect.Left() + nWidthHalf - nHeightHalf,
                          aRect.Top() + nHeightHalf - nWidthHalf );
        Size    aNewSize( aRect.GetHeight(), aRect.GetWidth() );
        aRect = Rectangle( aTopLeft, aNewSize );
    }
#endif // EES_WRITE_EPP

    nAngle *= 655;
    nAngle += 0x8000;
    nAngle &=~0xffff;                                   // nAngle auf volle Gradzahl runden
    mpEscherEx->AddOpt( ESCHER_Prop_Rotation, nAngle );

    rObj.SetAngle( nAngle );
    rObj.SetRect( aRect );
}

//  -----------------------------------------------------------------------

void ImplEESdrWriter::ImplWriteAny( ImplEESdrObject& rObj,
                                    UINT32 nFlags, BOOL bBezier,
                                    Polygon* pPolygon )
{
    PolyPolygon aPolyPolygon;
    Polygon     aPolygon;

    if ( pPolygon )
        aPolyPolygon.Insert( *pPolygon, POLYPOLY_APPEND );
    else
    {
        if ( bBezier )
        {
            if ( !rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("PolyPolygonBezier") ) )
                return;
        }
        else if ( !rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("PolyPolygon") ) )
                return;

        if ( bBezier )
        {
            PolyPolygonBezierCoords* pSourcePolyPolygon =
                    (PolyPolygonBezierCoords*)rObj.GetUsrAny().getValue();
            INT32 nOuterSequenceCount = pSourcePolyPolygon->Coordinates.getLength();

            // Zeiger auf innere sequences holen
            PointSequence* pOuterSequence = pSourcePolyPolygon->Coordinates.getArray();
            FlagSequence*  pOuterFlags = pSourcePolyPolygon->Flags.getArray();

            if ( ! ( pOuterSequence && pOuterFlags ) )
                return;

            INT32  a, b, nInnerSequenceCount;
            awt::Point* pArray;

            // dies wird ein Polygon set
            for ( a = 0; a < nOuterSequenceCount; a++ )
            {
                PointSequence* pInnerSequence = pOuterSequence++;
                FlagSequence*  pInnerFlags = pOuterFlags++;

                if ( ! ( pInnerSequence && pInnerFlags ) )
                    return;

                // Zeiger auf Arrays holen
                pArray = pInnerSequence->getArray();
                PolygonFlags* pFlags = pInnerFlags->getArray();

                if ( pArray && pFlags )
                {
                    nInnerSequenceCount = pInnerSequence->getLength();
                    aPolygon = Polygon( nInnerSequenceCount );
                    for( b = 0; b < nInnerSequenceCount; b++)
                    {
                        PolyFlags   ePolyFlags( *( (PolyFlags*)pFlags++ ) );
                        aPolygon[ b ] = Point( pArray->X, pArray->Y );
                        pArray++;
                        aPolygon.SetFlags( b, ePolyFlags );

                        if ( ePolyFlags == POLY_CONTROL )
                            continue;
                    }
                    aPolyPolygon.Insert( aPolygon, POLYPOLY_APPEND );
                }
            }
        }
        else
        {
            PointSequenceSequence* pSourcePolyPolygon =
                        (PointSequenceSequence*)rObj.GetUsrAny().getValue();
            INT32 nOuterSequenceCount = pSourcePolyPolygon->getLength();

            // Zeiger auf innere sequences holen
            PointSequence* pOuterSequence = pSourcePolyPolygon->getArray();
            if ( !( pOuterSequence ) )
                return;

            // ist dies ein Polygon oder gar ein PolyPolygon ?
            // sogar eine einfache Line wird als Polygon verpackt !!! ????

            if ( nFlags & ANY_FLAGS_LINE )
            {
                PointSequence* pInnerSequence = pOuterSequence++;
                if ( !( pInnerSequence ) )
                    return;

                awt::Point* pArray = pInnerSequence->getArray();
                if ( pArray )
                    rObj.SetRect( Rectangle( ImplMapPoint( Point( pArray[ 0 ].X, pArray[ 0 ].Y) ),
                                            ImplMapPoint( Point( pArray[ 1 ].X, pArray[ 1 ].Y) ) ));
                return;
            }
            INT32 a, b, nInnerSequenceCount;
            awt::Point* pArray;

            // dies wird ein Polygon set
            for( a = 0; a < nOuterSequenceCount; a++ )
            {
                PointSequence* pInnerSequence = pOuterSequence++;
                if ( !( pInnerSequence ) )
                    return;

                // Zeiger auf Arrays holen
                if ( pArray = pInnerSequence->getArray() )
                {
                    nInnerSequenceCount = pInnerSequence->getLength();
                    aPolygon = Polygon( nInnerSequenceCount );
                    for( b = 0; b < nInnerSequenceCount; b++)
                        aPolygon[ b ] = Point( pArray->X, pArray->Y );
                        pArray++;
                    aPolyPolygon.Insert( aPolygon, POLYPOLY_APPEND );
                }
            }
        }
    }
    INT32 i, j, k, nPoints, nBezPoints, nPolyCount = aPolyPolygon.Count();

    Rectangle   aGeoRect( aPolyPolygon.GetBoundRect() );
    rObj.SetRect( ImplMapPoint( aGeoRect.TopLeft() ),
                  ImplMapSize( aGeoRect.GetSize() ) );

    for ( nBezPoints = nPoints = i = 0; i < nPolyCount; i++ )
    {
        k = aPolyPolygon[ i ].GetSize();
        nPoints += k;
        for ( j = 0; j < k; j++ )
        {
            if ( aPolyPolygon[ i ].GetFlags( j ) != POLY_CONTROL )
                nBezPoints++;
        }
    }
    UINT32 nVerticesBufSize = ( nPoints << 2 ) + 6;
    BYTE* pVerticesBuf = new BYTE[ nVerticesBufSize ];


    UINT32 nSegmentBufSize = ( ( nBezPoints << 2 ) + 8 );
    if ( nPolyCount > 1 )
        nSegmentBufSize += ( nPolyCount << 1 );
    BYTE* pSegmentBuf = new BYTE[ nSegmentBufSize ];

    BYTE* pPtr = pVerticesBuf;
    *pPtr++ = (BYTE)( nPoints );                    // Little endian // Little endian
    *pPtr++ = (BYTE)( nPoints >> 8 );
    *pPtr++ = (BYTE)( nPoints );
    *pPtr++ = (BYTE)( nPoints >> 8 );
    *pPtr++ = (BYTE)0xf0;
    *pPtr++ = (BYTE)0xff;

    for ( j = 0; j < nPolyCount; j++ )
    {
        aPolygon = aPolyPolygon[ j ];
        nPoints = aPolygon.GetSize();
        for ( i = 0; i < nPoints; i++ )             // Punkte aus Polygon in Buffer schreiben
        {
            Point aPoint = aPolygon[ i ];
            aPoint.X() -= aGeoRect.Left();
            aPoint.Y() -= aGeoRect.Top();

            *pPtr++ = (BYTE)( aPoint.X() );
            *pPtr++ = (BYTE)( aPoint.X() >> 8 );
            *pPtr++ = (BYTE)( aPoint.Y() );
            *pPtr++ = (BYTE)( aPoint.Y() >> 8 );
        }
    }

    pPtr = pSegmentBuf;
    *pPtr++ = (BYTE)( ( nSegmentBufSize - 6 ) >> 1 );
    *pPtr++ = (BYTE)( ( nSegmentBufSize - 6 ) >> 9 );
    *pPtr++ = (BYTE)( ( nSegmentBufSize - 6 ) >> 1 );
    *pPtr++ = (BYTE)( ( nSegmentBufSize - 6 ) >> 9 );
    *pPtr++ = (BYTE)2;
    *pPtr++ = (BYTE)0;

    for ( j = 0; j < nPolyCount; j++ )
    {
        *pPtr++ = 0x0;          // Polygon start
        *pPtr++ = 0x40;
        aPolygon = aPolyPolygon[ j ];
        nPoints = aPolygon.GetSize();
        for ( i = 0; i < nPoints; i++ )         // Polyflags in Buffer schreiben
        {
            *pPtr++ = 0;
            if ( bBezier )
                *pPtr++ = 0xb3;
            else
                *pPtr++ = 0xac;
            if ( ( i + 1 ) != nPoints )
            {
                *pPtr++ = 1;
                if ( aPolygon.GetFlags( i + 1 ) == POLY_CONTROL )
                {
                    *pPtr++ = 0x20;
                    i += 2;
                }
                else
                    *pPtr++ = 0;
            }
        }
        if ( nPolyCount > 1 )
        {
            *pPtr++ = 1;                        // end of polygon
            *pPtr++ = 0x60;
        }
    }
    *pPtr++ = 0;
    *pPtr++ = 0x80;

    mpEscherEx->AddOpt( ESCHER_Prop_geoRight, aGeoRect.GetSize().Width() );
    mpEscherEx->AddOpt( ESCHER_Prop_geoBottom, aGeoRect.GetSize().Height() );

    mpEscherEx->AddOpt( ESCHER_Prop_shapePath, ESCHER_ShapeComplex );
    mpEscherEx->AddOpt( ESCHER_Prop_pVertices, TRUE, nVerticesBufSize - 6, (BYTE*)pVerticesBuf, nVerticesBufSize );
    mpEscherEx->AddOpt( ESCHER_Prop_pSegmentInfo, TRUE, nSegmentBufSize, (BYTE*)pSegmentBuf, nSegmentBufSize );
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

#define SHAPE_TEXT( bFill )                                     \
{                                                               \
    mnTextStyle = EPP_TEXTSTYLE_TEXT;                           \
    mpEscherEx->OpenContainer( ESCHER_SpContainer );            \
    ADD_SHAPE( ESCHER_ShpInst_TextBox, 0xa00 );                 \
    mpEscherEx->BeginCount();                                   \
    if ( bFill )                                                \
        ImplWriteFillBundle( rObj, TRUE );                      \
    ImplWriteTextBundle( rObj );                                \
}

UINT32 ImplEESdrWriter::ImplWriteShape( ImplEESdrObject& rObj,
                                ImplEESdrSolverContainer& rSolverContainer,
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

        BYTE nPlaceHolderAtom = EPP_PLACEHOLDER_NONE;

        rObj.SetShadow( 2 );
        mnTextStyle = EPP_TEXTSTYLE_NORMAL;
#if EES_WRITE_EPP
        mpCurrentDefault->SetObject( TEXT );
#endif // EES_WRITE_EPP

        if ( rObj.GetType().EqualsAscii( "drawing.Rectangle" ))
        {
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            UINT32 nRadius = (UINT32)rObj.ImplGetInt32PropertyValue(
                                            ::rtl::OUString::createFromAscii("CornerRadius"));
            if( nRadius )
            {
                nRadius = ImplMapSize( Size( nRadius, 0 )).Width();
                ADD_SHAPE( ESCHER_ShpInst_RoundRectangle, 0xa00 );  // Flags: Connector | HasSpt
                mpEscherEx->BeginCount();
                INT32 nLenght = rObj.GetRect().GetWidth();
                if ( nLenght > rObj.GetRect().GetHeight() )
                    nLenght = rObj.GetRect().GetHeight();
                nLenght >>= 1;
                if ( nRadius >= nLenght )
                    nRadius = 0x2a30;                           // 0x2a30 ist PPTs maximum radius
                else
                    nRadius = ( 0x2a30 * nRadius ) / nLenght;
                mpEscherEx->AddOpt( ESCHER_Prop_adjustValue, nRadius );
            }
            else
            {
                ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0xa00 );           // Flags: Connector | HasSpt
                mpEscherEx->BeginCount();
            }
            ImplWriteFillBundle( rObj, TRUE );
            ImplWriteTextBundle( rObj );
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
                mpEscherEx->BeginCount();
                ImplWriteFillBundle( rObj, TRUE );
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

// warum??
//              maPosition = mXShape->getPosition();
//              maSize = mXShape->getSize();
//              maRect = Rectangle( maPosition, maSize );

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
                mpEscherEx->BeginCount();
                switch ( ePolyKind )
                {
                    case POLY_PIE :
                    case POLY_CHORD :
                    {
                        ImplWriteAny( rObj, ANY_FLAGS_POLYPOLYGON, FALSE,
                                        &aPolygon );
                        ImplWriteFillBundle( rObj, TRUE );
                    }
                    break;

                    case POLY_ARC :
                    {
                        ImplWriteAny( rObj, ANY_FLAGS_POLYLINE, FALSE,
                                    &aPolygon );
                        ImplWriteLineBundle( rObj, FALSE );
                    }
                    break;
                }
            }
            ImplWriteTextBundle( rObj );
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.Control" ))
        {
            break;
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.Connector" ))
        {
            Reference< XShape > aShapeA, aShapeB;

            Point aStartPoint, aEndPoint;

            if ( !rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("EdgeKind") ) )
                break;
            ConnectorType eCt;
            ::cppu::any2enum< ConnectorType >( eCt, rObj.GetUsrAny() );

            if ( !rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("EdgeStartPoint") ) )
                break;
            aStartPoint = *(Point*)rObj.GetUsrAny().getValue();

            if ( !rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("EdgeEndPoint") ) )
                break;
            aEndPoint = *(Point*)rObj.GetUsrAny().getValue();

            UINT32 nAdjustValue1, nAdjustValue2, nAdjustValue3;
            nAdjustValue1 = nAdjustValue2 = nAdjustValue3 = 0x2a30;

            if ( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("EdgeStartConnection") ) )
            {
                aShapeA = *(Reference< XShape >*)rObj.GetUsrAny().getValue();
            }
            if ( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("EdgeEndConnection") ) )
            {
                aShapeB = *(Reference< XShape >*)rObj.GetUsrAny().getValue();
            }

/*          if ( ImplGetPropertyValue( L"EdgeLine1Delta" ) )
            {
            }
            if ( ImplGetPropertyValue( L"EdgeLine2Delta" ) )
            {
            }
            if ( ImplGetPropertyValue( L"EdgeLine3Delta" ) )
            {
            }
            if ( ImplGetPropertyValue( L"EdgeNode1HorzDist" ) )
            {
            }
            if ( ImplGetPropertyValue( L"EdgeNode1VertDist" ) )
            {
            }
            if ( ImplGetPropertyValue( L"EdgeNode2HorzDist" ) )
            {
            }
            if ( ImplGetPropertyValue( L"EdgeNode2VertDist" ) )
            {
            }
*/
            rSolverContainer.AddConnector( rObj.GetShapeRef(), aStartPoint,
                                            aShapeA, aEndPoint, aShapeB );
            rObj.SetRect( Rectangle( ImplMapPoint( aStartPoint ),
                                     ImplMapPoint( aEndPoint ) ) );
            ImplWriteAny( rObj, ANY_FLAGS_LINE, FALSE );
            const Rectangle& rRect = rObj.GetRect();
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            UINT32 nFlags = 0xa00;                  // Flags: Connector | HasSpt
            if( rRect.Top() > rRect.Bottom() )
                nFlags |= 0x80;                     // Flags: VertMirror
            if( rRect.Left() > rRect.Right() )
                nFlags |= 0x40;                     // Flags: HorzMirror

            Rectangle aJustifiedRect( rRect );
            aJustifiedRect.Justify();

            switch ( eCt )
            {
                case ConnectorType_CURVE :
                {
                    ADD_SHAPE( ESCHER_ShpInst_CurvedConnector3, nFlags );
                    mpEscherEx->BeginCount();
                    mpEscherEx->AddOpt( ESCHER_Prop_cxstyle, ESCHER_cxstyleCurved );
                    mpEscherEx->AddOpt( ESCHER_Prop_adjustValue, nAdjustValue1 );
                    mpEscherEx->AddOpt( ESCHER_Prop_adjust2Value, -nAdjustValue2 );
                }
                break;

                case ConnectorType_STANDARD :   // Connector 2->5
                {
                    ADD_SHAPE( ESCHER_ShpInst_BentConnector3, nFlags );
                    mpEscherEx->BeginCount();
                    mpEscherEx->AddOpt( ESCHER_Prop_cxstyle, ESCHER_cxstyleBent );
                }
                break;

                default:
                case ConnectorType_LINE :
                case ConnectorType_LINES :      // Connector 2->5
                {
                    nFlags |= 0x100;
                    ADD_SHAPE( ESCHER_ShpInst_StraightConnector1, nFlags );
                    mpEscherEx->BeginCount();
                    mpEscherEx->AddOpt( ESCHER_Prop_cxstyle, ESCHER_cxstyleStraight );
                }
                break;
            }
            ImplWriteLineBundle( rObj, FALSE );
            rObj.SetAngle( 0 );
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
                mpEscherEx->BeginCount();
                mpEscherEx->AddOpt( ESCHER_Prop_shapePath, ESCHER_ShapeComplex );
                ImplWriteLineBundle( FALSE );
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
            ImplWriteAny( rObj, ANY_FLAGS_LINE, FALSE );
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
            mpEscherEx->BeginCount();
            mpEscherEx->AddOpt( ESCHER_Prop_shapePath, ESCHER_ShapeComplex );
            ImplWriteLineBundle( rObj, FALSE );
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
            mpEscherEx->BeginCount();
            ImplWriteAny( rObj, ANY_FLAGS_POLYPOLYGON, FALSE );
            ImplWriteFillBundle( rObj, TRUE );
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
            mpEscherEx->BeginCount();
            ImplWriteAny( rObj, ANY_FLAGS_POLYLINE, FALSE );
            ImplWriteLineBundle( rObj, FALSE );
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
            mpEscherEx->BeginCount();
            ImplWriteAny( rObj, ANY_FLAGS_POLYLINE, TRUE );
            ImplWriteLineBundle( rObj, FALSE );
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
            mpEscherEx->BeginCount();
            ImplWriteAny( rObj, ANY_FLAGS_POLYPOLYGON, TRUE );
            ImplWriteFillBundle( rObj, TRUE );
            rObj.SetAngle( 0 );
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.GraphicObject" ))
        {
            mpEscherEx->OpenContainer( ESCHER_SpContainer );

            // ein GraphicObject kann auch ein ClickMe Element sein
            if( rObj.IsEmptyPresObj() && ( ePageType == NORMAL ) )
            {
                nPlaceHolderAtom = 19;      // EPP_PLACEHOLDER_OBJECT;
#if EES_WRITE_EPP
                if ( mnLayout == 8 )
                    nPlaceHolderAtom = 21;  // EPP_PLACEHOLDER_OBJECT_TABLE
                else if ( ( mnLayout == 2 ) || ( mnLayout == 4 ) || ( mnLayout == 7 ) )
                    nPlaceHolderAtom = 20;  // EPP_PLACEHOLDER_GRAPH;
                else if ( ( mnLayout == 9 ) || ( mnLayout == 6 ) )
                    nPlaceHolderAtom = 22;  // EPP_PLACEHOLDER_CLIPART
#endif // EES_WRITE_EPP
                ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0x220 );               // Flags: HaveAnchor | HaveMaster
                mpEscherEx->BeginCount();
                UINT32 nTxtBxId = mpEscherEx->QueryTextID( rObj.GetShapeRef(),
                                                        rObj.GetShapeId() );
                mpEscherEx->AddOpt( ESCHER_Prop_lTxid, nTxtBxId );
                mpEscherEx->AddOpt( ESCHER_Prop_fNoFillHitTest, 0x10001 );
                mpEscherEx->AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x10001 );
                mpEscherEx->AddOpt( ESCHER_Prop_hspMaster, mnShapeMasterBody );
            }
            else
            {
                if( rObj.ImplGetText() )
                {
                    ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0xa00 );           // Flags: Connector | HasSpt
                    mpEscherEx->BeginCount();
                    if( ImplGetGraphic( rObj, ::rtl::OUString::createFromAscii("GraphicObjectFillBitmap"),TRUE ) )
                    {
                        mpEscherEx->AddOpt( ESCHER_Prop_WrapText,
                                            ESCHER_WrapNone );
                        mpEscherEx->AddOpt( ESCHER_Prop_AnchorText,
                                            ESCHER_AnchorMiddle );
                        mpEscherEx->AddOpt( ESCHER_Prop_fNoFillHitTest,
                                            0x140014 );
                        mpEscherEx->AddOpt( ESCHER_Prop_fillBackColor,
                                            0x8000000 );
                        mpEscherEx->AddOpt( ESCHER_Prop_fNoLineDrawDash,
                                            0x80000 );
                        ImplWriteTextBundle( rObj );
                    }
                }
                else
                {
                    ADD_SHAPE( ESCHER_ShpInst_PictureFrame, 0xa00 );
                    mpEscherEx->BeginCount();
                    if( ImplGetGraphic( rObj, ::rtl::OUString::createFromAscii("GraphicObjectFillBitmap"), FALSE ) )
                        mpEscherEx->AddOpt( ESCHER_Prop_LockAgainstGrouping,
                                            0x800080 );
                }
            }
        }
        else if ( rObj.GetType().EqualsAscii(  "drawing.Text" ))
        {
            SHAPE_TEXT( TRUE );
        }
        else if ( rObj.GetType().EqualsAscii( "presentation.TitleText" ))
        {
            if ( mbIsTitlePossible )
            {
                mbIsTitlePossible = FALSE;

                if ( ePageType == MASTER )
                    break;

                mpEscherEx->OpenContainer( ESCHER_SpContainer );
#if EES_WRITE_EPP
                mpCurrentDefault->SetObject( TITLETEXT );
#endif // EES_WRITE_EPP
                mnTextStyle = EPP_TEXTSTYLE_TITLE;
                nPlaceHolderAtom = EPP_PLACEHOLDER_TITLE;
                ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0x220 );           // Flags: HaveAnchor | HaveMaster
                mpEscherEx->BeginCount();
                mpEscherEx->AddOpt( ESCHER_Prop_hspMaster, mnShapeMasterTitle );
                ImplWriteFillBundle( rObj, TRUE );
                ImplWriteTextBundle( rObj );
                if( rObj.IsEmptyPresObj() )
                    mpEscherEx->AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90001 );
            }
            else
            {
                rObj.SetType( String( RTL_CONSTASCII_STRINGPARAM(
                                "drawing.Text" ),
                                RTL_TEXTENCODING_MS_1252 ));

                SHAPE_TEXT( TRUE );
            }
        }
        else if ( rObj.GetType().EqualsAscii( "presentation.Outliner" ))
        {
            mnOutlinerCount++;

            if ( ( mnOutlinerCount == 1 ) ||
                     ( ( mnOutlinerCount == 2 ) &&
                     ( mnPrevTextStyle == EPP_TEXTSTYLE_BODY ) ) )
            {
                if ( ePageType == MASTER )
                {
                    mnPrevTextStyle = EPP_TEXTSTYLE_TITLE;
                    break;
                }
#if EES_WRITE_EPP
                mpCurrentDefault->SetObject( OUTLINER );
#endif // EES_WRITE_EPP
                mnTextStyle = EPP_TEXTSTYLE_BODY;
                nPlaceHolderAtom = EPP_PLACEHOLDER_BODY;
                mpEscherEx->OpenContainer( ESCHER_SpContainer );
                ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0x220 );           // Flags: HaveAnchor | HaveMaster
                mpEscherEx->BeginCount();
                mpEscherEx->AddOpt( ESCHER_Prop_hspMaster, mnShapeMasterBody );
                ImplWriteFillBundle( rObj, TRUE );
                ImplWriteTextBundle( rObj );

                if ( rObj.IsEmptyPresObj() )
                    mpEscherEx->AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90001 );
            }
            else
            {
                rObj.SetType( String( RTL_CONSTASCII_STRINGPARAM(
                                "drawing.Text" ),
                                RTL_TEXTENCODING_MS_1252 ));
                SHAPE_TEXT( TRUE );
            }
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.Page" ))
        {
            nPlaceHolderAtom = EPP_PLACEHOLDER_GENERICTEXTOBJECT;
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0xa00 );
            mpEscherEx->BeginCount();
            mpEscherEx->AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x40004 );
            mpEscherEx->AddOpt( ESCHER_Prop_fFillOK, 0x100001 );
            mpEscherEx->AddOpt( ESCHER_Prop_fNoFillHitTest, 0x110011 );
            mpEscherEx->AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90008 );
            mpEscherEx->AddOpt( ESCHER_Prop_fshadowObscured, 0x10001 );
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
                nPlaceHolderAtom = 19;      // EPP_PLACEHOLDER_OBJECT;
#if EES_WRITE_EPP
                if ( mnLayout == 8 )
                    nPlaceHolderAtom = 21;  // EPP_PLACEHOLDER_OBJECT_TABLE
                else if ( ( mnLayout == 2 ) || ( mnLayout == 4 ) || ( mnLayout == 7 ) )
                    nPlaceHolderAtom = 20;  // EPP_PLACEHOLDER_GRAPH;
                else if ( ( mnLayout == 9 ) || ( mnLayout == 6 ) )
                    nPlaceHolderAtom = 22;  // EPP_PLACEHOLDER_CLIPART
#endif // EES_WRITE_EPP
                ADD_SHAPE( ESCHER_ShpInst_Rectangle, 0x220 );               // Flags: HaveAnchor | HaveMaster
                mpEscherEx->BeginCount();
                UINT32 nTxtBxId = mpEscherEx->QueryTextID( rObj.GetShapeRef(),
                                                        rObj.GetShapeId() );
                mpEscherEx->AddOpt( ESCHER_Prop_lTxid, nTxtBxId );
                mpEscherEx->AddOpt( ESCHER_Prop_fNoFillHitTest, 0x10001 );
                mpEscherEx->AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x10001 );
                mpEscherEx->AddOpt( ESCHER_Prop_hspMaster, mnShapeMasterBody );
            }
            else
            {
                //2do: could be made an option in HostAppData whether OLE object should be written or not
                BOOL bAppOLE = TRUE;
                ADD_SHAPE( ESCHER_ShpInst_PictureFrame,
                    0xa00 | (bAppOLE ? SHAPEFLAG_OLESHAPE : 0) );
                mpEscherEx->BeginCount();
                if ( ImplGetGraphic( rObj, ::rtl::OUString::createFromAscii("MetaFile"), FALSE ) )
                {
                    if ( bAppOLE )
                    {   // snooped from Xcl hex dump, nobody knows the trouble I have seen
                        mpEscherEx->AddOpt( ESCHER_Prop_FitTextToShape, 0x00080008 );
                        mpEscherEx->AddOpt( ESCHER_Prop_pictureId,      0x00000001 );
                        mpEscherEx->AddOpt( ESCHER_Prop_fillColor,      0x08000041 );
                        mpEscherEx->AddOpt( ESCHER_Prop_fillBackColor,  0x08000041 );
                        mpEscherEx->AddOpt( ESCHER_Prop_fNoFillHitTest, 0x00110010 );
                        mpEscherEx->AddOpt( ESCHER_Prop_lineColor,      0x08000040 );
                        mpEscherEx->AddOpt( ESCHER_Prop_fNoLineDrawDash,0x00080008 );
//                      mpEscherEx->AddOpt( ESCHER_Prop_fshadowObscured,0x00020000 );
                        mpEscherEx->AddOpt( ESCHER_Prop_fPrint,         0x00080000 );
                    }
                    mpEscherEx->AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
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
            mpEscherEx->BeginCount();

            if ( ImplGetGraphic( rObj, ::rtl::OUString::createFromAscii("Bitmap"), FALSE ) )
                mpEscherEx->AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
        }
        else if ( rObj.GetType().EqualsAscii( "drawing.dontknow" ))
        {
            rObj.SetAngle( 0 );
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            ADD_SHAPE( ESCHER_ShpInst_PictureFrame, 0xa00 );
            mpEscherEx->BeginCount();
            if ( ImplGetGraphic( rObj, ::rtl::OUString::createFromAscii("MetaFile"), FALSE ) )
                mpEscherEx->AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
        }
        else
        {
            break;
        }

        if( rObj.GetShadow() )  // shadow wird nur ausgegeben, wenn es mindestens einen LinesStyle oder FillStyle gibt
        {
            if ( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("Shadow") ) )
            {
                if ( ::cppu::any2bool( rObj.GetUsrAny() ) )
                {
                    mpEscherEx->AddOpt( ESCHER_Prop_fshadowObscured, 0x20002 );
                    if( rObj.ImplGetPropertyValue(  ::rtl::OUString::createFromAscii("ShadowColor") ) )
                        mpEscherEx->AddOpt( ESCHER_Prop_shadowColor,
                            mpEscherEx->GetColor(
                                    *((UINT32*)rObj.GetUsrAny().getValue()) ) );
                    if( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("ShadowXDistance") ) )
                        mpEscherEx->AddOpt( ESCHER_Prop_shadowOffsetX,
                                    *((INT32*)rObj.GetUsrAny().getValue()) * 360 );
                    if( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("ShadowYDistance") ) )
                        mpEscherEx->AddOpt( ESCHER_Prop_shadowOffsetY,
                                    *((INT32*)rObj.GetUsrAny().getValue()) * 360 );
                    if( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("ShadowTransparence") ) )
                        mpEscherEx->AddOpt( ESCHER_Prop_shadowOpacity,
                            ( ( (100 - (*((UINT16*)rObj.GetUsrAny().getValue())))
                                        << 16 ) / 100 ) );
                }
            }
        }

        if( USHRT_MAX != mpEscherEx->GetHellLayerId() &&
            rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("LayerID") ) &&
            (*((UINT16*)rObj.GetUsrAny().getValue()) ) == mpEscherEx->GetHellLayerId() )
        {
            mpEscherEx->AddOpt( ESCHER_Prop_fPrint, 0x200020 );
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
                    ImplFlipBoundingBox( rObj, ImplMapPoint( aRefPoint ) );
                }
            }
        }
        mpEscherEx->EndCount( ESCHER_OPT, 3 );

        if( mpEscherEx->GetGroupLevel() > 1 )
        {
            mpEscherEx->AddAtom( 16, ESCHER_ChildAnchor );
            const Rectangle& rRect = rObj.GetRect();
            mpEscherEx->GetStream() << (INT32)rRect.Left()
                                    << (INT32)rRect.Top()
                                       << (INT32)rRect.Right()
                                    << (INT32)rRect.Bottom();
        }

#if EES_WRITE_EPP
        // ClientAnchor
        mpEscherEx->AddClientAnchor( maRect );

        // ClientData
        AnimationEffect eAe = AnimationEffect_NONE;
        ClickAction eCa = ClickAction_NONE;
        if ( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("Effect") ))
            eAe = (AnimationEffect)rObj.GetUsrAny().getEnumAsINT32();

        if ( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("PresentationOrder") ))
            mnEffectCount = *(UINT16*)rObj.GetUsrAny().getValue();

        if ( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("OnClick") ))
            eCa = (ClickAction)rObj.GetUsrAny().getEnumAsINT32();

        INT32 nPlacementID = -1;

        BOOL bEffect = ( ( eAe != AnimationEffect_NONE ) ||
                        ( ( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("SoundOn") ) ) &&
                        rObj.GetUsrAny().getBOOL() ) );
        BOOL bClientData = ( bEffect || ( eCa != ClickAction_NONE ) ||
                            nPlaceHolderAtom );
        if ( bClientData )
        {
            mpEscherEx->OpenContainer( ESCHER_ClientData );

            if ( nPlaceHolderAtom )
            {
                if( mnTextStyle == EPP_TEXTSTYLE_TITLE
                    || mnTextStyle == EPP_TEXTSTYLE_BODY
                    || nPlaceHolderAtom == EPP_PLACEHOLDER_GENERICTEXTOBJECT
                    || nPlaceHolderAtom >= 19 )
                    nPlacementID = mnIndices++;

                mpEscherEx->AddAtom( 8, EPP_OEPlaceholderAtom );
                *mpStrm << nPlacementID                                 // PlacementID
                        << (BYTE)nPlaceHolderAtom                       // PlaceHolderID
                        << (BYTE)0                                      // Size of PlaceHolder ( 0 = FULL, 1 = HALF, 2 = QUARTER )
                        << (UINT16)0;                                   // padword
            }
            if ( bEffect )
                ImplWriteObjectEffect( eAe, ++mnEffectCount );

            if ( eCa != ClickAction_NONE )
                ImplWriteClickAction( eCa );

            mpEscherEx->CloseContainer();   // Escher_ClientData
        }

        // ClientTextbox
        if ( rObj.GetTextSize() )
        {
            mpEscherEx->OpenContainer( ESCHER_ClientTextbox );
            if ( mnTextStyle == EPP_TEXTSTYLE_TITLE )
            {
                mpEscherEx->AddAtom( 4, EPP_OutlineTextRefAtom );
                *mpStrm << nPlacementID;
            }
            else if ( mnTextStyle == EPP_TEXTSTYLE_BODY )
            {
                mpEscherEx->AddAtom( 4, EPP_OutlineTextRefAtom );
                *mpStrm << nPlacementID;
            }
            else
            {
                mpEscherEx->AddAtom( 4, EPP_TextHeaderAtom );
                *mpStrm << (UINT32)EPP_TEXTTYPE_Other;                              // Text in a Shape
            }
            if ( ( mnTextStyle != EPP_TEXTSTYLE_BODY ) &&
                ( mnTextStyle != EPP_TEXTSTYLE_TITLE ) )
                ImplWriteTextStyleAtom();
            mpEscherEx->CloseContainer();   // ESCHER_ClientTextBox
        }
        else if ( nPlaceHolderAtom >= 19 )
        {
            mpEscherEx->OpenContainer( ESCHER_ClientTextbox );
            mpEscherEx->AddAtom( 4, EPP_TextHeaderAtom );
            *mpStrm << (UINT32)7;
            mpEscherEx->CloseContainer();   // ESCHER_ClientTextBox
        }
#else // !EES_WRITE_EPP
        if ( mpHostAppData )
        {   //! with AdditionalText the App has to control whether these are written or not
            mpHostAppData->WriteClientAnchor( *mpEscherEx, rObj.GetRect() );
            mpHostAppData->WriteClientData( *mpEscherEx );
            mpHostAppData->WriteClientTextbox( *mpEscherEx );
        }
#endif // EES_WRITE_EPP

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

        rObj.SetAngle( rObj.ImplGetInt32PropertyValue( ::rtl::OUString::createFromAscii("RotateAngle")));
        INT32 nAngle = rObj.GetAngle();
        if( rObj.GetType().EqualsAscii( "drawing.Line" ))
        {
//2do: this does not work right
            double fDist = hypot( rObj.GetRect().GetWidth(),
                                    rObj.GetRect().GetHeight() );
            rObj.SetRect( Rectangle( rTextRefPoint, Point( rTextRefPoint.X() +
                                fDist, rTextRefPoint.Y() - 1 ) ));

            mnTextStyle = EPP_TEXTSTYLE_TEXT;
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            mpEscherEx->AddShape( ESCHER_ShpInst_TextBox, 0xa00 );
            mpEscherEx->BeginCount();
            ImplWriteTextBundle( rObj );

            mpEscherEx->AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90000 );
            mpEscherEx->AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100000 );
            mpEscherEx->AddOpt( ESCHER_Prop_FitTextToShape, 0x60006 );      // Size Shape To Fit Text
            if ( nAngle < 0 )
                nAngle = ( 36000 + nAngle ) % 36000;
            if ( nAngle )
                ImplFlipBoundingBox( rObj, rObj.GetRect().TopLeft() );
        }
        else
        {
            mnTextStyle = EPP_TEXTSTYLE_TEXT;
            mpEscherEx->OpenContainer( ESCHER_SpContainer );
            nShapeID = mpEscherEx->GetShapeID();
            mpEscherEx->AddShape( nShapeType = ESCHER_ShpInst_TextBox, 0xa00, nShapeID );
            mpEscherEx->BeginCount();
            ImplWriteTextBundle( rObj );

            mpEscherEx->AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90000 );
            mpEscherEx->AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100000 );

            if( nAngle < 0 )
                nAngle = ( 36000 + nAngle ) % 36000;
            else
                nAngle = ( 36000 - ( nAngle % 36000 ) );

            nAngle *= 655;
            nAngle += 0x8000;
            nAngle &=~0xffff;   // nAngle auf volle Gradzahl runden
            mpEscherEx->AddOpt( ESCHER_Prop_Rotation, nAngle );
            mpEscherEx->SetGroupSnapRect( mpEscherEx->GetGroupLevel(),
                                            rObj.GetRect() );
            mpEscherEx->SetGroupLogicRect( mpEscherEx->GetGroupLevel(),
                                            rObj.GetRect() );
        }
        rObj.SetAngle( nAngle );
        mpEscherEx->EndCount( ESCHER_OPT, 3 );

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
    mnPrevTextStyle = mnTextStyle;      // es darf nicht mehr als zwei geben
    mnEffectCount = 0;
    mbIsTitlePossible = TRUE;           // bei mehr als einem title geht powerpoint in die knie

    return TRUE;
}


// -------------------------------------------------------------------

void ImplEESdrWriter::ImplWritePage(
            ImplEESdrSolverContainer& rSolverContainer,
            ImplEESdrPageType ePageType, BOOL bBackGround )
{
#if 0
    ImplInitPageValues();

    UINT32  nShapes, nShapeCount, nLastPer;

    ResetGroupTable( nShapes = mXShapes->getCount() );

    nLastPer = nShapeCount = 0;

    while( GetNextGroupEntry() )
    {
        nShapeCount++;

        UINT32 nPer = ( 5 * nShapeCount ) / nShapes;
        if ( nPer != nLastPer )
        {
            nLastPer = nPer;
            UINT32 nValue = mnPagesWritten * 5 + nPer;
            if ( nValue > mnStatMaxValue )
                nValue = mnStatMaxValue;
            if ( mbStatusIndicator )
                mXStatusIndicator->setValue( nValue );
        }
        UINT32 nGroups = GetGroupsClosed();
        for ( UINT32 i = 0; i < nGroups; i++, mpEscherEx->LeaveGroup() );

        if ( ImplGetShapeByIndex( GetCurrentGroupIndex(), TRUE ) )
        {
            if( bBackGround && ( nShapeCount == 1 ) &&
                rObj.GetType().EqualsAscii( "drawing.Rectangle" ) )     // background shape auf der MasterPage
                continue;                                       // wird uebersprungen
            ImplWriteShape( rSolverContainer, ePageType );
        }
        mnPrevTextStyle = mnTextStyle;
    }
    ClearGroupTable();                              // gruppierungen wegschreiben, sofern noch irgendwelche offen sind, was eigendlich nicht sein sollte
    UINT32 nGroups = GetGroupsClosed();
    for ( UINT32 i = 0; i < nGroups; i++, mpEscherEx->LeaveGroup() );
    mnPagesWritten++;

#else

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

#if EES_WRITE_EPP
        mnPrevTextStyle = mnTextStyle;
#endif
    }

    mnPagesWritten++;

#endif
}


// -------------------------------------------------------------------

void ImplEESdrWriter::ImplWriteLineBundle( ImplEESdrObject& rObj, BOOL bEdge )
{
    UINT32 nLineFlags = 0x80008;
    ESCHER_LineEnd eLineEnd = ESCHER_LineArrowEnd;

    if( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("LineStart") ))
    {
        PointSequence* pPolyPolygon = (PointSequence*)rObj.GetUsrAny().getValue();
        INT32 nSequenceCount = pPolyPolygon->getLength();
        if ( nSequenceCount )
        {
            // Zeiger auf innere sequences holen
            awt::Point* pSequence = pPolyPolygon->getArray();
            if ( pSequence )
            {
                switch ( nSequenceCount )
                {
                    case 0x4 :
                    {
                        switch( pSequence->X )
                        {
                            case 0 : eLineEnd = ESCHER_LineArrowDiamondEnd; break;
                            case 0x529 : eLineEnd = ESCHER_LineArrowStealthEnd; break;
                        }
                    }
                    break;
                    case 0x7 : eLineEnd = ESCHER_LineArrowDiamondEnd; break;
                    case 0xa : eLineEnd = ESCHER_LineArrowStealthEnd; break;
                    case 0xd :
                    {
                        switch ( pSequence->X )
                        {
                            case 0 : eLineEnd = ESCHER_LineArrowDiamondEnd; break;
                            case 0x64 : eLineEnd = ESCHER_LineArrowOvalEnd; break;
                            case 0x87c : ESCHER_LineArrowStealthEnd; break;
                        }
                    }
                }
                mpEscherEx->AddOpt( ESCHER_Prop_lineStartArrowLength, 1 );
                mpEscherEx->AddOpt( ESCHER_Prop_lineStartArrowWidth, 1 );
                mpEscherEx->AddOpt( ESCHER_Prop_lineStartArrowhead, eLineEnd );
                nLineFlags |= 0x100010;
            }
        }
    }

    eLineEnd = ESCHER_LineArrowEnd;
    if( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("LineEnd") ))
    {
        PointSequence* pPolyPolygon = (PointSequence*)rObj.GetUsrAny().getValue();
        INT32 nSequenceCount = pPolyPolygon->getLength();
        if ( nSequenceCount )
        {
            // Zeiger auf innere sequences holen
            awt::Point* pSequence = pPolyPolygon->getArray();
            if ( pSequence )
            {
                switch ( nSequenceCount )
                {
                    case 0x4 :
                    {
                        switch( pSequence->X )
                        {
                            case 0 : eLineEnd = ESCHER_LineArrowDiamondEnd; break;
                            case 0x529 : eLineEnd = ESCHER_LineArrowStealthEnd; break;
                        }
                    }
                    break;
                    case 0x7 : eLineEnd = ESCHER_LineArrowDiamondEnd; break;
                    case 0xa : eLineEnd = ESCHER_LineArrowStealthEnd; break;
                    case 0xd :
                    {
                        switch ( pSequence->X )
                        {
                            case 0 : eLineEnd = ESCHER_LineArrowDiamondEnd; break;
                            case 0x64 : eLineEnd = ESCHER_LineArrowOvalEnd; break;
                              case 0x87c : ESCHER_LineArrowStealthEnd; break;
                        }
                    }
                }
                mpEscherEx->AddOpt( ESCHER_Prop_lineEndArrowLength, 1 );
                mpEscherEx->AddOpt( ESCHER_Prop_lineEndArrowWidth, 1 );
                mpEscherEx->AddOpt( ESCHER_Prop_lineEndArrowhead, eLineEnd );
                nLineFlags |= 0x100010;
            }
        }
    }

    rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("LineStyle"));

    LineStyle* pLineStyle = (LineStyle*)rObj.GetUsrAny().getValue();

    rObj.SetShadow( rObj.GetShadow() | 1 );
    if ( pLineStyle )
    {
        switch ( *pLineStyle )
        {
            case LineStyle_NONE :
            {
                mpEscherEx->AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90000 );         // 80000
                rObj.SetShadow( rObj.GetShadow() ^ 1 );
            }
            break;

            case LineStyle_DASH :
            {
                if ( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("LineDash") ))
                {
                    ESCHER_LineDashing eDash = ESCHER_LineSolid;
                    LineDash* pLineDash = (LineDash*)rObj.GetUsrAny().getValue();
                    UINT32 nDistance = pLineDash->Distance << 1;
                    switch ( pLineDash->Style )
                    {
                        case DashStyle_ROUND :
                        case DashStyle_ROUNDRELATIVE :
                            mpEscherEx->AddOpt( ESCHER_Prop_lineEndCapStyle, 0 );   // Style Round setzen
                        break;
                    }
                    if ( ((!(pLineDash->Dots )) ||
                        (!(pLineDash->Dashes )) ) ||
                        ( pLineDash->DotLen == pLineDash->DashLen ) )
                    {
                        UINT32 nLen = pLineDash->DotLen;
                        if ( pLineDash->Dashes )
                            nLen = pLineDash->DashLen;

                        if ( nLen >= nDistance )
                            eDash = ESCHER_LineLongDashGEL;
                        else if ( pLineDash->Dots )
                            eDash = ESCHER_LineDotSys;
                        else
                            eDash = ESCHER_LineDashGEL;
                    }
                    else                                                            // X Y
                    {
                        if ( pLineDash->Dots != pLineDash->Dashes )
                        {
                            if ( ( pLineDash->DashLen > nDistance ) ||
                                ( pLineDash->DotLen > nDistance ) )
                                eDash = ESCHER_LineLongDashDotDotGEL;
                            else
                                eDash = ESCHER_LineDashDotDotSys;
                        }
                        else                                                        // X Y Y
                        {
                            if ( ( pLineDash->DashLen > nDistance ) ||
                                ( pLineDash->DotLen > nDistance ) )
                                eDash = ESCHER_LineLongDashDotGEL;
                            else
                                eDash = ESCHER_LineDashDotGEL;

                        }
                    }
                    mpEscherEx->AddOpt( ESCHER_Prop_lineDashing, eDash );
                }
            }
            case LineStyle_SOLID :
            default:
            {
                mpEscherEx->AddOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags );
            }
            break;
        }
        if ( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("LineColor") ))
        {
            UINT32 nLineColor = mpEscherEx->GetColor(
                                    *((UINT32*)rObj.GetUsrAny().getValue()) );
            mpEscherEx->AddOpt( ESCHER_Prop_lineColor, nLineColor );
            mpEscherEx->AddOpt( ESCHER_Prop_lineBackColor, nLineColor ^ 0xffffff );
        }
    }

    UINT32 nLineSize = (UINT32)rObj.ImplGetInt32PropertyValue( ::rtl::OUString::createFromAscii("LineWidth"));

    if ( nLineSize > 1 )
        mpEscherEx->AddOpt( ESCHER_Prop_lineWidth, nLineSize * 360 );   // 100TH MM -> PT , 1PT = 12700 EMU
    if ( bEdge == FALSE )
    {
        mpEscherEx->AddOpt( ESCHER_Prop_fFillOK, 0x1001 );
        mpEscherEx->AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100000 );
    }
}


// -------------------------------------------------------------------

void ImplEESdrWriter::ImplWriteFillBundle( ImplEESdrObject& rObj, BOOL bEdge )
{
    mpEscherEx->AddOpt( ESCHER_Prop_WrapText, ESCHER_WrapNone );
    mpEscherEx->AddOpt( ESCHER_Prop_AnchorText, ESCHER_AnchorMiddle );

    UINT32 nFillBackColor = 0;
    rObj.SetShadow( rObj.GetShadow() | 2 );

    FillStyle* pFillStyle = ( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("FillStyle") ))
                                ? (FillStyle*)rObj.GetUsrAny().getValue()
                                : 0;
    if ( pFillStyle )
    {
        switch( *pFillStyle )
        {
            case FillStyle_GRADIENT :
            {
                if ( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("FillGradient") ))
                {
                    awt::Gradient* pVCLGradient = (awt::Gradient*)rObj.GetUsrAny().getValue();
                    mpEscherEx->WriteGradient( pVCLGradient );
                }
                mpEscherEx->AddOpt( ESCHER_Prop_fNoFillHitTest, 0x140014 );
            }
            break;

            case FillStyle_BITMAP :
            {
                ImplGetGraphic( rObj, ::rtl::OUString::createFromAscii("FillBitmap"), TRUE );
                mpEscherEx->AddOpt( ESCHER_Prop_fNoFillHitTest, 0x140014 );
                mpEscherEx->AddOpt( ESCHER_Prop_fillBackColor, nFillBackColor  );
            }
            break;
            case FillStyle_HATCH :
            case FillStyle_SOLID :
            default:
            {
                sal_Int16 nTransparency = 0;
                if( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("FillTransparence") ))
                    rObj.GetUsrAny() >>= nTransparency;
                if ( nTransparency != 100 )
                {
                    if ( nTransparency )    // opacity
                        mpEscherEx->AddOpt( ESCHER_Prop_fillOpacity, ( ( 100 - nTransparency ) << 16 ) / 100 );
                    if ( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("FillColor") ))
                    {
                        UINT32 nFillColor = mpEscherEx->GetColor(
                                    *((UINT32*)rObj.GetUsrAny().getValue()) );
                        nFillBackColor = nFillColor ^ 0xffffff;
                        mpEscherEx->AddOpt( ESCHER_Prop_fillColor, nFillColor );
                    }
                    mpEscherEx->AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100010 );
                    mpEscherEx->AddOpt( ESCHER_Prop_fillBackColor, nFillBackColor );
                    break;
                }
            }
            case FillStyle_NONE :
            {
                mpEscherEx->AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100000 );
                rObj.SetShadow( rObj.GetShadow() ^ 2 );
            }
            break;
        }
    }
    ImplWriteLineBundle( rObj, bEdge );
}


// -------------------------------------------------------------------
#ifdef USED
BOOL ImplEESdrWriter::ImplIsMetaFile( ImplEESdrObject& rObj )
{
    if( !rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("GraphicObjectFillBitmap") ))
        return FALSE;
    return ( rObj.GetUsrAny().getReflection() != XBitmap_getReflection() );
}
#endif

// -------------------------------------------------------------------

BOOL ImplEESdrWriter::ImplGetGraphic( ImplEESdrObject& rObj,
                                const sal_Unicode* pName, BOOL bFillBitmap )
{
    if( rObj.ImplGetPropertyValue( pName ) )
    {
#if EES_WRITE_EEP
        if ( !mpPicStrm )
            mpPicStrm = mrStg->OpenStream( "Pictures" );
#else // !EES_WRITE_EEP
        if ( !mpPicStrm )
            mpPicStrm = mpEscherEx->QueryPicStream();
#endif // EES_WRITE_EEP
        if ( mpPicStrm )
        {
            if( rObj.GetUsrAny().getValueType() == ::getCppuType(( const Reference< awt::XBitmap >*)0) )        // bitmap oder wmf ?
            {
                Reference< awt::XBitmap > xBitmap( *(Reference< awt::XBitmap >*)rObj.GetUsrAny().getValue() );
                if ( xBitmap.is() )
                {
                    BitmapEx aBitmapEx( VCLUnoHelper::GetBitmap(xBitmap) );
                    OUString aName( pName );

                    if( aName.compareToAscii("FillBitmap") == 0 &&
                        rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("FillBitmapTile")) &&
                        ::cppu::any2bool( rObj.GetUsrAny() ) )
                        mpEscherEx->AddOpt( ESCHER_Prop_fillType,
                                            ESCHER_FillTexture );
                    else
                    {
                        if( rObj.GetAngle() )       // PPoint kann keine gedrehten Grafiken darstellen !
                        {
                            rObj.SetAngle( ( rObj.GetAngle() + 5 ) / 10 );
                            Polygon aPoly( rObj.GetRect() );
                            aPoly.Rotate( rObj.GetRect().TopLeft(),
                                            rObj.GetAngle() );
                            rObj.SetRect( aPoly.GetBoundRect() );
                            aBitmapEx.Rotate( rObj.GetAngle(), COL_TRANSPARENT );
                            rObj.SetAngle( 0 );
                        }
                        mpEscherEx->AddOpt( ESCHER_Prop_fillType,
                                            ESCHER_FillPicture );
                    }
                    mpEscherEx->AddOpt( bFillBitmap ? ESCHER_Prop_fillBlip
                                                    : ESCHER_Prop_pib,
                                        mpEscherEx->AddGraphic( *mpPicStrm,
                                            Graphic( aBitmapEx ) ), TRUE );
                    return TRUE;
                }
            }
            else
            {
                BOOL        bExportEMF( FALSE );
                Rectangle   aRect( rObj.GetRect() );
                Any         aOLE2Any( rObj.GetUsrAny() );

                if ( !aOLE2Any.hasValue() )
                    return FALSE;

                if ( mpEscherEx->IsOleEmf() &&
                    rObj.GetType().EqualsAscii( "drawing.OLE2" ))
                {
                    if ( rObj.ImplGetPropertyValue( ::rtl::OUString::createFromAscii("OriginalSize") ) )
                    {
                        Any aAny( rObj.GetUsrAny() );
                        if ( aAny.getValueType() == ::getCppuType(( const awt::Size *)0) )
                        {
                            awt::Size aSize( *(awt::Size*)aAny.getValue() );
                            aRect = Rectangle( Point(), Size( aSize.Width, aSize.Height ) );    // only real ole2 objects supports this property
                            bExportEMF = TRUE;
                        }
                    }
                }
                if ( bExportEMF )
                {
                    Graphic aGraphic;
                    int nRes = 0;
                    {
                        Sequence<sal_Int8> aWmfSeq(*(Sequence<sal_Int8>*)aOLE2Any.getValue());
                        const sal_Int8* pAry = aWmfSeq.getArray();
                        UINT32 nAryLen = aWmfSeq.getLength();
                        if ( pAry && nAryLen )
                        {
                            SvMemoryStream aSource( (sal_Char*)pAry, nAryLen, STREAM_READ );
                            nRes = GraphicConverter::Import( aSource, aGraphic, CVT_WMF );
                        }
                    }
                    SvMemoryStream aDest;
                    nRes = GraphicConverter::Export( aDest, aGraphic, CVT_EMF );

                    if ( nRes )
                        return FALSE;

                    aDest.Seek( STREAM_SEEK_TO_END );
                    mpEscherEx->AddOpt( ESCHER_Prop_pib, mpEscherEx->AddEMF(
                                        *mpPicStrm, (const BYTE*)aDest.GetData(), aDest.Tell(),
                                        aRect ), TRUE );
                }
                else
                {
                    Sequence<sal_Int8> aWmfSeq(*(Sequence<sal_Int8>*)aOLE2Any.getValue());
                    const sal_Int8* pAry = aWmfSeq.getArray();
                    if ( pAry )
                        mpEscherEx->AddOpt( ESCHER_Prop_pib, mpEscherEx->AddWMF(
                                            *mpPicStrm, (BYTE*)pAry, aWmfSeq.getLength(),
                                            aRect ), TRUE );
                }
                return TRUE;
            }
        }
    }
    return FALSE;
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

            mpSolverContainer = new ImplEESdrSolverContainer;
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
        mpSolverContainer->WriteSolver( *mpEscherEx );
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
    mnShapeId( 0 ), mnAngle( 0 ), mnShadow( 0 ), mnTextSize( 0 )
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
            mXShape = pSvxDrawPage->_CreateShape( (SdrObject*) &rObj );
            Init( rEx );
        }
    }
}

ImplEESdrObject::ImplEESdrObject( ImplEESdrWriter& rEx,
                                    const Reference< XShape >& rShape )
    : mbValid( FALSE ), mbPresObj( FALSE ), mbEmptyPresObj( FALSE ),
    mnShapeId( 0 ), mnAngle( 0 ), mnShadow( 0 ), mXShape( rShape ),
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
        catch(...)
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
        TRY
        {
            mAny = rXPropSet->getPropertyValue( rString );
            if( 0 != mAny.get() )
                bRetValue = TRUE;
        }
        CATCH_ALL()
        {
            bRetValue = FALSE;
        }
        END_CATCH;
    }
    return bRetValue;
}
#endif

void ImplEESdrObject::SetRect( const Point& rPos, const Size& rSz )
{
    maPosition = rPos;
    maSize = rSz;
    maRect = Rectangle( maPosition, maSize );
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
    return xXText.is();
}

