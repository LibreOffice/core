/*************************************************************************
 *
 *  $RCSfile: GraphicPropertyItemConverter.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: bm $ $Date: 2003-11-04 12:37:17 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "GraphicPropertyItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ItemPropertyMap.hxx"

// #ifndef _SFXINTITEM_HXX
// #include <svtools/intitem.hxx>
// #endif
#ifndef _SVX_XFLCLIT_HXX
#include <svx/xflclit.hxx>
#endif
#ifndef _SVX_XLNCLIT_HXX
#include <svx/xlnclit.hxx>
#endif
#ifndef _SVX_XFLBMTIT_HXX
#include <svx/xflbmtit.hxx>
#endif
#ifndef _SVX_XFLBSTIT_HXX
#include <svx/xflbstit.hxx>
#endif
#ifndef _SVX_XBTMPIT_HXX
#include <svx/xbtmpit.hxx>
#endif
#ifndef _SVX_XFLFTRIT_HXX
#include <svx/xflftrit.hxx>
#endif
#ifndef _SVX_XLNDSIT_HXX
#include <svx/xlndsit.hxx>
#endif
#ifndef _SVX_XFLHTIT_HXX
#include <svx/xflhtit.hxx>
#endif
#ifndef _SVX_XFLGRIT_HXX
#include <svx/xflgrit.hxx>
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX
#define ITEMID_UNDERLINE EE_CHAR_UNDERLINE
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#define ITEMID_FONT EE_CHAR_FONTINFO
#include <svx/fontitem.hxx>
#endif
// for SfxBoolItem
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
// for XFillGradientStepCountItem
#ifndef _SVX_XGRSCIT_HXX
#include <svx/xgrscit.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_FILLBITMAP_HPP_
#include <drafts/com/sun/star/chart2/FillBitmap.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_TRANSPARENCYSTYLE_HPP_
#include <drafts/com/sun/star/chart2/TransparencyStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

namespace
{
::comphelper::ItemPropertyMapType & lcl_GetDataPointFilledPropertyMap()
{
    static ::comphelper::ItemPropertyMapType aDataPointPropertyFilledMap(
        ::comphelper::MakeItemPropertyMap
        ( XATTR_FILLSTYLE,                C2U( "FillStyle" ))
        ( XATTR_FILLCOLOR,                C2U( "Color" ))
        ( XATTR_FILLTRANSPARENCE,         C2U( "Transparency" ))
//         ( XATTR_FILLGRADIENT,             C2U( "Gradient" ))
//         ( XATTR_FILLHATCH,                C2U( "Hatch" ))
        ( XATTR_LINECOLOR,                C2U( "BorderColor" ))
        ( XATTR_LINESTYLE,                C2U( "BorderStyle" ))
        ( XATTR_LINEWIDTH,                C2U( "BorderWidth" ))
//         ( XATTR_LINEDASH,                 C2U( "BorderDash" ))
        ( XATTR_LINETRANSPARENCE,         C2U( "BorderTransparency" ))
        );

    return aDataPointPropertyFilledMap;
}
::comphelper::ItemPropertyMapType & lcl_GetDataPointLinePropertyMap()
{
    static ::comphelper::ItemPropertyMapType aDataPointPropertyLineMap(
        ::comphelper::MakeItemPropertyMap
        ( XATTR_LINECOLOR,                C2U( "Color" ))
        ( XATTR_LINETRANSPARENCE,         C2U( "Transparency" ))
        ( XATTR_LINESTYLE,                C2U( "LineStyle" ))
        ( XATTR_LINEWIDTH,                C2U( "LineWidth" ))
//         ( XATTR_LINEDASH,                 C2U( "LineDash" ))
        );

    return aDataPointPropertyLineMap;
}
::comphelper::ItemPropertyMapType & lcl_GetLinePropertyMap()
{
    static ::comphelper::ItemPropertyMapType aLinePropertyMap(
        ::comphelper::MakeItemPropertyMap
        ( XATTR_LINESTYLE,                C2U( "LineStyle" ))
        ( XATTR_LINEWIDTH,                C2U( "LineWidth" ))
//         ( XATTR_LINEDASH,                 C2U( "LineDash" ))
        ( XATTR_LINECOLOR,                C2U( "LineColor" ))
        ( XATTR_LINETRANSPARENCE,         C2U( "LineTransparence" ))
        ( XATTR_LINEJOINT,                C2U( "LineJoint" ))
        );

    return aLinePropertyMap;
}
::comphelper::ItemPropertyMapType & lcl_GetFillPropertyMap()
{
    static ::comphelper::ItemPropertyMapType aFillPropertyMap(
        ::comphelper::MakeItemPropertyMap
        ( XATTR_FILLSTYLE,                C2U( "FillStyle" ))
        ( XATTR_FILLCOLOR,                C2U( "FillColor" ))
        ( XATTR_FILLTRANSPARENCE,         C2U( "FillTransparence" ))
//         ( XATTR_FILLGRADIENT,             C2U( "FillGradient" ))
//         ( XATTR_FILLHATCH,                C2U( "FillHatch" ))
        );

    return aFillPropertyMap;
}
} // anonymous namespace

// ========================================

namespace chart
{
namespace wrapper
{

GraphicPropertyItemConverter::GraphicPropertyItemConverter(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    eGraphicObjectType eObjectType /* = FILL_PROPERTIES */ ) :
        ItemConverter( rPropertySet, rItemPool ),
        m_eGraphicObjectType( eObjectType ),
        m_rDrawModel( rDrawModel )
{}

GraphicPropertyItemConverter::~GraphicPropertyItemConverter()
{}

const USHORT * GraphicPropertyItemConverter::GetWhichPairs() const
{
    const USHORT * pResult = NULL;

    switch( m_eGraphicObjectType )
    {
        case LINE_DATA_POINT:
        case FILLED_DATA_POINT:
            pResult = nRowWhichPairs; break;
        case LINE_PROPERTIES:
            pResult = nLinePropertyWhichPairs; break;
        case FILL_PROPERTIES:
            pResult = nFillPropertyWhichPairs; break;
        case LINE_AND_FILL_PROPERTIES:
            pResult = nLineAndFillPropertyWhichPairs; break;
    }

    return pResult;
}

bool GraphicPropertyItemConverter::GetItemPropertyName( USHORT nWhichId, ::rtl::OUString & rOutName ) const
{
    ::comphelper::ItemPropertyMapType::const_iterator aEndIt;
    ::comphelper::ItemPropertyMapType::const_iterator aIt;

    switch( m_eGraphicObjectType )
    {
        case LINE_DATA_POINT:
            aEndIt = lcl_GetDataPointLinePropertyMap().end();
            aIt = lcl_GetDataPointLinePropertyMap().find( nWhichId );
            break;
        case FILLED_DATA_POINT:
            aEndIt = lcl_GetDataPointFilledPropertyMap().end();
            aIt = lcl_GetDataPointFilledPropertyMap().find( nWhichId );
            break;
        case LINE_PROPERTIES:
            aEndIt = lcl_GetLinePropertyMap().end();
            aIt = lcl_GetLinePropertyMap().find( nWhichId );
            break;

        case FILL_PROPERTIES:
            aEndIt = lcl_GetFillPropertyMap().end();
            aIt = lcl_GetFillPropertyMap().find( nWhichId );
            break;

        case LINE_AND_FILL_PROPERTIES:
            // line
            aEndIt = lcl_GetLinePropertyMap().end();
            aIt = lcl_GetLinePropertyMap().find( nWhichId );

            // not found => try fill
            if( aIt == aEndIt )
            {
                aEndIt = lcl_GetFillPropertyMap().end();
                aIt = lcl_GetFillPropertyMap().find( nWhichId );
            }
            break;
    }

    if( aIt == aEndIt )
        return false;

    rOutName =(*aIt).second;
    return true;
}

void GraphicPropertyItemConverter::FillSpecialItem(
    USHORT nWhichId, SfxItemSet & rOutItemSet ) const
    throw( uno::Exception )
{
//     if( m_eGraphicObjectType == LINE_DATA_POINT ||
//         m_eGraphicObjectType == LINE_PROPERTIES )
//         return;

    switch( nWhichId )
    {
        // bitmap property
        case XATTR_FILLBITMAP:
        case XATTR_FILLBMP_TILE:
        case XATTR_FILLBMP_POS:
        case XATTR_FILLBMP_SIZEX:
        case XATTR_FILLBMP_SIZEY:
        case XATTR_FILLBMP_SIZELOG:
        case XATTR_FILLBMP_TILEOFFSETX:
        case XATTR_FILLBMP_TILEOFFSETY:
        case XATTR_FILLBMP_STRETCH:
        case XATTR_FILLBMP_POSOFFSETX:
        case XATTR_FILLBMP_POSOFFSETY:
            // avoid assertions while bitmap stuff is not working
//             try
//             {
//                 FillBitmapItem( nWhichId, rOutItemSet );
//             }
//             catch( beans::UnknownPropertyException ex )
//             {
//                 ASSERT_EXCEPTION( ex );
//             }
            break;

        case XATTR_FILLFLOATTRANSPARENCE:
        {
            try
            {
                chart2::TransparencyStyle aTranspStyle;
                ( GetPropertySet()->getPropertyValue( C2U( "TransparencyStyle" )))
                    >>= aTranspStyle;

                uno::Any aValue( GetPropertySet()->getPropertyValue( C2U( "TransparencyGradient" )));
                if( aValue.hasValue())
                {
                    XFillFloatTransparenceItem aItem;
                    aItem.PutValue( aValue );

                    // this is important to enable the item
                    aItem.SetEnabled( ( aTranspStyle == chart2::TransparencyStyle_GRADIENT ));
                    rOutItemSet.Put( aItem );
                }
            }
            catch( beans::UnknownPropertyException ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
        break;

        case XATTR_GRADIENTSTEPCOUNT:
        {
            ::rtl::OUString aPropName =
                  (m_eGraphicObjectType == FILLED_DATA_POINT)
                  ? C2U( "Gradient" )
                  : C2U( "FillGradient" );
            try
            {
                awt::Gradient aGradient;
                if( GetPropertySet()->getPropertyValue( aPropName ) >>= aGradient )
                {
                    rOutItemSet.Put( XGradientStepCountItem( aGradient.StepCount ));
                }
            }
            catch( beans::UnknownPropertyException ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
        break;

        case XATTR_LINEDASH:
        {
            ::rtl::OUString aPropName =
                  (m_eGraphicObjectType == FILLED_DATA_POINT)
                  ? C2U( "BorderDash" )
                  : C2U( "LineDash" );

            XLineDashItem aItem;
            aItem.PutValue( GetPropertySet()->getPropertyValue( aPropName ) );

            // find the name for the dialog table
            XLineDashItem * pItemToPut = & aItem;
            pItemToPut = aItem.checkForUniqueItem( & m_rDrawModel );

            rOutItemSet.Put( * pItemToPut );
        }
        break;

        case XATTR_FILLGRADIENT:
        {
            ::rtl::OUString aPropName =
                  (m_eGraphicObjectType == FILLED_DATA_POINT)
                  ? C2U( "Gradient" )
                  : C2U( "FillGradient" );

            XFillGradientItem aItem;
            aItem.PutValue( GetPropertySet()->getPropertyValue( aPropName ) );

            // find the name for the dialog table
            XFillGradientItem * pItemToPut = & aItem;
            pItemToPut = aItem.checkForUniqueItem( & m_rDrawModel );

            rOutItemSet.Put( * pItemToPut );
        }
        break;

        case XATTR_FILLHATCH:
        {
            ::rtl::OUString aPropName =
                  (m_eGraphicObjectType == FILLED_DATA_POINT)
                  ? C2U( "Hatch" )
                  : C2U( "FillHatch" );

            XFillHatchItem aItem;
            aItem.PutValue( GetPropertySet()->getPropertyValue( aPropName ) );

            // find the name for the dialog table
            XFillHatchItem * pItemToPut = & aItem;
            pItemToPut = aItem.checkForUniqueItem( & m_rDrawModel );

            rOutItemSet.Put( * pItemToPut );
        }
        break;

    }
}

bool GraphicPropertyItemConverter::ApplySpecialItem(
    USHORT nWhichId, const SfxItemSet & rItemSet )
    throw( uno::Exception )
{
//     if( m_eGraphicObjectType == LINE_DATA_POINT ||
//         m_eGraphicObjectType == LINE_PROPERTIES )
//         return false;

    bool bChanged = false;
    uno::Any aValue;

    switch( nWhichId )
    {
        // bitmap property
        case XATTR_FILLBITMAP:
        case XATTR_FILLBMP_TILE:
        case XATTR_FILLBMP_POS:
        case XATTR_FILLBMP_SIZEX:
        case XATTR_FILLBMP_SIZEY:
        case XATTR_FILLBMP_SIZELOG:
        case XATTR_FILLBMP_TILEOFFSETX:
        case XATTR_FILLBMP_TILEOFFSETY:
        case XATTR_FILLBMP_STRETCH:
        case XATTR_FILLBMP_POSOFFSETX:
        case XATTR_FILLBMP_POSOFFSETY:
            // avoid assertions while bitmap stuff is not working
//             try
//             {
//                 ApplyBitmapItem( nWhichId, rItemSet );
//                 bChanged = true;
//             }
//             catch( beans::UnknownPropertyException ex )
//             {
//                 ASSERT_EXCEPTION( ex );
//             }
            break;

        case XATTR_FILLFLOATTRANSPARENCE:
            try
            {
                const XFillFloatTransparenceItem & rItem =
                    reinterpret_cast< const XFillFloatTransparenceItem & >(
                        rItemSet.Get( nWhichId ));
                if( rItem.IsEnabled() )
                {
                    rItem.QueryValue( aValue );
                    GetPropertySet()->setPropertyValue( C2U( "TransparencyGradient" ), aValue );
                    GetPropertySet()->setPropertyValue( C2U( "TransparencyStyle" ),
                                                        uno::makeAny( chart2::TransparencyStyle_GRADIENT ));
                }
                else
                {
                    GetPropertySet()->setPropertyValue(
                        C2U( "TransparencyStyle" ),
                        uno::makeAny( chart2::TransparencyStyle_LINEAR ));
                }
                bChanged = true;
            }
            catch( beans::UnknownPropertyException ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        break;

        case XATTR_GRADIENTSTEPCOUNT:
        {
            ::rtl::OUString aPropName =
                  (m_eGraphicObjectType == FILLED_DATA_POINT)
                  ? C2U( "Gradient" )
                  : C2U( "FillGradient" );
            try
            {
                sal_Int16 nStepCount =
                    ( reinterpret_cast< const XGradientStepCountItem & >(
                        rItemSet.Get( nWhichId ))).GetValue();

                awt::Gradient aGradient;
                if( GetPropertySet()->getPropertyValue( aPropName ) >>= aGradient )
                {
                    if( aGradient.StepCount != nStepCount )
                    {
                        aGradient.StepCount = nStepCount;
                        GetPropertySet()->setPropertyValue(
                            aPropName,
                            uno::makeAny( aGradient ));
                        bChanged = true;
                    }
                }
            }
            catch( beans::UnknownPropertyException ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
        break;

        case XATTR_LINEDASH:
        {
            ::rtl::OUString aPropName =
                  (m_eGraphicObjectType == FILLED_DATA_POINT)
                  ? C2U( "BorderDash" )
                  : C2U( "LineDash" );

            const XLineDashItem & rItem =
                reinterpret_cast< const XLineDashItem & >(
                    rItemSet.Get( nWhichId ));

            if( rItem.QueryValue( aValue ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( aPropName ))
                {
                    GetPropertySet()->setPropertyValue( aPropName, aValue );
                    bChanged = true;
                }
            }
        }
        break;

        case XATTR_FILLGRADIENT:
        {
            ::rtl::OUString aPropName =
                  (m_eGraphicObjectType == FILLED_DATA_POINT)
                  ? C2U( "Gradient" )
                  : C2U( "FillGradient" );

            const XFillGradientItem & rItem =
                reinterpret_cast< const XFillGradientItem & >(
                    rItemSet.Get( nWhichId ));

            if( rItem.QueryValue( aValue ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( aPropName ))
                {
                    GetPropertySet()->setPropertyValue( aPropName, aValue );
                    bChanged = true;
                }
            }
        }
        break;

        case XATTR_FILLHATCH:
        {
            ::rtl::OUString aPropName =
                  (m_eGraphicObjectType == FILLED_DATA_POINT)
                  ? C2U( "Hatch" )
                  : C2U( "FillHatch" );

            const XFillHatchItem & rItem =
                reinterpret_cast< const XFillHatchItem & >(
                    rItemSet.Get( nWhichId ));

            if( rItem.QueryValue( aValue ))
            {
                if( aValue != GetPropertySet()->getPropertyValue( aPropName ))
                {
                    GetPropertySet()->setPropertyValue( aPropName, aValue );
                    bChanged = true;
                }
            }
        }
        break;
    }

    return bChanged;
}

void GraphicPropertyItemConverter::FillBitmapItem(
    USHORT nWhichId, SfxItemSet & rOutItemSet ) const
    throw( beans::UnknownPropertyException )
{
    uno::Reference< beans::XPropertySet > xProp( GetPropertySet() );
    OSL_ASSERT( xProp.is());
    chart2::FillBitmap aBitmap;
    ( xProp->getPropertyValue( C2U( "Bitmap" ))) >>= aBitmap;

    switch( nWhichId )
    {
        // bitmap property
        case XATTR_FILLBITMAP:
        {
            XFillBitmapItem aBmpIt;
            aBmpIt.PutValue( uno::makeAny( aBitmap.aURL ), MID_GRAFURL );
            rOutItemSet.Put( aBmpIt );
        }
        break;

        case XATTR_FILLBMP_TILE:
            rOutItemSet.Put( XFillBmpTileItem(
                                 aBitmap.aBitmapMode == drawing::BitmapMode_REPEAT ));
            rOutItemSet.Put( XFillBmpStretchItem( FALSE ));
            break;
        case XATTR_FILLBMP_STRETCH:
            rOutItemSet.Put( XFillBmpStretchItem(
                                 aBitmap.aBitmapMode == drawing::BitmapMode_STRETCH ));
            rOutItemSet.Put( XFillBmpTileItem( FALSE ));
            break;
        case XATTR_FILLBMP_POS:
            break;
        case XATTR_FILLBMP_SIZEX:
            break;
        case XATTR_FILLBMP_SIZEY:
            break;
        case XATTR_FILLBMP_SIZELOG:
            break;
        case XATTR_FILLBMP_TILEOFFSETX:
            break;
        case XATTR_FILLBMP_TILEOFFSETY:
            break;
        case XATTR_FILLBMP_POSOFFSETX:
            break;
        case XATTR_FILLBMP_POSOFFSETY:
            break;
    }
}

void GraphicPropertyItemConverter::ApplyBitmapItem(
    USHORT nWhichId, const SfxItemSet & rItemSet )
    throw( beans::UnknownPropertyException )
{
    static const ::rtl::OUString aBitmapPropName(
        RTL_CONSTASCII_USTRINGPARAM( "Bitmap" ));
    uno::Reference< beans::XPropertySet > xProp( GetPropertySet() );
    OSL_ASSERT( xProp.is());
    chart2::FillBitmap aBitmap;
    uno::Any aValue;

    try
    {
        ( xProp->getPropertyValue( aBitmapPropName )) >>= aBitmap;

        switch( nWhichId )
        {
            // bitmap property
            case XATTR_FILLBITMAP:
            {
                const XFillBitmapItem & rBitmapItem =
                    reinterpret_cast< const XFillBitmapItem & >(
                        rItemSet.Get( nWhichId ));
                rBitmapItem.QueryValue( aValue, MID_GRAFURL );
                aValue >>= aBitmap.aURL;

                // must remember bitmap, so that URL stays valid
                //ToDo: Never deleted yet !!!
//                 GraphicObject * pTESTING_CACHE = new GraphicObject(
//                     rBitmapItem.GetValue().GetGraphicObject() );
            }
            break;

            case XATTR_FILLBMP_TILE:
            case XATTR_FILLBMP_STRETCH:
            {
                const XFillBmpStretchItem & rStretchItem =
                    reinterpret_cast< const XFillBmpStretchItem & >(
                        rItemSet.Get( XATTR_FILLBMP_STRETCH ));
                const XFillBmpTileItem & rTileItem =
                    reinterpret_cast< const XFillBmpTileItem & >(
                        rItemSet.Get( XATTR_FILLBMP_TILE ));

                if( rTileItem.GetValue() != FALSE )
                    aBitmap.aBitmapMode = drawing::BitmapMode_REPEAT;
                else if( rStretchItem.GetValue() != FALSE )
                    aBitmap.aBitmapMode = drawing::BitmapMode_STRETCH;
                else
                    aBitmap.aBitmapMode = drawing::BitmapMode_NO_REPEAT;
            }
            break;

            case XATTR_FILLBMP_POS:
                break;
            case XATTR_FILLBMP_SIZEX:
                break;
            case XATTR_FILLBMP_SIZEY:
                break;
            case XATTR_FILLBMP_SIZELOG:
                break;
            case XATTR_FILLBMP_TILEOFFSETX:
                break;
            case XATTR_FILLBMP_TILEOFFSETY:
                break;
            case XATTR_FILLBMP_POSOFFSETX:
                break;
            case XATTR_FILLBMP_POSOFFSETY:
                break;
        }

        xProp->setPropertyValue( aBitmapPropName, uno::makeAny( aBitmap ));
    }
    catch( beans::UnknownPropertyException ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

} //  namespace wrapper
} //  namespace chart
