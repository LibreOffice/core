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

#include "WrappedSymbolProperties.hxx"
#include "WrappedSeriesOrDiagramProperty.hxx"
#include <FastPropertyIdRanges.hxx>
#include <ChartTypeHelper.hxx>
#include <com/sun/star/chart2/Symbol.hpp>
#include <com/sun/star/chart2/SymbolStyle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart/ChartSymbolType.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <vcl/GraphicLoader.hxx>

#include <editeng/unoprnms.hxx>
#include <vcl/graph.hxx>
#include <vcl/GraphicObject.hxx>
#include <vcl/outdev.hxx>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;

namespace chart
{
namespace wrapper
{

class WrappedSymbolTypeProperty : public WrappedSeriesOrDiagramProperty< sal_Int32 >
{
public:
    virtual sal_Int32 getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const override;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const sal_Int32& aNewValue ) const override;

    virtual Any getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const override;
    virtual beans::PropertyState getPropertyState( const Reference< beans::XPropertyState >& xInnerPropertyState ) const override;

    explicit WrappedSymbolTypeProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact,
                                       tSeriesOrDiagramPropertyType ePropertyType);
};

class WrappedSymbolBitmapURLProperty : public WrappedSeriesOrDiagramProperty<OUString>
{
public:
    virtual OUString getValueFromSeries(const Reference<beans::XPropertySet>& xSeriesPropertySet) const override;
    virtual void setValueToSeries(const Reference<beans::XPropertySet> & xSeriesPropertySet, OUString const & xNewGraphicURL) const override;

    explicit WrappedSymbolBitmapURLProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact,
                                            tSeriesOrDiagramPropertyType ePropertyType);
};

class WrappedSymbolBitmapProperty : public WrappedSeriesOrDiagramProperty<uno::Reference<graphic::XGraphic>>
{
public:
    virtual uno::Reference<graphic::XGraphic> getValueFromSeries(const Reference<beans::XPropertySet>& xSeriesPropertySet) const override;
    virtual void setValueToSeries(const Reference<beans::XPropertySet> & xSeriesPropertySet, uno::Reference<graphic::XGraphic> const & xNewGraphic) const override;

    explicit WrappedSymbolBitmapProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact,
                                            tSeriesOrDiagramPropertyType ePropertyType);
};

class WrappedSymbolSizeProperty : public WrappedSeriesOrDiagramProperty< awt::Size >
{
public:
    virtual awt::Size getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const override;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const awt::Size& aNewSize ) const override;
    virtual beans::PropertyState getPropertyState( const Reference< beans::XPropertyState >& xInnerPropertyState ) const override;

    explicit WrappedSymbolSizeProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact,
                                       tSeriesOrDiagramPropertyType ePropertyType);
};

class WrappedSymbolAndLinesProperty : public WrappedSeriesOrDiagramProperty< bool >
{
public:
    virtual bool getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const override;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const bool& bDrawLines ) const override;
    virtual beans::PropertyState getPropertyState( const Reference< beans::XPropertyState >& xInnerPropertyState ) const override;

    explicit WrappedSymbolAndLinesProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact,
                                           tSeriesOrDiagramPropertyType ePropertyType);
};

namespace
{
enum
{
    //symbol properties
    PROP_CHART_SYMBOL_TYPE = FAST_PROPERTY_ID_START_CHART_SYMBOL_PROP,
    PROP_CHART_SYMBOL_BITMAP_URL,
    PROP_CHART_SYMBOL_BITMAP,
    PROP_CHART_SYMBOL_SIZE,
    PROP_CHART_SYMBOL_AND_LINES
};

sal_Int32 lcl_getSymbolType( const css::chart2::Symbol& rSymbol )
{
    sal_Int32 nSymbol = css::chart::ChartSymbolType::NONE;
    switch( rSymbol.Style )
    {
        case chart2::SymbolStyle_NONE:
            break;
        case chart2::SymbolStyle_AUTO:
            nSymbol = css::chart::ChartSymbolType::AUTO;
            break;
        case chart2::SymbolStyle_STANDARD:
            nSymbol = rSymbol.StandardSymbol%15;
            break;
        case chart2::SymbolStyle_POLYGON://new feature
            nSymbol = css::chart::ChartSymbolType::AUTO;
            break;
        case chart2::SymbolStyle_GRAPHIC:
            nSymbol = css::chart::ChartSymbolType::BITMAPURL;
            break;
        default:
            nSymbol = css::chart::ChartSymbolType::AUTO;
            break;
    }
    return nSymbol;
}
void lcl_setSymbolTypeToSymbol( sal_Int32 nSymbolType, chart2::Symbol& rSymbol )
{
    switch( nSymbolType )
    {
        case css::chart::ChartSymbolType::NONE:
            rSymbol.Style = chart2::SymbolStyle_NONE;
            break;
        case css::chart::ChartSymbolType::AUTO:
            rSymbol.Style = chart2::SymbolStyle_AUTO;
            break;
        case css::chart::ChartSymbolType::BITMAPURL:
            rSymbol.Style = chart2::SymbolStyle_GRAPHIC;
            break;
        default:
            rSymbol.Style = chart2::SymbolStyle_STANDARD;
            rSymbol.StandardSymbol = nSymbolType;
            break;
    }
}

void lcl_addWrappedProperties( std::vector< std::unique_ptr<WrappedProperty> >& rList
                                    , const std::shared_ptr< Chart2ModelContact >& spChart2ModelContact
                                    , tSeriesOrDiagramPropertyType ePropertyType )
{
    rList.emplace_back( new WrappedSymbolTypeProperty( spChart2ModelContact, ePropertyType ) );
    rList.emplace_back( new WrappedSymbolBitmapURLProperty( spChart2ModelContact, ePropertyType ) );
    rList.emplace_back( new WrappedSymbolBitmapProperty( spChart2ModelContact, ePropertyType ) );
    rList.emplace_back( new WrappedSymbolSizeProperty( spChart2ModelContact, ePropertyType  ) );
    rList.emplace_back( new WrappedSymbolAndLinesProperty( spChart2ModelContact, ePropertyType  ) );
}

}//anonymous namespace

void WrappedSymbolProperties::addProperties( std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "SymbolType",
                  PROP_CHART_SYMBOL_TYPE,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "SymbolBitmapURL",
                  PROP_CHART_SYMBOL_BITMAP_URL,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "SymbolBitmap",
                  PROP_CHART_SYMBOL_BITMAP,
                  cppu::UnoType<graphic::XGraphic>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "SymbolSize",
                  PROP_CHART_SYMBOL_SIZE,
                  cppu::UnoType<awt::Size>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "Lines",
                  PROP_CHART_SYMBOL_AND_LINES,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
}

void WrappedSymbolProperties::addWrappedPropertiesForSeries( std::vector< std::unique_ptr<WrappedProperty> >& rList
                                    , const std::shared_ptr< Chart2ModelContact >& spChart2ModelContact )
{
    lcl_addWrappedProperties( rList, spChart2ModelContact, DATA_SERIES );
}

void WrappedSymbolProperties::addWrappedPropertiesForDiagram( std::vector< std::unique_ptr<WrappedProperty> >& rList
                                    , const std::shared_ptr< Chart2ModelContact >& spChart2ModelContact )
{
    lcl_addWrappedProperties( rList, spChart2ModelContact, DIAGRAM );
}

WrappedSymbolTypeProperty::WrappedSymbolTypeProperty(
    const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedSeriesOrDiagramProperty< sal_Int32 >( "SymbolType"
            , uno::Any( css::chart::ChartSymbolType::NONE )
            , spChart2ModelContact
            , ePropertyType )
{
}

sal_Int32 WrappedSymbolTypeProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    sal_Int32 aRet = 0;
    m_aDefaultValue >>= aRet;
    chart2::Symbol aSymbol;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue("Symbol") >>= aSymbol ) )
        aRet = lcl_getSymbolType( aSymbol );
    return aRet;
}

void WrappedSymbolTypeProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, const sal_Int32& nSymbolType ) const
{
    if(!xSeriesPropertySet.is())
        return;

    chart2::Symbol aSymbol;
    xSeriesPropertySet->getPropertyValue("Symbol") >>= aSymbol;

    lcl_setSymbolTypeToSymbol( nSymbolType, aSymbol );
    xSeriesPropertySet->setPropertyValue( "Symbol", uno::Any( aSymbol ) );
}

Any WrappedSymbolTypeProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
{
    //the old chart (< OOo 2.3) needs symbol-type="automatic" at the plot-area if any of the series should be able to have symbols
    if( m_ePropertyType == DIAGRAM )
    {
        bool bHasAmbiguousValue = false;
        sal_Int32 aValue = 0;
        if( detectInnerValue( aValue, bHasAmbiguousValue ) )
        {
            if(bHasAmbiguousValue)
            {
                m_aOuterValue <<= css::chart::ChartSymbolType::AUTO;
            }
            else
            {
                if( aValue == css::chart::ChartSymbolType::NONE )
                    m_aOuterValue <<= css::chart::ChartSymbolType::NONE;
                else
                    m_aOuterValue <<= css::chart::ChartSymbolType::AUTO;
            }
        }
        return m_aOuterValue;
    }
    else
    {
        css::uno::Any aRet( m_aDefaultValue );
        aRet <<= getValueFromSeries( xInnerPropertySet );
        return aRet;
    }
}

beans::PropertyState WrappedSymbolTypeProperty::getPropertyState( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
{
    //the special situation for this property here is that the diagram default can be
    //different from the normal default and different from all singles series values
    //so we need to return PropertyState_DIRECT_VALUE for more cases

    if( m_ePropertyType == DATA_SERIES && //single series or point
        m_spChart2ModelContact.get())
    {
        Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
        Reference< chart2::XDataSeries > xSeries( xInnerPropertyState, uno::UNO_QUERY );
        Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeOfSeries( xDiagram, xSeries ) );
        if( ChartTypeHelper::isSupportingSymbolProperties( xChartType, 2 ) )
            return beans::PropertyState_DIRECT_VALUE;
    }
    return WrappedProperty::getPropertyState( xInnerPropertyState );
}

WrappedSymbolBitmapURLProperty::WrappedSymbolBitmapURLProperty(
    const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedSeriesOrDiagramProperty<OUString>("SymbolBitmapURL",
            uno::Any(OUString()), spChart2ModelContact, ePropertyType)
{
}

OUString WrappedSymbolBitmapURLProperty::getValueFromSeries(const Reference< beans::XPropertySet >& /*xSeriesPropertySet*/) const
{
    return OUString();
}

void WrappedSymbolBitmapURLProperty::setValueToSeries(
    const Reference< beans::XPropertySet >& xSeriesPropertySet,
    OUString const & xNewGraphicURL) const
{
    if (!xSeriesPropertySet.is())
        return;

    chart2::Symbol aSymbol;
    if (xSeriesPropertySet->getPropertyValue("Symbol") >>= aSymbol)
    {
        if (!xNewGraphicURL.isEmpty())
        {
            Graphic aGraphic = vcl::graphic::loadFromURL(xNewGraphicURL);
            aSymbol.Graphic.set(aGraphic.GetXGraphic());
            xSeriesPropertySet->setPropertyValue("Symbol", uno::Any(aSymbol));
        }
    }
}

WrappedSymbolBitmapProperty::WrappedSymbolBitmapProperty(
    const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedSeriesOrDiagramProperty<uno::Reference<graphic::XGraphic>>("SymbolBitmap",
            uno::Any(uno::Reference<graphic::XGraphic>()), spChart2ModelContact, ePropertyType)
{
}

uno::Reference<graphic::XGraphic> WrappedSymbolBitmapProperty::getValueFromSeries(const Reference< beans::XPropertySet >& xSeriesPropertySet) const
{
    uno::Reference<graphic::XGraphic> xGraphic;
    m_aDefaultValue >>= xGraphic;

    chart2::Symbol aSymbol;
    if (xSeriesPropertySet.is() && (xSeriesPropertySet->getPropertyValue("Symbol") >>= aSymbol)
        && aSymbol.Graphic.is())
    {
        xGraphic = aSymbol.Graphic;
    }
    return xGraphic;
}

void WrappedSymbolBitmapProperty::setValueToSeries(
    const Reference< beans::XPropertySet >& xSeriesPropertySet,
    uno::Reference<graphic::XGraphic> const & xNewGraphic) const
{
    if (!xSeriesPropertySet.is())
        return;

    chart2::Symbol aSymbol;
    if (xSeriesPropertySet->getPropertyValue("Symbol") >>= aSymbol)
    {
        if (xNewGraphic.is())
        {
            aSymbol.Graphic.set(xNewGraphic);
            xSeriesPropertySet->setPropertyValue("Symbol", uno::Any(aSymbol));
        }
    }
}

namespace
{

void lcl_correctSymbolSizeForBitmaps( chart2::Symbol& rSymbol )
{
    if( rSymbol.Style != chart2::SymbolStyle_GRAPHIC )
        return;
    if( rSymbol.Size.Width != -1 )
        return;
    if( rSymbol.Size.Height != -1 )
        return;

    //find a good automatic size
    try
    {
        const awt::Size aDefaultSize(250,250);
        awt::Size aSize = aDefaultSize;
        uno::Reference< beans::XPropertySet > xProp( rSymbol.Graphic, uno::UNO_QUERY );
        if( xProp.is() )
        {
            bool bFoundSize = false;
            try
            {
                if( xProp->getPropertyValue( "Size100thMM" ) >>= aSize )
                {
                    if( aSize.Width == 0 && aSize.Height == 0 )
                        aSize = aDefaultSize;
                    else
                        bFoundSize = true;
                }
            }
            catch( const uno::Exception& e )
            {
                SAL_WARN("chart2", "Exception caught. " << e );
            }

            if(!bFoundSize)
            {
                awt::Size aAWTPixelSize(10,10);
                if( xProp->getPropertyValue( "SizePixel" ) >>= aAWTPixelSize )
                {
                    Size aPixelSize(aAWTPixelSize.Width,aAWTPixelSize.Height);
                    Size aNewSize = OutputDevice::LogicToLogic(aPixelSize, MapMode(MapUnit::MapPixel), MapMode(MapUnit::Map100thMM));
                    aSize = awt::Size( aNewSize.Width(), aNewSize.Height() );

                    if( aSize.Width == 0 && aSize.Height == 0 )
                        aSize = aDefaultSize;
                }
            }
        }
        rSymbol.Size = aSize;
    }
    catch( const uno::Exception& e )
    {
        SAL_WARN("chart2", "Exception caught. " << e );
    }
}

}//end anonymous namespace

WrappedSymbolSizeProperty::WrappedSymbolSizeProperty(
    const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedSeriesOrDiagramProperty< awt::Size >( "SymbolSize"
            , uno::Any( awt::Size(250,250) ), spChart2ModelContact, ePropertyType  )
{
}

awt::Size WrappedSymbolSizeProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    awt::Size aRet;
    m_aDefaultValue >>= aRet;
    chart2::Symbol aSymbol;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue("Symbol") >>= aSymbol ))
        aRet = aSymbol.Size;
    return aRet;
}

void WrappedSymbolSizeProperty::setValueToSeries(
    const Reference< beans::XPropertySet >& xSeriesPropertySet,
    const awt::Size& aNewSize ) const
{
    if(!xSeriesPropertySet.is())
        return;

    chart2::Symbol aSymbol;
    if( xSeriesPropertySet->getPropertyValue("Symbol") >>= aSymbol )
    {
        aSymbol.Size = aNewSize;
        lcl_correctSymbolSizeForBitmaps(aSymbol);
        xSeriesPropertySet->setPropertyValue( "Symbol", uno::Any( aSymbol ) );
    }
}

beans::PropertyState WrappedSymbolSizeProperty::getPropertyState( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
{
    //only export symbol size if necessary
    if( m_ePropertyType == DIAGRAM )
        return beans::PropertyState_DEFAULT_VALUE;

    try
    {
        chart2::Symbol aSymbol;
        Reference< beans::XPropertySet > xSeriesPropertySet( xInnerPropertyState, uno::UNO_QUERY );
        if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue("Symbol") >>= aSymbol ))
        {
            if(  aSymbol.Style != chart2::SymbolStyle_NONE )
                return beans::PropertyState_DIRECT_VALUE;
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    return beans::PropertyState_DEFAULT_VALUE;
}

WrappedSymbolAndLinesProperty::WrappedSymbolAndLinesProperty(
    const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedSeriesOrDiagramProperty< bool >( "Lines"
            , uno::Any( true ), spChart2ModelContact, ePropertyType  )
{
}

bool WrappedSymbolAndLinesProperty::getValueFromSeries( const Reference< beans::XPropertySet >& /*xSeriesPropertySet*/ ) const
{
    //do not export this property anymore, instead use a linestyle none for no lines
    return true;
}

void WrappedSymbolAndLinesProperty::setValueToSeries(
    const Reference< beans::XPropertySet >& xSeriesPropertySet,
    const bool& bDrawLines ) const
{
    if(!xSeriesPropertySet.is())
        return;

    drawing::LineStyle eOldLineStyle( drawing::LineStyle_SOLID );
    xSeriesPropertySet->getPropertyValue( "LineStyle" ) >>= eOldLineStyle;
    if( bDrawLines )
    {
        //#i114298# don't overwrite dashed lines with solid lines here
        if( eOldLineStyle == drawing::LineStyle_NONE )
            xSeriesPropertySet->setPropertyValue( "LineStyle", uno::Any( drawing::LineStyle_SOLID ) );
    }
    else
    {
        if( eOldLineStyle != drawing::LineStyle_NONE )
            xSeriesPropertySet->setPropertyValue( "LineStyle", uno::Any( drawing::LineStyle_NONE ) );
    }
}

beans::PropertyState WrappedSymbolAndLinesProperty::getPropertyState( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    //do not export this property anymore, instead use a linestyle none for no lines
    return beans::PropertyState_DEFAULT_VALUE;
}

} //namespace wrapper
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
