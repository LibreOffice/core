/*************************************************************************
 *
 *  $RCSfile: epptso.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: sj $ $Date: 2000-11-10 08:21:52 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EPPT_HXX_
#include <eppt.hxx>
#endif
#ifndef _EPPT_DEF_HXX
#include "epptdef.hxx"
#endif
#ifndef __EscherEX_HXX
#include "escherex.hxx"
#endif
#ifndef _SV_POLY_HXX
#include <vcl/poly.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif
#ifndef _SV_GRADIENT_HXX
#include <vcl/gradient.hxx>
#endif
#ifndef _SV_GFXLINK_HXX
#include <vcl/gfxlink.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif
#ifndef _SV_GRADIENT_HXX
#include <vcl/gradient.hxx>
#endif
//#ifndef _SVX_XIT_HXX
//#include <svx/xit.hxx>
//#endif
#ifndef _SVX_SVXENUM_HXX
#include <svx/svxenum.hxx>
#endif
#ifndef _COM_SUN_STAR_STYLE_VERTICALALIGNMENT_HPP_
#include <com/sun/star/style/VerticalAlignment.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_XPRESENTATIONPAGE_HPP_
#include <com/sun/star/presentation/XPresentationPage.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFONT_HPP_
#include <com/sun/star/awt/XFont.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFONTWEIGHT_HPP_
#include <com/sun/star/awt/FontWeight.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_TABSTOP_HPP_
#include <com/sun/star/style/TabStop.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_PARAGRAPHADJUST_HPP_
#include <com/sun/star/style/ParagraphAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_LINESPACING_HPP_
#include <com/sun/star/style/LineSpacing.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_LINESPACINGMODE_HPP_
#include <com/sun/star/style/LineSpacingMode.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_PP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_CHARSET_HPP_
#include <com/sun/star/awt/CharSet.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FLAGSEQUENCE_HPP_
#include <com/sun/star/drawing/FlagSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYGONFLAGS_HPP_
#include <com/sun/star/drawing/PolygonFlags.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_COLORMODE_HPP_
#include <com/sun/star/drawing/ColorMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_GRAPHICCROP_HPP_
#include <com/sun/star/text/GraphicCrop.hpp>
#endif
#ifndef _SV_CVTGRF_HXX
#include <vcl/cvtgrf.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _CPPUHELPER_PROPTYPEHLP_HXX_
#include <cppuhelper/proptypehlp.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX_
#include <ucbhelper/content.hxx>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX_
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/unohlp.hxx>
#endif
#ifndef _RTL_CRC_H_
#include <rtl/crc.h>
#endif
#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#include <svtools/fltcall.hxx>

//#include <svx/xbtmpit.hxx>


#include <vos/xception.hxx>
#ifndef _VOS_NO_NAMESPACE
using namespace vos;
#endif

#define ANY_FLAGS_LINE          0x01
#define ANY_FLAGS_POLYLINE      0x02
#define ANY_FLAGS_POLYPOLYGON   0x04

// ---------------------------------------------------------------------------------------------

sal_Int16 EncodeAnyTosal_Int16( ::com::sun::star::uno::Any& rAny )
{
        sal_Int16 nVal = 0;
#ifdef __BIGENDIAN
        switch( rAny.getValueType().getTypeClass() )
        {
            case ::com::sun::star::uno::TypeClass_SHORT :
            case ::com::sun::star::uno::TypeClass_UNSIGNED_SHORT : nVal = *((short*)rAny.getValue() ); break;
            case ::com::sun::star::uno::TypeClass_LONG :
            case ::com::sun::star::uno::TypeClass_UNSIGNED_LONG : nVal = (sal_Int16)(*((long*)rAny.getValue() ) ); break;
            case ::com::sun::star::uno::TypeClass_ENUM :
            {
                 nVal = (sal_Int16)(*( ::com::sun::star::drawing::TextAdjust*)rAny.getValue() ); break;
            }
            break;
        }
#else
        nVal = *((sal_Int16*)rAny.getValue());
#endif
        return nVal;
}

PPTExBulletProvider::PPTExBulletProvider()
{
    pGraphicProv = new _EscherGraphicProvider( aBuExPictureStream, _E_GRAPH_PROV_USE_INSTANCES  | _E_GRAPH_PROV_DO_NOT_ROTATE_METAFILES );
}

PPTExBulletProvider::~PPTExBulletProvider()
{
    delete pGraphicProv;
}

sal_uInt16 PPTExBulletProvider::GetId( Graphic& rGraphic )
{
    sal_uInt16 nRetValue = 0xffff;
    if ( !!rGraphic )
    {
        sal_uInt32 nId = 0;
        if ( rGraphic.GetType() == GRAPHIC_BITMAP )
        {
            Rectangle   aRect;
            GraphicObject aGraphicObject( rGraphic );
            nId = pGraphicProv->ImplGetBlibID( aGraphicObject.GetUniqueID(), aRect, NULL );
        }
        else
        {
//          sal_uInt32 nId = pGraphicProv->ImplGetBlibID( aDestStrm, WMF );
            nId = 0;
        }
        if ( nId && ( nId < 0x10000 ) )
            nRetValue = (sal_uInt16)nId - 1;
    }
    return nRetValue;
}

// ---------------------------------------------------------------------------------------------

GroupTable::GroupTable() :
    mpGroupEntry        ( NULL ),
    mnMaxGroupEntry     ( 0 ),
    mnCurrentGroupEntry ( 0 ),
    mnGroupsClosed      ( 0 )
{
    ImplResizeGroupTable( 32 );
}

// ---------------------------------------------------------------------------------------------

GroupTable::~GroupTable()
{
    for ( sal_uInt32 i = 0; i < mnCurrentGroupEntry; delete mpGroupEntry[ i++ ] );
    delete mpGroupEntry;
}

// ---------------------------------------------------------------------------------------------

void GroupTable::ImplResizeGroupTable( sal_uInt32 nEntrys )
{
    if ( nEntrys > mnMaxGroupEntry )
    {
        mnMaxGroupEntry         = nEntrys;
        GroupEntry** pTemp = new GroupEntry*[ nEntrys ];
        for ( sal_uInt32 i = 0; i < mnCurrentGroupEntry; i++ )
            pTemp[ i ] = mpGroupEntry[ i ];
        if ( mpGroupEntry )
            delete mpGroupEntry;
        mpGroupEntry = pTemp;
    }
}

// ---------------------------------------------------------------------------------------------

void GroupTable::EnterGroup( ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rXIndexAccessRef )
{
    if ( rXIndexAccessRef.is() )
    {
        if ( mnMaxGroupEntry == mnCurrentGroupEntry )
            ImplResizeGroupTable( mnMaxGroupEntry + 8 );
        mpGroupEntry[ mnCurrentGroupEntry++ ] = new GroupEntry( rXIndexAccessRef );
    }
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 GroupTable::GetGroupsClosed()
{
    sal_uInt32 nRet = mnGroupsClosed;
    mnGroupsClosed = 0;
    return nRet;
}

// ---------------------------------------------------------------------------------------------

void GroupTable::ClearGroupTable()
{
    for ( sal_uInt32 i = 0; i < mnCurrentGroupEntry; i++, delete mpGroupEntry );
    mnCurrentGroupEntry = 0;
}

// ---------------------------------------------------------------------------------------------

void GroupTable::ResetGroupTable( sal_uInt32 nCount )
{
    ClearGroupTable();
    mpGroupEntry[ mnCurrentGroupEntry++ ] = new GroupEntry( nCount );
}

// ---------------------------------------------------------------------------------------------

sal_Bool GroupTable::GetNextGroupEntry()
{
    while ( mnCurrentGroupEntry )
    {
        mnIndex = mpGroupEntry[ mnCurrentGroupEntry - 1 ]->mnCurrentPos++;

        if ( mpGroupEntry[ mnCurrentGroupEntry - 1 ]->mnCount > mnIndex )
            return TRUE;

        delete ( mpGroupEntry[ --mnCurrentGroupEntry ] );

        if ( mnCurrentGroupEntry )
            mnGroupsClosed++;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------------------------

void GroupTable::SkipCurrentGroup()
{
    if ( mnCurrentGroupEntry )
        delete ( mpGroupEntry[ --mnCurrentGroupEntry ] );
}

// ---------------------------------------------------------------------------------------------

Collection::~Collection()
{
    for( void* pStr = List::First(); pStr; pStr = List::Next() )
        delete (String*) pStr;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 Collection::GetId( const String& rString )
{
    if( rString.Len() )
    {
        const sal_uInt32 nCount = GetCount();

        for( sal_uInt32 i = 0; i < nCount; i++ )
            if( *GetById( i ) == rString )
                return i;

        List::Insert( new String( rString ), LIST_APPEND );
        return nCount;
    }
    return 0;
}

sal_uInt32 Collection::GetCount() const
{
    return List::Count();
}

const String* Collection::GetById( sal_uInt32 nId )
{
    return (String*) List::GetObject( nId );
}

// ---------------------------------------------------------------------------------------------
// bei Rechtecken           bei Ellipsen    bei Polygonen
//
// nRule =  0 ->Top         0 ->Top         nRule = Index auf ein (Poly)Polygon Punkt
//          1 ->Left        2 ->Left
//          2 ->Bottom      4 ->Bottom
//          3 ->Right       6 ->Right


sal_uInt32 ConnectorListEntry::GetClosestPoint( const Polygon& rPoly, const ::com::sun::star::awt::Point& rPoint )
{
    sal_uInt32 nCount = rPoly.GetSize();
    sal_uInt32 nClosest = nCount;
    double fDist = (sal_uInt32)0xffffffff;
    while( nCount-- )
    {
        double fDistance = hypot( rPoint.X - rPoly[ nCount ].X(), rPoint.Y - rPoly[ nCount ].Y() );
        if ( fDistance < fDist )
        {
            nClosest =  nCount;
            fDist = fDistance;
        }
    }
    return nClosest;
};

sal_uInt32 ConnectorListEntry::GetConnectorRule( sal_Bool bFirst )
{
    sal_uInt32 nRule = 0;

    ::com::sun::star::uno::Any aAny;
    ::com::sun::star::awt::Point aRefPoint( ( bFirst ) ? maPointA : maPointB );
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        aXShape( ( bFirst ) ? mXConnectToA : mXConnectToB );

    String aString( (::rtl::OUString)aXShape->getShapeType() );
    ByteString aType( aString, RTL_TEXTENCODING_UTF8 );
    aType.Erase( 0, 19 );   // "smart.com.sun.star." entfernen
    sal_uInt16 nPos = aType.Search( "Shape" );
    aType.Erase( nPos, 5 );

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
        aPropertySet( aXShape, ::com::sun::star::uno::UNO_QUERY );

    if ( aType == "drawing.PolyPolygon" || aType == "drawing.PolyLine" )
    {
        if ( aPropertySet.is() )
        {
            if ( PropValue::GetPropertyValue( aAny, aPropertySet, String( RTL_CONSTASCII_USTRINGPARAM( "PolyPolygon" ) ) ) )
            {
                ::com::sun::star::drawing::PointSequenceSequence* pSourcePolyPolygon =
                    (::com::sun::star::drawing::PointSequenceSequence*)aAny.getValue();
                sal_Int32 nOuterSequenceCount = pSourcePolyPolygon->getLength();
                ::com::sun::star::drawing::PointSequence* pOuterSequence = pSourcePolyPolygon->getArray();

                if ( pOuterSequence )
                {
                    sal_Int32 a, b, nIndex = 0;
                    sal_uInt32 nDistance = 0xffffffff;
                    for( a = 0; a < nOuterSequenceCount; a++ )
                    {
                        ::com::sun::star::drawing::PointSequence* pInnerSequence = pOuterSequence++;
                        if ( pInnerSequence )
                        {
                            ::com::sun::star::awt::Point* pArray = pInnerSequence->getArray();
                            if ( pArray )
                            {
                                for ( b = 0; b < pInnerSequence->getLength(); b++, nIndex++, pArray++ )
                                {
                                    sal_uInt32 nDist = (sal_uInt32)hypot( aRefPoint.X - pArray->X, aRefPoint.Y - pArray->Y );
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
    }
    else if ( ( aType == "drawing.OpenBezier" ) || ( aType == "drawing.OpenFreeHand" ) || ( aType == "drawing.PolyLinePath" )
        || ( aType == "drawing.ClosedBezier" ) || ( aType == "drawing.ClosedFreeHand" ) || ( aType == "drawing.PolyPolygonPath" ) )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
            aPropertySet( aXShape, ::com::sun::star::uno::UNO_QUERY );
        if ( aPropertySet.is() )
        {
            if ( PropValue::GetPropertyValue( aAny, aPropertySet, String( RTL_CONSTASCII_USTRINGPARAM( "PolyPolygonBezier" ) ) ) )
            {
                ::com::sun::star::drawing::PolyPolygonBezierCoords* pSourcePolyPolygon =
                    (::com::sun::star::drawing::PolyPolygonBezierCoords*)aAny.getValue();
                sal_Int32 nOuterSequenceCount = pSourcePolyPolygon->Coordinates.getLength();

                // Zeiger auf innere sequences holen
                ::com::sun::star::drawing::PointSequence* pOuterSequence =
                    pSourcePolyPolygon->Coordinates.getArray();
                ::com::sun::star::drawing::FlagSequence*  pOuterFlags =
                    pSourcePolyPolygon->Flags.getArray();

                if ( pOuterSequence && pOuterFlags )
                {
                    sal_Int32 a, b, nIndex = 0;
                    sal_uInt32 nDistance = 0xffffffff;

                    for ( a = 0; a < nOuterSequenceCount; a++ )
                    {
                        ::com::sun::star::drawing::PointSequence* pInnerSequence = pOuterSequence++;
                        ::com::sun::star::drawing::FlagSequence*  pInnerFlags = pOuterFlags++;
                        if ( pInnerSequence && pInnerFlags )
                        {
                            ::com::sun::star::awt::Point* pArray = pInnerSequence->getArray();
                            ::com::sun::star::drawing::PolygonFlags* pFlags = pInnerFlags->getArray();
                            if ( pArray && pFlags )
                            {
                                for ( b = 0; b < pInnerSequence->getLength(); b++, pArray++ )
                                {
                                    PolyFlags ePolyFlags = *( (PolyFlags*)pFlags++ );
                                    if ( ePolyFlags == POLY_CONTROL )
                                        continue;
                                    sal_uInt32 nDist = (sal_uInt32)hypot( aRefPoint.X - pArray->X, aRefPoint.Y - pArray->Y );
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
    }
    else
    {
        ::com::sun::star::awt::Point aPoint( aXShape->getPosition() );
        ::com::sun::star::awt::Size  aSize( aXShape->getSize() );

        Rectangle   aRect( Point( aPoint.X, aPoint.Y ), Size( aSize.Width, aSize.Height ) );
        Point       aCenter( aRect.Center() );
        Polygon     aPoly( 4 );

        aPoly[ 0 ] = Point( aCenter.X(), aRect.Top() );
        aPoly[ 1 ] = Point( aRect.Left(), aCenter.Y() );
        aPoly[ 2 ] = Point( aCenter.X(), aRect.Bottom() );
        aPoly[ 3 ] = Point( aRect.Right(), aCenter.Y() );

        sal_Int32 nAngle = ( PropValue::GetPropertyValue( aAny,
                                aPropertySet, String( RTL_CONSTASCII_USTRINGPARAM( "RotateAngle" ) ) ) )
            ? *((sal_Int32*)aAny.getValue() ) : 0;
        if ( nAngle )
            aPoly.Rotate( aRect.TopLeft(), ( nAngle + 5 ) / 10 );
        nRule = GetClosestPoint( aPoly, aRefPoint );
    }
    if ( aType == "drawing.Ellipse" )
        nRule <<= 1;    // In PPT hat eine Ellipse 8 Möglichkeiten sich zu connecten

    return nRule;
}

// ---------------------------------------------------------------------------------------------

SolverContainer::~SolverContainer()
{
    for( void* pP = maShapeList.First(); pP; pP = maShapeList.Next() )
        delete (ShapeListEntry*)pP;
    for( pP = maConnectorList.First(); pP; pP = maConnectorList.Next() )
        delete (ConnectorListEntry*)pP;
}

// ---------------------------------------------------------------------------------------------

void SolverContainer::AddShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rXShape, sal_uInt32 nId )
{
    maShapeList.Insert( new ShapeListEntry( rXShape, nId ), LIST_APPEND );
}

// ---------------------------------------------------------------------------------------------

void SolverContainer::AddConnector( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rConnector,
                                        const ::com::sun::star::awt::Point& rPA,
                                    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rConA,
                                        const ::com::sun::star::awt::Point& rPB,
                                    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rConB )
{
    maConnectorList.Insert( new ConnectorListEntry( rConnector, rPA, rConA, rPB, rConB ), LIST_APPEND );
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 SolverContainer::ImplGetId( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rXShape )
{
    for ( ShapeListEntry* pPtr = (ShapeListEntry*)maShapeList.First(); pPtr; pPtr = (ShapeListEntry*)maShapeList.Next() )
    {
        if ( rXShape == pPtr->aXShape )
            return ( pPtr->n_EscherId );
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------

void SolverContainer::WriteSolver( SvStream* pStrm, _EscherEx* p_EscherEx )
{
    sal_uInt32 nCount = maConnectorList.Count();
    if ( nCount )
    {
        p_EscherEx->OpenContainer( _Escher_SolverContainer, nCount );

        ConnectorRule aConnectorRule;
        aConnectorRule.nRuleId = 2;
        for ( ConnectorListEntry* pPtr = (ConnectorListEntry*)maConnectorList.First(); pPtr; pPtr = (ConnectorListEntry*)maConnectorList.Next() )
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
            p_EscherEx->AddAtom( 24, _Escher_ConnectorRule, 1 );
            *pStrm  << aConnectorRule.nRuleId
                    << aConnectorRule.nShapeA
                    << aConnectorRule.nShapeB
                    << aConnectorRule.nShapeC
                    << aConnectorRule.ncptiA
                    << aConnectorRule.ncptiB;

            aConnectorRule.nRuleId += 2;
        }
        p_EscherEx->CloseContainer();   // _Escher_SolverContainer
    }
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplVBAInfoContainer( SvStream* pStrm )
{
    sal_uInt32 nSize = 28;
    if ( pStrm )
    {
        *pStrm << (sal_uInt32)( 0x1f | ( EPP_VBAInfo << 16 ) )
               << (sal_uInt32)( nSize - 8 )
               << (sal_uInt32)( 2 | ( EPP_VBAInfoAtom << 16 ) )
               << (sal_uInt32)12;
        mp_EscherEx->InsertPersistOffset( EPP_Persist_VBAInfoAtom, pStrm->Tell() );
        *pStrm << (sal_uInt32)0
               << (sal_uInt32)0
               << (sal_uInt32)1;
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplSlideViewInfoContainer( sal_uInt32 nInstance, SvStream* pStrm )
{
    sal_uInt32 nSize = 111;
    if ( pStrm )
    {
        sal_uInt8 bShowGuides = 0;
        sal_uInt8 bSnapToGrid = 1;
        sal_uInt8 bSnapToShape = 0;

        sal_Int32 nScaling = 85;
        sal_Int32 nMasterCoordinate = 0xdda;
        sal_Int32 nXOrigin = -780;
        sal_Int32 nYOrigin = -84;

        sal_Int32 nPosition1 = 0x870;
        sal_Int32 nPosition2 = 0xb40;

        if ( nInstance )
        {
            bShowGuides = 1;
            nScaling = 0x3b;
            nMasterCoordinate = 0xf0c;
            nXOrigin = -1752;
            nYOrigin = -72;
            nPosition1 = 0xb40;
            nPosition2 = 0x870;
        }
        *pStrm << (sal_uInt32)( 0xf | ( EPP_SlideViewInfo << 16 ) | ( nInstance << 4 ) )
               << (sal_uInt32)( nSize - 8 )
               << (sal_uInt32)( EPP_SlideViewInfoAtom << 16 ) << (sal_uInt32)3
               << bShowGuides << bSnapToGrid << bSnapToShape
               << (sal_uInt32)( EPP_ViewInfoAtom << 16 ) << (sal_uInt32)52
               << nScaling << (sal_Int32)100 << nScaling << (sal_Int32)100  // scaling atom - Keeps the current scale
               << nScaling << (sal_Int32)100 << nScaling << (sal_Int32)100  // scaling atom - Keeps the previous scale
               << (sal_Int32)0x17ac << nMasterCoordinate// Origin - Keeps the origin in master coordinates
               << nXOrigin << nYOrigin              // Origin
               << (sal_uInt8)1                          // Bool1 varScale - Set if zoom to fit is set
               << (sal_uInt8)0                          // bool1 draftMode - Not used
               << (sal_uInt16)0                         // padword
               << (sal_uInt32)( ( 7 << 4 ) | ( EPP_GuideAtom << 16 ) ) << (sal_uInt32)8
               << (sal_uInt32)0     // Type of the guide. If the guide is horizontal this value is zero. If it's vertical, it's one.
               << nPosition1    // Position of the guide in master coordinates. X coordinate if it's vertical, and Y coordinate if it's horizontal.
               << (sal_uInt32)( ( 7 << 4 ) | ( EPP_GuideAtom << 16 ) ) << (sal_uInt32)8
               << (sal_Int32)1      // Type of the guide. If the guide is horizontal this value is zero. If it's vertical, it's one.
               << nPosition2;   // Position of the guide in master coordinates. X coordinate if it's vertical, and Y coordinate if it's horizontal.
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplOutlineViewInfoContainer( SvStream* pStrm )
{
    sal_uInt32 nSize = 68;
    if ( pStrm )
    {
        *pStrm << (sal_uInt32)( 0xf | ( EPP_OutlineViewInfo << 16 ) ) << (sal_uInt32)( nSize - 8 )
               << (sal_uInt32)( EPP_ViewInfoAtom << 16 ) << (sal_uInt32)52
               << (sal_Int32)170 << (sal_Int32)200 << (sal_Int32)170 << (sal_Int32)200  // scaling atom - Keeps the current scale
               << (sal_Int32)170 << (sal_Int32)200 << (sal_Int32)170 << (sal_Int32)200  // scaling atom - Keeps the previous scale
               << (sal_Int32)0x17ac << 0xdda    // Origin - Keeps the origin in master coordinates
               << (sal_Int32)-780 << (sal_Int32)-84 // Origin
               << (sal_uInt8)1                  // bool1 varScale - Set if zoom to fit is set
               << (sal_uInt8)0                  // bool1 draftMode - Not used
               << (sal_uInt16)0;                // padword
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplProgBinaryTag( SvStream* pStrm )
{
    sal_uInt32 nPictureStreamSize, nOutlineStreamSize, nSize = 8;

    nPictureStreamSize = aBuExPictureStream.Tell();
    if ( nPictureStreamSize )
        nSize += nPictureStreamSize + 8;

    nOutlineStreamSize = aBuExOutlineStream.Tell();
    if ( nOutlineStreamSize )
        nSize += nOutlineStreamSize + 8;

    if ( pStrm )
    {
        *pStrm << (sal_uInt32)( EPP_BinaryTagData << 16 ) << (sal_uInt32)( nSize - 8 );
        if ( nPictureStreamSize )
        {
            *pStrm << (sal_uInt32)( 0xf | ( EPP_PST_ExtendedBuGraContainer << 16 ) ) << nPictureStreamSize;
            pStrm->Write( aBuExPictureStream.GetData(), nPictureStreamSize );
        }
        if ( nOutlineStreamSize )
        {
            *pStrm << (sal_uInt32)( 0xf | ( EPP_PST_ExtendedPresRuleContainer << 16 ) ) << nOutlineStreamSize;
            pStrm->Write( aBuExOutlineStream.GetData(), nOutlineStreamSize );
        }
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplProgBinaryTagContainer( SvStream* pStrm, SvMemoryStream* pBinTagStrm )
{
    sal_uInt32 nSize = 8 + 8 + 14;
    if ( pStrm )
    {
        *pStrm << (sal_uInt32)( 0xf | ( EPP_ProgBinaryTag << 16 ) ) << (sal_uInt32)0
               << (sal_uInt32)( EPP_CString << 16 ) << (sal_uInt32)14
               << (sal_uInt32)0x5f005f << (sal_uInt32)0x50005f
               << (sal_uInt32)0x540050 << (sal_uInt16)0x39;
    }
    if ( pBinTagStrm )
    {
        sal_uInt32 nLen = pBinTagStrm->Tell();
        nSize += nLen + 8;
        *pStrm << (sal_uInt32)( EPP_BinaryTagData << 16 ) << nLen;
        pStrm->Write( pBinTagStrm->GetData(), nLen );
    }
    else
        nSize += ImplProgBinaryTag( pStrm );

    if ( pStrm )
    {
        pStrm->SeekRel( - ( nSize - 4 ) );
        *pStrm << (sal_uInt32)( nSize - 8 );
        pStrm->SeekRel( nSize - 8 );
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplProgTagContainer( SvStream* pStrm, SvMemoryStream* pBinTagStrm )
{
    sal_uInt32 nSize = 0;
    if ( aBuExPictureStream.Tell() || aBuExOutlineStream.Tell() || pBinTagStrm )
    {
        nSize = 8;
        if ( pStrm )
        {
            *pStrm << (sal_uInt32)( 0xf | ( EPP_ProgTags << 16 ) ) << (sal_uInt32)0;
        }
        nSize += ImplProgBinaryTagContainer( pStrm, pBinTagStrm );
        if ( pStrm )
        {
            pStrm->SeekRel( - ( nSize - 4 ) );
            *pStrm << (sal_uInt32)( nSize - 8 );
            pStrm->SeekRel( nSize - 8 );
        }
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplDocumentListContainer( SvStream* pStrm )
{
    sal_uInt32 nSize = 8;
    if ( pStrm )
    {
        *pStrm << (sal_uInt32)( ( EPP_List << 16 ) | 0xf ) << (sal_uInt32)0;
    }

    nSize += ImplVBAInfoContainer( pStrm );
    nSize += ImplSlideViewInfoContainer( 0, pStrm );
    nSize += ImplOutlineViewInfoContainer( pStrm );
    nSize += ImplSlideViewInfoContainer( 1, pStrm );
    nSize += ImplProgTagContainer( pStrm );

    if ( pStrm )
    {
        pStrm->SeekRel( - ( nSize - 4 ) );
        *pStrm << (sal_uInt32)( nSize - 8 );
        pStrm->SeekRel( nSize - 8 );
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplMasterSlideListContainer( SvStream* pStrm )
{
    sal_uInt32 i, nSize = 28 * mnMasterPages + 8;
    if ( pStrm )
    {
        *pStrm << (sal_uInt32)( 0x1f | ( EPP_SlideListWithText << 16 ) ) << (sal_uInt32)( nSize - 8 );

        for ( i = 0; i < mnMasterPages; i++ )
        {
            *pStrm << (sal_uInt32)( EPP_SlidePersistAtom << 16 ) << (sal_uInt32)20;
            mp_EscherEx->InsertPersistOffset( EPP_MAINMASTER_PERSIST_KEY | i, pStrm->Tell() );
            *pStrm << (sal_uInt32)0                 // psrReference - logical reference to the slide persist object ( EPP_MAINMASTER_PERSIST_KEY )
                   << (sal_uInt32)0                 // flags - only bit 3 used, if set then slide contains shapes other than placeholders
                   << (sal_Int32)0                  // numberTexts - number of placeholder texts stored with the persist object. Allows to display outline view without loading the slide persist objects
                   << (sal_Int32)( 0x80000000 | i ) // slideId - Unique slide identifier, used for OLE link monikers for example
                   << (sal_uInt32)0;                // reserved, usualy 0
        }
    }
    return nSize;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplCloseDocument()
{
    sal_uInt32 nOfs = mp_EscherEx->PtGetOffsetByID( EPP_Persist_Document );
    if ( nOfs )
    {
        mp_EscherEx->PtReplaceOrInsert( EPP_Persist_CurrentPos, mpStrm->Tell() );
        mpStrm->Seek( nOfs );

        mpExEmbed->Seek( STREAM_SEEK_TO_END );
        sal_uInt32 nExEmbedSize = mpExEmbed->Tell();

        // nEnviroment : Gesamtgroesse des Environment Containers
        sal_uInt32 nEnvironment = maFontCollection.GetCount() * 76  // 68 bytes pro Fontenityatom und je 8 Bytes fuer die Header
                                + 8                             // 1 FontCollection Container
                                + 20                            // SrKinsoku Container
                                + 18                            // 1 TxSiStyleAtom
                                + 118;                          // 1 TxMasterStyleAtom;

        sal_uInt32 nBytesToInsert = nEnvironment + 8;

        if ( nExEmbedSize )
            nBytesToInsert += nExEmbedSize + 8 + 12;

        // die Gesamtgroesse des evntl. spaeter folgenden Sound Containers ermitteln
        sal_uInt32 nSound = 0;
        sal_uInt32 nValidSoundCount = 0;
        if ( maSoundCollection.GetCount() )
        {
            for ( sal_uInt32 i = 0; i < maSoundCollection.GetCount(); i++ )
            {
                const String* pSoundFile = maSoundCollection.GetById( i );
                if ( pSoundFile )
                {
                    USHORT nStringLen = pSoundFile->Len();
                    if ( nStringLen )
                    {
                        try
                        {
                            ::ucb::Content aCnt( *pSoundFile,
                                                    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );
                            sal_Int64 nVal;

                            ::cppu::convertPropertyValue( nVal, aCnt.getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "size" ) ) ) );
                            sal_uInt32 nSizeOfSound = (sal_uInt32) nVal;

                            if ( nSizeOfSound )
                            {
                                nSound += 2 * nStringLen + 8;   // Name Of Sound ( instance 0 )
                                if ( nStringLen > 4 )
                                {
                                    if ( '.' == pSoundFile->GetChar( nStringLen - 4 ) )
                                        nSound += 16;           // Type Of Sound ( instance 1 )
                                }
                                String aString = UniString::CreateFromInt32( i + 1 );
                                nSound += 2 * aString.Len() + 8;// reference Id  ( instance 2 )
                                nSound += nSizeOfSound + 8;     // SoundData Atom;
                                nValidSoundCount++;
                            }
                        }
                        catch( ... )
                        {
                        }
                    }
                }
            }
            if ( nValidSoundCount )
            {
                nSound += 12 + nValidSoundCount * 8;    // Size der SoundContainer + SoundCollAtom
                nBytesToInsert += ( nSound + 8 );       // Size der SoundCollection Container + nSound
            }
        }
        nBytesToInsert += mp_EscherEx->DrawingGroupContainerSize();
        nBytesToInsert += ImplMasterSlideListContainer( NULL );
        nBytesToInsert += ImplDocumentListContainer( NULL );

        // nBytes im Stream einfuegen, und abhaengige Container anpassen
        mp_EscherEx->InsertAtCurrentPos( nBytesToInsert, TRUE );

        // CREATE HYPERLINK CONTAINER
        if ( nExEmbedSize )
        {
            *mpStrm << (sal_uInt16)0xf
                    << (sal_uInt16)EPP_ExObjList
                    << (sal_uInt32)( nExEmbedSize + 12 )
                    << (sal_uInt16)0
                    << (sal_uInt16)EPP_ExObjListAtom
                    << (sal_uInt32)4
                    << (sal_uInt32)mnExEmbed;
            mp_EscherEx->InsertPersistOffset( EPP_Persist_ExObj, mpStrm->Tell() );
            mpStrm->Write( mpExEmbed->GetData(), nExEmbedSize );
        }

        // CREATE ENVIRONMENT
        *mpStrm << (sal_uInt16)0xf << (sal_uInt16)EPP_Environment << (sal_uInt32)nEnvironment;

        // Open Container ( EPP_SrKinsoku )
        *mpStrm << (sal_uInt16)0x2f << (sal_uInt16)EPP_SrKinsoku << (sal_uInt32)12;
        mp_EscherEx->AddAtom( 4, EPP_SrKinsokuAtom, 0, 3 );
        *mpStrm << (sal_Int32)0;                        // SrKinsoku Level 0

        // Open Container ( EPP_FontCollection )
        *mpStrm << (sal_uInt16)0xf << (sal_uInt16)EPP_FontCollection << (sal_uInt32)maFontCollection.GetCount() * 76;

        for ( sal_uInt32 i = 0; i < maFontCollection.GetCount(); i++ )
        {
            mp_EscherEx->AddAtom( 68, EPP_FontEnityAtom, 0, i );
            const String* pEntry = maFontCollection.GetById( i );
            sal_uInt32 nFontLen = pEntry->Len();
            for ( sal_uInt32 n = 0; n < 34; n++ )
            {
                sal_Unicode nUniCode = 0;
                if ( ( n < 31 ) && ( n < nFontLen ) )
                    nUniCode = pEntry->GetChar( n );
                *mpStrm << nUniCode;
            }
        }
        mp_EscherEx->AddAtom( 10, EPP_TxSIStyleAtom );
        *mpStrm << (sal_uInt32)7                        // ?
                << (sal_Int16)2                         // ?
                << (sal_uInt8)9                          // ?
                << (sal_uInt8)8                          // ?
                << (sal_Int16)0;                        // ?
        mp_EscherEx->AddAtom( 110, EPP_TxMasterStyleAtom, 0, 4 );
        static sal_uInt8 aTxSuStyleAtom[ 110 ] =
        {
            0x05, 0x00, 0xff, 0xfd, 0x3f, 0x00, 0x00, 0x00, 0x22, 0x20, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02,
            0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xff, 0xff, 0xef, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x18, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05,
            0x00, 0x00, 0x20, 0x01, 0x20, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x40, 0x02,
            0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x60, 0x03, 0x60, 0x03, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x80, 0x04, 0x80, 0x04, 0x00, 0x00, 0x00, 0x00
        };
        mpStrm->Write( &aTxSuStyleAtom, 110 );

        if ( nValidSoundCount )
        {
            sal_uInt8* pBuf = new sal_uInt8[ 0x10000 ];   // 64 kB  Buffer

            // Create Sound Container
            *mpStrm << (sal_uInt16)0xf << (sal_uInt16)EPP_SoundCollection << (sal_uInt32)nSound;

            // Create SoundCollAtom ( Referenz auf die naechste Freie SoundId );
            mp_EscherEx->AddAtom( 4, EPP_SoundCollAtom );
            *mpStrm << (sal_uInt32)( maSoundCollection.GetCount() );

            for ( sal_uInt32 i = 0; i < maSoundCollection.GetCount(); i++ )
            {
                const String* pSoundFile = maSoundCollection.GetById( i );
                if ( pSoundFile )
                {
                    USHORT nStringLen = pSoundFile->Len();
                    if ( nStringLen )
                    {
                        try
                        {
                            ::ucb::Content aCnt( *pSoundFile,
                                                 ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );
                            sal_Int64 nVal;

                            ::cppu::convertPropertyValue( nVal, aCnt.getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "size" ) ) ) );
                            sal_uInt32 nSizeOfSound = (sal_uInt32) nVal;

                            if ( nSizeOfSound )
                            {
                                *mpStrm << (sal_uInt16)0xf << (sal_uInt16)EPP_Sound;
                                sal_uInt32 nOldSoundPos = mpStrm->Tell();
                                *mpStrm << (sal_uInt32)0;

                                mp_EscherEx->AddAtom( nStringLen * 2, EPP_CString );        // Name Of Sound ( instance 0 )
                                for ( USHORT k = 0; k < nStringLen; k++ )
                                    *mpStrm << pSoundFile->GetChar( k ) << (sal_uInt8)0;

                                if ( nStringLen > 4 )
                                {
                                    if ( (sal_Char)'.' == pSoundFile->GetChar( nStringLen - 4 ) )
                                    {
                                        mp_EscherEx->AddAtom( 8, EPP_CString, 0, 1 );   // Type Of Sound ( instance 1 )
                                        for ( k = nStringLen - 4; k < nStringLen; k++ )
                                            *mpStrm << pSoundFile->GetChar( k ) << (sal_uInt8)0;
                                    }
                                }
                                ByteString aString( i + 1 );
                                mp_EscherEx->AddAtom( aString.Len() * 2, EPP_CString, 0, 2 );
                                for ( k = 0; k < aString.Len(); k++ )
                                    *mpStrm << aString.GetChar( k ) << (sal_uInt8)0;
                                mp_EscherEx->AddAtom( nSizeOfSound, EPP_SoundData );

                                sal_uInt32 nBytesLeft = nSizeOfSound;
                                SvStream* pSourceFile = ::utl::UcbStreamHelper::CreateStream( *pSoundFile, STREAM_READ );
                                if ( pSourceFile )
                                {
                                    while ( nBytesLeft )
                                    {
                                        sal_uInt32 nToDo = ( nBytesLeft > 0x10000 ) ? 0x10000 : nBytesLeft;
                                        pSourceFile->Read( pBuf, nToDo );
                                        mpStrm->Write( pBuf, nToDo );
                                        nBytesLeft -= nToDo;
                                    }
                                    delete pSourceFile;
                                }
                                sal_uInt32 nCurSoundPos = mpStrm->Tell();
                                mpStrm->Seek( nOldSoundPos );
                                *mpStrm << (sal_uInt32)( nCurSoundPos - nOldSoundPos - 4 );
                                mpStrm->Seek( nCurSoundPos );
                            }
                        }
                        catch( ... )
                        {
                        }
                    }
                }
            }
            delete pBuf;
        }

        mp_EscherEx->WriteDrawingGroupContainer( *mpStrm );
        ImplMasterSlideListContainer( mpStrm );
        ImplDocumentListContainer( mpStrm );

        sal_uInt32 nOldPos = mp_EscherEx->PtGetOffsetByID( EPP_Persist_CurrentPos );
        if ( nOldPos )
        {
            mpStrm->Seek( nOldPos );
            return TRUE;
        }
    }
    return FALSE;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PropValue::GetPropertyValue( ::com::sun::star::uno::Any& rAny,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & aXPropSet,
                const String& rString )
{
    sal_Bool bRetValue = TRUE;
    TRY
    {
        rAny = aXPropSet->getPropertyValue( rString );
        if ( !rAny.hasValue() )
            bRetValue = FALSE;
    }
    CATCH_ALL()
    {
        bRetValue = FALSE;
    }
    END_CATCH;

    return bRetValue;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PropValue::ImplGetPropertyValue( const String& rString )
{
    return GetPropertyValue( mAny, mXPropSet, rString );
}

// ---------------------------------------------------------------------------------------------

sal_Bool PropValue::ImplGetPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & aXPropSet, const String& rString )
{
    return GetPropertyValue( mAny, aXPropSet, rString );
}

// ---------------------------------------------------------------------------------------------

sal_Bool PropStateValue::ImplGetPropertyValue( const String& rString, sal_Bool bGetPropertyState )
{
    ePropState = ::com::sun::star::beans::PropertyState_AMBIGUOUS_VALUE;
    sal_Bool bRetValue = TRUE;
    TRY
    {
        mAny = mXPropSet->getPropertyValue( rString );
        if ( !mAny.hasValue() )
            bRetValue = FALSE;
        else if ( bGetPropertyState )
            ePropState = mXPropState->getPropertyState( rString );
        else
            ePropState = ::com::sun::star::beans::PropertyState_DIRECT_VALUE;
    }
    CATCH_ALL()
    {
        bRetValue = FALSE;
    }
    END_CATCH;

    return bRetValue;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplInitSOIface()
{
    while( TRUE )
    {
        mXDrawPagesSupplier = ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XDrawPagesSupplier >
                ( mXModel, ::com::sun::star::uno::UNO_QUERY );
        if ( !mXDrawPagesSupplier.is() )
            break;

        mXMasterPagesSupplier = ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XMasterPagesSupplier >
                ( mXModel, ::com::sun::star::uno::UNO_QUERY );
        if ( !mXMasterPagesSupplier.is() )
            break;
        mXDrawPages = mXMasterPagesSupplier->getMasterPages();
        if ( !mXDrawPages.is() )
            break;
        mnMasterPages = mXDrawPages->getCount();
        mXDrawPages = mXDrawPagesSupplier->getDrawPages();
        if( !mXDrawPages.is() )
            break;
        mnPages =  mXDrawPages->getCount();
        if ( !ImplGetPageByIndex( 0, NORMAL ) )
            break;

        return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplGetPageByIndex( sal_uInt32 nIndex, PageType ePageType )
{
    while( TRUE )
    {
        if ( ePageType != meLatestPageType )
        {
            switch( ePageType )
            {
                case NORMAL :
                case NOTICE :
                {
                    mXDrawPages = mXDrawPagesSupplier->getDrawPages();
                    if( !mXDrawPages.is() )
                        return FALSE;
                }
                break;

                case MASTER :
                {
                    mXDrawPages = mXMasterPagesSupplier->getMasterPages();
                    if( !mXDrawPages.is() )
                        return FALSE;
                }
                break;
            }
            meLatestPageType = ePageType;
        }
        ::com::sun::star::uno::Any aAny( mXDrawPages->getByIndex( nIndex ) );
        aAny >>= mXDrawPage;
        if ( !mXDrawPage.is() )
            break;
        if ( ePageType == NOTICE )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XPresentationPage >
                aXPresentationPage( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );
            if ( !aXPresentationPage.is() )
                break;
            mXDrawPage = aXPresentationPage->getNotesPage();
            if ( !mXDrawPage.is() )
                break;
        }
        mXPagePropSet = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
            ( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );
        if ( !mXPagePropSet.is() )
            break;

        mXShapes = ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XShapes >
                ( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );
        if ( !mXShapes.is() )
            break;
        return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplGetShapeByIndex( sal_uInt32 nIndex, sal_Bool bGroup )
{
    while(TRUE)
    {
        if (  ( bGroup == FALSE ) || ( GetCurrentGroupLevel() == 0 ) )
        {
            ::com::sun::star::uno::Any aAny( mXShapes->getByIndex( nIndex ) );
            aAny >>= mXShape;
        }
        else
        {
            ::com::sun::star::uno::Any aAny( GetCurrentGroupAccess()->getByIndex( GetCurrentGroupIndex() ) );
            aAny >>= mXShape;
        }
        if ( !mXShape.is() )
            break;

        ::com::sun::star::uno::Any aAny( mXShape->queryInterface( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >*) 0 ) ));
        aAny >>= mXPropSet;

        if ( !mXPropSet.is() )
            break;
        maPosition = ImplMapPoint( mXShape->getPosition() );
        maSize = ImplMapSize( mXShape->getSize() );
        maRect = Rectangle( Point( maPosition.X, maPosition.Y ), Size( maSize.Width, maSize.Height ) );
        mType = ByteString( String( mXShape->getShapeType() ), RTL_TEXTENCODING_UTF8 );
        mType.Erase( 0, 13 );                                   // "com.sun.star." entfernen
        sal_uInt16 nPos = mType.Search( (const char*)"Shape" );
        mType.Erase( nPos, 5 );

        mbPresObj = mbEmptyPresObj = FALSE;
        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsPresentationObject" ) ) ) )
            mAny >>= mbPresObj;

        if ( mbPresObj && ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsEmptyPresentationObject" ) ) ) )
            mAny >>= mbEmptyPresObj;

        mnAngle = ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "RotateAngle" ) ) ) )
                        ? *( (sal_Int32*)mAny.getValue() ) : 0;

        return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------------------------

void PPTWriter::ImplWriteLineBundle( sal_Bool bEdge )
{
    sal_uInt32 nLineFlags = 0x80008;
    _Escher_LineEnd eLineEnd = _Escher_LineArrowEnd;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "LineStart" ) ) ) )
    {
        ::com::sun::star::drawing::PointSequence* pPolyPolygon = (::com::sun::star::drawing::PointSequence*)mAny.getValue();
        sal_Int32 nSequenceCount = pPolyPolygon->getLength();
        if ( nSequenceCount )
        {
            // Zeiger auf innere sequences holen
            ::com::sun::star::awt::Point* pSequence = pPolyPolygon->getArray();
            if ( pSequence )
            {
                switch ( nSequenceCount )
                {
                    case 0x4 :
                    {
                        switch( pSequence->X )
                        {
                            case 0 : eLineEnd = _Escher_LineArrowDiamondEnd;    break;
                            case 0x529 : eLineEnd = _Escher_LineArrowStealthEnd; break;
                        }
                    }
                    break;
                    case 0x7 : eLineEnd = _Escher_LineArrowDiamondEnd; break;
                    case 0xa : eLineEnd = _Escher_LineArrowStealthEnd; break;
                    case 0xd :
                    {
                        switch ( pSequence->X )
                        {
                            case 0 : eLineEnd = _Escher_LineArrowDiamondEnd; break;
                            case 0x64 : eLineEnd = _Escher_LineArrowOvalEnd; break;
                            case 0x87c : eLineEnd = _Escher_LineArrowStealthEnd; break;
                        }
                    }
                }
                mp_EscherEx->AddOpt( _Escher_Prop_lineStartArrowLength, 2 );
                mp_EscherEx->AddOpt( _Escher_Prop_lineStartArrowWidth, 2 );
                mp_EscherEx->AddOpt( _Escher_Prop_lineStartArrowhead, eLineEnd );
                nLineFlags |= 0x100010;
            }
        }
    }

    eLineEnd = _Escher_LineArrowEnd;
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "LineEnd" ) ) ) )
    {
        ::com::sun::star::drawing::PointSequence* pPolyPolygon = (::com::sun::star::drawing::PointSequence*)mAny.getValue();
        sal_Int32 nSequenceCount = pPolyPolygon->getLength();
        if ( nSequenceCount )
        {
            // Zeiger auf innere sequences holen
            ::com::sun::star::awt::Point* pSequence = pPolyPolygon->getArray();
            if ( pSequence )
            {
                switch ( nSequenceCount )
                {
                    case 0x4 :
                    {
                        switch( pSequence->X )
                        {
                            case 0 : eLineEnd = _Escher_LineArrowDiamondEnd;    break;
                            case 0x529 : eLineEnd = _Escher_LineArrowStealthEnd; break;
                        }
                    }
                    break;
                    case 0x7 : eLineEnd = _Escher_LineArrowDiamondEnd; break;
                    case 0xa : eLineEnd = _Escher_LineArrowStealthEnd; break;
                    case 0xd :
                    {
                        switch ( pSequence->X )
                        {
                            case 0 : eLineEnd = _Escher_LineArrowDiamondEnd; break;
                            case 0x64 : eLineEnd = _Escher_LineArrowOvalEnd; break;
                            case 0x87c : eLineEnd = _Escher_LineArrowStealthEnd; break;
                        }
                    }
                }
                mp_EscherEx->AddOpt( _Escher_Prop_lineEndArrowLength, 2 );
                mp_EscherEx->AddOpt( _Escher_Prop_lineEndArrowWidth, 2 );
                mp_EscherEx->AddOpt( _Escher_Prop_lineEndArrowhead, eLineEnd );
                nLineFlags |= 0x100010;
            }
        }
    }

    mnShadow |= 1;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "LineStyle" ) ) ) )
    {
        ::com::sun::star::drawing::LineStyle aLS;
        mAny >>= aLS;
        switch ( aLS )
        {
            case ::com::sun::star::drawing::LineStyle_NONE :
            {
                mp_EscherEx->AddOpt( _Escher_Prop_fNoLineDrawDash, 0x90000 );           // 80000
                mnShadow ^= 1;
            }
            break;

            case ::com::sun::star::drawing::LineStyle_DASH :
            {
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "LineDash" ) ) ) )
                {
                    _Escher_LineDashing eDash = _Escher_LineSolid;
                    ::com::sun::star::drawing::LineDash* pLineDash = (::com::sun::star::drawing::LineDash*)mAny.getValue();
                    sal_uInt32 nDistance = pLineDash->Distance << 1;
                    switch ( pLineDash->Style )
                    {
                        case ::com::sun::star::drawing::DashStyle_ROUND :
                        case ::com::sun::star::drawing::DashStyle_ROUNDRELATIVE :
                            mp_EscherEx->AddOpt( _Escher_Prop_lineEndCapStyle, 0 ); // Style Round setzen
                        break;
                    }
                    if ( ((!(pLineDash->Dots )) || (!(pLineDash->Dashes )) ) || ( pLineDash->DotLen == pLineDash->DashLen ) )
                    {
                        sal_uInt32 nLen = pLineDash->DotLen;
                        if ( pLineDash->Dashes )
                            nLen = pLineDash->DashLen;

                        if ( nLen >= nDistance )
                            eDash = _Escher_LineLongDashGEL;
                        else if ( pLineDash->Dots )
                            eDash = _Escher_LineDotSys;
                        else
                            eDash = _Escher_LineDashGEL;
                    }
                    else                                                            // X Y
                    {
                        if ( pLineDash->Dots != pLineDash->Dashes )
                        {
                            if ( ( pLineDash->DashLen > nDistance ) || ( pLineDash->DotLen > nDistance ) )
                                eDash = _Escher_LineLongDashDotDotGEL;
                            else
                                eDash = _Escher_LineDashDotDotSys;
                        }
                        else                                                        // X Y Y
                        {
                            if ( ( pLineDash->DashLen > nDistance ) || ( pLineDash->DotLen > nDistance ) )
                                eDash = _Escher_LineLongDashDotGEL;
                            else
                                eDash = _Escher_LineDashDotGEL;

                        }
                    }
                    mp_EscherEx->AddOpt( _Escher_Prop_lineDashing, eDash );
                }
            }
            case ::com::sun::star::drawing::LineStyle_SOLID :
            default:
            {
                mp_EscherEx->AddOpt( _Escher_Prop_fNoLineDrawDash, nLineFlags );
            }
            break;
        }
        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "LineColor" ) ) ) )
        {
            sal_uInt32 nLineColor = mp_EscherEx->GetColor( *((sal_uInt32*)mAny.getValue()) );
            mp_EscherEx->AddOpt( _Escher_Prop_lineColor, nLineColor );
            mp_EscherEx->AddOpt( _Escher_Prop_lineBackColor, nLineColor ^ 0xffffff );
        }
    }

    sal_uInt32 nLineSize = ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "LineWidth" ) ) ) )
                                ? *((sal_uInt32*)mAny.getValue() ) : 0;

    if ( nLineSize > 1 )
        mp_EscherEx->AddOpt( _Escher_Prop_lineWidth, nLineSize * 360 ); // 100TH MM -> PT , 1PT = 12700 EMU
    if ( bEdge == FALSE )
    {
        mp_EscherEx->AddOpt( _Escher_Prop_fFillOK, 0x1001 );
        mp_EscherEx->AddOpt( _Escher_Prop_fNoFillHitTest, 0x100000 );
    }
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplWriteFillBundle( sal_Bool bEdge )
{
    mp_EscherEx->AddOpt( _Escher_Prop_WrapText, _Escher_WrapNone );
    mp_EscherEx->AddOpt( _Escher_Prop_AnchorText, _Escher_AnchorMiddle );

    sal_uInt32 nFillBackColor = 0;
    mnShadow |= 2;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "FillStyle" ) ) ) )
    {
        ::com::sun::star::drawing::FillStyle aFS;
        mAny >>= aFS;
        switch( aFS )
        {
            case ::com::sun::star::drawing::FillStyle_GRADIENT :
            {
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "FillGradient" ) ) ) )
                    mp_EscherEx->WriteGradient( (::com::sun::star::awt::Gradient*)mAny.getValue() );
                mp_EscherEx->AddOpt( _Escher_Prop_fNoFillHitTest, 0x140014 );
            }
            break;

            case ::com::sun::star::drawing::FillStyle_BITMAP :
            {
                ImplGetGraphic( mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillBitmapURL" ) ), TRUE );
                mp_EscherEx->AddOpt( _Escher_Prop_fNoFillHitTest, 0x140014 );
                mp_EscherEx->AddOpt( _Escher_Prop_fillBackColor, nFillBackColor  );
            }
            break;
            case ::com::sun::star::drawing::FillStyle_HATCH :
            case ::com::sun::star::drawing::FillStyle_SOLID :
            default:
            {
                sal_uInt16 nTransparency = ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "FillTransparence" ) ) ) )
                                                    ? *((sal_Int16*)mAny.getValue()) : NULL;
                if ( nTransparency != 100 )
                {
                    if ( nTransparency )    // opacity
                        mp_EscherEx->AddOpt( _Escher_Prop_fillOpacity, ( ( 100 - nTransparency ) << 16 ) / 100 );
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "FillColor" ) ) ) )
                    {
                        sal_uInt32 nFillColor = mp_EscherEx->GetColor( *((sal_uInt32*)mAny.getValue()) );
                        nFillBackColor = nFillColor ^ 0xffffff;
                        mp_EscherEx->AddOpt( _Escher_Prop_fillColor, nFillColor );
                    }
                    mp_EscherEx->AddOpt( _Escher_Prop_fNoFillHitTest, 0x100010 );
                    mp_EscherEx->AddOpt( _Escher_Prop_fillBackColor, nFillBackColor );
                    break;
                }
            }
            case ::com::sun::star::drawing::FillStyle_NONE :
            {
                mp_EscherEx->AddOpt( _Escher_Prop_fNoFillHitTest, 0x100000 );
                mnShadow ^= 2;
            }
            break;
        }
    }
    ImplWriteLineBundle( bEdge );
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplWriteTextBundle( sal_Bool bDisableAutoGrowHeight )
{
    if ( ImplGetText() )
    {
        _Escher_AnchorText  eAnchor = _Escher_AnchorTop;
        sal_uInt32              nTextAttr = 0x40004;    // rotate text with shape

        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "TextVerticalAdjust" ) ) ) )
        {
            ::com::sun::star::style::VerticalAlignment aVA;
            mAny >>= aVA;
            switch ( aVA )
            {
                case ::com::sun::star::style::VerticalAlignment_MIDDLE :
                    eAnchor = _Escher_AnchorMiddle;
                break;

                case ::com::sun::star::style::VerticalAlignment_BOTTOM :
                    eAnchor = _Escher_AnchorBottom;
                break;

                default :
                case ::com::sun::star::style::VerticalAlignment_TOP :
                    eAnchor = _Escher_AnchorTop;
                break;
            }
        }
        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "TextHorizontalAdjust" ) ) ) )
        {
            ::com::sun::star::drawing::TextAdjust aTA;
            mAny >>= aTA;
            switch ( aTA )
            {
                case ::com::sun::star::drawing::TextAdjust_CENTER :
                {
                    switch( eAnchor )
                    {
                        case _Escher_AnchorMiddle :
                            eAnchor = _Escher_AnchorMiddleCentered;
                        break;
                        case _Escher_AnchorBottom :
                            eAnchor = _Escher_AnchorBottomCentered;
                        break;
                        case _Escher_AnchorTop :
                            eAnchor = _Escher_AnchorTopCentered;
                        break;
                    }
                }
                break;
                case ::com::sun::star::drawing::TextAdjust_RIGHT :
                case ::com::sun::star::drawing::TextAdjust_LEFT :
                case ::com::sun::star::drawing::TextAdjust_STRETCH :
                case ::com::sun::star::drawing::TextAdjust_BLOCK :
                break;
            }
        }
/*
        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "TextFitToSize" ) ) ) )
        {
            if ( *( (sal_Int16*)mAny.get() ) == 1 )
            {
                nTextAttr |= 0x10001;
                mp_EscherEx->AddOpt( _Escher_Prop_scaleText, ? );
            }
        }
*/

        sal_Int32 nLeft = ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "TextLeftDistance" ) ) ) ) ? *(sal_Int32*)mAny.getValue() : 0;
        sal_Int32 nTop = ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "TextUpperDistance" ) ) ) ) ? *(sal_Int32*)mAny.getValue() : 0;
        sal_Int32 nRight = ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "TextRightDistance" ) ) ) ) ? *(sal_Int32*)mAny.getValue() : 0;
        sal_Int32 nBottom = ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "TextLowerDistance" ) ) ) ) ? *(sal_Int32*)mAny.getValue() : 0;


        _Escher_WrapMode eWrapMode = _Escher_WrapSquare;
        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "TextAutoGrowWidth" ) ) ) )
        {
            sal_Bool bBool;
            mAny >>= bBool;
            if ( bBool )
                eWrapMode = _Escher_WrapNone;
        }
        if ( !bDisableAutoGrowHeight && ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "TextAutoGrowHeight" ) ) ) )
        {
            sal_Bool bBool;
            mAny >>= bBool;
            if ( bBool )
                nTextAttr |= 0x20002;
        }
        mp_EscherEx->AddOpt( _Escher_Prop_dxTextLeft, nLeft * 360 );
        mp_EscherEx->AddOpt( _Escher_Prop_dxTextRight, nRight * 360 );
        mp_EscherEx->AddOpt( _Escher_Prop_dyTextTop, nTop * 360 );
        mp_EscherEx->AddOpt( _Escher_Prop_dyTextBottom, nBottom * 360 );

        mp_EscherEx->AddOpt( _Escher_Prop_WrapText, eWrapMode );
        mp_EscherEx->AddOpt( _Escher_Prop_AnchorText, eAnchor );
        mp_EscherEx->AddOpt( _Escher_Prop_FitTextToShape, nTextAttr );
        mp_EscherEx->AddOpt( _Escher_Prop_lTxid, mnTxId += 0x60 );
    }
}

//  -----------------------------------------------------------------------

sal_uInt32 PPTWriter::ImplGetMasterIndex( PageType ePageType )
{
    sal_uInt32 nRetValue = 0;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XMasterPageTarget >
        aXMasterPageTarget( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );

    if ( aXMasterPageTarget.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >
            aXDrawPage = aXMasterPageTarget->getMasterPage();
        if ( aXDrawPage.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                aXPropertySet( aXDrawPage, ::com::sun::star::uno::UNO_QUERY );

            if ( aXPropertySet.is() )
            {
                if ( ImplGetPropertyValue( aXPropertySet, String( RTL_CONSTASCII_USTRINGPARAM( "Number" ) ) ) )
                    nRetValue |= *(sal_Int16*)mAny.getValue();
                if ( nRetValue & 0xffff )           // ueberlauf vermeiden
                    nRetValue--;
            }
        }
    }
    if ( ePageType == NOTICE )
        nRetValue += mnMasterPages;
    return nRetValue;
}

//  -----------------------------------------------------------------------

sal_Bool PPTWriter::ImplGetStyleSheets()
{
    int nInstance, nLevel;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
        aXPropSet( mXModel, ::com::sun::star::uno::UNO_QUERY );

    sal_uInt16 nDefaultTab = ( aXPropSet.is() && ImplGetPropertyValue( aXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "TabStop" ) ) ) )
        ? (sal_uInt16)( *(sal_Int32*)mAny.getValue() / 4.40972 )
        : 1250;

    mpStyleSheet = new PPTExStyleSheet( nDefaultTab, (PPTExBulletProvider&)*this );

    if ( !ImplGetPageByIndex( 0, MASTER ) )
        return FALSE;

    for ( nInstance = EPP_TEXTTYPE_Title; nInstance <= EPP_TEXTTYPE_Other; nInstance++ )
    {
        String aStyle;
        switch ( nInstance )
        {
            case EPP_TEXTTYPE_Title :
                aStyle = String( RTL_CONSTASCII_USTRINGPARAM( "title" ) );
            break;
            case EPP_TEXTTYPE_Body :
                aStyle = String( RTL_CONSTASCII_USTRINGPARAM( "outline1" ) );      // SD_LT_SEPARATOR
            break;
            case EPP_TEXTTYPE_Other :
                aStyle = String( RTL_CONSTASCII_USTRINGPARAM( "standard" ) );
            break;
        }
        if ( aStyle.Len() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed >
                aXNamed( mXDrawPage, ::com::sun::star::uno::UNO_QUERY );

            if ( aXNamed.is() )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyleFamiliesSupplier >
                    aXStyleFamiliesSupplier( mXModel, ::com::sun::star::uno::UNO_QUERY );
                if ( aXStyleFamiliesSupplier.is() )
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                        aXNameAccess( aXStyleFamiliesSupplier->getStyleFamilies() );

                    if ( aXNameAccess.is() )
                    {
                        TRY
                        {
                            ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >xNameAccess;
                            ::com::sun::star::uno::Any aAny( aXNameAccess->getByName( aXNamed->getName() ) );
                            if( aAny.getValue() && ::cppu::extractInterface( xNameAccess, aAny ) )
                            {
                                ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > aXFamily;
                                if ( aAny >>= aXFamily )
                                {
                                    ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle > xStyle;
                                    aAny = aXFamily->getByName( aStyle );
                                    if( aAny.getValue() && ::cppu::extractInterface( xStyle, aAny ) )
                                    {
                                        ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle > aXStyle;
                                        aAny >>= aXStyle;
                                        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                            xPropSet( aXStyle, ::com::sun::star::uno::UNO_QUERY );
                                        if( xPropSet.is() )
                                            mpStyleSheet->SetStyleSheet( xPropSet, maFontCollection, nInstance, 0 );
                                        if ( nInstance == EPP_TEXTTYPE_Body )
                                        {
                                            for ( nLevel = 1; nLevel < 5; nLevel++ )
                                            {
                                                sal_Unicode cTemp = aStyle.GetChar( aStyle.Len() - 1 );
                                                aStyle.SetChar( aStyle.Len() - 1, ++cTemp );
                                                aAny = aXFamily->getByName( aStyle );
                                                if( aAny.getValue() && ::cppu::extractInterface( xStyle, aAny ) )
                                                {
                                                    ::com::sun::star::uno::Reference< ::com::sun::star::style::XStyle > aXStyle;
                                                    aAny >>= aXStyle;
                                                    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                                        xPropSet( aXStyle, ::com::sun::star::uno::UNO_QUERY );
                                                    if ( xPropSet.is() )
                                                        mpStyleSheet->SetStyleSheet( xPropSet, maFontCollection, nInstance, nLevel );
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        CATCH_ALL()
                        {
                        //
                        }
                        END_CATCH;
                    }
                }
            }
        }
    }
    for ( ; nInstance <= EPP_TEXTTYPE_QuarterBody; nInstance++ )
    {

    }
    return TRUE;
}

//  -----------------------------------------------------------------------

sal_Bool PPTWriter::ImplGetMasterTitleAndBody()
{
    sal_uInt32  i, nSearchFor, nShapes;

    if ( !ImplGetPageByIndex( 0, MASTER ) )
        return FALSE;

    nShapes = mXShapes->getCount();

    // ueber die Seite gehen um den Index des Title and Body Objects zu bekommen
    for ( nSearchFor = 2, i = 0; i < nShapes; i++ )
    {
        if ( !ImplGetShapeByIndex( i ) )
            break;

        if ( mType == "presentation.TitleText" )
        {
            mnMasterTitleIndex = i;
            if ( ! ( --nSearchFor ) )
                break;
        }
        else if ( mType == "presentation.Outliner" )
        {
            mnMasterBodyIndex = i;
            if ( ! ( --nSearchFor ) )
                break;
        }
    };
    return TRUE;
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplWriteParagraphs( SvStream& rOut, TextObj& rTextObj, sal_uInt32 nTextStyle )
{
    sal_Bool            bFirstParagraph = TRUE;
    sal_uInt32          nCharCount;
    sal_uInt32          nPropertyFlags = 0;
    sal_uInt16          nDepth = 0;
    sal_Int16           nLineSpacing;
    int             nInstance = rTextObj.GetInstance();

    for ( ParagraphObj* pPara = rTextObj.First() ; pPara; pPara = rTextObj.Next(), bFirstParagraph = FALSE )
    {
        nCharCount = pPara->Count();

        nDepth = pPara->nDepth;
        if ( nDepth > 4)
            nDepth = 4;

        if ( ( pPara->meTextAdjust == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
            ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, ParaAttr_Adjust, pPara->mnTextAdjust ) ) )
            nPropertyFlags |= 0x00000800;
        nLineSpacing = pPara->mnLineSpacing;
        if ( bFirstParagraph && ( nLineSpacing > 100 ) )
        {
            nLineSpacing = 100;
            nPropertyFlags |= 0x00001000;
        }
        else if ( ( pPara->meLineSpacing == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
            ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, ParaAttr_LineFeed, nLineSpacing ) ) )
            nPropertyFlags |= 0x00001000;
        if ( ( pPara->meLineSpacingBottom == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
            ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, ParaAttr_LowerDist, pPara->mnLineSpacingBottom ) ) )
            nPropertyFlags |= 0x00004000;
        if ( ( pPara->meLineSpacingTop == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
            ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, ParaAttr_UpperDist, pPara->mnLineSpacingTop ) ) )
            nPropertyFlags |= 0x00002000;

        sal_Int32 nBuRealSize = pPara->nBulletRealSize;
        sal_Int16 nBulletFlags = pPara->nBulletFlags;

        if ( pPara->bExtendedParameters )
            nPropertyFlags |= pPara->nParaFlags;
        else
        {
            nPropertyFlags |= 1;            // turn off bullet explicit
            nBulletFlags = 0;
        }
        PortionObj* pPortion = (PortionObj*)pPara->First();
        if ( pPortion ) // in SO the bulletrealsize does not depend to the following portion charactersize
        {
            if ( pPortion->mnCharHeight )
            {
                if ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, CharAttr_FontHeight, pPortion->mnCharHeight ) )
                {   // the fontsize is unequal to the StyleSheetFontSize, so the BulletRealSize has to be set again
                    nPropertyFlags |= 0x44;
                    nBulletFlags |= 8;
                    nBuRealSize *= mpStyleSheet->GetCharSheet( nInstance ).maCharLevel[ pPara->nDepth ].mnFontHeight;
                    nBuRealSize /= pPortion->mnCharHeight;
                }
            }
        }
        rOut << nCharCount
             << nDepth                          // Level
             << (sal_uInt32)nPropertyFlags;     // Paragraph Attribut Set

        if ( nPropertyFlags & 0xf )
            rOut << nBulletFlags;
        if ( nPropertyFlags & 0x80 )
            rOut << (sal_uInt16)( pPara->cBulletId );
        if ( nPropertyFlags & 0x10 )
            rOut << (sal_uInt16)( maFontCollection.GetId( String( pPara->aFontDesc.Name ) ) );
        if ( nPropertyFlags & 0x40 )
            rOut << (sal_Int16)nBuRealSize;
        if ( nPropertyFlags & 0x20 )
            rOut << pPara->nBulletColor;
        if ( nPropertyFlags & 0x00000800 )
            rOut << (sal_uInt16)( pPara->mnTextAdjust );
        if ( nPropertyFlags & 0x00001000 )
            rOut << (sal_uInt16)( nLineSpacing );
        if ( nPropertyFlags & 0x00002000 )
            rOut << (sal_uInt16)( pPara->mnLineSpacingTop );
        if ( nPropertyFlags & 0x00004000 )
            rOut << (sal_uInt16)( pPara->mnLineSpacingBottom );
    }
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplWritePortions( SvStream& rOut, TextObj& rTextObj )
{
    sal_uInt32  nPropertyFlags, i = 0;
    int     nInstance = rTextObj.GetInstance();

    for ( ParagraphObj* pPara = rTextObj.First(); pPara; pPara = rTextObj.Next(), i++ )
    {
        for ( PortionObj* pPortion = (PortionObj*)pPara->First(); pPortion; pPortion = (PortionObj*)pPara->Next() )
        {
            nPropertyFlags = 0;
            sal_uInt32 nCharAttr = pPortion->mnCharAttr;

            if ( ( pPortion->mnCharAttrHard & 1 ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, CharAttr_Bold, nCharAttr ) ) )
                nPropertyFlags |= 1;
            if ( ( pPortion->mnCharAttrHard & 2 ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, CharAttr_Italic, nCharAttr ) ) )
                nPropertyFlags |= 2;
            if ( ( pPortion->mnCharAttrHard & 4 ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, CharAttr_Underline, nCharAttr ) ) )
                nPropertyFlags |= 4;
            if ( ( pPortion->mnCharAttrHard & 0x10 ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, CharAttr_Shadow, nCharAttr ) ) )
                nPropertyFlags |= 0x10;
            if ( rTextObj.HasExtendedBullets() )
            {
                if ( i > 63 )
                    i = 63;

                nPropertyFlags |= i << 10 ;
                nCharAttr  |= i << 10;
            }
            if ( ( pPortion->meFontName == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, CharAttr_Font, pPortion->mnFont ) ) )
                nPropertyFlags |= 0x00010000;
            if ( ( pPortion->meCharHeight == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, CharAttr_FontHeight, pPortion->mnCharHeight ) ) )
                nPropertyFlags |= 0x00020000;
            if ( ( pPortion->meCharColor == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, CharAttr_FontColor, pPortion->mnCharColor ) ) )
                nPropertyFlags |= 0x00040000;
            if ( ( pPortion->meCharEscapement == ::com::sun::star::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->bDepth, CharAttr_Escapement, pPortion->mnCharEscapement ) ) )
                nPropertyFlags |= 0x00080000;

            sal_uInt32 nCharCount = pPortion->Count();

            rOut << nCharCount
                 << nPropertyFlags;          //PropertyFlags

            if ( nPropertyFlags & 0xffff )
                rOut << (sal_uInt16)( nCharAttr );
            if ( nPropertyFlags & 0x00010000 )
                rOut << pPortion->mnFont;
            if ( nPropertyFlags & 0x00020000 )
                rOut << (sal_uInt16)( pPortion->mnCharHeight );
            if ( nPropertyFlags & 0x00040000 )
                rOut << (sal_uInt32)pPortion->mnCharColor;
            if ( nPropertyFlags & 0x00080000 )
                rOut << pPortion->mnCharEscapement;
        }
    }
}

//  ----------------------------------------------------------------------------------------
//  laedt und konvertiert text aus shape, ergebnis ist mnTextSize gespeichert;
sal_Bool PPTWriter::ImplGetText()
{
    mnTextSize = 0;
    mXText = ::com::sun::star::uno::Reference<
        ::com::sun::star::text::XSimpleText >
            ( mXShape, ::com::sun::star::uno::UNO_QUERY );

    if ( mXText.is() )
        mnTextSize = mXText->getString().len();
    return ( mnTextSize != 0 );
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplFlipBoundingBox( const ::com::sun::star::awt::Point& rRefPoint )
{
    if ( mnAngle < 0 )
        mnAngle = ( 36000 + mnAngle ) % 36000;
    else
        mnAngle = ( 36000 - ( mnAngle % 36000 ) );

    double  fCos = cos( (double)mnAngle * F_PI18000 );
    double  fSin = sin( (double)mnAngle * F_PI18000 );

    double  nWidthHalf = maRect.GetWidth() / 2;
    double  nHeightHalf = maRect.GetHeight() / 2;

    sal_Int32 nXDiff = fCos * nWidthHalf + fSin * (-nHeightHalf);
    sal_Int32 nYDiff = - ( fSin * nWidthHalf - fCos * ( -nHeightHalf ) );

    maRect.Move( -( nWidthHalf - nXDiff ), - ( nHeightHalf + nYDiff ) );

    if ( ( mnAngle > 4500 && mnAngle <= 13500 ) || ( mnAngle > 22500 && mnAngle <= 31500 ) )
    {
        // In diesen beiden Bereichen steht in PPT gemeinerweise die
        // BoundingBox bereits senkrecht. Daher muss diese VOR
        // DER ROTATION flachgelegt werden.
        ::com::sun::star::awt::Point
            aTopLeft( maRect.Left() + nWidthHalf - nHeightHalf, maRect.Top() + nHeightHalf - nWidthHalf );
        Size    aNewSize( maRect.GetHeight(), maRect.GetWidth() );
        maRect = Rectangle( Point( aTopLeft.X, aTopLeft.Y ), aNewSize );
    }
    mnAngle *= 655;
    mnAngle += 0x8000;
    mnAngle &=~0xffff;                                  // nAngle auf volle Gradzahl runden
    mp_EscherEx->AddOpt( _Escher_Prop_Rotation, mnAngle );
}

//  -----------------------------------------------------------------------

struct FieldEntry
{
    sal_uInt32  nFieldType;
    sal_uInt32  nFieldStartPos;
    sal_uInt32  nFieldEndPos;
    String      aFieldUrl;

    FieldEntry( sal_uInt32 nType, sal_uInt32 nStart, sal_uInt32 nEnd )
    {
        nFieldType = nType;
        nFieldStartPos = nStart;
        nFieldEndPos = nEnd;
    }
    FieldEntry( FieldEntry& rFieldEntry )
    {
        nFieldType = rFieldEntry.nFieldType;
        nFieldStartPos = rFieldEntry.nFieldStartPos;
        nFieldEndPos = rFieldEntry.nFieldEndPos;
        aFieldUrl = rFieldEntry.aFieldUrl;
    }
};

//  -----------------------------------------------------------------------

PortionObj::PortionObj( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                Collection& rFontCollection ) :
    mbLastPortion   ( TRUE ),
    mnCharAttrHard  ( 0 ),
    mnCharAttr      ( 0 ),
    mnTextSize      ( 0 ),
    mnFont          ( 0 ),
    mpFieldEntry    ( NULL ),
    mpText          ( NULL )
{
    mXPropSet = rXPropSet;

    ImplGetPortionValues( rFontCollection, FALSE );
}

PortionObj::PortionObj( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & rXTextRange,
                            sal_Bool bLast, Collection& rFontCollection ) :
    mbLastPortion   ( bLast ),
    mnCharAttrHard  ( 0 ),
    mnCharAttr      ( 0 ),
    mnFont          ( 0 ),
    mpFieldEntry    ( NULL ),
    mpText          ( NULL )
{
    String aString( rXTextRange->getString() );
    String aURL;

    mnTextSize = aString.Len();
    if ( bLast )
        mnTextSize++;

    if ( mnTextSize )
    {
        mpFieldEntry = NULL;
        sal_uInt32 nFieldType = 0;

        mXPropSet = ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >
                ( rXTextRange, ::com::sun::star::uno::UNO_QUERY );
        mXPropState = ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertyState >
                ( rXTextRange, ::com::sun::star::uno::UNO_QUERY );

        sal_Bool bPropSetsValid = ( mXPropSet.is() && mXPropState.is() );
        if ( bPropSetsValid )
            nFieldType = ImplGetTextField( rXTextRange, aURL );
        if ( nFieldType )
        {
            mpFieldEntry = new FieldEntry( nFieldType, 0, mnTextSize );
            if ( ( nFieldType >> 28 == 4 ) )
                mpFieldEntry->aFieldUrl = aString;
        }

        sal_Bool bSymbol = FALSE;
        if ( bPropSetsValid && ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "FontCharset" ) ), FALSE ) )
        {
            sal_Int16 nCharset;
            mAny >>= nCharset;
            if ( nCharset == ::com::sun::star::awt::CharSet::SYMBOL )
                bSymbol = TRUE;
        }
        if ( mpFieldEntry && ( nFieldType & 0x800000 ) )    // placeholder ?
        {
            mnTextSize = 1;
            if ( bLast )
                mnTextSize++;
            mpText = new sal_uInt16[ mnTextSize ];
            mpText[ 0 ] = 0x2a;
        }
        else
        {
            mpText = new sal_uInt16[ mnTextSize ];
            const sal_Unicode* pText = aString.GetBuffer();
            sal_uInt16 nChar;
            for ( int i = 0; i < aString.Len(); i++ )
            {
                nChar = (sal_uInt16)pText[ i ];
                if ( nChar == 0xa )
                    nChar++;
                else if ( !bSymbol )
                {
                    switch ( nChar )
                    {
                        // Currency
                        case 128:   nChar = 0x20AC; break;
                        // Punctuation and other
                        case 130:   nChar = 0x201A; break;// SINGLE LOW-9 QUOTATION MARK
                        case 131:   nChar = 0x0192; break;// LATIN SMALL LETTER F WITH HOOK
                        case 132:   nChar = 0x201E; break;// DOUBLE LOW-9 QUOTATION MARK
                                                              // LOW DOUBLE PRIME QUOTATION MARK
                        case 133:   nChar = 0x2026; break;// HORIZONTAL ELLIPSES
                        case 134:   nChar = 0x2020; break;// DAGGER
                        case 135:   nChar = 0x2021; break;// DOUBLE DAGGER
                        case 136:   nChar = 0x02C6; break;// MODIFIER LETTER CIRCUMFLEX ACCENT
                        case 137:   nChar = 0x2030; break;// PER MILLE SIGN
                        case 138:   nChar = 0x0160; break;// LATIN CAPITAL LETTER S WITH CARON
                        case 139:   nChar = 0x2039; break;// SINGLE LEFT-POINTING ANGLE QUOTATION MARK
                        case 140:   nChar = 0x0152; break;// LATIN CAPITAL LIGATURE OE
                        case 142:   nChar = 0x017D; break;// LATIN CAPITAL LETTER Z WITH CARON
                        case 145:   nChar = 0x2018; break;// LEFT SINGLE QUOTATION MARK
                                                              // MODIFIER LETTER TURNED COMMA
                        case 146:   nChar = 0x2019; break;// RIGHT SINGLE QUOTATION MARK
                                                              // MODIFIER LETTER APOSTROPHE
                        case 147:   nChar = 0x201C; break;// LEFT DOUBLE QUOTATION MARK
                                                              // REVERSED DOUBLE PRIME QUOTATION MARK
                        case 148:   nChar = 0x201D; break;// RIGHT DOUBLE QUOTATION MARK
                                                              // REVERSED DOUBLE PRIME QUOTATION MARK
                        case 149:   nChar = 0x2022; break;// BULLET
                        case 150:   nChar = 0x2013; break;// EN DASH
                        case 151:   nChar = 0x2014; break;// EM DASH
                        case 152:   nChar = 0x02DC; break;// SMALL TILDE
                        case 153:   nChar = 0x2122; break;// TRADE MARK SIGN
                        case 154:   nChar = 0x0161; break;// LATIN SMALL LETTER S WITH CARON
                        case 155:   nChar = 0x203A; break;// SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
                        case 156:   nChar = 0x0153; break;// LATIN SMALL LIGATURE OE
                        case 158:   nChar = 0x017E; break;// LATIN SMALL LETTER Z WITH CARON
                        case 159:   nChar = 0x0178; break;// LATIN CAPITAL LETTER Y WITH DIAERESIS
//                      case 222:   nChar = 0x00B6; break;// PILCROW SIGN / PARAGRAPH SIGN
                    }
                }
                mpText[ i ] = nChar;
            }
        }
        if ( bLast )
            mpText[ mnTextSize - 1 ] = 0xd;

        if ( bPropSetsValid )
            ImplGetPortionValues( rFontCollection, TRUE );
    }
}

PortionObj::PortionObj( PortionObj& rPortionObj )
{
    ImplConstruct( rPortionObj );
}

PortionObj::~PortionObj()
{
    ImplClear();
}

void PortionObj::Write( SvStream* pStrm, sal_Bool bLast )
{
    sal_uInt32 nCount = mnTextSize;
    if ( bLast && mbLastPortion )
        nCount--;
    for ( sal_uInt32 i = 0; i < nCount; i++ )
        *pStrm << (sal_uInt16)mpText[ i ];
}

void PortionObj::ImplGetPortionValues( Collection& rFontCollection, sal_Bool bGetPropStateValue )
{
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharFontName" ) ), bGetPropStateValue ) )
    {
        String aString( *(::rtl::OUString*)mAny.getValue() );
        mnFont = (sal_uInt16)rFontCollection.GetId( aString );
    }
    meFontName = ePropState;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharWeight" ) ), bGetPropStateValue ) )
    {
        float fFloat;
        mAny >>= fFloat;
        if ( fFloat >= ::com::sun::star::awt::FontWeight::SEMIBOLD )
            mnCharAttr |= 1;
    }
    if ( ePropState == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        mnCharAttrHard |= 1;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharPosture" ) ), bGetPropStateValue ) )
    {
        ::com::sun::star::awt::FontSlant aFS;
        mAny >>= aFS;
        switch ( aFS )
        {
            case ::com::sun::star::awt::FontSlant_OBLIQUE :
            case ::com::sun::star::awt::FontSlant_ITALIC :
                mnCharAttr |= 2;
        }
    }
    if ( ePropState == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        mnCharAttrHard |= 2;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharUnderline" ) ), bGetPropStateValue ) )
    {
        sal_Int16 nVal;
        mAny >>= nVal;
        switch ( nVal )
        {
            case ::com::sun::star::awt::FontUnderline::SINGLE :
            case ::com::sun::star::awt::FontUnderline::DOUBLE :
            case ::com::sun::star::awt::FontUnderline::DOTTED :
                mnCharAttr |= 4;
        }
    }
    if ( ePropState == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        mnCharAttrHard |= 4;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharShadowed" ) ), bGetPropStateValue ) )
    {
        sal_Bool bBool;
        mAny >>= bBool;
        if ( bBool )
            mnCharAttr |= 0x10;
    }
    if ( ePropState == ::com::sun::star::beans::PropertyState_DIRECT_VALUE )
        mnCharAttrHard |= 16;

    mnCharHeight = 24;
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharHeight" ) ), bGetPropStateValue ) )
    {
        float fVal;
        mAny >>= fVal;
        mnCharHeight = (sal_uInt16)( fVal + 0.5 );
    }
    meCharHeight = ePropState;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharColor" ) ), bGetPropStateValue ) )
    {
        sal_uInt32 nSOColor = *( (sal_uInt32*)mAny.getValue() );
        mnCharColor = nSOColor & 0xff00;                                // green
        mnCharColor |= (sal_uInt8)( nSOColor ) << 16;                   // red
        mnCharColor |= (sal_uInt8)( nSOColor >> 16 ) | 0xfe000000;      // blue
    }
    meCharColor = ePropState;

    mnCharEscapement = 0;
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CharEscapement" ) ), bGetPropStateValue ) )
    {
        mAny >>= mnCharEscapement;
        if ( mnCharEscapement > 100 )
            mnCharEscapement = 33;
        else if ( mnCharEscapement < -100 )
            mnCharEscapement = -33;
    }
    meCharEscapement = ePropState;
}

void PortionObj::ImplClear()
{
    delete (FieldEntry*)mpFieldEntry;
    delete mpText;
}

void PortionObj::ImplConstruct( PortionObj& rPortionObj )
{
    mbLastPortion = rPortionObj.mbLastPortion;
    mnTextSize = rPortionObj.mnTextSize;
    mnCharColor = rPortionObj.mnCharColor;
    mnCharEscapement = rPortionObj.mnCharEscapement;
    mnCharAttr = rPortionObj.mnCharAttr;
    mnCharHeight = rPortionObj.mnCharHeight;
    mnFont = rPortionObj.mnFont;

    if ( rPortionObj.mpText )
    {
        mpText = new sal_uInt16[ mnTextSize ];
        memcpy( mpText, rPortionObj.mpText, mnTextSize << 1 );
    }
    else
        mpText = NULL;

    if ( rPortionObj.mpFieldEntry )
        mpFieldEntry = new FieldEntry( *( rPortionObj.mpFieldEntry ) );
    else
        mpFieldEntry = NULL;
}

sal_uInt32 PortionObj::ImplCalculateTextPositions( sal_uInt32 nCurrentTextPosition )
{
    if ( mpFieldEntry && ( !mpFieldEntry->nFieldStartPos ) )
    {
        mpFieldEntry->nFieldStartPos += nCurrentTextPosition;
        mpFieldEntry->nFieldEndPos += nCurrentTextPosition;
    }
    return mnTextSize;
}

//  -----------------------------------------------------------------------
// Rueckgabe:                           0 = kein TextField
//  bit28->31   art des TextFields :
//                                      1 = Date
//                                      2 = Time
//                                      3 = SlideNumber
//                                      4 = Url
//  bit24->27   PPT Textfield type
//     23->     PPT Textfield needs a placeholder

sal_uInt32 PortionObj::ImplGetTextField( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >
                                            & rXCursorText, String& rURL )
{
    sal_uInt32 nRetValue = 0;
    sal_Int32 nFormat;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextField >
        aXTextField( rXCursorText, ::com::sun::star::uno::UNO_QUERY );

    if ( aXTextField.is() )
    {
        mXPropSet = ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >
                ( aXTextField, ::com::sun::star::uno::UNO_QUERY );

        if ( mXPropSet.is() )
        {
            String aFieldKind( aXTextField->getPresentation( TRUE ) );
            if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Date" ) ) )
            {
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsFix" ) ) ) )
                {
                    sal_Bool bBool;
                    mAny >>= bBool;
                    if ( !bBool )  // Fixed DateFields gibt es in PPT nicht
                    {
                        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Format" ) ) ) )
                        {
                            nFormat = *(sal_Int32*)mAny.getValue();
                            switch ( nFormat )
                            {
                                default:
                                case 5 :
                                case 4 :
                                case 2 : nFormat = 0; break;
                                case 8 :
                                case 9 :
                                case 3 : nFormat = 1; break;
                                case 7 :
                                case 6 : nFormat = 2; break;
                            }
                            nRetValue |= ( ( ( 1 << 4 ) | nFormat ) << 24 ) | 0x800000;
                        }
                    }
                }
            }
            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Url" ) ) )
            {
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "URL" ) ) ) )
                    rURL = String( *(::rtl::OUString*)mAny.getValue() );
                nRetValue = 4 << 28;
            }
            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Page" ) ) )
            {
                nRetValue = 3 << 28 | 0x800000;
            }
            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Pages" ) ) )
            {

            }
            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Time" ) ) )
            {
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsFix" ) ) ) )
                {
                    sal_Bool bBool;
                    mAny >>= bBool;
                    if ( !bBool )
                    {
                        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsFix" ) ) ) )
                        {
                            nFormat = *(sal_Int32*)mAny.getValue();
                            nRetValue |= ( ( ( 2 << 4 ) | nFormat ) << 24 ) | 0x800000;
                        }
                    }
                }
            }
            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "File" ) ) )
            {

            }
            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "Table" ) ) )
            {

            }
            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "ExtTime" ) ) )
            {
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsFix" ) ) ) )
                {
                    sal_Bool bBool;
                    mAny >>= bBool;
                    if ( !bBool )
                    {
                        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Format" ) ) ) )
                        {
                            nFormat = *(sal_Int32*)mAny.getValue();
                            switch ( nFormat )
                            {
                                default:
                                case 6 :
                                case 7 :
                                case 8 :
                                case 2 : nFormat = 12; break;
                                case 3 : nFormat = 9; break;
                                case 5 :
                                case 4 : nFormat = 10; break;

                            }
                            nRetValue |= ( ( ( 2 << 4 ) | nFormat ) << 24 ) | 0x800000;
                        }
                    }
                }
            }
            else if ( aFieldKind == String( RTL_CONSTASCII_USTRINGPARAM( "ExtFile" ) ) )
            {

            }
            else if ( aFieldKind ==  String( RTL_CONSTASCII_USTRINGPARAM( "Author" ) ) )
            {

            }
        }
    }
    return nRetValue;
}

PortionObj& PortionObj::operator=( PortionObj& rPortionObj )
{
    if ( this != &rPortionObj )
    {
        ImplClear();
        ImplConstruct( rPortionObj );
    }
    return *this;
}

//  -----------------------------------------------------------------------

ParagraphObj::ParagraphObj( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet,
                PPTExBulletProvider& rProv ) :
    maMapModeSrc        ( MAP_100TH_MM ),
    maMapModeDest       ( MAP_INCH, Point(), Fraction( 1, 576 ), Fraction( 1, 576 ) ),
    mpTab               ( NULL )
{
    mXPropSet = rXPropSet;

    bDepth = bExtendedParameters = FALSE;

    nDepth = 0;
    nBulletFlags = 0;
    nParaFlags = 0;

    ImplGetParagraphValues( rProv, FALSE );
}

    ParagraphObj::ParagraphObj( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > & rXTextContent,
                    ParaFlags aParaFlags, Collection& rFontCollection, PPTExBulletProvider& rProv ) :
    maMapModeSrc        ( MAP_100TH_MM ),
    maMapModeDest       ( MAP_INCH, Point(), Fraction( 1, 576 ), Fraction( 1, 576 ) ),
    mbFirstParagraph    ( aParaFlags.bFirstParagraph ),
    mbLastParagraph     ( aParaFlags.bLastParagraph ),
    mpTab               ( NULL )
{
    bDepth = bExtendedParameters = FALSE;

    nDepth = 0;
    nBulletFlags = 0;
    nParaFlags = 0;

    mXPropSet = ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
            ( rXTextContent, ::com::sun::star::uno::UNO_QUERY );

    mXPropState = ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertyState >
            ( rXTextContent, ::com::sun::star::uno::UNO_QUERY );

    if ( mXPropSet.is() && mXPropState.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess >
            aXTextPortionEA( rXTextContent, ::com::sun::star::uno::UNO_QUERY );
        if ( aXTextPortionEA.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >
                aXTextPortionE( aXTextPortionEA->createEnumeration() );
            if ( aXTextPortionE.is() )
            {
                while ( aXTextPortionE->hasMoreElements() )
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > aXCursorText;
                    ::com::sun::star::uno::Any aAny( aXTextPortionE->nextElement() );
                    ;
                    if ( aAny >>= aXCursorText )
                        Insert( new PortionObj( aXCursorText, !aXTextPortionE->hasMoreElements(), rFontCollection ), LIST_APPEND );
                }
            }
        }
        ImplGetParagraphValues( rProv, TRUE );//
    }
}

ParagraphObj::ParagraphObj( ParagraphObj& rObj )
{
    ImplConstruct( rObj );
}

ParagraphObj::~ParagraphObj()
{
    ImplClear();
}

void ParagraphObj::Write( SvStream* pStrm )
{
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        ((PortionObj*)pPtr)->Write( pStrm, mbLastParagraph );
}

void ParagraphObj::ImplClear()
{
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        delete (PortionObj*)pPtr;
    delete mpTab;
}

void ParagraphObj::ImplGetNumberingLevel( PPTExBulletProvider& rBuProv, sal_Int16 nDepth, sal_Bool bGetPropStateValue )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace > aXIndexReplace;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "NumberingRules" ) ), bGetPropStateValue ) )
    {
        if ( ( mAny >>= aXIndexReplace ) && nDepth < aXIndexReplace->getCount() )
        {
            mAny <<= aXIndexReplace->getByIndex( nDepth );
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>
                aPropertySequence( *( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>*)mAny.getValue() );

            const ::com::sun::star::beans::PropertyValue* pPropValue = aPropertySequence.getArray();

            sal_Int16 nCount = aPropertySequence.getLength();
            if ( nCount )
            {
                bExtendedParameters = TRUE;
                nBulletRealSize = 0;

                sal_Bool bFastFind = TRUE;

                Size aBuGraSize( 0, 0 );

                nMappedNumType = 0;

                for ( sal_Int16 i = 0; i < nCount; i++ )
                {
                    sal_Int16 nQuickIndex = i;

                    const void* pValue = pPropValue[ i ].Value.getValue();
                    if ( pValue )
                    {
                        String aString( pPropValue[ i ].Name );
                        ByteString aPropName( aString, RTL_TEXTENCODING_UTF8 );
                        if ( !bFastFind )
                        {
                            if ( aPropName == "NumberingType" )
                                nQuickIndex = 0;
                            else if ( aPropName == "Adjust" )
                                nQuickIndex = 1;
                            else if ( aPropName == "Prefix" )
                                nQuickIndex = 2;
                            else if ( aPropName == "Suffix" )
                                nQuickIndex = 3;
                            else if ( aPropName == "BulletChar" )
                                nQuickIndex = 4;
                            else if ( aPropName == "BulletFont" )
                                nQuickIndex = 5;
                            else if ( aPropName == "Graphic" )
                                nQuickIndex = 6;
                            else if ( aPropName == "GraphicSize" )
                                nQuickIndex = 7;
                            else if ( aPropName == "StartWith" )
                                nQuickIndex = 8;
                            else if ( aPropName ==  "LeftMargin" )
                                nQuickIndex = 9;
                            else if ( aPropName == "FirstLineOffset" )
                                nQuickIndex = 10;
                            else if ( aPropName == "BulletColor" )
                                nQuickIndex = 11;
                            else if ( aPropName == "BulletRelSize" )
                                nQuickIndex = 12;
                            else
                            {
                                DBG_ERROR( "Unbekanntes Property" );
                                bFastFind = TRUE;
                                continue;   // unbekanntes Property;
                            }
                            pValue = pPropValue[ i ].Value.getValue();
                        }
                        switch( nQuickIndex )
                        {
                            case 0 :
                            {
                                if ( aPropName == "NumberingType" )
                                {
                                    nNumberingType = *( (sal_Int16*)pValue );
                                    continue;
                                }
                            }
                            break;
                            case 1 :
                            {
                                if ( aPropName == "Adjust" )
                                {
                                    nHorzAdjust = *( (sal_Int16*)pValue );
                                    continue;
                                }
                            }
                            break;
                            case 2 :
                            {
                                if ( aPropName == "Prefix" )
                                {
//                                  sPrefix = *( (String*)pValue );
                                    continue;
                                }
                            }
                            break;
                            case 3 :
                            {
                                if ( aPropName == "Suffix" )
                                {
//                                  sSuffix = *( (String*)pValue );
                                    continue;
                                }
                            }
                            break;
                            case 4 :
                            {
                                if ( aPropName == "BulletChar" )
                                {
                                    String aString( *( (String*)pValue ) );
                                    if ( aString.Len() )
                                        cBulletId = aString.GetChar( 0 );
                                    continue;
                                }
                            }
                            break;
                            case 5 :
                            {
                                if ( aPropName == "BulletFont" )
                                {
                                    aFontDesc = *( (::com::sun::star::awt::FontDescriptor*)pValue );
                                    continue;
                                }
                            }
                            break;
                            case 6 :
                            {
                                if ( aPropName == "Graphic" )
                                {
                                    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >xBitmap;
                                    if ( ::cppu::extractInterface( xBitmap, pPropValue[ i ].Value ) )
                                    {
                                        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >
                                            xBitmap( *(::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >*)pValue );
                                        if ( xBitmap.is() )
                                        {
                                            Graphic aGraphic( VCLUnoHelper::GetBitmap( xBitmap ) );
                                            nBulletId = rBuProv.GetId( aGraphic );
                                            if ( nBulletId != 0xffff )
                                                bExtendedBulletsUsed = TRUE;
                                        }
                                    }
                                    continue;
                                }
                            }
                            break;
                            case 7 :
                            {
                                if ( aPropName == "GraphicSize" )
                                {
                                    if ( pPropValue[ i ].Value.getValueType() == ::getCppuType( (::com::sun::star::awt::Size*)0) )
                                        aBuGraSize =  *(Size*)pValue;
                                    continue;
                                }
                            }
                            break;
                            case 8 :
                            {
                                if ( aPropName == "StartWith" )
                                {
                                    nStartWith = *( (sal_Int16*)pValue );
                                    continue;
                                }
                            }
                            break;
                            case 9 :
                            {
                                if ( aPropName == "LeftMargin" )
                                {
                                    nTextOfs = *( (sal_Int32*)pValue );
                                    nTextOfs /= 4.40972;
                                    continue;
                                }
                            }
                            break;
                            case 10 :
                            {
                                if ( aPropName == "FirstLineOffset" )
                                {
                                    nBulletOfs = *( (sal_Int32*)pValue );
                                    nBulletOfs /= 4.40972;
                                    continue;
                                }
                            }
                            break;
                            case 11 :
                            {
                                if ( aPropName == "BulletColor" )
                                {
                                    sal_uInt32 nSOColor = *( (sal_uInt32*)pValue );
                                    nBulletColor = nSOColor & 0xff00;                           // GRUEN
                                    nBulletColor |= (sal_uInt8)( nSOColor ) << 16;              // ROT
                                    nBulletColor |= (sal_uInt8)( nSOColor >> 16 ) | 0xfe000000; // BLAU
                                    continue;
                                }
                            }
                            break;
                            case 12 :
                            {
                                if ( aPropName == "BulletRelSize" )
                                {
                                    nBulletRealSize = *( (sal_Int16*)pValue );
                                    nParaFlags |= 0x40;
                                    nBulletFlags |= 8;
                                    continue;
                                }
                            }
                        }
                        if ( bFastFind )
                        {
                            bFastFind = FALSE;
                            i--;    // nochmal den letzten index, aber diesmal andere Reihenfolge beruecksichtigen
                        }
                    }
                }
                if ( ( (SvxExtNumType)nNumberingType == SVX_NUM_BITMAP ) && ( nBulletId != 0xffff ) )
                {
                    // calculate the bulletrealsize for this grafik
                    nBulletRealSize = 100;
                    if ( aBuGraSize.Width() && aBuGraSize.Height() )
                    {
                        sal_Int32 nCharHeight = 24;
                        PortionObj* pPortion = (PortionObj*)First();
                        if ( pPortion )
                            nCharHeight = pPortion->mnCharHeight;

                        sal_Int32 nLen = aBuGraSize.Width();
                        if ( aBuGraSize.Height() > nLen )
                            nLen = aBuGraSize.Height();

                        nCharHeight = (double)nCharHeight * 25.40;
                        double fQuo = (double)nLen / (double)nCharHeight;
                        nBulletRealSize = 100 * fQuo;
                        if ( (sal_uInt16)nBulletRealSize > 400 )
                            nBulletRealSize = 400;
                    }
                }
                switch( (SvxExtNumType)nNumberingType )
                {
                    case SVX_NUM_NUMBER_NONE : nParaFlags |= 0xf; break;

                    case SVX_NUM_CHAR_SPECIAL :                           // Bullet
                    {
                        if ( aFontDesc.Name.len() )
                        {
                            if ( aFontDesc.CharSet != ::com::sun::star::awt::CharSet::SYMBOL )
                            {
                                switch ( cBulletId )
                                {
                                    // Currency
                                    case 128:   cBulletId = 0x20AC; break;
                                    // Punctuation and other
                                    case 130:   cBulletId = 0x201A; break;// SINGLE LOW-9 QUOTATION MARK
                                    case 131:   cBulletId = 0x0192; break;// LATIN SMALL LETTER F WITH HOOK
                                    case 132:   cBulletId = 0x201E; break;// DOUBLE LOW-9 QUOTATION MARK
                                                                          // LOW DOUBLE PRIME QUOTATION MARK
                                    case 133:   cBulletId = 0x2026; break;// HORIZONTAL ELLIPSES
                                    case 134:   cBulletId = 0x2020; break;// DAGGER
                                    case 135:   cBulletId = 0x2021; break;// DOUBLE DAGGER
                                    case 136:   cBulletId = 0x02C6; break;// MODIFIER LETTER CIRCUMFLEX ACCENT
                                    case 137:   cBulletId = 0x2030; break;// PER MILLE SIGN
                                    case 138:   cBulletId = 0x0160; break;// LATIN CAPITAL LETTER S WITH CARON
                                    case 139:   cBulletId = 0x2039; break;// SINGLE LEFT-POINTING ANGLE QUOTATION MARK
                                    case 140:   cBulletId = 0x0152; break;// LATIN CAPITAL LIGATURE OE
                                    case 142:   cBulletId = 0x017D; break;// LATIN CAPITAL LETTER Z WITH CARON
                                    case 145:   cBulletId = 0x2018; break;// LEFT SINGLE QUOTATION MARK
                                                                          // MODIFIER LETTER TURNED COMMA
                                    case 146:   cBulletId = 0x2019; break;// RIGHT SINGLE QUOTATION MARK
                                                                          // MODIFIER LETTER APOSTROPHE
                                    case 147:   cBulletId = 0x201C; break;// LEFT DOUBLE QUOTATION MARK
                                                                          // REVERSED DOUBLE PRIME QUOTATION MARK
                                    case 148:   cBulletId = 0x201D; break;// RIGHT DOUBLE QUOTATION MARK
                                                                          // REVERSED DOUBLE PRIME QUOTATION MARK
                                    case 149:   cBulletId = 0x2022; break;// BULLET
                                    case 150:   cBulletId = 0x2013; break;// EN DASH
                                    case 151:   cBulletId = 0x2014; break;// EM DASH
                                    case 152:   cBulletId = 0x02DC; break;// SMALL TILDE
                                    case 153:   cBulletId = 0x2122; break;// TRADE MARK SIGN
                                    case 154:   cBulletId = 0x0161; break;// LATIN SMALL LETTER S WITH CARON
                                    case 155:   cBulletId = 0x203A; break;// SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
                                    case 156:   cBulletId = 0x0153; break;// LATIN SMALL LIGATURE OE
                                    case 158:   cBulletId = 0x017E; break;// LATIN SMALL LETTER Z WITH CARON
                                    case 159:   cBulletId = 0x0178; break;// LATIN CAPITAL LETTER Y WITH DIAERESIS
//                                  case 222:   cBulletId = 0x00B6; break;// PILCROW SIGN / PARAGRAPH SIGN
                                }
                            }
                            nParaFlags |= 0x90; // wir geben den Font und den Charset vor
                        }
                    }
                    case SVX_NUM_CHARS_UPPER_LETTER :       // zaehlt von a-z, aa - az, ba - bz, ...
                    case SVX_NUM_CHARS_LOWER_LETTER :
                    case SVX_NUM_ROMAN_UPPER :
                    case SVX_NUM_ROMAN_LOWER :
                    case SVX_NUM_ARABIC :
                    case SVX_NUM_PAGEDESC :                 // Numerierung aus der Seitenvorlage
                    case SVX_NUM_BITMAP :
                    case SVX_NUM_CHARS_UPPER_LETTER_N :     // zaehlt von  a-z, aa-zz, aaa-zzz
                    case SVX_NUM_CHARS_LOWER_LETTER_N :
                    {
                        if ( nNumberingType != SVX_NUM_CHAR_SPECIAL )
                        {
                            bExtendedBulletsUsed = TRUE;
                            if ( nDepth & 1 )
                                cBulletId = 0x2013;         // defaulting bullet characters for ppt97
                            else if ( nDepth == 4 )
                                cBulletId = 0xbb;
                            else
                                cBulletId = 0x2022;

                            switch( (SvxExtNumType)nNumberingType )
                            {
                                case SVX_NUM_CHARS_UPPER_LETTER :
                                case SVX_NUM_CHARS_UPPER_LETTER_N :
                                {
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
                                            nMappedNumType = 0xa0001;   // (A)
                                        else
                                            nMappedNumType = 0xb0001;   // A)
                                    }
                                    else
                                        nMappedNumType = 0x10001;       // A.
                                }
                                break;
                                case SVX_NUM_CHARS_LOWER_LETTER :
                                case SVX_NUM_CHARS_LOWER_LETTER_N :
                                {
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
                                            nMappedNumType = 0x80001;   // (a)
                                        else
                                            nMappedNumType = 0x90001;   // a)
                                    }
                                    else
                                        nMappedNumType = 0x00001;       // a.
                                }
                                break;
                                case SVX_NUM_ROMAN_UPPER :
                                {
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
                                            nMappedNumType = 0xe0001;   // (I)
                                        else
                                            nMappedNumType = 0xf0001;   // I)
                                    }
                                    else
                                        nMappedNumType = 0x70001;       // I.
                                }
                                break;
                                case SVX_NUM_ROMAN_LOWER :
                                {
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
                                            nMappedNumType = 0x40001;   // (i)
                                        else
                                            nMappedNumType = 0x50001;   // i)
                                    }
                                    else
                                        nMappedNumType = 0x60001;       // i.
                                }
                                break;
                                case SVX_NUM_ARABIC :
                                {
                                    if ( sSuffix == String( RTL_CONSTASCII_USTRINGPARAM( ")" ) ) )
                                    {
                                        if ( sPrefix == String( RTL_CONSTASCII_USTRINGPARAM( "(" ) ) )
                                            nMappedNumType = 0xc0001;   // (1)
                                        else
                                            nMappedNumType = 0x20001;   // 1)
                                    }
                                    else
                                    {
                                        if ( ! ( sSuffix.Len() + sPrefix.Len() ) )
                                            nMappedNumType = 0xd0001;   // 1
                                        else
                                            nMappedNumType = 0x30001;   // 1.
                                    }
                                }
                                break;
                            }
                        }
                        nParaFlags |= 0x2f;
                        nBulletFlags |= 6;
                        if ( mbIsBullet )
                            nBulletFlags |= 1;
                    }
                }
            }
            nBulletOfs = nTextOfs + nBulletOfs;
            if ( nBulletOfs < 0 )
                nBulletOfs = 0;
        }
    }
}

void ParagraphObj::ImplGetParagraphValues( PPTExBulletProvider& rBuProv, sal_Bool bGetPropStateValue )
{
    meBullet = ::com::sun::star::beans::PropertyState_DIRECT_VALUE;
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsNumbering" ) ), bGetPropStateValue ) )
    {
        meBullet = ePropState;
        mAny >>= mbIsBullet;
        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "NumberingLevel" ) ), bGetPropStateValue ) )
        {
            meBullet = ePropState;
            nDepth = *( (sal_Int16*)mAny.getValue() );
            if ( nDepth > 4 )
                nDepth = 4;
            bDepth = TRUE;
        }
        else
            nDepth = 0;
        ImplGetNumberingLevel( rBuProv, nDepth, bGetPropStateValue );
    }
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaTabstops" ) ), bGetPropStateValue ) )
    {
        ::com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop>& rSeq =
            *( ::com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop>*)mAny.getValue();
        sal_Int32 nCount = rSeq.getLength();
        ::com::sun::star::style::TabStop* pTabStop = (::com::sun::star::style::TabStop*)rSeq.getConstArray();
        if ( nCount && pTabStop )
        {
            mpTab = new sal_Int32[ nCount + 1 ];
            mpTab[ 0 ] = nCount;
            for ( sal_uInt32 i = 0; i < nCount; i++ )
                mpTab[ i + 1 ] = ( pTabStop[ i ].Position / 4.40972 ) + nTextOfs;
        }
    }
    ::com::sun::star::drawing::TextAdjust eTextAdjust( ::com::sun::star::drawing::TextAdjust_LEFT );
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaAdjust" ) ), bGetPropStateValue ) )
        eTextAdjust = (::com::sun::star::drawing::TextAdjust)EncodeAnyTosal_Int16( mAny );
    switch ( eTextAdjust )
    {
        case ::com::sun::star::drawing::TextAdjust_CENTER :
            mnTextAdjust = 1;
        break;
        case ::com::sun::star::drawing::TextAdjust_RIGHT :
            mnTextAdjust = 2;
        break;
        default :
        case ::com::sun::star::drawing::TextAdjust_LEFT :
            mnTextAdjust = 0;
        break;
    }
    meTextAdjust = ePropState;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaLineSpacing" ) ), bGetPropStateValue ) )
    {
        ::com::sun::star::style::LineSpacing aLineSpacing
            = *( (::com::sun::star::style::LineSpacing*)mAny.getValue() );
        switch ( aLineSpacing.Mode )
        {
            case ::com::sun::star::style::LineSpacingMode::MINIMUM :
            case ::com::sun::star::style::LineSpacingMode::LEADING :
            case ::com::sun::star::style::LineSpacingMode::FIX :
                mnLineSpacing = (sal_Int16)(-( aLineSpacing.Height / 4.40972 ) );
            break;

            case ::com::sun::star::style::LineSpacingMode::PROP :
            default:
                mnLineSpacing = (sal_Int16)( aLineSpacing.Height );
            break;
        }
    }
    meLineSpacing = ePropState;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaBottomMargin" ) ), bGetPropStateValue ) )
    {
        sal_uInt32 nSpacing = *( (sal_uInt32*)mAny.getValue() );
        mnLineSpacingBottom = (sal_Int16)(-( nSpacing / 4.40972 ) );
    }
    meLineSpacingBottom = ePropState;

    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ParaTopMargin" ) ), bGetPropStateValue ) )
    {
        sal_uInt32 nSpacing = *( (sal_uInt32*)mAny.getValue() );
        mnLineSpacingTop = (sal_Int16)(-( nSpacing / 4.40972 ) );
    }
    meLineSpacingTop = ePropState;
}

void ParagraphObj::ImplConstruct( ParagraphObj& rParagraphObj )
{
    mnTextSize = rParagraphObj.mnTextSize;
    mnTextAdjust = rParagraphObj.mnTextAdjust;
    mnLineSpacing = rParagraphObj.mnLineSpacing;
    mnLineSpacingTop = rParagraphObj.mnLineSpacingTop;
    mnLineSpacingBottom = rParagraphObj.mnLineSpacingBottom;
    mbFirstParagraph = rParagraphObj.mbFirstParagraph;
    mbLastParagraph = rParagraphObj.mbLastParagraph;

    for ( void* pPtr = rParagraphObj.First(); pPtr; pPtr = rParagraphObj.Next() )
        Insert( new PortionObj( *(PortionObj*)pPtr ), LIST_APPEND );

    if ( rParagraphObj.mpTab && rParagraphObj.mpTab[ 0 ] )
    {
        sal_uInt32 nCount = rParagraphObj.mpTab[ 0 ] + 1;
        mpTab = new sal_Int32[ nCount ];
        memcpy( mpTab, rParagraphObj.mpTab, nCount << 2 );
    }
    else
        mpTab = NULL;

    bDepth = rParagraphObj.bDepth;
    bExtendedParameters = rParagraphObj.bExtendedParameters;
    nParaFlags = rParagraphObj.nParaFlags;
    nBulletFlags = rParagraphObj.nBulletFlags;
    sPrefix = rParagraphObj.sPrefix;
    sSuffix = rParagraphObj.sSuffix;
    sGraphicUrl = rParagraphObj.sGraphicUrl;            // String auf eine Graphic
    nNumberingType = rParagraphObj.nNumberingType;      // in wirlichkeit ist dies ein SvxEnum
    nHorzAdjust = rParagraphObj.nHorzAdjust;
    nBulletColor = rParagraphObj.nBulletColor;
    nBulletOfs = rParagraphObj.nBulletOfs;
    nStartWith = rParagraphObj.nStartWith;              // Start der nummerierung
    nTextOfs = rParagraphObj.nTextOfs;
    nBulletRealSize = rParagraphObj.nBulletRealSize;    // GroessenVerhaeltnis in Proz
    nDepth = rParagraphObj.nDepth;                      // aktuelle tiefe
    cBulletId = rParagraphObj.cBulletId;                // wenn Numbering Type == CharSpecial
    aFontDesc = rParagraphObj.aFontDesc;

    bExtendedBulletsUsed = rParagraphObj.bExtendedBulletsUsed;
    nBulletId = rParagraphObj.nBulletId;
}

::com::sun::star::awt::Size ParagraphObj::ImplMapSize( const ::com::sun::star::awt::Size& rSize )
{
    Size aSize( OutputDevice::LogicToLogic( Size( rSize.Width, rSize.Height ), maMapModeSrc, maMapModeDest ) );
    if ( !aSize.Width() )
        aSize.Width()++;
    if ( !aSize.Height() )
        aSize.Height()++;
    return ::com::sun::star::awt::Size( aSize.Width(), aSize.Height() );
}

sal_uInt32 ParagraphObj::ImplCalculateTextPositions( sal_uInt32 nCurrentTextPosition )
{
    mnTextSize = 0;
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        mnTextSize += ((PortionObj*)pPtr)->ImplCalculateTextPositions( nCurrentTextPosition + mnTextSize );
    return mnTextSize;
}

ParagraphObj& ParagraphObj::operator=( ParagraphObj& rParagraphObj )
{
    if ( this != &rParagraphObj )
    {
        ImplClear();
        ImplConstruct( rParagraphObj );
    }
    return *this;
}

//  -----------------------------------------------------------------------

ImplTextObj::ImplTextObj( int nInstance )
{
    mnRefCount = 1;
    mnTextSize = 0;
    mnInstance = nInstance;
    mpList = new List;
    mbHasExtendedBullets = FALSE;
}

ImplTextObj::~ImplTextObj()
{
    for ( ParagraphObj* pPtr = (ParagraphObj*)mpList->First(); pPtr; pPtr = (ParagraphObj*)mpList->Next() )
        delete pPtr;
    delete mpList;
}

TextObj::TextObj( ::com::sun::star::uno::Reference< ::com::sun::star::text::XSimpleText > & rXTextRef,
            int nInstance, Collection& rFontCollection, PPTExBulletProvider& rProv )
{
    mpImplTextObj = new ImplTextObj( nInstance );

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess >
        aXTextParagraphEA( rXTextRef, ::com::sun::star::uno::UNO_QUERY );

    if ( aXTextParagraphEA.is()  )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >
            aXTextParagraphE( aXTextParagraphEA->createEnumeration() );
        if ( aXTextParagraphE.is() )
        {
            ParaFlags aParaFlags;
            while ( aXTextParagraphE->hasMoreElements() )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > aXParagraph;
                ::com::sun::star::uno::Any aAny( aXTextParagraphE->nextElement() );
                if ( aAny >>= aXParagraph )
                {
                    if ( !aXTextParagraphE->hasMoreElements() )
                        aParaFlags.bLastParagraph = TRUE;
                    ParagraphObj* pPara = new ParagraphObj( aXParagraph, aParaFlags, rFontCollection, rProv );
                    mpImplTextObj->mbHasExtendedBullets |= pPara->bExtendedBulletsUsed;
                    mpImplTextObj->mpList->Insert( pPara, LIST_APPEND );
                    aParaFlags.bFirstParagraph = FALSE;
                }
            }
        }
    }
    ImplCalculateTextPositions();
}

TextObj::TextObj( TextObj& rTextObj )
{
    mpImplTextObj = rTextObj.mpImplTextObj;
    mpImplTextObj->mnRefCount++;
}

TextObj::~TextObj()
{
    if ( ! ( --mpImplTextObj->mnRefCount ) )
        delete mpImplTextObj;
}

void TextObj::Write( SvStream* pStrm )
{
    sal_uInt32 nSize, nPos = pStrm->Tell();
    *pStrm << (sal_uInt32)( EPP_TextCharsAtom << 16 ) << (sal_uInt32)0;
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        ((ParagraphObj*)pPtr)->Write( pStrm );
    nSize = pStrm->Tell() - nPos;
    pStrm->SeekRel( - ( nSize - 4 ) );
    *pStrm << (sal_uInt32)( nSize - 8 );
    pStrm->SeekRel( nSize - 8 );
}

void TextObj::ImplCalculateTextPositions()
{
    mpImplTextObj->mnTextSize = 0;
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        mpImplTextObj->mnTextSize += ((ParagraphObj*)pPtr)->ImplCalculateTextPositions( mpImplTextObj->mnTextSize );
}

TextObj& TextObj::operator=( TextObj& rTextObj )
{
    if ( this != &rTextObj )
    {
        if ( ! ( --mpImplTextObj->mnRefCount ) )
            delete mpImplTextObj;
        mpImplTextObj = rTextObj.mpImplTextObj;
        mpImplTextObj->mnRefCount++;
    }
    return *this;
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplWriteTextStyleAtom( SvStream& rOut, int nTextInstance,
                    sal_uInt32 nAtomInstance, TextRuleEntry* pTextRule, SvStream& rExtBuStr )
{
    PPTExParaSheet& rParaSheet = mpStyleSheet->GetParaSheet( nTextInstance );

    rOut << (sal_uInt32)( ( EPP_TextHeaderAtom << 16 ) | ( nAtomInstance << 4 ) ) << (sal_uInt32)4
         << nTextInstance;

    List* pFieldList = NULL;

    if ( mbEmptyPresObj )
        mnTextSize = 0;
    if ( mnTextSize )
    {
        TextObj aTextObj( mXText, nTextInstance, maFontCollection, (PPTExBulletProvider&)*this );
        aTextObj.Write( &rOut );

        sal_uInt32 nSize, nPos = rOut.Tell();
        rOut << (sal_uInt32)( EPP_StyleTextPropAtom << 16 ) << (sal_uInt32)0;
        ImplWriteParagraphs( rOut, aTextObj, mnTextStyle );
        ImplWritePortions( rOut, aTextObj );
        nSize = rOut.Tell() - nPos;
        rOut.SeekRel( - ( nSize - 4 ) );
        rOut << (sal_uInt32)( nSize - 8 );
        rOut.SeekRel( nSize - 8 );

        for ( ParagraphObj* pPara = aTextObj.First(); pPara; pPara = aTextObj.Next() )
        {
            for ( PortionObj* pPortion = (PortionObj*)pPara->First(); pPortion; pPortion = (PortionObj*)pPara->Next() )
            {
                if ( pPortion->mpFieldEntry )
                {
                    const FieldEntry* pFieldEntry = pPortion->mpFieldEntry;

                    switch ( pFieldEntry->nFieldType >> 28 )
                    {
                        case 1 :
                        case 2 :
                        {
                            rOut << (sal_uInt32)( EPP_DateTimeMCAtom << 16 ) << (sal_uInt32)8
                                 << (sal_uInt32)( pFieldEntry->nFieldStartPos )          // TxtOffset auf TxtField;
                                 << (sal_uInt8)( ( pFieldEntry->nFieldType >> 24 ) & 0xf )// Type
                                 << (sal_uInt8)0 << (sal_uInt16)0;                            // PadBytes
                        }
                        break;
                        case 3 :
                        {
                            rOut << (sal_uInt32)( EPP_SlideNumberMCAtom << 16 ) << (sal_uInt32 ) 4
                                 << (sal_uInt32)( pFieldEntry->nFieldStartPos );
                        }
                        break;
                        case 4 :
                        {
                            sal_uInt32 nHyperId = ++mnExEmbed;

                            rOut << (sal_uInt32)( ( EPP_InteractiveInfo << 16 ) | 0xf ) << (sal_uInt32)24
                                 << (sal_uInt32)( EPP_InteractiveInfoAtom << 16 ) << (sal_uInt32)16
                                 << (sal_uInt32)0                                    // soundref
                                 << nHyperId                                     // hyperlink id
                                 << (sal_uInt8)4                                      // hyperlink action
                                 << (sal_uInt8)0                                      // ole verb
                                 << (sal_uInt8)0                                      // jump
                                 << (sal_uInt8)0                                      // flags
                                 << (sal_uInt8)8                                      // hyperlink type ?
                                 << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0
                                 << (sal_uInt32)( EPP_TxInteractiveInfoAtom << 16 ) << (sal_uInt32)8
                                 << (sal_uInt32)( pFieldEntry->nFieldStartPos )
                                 << (sal_uInt32)( pFieldEntry->nFieldEndPos );

                            maHyperlink.Insert( new EPPTHyperlink( pFieldEntry->aFieldUrl, 2 | ( nHyperId << 8 ) ), LIST_APPEND );

                            *mpExEmbed  << (sal_uInt16)0xf
                                        << (sal_uInt16)EPP_ExHyperlink
                                        << (sal_uInt32)12
                                        << (sal_uInt16)0
                                        << (sal_uInt16)EPP_ExHyperlinkAtom
                                        << (sal_uInt32)4
                                        << nHyperId;
                        }
                        default:
                        break;
                    }
                }
            }
        }
        // SJ: if this textspecinfoatom is missing, ppt will crash as often as possible
        // MS documentation : special parsing code is needed to parse this content !?!
        rOut << (sal_uInt32)( EPP_TextSpecInfoAtom << 16 ) << (sal_uInt32)10            // ??????????????????????????
             << (sal_uInt32)aTextObj.Count() << (sal_uInt32)2 << (sal_uInt8)9 << (sal_uInt8)8;  // ??????????????????????????

        // Star Office Default TabSizes schreiben ( wenn noetig )
        pPara = aTextObj.First();
        if ( pPara )
        {
            sal_uInt32  nParaFlags = 0x1f;
            sal_Int16   nDepth, nMask, nNumberingRule[ 10 ];
            const   sal_Int32* pTab = pPara->mpTab;

            for ( ; pPara; pPara = aTextObj.Next() )
            {
                if ( pPara->bExtendedParameters )
                {
                    nDepth = pPara->nDepth;
                    if ( nDepth < 5 )
                    {
                        nMask = 1 << nDepth;
                        if ( nParaFlags & nMask )
                        {
                            nParaFlags &=~ nMask;
                            if ( ( rParaSheet.maParaLevel[ nDepth ].mnTextOfs != pPara->nTextOfs ) ||
                                ( rParaSheet.maParaLevel[ nDepth ].mnBulletOfs != pPara->nBulletOfs ) )
                            {
                                nParaFlags |= nMask << 16;
                                nNumberingRule[ nDepth << 1 ] = pPara->nTextOfs;
                                nNumberingRule[ ( nDepth << 1 ) + 1 ] = pPara->nBulletOfs;
                            }
                        }
                    }
                }
            }
            nParaFlags >>= 16;

            sal_uInt32  nDefaultTabSize = ImplMapSize( ::com::sun::star::awt::Size( 2011, 1 ) ).Width;
            sal_uInt32  nTabs = ( pTab ) ? pTab[ 0 ] : 0;
            sal_Int32   nDefaultTabs = abs( maRect.GetWidth() ) / nDefaultTabSize;
            if ( nTabs )
                nDefaultTabs -= pTab[ nTabs ] / nDefaultTabSize;
            if ( nDefaultTabs < 0 )
                nDefaultTabs = 0;

            sal_uInt32 nTabCount = nTabs + nDefaultTabs;
            sal_uInt32 i, nTextRulerAtomFlags = 0;

            if ( nTabCount )
                nTextRulerAtomFlags |= 4;
            if ( nParaFlags )
                nTextRulerAtomFlags |= ( ( nParaFlags << 3 ) | ( nParaFlags << 8 ) );

            if ( nTextRulerAtomFlags )
            {
                SvStream* pRuleOut = &rOut;
                if ( pTextRule )
                    pRuleOut = pTextRule->pOut = new SvMemoryStream( 0x100, 0x100 );

                sal_uInt32 nPos = pRuleOut->Tell();
                *pRuleOut << (sal_uInt32)( EPP_TextRulerAtom << 16 ) << (sal_uInt32)0;
                *pRuleOut << nTextRulerAtomFlags;
                if ( nTextRulerAtomFlags & 4 )
                {
                    *pRuleOut << (sal_uInt16)nTabCount;
                    for ( sal_uInt32 i = 1; i <= nTabs; i++ )
                        *pRuleOut << (sal_uInt32)pTab[ i ];

                    sal_uInt32 nWidth = 1;
                    if ( pTab )
                        nWidth += ( pTab[ nTabs ] / nDefaultTabSize );
                    nWidth *= nDefaultTabSize;
                    for ( i = 0; i < nDefaultTabs; i++, nWidth += nDefaultTabSize )
                        *pRuleOut << nWidth;
                }
                for ( i = 0; i < 5; i++ )
                {
                    if ( nTextRulerAtomFlags & ( 8 << i ) )
                        *pRuleOut << nNumberingRule[ i << 1 ];
                    if ( nTextRulerAtomFlags & ( 256 << i ) )
                        *pRuleOut << nNumberingRule[ ( i << 1 ) + 1 ];
                }
                sal_uInt32 nBufSize = pRuleOut->Tell() - nPos;
                pRuleOut->SeekRel( - ( nBufSize - 4 ) );
                *pRuleOut << (sal_uInt32)( nBufSize - 8 );
                pRuleOut->SeekRel( nBufSize - 8 );
            }
        }
        if ( aTextObj.HasExtendedBullets() )
        {
            ParagraphObj* pPara = aTextObj.First();
            if ( pPara )
            {
                sal_uInt32  nBulletFlags;
                sal_uInt32  nNumberingType, nPos = rExtBuStr.Tell();

                rExtBuStr << (sal_uInt32)( EPP_PST_ExtendedParagraphAtom << 16 ) << (sal_uInt32)0;

                for ( ; pPara; pPara = aTextObj.Next() )
                {
                    nBulletFlags = 0;
                    sal_uInt16 nBulletId = pPara->nBulletId;
                    if ( pPara->bExtendedBulletsUsed )
                    {
                        nBulletFlags = 0x800000;
                        if ( pPara->nNumberingType != SVX_NUM_BITMAP )
                            nBulletFlags = 0x3000000;
                    }
                    rExtBuStr << (sal_uInt32)nBulletFlags;

                    if ( nBulletFlags & 0x800000 )
                        rExtBuStr << nBulletId;
                    if ( nBulletFlags & 0x1000000 )
                    {
                        switch( pPara->nNumberingType )
                        {
                            case SVX_NUM_NUMBER_NONE :
                            case SVX_NUM_CHAR_SPECIAL :
                                nNumberingType = 0;
                            break;
                            case SVX_NUM_CHARS_UPPER_LETTER :
                            case SVX_NUM_CHARS_UPPER_LETTER_N :
                            case SVX_NUM_CHARS_LOWER_LETTER :
                            case SVX_NUM_CHARS_LOWER_LETTER_N :
                            case SVX_NUM_ROMAN_UPPER :
                            case SVX_NUM_ROMAN_LOWER :
                            case SVX_NUM_ARABIC :
                                nNumberingType = pPara->nMappedNumType;
                            break;

    //                      case SVX_NUM_PAGEDESC :
                            case SVX_NUM_BITMAP :
                                nNumberingType = 0;
                            break;

                        }
                        rExtBuStr << (sal_uInt32)nNumberingType;
                    }
                    if ( nBulletFlags & 0x2000000 )
                        rExtBuStr << (sal_uInt16)pPara->nStartWith;
                    rExtBuStr << (sal_uInt32)0 << (sal_uInt32)0;
                }
                sal_uInt32 nSize = ( rExtBuStr.Tell() - nPos ) - 8;
                rExtBuStr.SeekRel( - ( nSize + 4 ) );
                rExtBuStr << nSize;
                rExtBuStr.SeekRel( nSize );
            }
        }
    }
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplWriteAny( sal_uInt32 nFlags, sal_Bool bBezier, Polygon* pPolygon )
{

    PolyPolygon aPolyPolygon;
    Polygon     aPolygon;

    if ( pPolygon )
        aPolyPolygon.Insert( *pPolygon, POLYPOLY_APPEND );
    else
    {
        if ( bBezier )
        {
            if ( !ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "PolyPolygonBezier" ) ) ) )
                return;
        }
        else if ( !ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "PolyPolygon" ) ) ) )
                return;

        if ( bBezier )
        {
            ::com::sun::star::drawing::PolyPolygonBezierCoords* pSourcePolyPolygon
                = (::com::sun::star::drawing::PolyPolygonBezierCoords*)mAny.getValue();
            sal_Int32 nOuterSequenceCount = pSourcePolyPolygon->Coordinates.getLength();

            // Zeiger auf innere sequences holen
            ::com::sun::star::drawing::PointSequence* pOuterSequence = pSourcePolyPolygon->Coordinates.getArray();
            ::com::sun::star::drawing::FlagSequence*  pOuterFlags = pSourcePolyPolygon->Flags.getArray();

            if ( ! ( pOuterSequence && pOuterFlags ) )
                return;

            sal_Int32  a, b, nInnerSequenceCount;
            ::com::sun::star::awt::Point* pArray;

            // dies wird ein Polygon set
            for ( a = 0; a < nOuterSequenceCount; a++ )
            {
                ::com::sun::star::drawing::PointSequence* pInnerSequence = pOuterSequence++;
                ::com::sun::star::drawing::FlagSequence*  pInnerFlags = pOuterFlags++;

                if ( ! ( pInnerSequence && pInnerFlags ) )
                    return;

                // Zeiger auf Arrays holen
                pArray = pInnerSequence->getArray();
                ::com::sun::star::drawing::PolygonFlags* pFlags = pInnerFlags->getArray();

                if ( pArray && pFlags )
                {
                    nInnerSequenceCount = pInnerSequence->getLength();
                    aPolygon = Polygon( nInnerSequenceCount );
                    for( b = 0; b < nInnerSequenceCount; b++)
                    {
                        PolyFlags   ePolyFlags( *( (PolyFlags*)pFlags++ ) );
                        ::com::sun::star::awt::Point aPoint( (::com::sun::star::awt::Point)*(pArray++) );
                        aPolygon[ b ] = Point( aPoint.X, aPoint.Y );
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
            ::com::sun::star::drawing::PointSequenceSequence* pSourcePolyPolygon
                = (::com::sun::star::drawing::PointSequenceSequence*)mAny.getValue();
            sal_Int32 nOuterSequenceCount = pSourcePolyPolygon->getLength();

            // Zeiger auf innere sequences holen
            ::com::sun::star::drawing::PointSequence* pOuterSequence = pSourcePolyPolygon->getArray();
            if ( !( pOuterSequence ) )
                return;

            // ist dies ein Polygon oder gar ein PolyPolygon ?
            // sogar eine einfache Line wird als Polygon verpackt !!! ????

            if ( nFlags & ANY_FLAGS_LINE )
            {
                ::com::sun::star::drawing::PointSequence* pInnerSequence = pOuterSequence++;
                if ( !( pInnerSequence ) )
                    return;

                ::com::sun::star::awt::Point* pArray = pInnerSequence->getArray();
                if ( pArray )
                {
                    ::com::sun::star::awt::Point aTopLeft( ImplMapPoint( pArray[ 0 ] ) );
                    ::com::sun::star::awt::Point aBottomRight( ImplMapPoint( pArray[ 1 ] ) );
                    maRect = Rectangle( Point( aTopLeft.X, aTopLeft.Y ), Point( aBottomRight.X, aBottomRight.Y ) );
                }
                return;
            }
            sal_Int32 a, b, nInnerSequenceCount;
            ::com::sun::star::awt::Point* pArray;

            // dies wird ein Polygon set
            for( a = 0; a < nOuterSequenceCount; a++ )
            {
                ::com::sun::star::drawing::PointSequence* pInnerSequence = pOuterSequence++;
                if ( !( pInnerSequence ) )
                    return;

                // Zeiger auf Arrays holen
                if ( pArray = pInnerSequence->getArray() )
                {
                    nInnerSequenceCount = pInnerSequence->getLength();
                    aPolygon = Polygon( nInnerSequenceCount );
                    for( b = 0; b < nInnerSequenceCount; b++)
                    {
                        aPolygon[ b ] = Point( pArray->X, pArray->Y );
                        pArray++;
                    }
                    aPolyPolygon.Insert( aPolygon, POLYPOLY_APPEND );
                }
            }
        }
    }
    sal_Int32 i, j, k, nPoints, nBezPoints, nPolyCount = aPolyPolygon.Count();

    Rectangle   aGeoRect( aPolyPolygon.GetBoundRect() );

    maPosition = ImplMapPoint( ::com::sun::star::awt::Point( aGeoRect.Left(), aGeoRect.Top() ) );
    maSize = ImplMapSize( ::com::sun::star::awt::Size( aGeoRect.GetWidth(), aGeoRect.GetHeight() ) );
    maRect = Rectangle( Point( maPosition.X, maPosition.Y ), Size( maSize.Width, maSize.Height ) );

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
    sal_uInt32 nVerticesBufSize = ( nPoints << 2 ) + 6;
    sal_uInt8* pVerticesBuf = new sal_uInt8[ nVerticesBufSize ];


    sal_uInt32 nSegmentBufSize = ( ( nBezPoints << 2 ) + 8 );
    if ( nPolyCount > 1 )
        nSegmentBufSize += ( nPolyCount << 1 );
    sal_uInt8* pSegmentBuf = new sal_uInt8[ nSegmentBufSize ];

    sal_uInt8* pPtr = pVerticesBuf;
    *pPtr++ = (sal_uInt8)( nPoints );                    // Little endian
    *pPtr++ = (sal_uInt8)( nPoints >> 8 );
    *pPtr++ = (sal_uInt8)( nPoints );
    *pPtr++ = (sal_uInt8)( nPoints >> 8 );
    *pPtr++ = (sal_uInt8)0xf0;
    *pPtr++ = (sal_uInt8)0xff;

    for ( j = 0; j < nPolyCount; j++ )
    {
        aPolygon = aPolyPolygon[ j ];
        nPoints = aPolygon.GetSize();
        for ( i = 0; i < nPoints; i++ )             // Punkte aus Polygon in Buffer schreiben
        {
            Point aPoint = aPolygon[ i ];
            aPoint.X() -= aGeoRect.Left();
            aPoint.Y() -= aGeoRect.Top();

            *pPtr++ = (sal_uInt8)( aPoint.X() );
            *pPtr++ = (sal_uInt8)( aPoint.X() >> 8 );
            *pPtr++ = (sal_uInt8)( aPoint.Y() );
            *pPtr++ = (sal_uInt8)( aPoint.Y() >> 8 );
        }
    }

    pPtr = pSegmentBuf;
    *pPtr++ = (sal_uInt8)( ( nSegmentBufSize - 6 ) >> 1 );
    *pPtr++ = (sal_uInt8)( ( nSegmentBufSize - 6 ) >> 9 );
    *pPtr++ = (sal_uInt8)( ( nSegmentBufSize - 6 ) >> 1 );
    *pPtr++ = (sal_uInt8)( ( nSegmentBufSize - 6 ) >> 9 );
    *pPtr++ = (sal_uInt8)2;
    *pPtr++ = (sal_uInt8)0;

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

    mp_EscherEx->AddOpt( _Escher_Prop_geoRight, aGeoRect.GetSize().Width() );
    mp_EscherEx->AddOpt( _Escher_Prop_geoBottom, aGeoRect.GetSize().Height() );

    mp_EscherEx->AddOpt( _Escher_Prop_shapePath, _Escher_ShapeComplex );
    mp_EscherEx->AddOpt( _Escher_Prop_pVertices, TRUE, nVerticesBufSize - 6, (sal_uInt8*)pVerticesBuf, nVerticesBufSize );
    mp_EscherEx->AddOpt( _Escher_Prop_pSegmentInfo, TRUE, nSegmentBufSize, (sal_uInt8*)pSegmentBuf, nSegmentBufSize );
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplWriteObjectEffect( SvStream& rSt,
    ::com::sun::star::presentation::AnimationEffect eAe,
    ::com::sun::star::presentation::AnimationEffect eTe,
    sal_uInt16 nOrder )
{
    sal_uInt32  nDimColor = 0x7000000;  // color to use for dimming
    sal_uInt32  nFlags = 0x4400;        // set of flags that determine type of build
    sal_uInt32  nSoundRef = 0;          // 0 if storage is from clipboard. Otherwise index(ID) in SoundCollection list.
    sal_uInt32  nDelayTime = 0;         // delay before playing object
    sal_uInt16  nSlideCount = 1;        // number of slides to play object
    UINT8   nBuildType = 1;         // type of build
    UINT8   nFlyMethod = 0;         // animation effect( fly, zoom, appear, etc )
    UINT8   nFlyDirection = 0;      // Animation direction( left, right, up, down, etc )
    UINT8   nAfterEffect = 0;       // what to do after build
    UINT8   nSubEffect = 0;         // build by word or letter
    UINT8   nOleVerb = 0;           // Determines object's class (sound, video, other)

    if ( eAe == ::com::sun::star::presentation::AnimationEffect_NONE )
        eAe = eTe;
    switch ( eAe )
    {
        case ::com::sun::star::presentation::AnimationEffect_WAVYLINE_FROM_TOP :
        case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_TOP :
        case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_TOP :
            nFlyDirection++;
        case ::com::sun::star::presentation::AnimationEffect_WAVYLINE_FROM_LEFT :
        case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_LEFT :
        case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_LEFT :
            nFlyDirection++;
        case ::com::sun::star::presentation::AnimationEffect_WAVYLINE_FROM_BOTTOM :
        case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_BOTTOM :
        case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_BOTTOM :
            nFlyDirection++;
        case ::com::sun::star::presentation::AnimationEffect_WAVYLINE_FROM_RIGHT :
        case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_RIGHT :
        case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_RIGHT :
            nFlyMethod = 0xa;
        break;

        case ::com::sun::star::presentation::AnimationEffect_FADE_TO_CENTER :
            nFlyDirection++;
        case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_CENTER :
            nFlyMethod = 0xb;
        break;

        case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_BOTTOM :
            nFlyDirection++;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_RIGHT :
            nFlyDirection++;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_TOP :
            nFlyDirection++;
        case ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_LEFT :
            nFlyMethod = 0xc;
        break;

        case ::com::sun::star::presentation::AnimationEffect_VERTICAL_LINES :
        case ::com::sun::star::presentation::AnimationEffect_VERTICAL_STRIPES :
            nFlyDirection++;
        case ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_LINES :
        case ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_STRIPES :
            nFlyMethod = 8;
        break;

        case ::com::sun::star::presentation::AnimationEffect_CLOCKWISE :
            nFlyDirection++;
        case ::com::sun::star::presentation::AnimationEffect_COUNTERCLOCKWISE :
            nFlyMethod = 3;
        break;

        case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_UPPERLEFT :
        case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_UPPERLEFT :
            nFlyDirection++;
        case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_UPPERRIGHT :
        case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_UPPERRIGHT :
            nFlyDirection++;
        case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_LOWERLEFT :
        case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_LOWERLEFT :
            nFlyDirection++;
        case ::com::sun::star::presentation::AnimationEffect_LASER_FROM_LOWERRIGHT :
        case ::com::sun::star::presentation::AnimationEffect_FADE_FROM_LOWERRIGHT :
            nFlyDirection += 4;
            nFlyMethod = 9;
        break;

        case ::com::sun::star::presentation::AnimationEffect_CLOSE_VERTICAL :
            nFlyDirection++;
        case ::com::sun::star::presentation::AnimationEffect_OPEN_HORIZONTAL :
            nFlyDirection++;
        case ::com::sun::star::presentation::AnimationEffect_CLOSE_HORIZONTAL :
            nFlyDirection++;
        case ::com::sun::star::presentation::AnimationEffect_OPEN_VERTICAL :
            nFlyMethod = 0xd;
        break;

        case ::com::sun::star::presentation::AnimationEffect_PATH :
            nFlyDirection = 0x1c;
            nFlyMethod = 0xc;
        break;

        case ::com::sun::star::presentation::AnimationEffect_SPIRALIN_LEFT :
        case ::com::sun::star::presentation::AnimationEffect_SPIRALOUT_LEFT :
            nFlyDirection++;
        case ::com::sun::star::presentation::AnimationEffect_SPIRALIN_RIGHT :
        case ::com::sun::star::presentation::AnimationEffect_SPIRALOUT_RIGHT :
            nFlyMethod = 0x3;
        break;

        case ::com::sun::star::presentation::AnimationEffect_DISSOLVE :
            nFlyMethod = 5;
        break;

        case ::com::sun::star::presentation::AnimationEffect_NONE :
        case ::com::sun::star::presentation::AnimationEffect_APPEAR :
        break;

        case ::com::sun::star::presentation::AnimationEffect_HIDE :
        case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_LEFT :
        case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_TOP :
        case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_RIGHT :
        case ::com::sun::star::presentation::AnimationEffect_MOVE_TO_BOTTOM :
        case ::com::sun::star::presentation::AnimationEffect_RANDOM :
            nFlyMethod = 1;
        break;
    }
    if ( mnDiaMode >= 1 )
        nFlags |= 4;
    if ( eTe != ::com::sun::star::presentation::AnimationEffect_NONE )
        nBuildType = 2;
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "SoundOn" ) ) ) )
    {
        sal_Bool bBool;
        mAny >>= bBool;
        if ( bBool )
        {
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Sound" ) ) ) )
            {
                String aString( *(::rtl::OUString*)mAny.getValue() );
                if ( aString.Len() )
                {
                    nSoundRef = maSoundCollection.GetId( aString ) + 1;
                    nFlags |= 0x10;
                }
            }
        }
    }
    sal_Bool bDimHide = FALSE;
    sal_Bool bDimPrevious = FALSE;
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "DimHide" ) ) ) )
    mAny >>= bDimHide;
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "DimPrevious" ) ) ) )
        mAny >>= bDimPrevious;
    if ( bDimPrevious )
        nAfterEffect |= 1;
    if ( bDimHide )
        nAfterEffect |= 2;
    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "DimColor" ) ) ) )
        nDimColor = mp_EscherEx->GetColor( *((sal_uInt32*)mAny.getValue()) ) | 0xfe000000;

    rSt << (sal_uInt32)( ( EPP_AnimationInfo << 16 ) | 0xf )  << (sal_uInt32)36
        << (sal_uInt32)( ( EPP_AnimationInfoAtom << 16 ) | 1 )<< (sal_uInt32)28
        << nDimColor << nFlags << nSoundRef << nDelayTime
        << nOrder                                   // order of build ( 1.. )
        << nSlideCount << nBuildType << nFlyMethod << nFlyDirection
        << nAfterEffect << nSubEffect << nOleVerb
        << (sal_uInt16)0;                               // PadWord
}

//  -----------------------------------------------------------------------

void PPTWriter::ImplWriteClickAction( SvStream& rSt, ::com::sun::star::presentation::ClickAction eCa )
{
    sal_uInt32 nSoundRef = 0;   // a reference to a sound in the sound collection, or NULL.
    sal_uInt32 nHyperLinkID = 0;// a persistent unique identifier to an external hyperlink object (only valid when action == HyperlinkAction).
    sal_uInt8   nAction = 0;     // Action See Action Table
    sal_uInt8   nOleVerb = 0;    // OleVerb Only valid when action == OLEAction. OLE verb to use, 0 = first verb, 1 = second verb, etc.
    sal_uInt8   nJump = 0;       // Jump See Jump Table
    sal_uInt8   nFlags = 0;      // Bit 1: Animated. If 1, then button is animated
                            // Bit 2: Stop sound. If 1, then stop current sound when button is pressed.
                            // Bit 3: CustomShowReturn. If 1, and this is a jump to custom show, then return to this slide after custom show.
    sal_uInt8   nHyperLinkType = 0;// HyperlinkType a value from the LinkTo enum, such as LT_URL (only valid when action == HyperlinkAction).

    String  aFile;

    /*
        Action Table:       Action Value
        NoAction            0
        MacroAction         1
        RunProgramAction    2
        JumpAction          3
        HyperlinkAction     4
        OLEAction           5
        MediaAction         6
        CustomShowAction    7

        Jump Table:     Jump Value
        NoJump          0
        NextSlide,      1
        PreviousSlide,  2
        FirstSlide,     3
        LastSlide,      4
        LastSlideViewed 5
        EndShow         6
    */

    switch( eCa )
    {
        case ::com::sun::star::presentation::ClickAction_STOPPRESENTATION :
            nJump += 2;
        case ::com::sun::star::presentation::ClickAction_LASTPAGE :
            nJump++;
        case ::com::sun::star::presentation::ClickAction_FIRSTPAGE :
            nJump++;
        case ::com::sun::star::presentation::ClickAction_PREVPAGE :
            nJump++;
        case ::com::sun::star::presentation::ClickAction_NEXTPAGE :
        {
            nJump++;
            nAction = 3;
        }
        break;
        case ::com::sun::star::presentation::ClickAction_SOUND :
        {
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Bookmark" ) ) ) )
            {
                String aString( *(::rtl::OUString*)mAny.getValue() );
                nSoundRef = maSoundCollection.GetId( aString ) + 1;
            }
        }
        break;
        case ::com::sun::star::presentation::ClickAction_PROGRAM :
        {
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Bookmark" ) ) ) )
            {
                INetURLObject aUrl( *(::rtl::OUString*)mAny.getValue() );
                if ( INET_PROT_FILE == aUrl.GetProtocol() )
                {
                    aFile = aUrl.PathToFileName();
                    nAction = 2;
                }
            }
        }
        break;

        case ::com::sun::star::presentation::ClickAction_BOOKMARK :
        {
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Bookmark" ) ) ) )
            {
                String  aBookmark( *(::rtl::OUString*)mAny.getValue() );
                sal_uInt32 nIndex = 0;
                for ( String* pStr = (String*)maSlideNameList.First(); pStr; pStr = (String*)maSlideNameList.Next(), nIndex++ )
                {
                    if ( *pStr == aBookmark )
                    {
                        // Bookmark ist ein link zu einer Dokumentseite
                        nAction = 4;
                        nHyperLinkID = ++mnExEmbed;                 //( maHyperlink.Count() + 1 ) << 1;
                        nHyperLinkType = 7;
                        String  aHyperString = UniString::CreateFromInt32( 256 + nIndex );
                        aHyperString.Append( String( RTL_CONSTASCII_USTRINGPARAM( "," ) ) );
                        aHyperString.Append( String::CreateFromInt32( nIndex + 1 ) );
                        aHyperString.Append( String( RTL_CONSTASCII_USTRINGPARAM( ",Slide " ) ) );
                        aHyperString.Append( String::CreateFromInt32( nIndex + 1 ) );
                        maHyperlink.Insert( new EPPTHyperlink( aHyperString, 1 | ( nIndex << 8 ) | ( 1 << 31 ) ), LIST_APPEND );

                        *mpExEmbed  << (sal_uInt16)0xf
                                    << (sal_uInt16)EPP_ExHyperlink
                                    << (sal_uInt32)12
                                    << (sal_uInt16)0
                                    << (sal_uInt16)EPP_ExHyperlinkAtom
                                    << (sal_uInt32)4
                                    << nHyperLinkID;
                        break;
                    }
                }
            }
        }
        break;

        case ::com::sun::star::presentation::ClickAction_DOCUMENT :
        case ::com::sun::star::presentation::ClickAction_INVISIBLE :
        case ::com::sun::star::presentation::ClickAction_VERB :
        case ::com::sun::star::presentation::ClickAction_VANISH :
        case ::com::sun::star::presentation::ClickAction_MACRO :
        default :
        break;
    }

    sal_uInt32 nContainerSize = 24;
    if ( nAction == 2 )
        nContainerSize += ( aFile.Len() * 2 ) + 8;
    rSt << (sal_uInt32)( ( EPP_InteractiveInfo << 16 ) | 0xf ) << (sal_uInt32)nContainerSize
        << (sal_uInt32)( EPP_InteractiveInfoAtom << 16 ) << (sal_uInt32)16
        << nSoundRef
        << nHyperLinkID
        << nAction
        << nOleVerb
        << nJump
        << nFlags
        << (sal_uInt32)nHyperLinkType;

    if ( nAction == 2 )     // run program Action
    {
        rSt << (sal_uInt32)( ( EPP_CString << 16 ) | 0x20 ) << (sal_uInt32)( aFile.Len() * 2 );
        for ( sal_Int32 i = 0; i < aFile.Len(); i++ )
            rSt << aFile.GetChar( i );
    }

    rSt << (sal_uInt32)( ( EPP_InteractiveInfo << 16 ) | 0x1f ) << (sal_uInt32)24   // Mouse Over Action
        << (sal_uInt32)( EPP_InteractiveInfo << 16 ) << (sal_uInt32)16;
    for ( int i = 0; i < 4; i++, rSt << (sal_uInt32)0 );
}

//  -----------------------------------------------------------------------

sal_Bool PPTWriter::ImplIsAutoShape ( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rXShape,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropertySet,
                                            sal_Bool bIsGroup, sal_Int32 nAngle, sal_uInt32& nShapeType, sal_uInt32& nReplace, List& rAdjustmentList,
                                                Rectangle& rPolyBoundRect )
{
    sal_Bool    bIsAutoShape = FALSE;
    TRY
    {
        sal_uInt32  i;
        sal_uInt32  nSequenceCount = 0;
        sal_uInt32* pPtr = NULL;

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > aXIndexAccess;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > aXShape( rXShape );
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > aXPropSet( rXPropertySet );
        ::com::sun::star::uno::Sequence< sal_uInt32 > aSequence;

        if ( !bIsGroup )
            nReplace = 1;
        else
        {
            aXIndexAccess = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >
                            ( rXShape, ::com::sun::star::uno::UNO_QUERY );

            if ( !aXIndexAccess.is() )
                return FALSE;
            nReplace = aXIndexAccess->getCount();
            if ( !nReplace )
                return FALSE;
        }

        sal_uInt32 nChecksum = 0;
        sal_Int32 aVal[ 3 ];
        double fAngle;
        if ( nAngle )
            fAngle = F_PI18000 * ( 36000 - ( nAngle % 36000 ) );

        ::com::sun::star::awt::Point aPos( rXShape->getPosition() );
        ::com::sun::star::awt::Size  aSize( rXShape->getSize() );
        ::com::sun::star::awt::Point aCenter( aPos.X + ( aSize.Width >> 1 ), aPos.Y + ( aSize.Height >> 1 ) );

        for ( i = 0; i < nReplace; i++ )
        {
            sal_Bool    bBezier;
            ByteString  aType;

            if ( bIsGroup )
            {
                ::com::sun::star::uno::Any aAny( aXIndexAccess->getByIndex( i ) );
                if (!( aAny >>= aXShape ) )
                    return FALSE;
                aType = ByteString( String( aXShape->getShapeType() ), RTL_TEXTENCODING_UTF8 );
                aAny = ::com::sun::star::uno::Any( aXShape->queryInterface( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >*) 0 ) ));
                if (!( aAny >>= aXPropSet ) )
                    return FALSE;
            }
            else
                aType = ByteString( String( aXShape->getShapeType() ), RTL_TEXTENCODING_UTF8 );

            sal_Bool bPossibleAutoShape =
                    ( aType == "com.sun.star.drawing.PolyPolygonShape" )
                        || ( aType == "com.sun.star.drawing.PolyLineShape" )
                                || ( aType == "com.sun.star.drawing.OpenBezierShape" )
                                    || ( aType == "com.sun.star.drawing.OpenFreeHandShape" )
                                        || ( aType == "com.sun.star.drawing.PolyLinePathShape" )
                                            || ( aType == "com.sun.star.drawing.ClosedBezierShape" )
                                                || ( aType == "com.sun.star.drawing.ClosedFreeHandShape" )
                                                    || ( aType == "com.sun.star.drawing.PolyPolygonPathShape" );

            if ( !bPossibleAutoShape )
                return FALSE;

            bBezier = ( aType != "com.sun.star.drawing.PolyPolygonShape" )
                        && ( aType != "com.sun.star.drawing.PolyLineShape" );

            if ( !i )
            {
                ::com::sun::star::uno::Any aAny( aXPropSet->getPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "AutoShapeAdjustment" ) ) ) );
                if( !( aAny >>= aSequence ) )
                    return FALSE;
                nSequenceCount = aSequence.getLength();
                if ( nSequenceCount < 3 )
                    return FALSE;
                pPtr = aSequence.getArray();
                if ( pPtr[ nSequenceCount - 1 ] != 0x80001234 )             // this is a magic number, perhaps a ms autoshape is possible
                    return FALSE;
                if ( (sal_uInt16)pPtr[ nSequenceCount - 2 ] != nReplace )   // this is the original number of polygons the autoshape
                    return FALSE;                                           // was created to by the AutoShapeImport
                nShapeType = pPtr[ nSequenceCount - 2 ] >> 16;              // now we are setting the destination ms shapetype
            }

            ::com::sun::star::uno::Any aAny;
            if ( bBezier )
                aAny = ::com::sun::star::uno::Any( aXPropSet->getPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "PolyPolygonBezier" ) ) ) );
            else
                aAny = ::com::sun::star::uno::Any( aXPropSet->getPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "PolyPolygon" ) ) ) );

            if ( aAny.getValue() )
            {
                sal_Int32 nOuterSequenceCount, nInnerSequenceCount;
                ::com::sun::star::drawing::PointSequence*   pOuterSequence = NULL;
                ::com::sun::star::drawing::FlagSequence*    pOuterFlags = NULL;
                if ( bBezier )
                {
                    ::com::sun::star::drawing::PolyPolygonBezierCoords* pSourcePolyPolygon =
                        ( ::com::sun::star::drawing::PolyPolygonBezierCoords* )aAny.getValue();
                    nOuterSequenceCount = pSourcePolyPolygon->Coordinates.getLength();
                    pOuterSequence = pSourcePolyPolygon->Coordinates.getArray();
                    pOuterFlags = pSourcePolyPolygon->Flags.getArray();
                }
                else
                {
                    ::com::sun::star::drawing::PointSequenceSequence* pSourcePolyPolygon =
                        ( ::com::sun::star::drawing::PointSequenceSequence*)aAny.getValue();
                    nOuterSequenceCount = pSourcePolyPolygon->getLength();
                    pOuterSequence = pSourcePolyPolygon->getArray();
                }
                while( nOuterSequenceCount )
                {
                    nOuterSequenceCount--;
                    ::com::sun::star::drawing::PointSequence* pInnerSequence = pOuterSequence + nOuterSequenceCount;
                    ::com::sun::star::drawing::FlagSequence*  pInnerFlags = ( pOuterFlags ) ? pOuterFlags + nOuterSequenceCount : NULL;
                    ::com::sun::star::awt::Point* pArray = pInnerSequence->getArray();
                    ::com::sun::star::drawing::PolygonFlags* pFlags = ( pInnerFlags ) ? pInnerFlags->getArray() : NULL;
                    nInnerSequenceCount = pInnerSequence->getLength();
                    while ( nInnerSequenceCount )
                    {
                        nInnerSequenceCount--;
                        ::com::sun::star::drawing::PolygonFlags ePolyFlags
                            = ( pFlags ) ? pFlags[ nInnerSequenceCount ]
                            : ::com::sun::star::drawing::PolygonFlags_NORMAL;
                        Point aPoint( pArray[ nInnerSequenceCount ].X, pArray[ nInnerSequenceCount ].Y );

                        if ( nAngle )
                        {
                            sal_Int32 nX = aPoint.X() - aCenter.X;
                            sal_Int32 nY = aPoint.Y() - aCenter.Y;
                            Point aRotPoint( (sal_Int32)( cos( fAngle ) * nX + sin( fAngle ) * nY + 0.5 ),
                                                -(sal_Int32)( sin( fAngle ) * nX - cos( fAngle ) * nY + 0.5 ) );
                            Rectangle aRotPointRect( aRotPoint, Size( 1, 1 ) );
                            rPolyBoundRect.Union( aRotPointRect );
                        }
#ifdef __LITTLEENDIAN
                        aVal[ 0 ] = SWAPLONG( aPoint.X() - aPos.X );
                        aVal[ 1 ] = SWAPLONG( aPoint.Y() - aPos.Y );
                        aVal[ 2 ] = SWAPLONG( ePolyFlags );
#else
                        aVal[ 0 ] = aPoint.X() - aPos.X;
                        aVal[ 1 ] = aPoint.Y() - aPos.Y;
                        aVal[ 2 ] = ePolyFlags;
#endif
                        nChecksum = rtl_crc32( nChecksum, &aVal[ 0 ], 12 );
                    }
                }
            }
        }

        bIsAutoShape = pPtr[ nSequenceCount - 3 ] == nChecksum; // testing the checksum
        if ( bIsAutoShape )
        {
            if ( !nAngle )
                rPolyBoundRect = Rectangle( Point( aPos.X, aPos.Y ), Size( aSize.Width, aSize.Height ) );
            else
                rPolyBoundRect.Move( aCenter.X, aCenter.Y );
            rAdjustmentList.Clear();
            for ( i = 0; i < ( nSequenceCount - 3 ); i++ )
                rAdjustmentList.Insert( (void*)pPtr[ i ], LIST_APPEND );
        }
        if ( bIsGroup ) // the groupshape is to be removed too
            nReplace++;
    }

    CATCH_ALL()
    {
        bIsAutoShape = FALSE;
    }
    END_CATCH;

    return bIsAutoShape;
}

//  -----------------------------------------------------------------------

#define ADD_SHAPE( nType, nFlags )                              \
{                                                               \
    sal_uInt32 nId = mp_EscherEx->GetShapeID();                     \
    mp_EscherEx->AddShape( (sal_uInt32)nType, (sal_uInt32)nFlags, nId );    \
    aSolverContainer.AddShape( mXShape, nId );                  \
}

#define SHAPE_TEXT( bFill )                                     \
{                                                               \
    mnTextStyle = EPP_TEXTSTYLE_TEXT;                           \
    mp_EscherEx->OpenContainer( _Escher_SpContainer );          \
    ADD_SHAPE( _Escher_ShpInst_TextBox, 0xa00 );                \
    mp_EscherEx->BeginCount();                                  \
    if ( bFill )                                                \
        ImplWriteFillBundle( TRUE );                            \
    ImplWriteTextBundle();                                      \
}

void PPTWriter::ImplWritePage( SolverContainer& aSolverContainer, PageType ePageType, sal_Bool bMasterPage, int nPageNumber )
{
    sal_uInt32  nInstance, nGroups, nShapes, nShapeCount, nPer, nLastPer, nIndices, nGroupLevel, nOlePictureId;
    sal_uInt16  nEffectCount;
    ::com::sun::star::awt::Point   aTextRefPoint;

    ResetGroupTable( nShapes = mXShapes->getCount() );

    nIndices = nInstance = nLastPer = nShapeCount = nEffectCount = 0;

    sal_Bool bIsTitlePossible = TRUE;           // bei mehr als einem title geht powerpoint in die knie

    sal_uInt32  nOutlinerCount = 0;             // die gliederungsobjekte muessen dem layout entsprechen,
    sal_uInt32  nPrevTextStyle;                 // es darf nicht mehr als zwei geben

    nOlePictureId = 0;

    sal_Bool bAdditionalText = FALSE;

    SvMemoryStream* pClientTextBox = NULL;
    SvMemoryStream* pClientData = NULL;

    const PHLayout& rLayout =
        ( ePageType == NORMAL ) ? pPHLayout[ mnLayout ]
                                : ( ePageType == MASTER ) ? pPHLayout[ 0 ] : pPHLayout[ 20 ];

    while( GetNextGroupEntry() )
    {
        nShapeCount++;

        nPer = ( 5 * nShapeCount ) / nShapes;
        if ( nPer != nLastPer )
        {
            nLastPer = nPer;
            sal_uInt32 nValue = mnPagesWritten * 5 + nPer;
            if ( nValue > mnStatMaxValue )
                nValue = mnStatMaxValue;
            if ( mbStatusIndicator && ( nValue > mnLatestStatValue ) )
            {
                mXStatusIndicator->setValue( nValue );
                mnLatestStatValue = nValue;
            }
        }
        nGroups = GetGroupsClosed();
        for ( sal_uInt32 i = 0; i < nGroups; i++, mp_EscherEx->LeaveGroup() );

        if ( ImplGetShapeByIndex( GetCurrentGroupIndex(), TRUE ) )
        {
            sal_Bool bIsAutoShape = FALSE;
            sal_Bool bGroup = mType == "drawing.Group";
            sal_Bool bOpenBezier   = ( mType == "drawing.OpenBezier" ) || ( mType == "drawing.OpenFreeHand" )
                                            || ( mType == "drawing.PolyLinePath" );
            sal_Bool bClosedBezier = ( mType == "drawing.ClosedBezier" ) || ( mType == "drawing.ClosedFreeHand" )
                                            || ( mType == "drawing.PolyPolygonPath" );
            sal_Bool bPolyPolygon  = mType == "drawing.PolyPolygon";
            sal_Bool bPolyLine = mType == "drawing.PolyLine";


            sal_uInt32  nReplace;
            sal_uInt32  nShapeType;
            List        aAdjustmentList;
            Rectangle   aPolyBoundRect;
            if ( bGroup || bOpenBezier || bClosedBezier || bPolyPolygon || bPolyLine )
                bIsAutoShape = ImplIsAutoShape( mXShape, mXPropSet, bGroup, mnAngle, nShapeType, nReplace, aAdjustmentList, aPolyBoundRect );

            if ( bIsAutoShape )
            {
                if ( bGroup )
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >
                        aXIndexAccess( mXShape, ::com::sun::star::uno::UNO_QUERY );
                    EnterGroup( aXIndexAccess );
                    GetNextGroupEntry();
                    ImplGetShapeByIndex( GetCurrentGroupIndex(), TRUE );
                    SkipCurrentGroup();
                }
            }
            else
            {
                if ( bGroup )
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >
                        aXIndexAccess( mXShape, ::com::sun::star::uno::UNO_QUERY );
                    EnterGroup( aXIndexAccess );
                    mp_EscherEx->EnterGroup( &maRect );
                    continue;
                }
                sal_Bool bIsFontwork = FALSE;
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "IsFontwork" ) ) ) )
                    mAny >>= bIsFontwork;
                if ( bIsFontwork || ( mType == "drawing.Measure" ) || ( mType == "drawing.Caption" ) )
                {
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "BoundRect" ) ) ) )
                    {
                        ::com::sun::star::awt::Rectangle aRect( *(::com::sun::star::awt::Rectangle*)mAny.getValue() );
                        maPosition = ImplMapPoint( ::com::sun::star::awt::Point( aRect.X, aRect.Y ) );
                        maSize = ImplMapSize( ::com::sun::star::awt::Size( aRect.Width, aRect.Height ) );
                        maRect = Rectangle( Point( maPosition.X, maPosition.Y ), Size( maSize.Width, maSize.Height ) );
                    }
                    mType = "drawing.dontknow";
                }
            }

            sal_uInt8 nPlaceHolderAtom = EPP_PLACEHOLDER_NONE;

            mnShadow = 2;
            mnTextSize = 0;
            mnTextStyle = EPP_TEXTSTYLE_NORMAL;

            if ( bIsAutoShape )
            {
                mp_EscherEx->OpenContainer( _Escher_SpContainer );
                ADD_SHAPE( nShapeType, 0xa00 );
                mp_EscherEx->BeginCount();

                ::com::sun::star::awt::Point aP( aPolyBoundRect.Left(), aPolyBoundRect.Top() );
                ::com::sun::star::awt::Size  aS( aPolyBoundRect.GetWidth(), aPolyBoundRect.GetHeight() );
                maPosition = ImplMapPoint( aP );
                maSize = ImplMapSize( aS );
                maRect = Rectangle( Point( maPosition.X, maPosition.Y ), Size( maSize.Width, maSize.Height ) );
                if ( mnAngle < 0 )
                    mnAngle = ( 36000 + mnAngle ) % 36000;
                else
                    mnAngle = ( 36000 - ( mnAngle % 36000 ) );
                if ( ( mnAngle > 4500 && mnAngle <= 13500 ) || ( mnAngle > 22500 && mnAngle <= 31500 ) )
                {
                    double  nWidthHalf = maRect.GetWidth() / 2;
                    double  nHeightHalf = maRect.GetHeight() / 2;

                    Point aTopLeft( maRect.Left() + nWidthHalf - nHeightHalf,
                                        maRect.Top() + nHeightHalf - nWidthHalf );
                    Size    aNewSize( maRect.GetHeight(), maRect.GetWidth() );
                    maRect = Rectangle( aTopLeft, aNewSize );
                }
                mnAngle *= 655;
                mnAngle += 0x8000;
                mnAngle &=~0xffff;                                  // nAngle auf volle Gradzahl runden
                mp_EscherEx->AddOpt( _Escher_Prop_Rotation, mnAngle );
                mnAngle = 0;

                sal_uInt32 nAdjCount;
                for ( nAdjCount = 0; nAdjCount < aAdjustmentList.Count(); nAdjCount++ )
                    mp_EscherEx->AddOpt( _Escher_Prop_adjustValue + nAdjCount, (sal_uInt32)aAdjustmentList.GetObject( nAdjCount ) );

                if ( bPolyLine || bOpenBezier )
                    ImplWriteLineBundle( FALSE );
                else
                    ImplWriteFillBundle( TRUE );
                ImplWriteTextBundle();
            }
            else if ( mType == "drawing.Rectangle" )
            {
                sal_Int32 nRadius = 0;
                mp_EscherEx->OpenContainer( _Escher_SpContainer );
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CornerRadius" ) ) ) )
                {
                    mAny >>= nRadius;
                    nRadius = ImplMapSize( ::com::sun::star::awt::Size( nRadius, 0 ) ).Width;
                }
                if ( nRadius )
                {
                    ADD_SHAPE( _Escher_ShpInst_RoundRectangle, 0xa00 ); // Flags: Connector | HasSpt
                    mp_EscherEx->BeginCount();
                    sal_Int32 nLenght = maRect.GetWidth();
                    if ( nLenght > maRect.GetHeight() )
                        nLenght = maRect.GetHeight();
                    nLenght >>= 1;
                    if ( nRadius >= nLenght )
                        nRadius = 0x2a30;                           // 0x2a30 ist PPTs maximum radius
                    else
                        nRadius = ( 0x2a30 * nRadius ) / nLenght;
                    mp_EscherEx->AddOpt( _Escher_Prop_adjustValue, nRadius );
                }
                else
                {
                    ADD_SHAPE( _Escher_ShpInst_Rectangle, 0xa00 );          // Flags: Connector | HasSpt
                    mp_EscherEx->BeginCount();
                }
                ImplWriteFillBundle( TRUE );
                ImplWriteTextBundle();
            }
            else if ( mType == "drawing.Ellipse" )
            {
                ::com::sun::star::drawing::CircleKind  eCircleKind( ::com::sun::star::drawing::CircleKind_FULL );
                PolyStyle   ePolyKind;
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CircleKind" ) ) ) )
                {
                    mAny >>= eCircleKind;
                    switch ( eCircleKind )
                    {
                        case ::com::sun::star::drawing::CircleKind_SECTION :
                        {
                            ePolyKind = POLY_PIE;
                        }
                        break;
                        case ::com::sun::star::drawing::CircleKind_ARC :
                        {
                            ePolyKind = POLY_ARC;
                        }
                        break;

                        case ::com::sun::star::drawing::CircleKind_CUT :
                        {
                            ePolyKind = POLY_CHORD;
                        }
                        break;

                        default:
                            eCircleKind = ::com::sun::star::drawing::CircleKind_FULL;
                    }
                }
                if ( eCircleKind == ::com::sun::star::drawing::CircleKind_FULL )
                {
                    mp_EscherEx->OpenContainer( _Escher_SpContainer );
                    ADD_SHAPE( _Escher_ShpInst_Ellipse, 0xa00 );            // Flags: Connector | HasSpt
                    mp_EscherEx->BeginCount();
                    ImplWriteFillBundle( TRUE );
                }
                else
                {
                    sal_Int32 nStartAngle, nEndAngle;
                    if ( !ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CircleStartAngle" ) ) ) )
                        continue;
                    nStartAngle = *( (sal_Int32*)mAny.getValue() );
                    if( !ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CircleEndAngle" ) ) ) )
                        continue;
                    nEndAngle = *( (sal_Int32*)mAny.getValue() );

                    maPosition = mXShape->getPosition();
                    maSize = mXShape->getSize();
                    maRect = Rectangle( Point( maPosition.X, maPosition.Y ), Size( maSize.Width, maSize.Height ) );

                    ::com::sun::star::awt::Point aStart, aEnd, aCenter;
                    aStart.X = (sal_Int32)( ( cos( (double)( nStartAngle * F_PI18000 ) ) * 100.0 ) );
                    aStart.Y = - (sal_Int32)( ( sin( (double)( nStartAngle * F_PI18000 ) ) * 100.0 ) );
                    aEnd.X = (sal_Int32)( ( cos( (double)( nEndAngle * F_PI18000 ) ) * 100.0 ) );
                    aEnd.Y = - (sal_Int32)( ( sin( (double)( nEndAngle * F_PI18000 ) ) * 100.0 ) );
                    aCenter.X = maRect.Left() + ( maRect.GetWidth() / 2 );
                    aCenter.Y = maRect.Top() + ( maRect.GetHeight() / 2 );
                    aStart.X += aCenter.X;
                    aStart.Y += aCenter.Y;
                    aEnd.X += aCenter.X;
                    aEnd.Y += aCenter.Y;
                    Polygon aPolygon( maRect, Point( aStart.X, aStart.Y ), Point( aEnd.X, aEnd.Y ), ePolyKind );
                    mp_EscherEx->OpenContainer( _Escher_SpContainer );
                    ADD_SHAPE( _Escher_ShpInst_NotPrimitive, 0xa00 );       // Flags: Connector | HasSpt
                    mp_EscherEx->BeginCount();
                    switch ( ePolyKind )
                    {
                        case POLY_PIE :
                        case POLY_CHORD :
                        {
                            ImplWriteAny( ANY_FLAGS_POLYPOLYGON, FALSE, &aPolygon );
                            ImplWriteFillBundle( TRUE );
                        }
                        break;

                        case POLY_ARC :
                        {
                            ImplWriteAny( ANY_FLAGS_POLYLINE, FALSE, &aPolygon );
                            ImplWriteLineBundle( FALSE );
                        }
                        break;
                    }
                }
                ImplWriteTextBundle();
            }
            else if ( mType == "drawing.Control" )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XControlShape  >
                    aXControlShape( mXShape, ::com::sun::star::uno::UNO_QUERY );
                if ( !aXControlShape.is() )
                    continue;
                ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >
                    aXControlModel( aXControlShape->getControl() );
                if ( !aXControlModel.is() )
                    continue;

                *mpExEmbed  << (sal_uInt32)( 0xf | ( EPP_ExControl << 16 ) )
                            << (sal_uInt32)0;               // Size of this container

                sal_uInt32 nSize, nOldPos = mpExEmbed->Tell();

                sal_uInt32 nPageId = nPageNumber;
                if ( ePageType == MASTER )
                    nPageId |= 0x80000000;
                else
                    nPageId += 0x100;
                *mpExEmbed  << (sal_uInt32)( EPP_ExControlAtom << 16 )
                            << (sal_uInt32)4
                            << nPageId;
                PPTExOleObjEntry* pEntry = new PPTExOleObjEntry( OCX_CONTROL, String(), mpExEmbed->Tell() );
                pEntry->xControlModel = aXControlModel;
                maExOleObj.Insert( pEntry );

                mnExEmbed++;

                *mpExEmbed  << (sal_uInt32)( 1 | ( EPP_ExOleObjAtom << 16 ) )
                            << (sal_uInt32)24
                            << (sal_uInt32)1
                            << (sal_uInt32)2
                            << (sal_uInt32)mnExEmbed
                            << (sal_uInt32)0
                            << (sal_uInt32)4    // index to the persist table
                            << (sal_uInt32)0x0012de00;


                ::com::sun::star::awt::Size     aSize;
                String          aControlName;
                SvStorageRef    xTemp( new SvStorage( new SvMemoryStream(), TRUE ) );
                if ( SvxMSConvertOCXControls::WriteOCXStream( xTemp, aXControlModel, aSize, aControlName ) )
                {
                    String  aUserName( xTemp->GetUserName() );
                    String  aOleIdentifier;
                    if ( aUserName.Len() )
                    {
                        SvStorageStreamRef xCompObj = xTemp->OpenStream(
                            String( RTL_CONSTASCII_USTRINGPARAM( "\1CompObj" ) ),
                                STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYALL );
                        xCompObj->Seek( STREAM_SEEK_TO_END );
                        sal_uInt32  nStreamLen = xCompObj->Tell();
                        xCompObj->Seek( 0 );
                        sal_Int16   nVersion, nByteOrder;
                        sal_Int32   nWinVersion, nVal, nStringLen;
                        *xCompObj   >> nVersion
                                    >> nByteOrder
                                    >> nWinVersion
                                    >> nVal;
                        xCompObj->SeekRel( 16 );    // skipping clsid
                        *xCompObj   >> nStringLen;
                        if ( ( xCompObj->Tell() + nStringLen ) < nStreamLen )
                        {
                            xCompObj->SeekRel( nStringLen );        // now skipping the UserName;
                            *xCompObj >> nStringLen;
                            if ( ( xCompObj->Tell() + nStringLen ) < nStreamLen )
                            {
                                xCompObj->SeekRel( nStringLen );    // now skipping the clipboard formatname
                                *xCompObj   >> nStringLen;
                                if ( ( nStringLen > 1 ) && ( ( xCompObj->Tell() + nStringLen ) < nStreamLen ) )
                                {   // i think that the OleIdentifier will follow
                                    ByteString aTemp;
                                    sal_Char* p = aTemp.AllocBuffer( nStringLen - 1 );
                                    xCompObj->Read( p, nStringLen - 1 );
                                    aOleIdentifier = String( aTemp, gsl_getSystemTextEncoding() );
                                }
                            }
                        }
                    }
                    if ( aControlName.Len() )
                         ImplWriteCString( *mpExEmbed, aControlName, 1 );
                    if ( aOleIdentifier.Len() )
                        ImplWriteCString( *mpExEmbed, aOleIdentifier, 2 );
                    if ( aUserName.Len() )
                        ImplWriteCString( *mpExEmbed, aUserName, 3 );
                }
                nSize = mpExEmbed->Tell() - nOldPos;
                mpExEmbed->Seek( nOldPos - 4 );
                *mpExEmbed << nSize;
                mpExEmbed->Seek( STREAM_SEEK_TO_END );
                nOlePictureId = mnExEmbed;

                mp_EscherEx->OpenContainer( _Escher_SpContainer );
                sal_uInt32 nSpFlags = SHAPEFLAG_HAVESPT | SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_OLESHAPE;
                ADD_SHAPE( _Escher_ShpInst_HostControl, nSpFlags );
                mp_EscherEx->BeginCount();
                if ( ImplGetGraphic( mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "MetaFile" ) ), FALSE, TRUE ) )
                    mp_EscherEx->AddOpt( _Escher_Prop_LockAgainstGrouping, 0x800080 );
                mp_EscherEx->AddOpt( _Escher_Prop_pictureId, mnExEmbed );
                mp_EscherEx->AddOpt( _Escher_Prop_pictureActive, 0x10000 );

                if ( aControlName.Len() )
                {
                    sal_uInt32 i, nBufSize;
                    nBufSize = ( aControlName.Len() + 1 ) << 1;
                    sal_uInt8* pBuf = new sal_uInt8[ nBufSize ];
                    sal_uInt8* pTmp = pBuf;
                    for ( i = 0; i < aControlName.Len(); i++ )
                    {
                        sal_Unicode nUnicode = aControlName.GetChar( i );
                        *pTmp++ = (sal_uInt8)nUnicode;
                        *pTmp++ = (sal_uInt8)( nUnicode >> 8 );
                    }
                    *pTmp++ = 0;
                    *pTmp = 0;
                    mp_EscherEx->AddOpt( _Escher_Prop_wzName, TRUE, nBufSize, pBuf, nBufSize );
                }
           }
            else if ( mType == "drawing.Connector" )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > aShapeA, aShapeB;
                ::com::sun::star::awt::Point aStartPoint, aEndPoint;

                if ( !ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "EdgeKind" ) ) ) )
                    continue;

                ::com::sun::star::drawing::ConnectorType eCt;
                mAny >>= eCt;

                if ( !ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "EdgeStartPoint" ) ) ) )
                    continue;
                aStartPoint = *(::com::sun::star::awt::Point*)mAny.getValue();

                if ( !ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "EdgeEndPoint" ) ) ) )
                    continue;
                aEndPoint = *(::com::sun::star::awt::Point*)mAny.getValue();

                sal_uInt32 nAdjustValue1, nAdjustValue2, nAdjustValue3;
                nAdjustValue1 = nAdjustValue2 = nAdjustValue3 = 0x2a30;

                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "EdgeStartConnection" ) ) ) )
                    mAny >>= aShapeA;
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "EdgeEndConnection" ) ) ) )
                    mAny >>= aShapeB;
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "EdgeLine1Delta" ) ) ) )
                {
                }
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "EdgeLine2Delta" ) ) ) )
                {
                }
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "EdgeLine3Delta" ) ) ) )
                {
                }
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "EdgeNode1HorzDist" ) ) ) )
                {
                }
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "EdgeNode1VertDist" ) ) ) )
                {
                }
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "EdgeNode2HorzDist" ) ) ) )
                {
                }
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "EdgeNode2VertDist" ) ) ) )
                {
                }
                aSolverContainer.AddConnector( mXShape, aStartPoint, aShapeA, aEndPoint, aShapeB );
                ::com::sun::star::awt::Point aBeg( ImplMapPoint( aStartPoint ) );
                ::com::sun::star::awt::Point aEnd( ImplMapPoint( aEndPoint ) );
                maRect = Rectangle( Point( aBeg.X, aBeg.Y ), Point( aEnd.X, aEnd.Y ) );
                ImplWriteAny( ANY_FLAGS_LINE, FALSE );
                mp_EscherEx->OpenContainer( _Escher_SpContainer );
                sal_uInt32 nFlags = 0xa00;                                  // Flags: Connector | HasSpt
                if ( maRect.Top() > maRect.Bottom() )
                    nFlags |= 0x80;                                         // Flags: VertMirror
                if ( maRect.Left() > maRect.Right() )
                    nFlags |= 0x40;                                         // Flags: HorzMirror

                Rectangle aJustifiedRect( maRect );
                aJustifiedRect.Justify();

                switch ( eCt )
                {
                    case ::com::sun::star::drawing::ConnectorType_CURVE :
                    {
                        ADD_SHAPE( _Escher_ShpInst_CurvedConnector3, nFlags );
                        mp_EscherEx->BeginCount();
                        mp_EscherEx->AddOpt( _Escher_Prop_cxstyle, _Escher_cxstyleCurved );
                        mp_EscherEx->AddOpt( _Escher_Prop_adjustValue, nAdjustValue1 );
                        mp_EscherEx->AddOpt( _Escher_Prop_adjust2Value, -nAdjustValue2 );
                    }
                    break;

                    case ::com::sun::star::drawing::ConnectorType_STANDARD :// Connector 2->5
                    {
                        ADD_SHAPE( _Escher_ShpInst_BentConnector3, nFlags );
                        mp_EscherEx->BeginCount();
                        mp_EscherEx->AddOpt( _Escher_Prop_cxstyle, _Escher_cxstyleBent );
                    }
                    break;

                    default:
                    case ::com::sun::star::drawing::ConnectorType_LINE :
                    case ::com::sun::star::drawing::ConnectorType_LINES :   // Connector 2->5
                    {
                        nFlags |= 0x100;
                        ADD_SHAPE( _Escher_ShpInst_StraightConnector1, nFlags );
                        mp_EscherEx->BeginCount();
                        mp_EscherEx->AddOpt( _Escher_Prop_cxstyle, _Escher_cxstyleStraight );
                    }
                    break;
                }
                ImplWriteLineBundle( FALSE );
                mnAngle = 0;
            }
            else if ( mType == "drawing.Measure" )
            {
/*
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureKind" ) ) ) )
                {
                    mp_EscherEx->EnterGroup( &maRect );
                    mp_EscherEx->OpenContainer( _Escher_SpContainer );
                    ImplWriteAny( ANY_FLAGS_LINE, FALSE );
                    sal_uInt32 nFlags = 0xa00;                              // Flags: Connector | HasSpt
                    if ( maRect.Top() > maRect.Bottom() )
                        nFlags |= 0x80;                                     // Flags: VertMirror
                    if ( maRect.Left() > maRect.Right() )
                        nFlags |= 0x40;                                     // Flags: HorzMirror

                    ADD_SHAPE( _Escher_ShpInst_Line, nFlags );
                    mp_EscherEx->BeginCount();
                    mp_EscherEx->AddOpt( _Escher_Prop_shapePath, _Escher_ShapeComplex );
                    ImplWriteLineBundle( FALSE );
                    mp_EscherEx->EndCount( _Escher_OPT, 3 );
                    maRect.Justify();
                    mp_EscherEx->AddClientAnchor( maRect );
                    mp_EscherEx->CloseContainer();          // _Escher_SpContainer

                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureTextHorizontalPosition" ) ) ) )
                    {
                    }
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureTextVerticalPosition" ) ) ) )
                    {
                    }
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureLineDistance" ) ) ) )
                    {
                    }
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureHelpLineOverhang" ) ) ) )
                    {
                    }
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureHelpLineDistance" ) ) ) )
                    {
                    }
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureHelpLine1Length" ) ) ) )
                    {
                    }
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureHelpLine2Length" ) ) ) )
                    {
                    }
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureBelowReferenceEdge" ) ) ) )
                    {
                    }
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureTextRotate90" ) ) ) )
                    {
                    }
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureTextUpsideDown" ) ) ) )
                    {
                    }
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureOverhang" ) ) ) )
                    {
                    }
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureUnit" ) ) ) )
                    {
                    }
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureScale" ) ) ) )
                    {
                    }
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureShowUnit" ) ) ) )
                    {
                    }
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureFormatString" ) ) ) )
                    {
                    }
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureTextAutoAngle" ) ) ) )
                    {
                    }
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureTextAutoAngleView" ) ) ) )
                    {
                    }
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureTextIsFixedAngle" ) ) ) )
                    {
                    }
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "MeasureTextFixedAngle" ) ) ) )
                    {
                    }
                    mp_EscherEx->LeaveGroup();
                }
*/
                continue;
            }
            else if ( mType == "drawing.Line" )
            {
                ImplWriteAny( ANY_FLAGS_LINE, FALSE );
                if ( ImplGetText() )
                {
                    aTextRefPoint = ::com::sun::star::awt::Point( maRect.Left(), maRect.Top() );
                    mnTextSize = 0;
                    bAdditionalText = TRUE;
                    mp_EscherEx->EnterGroup( &maRect );
                }
                mp_EscherEx->OpenContainer( _Escher_SpContainer );
                sal_uInt32 nFlags = 0xa00;                                  // Flags: Connector | HasSpt
                if ( maRect.Top() > maRect.Bottom() )
                    nFlags |= 0x80;                                         // Flags: VertMirror
                if ( maRect.Left() > maRect.Right() )
                    nFlags |= 0x40;                                         // Flags: HorzMirror

                ADD_SHAPE( _Escher_ShpInst_Line, nFlags );
                mp_EscherEx->BeginCount();
                mp_EscherEx->AddOpt( _Escher_Prop_shapePath, _Escher_ShapeComplex );
                ImplWriteLineBundle( FALSE );
                mnAngle = 0;
            }
            else if ( bPolyPolygon )
            {
                if ( ImplGetText() )
                {
                    mp_EscherEx->EnterGroup( NULL );
                    nGroupLevel = mp_EscherEx->GetGroupLevel();
                    bAdditionalText = TRUE;
                    mnTextSize = 0;
                }
                mp_EscherEx->OpenContainer( _Escher_SpContainer );
                ADD_SHAPE( _Escher_ShpInst_NotPrimitive, 0xa00 );           // Flags: Connector | HasSpt
                mp_EscherEx->BeginCount();
                ImplWriteAny( ANY_FLAGS_POLYPOLYGON, FALSE );
                ImplWriteFillBundle( TRUE );
                mnAngle = 0;
            }
            else if ( bPolyLine )
            {
                if ( ImplGetText() )
                {
                    mp_EscherEx->EnterGroup( NULL );
                    nGroupLevel = mp_EscherEx->GetGroupLevel();
                    bAdditionalText = TRUE;
                    mnTextSize = 0;
                }
                mp_EscherEx->OpenContainer( _Escher_SpContainer );
                ADD_SHAPE( _Escher_ShpInst_NotPrimitive, 0xa00 );           // Flags: Connector | HasSpt
                mp_EscherEx->BeginCount();
                ImplWriteAny( ANY_FLAGS_POLYLINE, FALSE );
                ImplWriteLineBundle( FALSE );
                mnAngle = 0;
            }
            else if ( bOpenBezier )
            {
                if ( ImplGetText() )
                {
                    mp_EscherEx->EnterGroup( NULL );
                    nGroupLevel = mp_EscherEx->GetGroupLevel();
                    bAdditionalText = TRUE;
                    mnTextSize = 0;
                }
                mp_EscherEx->OpenContainer( _Escher_SpContainer );
                ADD_SHAPE( _Escher_ShpInst_NotPrimitive, 0xa00 );           // Flags: Connector | HasSpt
                mp_EscherEx->BeginCount();
                ImplWriteAny( ANY_FLAGS_POLYLINE, TRUE );
                ImplWriteLineBundle( FALSE );
                mnAngle = 0;
            }
            else if ( bClosedBezier )
            {
                if ( ImplGetText() )
                {
                    mp_EscherEx->EnterGroup( NULL );
                    nGroupLevel = mp_EscherEx->GetGroupLevel();
                    bAdditionalText = TRUE;
                    mnTextSize = 0;
                }
                mp_EscherEx->OpenContainer( _Escher_SpContainer );
                ADD_SHAPE( _Escher_ShpInst_NotPrimitive, 0xa00 );           // Flags: Connector | HasSpt
                mp_EscherEx->BeginCount();
                ImplWriteAny( ANY_FLAGS_POLYPOLYGON, TRUE );
                ImplWriteFillBundle( TRUE );
                mnAngle = 0;
            }
            else if ( ( mType == "drawing.GraphicObject" ) || ( mType == "presentation.GraphicObject" ) )
            {
                mp_EscherEx->OpenContainer( _Escher_SpContainer );

                // ein GraphicObject kann auch ein ClickMe Element sein
                if ( mbEmptyPresObj && ( ePageType == NORMAL ) )
                {
                    nPlaceHolderAtom = 19;      // EPP_PLACEHOLDER_OBJECT;
                    if ( mnLayout == 8 )
                        nPlaceHolderAtom = 21;  // EPP_PLACEHOLDER_OBJECT_TABLE
                    else if ( ( mnLayout == 2 ) || ( mnLayout == 4 ) || ( mnLayout == 7 ) )
                        nPlaceHolderAtom = 20;  // EPP_PLACEHOLDER_GRAPH;
                    else if ( ( mnLayout == 9 ) || ( mnLayout == 6 ) )
                        nPlaceHolderAtom = 22;  // EPP_PLACEHOLDER_CLIPART
                    ADD_SHAPE( _Escher_ShpInst_Rectangle, 0x220 );          // Flags: HaveAnchor | HaveMaster
                    mp_EscherEx->BeginCount();
                    mp_EscherEx->AddOpt( _Escher_Prop_lTxid, mnTxId += 0x60 );
                    mp_EscherEx->AddOpt( _Escher_Prop_fNoFillHitTest, 0x10001 );
                    mp_EscherEx->AddOpt( _Escher_Prop_fNoLineDrawDash, 0x10001 );
                    mp_EscherEx->AddOpt( _Escher_Prop_hspMaster, mnShapeMasterBody );
                }
                else
                {
                    mXText = ::com::sun::star::uno::Reference<
                        ::com::sun::star::text::XSimpleText >
                            ( mXShape, ::com::sun::star::uno::UNO_QUERY );

                    if ( mXText.is() )
                        mnTextSize = mXText->getString().len();

                    if ( mnTextSize )                                       // graphic object oder Flachenfuellung
                    {
                        ADD_SHAPE( _Escher_ShpInst_Rectangle, 0xa00 );      // Flags: Connector | HasSpt
                        mp_EscherEx->BeginCount();
                        if ( ImplGetGraphic( mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "GraphicURL" ) ), TRUE ) )
                        {
                            mp_EscherEx->AddOpt( _Escher_Prop_WrapText, _Escher_WrapNone );
                            mp_EscherEx->AddOpt( _Escher_Prop_AnchorText, _Escher_AnchorMiddle );
                            mp_EscherEx->AddOpt( _Escher_Prop_fNoFillHitTest, 0x140014 );
                            mp_EscherEx->AddOpt( _Escher_Prop_fillBackColor, 0x8000000 );
                            mp_EscherEx->AddOpt( _Escher_Prop_fNoLineDrawDash, 0x80000 );
                            ImplWriteTextBundle();
                        }
                    }
                    else
                    {
                        ADD_SHAPE( _Escher_ShpInst_PictureFrame, 0xa00 );
                        mp_EscherEx->BeginCount();
                        if ( ImplGetGraphic( mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "GraphicURL" ) ), FALSE ) )
                            mp_EscherEx->AddOpt( _Escher_Prop_LockAgainstGrouping, 0x800080 );
                    }
                    sal_uInt32 nPicFlags = 0;
                    ::com::sun::star::drawing::ColorMode eColorMode( ::com::sun::star::drawing::ColorMode_STANDARD );
                    sal_Int16 nLuminance = 0;
                    sal_Int16 nContrast = 0;
                    sal_Int16 nRed = 0;
                    sal_Int16 nGreen = 0;
                    sal_Int16 nBlue = 0;
                    double fGamma = 1.0;
                    sal_Int16 nTransparency = 0;

                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "GraphicColorMode" ) ) ) )
                        mAny >>= eColorMode;
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "AdjustLuminance" ) ) ) )
                        mAny >>= nLuminance;
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "AdjustContrast" ) ) ) )
                        mAny >>= nContrast;
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "AdjustRed" ) ) ) )
                        mAny >>= nRed;
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "AdjustGreen" ) ) ) )
                        mAny >>= nGreen;
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "AdjustBlue" ) ) ) )
                        mAny >>= nBlue;
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Gamma" ) ) ) )
                        mAny >>= fGamma;
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Transparency" ) ) ) )
                        mAny >>= nTransparency;

                    if ( ( eColorMode == ::com::sun::star::drawing::ColorMode_WATERMARK )
                            && ( nLuminance || nContrast ) )
                    {
                        eColorMode = ::com::sun::star::drawing::ColorMode_STANDARD;
                        nLuminance += 5000;
                        if ( nLuminance > 10000 )
                            nLuminance = 10000;
                        nContrast -= 7000;
                        if ( nContrast < 10000 )
                            nContrast = 10000;
                    }
                    nContrast *= 327;
                    nLuminance *= 327;
                    switch ( eColorMode )
                    {
                        case ::com::sun::star::drawing::ColorMode_GREYS :
                            nPicFlags |= 0x40004;
                        break;
                        case ::com::sun::star::drawing::ColorMode_MONO :
                            nPicFlags |= 0x60006;
                        break;
                        case ::com::sun::star::drawing::ColorMode_WATERMARK :
                        {
                            nContrast = 0x4ccd;
                            nLuminance = 0x599a;
                        }
                        break;
                    }
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "GraphicCrop" ) ) ) )
                    {
                        ::com::sun::star::text::GraphicCrop aGraphCrop;
                        if ( mAny >>= aGraphCrop )
                        {
                            ::com::sun::star::awt::Size aCropSize( mXShape->getSize() );
                            if ( aGraphCrop.Left )
                            {
                                sal_uInt32 nLeft = ( aGraphCrop.Left * 65536 ) / aCropSize.Width;
                                mp_EscherEx->AddOpt( _Escher_Prop_cropFromLeft, nLeft );
                            }
                            if ( aGraphCrop.Top )
                            {
                                sal_uInt32 nTop = ( aGraphCrop.Top * 65536 ) / aCropSize.Height;
                                mp_EscherEx->AddOpt( _Escher_Prop_cropFromTop, nTop );
                            }
                            if ( aGraphCrop.Right )
                            {
                                sal_uInt32 nRight = ( aGraphCrop.Right * 65536 ) / aCropSize.Width;
                                mp_EscherEx->AddOpt( _Escher_Prop_cropFromRight, nRight );
                            }
                            if ( aGraphCrop.Bottom )
                            {
                                sal_uInt32 nBottom = ( aGraphCrop.Bottom * 65536 ) / aCropSize.Height;
                                mp_EscherEx->AddOpt( _Escher_Prop_cropFromBottom, nBottom );
                            }
                        }
                    }
                    if ( nContrast )
                        mp_EscherEx->AddOpt( _Escher_Prop_pictureContrast, nContrast );
                    if ( nLuminance )
                        mp_EscherEx->AddOpt( _Escher_Prop_pictureBrightness, nLuminance );
                    if ( nPicFlags )
                        mp_EscherEx->AddOpt( _Escher_Prop_pictureActive, nPicFlags );
                }
            }
            else if ( ( mType == "drawing.Text" ) || ( mType == "presentation.Subtitle" ) || ( mType == "presentation.Notes" ) )
            {
                if ( ( ePageType == NOTICE ) && mbPresObj )
                {
                    if ( bMasterPage )
                        nPlaceHolderAtom = EPP_PLACEHOLDER_MASTERDATE;  // be sure to correct all defines, they are not in the right order,
                    else                                                // cause this is not a masterdate,´it is realy a masternotesbodyimage
                        nPlaceHolderAtom = EPP_PLACEHOLDER_NOTESBODY;
                }
                SHAPE_TEXT( TRUE );
            }
            else if ( mType == "presentation.TitleText" )
            {
                if ( mbPresObj )
                {
                    if ( ( ePageType == NOTICE ) && mbEmptyPresObj )
                    {
                        mp_EscherEx->OpenContainer( _Escher_SpContainer );
                        nPlaceHolderAtom = EPP_PLACEHOLDER_MASTERNOTESBODYIMAGE;
                        ADD_SHAPE( _Escher_ShpInst_Rectangle, 0x200 );
                        mp_EscherEx->BeginCount();
                        ImplWriteLineBundle( FALSE );
                        mp_EscherEx->AddOpt( _Escher_Prop_fNoFillHitTest, 0x10001 );
                    }
                    else if ( rLayout.bTitlePossible && bIsTitlePossible )
                    {
                        bIsTitlePossible = FALSE;

                        if ( ePageType == MASTER )
                            continue;

                        mp_EscherEx->OpenContainer( _Escher_SpContainer );
                        mnTextStyle = EPP_TEXTSTYLE_TITLE;
                        nPlaceHolderAtom = EPP_PLACEHOLDER_TITLE;
                        ADD_SHAPE( _Escher_ShpInst_Rectangle, 0x220 );          // Flags: HaveAnchor | HaveMaster
                        mp_EscherEx->BeginCount();
                        mp_EscherEx->AddOpt( _Escher_Prop_hspMaster, mnShapeMasterTitle );
                        ImplWriteFillBundle( TRUE );
                        ImplWriteTextBundle( TRUE );
                        if ( mbEmptyPresObj )
                        {
                            sal_uInt32 nNoLineDrawDash = 0;
                            mp_EscherEx->GetOpt( _Escher_Prop_fNoLineDrawDash, nNoLineDrawDash );
                            nNoLineDrawDash |= 0x10001;
                            mp_EscherEx->AddOpt( _Escher_Prop_fNoLineDrawDash, nNoLineDrawDash );
                        }
                    }
                    else
                        mbPresObj = FALSE;
                }
                if ( !mbPresObj )
                {
                    mType = "drawing.Text";
                    SHAPE_TEXT( TRUE );
                }
            }
            else if ( mType == "presentation.Outliner" )
            {
                if ( mbPresObj )
                {
                    nOutlinerCount++;
                    if ( rLayout.bOutlinerPossible && ( nOutlinerCount == 1 ) ||
                        ( ( rLayout.bSecOutlinerPossible && ( nOutlinerCount == 2 ) )
                            && ( nPrevTextStyle == EPP_TEXTSTYLE_BODY ) ) )
                    {
                        if ( ePageType == MASTER )
                        {
                            nPrevTextStyle = EPP_TEXTSTYLE_TITLE;
                            continue;
                        }
                        sal_uInt32 nOutlCount = 0;
                        for ( sal_uInt32 nI = 0; nI < 8; nI++ )
                        {
                            sal_uInt8 nC = pPHLayout[ mnLayout ].nPlaceHolder[ nI ];
                            if ( !nC )
                                break;
                            if ( nC == 0xe )
                                nOutlCount++;
                        }
                        if ( nOutlCount >= nOutlinerCount )
                        {
                            mnTextStyle = EPP_TEXTSTYLE_BODY;
                            nPlaceHolderAtom = EPP_PLACEHOLDER_BODY;
                            mp_EscherEx->OpenContainer( _Escher_SpContainer );
                            ADD_SHAPE( _Escher_ShpInst_Rectangle, 0x220 );          // Flags: HaveAnchor | HaveMaster
                            mp_EscherEx->BeginCount();
                            mp_EscherEx->AddOpt( _Escher_Prop_hspMaster, mnShapeMasterBody );
                            ImplWriteFillBundle( TRUE );
                            ImplWriteTextBundle( TRUE );
                            if ( mbEmptyPresObj )
                            {
                                sal_uInt32 nNoLineDrawDash = 0;
                                mp_EscherEx->GetOpt( _Escher_Prop_fNoLineDrawDash, nNoLineDrawDash );
                                nNoLineDrawDash |= 0x10001;
                                mp_EscherEx->AddOpt( _Escher_Prop_fNoLineDrawDash, nNoLineDrawDash );
                            }
                        }
                        else mbPresObj = FALSE;
                    }
                    else mbPresObj = FALSE;
                }
                if ( !mbPresObj )
                {
                    mType = "drawing.Text";
                    SHAPE_TEXT( TRUE );
                }
            }
            else if ( ( mType == "drawing.Page" ) || ( mType == "presentation.Page" ) )
            {
                nPlaceHolderAtom = EPP_PLACEHOLDER_GENERICTEXTOBJECT;
                mp_EscherEx->OpenContainer( _Escher_SpContainer );
                ADD_SHAPE( _Escher_ShpInst_Rectangle, 0xa00 );
                mp_EscherEx->BeginCount();
                mp_EscherEx->AddOpt( _Escher_Prop_LockAgainstGrouping, 0x40004 );
                mp_EscherEx->AddOpt( _Escher_Prop_fFillOK, 0x100001 );
                mp_EscherEx->AddOpt( _Escher_Prop_fNoFillHitTest, 0x110011 );
                mp_EscherEx->AddOpt( _Escher_Prop_fNoLineDrawDash, 0x90008 );
                mp_EscherEx->AddOpt( _Escher_Prop_fshadowObscured, 0x10001 );
            }
            else if ( mType == "drawing.Frame" )
            {
                continue;
            }
            else if ( ( mType == "drawing.OLE2" ) || ( mType == "presentation.OLE2" )
                        || ( mType == "presentation.Chart" ) || ( mType == "presentation.Table" )
                            || ( mType == "presentation.OrgChart" ) )
            {
                mp_EscherEx->OpenContainer( _Escher_SpContainer );
                if ( mbEmptyPresObj && ( ePageType == NORMAL ) )
                {
                    nPlaceHolderAtom = 19;      // EPP_PLACEHOLDER_OBJECT;
                    if ( mnLayout == 8 )
                        nPlaceHolderAtom = 21;  // EPP_PLACEHOLDER_OBJECT_TABLE
                    else if ( ( mnLayout == 2 ) || ( mnLayout == 4 ) || ( mnLayout == 7 ) )
                        nPlaceHolderAtom = 20;  // EPP_PLACEHOLDER_GRAPH;
                    else if ( ( mnLayout == 9 ) || ( mnLayout == 6 ) )
                        nPlaceHolderAtom = 22;  // EPP_PLACEHOLDER_CLIPART
                    ADD_SHAPE( _Escher_ShpInst_Rectangle, 0x220 );              // Flags: HaveAnchor | HaveMaster
                    mp_EscherEx->BeginCount();
                    mp_EscherEx->AddOpt( _Escher_Prop_lTxid, mnTxId += 0x60 );
                    mp_EscherEx->AddOpt( _Escher_Prop_fNoFillHitTest, 0x10001 );
                    mp_EscherEx->AddOpt( _Escher_Prop_fNoLineDrawDash, 0x10001 );
                    mp_EscherEx->AddOpt( _Escher_Prop_hspMaster, mnShapeMasterBody );
                }
                else
                {
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "LinkDisplayName" ) ) ) )
                    {
                        String  aString( *(::rtl::OUString*)mAny.getValue() );
                        if ( aString.Len() )
                        {
                            SvStorageRef xSrcStor = mXSource->OpenStorage( aString, STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYALL );
                            if ( xSrcStor.Is() )
                            {
                                SvStorageStreamRef xSrcTst = xSrcStor->OpenStream(
                                    String( RTL_CONSTASCII_USTRINGPARAM( "\1Ole" ) ),
                                        STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYALL );
                                sal_uInt8 aTestA[ 10 ];
                                if ( sizeof( aTestA ) == xSrcTst->Read( aTestA, sizeof( aTestA ) ) )
                                {
/*
                                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "CLSID" ) ) ) )
                                    {
                                        String aCLSID( *( ::rtl::OUString*)mAny.getValue() );
                                    }
*/
                                    *mpExEmbed  << (sal_uInt32)( 0xf | ( EPP_ExEmbed << 16 ) )
                                                << (sal_uInt32)0;               // Size of this container

                                    sal_uInt32 nSize, nOldPos = mpExEmbed->Tell();

                                    *mpExEmbed  << (sal_uInt32)( EPP_ExEmbedAtom << 16 )
                                                << (sal_uInt32)8
                                                << (sal_uInt32)0    // follow colorscheme : 0->do not follow
                                                                    //                      1->follow collorscheme
                                                                    //                      2->follow text and background scheme
                                                << (sal_uInt8)1     // (bool)set if embedded server can not be locked
                                                << (sal_uInt8)0     // (bool)do not need to send dimension
                                                << (sal_uInt8)0     // (bool)is object a world table
                                                << (sal_uInt8)0;    // pad byte

                                    maExOleObj.Insert( new PPTExOleObjEntry( NORMAL_OLE_OBJECT, aString, mpExEmbed->Tell() ) );

                                    mnExEmbed++;

                                    *mpExEmbed  << (sal_uInt32)( 1 | ( EPP_ExOleObjAtom << 16 ) )
                                                << (sal_uInt32)24
                                                << (sal_uInt32)1
                                                << (sal_uInt32)0
                                                << (sal_uInt32)mnExEmbed    // index to the persist table
                                                << (sal_uInt32)0            // subtype
                                                << (sal_uInt32)0
                                                << (sal_uInt32)0x0012b600;

//                                  ImplWriteCString( *mpExEmbed, "Photo Editor Photo", 1 );
//                                  ImplWriteCString( *mpExEmbed, "MSPhotoEd.3", 2 );
//                                  ImplWriteCString( *mpExEmbed, "Microsoft Photo Editor 3.0 Photo", 3 );

                                    nSize = mpExEmbed->Tell() - nOldPos;
                                    mpExEmbed->Seek( nOldPos - 4 );
                                    *mpExEmbed << nSize;
                                    mpExEmbed->Seek( STREAM_SEEK_TO_END );
                                    nOlePictureId = mnExEmbed;
                                }
                            }
                        }
                    }
                    sal_uInt32 nSpFlags = 0xa00;
                    if ( nOlePictureId )
                        nSpFlags |= 0x10;
                    ADD_SHAPE( _Escher_ShpInst_PictureFrame, nSpFlags );
                    mp_EscherEx->BeginCount();
                    if ( ImplGetGraphic( mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "MetaFile" ) ), FALSE, TRUE ) )
                        mp_EscherEx->AddOpt( _Escher_Prop_LockAgainstGrouping, 0x800080 );
                    if ( nOlePictureId )
                        mp_EscherEx->AddOpt( _Escher_Prop_pictureId, nOlePictureId );
                }
            }
            else if ( ( (sal_Char)'3' == mType.GetChar( 8 ) ) && ( (char)'D' == mType.GetChar( 9 ) ) )  // drawing.3D
            {
                // SceneObject, CubeObject, SphereObject, LatheObject, ExtrudeObject, PolygonObject
                if ( !ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Bitmap" ) ) ) )
                    continue;

                mp_EscherEx->OpenContainer( _Escher_SpContainer );
                ADD_SHAPE( _Escher_ShpInst_PictureFrame, 0xa00 );
                mp_EscherEx->BeginCount();

                if ( ImplGetGraphic( mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "Bitmap" ) ), FALSE ) )
                    mp_EscherEx->AddOpt( _Escher_Prop_LockAgainstGrouping, 0x800080 );
            }
            else if ( mType == "drawing.dontknow" )
            {
                mnAngle = 0;
                mp_EscherEx->OpenContainer( _Escher_SpContainer );
                ADD_SHAPE( _Escher_ShpInst_PictureFrame, 0xa00 );
                mp_EscherEx->BeginCount();
                if ( ImplGetGraphic( mXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "MetaFile" ) ), FALSE ) )
                    mp_EscherEx->AddOpt( _Escher_Prop_LockAgainstGrouping, 0x800080 );
            }
            else
            {
                continue;
            }
            if ( mnShadow ) // shadow wird nur ausgegeben, wenn es mindestens einen LinesStyle oder FillStyle gibt
            {
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Shadow" ) ) ) )
                {
                    sal_Bool bBool;
                    mAny >>= bBool;
                    if ( bBool )
                    {
                        mp_EscherEx->AddOpt( _Escher_Prop_fshadowObscured, 0x20002 );
                        if ( ImplGetPropertyValue(  String( RTL_CONSTASCII_USTRINGPARAM( "ShadowColor" ) ) ) )
                            mp_EscherEx->AddOpt( _Escher_Prop_shadowColor, mp_EscherEx->GetColor( *((sal_uInt32*)mAny.getValue()) ) );
                        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ShadowXDistance" ) ) ) )
                            mp_EscherEx->AddOpt( _Escher_Prop_shadowOffsetX, *((sal_Int32*)mAny.getValue()) * 360 );
                        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ShadowYDistance" ) ) ) )
                            mp_EscherEx->AddOpt( _Escher_Prop_shadowOffsetY, *((sal_Int32*)mAny.getValue()) * 360 );
                        if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "ShadowTransparence" ) ) ) )
                            mp_EscherEx->AddOpt( _Escher_Prop_shadowOpacity,  ( ( ( 100 - (*((sal_uInt16*)mAny.getValue()) ) ) << 16 ) / 100 ) );
                    }
                }
            }
            maRect.Justify();

            if ( mnAngle )
            {
                if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "RotationPointX" ) ) ) )
                {
                    ::com::sun::star::awt::Point aRefPoint( *( (sal_Int32*)mAny.getValue() ), 0 );
                    if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "RotationPointY" ) ) ) )
                    {
                        aRefPoint.Y = *( (sal_Int32*)mAny.getValue() );
                        ImplFlipBoundingBox( ImplMapPoint( aRefPoint ) );
                    }
                }
            }
            mp_EscherEx->EndCount( _Escher_OPT, 3 );
            mp_EscherEx->AddClientAnchor( maRect );

            ::com::sun::star::presentation::AnimationEffect eAe( ::com::sun::star::presentation::AnimationEffect_NONE );
            ::com::sun::star::presentation::AnimationEffect eTe( ::com::sun::star::presentation::AnimationEffect_NONE );
            ::com::sun::star::presentation::ClickAction eCa = ::com::sun::star::presentation::ClickAction_NONE;
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "Effect" ) ) ) )
                mAny >>= eAe;
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "TextEffect" ) ) ) )
                mAny >>= eTe;
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "PresentationOrder" ) ) ) )
                nEffectCount = *(sal_uInt16*)mAny.getValue();

            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "OnClick" ) ) ) )
                mAny >>= eCa;

            sal_Int32 nPlacementID = -1;

            sal_Bool bIsSound = FALSE;
            if ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "SoundOn" ) ) ) )
                mAny >>= bIsSound;
            sal_Bool bEffect = ( ( eAe != ::com::sun::star::presentation::AnimationEffect_NONE )
                              || ( eTe != ::com::sun::star::presentation::AnimationEffect_NONE ) || bIsSound );
            sal_Bool bClientData = ( bEffect || ( eCa != ::com::sun::star::presentation::ClickAction_NONE ) ||
                                        nPlaceHolderAtom || nOlePictureId );
            if ( bClientData )
            {
                if ( nPlaceHolderAtom )
                {
                    if ( ( mnTextStyle == EPP_TEXTSTYLE_TITLE ) || ( mnTextStyle == EPP_TEXTSTYLE_BODY ) )
                        nPlacementID = nIndices++;
                    else
                    {
                        switch ( nPlaceHolderAtom )
                        {
                            default :
                            {
                                if ( nPlaceHolderAtom < 19 )
                                    break;
                            }
                            case EPP_PLACEHOLDER_NOTESBODY :
                            case EPP_PLACEHOLDER_MASTERDATE :
                            case EPP_PLACEHOLDER_GENERICTEXTOBJECT :
                            case EPP_PLACEHOLDER_MASTERNOTESBODYIMAGE :
                                nPlacementID = nIndices++;
                        }
                    }
                    if ( !pClientData )
                        pClientData = new SvMemoryStream( 0x200, 0x200 );

                    *pClientData << (sal_uInt32)( EPP_OEPlaceholderAtom << 16 ) << (sal_uInt32)8
                                 << nPlacementID                // PlacementID
                                 << (sal_uInt8)nPlaceHolderAtom     // PlaceHolderID
                                 << (sal_uInt8)0                        // Size of PlaceHolder ( 0 = FULL, 1 = HALF, 2 = QUARTER )
                                 << (sal_uInt16)0;                  // padword
                }
                if ( nOlePictureId )
                {
                    if ( !pClientData )
                        pClientData = new SvMemoryStream( 0x200, 0x200 );

                    *pClientData << (sal_uInt32)( EPP_ExObjRefAtom << 16 ) << (sal_uInt32)4
                                 << nOlePictureId;
                    nOlePictureId = 0;
                }
                if ( bEffect )
                {
                    if ( !pClientData )
                        pClientData = new SvMemoryStream( 0x200, 0x200 );

                    ImplWriteObjectEffect( *pClientData, eAe, eTe, ++nEffectCount );
                }

                if ( eCa != ::com::sun::star::presentation::ClickAction_NONE )
                {
                    if ( !pClientData )
                        pClientData = new SvMemoryStream( 0x200, 0x200 );
                    ImplWriteClickAction( *pClientData, eCa );
                }
            }
            if ( mnTextSize )
            {
                if ( ( mnTextStyle == EPP_TEXTSTYLE_TITLE ) || ( mnTextStyle == EPP_TEXTSTYLE_BODY ) )
                {
                    if ( !pClientTextBox )
                        pClientTextBox = new SvMemoryStream( 0x200, 0x200 );

                    *pClientTextBox << (sal_uInt32)( EPP_OutlineTextRefAtom << 16 ) << (sal_uInt32)4
                                    << nPlacementID;

                    if ( mbEmptyPresObj == FALSE )
                    {
                        if ( ( ePageType == NORMAL ) && ( bMasterPage == FALSE ) )
                        {   // try to allocate the textruleratom
                            TextRuleEntry*  pTextRule = (TextRuleEntry*)maTextRuleList.GetCurObject();
                            while ( pTextRule )
                            {
                                int nRulePage = pTextRule->nPageNumber;
                                if ( nRulePage > nPageNumber )
                                    break;
                                else if ( nRulePage < nPageNumber )
                                    pTextRule = (TextRuleEntry*)maTextRuleList.Next();
                                else
                                {
                                    SvMemoryStream* pOut = pTextRule->pOut;
                                    if ( pOut )
                                    {
                                        pClientTextBox->Write( pOut->GetData(), pOut->Tell() );
                                        delete pOut, pTextRule->pOut = NULL;
                                    }
                                    maTextRuleList.Next();
                                    break;
                                }
                            }
                        }
                    }
                }
                else
                {
                    int nInstance;
                    if ( ( nPlaceHolderAtom == EPP_PLACEHOLDER_MASTERDATE ) || ( nPlaceHolderAtom == EPP_PLACEHOLDER_NOTESBODY ) )
                        nInstance = 2;
                    else
                        nInstance = EPP_TEXTTYPE_Other;     // Text in a Shape

                    if ( !pClientTextBox )
                        pClientTextBox = new SvMemoryStream( 0x200, 0x200 );

                    SvMemoryStream  aExtBu( 0x200, 0x200 );
                    ImplWriteTextStyleAtom( *pClientTextBox, nInstance, 0, NULL, aExtBu );
                    if ( aExtBu.Tell() )
                    {
                        if ( !pClientData )
                            pClientData = new SvMemoryStream( 0x200, 0x200 );
                        ImplProgTagContainer( pClientData, &aExtBu );
                    }
                }
            }
            else if ( nPlaceHolderAtom >= 19 )
            {
                if ( !pClientTextBox )
                    pClientTextBox = new SvMemoryStream( 12 );

                *pClientTextBox << (sal_uInt32)( EPP_TextHeaderAtom << 16 ) << (sal_uInt32)4
                                << (sal_uInt32)7;
            }

            if ( pClientData )
            {
                *mpStrm << (sal_uInt32)( ( _Escher_ClientData << 16 ) | 0xf )
                        << (sal_uInt32)pClientData->Tell();

                mpStrm->Write( pClientData->GetData(), pClientData->Tell() );
                delete pClientData, pClientData = NULL;
            }
            if ( pClientTextBox )
            {
                *mpStrm << (sal_uInt32)( ( _Escher_ClientTextbox << 16 ) | 0xf )
                        << (sal_uInt32)pClientTextBox->Tell();

                mpStrm->Write( pClientTextBox->GetData(), pClientTextBox->Tell() );
                delete pClientTextBox, pClientTextBox = NULL;
            }
            mp_EscherEx->CloseContainer();      // _Escher_SpContainer
        }
        nPrevTextStyle = mnTextStyle;

        if ( bAdditionalText )
        {
            bAdditionalText = FALSE;

            mnAngle = ( ImplGetPropertyValue( String( RTL_CONSTASCII_USTRINGPARAM( "RotateAngle" ) )) )
                            ? *( (sal_Int32*)mAny.getValue() ) : 0;

            if ( mType == "drawing.Line" )
            {
                double fDist = hypot( maRect.GetWidth(), maRect.GetHeight() );
                maRect = Rectangle( Point( aTextRefPoint.X, aTextRefPoint.Y ),
                                        Point( aTextRefPoint.X + fDist, aTextRefPoint.Y - 1 ) );
                SHAPE_TEXT( FALSE );
                mp_EscherEx->AddOpt( _Escher_Prop_fNoLineDrawDash, 0x90000 );
                mp_EscherEx->AddOpt( _Escher_Prop_fNoFillHitTest, 0x100000 );
                mp_EscherEx->AddOpt( _Escher_Prop_FitTextToShape, 0x60006 );        // Size Shape To Fit Text
                if ( mnAngle < 0 )
                    mnAngle = ( 36000 + mnAngle ) % 36000;
                if ( mnAngle )
                    ImplFlipBoundingBox( ::com::sun::star::awt::Point( maRect.Left(), maRect.Top() ) );
            }
            else
            {
                SHAPE_TEXT( FALSE );
                mp_EscherEx->AddOpt( _Escher_Prop_fNoLineDrawDash, 0x90000 );
                mp_EscherEx->AddOpt( _Escher_Prop_fNoFillHitTest, 0x100000 );

                if ( mnAngle < 0 )
                    mnAngle = ( 36000 + mnAngle ) % 36000;
                else
                    mnAngle = ( 36000 - ( mnAngle % 36000 ) );

                mnAngle *= 655;
                mnAngle += 0x8000;
                mnAngle &=~0xffff;  // nAngle auf volle Gradzahl runden
                mp_EscherEx->AddOpt( _Escher_Prop_Rotation, mnAngle );
                mp_EscherEx->SetGroupSnapRect( nGroupLevel, maRect );
                mp_EscherEx->SetGroupLogicRect( nGroupLevel, maRect );
            }
            mp_EscherEx->EndCount( _Escher_OPT, 3 );
            mp_EscherEx->AddClientAnchor( maRect );

            if ( !pClientTextBox )
                pClientTextBox = new SvMemoryStream( 0x200, 0x200 );

            SvMemoryStream  aExtBu( 0x200, 0x200 );
            ImplWriteTextStyleAtom( *pClientTextBox, EPP_TEXTTYPE_Other, 0, NULL, aExtBu );

            *mpStrm << (sal_uInt32)( ( _Escher_ClientTextbox << 16 ) | 0xf )
                    << (sal_uInt32)pClientTextBox->Tell();

            mpStrm->Write( pClientTextBox->GetData(), pClientTextBox->Tell() );
            delete pClientTextBox, pClientTextBox = NULL;

            mp_EscherEx->CloseContainer();  // _Escher_SpContainer
            mp_EscherEx->LeaveGroup();
        }
    }
    ClearGroupTable();                              // gruppierungen wegschreiben, sofern noch irgendwelche offen sind, was eigendlich nicht sein sollte
    nGroups = GetGroupsClosed();
    for ( sal_uInt32 i = 0; i < nGroups; i++, mp_EscherEx->LeaveGroup() );
    mnPagesWritten++;
}

// ---------------------------------------------------------------------------------------------

sal_Bool PPTWriter::ImplGetGraphic( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSet, const String& rSource, sal_Bool bFillBitmap, sal_Bool bOle )
{
    sal_Bool        bRetValue = FALSE;
    sal_Bool        bMirrored = FALSE;
    sal_Bool        bBitmapTile = FALSE;
    GraphicAttr*    pGraphicAttr = NULL;
    GraphicObject   aGraphicObject;
    String          aGraphicUrl;
    ByteString      aUniqueId;

    ::com::sun::star::uno::Any aAny;

    if ( GetPropertyValue( aAny, rXPropSet, rSource ) )
    {
        ::com::sun::star::awt::Size aSize( mXShape->getSize() );
        Point aEmptyPoint;
        Rectangle aRect( aEmptyPoint, Size( aSize.Width, aSize.Height ) );

        if ( rSource == String( RTL_CONSTASCII_USTRINGPARAM( "MetaFile" ) ) )
        {
            ::com::sun::star::uno::Sequence<sal_uInt8> aSeq = *(::com::sun::star::uno::Sequence<sal_uInt8>*)aAny.getValue();
            const sal_uInt8*    pAry = aSeq.getArray();
            sal_uInt32          nAryLen = aSeq.getLength();

            if ( pAry && nAryLen )
            {
                Graphic         aGraphic;
                SvMemoryStream  aTemp( (void*)pAry, nAryLen, STREAM_READ );
                sal_uInt32 nErrCode = GraphicConverter::Import( aTemp, aGraphic, CVT_WMF );
                if ( nErrCode == ERRCODE_NONE )
                {
                    aGraphicObject = aGraphic;
                    aUniqueId = aGraphicObject.GetUniqueID();
                }
            }
        }
        else if ( rSource == String( RTL_CONSTASCII_USTRINGPARAM( "Bitmap" ) ) )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >xBitmap;
            if ( ::cppu::extractInterface( xBitmap, aAny ) )
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap > xBitmap;
                if ( aAny >>= xBitmap )
                {
                    BitmapEx    aBitmapEx( VCLUnoHelper::GetBitmap( xBitmap ) );
                    Graphic     aGraphic( aBitmapEx );
                    aGraphicObject = aGraphic;
                    aUniqueId = aGraphicObject.GetUniqueID();
                }
            }
        }
        else if ( rSource == String( RTL_CONSTASCII_USTRINGPARAM( "FillBitmapURL" ) ) )
        {
            aGraphicUrl = *(::rtl::OUString*)aAny.getValue();
        }
        else if ( rSource == String( RTL_CONSTASCII_USTRINGPARAM( "GraphicURL" ) ) )
        {
            aGraphicUrl = *(::rtl::OUString*)aAny.getValue();
        }
        if ( aGraphicUrl.Len() )
        {
            xub_StrLen nIndex = aGraphicUrl.Search( (sal_Unicode)':', 0 );
            if ( nIndex != STRING_NOTFOUND )
            {
                nIndex++;
                if ( aGraphicUrl.Len() > nIndex  )
                    aUniqueId = ByteString( aGraphicUrl, nIndex, aGraphicUrl.Len() - nIndex, RTL_TEXTENCODING_UTF8 );
            }
        }
        if ( aUniqueId.Len() )
        {
            if ( !mpPicStrm )
                mpPicStrm = mrStg->OpenStream( String( RTL_CONSTASCII_USTRINGPARAM( "Pictures" ) ) );
            if ( mpPicStrm )
            {
                if ( GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "IsMirrored" ) ) ) )
                    aAny >>= bMirrored;
                if ( rSource == String( RTL_CONSTASCII_USTRINGPARAM( "FillBitmap" ) ) )
                {
                    if ( GetPropertyValue( aAny, rXPropSet, String( RTL_CONSTASCII_USTRINGPARAM( "FillBitmapTile" ) ) ) )
                        aAny >>= bBitmapTile;
                }
                if ( !bFillBitmap )
                {
                    if ( bMirrored || mnAngle )
                    {
                        mnAngle = ( mnAngle + 5 ) / 10;
                        pGraphicAttr = new GraphicAttr;
                        pGraphicAttr->SetRotation( mnAngle );
                        if ( bMirrored )
                            pGraphicAttr->SetMirrorFlags( BMP_MIRROR_HORZ );
                        if ( mnAngle )  // ppoint does not rotate graphics !
                        {
                            Polygon aPoly( maRect );
                            aPoly.Rotate( maRect.TopLeft(), mnAngle );
                            maRect = aPoly.GetBoundRect();
                            mnAngle = 0;
                        }
                    }
                }
                if ( ( rSource == String( RTL_CONSTASCII_USTRINGPARAM( "FillBitmap" ) ) ) && bBitmapTile )
                    mp_EscherEx->AddOpt( _Escher_Prop_fillType, _Escher_FillTexture );
                else
                    mp_EscherEx->AddOpt( _Escher_Prop_fillType, _Escher_FillPicture );

                mp_EscherEx->AddOpt( ( bFillBitmap )
                    ? _Escher_Prop_fillBlip
                    : _Escher_Prop_pib, mp_EscherEx->AddGraphic( *mpPicStrm, aUniqueId, aRect, pGraphicAttr ), TRUE );
                bRetValue = TRUE;
            }
        }
        delete pGraphicAttr;
    }
    return bRetValue;
}

//  -----------------------------------------------------------------------

::com::sun::star::awt::Point PPTWriter::ImplMapPoint( const ::com::sun::star::awt::Point& rPoint )
{
    Point aRet( OutputDevice::LogicToLogic( Point( rPoint.X, rPoint.Y ), maMapModeSrc, maMapModeDest ) );
    return ::com::sun::star::awt::Point( aRet.X(), aRet.Y() );
}

//  -----------------------------------------------------------------------

::com::sun::star::awt::Size PPTWriter::ImplMapSize( const ::com::sun::star::awt::Size& rSize )
{
    Size aRetSize( OutputDevice::LogicToLogic( Size( rSize.Width, rSize.Height ), maMapModeSrc, maMapModeDest ) );

    if ( !aRetSize.Width() )
        aRetSize.Width()++;
    if ( !aRetSize.Height() )
        aRetSize.Height()++;
    return ::com::sun::star::awt::Size( aRetSize.Width(), aRetSize.Height() );
}

