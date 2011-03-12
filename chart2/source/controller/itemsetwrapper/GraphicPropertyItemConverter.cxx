/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "GraphicPropertyItemConverter.hxx"
#include "SchWhichPairs.hxx"
#include "macros.hxx"
#include "ItemPropertyMap.hxx"
#include "PropertyHelper.hxx"
#include "CommonConverters.hxx"
#include <editeng/memberids.hrc>
#include <svx/xflclit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xlntrit.hxx>
#include <editeng/eeitem.hxx>
// for SfxBoolItem
#include <svl/eitem.hxx>
// for XFillGradientStepCountItem
#include <svx/xgrscit.hxx>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/chart2/FillBitmap.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

using namespace ::com::sun::star;

namespace
{
::comphelper::ItemPropertyMapType & lcl_GetDataPointFilledPropertyMap()
{
    static ::comphelper::ItemPropertyMapType aDataPointPropertyFilledMap(
        ::comphelper::MakeItemPropertyMap
        IPM_MAP_ENTRY( XATTR_FILLSTYLE, "FillStyle", 0 )
        IPM_MAP_ENTRY( XATTR_FILLCOLOR, "Color", 0 )
        IPM_MAP_ENTRY( XATTR_LINECOLOR, "BorderColor", 0 )
        IPM_MAP_ENTRY( XATTR_LINESTYLE, "BorderStyle", 0 )
        IPM_MAP_ENTRY( XATTR_LINEWIDTH, "BorderWidth", 0 )
        IPM_MAP_ENTRY( XATTR_FILLBACKGROUND, "FillBackground", 0 )
        IPM_MAP_ENTRY( XATTR_FILLBMP_POS, "FillBitmapRectanglePoint", 0 )
        IPM_MAP_ENTRY( XATTR_FILLBMP_SIZEX, "FillBitmapSizeX", 0 )
        IPM_MAP_ENTRY( XATTR_FILLBMP_SIZEY, "FillBitmapSizeY", 0 )
        IPM_MAP_ENTRY( XATTR_FILLBMP_SIZELOG, "FillBitmapLogicalSize", 0 )
        IPM_MAP_ENTRY( XATTR_FILLBMP_TILEOFFSETX, "FillBitmapOffsetX", 0 )
        IPM_MAP_ENTRY( XATTR_FILLBMP_TILEOFFSETY, "FillBitmapOffsetY", 0 )
        IPM_MAP_ENTRY( XATTR_FILLBMP_POSOFFSETX, "FillBitmapPositionOffsetX", 0 )
        IPM_MAP_ENTRY( XATTR_FILLBMP_POSOFFSETY, "FillBitmapPositionOffsetY", 0 )
        );

    return aDataPointPropertyFilledMap;
}
::comphelper::ItemPropertyMapType & lcl_GetDataPointLinePropertyMap()
{
    static ::comphelper::ItemPropertyMapType aDataPointPropertyLineMap(
        ::comphelper::MakeItemPropertyMap
        IPM_MAP_ENTRY( XATTR_LINECOLOR, "Color", 0 )
        IPM_MAP_ENTRY( XATTR_LINESTYLE, "LineStyle", 0 )
        IPM_MAP_ENTRY( XATTR_LINEWIDTH, "LineWidth", 0 )
        );

    return aDataPointPropertyLineMap;
}
::comphelper::ItemPropertyMapType & lcl_GetLinePropertyMap()
{
    static ::comphelper::ItemPropertyMapType aLinePropertyMap(
        ::comphelper::MakeItemPropertyMap
        IPM_MAP_ENTRY( XATTR_LINESTYLE, "LineStyle", 0 )
        IPM_MAP_ENTRY( XATTR_LINEWIDTH, "LineWidth", 0 )
        IPM_MAP_ENTRY( XATTR_LINECOLOR, "LineColor", 0 )
        IPM_MAP_ENTRY( XATTR_LINEJOINT, "LineJoint", 0 )
        );

    return aLinePropertyMap;
}
::comphelper::ItemPropertyMapType & lcl_GetFillPropertyMap()
{
    static ::comphelper::ItemPropertyMapType aFillPropertyMap(
        ::comphelper::MakeItemPropertyMap
        IPM_MAP_ENTRY( XATTR_FILLSTYLE, "FillStyle", 0 )
        IPM_MAP_ENTRY( XATTR_FILLCOLOR, "FillColor", 0 )
        IPM_MAP_ENTRY( XATTR_FILLBACKGROUND, "FillBackground", 0 )
        IPM_MAP_ENTRY( XATTR_FILLBMP_POS, "FillBitmapRectanglePoint", 0 )
        IPM_MAP_ENTRY( XATTR_FILLBMP_SIZEX, "FillBitmapSizeX", 0 )
        IPM_MAP_ENTRY( XATTR_FILLBMP_SIZEY, "FillBitmapSizeY", 0 )
        IPM_MAP_ENTRY( XATTR_FILLBMP_SIZELOG, "FillBitmapLogicalSize", 0 )
        IPM_MAP_ENTRY( XATTR_FILLBMP_TILEOFFSETX, "FillBitmapOffsetX", 0 )
        IPM_MAP_ENTRY( XATTR_FILLBMP_TILEOFFSETY, "FillBitmapOffsetY", 0 )
        IPM_MAP_ENTRY( XATTR_FILLBMP_POSOFFSETX, "FillBitmapPositionOffsetX", 0 )
        IPM_MAP_ENTRY( XATTR_FILLBMP_POSOFFSETY, "FillBitmapPositionOffsetY", 0 )
        );

    return aFillPropertyMap;
}

bool lcl_supportsFillProperties( ::chart::wrapper::GraphicPropertyItemConverter::eGraphicObjectType eType )
{
    return ( eType == ::chart::wrapper::GraphicPropertyItemConverter::FILLED_DATA_POINT ||
             eType == ::chart::wrapper::GraphicPropertyItemConverter::FILL_PROPERTIES ||
             eType == ::chart::wrapper::GraphicPropertyItemConverter::LINE_AND_FILL_PROPERTIES );
}

bool lcl_supportsLineProperties( ::chart::wrapper::GraphicPropertyItemConverter::eGraphicObjectType eType )
{
    return ( eType != ::chart::wrapper::GraphicPropertyItemConverter::FILL_PROPERTIES );
}

bool lcl_SetContentForNamedProperty(
    const uno::Reference< lang::XMultiServiceFactory > & xFactory,
    const ::rtl::OUString & rTableName,
    NameOrIndex & rItem, BYTE nMemberId )
{
    bool bResult = false;
    if( xFactory.is())
    {
        ::rtl::OUString aPropertyValue( rItem.GetName());
        uno::Reference< container::XNameAccess > xNameAcc(
            xFactory->createInstance( rTableName ),
            uno::UNO_QUERY );
        if( xNameAcc.is() &&
            xNameAcc->hasByName( aPropertyValue ))
        {
            rItem.PutValue( xNameAcc->getByName( aPropertyValue ), nMemberId );
            bResult = true;
        }
    }
    return bResult;
}

} // anonymous namespace

// ========================================

namespace chart
{
namespace wrapper
{

GraphicPropertyItemConverter::GraphicPropertyItemConverter(
    const uno::Reference<
    beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const uno::Reference< lang::XMultiServiceFactory > & xNamedPropertyContainerFactory,
    eGraphicObjectType eObjectType /* = FILL_PROPERTIES */ ) :
        ItemConverter( rPropertySet, rItemPool ),
        m_eGraphicObjectType( eObjectType ),
        m_rDrawModel( rDrawModel ),
        m_xNamedPropertyTableFactory( xNamedPropertyContainerFactory )
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

bool GraphicPropertyItemConverter::GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const
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

    rOutProperty =(*aIt).second;
    return true;
}

void GraphicPropertyItemConverter::FillSpecialItem(
    USHORT nWhichId, SfxItemSet & rOutItemSet ) const
    throw( uno::Exception )
{
    switch( nWhichId )
    {
        // bitmap property
        case XATTR_FILLBMP_TILE:
        case XATTR_FILLBMP_STRETCH:
        {
            drawing::BitmapMode aMode = drawing::BitmapMode_REPEAT;
            if( GetPropertySet()->getPropertyValue( C2U("FillBitmapMode")) >>= aMode )
            {
                rOutItemSet.Put( XFillBmpTileItem( aMode == drawing::BitmapMode_REPEAT ));
                rOutItemSet.Put( XFillBmpStretchItem( aMode == drawing::BitmapMode_STRETCH ));
            }
        }
        break;

        case XATTR_FILLFLOATTRANSPARENCE:
            try
            {
                if( lcl_supportsFillProperties( m_eGraphicObjectType ))
                {
                    ::rtl::OUString aPropName =
                          (m_eGraphicObjectType == FILLED_DATA_POINT)
                          ? C2U( "TransparencyGradientName" )
                          : C2U( "FillTransparenceGradientName" );

                    uno::Any aValue( GetPropertySet()->getPropertyValue( aPropName ));
                    if( aValue.hasValue())
                    {
                        XFillFloatTransparenceItem aItem;
                        aItem.PutValue( aValue, MID_NAME );

                        lcl_SetContentForNamedProperty(
                            m_xNamedPropertyTableFactory, C2U("com.sun.star.drawing.TransparencyGradientTable"),
                            aItem, MID_FILLGRADIENT );

                        // this is important to enable the item
                        ::rtl::OUString aName;
                        if( (aValue >>= aName) &&
                            aName.getLength())
                        {
                            aItem.SetEnabled( TRUE );
                            rOutItemSet.Put( aItem );
                        }
                    }
                }
            }
            catch( beans::UnknownPropertyException &ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        break;

        case XATTR_GRADIENTSTEPCOUNT:
            if( lcl_supportsFillProperties( m_eGraphicObjectType ))
            {
                ::rtl::OUString aPropName =
                    (m_eGraphicObjectType == FILLED_DATA_POINT)
                    ? C2U( "GradientStepCount" )
                    : C2U( "FillGradientStepCount" );

                uno::Any aValue( GetPropertySet()->getPropertyValue( aPropName ) );
                if( hasLongOrShortValue(aValue) )
                {
                    sal_Int16 nStepCount = getShortForLongAlso(aValue);
                    rOutItemSet.Put( XGradientStepCountItem( nStepCount ));
                }
            }
        break;

        case XATTR_LINEDASH:
            if( lcl_supportsLineProperties( m_eGraphicObjectType ))
            {
                ::rtl::OUString aPropName =
                    (m_eGraphicObjectType == FILLED_DATA_POINT)
                    ? C2U( "BorderDashName" )
                    : C2U( "LineDashName" );

                XLineDashItem aItem;
                aItem.PutValue( GetPropertySet()->getPropertyValue( aPropName ), MID_NAME );

                lcl_SetContentForNamedProperty(
                    m_xNamedPropertyTableFactory, C2U("com.sun.star.drawing.DashTable"),
                    aItem, MID_LINEDASH );

                // translate model name to UI-name for predefined entries, so
                // that the correct entry is chosen in the list of UI-names
                XLineDashItem * pItemToPut = & aItem;
                pItemToPut = aItem.checkForUniqueItem( & m_rDrawModel );

                rOutItemSet.Put( * pItemToPut );
            }
        break;

        case XATTR_FILLGRADIENT:
            if( lcl_supportsFillProperties( m_eGraphicObjectType ))
            {
                ::rtl::OUString aPropName =
                    (m_eGraphicObjectType == FILLED_DATA_POINT)
                    ? C2U( "GradientName" )
                    : C2U( "FillGradientName" );

                XFillGradientItem aItem;
                aItem.PutValue( GetPropertySet()->getPropertyValue( aPropName ), MID_NAME );

                lcl_SetContentForNamedProperty(
                    m_xNamedPropertyTableFactory, C2U("com.sun.star.drawing.GradientTable"),
                    aItem, MID_FILLGRADIENT );

                // translate model name to UI-name for predefined entries, so
                // that the correct entry is chosen in the list of UI-names
                XFillGradientItem * pItemToPut = & aItem;
                pItemToPut = aItem.checkForUniqueItem( & m_rDrawModel );

                rOutItemSet.Put( * pItemToPut );
            }
        break;

        case XATTR_FILLHATCH:
            if( lcl_supportsFillProperties( m_eGraphicObjectType ))
            {
                ::rtl::OUString aPropName =
                    (m_eGraphicObjectType == FILLED_DATA_POINT)
                    ? C2U( "HatchName" )
                    : C2U( "FillHatchName" );

                XFillHatchItem aItem;
                aItem.PutValue( GetPropertySet()->getPropertyValue( aPropName ), MID_NAME );

                lcl_SetContentForNamedProperty(
                    m_xNamedPropertyTableFactory, C2U("com.sun.star.drawing.HatchTable"),
                    aItem, MID_FILLHATCH );

                // translate model name to UI-name for predefined entries, so
                // that the correct entry is chosen in the list of UI-names
                XFillHatchItem * pItemToPut = & aItem;
                pItemToPut = aItem.checkForUniqueItem( & m_rDrawModel );

                rOutItemSet.Put( * pItemToPut );
            }
        break;

        case XATTR_FILLBITMAP:
            if( lcl_supportsFillProperties( m_eGraphicObjectType ))
            {
                XFillBitmapItem aItem;
                aItem.PutValue( GetPropertySet()->getPropertyValue( C2U( "FillBitmapName" ) ), MID_NAME );

                lcl_SetContentForNamedProperty(
                    m_xNamedPropertyTableFactory, C2U("com.sun.star.drawing.BitmapTable"),
                    aItem, MID_GRAFURL );

                // translate model name to UI-name for predefined entries, so
                // that the correct entry is chosen in the list of UI-names
                XFillBitmapItem * pItemToPut = & aItem;
                pItemToPut = aItem.checkForUniqueItem( & m_rDrawModel );

                rOutItemSet.Put( * pItemToPut );
            }
        break;

        // hack, because QueryValue of XLineTransparenceItem returns sal_Int32
        // instead of sal_Int16
        case XATTR_LINETRANSPARENCE:
            if( lcl_supportsLineProperties( m_eGraphicObjectType ))
            {
                ::rtl::OUString aPropName =
                      (m_eGraphicObjectType == FILLED_DATA_POINT)
                      ? C2U( "BorderTransparency" )
                      : (m_eGraphicObjectType == LINE_DATA_POINT)
                      ? C2U( "Transparency" )
                      : C2U( "LineTransparence" );

                XLineTransparenceItem aItem;
                aItem.PutValue( GetPropertySet()->getPropertyValue( aPropName ) );

                rOutItemSet.Put( aItem );
            }
            break;

        // hack, because QueryValue of XFillTransparenceItem returns sal_Int32
        // instead of sal_Int16
        case XATTR_FILLTRANSPARENCE:
            if( lcl_supportsFillProperties( m_eGraphicObjectType ))
            {
                ::rtl::OUString aPropName =
                      (m_eGraphicObjectType == FILLED_DATA_POINT)
                      ? C2U( "Transparency" )
                      : C2U( "FillTransparence" );

                XFillTransparenceItem aItem;
                aItem.PutValue( GetPropertySet()->getPropertyValue( aPropName ) );

                rOutItemSet.Put( aItem );
            }
            break;
    }
}

bool GraphicPropertyItemConverter::ApplySpecialItem(
    USHORT nWhichId, const SfxItemSet & rItemSet )
    throw( uno::Exception )
{
    bool bChanged = false;
    uno::Any aValue;

    switch( nWhichId )
    {
        // bitmap property
        case XATTR_FILLBMP_STRETCH:
            if( lcl_supportsFillProperties( m_eGraphicObjectType ))
            {
                const ::rtl::OUString aModePropName = C2U("FillBitmapMode");
                bool bStretched = static_cast< const XFillBmpStretchItem & >(
                    rItemSet.Get( XATTR_FILLBMP_STRETCH )).GetValue();
                drawing::BitmapMode aMode =
                    (bStretched ? drawing::BitmapMode_STRETCH : drawing::BitmapMode_NO_REPEAT);

                aValue <<= aMode;
                if( aValue != GetPropertySet()->getPropertyValue( aModePropName ))
                {
                    GetPropertySet()->setPropertyValue( aModePropName, aValue );
                    bChanged = true;
                }
            }
            break;

        case XATTR_FILLBMP_TILE:
            if( lcl_supportsFillProperties( m_eGraphicObjectType ))
            {
                const ::rtl::OUString aModePropName = C2U("FillBitmapMode");
                bool bTiled = static_cast< const XFillBmpTileItem & >(
                    rItemSet.Get( XATTR_FILLBMP_TILE )).GetValue();
                drawing::BitmapMode aMode =
                    (bTiled ? drawing::BitmapMode_REPEAT : drawing::BitmapMode_NO_REPEAT);

                aValue <<= aMode;
                if( aValue != GetPropertySet()->getPropertyValue( aModePropName ))
                {
                    GetPropertySet()->setPropertyValue( aModePropName, aValue );
                    bChanged = true;
                }
            }
            break;

        case XATTR_FILLFLOATTRANSPARENCE:
            try
            {
                if( lcl_supportsFillProperties( m_eGraphicObjectType ))
                {
                    ::rtl::OUString aPropName =
                          (m_eGraphicObjectType == FILLED_DATA_POINT)
                          ? C2U( "TransparencyGradientName" )
                          : C2U( "FillTransparenceGradientName" );

                    const XFillFloatTransparenceItem & rItem =
                        static_cast< const XFillFloatTransparenceItem & >(
                            rItemSet.Get( nWhichId ));

                    if( rItem.IsEnabled() &&
                        rItem.QueryValue( aValue, MID_NAME ))
                    {
                        uno::Any aGradient;
                        rItem.QueryValue( aGradient, MID_FILLGRADIENT );

                        // add TransparencyGradient to list if it does not already exist
                        ::rtl::OUString aPreferredName;
                        aValue >>= aPreferredName;
                        aValue <<= PropertyHelper::addTransparencyGradientUniqueNameToTable(
                            aGradient, m_xNamedPropertyTableFactory, aPreferredName );

                        if( aValue != GetPropertySet()->getPropertyValue( aPropName ))
                        {
                            GetPropertySet()->setPropertyValue( aPropName, aValue );
                            bChanged = true;
                        }
                    }
                    else
                    {
                        ::rtl::OUString aName;
                        if( ( GetPropertySet()->getPropertyValue( aPropName ) >>= aName )
                            && aName.getLength() > 0 )
                        {
                            uno::Reference< beans::XPropertyState > xState( GetPropertySet(), uno::UNO_QUERY );
                            if( xState.is())
                                xState->setPropertyToDefault( aPropName );
                            bChanged = true;
                        }
                    }
                }
            }
            catch( beans::UnknownPropertyException &ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        break;

        case XATTR_GRADIENTSTEPCOUNT:
        {
            if( lcl_supportsFillProperties( m_eGraphicObjectType ))
            {
                ::rtl::OUString aPropName =
                    (m_eGraphicObjectType == FILLED_DATA_POINT)
                    ? C2U( "GradientStepCount" )
                    : C2U( "FillGradientStepCount" );

                sal_Int16 nStepCount = ( static_cast< const XGradientStepCountItem & >(
                            rItemSet.Get( nWhichId ))).GetValue();

                aValue <<= nStepCount;
                if( aValue != GetPropertySet()->getPropertyValue( aPropName ))
                {
                    GetPropertySet()->setPropertyValue( aPropName, aValue );
                    bChanged = true;
                }
            }
        }
        break;

        case XATTR_LINEDASH:
        {
            if( lcl_supportsLineProperties( m_eGraphicObjectType ))
            {

                ::rtl::OUString aPropName =
                    (m_eGraphicObjectType == FILLED_DATA_POINT)
                    ? C2U( "BorderDashName" )
                    : C2U( "LineDashName" );

                const XLineDashItem & rItem =
                    static_cast< const XLineDashItem & >(
                        rItemSet.Get( nWhichId ));

                if( rItem.QueryValue( aValue, MID_NAME ))
                {
                    if( aValue != GetPropertySet()->getPropertyValue( aPropName ))
                    {
                        // add LineDash to list
                        uno::Any aLineDash;
                        rItem.QueryValue( aLineDash, MID_LINEDASH );
                        ::rtl::OUString aPreferredName;
                        aValue >>= aPreferredName;
                        aValue <<= PropertyHelper::addLineDashUniqueNameToTable(
                            aLineDash, m_xNamedPropertyTableFactory, aPreferredName );

                        GetPropertySet()->setPropertyValue( aPropName, aValue );
                        bChanged = true;
                    }
                }
            }
        }
        break;

        case XATTR_FILLGRADIENT:
        {
            if( lcl_supportsFillProperties( m_eGraphicObjectType ))
            {
                ::rtl::OUString aPropName =
                    (m_eGraphicObjectType == FILLED_DATA_POINT)
                    ? C2U( "GradientName" )
                    : C2U( "FillGradientName" );

                const XFillGradientItem & rItem =
                    static_cast< const XFillGradientItem & >(
                        rItemSet.Get( nWhichId ));

                if( rItem.QueryValue( aValue, MID_NAME ))
                {
                    if( aValue != GetPropertySet()->getPropertyValue( aPropName ))
                    {
                        // add Gradient to list
                        uno::Any aGradient;
                        rItem.QueryValue( aGradient, MID_FILLGRADIENT );
                        ::rtl::OUString aPreferredName;
                        aValue >>= aPreferredName;
                        aValue <<= PropertyHelper::addGradientUniqueNameToTable(
                            aGradient, m_xNamedPropertyTableFactory, aPreferredName );

                        GetPropertySet()->setPropertyValue( aPropName, aValue );
                        bChanged = true;
                    }
                }
            }
        }
        break;

        case XATTR_FILLHATCH:
        {
            if( lcl_supportsFillProperties( m_eGraphicObjectType ))
            {
                ::rtl::OUString aPropName =
                    (m_eGraphicObjectType == FILLED_DATA_POINT)
                    ? C2U( "HatchName" )
                    : C2U( "FillHatchName" );

                const XFillHatchItem & rItem =
                    static_cast< const XFillHatchItem & >(
                        rItemSet.Get( nWhichId ));

                if( rItem.QueryValue( aValue, MID_NAME ))
                {
                    if( aValue != GetPropertySet()->getPropertyValue( aPropName ))
                    {
                        // add Hatch to list
                        uno::Any aHatch;
                        rItem.QueryValue( aHatch, MID_FILLHATCH );
                        ::rtl::OUString aPreferredName;
                        aValue >>= aPreferredName;
                        aValue <<= PropertyHelper::addHatchUniqueNameToTable(
                            aHatch, m_xNamedPropertyTableFactory, aPreferredName );

                        GetPropertySet()->setPropertyValue( aPropName, aValue );
                        bChanged = true;
                    }
                }
            }
        }
        break;

        case XATTR_FILLBITMAP:
        {
            if( lcl_supportsFillProperties( m_eGraphicObjectType ))
            {
                const XFillBitmapItem & rItem =
                    static_cast< const XFillBitmapItem & >(
                        rItemSet.Get( nWhichId ));

                if( rItem.QueryValue( aValue, MID_NAME ))
                {
                    if( aValue != GetPropertySet()->getPropertyValue( C2U( "FillBitmapName" )))
                    {
                        // add Bitmap to list
                        uno::Any aBitmap;
                        rItem.QueryValue( aBitmap, MID_GRAFURL );
                        ::rtl::OUString aPreferredName;
                        aValue >>= aPreferredName;
                        aValue <<= PropertyHelper::addBitmapUniqueNameToTable(
                            aBitmap, m_xNamedPropertyTableFactory, aPreferredName );

                        GetPropertySet()->setPropertyValue( C2U( "FillBitmapName" ), aValue );
                        bChanged = true;
                    }
                }
            }
        }
        break;

        // hack, because QueryValue of XLineTransparenceItem returns sal_Int32
        // instead of sal_Int16
        case XATTR_LINETRANSPARENCE:
            if( lcl_supportsLineProperties( m_eGraphicObjectType ))
            {
                ::rtl::OUString aPropName =
                      (m_eGraphicObjectType == FILLED_DATA_POINT)
                      ? C2U( "BorderTransparency" )
                      : (m_eGraphicObjectType == LINE_DATA_POINT)
                      ? C2U( "Transparency" )
                      : C2U( "LineTransparence" );

                const XLineTransparenceItem & rItem =
                    static_cast< const XLineTransparenceItem & >(
                        rItemSet.Get( nWhichId ));

                if( rItem.QueryValue( aValue ))
                {
                    OSL_ENSURE( ! aValue.isExtractableTo(
                                    ::getCppuType( reinterpret_cast< const sal_Int16 * >(0))),
                                "TransparenceItem QueryValue bug is fixed. Remove hack." );
                    sal_Int32 nValue = 0;
                    if( aValue >>= nValue )
                    {
                        OSL_ENSURE( nValue < SAL_MAX_INT16, "Transparency value too large" );
                        sal_Int16 nValueToSet( static_cast< sal_Int16 >( nValue ));
                        aValue <<= nValueToSet;

                        GetPropertySet()->setPropertyValue( aPropName, aValue );
                        bChanged = true;
                    }
                    else
                    {
                        OSL_FAIL( "Wrong type in Transparency Any" );
                    }
                }
            }
            break;

        // hack, because QueryValue of XFillTransparenceItem returns sal_Int32
        // instead of sal_Int16
        case XATTR_FILLTRANSPARENCE:
            if( lcl_supportsFillProperties( m_eGraphicObjectType ))
            {
                ::rtl::OUString aPropName =
                      (m_eGraphicObjectType == FILLED_DATA_POINT)
                      ? C2U( "Transparency" )
                      : C2U( "FillTransparence" );

                const XFillTransparenceItem & rItem =
                    static_cast< const XFillTransparenceItem & >(
                        rItemSet.Get( nWhichId ));

                if( rItem.QueryValue( aValue ))
                {
                    OSL_ENSURE( ! aValue.isExtractableTo(
                                    ::getCppuType( reinterpret_cast< const sal_Int16 * >(0))),
                                "TransparenceItem QueryValue bug is fixed. Remove hack." );
                    sal_Int32 nValue = 0;
                    if( aValue >>= nValue )
                    {
                        OSL_ENSURE( nValue < SAL_MAX_INT16, "Transparency value too large" );
                        sal_Int16 nValueToSet( static_cast< sal_Int16 >( nValue ));
                        aValue <<= nValueToSet;

                        GetPropertySet()->setPropertyValue( aPropName, aValue );
                        // if linear or no transparence is set, delete the gradient
                        ::rtl::OUString aTransGradPropName =
                              (m_eGraphicObjectType == FILLED_DATA_POINT)
                              ? C2U( "TransparencyGradientName" )
                              : C2U( "FillTransparenceGradientName" );
                        GetPropertySet()->setPropertyValue(
                            aTransGradPropName, uno::makeAny( ::rtl::OUString() ));

                        bChanged = true;
                    }
                    else
                    {
                        OSL_FAIL( "Wrong type in Transparency Any" );
                    }
                }
            }
            break;
    }

    return bChanged;
}

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
