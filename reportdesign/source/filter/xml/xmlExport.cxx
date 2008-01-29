/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlExport.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:45:00 $
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
#include "precompiled_reportdesign.hxx"

#ifndef RPT_XMLEXPORT_HXX
#include "xmlExport.hxx"
#endif
#ifndef RPT_XMLAUTOSTYLE_HXX
#include "xmlAutoStyle.hxx"
#endif
#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#include <xmloff/ProgressBarHelper.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_TEXTIMP_HXX_
#include <xmloff/txtimp.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef RPT_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_TEXTPRMAP_HXX_
#include <xmloff/txtprmap.hxx>
#endif
#include <xmloff/numehelp.hxx>
#ifndef RPT_XMLHELPER_HXX
#include "xmlHelper.hxx"
#endif
#include "xmlstrings.hrc"
#ifndef RPT_XMLPROPHANDLER_HXX
#include "xmlPropertyHandler.hxx"
#endif
#ifndef _COM_SUN_STAR_AWT_IMAGEPOSITION_HPP_
#include <com/sun/star/awt/ImagePosition.hpp>
#endif
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/report/GroupOn.hpp>
#ifndef _COM_SUN_STAR_REPORT_XFIXEDTEXT_HPP_
#include <com/sun/star/report/XFixedText.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XImageControl_HPP_
#include <com/sun/star/report/XImageControl.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XSHAPE_HPP_
#include <com/sun/star/report/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XFUNCTION_HPP_
#include <com/sun/star/report/XFunction.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/report/XFixedLine.hpp>
#ifndef _REPORT_RPTUIDEF_HXX
#include "RptDef.hxx"
#endif
// for locking SolarMutex: svapp + mutex
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#include <boost/bind.hpp>

//--------------------------------------------------------------------------
namespace rptxml
{
    using namespace xmloff;
    using namespace comphelper;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::xml;

    //---------------------------------------------------------------------
    Reference< XInterface > ORptExportHelper::create(Reference< XComponentContext > const & xContext)
    {
        return static_cast< XServiceInfo* >(new ORptExport(Reference< XMultiServiceFactory >(xContext->getServiceManager(),UNO_QUERY),EXPORT_SETTINGS ));
    }
    //---------------------------------------------------------------------
    ::rtl::OUString ORptExportHelper::getImplementationName_Static(  ) throw (RuntimeException)
    {
        return ::rtl::OUString::createFromAscii("com.sun.star.comp.report.XMLSettingsExporter");
    }
    //---------------------------------------------------------------------
    Sequence< ::rtl::OUString > ORptExportHelper::getSupportedServiceNames_Static(  ) throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported(1);
        aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.document.ExportFilter");
        return aSupported;
    }
    //---------------------------------------------------------------------
    Reference< XInterface > ORptContentExportHelper::create(Reference< XComponentContext > const & xContext)
    {
        return static_cast< XServiceInfo* >(new ORptExport(Reference< XMultiServiceFactory >(xContext->getServiceManager(),UNO_QUERY),EXPORT_CONTENT ));
    }
    //---------------------------------------------------------------------
    ::rtl::OUString ORptContentExportHelper::getImplementationName_Static(  ) throw (RuntimeException)
    {
        return ::rtl::OUString::createFromAscii("com.sun.star.comp.report.XMLContentExporter");
    }
    //---------------------------------------------------------------------
    Sequence< ::rtl::OUString > ORptContentExportHelper::getSupportedServiceNames_Static(  ) throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported(1);
        aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.document.ExportFilter");
        return aSupported;
    }

    //---------------------------------------------------------------------
    Reference< XInterface > ORptStylesExportHelper::create(Reference< XComponentContext > const & xContext)
    {
        return static_cast< XServiceInfo* >(new ORptExport(Reference< XMultiServiceFactory >(xContext->getServiceManager(),UNO_QUERY),EXPORT_STYLES | EXPORT_MASTERSTYLES | EXPORT_AUTOSTYLES |
            EXPORT_FONTDECLS|EXPORT_OASIS ));
    }
    //---------------------------------------------------------------------
    ::rtl::OUString ORptStylesExportHelper::getImplementationName_Static(  ) throw (RuntimeException)
    {
        return ::rtl::OUString::createFromAscii("com.sun.star.comp.report.XMLStylesExporter");
    }
    //---------------------------------------------------------------------
    Sequence< ::rtl::OUString > ORptStylesExportHelper::getSupportedServiceNames_Static(  ) throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported(1);
        aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.document.ExportFilter");
        return aSupported;
    }

    //---------------------------------------------------------------------
    Reference< XInterface > ODBFullExportHelper::create(Reference< XComponentContext > const & xContext)
    {
        return static_cast< XServiceInfo* >(new ORptExport(Reference< XMultiServiceFactory >(xContext->getServiceManager(),UNO_QUERY),EXPORT_ALL));
    }
    //---------------------------------------------------------------------
    ::rtl::OUString ODBFullExportHelper::getImplementationName_Static(  ) throw (RuntimeException)
    {
        return ::rtl::OUString::createFromAscii("com.sun.star.comp.report.XMLFullExporter");
    }
    //---------------------------------------------------------------------
    Sequence< ::rtl::OUString > ODBFullExportHelper::getSupportedServiceNames_Static(  ) throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported(1);
        aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.document.ExportFilter");
        return aSupported;
    }

    //---------------------------------------------------------------------
    ::rtl::OUString lcl_implGetPropertyXMLType(const Type& _rType)
    {
        // possible types we can write (either because we recognize them directly or because we convert _rValue
        // into one of these types)
        static const ::rtl::OUString s_sTypeBoolean (RTL_CONSTASCII_USTRINGPARAM("boolean"));
        static const ::rtl::OUString s_sTypeShort   (RTL_CONSTASCII_USTRINGPARAM("short"));
        static const ::rtl::OUString s_sTypeInteger (RTL_CONSTASCII_USTRINGPARAM("int"));
        static const ::rtl::OUString s_sTypeLong    (RTL_CONSTASCII_USTRINGPARAM("long"));
        static const ::rtl::OUString s_sTypeDouble  (RTL_CONSTASCII_USTRINGPARAM("double"));
        static const ::rtl::OUString s_sTypeString  (RTL_CONSTASCII_USTRINGPARAM("string"));

        // handle the type description
        switch (_rType.getTypeClass())
        {
            case TypeClass_STRING:
                return s_sTypeString;
            case TypeClass_DOUBLE:
                return s_sTypeDouble;
            case TypeClass_BOOLEAN:
                return s_sTypeBoolean;
            case TypeClass_BYTE:
            case TypeClass_SHORT:
                return s_sTypeShort;
            case TypeClass_LONG:
                return s_sTypeInteger;
            case TypeClass_HYPER:
                return s_sTypeLong;
            case TypeClass_ENUM:
                return s_sTypeInteger;

            default:
                return s_sTypeDouble;
        }
    }

    class OSpecialHanldeXMLExportPropertyMapper : public SvXMLExportPropertyMapper
    {
    public:
        OSpecialHanldeXMLExportPropertyMapper(const UniReference< XMLPropertySetMapper >& rMapper) : SvXMLExportPropertyMapper(rMapper )
        {
        }
        /** this method is called for every item that has the
        MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
        virtual void handleSpecialItem(
                SvXMLAttributeList& /*rAttrList*/,
                const XMLPropertyState& /*rProperty*/,
                const SvXMLUnitConverter& /*rUnitConverter*/,
                const SvXMLNamespaceMap& /*rNamespaceMap*/,
                const ::std::vector< XMLPropertyState >* /*pProperties*/ = 0,
                sal_uInt32 /*nIdx*/ = 0 ) const
        {
            // nothing to do here
        }
    };
// -----------------------------------------------------------------------------
void lcl_adjustColumnSpanOverRows(ORptExport::TSectionsGrid& _rGrid)
{
    ORptExport::TSectionsGrid::iterator aSectionIter = _rGrid.begin();
    ORptExport::TSectionsGrid::iterator aSectionEnd = _rGrid.end();
    for (;aSectionIter != aSectionEnd ; ++aSectionIter)
    {
        ORptExport::TGrid::iterator aRowIter = aSectionIter->second.begin();
        ORptExport::TGrid::iterator aRowEnd = aSectionIter->second.end();
        for (; aRowIter != aRowEnd; ++aRowIter)
        {
            if ( aRowIter->first )
            {
                ::std::vector< ORptExport::TCell >::iterator aColIter = aRowIter->second.begin();
                ::std::vector< ORptExport::TCell >::iterator aColEnd = aRowIter->second.end();
                for (; aColIter != aColEnd; ++aColIter)
                {
                    if ( aColIter->nRowSpan > 1 )
                    {
                        sal_Int32 nColSpan = aColIter->nColSpan;
                        sal_Int32 nColIndex = aColIter - aRowIter->second.begin();
                        for (sal_Int32 i = 1; i < aColIter->nRowSpan; ++i)
                        {
                            (aRowIter+i)->second[nColIndex].nColSpan = nColSpan;
                        }
                    }
                }
            }
        }
    }
}
// -----------------------------------------------------------------------------
ORptExport::ORptExport(const Reference< XMultiServiceFactory >& _rxMSF,sal_uInt16 nExportFlag)
: SvXMLExport( _rxMSF,MAP_100TH_MM,XML_REPORT, EXPORT_OASIS)
,m_bAllreadyFilled(sal_False)
{
    setExportFlags( EXPORT_OASIS | nExportFlag);
    GetMM100UnitConverter().setCoreMeasureUnit(MAP_100TH_MM);
    GetMM100UnitConverter().setXMLMeasureUnit(MAP_CM);

    // (getExportFlags() & EXPORT_CONTENT) != 0 ? : XML_N_OOO
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_OFFICE), GetXMLToken(XML_N_OFFICE ), XML_NAMESPACE_OFFICE );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_OOO), GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO );

    _GetNamespaceMap().Add( GetXMLToken(XML_NP_RPT), GetXMLToken(XML_N_RPT), XML_NAMESPACE_REPORT );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_SVG), GetXMLToken(XML_N_SVG_COMPAT),  XML_NAMESPACE_SVG );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_FORM), GetXMLToken(XML_N_FORM), XML_NAMESPACE_FORM );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_DRAW), GetXMLToken(XML_N_DRAW), XML_NAMESPACE_DRAW );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_TEXT), GetXMLToken(XML_N_TEXT), XML_NAMESPACE_TEXT );


    if( (getExportFlags() & (EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_FONTDECLS) ) != 0 )
        _GetNamespaceMap().Add( GetXMLToken(XML_NP_FO), GetXMLToken(XML_N_FO_COMPAT), XML_NAMESPACE_FO );

    if( (getExportFlags() & (EXPORT_META|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_SETTINGS) ) != 0 )
    {
        _GetNamespaceMap().Add( GetXMLToken(XML_NP_XLINK), GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK );
    }
    if( (getExportFlags() & EXPORT_SETTINGS) != 0 )
    {
        _GetNamespaceMap().Add( GetXMLToken(XML_NP_CONFIG), GetXMLToken(XML_N_CONFIG), XML_NAMESPACE_CONFIG );
    }

    if( (getExportFlags() & (EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_FONTDECLS) ) != 0 )
    {
        _GetNamespaceMap().Add( GetXMLToken(XML_NP_STYLE), GetXMLToken(XML_N_STYLE), XML_NAMESPACE_STYLE );
    }

    _GetNamespaceMap().Add( GetXMLToken(XML_NP_TABLE), GetXMLToken(XML_N_TABLE), XML_NAMESPACE_TABLE );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_NUMBER), GetXMLToken(XML_N_NUMBER), XML_NAMESPACE_NUMBER );

    m_sTableStyle = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TABLE, GetXMLToken(XML_STYLE_NAME) );
    m_sColumnStyle = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TABLE, GetXMLToken(XML_COLUMN) );
    m_sCellStyle = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_REPORT, GetXMLToken(XML_STYLE_NAME) );


    m_xPropHdlFactory = new OXMLRptPropHdlFactory();
    UniReference < XMLPropertyHandlerFactory> xFac = new ::xmloff::OControlPropertyHandlerFactory();
    UniReference < XMLPropertySetMapper > xTableStylesPropertySetMapper1 = new XMLPropertySetMapper(OXMLHelper::GetTableStyleProps(),xFac);
    UniReference < XMLPropertySetMapper > xTableStylesPropertySetMapper2 = new XMLTextPropertySetMapper(TEXT_PROP_MAP_TABLE_DEFAULTS );
    xTableStylesPropertySetMapper1->AddMapperEntry(xTableStylesPropertySetMapper2);

    m_xTableStylesExportPropertySetMapper = new SvXMLExportPropertyMapper(xTableStylesPropertySetMapper1);
    //m_xTableStylesExportPropertySetMapper->ChainExportMapper(xTableStylesPropertySetMapper2);

    m_xCellStylesPropertySetMapper = OXMLHelper::GetCellStylePropertyMap();
    m_xCellStylesExportPropertySetMapper = new OSpecialHanldeXMLExportPropertyMapper(m_xCellStylesPropertySetMapper);
    m_xCellStylesExportPropertySetMapper->ChainExportMapper(XMLTextParagraphExport::CreateParaExtPropMapper(*this));

    UniReference < XMLPropertySetMapper > xColumnStylesPropertySetMapper = new XMLPropertySetMapper(OXMLHelper::GetColumnStyleProps(), m_xPropHdlFactory);
    m_xColumnStylesExportPropertySetMapper = new OSpecialHanldeXMLExportPropertyMapper(xColumnStylesPropertySetMapper);

    UniReference < XMLPropertySetMapper > xRowStylesPropertySetMapper = new XMLPropertySetMapper(OXMLHelper::GetRowStyleProps(), m_xPropHdlFactory);
    m_xRowStylesExportPropertySetMapper = new OSpecialHanldeXMLExportPropertyMapper(xRowStylesPropertySetMapper);

    UniReference < XMLPropertySetMapper > xPropMapper(new XMLTextPropertySetMapper( TEXT_PROP_MAP_PARA ));
    m_xParaPropMapper = new OSpecialHanldeXMLExportPropertyMapper( xPropMapper);

    ::rtl::OUString sFamily( GetXMLToken(XML_PARAGRAPH) );
    ::rtl::OUString aPrefix( String( 'P' ) );
    GetAutoStylePool()->AddFamily( XML_STYLE_FAMILY_TEXT_PARAGRAPH, sFamily,
                              m_xParaPropMapper, aPrefix );

    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_CELL, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME)),
        m_xCellStylesExportPropertySetMapper, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX)));
    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_COLUMN, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME)),
        m_xColumnStylesExportPropertySetMapper, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX)));
    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_ROW, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_ROW_STYLES_NAME)),
        m_xRowStylesExportPropertySetMapper, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_ROW_STYLES_PREFIX)));
    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_TABLE, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME)),
        m_xTableStylesExportPropertySetMapper, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_TABLE_STYLES_PREFIX)));
}
// -----------------------------------------------------------------------------
Reference< XInterface > ORptExport::create(Reference< XComponentContext > const & xContext)
{
    return *(new ORptExport(Reference< XMultiServiceFactory >(xContext->getServiceManager(),UNO_QUERY)));
}

// -----------------------------------------------------------------------------
::rtl::OUString ORptExport::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.ExportFilter"));
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL ORptExport::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}
//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > ORptExport::getSupportedServiceNames_Static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aServices(1);
    aServices.getArray()[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.ExportFilter"));

    return aServices;
}
//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL ORptExport::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}
//------------------------------------------------------------------------------
sal_Bool SAL_CALL ORptExport::supportsService(const ::rtl::OUString& ServiceName) throw( uno::RuntimeException )
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_Static());
}
// -----------------------------------------------------------------------------
void ORptExport::exportFunctions(const Reference<XIndexAccess>& _xFunctions)
{
    const sal_Int32 nCount = _xFunctions->getCount();
    for (sal_Int32 i = 0; i< nCount; ++i)
    {
        uno::Reference< report::XFunction> xFunction(_xFunctions->getByIndex(i),uno::UNO_QUERY_THROW);
        OSL_ENSURE(xFunction.is(),"Function object is NULL!");
        exportFunction(xFunction);
    }
}
// -----------------------------------------------------------------------------
void ORptExport::exportFunction(const uno::Reference< XFunction>& _xFunction)
{
    exportFormula(XML_FORMULA,_xFunction->getFormula());
    beans::Optional< ::rtl::OUString> aInitial = _xFunction->getInitialFormula();
    if ( aInitial.IsPresent && aInitial.Value.getLength() )
        exportFormula(XML_INITIAL_FORMULA ,aInitial.Value );
    AddAttribute( XML_NAMESPACE_REPORT, XML_NAME , _xFunction->getName() );
    if ( _xFunction->getPreEvaluated() )
        AddAttribute( XML_NAMESPACE_REPORT, XML_PRE_EVALUATED , XML_TRUE );
    if ( _xFunction->getDeepTraversing() )
        AddAttribute( XML_NAMESPACE_REPORT, XML_DEEP_TRAVERSING , XML_TRUE );

    SvXMLElementExport aFunction(*this,XML_NAMESPACE_REPORT, XML_FUNCTION, sal_True, sal_True);
}
// -----------------------------------------------------------------------------
void ORptExport::exportMasterDetailFields(const Reference<XReportDefinition>& _xReportDefinition)
{
    uno::Sequence< ::rtl::OUString> aMasterFields = _xReportDefinition->getMasterFields();
    if ( aMasterFields.getLength() )
    {
        SvXMLElementExport aElement(*this,XML_NAMESPACE_REPORT, XML_MASTER_DETAIL_FIELDS, sal_True, sal_True);
        uno::Sequence< ::rtl::OUString> aDetailFields = _xReportDefinition->getDetailFields();

        OSL_ENSURE(aDetailFields.getLength() == aMasterFields.getLength(),"not equal length for amster and detail fields!");

        const ::rtl::OUString* pDetailFieldsIter = aDetailFields.getConstArray();
        const ::rtl::OUString* pIter = aMasterFields.getConstArray();
        const ::rtl::OUString* pEnd   = pIter + aMasterFields.getLength();
        for(;pIter != pEnd;++pIter,++pDetailFieldsIter)
        {
            AddAttribute( XML_NAMESPACE_REPORT, XML_MASTER , *pIter );
            if ( pDetailFieldsIter->getLength() )
                AddAttribute( XML_NAMESPACE_REPORT, XML_DETAIL , *pDetailFieldsIter );
            SvXMLElementExport aPair(*this,XML_NAMESPACE_REPORT, XML_MASTER_DETAIL_FIELD, sal_True, sal_True);
        }
    }
}
// -----------------------------------------------------------------------------
void ORptExport::exportReport(const Reference<XReportDefinition>& _xReportDefinition)
{
    if ( _xReportDefinition.is() )
    {
        exportFunctions(_xReportDefinition->getFunctions().get());
        exportGroupsExpressionAsFunction(_xReportDefinition->getGroups());

        if ( _xReportDefinition->getReportHeaderOn() )
        {
            SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_REPORT_HEADER, sal_True, sal_True);
            exportSection(_xReportDefinition->getReportHeader());
        } // if ( _xReportDefinition->getReportHeaderOn() )
        if ( _xReportDefinition->getPageHeaderOn() )
        {
            ::rtl::OUStringBuffer sValue;
            sal_uInt16 nRet = _xReportDefinition->getPageHeaderOption();
            const SvXMLEnumMapEntry* aXML_EnumMap = OXMLHelper::GetReportPrintOptions();
            if ( SvXMLUnitConverter::convertEnum( sValue, nRet,aXML_EnumMap ) )
                AddAttribute(XML_NAMESPACE_REPORT, XML_PAGE_PRINT_OPTION,sValue.makeStringAndClear());

            SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_PAGE_HEADER, sal_True, sal_True);
            exportSection(_xReportDefinition->getPageHeader(),true);
        } // if ( _xReportDefinition->getPageHeaderOn() )

        exportGroup(_xReportDefinition,0);

        if ( _xReportDefinition->getPageFooterOn() )
        {
            ::rtl::OUStringBuffer sValue;
            sal_uInt16 nRet = _xReportDefinition->getPageFooterOption();
            const SvXMLEnumMapEntry* aXML_EnumMap = OXMLHelper::GetReportPrintOptions();
            if ( SvXMLUnitConverter::convertEnum( sValue, nRet,aXML_EnumMap ) )
                AddAttribute(XML_NAMESPACE_REPORT, XML_PAGE_PRINT_OPTION,sValue.makeStringAndClear());
            SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_PAGE_FOOTER, sal_True, sal_True);
            exportSection(_xReportDefinition->getPageFooter(),true);
        } // if ( _xReportDefinition->getPageFooterOn() )
        if ( _xReportDefinition->getReportFooterOn() )
        {
            SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_REPORT_FOOTER, sal_True, sal_True);
            exportSection(_xReportDefinition->getReportFooter());
        } // if ( _xReportDefinition->getReportFooterOn() )
    }
}
// -----------------------------------------------------------------------------
void ORptExport::exportImagePosition(sal_Int16 _nImagePosition)
{
    OSL_ENSURE( ( _nImagePosition >= awt::ImagePosition::LeftTop ) && ( _nImagePosition <= awt::ImagePosition::Centered ),
        "ORptExport::export: don't know this image position!" );

    if ( ( _nImagePosition < awt::ImagePosition::LeftTop ) || ( _nImagePosition > awt::ImagePosition::Centered ) )
        // this is important to prevent potential buffer overflows below, so don't optimize
        _nImagePosition = awt::ImagePosition::Centered;

    if ( _nImagePosition == awt::ImagePosition::Centered )
    {
        AddAttribute( XML_NAMESPACE_FORM, XML_IMAGE_POSITION , GetXMLToken( XML_CENTER ) );
    }
    else
    {
        XMLTokenEnum eXmlImagePositions[] =
        {
            XML_START, XML_END, XML_TOP, XML_BOTTOM
        };
        XMLTokenEnum eXmlImageAligns[] =
        {
            XML_START, XML_CENTER, XML_END
        };

        XMLTokenEnum eXmlImagePosition = eXmlImagePositions[ _nImagePosition / 3 ];
        XMLTokenEnum eXmlImageAlign    = eXmlImageAligns   [ _nImagePosition % 3 ];

        AddAttribute( XML_NAMESPACE_FORM, XML_IMAGE_POSITION , GetXMLToken( eXmlImagePosition ) );
        AddAttribute( XML_NAMESPACE_FORM, XML_IMAGE_ALIGN    , GetXMLToken( eXmlImageAlign    ) );
    }
}
// -----------------------------------------------------------------------------
void ORptExport::exportComponent(const Reference<XReportComponent>& _xReportComponent)
{
    OSL_ENSURE(_xReportComponent.is(),"No component interface!");
    if ( !_xReportComponent.is() )
        return;

    AddAttribute(XML_NAMESPACE_DRAW, XML_NAME,_xReportComponent->getName());

    SvXMLElementExport aElem(*this,XML_NAMESPACE_REPORT, XML_REPORT_COMPONENT, sal_False, sal_False);
}
// -----------------------------------------------------------------------------
void ORptExport::exportFormatConditions(const Reference<XReportControlModel>& _xReportElement)
{
    OSL_ENSURE(_xReportElement.is(),"_xReportElement is NULL -> GPF");
    ::rtl::OUString sDataField = convertFormula(_xReportElement->getDataField());
    sal_Int32 nCount = _xReportElement->getCount();
    try
    {
        for (sal_Int32 i = 0; i < nCount ; ++i)
        {
            uno::Reference< report::XFormatCondition > xCond(_xReportElement->getByIndex(i),uno::UNO_QUERY);
            if ( !xCond->getEnabled() )
                AddAttribute(XML_NAMESPACE_REPORT, XML_ENABLED,XML_FALSE);

            AddAttribute(XML_NAMESPACE_REPORT, XML_FORMULA,xCond->getFormula());

            exportStyleName(xCond.get(),GetAttrList(),m_sCellStyle);
            SvXMLElementExport aElem(*this,XML_NAMESPACE_REPORT, XML_FORMAT_CONDITION, sal_True, sal_True);
        } // for (sal_Int32 i = 0; i < nCount ; ++i)
    }
    catch(uno::Exception&)
    {
        OSL_ENSURE(0,"Can not access format condition!");
    }
}
// -----------------------------------------------------------------------------
void ORptExport::exportReportElement(const Reference<XReportControlModel>& _xReportElement)
{
    OSL_ENSURE(_xReportElement.is(),"_xReportElement is NULL -> GPF");
    if ( _xReportElement->getPrintWhenGroupChange() )
        AddAttribute(XML_NAMESPACE_REPORT, XML_PRINT_ONLY_WHEN_GROUP_CHANGE, XML_TRUE );

    if ( !_xReportElement->getPrintRepeatedValues() )
        AddAttribute(XML_NAMESPACE_REPORT, XML_PRINT_REPEATED_VALUES,XML_FALSE);

    SvXMLElementExport aElem(*this,XML_NAMESPACE_REPORT, XML_REPORT_ELEMENT, sal_False, sal_False);
    if ( _xReportElement->getCount() )
    {
        exportFormatConditions(_xReportElement);
    }

    ::rtl::OUString sExpr = _xReportElement->getConditionalPrintExpression();
    if ( sExpr.getLength() )
    {
        exportFormula(XML_FORMULA,sExpr);
        SvXMLElementExport aPrintExpr(*this,XML_NAMESPACE_REPORT, XML_CONDITIONAL_PRINT_EXPRESSION, sal_False, sal_False);
    } // if ( sExpr.getLength() )

    // only export when parent exists
    uno::Reference< report::XSection> xParent(_xReportElement->getParent(),uno::UNO_QUERY);
    if ( xParent.is() )
        exportComponent(_xReportElement.get());
}
// -----------------------------------------------------------------------------
void lcl_calculate(const ::std::vector<sal_Int32>& _aPosX,const ::std::vector<sal_Int32>& _aPosY,ORptExport::TGrid& _rColumns)
{
    sal_Int32 nCountX = _aPosX.size() - 1;
    sal_Int32 nCountY = _aPosY.size() - 1;
    for (sal_Int32 j = 0; j < nCountY; ++j)
    {
        sal_Int32 nHeight = _aPosY[j+1] - _aPosY[j];
        if ( nHeight )
            for (sal_Int32 i = 0; i < nCountX ; ++i)
            {
                _rColumns[j].second[i] = ORptExport::TCell(_aPosX[i+1] - _aPosX[i],nHeight,1,1);
                _rColumns[j].second[i].bSet = sal_True;
            }
    }
}
// -----------------------------------------------------------------------------
void ORptExport::collectStyleNames(sal_Int32 _nFamily,const ::std::vector< sal_Int32>& _aSize, ORptExport::TStringVec& _rStyleNames)
{
    ::std::vector< XMLPropertyState > aPropertyStates;
    aPropertyStates.push_back(XMLPropertyState(0));
    ::std::vector<sal_Int32>::const_iterator aIter = _aSize.begin();
    ::std::vector<sal_Int32>::const_iterator aIter2 = aIter + 1;
    ::std::vector<sal_Int32>::const_iterator aEnd = _aSize.end();
    for (;aIter2 != aEnd ; ++aIter,++aIter2)
    {
        sal_Int32 nValue = static_cast<sal_Int32>(*aIter2 - *aIter);
        aPropertyStates[0].maValue <<= nValue;
        _rStyleNames.push_back(GetAutoStylePool()->Add(_nFamily, aPropertyStates ));
    }
}
// -----------------------------------------------------------------------------
void ORptExport::exportSectionAutoStyle(const Reference<XSection>& _xProp)
{
    OSL_ENSURE(_xProp != NULL,"Section is NULL -> GPF");
    exportAutoStyle(_xProp);

    Reference<XReportDefinition> xReport = _xProp->getReportDefinition();
    awt::Size aSize     = rptui::getStyleProperty<awt::Size>(xReport,PROPERTY_PAPERSIZE);
    sal_Int32 nOffset   = rptui::getStyleProperty<sal_Int32>(xReport,PROPERTY_LEFTMARGIN);
    sal_Int32 nCount    = _xProp->getCount();

    ::std::vector<sal_Int32> aColumnPos;
    aColumnPos.reserve(2*(nCount + 1));
    aColumnPos.push_back(nOffset);
    aColumnPos.push_back(aSize.Width - rptui::getStyleProperty<sal_Int32>(xReport,PROPERTY_RIGHTMARGIN));

    ::std::vector<sal_Int32> aRowPos;
    aRowPos.reserve(2*(nCount + 1));
    aRowPos.push_back(0);
    aRowPos.push_back(_xProp->getHeight());

    sal_Int32 i;
    for (i = 0 ; i< nCount ; ++i)
    {
        Reference<XReportComponent> xReportElement(_xProp->getByIndex(i),uno::UNO_QUERY);
        uno::Reference< XShape> xShape(xReportElement,uno::UNO_QUERY);
        if ( xShape.is() )
            continue;
        OSL_ENSURE( xReportElement.is(),"NULL Element in Section!" );
        if ( !xReportElement.is() )
            continue;
        sal_Int32 nX = xReportElement->getPositionX();
        aColumnPos.push_back(nX);
        Reference<XFixedLine> xFixedLine(xReportElement,uno::UNO_QUERY);
        if ( xFixedLine.is() && xFixedLine->getOrientation() == 1 ) // vertical
        {
            sal_Int32 nWidth = static_cast<sal_Int32>(xReportElement->getWidth()*0.5);
            nX += nWidth;
            aColumnPos.push_back(nX);
            nX += xReportElement->getWidth() - nWidth;
        }
        else
            nX += xReportElement->getWidth();
        aColumnPos.push_back(nX); // --nX why?

        sal_Int32 nY = xReportElement->getPositionY();
        aRowPos.push_back(nY);
        nY += xReportElement->getHeight();
        aRowPos.push_back(nY); // --nY why?
    }

    ::std::sort(aColumnPos.begin(),aColumnPos.end(),::std::less<sal_Int32>());
    aColumnPos.erase(::std::unique(aColumnPos.begin(),aColumnPos.end()),aColumnPos.end());

    ::std::sort(aRowPos.begin(),aRowPos.end(),::std::less<sal_Int32>());
    aRowPos.erase(::std::unique(aRowPos.begin(),aRowPos.end()),aRowPos.end());

    TSectionsGrid::iterator aInsert = m_aSectionsGrid.insert(
        TSectionsGrid::value_type(
                                    _xProp.get(),
                                    TGrid(aRowPos.size() - 1,TGrid::value_type(sal_False,TRow(aColumnPos.size() - 1)))
                                  )
        ).first;
    lcl_calculate(aColumnPos,aRowPos,aInsert->second);

    TGridStyleMap::iterator aPos = m_aColumnStyleNames.insert(TGridStyleMap::value_type(_xProp.get(),TStringVec())).first;
    collectStyleNames(XML_STYLE_FAMILY_TABLE_COLUMN,aColumnPos,aPos->second);
    aPos = m_aRowStyleNames.insert(TGridStyleMap::value_type(_xProp.get(),TStringVec())).first;
    collectStyleNames(XML_STYLE_FAMILY_TABLE_ROW,aRowPos,aPos->second);

    sal_Int32 x1 = 0;
    sal_Int32 y1 = 0;
    sal_Int32 x2 = 0;
    sal_Int32 y2 = 0;
    sal_Int32 xi = 0;
    sal_Int32 yi = 0;
    bool isOverlap = false;

    for (i = 0 ; i< nCount ; ++i)
    {
        Reference<XReportComponent> xReportElement(_xProp->getByIndex(i),uno::UNO_QUERY);
        uno::Reference< XShape> xShape(xReportElement,uno::UNO_QUERY);
        if ( xShape.is() )
            continue;
        sal_Int32 nPos = xReportElement->getPositionX();
        x1 = (::std::find(aColumnPos.begin(),aColumnPos.end(),nPos) - aColumnPos.begin());
        Reference<XFixedLine> xFixedLine(xReportElement,uno::UNO_QUERY);
        if ( xFixedLine.is() && xFixedLine->getOrientation() == 1 ) // vertical
            nPos += static_cast<sal_Int32>(xReportElement->getWidth()*0.5);
        else
            nPos += xReportElement->getWidth(); // -1 why
        x2 = (::std::find(aColumnPos.begin(),aColumnPos.end(),nPos) - aColumnPos.begin());

        nPos = xReportElement->getPositionY();
        y1 = (::std::find(aRowPos.begin(),aRowPos.end(),nPos) - aRowPos.begin());
        nPos += xReportElement->getHeight(); // -1 why?
        y2 = (::std::find(aRowPos.begin(),aRowPos.end(),nPos) - aRowPos.begin());

        isOverlap = false;
        yi = y1;
        while(yi < y2 && !isOverlap) // find overlapping controls
        {
            xi = x1;
            while(xi < x2 && !isOverlap)
            {
                if ( aInsert->second[yi].second[xi].xElement.is() )
                {
                    isOverlap = true;
                }
                ++xi;
            }
            ++yi;
        }

        if (!isOverlap)
        {
            yi = y1;
            while(yi < y2)
            {
                xi = x1;
                while(xi < x2)
                {
                    aInsert->second[yi].second[xi] = TCell();
                    ++xi;
                }
                aInsert->second[yi].first = sal_True;
                ++yi;
            }

            if (x2 - x1 != 0 && y2 - y1 != 0)
            {
                awt::Size aElementSize = xReportElement->getSize();
                if ( xFixedLine.is() && xFixedLine->getOrientation() == 1 ) // vertical
                    aElementSize.Width = static_cast<sal_Int32>(xFixedLine->getWidth()*0.5);

                sal_Int32 nColSpan = x2 - x1;
                sal_Int32 nRowSpan = y2 - y1;
                aInsert->second[y1].second[x1] =
                    TCell(
                        aElementSize.Width , // -1 why?
                        aElementSize.Height, // -1 why?
                        nColSpan,
                        nRowSpan,
                        xReportElement
                        );
            }
        }
    }

    lcl_adjustColumnSpanOverRows(m_aSectionsGrid);
    exportReportComponentAutoStyles(_xProp);
}
// -----------------------------------------------------------------------------
void ORptExport::exportReportComponentAutoStyles(const Reference<XSection>& _xProp)
{
    const sal_Int32 nCount = _xProp->getCount();
    for (sal_Int32 i = 0 ; i< nCount ; ++i)
    {
        const Reference<XReportComponent> xReportElement(_xProp->getByIndex(i),uno::UNO_QUERY);
        const Reference< report::XShape > xShape(xReportElement,uno::UNO_QUERY);
        if ( xShape.is() )
        {
            UniReference< XMLShapeExport > xShapeExport = GetShapeExport();
            xShapeExport->seekShapes(_xProp.get());
            vos::OGuard aGuard(Application::GetSolarMutex());
            xShapeExport->collectShapeAutoStyles(xShape.get());
        }
        else
        {
            exportAutoStyle(xReportElement.get());

            Reference<XFormattedField> xFormattedField(xReportElement,uno::UNO_QUERY);
            if ( xFormattedField.is() )
            {
                try
                {
                    const sal_Int32 nFormatCount = xFormattedField->getCount();
                    for (sal_Int32 j = 0; j < nFormatCount ; ++j)
                    {
                        uno::Reference< report::XFormatCondition > xCond(xFormattedField->getByIndex(j),uno::UNO_QUERY);
                        exportAutoStyle(xCond.get(),xFormattedField);
                    } // for (sal_Int32 j = 0; j < nCount ; ++j)
                }
                catch(uno::Exception&)
                {
                    OSL_ENSURE(0,"Can not access format condition!");
                }
            }
        }
    }
}
// -----------------------------------------------------------------------------
void ORptExport::exportSection(const Reference<XSection>& _xSection,bool bHeader)
{
    OSL_ENSURE(_xSection.is(),"Section is NULL -> GPF");
    ::rtl::OUStringBuffer sValue;
    AddAttribute(XML_NAMESPACE_TABLE, XML_NAME,_xSection->getName());

    if ( !_xSection->getVisible() )
        AddAttribute(XML_NAMESPACE_REPORT, XML_VISIBLE,XML_FALSE);

    if ( !bHeader )
    {
        sal_uInt16 nRet = _xSection->getForceNewPage();
        const SvXMLEnumMapEntry* aXML_EnumMap = OXMLHelper::GetForceNewPageOptions();
        if ( SvXMLUnitConverter::convertEnum( sValue, nRet,aXML_EnumMap ) )
            AddAttribute(XML_NAMESPACE_REPORT, XML_FORCE_NEW_PAGE,sValue.makeStringAndClear());

        nRet = _xSection->getNewRowOrCol();
        if ( SvXMLUnitConverter::convertEnum( sValue, nRet,aXML_EnumMap ) )
            AddAttribute(XML_NAMESPACE_REPORT, XML_FORCE_NEW_COLUMN,sValue.makeStringAndClear());
        if ( _xSection->getKeepTogether() )
            AddAttribute(XML_NAMESPACE_REPORT, XML_KEEP_TOGETHER, XML_TRUE );
    }

    exportStyleName(_xSection.get(),GetAttrList(),m_sTableStyle);

    /// TODO export as table layout
    SvXMLElementExport aComponents(*this,XML_NAMESPACE_TABLE, XML_TABLE, sal_True, sal_True);

    ::rtl::OUString sExpr = _xSection->getConditionalPrintExpression();
    if ( sExpr.getLength() )
    {
        exportFormula(XML_FORMULA,sExpr);
        SvXMLElementExport aPrintExpr(*this,XML_NAMESPACE_REPORT, XML_CONDITIONAL_PRINT_EXPRESSION, sal_True, sal_False);
    }

    exportContainer(_xSection);
}
// -----------------------------------------------------------------------------
void ORptExport::exportTableColumns(const Reference< XSection>& _xSection)
{
    SvXMLElementExport aColumns(*this,XML_NAMESPACE_TABLE, XML_TABLE_COLUMNS, sal_True, sal_True);
    TGridStyleMap::iterator aColFind = m_aColumnStyleNames.find(_xSection.get());
    OSL_ENSURE(aColFind != m_aColumnStyleNames.end(),"ORptExport::exportTableColumns: Section not found in m_aColumnStyleNames!");
    if ( aColFind == m_aColumnStyleNames.end() )
        return;

    TStringVec::iterator aColIter = aColFind->second.begin();
    TStringVec::iterator aColEnd = aColFind->second.end();
    for (; aColIter != aColEnd; ++aColIter)
    {
        AddAttribute( m_sTableStyle,*aColIter );
        SvXMLElementExport aColumn(*this,XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, sal_True, sal_True);
    }
}
// -----------------------------------------------------------------------------
void ORptExport::exportContainer(const Reference< XSection>& _xSection)
{
    OSL_ENSURE(_xSection.is(),"Section is NULL -> GPF");

    exportTableColumns(_xSection);

    TSectionsGrid::iterator aFind = m_aSectionsGrid.find(_xSection.get());
    OSL_ENSURE(aFind != m_aSectionsGrid.end(),"ORptExport::exportContainer: Section not found in grid!");
    if ( aFind == m_aSectionsGrid.end() )
        return;
    TGrid::iterator aRowIter = aFind->second.begin();
    TGrid::iterator aRowEnd = aFind->second.end();

    sal_Int32 nEmptyCellColSpan = 0;
    TGridStyleMap::iterator aRowFind = m_aRowStyleNames.find(_xSection.get());
    TStringVec::iterator aHeightIter = aRowFind->second.begin();
    OSL_ENSURE(aRowFind->second.size() == aFind->second.size(),"Different count for rows");

    bool bShapeHandled = false;
    ::std::map<sal_Int32,sal_Int32> aRowSpan;
    for (sal_Int32 j = 0; aRowIter != aRowEnd; ++aRowIter,++j,++aHeightIter)
    {
        AddAttribute( m_sTableStyle,*aHeightIter );
        SvXMLElementExport aRow(*this,XML_NAMESPACE_TABLE, XML_TABLE_ROW, sal_True, sal_True);
        if ( aRowIter->first )
        {
            ::std::vector< TCell >::iterator aColIter = aRowIter->second.begin();
            ::std::vector< TCell >::iterator aColEnd = aRowIter->second.end();
            nEmptyCellColSpan = 0;
            for (; aColIter != aColEnd; ++aColIter)
            {
                sal_Bool bCoveredCell = sal_False;
                sal_Int32 nColSpan = 0;
                sal_Int32 nColIndex = aColIter - aRowIter->second.begin();
                ::std::map<sal_Int32,sal_Int32>::iterator aRowSpanFind = aRowSpan.find(nColIndex);
                if ( aRowSpanFind != aRowSpan.end() )
                {
#if OSL_DEBUG_LEVEL > 0
                    sal_Int32 nRowSpan = aRowSpanFind->second;
                    nRowSpan = nRowSpan;
                    sal_Int32 nColSpanTemp = aColIter->nColSpan;
                    (void) nColSpanTemp;
#endif
                    nColSpan = 1;
                    if ( !--(aRowSpanFind->second) )
                        aRowSpan.erase(aRowSpanFind);

                    if ( aColIter->nColSpan > 1 )
                        nColSpan += aColIter->nColSpan - 1;

                    bCoveredCell = sal_True;
                    aColIter = aColIter + (aColIter->nColSpan - 1);
                }
                else if ( aColIter->bSet )
                {
                    if ( nEmptyCellColSpan > 0 )
                    {
                        AddAttribute( XML_NAMESPACE_TABLE,XML_NUMBER_COLUMNS_SPANNED,implConvertNumber(nEmptyCellColSpan) );
                        bCoveredCell = sal_True;
                        nColSpan = nEmptyCellColSpan - 1;
                        nEmptyCellColSpan = 0;
                    }
                    sal_Int32 nSpan = aColIter->nColSpan;
                    if ( nSpan > 1 )
                    {
                        AddAttribute( XML_NAMESPACE_TABLE,XML_NUMBER_COLUMNS_SPANNED,implConvertNumber(nSpan) );
                        nColSpan = nSpan - 1;
                        bCoveredCell = sal_True;
                    }
                    nSpan = aColIter->nRowSpan;
                    if ( nSpan > 1 )
                    {
                        AddAttribute( XML_NAMESPACE_TABLE,XML_NUMBER_ROWS_SPANNED,implConvertNumber(nSpan) );
                        aRowSpan[nColIndex] = nSpan - 1;
                    }
                    if ( aColIter->xElement.is() )
                        exportStyleName(aColIter->xElement.get(),GetAttrList(),m_sTableStyle);

                    // start <table:table-cell>
                    Reference<XFormattedField> xFormattedField(aColIter->xElement,uno::UNO_QUERY);
                    if ( xFormattedField.is() )
                    {
                        sal_Int32 nFormatKey = xFormattedField->getFormatKey();
                        if ( 0 != nFormatKey )
                        {
                            XMLNumberFormatAttributesExportHelper aHelper(GetNumberFormatsSupplier(),*this);
                            sal_Bool bIsStandard = sal_False;
                            ::rtl::OUString sEmpty;
                            if ( util::NumberFormat::TEXT == aHelper.GetCellType(nFormatKey,bIsStandard) )
                                aHelper.SetNumberFormatAttributes(sEmpty, sEmpty);
                            else
                                aHelper.SetNumberFormatAttributes(nFormatKey, 0.0,sal_False);
                        }
                    }
                    SvXMLElementExport aCell(*this,XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_False);

                    if ( aColIter->xElement.is() )
                    {
                        // start <text:p>
                        SvXMLElementExport aParagraphContent(*this,XML_NAMESPACE_TEXT, XML_P, sal_True,sal_False);
                        Reference<XServiceInfo> xElement(aColIter->xElement,uno::UNO_QUERY);
                        Reference<XReportComponent> xReportComponent = aColIter->xElement;

                        if ( !bShapeHandled )
                        {
                            bShapeHandled = true;
                            exportShapes(_xSection,false);
                        }
                        uno::Reference< XShape > xShape(xElement,uno::UNO_QUERY);
                        uno::Reference< XFixedLine > xFixedLine(xElement,uno::UNO_QUERY);
                        if ( !xShape.is() && !xFixedLine.is() )
                        {
                            Reference<XReportControlModel> xReportElement(xElement,uno::UNO_QUERY);
                            Reference<XReportDefinition> xReportDefinition(xElement,uno::UNO_QUERY);
                            Reference< XImageControl > xImage(xElement,uno::UNO_QUERY);
                            Reference<XSection> xSection(xElement,uno::UNO_QUERY);

                            XMLTokenEnum eToken = XML_SECTION;
                            sal_Bool bExportData = sal_False;
                            if ( xElement->supportsService(SERVICE_FIXEDTEXT) )
                            {
                                eToken = XML_FIXED_CONTENT;
                            }
                            else if ( xElement->supportsService(SERVICE_FORMATTEDFIELD) )
                            {
                                eToken = XML_FORMATTED_TEXT;
                                bExportData = sal_True;
                            }
                            else if ( xElement->supportsService(SERVICE_IMAGECONTROL) )
                            {
                                eToken = XML_IMAGE;
                                ::rtl::OUString sTargetLocation = xImage->getImageURL();
                                if ( sTargetLocation.getLength() )
                                {
                                    sTargetLocation = GetRelativeReference(sTargetLocation);
                                    AddAttribute(XML_NAMESPACE_FORM, XML_IMAGE_DATA,sTargetLocation);
                                }
                                bExportData = sal_True;
                                if ( xImage->getScaleImage() )
                                    AddAttribute(XML_NAMESPACE_REPORT, XML_SCALE, XML_TRUE );
                            }
                            else if ( xReportDefinition.is() )
                            {
                                eToken = XML_SUB_DOCUMENT;
                            }
                            else if ( xSection.is() )
                            {
                            }

                            bool bPageSet = false;
                            if ( bExportData )
                            {
                                bPageSet = exportFormula(XML_FORMULA,xReportElement->getDataField());
                                if ( bPageSet )
                                    eToken = XML_FIXED_CONTENT;
                                else if ( eToken == XML_IMAGE )
                                    AddAttribute(XML_NAMESPACE_REPORT, XML_PRESERVE_IRI, xImage->getPreserveIRI() ? XML_TRUE : XML_FALSE );
                            }

                            //if ( !bPageSet )
                            {
                                // start <report:eToken>
                                SvXMLElementExport aComponents(*this,XML_NAMESPACE_REPORT, eToken, sal_False, sal_False);
                                if ( eToken == XML_FIXED_CONTENT )
                                    exportParagraph(xReportElement);
                                if ( xReportElement.is() )
                                    exportReportElement(xReportElement);

                                if ( eToken == XML_GROUP && xSection.is() )
                                    exportContainer(xSection);
                                else if ( eToken == XML_SUB_DOCUMENT )
                                {
                                    exportMasterDetailFields(xReportDefinition);
                                    SvXMLElementExport aOfficeElement( *this, XML_NAMESPACE_OFFICE, XML_BODY,sal_True, sal_True );
                                    SvXMLElementExport aElem( *this, sal_True,
                                                            XML_NAMESPACE_OFFICE, XML_REPORT,
                                                              sal_True, sal_True );

                                    exportReportAttributes(xReportDefinition);
                                    exportReport(xReportDefinition);
                                }
                                else if ( xSection.is() )
                                    exportSection(xSection);
                            }
                        }
                    }
                    aColIter = aColIter + (aColIter->nColSpan - 1);
                }
                else
                    ++nEmptyCellColSpan;
                if ( bCoveredCell )
                {
                    for (sal_Int32 k = 0; k < nColSpan; ++k)
                    {
                        SvXMLElementExport aCell(*this,XML_NAMESPACE_TABLE, XML_COVERED_TABLE_CELL, sal_True, sal_True);
                    }

                }
            } // for (sal_Int32 i = 0 ; i< nCount ; ++i)
            if ( nEmptyCellColSpan )
            {
                {
                    AddAttribute( XML_NAMESPACE_TABLE,XML_NUMBER_COLUMNS_SPANNED,implConvertNumber(nEmptyCellColSpan) );
                    SvXMLElementExport aCell(*this,XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True);
                    if ( !bShapeHandled )
                    {
                        bShapeHandled = true;
                        exportShapes(_xSection);
                    }
                }
                for (sal_Int32 k = 0; k < nEmptyCellColSpan; ++k)
                {
                    SvXMLElementExport aCoveredCell(*this,XML_NAMESPACE_TABLE, XML_COVERED_TABLE_CELL, sal_True, sal_True);
                }
                nEmptyCellColSpan = 0;
            }
        }
        else
        { // empty rows
            nEmptyCellColSpan = aRowIter->second.size();
            if ( nEmptyCellColSpan )
            {
                {
                    AddAttribute( XML_NAMESPACE_TABLE,XML_NUMBER_COLUMNS_SPANNED,implConvertNumber(nEmptyCellColSpan) );
                    SvXMLElementExport aCell(*this,XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True);
                    if ( !bShapeHandled )
                    {
                        bShapeHandled = true;
                        exportShapes(_xSection);
                    }
                }
                for (sal_Int32 k = 1; k < nEmptyCellColSpan; ++k)
                {
                    SvXMLElementExport aCoveredCell(*this,XML_NAMESPACE_TABLE, XML_COVERED_TABLE_CELL, sal_True, sal_True);
                }
                nEmptyCellColSpan = 0;
            }
        }
    }
}
// -----------------------------------------------------------------------------
::rtl::OUString ORptExport::convertFormula(const ::rtl::OUString& _sFormula)
{
    ::rtl::OUString sFormula = _sFormula;
    if ( _sFormula.equalsAsciiL("rpt:",4) )
        sFormula = ::rtl::OUString();
    //sal_Int32 nLength = _sFormula.getLength();
    //if ( nLength )
    //{
    //    sal_Int32 nPos = 0;
    //    if ( _sFormula.matchAsciiL("=",1) != 0 )
    //    {
    //        nPos = 1;
    //        --nLength;
    //    }
    //    bool bFormula = _sFormula.indexOf('[') != -1 || _sFormula.indexOf('(') != -1;
    //    if ( bFormula )
    //        sFormula = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("rpt:")) + _sFormula.copy(nPos,nLength);
    //    else
    //        sFormula = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("field:[")) + _sFormula.copy(nPos,nLength) + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("]"));;
    //}
    return sFormula;
}
// -----------------------------------------------------------------------------
bool ORptExport::exportFormula(enum ::xmloff::token::XMLTokenEnum eName,const ::rtl::OUString& _sFormula)
{
    const ::rtl::OUString sFieldData = convertFormula(_sFormula);
    static const ::rtl::OUString s_sPageNumber(RTL_CONSTASCII_USTRINGPARAM("PageNumber()"));
    static const ::rtl::OUString s_sPageCount(RTL_CONSTASCII_USTRINGPARAM("PageCount()"));
    sal_Int32 nPageNumberIndex = sFieldData.indexOf(s_sPageNumber);
    sal_Int32 nPageCountIndex = sFieldData.indexOf(s_sPageCount);
    bool bRet = nPageNumberIndex != -1 || nPageCountIndex != -1;
    if ( !bRet )
        AddAttribute(XML_NAMESPACE_REPORT, eName,sFieldData);

    return bRet;
}
// -----------------------------------------------------------------------------
void ORptExport::exportStyleName(XPropertySet* _xProp,SvXMLAttributeList& _rAtt,const ::rtl::OUString& _sName)
{
    Reference<XPropertySet> xFind(_xProp);
    TPropertyStyleMap::iterator aFind = m_aAutoStyleNames.find(xFind);
    if ( aFind != m_aAutoStyleNames.end() )
    {
        _rAtt.AddAttribute( _sName,
                            aFind->second );
        m_aAutoStyleNames.erase(aFind);
    }
}
// -----------------------------------------------------------------------------
sal_Bool ORptExport::exportGroup(const Reference<XReportDefinition>& _xReportDefinition,sal_Int32 _nPos,sal_Bool _bExportAutoStyle)
{
    sal_Bool bGroupExported = sal_False;
    if ( _xReportDefinition.is() )
    {
        Reference< XGroups > xGroups = _xReportDefinition->getGroups();
        if ( xGroups.is() )
        {
            sal_Int32 nCount = xGroups->getCount();
            if ( _nPos >= 0 && _nPos < nCount )
            {
                bGroupExported = sal_True;
                Reference<XGroup> xGroup(xGroups->getByIndex(_nPos),uno::UNO_QUERY);
                OSL_ENSURE(xGroup.is(),"No Group prepare for GPF");
                if ( _bExportAutoStyle )
                {
                    if ( xGroup->getHeaderOn() )
                        exportSectionAutoStyle(xGroup->getHeader());
                    exportGroup(_xReportDefinition,_nPos+1,_bExportAutoStyle);
                    if ( xGroup->getFooterOn() )
                        exportSectionAutoStyle(xGroup->getFooter());
                }
                else
                {
                    if ( xGroup->getSortAscending() )
                        AddAttribute(XML_NAMESPACE_REPORT, XML_SORT_ASCENDING, XML_TRUE );

                    if ( xGroup->getStartNewColumn() )
                        AddAttribute(XML_NAMESPACE_REPORT, XML_START_NEW_COLUMN, XML_TRUE);
                    if ( xGroup->getResetPageNumber() )
                        AddAttribute(XML_NAMESPACE_REPORT, XML_RESET_PAGE_NUMBER, XML_TRUE );

                    ::rtl::OUString sExpression = xGroup->getExpression();
                    if ( sExpression.getLength() )
                    {
                        ::rtl::OUString sFormula(RTL_CONSTASCII_USTRINGPARAM("rpt:HASCHANGED(\""));

                        TGroupFunctionMap::iterator aGroupFind = m_aGroupFunctionMap.find(xGroup);
                        if ( aGroupFind != m_aGroupFunctionMap.end() )
                            sExpression = aGroupFind->second->getName();
                        sFormula += sExpression;
                        sFormula += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\")"));
                        sExpression = sFormula;
                    }
                    AddAttribute(XML_NAMESPACE_REPORT, XML_GROUP_EXPRESSION,sExpression);
                    sal_Int16 nRet = xGroup->getKeepTogether();
                    ::rtl::OUStringBuffer sValue;
                    const SvXMLEnumMapEntry* aXML_KeepTogetherEnumMap = OXMLHelper::GetKeepTogetherOptions();
                    if ( SvXMLUnitConverter::convertEnum( sValue, nRet,aXML_KeepTogetherEnumMap ) )
                        AddAttribute(XML_NAMESPACE_REPORT, XML_KEEP_TOGETHER,sValue.makeStringAndClear());

                    SvXMLElementExport aGroup(*this,XML_NAMESPACE_REPORT, XML_GROUP, sal_True, sal_True);
                    exportFunctions(xGroup->getFunctions().get());
                    if ( xGroup->getHeaderOn() )
                    {
                        Reference<XSection> xSection = xGroup->getHeader();
                        if ( xSection->getRepeatSection() )
                            AddAttribute(XML_NAMESPACE_REPORT, XML_REPEAT_SECTION,XML_TRUE );
                        SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_GROUP_HEADER, sal_True, sal_True);
                        exportSection(xSection);
                    }
                    exportGroup(_xReportDefinition,_nPos+1,_bExportAutoStyle);
                    if ( xGroup->getFooterOn() )
                    {
                        Reference<XSection> xSection = xGroup->getFooter();
                        if ( xSection->getRepeatSection() )
                            AddAttribute(XML_NAMESPACE_REPORT, XML_REPEAT_SECTION,XML_TRUE );
                        SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_GROUP_FOOTER, sal_True, sal_True);
                        exportSection(xSection);
                    } // if ( xGroup->getFooterOn() )
                }
            }
            else if ( _bExportAutoStyle )
            {
                exportSectionAutoStyle(_xReportDefinition->getDetail());
            }
            else
            {
                SvXMLElementExport aGroupSection(*this,XML_NAMESPACE_REPORT, XML_DETAIL, sal_True, sal_True);
                exportSection(_xReportDefinition->getDetail());
            }
        }
    }
    return bGroupExported;
}
// -----------------------------------------------------------------------------
void ORptExport::exportAutoStyle(XPropertySet* _xProp,const Reference<XFormattedField>& _xParentFormattedField)
{
    const uno::Reference<report::XReportControlFormat> xFormat(_xProp,uno::UNO_QUERY);
    if ( xFormat.is() )
    {
        try
        {
            const awt::FontDescriptor aFont = xFormat->getFontDescriptor();
            OSL_ENSURE(aFont.Name.getLength(),"No Font Name !");
            GetFontAutoStylePool()->Add(aFont.Name,aFont.StyleName,aFont.Family,aFont.Pitch,aFont.CharSet );
        }
        catch(beans::UnknownPropertyException&)
        {
            // not interested in
        }
    }
    const uno::Reference< report::XShape> xShape(_xProp,uno::UNO_QUERY);
    if ( xShape.is() )
    {
        ::std::vector< XMLPropertyState > aPropertyStates( m_xParaPropMapper->Filter(_xProp) );
        if ( !aPropertyStates.empty() )
            m_aAutoStyleNames.insert( TPropertyStyleMap::value_type(_xProp,GetAutoStylePool()->Add( XML_STYLE_FAMILY_TEXT_PARAGRAPH, aPropertyStates )));
    }
    ::std::vector< XMLPropertyState > aPropertyStates( m_xCellStylesExportPropertySetMapper->Filter(_xProp) );
    Reference<XFixedLine> xFixedLine(_xProp,uno::UNO_QUERY);
    if ( xFixedLine.is() )
    {
        uno::Reference<beans::XPropertySet> xBorderProp = OXMLHelper::createBorderPropertySet();
        table::BorderLine aValue;
        aValue.Color = COL_BLACK;
        aValue.InnerLineWidth = aValue.LineDistance = 0;
        aValue.OuterLineWidth = 2;

        awt::Point aPos = xFixedLine->getPosition();
        awt::Size aSize = xFixedLine->getSize();
        sal_Int32 nSectionHeight = xFixedLine->getSection()->getHeight();

        ::rtl::OUString sBorderProp;
        ::std::vector< ::rtl::OUString> aProps;
        if ( xFixedLine->getOrientation() == 1 ) // vertical
        {
            // check if border should be left
            if ( !aPos.X )
            {
                sBorderProp = PROPERTY_BORDERLEFT;
                aProps.push_back(PROPERTY_BORDERRIGHT);
            }
            else
            {
                sBorderProp = PROPERTY_BORDERRIGHT;
                aProps.push_back(PROPERTY_BORDERLEFT);
            }
            aProps.push_back(PROPERTY_BORDERTOP);
            aProps.push_back(PROPERTY_BORDERBOTTOM);
        }
        else // horizontal
        {
            // check if border should be bottom
            if ( (aPos.Y + aSize.Height) == nSectionHeight )
            {
                sBorderProp = PROPERTY_BORDERBOTTOM;
                aProps.push_back(PROPERTY_BORDERTOP);
            }
            else
            {
                sBorderProp = PROPERTY_BORDERTOP;
                aProps.push_back(PROPERTY_BORDERBOTTOM);
            }
            aProps.push_back(PROPERTY_BORDERRIGHT);
            aProps.push_back(PROPERTY_BORDERLEFT);
        }

        xBorderProp->setPropertyValue(sBorderProp,uno::makeAny(aValue));

        aValue.Color = aValue.OuterLineWidth = 0;
        uno::Any aEmpty;
        aEmpty <<= aValue;
        ::std::for_each(aProps.begin(),aProps.end(),
            ::boost::bind(&beans::XPropertySet::setPropertyValue,xBorderProp,_1,aEmpty));

        ::std::vector< XMLPropertyState > aBorderStates(m_xCellStylesExportPropertySetMapper->Filter(xBorderProp));
        ::std::copy(aBorderStates.begin(),aBorderStates.end(),::std::back_inserter(aPropertyStates));
    }
    else
    {
        sal_Int32 nTextAlignIndex = m_xCellStylesExportPropertySetMapper->getPropertySetMapper()->FindEntryIndex( CTF_SD_SHAPE_PARA_ADJUST );
        if ( nTextAlignIndex != -1 )
        {
            ::std::vector< XMLPropertyState >::iterator aIter = aPropertyStates.begin();
            ::std::vector< XMLPropertyState >::iterator aEnd = aPropertyStates.end();
            for (; aIter != aEnd; ++aIter)
            {
                if ( aIter->mnIndex == nTextAlignIndex )
                {
                    sal_Int16 nTextAlign = 0;
                    aIter->maValue >>= nTextAlign;
                    switch(nTextAlign)
                    {
                        case awt::TextAlign::LEFT:
                            nTextAlign = style::ParagraphAdjust_LEFT;
                            break;
                        case awt::TextAlign::CENTER:
                            nTextAlign = style::ParagraphAdjust_CENTER;
                            break;
                        case awt::TextAlign::RIGHT:
                            nTextAlign = style::ParagraphAdjust_RIGHT;
                            break;
                        default:
                            OSL_ENSURE(0,"Illegal text alignment value!");
                            break;
                    }
                    aIter->maValue <<= nTextAlign;
                    break;
                }
            }
        }
        const Reference<XFormattedField> xFormattedField(_xProp,uno::UNO_QUERY);
        if ( (_xParentFormattedField.is() || xFormattedField.is()) && !aPropertyStates.empty() )
        {
            sal_Int32 nNumberFormat = 0;
            if ( _xParentFormattedField.is() )
                nNumberFormat = _xParentFormattedField->getFormatKey();
            else
                nNumberFormat = xFormattedField->getFormatKey();
            {
                sal_Int32 nStyleMapIndex = m_xCellStylesExportPropertySetMapper->getPropertySetMapper()->FindEntryIndex( CTF_RPT_NUMBERFORMAT );
                addDataStyle(nNumberFormat);
                XMLPropertyState aNumberStyleState( nStyleMapIndex, uno::makeAny( getDataStyleName(nNumberFormat) ) );
                aPropertyStates.push_back( aNumberStyleState );
            }
        }
    }


    if ( !aPropertyStates.empty() )
        m_aAutoStyleNames.insert( TPropertyStyleMap::value_type(_xProp,GetAutoStylePool()->Add( XML_STYLE_FAMILY_TABLE_CELL, aPropertyStates )));
}
// -----------------------------------------------------------------------------
void ORptExport::exportAutoStyle(const Reference<XSection>& _xProp)
{
    ::std::vector< XMLPropertyState > aPropertyStates( m_xTableStylesExportPropertySetMapper->Filter(_xProp.get()) );
    if ( !aPropertyStates.empty() )
        m_aAutoStyleNames.insert( TPropertyStyleMap::value_type(_xProp.get(),GetAutoStylePool()->Add( XML_STYLE_FAMILY_TABLE_TABLE, aPropertyStates )));
}
// -----------------------------------------------------------------------------
void ORptExport::SetBodyAttributes()
{
    Reference<XReportDefinition> xProp(getReportDefinition());
    exportReportAttributes(xProp);
}
// -----------------------------------------------------------------------------
void ORptExport::exportReportAttributes(const Reference<XReportDefinition>& _xReport)
{
    if ( _xReport.is() )
    {
        ::rtl::OUStringBuffer sValue;
        const SvXMLEnumMapEntry* aXML_CommnadTypeEnumMap = OXMLHelper::GetCommandTypeOptions();
        if ( SvXMLUnitConverter::convertEnum( sValue, static_cast<sal_uInt16>(_xReport->getCommandType()),aXML_CommnadTypeEnumMap ) )
            AddAttribute(XML_NAMESPACE_REPORT, XML_COMMAND_TYPE,sValue.makeStringAndClear());

        ::rtl::OUString sComamnd = _xReport->getCommand();
        if ( sComamnd.getLength() )
            AddAttribute(XML_NAMESPACE_REPORT, XML_COMMAND, sComamnd);

        ::rtl::OUString sFilter( _xReport->getFilter() );
        if ( sFilter.getLength() )
            AddAttribute( XML_NAMESPACE_REPORT, XML_FILTER, sFilter );

        AddAttribute(XML_NAMESPACE_OFFICE, XML_MIMETYPE,_xReport->getMimeType());

        sal_Bool bEscapeProcessing( _xReport->getEscapeProcessing() );
        if ( !bEscapeProcessing )
            AddAttribute( XML_NAMESPACE_REPORT, XML_ESCAPE_PROCESSING, ::xmloff::token::GetXMLToken( XML_FALSE ) );

        ::rtl::OUString sName = _xReport->getCaption();
        if ( sName.getLength() )
            AddAttribute(XML_NAMESPACE_OFFICE, XML_CAPTION,sName);
        sName = _xReport->getName();
        if ( sName.getLength() )
            AddAttribute(XML_NAMESPACE_DRAW, XML_NAME,sName);
    }
}
// -----------------------------------------------------------------------------
void ORptExport::_ExportContent()
{
    exportReport(getReportDefinition());
}
// -----------------------------------------------------------------------------
void ORptExport::_ExportMasterStyles()
{
    GetPageExport()->exportMasterStyles( sal_True );
}
// -----------------------------------------------------------------------------
void ORptExport::collectComponentStyles()
{
    if ( m_bAllreadyFilled )
        return;

    m_bAllreadyFilled = sal_True;
    Reference<XReportDefinition> xProp(getReportDefinition());
    if ( xProp.is() )
    {
        uno::Reference< report::XSection> xParent(xProp->getParent(),uno::UNO_QUERY);
        if ( xParent.is() )
            exportAutoStyle(xProp.get());

        if ( xProp->getReportHeaderOn() )
            exportSectionAutoStyle(xProp->getReportHeader());
        if ( xProp->getPageHeaderOn() )
            exportSectionAutoStyle(xProp->getPageHeader());

        exportGroup(xProp,0,sal_True);

        if ( xProp->getPageFooterOn() )
            exportSectionAutoStyle(xProp->getPageFooter());
        if ( xProp->getReportFooterOn() )
            exportSectionAutoStyle(xProp->getReportFooter());
    }
}
// -----------------------------------------------------------------------------
void ORptExport::_ExportAutoStyles()
{
    // there are no styles that require their own autostyles
    if ( getExportFlags() & EXPORT_CONTENT )
    {
        collectComponentStyles();
        GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_TABLE
                                        ,GetDocHandler()
                                        ,GetMM100UnitConverter()
                                        ,GetNamespaceMap()
        );
        GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_COLUMN
                                        ,GetDocHandler()
                                        ,GetMM100UnitConverter()
                                        ,GetNamespaceMap()
        );
        GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_ROW
                                        ,GetDocHandler()
                                        ,GetMM100UnitConverter()
                                        ,GetNamespaceMap()
        );

        GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_CELL
                                        ,GetDocHandler()
                                        ,GetMM100UnitConverter()
                                        ,GetNamespaceMap()
        );


        /*GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_REPORT_ID
                                        ,GetDocHandler()
                                        ,GetMM100UnitConverter()
                                        ,GetNamespaceMap()
        );*/
        exportDataStyles();
        GetShapeExport()->exportAutoStyles();
    }
    // exported in _ExportMasterStyles
    if( (getExportFlags() & EXPORT_MASTERSTYLES) != 0 )
        GetPageExport()->collectAutoStyles( sal_False );
    if( (getExportFlags() & EXPORT_MASTERSTYLES) != 0 )
        GetPageExport()->exportAutoStyles();
}
// -----------------------------------------------------------------------------
void ORptExport::_ExportStyles(BOOL bUsed)
{
    SvXMLExport::_ExportStyles(bUsed);

    // write draw:style-name for object graphic-styles
    GetShapeExport()->ExportGraphicDefaults();
}
// -----------------------------------------------------------------------------
sal_uInt32 ORptExport::exportDoc(enum ::xmloff::token::XMLTokenEnum eClass)
{
    return SvXMLExport::exportDoc( eClass );
}
// -----------------------------------------------------------------------------
void ORptExport::GetViewSettings(Sequence<PropertyValue>& /*aProps*/)
{
    /*Reference<XReportDefinition> xReport(getReportDefinition());
    if ( xReport.is() && (Reference<frame::XController > xController = xReport->getCurrentController()).is() )
    {
        xController->getViewData() >>= aProps;
    }*/
}
// -----------------------------------------------------------------------------
void ORptExport::GetConfigurationSettings(Sequence<PropertyValue>& /*aProps*/)
{
}
// -----------------------------------------------------------------------------
::rtl::OUString ORptExport::implConvertNumber(sal_Int32 _nValue)
{
    ::rtl::OUStringBuffer aBuffer;
    GetMM100UnitConverter().convertNumber(aBuffer, _nValue);
    return aBuffer.makeStringAndClear();
}
// -----------------------------------------------------------------------------
::rtl::OUString ORptExport::implConvertMeasure(sal_Int32 _nValue)
{
    ::rtl::OUStringBuffer aBuffer;
    GetMM100UnitConverter().convertMeasure(aBuffer, _nValue);
    return aBuffer.makeStringAndClear();
}
// -----------------------------------------------------------------------------
::rtl::OUString ORptExport::implConvertAny(const Any& _rValue)
{
    ::rtl::OUStringBuffer aBuffer;
    switch (_rValue.getValueTypeClass())
    {
        case TypeClass_STRING:
        {   // extract the string
            ::rtl::OUString sCurrentValue;
            _rValue >>= sCurrentValue;
            aBuffer.append(sCurrentValue);
        }
        break;
        case TypeClass_DOUBLE:
            // let the unit converter format is as string
            GetMM100UnitConverter().convertDouble(aBuffer, getDouble(_rValue));
            break;
        case TypeClass_BOOLEAN:
            aBuffer = getBOOL(_rValue) ? ::xmloff::token::GetXMLToken(XML_TRUE) : ::xmloff::token::GetXMLToken(XML_FALSE);
            break;
        case TypeClass_BYTE:
        case TypeClass_SHORT:
        case TypeClass_LONG:
            // let the unit converter format is as string
            GetMM100UnitConverter().convertNumber(aBuffer, getINT32(_rValue));
            break;
        default:
            OSL_ENSURE(0,"ORptExport::implConvertAny: Invalid type");
    }

    return aBuffer.makeStringAndClear();
}
// -----------------------------------------------------------------------------
UniReference < XMLPropertySetMapper > ORptExport::GetCellStylePropertyMapper() const
{
    return m_xCellStylesPropertySetMapper;
}
// -----------------------------------------------------------------------------
SvXMLAutoStylePoolP* ORptExport::CreateAutoStylePool()
{
    return new OXMLAutoStylePoolP(*this);
}
// -----------------------------------------------------------------------------
void SAL_CALL ORptExport::setSourceDocument( const Reference< XComponent >& xDoc ) throw(IllegalArgumentException, RuntimeException)
{
    m_xReportDefinition.set(xDoc,UNO_QUERY_THROW);
    OSL_ENSURE(m_xReportDefinition.is(),"DataSource is NULL!");

    //if ( !GetNumberFormatsSupplier().is() )
    //    SetNumberFormatsSupplier(OXMLHelper::GetNumberFormatsSupplier(m_xReportDefinition));

    SvXMLExport::setSourceDocument(xDoc);
}
// -----------------------------------------------------------------------------
void ORptExport::_ExportFontDecls()
{
    GetFontAutoStylePool(); // make sure the pool is created
    collectComponentStyles();
    SvXMLExport::_ExportFontDecls();
}
// -----------------------------------------------------------------------------
void ORptExport::exportParagraph(const Reference< XReportControlModel >& _xReportElement)
{
    OSL_PRECOND(_xReportElement.is(),"Element is null!");
    // start <text:p>
    SvXMLElementExport aParagraphContent(*this,XML_NAMESPACE_TEXT, XML_P, sal_False, sal_False);
    if ( Reference<XFormattedField>(_xReportElement,uno::UNO_QUERY).is() )
    {
        ::rtl::OUString sFieldData = _xReportElement->getDataField();
        static const ::rtl::OUString s_sPageNumber(RTL_CONSTASCII_USTRINGPARAM("PageNumber()"));
        static const ::rtl::OUString s_sPageCount(RTL_CONSTASCII_USTRINGPARAM("PageCount()"));
        static const ::rtl::OUString s_sReportPrefix(RTL_CONSTASCII_USTRINGPARAM("rpt:"));
        static const ::rtl::OUString s_sConcatOperator(RTL_CONSTASCII_USTRINGPARAM("&"));
        sFieldData = sFieldData.copy(s_sReportPrefix.getLength(),sFieldData.getLength() - s_sReportPrefix.getLength());
        sal_Int32 nPageNumberIndex = sFieldData.indexOf(s_sPageNumber);
        if ( nPageNumberIndex != -1 )
        {
            sal_Int32 nIndex = 0;
            do
            {
                ::rtl::OUString sToken = sFieldData.getToken( 0, '&', nIndex );
                sToken = sToken.trim();
                if ( sToken.getLength() )
                {
                    if ( sToken == s_sPageNumber )
                    {
                        static const ::rtl::OUString s_sCurrent(RTL_CONSTASCII_USTRINGPARAM("current"));
                        AddAttribute(XML_NAMESPACE_TEXT, XML_SELECT_PAGE, s_sCurrent );
                        SvXMLElementExport aPageNumber(*this,XML_NAMESPACE_TEXT, XML_PAGE_NUMBER, sal_False, sal_False);
                        Characters(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("1")));
                    }
                    else if ( sToken == s_sPageCount )
                    {
                        SvXMLElementExport aPageNumber(*this,XML_NAMESPACE_TEXT, XML_PAGE_COUNT, sal_False, sal_False);
                        Characters(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("1")));
                    }
                    else
                    {

                        if ( sToken.indexOf('"') == 0 && sToken.lastIndexOf('"') == sToken.getLength()-1 )
                            sToken = sToken.copy(1,sToken.getLength()-2);

                        sal_Bool bPrevCharIsSpace = sal_False;
                        GetTextParagraphExport()->exportText(sToken,bPrevCharIsSpace);
                    }
                }
            }
            while ( nIndex >= 0 );
        }
    }
    Reference< XFixedText > xFT(_xReportElement,UNO_QUERY);
    if ( xFT.is() )
    {
        ::rtl::OUString sExpr = xFT->getLabel();
        sal_Bool bPrevCharIsSpace = sal_False;
        GetTextParagraphExport()->exportText(sExpr,bPrevCharIsSpace);
    }
}
// -----------------------------------------------------------------------------
XMLShapeExport* ORptExport::CreateShapeExport()
{
    XMLShapeExport* pShapeExport = new XMLShapeExport( *this, XMLTextParagraphExport::CreateShapeExtPropMapper( *this ) );
    return pShapeExport;
}
// -----------------------------------------------------------------------------
void ORptExport::exportShapes(const Reference< XSection>& _xSection,bool _bAddParagraph)
{
    UniReference< XMLShapeExport > xShapeExport = GetShapeExport();
    xShapeExport->seekShapes(_xSection.get());
    const sal_Int32 nCount = _xSection->getCount();
    awt::Point aRefPoint;
    ::std::auto_ptr<SvXMLElementExport> pParagraphContent;
    if ( _bAddParagraph )
        pParagraphContent.reset(new SvXMLElementExport(*this,XML_NAMESPACE_TEXT, XML_P, sal_True, sal_False));
    aRefPoint.X = rptui::getStyleProperty<sal_Int32>(_xSection->getReportDefinition(),PROPERTY_LEFTMARGIN);
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        uno::Reference< XShape > xShape(_xSection->getByIndex(i),uno::UNO_QUERY);
        if ( xShape.is() )
        {
            AddAttribute( XML_NAMESPACE_TEXT, XML_ANCHOR_TYPE, XML_PARAGRAPH );
            xShapeExport->exportShape(xShape.get(),SEF_DEFAULT|SEF_EXPORT_NO_WS,&aRefPoint);
        }
    }
}
// -----------------------------------------------------------------------------
void ORptExport::exportGroupsExpressionAsFunction(const Reference< XGroups>& _xGroups)
{
    if ( _xGroups.is() )
    {
        uno::Reference< XFunctions> xFunctions = _xGroups->getReportDefinition()->getFunctions();
        const sal_Int32 nCount = _xGroups->getCount();
        for (sal_Int32 i = 0; i < nCount; ++i)
        {
            uno::Reference< XGroup> xGroup(_xGroups->getByIndex(i),uno::UNO_QUERY_THROW);
            const ::sal_Int16 nGroupOn = xGroup->getGroupOn();
            if ( nGroupOn != report::GroupOn::DEFAULT )
            {
                uno::Reference< XFunction> xFunction = xFunctions->createFunction();
                ::rtl::OUString sFunction,sPrefix,sPostfix;
                ::rtl::OUString sExpression = xGroup->getExpression();
                switch(nGroupOn)
                {
                    case report::GroupOn::PREFIX_CHARACTERS:
                        sFunction = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LEFT"));
                        sPrefix = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(";")) + ::rtl::OUString::valueOf(xGroup->getGroupInterval());
                        break;
                    case report::GroupOn::YEAR:
                        sFunction = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("YEAR"));
                        break;
                    case report::GroupOn::QUARTAL:
                        sFunction = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MONTH"));
                        sPostfix = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/4"));
                        break;
                    case report::GroupOn::MONTH:
                        sFunction = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MONTH"));
                        break;
                    case report::GroupOn::WEEK:
                        sFunction = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WEEK"));
                        break;
                    case report::GroupOn::DAY:
                        sFunction = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DAY"));
                        break;
                    case report::GroupOn::HOUR:
                        sFunction = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HOUR"));
                        break;
                    case report::GroupOn::MINUTE:
                        sFunction = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MINUTE"));
                        break;
                    case report::GroupOn::INTERVAL:
                        {
                            sFunction = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("INT"));
                            uno::Reference< XFunction> xCountFunction = xFunctions->createFunction();
                            xCountFunction->setInitialFormula(beans::Optional< ::rtl::OUString>(sal_True,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("rpt:1"))));
                            ::rtl::OUString sCountName = sFunction + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_count_")) + sExpression;
                            xCountFunction->setName(sCountName);
                            xCountFunction->setFormula(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("rpt:[")) + sCountName + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("] + 1")));
                            exportFunction(xCountFunction);
                            sExpression = sCountName;
                            sPrefix = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" / ")) + ::rtl::OUString::valueOf(xGroup->getGroupInterval());
                        }
                        break;
                    default:
                        ;
                }
                if ( sFunction.getLength() )
                {

                    xFunction->setName(sFunction + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_")) + sExpression);
                    sFunction = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("rpt:")) + sFunction;
                    sFunction += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("(["));
                    sFunction += sExpression;
                    sFunction += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("]"));

                    if ( sPrefix.getLength() )
                        sFunction += sPrefix;
                    sFunction += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")"));
                    if ( sPostfix.getLength() )
                        sFunction += sPostfix;
                    xFunction->setFormula(sFunction);
                    exportFunction(xFunction);
                    m_aGroupFunctionMap.insert(TGroupFunctionMap::value_type(xGroup,xFunction));
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
}// rptxml
// -----------------------------------------------------------------------------

