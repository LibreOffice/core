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
    const OUString & rTableName,
    NameOrIndex & rItem, sal_uInt8 nMemberId )
{
    bool bResult = false;
    if( xFactory.is())
    {
        OUString aPropertyValue( rItem.GetName());
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

const sal_uInt16 * GraphicPropertyItemConverter::GetWhichPairs() const
{
    const sal_uInt16 * pResult = NULL;

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
    sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
    throw( uno::Exception )
{
    switch( nWhichId )
    {
        // bitmap property
        case XATTR_FILLBMP_TILE:
        case XATTR_FILLBMP_STRETCH:
        {
            drawing::BitmapMode aMode = drawing::BitmapMode_REPEAT;
            if( GetPropertySet()->getPropertyValue( "FillBitmapMode" ) >>= aMode )
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
                    OUString aPropName =
                          (m_eGraphicObjectType == FILLED_DATA_POINT)
                          ? OUString( "TransparencyGradientName" )
                          : OUString( "FillTransparenceGradientName" );

                    uno::Any aValue( GetPropertySet()->getPropertyValue( aPropName ));
                    if( aValue.hasValue())
                    {
                        XFillFloatTransparenceItem aItem;
                        aItem.PutValue( aValue, MID_NAME );

                        lcl_SetContentForNamedProperty(
                            m_xNamedPropertyTableFactory, "com.sun.star.drawing.TransparencyGradientTable" ,
                            aItem, MID_FILLGRADIENT );

                        // this is important to enable the item
                        OUString aName;
                        if( (aValue >>= aName) &&
                            !aName.isEmpty())
                        {
                            aItem.SetEnabled( sal_True );
                            rOutItemSet.Put( aItem );
                        }
                    }
                }
            }
            catch( const beans::UnknownPropertyException &ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        break;

        case XATTR_GRADIENTSTEPCOUNT:
            if( lcl_supportsFillProperties( m_eGraphicObjectType ))
            {
                OUString aPropName =
                    (m_eGraphicObjectType == FILLED_DATA_POINT)
                    ? OUString( "GradientStepCount" )
                    : OUString( "FillGradientStepCount" );

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
                OUString aPropName =
                    (m_eGraphicObjectType == FILLED_DATA_POINT)
                    ? OUString( "BorderDashName" )
                    : OUString( "LineDashName" );

                XLineDashItem aItem;
                aItem.PutValue( GetPropertySet()->getPropertyValue( aPropName ), MID_NAME );

                lcl_SetContentForNamedProperty(
                    m_xNamedPropertyTableFactory, "com.sun.star.drawing.DashTable" ,
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
                OUString aPropName =
                    (m_eGraphicObjectType == FILLED_DATA_POINT)
                    ? OUString( "GradientName" )
                    : OUString( "FillGradientName" );

                XFillGradientItem aItem;
                aItem.PutValue( GetPropertySet()->getPropertyValue( aPropName ), MID_NAME );

                lcl_SetContentForNamedProperty(
                    m_xNamedPropertyTableFactory, "com.sun.star.drawing.GradientTable" ,
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
                OUString aPropName =
                    (m_eGraphicObjectType == FILLED_DATA_POINT)
                    ? OUString( "HatchName" )
                    : OUString( "FillHatchName" );

                XFillHatchItem aItem;
                aItem.PutValue( GetPropertySet()->getPropertyValue( aPropName ), MID_NAME );

                lcl_SetContentForNamedProperty(
                    m_xNamedPropertyTableFactory, "com.sun.star.drawing.HatchTable" ,
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
                aItem.PutValue( GetPropertySet()->getPropertyValue( "FillBitmapName" ), MID_NAME );

                lcl_SetContentForNamedProperty(
                    m_xNamedPropertyTableFactory, "com.sun.star.drawing.BitmapTable" ,
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
                OUString aPropName =
                      (m_eGraphicObjectType == FILLED_DATA_POINT)
                      ? OUString( "BorderTransparency" )
                      : (m_eGraphicObjectType == LINE_DATA_POINT)
                      ? OUString( "Transparency" )
                      : OUString( "LineTransparence" );

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
                OUString aPropName =
                      (m_eGraphicObjectType == FILLED_DATA_POINT)
                      ? OUString( "Transparency" )
                      : OUString( "FillTransparence" );

                XFillTransparenceItem aItem;
                aItem.PutValue( GetPropertySet()->getPropertyValue( aPropName ) );

                rOutItemSet.Put( aItem );
            }
            break;
    }
}

bool GraphicPropertyItemConverter::ApplySpecialItem(
    sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
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
                const OUString aModePropName("FillBitmapMode");
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
                const OUString aModePropName("FillBitmapMode");
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
                    OUString aPropName =
                          (m_eGraphicObjectType == FILLED_DATA_POINT)
                          ? OUString( "TransparencyGradientName" )
                          : OUString( "FillTransparenceGradientName" );

                    const XFillFloatTransparenceItem & rItem =
                        static_cast< const XFillFloatTransparenceItem & >(
                            rItemSet.Get( nWhichId ));

                    if( rItem.IsEnabled() &&
                        rItem.QueryValue( aValue, MID_NAME ))
                    {
                        uno::Any aGradient;
                        rItem.QueryValue( aGradient, MID_FILLGRADIENT );

                        // add TransparencyGradient to list if it does not already exist
                        OUString aPreferredName;
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
                        OUString aName;
                        if( ( GetPropertySet()->getPropertyValue( aPropName ) >>= aName )
                            && !aName.isEmpty() )
                        {
                            uno::Reference< beans::XPropertyState > xState( GetPropertySet(), uno::UNO_QUERY );
                            if( xState.is())
                                xState->setPropertyToDefault( aPropName );
                            bChanged = true;
                        }
                    }
                }
            }
            catch( const beans::UnknownPropertyException &ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        break;

        case XATTR_GRADIENTSTEPCOUNT:
        {
            if( lcl_supportsFillProperties( m_eGraphicObjectType ))
            {
                OUString aPropName =
                    (m_eGraphicObjectType == FILLED_DATA_POINT)
                    ? OUString( "GradientStepCount" )
                    : OUString( "FillGradientStepCount" );

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

                OUString aPropName =
                    (m_eGraphicObjectType == FILLED_DATA_POINT)
                    ? OUString( "BorderDashName" )
                    : OUString( "LineDashName" );

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
                        OUString aPreferredName;
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
                OUString aPropName =
                    (m_eGraphicObjectType == FILLED_DATA_POINT)
                    ? OUString( "GradientName" )
                    : OUString( "FillGradientName" );

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
                        OUString aPreferredName;
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
                OUString aPropName =
                    (m_eGraphicObjectType == FILLED_DATA_POINT)
                    ? OUString( "HatchName" )
                    : OUString( "FillHatchName" );

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
                        OUString aPreferredName;
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
                    if( aValue != GetPropertySet()->getPropertyValue( "FillBitmapName" ))
                    {
                        // add Bitmap to list
                        uno::Any aBitmap;
                        rItem.QueryValue( aBitmap, MID_GRAFURL );
                        OUString aPreferredName;
                        aValue >>= aPreferredName;
                        aValue <<= PropertyHelper::addBitmapUniqueNameToTable(
                            aBitmap, m_xNamedPropertyTableFactory, aPreferredName );

                        GetPropertySet()->setPropertyValue( "FillBitmapName" , aValue );
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
                OUString aPropName =
                      (m_eGraphicObjectType == FILLED_DATA_POINT)
                      ? OUString( "BorderTransparency" )
                      : (m_eGraphicObjectType == LINE_DATA_POINT)
                      ? OUString( "Transparency" )
                      : OUString( "LineTransparence" );

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
                OUString aPropName =
                      (m_eGraphicObjectType == FILLED_DATA_POINT)
                      ? OUString( "Transparency" )
                      : OUString( "FillTransparence" );

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
                        OUString aTransGradPropName =
                              (m_eGraphicObjectType == FILLED_DATA_POINT)
                              ? OUString( "TransparencyGradientName" )
                              : OUString( "FillTransparenceGradientName" );
                        GetPropertySet()->setPropertyValue(
                            aTransGradPropName, uno::makeAny( OUString() ));

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
