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

#include <GraphicPropertyItemConverter.hxx>
#include "SchWhichPairs.hxx"
#include <ItemPropertyMap.hxx>
#include <PropertyHelper.hxx>
#include <CommonConverters.hxx>
#include <editeng/memberids.h>
#include <svx/unomid.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xgrscit.hxx>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <utility>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;

namespace chart::wrapper {

namespace {

ItemPropertyMapType & lcl_GetDataPointFilledPropertyMap()
{
    static ItemPropertyMapType aDataPointPropertyFilledMap{
        {XATTR_FILLSTYLE, {"FillStyle", 0}},
        {XATTR_FILLCOLOR, {"Color", 0}},
        {XATTR_LINECOLOR, {"BorderColor", 0}},
        {XATTR_LINESTYLE, {"BorderStyle", 0}},
        {XATTR_LINEWIDTH, {"BorderWidth", 0}},
        {XATTR_FILLBACKGROUND, {"FillBackground", 0}},
        {XATTR_FILLBMP_POS, {"FillBitmapRectanglePoint", 0}},
        {XATTR_FILLBMP_SIZEX, {"FillBitmapSizeX", 0}},
        {XATTR_FILLBMP_SIZEY, {"FillBitmapSizeY", 0}},
        {XATTR_FILLBMP_SIZELOG, {"FillBitmapLogicalSize", 0}},
        {XATTR_FILLBMP_TILEOFFSETX, {"FillBitmapOffsetX", 0}},
        {XATTR_FILLBMP_TILEOFFSETY, {"FillBitmapOffsetY", 0}},
        {XATTR_FILLBMP_POSOFFSETX, {"FillBitmapPositionOffsetX", 0}},
        {XATTR_FILLBMP_POSOFFSETY, {"FillBitmapPositionOffsetY", 0}}};
    return aDataPointPropertyFilledMap;
}
ItemPropertyMapType & lcl_GetDataPointLinePropertyMap()
{
    static ItemPropertyMapType aDataPointPropertyLineMap{
        {XATTR_LINECOLOR, {"Color", 0}},
        {XATTR_LINESTYLE, {"LineStyle", 0}},
        {XATTR_LINEWIDTH, {"LineWidth", 0}},
        {XATTR_LINECAP, {"LineCap", 0}}};
    return aDataPointPropertyLineMap;
}
ItemPropertyMapType & lcl_GetLinePropertyMap()
{
    static ItemPropertyMapType aLinePropertyMap{
        {XATTR_LINESTYLE, {"LineStyle", 0}},
        {XATTR_LINEWIDTH, {"LineWidth", 0}},
        {XATTR_LINECOLOR, {"LineColor", 0}},
        {XATTR_LINEJOINT, {"LineJoint", 0}},
        {XATTR_LINECAP, {"LineCap", 0}}};
    return aLinePropertyMap;
}
ItemPropertyMapType & lcl_GetFillPropertyMap()
{
    static ItemPropertyMapType aFillPropertyMap{
        {XATTR_FILLSTYLE, {"FillStyle", 0}},
        {XATTR_FILLCOLOR, {"FillColor", 0}},
        {XATTR_FILLBACKGROUND, {"FillBackground", 0}},
        {XATTR_FILLBMP_POS, {"FillBitmapRectanglePoint", 0}},
        {XATTR_FILLBMP_SIZEX, {"FillBitmapSizeX", 0}},
        {XATTR_FILLBMP_SIZEY, {"FillBitmapSizeY", 0}},
        {XATTR_FILLBMP_SIZELOG, {"FillBitmapLogicalSize", 0}},
        {XATTR_FILLBMP_TILEOFFSETX, {"FillBitmapOffsetX", 0}},
        {XATTR_FILLBMP_TILEOFFSETY, {"FillBitmapOffsetY", 0}},
        {XATTR_FILLBMP_POSOFFSETX, {"FillBitmapPositionOffsetX", 0}},
        {XATTR_FILLBMP_POSOFFSETY, {"FillBitmapPositionOffsetY", 0}}};
    return aFillPropertyMap;
}

bool lcl_supportsFillProperties( ::chart::wrapper::GraphicObjectType eType )
{
    return ( eType == ::chart::wrapper::GraphicObjectType::FilledDataPoint ||
             eType == ::chart::wrapper::GraphicObjectType::LineAndFillProperties );
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

GraphicPropertyItemConverter::GraphicPropertyItemConverter(
    const uno::Reference<
    beans::XPropertySet > & rPropertySet,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    uno::Reference< lang::XMultiServiceFactory > xNamedPropertyContainerFactory,
    GraphicObjectType eObjectType /* = FILL_PROPERTIES */ ) :
        ItemConverter( rPropertySet, rItemPool ),
        m_GraphicObjectType( eObjectType ),
        m_rDrawModel( rDrawModel ),
        m_xNamedPropertyTableFactory(std::move( xNamedPropertyContainerFactory ))
{}

GraphicPropertyItemConverter::~GraphicPropertyItemConverter()
{}

const WhichRangesContainer& GraphicPropertyItemConverter::GetWhichPairs() const
{
    switch( m_GraphicObjectType )
    {
        case GraphicObjectType::LineDataPoint:
        case GraphicObjectType::FilledDataPoint:
            return nRowWhichPairs;
        case GraphicObjectType::LineProperties:
            return nLinePropertyWhichPairs;
        case GraphicObjectType::LineAndFillProperties:
            return nLineAndFillPropertyWhichPairs;
    }

    static const WhichRangesContainer empty;
    return empty;
}

bool GraphicPropertyItemConverter::GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const
{
    ItemPropertyMapType::const_iterator aEndIt;
    ItemPropertyMapType::const_iterator aIt;

    switch( m_GraphicObjectType )
    {
        case GraphicObjectType::LineDataPoint:
            aEndIt = lcl_GetDataPointLinePropertyMap().end();
            aIt = lcl_GetDataPointLinePropertyMap().find( nWhichId );
            break;
        case GraphicObjectType::FilledDataPoint:
            aEndIt = lcl_GetDataPointFilledPropertyMap().end();
            aIt = lcl_GetDataPointFilledPropertyMap().find( nWhichId );
            break;
        case GraphicObjectType::LineProperties:
            aEndIt = lcl_GetLinePropertyMap().end();
            aIt = lcl_GetLinePropertyMap().find( nWhichId );
            break;

        case GraphicObjectType::LineAndFillProperties:
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
{
    switch( nWhichId )
    {
        // bitmap property
        case XATTR_FILLBMP_TILE:
        case XATTR_FILLBMP_STRETCH:
        {
            drawing::BitmapMode aMode = drawing::BitmapMode_REPEAT;
            if( GetPropertySet()->getPropertyValue( u"FillBitmapMode"_ustr ) >>= aMode )
            {
                rOutItemSet.Put( XFillBmpTileItem( aMode == drawing::BitmapMode_REPEAT ));
                rOutItemSet.Put( XFillBmpStretchItem( aMode == drawing::BitmapMode_STRETCH ));
            }
        }
        break;

        case XATTR_FILLFLOATTRANSPARENCE:
            try
            {
                if( lcl_supportsFillProperties( m_GraphicObjectType ))
                {
                    OUString aPropName =
                          (m_GraphicObjectType == GraphicObjectType::FilledDataPoint)
                          ? u"TransparencyGradientName"_ustr
                          : u"FillTransparenceGradientName"_ustr;

                    uno::Any aValue( GetPropertySet()->getPropertyValue( aPropName ));
                    if( aValue.hasValue())
                    {
                        XFillFloatTransparenceItem aItem;
                        aItem.PutValue( aValue, MID_NAME );

                        lcl_SetContentForNamedProperty(
                            m_xNamedPropertyTableFactory, u"com.sun.star.drawing.TransparencyGradientTable"_ustr ,
                            aItem, MID_FILLGRADIENT );

                        // this is important to enable the item
                        OUString aName;
                        if( (aValue >>= aName) &&
                            !aName.isEmpty())
                        {
                            aItem.SetEnabled( true );
                            rOutItemSet.Put( aItem );
                        }
                    }
                }
            }
            catch( const beans::UnknownPropertyException & )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
        break;

        case XATTR_GRADIENTSTEPCOUNT:
            if( lcl_supportsFillProperties( m_GraphicObjectType ))
            {
                OUString aPropName =
                    (m_GraphicObjectType == GraphicObjectType::FilledDataPoint)
                    ? u"GradientStepCount"_ustr
                    : u"FillGradientStepCount"_ustr;

                uno::Any aValue( GetPropertySet()->getPropertyValue( aPropName ) );
                if( hasLongOrShortValue(aValue) )
                {
                    sal_Int16 nStepCount = getShortForLongAlso(aValue);
                    rOutItemSet.Put( XGradientStepCountItem( nStepCount ));
                }
            }
        break;

        case XATTR_LINEDASH:
        {
            OUString aPropName =
                (m_GraphicObjectType == GraphicObjectType::FilledDataPoint)
                 ? u"BorderDashName"_ustr
                 : u"LineDashName"_ustr;

            XLineDashItem aItem;
            aItem.PutValue( GetPropertySet()->getPropertyValue( aPropName ), MID_NAME );

            lcl_SetContentForNamedProperty(
                m_xNamedPropertyTableFactory, u"com.sun.star.drawing.DashTable"_ustr ,
                aItem, MID_LINEDASH );

            // translate model name to UI-name for predefined entries, so
            // that the correct entry is chosen in the list of UI-names
            std::unique_ptr<XLineDashItem> pItemToPut = aItem.checkForUniqueItem( & m_rDrawModel );

            if(pItemToPut)
                 rOutItemSet.Put( std::move(pItemToPut) );
            else
                rOutItemSet.Put(aItem);
        }
        break;

        case XATTR_FILLGRADIENT:
            if( lcl_supportsFillProperties( m_GraphicObjectType ))
            {
                OUString aPropName =
                    (m_GraphicObjectType == GraphicObjectType::FilledDataPoint)
                    ? u"GradientName"_ustr
                    : u"FillGradientName"_ustr;

                XFillGradientItem aItem;
                aItem.PutValue( GetPropertySet()->getPropertyValue( aPropName ), MID_NAME );

                lcl_SetContentForNamedProperty(
                    m_xNamedPropertyTableFactory, u"com.sun.star.drawing.GradientTable"_ustr ,
                    aItem, MID_FILLGRADIENT );

                // translate model name to UI-name for predefined entries, so
                // that the correct entry is chosen in the list of UI-names
                std::unique_ptr<XFillGradientItem> pItemToPut = aItem.checkForUniqueItem( & m_rDrawModel );

                if(pItemToPut)
                    rOutItemSet.Put(std::move(pItemToPut) );
                else
                    rOutItemSet.Put(aItem);
            }
        break;

        case XATTR_FILLHATCH:
            if( lcl_supportsFillProperties( m_GraphicObjectType ))
            {
                OUString aPropName =
                    (m_GraphicObjectType == GraphicObjectType::FilledDataPoint)
                    ? u"HatchName"_ustr
                    : u"FillHatchName"_ustr;

                XFillHatchItem aItem;
                aItem.PutValue( GetPropertySet()->getPropertyValue( aPropName ), MID_NAME );

                lcl_SetContentForNamedProperty(
                    m_xNamedPropertyTableFactory, u"com.sun.star.drawing.HatchTable"_ustr ,
                    aItem, MID_FILLHATCH );

                // translate model name to UI-name for predefined entries, so
                // that the correct entry is chosen in the list of UI-names
                std::unique_ptr<XFillHatchItem> pItemToPut = aItem.checkForUniqueItem( & m_rDrawModel );

                if(pItemToPut)
                    rOutItemSet.Put( std::move(pItemToPut) );
                else
                    rOutItemSet.Put(aItem);
            }
        break;

        case XATTR_FILLBITMAP:
            if( lcl_supportsFillProperties( m_GraphicObjectType ))
            {
                XFillBitmapItem aItem;
                aItem.PutValue( GetPropertySet()->getPropertyValue( u"FillBitmapName"_ustr ), MID_NAME );

                lcl_SetContentForNamedProperty(
                    m_xNamedPropertyTableFactory, u"com.sun.star.drawing.BitmapTable"_ustr ,
                    aItem, MID_BITMAP );

                // translate model name to UI-name for predefined entries, so
                // that the correct entry is chosen in the list of UI-names
                std::unique_ptr<XFillBitmapItem> pItemToPut = aItem.checkForUniqueItem( & m_rDrawModel );

                if(pItemToPut)
                    rOutItemSet.Put( std::move(pItemToPut) );
                else
                    rOutItemSet.Put(aItem);
            }
        break;

        // hack, because QueryValue of XLineTransparenceItem returns sal_Int32
        // instead of sal_Int16
        case XATTR_LINETRANSPARENCE:
        {
            OUString aPropName =
                  (m_GraphicObjectType == GraphicObjectType::FilledDataPoint)
                  ? u"BorderTransparency"_ustr
                  : (m_GraphicObjectType == GraphicObjectType::LineDataPoint)
                  ? u"Transparency"_ustr
                  : u"LineTransparence"_ustr;

            XLineTransparenceItem aItem;
            aItem.PutValue( GetPropertySet()->getPropertyValue( aPropName ), 0 );

            rOutItemSet.Put( aItem );
        }
        break;

        // hack, because QueryValue of XFillTransparenceItem returns sal_Int32
        // instead of sal_Int16
        case XATTR_FILLTRANSPARENCE:
            if( lcl_supportsFillProperties( m_GraphicObjectType ))
            {
                OUString aPropName =
                      (m_GraphicObjectType == GraphicObjectType::FilledDataPoint)
                      ? u"Transparency"_ustr
                      : u"FillTransparence"_ustr;

                XFillTransparenceItem aItem;
                aItem.PutValue( GetPropertySet()->getPropertyValue( aPropName ), 0 );

                rOutItemSet.Put( aItem );
            }
            break;
    }
}

bool GraphicPropertyItemConverter::ApplySpecialItem(
    sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
{
    bool bChanged = false;
    uno::Any aValue;

    switch( nWhichId )
    {
        // bitmap property
        case XATTR_FILLBMP_STRETCH:
            if( lcl_supportsFillProperties( m_GraphicObjectType ))
            {
                static constexpr OUString aModePropName(u"FillBitmapMode"_ustr);
                bool bStretched = rItemSet.Get( XATTR_FILLBMP_STRETCH ).GetValue();
                drawing::BitmapMode aMode =
                    (bStretched ? drawing::BitmapMode_STRETCH : drawing::BitmapMode_NO_REPEAT);
                drawing::BitmapMode aOtherMode = drawing::BitmapMode_NO_REPEAT;

                aValue <<= aMode;
                GetPropertySet()->getPropertyValue( aModePropName ) >>= aOtherMode;

                // don't overwrite if it has been set to BitmapMode_REPEAT (= tiled) already
                // XATTR_FILLBMP_STRETCH and XATTR_FILLBMP_TILE often come in pairs, tdf#104658
                if( aMode != aOtherMode && aOtherMode != drawing::BitmapMode_REPEAT )
                {
                    GetPropertySet()->setPropertyValue( aModePropName, aValue );
                    bChanged = true;
                }
            }
            break;

        case XATTR_FILLBMP_TILE:
            if( lcl_supportsFillProperties( m_GraphicObjectType ))
            {
                static constexpr OUString aModePropName(u"FillBitmapMode"_ustr);
                bool bTiled = rItemSet.Get( XATTR_FILLBMP_TILE ).GetValue();
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
                if( lcl_supportsFillProperties( m_GraphicObjectType ))
                {
                    OUString aPropName =
                          (m_GraphicObjectType == GraphicObjectType::FilledDataPoint)
                          ? u"TransparencyGradientName"_ustr
                          : u"FillTransparenceGradientName"_ustr;

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
            catch( const beans::UnknownPropertyException & )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
        break;

        case XATTR_GRADIENTSTEPCOUNT:
        {
            if( lcl_supportsFillProperties( m_GraphicObjectType ))
            {
                OUString aPropName =
                    (m_GraphicObjectType == GraphicObjectType::FilledDataPoint)
                    ? u"GradientStepCount"_ustr
                    : u"FillGradientStepCount"_ustr;

                sal_Int16 nStepCount = static_cast< const XGradientStepCountItem & >(
                            rItemSet.Get( nWhichId )).GetValue();

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
            OUString aPropName =
                (m_GraphicObjectType == GraphicObjectType::FilledDataPoint)
                ? u"BorderDashName"_ustr
                : u"LineDashName"_ustr;

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
        break;

        case XATTR_FILLGRADIENT:
        {
            if( lcl_supportsFillProperties( m_GraphicObjectType ))
            {
                OUString aPropName =
                    (m_GraphicObjectType == GraphicObjectType::FilledDataPoint)
                    ? u"GradientName"_ustr
                    : u"FillGradientName"_ustr;

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
            if( lcl_supportsFillProperties( m_GraphicObjectType ))
            {
                OUString aPropName =
                    (m_GraphicObjectType == GraphicObjectType::FilledDataPoint)
                    ? u"HatchName"_ustr
                    : u"FillHatchName"_ustr;

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
            if( lcl_supportsFillProperties( m_GraphicObjectType ))
            {
                const XFillBitmapItem & rItem =
                    static_cast< const XFillBitmapItem & >(
                        rItemSet.Get( nWhichId ));

                if( rItem.QueryValue( aValue, MID_NAME ))
                {
                    if( aValue != GetPropertySet()->getPropertyValue( u"FillBitmapName"_ustr ))
                    {
                        // add Bitmap to list
                        uno::Any aBitmap;
                        rItem.QueryValue(aBitmap, MID_BITMAP);
                        OUString aPreferredName;
                        aValue >>= aPreferredName;
                        aValue <<= PropertyHelper::addBitmapUniqueNameToTable(
                            aBitmap, m_xNamedPropertyTableFactory, aPreferredName );

                        GetPropertySet()->setPropertyValue( u"FillBitmapName"_ustr , aValue );
                        bChanged = true;
                    }
                }
            }
        }
        break;

        // hack, because QueryValue of XLineTransparenceItem returns sal_Int32
        // instead of sal_Int16
        case XATTR_LINETRANSPARENCE:
        {
            OUString aPropName =
                  (m_GraphicObjectType == GraphicObjectType::FilledDataPoint)
                  ? u"BorderTransparency"_ustr
                  : (m_GraphicObjectType == GraphicObjectType::LineDataPoint)
                  ? u"Transparency"_ustr
                  : u"LineTransparence"_ustr;

            const XLineTransparenceItem & rItem =
                static_cast< const XLineTransparenceItem & >(
                    rItemSet.Get( nWhichId ));

            if( rItem.QueryValue( aValue ))
            {
                OSL_ENSURE( ! aValue.isExtractableTo(
                                cppu::UnoType<sal_Int16>::get()),
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
            if( lcl_supportsFillProperties( m_GraphicObjectType ))
            {
                OUString aPropName =
                      (m_GraphicObjectType == GraphicObjectType::FilledDataPoint)
                      ? u"Transparency"_ustr
                      : u"FillTransparence"_ustr;

                const XFillTransparenceItem & rItem =
                    static_cast< const XFillTransparenceItem & >(
                        rItemSet.Get( nWhichId ));

                if( rItem.QueryValue( aValue ))
                {
                    OSL_ENSURE( ! aValue.isExtractableTo(
                                    cppu::UnoType<sal_Int16>::get()),
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
                              (m_GraphicObjectType == GraphicObjectType::FilledDataPoint)
                              ? u"TransparencyGradientName"_ustr
                              : u"FillTransparenceGradientName"_ustr;
                        GetPropertySet()->setPropertyValue(
                            aTransGradPropName, uno::Any( OUString() ));

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

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
