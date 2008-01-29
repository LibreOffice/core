#ifndef RPT_XMLEXPORT_HXX
#define RPT_XMLEXPORT_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlExport.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:45:12 $
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


#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XIMPORTER_HPP_
#include <com/sun/star/document/XImporter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTDEFINITION_HPP_
#include <com/sun/star/report/XReportDefinition.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XSECTION_HPP_
#include <com/sun/star/report/XSection.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTCONTROLMODEL_HPP_
#include <com/sun/star/report/XReportControlModel.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XFORMATTEDFIELD_HPP_
#include <com/sun/star/report/XFormattedField.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include <xmloff/xmlexp.hxx>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#include <memory>

namespace rptxml
{
using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::report;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::xml::sax;
// -------------
// - ORptExport -
// -------------
#define PROGRESS_BAR_STEP 20

class ORptExport : public SvXMLExport
{
public:
    struct TCell
    {
        sal_Int32 nWidth;
        sal_Int32 nHeight;
        sal_Int32 nColSpan;
        sal_Int32 nRowSpan;
        Reference<XReportComponent> xElement;
        bool      bSet;
        TCell(  sal_Int32 _nWidth,
                sal_Int32 _nHeight,
                sal_Int32 _nColSpan,
                sal_Int32 _nRowSpan,
                Reference<XReportComponent> _xElement = Reference<XReportComponent>()) :
        nWidth(_nWidth)
        ,nHeight(_nHeight)
        ,nColSpan(_nColSpan)
        ,nRowSpan(_nRowSpan)
        ,xElement(_xElement)
        ,bSet(xElement.is())
        {}

        TCell( ) :
        nWidth(0)
        ,nHeight(0)
        ,nColSpan(1)
        ,nRowSpan(1)
        ,bSet(true)
        {}
    };
    typedef ::std::pair< ::rtl::OUString ,::rtl::OUString> TStringPair;
    typedef struct
    {
        ::rtl::OUString sText;
        ::rtl::OUString sField;
        ::rtl::OUString sDecimal;
        ::rtl::OUString sThousand;
    } TDelimiter;
    typedef ::std::vector< ::rtl::OUString>                         TStringVec;
    typedef ::std::map< Reference<XPropertySet> ,::rtl::OUString >  TPropertyStyleMap;
    typedef ::std::map< Reference<XPropertySet> ,  TStringVec>      TGridStyleMap;
    typedef ::std::vector< TCell >                                  TRow;
    typedef ::std::vector< ::std::pair< sal_Bool, TRow > >          TGrid;
    typedef ::std::map< Reference<XPropertySet> ,TGrid >            TSectionsGrid;
    typedef ::std::map< Reference<XGroup> ,Reference<XFunction> >   TGroupFunctionMap;
private:
    ::std::auto_ptr< TStringPair >                  m_aAutoIncrement;
    ::std::auto_ptr< TDelimiter >                   m_aDelimiter;
    ::std::vector< Any >                            m_aDataSourceSettings;
    TSectionsGrid                                   m_aSectionsGrid;

    TPropertyStyleMap                               m_aAutoStyleNames;
    TGridStyleMap                                   m_aColumnStyleNames;
    TGridStyleMap                                   m_aRowStyleNames;
    TGroupFunctionMap                               m_aGroupFunctionMap;

    ::rtl::OUString                                 m_sCharSet;
    ::rtl::OUString                                 m_sTableStyle;
    ::rtl::OUString                                 m_sCellStyle;
    ::rtl::OUString                                 m_sColumnStyle;
    Any                                             m_aPreviewMode;
    UniReference < SvXMLExportPropertyMapper>       m_xExportHelper;
    UniReference < SvXMLExportPropertyMapper>       m_xSectionPropMapper;
    UniReference < SvXMLExportPropertyMapper>       m_xTableStylesExportPropertySetMapper;
    UniReference < SvXMLExportPropertyMapper>       m_xCellStylesExportPropertySetMapper;
    UniReference < SvXMLExportPropertyMapper>       m_xColumnStylesExportPropertySetMapper;
    UniReference < SvXMLExportPropertyMapper>       m_xRowStylesExportPropertySetMapper;
    UniReference < SvXMLExportPropertyMapper >      m_xParaPropMapper;
    UniReference < XMLPropertyHandlerFactory >      m_xPropHdlFactory;

    mutable UniReference < XMLPropertySetMapper >   m_xControlStylePropertyMapper;
    mutable UniReference < XMLPropertySetMapper >   m_xColumnStylesPropertySetMapper;
    mutable UniReference < XMLPropertySetMapper >   m_xCellStylesPropertySetMapper;
    Reference<XReportDefinition>                    m_xReportDefinition;
    sal_Bool                                        m_bAllreadyFilled;

    void                    exportReport(const Reference<XReportDefinition>& _xReportDefinition); /// <element name="office:report">
    void                    exportReportAttributes(const Reference<XReportDefinition>& _xReport);
    void                    exportFunctions(const Reference<XIndexAccess>& _xFunctions); /// <ref name="rpt-function"/>
    void                    exportFunction(const Reference< XFunction>& _xFunction);
    void                    exportMasterDetailFields(const Reference<XReportDefinition>& _xReportDefinition);
    void                    exportComponent(const Reference<XReportComponent>& _xReportComponent);
    void                    exportImagePosition(sal_Int16 _nImagePosition);
    sal_Bool                exportGroup(const Reference<XReportDefinition>& _xReportDefinition,sal_Int32 _nPos,sal_Bool _bExportAutoStyle = sal_False);
    void                    exportStyleName(XPropertySet* _xProp,SvXMLAttributeList& _rAtt,const ::rtl::OUString& _sName);
    void                    exportSection(const Reference<XSection>& _xProp,bool bHeader = false);
    void                    exportContainer(const Reference< XSection>& _xSection);
    void                    exportShapes(const Reference< XSection>& _xSection,bool _bAddParagraph = true);
    void                    exportTableColumns(const Reference< XSection>& _xSection);
    void                    exportSectionAutoStyle(const Reference<XSection>& _xProp);
    void                    exportReportElement(const Reference<XReportControlModel>& _xReportElement);
    void                    exportFormatConditions(const Reference<XReportControlModel>& _xReportElement);
    void                    exportAutoStyle(XPropertySet* _xProp,const Reference<XFormattedField>& _xParentFormattedField = Reference<XFormattedField>());
    void                    exportAutoStyle(const Reference<XSection>& _xProp);
    void                    exportReportComponentAutoStyles(const Reference<XSection>& _xProp);
    void                    collectComponentStyles();
    void                    collectStyleNames(sal_Int32 _nFamily,const ::std::vector< sal_Int32>& _aSize, ORptExport::TStringVec& _rStyleNames);
    void                    exportParagraph(const Reference< XReportControlModel >& _xReportElement);
    bool                    exportFormula(enum ::xmloff::token::XMLTokenEnum eName,const ::rtl::OUString& _sFormula);
    void                    exportGroupsExpressionAsFunction(const Reference< XGroups>& _xGroups);
    ::rtl::OUString         convertFormula(const ::rtl::OUString& _sFormula);

    ::rtl::OUString         implConvertAny(const Any& _rValue);
    ::rtl::OUString         implConvertNumber(sal_Int32 _nValue);
    ::rtl::OUString         implConvertMeasure(sal_Int32 _nValue);

private:
                                    ORptExport();
    virtual void                    SetBodyAttributes();
protected:

    virtual void                    _ExportStyles( BOOL bUsed );
    virtual void                    _ExportAutoStyles();
    virtual void                    _ExportContent();
    virtual void                    _ExportMasterStyles();
    virtual void                    _ExportFontDecls();
    virtual sal_uInt32              exportDoc( enum ::xmloff::token::XMLTokenEnum eClass );
    virtual SvXMLAutoStylePoolP*    CreateAutoStylePool();
    virtual XMLShapeExport*         CreateShapeExport();

    virtual void GetViewSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps);
    virtual void GetConfigurationSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps);

    virtual                 ~ORptExport(){};
public:

    ORptExport(const Reference< XMultiServiceFactory >& _rxMSF, sal_uInt16 nExportFlag = (EXPORT_CONTENT | EXPORT_AUTOSTYLES | EXPORT_FONTDECLS));
    // XServiceInfo
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);

    // XExporter
    virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    inline Reference<XReportDefinition> getReportDefinition() const { return m_xReportDefinition; }

    UniReference < XMLPropertySetMapper > GetCellStylePropertyMapper() const;
};

/** Exports only settings
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptExportHelper
{
public:
    static ::rtl::OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
};

/** Exports only content
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptContentExportHelper
{
public:
    static ::rtl::OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
};

/** Exports only styles
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptStylesExportHelper
{
public:
    static ::rtl::OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
};

/** Exports all
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ODBFullExportHelper
{
public:
    static ::rtl::OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
};
// -----------------------------------------------------------------------------
} // rptxml
// -----------------------------------------------------------------------------
#endif // RPT_XMLEXPORT_HXX
