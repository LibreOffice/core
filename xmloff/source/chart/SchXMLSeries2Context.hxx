/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SchXMLSeries2Context.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:56:07 $
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
#ifndef XMLOFF_SCHXMLSERIES2CONTEXT_HXX
#define XMLOFF_SCHXMLSERIES2CONTEXT_HXX

#include "transporttypes.hxx"
#include "SchXMLChartContext.hxx"

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef _XMLOFF_SCH_XMLIMPORTHELPER_HXX_
#include <xmloff/SchXMLImportHelper.hxx>
#endif
// header for class SvXMLStyleContext
#ifndef _XMLOFF_XMLSTYLE_HXX
#include <xmloff/xmlstyle.hxx>
#endif

#include <vector>
#include <list>

namespace com { namespace sun { namespace star {
    namespace chart2 {
        class XChartDocument;
        class XDataSeries;
    }
    namespace awt {
        struct Size;
    }
}}}

// class for child contexts: series, data point and statistics objects
class SchXMLSeries2Context : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > mxNewDoc;
    ::std::vector< SchXMLAxis >& mrAxes;
    ::std::list< DataRowPointStyle >& mrStyleList;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > m_xSeries;
    sal_Int32 mnSeriesIndex;
    sal_Int32 mnDataPointIndex;
    sal_Int32& mrMaxSeriesLength;
    sal_Int32& mrNumOfLines;
    sal_Bool& mrStockHasVolume;
    ::rtl::OUString& mrFirstFirstDomainAddress;
    sal_Int32& mrFirstFirstDomainIndex;
    sal_Bool& mrAllRangeAddressesAvailable;
    SchXMLAxis* mpAttachedAxis;
    sal_Int32 mnAttachedAxis;
    ::rtl::OUString msAutoStyleName;
    ::std::vector< ::rtl::OUString > maDomainAddresses;
    ::rtl::OUString maGlobalChartTypeName;
    ::rtl::OUString maSeriesChartTypeName;
    ::rtl::OUString m_aSeriesRange;
    ::rtl::OUString m_aSeriesLabelRange;
    bool            m_bHasDomainContext;
    tSchXMLLSequencesPerIndex & mrLSequencesPerIndex;
    sal_Int32& mrCurrentDataIndex;
    tSchXMLLSequencesPerIndex maPostponedSequences;
    bool& mrGlobalChartTypeUsedBySeries;
    bool mbSymbolSizeIsMissingInFile;
    ::com::sun::star::awt::Size maChartSize;

public:
    SchXMLSeries2Context( SchXMLImportHelper& rImpHelper,
                          SvXMLImport& rImport, const rtl::OUString& rLocalName,
                          const ::com::sun::star::uno::Reference<
                              ::com::sun::star::chart2::XChartDocument > & xNewDoc,
                          std::vector< SchXMLAxis >& rAxes,
                          ::std::list< DataRowPointStyle >& rStyleList,
                          sal_Int32 nSeriesIndex,
                          sal_Int32& rMaxSeriesLength,
                          sal_Int32& rNumOfLines,
                          sal_Bool&  rStockHasVolume,
                          ::rtl::OUString& rFirstFirstDomainAddress,
                          sal_Int32& rFirstFirstDomainIndex,
                          sal_Bool&  rAllRangeAddressesAvailable,
                          const ::rtl::OUString & aGlobalChartTypeName,
                          tSchXMLLSequencesPerIndex & rLSequencesPerIndex,
                          sal_Int32& rCurrentDataIndex,
                          bool& rGlobalChartTypeUsedBySeries,
                          const ::com::sun::star::awt::Size & rChartSize );
    virtual ~SchXMLSeries2Context();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext* CreateChildContext(
        USHORT nPrefix,
        const rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    static void initSeriesPropertySets( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles
        , const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel );

    static void setDefaultsToSeries( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles );

    static void setStylesToSeries( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles
        , const SvXMLStylesContext* pStylesCtxt
        , const SvXMLStyleContext*& rpStyle
        , ::rtl::OUString& rCurrStyleName
        , SchXMLImportHelper& rImportHelper
        , const SvXMLImport& rImport
        , bool bIsStockChart
        , tSchXMLLSequencesPerIndex & rInOutLSequencesPerIndex );

    static void setStylesToStatisticsObjects( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles
        , const SvXMLStylesContext* pStylesCtxt
        , const SvXMLStyleContext*& rpStyle
        , ::rtl::OUString& rCurrStyleName );

    static void setStylesToDataPoints( SeriesDefaultsAndStyles& rSeriesDefaultsAndStyles
        , const SvXMLStylesContext* pStylesCtxt
        , const SvXMLStyleContext*& rpStyle
        , ::rtl::OUString& rCurrStyleName
        , SchXMLImportHelper& rImportHelper
        , const SvXMLImport& rImport
        , bool bIsStockChart, bool bIsDonutChart, bool bSwitchOffLinesForScatter );

    static void switchSeriesLinesOff( ::std::list< DataRowPointStyle >& rSeriesStyleList );
};

// XMLOFF_SCHXMLSERIES2CONTEXT_HXX
#endif
