/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WrappedSymbolProperties.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:20:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "WrappedSymbolProperties.hxx"
#include "WrappedSeriesOrDiagramProperty.hxx"
#include "macros.hxx"
#include "FastPropertyIdRanges.hxx"
#include "ChartTypeHelper.hxx"

#ifndef _COM_SUN_STAR_CHART2_SYMBOL_HPP_
#include <com/sun/star/chart2/Symbol.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_SYMBOLSTYLE_HPP_
#include <com/sun/star/chart2/SymbolStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTSYMBOLTYPE_HPP_
#include <com/sun/star/chart/ChartSymbolType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif

#include <com/sun/star/graphic/XGraphicProvider.hpp>

// for UNO_NAME_GRAPHOBJ_URLPREFIX
#include <svx/unoprnms.hxx>

// for Graphic
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
// for GraphicObject
#ifndef _GRFMGR_HXX
#include <goodies/grfmgr.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;
using ::rtl::OUString;

//.............................................................................
namespace chart
{
namespace wrapper
{


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class WrappedSymbolTypeProperty : public WrappedSeriesOrDiagramProperty< sal_Int32 >
{
public:
    virtual sal_Int32 getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, sal_Int32 aNewValue ) const;

    virtual Any getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                            throw ( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException);
    virtual beans::PropertyState getPropertyState( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, uno::RuntimeException);

    explicit WrappedSymbolTypeProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                        tSeriesOrDiagramPropertyType ePropertyType );
    virtual ~WrappedSymbolTypeProperty();
};

class WrappedSymbolBitmapURLProperty : public WrappedSeriesOrDiagramProperty< OUString >
{
public:
    virtual OUString getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, OUString aNewGraphicURL ) const;

    explicit WrappedSymbolBitmapURLProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                             tSeriesOrDiagramPropertyType ePropertyType );
    virtual ~WrappedSymbolBitmapURLProperty();
};

// this operator is not defined by default
bool operator!=( const awt::Size & rSize1, const awt::Size & rSize2 )
{
    return (rSize1.Width != rSize2.Width) || (rSize1.Height != rSize2.Height);
}

class WrappedSymbolSizeProperty : public WrappedSeriesOrDiagramProperty< awt::Size >
{
public:
    virtual awt::Size getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, awt::Size aNewSize ) const;
    virtual beans::PropertyState getPropertyState( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, uno::RuntimeException);

    explicit WrappedSymbolSizeProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                        tSeriesOrDiagramPropertyType ePropertyType );
    virtual ~WrappedSymbolSizeProperty();
};

class WrappedSymbolAndLinesProperty : public WrappedSeriesOrDiagramProperty< sal_Bool >
{
public:
    virtual sal_Bool getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const;
    virtual void setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, sal_Bool bDrawLines ) const;
    virtual beans::PropertyState getPropertyState( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, uno::RuntimeException);

    explicit WrappedSymbolAndLinesProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
                                            tSeriesOrDiagramPropertyType ePropertyType );
    virtual ~WrappedSymbolAndLinesProperty();
};

namespace
{
enum
{
    //symbol properties
    PROP_CHART_SYMBOL_TYPE = FAST_PROPERTY_ID_START_CHART_SYMBOL_PROP,
    PROP_CHART_SYMBOL_BITMAP_URL,
    PROP_CHART_SYMBOL_SIZE,
    PROP_CHART_SYMBOL_AND_LINES
};

sal_Int32 lcl_getSymbolType( const ::com::sun::star::chart2::Symbol& rSymbol )
{
    sal_Int32 nSymbol = ::com::sun::star::chart::ChartSymbolType::NONE;
    switch( rSymbol.Style )
    {
        case chart2::SymbolStyle_NONE:
            break;
        case chart2::SymbolStyle_AUTO:
            nSymbol = ::com::sun::star::chart::ChartSymbolType::AUTO;
            break;
        case chart2::SymbolStyle_STANDARD:
            nSymbol = rSymbol.StandardSymbol%8;
            break;
        case chart2::SymbolStyle_POLYGON://new feature
            nSymbol = ::com::sun::star::chart::ChartSymbolType::AUTO;
            break;
        case chart2::SymbolStyle_GRAPHIC:
            nSymbol = ::com::sun::star::chart::ChartSymbolType::BITMAPURL;
            break;
        default:
            nSymbol = ::com::sun::star::chart::ChartSymbolType::AUTO;
            break;
    }
    return nSymbol;
}
void lcl_setSymbolTypeToSymbol( sal_Int32 nSymbolType, chart2::Symbol& rSymbol )
{
    switch( nSymbolType )
    {
        case ::com::sun::star::chart::ChartSymbolType::NONE:
            rSymbol.Style = chart2::SymbolStyle_NONE;
            break;
        case ::com::sun::star::chart::ChartSymbolType::AUTO:
            rSymbol.Style = chart2::SymbolStyle_AUTO;
            break;
        case ::com::sun::star::chart::ChartSymbolType::BITMAPURL:
            rSymbol.Style = chart2::SymbolStyle_GRAPHIC;
            break;
        default:
            rSymbol.Style = chart2::SymbolStyle_STANDARD;
            rSymbol.StandardSymbol = nSymbolType;
            break;
    }
}

void lcl_addWrappedProperties( std::vector< WrappedProperty* >& rList
                                    , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact
                                    , tSeriesOrDiagramPropertyType ePropertyType )
{
    rList.push_back( new WrappedSymbolTypeProperty( spChart2ModelContact, ePropertyType ) );
    rList.push_back( new WrappedSymbolBitmapURLProperty( spChart2ModelContact, ePropertyType ) );
    rList.push_back( new WrappedSymbolSizeProperty( spChart2ModelContact, ePropertyType  ) );
    rList.push_back( new WrappedSymbolAndLinesProperty( spChart2ModelContact, ePropertyType  ) );
}

}//anonymous namespace

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//static
void WrappedSymbolProperties::addProperties( ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "SymbolType" ),
                  PROP_CHART_SYMBOL_TYPE,
                  ::getCppuType( reinterpret_cast< sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "SymbolBitmapURL" ),
                  PROP_CHART_SYMBOL_BITMAP_URL,
                  ::getCppuType( reinterpret_cast< ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "SymbolSize" ),
                  PROP_CHART_SYMBOL_SIZE,
                  ::getCppuType( reinterpret_cast< awt::Size * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "Lines" ),
                  PROP_CHART_SYMBOL_AND_LINES,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//static
void WrappedSymbolProperties::addWrappedPropertiesForSeries( std::vector< WrappedProperty* >& rList
                                    , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
{
    lcl_addWrappedProperties( rList, spChart2ModelContact, DATA_SERIES );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//static
void WrappedSymbolProperties::addWrappedPropertiesForDiagram( std::vector< WrappedProperty* >& rList
                                    , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
{
    lcl_addWrappedProperties( rList, spChart2ModelContact, DIAGRAM );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

WrappedSymbolTypeProperty::WrappedSymbolTypeProperty(
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedSeriesOrDiagramProperty< sal_Int32 >( C2U("SymbolType")
            , uno::makeAny( ::com::sun::star::chart::ChartSymbolType::NONE )
            , spChart2ModelContact
            , ePropertyType )
{
}
WrappedSymbolTypeProperty::~WrappedSymbolTypeProperty()
{
}

sal_Int32 WrappedSymbolTypeProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    sal_Int32 aRet = 0;
    m_aDefaultValue >>= aRet;
    chart2::Symbol aSymbol;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue(C2U("Symbol")) >>= aSymbol ) )
        aRet = lcl_getSymbolType( aSymbol );
    return aRet;
}

void WrappedSymbolTypeProperty::setValueToSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet, sal_Int32 nSymbolType ) const
{
    if(!xSeriesPropertySet.is())
        return;

    chart2::Symbol aSymbol;
    xSeriesPropertySet->getPropertyValue(C2U("Symbol")) >>= aSymbol;

    lcl_setSymbolTypeToSymbol( nSymbolType, aSymbol );
    xSeriesPropertySet->setPropertyValue( C2U("Symbol"), uno::makeAny( aSymbol ) );
}

Any WrappedSymbolTypeProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                            throw ( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
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
                m_aOuterValue = uno::makeAny( ::com::sun::star::chart::ChartSymbolType::AUTO );
            }
            else
            {
                if( ::com::sun::star::chart::ChartSymbolType::NONE == aValue )
                    m_aOuterValue = uno::makeAny( ::com::sun::star::chart::ChartSymbolType::NONE );
                else
                    m_aOuterValue = uno::makeAny( ::com::sun::star::chart::ChartSymbolType::AUTO );
            }
        }
        return m_aOuterValue;
    }
    else
    {
        ::com::sun::star::uno::Any aRet( m_aDefaultValue );
        aRet <<= getValueFromSeries( xInnerPropertySet );
        return aRet;
    }
}

beans::PropertyState WrappedSymbolTypeProperty::getPropertyState( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    //the special situation for this property here is that the diagram default can be
    //different from the normal default and different from all sinlges series values
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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

WrappedSymbolBitmapURLProperty::WrappedSymbolBitmapURLProperty(
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedSeriesOrDiagramProperty< OUString >( C2U("SymbolBitmapURL")
            , uno::makeAny( OUString() ), spChart2ModelContact, ePropertyType  )
{
}

WrappedSymbolBitmapURLProperty::~WrappedSymbolBitmapURLProperty()
{
}

OUString WrappedSymbolBitmapURLProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    OUString aRet;
    m_aDefaultValue >>= aRet;
    chart2::Symbol aSymbol;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue(C2U("Symbol")) >>= aSymbol )
        && aSymbol.Graphic.is())
    {
        GraphicObject aGrObj( Graphic( aSymbol.Graphic ));
        aRet = OUString( RTL_CONSTASCII_USTRINGPARAM( UNO_NAME_GRAPHOBJ_URLPREFIX ));
        aRet += OUString::createFromAscii( aGrObj.GetUniqueID().GetBuffer());
    }
    return aRet;
}

void WrappedSymbolBitmapURLProperty::setValueToSeries(
    const Reference< beans::XPropertySet >& xSeriesPropertySet,
    OUString aNewGraphicURL ) const
{
    if(!xSeriesPropertySet.is())
        return;

    chart2::Symbol aSymbol;
    if( xSeriesPropertySet->getPropertyValue(C2U("Symbol")) >>= aSymbol )
    {
        bool bMatchesPrefix =
            aNewGraphicURL.matchAsciiL( RTL_CONSTASCII_STRINGPARAM( UNO_NAME_GRAPHOBJ_URLPREFIX ));
        OSL_ENSURE( bMatchesPrefix, "Invalid URL for Symbol Bitmap" );
        if( bMatchesPrefix )
        {
            GraphicObject aGrObj = GraphicObject(
                ByteString( U2C( aNewGraphicURL.copy( sizeof( UNO_NAME_GRAPHOBJ_URLPREFIX ) - 1 ))));
            aSymbol.Graphic.set( aGrObj.GetGraphic().GetXGraphic());
            xSeriesPropertySet->setPropertyValue( C2U("Symbol"), uno::makeAny( aSymbol ) );
        }
        else
        {
            try
            {
                // @todo: get factory from some context?
                Reference< lang::XMultiServiceFactory > xFact( comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
                Reference< graphic::XGraphicProvider > xGraphProv(
                    xFact->createInstance( C2U("com.sun.star.graphic.GraphicProvider")), uno::UNO_QUERY_THROW );
                Sequence< beans::PropertyValue > aArgs(1);
                aArgs[0] = beans::PropertyValue(
                    C2U("URL"), -1, uno::makeAny( aNewGraphicURL ),
                    beans::PropertyState_DIRECT_VALUE );
                aSymbol.Graphic.set( xGraphProv->queryGraphic( aArgs ));
                xSeriesPropertySet->setPropertyValue( C2U("Symbol"), uno::makeAny( aSymbol ) );
            }
            catch( const uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

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
                if( (xProp->getPropertyValue( C2U( "Size100thMM" ) ) >>= aSize) )
                {
                    if( aSize.Width == 0 && aSize.Height == 0 )
                        aSize = aDefaultSize;
                    else
                        bFoundSize = true;
                }
            }
            catch( uno::Exception& e )
            {
                ASSERT_EXCEPTION( e );
            }

            if(!bFoundSize)
            {
                awt::Size aAWTPixelSize(10,10);
                if(  (xProp->getPropertyValue( C2U( "SizePixel" ) ) >>= aAWTPixelSize) )
                {
                    Size aPixelSize(aAWTPixelSize.Width,aAWTPixelSize.Height);
                    Size aNewSize = ( OutputDevice::LogicToLogic( aPixelSize, MAP_PIXEL, MAP_100TH_MM ));
                    aSize = awt::Size( aNewSize.Width(), aNewSize.Height() );

                    if( aSize.Width == 0 && aSize.Height == 0 )
                        aSize = aDefaultSize;
                }
            }
        }
        rSymbol.Size = aSize;
    }
    catch( uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
}

}//end anonymous namespace

WrappedSymbolSizeProperty::WrappedSymbolSizeProperty(
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedSeriesOrDiagramProperty< awt::Size >( C2U("SymbolSize")
            , uno::makeAny( awt::Size(250,250) ), spChart2ModelContact, ePropertyType  )
{
}

WrappedSymbolSizeProperty::~WrappedSymbolSizeProperty()
{
}

awt::Size WrappedSymbolSizeProperty::getValueFromSeries( const Reference< beans::XPropertySet >& xSeriesPropertySet ) const
{
    awt::Size aRet;
    m_aDefaultValue >>= aRet;
    chart2::Symbol aSymbol;
    if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue(C2U("Symbol")) >>= aSymbol ))
        aRet = aSymbol.Size;
    return aRet;
}

void WrappedSymbolSizeProperty::setValueToSeries(
    const Reference< beans::XPropertySet >& xSeriesPropertySet,
    awt::Size aNewSize ) const
{
    if(!xSeriesPropertySet.is())
        return;

    chart2::Symbol aSymbol;
    if( xSeriesPropertySet->getPropertyValue(C2U("Symbol")) >>= aSymbol )
    {
        aSymbol.Size = aNewSize;
        lcl_correctSymbolSizeForBitmaps(aSymbol);
        xSeriesPropertySet->setPropertyValue( C2U("Symbol"), uno::makeAny( aSymbol ) );
    }
}

beans::PropertyState WrappedSymbolSizeProperty::getPropertyState( const Reference< beans::XPropertyState >& xInnerPropertyState ) const
                        throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    //only export symbol size if necessary
    if( m_ePropertyType == DIAGRAM )
        return beans::PropertyState_DEFAULT_VALUE;

    try
    {
        chart2::Symbol aSymbol;
        Reference< beans::XPropertySet > xSeriesPropertySet( xInnerPropertyState, uno::UNO_QUERY );
        if( xSeriesPropertySet.is() && ( xSeriesPropertySet->getPropertyValue(C2U("Symbol")) >>= aSymbol ))
        {
            if(  chart2::SymbolStyle_NONE != aSymbol.Style )
                return beans::PropertyState_DIRECT_VALUE;
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return beans::PropertyState_DEFAULT_VALUE;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

WrappedSymbolAndLinesProperty::WrappedSymbolAndLinesProperty(
    ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact,
    tSeriesOrDiagramPropertyType ePropertyType )
        : WrappedSeriesOrDiagramProperty< sal_Bool >( C2U("Lines")
            , uno::makeAny( sal_True ), spChart2ModelContact, ePropertyType  )
{
}

WrappedSymbolAndLinesProperty::~WrappedSymbolAndLinesProperty()
{
}

sal_Bool WrappedSymbolAndLinesProperty::getValueFromSeries( const Reference< beans::XPropertySet >& /*xSeriesPropertySet*/ ) const
{
    //do not export this property anymore, instead use a linestyle none for no lines
    return sal_True;
}

void WrappedSymbolAndLinesProperty::setValueToSeries(
    const Reference< beans::XPropertySet >& xSeriesPropertySet,
    sal_Bool bDrawLines ) const
{
    if(!xSeriesPropertySet.is())
        return;

    drawing::LineStyle eLineStyle( bDrawLines ? drawing::LineStyle_SOLID : drawing::LineStyle_NONE  );
    xSeriesPropertySet->setPropertyValue( C2U("LineStyle"), uno::makeAny( eLineStyle ) );
}

beans::PropertyState WrappedSymbolAndLinesProperty::getPropertyState( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    //do not export this property anymore, instead use a linestyle none for no lines
    return beans::PropertyState_DEFAULT_VALUE;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

} //namespace wrapper
} //namespace chart
//.............................................................................
