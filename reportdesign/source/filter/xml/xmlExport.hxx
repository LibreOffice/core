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

#ifndef INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLEXPORT_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLEXPORT_HXX

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

// - ORptExport -

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
    typedef ::std::pair< OUString ,OUString> TStringPair;
    typedef struct
    {
        OUString sText;
        OUString sField;
        OUString sDecimal;
        OUString sThousand;
    } TDelimiter;
    typedef ::std::vector< OUString>                         TStringVec;
    typedef ::std::map< Reference<XPropertySet> ,OUString >  TPropertyStyleMap;
    typedef ::std::map< Reference<XPropertySet> ,  TStringVec>      TGridStyleMap;
    typedef ::std::vector< TCell >                                  TRow;
    typedef ::std::vector< ::std::pair< sal_Bool, TRow > >          TGrid;
    typedef ::std::map< Reference<XPropertySet> ,TGrid >            TSectionsGrid;
    typedef ::std::map< Reference<XGroup> ,Reference<XFunction> >   TGroupFunctionMap;
private:
    ::std::unique_ptr< TStringPair >                m_aAutoIncrement;
    ::std::unique_ptr< TDelimiter >                 m_aDelimiter;
    ::std::vector< Any >                            m_aDataSourceSettings;
    TSectionsGrid                                   m_aSectionsGrid;

    TPropertyStyleMap                               m_aAutoStyleNames;
    TGridStyleMap                                   m_aColumnStyleNames;
    TGridStyleMap                                   m_aRowStyleNames;
    TGroupFunctionMap                               m_aGroupFunctionMap;

    OUString                                 m_sCharSet;
    OUString                                 m_sTableStyle;
    OUString                                 m_sCellStyle;
    OUString                                 m_sColumnStyle;
    Any                                             m_aPreviewMode;
    rtl::Reference < SvXMLExportPropertyMapper>       m_xExportHelper;
    rtl::Reference < SvXMLExportPropertyMapper>       m_xSectionPropMapper;
    rtl::Reference < SvXMLExportPropertyMapper>       m_xTableStylesExportPropertySetMapper;
    rtl::Reference < SvXMLExportPropertyMapper>       m_xCellStylesExportPropertySetMapper;
    rtl::Reference < SvXMLExportPropertyMapper>       m_xColumnStylesExportPropertySetMapper;
    rtl::Reference < SvXMLExportPropertyMapper>       m_xRowStylesExportPropertySetMapper;
    rtl::Reference < SvXMLExportPropertyMapper >      m_xParaPropMapper;
    rtl::Reference < XMLPropertyHandlerFactory >      m_xPropHdlFactory;

    mutable rtl::Reference < XMLPropertySetMapper >   m_xControlStylePropertyMapper;
    mutable rtl::Reference < XMLPropertySetMapper >   m_xColumnStylesPropertySetMapper;
    mutable rtl::Reference < XMLPropertySetMapper >   m_xCellStylesPropertySetMapper;
    Reference<XReportDefinition>                    m_xReportDefinition;
    bool                                        m_bAllreadyFilled;

    void                    exportReport(const Reference<XReportDefinition>& _xReportDefinition); /// <element name="office:report">
    void                    exportReportAttributes(const Reference<XReportDefinition>& _xReport);
    void                    exportFunctions(const Reference<XIndexAccess>& _xFunctions); /// <ref name="rpt-function"/>
    void                    exportFunction(const Reference< XFunction>& _xFunction);
    void                    exportMasterDetailFields(const Reference<XReportComponent>& _xReportComponet);
    void                    exportComponent(const Reference<XReportComponent>& _xReportComponent);
    bool                exportGroup(const Reference<XReportDefinition>& _xReportDefinition,sal_Int32 _nPos,bool _bExportAutoStyle = false);
    void                    exportStyleName(XPropertySet* _xProp,SvXMLAttributeList& _rAtt,const OUString& _sName);
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
    bool                    exportFormula(enum ::xmloff::token::XMLTokenEnum eName,const OUString& _sFormula);
    void                    exportGroupsExpressionAsFunction(const Reference< XGroups>& _xGroups);
    OUString         convertFormula(const OUString& _sFormula);

    OUString         implConvertNumber(sal_Int32 _nValue);

private:
                                    ORptExport();
    virtual void                    SetBodyAttributes() SAL_OVERRIDE;
protected:

    virtual void                    _ExportStyles( bool bUsed ) SAL_OVERRIDE;
    virtual void                    _ExportAutoStyles() SAL_OVERRIDE;
    virtual void                    _ExportContent() SAL_OVERRIDE;
    virtual void                    _ExportMasterStyles() SAL_OVERRIDE;
    virtual void                    _ExportFontDecls() SAL_OVERRIDE;
    virtual sal_uInt32              exportDoc( enum ::xmloff::token::XMLTokenEnum eClass ) SAL_OVERRIDE;
    virtual SvXMLAutoStylePoolP*    CreateAutoStylePool() SAL_OVERRIDE;
    virtual XMLShapeExport*         CreateShapeExport() SAL_OVERRIDE;

    virtual                 ~ORptExport(){};
public:

    ORptExport(const Reference< XComponentContext >& _rxContext, OUString const & implementationName, SvXMLExportFlags nExportFlag);

    static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( ::com::sun::star::uno::RuntimeException );
    static OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);

    // XExporter
    virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    inline Reference<XReportDefinition> getReportDefinition() const { return m_xReportDefinition; }

    rtl::Reference < XMLPropertySetMapper > GetCellStylePropertyMapper() const { return m_xCellStylesPropertySetMapper;}
};

/** Exports only settings
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptExportHelper
{
public:
    static OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
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
    static OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
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
    static OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
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
    static OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
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
    static OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
};

} // rptxml

#endif // INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
