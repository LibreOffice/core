/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SchXMLSeries2Context.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:55:34 $
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
#include "precompiled_xmloff.hxx"

#include "SchXMLSeries2Context.hxx"
#include "SchXMLPlotAreaContext.hxx"
#include "SchXMLSeriesHelper.hxx"
#include "SchXMLTools.hxx"
#include "PropertyMap.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XRegressionCurve.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>

#include <com/sun/star/chart/ChartAxisAssign.hpp>
#include <com/sun/star/chart/ChartSymbolType.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/chart/ChartLegendPosition.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

// header for define DBG_ERROR1
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef SCH_XMLIMPORT_HXX_
#include "SchXMLImport.hxx"
#endif
// header for class XMLPropStyleContext
#ifndef _XMLOFF_PRSTYLEI_HXX_
#include <xmloff/prstylei.hxx>
#endif
#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include <xmloff/xmlprmap.hxx>
#endif

#include <typeinfo>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ================================================================================

namespace
{

OUString lcl_ConvertRange( const ::rtl::OUString & rRange, const Reference< chart2::XChartDocument > & xDoc )
{
    OUString aResult = rRange;
    Reference< chart2::data::XRangeXMLConversion > xConversion(
        SchXMLImportHelper::GetDataProvider( xDoc ), uno::UNO_QUERY );
    if( xConversion.is())
        aResult = xConversion->convertRangeFromXML( rRange );
    return aResult;
}

class SchXMLDomain2Context : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    ::std::vector< OUString > & mrAddresses;

public:
    SchXMLDomain2Context( SchXMLImportHelper& rImpHelper,
                          SvXMLImport& rImport,
                          sal_uInt16 nPrefix,
                          const rtl::OUString& rLocalName,
                          ::std::vector< OUString > & rAddresses );
    virtual ~SchXMLDomain2Context();
    virtual void StartElement( const Reference< xml::sax::XAttributeList >& xAttrList );
};

SchXMLDomain2Context::SchXMLDomain2Context(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName,
    ::std::vector< ::rtl::OUString > & rAddresses ) :
        SvXMLImportContext( rImport, nPrefix, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrAddresses( rAddresses )
{
}

SchXMLDomain2Context::~SchXMLDomain2Context()
{
}

void SchXMLDomain2Context::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if( nPrefix == XML_NAMESPACE_TABLE &&
            IsXMLToken( aLocalName, XML_CELL_RANGE_ADDRESS ) )
        {
            Reference< chart2::XChartDocument > xNewDoc( GetImport().GetModel(), uno::UNO_QUERY );
            mrAddresses.push_back( xAttrList->getValueByIndex( i ));
        }
    }
}

void lcl_setAutomaticSymbolSize( const uno::Reference< beans::XPropertySet >& xSeriesOrPointProp, const SvXMLImport& rImport )
{
    awt::Size aSymbolSize(140,140);//old default for standard sized charts 7cm height

    double fScale = 1;
    uno::Reference< chart::XChartDocument > xChartDoc( rImport.GetModel(), uno::UNO_QUERY );
    if( xChartDoc.is() )
    {
        uno::Reference< beans::XPropertySet > xLegendProp( xChartDoc->getLegend(), uno::UNO_QUERY );
        chart::ChartLegendPosition aLegendPosition = chart::ChartLegendPosition_NONE;
        if( xLegendProp.is() && (xLegendProp->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Alignment" ))) >>= aLegendPosition)
            && chart::ChartLegendPosition_NONE != aLegendPosition )
        {

            double fFontHeight = 6.0;
            if( xLegendProp->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "CharHeight" ))) >>= fFontHeight )
                fScale = 0.75*fFontHeight/6.0;
        }
        else
        {
            uno::Reference< embed::XVisualObject > xVisualObject( rImport.GetModel(), uno::UNO_QUERY );
            if( xVisualObject.is() )
            {
                awt::Size aPageSize( xVisualObject->getVisualAreaSize( embed::Aspects::MSOLE_CONTENT ) );
                fScale = aPageSize.Height/7000.0;
            }
        }
        if( fScale>0 )
        {
            aSymbolSize.Height = static_cast<sal_Int32>( fScale * aSymbolSize.Height );
            aSymbolSize.Width = aSymbolSize.Height;
        }
    }
    xSeriesOrPointProp->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SymbolSize")),uno::makeAny( aSymbolSize ));
}

void lcl_setSymbolSizeIfNeeded( const uno::Reference< beans::XPropertySet >& xSeriesOrPointProp, const SvXMLImport& rImport )
{
    if( !xSeriesOrPointProp.is() )
        return;

    sal_Int32 nSymbolType = chart::ChartSymbolType::NONE;
    if( xSeriesOrPointProp.is() && ( xSeriesOrPointProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SymbolType"))) >>= nSymbolType) )
    {
        if(chart::ChartSymbolType::NONE!=nSymbolType)
        {
            if( chart::ChartSymbolType::BITMAPURL==nSymbolType )
            {
                //set special size for graphics to indicate to use the bitmap size itself
                xSeriesOrPointProp->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SymbolSize")),uno::makeAny( awt::Size(-1,-1) ));
            }
            else
            {
                lcl_setAutomaticSymbolSize( xSeriesOrPointProp, rImport );
            }
        }
    }
}

void lcl_resetSymbolSizeForPointsIfNecessary( const uno::Reference< beans::XPropertySet >& xPointProp, const SvXMLImport& rImport
    , const XMLPropStyleContext * pPropStyleContext, const SvXMLStylesContext* pStylesCtxt )
{
    uno::Any aASymbolSize( SchXMLTools::getPropertyFromContext( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SymbolSize")), pPropStyleContext, pStylesCtxt ) );
    if( !aASymbolSize.hasValue() )
        lcl_setSymbolSizeIfNeeded( xPointProp, rImport );
}

void lcl_insertErrorBarLSequencesToMap(
    tSchXMLLSequencesPerIndex & rInOutMap,
    const uno::Reference< beans::XPropertySet > & xSeriesProp,
    bool bYError = true )
{
    Reference< chart2::data::XDataSource > xErrorBarSource;
    const OUString aPropName(
        bYError
        ? ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ErrorBarY" ))
        : ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ErrorBarX" )));
    if( ( xSeriesProp->getPropertyValue( aPropName ) >>= xErrorBarSource ) &&
        xErrorBarSource.is() )
    {
        Sequence< Reference< chart2::data::XLabeledDataSequence > > aLSequences(
            xErrorBarSource->getDataSequences());
        for( sal_Int32 nIndex = 0; nIndex < aLSequences.getLength(); ++nIndex )
        {
            // use "0" as data index. This is ok, as it is not used for error bars
            rInOutMap.insert(
                tSchXMLLSequencesPerIndex::value_type(
                    tSchXMLIndexWithPart( 0, SCH_XML_PART_ERROR_BARS ), aLSequences[ nIndex ] ));
        }
    }
}

} // anonymous namespace

// ================================================================================

SchXMLSeries2Context::SchXMLSeries2Context(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport, const rtl::OUString& rLocalName,
    const Reference< chart2::XChartDocument > & xNewDoc,
    std::vector< SchXMLAxis >& rAxes,
    ::std::list< DataRowPointStyle >& rStyleList,
    sal_Int32 nSeriesIndex,
    sal_Int32& rMaxSeriesLength,
    sal_Int32& rNumOfLines,
    sal_Bool&  rStockHasVolume,
    ::rtl::OUString& rFirstFirstDomainAddress,
    sal_Int32& rFirstFirstDomainIndex,
    sal_Bool&  rAllRangeAddressesAvailable,
    const OUString & aGlobalChartTypeName,
    tSchXMLLSequencesPerIndex & rLSequencesPerIndex,
    sal_Int32& rCurrentDataIndex,
    bool& rGlobalChartTypeUsedBySeries,
    const awt::Size & rChartSize ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_CHART, rLocalName ),
        mrImportHelper( rImpHelper ),
        mxNewDoc( xNewDoc ),
        mrAxes( rAxes ),
        mrStyleList( rStyleList ),
        m_xSeries(0),
        mnSeriesIndex( nSeriesIndex ),
        mnDataPointIndex( 0 ),
        mrMaxSeriesLength( rMaxSeriesLength ),
        mrNumOfLines( rNumOfLines ),
        mrStockHasVolume( rStockHasVolume ),
        mrFirstFirstDomainAddress(rFirstFirstDomainAddress),
        mrFirstFirstDomainIndex(rFirstFirstDomainIndex),
        mrAllRangeAddressesAvailable( rAllRangeAddressesAvailable ),
        mpAttachedAxis( NULL ),
        maGlobalChartTypeName( aGlobalChartTypeName ),
        maSeriesChartTypeName( aGlobalChartTypeName ),
        m_bHasDomainContext(false),
        mrLSequencesPerIndex( rLSequencesPerIndex ),
        mrCurrentDataIndex( rCurrentDataIndex ),
        mrGlobalChartTypeUsedBySeries( rGlobalChartTypeUsedBySeries ),
        mbSymbolSizeIsMissingInFile(false),
        maChartSize( rChartSize )
{
    if( 0 == aGlobalChartTypeName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart2.DonutChartType" ) ) )
    {
        maSeriesChartTypeName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart2.PieChartType" ));
        maGlobalChartTypeName = maSeriesChartTypeName;
    }
}

SchXMLSeries2Context::~SchXMLSeries2Context()
{
    OSL_ASSERT( maPostponedSequences.empty());
}

void SchXMLSeries2Context::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetSeriesAttrTokenMap();
    mnAttachedAxis = 1;

    bool bHasRange = false;
    bool bHasLabelRange = false;

    Reference< chart2::data::XRangeXMLConversion > xRangeConversion;
    if( mxNewDoc.is())
        xRangeConversion.set( mrImportHelper.GetDataProvider( mxNewDoc ), uno::UNO_QUERY );

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        rtl::OUString aValue = xAttrList->getValueByIndex( i );
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_SERIES_CELL_RANGE:
                m_aSeriesRange = aValue;
                bHasRange = true;
                break;
            case XML_TOK_SERIES_LABEL_ADDRESS:
                m_aSeriesLabelRange = aValue;
                bHasLabelRange = true;
                break;
            case XML_TOK_SERIES_ATTACHED_AXIS:
                {
                    sal_Int32 nNumOfAxes = mrAxes.size();
                    for( sal_Int32 nCurrent = 0; nCurrent < nNumOfAxes; nCurrent++ )
                    {
                        if( aValue.equals( mrAxes[ nCurrent ].aName ) &&
                            mrAxes[ nCurrent ].eClass == SCH_XML_AXIS_Y )
                        {
                            mpAttachedAxis = &( mrAxes[ nCurrent ] );
                        }
                    }
                }
                break;
            case XML_TOK_SERIES_STYLE_NAME:
                msAutoStyleName = aValue;
                break;
            case XML_TOK_SERIES_CHART_CLASS:
                {
                    OUString aClassName;
                    sal_uInt16 nClassPrefix =
                        GetImport().GetNamespaceMap().GetKeyByAttrName(
                            aValue, &aClassName );
                    if( XML_NAMESPACE_CHART == nClassPrefix )
                        maSeriesChartTypeName = SchXMLTools::GetChartTypeByClassName( aClassName, false /* bUseOldNames */ );

                    if( ! maSeriesChartTypeName.getLength())
                        maSeriesChartTypeName = aClassName;
                }
                break;
        }
    }

    if( mpAttachedAxis )
    {
        if( mpAttachedAxis->nIndexInCategory > 0 )
        {
            // secondary axis => property has to be set (primary is default)
            mnAttachedAxis = 2;
        }
    }

    try
    {
        OSL_ASSERT( mxNewDoc.is());
        if( mxNewDoc.is())
        {
            if( mrAllRangeAddressesAvailable && ! bHasRange )
                mrAllRangeAddressesAvailable = sal_False;

            Reference< chart2::data::XDataProvider > xDataProvider( mrImportHelper.GetDataProvider( mxNewDoc ));
            if( xDataProvider.is())
            {
                bool bIsCandleStick = maGlobalChartTypeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart2.CandleStickChartType"));
                if( maSeriesChartTypeName.getLength() )
                {
                    bIsCandleStick = maSeriesChartTypeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart2.CandleStickChartType"));
                }
                else
                {
                    if( bIsCandleStick
                        && mrStockHasVolume
                        && mnSeriesIndex == 0 )
                    {
                        maSeriesChartTypeName = OUString::createFromAscii( "com.sun.star.chart2.ColumnChartType" );
                        bIsCandleStick = false;
                    }
                    else
                    {
                        maSeriesChartTypeName = maGlobalChartTypeName;
                    }
                }
                if( ! mrGlobalChartTypeUsedBySeries )
                    mrGlobalChartTypeUsedBySeries = (maSeriesChartTypeName.equals( maGlobalChartTypeName ));
                sal_Int32 nCoordinateSystemIndex = 0;//so far we can only import one coordinate system
                m_xSeries.set(
                    mrImportHelper.GetNewDataSeries( mxNewDoc, nCoordinateSystemIndex, maSeriesChartTypeName, ! mrGlobalChartTypeUsedBySeries ));
                Reference< chart2::data::XLabeledDataSequence > xLabeledSeq(
                    SchXMLTools::GetNewLabeledDataSequence());

                if( bIsCandleStick )
                {
                    // set default color for range-line to black (before applying styles)
                    Reference< beans::XPropertySet > xSeriesProp( m_xSeries, uno::UNO_QUERY );
                    if( xSeriesProp.is())
                        xSeriesProp->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Color")),
                                                       uno::makeAny( sal_Int32( 0x000000 ))); // black
                }
                else if( maSeriesChartTypeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart2.PieChartType")))
                {
                    //@todo: this property should be saved
                    Reference< beans::XPropertySet > xSeriesProp( m_xSeries, uno::UNO_QUERY );
                    if( xSeriesProp.is())
                        xSeriesProp->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VaryColorsByPoint")),
                                                       uno::makeAny( true ));
                }

                // values
                Reference< chart2::data::XDataSequence > xSeq;
                if( bHasRange )
                    try
                    {
                        xSeq.set( xDataProvider->createDataSequenceByRangeRepresentation( lcl_ConvertRange( m_aSeriesRange, mxNewDoc )));
                        SchXMLTools::setXMLRangePropertyAtDataSequence( xSeq, m_aSeriesRange );
                    }
                    catch( const lang::IllegalArgumentException & ex )
                    {
                        (void)ex; // avoid warning for pro build
                        OSL_ENSURE( false, ::rtl::OUStringToOString(
                                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IllegalArgumentException caught, Message: " )) +
                                        ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
                    }

                Reference< beans::XPropertySet > xSeqProp( xSeq, uno::UNO_QUERY );
                if( xSeqProp.is())
                {
                    //@todo: set correct role ("main role" dependent on chart type)
                    xSeqProp->setPropertyValue(OUString::createFromAscii("Role"),
                                               uno::makeAny( OUString::createFromAscii("values-y")));
                }
                xLabeledSeq->setValues( xSeq );

                // register for setting local data if external data provider is not present
                maPostponedSequences.insert(
                    tSchXMLLSequencesPerIndex::value_type(
                        tSchXMLIndexWithPart( mrCurrentDataIndex, SCH_XML_PART_VALUES ), xLabeledSeq ));

                // label
                if( bHasLabelRange )
                {
                    try
                    {
                        Reference< chart2::data::XDataSequence > xLabelSequence(
                            xDataProvider->createDataSequenceByRangeRepresentation(
                                lcl_ConvertRange( m_aSeriesLabelRange, mxNewDoc )));
                        xLabeledSeq->setLabel( xLabelSequence );
                        SchXMLTools::setXMLRangePropertyAtDataSequence( xLabelSequence, m_aSeriesLabelRange );
                    }
                    catch( const lang::IllegalArgumentException & ex )
                    {
                        (void)ex; // avoid warning for pro build
                        OSL_ENSURE( false, ::rtl::OUStringToOString(
                                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IllegalArgumentException caught, Message: " )) +
                                        ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
                    }
                }

                // Note: Even if we have no label, we have to register the label
                // for creation, because internal data always has labels. If
                // they don't exist in the original, auto-generated labels are
                // used for the internal data.
                maPostponedSequences.insert(
                    tSchXMLLSequencesPerIndex::value_type(
                        tSchXMLIndexWithPart( mrCurrentDataIndex, SCH_XML_PART_LABEL ), xLabeledSeq ));


                Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeq( &xLabeledSeq, 1 );
                Reference< chart2::data::XDataSink > xSink( m_xSeries, uno::UNO_QUERY_THROW );
                xSink->setData( aSeq );
            }
        }
    }
    catch( uno::Exception & ex )
    {
        (void)ex; // avoid warning for pro build
        OSL_ENSURE( false, ::rtl::OUStringToOString(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Exception caught. Type: " )) +
                        ::rtl::OUString::createFromAscii( typeid( ex ).name()) +
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ", Message: " )) +
                        ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
    }

    //init mbSymbolSizeIsMissingInFile:
    try
    {
        if( msAutoStyleName.getLength() )
        {
            const SvXMLStylesContext* pStylesCtxt = mrImportHelper.GetAutoStylesContext();
            if( pStylesCtxt )
            {
                const SvXMLStyleContext* pStyle = pStylesCtxt->FindStyleChildContext(
                    mrImportHelper.GetChartFamilyID(), msAutoStyleName );

                const XMLPropStyleContext* pPropStyleContext = dynamic_cast< const XMLPropStyleContext * >( pStyle );

                uno::Any aASymbolSize( SchXMLTools::getPropertyFromContext( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SymbolSize"))
                    , pPropStyleContext, pStylesCtxt ) );
                mbSymbolSizeIsMissingInFile = !aASymbolSize.hasValue();
            }
        }
    }
    catch( uno::Exception & ex )
    {
        (void)ex; // avoid warning for pro build
    }
}

void SchXMLSeries2Context::EndElement()
{
    // special handling for different chart types.  This is necessary as the
    // roles are not yet saved in the file format
    OUString aXValuesRange( mrFirstFirstDomainAddress );
    sal_Int32 nCurrentDataIndexBeforeDomains = mrCurrentDataIndex;
    sal_Int32 nDomainOffset = 0;
    bool bCreateXValues = false;
    bool bHasOwnDomains = false;
    bool bDeleteSeries = false;
    if( maDomainAddresses.size() == 1 ||
        maSeriesChartTypeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart2.ScatterChartType")) )
    {
        bCreateXValues = true;
        if( maDomainAddresses.size() > 0 )
        {
            aXValuesRange = maDomainAddresses.front();
            bHasOwnDomains = true;
            if(!mrFirstFirstDomainAddress.getLength())
            {
                mrFirstFirstDomainAddress=aXValuesRange;
                mrFirstFirstDomainIndex=mrCurrentDataIndex;
            }
        }
        else if( !mrFirstFirstDomainAddress.getLength() && !m_bHasDomainContext && mnSeriesIndex==0 )
        {
            sal_Int32 nBuildId = 0;
            sal_Int32 nUPD;
            if( !GetImport().getBuildIds( nUPD, nBuildId ) ) //wrong old chart files:
            {
                //for xy charts the first series needs to have a domain
                //if this by error iss not the case the first series is taken s x values
                //needed for wrong files created while having an addin (e.g. BoxPlot)
                mrFirstFirstDomainAddress = m_aSeriesRange;
                mrFirstFirstDomainIndex = mrCurrentDataIndex;
                bDeleteSeries = true;
                bCreateXValues = false;//they will be created for the next series
            }
        }
    }

    if( mrMaxSeriesLength < mnDataPointIndex )
        mrMaxSeriesLength = mnDataPointIndex;

    if( bDeleteSeries )
    {
        //delete created series
        SchXMLImportHelper::DeleteDataSeries(
            m_xSeries, Reference< chart2::XChartDocument >( GetImport().GetModel(), uno::UNO_QUERY ) );
    }
    else
    {
        //add style
        if( msAutoStyleName.getLength() ||
            mnAttachedAxis != 1 )
        {
            DataRowPointStyle aStyle(
                DataRowPointStyle::DATA_SERIES,
                m_xSeries,
                -1, 1,
                msAutoStyleName, mnAttachedAxis );
            aStyle.mbSymbolSizeForSeriesIsMissingInFile=mbSymbolSizeIsMissingInFile;
            mrStyleList.push_back( aStyle );
        }
    }

    if( bCreateXValues && aXValuesRange.getLength())
    {
        Reference< chart2::data::XDataProvider > xDataProvider( mrImportHelper.GetDataProvider( mxNewDoc ));
        if( !(m_xSeries.is() && xDataProvider.is()))
            return;

        Reference< chart2::data::XDataSource > xSeriesSource( m_xSeries,uno::UNO_QUERY );
        if( ! xSeriesSource.is())
            return;

        // assume we have a scatter chart

        // create new sequence for x-values
        Reference< chart2::data::XLabeledDataSequence > xLabeledSeq(
            SchXMLTools::GetNewLabeledDataSequence());

        // values
        Reference< chart2::data::XDataSequence > xSeq;
        try
        {
            xSeq.set( xDataProvider->createDataSequenceByRangeRepresentation( lcl_ConvertRange( aXValuesRange, mxNewDoc )));
            SchXMLTools::setXMLRangePropertyAtDataSequence( xSeq, aXValuesRange );
        }
        catch( const lang::IllegalArgumentException & ex )
        {
            (void)ex; // avoid warning for pro build
            OSL_ENSURE( false, ::rtl::OUStringToOString(
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IllegalArgumentException caught, Message: " )) +
                            ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
        }

        Reference< beans::XPropertySet > xSeqProp( xSeq, uno::UNO_QUERY );
        if( xSeqProp.is())
        {
            xSeqProp->setPropertyValue(OUString::createFromAscii("Role"),
                                       uno::makeAny( OUString::createFromAscii("values-x")));
        }
        xLabeledSeq->setValues( xSeq );

        // register for setting local data if external data provider is not present
        if( bHasOwnDomains )
        {
            mrLSequencesPerIndex.insert(
                tSchXMLLSequencesPerIndex::value_type(
                    tSchXMLIndexWithPart( mrCurrentDataIndex, SCH_XML_PART_VALUES ), xLabeledSeq ));
            ++mrCurrentDataIndex;

            nDomainOffset = mrCurrentDataIndex - nCurrentDataIndexBeforeDomains;
        }
        else
        {
            mrLSequencesPerIndex.insert(
                tSchXMLLSequencesPerIndex::value_type(
                    tSchXMLIndexWithPart( mrFirstFirstDomainIndex, SCH_XML_PART_VALUES ), xLabeledSeq ));
        }

        // @todo? export and import labels for domains?

        // add new sequence to data series
        Sequence< Reference< chart2::data::XLabeledDataSequence > > aSeq( xSeriesSource->getDataSequences());
        aSeq.realloc( aSeq.getLength() + 1 );
        aSeq[aSeq.getLength()-1] = xLabeledSeq;
        Reference< chart2::data::XDataSink > xSink( xSeriesSource, uno::UNO_QUERY );
        if( xSink.is())
            xSink->setData( aSeq );
    }

    if( !bDeleteSeries )
    {
        for( tSchXMLLSequencesPerIndex::const_iterator aIt( maPostponedSequences.begin());
            aIt != maPostponedSequences.end(); ++aIt )
        {
            sal_Int32 nNewIndex = aIt->first.first + nDomainOffset;
            mrLSequencesPerIndex.insert(
                tSchXMLLSequencesPerIndex::value_type(
                    tSchXMLIndexWithPart( nNewIndex, aIt->first.second ), aIt->second ));
        }
    }
    maPostponedSequences.clear();

    if( bHasOwnDomains )
        mrCurrentDataIndex += nDomainOffset;
    else
        ++mrCurrentDataIndex;
}

SvXMLImportContext* SchXMLSeries2Context::CreateChildContext(
    USHORT nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >&  )
{
    SvXMLImportContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = mrImportHelper.GetSeriesElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ))
    {
        case XML_TOK_SERIES_DOMAIN:
            if( m_xSeries.is())
            {
                m_bHasDomainContext = true;
                pContext = new SchXMLDomain2Context(
                    mrImportHelper, GetImport(),
                    nPrefix, rLocalName,
                    maDomainAddresses );
            }
            break;

        case XML_TOK_SERIES_MEAN_VALUE_LINE:
            pContext = new SchXMLStatisticsObjectContext(
                mrImportHelper, GetImport(),
                nPrefix, rLocalName,
                mrStyleList, m_xSeries,
                SchXMLStatisticsObjectContext::CONTEXT_TYPE_MEAN_VALUE_LINE,
                maChartSize );
            break;
        case XML_TOK_SERIES_REGRESSION_CURVE:
            pContext = new SchXMLStatisticsObjectContext(
                mrImportHelper, GetImport(),
                nPrefix, rLocalName,
                mrStyleList, m_xSeries,
                SchXMLStatisticsObjectContext::CONTEXT_TYPE_REGRESSION_CURVE,
                maChartSize );
            break;
        case XML_TOK_SERIES_ERROR_INDICATOR:
            pContext = new SchXMLStatisticsObjectContext(
                mrImportHelper, GetImport(),
                nPrefix, rLocalName,
                mrStyleList, m_xSeries,
                SchXMLStatisticsObjectContext::CONTEXT_TYPE_ERROR_INDICATOR,
                maChartSize );
            break;

        case XML_TOK_SERIES_DATA_POINT:
            pContext = new SchXMLDataPointContext( mrImportHelper, GetImport(), rLocalName,
                                                   mrStyleList, m_xSeries, mnDataPointIndex, mbSymbolSizeIsMissingInFile );
            break;

        default:
            pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

//static
void SchXMLSeries2Context::initSeriesPropertySets( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles
        , const uno::Reference< frame::XModel >& xChartModel )
{
    ::std::list< DataRowPointStyle >::iterator iStyle;

    // iterate over series first and remind propertysets in map
    // new api <-> old api wrapper
    ::std::map< Reference< chart2::XDataSeries >, Reference< beans::XPropertySet > > aSeriesMap;
    for( iStyle = rSeriesDefaultsAndStyles.maSeriesStyleList.begin(); iStyle != rSeriesDefaultsAndStyles.maSeriesStyleList.end(); iStyle++ )
    {
        if( iStyle->meType != DataRowPointStyle::DATA_SERIES )
            continue;

        if( !iStyle->m_xOldAPISeries.is() )
            iStyle->m_xOldAPISeries = SchXMLSeriesHelper::createOldAPISeriesPropertySet( iStyle->m_xSeries, xChartModel );

        aSeriesMap[iStyle->m_xSeries] = iStyle->m_xOldAPISeries;

    }

    //initialize m_xOldAPISeries for all other styles also
    for( iStyle = rSeriesDefaultsAndStyles.maSeriesStyleList.begin(); iStyle != rSeriesDefaultsAndStyles.maSeriesStyleList.end(); iStyle++ )
    {
        if( iStyle->meType == DataRowPointStyle::DATA_SERIES )
            continue;
        iStyle->m_xOldAPISeries = aSeriesMap[iStyle->m_xSeries];
    }
}

//static
void SchXMLSeries2Context::setDefaultsToSeries( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles )
{
    ::std::list< DataRowPointStyle >::iterator iStyle;
    // iterate over series
    // call initSeriesPropertySets first

    for( iStyle = rSeriesDefaultsAndStyles.maSeriesStyleList.begin(); iStyle != rSeriesDefaultsAndStyles.maSeriesStyleList.end(); iStyle++ )
    {
        if( iStyle->meType != DataRowPointStyle::DATA_SERIES )
            continue;

        try
        {
            uno::Reference< beans::XPropertySet > xSeries( iStyle->m_xOldAPISeries );
            if( !xSeries.is() )
                continue;

            if( rSeriesDefaultsAndStyles.maSymbolTypeDefault.hasValue() )
                xSeries->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SymbolType")),rSeriesDefaultsAndStyles.maSymbolTypeDefault);
            if( rSeriesDefaultsAndStyles.maDataCaptionDefault.hasValue() )
                xSeries->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataCaption")),rSeriesDefaultsAndStyles.maDataCaptionDefault);

            if( rSeriesDefaultsAndStyles.maErrorIndicatorDefault.hasValue() )
                xSeries->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ErrorIndicator")),rSeriesDefaultsAndStyles.maErrorIndicatorDefault);
            if( rSeriesDefaultsAndStyles.maErrorCategoryDefault.hasValue() )
                xSeries->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ErrorCategory")),rSeriesDefaultsAndStyles.maErrorCategoryDefault);
            if( rSeriesDefaultsAndStyles.maConstantErrorLowDefault.hasValue() )
                xSeries->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ConstantErrorLow")),rSeriesDefaultsAndStyles.maConstantErrorLowDefault);
            if( rSeriesDefaultsAndStyles.maConstantErrorHighDefault.hasValue() )
                xSeries->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ConstantErrorHigh")),rSeriesDefaultsAndStyles.maConstantErrorHighDefault);
            if( rSeriesDefaultsAndStyles.maPercentageErrorDefault.hasValue() )
                xSeries->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PercentageError")),rSeriesDefaultsAndStyles.maPercentageErrorDefault);
            if( rSeriesDefaultsAndStyles.maErrorMarginDefault.hasValue() )
                xSeries->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ErrorMargin")),rSeriesDefaultsAndStyles.maErrorMarginDefault);

            if( rSeriesDefaultsAndStyles.maMeanValueDefault.hasValue() )
                xSeries->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MeanValue")),rSeriesDefaultsAndStyles.maMeanValueDefault);
            if( rSeriesDefaultsAndStyles.maRegressionCurvesDefault.hasValue() )
                xSeries->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RegressionCurves")),rSeriesDefaultsAndStyles.maRegressionCurvesDefault);
        }
        catch( uno::Exception &  )
        {
            //end of series reached
        }
    }
}

//static
void SchXMLSeries2Context::setStylesToSeries( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles
        , const SvXMLStylesContext* pStylesCtxt
        , const SvXMLStyleContext*& rpStyle
        , ::rtl::OUString& rCurrStyleName
        , SchXMLImportHelper& rImportHelper
        , const SvXMLImport& rImport
        , bool bIsStockChart
        , tSchXMLLSequencesPerIndex & rInOutLSequencesPerIndex )
{
    ::std::list< DataRowPointStyle >::iterator iStyle;

    // iterate over series
    for( iStyle = rSeriesDefaultsAndStyles.maSeriesStyleList.begin(); iStyle != rSeriesDefaultsAndStyles.maSeriesStyleList.end(); iStyle++ )
    {
        if( iStyle->meType == DataRowPointStyle::DATA_SERIES )
        {
            try
            {
                uno::Reference< beans::XPropertySet > xSeriesProp( iStyle->m_xOldAPISeries );
                if( !xSeriesProp.is() )
                    continue;

                if( iStyle->mnAttachedAxis != 1 )
                {
                    xSeriesProp->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Axis" ))
                        , uno::makeAny(chart::ChartAxisAssign::SECONDARY_Y) );
                }

                if( (iStyle->msStyleName).getLength())
                {
                    if( ! rCurrStyleName.equals( iStyle->msStyleName ))
                    {
                        rCurrStyleName = iStyle->msStyleName;
                        rpStyle = pStylesCtxt->FindStyleChildContext(
                            rImportHelper.GetChartFamilyID(), rCurrStyleName );
                    }

                    //set style to series
                    // note: SvXMLStyleContext::FillPropertySet is not const
                    XMLPropStyleContext * pPropStyleContext =
                        const_cast< XMLPropStyleContext * >(
                            dynamic_cast< const XMLPropStyleContext * >( rpStyle ));
                    if( pPropStyleContext )
                    {
                        // error bar style must be set before the other error
                        // bar properties (which may be alphabetically before
                        // this property)
                        bool bHasErrorBarRangesFromData = false;
                        {
                            const ::rtl::OUString aErrorBarStylePropName( RTL_CONSTASCII_USTRINGPARAM("ErrorBarStyle"));
                            uno::Any aErrorBarStyle(
                                SchXMLTools::getPropertyFromContext( aErrorBarStylePropName, pPropStyleContext, pStylesCtxt ));
                            if( aErrorBarStyle.hasValue())
                            {
                                xSeriesProp->setPropertyValue( aErrorBarStylePropName, aErrorBarStyle );
                                sal_Int32 eEBStyle = chart::ErrorBarStyle::NONE;
                                bHasErrorBarRangesFromData =
                                    ( ( aErrorBarStyle >>= eEBStyle ) &&
                                      eEBStyle == chart::ErrorBarStyle::FROM_DATA );
                            }
                        }

                        //don't set the style to the min max line series of a stock chart
                        //otherwise the min max line properties gets overwritten and the series becomes invisible typically
                        bool bIsMinMaxSeries = false;
                        if( bIsStockChart )
                        {
                            if( SchXMLSeriesHelper::isCandleStickSeries( iStyle->m_xSeries
                                    , uno::Reference< frame::XModel >( rImportHelper.GetChartDocument(), uno::UNO_QUERY ) ) )
                                bIsMinMaxSeries = true;
                        }
                        if( !bIsMinMaxSeries )
                        {
                            pPropStyleContext->FillPropertySet( xSeriesProp );
                            if( iStyle->mbSymbolSizeForSeriesIsMissingInFile )
                                lcl_setSymbolSizeIfNeeded( xSeriesProp, rImport );
                            if( bHasErrorBarRangesFromData )
                                lcl_insertErrorBarLSequencesToMap( rInOutLSequencesPerIndex, xSeriesProp );
                        }
                    }
                }
            }
            catch( uno::Exception & rEx )
            {
                (void)rEx; // avoid warning for pro build
                DBG_ERROR1( "Exception caught during setting styles to series: %s",
                                OUStringToOString( rEx.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
            }
        }
    }
}

// static
void SchXMLSeries2Context::setStylesToStatisticsObjects( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles
        , const SvXMLStylesContext* pStylesCtxt
        , const SvXMLStyleContext*& rpStyle
        , ::rtl::OUString& rCurrStyleName )
{
    ::std::list< DataRowPointStyle >::iterator iStyle;

    // iterate over regession etc
    for( iStyle = rSeriesDefaultsAndStyles.maSeriesStyleList.begin(); iStyle != rSeriesDefaultsAndStyles.maSeriesStyleList.end(); iStyle++ )
    {
        if( iStyle->meType == DataRowPointStyle::REGRESSION ||
            iStyle->meType == DataRowPointStyle::ERROR_INDICATOR ||
            iStyle->meType == DataRowPointStyle::MEAN_VALUE )
        {
            try
            {
                uno::Reference< beans::XPropertySet > xSeriesProp( iStyle->m_xOldAPISeries );
                if( !xSeriesProp.is() )
                    continue;

                if( (iStyle->msStyleName).getLength())
                {
                    if( ! rCurrStyleName.equals( iStyle->msStyleName ))
                    {
                        rCurrStyleName = iStyle->msStyleName;
                        rpStyle = pStylesCtxt->FindStyleChildContext(
                            SchXMLImportHelper::GetChartFamilyID(), rCurrStyleName );
                    }

                    // note: SvXMLStyleContext::FillPropertySet is not const
                    XMLPropStyleContext * pPropStyleContext =
                        const_cast< XMLPropStyleContext * >(
                            dynamic_cast< const XMLPropStyleContext * >( rpStyle ));
                    if( pPropStyleContext )
                    {
                        Reference< beans::XPropertySet > xStatPropSet;
                        switch( iStyle->meType )
                        {
                            case DataRowPointStyle::MEAN_VALUE:
                                xSeriesProp->getPropertyValue(
                                    OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                  "DataMeanValueProperties" ))) >>= xStatPropSet;
                                break;
                            case DataRowPointStyle::REGRESSION:
                                xSeriesProp->getPropertyValue(
                                    OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                  "DataRegressionProperties" ))) >>= xStatPropSet;
                                break;
                            case DataRowPointStyle::ERROR_INDICATOR:
                                xSeriesProp->getPropertyValue(
                                    OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                  "DataErrorProperties" )))  >>= xStatPropSet;
                                break;
                            default:
                                break;
                        }
                        if( xStatPropSet.is())
                            pPropStyleContext->FillPropertySet( xStatPropSet );
                    }
                }

                // set equation properties at a regression curve
                // note: this must be done after setting the regression
                // properties at the old API, otherwise the curve itself does
                // not exist here
                if( iStyle->meType == DataRowPointStyle::REGRESSION && iStyle->m_xEquationProperties.is())
                {
                    OSL_ASSERT( iStyle->m_xSeries.is());
                    Reference< chart2::XRegressionCurve > xRegCurve( SchXMLTools::getRegressionCurve( iStyle->m_xSeries ));
                    if( xRegCurve.is())
                        xRegCurve->setEquationProperties( iStyle->m_xEquationProperties );
                }
            }
            catch( uno::Exception & rEx )
            {
                (void)rEx; // avoid warning for pro build
                DBG_ERROR1( "Exception caught during setting styles to series: %s",
                                OUStringToOString( rEx.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
            }
        }
    }
}

//static
void SchXMLSeries2Context::setStylesToDataPoints( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles
        , const SvXMLStylesContext* pStylesCtxt
        , const SvXMLStyleContext*& rpStyle
        , ::rtl::OUString& rCurrStyleName
        , SchXMLImportHelper& rImportHelper
        , const SvXMLImport& rImport
        , bool bIsStockChart, bool bIsDonutChart, bool bSwitchOffLinesForScatter )
{
    ::std::list< DataRowPointStyle >::iterator iStyle;
    for( iStyle = rSeriesDefaultsAndStyles.maSeriesStyleList.begin(); iStyle != rSeriesDefaultsAndStyles.maSeriesStyleList.end(); iStyle++ )
    {
        if( iStyle->meType != DataRowPointStyle::DATA_POINT )
            continue;

        if( iStyle->m_nPointIndex == -1 )
            continue;

        //ignore datapoint properties for stock charts
        //... todo ...
        if( bIsStockChart )
        {
            if( SchXMLSeriesHelper::isCandleStickSeries( iStyle->m_xSeries, uno::Reference< frame::XModel >( rImportHelper.GetChartDocument(), uno::UNO_QUERY ) ) )
                continue;
        }

        // data point style
        for( sal_Int32 i = 0; i < iStyle->m_nPointRepeat; i++ )
        {
            try
            {
                uno::Reference< beans::XPropertySet > xSeriesProp( iStyle->m_xOldAPISeries );
                if(!xSeriesProp.is())
                    continue;

                uno::Reference< beans::XPropertySet > xPointProp(
                    SchXMLSeriesHelper::createOldAPIDataPointPropertySet( iStyle->m_xSeries, iStyle->m_nPointIndex + i
                        , uno::Reference< frame::XModel >( rImportHelper.GetChartDocument(), uno::UNO_QUERY ) ) );

                if( !xPointProp.is() )
                    continue;

                if( bIsDonutChart )
                {
                    //set special series styles for donut charts first
                    if( !rCurrStyleName.equals( iStyle->msSeriesStyleNameForDonuts ) )
                    {
                        rCurrStyleName = iStyle->msSeriesStyleNameForDonuts;
                        rpStyle = pStylesCtxt->FindStyleChildContext(
                            rImportHelper.GetChartFamilyID(), rCurrStyleName );
                    }

                    // note: SvXMLStyleContext::FillPropertySet is not const
                    XMLPropStyleContext * pPropStyleContext =
                        const_cast< XMLPropStyleContext * >(
                            dynamic_cast< const XMLPropStyleContext * >( rpStyle ));
                    if( pPropStyleContext )
                        pPropStyleContext->FillPropertySet( xPointProp );
                }

                try
                {
                    //need to set this explicitely here for old files as the new api does not support this property fully anymore
                    if( bSwitchOffLinesForScatter )
                        xPointProp->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Lines")),uno::makeAny(sal_False));
                }
                catch( uno::Exception & rEx )
                {
                    (void)rEx; // avoid warning for pro build
                }

                if( !rCurrStyleName.equals( iStyle->msStyleName ) )
                {
                    rCurrStyleName = iStyle->msStyleName;
                    rpStyle = pStylesCtxt->FindStyleChildContext(
                        rImportHelper.GetChartFamilyID(), rCurrStyleName );
                }

                // note: SvXMLStyleContext::FillPropertySet is not const
                XMLPropStyleContext * pPropStyleContext =
                    const_cast< XMLPropStyleContext * >(
                        dynamic_cast< const XMLPropStyleContext * >( rpStyle ));
                if( pPropStyleContext )
                {
                    pPropStyleContext->FillPropertySet( xPointProp );
                    if( iStyle->mbSymbolSizeForSeriesIsMissingInFile )
                        lcl_resetSymbolSizeForPointsIfNecessary( xPointProp, rImport, pPropStyleContext, pStylesCtxt );
                }
            }
            catch( uno::Exception & rEx )
            {
                (void)rEx; // avoid warning for pro build
                DBG_ERROR1( "Exception caught during setting styles to data points: %s",
                                    OUStringToOString( rEx.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
            }
        }
    }   // styles iterator
}

//static
void SchXMLSeries2Context::switchSeriesLinesOff( ::std::list< DataRowPointStyle >& rSeriesStyleList )
{
    ::std::list< DataRowPointStyle >::iterator iStyle;
    // iterate over series

    for( iStyle = rSeriesStyleList.begin(); iStyle != rSeriesStyleList.end(); iStyle++ )
    {
        if( iStyle->meType != DataRowPointStyle::DATA_SERIES )
            continue;

        try
        {
            uno::Reference< beans::XPropertySet > xSeries( iStyle->m_xOldAPISeries );
            if( !xSeries.is() )
                continue;

            xSeries->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Lines")),uno::makeAny(sal_False));
        }
        catch( uno::Exception &  )
        {
            //end of series reached
        }
    }
}
