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

#ifndef RPT_XMLEXPORT_HXX
#define RPT_XMLEXPORT_HXX

#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/report/XSection.hpp>
#include <com/sun/star/report/XReportControlModel.hpp>
#include <com/sun/star/report/XFormattedField.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase5.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <osl/diagnose.h>
#include <unotools/tempfile.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <comphelper/stl_types.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
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
    void                    exportMasterDetailFields(const Reference<XReportComponent>& _xReportComponet);
    void                    exportComponent(const Reference<XReportComponent>& _xReportComponent);
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

    ::rtl::OUString         implConvertNumber(sal_Int32 _nValue);

private:
                                    ORptExport();
    virtual void                    SetBodyAttributes();
protected:

    virtual void                    _ExportStyles( sal_Bool bUsed );
    virtual void                    _ExportAutoStyles();
    virtual void                    _ExportContent();
    virtual void                    _ExportMasterStyles();
    virtual void                    _ExportFontDecls();
    virtual sal_uInt32              exportDoc( enum ::xmloff::token::XMLTokenEnum eClass );
    virtual SvXMLAutoStylePoolP*    CreateAutoStylePool();
    virtual XMLShapeExport*         CreateShapeExport();

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

/** Exports only meta data
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptMetaExportHelper
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
