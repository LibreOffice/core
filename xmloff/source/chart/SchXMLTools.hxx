/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SchXMLTools.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 11:38:23 $
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
#ifndef SCH_XML_TOOLS_HXX_
#define SCH_XML_TOOLS_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef SCH_XML_TRANSPORTTYPES_HXX_
#include "transporttypes.hxx"
#endif

namespace com { namespace sun { namespace star {
    namespace chart2 {
        class XChartDocument;
        class XRegressionCurve;
        namespace data {
            class XDataProvider;
            class XLabeledDataSequence;
        }
    }
}}}

class XMLPropStyleContext;
class SvXMLStylesContext;
class SvXMLExport;

namespace SchXMLTools
{
    enum SchXMLChartTypeEnum
    {
        XML_CHART_CLASS_LINE,
        XML_CHART_CLASS_AREA,
        XML_CHART_CLASS_CIRCLE,
        XML_CHART_CLASS_RING,
        XML_CHART_CLASS_SCATTER,
        XML_CHART_CLASS_RADAR,
        XML_CHART_CLASS_BAR,
        XML_CHART_CLASS_STOCK,
        XML_CHART_CLASS_BUBBLE, // not yet implemented
        XML_CHART_CLASS_ADDIN,
        XML_CHART_CLASS_UNKNOWN
    };

    SchXMLChartTypeEnum GetChartTypeEnum( const ::rtl::OUString& rClassName );

    ::rtl::OUString GetChartTypeByClassName(
        const ::rtl::OUString & rClassName, bool bUseOldNames );

    ::xmloff::token::XMLTokenEnum getTokenByChartType(
        const ::rtl::OUString & rChartTypeService, bool bUseOldNames );

    ::rtl::OUString GetNewChartTypeName( const ::rtl::OUString & rOldChartTypeName );

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XLabeledDataSequence > GetNewLabeledDataSequence();

    void CreateCategories(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataProvider > & xDataProvider,
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument > & xNewDoc,
        const ::rtl::OUString & rRangeAddress,
        sal_Int32 nCooSysIndex,
        sal_Int32 nDimensionIndex,
        tSchXMLLSequencesPerIndex * pLSequencesPerIndex = 0 );

    ::com::sun::star::uno::Any getPropertyFromContext( const ::rtl::OUString& rPropertyName, const XMLPropStyleContext * pPropStyleContext, const SvXMLStylesContext* pStylesCtxt );

    void exportText( SvXMLExport& rExport, const ::rtl::OUString& rText, bool bConvertTabsLFs );

    /** returns the properties of the equation of the first regression curve
        that is no mean-value line
     */
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XRegressionCurve > getRegressionCurve(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > & xDataSeries );
}

#endif  // SCH_XML_TOOLS_HXX_
