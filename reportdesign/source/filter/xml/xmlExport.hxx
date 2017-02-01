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
using namespace css::uno;
using namespace css::container;
using namespace css::lang;
using namespace css::beans;
using namespace css::document;
using namespace css::text;
using namespace css::report;
using namespace css::io;
using namespace css::xml::sax;


#define PROGRESS_BAR_STEP 20

class ORptExport : public SvXMLExport
{
public:
    struct TCell
    {
        sal_Int32 nColSpan;
        sal_Int32 nRowSpan;
        Reference<XReportComponent> xElement;
        bool      bSet;
        TCell(  sal_Int32 _nColSpan,
                sal_Int32 _nRowSpan,
                Reference<XReportComponent> const & _xElement = Reference<XReportComponent>()) :
        nColSpan(_nColSpan)
        ,nRowSpan(_nRowSpan)
        ,xElement(_xElement)
        ,bSet(xElement.is())
        {}

        TCell( ) :
        nColSpan(1)
        ,nRowSpan(1)
        ,bSet(true)
        {}
    };
    typedef ::std::pair< OUString ,OUString> TStringPair;
    typedef ::std::vector< OUString>                         TStringVec;
    typedef ::std::map< Reference<XPropertySet> ,OUString >  TPropertyStyleMap;
    typedef ::std::map< Reference<XPropertySet> ,  TStringVec>      TGridStyleMap;
    typedef ::std::vector< TCell >                                  TRow;
    typedef ::std::vector< ::std::pair< bool, TRow > >              TGrid;
    typedef ::std::map< Reference<XPropertySet> ,TGrid >            TSectionsGrid;
    typedef ::std::map< Reference<XGroup> ,Reference<XFunction> >   TGroupFunctionMap;
private:
    TSectionsGrid                                   m_aSectionsGrid;

    TPropertyStyleMap                               m_aAutoStyleNames;
    TGridStyleMap                                   m_aColumnStyleNames;
    TGridStyleMap                                   m_aRowStyleNames;
    TGroupFunctionMap                               m_aGroupFunctionMap;

    OUString                                 m_sTableStyle;
    OUString                                 m_sCellStyle;
    OUString                                 m_sColumnStyle;
    rtl::Reference < SvXMLExportPropertyMapper>       m_xTableStylesExportPropertySetMapper;
    rtl::Reference < SvXMLExportPropertyMapper>       m_xCellStylesExportPropertySetMapper;
    rtl::Reference < SvXMLExportPropertyMapper>       m_xColumnStylesExportPropertySetMapper;
    rtl::Reference < SvXMLExportPropertyMapper>       m_xRowStylesExportPropertySetMapper;
    rtl::Reference < SvXMLExportPropertyMapper >      m_xParaPropMapper;
    rtl::Reference < XMLPropertyHandlerFactory >      m_xPropHdlFactory;

    mutable rtl::Reference < XMLPropertySetMapper >   m_xCellStylesPropertySetMapper;
    Reference<XReportDefinition>                    m_xReportDefinition;
    bool                                        m_bAllreadyFilled;

    void                    exportReport(const Reference<XReportDefinition>& _xReportDefinition); /// <element name="office:report">
    void                    exportReportAttributes(const Reference<XReportDefinition>& _xReport);
    void                    exportFunctions(const Reference<XIndexAccess>& _xFunctions); /// <ref name="rpt-function"/>
    void                    exportFunction(const Reference< XFunction>& _xFunction);
    void                    exportMasterDetailFields(const Reference<XReportComponent>& _xReportComponet);
    void                    exportComponent(const Reference<XReportComponent>& _xReportComponent);
    void                    exportGroup(const Reference<XReportDefinition>& _xReportDefinition,sal_Int32 _nPos,bool _bExportAutoStyle = false);
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
    static OUString  convertFormula(const OUString& _sFormula);

    virtual void                    SetBodyAttributes() override;

protected:

    virtual void                    ExportStyles_( bool bUsed ) override;
    virtual void                    ExportAutoStyles_() override;
    virtual void                    ExportContent_() override;
    virtual void                    ExportMasterStyles_() override;
    virtual void                    ExportFontDecls_() override;
    virtual SvXMLAutoStylePoolP*    CreateAutoStylePool() override;
    virtual XMLShapeExport*         CreateShapeExport() override;

    virtual                 ~ORptExport() override {};
public:

    ORptExport(const Reference< XComponentContext >& _rxContext, OUString const & implementationName, SvXMLExportFlags nExportFlag);

    /// @throws css::uno::RuntimeException
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();
    /// @throws css::uno::RuntimeException
    static OUString getImplementationName_Static();
    static css::uno::Reference< css::uno::XInterface > SAL_CALL
        create(css::uno::Reference< css::uno::XComponentContext > const & xContext);

    // XExporter
    virtual void SAL_CALL setSourceDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) override;

    const Reference<XReportDefinition>& getReportDefinition() const { return m_xReportDefinition; }

    const rtl::Reference < XMLPropertySetMapper >& GetCellStylePropertyMapper() const { return m_xCellStylesPropertySetMapper;}
};

/** Exports only settings
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptExportHelper
{
public:
    /// @throws css::uno::RuntimeException
    static OUString getImplementationName_Static(  );
    /// @throws css::uno::RuntimeException
    static Sequence< OUString > getSupportedServiceNames_Static(  );
    static css::uno::Reference< css::uno::XInterface > SAL_CALL
        create(css::uno::Reference< css::uno::XComponentContext > const & xContext);
};

/** Exports only content
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptContentExportHelper
{
public:
    /// @throws css::uno::RuntimeException
    static OUString getImplementationName_Static(  );
    /// @throws css::uno::RuntimeException
    static Sequence< OUString > getSupportedServiceNames_Static(  );
    static css::uno::Reference< css::uno::XInterface > SAL_CALL
        create(css::uno::Reference< css::uno::XComponentContext > const & xContext);
};

/** Exports only styles
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptStylesExportHelper
{
public:
    /// @throws css::uno::RuntimeException
    static OUString getImplementationName_Static(  );
    /// @throws css::uno::RuntimeException
    static Sequence< OUString > getSupportedServiceNames_Static(  );
    static css::uno::Reference< css::uno::XInterface > SAL_CALL
        create(css::uno::Reference< css::uno::XComponentContext > const & xContext);
};

/** Exports only meta data
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptMetaExportHelper
{
public:
    /// @throws css::uno::RuntimeException
    static OUString getImplementationName_Static(  );
    /// @throws css::uno::RuntimeException
    static Sequence< OUString > getSupportedServiceNames_Static(  );
    static css::uno::Reference< css::uno::XInterface > SAL_CALL
        create(css::uno::Reference< css::uno::XComponentContext > const & xContext);
};
/** Exports all
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ODBFullExportHelper
{
public:
    /// @throws css::uno::RuntimeException
    static OUString getImplementationName_Static(  );
    /// @throws css::uno::RuntimeException
    static Sequence< OUString > getSupportedServiceNames_Static(  );
    static css::uno::Reference< css::uno::XInterface > SAL_CALL
        create(css::uno::Reference< css::uno::XComponentContext > const & xContext);
};

} // rptxml

#endif // INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
