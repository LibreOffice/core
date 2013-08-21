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

#include <editeng/eeitem.hxx>

#include "xmlexprt.hxx"
#include "XMLConverter.hxx"
#include "xmlstyle.hxx"
#include "unonames.hxx"
#include "document.hxx"
#include "olinetab.hxx"
#include "cellsuno.hxx"
#include "formulacell.hxx"
#include "rangenam.hxx"
#include "XMLTableMasterPageExport.hxx"
#include "drwlayer.hxx"
#include "XMLExportDataPilot.hxx"
#include "XMLExportDatabaseRanges.hxx"
#include "XMLExportDDELinks.hxx"
#include "XMLExportIterator.hxx"
#include "XMLColumnRowGroupExport.hxx"
#include "XMLStylesExportHelper.hxx"
#include "XMLChangeTrackingExportHelper.hxx"
#include "sheetdata.hxx"
#include "docoptio.hxx"
#include "XMLExportSharedData.hxx"
#include "chgviset.hxx"
#include "docuno.hxx"
#include "textuno.hxx"
#include "chartlis.hxx"
#include "scitems.hxx"
#include "docpool.hxx"
#include "userdat.hxx"
#include "dociter.hxx"
#include "chgtrack.hxx"
#include "rangeutl.hxx"
#include "convuno.hxx"
#include "postit.hxx"
#include "externalrefmgr.hxx"
#include "editutil.hxx"
#include "tabprotection.hxx"
#include "cachedattraccess.hxx"
#include "colorscale.hxx"
#include "conditio.hxx"
#include "cellvalue.hxx"
#include "stylehelper.hxx"
#include "edittextiterator.hxx"
#include "editattributemap.hxx"

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/families.hxx>
#include <xmloff/numehelp.hxx>
#include <xmloff/txtparae.hxx>
#include "editeng/autokernitem.hxx"
#include "editeng/charreliefitem.hxx"
#include "editeng/charscaleitem.hxx"
#include "editeng/colritem.hxx"
#include "editeng/contouritem.hxx"
#include "editeng/crossedoutitem.hxx"
#include "editeng/emphasismarkitem.hxx"
#include "editeng/escapementitem.hxx"
#include "editeng/fhgtitem.hxx"
#include "editeng/fontitem.hxx"
#include "editeng/kernitem.hxx"
#include "editeng/langitem.hxx"
#include "editeng/postitem.hxx"
#include "editeng/sectionattribute.hxx"
#include "editeng/shdditem.hxx"
#include "editeng/udlnitem.hxx"
#include "editeng/wghtitem.hxx"
#include "editeng/wrlmitem.hxx"
#include "editeng/xmlcnitm.hxx"
#include <xmloff/xmlerror.hxx>
#include <xmloff/XMLEventExport.hxx>

#include <sax/tools/converter.hxx>

#include <rtl/ustring.hxx>

#include "tools/color.hxx"
#include <rtl/math.hxx>
#include <svl/zforlist.hxx>
#include <svx/unoshape.hxx>
#include <comphelper/extract.hxx>
#include <toolkit/helper/convert.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdocapt.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svditer.hxx>
#include <svx/svdpage.hxx>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/form/XFormsSupplier2.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/sheet/XUsedAreaCursor.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XAreaLinks.hpp>
#include <com/sun/star/sheet/XAreaLink.hpp>
#include <com/sun/star/sheet/XPrintAreas.hpp>
#include <com/sun/star/sheet/XUniqueCellFormatRangesSupplier.hpp>
#include <com/sun/star/sheet/XCellRangesQuery.hpp>
#include <com/sun/star/sheet/CellFlags.hpp>
#include <com/sun/star/sheet/XArrayFormulaRange.hpp>
#include <com/sun/star/sheet/XLabelRanges.hpp>
#include <com/sun/star/sheet/XLabelRange.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>
#include <com/sun/star/sheet/XSheetLinkable.hpp>
#include <com/sun/star/sheet/GlobalSheetSettings.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/util/XMergeable.hpp>
#include <com/sun/star/util/XProtectable.hpp>

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include "XMLCodeNameProvider.hxx"

#include <sfx2/objsh.hxx>

#include <vector>
#include <vbahelper/vbaaccesshelper.hxx>

//! not found in unonames.hxx
#define SC_LAYERID "LayerID"

#define SC_VIEWCHANGES_COUNT                        13
#define SC_SHOW_CHANGES                             0
#define SC_SHOW_ACCEPTED_CHANGES                    1
#define SC_SHOW_REJECTED_CHANGES                    2
#define SC_SHOW_CHANGES_BY_DATETIME                 3
#define SC_SHOW_CHANGES_BY_DATETIME_MODE            4
#define SC_SHOW_CHANGES_BY_DATETIME_FIRST_DATETIME  5
#define SC_SHOW_CHANGES_BY_DATETIME_SECOND_DATETIME 6
#define SC_SHOW_CHANGES_BY_AUTHOR                   7
#define SC_SHOW_CHANGES_BY_AUTHOR_NAME              8
#define SC_SHOW_CHANGES_BY_COMMENT                  9
#define SC_SHOW_CHANGES_BY_COMMENT_TEXT             10
#define SC_SHOW_CHANGES_BY_RANGES                   11
#define SC_SHOW_CHANGES_BY_RANGES_LIST              12

using namespace formula;
using namespace com::sun::star;
using namespace xmloff::token;
using ::std::vector;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;

//----------------------------------------------------------------------------

namespace
{
OUString lcl_RangeSequenceToString(
    const uno::Sequence< OUString > & rRanges,
    const uno::Reference< chart2::data::XRangeXMLConversion > & xFormatConverter )
{
    OUStringBuffer aResult;
    const sal_Int32 nMaxIndex( rRanges.getLength() - 1 );
    const sal_Unicode cSep( sal_Char(' '));
    for( sal_Int32 i=0; i<=nMaxIndex; ++i )
    {
        OUString aRange( rRanges[i] );
        if( xFormatConverter.is())
            aRange = xFormatConverter->convertRangeToXML( aRange );
        aResult.append( aRange );
        if( i < nMaxIndex )
            aResult.append( cSep );
    }
    return aResult.makeStringAndClear();
}

OUString lcl_GetRawString( ScDocument* pDoc, const ScAddress& rPos )
{
    // return text/edit cell string content, with line feeds in edit cells

    if (!pDoc)
        return EMPTY_OUSTRING;

    switch (pDoc->GetCellType(rPos))
    {
        case CELLTYPE_STRING:
            return pDoc->GetString(rPos);
        case CELLTYPE_EDIT:
        {
            const EditTextObject* pData = pDoc->GetEditText(rPos);
            if (!pData)
                return EMPTY_OUSTRING;

            EditEngine& rEngine = pDoc->GetEditEngine();
            rEngine.SetText(*pData);
            return rEngine.GetText(LINEEND_LF);
        }
        break;
        default:
            ;
    }

    return EMPTY_OUSTRING;
}

} // anonymous namespace

//----------------------------------------------------------------------------

OUString SAL_CALL ScXMLOOoExport_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLExporter" );
}

uno::Sequence< OUString > SAL_CALL ScXMLOOoExport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLOOoExport_getImplementationName() );
    return uno::Sequence< OUString >( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new ScXMLExport( comphelper::getComponentContext(rSMgr), EXPORT_ALL );
}

OUString SAL_CALL ScXMLOOoExport_Meta_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLMetaExporter" );
}

uno::Sequence< OUString > SAL_CALL ScXMLOOoExport_Meta_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLOOoExport_Meta_getImplementationName() );
    return uno::Sequence< OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_Meta_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new ScXMLExport( comphelper::getComponentContext(rSMgr), EXPORT_META );
}

OUString SAL_CALL ScXMLOOoExport_Styles_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLStylesExporter" );
}

uno::Sequence< OUString > SAL_CALL ScXMLOOoExport_Styles_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLOOoExport_Styles_getImplementationName() );
    return uno::Sequence< OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_Styles_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new ScXMLExport( comphelper::getComponentContext(rSMgr), EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_FONTDECLS);
}

OUString SAL_CALL ScXMLOOoExport_Content_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLContentExporter" );
}

uno::Sequence< OUString > SAL_CALL ScXMLOOoExport_Content_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLOOoExport_Content_getImplementationName() );
    return uno::Sequence< OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_Content_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new ScXMLExport( comphelper::getComponentContext(rSMgr), EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS);
}

OUString SAL_CALL ScXMLOOoExport_Settings_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLSettingsExporter" );
}

uno::Sequence< OUString > SAL_CALL ScXMLOOoExport_Settings_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLOOoExport_Settings_getImplementationName() );
    return uno::Sequence< OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_Settings_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new ScXMLExport( comphelper::getComponentContext(rSMgr), EXPORT_SETTINGS );
}

// Oasis Filter

OUString SAL_CALL ScXMLOasisExport_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLOasisExporter" );
}

uno::Sequence< OUString > SAL_CALL ScXMLOasisExport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLOasisExport_getImplementationName() );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new ScXMLExport(comphelper::getComponentContext(rSMgr), EXPORT_ALL|EXPORT_OASIS);
}

OUString SAL_CALL ScXMLOasisExport_Meta_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLOasisMetaExporter" );
}

uno::Sequence< OUString > SAL_CALL ScXMLOasisExport_Meta_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLOasisExport_Meta_getImplementationName() );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_Meta_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new ScXMLExport(comphelper::getComponentContext(rSMgr), EXPORT_META|EXPORT_OASIS);
}

OUString SAL_CALL ScXMLOasisExport_Styles_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLOasisStylesExporter" );
}

uno::Sequence< OUString > SAL_CALL ScXMLOasisExport_Styles_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLOasisExport_Styles_getImplementationName() );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_Styles_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new ScXMLExport(comphelper::getComponentContext(rSMgr), EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_FONTDECLS|EXPORT_OASIS);
}

OUString SAL_CALL ScXMLOasisExport_Content_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLOasisContentExporter" );
}

uno::Sequence< OUString > SAL_CALL ScXMLOasisExport_Content_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLOasisExport_Content_getImplementationName() );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_Content_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new ScXMLExport(comphelper::getComponentContext(rSMgr), EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS|EXPORT_OASIS);
}

OUString SAL_CALL ScXMLOasisExport_Settings_getImplementationName() throw()
{
    return OUString( "com.sun.star.comp.Calc.XMLOasisSettingsExporter" );
}

uno::Sequence< OUString > SAL_CALL ScXMLOasisExport_Settings_getSupportedServiceNames() throw()
{
    const OUString aServiceName( ScXMLOasisExport_Settings_getImplementationName() );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_Settings_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new ScXMLExport(comphelper::getComponentContext(rSMgr), EXPORT_SETTINGS|EXPORT_OASIS);
}
//----------------------------------------------------------------------------

class ScXMLShapeExport : public XMLShapeExport
{
public:
    ScXMLShapeExport(SvXMLExport& rExp) : XMLShapeExport(rExp) {}
    ~ScXMLShapeExport();

    /** is called before a shape element for the given XShape is exported */
    virtual void onExport( const uno::Reference < drawing::XShape >& xShape );
};

ScXMLShapeExport::~ScXMLShapeExport()
{
}

void ScXMLShapeExport::onExport( const uno::Reference < drawing::XShape >& xShape )
{
    uno::Reference< beans::XPropertySet > xShapeProp( xShape, uno::UNO_QUERY );
    if( xShapeProp.is() )
    {
        sal_Int16 nLayerID = 0;
        if( (xShapeProp->getPropertyValue(OUString( SC_LAYERID )) >>= nLayerID) && (nLayerID == SC_LAYER_BACK) )
            GetExport().AddAttribute(XML_NAMESPACE_TABLE, XML_TABLE_BACKGROUND, XML_TRUE);
    }
}

//----------------------------------------------------------------------------

sal_Int16 ScXMLExport::GetFieldUnit()
{
    css::uno::Reference<css::sheet::XGlobalSheetSettings> xProperties =
                css::sheet::GlobalSheetSettings::create( comphelper::getProcessComponentContext() );
    return xProperties->getMetric();
}


// #110680#
ScXMLExport::ScXMLExport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext,
    const sal_uInt16 nExportFlag)
:   SvXMLExport( SvXMLUnitConverter::GetMeasureUnit(GetFieldUnit()),
        xContext, XML_SPREADSHEET, nExportFlag ),
    pDoc(NULL),
    nSourceStreamPos(0),
    pNumberFormatAttributesExportHelper(NULL),
    pSharedData(NULL),
    pColumnStyles(NULL),
    pRowStyles(NULL),
    pCellStyles(NULL),
    pRowFormatRanges(NULL),
    aTableStyles(),
    pGroupColumns (NULL),
    pGroupRows (NULL),
    pDefaults(NULL),
    pChartListener(NULL),
    pCurrentCell(NULL),
    pMergedRangesContainer(NULL),
    pValidationsContainer(NULL),
    pCellsItr(NULL),
    pChangeTrackingExportHelper(NULL),
    sLayerID( SC_LAYERID ),
    sCaptionShape("com.sun.star.drawing.CaptionShape"),
    nOpenRow(-1),
    nProgressCount(0),
    nCurrentTable(0),
    bHasRowHeader(false),
    bRowHeaderOpen(false),
    mbShowProgress( false )
{
    if (getExportFlags() & EXPORT_CONTENT)
    {
        pGroupColumns = new ScMyOpenCloseColumnRowGroup(*this, XML_TABLE_COLUMN_GROUP);
        pGroupRows = new ScMyOpenCloseColumnRowGroup(*this, XML_TABLE_ROW_GROUP);
        pColumnStyles = new ScColumnStyles();
        pRowStyles = new ScRowStyles();
        pRowFormatRanges = new ScRowFormatRanges();
        pMergedRangesContainer = new ScMyMergedRangesContainer();
        pValidationsContainer = new ScMyValidationsContainer();
        pCellsItr = new ScMyNotEmptyCellsIterator(*this);
        pDefaults = new ScMyDefaultStyles();
    }
    pCellStyles = new ScFormatRangeStyles();

    // document is not set here - create ScChangeTrackingExportHelper later

    xScPropHdlFactory = new XMLScPropHdlFactory;
    xCellStylesPropertySetMapper = new XMLPropertySetMapper((XMLPropertyMapEntry*)aXMLScCellStylesProperties, xScPropHdlFactory);
    xColumnStylesPropertySetMapper = new XMLPropertySetMapper((XMLPropertyMapEntry*)aXMLScColumnStylesProperties, xScPropHdlFactory);
    xRowStylesPropertySetMapper = new XMLPropertySetMapper((XMLPropertyMapEntry*)aXMLScRowStylesProperties, xScPropHdlFactory);
    xTableStylesPropertySetMapper = new XMLPropertySetMapper((XMLPropertyMapEntry*)aXMLScTableStylesProperties, xScPropHdlFactory);
    xCellStylesExportPropertySetMapper = new ScXMLCellExportPropertyMapper(xCellStylesPropertySetMapper);
    xCellStylesExportPropertySetMapper->ChainExportMapper(XMLTextParagraphExport::CreateParaExtPropMapper(*this));
    xColumnStylesExportPropertySetMapper = new ScXMLColumnExportPropertyMapper(xColumnStylesPropertySetMapper);
    xRowStylesExportPropertySetMapper = new ScXMLRowExportPropertyMapper(xRowStylesPropertySetMapper);
    xTableStylesExportPropertySetMapper = new ScXMLTableExportPropertyMapper(xTableStylesPropertySetMapper);

    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_CELL, OUString(XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME),
        xCellStylesExportPropertySetMapper, OUString(XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX));
    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_COLUMN, OUString(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME),
        xColumnStylesExportPropertySetMapper, OUString(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX));
    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_ROW, OUString(XML_STYLE_FAMILY_TABLE_ROW_STYLES_NAME),
        xRowStylesExportPropertySetMapper, OUString(XML_STYLE_FAMILY_TABLE_ROW_STYLES_PREFIX));
    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_TABLE, OUString(XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME),
        xTableStylesExportPropertySetMapper, OUString(XML_STYLE_FAMILY_TABLE_TABLE_STYLES_PREFIX));

    if( (getExportFlags() & (EXPORT_STYLES|EXPORT_AUTOSTYLES|EXPORT_MASTERSTYLES|EXPORT_CONTENT) ) != 0 )
    {
        // This name is reserved for the external ref cache tables.  This
        // should not conflict with user-defined styles since this name is
        // used for a table style which is not available in the UI.
        sExternalRefTabStyleName = OUString("ta_extref");
        GetAutoStylePool()->RegisterName(XML_STYLE_FAMILY_TABLE_TABLE, sExternalRefTabStyleName);

        sAttrName = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TABLE, GetXMLToken(XML_NAME));
        sAttrStyleName = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TABLE, GetXMLToken(XML_STYLE_NAME));
        sAttrColumnsRepeated = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TABLE, GetXMLToken(XML_NUMBER_COLUMNS_REPEATED));
        sAttrFormula = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TABLE, GetXMLToken(XML_FORMULA));
        sAttrStringValue = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_OFFICE, GetXMLToken(XML_STRING_VALUE));
        sAttrValueType = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_OFFICE, GetXMLToken(XML_VALUE_TYPE));
        sElemCell = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TABLE, GetXMLToken(XML_TABLE_CELL));
        sElemCoveredCell = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TABLE, GetXMLToken(XML_COVERED_TABLE_CELL));
        sElemCol = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TABLE, GetXMLToken(XML_TABLE_COLUMN));
        sElemRow = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TABLE, GetXMLToken(XML_TABLE_ROW));
        sElemTab = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TABLE, GetXMLToken(XML_TABLE));
        sElemP = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TEXT, GetXMLToken(XML_P));
    }
}


ScXMLExport::~ScXMLExport()
{
        delete pGroupColumns;
        delete pGroupRows;
        delete pColumnStyles;
        delete pRowStyles;
        delete pCellStyles;
        delete pRowFormatRanges;
        delete pMergedRangesContainer;
        delete pValidationsContainer;
        delete pChangeTrackingExportHelper;
        delete pChartListener;
        delete pCellsItr;
        delete pDefaults;
        delete pNumberFormatAttributesExportHelper;
}

void ScXMLExport::SetSourceStream( const uno::Reference<io::XInputStream>& xNewStream )
{
    xSourceStream = xNewStream;

    if ( xSourceStream.is() )
    {
        // make sure it's a plain UTF-8 stream as written by OOo itself

        const sal_Char pXmlHeader[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
        sal_Int32 nLen = strlen(pXmlHeader);

        uno::Sequence<sal_Int8> aFileStart(nLen);
        sal_Int32 nRead = xSourceStream->readBytes( aFileStart, nLen );

        if ( nRead != nLen || memcmp( aFileStart.getConstArray(), pXmlHeader, nLen ) != 0 )
        {
            // invalid - ignore stream, save normally
            xSourceStream.clear();
        }
        else
        {
            // keep track of the bytes already read
            nSourceStreamPos = nRead;

            const ScSheetSaveData* pSheetData = ScModelObj::getImplementation(GetModel())->GetSheetSaveData();
            if (pSheetData)
            {
                // add the loaded namespaces to the name space map

                if ( !pSheetData->AddLoadedNamespaces( _GetNamespaceMap() ) )
                {
                    // conflicts in the namespaces - ignore the stream, save normally
                    xSourceStream.clear();
                }
            }
        }
    }
}

sal_Int32 ScXMLExport::GetNumberFormatStyleIndex(sal_Int32 nNumFmt) const
{
    NumberFormatIndexMap::const_iterator itr = aNumFmtIndexMap.find(nNumFmt);
    if (itr == aNumFmtIndexMap.end())
        return -1;

    return itr->second;
}

namespace {

/**
 * Update the progress bar state when an instance of this class goes out of
 * scope.
 */
class ProgressBarUpdater
{
    ProgressBarHelper& mrHelper;
    const sal_Int32& mrTableCount;
    const sal_Int32& mrShapesCount;
    const sal_Int32 mnCellCount;
public:
    ProgressBarUpdater(ProgressBarHelper& rHelper,
                       const sal_Int32& rTableCount, const sal_Int32& rShapesCount,
                       const sal_Int32 nCellCount) :
        mrHelper(rHelper),
        mrTableCount(rTableCount),
        mrShapesCount(rShapesCount),
        mnCellCount(nCellCount) {}

    ~ProgressBarUpdater()
    {
        sal_Int32 nRef = mnCellCount + (2 * mrTableCount) + (2 * mrShapesCount);
        mrHelper.SetReference(nRef);
        mrHelper.SetValue(0);
    }
};

}

void ScXMLExport::CollectSharedData(sal_Int32& nTableCount, sal_Int32& nShapesCount)
{
    if (!GetModel().is())
        return;

    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc(GetModel(), uno::UNO_QUERY);
    if (!xSpreadDoc.is())
        return;

    uno::Reference<container::XIndexAccess> xIndex(xSpreadDoc->getSheets(), uno::UNO_QUERY);
    if (!xIndex.is())
        return;

    nTableCount = xIndex->getCount();
    if (!pSharedData)
        CreateSharedData(nTableCount);

    pCellStyles->AddNewTable(nTableCount - 1);

    for (SCTAB nTable = 0; nTable < nTableCount; ++nTable)
    {
        pDoc->GetNotes(nTable)->CreateAllNoteCaptions(nTable);
        nCurrentTable = sal::static_int_cast<sal_uInt16>(nTable);
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xIndex->getByIndex(nTable), uno::UNO_QUERY);
        if (!xDrawPageSupplier.is())
            continue;

        uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage());
        ScMyDrawPage aDrawPage;
        aDrawPage.bHasForms = false;
        aDrawPage.xDrawPage.set(xDrawPage);
        pSharedData->AddDrawPage(aDrawPage, nTable);
        uno::Reference<container::XIndexAccess> xShapesIndex(xDrawPage, uno::UNO_QUERY);
        if (!xShapesIndex.is())
            continue;

        sal_Int32 nShapes = xShapesIndex->getCount();
        for (sal_Int32 nShape = 0; nShape < nShapes; ++nShape)
        {
            uno::Reference<drawing::XShape> xShape(xShapesIndex->getByIndex(nShape), uno::UNO_QUERY);
            if (!xShape.is())
                continue;

            uno::Reference<beans::XPropertySet> xShapeProp(xShape, uno::UNO_QUERY);
            if (!xShapeProp.is())
                continue;

            sal_Int16 nLayerID = 0;
            bool bExtracted = xShapeProp->getPropertyValue(sLayerID) >>= nLayerID;
            if (!bExtracted)
                continue;

            if ((nLayerID == SC_LAYER_INTERN) || (nLayerID == SC_LAYER_HIDDEN))
            {
                CollectInternalShape(xShape);
                continue;
            }

            ++nShapesCount;

            SvxShape* pShapeImp = SvxShape::getImplementation(xShape);
            if (!pShapeImp)
                continue;

            SdrObject* pSdrObj = pShapeImp->GetSdrObject();
            if (!pSdrObj)
                continue;

            if (ScDrawObjData *pAnchor = ScDrawLayer::GetNonRotatedObjData(pSdrObj))
            {
                ScMyShape aMyShape;
                aMyShape.aAddress = pAnchor->maStart;
                SAL_WARN_IF(aMyShape.aAddress.Tab() != nTable, "sc", "not anchored to current sheet!");
                aMyShape.aAddress.SetTab(nTable);
                aMyShape.aEndAddress = pAnchor->maEnd;
                aMyShape.aEndAddress.SetTab( nTable );
                aMyShape.nEndX = pAnchor->maEndOffset.X();
                aMyShape.nEndY = pAnchor->maEndOffset.Y();
                aMyShape.xShape = xShape;
                pSharedData->AddNewShape(aMyShape);
                pSharedData->SetLastColumn(nTable, pAnchor->maStart.Col());
                pSharedData->SetLastRow(nTable, pAnchor->maStart.Row());
            }
            else
                pSharedData->AddTableShape(nTable, xShape);
        }
    }
}

void ScXMLExport::CollectShapesAutoStyles(const sal_Int32 nTableCount)
{
    // #i84077# To avoid compiler warnings about uninitialized aShapeItr,
    // it's initialized using this dummy list. The iterator contains shapes
    // from all sheets, so it can't be declared inside the nTable loop where
    // it is used.
    ScMyShapeList aDummyInitList;

    pSharedData->SortShapesContainer();
    pSharedData->SortNoteShapes();
    const ScMyShapeList* pShapeList(NULL);
    ScMyShapeList::const_iterator aShapeItr = aDummyInitList.end();
    if (pSharedData->GetShapesContainer())
    {
        pShapeList = pSharedData->GetShapesContainer()->GetShapes();
        aShapeItr = pShapeList->begin();
    }
    if (pSharedData->HasDrawPage())
    {
        for (SCTAB nTable = 0; nTable < nTableCount; ++nTable)
        {
            uno::Reference<drawing::XDrawPage> xDrawPage(pSharedData->GetDrawPage(nTable));
            uno::Reference<drawing::XShapes> xShapes (xDrawPage, uno::UNO_QUERY);

            if (xShapes.is())
            {
                GetShapeExport()->seekShapes(xShapes);
                uno::Reference< form::XFormsSupplier2 > xFormsSupplier( xDrawPage, uno::UNO_QUERY );
                if( xFormsSupplier.is() && xFormsSupplier->hasForms() )
                {
                    GetFormExport()->examineForms(xDrawPage);
                    pSharedData->SetDrawPageHasForms(nTable, true);
                }
                ScMyTableShapes* pTableShapes(pSharedData->GetTableShapes());
                if (pTableShapes)
                {
                    ScMyTableXShapes::iterator aItr((*pTableShapes)[nTable].begin());
                    ScMyTableXShapes::iterator aEndItr((*pTableShapes)[nTable].end());
                    while (aItr != aEndItr)
                    {
                        GetShapeExport()->collectShapeAutoStyles(*aItr);
                        IncrementProgressBar(false);
                        ++aItr;
                    }
                }
                if (pShapeList)
                {
                    ScMyShapeList::const_iterator aEndItr(pShapeList->end());
                    while (aShapeItr != aEndItr && (static_cast<sal_Int32>(aShapeItr->aAddress.Tab()) == nTable))
                    {
                        GetShapeExport()->collectShapeAutoStyles(aShapeItr->xShape);
                        IncrementProgressBar(false);
                        ++aShapeItr;
                    }
                }
                if (pSharedData->GetNoteShapes())
                {
                    const ScMyNoteShapeList* pNoteShapes = pSharedData->GetNoteShapes()->GetNotes();
                    if (pNoteShapes)
                    {
                        for (ScMyNoteShapeList::const_iterator aNoteShapeItr = pNoteShapes->begin(), aNoteShapeEndItr = pNoteShapes->end();
                             aNoteShapeItr != aNoteShapeEndItr; ++aNoteShapeItr)
                        {
                            if (static_cast<sal_Int32>(aNoteShapeItr->aPos.Tab()) == nTable)
                                GetShapeExport()->collectShapeAutoStyles(aNoteShapeItr->xShape);
                        }
                    }
                }
            }
        }
    }
    pSharedData->SortNoteShapes(); // sort twice, because some more shapes are added
}

void ScXMLExport::_ExportMeta()
{
    sal_Int32 nCellCount(pDoc ? pDoc->GetCellCount() : 0);
    sal_Int32 nTableCount(0);
    sal_Int32 nShapesCount(0);
    GetAutoStylePool()->ClearEntries();
    CollectSharedData(nTableCount, nShapesCount);

    uno::Sequence<beans::NamedValue> stats(3);
    stats[0] = beans::NamedValue(OUString("TableCount"),
                uno::makeAny(nTableCount));
    stats[1] = beans::NamedValue(OUString("CellCount"),
                uno::makeAny(nCellCount));
    stats[2] = beans::NamedValue(OUString("ObjectCount"),
                uno::makeAny(nShapesCount));

    // update document statistics at the model
    uno::Reference<document::XDocumentPropertiesSupplier> xPropSup(GetModel(),
        uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps(
        xPropSup->getDocumentProperties());
    if (xDocProps.is()) {
        xDocProps->setDocumentStatistics(stats);
    }

    // export document properties
    SvXMLExport::_ExportMeta();
}

void ScXMLExport::_ExportFontDecls()
{
    GetFontAutoStylePool(); // make sure the pool is created
    SvXMLExport::_ExportFontDecls();
}

table::CellRangeAddress ScXMLExport::GetEndAddress(const uno::Reference<sheet::XSpreadsheet>& xTable, const sal_Int32 /* nTable */)
{
    table::CellRangeAddress aCellAddress;
    uno::Reference<sheet::XSheetCellCursor> xCursor(xTable->createCursor());
    uno::Reference<sheet::XUsedAreaCursor> xUsedArea (xCursor, uno::UNO_QUERY);
    uno::Reference<sheet::XCellRangeAddressable> xCellAddress (xCursor, uno::UNO_QUERY);
    if (xUsedArea.is() && xCellAddress.is())
    {
        xUsedArea->gotoEndOfUsedArea(true);
        aCellAddress = xCellAddress->getRangeAddress();
    }
    return aCellAddress;
}

void ScXMLExport::GetAreaLinks( uno::Reference< sheet::XSpreadsheetDocument>& xSpreadDoc,
                                ScMyAreaLinksContainer& rAreaLinks )
{
    uno::Reference< beans::XPropertySet > xPropSet( xSpreadDoc, uno::UNO_QUERY );
    if( !xPropSet.is() ) return;

    uno::Reference< container::XIndexAccess > xLinksIAccess( xPropSet->getPropertyValue( OUString( SC_UNO_AREALINKS ) ), uno::UNO_QUERY);
    if( xLinksIAccess.is() )
    {
        const OUString sFilter( SC_UNONAME_FILTER );
        const OUString sFilterOpt( SC_UNONAME_FILTOPT );
        const OUString sURL( SC_UNONAME_LINKURL );
        const OUString sRefresh( SC_UNONAME_REFDELAY );

        sal_Int32 nCount(xLinksIAccess->getCount());
        for( sal_Int32 nIndex = 0; nIndex < nCount; ++nIndex )
        {
            uno::Reference< sheet::XAreaLink > xAreaLink(xLinksIAccess->getByIndex( nIndex ), uno::UNO_QUERY);
            if( xAreaLink.is() )
            {
                ScMyAreaLink aAreaLink;
                aAreaLink.aDestRange = xAreaLink->getDestArea();
                aAreaLink.sSourceStr = xAreaLink->getSourceArea();
                uno::Reference< beans::XPropertySet > xLinkProp( xAreaLink, uno::UNO_QUERY );
                if( xLinkProp.is() )
                {
                    xLinkProp->getPropertyValue( sFilter ) >>= aAreaLink.sFilter;
                    xLinkProp->getPropertyValue( sFilterOpt ) >>= aAreaLink.sFilterOptions;
                    xLinkProp->getPropertyValue( sURL ) >>= aAreaLink.sURL;
                    xLinkProp->getPropertyValue( sRefresh ) >>= aAreaLink.nRefresh;
                }
                rAreaLinks.AddNewAreaLink( aAreaLink );
            }
        }
    }
    rAreaLinks.Sort();
}

// core implementation
void ScXMLExport::GetDetectiveOpList( ScMyDetectiveOpContainer& rDetOp )
{
    if (pDoc)
    {
        ScDetOpList* pOpList(pDoc->GetDetOpList());
        if( pOpList )
        {
            size_t nCount = pOpList->Count();
            for (size_t nIndex = 0; nIndex < nCount; ++nIndex )
            {
                const ScDetOpData* pDetData = pOpList->GetObject( nIndex);
                if( pDetData )
                {
                    const ScAddress& rDetPos = pDetData->GetPos();
                    SCTAB nTab = rDetPos.Tab();
                    if ( nTab < pDoc->GetTableCount() )
                    {
                        rDetOp.AddOperation( pDetData->GetOperation(), rDetPos, static_cast<sal_uInt32>( nIndex) );

                        // cells with detective operations are written even if empty
                        pSharedData->SetLastColumn( nTab, rDetPos.Col() );
                        pSharedData->SetLastRow( nTab, rDetPos.Row() );
                    }
                }
            }
            rDetOp.Sort();
        }
    }
}

void ScXMLExport::WriteSingleColumn(const sal_Int32 nRepeatColumns, const sal_Int32 nStyleIndex,
    const sal_Int32 nIndex, const bool bIsAutoStyle, const bool bIsVisible)
{
    CheckAttrList();
    AddAttribute(sAttrStyleName, *pColumnStyles->GetStyleNameByIndex(nStyleIndex));
    if (!bIsVisible)
        AddAttribute(XML_NAMESPACE_TABLE, XML_VISIBILITY, XML_COLLAPSE);
    if (nRepeatColumns > 1)
    {
        OUString sOUEndCol(OUString::number(nRepeatColumns));
        AddAttribute(sAttrColumnsRepeated, sOUEndCol);
    }
    if (nIndex != -1)
        AddAttribute(XML_NAMESPACE_TABLE, XML_DEFAULT_CELL_STYLE_NAME, *pCellStyles->GetStyleNameByIndex(nIndex, bIsAutoStyle));
    SvXMLElementExport aElemC(*this, sElemCol, true, true);
}

void ScXMLExport::WriteColumn(const sal_Int32 nColumn, const sal_Int32 nRepeatColumns,
    const sal_Int32 nStyleIndex, const bool bIsVisible)
{
    sal_Int32 nRepeat(1);
    sal_Int32 nPrevIndex((*pDefaults->GetColDefaults())[nColumn].nIndex);
    bool bPrevAutoStyle((*pDefaults->GetColDefaults())[nColumn].bIsAutoStyle);
    for (sal_Int32 i = nColumn + 1; i < nColumn + nRepeatColumns; ++i)
    {
        if (((*pDefaults->GetColDefaults())[i].nIndex != nPrevIndex) ||
            ((*pDefaults->GetColDefaults())[i].bIsAutoStyle != bPrevAutoStyle))
        {
            WriteSingleColumn(nRepeat, nStyleIndex, nPrevIndex, bPrevAutoStyle, bIsVisible);
            nPrevIndex = (*pDefaults->GetColDefaults())[i].nIndex;
            bPrevAutoStyle = (*pDefaults->GetColDefaults())[i].bIsAutoStyle;
            nRepeat = 1;
        }
        else
            ++nRepeat;
    }
    WriteSingleColumn(nRepeat, nStyleIndex, nPrevIndex, bPrevAutoStyle, bIsVisible);
}

void ScXMLExport::OpenHeaderColumn()
{
    StartElement( XML_NAMESPACE_TABLE, XML_TABLE_HEADER_COLUMNS, true );
}

void ScXMLExport::CloseHeaderColumn()
{
    EndElement(XML_NAMESPACE_TABLE, XML_TABLE_HEADER_COLUMNS, true);
}

void ScXMLExport::ExportColumns(const sal_Int32 nTable, const table::CellRangeAddress& aColumnHeaderRange, const bool bHasColumnHeader)
{
    sal_Int32 nColsRepeated (1);
    sal_Int32 nIndex;
    sal_Int32 nPrevColumn(0);
    bool bPrevIsVisible (true);
    bool bWasHeader (false);
    bool bIsHeader (false);
    bool bIsClosed (true);
    sal_Int32 nPrevIndex (-1);
    sal_Int32 nColumn;
    for (nColumn = 0; nColumn <= pSharedData->GetLastColumn(nTable); ++nColumn)
    {
        CheckAttrList();
        bool bIsVisible(true);
        nIndex = pColumnStyles->GetStyleNameIndex(nTable, nColumn, bIsVisible);

        bIsHeader = bHasColumnHeader && (aColumnHeaderRange.StartColumn <= nColumn) && (nColumn <= aColumnHeaderRange.EndColumn);
        if (bIsHeader != bWasHeader)
        {
            if (bIsHeader)
            {
                if (nColumn > 0)
                {
                    WriteColumn(nPrevColumn, nColsRepeated, nPrevIndex, bPrevIsVisible);
                    if (pGroupColumns->IsGroupEnd(nColumn - 1))
                        pGroupColumns->CloseGroups(nColumn - 1);
                }
                bPrevIsVisible = bIsVisible;
                nPrevIndex = nIndex;
                nPrevColumn = nColumn;
                nColsRepeated = 1;
                if(pGroupColumns->IsGroupStart(nColumn))
                    pGroupColumns->OpenGroups(nColumn);
                OpenHeaderColumn();
                bWasHeader = true;
                bIsClosed = false;
            }
            else
            {
                WriteColumn(nPrevColumn, nColsRepeated, nPrevIndex, bPrevIsVisible);
                CloseHeaderColumn();
                if (pGroupColumns->IsGroupEnd(nColumn - 1))
                    pGroupColumns->CloseGroups(nColumn - 1);
                if(pGroupColumns->IsGroupStart(nColumn))
                    pGroupColumns->OpenGroups(nColumn);
                bPrevIsVisible = bIsVisible;
                nPrevIndex = nIndex;
                nPrevColumn = nColumn;
                nColsRepeated = 1;
                bWasHeader = false;
                bIsClosed = true;
            }
        }
        else if (nColumn == 0)
        {
            if (pGroupColumns->IsGroupStart(nColumn))
                pGroupColumns->OpenGroups(nColumn);
            bPrevIsVisible = bIsVisible;
            nPrevIndex = nIndex;
        }
        else if ((bIsVisible == bPrevIsVisible) && (nIndex == nPrevIndex) &&
            !pGroupColumns->IsGroupStart(nColumn) && !pGroupColumns->IsGroupEnd(nColumn - 1))
            ++nColsRepeated;
        else
        {
            WriteColumn(nPrevColumn, nColsRepeated, nPrevIndex, bPrevIsVisible);
            if (pGroupColumns->IsGroupEnd(nColumn - 1))
            {
                if (bIsHeader)
                    CloseHeaderColumn();
                pGroupColumns->CloseGroups(nColumn - 1);
                if (bIsHeader)
                    OpenHeaderColumn();
            }
            if (pGroupColumns->IsGroupStart(nColumn))
            {
                if (bIsHeader)
                    CloseHeaderColumn();
                pGroupColumns->OpenGroups(nColumn);
                if (bIsHeader)
                    OpenHeaderColumn();
            }
            bPrevIsVisible = bIsVisible;
            nPrevIndex = nIndex;
            nPrevColumn = nColumn;
            nColsRepeated = 1;
        }
    }
    WriteColumn(nPrevColumn, nColsRepeated, nPrevIndex, bPrevIsVisible);
    if (!bIsClosed)
        CloseHeaderColumn();
    if (pGroupColumns->IsGroupEnd(nColumn - 1))
        pGroupColumns->CloseGroups(nColumn - 1);
}

void ScXMLExport::ExportExternalRefCacheStyles()
{
    sal_Int32 nEntryIndex = GetCellStylesPropertySetMapper()->FindEntryIndex(
        "NumberFormat", XML_NAMESPACE_STYLE, OUString("data-style-name"));

    if (nEntryIndex < 0)
        // No entry index for the number format is found.
        return;

    ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
    if (!pRefMgr->hasExternalData())
        // No external reference data cached.
        return;

    // Export each unique number format used in the external ref cache.
    vector<sal_uInt32> aNumFmts;
    pRefMgr->getAllCachedNumberFormats(aNumFmts);
    const OUString aDefaultStyle = OUString("Default").intern();
    for (vector<sal_uInt32>::const_iterator itr = aNumFmts.begin(), itrEnd = aNumFmts.end();
          itr != itrEnd; ++itr)
    {
        sal_Int32 nNumFmt = static_cast<sal_Int32>(*itr);

        addDataStyle(nNumFmt);

        uno::Any aVal;
        aVal <<= nNumFmt;
        vector<XMLPropertyState> aProps;
        aProps.push_back(XMLPropertyState(nEntryIndex, aVal));
        aVal <<= aDefaultStyle;
        aProps.push_back(XMLPropertyState(nEntryIndex, aVal));

        OUString aName;
        sal_Int32 nIndex;
        if (GetAutoStylePool()->Add(aName, XML_STYLE_FAMILY_TABLE_CELL, aDefaultStyle, aProps))
        {
            OUString* pTemp(new OUString(aName));
            if (!pCellStyles->AddStyleName(pTemp, nIndex, true))
                delete pTemp;
        }
        else
        {
            bool bIsAuto;
            nIndex = pCellStyles->GetIndexOfStyleName(
                aName, OUString(XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX), bIsAuto);
        }

        // store the number format to index mapping for later use.
        aNumFmtIndexMap.insert(NumberFormatIndexMap::value_type(nNumFmt, nIndex));
    }
}

void ScXMLExport::ExportCellTextAutoStyles(sal_Int32 nTable)
{
    if (!ValidTab(nTable))
        return;

    UniReference<XMLPropertySetMapper> xMapper = GetTextParagraphExport()->GetTextPropMapper()->getPropertySetMapper();
    sal_Int32 nEntryCount = xMapper->GetEntryCount();
    UniReference<SvXMLAutoStylePoolP> xStylePool = GetAutoStylePool();
    const ScXMLEditAttributeMap& rAttrMap = GetEditAttributeMap();

    sc::EditTextIterator aIter(*pDoc, nTable);
    sal_Int32 nCellCount = 0;
    for (const EditTextObject* pEdit = aIter.first(); pEdit; pEdit = aIter.next(), ++nCellCount)
    {
        std::vector<editeng::SectionAttribute> aAttrs;
        pEdit->GetAllSectionAttributes(aAttrs);
        if (aAttrs.empty())
            continue;

        std::vector<editeng::SectionAttribute>::const_iterator itSec = aAttrs.begin(), itSecEnd = aAttrs.end();
        for (; itSec != itSecEnd; ++itSec)
        {
            const std::vector<const SfxPoolItem*>& rSecAttrs = itSec->maAttributes;
            if (rSecAttrs.empty())
                // No formats applied to this section. Skip it.
                continue;

            std::vector<XMLPropertyState> aPropStates;
            aPropStates.reserve(rSecAttrs.size());
            std::vector<const SfxPoolItem*>::const_iterator it = rSecAttrs.begin(), itEnd = rSecAttrs.end();
            for (; it != itEnd; ++it)
            {
                const SfxPoolItem* p = *it;
                const ScXMLEditAttributeMap::Entry* pEntry = rAttrMap.getEntryByItemID(p->Which());
                if (!pEntry)
                    continue;

                sal_Int32 nIndex = xMapper->GetEntryIndex(
                    pEntry->nmXMLNS, OUString::createFromAscii(pEntry->mpXMLName), 0);

                if (nIndex == -1 || nIndex >= nEntryCount)
                    continue;

                uno::Any aAny;
                switch (p->Which())
                {
                    case EE_CHAR_FONTINFO:
                    case EE_CHAR_FONTINFO_CJK:
                    case EE_CHAR_FONTINFO_CTL:
                    {
                        if (!static_cast<const SvxFontItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                            continue;

                        aPropStates.push_back(XMLPropertyState(nIndex, aAny));
                    }
                    break;
                    case EE_CHAR_WEIGHT:
                    case EE_CHAR_WEIGHT_CJK:
                    case EE_CHAR_WEIGHT_CTL:
                    {
                        if (!static_cast<const SvxWeightItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                            continue;

                        aPropStates.push_back(XMLPropertyState(nIndex, aAny));
                    }
                    break;
                    case EE_CHAR_FONTHEIGHT:
                    case EE_CHAR_FONTHEIGHT_CJK:
                    case EE_CHAR_FONTHEIGHT_CTL:
                    {
                        if (!static_cast<const SvxFontHeightItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                            continue;

                        aPropStates.push_back(XMLPropertyState(nIndex, aAny));
                    }
                    break;
                    case EE_CHAR_ITALIC:
                    case EE_CHAR_ITALIC_CJK:
                    case EE_CHAR_ITALIC_CTL:
                    {
                        if (!static_cast<const SvxPostureItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                            continue;

                        aPropStates.push_back(XMLPropertyState(nIndex, aAny));
                    }
                    break;
                    case EE_CHAR_OVERLINE:
                    {
                        if (!static_cast<const SvxOverlineItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                            continue;

                        aPropStates.push_back(XMLPropertyState(nIndex, aAny));
                    }
                    break;
                    case EE_CHAR_COLOR:
                    {
                        if (!static_cast<const SvxColorItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                            continue;

                        aPropStates.push_back(XMLPropertyState(nIndex, aAny));
                    }
                    break;
                    case EE_CHAR_WLM:
                    {
                        if (!static_cast<const SvxWordLineModeItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                            continue;

                        aPropStates.push_back(XMLPropertyState(nIndex, aAny));
                    }
                    break;
                    case EE_CHAR_STRIKEOUT:
                    {
                        if (!static_cast<const SvxCrossedOutItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                            continue;

                        aPropStates.push_back(XMLPropertyState(nIndex, aAny));
                    }
                    break;
                    case EE_CHAR_RELIEF:
                    {
                        if (!static_cast<const SvxCharReliefItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                            continue;

                        aPropStates.push_back(XMLPropertyState(nIndex, aAny));
                    }
                    break;
                    case EE_CHAR_OUTLINE:
                    {
                        if (!static_cast<const SvxContourItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                            continue;

                        aPropStates.push_back(XMLPropertyState(nIndex, aAny));
                    }
                    break;
                    case EE_CHAR_SHADOW:
                    {
                        if (!static_cast<const SvxShadowedItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                            continue;

                        aPropStates.push_back(XMLPropertyState(nIndex, aAny));
                    }
                    break;
                    case EE_CHAR_KERNING:
                    {
                        if (!static_cast<const SvxKerningItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                            continue;

                        aPropStates.push_back(XMLPropertyState(nIndex, aAny));
                    }
                    break;
                    case EE_CHAR_PAIRKERNING:
                    {
                        if (!static_cast<const SvxAutoKernItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                            continue;

                        aPropStates.push_back(XMLPropertyState(nIndex, aAny));
                    }
                    break;
                    case EE_CHAR_FONTWIDTH:
                    {
                        if (!static_cast<const SvxCharScaleWidthItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                            continue;

                        aPropStates.push_back(XMLPropertyState(nIndex, aAny));
                    }
                    break;
                    case EE_CHAR_ESCAPEMENT:
                    {
                        if (!static_cast<const SvxEscapementItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                            continue;

                        aPropStates.push_back(XMLPropertyState(nIndex, aAny));
                    }
                    break;
                    case EE_CHAR_EMPHASISMARK:
                    {
                        if (!static_cast<const SvxEmphasisMarkItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                            continue;

                        aPropStates.push_back(XMLPropertyState(nIndex, aAny));
                    }
                    break;
                    case EE_CHAR_LANGUAGE:
                    case EE_CHAR_LANGUAGE_CJK:
                    case EE_CHAR_LANGUAGE_CTL:
                    {
                        if (!static_cast<const SvxLanguageItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                            continue;

                        aPropStates.push_back(XMLPropertyState(nIndex, aAny));
                    }
                    break;
                    default:
                        continue;
                }
            }

            if (!aPropStates.empty())
                OUString aName = xStylePool->Add(XML_STYLE_FAMILY_TEXT_TEXT, OUString(), aPropStates, false);
        }
    }

    GetProgressBarHelper()->ChangeReference(GetProgressBarHelper()->GetReference() + nCellCount);
}

void ScXMLExport::WriteRowContent()
{
    ScMyRowFormatRange aRange;
    sal_Int32 nIndex(-1);
#if OSL_DEBUG_LEVEL > 0
    sal_Int32 nPrevCol(0);
#endif
    sal_Int32 nCols(0);
    sal_Int32 nPrevValidationIndex(-1);
    bool bIsAutoStyle(true);
    bool bIsFirst(true);
    while (pRowFormatRanges->GetNext(aRange))
    {
#if OSL_DEBUG_LEVEL > 0
        OSL_ENSURE(bIsFirst || (!bIsFirst && (nPrevCol + nCols == aRange.nStartColumn)), "here are some columns missing");
#endif
        if (bIsFirst)
        {
            nIndex = aRange.nIndex;
            nPrevValidationIndex = aRange.nValidationIndex;
            bIsAutoStyle = aRange.bIsAutoStyle;
            nCols = aRange.nRepeatColumns;
            bIsFirst = false;
#if OSL_DEBUG_LEVEL > 0
            nPrevCol = aRange.nStartColumn;
#endif
        }
        else
        {
            if (((aRange.nIndex == nIndex && aRange.bIsAutoStyle == bIsAutoStyle) ||
                (aRange.nIndex == nIndex && nIndex == -1)) &&
                nPrevValidationIndex == aRange.nValidationIndex)
                nCols += aRange.nRepeatColumns;
            else
            {
                if (nIndex != -1)
                    AddAttribute(sAttrStyleName, *pCellStyles->GetStyleNameByIndex(nIndex, bIsAutoStyle));
                if (nPrevValidationIndex > -1)
                    AddAttribute(XML_NAMESPACE_TABLE, XML_CONTENT_VALIDATION_NAME, pValidationsContainer->GetValidationName(nPrevValidationIndex));
                if (nCols > 1)
                {
                    OUStringBuffer aBuf;
                    ::sax::Converter::convertNumber(aBuf, nCols);
                    AddAttribute(sAttrColumnsRepeated, aBuf.makeStringAndClear());
                }
                SvXMLElementExport aElemC(*this, sElemCell, true, true);
                nIndex = aRange.nIndex;
                bIsAutoStyle = aRange.bIsAutoStyle;
                nCols = aRange.nRepeatColumns;
                nPrevValidationIndex = aRange.nValidationIndex;
#if OSL_DEBUG_LEVEL > 0
                nPrevCol = aRange.nStartColumn;
#endif
            }
        }
    }
    if (!bIsFirst)
    {
        table::CellAddress aCellAddress;
        if (nIndex != -1)
            AddAttribute(sAttrStyleName, *pCellStyles->GetStyleNameByIndex(nIndex, bIsAutoStyle));
        if (nPrevValidationIndex > -1)
            AddAttribute(XML_NAMESPACE_TABLE, XML_CONTENT_VALIDATION_NAME, pValidationsContainer->GetValidationName(nPrevValidationIndex));
        if (nCols > 1)
        {
            OUStringBuffer aBuf;
            ::sax::Converter::convertNumber(aBuf, nCols);
            AddAttribute(sAttrColumnsRepeated, aBuf.makeStringAndClear());
        }
        SvXMLElementExport aElemC(*this, sElemCell, true, true);
    }
}

void ScXMLExport::WriteRowStartTag(
    sal_Int32 nRow, const sal_Int32 nIndex, const sal_Int32 nEqualRows,
    bool bHidden, bool bFiltered)
{
    AddAttribute(sAttrStyleName, *pRowStyles->GetStyleNameByIndex(nIndex));
    if (bHidden)
    {
        if (bFiltered)
            AddAttribute(XML_NAMESPACE_TABLE, XML_VISIBILITY, XML_FILTER);
        else
            AddAttribute(XML_NAMESPACE_TABLE, XML_VISIBILITY, XML_COLLAPSE);
    }
    if (nEqualRows > 1)
    {
        OUStringBuffer aBuf;
        ::sax::Converter::convertNumber(aBuf, nEqualRows);
        AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_ROWS_REPEATED, aBuf.makeStringAndClear());
    }

    const ScMyDefaultStyleList& rRowDefaults = *pDefaults->GetRowDefaults();
    if ( nRow >= sal::static_int_cast<sal_Int32>( rRowDefaults.size() ) )
    {
        // used to happen with detective operations - if there are more cases, use the last row's style
        OSL_FAIL("WriteRowStartTag: not enough defaults");
        nRow = rRowDefaults.size() - 1;
    }
    sal_Int32 nCellStyleIndex(rRowDefaults[nRow].nIndex);
    if (nCellStyleIndex != -1)
        AddAttribute(XML_NAMESPACE_TABLE, XML_DEFAULT_CELL_STYLE_NAME,
            *pCellStyles->GetStyleNameByIndex(nCellStyleIndex,
                (*pDefaults->GetRowDefaults())[nRow].bIsAutoStyle));
    StartElement( sElemRow, true);
}

void ScXMLExport::OpenHeaderRows()
{
    StartElement( XML_NAMESPACE_TABLE, XML_TABLE_HEADER_ROWS, true);
    bRowHeaderOpen = true;
}

void ScXMLExport::CloseHeaderRows()
{
    EndElement(XML_NAMESPACE_TABLE, XML_TABLE_HEADER_ROWS, true);
}

void ScXMLExport::OpenNewRow(
    const sal_Int32 nIndex, const sal_Int32 nStartRow, const sal_Int32 nEqualRows,
    bool bHidden, bool bFiltered)
{
    nOpenRow = nStartRow;
    if (pGroupRows->IsGroupStart(nStartRow))
    {
        if (bHasRowHeader && bRowHeaderOpen)
            CloseHeaderRows();
        pGroupRows->OpenGroups(nStartRow);
        if (bHasRowHeader && bRowHeaderOpen)
            OpenHeaderRows();
    }
    if (bHasRowHeader && !bRowHeaderOpen && nStartRow >= aRowHeaderRange.StartRow && nStartRow <= aRowHeaderRange.EndRow)
    {
        if (nStartRow == aRowHeaderRange.StartRow)
            OpenHeaderRows();
        sal_Int32 nEquals;
        if (aRowHeaderRange.EndRow < nStartRow + nEqualRows - 1)
            nEquals = aRowHeaderRange.EndRow - nStartRow + 1;
        else
            nEquals = nEqualRows;
        WriteRowStartTag(nStartRow, nIndex, nEquals, bHidden, bFiltered);
        nOpenRow = nStartRow + nEquals - 1;
        if (nEquals < nEqualRows)
        {
            CloseRow(nStartRow + nEquals - 1);
            WriteRowStartTag(nStartRow, nIndex, nEqualRows - nEquals, bHidden, bFiltered);
            nOpenRow = nStartRow + nEqualRows - 1;
        }
    }
    else
        WriteRowStartTag(nStartRow, nIndex, nEqualRows, bHidden, bFiltered);
}

void ScXMLExport::OpenAndCloseRow(
    const sal_Int32 nIndex, const sal_Int32 nStartRow, const sal_Int32 nEqualRows,
    bool bHidden, bool bFiltered)
{
    OpenNewRow(nIndex, nStartRow, nEqualRows, bHidden, bFiltered);
    WriteRowContent();
    CloseRow(nStartRow + nEqualRows - 1);
    pRowFormatRanges->Clear();
}

void ScXMLExport::OpenRow(const sal_Int32 nTable, const sal_Int32 nStartRow, const sal_Int32 nRepeatRow, ScXMLCachedRowAttrAccess& rRowAttr)
{
    if (nRepeatRow > 1)
    {
        sal_Int32 nPrevIndex(0), nIndex;
        bool bPrevHidden = false;
        bool bPrevFiltered = false;
        bool bHidden = false;
        bool bFiltered = false;
        sal_Int32 nEqualRows(1);
        sal_Int32 nEndRow(nStartRow + nRepeatRow);
        sal_Int32 nRow;
        for (nRow = nStartRow; nRow < nEndRow; ++nRow)
        {
            if (nRow == nStartRow)
            {
                nPrevIndex = pRowStyles->GetStyleNameIndex(nTable, nRow);
                if (pDoc)
                {
                    bPrevHidden = rRowAttr.rowHidden(nTable, nRow);
                    bPrevFiltered = rRowAttr.rowFiltered(nTable, nRow);
                }
            }
            else
            {
                nIndex = pRowStyles->GetStyleNameIndex(nTable, nRow);
                if (pDoc)
                {
                    bHidden = rRowAttr.rowHidden(nTable, nRow);
                    bFiltered = rRowAttr.rowFiltered(nTable, nRow);
                }
                if (nIndex == nPrevIndex && bHidden == bPrevHidden && bFiltered == bPrevFiltered &&
                    !(bHasRowHeader && ((nRow == aRowHeaderRange.StartRow) || (nRow - 1 == aRowHeaderRange.EndRow))) &&
                    !(pGroupRows->IsGroupStart(nRow)) &&
                    !(pGroupRows->IsGroupEnd(nRow - 1)))
                    ++nEqualRows;
                else
                {
                    if (nRow < nEndRow)
                    {
                        ScRowFormatRanges* pTempRowFormatRanges = new ScRowFormatRanges(pRowFormatRanges);
                        OpenAndCloseRow(nPrevIndex, nRow - nEqualRows, nEqualRows, bPrevHidden, bPrevFiltered);
                        delete pRowFormatRanges;
                        pRowFormatRanges = pTempRowFormatRanges;
                    }
                    else
                        OpenAndCloseRow(nPrevIndex, nRow - nEqualRows, nEqualRows, bPrevHidden, bPrevFiltered);
                    nEqualRows = 1;
                    nPrevIndex = nIndex;
                    bPrevHidden = bHidden;
                    bPrevFiltered = bFiltered;
                }
            }
        }
        OpenNewRow(nPrevIndex, nRow - nEqualRows, nEqualRows, bPrevHidden, bPrevFiltered);
    }
    else
    {
        sal_Int32 nIndex = pRowStyles->GetStyleNameIndex(nTable, nStartRow);
        bool bHidden = false;
        bool bFiltered = false;
        if (pDoc)
        {
            bHidden = rRowAttr.rowHidden(nTable, nStartRow);
            bFiltered = rRowAttr.rowFiltered(nTable, nStartRow);
        }
        OpenNewRow(nIndex, nStartRow, 1, bHidden, bFiltered);
    }
    nOpenRow = nStartRow + nRepeatRow - 1;
}

void ScXMLExport::CloseRow(const sal_Int32 nRow)
{
    if (nOpenRow > -1)
    {
        EndElement(sElemRow, true);
        if (bHasRowHeader && nRow == aRowHeaderRange.EndRow)
        {
            CloseHeaderRows();
            bRowHeaderOpen = false;
        }
        if (pGroupRows->IsGroupEnd(nRow))
        {
            if (bHasRowHeader && bRowHeaderOpen)
                CloseHeaderRows();
            pGroupRows->CloseGroups(nRow);
            if (bHasRowHeader && bRowHeaderOpen)
                OpenHeaderRows();
        }
    }
    nOpenRow = -1;
}

void ScXMLExport::ExportFormatRanges(const sal_Int32 nStartCol, const sal_Int32 nStartRow,
    const sal_Int32 nEndCol, const sal_Int32 nEndRow, const sal_Int32 nSheet)
{
    pRowFormatRanges->Clear();
    ScXMLCachedRowAttrAccess aRowAttr(pDoc);
    if (nStartRow == nEndRow)
    {
        pCellStyles->GetFormatRanges(nStartCol, nEndCol, nStartRow, nSheet, pRowFormatRanges);
        if (nOpenRow == - 1)
            OpenRow(nSheet, nStartRow, 1, aRowAttr);
        WriteRowContent();
        pRowFormatRanges->Clear();
    }
    else
    {
        if (nOpenRow > -1)
        {
            pCellStyles->GetFormatRanges(nStartCol, pSharedData->GetLastColumn(nSheet), nStartRow, nSheet, pRowFormatRanges);
            WriteRowContent();
            CloseRow(nStartRow);
            sal_Int32 nRows(1);
            sal_Int32 nTotalRows(nEndRow - nStartRow + 1 - 1);
            while (nRows < nTotalRows)
            {
                pRowFormatRanges->Clear();
                pCellStyles->GetFormatRanges(0, pSharedData->GetLastColumn(nSheet), nStartRow + nRows, nSheet, pRowFormatRanges);
                sal_Int32 nMaxRows = pRowFormatRanges->GetMaxRows();
                OSL_ENSURE(nMaxRows, "something wents wrong");
                if (nMaxRows >= nTotalRows - nRows)
                {
                    OpenRow(nSheet, nStartRow + nRows, nTotalRows - nRows, aRowAttr);
                    nRows += nTotalRows - nRows;
                }
                else
                {
                    OpenRow(nSheet, nStartRow + nRows, nMaxRows, aRowAttr);
                    nRows += nMaxRows;
                }
                if (!pRowFormatRanges->GetSize())
                    pCellStyles->GetFormatRanges(0, pSharedData->GetLastColumn(nSheet), nStartRow + nRows, nSheet, pRowFormatRanges);
                WriteRowContent();
                CloseRow(nStartRow + nRows - 1);
            }
            if (nTotalRows == 1)
                CloseRow(nStartRow);
            OpenRow(nSheet, nEndRow, 1, aRowAttr);
            pRowFormatRanges->Clear();
            pCellStyles->GetFormatRanges(0, nEndCol, nEndRow, nSheet, pRowFormatRanges);
            WriteRowContent();
        }
        else
        {
            sal_Int32 nRows(0);
            sal_Int32 nTotalRows(nEndRow - nStartRow + 1 - 1);
            while (nRows < nTotalRows)
            {
                pCellStyles->GetFormatRanges(0, pSharedData->GetLastColumn(nSheet), nStartRow + nRows, nSheet, pRowFormatRanges);
                sal_Int32 nMaxRows = pRowFormatRanges->GetMaxRows();
                if (nMaxRows >= nTotalRows - nRows)
                {
                    OpenRow(nSheet, nStartRow + nRows, nTotalRows - nRows, aRowAttr);
                    nRows += nTotalRows - nRows;
                }
                else
                {
                    OpenRow(nSheet, nStartRow + nRows, nMaxRows, aRowAttr);
                    nRows += nMaxRows;
                }
                if (!pRowFormatRanges->GetSize())
                    pCellStyles->GetFormatRanges(0, pSharedData->GetLastColumn(nSheet), nStartRow + nRows, nSheet, pRowFormatRanges);
                WriteRowContent();
                CloseRow(nStartRow + nRows - 1);
            }
            OpenRow(nSheet, nEndRow, 1, aRowAttr);
            pRowFormatRanges->Clear();
            pCellStyles->GetFormatRanges(0, nEndCol, nEndRow, nSheet, pRowFormatRanges);
            WriteRowContent();
        }
    }
}

void ScXMLExport::GetColumnRowHeader(bool& rHasColumnHeader, table::CellRangeAddress& rColumnHeaderRange,
                                     bool& rHasRowHeader, table::CellRangeAddress& rRowHeaderRange,
                                     OUString& rPrintRanges) const
{
    uno::Reference <sheet::XPrintAreas> xPrintAreas (xCurrentTable, uno::UNO_QUERY);
    if (xPrintAreas.is())
    {
        rHasRowHeader = xPrintAreas->getPrintTitleRows();
        rHasColumnHeader = xPrintAreas->getPrintTitleColumns();
        rRowHeaderRange = xPrintAreas->getTitleRows();
        rColumnHeaderRange = xPrintAreas->getTitleColumns();
        uno::Sequence< table::CellRangeAddress > aRangeList( xPrintAreas->getPrintAreas() );
        ScRangeStringConverter::GetStringFromRangeList( rPrintRanges, aRangeList, pDoc, FormulaGrammar::CONV_OOO );
    }
}

void ScXMLExport::FillFieldGroup(ScOutlineArray* pFields, ScMyOpenCloseColumnRowGroup* pGroups)
{
    size_t nDepth = pFields->GetDepth();
    for (size_t i = 0; i < nDepth; ++i)
    {
        size_t nFields = pFields->GetCount(i);
        for (size_t j = 0; j < nFields; ++j)
        {
            ScMyColumnRowGroup aGroup;
            const ScOutlineEntry* pEntry = pFields->GetEntry(i, j);
            aGroup.nField = pEntry->GetStart();
            aGroup.nLevel = static_cast<sal_Int16>(i);
            aGroup.bDisplay = !(pEntry->IsHidden());
            pGroups->AddGroup(aGroup, pEntry->GetEnd());
        }
    }
    if (nDepth)
        pGroups->Sort();
}

void ScXMLExport::FillColumnRowGroups()
{
    if (pDoc)
    {
        ScOutlineTable* pOutlineTable = pDoc->GetOutlineTable( static_cast<SCTAB>(nCurrentTable), false );
        if(pOutlineTable)
        {
            ScOutlineArray* pCols(pOutlineTable->GetColArray());
            ScOutlineArray* pRows(pOutlineTable->GetRowArray());
            if (pCols)
                FillFieldGroup(pCols, pGroupColumns);
            if (pRows)
                FillFieldGroup(pRows, pGroupRows);
            pSharedData->SetLastColumn(nCurrentTable, pGroupColumns->GetLast());
            pSharedData->SetLastRow(nCurrentTable, pGroupRows->GetLast());
        }
    }
}

void ScXMLExport::SetBodyAttributes()
{
    if (pDoc && pDoc->IsDocProtected())
    {
        AddAttribute(XML_NAMESPACE_TABLE, XML_STRUCTURE_PROTECTED, XML_TRUE);
        OUStringBuffer aBuffer;
        uno::Sequence<sal_Int8> aPassHash;
        ScPasswordHash eHashUsed = PASSHASH_UNSPECIFIED;
        const ScDocProtection* p = pDoc->GetDocProtection();
        if (p)
        {
            if (p->hasPasswordHash(PASSHASH_SHA1))
            {
                aPassHash = p->getPasswordHash(PASSHASH_SHA1);
                eHashUsed = PASSHASH_SHA1;
            }
            else if (p->hasPasswordHash(PASSHASH_XL, PASSHASH_SHA1))
            {
                aPassHash = p->getPasswordHash(PASSHASH_XL, PASSHASH_SHA1);
                eHashUsed = PASSHASH_XL;
            }
        }
        ::sax::Converter::encodeBase64(aBuffer, aPassHash);
        if (!aBuffer.isEmpty())
        {
            AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY, aBuffer.makeStringAndClear());
            if ( getDefaultVersion() >= SvtSaveOptions::ODFVER_012 )
            {
                if (eHashUsed == PASSHASH_XL)
                {
                    AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY_DIGEST_ALGORITHM,
                                 ScPassHashHelper::getHashURI(PASSHASH_XL));
                    AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY_DIGEST_ALGORITHM_2,
                                 ScPassHashHelper::getHashURI(PASSHASH_SHA1));
                }
                else if (eHashUsed == PASSHASH_SHA1)
                    AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY_DIGEST_ALGORITHM,
                                 ScPassHashHelper::getHashURI(PASSHASH_SHA1));
            }
        }
    }
}

static bool lcl_CopyStreamElement( const uno::Reference< io::XInputStream >& xInput,
                            const uno::Reference< io::XOutputStream >& xOutput,
                            sal_Int32 nCount )
{
    const sal_Int32 nBufSize = 16*1024;
    uno::Sequence<sal_Int8> aSequence(nBufSize);

    sal_Int32 nRemaining = nCount;
    bool bFirst = true;

    while ( nRemaining > 0 )
    {
        sal_Int32 nRead = xInput->readBytes( aSequence, std::min( nRemaining, nBufSize ) );
        if (bFirst)
        {
            // safety check: Make sure the copied part actually points to the start of an element
            if ( nRead < 1 || aSequence[0] != static_cast<sal_Int8>('<') )
            {
                return false;   // abort and set an error
            }
            bFirst = false;
        }
        if (nRead == nRemaining)
        {
            // safety check: Make sure the copied part also ends at the end of an element
            if ( aSequence[nRead-1] != static_cast<sal_Int8>('>') )
            {
                return false;   // abort and set an error
            }
        }

        if ( nRead == nBufSize )
        {
            xOutput->writeBytes( aSequence );
            nRemaining -= nRead;
        }
        else
        {
            if ( nRead > 0 )
            {
                uno::Sequence<sal_Int8> aTempBuf( aSequence.getConstArray(), nRead );
                xOutput->writeBytes( aTempBuf );
            }
            nRemaining = 0;
        }
    }
    return true;    // successful
}

static void lcl_SkipBytesInBlocks( const uno::Reference< io::XInputStream >& xInput, sal_Int32 nBytesToSkip )
{
    // skipBytes in zip stream is implemented as reading.
    // For now, split into several calls to avoid allocating a large buffer.
    // Later, skipBytes should be changed.

    const sal_Int32 nMaxSize = 32*1024;

    if ( nBytesToSkip > 0 )
    {
        sal_Int32 nRemaining = nBytesToSkip;
        while ( nRemaining > 0 )
        {
            sal_Int32 nSkip = std::min( nRemaining, nMaxSize );
            xInput->skipBytes( nSkip );
            nRemaining -= nSkip;
        }
    }
}

void ScXMLExport::CopySourceStream( sal_Int32 nStartOffset, sal_Int32 nEndOffset, sal_Int32& rNewStart, sal_Int32& rNewEnd )
{
    uno::Reference<xml::sax::XDocumentHandler> xHandler = GetDocHandler();
    uno::Reference<io::XActiveDataSource> xDestSource( xHandler, uno::UNO_QUERY );
    if ( xDestSource.is() )
    {
        uno::Reference<io::XOutputStream> xDestStream = xDestSource->getOutputStream();
        uno::Reference<io::XSeekable> xDestSeek( xDestStream, uno::UNO_QUERY );
        if ( xDestSeek.is() )
        {
            // temporary: set same stream again to clear buffer
            xDestSource->setOutputStream( xDestStream );

            if ( getExportFlags() & EXPORT_PRETTY )
            {
                const OString aOutStr("\n   ");
                uno::Sequence<sal_Int8> aOutSeq( (sal_Int8*)aOutStr.getStr(), aOutStr.getLength() );
                xDestStream->writeBytes( aOutSeq );
            }

            rNewStart = (sal_Int32)xDestSeek->getPosition();

            if ( nStartOffset > nSourceStreamPos )
                lcl_SkipBytesInBlocks( xSourceStream, nStartOffset - nSourceStreamPos );

            if ( !lcl_CopyStreamElement( xSourceStream, xDestStream, nEndOffset - nStartOffset ) )
            {
                // If copying went wrong, set an error.
                // ScXMLImportWrapper then resets all stream flags, so the next save attempt will use normal saving.

                uno::Sequence<OUString> aEmptySeq;
                SetError(XMLERROR_CANCEL|XMLERROR_FLAG_SEVERE, aEmptySeq);
            }
            nSourceStreamPos = nEndOffset;

            rNewEnd = (sal_Int32)xDestSeek->getPosition();
        }
    }
}

const ScXMLEditAttributeMap& ScXMLExport::GetEditAttributeMap() const
{
    if (!mpEditAttrMap)
        mpEditAttrMap.reset(new ScXMLEditAttributeMap);
    return *mpEditAttrMap;
}

void ScXMLExport::_ExportContent()
{
    nCurrentTable = 0;
    if (!pSharedData)
    {
        sal_Int32 nTableCount(0);
        sal_Int32 nShapesCount(0);
        CollectSharedData(nTableCount, nShapesCount);
        OSL_FAIL("no shared data setted");
    }
    ScXMLExportDatabaseRanges aExportDatabaseRanges(*this);
    if (!GetModel().is())
        return;

    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( GetModel(), uno::UNO_QUERY );
    if ( !xSpreadDoc.is() )
        return;

    ScSheetSaveData* pSheetData = ScModelObj::getImplementation(xSpreadDoc)->GetSheetSaveData();
    if (pSheetData)
        pSheetData->ResetSaveEntries();

    uno::Reference<container::XIndexAccess> xIndex( xSpreadDoc->getSheets(), uno::UNO_QUERY );
    if ( xIndex.is() )
    {
        //_GetNamespaceMap().ClearQNamesCache();
        pChangeTrackingExportHelper->CollectAndWriteChanges();
        WriteCalculationSettings(xSpreadDoc);
        sal_Int32 nTableCount(xIndex->getCount());
        ScMyAreaLinksContainer aAreaLinks;
        GetAreaLinks( xSpreadDoc, aAreaLinks );
        ScMyEmptyDatabaseRangesContainer aEmptyRanges(aExportDatabaseRanges.GetEmptyDatabaseRanges());
        ScMyDetectiveOpContainer aDetectiveOpContainer;
        GetDetectiveOpList( aDetectiveOpContainer );

        pCellStyles->Sort();
        pMergedRangesContainer->Sort();
        pSharedData->GetDetectiveObjContainer()->Sort();

        pCellsItr->Clear();
        pCellsItr->SetShapes( pSharedData->GetShapesContainer() );
        pCellsItr->SetNoteShapes( pSharedData->GetNoteShapes() );
        pCellsItr->SetMergedRanges( pMergedRangesContainer );
        pCellsItr->SetAreaLinks( &aAreaLinks );
        pCellsItr->SetEmptyDatabaseRanges( &aEmptyRanges );
        pCellsItr->SetDetectiveObj( pSharedData->GetDetectiveObjContainer() );
        pCellsItr->SetDetectiveOp( &aDetectiveOpContainer );

        if (nTableCount > 0)
            pValidationsContainer->WriteValidations(*this);
        WriteTheLabelRanges( xSpreadDoc );
        for (sal_Int32 nTable = 0; nTable < nTableCount; ++nTable)
        {
            sal_Int32 nStartOffset = -1;
            sal_Int32 nEndOffset = -1;
            if (pSheetData && pDoc && pDoc->IsStreamValid((SCTAB)nTable) && !pDoc->GetChangeTrack())
                pSheetData->GetStreamPos( nTable, nStartOffset, nEndOffset );

            if ( nStartOffset >= 0 && nEndOffset >= 0 && xSourceStream.is() )
            {
                sal_Int32 nNewStart = -1;
                sal_Int32 nNewEnd = -1;
                CopySourceStream( nStartOffset, nEndOffset, nNewStart, nNewEnd );

                // store position of copied sheet in output
                pSheetData->AddSavePos( nTable, nNewStart, nNewEnd );

                // skip iterator entries for this sheet
                pCellsItr->SkipTable(static_cast<SCTAB>(nTable));
            }
            else
            {
                uno::Reference<sheet::XSpreadsheet> xTable(xIndex->getByIndex(nTable), uno::UNO_QUERY);
                WriteTable(nTable, xTable);
            }
            IncrementProgressBar(false);
        }
    }
    WriteExternalRefCaches();
    WriteNamedExpressions();
    aExportDatabaseRanges.WriteDatabaseRanges();
    ScXMLExportDataPilot aExportDataPilot(*this);
    aExportDataPilot.WriteDataPilots(xSpreadDoc);
    WriteConsolidation();
    ScXMLExportDDELinks aExportDDELinks(*this);
    aExportDDELinks.WriteDDELinks(xSpreadDoc);
    IncrementProgressBar(true, 0);
    GetProgressBarHelper()->SetValue(GetProgressBarHelper()->GetReference());
}

void ScXMLExport::_ExportStyles( sal_Bool bUsed )
{
    if (!pSharedData)
    {
        sal_Int32 nTableCount(0);
        sal_Int32 nShapesCount(0);
        CollectSharedData(nTableCount, nShapesCount);
    }
    ScXMLStyleExport aStylesExp(*this, OUString(), GetAutoStylePool().get());
    if (GetModel().is())
    {
        uno::Reference <lang::XMultiServiceFactory> xMultiServiceFactory(GetModel(), uno::UNO_QUERY);
        if (xMultiServiceFactory.is())
        {
            uno::Reference <beans::XPropertySet> xProperties(xMultiServiceFactory->createInstance("com.sun.star.sheet.Defaults"), uno::UNO_QUERY);
            if (xProperties.is())
                aStylesExp.exportDefaultStyle(xProperties, OUString(XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME), xCellStylesExportPropertySetMapper);
            if (pSharedData->HasShapes())
            {
                GetShapeExport()->ExportGraphicDefaults();
            }
        }
        uno::Reference <style::XStyleFamiliesSupplier> xStyleFamiliesSupplier (GetModel(), uno::UNO_QUERY);
        if (xStyleFamiliesSupplier.is())
        {
            uno::Reference <container::XNameAccess> xStylesFamilies(xStyleFamiliesSupplier->getStyleFamilies());
            if (xStylesFamilies.is())
            {
                uno::Reference <container::XIndexAccess> xCellStyles(xStylesFamilies->getByName("CellStyles"), uno::UNO_QUERY);
                if (xCellStyles.is())
                {
                    sal_Int32 nCount(xCellStyles->getCount());
                    OUString sNumberFormat(SC_UNONAME_NUMFMT);
                    for (sal_Int32 i = 0; i < nCount; ++i)
                    {
                        uno::Reference <beans::XPropertySet> xCellProperties(xCellStyles->getByIndex(i), uno::UNO_QUERY);
                        if (xCellProperties.is())
                        {
                            sal_Int32 nNumberFormat = 0;
                            if (xCellProperties->getPropertyValue(sNumberFormat) >>= nNumberFormat)
                                addDataStyle(nNumberFormat);
                        }
                    }
                }
            }
        }
    }
    exportDataStyles();

    aStylesExp.exportStyleFamily(OUString("CellStyles"),
        OUString(XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME), xCellStylesExportPropertySetMapper, false, XML_STYLE_FAMILY_TABLE_CELL);

    SvXMLExport::_ExportStyles(bUsed);
}

void ScXMLExport::AddStyleFromCells(const uno::Reference<beans::XPropertySet>& xProperties,
                                    const uno::Reference<sheet::XSpreadsheet>& xTable,
                                    sal_Int32 nTable, const OUString* pOldName)
{
    //! pass xCellRanges instead
    uno::Reference<sheet::XSheetCellRanges> xCellRanges( xProperties, uno::UNO_QUERY );

    OUString SC_SCELLPREFIX(XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX);
    OUString SC_NUMBERFORMAT(SC_UNONAME_NUMFMT);

    OUString sStyleName;
    sal_Int32 nNumberFormat(-1);
    sal_Int32 nValidationIndex(-1);
    std::vector< XMLPropertyState > xPropStates(xCellStylesExportPropertySetMapper->Filter( xProperties ));
    std::vector< XMLPropertyState >::iterator aItr(xPropStates.begin());
    std::vector< XMLPropertyState >::iterator aEndItr(xPropStates.end());
    sal_Int32 nCount(0);
    while (aItr != aEndItr)
    {
        if (aItr->mnIndex != -1)
        {
            switch (xCellStylesPropertySetMapper->GetEntryContextId(aItr->mnIndex))
            {
                case CTF_SC_VALIDATION :
                {
                    pValidationsContainer->AddValidation(aItr->maValue, nValidationIndex);
                    // this is not very slow, because it is most the last property or
                    // if it is not the last property it is the property before the last property,
                    // so in the worst case only one property has to be copied, but in the best case no
                    // property has to be copied
                    aItr = xPropStates.erase(aItr);
                    aEndItr = xPropStates.end();    // old aEndItr is invalidated!
                }
                break;
                case CTF_SC_CELLSTYLE :
                {
                    aItr->maValue >>= sStyleName;
                    aItr->mnIndex = -1;
                    ++aItr;
                    ++nCount;
                }
                break;
                case CTF_SC_NUMBERFORMAT :
                {
                    if (aItr->maValue >>= nNumberFormat)
                        addDataStyle(nNumberFormat);
                    ++aItr;
                    ++nCount;
                }
                break;
                default:
                {
                    ++aItr;
                    ++nCount;
                }
                break;
            }
        }
        else
        {
            ++aItr;
            ++nCount;
        }
    }
    if (nCount == 1) // this is the CellStyle and should be removed if alone
        xPropStates.clear();
    if (nNumberFormat == -1)
        xProperties->getPropertyValue(SC_NUMBERFORMAT) >>= nNumberFormat;
    if (!sStyleName.isEmpty())
    {
        if (!xPropStates.empty())
        {
            sal_Int32 nIndex;
            if (pOldName)
            {
                if (GetAutoStylePool()->AddNamed(*pOldName, XML_STYLE_FAMILY_TABLE_CELL, sStyleName, xPropStates))
                {
                    GetAutoStylePool()->RegisterName(XML_STYLE_FAMILY_TABLE_CELL, *pOldName);
                    // add to pCellStyles, so the name is found for normal sheets
                    OUString* pTemp(new OUString(*pOldName));
                    if (!pCellStyles->AddStyleName(pTemp, nIndex))
                        delete pTemp;
                }
            }
            else
            {
                OUString sName;
                bool bIsAutoStyle(true);
                if (GetAutoStylePool()->Add(sName, XML_STYLE_FAMILY_TABLE_CELL, sStyleName, xPropStates))
                {
                    OUString* pTemp(new OUString(sName));
                    if (!pCellStyles->AddStyleName(pTemp, nIndex))
                        delete pTemp;
                }
                else
                    nIndex = pCellStyles->GetIndexOfStyleName(sName, SC_SCELLPREFIX, bIsAutoStyle);

                uno::Sequence<table::CellRangeAddress> aAddresses(xCellRanges->getRangeAddresses());
                table::CellRangeAddress* pAddresses(aAddresses.getArray());
                bool bGetMerge(true);
                for (sal_Int32 i = 0; i < aAddresses.getLength(); ++i, ++pAddresses)
                {
                    pSharedData->SetLastColumn(nTable, pAddresses->EndColumn);
                    pSharedData->SetLastRow(nTable, pAddresses->EndRow);
                    pCellStyles->AddRangeStyleName(*pAddresses, nIndex, bIsAutoStyle, nValidationIndex, nNumberFormat);
                    if (bGetMerge)
                        bGetMerge = GetMerged(pAddresses, xTable);
                }
            }
        }
        else
        {
            OUString* pTemp(new OUString(EncodeStyleName(sStyleName)));
            sal_Int32 nIndex(0);
            if (!pCellStyles->AddStyleName(pTemp, nIndex, false))
            {
                delete pTemp;
                pTemp = NULL;
            }
            if ( !pOldName )
            {
                uno::Sequence<table::CellRangeAddress> aAddresses(xCellRanges->getRangeAddresses());
                table::CellRangeAddress* pAddresses(aAddresses.getArray());
                bool bGetMerge(true);
                for (sal_Int32 i = 0; i < aAddresses.getLength(); ++i, ++pAddresses)
                {
                    if (bGetMerge)
                        bGetMerge = GetMerged(pAddresses, xTable);
                    pCellStyles->AddRangeStyleName(*pAddresses, nIndex, false, nValidationIndex, nNumberFormat);
                    if (!sStyleName.equalsAsciiL("Default", 7) || nValidationIndex != -1)
                    {
                        pSharedData->SetLastColumn(nTable, pAddresses->EndColumn);
                        pSharedData->SetLastRow(nTable, pAddresses->EndRow);
                    }
                }
            }
        }
    }
}

void ScXMLExport::AddStyleFromColumn(const uno::Reference<beans::XPropertySet>& xColumnProperties,
                                     const OUString* pOldName, sal_Int32& rIndex, bool& rIsVisible)
{
    OUString SC_SCOLUMNPREFIX(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX);

    std::vector<XMLPropertyState> xPropStates(xColumnStylesExportPropertySetMapper->Filter(xColumnProperties));
    if(!xPropStates.empty())
    {
        std::vector< XMLPropertyState >::iterator aItr(xPropStates.begin());
        std::vector< XMLPropertyState >::iterator aEndItr(xPropStates.end());
        while (aItr != aEndItr)
        {
            if (xColumnStylesPropertySetMapper->GetEntryContextId(aItr->mnIndex) == CTF_SC_ISVISIBLE)
            {
                aItr->maValue >>= rIsVisible;
                break;
            }
            ++aItr;
        }

        OUString sParent;
        if (pOldName)
        {
            if (GetAutoStylePool()->AddNamed(*pOldName, XML_STYLE_FAMILY_TABLE_COLUMN, sParent, xPropStates))
            {
                GetAutoStylePool()->RegisterName(XML_STYLE_FAMILY_TABLE_COLUMN, *pOldName);
                // add to pColumnStyles, so the name is found for normal sheets
                OUString* pTemp(new OUString(*pOldName));
                rIndex = pColumnStyles->AddStyleName(pTemp);
            }
        }
        else
        {
            OUString sName;
            if (GetAutoStylePool()->Add(sName, XML_STYLE_FAMILY_TABLE_COLUMN, sParent, xPropStates))
            {
                OUString* pTemp(new OUString(sName));
                rIndex = pColumnStyles->AddStyleName(pTemp);
            }
            else
                rIndex = pColumnStyles->GetIndexOfStyleName(sName, SC_SCOLUMNPREFIX);
        }
    }
}

void ScXMLExport::AddStyleFromRow(const uno::Reference<beans::XPropertySet>& xRowProperties,
                                  const OUString* pOldName, sal_Int32& rIndex)
{
    OUString SC_SROWPREFIX(XML_STYLE_FAMILY_TABLE_ROW_STYLES_PREFIX);

    std::vector<XMLPropertyState> xPropStates(xRowStylesExportPropertySetMapper->Filter(xRowProperties));
    if(!xPropStates.empty())
    {
        OUString sParent;
        if (pOldName)
        {
            if (GetAutoStylePool()->AddNamed(*pOldName, XML_STYLE_FAMILY_TABLE_ROW, sParent, xPropStates))
            {
                GetAutoStylePool()->RegisterName(XML_STYLE_FAMILY_TABLE_ROW, *pOldName);
                // add to pRowStyles, so the name is found for normal sheets
                OUString* pTemp(new OUString(*pOldName));
                rIndex = pRowStyles->AddStyleName(pTemp);
            }
        }
        else
        {
            OUString sName;
            if (GetAutoStylePool()->Add(sName, XML_STYLE_FAMILY_TABLE_ROW, sParent, xPropStates))
            {
                OUString* pTemp(new OUString(sName));
                rIndex = pRowStyles->AddStyleName(pTemp);
            }
            else
                rIndex = pRowStyles->GetIndexOfStyleName(sName, SC_SROWPREFIX);
        }
    }
}

static uno::Any lcl_GetEnumerated( uno::Reference<container::XEnumerationAccess> xEnumAccess, sal_Int32 nIndex )
{
    uno::Any aRet;
    uno::Reference<container::XEnumeration> xEnum( xEnumAccess->createEnumeration() );
    try
    {
        sal_Int32 nSkip = nIndex;
        while ( nSkip > 0 )
        {
            (void) xEnum->nextElement();
            --nSkip;
        }
        aRet = xEnum->nextElement();
    }
    catch (container::NoSuchElementException&)
    {
        // leave aRet empty
    }
    return aRet;
}

void ScXMLExport::_ExportAutoStyles()
{
    if (!GetModel().is())
        return;

    Reference <sheet::XSpreadsheetDocument> xSpreadDoc( GetModel(), uno::UNO_QUERY );
    if (!xSpreadDoc.is())
        return;

    Reference<container::XIndexAccess> xIndex( xSpreadDoc->getSheets(), uno::UNO_QUERY );
    if (!xIndex.is())
        return;

    if (getExportFlags() & EXPORT_CONTENT)
    {
        //  re-create automatic styles with old names from stored data
        ScSheetSaveData* pSheetData = ScModelObj::getImplementation(xSpreadDoc)->GetSheetSaveData();
        if (pSheetData && pDoc)
        {
            // formulas have to be calculated now, to detect changed results
            // (during normal save, they will be calculated anyway)
            SCTAB nTabCount = pDoc->GetTableCount();
            for (SCTAB nTab=0; nTab<nTabCount; ++nTab)
                if (pDoc->IsStreamValid(nTab))
                    pDoc->InterpretDirtyCells(ScRange(0, 0, nTab, MAXCOL, MAXROW, nTab));

            // stored cell styles
            const std::vector<ScCellStyleEntry>& rCellEntries = pSheetData->GetCellStyles();
            std::vector<ScCellStyleEntry>::const_iterator aCellIter = rCellEntries.begin();
            std::vector<ScCellStyleEntry>::const_iterator aCellEnd = rCellEntries.end();
            while (aCellIter != aCellEnd)
            {
                ScAddress aPos = aCellIter->maCellPos;
                sal_Int32 nTable = aPos.Tab();
                bool bCopySheet = pDoc->IsStreamValid( static_cast<SCTAB>(nTable) );
                if (bCopySheet)
                {
                    Reference <sheet::XSpreadsheet> xTable(xIndex->getByIndex(nTable), uno::UNO_QUERY);
                    Reference <beans::XPropertySet> xProperties(
                        xTable->getCellByPosition( aPos.Col(), aPos.Row() ), uno::UNO_QUERY );

                    AddStyleFromCells(xProperties, xTable, nTable, &aCellIter->maName);
                }
                ++aCellIter;
            }

            // stored column styles
            const std::vector<ScCellStyleEntry>& rColumnEntries = pSheetData->GetColumnStyles();
            std::vector<ScCellStyleEntry>::const_iterator aColumnIter = rColumnEntries.begin();
            std::vector<ScCellStyleEntry>::const_iterator aColumnEnd = rColumnEntries.end();
            while (aColumnIter != aColumnEnd)
            {
                ScAddress aPos = aColumnIter->maCellPos;
                sal_Int32 nTable = aPos.Tab();
                bool bCopySheet = pDoc->IsStreamValid( static_cast<SCTAB>(nTable) );
                if (bCopySheet)
                {
                    Reference<table::XColumnRowRange> xColumnRowRange(xIndex->getByIndex(nTable), uno::UNO_QUERY);
                    Reference<table::XTableColumns> xTableColumns(xColumnRowRange->getColumns());
                    Reference<beans::XPropertySet> xColumnProperties(xTableColumns->getByIndex( aPos.Col() ), uno::UNO_QUERY);

                    sal_Int32 nIndex(-1);
                    bool bIsVisible(true);
                    AddStyleFromColumn( xColumnProperties, &aColumnIter->maName, nIndex, bIsVisible );
                }
                ++aColumnIter;
            }

            // stored row styles
            const std::vector<ScCellStyleEntry>& rRowEntries = pSheetData->GetRowStyles();
            std::vector<ScCellStyleEntry>::const_iterator aRowIter = rRowEntries.begin();
            std::vector<ScCellStyleEntry>::const_iterator aRowEnd = rRowEntries.end();
            while (aRowIter != aRowEnd)
            {
                ScAddress aPos = aRowIter->maCellPos;
                sal_Int32 nTable = aPos.Tab();
                bool bCopySheet = pDoc->IsStreamValid( static_cast<SCTAB>(nTable) );
                if (bCopySheet)
                {
                    Reference<table::XColumnRowRange> xColumnRowRange(xIndex->getByIndex(nTable), uno::UNO_QUERY);
                    Reference<table::XTableRows> xTableRows(xColumnRowRange->getRows());
                    Reference<beans::XPropertySet> xRowProperties(xTableRows->getByIndex( aPos.Row() ), uno::UNO_QUERY);

                    sal_Int32 nIndex(-1);
                    AddStyleFromRow( xRowProperties, &aRowIter->maName, nIndex );
                }
                ++aRowIter;
            }

            // stored table styles
            const std::vector<ScCellStyleEntry>& rTableEntries = pSheetData->GetTableStyles();
            std::vector<ScCellStyleEntry>::const_iterator aTableIter = rTableEntries.begin();
            std::vector<ScCellStyleEntry>::const_iterator aTableEnd = rTableEntries.end();
            while (aTableIter != aTableEnd)
            {
                ScAddress aPos = aTableIter->maCellPos;
                sal_Int32 nTable = aPos.Tab();
                bool bCopySheet = pDoc->IsStreamValid( static_cast<SCTAB>(nTable) );
                if (bCopySheet)
                {
                    //! separate method AddStyleFromTable needed?
                    Reference<beans::XPropertySet> xTableProperties(xIndex->getByIndex(nTable), uno::UNO_QUERY);
                    if (xTableProperties.is())
                    {
                        std::vector<XMLPropertyState> xPropStates(xTableStylesExportPropertySetMapper->Filter(xTableProperties));
                        OUString sParent;
                        OUString sName( aTableIter->maName );
                        GetAutoStylePool()->AddNamed(sName, XML_STYLE_FAMILY_TABLE_TABLE, sParent, xPropStates);
                        GetAutoStylePool()->RegisterName(XML_STYLE_FAMILY_TABLE_TABLE, sName);
                    }
                }
                ++aTableIter;
            }

            // stored styles for notes

            UniReference<SvXMLExportPropertyMapper> xShapeMapper = XMLShapeExport::CreateShapePropMapper( *this );
            GetShapeExport(); // make sure the graphics styles family is added

            const std::vector<ScNoteStyleEntry>& rNoteEntries = pSheetData->GetNoteStyles();
            std::vector<ScNoteStyleEntry>::const_iterator aNoteIter = rNoteEntries.begin();
            std::vector<ScNoteStyleEntry>::const_iterator aNoteEnd = rNoteEntries.end();
            while (aNoteIter != aNoteEnd)
            {
                ScAddress aPos = aNoteIter->maCellPos;
                SCTAB nTable = aPos.Tab();
                bool bCopySheet = pDoc->IsStreamValid( nTable );
                if (bCopySheet)
                {
                    //! separate method AddStyleFromNote needed?

                    ScPostIt* pNote = pDoc->GetNotes( nTable )->findByAddress(aPos);
                    OSL_ENSURE( pNote, "note not found" );
                    if (pNote)
                    {
                        SdrCaptionObj* pDrawObj = pNote->GetOrCreateCaption( aPos );
                        // all uno shapes are created anyway in CollectSharedData
                        Reference<beans::XPropertySet> xShapeProperties( pDrawObj->getUnoShape(), uno::UNO_QUERY );
                        if (xShapeProperties.is())
                        {
                            if ( !aNoteIter->maStyleName.isEmpty() )
                            {
                                std::vector<XMLPropertyState> xPropStates(xShapeMapper->Filter(xShapeProperties));
                                OUString sParent;
                                OUString sName( aNoteIter->maStyleName );
                                GetAutoStylePool()->AddNamed(sName, XML_STYLE_FAMILY_SD_GRAPHICS_ID, sParent, xPropStates);
                                GetAutoStylePool()->RegisterName(XML_STYLE_FAMILY_SD_GRAPHICS_ID, sName);
                            }
                            if ( !aNoteIter->maTextStyle.isEmpty() )
                            {
                                std::vector<XMLPropertyState> xPropStates(
                                    GetTextParagraphExport()->GetParagraphPropertyMapper()->Filter(xShapeProperties));
                                OUString sParent;
                                OUString sName( aNoteIter->maTextStyle );
                                GetAutoStylePool()->AddNamed(sName, XML_STYLE_FAMILY_TEXT_PARAGRAPH, sParent, xPropStates);
                                GetAutoStylePool()->RegisterName(XML_STYLE_FAMILY_TEXT_PARAGRAPH, sName);
                            }
                        }
                    }
                }
                ++aNoteIter;
            }

            // note paragraph styles

            UniReference<SvXMLExportPropertyMapper> xParaPropMapper = GetTextParagraphExport()->GetParagraphPropertyMapper();

            const std::vector<ScTextStyleEntry>& rNoteParaEntries = pSheetData->GetNoteParaStyles();
            std::vector<ScTextStyleEntry>::const_iterator aNoteParaIter = rNoteParaEntries.begin();
            std::vector<ScTextStyleEntry>::const_iterator aNoteParaEnd = rNoteParaEntries.end();
            while (aNoteParaIter != aNoteParaEnd)
            {
                ScAddress aPos = aNoteParaIter->maCellPos;
                SCTAB nTable = aPos.Tab();
                bool bCopySheet = pDoc->IsStreamValid( nTable );
                if (bCopySheet)
                {
                    ScPostIt* pNote = pDoc->GetNotes(nTable)->findByAddress( aPos );
                    OSL_ENSURE( pNote, "note not found" );
                    if (pNote)
                    {
                        SdrCaptionObj* pDrawObj = pNote->GetOrCreateCaption( aPos );
                        Reference<container::XEnumerationAccess> xCellText(pDrawObj->getUnoShape(), uno::UNO_QUERY);
                        Reference<beans::XPropertySet> xParaProp(
                            lcl_GetEnumerated( xCellText, aNoteParaIter->maSelection.nStartPara ), uno::UNO_QUERY );
                        if ( xParaProp.is() )
                        {
                            std::vector<XMLPropertyState> xPropStates(xParaPropMapper->Filter(xParaProp));
                            OUString sParent;
                            OUString sName( aNoteParaIter->maName );
                            GetAutoStylePool()->AddNamed(sName, XML_STYLE_FAMILY_TEXT_PARAGRAPH, sParent, xPropStates);
                            GetAutoStylePool()->RegisterName(XML_STYLE_FAMILY_TEXT_PARAGRAPH, sName);
                        }
                    }
                }
                ++aNoteParaIter;
            }

            // note text styles

            UniReference<SvXMLExportPropertyMapper> xTextPropMapper = XMLTextParagraphExport::CreateCharExtPropMapper( *this );

            const std::vector<ScTextStyleEntry>& rNoteTextEntries = pSheetData->GetNoteTextStyles();
            std::vector<ScTextStyleEntry>::const_iterator aNoteTextIter = rNoteTextEntries.begin();
            std::vector<ScTextStyleEntry>::const_iterator aNoteTextEnd = rNoteTextEntries.end();
            while (aNoteTextIter != aNoteTextEnd)
            {
                ScAddress aPos = aNoteTextIter->maCellPos;
                SCTAB nTable = aPos.Tab();
                bool bCopySheet = pDoc->IsStreamValid( nTable );
                if (bCopySheet)
                {
                    ScPostIt* pNote = pDoc->GetNotes(nTable)->findByAddress( aPos );
                    OSL_ENSURE( pNote, "note not found" );
                    if (pNote)
                    {
                        SdrCaptionObj* pDrawObj = pNote->GetOrCreateCaption( aPos );
                        Reference<text::XSimpleText> xCellText(pDrawObj->getUnoShape(), uno::UNO_QUERY);
                        Reference<beans::XPropertySet> xCursorProp(xCellText->createTextCursor(), uno::UNO_QUERY);
                        ScDrawTextCursor* pCursor = ScDrawTextCursor::getImplementation( xCursorProp );
                        if (pCursor)
                        {
                            pCursor->SetSelection( aNoteTextIter->maSelection );

                            std::vector<XMLPropertyState> xPropStates(xTextPropMapper->Filter(xCursorProp));
                            OUString sParent;
                            OUString sName( aNoteTextIter->maName );
                            GetAutoStylePool()->AddNamed(sName, XML_STYLE_FAMILY_TEXT_TEXT, sParent, xPropStates);
                            GetAutoStylePool()->RegisterName(XML_STYLE_FAMILY_TEXT_TEXT, sName);
                        }
                    }
                }
                ++aNoteTextIter;
            }

            // stored text styles

            const std::vector<ScTextStyleEntry>& rTextEntries = pSheetData->GetTextStyles();
            std::vector<ScTextStyleEntry>::const_iterator aTextIter = rTextEntries.begin();
            std::vector<ScTextStyleEntry>::const_iterator aTextEnd = rTextEntries.end();
            while (aTextIter != aTextEnd)
            {
                ScAddress aPos = aTextIter->maCellPos;
                sal_Int32 nTable = aPos.Tab();
                bool bCopySheet = pDoc->IsStreamValid( static_cast<SCTAB>(nTable) );
                if (bCopySheet)
                {
                    //! separate method AddStyleFromText needed?
                    //! cache sheet object

                    Reference<table::XCellRange> xCellRange(xIndex->getByIndex(nTable), uno::UNO_QUERY);
                    Reference<text::XSimpleText> xCellText(xCellRange->getCellByPosition(aPos.Col(), aPos.Row()), uno::UNO_QUERY);
                    Reference<beans::XPropertySet> xCursorProp(xCellText->createTextCursor(), uno::UNO_QUERY);
                    ScCellTextCursor* pCursor = ScCellTextCursor::getImplementation( xCursorProp );
                    if (pCursor)
                    {
                        pCursor->SetSelection( aTextIter->maSelection );

                        std::vector<XMLPropertyState> xPropStates(xTextPropMapper->Filter(xCursorProp));
                        OUString sParent;
                        OUString sName( aTextIter->maName );
                        GetAutoStylePool()->AddNamed(sName, XML_STYLE_FAMILY_TEXT_TEXT, sParent, xPropStates);
                        GetAutoStylePool()->RegisterName(XML_STYLE_FAMILY_TEXT_TEXT, sName);
                    }
                }
                ++aTextIter;
            }
        }

        ExportExternalRefCacheStyles();

        if (!pSharedData)
        {
            sal_Int32 nTableCount(0);
            sal_Int32 nShapesCount(0);
            CollectSharedData(nTableCount, nShapesCount);
        }
        sal_Int32 nTableCount(xIndex->getCount());
        pCellStyles->AddNewTable(nTableCount - 1);
        CollectShapesAutoStyles(nTableCount);
        for (sal_Int32 nTable = 0; nTable < nTableCount; ++nTable, IncrementProgressBar(false))
        {
            bool bUseStream = pSheetData && pDoc && pDoc->IsStreamValid((SCTAB)nTable) &&
                              pSheetData->HasStreamPos(nTable) && xSourceStream.is() &&
                              !pDoc->GetChangeTrack();

            Reference <sheet::XSpreadsheet> xTable(xIndex->getByIndex(nTable), uno::UNO_QUERY);
            if (!xTable.is())
                continue;

            // table styles array must be complete, including copied tables - Add should find the stored style
            Reference<beans::XPropertySet> xTableProperties(xTable, uno::UNO_QUERY);
            if (xTableProperties.is())
            {
                std::vector<XMLPropertyState> xPropStates(xTableStylesExportPropertySetMapper->Filter(xTableProperties));
                if(!xPropStates.empty())
                {
                    OUString sParent;
                    OUString sName;
                    GetAutoStylePool()->Add(sName, XML_STYLE_FAMILY_TABLE_TABLE, sParent, xPropStates);
                    aTableStyles.push_back(sName);
                }
            }

            if (bUseStream)
                continue;

            // collect other auto-styles only for non-copied sheets
            Reference<sheet::XUniqueCellFormatRangesSupplier> xCellFormatRanges ( xTable, uno::UNO_QUERY );
            if ( xCellFormatRanges.is() )
            {
                Reference<container::XIndexAccess> xFormatRangesIndex(xCellFormatRanges->getUniqueCellFormatRanges());
                if (xFormatRangesIndex.is())
                {
                    sal_Int32 nFormatRangesCount(xFormatRangesIndex->getCount());
                    GetProgressBarHelper()->ChangeReference(GetProgressBarHelper()->GetReference() + nFormatRangesCount);
                    for (sal_Int32 nFormatRange = 0; nFormatRange < nFormatRangesCount; ++nFormatRange)
                    {
                        Reference< sheet::XSheetCellRanges> xCellRanges(xFormatRangesIndex->getByIndex(nFormatRange), uno::UNO_QUERY);
                        if (xCellRanges.is())
                        {
                            Reference <beans::XPropertySet> xProperties (xCellRanges, uno::UNO_QUERY);
                            if (xProperties.is())
                            {
                                AddStyleFromCells(xProperties, xTable, nTable, NULL);
                                IncrementProgressBar(false);
                            }
                        }
                    }
                }
            }
            Reference<table::XColumnRowRange> xColumnRowRange (xTable, uno::UNO_QUERY);
            if (xColumnRowRange.is())
            {
                if (pDoc)
                {
                    pDoc->SyncColRowFlags();
                    Reference<table::XTableColumns> xTableColumns(xColumnRowRange->getColumns());
                    if (xTableColumns.is())
                    {
                        sal_Int32 nColumns(pDoc->GetLastChangedCol(sal::static_int_cast<SCTAB>(nTable)));
                        pSharedData->SetLastColumn(nTable, nColumns);
                        table::CellRangeAddress aCellAddress(GetEndAddress(xTable, nTable));
                        if (aCellAddress.EndColumn > nColumns)
                        {
                            ++nColumns;
                            pColumnStyles->AddNewTable(nTable, aCellAddress.EndColumn);
                        }
                        else
                            pColumnStyles->AddNewTable(nTable, nColumns);
                        sal_Int32 nColumn = 0;
                        while (nColumn <= MAXCOL)
                        {
                            sal_Int32 nIndex(-1);
                            bool bIsVisible(true);
                            Reference <beans::XPropertySet> xColumnProperties(xTableColumns->getByIndex(nColumn), uno::UNO_QUERY);
                            if (xColumnProperties.is())
                            {
                                AddStyleFromColumn( xColumnProperties, NULL, nIndex, bIsVisible );
                                pColumnStyles->AddFieldStyleName(nTable, nColumn, nIndex, bIsVisible);
                            }
                            sal_Int32 nOld(nColumn);
                            nColumn = pDoc->GetNextDifferentChangedCol(sal::static_int_cast<SCTAB>(nTable), static_cast<SCCOL>(nColumn));
                            for (sal_Int32 i = nOld + 1; i < nColumn; ++i)
                                pColumnStyles->AddFieldStyleName(nTable, i, nIndex, bIsVisible);
                        }
                        if (aCellAddress.EndColumn > nColumns)
                        {
                            bool bIsVisible(true);
                            sal_Int32 nIndex(pColumnStyles->GetStyleNameIndex(nTable, nColumns, bIsVisible));
                            for (sal_Int32 i = nColumns + 1; i <= aCellAddress.EndColumn; ++i)
                                pColumnStyles->AddFieldStyleName(nTable, i, nIndex, bIsVisible);
                        }
                    }
                    Reference<table::XTableRows> xTableRows(xColumnRowRange->getRows());
                    if (xTableRows.is())
                    {
                        sal_Int32 nRows(pDoc->GetLastChangedRow(sal::static_int_cast<SCTAB>(nTable)));
                        pSharedData->SetLastRow(nTable, nRows);

                        pRowStyles->AddNewTable(nTable, MAXROW);
                        sal_Int32 nRow = 0;
                        while (nRow <= MAXROW)
                        {
                            sal_Int32 nIndex = 0;
                            Reference <beans::XPropertySet> xRowProperties(xTableRows->getByIndex(nRow), uno::UNO_QUERY);
                            if(xRowProperties.is())
                            {
                                AddStyleFromRow( xRowProperties, NULL, nIndex );
                                pRowStyles->AddFieldStyleName(nTable, nRow, nIndex);
                            }
                            sal_Int32 nOld(nRow);
                            nRow = pDoc->GetNextDifferentChangedRow(sal::static_int_cast<SCTAB>(nTable), static_cast<SCROW>(nRow), false);
                            if (nRow > nOld + 1)
                                pRowStyles->AddFieldStyleName(nTable, nOld + 1, nIndex, nRow - 1);
                        }
                    }
                }
            }

            ExportCellTextAutoStyles(nTable);
        }

        pChangeTrackingExportHelper->CollectAutoStyles();

        GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_COLUMN,
            GetDocHandler(), GetMM100UnitConverter(), GetNamespaceMap());
        GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_ROW,
            GetDocHandler(), GetMM100UnitConverter(), GetNamespaceMap());
        GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_TABLE,
            GetDocHandler(), GetMM100UnitConverter(), GetNamespaceMap());
        exportAutoDataStyles();
        GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_CELL,
            GetDocHandler(), GetMM100UnitConverter(), GetNamespaceMap());

        GetShapeExport()->exportAutoStyles();
        GetFormExport()->exportAutoStyles( );

        if (pDoc)
        {
            ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
            // #i100879# write the table style for cached tables only if there are cached tables
            // (same logic as in ExportExternalRefCacheStyles)
            if (pRefMgr->hasExternalData())
            {
                // Special table style for the external ref cache tables.
                AddAttribute(XML_NAMESPACE_STYLE, XML_NAME, sExternalRefTabStyleName);
                AddAttribute(XML_NAMESPACE_STYLE, XML_FAMILY, XML_TABLE);
                SvXMLElementExport aElemStyle(*this, XML_NAMESPACE_STYLE, XML_STYLE, true, true);
                AddAttribute(XML_NAMESPACE_TABLE,  XML_DISPLAY, XML_FALSE);
                SvXMLElementExport aElemStyleTabProps(*this, XML_NAMESPACE_STYLE, XML_TABLE_PROPERTIES, true, true);
            }
        }
    }

    if (getExportFlags() & EXPORT_MASTERSTYLES)
    {
        GetPageExport()->collectAutoStyles(true);
        GetPageExport()->exportAutoStyles();
    }

    // #i30251#; only write Text Styles once

    if ((getExportFlags() & EXPORT_CONTENT) || (getExportFlags() & EXPORT_MASTERSTYLES))
        GetTextParagraphExport()->exportTextAutoStyles();
}

void ScXMLExport::_ExportMasterStyles()
{
    GetPageExport()->exportMasterStyles( true );
}

void ScXMLExport::CollectInternalShape( uno::Reference< drawing::XShape > xShape )
{
    // detective objects and notes
    if( SvxShape* pShapeImp = SvxShape::getImplementation( xShape ) )
    {
        if( SdrObject* pObject = pShapeImp->GetSdrObject() )
        {
            // collect note caption objects from all layers (internal or hidden)
            if( ScDrawObjData* pCaptData = ScDrawLayer::GetNoteCaptionData( pObject, static_cast< SCTAB >( nCurrentTable ) ) )
            {
                if(pDoc->GetNotes(nCurrentTable)->findByAddress(pCaptData->maStart))
                {
                    pSharedData->AddNoteObj( xShape, pCaptData->maStart );

                    // #i60851# When the file is saved while editing a new note,
                    // the cell is still empty -> last column/row must be updated
                    OSL_ENSURE( pCaptData->maStart.Tab() == nCurrentTable, "invalid table in object data" );
                    pSharedData->SetLastColumn( nCurrentTable, pCaptData->maStart.Col() );
                    pSharedData->SetLastRow( nCurrentTable, pCaptData->maStart.Row() );
                }
            }
            // other objects from internal layer only (detective)
            else if( pObject->GetLayer() == SC_LAYER_INTERN )
            {
                ScDetectiveFunc aDetFunc( pDoc, static_cast<SCTAB>(nCurrentTable) );
                ScAddress       aPosition;
                ScRange         aSourceRange;
                bool            bRedLine;
                ScDetectiveObjType eObjType = aDetFunc.GetDetectiveObjectType(
                    pObject, nCurrentTable, aPosition, aSourceRange, bRedLine );
                pSharedData->GetDetectiveObjContainer()->AddObject( eObjType, static_cast<SCTAB>(nCurrentTable), aPosition, aSourceRange, bRedLine );
            }
        }
    }
}

bool ScXMLExport::GetMerged (const table::CellRangeAddress* pCellAddress,
                            const uno::Reference <sheet::XSpreadsheet>& xTable)
{
    bool bReady(false);
    sal_Int32 nRow(pCellAddress->StartRow);
    sal_Int32 nCol(pCellAddress->StartColumn);
    sal_Int32 nEndRow(pCellAddress->EndRow);
    sal_Int32 nEndCol(pCellAddress->EndColumn);
    bool bRowInc(nEndRow > nRow);
    while(!bReady && nRow <= nEndRow && nCol <= nEndCol)
    {
        uno::Reference<sheet::XSheetCellRange> xSheetCellRange(xTable->getCellRangeByPosition(nCol, nRow, nCol, nRow), uno::UNO_QUERY);
        if (xSheetCellRange.is())
        {
            uno::Reference<sheet::XSheetCellCursor> xCursor(xTable->createCursorByRange(xSheetCellRange));
            if(xCursor.is())
            {
                uno::Reference<sheet::XCellRangeAddressable> xCellAddress (xCursor, uno::UNO_QUERY);
                xCursor->collapseToMergedArea();
                table::CellRangeAddress aCellAddress2(xCellAddress->getRangeAddress());
                if ((aCellAddress2.EndRow > nRow ||
                    aCellAddress2.EndColumn > nCol) &&
                    aCellAddress2.StartRow == nRow &&
                    aCellAddress2.StartColumn == nCol)
                {
                    pMergedRangesContainer->AddRange(aCellAddress2);
                    pSharedData->SetLastColumn(aCellAddress2.Sheet, aCellAddress2.EndColumn);
                    pSharedData->SetLastRow(aCellAddress2.Sheet, aCellAddress2.EndRow);
                }
                else
                    bReady = true;
            }
        }
        if (!bReady)
        {
            if (bRowInc)
                ++nRow;
            else
                ++nCol;
        }
    }
    OSL_ENSURE(!(!bReady && nEndRow > nRow && nEndCol > nCol), "should not be possible");
    return !bReady;
}

bool ScXMLExport::IsMatrix (const ScAddress& aCell,
                            table::CellRangeAddress& aCellAddress, bool& bIsFirst) const
{
    bIsFirst = false;

    ScRange aMatrixRange;

    if (pDoc && pDoc->GetMatrixFormulaRange(aCell, aMatrixRange))
    {
        ScUnoConversion::FillApiRange( aCellAddress, aMatrixRange );
        if ((aCellAddress.StartColumn == aCell.Col() && aCellAddress.StartRow == aCell.Row()) &&
            (aCellAddress.EndColumn > aCell.Col() || aCellAddress.EndRow > aCell.Row()))
        {
            bIsFirst = true;
            return true;
        }
        else if (aCellAddress.StartColumn != aCell.Col() || aCellAddress.StartRow != aCell.Row() ||
            aCellAddress.EndColumn != aCell.Col() || aCellAddress.EndRow != aCell.Row())
            return true;
        else
        {
            bIsFirst = true;
            return true;
        }
    }

    return false;
}

void ScXMLExport::GetCellText (ScMyCell& rMyCell, const ScAddress& aPos) const
{
    if (!rMyCell.bHasStringValue)
    {
        rMyCell.sStringValue = ScCellObj::GetOutputString_Impl(pDoc, aPos);
        rMyCell.bHasStringValue = true;
    }
}

void ScXMLExport::WriteTable(sal_Int32 nTable, const Reference<sheet::XSpreadsheet>& xTable)
{
    if (!xTable.is())
        return;

    xCurrentTable.set(xTable);
    xCurrentTableCellRange.set(xTable, uno::UNO_QUERY);
    uno::Reference<container::XNamed> xName (xTable, uno::UNO_QUERY );
    if (!xName.is())
        return;

    nCurrentTable = sal::static_int_cast<sal_uInt16>( nTable );
    OUString sOUTableName(xName->getName());
    AddAttribute(sAttrName, sOUTableName);
    AddAttribute(sAttrStyleName, aTableStyles[nTable]);

    uno::Reference<util::XProtectable> xProtectable (xTable, uno::UNO_QUERY);
    ScTableProtection* pProtect = NULL;
    if (xProtectable.is() && xProtectable->isProtected())
    {
        AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTED, XML_TRUE);
        if (pDoc)
        {
            pProtect = pDoc->GetTabProtection(nTable);
            if (pProtect)
            {
                OUStringBuffer aBuffer;
                ScPasswordHash eHashUsed = PASSHASH_UNSPECIFIED;
                if (pProtect->hasPasswordHash(PASSHASH_SHA1))
                {
                    ::sax::Converter::encodeBase64(aBuffer,
                        pProtect->getPasswordHash(PASSHASH_SHA1));
                    eHashUsed = PASSHASH_SHA1;
                }
                else if (pProtect->hasPasswordHash(PASSHASH_XL, PASSHASH_SHA1))
                {
                    // Double-hash this by SHA1 on top of the legacy xls hash.
                    uno::Sequence<sal_Int8> aHash = pProtect->getPasswordHash(PASSHASH_XL, PASSHASH_SHA1);
                    ::sax::Converter::encodeBase64(aBuffer, aHash);
                    eHashUsed = PASSHASH_XL;
                }
                if (!aBuffer.isEmpty())
                {
                    AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY, aBuffer.makeStringAndClear());
                    if ( getDefaultVersion() >= SvtSaveOptions::ODFVER_012 )
                    {
                        if (eHashUsed == PASSHASH_XL)
                        {
                            AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY_DIGEST_ALGORITHM,
                                         ScPassHashHelper::getHashURI(PASSHASH_XL));
                            AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY_DIGEST_ALGORITHM_2,
                                         ScPassHashHelper::getHashURI(PASSHASH_SHA1));
                        }
                        else if (eHashUsed == PASSHASH_SHA1)
                            AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY_DIGEST_ALGORITHM,
                                         ScPassHashHelper::getHashURI(PASSHASH_SHA1));
                    }
                }
            }
        }
    }
    OUString sPrintRanges;
    table::CellRangeAddress aColumnHeaderRange;
    bool bHasColumnHeader;
    GetColumnRowHeader(bHasColumnHeader, aColumnHeaderRange, bHasRowHeader, aRowHeaderRange, sPrintRanges);
    if( !sPrintRanges.isEmpty() )
        AddAttribute( XML_NAMESPACE_TABLE, XML_PRINT_RANGES, sPrintRanges );
    else if (!pDoc->IsPrintEntireSheet(static_cast<SCTAB>(nTable)))
        AddAttribute( XML_NAMESPACE_TABLE, XML_PRINT, XML_FALSE);
    SvXMLElementExport aElemT(*this, sElemTab, true, true);

    if (pProtect && pProtect->isProtected())
    {
        if (pProtect->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS))
            AddAttribute(XML_NAMESPACE_TABLE, XML_SELECT_PROTECTED_CELLS, XML_TRUE);
        if (pProtect->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS))
            AddAttribute(XML_NAMESPACE_TABLE, XML_SELECT_UNPROTECTED_CELLS, XML_TRUE);

        OUString aElemName = GetNamespaceMap().GetQNameByKey(
            XML_NAMESPACE_TABLE, GetXMLToken(XML_TABLE_PROTECTION));

        SvXMLElementExport aElemProtected(*this, aElemName, true, true);
    }

    CheckAttrList();

    if ( pDoc && pDoc->GetSheetEvents( static_cast<SCTAB>(nTable) ) &&
         getDefaultVersion() >= SvtSaveOptions::ODFVER_012 )
    {
        // store sheet events
        uno::Reference<document::XEventsSupplier> xSupplier(xTable, uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xEvents(xSupplier->getEvents(), uno::UNO_QUERY);
        GetEventExport().ExportExt( xEvents );
    }

    WriteTableSource();
    WriteScenario();
    uno::Reference<drawing::XDrawPage> xDrawPage;
    if (pSharedData->HasForm(nTable, xDrawPage) && xDrawPage.is())
    {
        ::xmloff::OOfficeFormsExport aForms(*this);
        GetFormExport()->exportForms( xDrawPage );
        bool bRet(GetFormExport()->seekPage( xDrawPage ));
        OSL_ENSURE( bRet, "OFormLayerXMLExport::seekPage failed!" );
        (void)bRet;     // avoid warning in product version
    }
    if (pSharedData->HasDrawPage())
    {
        GetShapeExport()->seekShapes(uno::Reference<drawing::XShapes>(pSharedData->GetDrawPage(nTable), uno::UNO_QUERY));
        WriteTableShapes();
    }
    table::CellRangeAddress aRange(GetEndAddress(xTable, nTable));
    pSharedData->SetLastColumn(nTable, aRange.EndColumn);
    pSharedData->SetLastRow(nTable, aRange.EndRow);
    pCellsItr->SetCurrentTable(static_cast<SCTAB>(nTable), xCurrentTable);
    pGroupColumns->NewTable();
    pGroupRows->NewTable();
    FillColumnRowGroups();
    if (bHasColumnHeader)
        pSharedData->SetLastColumn(nTable, aColumnHeaderRange.EndColumn);
    bRowHeaderOpen = sal_False;
    if (bHasRowHeader)
        pSharedData->SetLastRow(nTable, aRowHeaderRange.EndRow);
    pDefaults->FillDefaultStyles(nTable, pSharedData->GetLastRow(nTable),
        pSharedData->GetLastColumn(nTable), pCellStyles, pDoc);
    pRowFormatRanges->SetRowDefaults(pDefaults->GetRowDefaults());
    pRowFormatRanges->SetColDefaults(pDefaults->GetColDefaults());
    pCellStyles->SetRowDefaults(pDefaults->GetRowDefaults());
    pCellStyles->SetColDefaults(pDefaults->GetColDefaults());
    ExportColumns(nTable, aColumnHeaderRange, bHasColumnHeader);
    bool bIsFirst(true);
    sal_Int32 nEqualCells(0);
    ScMyCell aCell;
    ScMyCell aPrevCell;
    while(pCellsItr->GetNext(aCell, pCellStyles))
    {
        if (bIsFirst)
        {
            ExportFormatRanges(0, 0, aCell.aCellAddress.Column - 1, aCell.aCellAddress.Row, nTable);
            aPrevCell = aCell;
            bIsFirst = sal_False;
        }
        else
        {
            if ((aPrevCell.aCellAddress.Row == aCell.aCellAddress.Row) &&
                (aPrevCell.aCellAddress.Column + nEqualCells + 1 == aCell.aCellAddress.Column))
            {
                if(IsCellEqual(aPrevCell, aCell))
                    ++nEqualCells;
                else
                {
                    WriteCell(aPrevCell, nEqualCells);
                    nEqualCells = 0;
                    aPrevCell = aCell;
                }
            }
            else
            {
                WriteCell(aPrevCell, nEqualCells);
                ExportFormatRanges(aPrevCell.aCellAddress.Column + nEqualCells + 1, aPrevCell.aCellAddress.Row,
                    aCell.aCellAddress.Column - 1, aCell.aCellAddress.Row, nTable);
                nEqualCells = 0;
                aPrevCell = aCell;
            }
        }
    }
    if (!bIsFirst)
    {
        WriteCell(aPrevCell, nEqualCells);
        ExportFormatRanges(aPrevCell.aCellAddress.Column + nEqualCells + 1, aPrevCell.aCellAddress.Row,
            pSharedData->GetLastColumn(nTable), pSharedData->GetLastRow(nTable), nTable);
    }
    else
        ExportFormatRanges(0, 0, pSharedData->GetLastColumn(nTable), pSharedData->GetLastRow(nTable), nTable);

    CloseRow(pSharedData->GetLastRow(nTable));
    nEqualCells = 0;

    if (pDoc)
    {
        // Export sheet-local named ranges.
        ScRangeName* pRangeName = pDoc->GetRangeName(nTable);
        if (pRangeName && !pRangeName->empty())
        {
            WriteNamedRange(pRangeName);
        }

        if(getDefaultVersion() > SvtSaveOptions::ODFVER_012)
        {
            //export new conditional format information
            ExportConditionalFormat(nTable);
        }

    }
}

void ScXMLExport::WriteCell(ScMyCell& aCell, sal_Int32 nEqualCellCount)
{
    // nEqualCellCount is the number of additional cells
    SetRepeatAttribute(nEqualCellCount, (aCell.nType != table::CellContentType_EMPTY));

    ScAddress aCellPos;
    ScUnoConversion::FillScAddress( aCellPos, aCell.aCellAddress );
    if (aCell.nStyleIndex != -1)
        AddAttribute(sAttrStyleName, *pCellStyles->GetStyleNameByIndex(aCell.nStyleIndex, aCell.bIsAutoStyle));
    if (aCell.nValidationIndex > -1)
        AddAttribute(XML_NAMESPACE_TABLE, XML_CONTENT_VALIDATION_NAME, pValidationsContainer->GetValidationName(aCell.nValidationIndex));
    bool bIsMatrix(aCell.bIsMatrixBase || aCell.bIsMatrixCovered);
    bool bIsFirstMatrixCell(aCell.bIsMatrixBase);
    if (bIsFirstMatrixCell)
    {
        sal_Int32 nColumns(aCell.aMatrixRange.EndColumn - aCell.aMatrixRange.StartColumn + 1);
        sal_Int32 nRows(aCell.aMatrixRange.EndRow - aCell.aMatrixRange.StartRow + 1);
        OUStringBuffer sColumns;
        OUStringBuffer sRows;
        ::sax::Converter::convertNumber(sColumns, nColumns);
        ::sax::Converter::convertNumber(sRows, nRows);
        AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_MATRIX_COLUMNS_SPANNED, sColumns.makeStringAndClear());
        AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_MATRIX_ROWS_SPANNED, sRows.makeStringAndClear());
    }
    bool bIsEmpty(false);
    switch (aCell.nType)
    {
        case table::CellContentType_EMPTY :
            {
                bIsEmpty = true;
            }
            break;
        case table::CellContentType_VALUE :
            {
                if (!aCell.bHasDoubleValue)
                {
                    aCell.fValue = pDoc->GetValue( aCellPos );
                    aCell.bHasDoubleValue = true;
                }
                GetNumberFormatAttributesExportHelper()->SetNumberFormatAttributes(
                    aCell.nNumberFormat, aCell.fValue);
                if( getDefaultVersion() > SvtSaveOptions::ODFVER_012 )
                    GetNumberFormatAttributesExportHelper()->SetNumberFormatAttributes(
                            aCell.nNumberFormat, aCell.fValue, false, XML_NAMESPACE_CALC_EXT, false);
            }
            break;
        case table::CellContentType_TEXT :
            {
                GetCellText(aCell, aCellPos);
                OUString sFormula(lcl_GetRawString(pDoc, aCellPos));
                GetNumberFormatAttributesExportHelper()->SetNumberFormatAttributes(
                        sFormula, aCell.sStringValue, true, true);
                if( getDefaultVersion() > SvtSaveOptions::ODFVER_012 )
                    GetNumberFormatAttributesExportHelper()->SetNumberFormatAttributes(
                            sFormula, aCell.sStringValue, false, true, XML_NAMESPACE_CALC_EXT);
            }
            break;
        case table::CellContentType_FORMULA :
            {
                ScRefCellValue aCellVal;
                aCellVal.assign(*pDoc, aCellPos);
                if (aCellVal.meType == CELLTYPE_FORMULA)
                {
                    OUStringBuffer sFormula;
                    ScFormulaCell* pFormulaCell = aCellVal.mpFormula;
                    if (!bIsMatrix || (bIsMatrix && bIsFirstMatrixCell))
                    {
                        const formula::FormulaGrammar::Grammar eGrammar = pDoc->GetStorageGrammar();
                        sal_uInt16 nNamespacePrefix = (eGrammar == formula::FormulaGrammar::GRAM_ODFF ? XML_NAMESPACE_OF : XML_NAMESPACE_OOOC);
                        pFormulaCell->GetFormula(sFormula, eGrammar);
                        OUString sOUFormula(sFormula.makeStringAndClear());
                        if (!bIsMatrix)
                        {
                            AddAttribute(sAttrFormula, GetNamespaceMap().GetQNameByKey( nNamespacePrefix, sOUFormula, false ));
                        }
                        else
                        {
                            AddAttribute(sAttrFormula, GetNamespaceMap().GetQNameByKey( nNamespacePrefix, sOUFormula.copy(1, sOUFormula.getLength() - 2), false ));
                        }
                    }
                    if (pFormulaCell->GetErrCode())
                    {
                        GetCellText(aCell, aCellPos);
                        AddAttribute(sAttrValueType, XML_STRING);
                        AddAttribute(sAttrStringValue, aCell.sStringValue);
                        if( getDefaultVersion() > SvtSaveOptions::ODFVER_012 )
                        {
                            //export calcext:value-type="error"
                            AddAttribute(XML_NAMESPACE_CALC_EXT,XML_VALUE_TYPE, OUString("error"));
                        }
                    }
                    else if (pFormulaCell->IsValue())
                    {
                        bool bIsStandard;
                        OUString sCurrency;
                        GetNumberFormatAttributesExportHelper()->GetCellType(aCell.nNumberFormat, sCurrency, bIsStandard);
                        if (pDoc)
                        {
                            GetNumberFormatAttributesExportHelper()->SetNumberFormatAttributes(
                                    aCell.nNumberFormat, pDoc->GetValue( aCellPos ));
                            if( getDefaultVersion() >= SvtSaveOptions::ODFVER_012 )
                            {
                                GetNumberFormatAttributesExportHelper()->SetNumberFormatAttributes(
                                        aCell.nNumberFormat, pDoc->GetValue( aCellPos ), false, XML_NAMESPACE_CALC_EXT, false );
                            }
                        }
                    }
                    else
                    {
                        GetCellText(aCell, aCellPos);
                        if (!aCell.sStringValue.isEmpty())
                        {
                            AddAttribute(sAttrValueType, XML_STRING);
                            AddAttribute(sAttrStringValue, aCell.sStringValue);
                            if( getDefaultVersion() > SvtSaveOptions::ODFVER_012 )
                            {
                                AddAttribute(XML_NAMESPACE_CALC_EXT,XML_VALUE_TYPE, XML_STRING);
                            }
                        }
                    }
                }
            }
            break;
        default:
            break;
    }
    OUString* pCellString(&sElemCell);
    if (aCell.bIsCovered)
    {
        pCellString = &sElemCoveredCell;
    }
    else
    {
        if (aCell.bIsMergedBase)
        {
            sal_Int32 nColumns(aCell.aMergeRange.EndColumn - aCell.aMergeRange.StartColumn + 1);
            sal_Int32 nRows(aCell.aMergeRange.EndRow - aCell.aMergeRange.StartRow + 1);
            OUStringBuffer sColumns;
            OUStringBuffer sRows;
            ::sax::Converter::convertNumber(sColumns, nColumns);
            ::sax::Converter::convertNumber(sRows, nRows);
            AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_SPANNED, sColumns.makeStringAndClear());
            AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_ROWS_SPANNED, sRows.makeStringAndClear());
        }
    }
    SvXMLElementExport aElemC(*this, *pCellString, true, true);
    CheckAttrList();
    WriteAreaLink(aCell);
    WriteAnnotation(aCell);
    WriteDetective(aCell);

    bool bEditCell = false;

    if (!bIsEmpty)
    {
        if ((aCell.nType == table::CellContentType_TEXT && IsEditCell(aCell)) ||
            (aCell.nType == table::CellContentType_FORMULA && IsMultiLineFormulaCell(aCell)))
        {
            bEditCell = true;
            uno::Reference<text::XText> xText(xCurrentTableCellRange->getCellByPosition(aCell.aCellAddress.Column, aCell.aCellAddress.Row), uno::UNO_QUERY);
            if ( xText.is())
                GetTextParagraphExport()->exportText(xText, false, false);
        }
        else
        {
            SvXMLElementExport aElemP(*this, sElemP, true, false);
            bool bPrevCharWasSpace(true);
            GetCellText(aCell, aCellPos);
            GetTextParagraphExport()->exportText(aCell.sStringValue, bPrevCharWasSpace);
        }
    }
    WriteShapes(aCell);
    if (!bIsEmpty)
        IncrementProgressBar(bEditCell);
}

void ScXMLExport::ExportShape(const uno::Reference < drawing::XShape >& xShape, awt::Point* pPoint)
{
    uno::Reference < beans::XPropertySet > xShapeProps ( xShape, uno::UNO_QUERY );
    bool bIsChart( false );
    OUString sPropCLSID ("CLSID");
    OUString sPropModel ("Model");
    OUString sPersistName ("PersistName");
    if (xShapeProps.is())
    {
        sal_Int32 nZOrder = 0;
        if (xShapeProps->getPropertyValue("ZOrder") >>= nZOrder)
        {
            OUStringBuffer sBuffer;
            ::sax::Converter::convertNumber(sBuffer, nZOrder);
            AddAttribute(XML_NAMESPACE_DRAW, XML_ZINDEX, sBuffer.makeStringAndClear());
        }
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo = xShapeProps->getPropertySetInfo();
        if( xPropSetInfo->hasPropertyByName( sPropCLSID ) )
        {
            OUString sCLSID;
            if (xShapeProps->getPropertyValue( sPropCLSID ) >>= sCLSID)
            {
                if ( sCLSID.equalsIgnoreAsciiCase(GetChartExport()->getChartCLSID()) )
                {
                    // we have a chart
                    OUString sRanges;
                    if ( pDoc )
                    {
                        OUString aChartName;
                        xShapeProps->getPropertyValue( sPersistName ) >>= aChartName;
                        ScChartListenerCollection* pCollection = pDoc->GetChartListenerCollection();
                        if (pCollection)
                        {
                            ScChartListener* pListener = pCollection->findByName(aChartName);
                            if (pListener)
                            {
                                const ScRangeListRef& rRangeList = pListener->GetRangeList();
                                if ( rRangeList.Is() )
                                {
                                    ScRangeStringConverter::GetStringFromRangeList( sRanges, rRangeList, pDoc, FormulaGrammar::CONV_OOO );
                                    if ( !sRanges.isEmpty() )
                                    {
                                        bIsChart = true;
                                        SvXMLAttributeList* pAttrList = new SvXMLAttributeList();
                                        if ( pAttrList )
                                        {
                                            pAttrList->AddAttribute(
                                                GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_DRAW, GetXMLToken( XML_NOTIFY_ON_UPDATE_OF_RANGES ) ), sRanges );
                                        }
                                        GetShapeExport()->exportShape( xShape, SEF_EXPORT_NO_CHART_DATA | SEF_DEFAULT, pPoint, pAttrList );
                                    }
                                }
                            }
                        }
                    }

                    if ( sRanges.isEmpty() )
                    {
                        uno::Reference< frame::XModel > xChartModel;
                        if( ( xShapeProps->getPropertyValue( sPropModel ) >>= xChartModel ) &&
                            xChartModel.is())
                        {
                            uno::Reference< chart2::XChartDocument > xChartDoc( xChartModel, uno::UNO_QUERY );
                            uno::Reference< chart2::data::XDataReceiver > xReceiver( xChartModel, uno::UNO_QUERY );
                            if( xChartDoc.is() && xReceiver.is() &&
                                ! xChartDoc->hasInternalDataProvider())
                            {
                                // we have a chart that gets its data from Calc
                                bIsChart = true;
                                uno::Sequence< OUString > aRepresentations(
                                    xReceiver->getUsedRangeRepresentations());
                                SvXMLAttributeList* pAttrList = 0;
                                if(aRepresentations.getLength())
                                {
                                    // add the ranges used by the chart to the shape
                                    // element to be able to start listening after
                                    // load (when the chart is not yet loaded)
                                    uno::Reference< chart2::data::XRangeXMLConversion > xRangeConverter( xChartDoc->getDataProvider(), uno::UNO_QUERY );
                                    sRanges = lcl_RangeSequenceToString( aRepresentations, xRangeConverter );
                                    pAttrList = new SvXMLAttributeList();
                                    pAttrList->AddAttribute(
                                        GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_DRAW, GetXMLToken(XML_NOTIFY_ON_UPDATE_OF_RANGES) ), sRanges );
                                }
                                GetShapeExport()->exportShape(xShape, SEF_EXPORT_NO_CHART_DATA | SEF_DEFAULT, pPoint, pAttrList);
                            }
                        }
                    }
                }
            }
        }
    }
    if (!bIsChart)
    {
        // #i66550 HLINK_FOR_SHAPES
        OUString sHlink;
        try
        {
            uno::Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
            if ( xProps.is() )
                xProps->getPropertyValue( OUString( SC_UNONAME_HYPERLINK ) ) >>= sHlink;
        }
        catch ( const beans::UnknownPropertyException& )
        {
            // no hyperlink property
        }

        std::auto_ptr< SvXMLElementExport > pDrawA;
        // enlose shapes with <draw:a> element only if sHlink contains something
        if ( !sHlink.isEmpty() )
        {
            // need to get delete the attributes that are pre-loaded
            // for the shape export ( otherwise they will become
            // attributes of the draw:a element ) This *shouldn't*
            // affect performance adversely as there are only a
            // couple of attributes involved
            uno::Reference< xml::sax::XAttributeList > xSaveAttribs( new  SvXMLAttributeList( GetAttrList() ) );
            ClearAttrList();
            // Add Hlink
            AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
            AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, sHlink);
            pDrawA.reset( new SvXMLElementExport( *this, XML_NAMESPACE_DRAW, XML_A, false, false ) );
            // Attribute list has been cleared by previous operation
            // re-add pre-loaded attributes
            AddAttributeList( xSaveAttribs );
        }
        GetShapeExport()->exportShape(xShape, SEF_DEFAULT, pPoint);
    }
    IncrementProgressBar(false);
}

void ScXMLExport::WriteShapes(const ScMyCell& rMyCell)
{
    if( rMyCell.bHasShape && !rMyCell.aShapeList.empty() && pDoc )
    {
        awt::Point aPoint;
        Rectangle aRect = pDoc->GetMMRect(static_cast<SCCOL>(rMyCell.aCellAddress.Column), static_cast<SCROW>(rMyCell.aCellAddress.Row),
            static_cast<SCCOL>(rMyCell.aCellAddress.Column), static_cast<SCROW>(rMyCell.aCellAddress.Row), static_cast<SCTAB>(rMyCell.aCellAddress.Sheet));
        bool bNegativePage(pDoc->IsNegativePage(rMyCell.aCellAddress.Sheet));
        if (bNegativePage)
            aPoint.X = aRect.Right();
        else
            aPoint.X = aRect.Left();
        aPoint.Y = aRect.Top();
        ScMyShapeList::const_iterator aItr = rMyCell.aShapeList.begin();
        ScMyShapeList::const_iterator aEndItr(rMyCell.aShapeList.end());
        while (aItr != aEndItr)
        {
            if (aItr->xShape.is())
            {
                if (bNegativePage)
                    aPoint.X = 2 * aItr->xShape->getPosition().X + aItr->xShape->getSize().Width - aPoint.X;
                if ( !aItr->xShape->getShapeType().equals(sCaptionShape) )
                {
                    OUString sEndAddress;
                    ScRangeStringConverter::GetStringFromAddress(sEndAddress, aItr->aEndAddress, pDoc, FormulaGrammar::CONV_OOO);
                    AddAttribute(XML_NAMESPACE_TABLE, XML_END_CELL_ADDRESS, sEndAddress);
                    OUStringBuffer sBuffer;
                    GetMM100UnitConverter().convertMeasureToXML(
                            sBuffer, aItr->nEndX);
                    AddAttribute(XML_NAMESPACE_TABLE, XML_END_X, sBuffer.makeStringAndClear());
                    GetMM100UnitConverter().convertMeasureToXML(
                            sBuffer, aItr->nEndY);
                    AddAttribute(XML_NAMESPACE_TABLE, XML_END_Y, sBuffer.makeStringAndClear());
                }
                ExportShape(aItr->xShape, &aPoint);
            }
            ++aItr;
        }
    }
}

void ScXMLExport::WriteTableShapes()
{
    ScMyTableShapes* pTableShapes(pSharedData->GetTableShapes());
    if (pTableShapes && !(*pTableShapes)[nCurrentTable].empty())
    {
        OSL_ENSURE(pTableShapes->size() > static_cast<size_t>(nCurrentTable), "wrong Table");
        SvXMLElementExport aShapesElem(*this, XML_NAMESPACE_TABLE, XML_SHAPES, true, false);
        ScMyTableXShapes::iterator aItr((*pTableShapes)[nCurrentTable].begin());
        ScMyTableXShapes::iterator aEndItr((*pTableShapes)[nCurrentTable].end());
        while (aItr != aEndItr)
        {
            if (aItr->is())
            {
                if (pDoc->IsNegativePage(static_cast<SCTAB>(nCurrentTable)))
                {
                    awt::Point aPoint((*aItr)->getPosition());
                    awt::Size aSize((*aItr)->getSize());
                    aPoint.X += aPoint.X + aSize.Width;
                    aPoint.Y = 0;
                    ExportShape(*aItr, &aPoint);
                }
                else
                    ExportShape(*aItr, NULL);
            }
            aItr = (*pTableShapes)[nCurrentTable].erase(aItr);
        }
    }
}

void ScXMLExport::WriteAreaLink( const ScMyCell& rMyCell )
{
    if( rMyCell.bHasAreaLink )
    {
        const ScMyAreaLink& rAreaLink = rMyCell.aAreaLink;
        AddAttribute( XML_NAMESPACE_TABLE, XML_NAME, rAreaLink.sSourceStr );
        AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, GetRelativeReference(rAreaLink.sURL) );
        AddAttribute( XML_NAMESPACE_TABLE, XML_FILTER_NAME, rAreaLink.sFilter );
        if( !rAreaLink.sFilterOptions.isEmpty() )
            AddAttribute( XML_NAMESPACE_TABLE, XML_FILTER_OPTIONS, rAreaLink.sFilterOptions );
        OUStringBuffer sValue;
        ::sax::Converter::convertNumber( sValue, rAreaLink.GetColCount() );
        AddAttribute( XML_NAMESPACE_TABLE, XML_LAST_COLUMN_SPANNED, sValue.makeStringAndClear() );
        ::sax::Converter::convertNumber( sValue, rAreaLink.GetRowCount() );
        AddAttribute( XML_NAMESPACE_TABLE, XML_LAST_ROW_SPANNED, sValue.makeStringAndClear() );
        if( rAreaLink.nRefresh )
        {
            ::sax::Converter::convertDuration( sValue,
                    (double)rAreaLink.nRefresh / 86400 );
            AddAttribute( XML_NAMESPACE_TABLE, XML_REFRESH_DELAY, sValue.makeStringAndClear() );
        }
        SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE, XML_CELL_RANGE_SOURCE, true, true );
    }
}

void ScXMLExport::exportAnnotationMeta( const uno::Reference < drawing::XShape >& xShape)
{
    if (pCurrentCell && pCurrentCell->xNoteShape.is() && pCurrentCell->xNoteShape.get() == xShape.get() && pCurrentCell->xAnnotation.is())
    {
        OUString sAuthor(pCurrentCell->xAnnotation->getAuthor());
        if (!sAuthor.isEmpty())
        {
            SvXMLElementExport aCreatorElem( *this, XML_NAMESPACE_DC,
                                                XML_CREATOR, true,
                                                false );
            Characters(sAuthor);
        }

        String aDate(pCurrentCell->xAnnotation->getDate());
        if (pDoc)
        {
            SvNumberFormatter* pNumForm = pDoc->GetFormatTable();
            double fDate;
            sal_uInt32 nfIndex = pNumForm->GetFormatIndex(NF_DATE_SYS_DDMMYYYY, LANGUAGE_SYSTEM);
            if (pNumForm->IsNumberFormat(aDate, nfIndex, fDate))
            {
                OUStringBuffer sBuf;
                GetMM100UnitConverter().convertDateTime(sBuf, fDate,true);
                SvXMLElementExport aDateElem( *this, XML_NAMESPACE_DC,
                                                XML_DATE, true,
                                                false );
                Characters(sBuf.makeStringAndClear());
            }
            else
            {
                SvXMLElementExport aDateElem( *this, XML_NAMESPACE_META,
                                                XML_DATE_STRING, true,
                                                false );
                Characters(OUString(aDate));
            }
        }
        else
        {
            SvXMLElementExport aDateElem( *this, XML_NAMESPACE_META,
                                            XML_DATE_STRING, true,
                                            false );
            Characters(OUString(aDate));
        }
    }
}

void ScXMLExport::WriteAnnotation(ScMyCell& rMyCell)
{
    if( rMyCell.bHasAnnotation && rMyCell.xAnnotation.is())
    {

        if (rMyCell.xAnnotation->getIsVisible())
            AddAttribute(XML_NAMESPACE_OFFICE, XML_DISPLAY, XML_TRUE);

        pCurrentCell = &rMyCell;

        if(rMyCell.xNoteShape.is())
            GetShapeExport()->exportShape(rMyCell.xNoteShape, SEF_DEFAULT|SEF_EXPORT_ANNOTATION, NULL);

        pCurrentCell = NULL;

        rMyCell.xNoteShape.clear();
    }
}

void ScXMLExport::WriteDetective( const ScMyCell& rMyCell )
{
    if( rMyCell.bHasDetectiveObj || rMyCell.bHasDetectiveOp )
    {
        const ScMyDetectiveObjVec& rObjVec = rMyCell.aDetectiveObjVec;
        const ScMyDetectiveOpVec& rOpVec = rMyCell.aDetectiveOpVec;
        sal_Int32 nObjCount(rObjVec.size());
        sal_Int32 nOpCount(rOpVec.size());
        if( nObjCount || nOpCount )
        {
            SvXMLElementExport aDetElem( *this, XML_NAMESPACE_TABLE, XML_DETECTIVE, true, true );
            OUString sString;
            ScMyDetectiveObjVec::const_iterator aObjItr(rObjVec.begin());
            ScMyDetectiveObjVec::const_iterator aEndObjItr(rObjVec.end());
            while(aObjItr != aEndObjItr)
            {
                if (aObjItr->eObjType != SC_DETOBJ_CIRCLE)
                {
                    if( (aObjItr->eObjType == SC_DETOBJ_ARROW) || (aObjItr->eObjType == SC_DETOBJ_TOOTHERTAB))
                    {
                        ScRangeStringConverter::GetStringFromRange( sString, aObjItr->aSourceRange, pDoc, FormulaGrammar::CONV_OOO );
                        AddAttribute( XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, sString );
                    }
                    ScXMLConverter::GetStringFromDetObjType( sString, aObjItr->eObjType );
                    AddAttribute( XML_NAMESPACE_TABLE, XML_DIRECTION, sString );
                    if( aObjItr->bHasError )
                        AddAttribute( XML_NAMESPACE_TABLE, XML_CONTAINS_ERROR, XML_TRUE );
                }
                else
                    AddAttribute( XML_NAMESPACE_TABLE, XML_MARKED_INVALID, XML_TRUE );
                SvXMLElementExport aRangeElem( *this, XML_NAMESPACE_TABLE, XML_HIGHLIGHTED_RANGE, true, true );
                ++aObjItr;
            }
            OUStringBuffer aBuffer;
            ScMyDetectiveOpVec::const_iterator aOpItr(rOpVec.begin());
            ScMyDetectiveOpVec::const_iterator aEndOpItr(rOpVec.end());
            while(aOpItr != aEndOpItr)
            {
                OUString sOpString;
                ScXMLConverter::GetStringFromDetOpType( sOpString, aOpItr->eOpType );
                AddAttribute( XML_NAMESPACE_TABLE, XML_NAME, sOpString );
                ::sax::Converter::convertNumber( aBuffer, aOpItr->nIndex );
                AddAttribute( XML_NAMESPACE_TABLE, XML_INDEX, aBuffer.makeStringAndClear() );
                SvXMLElementExport aRangeElem( *this, XML_NAMESPACE_TABLE, XML_OPERATION, true, true );
                ++aOpItr;
            }
        }
    }
}

void ScXMLExport::SetRepeatAttribute(sal_Int32 nEqualCellCount, bool bIncProgress)
{
    // nEqualCellCount is additional cells, so the attribute value is nEqualCellCount+1
    if (nEqualCellCount > 0)
    {
        sal_Int32 nTemp(nEqualCellCount + 1);
        OUString sOUEqualCellCount(OUString::number(nTemp));
        AddAttribute(sAttrColumnsRepeated, sOUEqualCellCount);
        if (bIncProgress)
            IncrementProgressBar(sal_False, nEqualCellCount);
    }
}

bool ScXMLExport::IsCellTypeEqual (const ScMyCell& aCell1, const ScMyCell& aCell2) const
{
    return (aCell1.nType == aCell2.nType);
}

bool ScXMLExport::IsEditCell(const com::sun::star::table::CellAddress& aAddress, ScMyCell* pMyCell) const
{
    ScAddress aCoreAddress(static_cast<SCCOL>(aAddress.Column),
                        static_cast<SCROW>(aAddress.Row),
                        static_cast<SCTAB>(aAddress.Sheet));

    ScRefCellValue aCell;
    aCell.assign(const_cast<ScDocument&>(*GetDocument()), aCoreAddress);

    if (pMyCell)
        pMyCell->maBaseCell = aCell;

    return (aCell.meType == CELLTYPE_EDIT);
}

bool ScXMLExport::IsEditCell(ScMyCell& rCell) const
{
    if (rCell.bKnowWhetherIsEditCell)
        return rCell.bIsEditCell;
    else
    {
         rCell.bIsEditCell = IsEditCell(rCell.aCellAddress, &rCell);
        rCell.bKnowWhetherIsEditCell = true;
        return rCell.bIsEditCell;
    }
}

bool ScXMLExport::IsMultiLineFormulaCell(ScMyCell& rCell) const
{
    if (!rCell.maBaseCell.isEmpty())
    {
        if (rCell.maBaseCell.meType != CELLTYPE_FORMULA)
            return false;

        return rCell.maBaseCell.mpFormula->IsMultilineResult();
    }

    ScAddress aAddr(static_cast<SCCOL>(rCell.aCellAddress.Column),
                    static_cast<SCROW>(rCell.aCellAddress.Row),
                    static_cast<SCTAB>(rCell.aCellAddress.Sheet));

    rCell.maBaseCell.assign(*pDoc, aAddr);
    if (rCell.maBaseCell.meType != CELLTYPE_FORMULA)
        return false;

    return rCell.maBaseCell.mpFormula->IsMultilineResult();
}

bool ScXMLExport::IsCellEqual (ScMyCell& aCell1, ScMyCell& aCell2)
{
    ScAddress aCellPos1;
    ScUnoConversion::FillScAddress( aCellPos1, aCell1.aCellAddress );
    ScAddress aCellPos2;
    ScUnoConversion::FillScAddress( aCellPos2, aCell2.aCellAddress );
    bool bIsEqual = false;
    if( !aCell1.bIsMergedBase && !aCell2.bIsMergedBase &&
        aCell1.bIsCovered == aCell2.bIsCovered &&
        !aCell1.bIsMatrixBase && !aCell2.bIsMatrixBase &&
        aCell1.bIsMatrixCovered == aCell2.bIsMatrixCovered &&
        aCell1.bHasAnnotation == aCell2.bHasAnnotation &&
        !aCell1.bHasShape && !aCell2.bHasShape &&
        aCell1.bHasAreaLink == aCell2.bHasAreaLink &&
        !aCell1.bHasDetectiveObj && !aCell2.bHasDetectiveObj)
    {
        if( (aCell1.bHasAreaLink &&
            (aCell1.aAreaLink.GetColCount() == 1) &&
            (aCell2.aAreaLink.GetColCount() == 1) &&
            aCell1.aAreaLink.Compare( aCell2.aAreaLink ) ) ||
            !aCell1.bHasAreaLink )
        {
            if (!aCell1.bHasAnnotation || (aCell1.bHasAnnotation && false/*IsAnnotationEqual(aCell1.xCell, aCell2.xCell)*/)) // no longer compareable
            {
                if ((((aCell1.nStyleIndex == aCell2.nStyleIndex) && (aCell1.bIsAutoStyle == aCell2.bIsAutoStyle)) ||
                     ((aCell1.nStyleIndex == aCell2.nStyleIndex) && (aCell1.nStyleIndex == -1))) &&
                    (aCell1.nValidationIndex == aCell2.nValidationIndex) &&
                    IsCellTypeEqual(aCell1, aCell2))
                {
                    switch ( aCell1.nType )
                    {
                    case table::CellContentType_EMPTY :
                        {
                            bIsEqual = true;
                        }
                        break;
                    case table::CellContentType_VALUE :
                        {
                            if(!aCell1.bHasDoubleValue)
                            {
                                aCell1.fValue = pDoc->GetValue( aCellPos1 );
                                aCell1.bHasDoubleValue = true;
                            }
                            if (!aCell2.bHasDoubleValue)
                            {
                                aCell2.fValue = pDoc->GetValue( aCellPos2 );
                                aCell2.bHasDoubleValue = true;
                            }
                            // #i29101# number format may be different from column default styles,
                            // but can lead to different value types, so it must also be compared
                            bIsEqual = (aCell1.nNumberFormat == aCell2.nNumberFormat) &&
                                       (aCell1.fValue == aCell2.fValue);
                        }
                        break;
                    case table::CellContentType_TEXT :
                        {
                            if (IsEditCell(aCell1) || IsEditCell(aCell2))
                                bIsEqual = false;
                            else
                            {
                                GetCellText(aCell1, aCellPos1);
                                GetCellText(aCell2, aCellPos2);
                                bIsEqual = (aCell1.sStringValue == aCell2.sStringValue) &&
                                    (lcl_GetRawString(pDoc, aCellPos1) == lcl_GetRawString(pDoc, aCellPos2));
                            }
                        }
                        break;
                    case table::CellContentType_FORMULA :
                        {
                            bIsEqual = false;
                        }
                        break;
                    default :
                        {
                            bIsEqual = false;
                        }
                        break;
                    }
                }
            }
        }
    }
    return bIsEqual;
}

void ScXMLExport::WriteCalculationSettings(const uno::Reference <sheet::XSpreadsheetDocument>& xSpreadDoc)
{
    uno::Reference<beans::XPropertySet> xPropertySet(xSpreadDoc, uno::UNO_QUERY);
    if (xPropertySet.is())
    {
        bool bCalcAsShown (::cppu::any2bool( xPropertySet->getPropertyValue(OUString(SC_UNO_CALCASSHOWN)) ));
        bool bIgnoreCase (::cppu::any2bool( xPropertySet->getPropertyValue(OUString(SC_UNO_IGNORECASE)) ));
        bool bLookUpLabels (::cppu::any2bool( xPropertySet->getPropertyValue(OUString(SC_UNO_LOOKUPLABELS)) ));
        bool bMatchWholeCell (::cppu::any2bool( xPropertySet->getPropertyValue(OUString(SC_UNO_MATCHWHOLE)) ));
        bool bUseRegularExpressions (::cppu::any2bool( xPropertySet->getPropertyValue(OUString(SC_UNO_REGEXENABLED)) ));
        bool bIsIterationEnabled (::cppu::any2bool( xPropertySet->getPropertyValue(OUString(SC_UNO_ITERENABLED)) ));
        sal_uInt16 nYear2000 (pDoc ? pDoc->GetDocOptions().GetYear2000() : 0);
        sal_Int32 nIterationCount(100);
        xPropertySet->getPropertyValue( OUString(SC_UNO_ITERCOUNT)) >>= nIterationCount;
        double fIterationEpsilon = 0;
        xPropertySet->getPropertyValue( OUString(SC_UNO_ITEREPSILON)) >>= fIterationEpsilon;
        util::Date aNullDate;
        xPropertySet->getPropertyValue( OUString(SC_UNO_NULLDATE)) >>= aNullDate;
        if (bCalcAsShown || bIgnoreCase || !bLookUpLabels || !bMatchWholeCell || !bUseRegularExpressions ||
            bIsIterationEnabled || nIterationCount != 100 || !::rtl::math::approxEqual(fIterationEpsilon, 0.001) ||
            aNullDate.Day != 30 || aNullDate.Month != 12 || aNullDate.Year != 1899 || nYear2000 != 1930)
        {
            if (bIgnoreCase)
                AddAttribute(XML_NAMESPACE_TABLE, XML_CASE_SENSITIVE, XML_FALSE);
            if (bCalcAsShown)
                AddAttribute(XML_NAMESPACE_TABLE, XML_PRECISION_AS_SHOWN, XML_TRUE);
            if (!bMatchWholeCell)
                AddAttribute(XML_NAMESPACE_TABLE, XML_SEARCH_CRITERIA_MUST_APPLY_TO_WHOLE_CELL, XML_FALSE);
            if (!bLookUpLabels)
                AddAttribute(XML_NAMESPACE_TABLE, XML_AUTOMATIC_FIND_LABELS, XML_FALSE);
            if (!bUseRegularExpressions)
                AddAttribute(XML_NAMESPACE_TABLE, XML_USE_REGULAR_EXPRESSIONS, XML_FALSE);
            if (nYear2000 != 1930)
            {
                OUStringBuffer sBuffer;
                ::sax::Converter::convertNumber(sBuffer, nYear2000);
                AddAttribute(XML_NAMESPACE_TABLE, XML_NULL_YEAR, sBuffer.makeStringAndClear());
            }
            SvXMLElementExport aCalcSettings(*this, XML_NAMESPACE_TABLE, XML_CALCULATION_SETTINGS, true, true);
            {
                if (aNullDate.Day != 30 || aNullDate.Month != 12 || aNullDate.Year != 1899)
                {
                    OUStringBuffer sDate;
                    GetMM100UnitConverter().convertDateTime(sDate, 0.0, aNullDate);
                    AddAttribute(XML_NAMESPACE_TABLE, XML_DATE_VALUE, sDate.makeStringAndClear());
                    SvXMLElementExport aElemNullDate(*this, XML_NAMESPACE_TABLE, XML_NULL_DATE, true, true);
                }
                if (bIsIterationEnabled || nIterationCount != 100 || !::rtl::math::approxEqual(fIterationEpsilon, 0.001))
                {
                    OUStringBuffer sBuffer;
                    if (bIsIterationEnabled)
                        AddAttribute(XML_NAMESPACE_TABLE, XML_STATUS, XML_ENABLE);
                    if (nIterationCount != 100)
                    {
                        ::sax::Converter::convertNumber(sBuffer,
                                nIterationCount);
                        AddAttribute(XML_NAMESPACE_TABLE, XML_STEPS, sBuffer.makeStringAndClear());
                    }
                    if (!::rtl::math::approxEqual(fIterationEpsilon, 0.001))
                    {
                        ::sax::Converter::convertDouble(sBuffer,
                                fIterationEpsilon);
                        AddAttribute(XML_NAMESPACE_TABLE, XML_MAXIMUM_DIFFERENCE, sBuffer.makeStringAndClear());
                    }
                    SvXMLElementExport aElemIteration(*this, XML_NAMESPACE_TABLE, XML_ITERATION, true, true);
                }
            }
        }
    }
}

void ScXMLExport::WriteTableSource()
{
    uno::Reference <sheet::XSheetLinkable> xLinkable (xCurrentTable, uno::UNO_QUERY);
    if (xLinkable.is() && GetModel().is())
    {
        sheet::SheetLinkMode nMode (xLinkable->getLinkMode());
        if (nMode != sheet::SheetLinkMode_NONE)
        {
            OUString sLink (xLinkable->getLinkUrl());
            uno::Reference <beans::XPropertySet> xProps (GetModel(), uno::UNO_QUERY);
            if (xProps.is())
            {
                uno::Reference <container::XIndexAccess> xIndex(xProps->getPropertyValue(OUString(SC_UNO_SHEETLINKS)), uno::UNO_QUERY);
                if (xIndex.is())
                {
                    sal_Int32 nCount(xIndex->getCount());
                    if (nCount)
                    {
                        bool bFound(false);
                        uno::Reference <beans::XPropertySet> xLinkProps;
                        for (sal_Int32 i = 0; (i < nCount) && !bFound; ++i)
                        {
                            xLinkProps.set(xIndex->getByIndex(i), uno::UNO_QUERY);
                            if (xLinkProps.is())
                            {
                                OUString sNewLink;
                                if (xLinkProps->getPropertyValue(OUString(SC_UNONAME_LINKURL)) >>= sNewLink)
                                    bFound = sLink.equals(sNewLink);
                            }
                        }
                        if (bFound && xLinkProps.is())
                        {
                            OUString sFilter;
                            OUString sFilterOptions;
                            OUString sTableName (xLinkable->getLinkSheetName());
                            sal_Int32 nRefresh(0);
                            xLinkProps->getPropertyValue(OUString(SC_UNONAME_FILTER)) >>= sFilter;
                            xLinkProps->getPropertyValue(OUString(SC_UNONAME_FILTOPT)) >>= sFilterOptions;
                            xLinkProps->getPropertyValue(OUString(SC_UNONAME_REFDELAY)) >>= nRefresh;
                            if (!sLink.isEmpty())
                            {
                                AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE);
                                AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, GetRelativeReference(sLink));
                                if (!sTableName.isEmpty())
                                    AddAttribute(XML_NAMESPACE_TABLE, XML_TABLE_NAME, sTableName);
                                if (!sFilter.isEmpty())
                                    AddAttribute(XML_NAMESPACE_TABLE, XML_FILTER_NAME, sFilter);
                                if (!sFilterOptions.isEmpty())
                                    AddAttribute(XML_NAMESPACE_TABLE, XML_FILTER_OPTIONS, sFilterOptions);
                                if (nMode != sheet::SheetLinkMode_NORMAL)
                                    AddAttribute(XML_NAMESPACE_TABLE, XML_MODE, XML_COPY_RESULTS_ONLY);
                                if( nRefresh )
                                {
                                    OUStringBuffer sBuffer;
                                    ::sax::Converter::convertDuration( sBuffer,
                                            (double)nRefresh / 86400 );
                                    AddAttribute( XML_NAMESPACE_TABLE, XML_REFRESH_DELAY, sBuffer.makeStringAndClear() );
                                }
                                SvXMLElementExport aSourceElem(*this, XML_NAMESPACE_TABLE, XML_TABLE_SOURCE, true, true);
                            }
                        }
                    }
                }
            }
        }
    }
}

// core implementation
void ScXMLExport::WriteScenario()
{
    if (pDoc && pDoc->IsScenario(static_cast<SCTAB>(nCurrentTable)))
    {
        OUString sComment;
        Color       aColor;
        sal_uInt16  nFlags;
        pDoc->GetScenarioData(static_cast<SCTAB>(nCurrentTable), sComment, aColor, nFlags);
        if (!(nFlags & SC_SCENARIO_SHOWFRAME))
            AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_BORDER, XML_FALSE);
        OUStringBuffer aBuffer;
        ::sax::Converter::convertColor(aBuffer, aColor.GetColor());
        AddAttribute(XML_NAMESPACE_TABLE, XML_BORDER_COLOR, aBuffer.makeStringAndClear());
        if (!(nFlags & SC_SCENARIO_TWOWAY))
            AddAttribute(XML_NAMESPACE_TABLE, XML_COPY_BACK, XML_FALSE);
        if (!(nFlags & SC_SCENARIO_ATTRIB))
            AddAttribute(XML_NAMESPACE_TABLE, XML_COPY_STYLES, XML_FALSE);
        if (nFlags & SC_SCENARIO_VALUE)
            AddAttribute(XML_NAMESPACE_TABLE, XML_COPY_FORMULAS, XML_FALSE);
        if (nFlags & SC_SCENARIO_PROTECT)
            AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTED, XML_TRUE);
        ::sax::Converter::convertBool(aBuffer,
                pDoc->IsActiveScenario(static_cast<SCTAB>(nCurrentTable)));
        AddAttribute(XML_NAMESPACE_TABLE, XML_IS_ACTIVE, aBuffer.makeStringAndClear());
        const ScRangeList* pRangeList = pDoc->GetScenarioRanges(static_cast<SCTAB>(nCurrentTable));
        OUString sRangeListStr;
        ScRangeStringConverter::GetStringFromRangeList( sRangeListStr, pRangeList, pDoc, FormulaGrammar::CONV_OOO );
        AddAttribute(XML_NAMESPACE_TABLE, XML_SCENARIO_RANGES, sRangeListStr);
        if (!sComment.isEmpty())
            AddAttribute(XML_NAMESPACE_TABLE, XML_COMMENT, sComment);
        SvXMLElementExport aElem(*this, XML_NAMESPACE_TABLE, XML_SCENARIO, sal_True, sal_True);
    }
}

void ScXMLExport::WriteTheLabelRanges( const uno::Reference< sheet::XSpreadsheetDocument >& xSpreadDoc )
{
    uno::Reference< beans::XPropertySet > xDocProp( xSpreadDoc, uno::UNO_QUERY );
    if( !xDocProp.is() ) return;

    sal_Int32 nCount(0);
    uno::Reference< container::XIndexAccess > xColRangesIAccess(xDocProp->getPropertyValue( OUString( SC_UNO_COLLABELRNG ) ), uno::UNO_QUERY);
    if( xColRangesIAccess.is() )
        nCount += xColRangesIAccess->getCount();

    uno::Reference< container::XIndexAccess > xRowRangesIAccess(xDocProp->getPropertyValue( OUString( SC_UNO_ROWLABELRNG ) ), uno::UNO_QUERY);
    if( xRowRangesIAccess.is() )
        nCount += xRowRangesIAccess->getCount();

    if( nCount )
    {
        SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE, XML_LABEL_RANGES, true, true );
        WriteLabelRanges( xColRangesIAccess, true );
        WriteLabelRanges( xRowRangesIAccess, false );
    }
}

void ScXMLExport::WriteLabelRanges( const uno::Reference< container::XIndexAccess >& xRangesIAccess, bool bColumn )
{
    if( !xRangesIAccess.is() ) return;

    sal_Int32 nCount(xRangesIAccess->getCount());
    for( sal_Int32 nIndex = 0; nIndex < nCount; ++nIndex )
    {
        uno::Reference< sheet::XLabelRange > xRange(xRangesIAccess->getByIndex( nIndex ), uno::UNO_QUERY);
        if( xRange.is() )
        {
            OUString sRangeStr;
            table::CellRangeAddress aCellRange( xRange->getLabelArea() );
            ScRangeStringConverter::GetStringFromRange( sRangeStr, aCellRange, pDoc, FormulaGrammar::CONV_OOO );
            AddAttribute( XML_NAMESPACE_TABLE, XML_LABEL_CELL_RANGE_ADDRESS, sRangeStr );
            aCellRange = xRange->getDataArea();
            ScRangeStringConverter::GetStringFromRange( sRangeStr, aCellRange, pDoc, FormulaGrammar::CONV_OOO );
            AddAttribute( XML_NAMESPACE_TABLE, XML_DATA_CELL_RANGE_ADDRESS, sRangeStr );
            AddAttribute( XML_NAMESPACE_TABLE, XML_ORIENTATION, bColumn ? XML_COLUMN : XML_ROW );
            SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE, XML_LABEL_RANGE, true, true );
        }
    }
}

void ScXMLExport::WriteNamedExpressions()
{
    if (!pDoc)
        return;
    ScRangeName* pNamedRanges = pDoc->GetRangeName();
    WriteNamedRange(pNamedRanges);
}

void ScXMLExport::WriteNamedRange(ScRangeName* pRangeName)
{
    //write a global or local ScRangeName
    SvXMLElementExport aElemNEs(*this, XML_NAMESPACE_TABLE, XML_NAMED_EXPRESSIONS, true, true);
    for (ScRangeName::iterator it = pRangeName->begin(); it != pRangeName->end(); ++it)
    {
        AddAttribute(sAttrName, it->second->GetName());

        OUString sBaseCellAddress;
        it->second->ValidateTabRefs();
        ScRangeStringConverter::GetStringFromAddress( sBaseCellAddress, it->second->GetPos(), pDoc,
                            FormulaGrammar::CONV_OOO, ' ', false, SCA_ABS_3D);
        AddAttribute(XML_NAMESPACE_TABLE, XML_BASE_CELL_ADDRESS, sBaseCellAddress);

        String sSymbol;
        it->second->GetSymbol(sSymbol, pDoc->GetStorageGrammar());
        OUString sTempSymbol(sSymbol);
        ScRange aRange;
        if (it->second->IsReference(aRange))
        {

            OUString sContent(sTempSymbol.copy(1, sTempSymbol.getLength() -2 ));
            AddAttribute(XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, sContent);

            sal_Int32 nRangeType = it->second->GetUnoType();
            OUStringBuffer sBufferRangeType;
            if ((nRangeType & sheet::NamedRangeFlag::COLUMN_HEADER) == sheet::NamedRangeFlag::COLUMN_HEADER)
                sBufferRangeType.append(GetXMLToken(XML_REPEAT_COLUMN));
            if ((nRangeType & sheet::NamedRangeFlag::ROW_HEADER) == sheet::NamedRangeFlag::ROW_HEADER)
            {
                if (!sBufferRangeType.isEmpty())
                    sBufferRangeType.appendAscii(" ");
                sBufferRangeType.append(GetXMLToken(XML_REPEAT_ROW));
            }
            if ((nRangeType & sheet::NamedRangeFlag::FILTER_CRITERIA) == sheet::NamedRangeFlag::FILTER_CRITERIA)
            {
                if (!sBufferRangeType.isEmpty())
                    sBufferRangeType.appendAscii(" ");
                sBufferRangeType.append(GetXMLToken(XML_FILTER));
            }
            if ((nRangeType & sheet::NamedRangeFlag::PRINT_AREA) == sheet::NamedRangeFlag::PRINT_AREA)
            {
                if (!sBufferRangeType.isEmpty())
                    sBufferRangeType.appendAscii(" ");
                sBufferRangeType.append(GetXMLToken(XML_PRINT_RANGE));
            }
            OUString sRangeType = sBufferRangeType.makeStringAndClear();
            if (!sRangeType.isEmpty())
                AddAttribute(XML_NAMESPACE_TABLE, XML_RANGE_USABLE_AS, sRangeType);
            SvXMLElementExport aElemNR(*this, XML_NAMESPACE_TABLE, XML_NAMED_RANGE, true, true);

        }
        else
        {
            AddAttribute(XML_NAMESPACE_TABLE, XML_EXPRESSION, sTempSymbol);
            SvXMLElementExport aElemNE(*this, XML_NAMESPACE_TABLE, XML_NAMED_EXPRESSION, true, true);
        }
    }
}

namespace {

OUString getCondFormatEntryType(const ScColorScaleEntry& rEntry, bool bFirst = true)
{
    switch(rEntry.GetType())
    {
        case COLORSCALE_MIN:
            return OUString("minimum");
        case COLORSCALE_MAX:
            return OUString("maximum");
        case COLORSCALE_PERCENT:
            return OUString("percent");
        case COLORSCALE_PERCENTILE:
            return OUString("percentile");
        case COLORSCALE_FORMULA:
            return OUString("formula");
        case COLORSCALE_VALUE:
            return OUString("number");
        case COLORSCALE_AUTO:
            // only important for data bars
            if(bFirst)
                return OUString("auto-minimum");
            else
                return OUString("auto-maximum");
    }
    return OUString();
}

OUString getIconSetName(ScIconSetType eType)
{
    const char* pName = NULL;
    ScIconSetMap* pMap = ScIconSetFormat::getIconSetMap();
    for(;pMap->pName;++pMap)
    {
        if(pMap->eType == eType)
        {
            pName = pMap->pName;
            break;
        }
    }
    assert(pName);
    return OUString::createFromAscii(pName);
}

OUString getDateStringForType(condformat::ScCondFormatDateType eType)
{
    switch(eType)
    {
        case condformat::TODAY:
            return OUString("today");
        case condformat::YESTERDAY:
            return OUString("yesterday");
        case condformat::TOMORROW:
            return OUString("tomorrow");
        case condformat::LAST7DAYS:
            return OUString("last-7-days");
        case condformat::THISWEEK:
            return OUString("this-week");
        case condformat::LASTWEEK:
            return OUString("last-week");
        case condformat::NEXTWEEK:
            return OUString("next-week");
        case condformat::THISMONTH:
            return OUString("this-month");
        case condformat::LASTMONTH:
            return OUString("last-month");
        case condformat::NEXTMONTH:
            return OUString("next-month");
        case condformat::THISYEAR:
            return OUString("this-year");
        case condformat::LASTYEAR:
            return OUString("last-year");
        case condformat::NEXTYEAR:
            return OUString("next-year");
    }

    return OUString();
}

}

void ScXMLExport::ExportConditionalFormat(SCTAB nTab)
{
    ScConditionalFormatList* pCondFormatList = pDoc->GetCondFormList(nTab);
    if(pCondFormatList)
    {
        if(pCondFormatList && !pCondFormatList->size())
            return;

        SvXMLElementExport aElementCondFormats(*this, XML_NAMESPACE_CALC_EXT, XML_CONDITIONAL_FORMATS, true, true);

        if(pCondFormatList)
        {
            for(ScConditionalFormatList::const_iterator itr = pCondFormatList->begin();
                    itr != pCondFormatList->end(); ++itr)
            {
                OUString sRanges;
                const ScRangeList& rRangeList = itr->GetRange();
                ScRangeStringConverter::GetStringFromRangeList( sRanges, &rRangeList, pDoc, formula::FormulaGrammar::CONV_ODF );
                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TARGET_RANGE_ADDRESS, sRanges);
                SvXMLElementExport aElementCondFormat(*this, XML_NAMESPACE_CALC_EXT, XML_CONDITIONAL_FORMAT, true, true);
                size_t nEntries = itr->size();
                for(size_t i = 0; i < nEntries; ++i)
                {
                    const ScFormatEntry* pFormatEntry = itr->GetEntry(i);
                    if(pFormatEntry->GetType()==condformat::CONDITION)
                    {
                        const ScCondFormatEntry* pEntry = static_cast<const ScCondFormatEntry*>(pFormatEntry);
                        OUStringBuffer aCond;
                        ScAddress aPos = pEntry->GetSrcPos();
                        switch(pEntry->GetOperation())
                        {
                            case SC_COND_EQUAL:
                                aCond.append('=');
                                aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                                break;
                            case SC_COND_LESS:
                                aCond.append('<');
                                aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                                break;
                            case SC_COND_GREATER:
                                aCond.append('>');
                                aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                                break;
                            case SC_COND_EQLESS:
                                aCond.append("<=");
                                aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                                break;
                            case SC_COND_EQGREATER:
                                aCond.append(">=");
                                aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                                break;
                            case SC_COND_NOTEQUAL:
                                aCond.append("!=");
                                aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                                break;
                            case SC_COND_BETWEEN:
                                aCond.append(OUString("between("));
                                aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                                aCond.append(',');
                                aCond.append(pEntry->GetExpression(aPos, 1, 0, formula::FormulaGrammar::GRAM_ODFF));
                                aCond.append(')');
                                break;
                            case SC_COND_NOTBETWEEN:
                                aCond.append(OUString("not-between("));
                                aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                                aCond.append(',');
                                aCond.append(pEntry->GetExpression(aPos, 1, 0, formula::FormulaGrammar::GRAM_ODFF));
                                aCond.append(')');
                                break;
                            case SC_COND_DUPLICATE:
                                aCond.append("duplicate");
                                break;
                            case SC_COND_NOTDUPLICATE:
                                aCond.append("unique");
                                break;
                            case SC_COND_DIRECT:
                                aCond.append("formula-is(");
                                aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                                aCond.append(')');
                                break;
                            case SC_COND_TOP10:
                                aCond.append("top-elements(");
                                aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                                aCond.append(")");
                                break;
                            case SC_COND_BOTTOM10:
                                aCond.append("bottom-elements(");
                                aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                                aCond.append(")");
                                break;
                            case SC_COND_TOP_PERCENT:
                                aCond.append("top-percent(");
                                aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                                aCond.append(")");
                                break;
                            case SC_COND_BOTTOM_PERCENT:
                                aCond.append("bottom-percent(");
                                aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                                aCond.append(")");
                                break;
                            case SC_COND_ABOVE_AVERAGE:
                                aCond.append("above-average");
                                break;
                            case SC_COND_BELOW_AVERAGE:
                                aCond.append("below-average");
                                break;
                            case SC_COND_ABOVE_EQUAL_AVERAGE:
                                aCond.append("above-equal-average");
                                break;
                            case SC_COND_BELOW_EQUAL_AVERAGE:
                                aCond.append("below-equal-average");
                                break;
                            case SC_COND_ERROR:
                                aCond.append("is-error");
                                break;
                            case SC_COND_NOERROR:
                                aCond.append("is-no-error");
                                break;
                            case SC_COND_BEGINS_WITH:
                                aCond.append("begins-with(");
                                aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                                aCond.append(")");
                                break;
                            case SC_COND_ENDS_WITH:
                                aCond.append("ends-with(");
                                aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                                aCond.append(")");
                                break;
                            case SC_COND_CONTAINS_TEXT:
                                aCond.append("contains-text(");
                                aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                                aCond.append(")");
                                break;
                            case SC_COND_NOT_CONTAINS_TEXT:
                                aCond.append("not-contains-text(");
                                aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                                aCond.append(")");
                                break;
                            case SC_COND_NONE:
                                continue;
                            default:
                                SAL_WARN("sc", "unimplemented conditional format export");
                        }
                        OUString sStyle = ScStyleNameConversion::DisplayToProgrammaticName(pEntry->GetStyle(), SFX_STYLE_FAMILY_PARA);
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_APPLY_STYLE_NAME, sStyle);
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_VALUE, aCond.makeStringAndClear());

                        OUString sBaseAddress;
                        ScRangeStringConverter::GetStringFromAddress( sBaseAddress, aPos, pDoc,formula::FormulaGrammar::CONV_ODF );
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_BASE_CELL_ADDRESS, sBaseAddress);
                        SvXMLElementExport aElementCondEntry(*this, XML_NAMESPACE_CALC_EXT, XML_CONDITION, true, true);
                    }
                    else if(pFormatEntry->GetType() == condformat::COLORSCALE)
                    {
                        SvXMLElementExport aElementColorScale(*this, XML_NAMESPACE_CALC_EXT, XML_COLOR_SCALE, true, true);
                        const ScColorScaleFormat& mrColorScale = static_cast<const ScColorScaleFormat&>(*pFormatEntry);
                        for(ScColorScaleFormat::const_iterator it = mrColorScale.begin();
                                it != mrColorScale.end(); ++it)
                        {
                            if(it->GetType() == COLORSCALE_FORMULA)
                            {
                                OUString sFormula = it->GetFormula(formula::FormulaGrammar::GRAM_ODFF);
                                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_VALUE, sFormula);
                            }
                            else
                                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_VALUE, OUString::number(it->GetValue()));

                            AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, getCondFormatEntryType(*it));
                            OUStringBuffer aBuffer;
                            ::sax::Converter::convertColor(aBuffer, it->GetColor().GetColor());
                            AddAttribute(XML_NAMESPACE_CALC_EXT, XML_COLOR, aBuffer.makeStringAndClear());
                            SvXMLElementExport aElementColorScaleEntry(*this, XML_NAMESPACE_CALC_EXT, XML_COLOR_SCALE_ENTRY, true, true);
                        }
                    }
                    else if(pFormatEntry->GetType() == condformat::DATABAR)
                    {
                        const ScDataBarFormatData* pFormatData = static_cast<const ScDataBarFormat&>(*pFormatEntry).GetDataBarData();
                        if(!pFormatData->mbGradient)
                            AddAttribute(XML_NAMESPACE_CALC_EXT, XML_GRADIENT, XML_FALSE);
                        if(pFormatData->mbOnlyBar)
                            AddAttribute(XML_NAMESPACE_CALC_EXT, XML_SHOW_VALUE, XML_FALSE);

                        if(pFormatData->mbNeg)
                        {
                            if(pFormatData->mpNegativeColor)
                            {
                                OUStringBuffer aBuffer;
                                ::sax::Converter::convertColor(aBuffer, pFormatData->mpNegativeColor->GetColor());
                                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_NEGATIVE_COLOR, aBuffer.makeStringAndClear());
                            }
                            else
                            {
                                OUStringBuffer aBuffer;
                                ::sax::Converter::convertColor(aBuffer, Color(COL_LIGHTRED).GetColor());
                                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_NEGATIVE_COLOR, aBuffer.makeStringAndClear());
                            }
                        }

                        if(pFormatData->meAxisPosition != databar::AUTOMATIC)
                        {
                            if(pFormatData->meAxisPosition == databar::NONE)
                            {
                                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_AXIS_POSITION, OUString("none"));
                            }
                            else
                            {
                                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_AXIS_POSITION, OUString("middle"));
                            }
                        }

                        OUStringBuffer aBuffer;
                        ::sax::Converter::convertColor(aBuffer, pFormatData->maPositiveColor.GetColor());
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_POSITIVE_COLOR, aBuffer.makeStringAndClear());

                        aBuffer = OUStringBuffer();
                        ::sax::Converter::convertColor(aBuffer, pFormatData->maAxisColor.GetColor());
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_AXIS_COLOR, aBuffer.makeStringAndClear());
                        SvXMLElementExport aElementDataBar(*this, XML_NAMESPACE_CALC_EXT, XML_DATA_BAR, true, true);

                        {
                            if(pFormatData->mpLowerLimit->GetType() == COLORSCALE_FORMULA)
                            {
                                OUString sFormula = pFormatData->mpLowerLimit->GetFormula(formula::FormulaGrammar::GRAM_ODFF);
                                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_VALUE, sFormula);
                            }
                            else
                                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_VALUE, OUString::number(pFormatData->mpLowerLimit->GetValue()));
                            AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, getCondFormatEntryType(*pFormatData->mpLowerLimit, true));
                            SvXMLElementExport aElementDataBarEntryLower(*this, XML_NAMESPACE_CALC_EXT, XML_FORMATTING_ENTRY, true, true);
                        }

                        {
                            if(pFormatData->mpUpperLimit->GetType() == COLORSCALE_FORMULA)
                            {
                                OUString sFormula = pFormatData->mpUpperLimit->GetFormula(formula::FormulaGrammar::GRAM_ODFF);
                                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_VALUE, sFormula);
                            }
                            else
                                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_VALUE, OUString::number(pFormatData->mpUpperLimit->GetValue()));
                            AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, getCondFormatEntryType(*pFormatData->mpUpperLimit, false));
                            SvXMLElementExport aElementDataBarEntryUpper(*this, XML_NAMESPACE_CALC_EXT, XML_FORMATTING_ENTRY, true, true);
                        }
                    }
                    else if(pFormatEntry->GetType() == condformat::ICONSET)
                    {
                        const ScIconSetFormat& mrIconSet = static_cast<const ScIconSetFormat&>(*pFormatEntry);
                        OUString aIconSetName = getIconSetName(mrIconSet.GetIconSetData()->eIconSetType);
                        AddAttribute( XML_NAMESPACE_CALC_EXT, XML_ICON_SET_TYPE, aIconSetName );
                        SvXMLElementExport aElementColorScale(*this, XML_NAMESPACE_CALC_EXT, XML_ICON_SET, true, true);
                        if(!mrIconSet.GetIconSetData()->mbShowValue)
                            AddAttribute(XML_NAMESPACE_CALC_EXT, XML_SHOW_VALUE, XML_FALSE);
                        for(ScIconSetFormat::const_iterator it = mrIconSet.begin();
                                it != mrIconSet.end(); ++it)
                        {
                            if(it->GetType() == COLORSCALE_FORMULA)
                            {
                                OUString sFormula = it->GetFormula(formula::FormulaGrammar::GRAM_ODFF);
                                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_VALUE, sFormula);
                            }
                            else
                                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_VALUE, OUString::number(it->GetValue()));

                            AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, getCondFormatEntryType(*it));
                            SvXMLElementExport aElementColorScaleEntry(*this, XML_NAMESPACE_CALC_EXT, XML_FORMATTING_ENTRY, true, true);
                        }
                    }
                    else if(pFormatEntry->GetType() == condformat::DATE)
                    {
                        const ScCondDateFormatEntry& mrDateFormat = static_cast<const ScCondDateFormatEntry&>(*pFormatEntry);
                        OUString aDateType = getDateStringForType(mrDateFormat.GetDateType());
                        OUString aStyleName = ScStyleNameConversion::DisplayToProgrammaticName(mrDateFormat.GetStyleName(), SFX_STYLE_FAMILY_PARA );
                        AddAttribute( XML_NAMESPACE_CALC_EXT, XML_STYLE, aStyleName);
                        AddAttribute( XML_NAMESPACE_CALC_EXT, XML_DATE, aDateType);
                        SvXMLElementExport aElementDateFormat(*this, XML_NAMESPACE_CALC_EXT, XML_DATE_IS, true, true);
                    }
                }
            }
        }
    }
}

void ScXMLExport::WriteExternalRefCaches()
{
    if (!pDoc)
        return;

    ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
    pRefMgr->resetSrcFileData(GetOrigFileName());
    sal_uInt16 nCount = pRefMgr->getExternalFileCount();
    for (sal_uInt16 nFileId = 0; nFileId < nCount; ++nFileId)
    {
        const OUString* pUrl = pRefMgr->getExternalFileName(nFileId);
        if (!pUrl)
            continue;

        vector<OUString> aTabNames;
        pRefMgr->getAllCachedTableNames(nFileId, aTabNames);
        if (aTabNames.empty())
            continue;

        for (vector<OUString>::const_iterator itr = aTabNames.begin(), itrEnd = aTabNames.end();
              itr != itrEnd; ++itr)
        {
            ScExternalRefCache::TableTypeRef pTable = pRefMgr->getCacheTable(nFileId, *itr, false);
            if (!pTable.get() || !pTable->isReferenced())
                continue;

            OUStringBuffer aBuf;
            aBuf.append(sal_Unicode('\''));
            aBuf.append(*pUrl);
            aBuf.append(sal_Unicode('\''));
            aBuf.append(sal_Unicode('#'));
            aBuf.append(*itr);
            AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, aBuf.makeStringAndClear());
            AddAttribute(XML_NAMESPACE_TABLE, XML_PRINT, GetXMLToken(XML_FALSE));
            AddAttribute(XML_NAMESPACE_TABLE, XML_STYLE_NAME, sExternalRefTabStyleName);
            SvXMLElementExport aElemTable(*this, XML_NAMESPACE_TABLE, XML_TABLE, true, true);
            {
                const ScExternalRefManager::SrcFileData* pExtFileData = pRefMgr->getExternalFileData(nFileId);
                if (pExtFileData)
                {
                    OUString aRelUrl;
                    if (!pExtFileData->maRelativeName.isEmpty())
                        aRelUrl = pExtFileData->maRelativeName;
                    else
                        aRelUrl = GetRelativeReference(pExtFileData->maRelativeName);
                    AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE);
                    AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, aRelUrl);
                    AddAttribute(XML_NAMESPACE_TABLE, XML_TABLE_NAME, *itr);
                    if (!pExtFileData->maFilterName.isEmpty())
                        AddAttribute(XML_NAMESPACE_TABLE, XML_FILTER_NAME, pExtFileData->maFilterName);
                    if (!pExtFileData->maFilterOptions.isEmpty())
                        AddAttribute(XML_NAMESPACE_TABLE, XML_FILTER_OPTIONS, pExtFileData->maFilterOptions);
                    AddAttribute(XML_NAMESPACE_TABLE, XML_MODE, XML_COPY_RESULTS_ONLY);
                }
                SvXMLElementExport aElemTableSource(*this, XML_NAMESPACE_TABLE, XML_TABLE_SOURCE, true, true);
            }

            // Determine maximum column count of used area, for repeated cells.
            SCCOL nMaxColsUsed = 1;     // assume that there is at least one cell somewhere..
            vector<SCROW> aRows;
            pTable->getAllRows(aRows);
            for (vector<SCROW>::const_iterator itrRow = aRows.begin(), itrRowEnd = aRows.end();
                  itrRow != itrRowEnd; ++itrRow)
            {
                SCROW nRow = *itrRow;
                vector<SCCOL> aCols;
                pTable->getAllCols(nRow, aCols);
                if (!aCols.empty())
                {
                    SCCOL nCol = aCols.back();
                    if (nMaxColsUsed <= nCol)
                        nMaxColsUsed = nCol + 1;
                }
            }

            // Column definitions have to be present to make a valid file
            {
                if (nMaxColsUsed > 1)
                    AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED,
                                    OUString::number(nMaxColsUsed));
                SvXMLElementExport aElemColumn(*this, XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, true, true);
            }

            // Write cache content for this table.
            SCROW nLastRow = 0;
            bool bFirstRow = true;
            for (vector<SCROW>::const_iterator itrRow = aRows.begin(), itrRowEnd = aRows.end();
                  itrRow != itrRowEnd; ++itrRow)
            {
                SCROW nRow = *itrRow;
                if (bFirstRow)
                {
                    if (nRow > 0)
                    {
                        if (nRow > 1)
                        {
                            OUStringBuffer aVal;
                            aVal.append(nRow);
                            AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_ROWS_REPEATED, aVal.makeStringAndClear());
                        }
                        SvXMLElementExport aElemRow(*this, XML_NAMESPACE_TABLE, XML_TABLE_ROW, true, true);
                        OUStringBuffer aVal;
                        aVal.append(static_cast<sal_Int32>(nMaxColsUsed));
                        AddAttribute(XML_NAMESPACE_TABLE,  XML_NUMBER_COLUMNS_REPEATED, aVal.makeStringAndClear());
                        SvXMLElementExport aElemCell(*this, XML_NAMESPACE_TABLE, XML_TABLE_CELL, true, true);
                    }
                }
                else
                {
                    SCROW nRowGap = nRow - nLastRow;
                    if (nRowGap > 1)
                    {
                        if (nRowGap > 2)
                        {
                            OUStringBuffer aVal;
                            aVal.append(static_cast<sal_Int32>(nRowGap-1));
                            AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_ROWS_REPEATED, aVal.makeStringAndClear());
                        }
                        SvXMLElementExport aElemRow(*this, XML_NAMESPACE_TABLE, XML_TABLE_ROW, true, true);
                        OUStringBuffer aVal;
                        aVal.append(static_cast<sal_Int32>(nMaxColsUsed));
                        AddAttribute(XML_NAMESPACE_TABLE,  XML_NUMBER_COLUMNS_REPEATED, aVal.makeStringAndClear());
                        SvXMLElementExport aElemCell(*this, XML_NAMESPACE_TABLE, XML_TABLE_CELL, true, true);
                    }
                }
                SvXMLElementExport aElemRow(*this, XML_NAMESPACE_TABLE, XML_TABLE_ROW, true, true);

                vector<SCCOL> aCols;
                pTable->getAllCols(nRow, aCols);
                SCCOL nLastCol = 0;
                bool bFirstCol = true;
                for (vector<SCCOL>::const_iterator itrCol = aCols.begin(), itrColEnd = aCols.end();
                      itrCol != itrColEnd; ++itrCol)
                {
                    SCCOL nCol = *itrCol;
                    if (bFirstCol)
                    {
                        if (nCol > 0)
                        {
                            if (nCol > 1)
                            {
                                OUStringBuffer aVal;
                                aVal.append(static_cast<sal_Int32>(nCol));
                                AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED, aVal.makeStringAndClear());
                            }
                            SvXMLElementExport aElemCell(*this, XML_NAMESPACE_TABLE, XML_TABLE_CELL, true, true);
                        }
                    }
                    else
                    {
                        SCCOL nColGap = nCol - nLastCol;
                        if (nColGap > 1)
                        {
                            if (nColGap > 2)
                            {
                                OUStringBuffer aVal;
                                aVal.append(static_cast<sal_Int32>(nColGap-1));
                                AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED, aVal.makeStringAndClear());
                            }
                            SvXMLElementExport aElemCell(*this, XML_NAMESPACE_TABLE, XML_TABLE_CELL, true, true);
                        }
                    }

                    // Write out this cell.
                    sal_uInt32 nNumFmt = 0;
                    ScExternalRefCache::TokenRef pToken = pTable->getCell(nCol, nRow, &nNumFmt);
                    OUString aStrVal;
                    if (pToken.get())
                    {
                        sal_Int32 nIndex = GetNumberFormatStyleIndex(nNumFmt);
                        if (nIndex >= 0)
                        {
                            const OUString aStyleName = *pCellStyles->GetStyleNameByIndex(nIndex, true);
                            AddAttribute(XML_NAMESPACE_TABLE, XML_STYLE_NAME, aStyleName);
                        }

                        switch(pToken->GetType())
                        {
                            case svDouble:
                            {
                                AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_FLOAT);
                                OUStringBuffer aVal;
                                aVal.append(pToken->GetDouble());
                                aStrVal = aVal.makeStringAndClear();
                                AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE, aStrVal);
                            }
                            break;
                            case svString:
                            {
                                AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_STRING);
                                aStrVal = pToken->GetString();
                            }
                            break;
                            default:
                                ;
                        }
                    }
                    SvXMLElementExport aElemCell(*this, XML_NAMESPACE_TABLE, XML_TABLE_CELL, true, true);
                    SvXMLElementExport aElemText(*this, XML_NAMESPACE_TEXT, XML_P, true, false);
                    Characters(aStrVal);

                    nLastCol = nCol;
                    bFirstCol = false;
                }
                nLastRow = nRow;
                bFirstRow = false;
            }
        }
    }
}

// core implementation
void ScXMLExport::WriteConsolidation()
{
    if (pDoc)
    {
        const ScConsolidateParam* pCons(pDoc->GetConsolidateDlgData());
        if( pCons )
        {
            OUString sStrData;

            ScXMLConverter::GetStringFromFunction( sStrData, pCons->eFunction );
            AddAttribute( XML_NAMESPACE_TABLE, XML_FUNCTION, sStrData );

            sStrData = OUString();
            for( sal_Int32 nIndex = 0; nIndex < pCons->nDataAreaCount; ++nIndex )
                ScRangeStringConverter::GetStringFromArea( sStrData, *pCons->ppDataAreas[ nIndex ], pDoc, FormulaGrammar::CONV_OOO, true );
            AddAttribute( XML_NAMESPACE_TABLE, XML_SOURCE_CELL_RANGE_ADDRESSES, sStrData );

            ScRangeStringConverter::GetStringFromAddress( sStrData, ScAddress( pCons->nCol, pCons->nRow, pCons->nTab ), pDoc, FormulaGrammar::CONV_OOO );
            AddAttribute( XML_NAMESPACE_TABLE, XML_TARGET_CELL_ADDRESS, sStrData );

            if( pCons->bByCol && !pCons->bByRow )
                AddAttribute( XML_NAMESPACE_TABLE, XML_USE_LABEL, XML_COLUMN );
            else if( !pCons->bByCol && pCons->bByRow )
                AddAttribute( XML_NAMESPACE_TABLE, XML_USE_LABEL, XML_ROW );
            else if( pCons->bByCol && pCons->bByRow )
                AddAttribute( XML_NAMESPACE_TABLE, XML_USE_LABEL, XML_BOTH );

            if( pCons->bReferenceData )
                AddAttribute( XML_NAMESPACE_TABLE, XML_LINK_TO_SOURCE_DATA, XML_TRUE );

            SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE, XML_CONSOLIDATION, true, true );
        }
    }
}

SvXMLAutoStylePoolP* ScXMLExport::CreateAutoStylePool()
{
    return new ScXMLAutoStylePoolP(*this);
}

XMLPageExport* ScXMLExport::CreatePageExport()
{
    return new XMLTableMasterPageExport( *this );
}

void ScXMLExport::GetChangeTrackViewSettings(uno::Sequence<beans::PropertyValue>& rProps)
{
    ScChangeViewSettings* pViewSettings(GetDocument() ? GetDocument()->GetChangeViewSettings() : NULL);
    if (pViewSettings)
    {
        sal_Int32 nChangePos(rProps.getLength());
        rProps.realloc(nChangePos + 1);
        beans::PropertyValue* pProps(rProps.getArray());
        if (pProps)
        {
            uno::Sequence<beans::PropertyValue> aChangeProps(SC_VIEWCHANGES_COUNT);
            beans::PropertyValue* pChangeProps(aChangeProps.getArray());
            if (pChangeProps)
            {
                pChangeProps[SC_SHOW_CHANGES].Name = OUString("ShowChanges");
                pChangeProps[SC_SHOW_CHANGES].Value <<= pViewSettings->ShowChanges();
                pChangeProps[SC_SHOW_ACCEPTED_CHANGES].Name = OUString("ShowAcceptedChanges");
                pChangeProps[SC_SHOW_ACCEPTED_CHANGES].Value <<= pViewSettings->IsShowAccepted();
                pChangeProps[SC_SHOW_REJECTED_CHANGES].Name = OUString("ShowRejectedChanges");
                pChangeProps[SC_SHOW_REJECTED_CHANGES].Value <<= pViewSettings->IsShowRejected();
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME].Name = OUString("ShowChangesByDatetime");
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME].Value <<= pViewSettings->HasDate();
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_MODE].Name = OUString("ShowChangesByDatetimeMode");
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_MODE].Value <<= static_cast<sal_Int16>(pViewSettings->GetTheDateMode());
                util::DateTime aDateTime;
                ScXMLConverter::ConvertCoreToAPIDateTime(pViewSettings->GetTheFirstDateTime(), aDateTime);
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_FIRST_DATETIME].Name = OUString("ShowChangesByDatetimeFirstDatetime");
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_FIRST_DATETIME].Value <<= aDateTime;
                ScXMLConverter::ConvertCoreToAPIDateTime(pViewSettings->GetTheLastDateTime(), aDateTime);
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_SECOND_DATETIME].Name = OUString("ShowChangesByDatetimeSecondDatetime");
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_SECOND_DATETIME].Value <<= aDateTime;
                pChangeProps[SC_SHOW_CHANGES_BY_AUTHOR].Name = OUString("ShowChangesByAuthor");
                pChangeProps[SC_SHOW_CHANGES_BY_AUTHOR].Value <<= pViewSettings->HasAuthor();
                pChangeProps[SC_SHOW_CHANGES_BY_AUTHOR_NAME].Name = OUString("ShowChangesByAuthorName");
                pChangeProps[SC_SHOW_CHANGES_BY_AUTHOR_NAME].Value <<= OUString (pViewSettings->GetTheAuthorToShow());
                pChangeProps[SC_SHOW_CHANGES_BY_COMMENT].Name = OUString("ShowChangesByComment");
                pChangeProps[SC_SHOW_CHANGES_BY_COMMENT].Value <<= pViewSettings->HasComment();
                pChangeProps[SC_SHOW_CHANGES_BY_COMMENT_TEXT].Name = OUString("ShowChangesByCommentText");
                pChangeProps[SC_SHOW_CHANGES_BY_COMMENT_TEXT].Value <<= OUString (pViewSettings->GetTheComment());
                pChangeProps[SC_SHOW_CHANGES_BY_RANGES].Name = OUString("ShowChangesByRanges");
                pChangeProps[SC_SHOW_CHANGES_BY_RANGES].Value <<= pViewSettings->HasRange();
                OUString sRangeList;
                ScRangeStringConverter::GetStringFromRangeList(sRangeList, &(pViewSettings->GetTheRangeList()), GetDocument(), FormulaGrammar::CONV_OOO);
                pChangeProps[SC_SHOW_CHANGES_BY_RANGES_LIST].Name = OUString("ShowChangesByRangesList");
                pChangeProps[SC_SHOW_CHANGES_BY_RANGES_LIST].Value <<= sRangeList;

                pProps[nChangePos].Name = OUString("TrackedChangesViewSettings");
                pProps[nChangePos].Value <<= aChangeProps;
            }
        }
    }
}

void ScXMLExport::GetViewSettings(uno::Sequence<beans::PropertyValue>& rProps)
{
    rProps.realloc(4);
    beans::PropertyValue* pProps(rProps.getArray());
    if(pProps)
    {
        if (GetModel().is())
        {
            ScModelObj* pDocObj(ScModelObj::getImplementation( GetModel() ));
            if (pDocObj)
            {
                SfxObjectShell* pEmbeddedObj = pDocObj->GetEmbeddedObject();
                if (pEmbeddedObj)
                {
                    Rectangle aRect(pEmbeddedObj->GetVisArea());
                    sal_uInt16 i(0);
                    pProps[i].Name = OUString("VisibleAreaTop");
                    pProps[i].Value <<= static_cast<sal_Int32>(aRect.getY());
                    pProps[++i].Name = OUString("VisibleAreaLeft");
                    pProps[i].Value <<= static_cast<sal_Int32>(aRect.getX());
                    pProps[++i].Name = OUString("VisibleAreaWidth");
                    pProps[i].Value <<= static_cast<sal_Int32>(aRect.getWidth());
                    pProps[++i].Name = OUString("VisibleAreaHeight");
                    pProps[i].Value <<= static_cast<sal_Int32>(aRect.getHeight());
                }
            }
        }
    }
    GetChangeTrackViewSettings(rProps);
}




void ScXMLExport::GetConfigurationSettings(uno::Sequence<beans::PropertyValue>& rProps)
{
    if (GetModel().is())
    {
        uno::Reference <lang::XMultiServiceFactory> xMultiServiceFactory(GetModel(), uno::UNO_QUERY);
        if (xMultiServiceFactory.is())
        {
            uno::Reference <beans::XPropertySet> xProperties(xMultiServiceFactory->createInstance("com.sun.star.comp.SpreadsheetSettings"), uno::UNO_QUERY);
            if (xProperties.is())
                SvXMLUnitConverter::convertPropertySet(rProps, xProperties);

            sal_Int32 nPropsToAdd = 0;
            OUStringBuffer aTrackedChangesKey;
            if (GetDocument() && GetDocument()->GetChangeTrack() && GetDocument()->GetChangeTrack()->IsProtected())
            {
                ::sax::Converter::encodeBase64(aTrackedChangesKey,
                        GetDocument()->GetChangeTrack()->GetProtection());
                if (!aTrackedChangesKey.isEmpty())
                    ++nPropsToAdd;
            }

            bool bVBACompat = false;
            uno::Reference <container::XNameAccess> xCodeNameAccess;
            OSL_ENSURE( pDoc, "ScXMLExport::GetConfigurationSettings - no ScDocument!" );
            if( pDoc && pDoc->IsInVBAMode() )
            {
                // VBA compatibility mode
                bVBACompat = true;
                ++nPropsToAdd;
                // code names
                xCodeNameAccess = new XMLCodeNameProvider( pDoc );
                if( xCodeNameAccess->hasElements() )
                    ++nPropsToAdd;
                else
                    xCodeNameAccess.clear();
            }

            if( nPropsToAdd > 0 )
            {
                sal_Int32 nCount(rProps.getLength());
                rProps.realloc(nCount + nPropsToAdd);
                if (!aTrackedChangesKey.isEmpty())
                {
                    rProps[nCount].Name = OUString("TrackedChangesProtectionKey");
                    rProps[nCount].Value <<= aTrackedChangesKey.makeStringAndClear();
                    ++nCount;
                }
                if( bVBACompat )
                {
                    rProps[nCount].Name = OUString("VBACompatibilityMode");
                    rProps[nCount].Value <<= bVBACompat;
                    ++nCount;
                }
                if( xCodeNameAccess.is() )
                {
                    rProps[nCount].Name = OUString("ScriptConfiguration");
                    rProps[nCount].Value <<= xCodeNameAccess;
                    ++nCount;
                }
            }
        }
    }
}

XMLShapeExport* ScXMLExport::CreateShapeExport()
{
    return new ScXMLShapeExport(*this);
}

void ScXMLExport::CreateSharedData(const sal_Int32 nTableCount)
{
    pSharedData = new ScMySharedData(nTableCount);
}

XMLNumberFormatAttributesExportHelper* ScXMLExport::GetNumberFormatAttributesExportHelper()
{
    if (!pNumberFormatAttributesExportHelper)
        pNumberFormatAttributesExportHelper = new XMLNumberFormatAttributesExportHelper(GetNumberFormatsSupplier(), *this );
    return pNumberFormatAttributesExportHelper;
}

void ScXMLExport::CollectUserDefinedNamespaces(const SfxItemPool* pPool, sal_uInt16 nAttrib)
{
    const SfxPoolItem* pItem;
    sal_uInt32 nItems(pPool->GetItemCount2( nAttrib ));
    for( sal_uInt32 i = 0; i < nItems; ++i )
    {
        if( 0 != (pItem = pPool->GetItem2( nAttrib, i ) ) )
        {
            const SvXMLAttrContainerItem *pUnknown((const SvXMLAttrContainerItem *)pItem);
            if( (pUnknown->GetAttrCount() > 0) )
            {
                sal_uInt16 nIdx(pUnknown->GetFirstNamespaceIndex());
                while( USHRT_MAX != nIdx )
                {
                    if( (XML_NAMESPACE_UNKNOWN_FLAG & nIdx) != 0 )
                    {
                        const OUString& rPrefix = pUnknown->GetPrefix( nIdx );
                        // Add namespace declaration for unknown attributes if
                        // there aren't existing ones for the prefix used by the
                        // attibutes
                        _GetNamespaceMap().Add( rPrefix,
                                                pUnknown->GetNamespace( nIdx ),
                                                XML_NAMESPACE_UNKNOWN );
                    }
                    nIdx = pUnknown->GetNextNamespaceIndex( nIdx );
                }
            }
        }
    }

    // #i66550# needed for 'presentation:event-listener' element for URLs in shapes
    _GetNamespaceMap().Add(
        GetXMLToken( XML_NP_PRESENTATION ),
        GetXMLToken( XML_N_PRESENTATION ),
        XML_NAMESPACE_PRESENTATION );
}

void ScXMLExport::IncrementProgressBar(bool bEditCell, sal_Int32 nInc)
{
    nProgressCount += nInc;
    if (bEditCell || nProgressCount > 100)
    {
        GetProgressBarHelper()->Increment(nProgressCount);
        nProgressCount = 0;
    }
}

sal_uInt32 ScXMLExport::exportDoc( enum XMLTokenEnum eClass )
{
    if( (getExportFlags() & (EXPORT_FONTDECLS|EXPORT_STYLES|
                             EXPORT_MASTERSTYLES|EXPORT_CONTENT)) != 0 )
    {
        if (GetDocument())
        {
            // if source doc was Excel then
            uno::Reference< frame::XModel > xModel = GetModel();
            if ( xModel.is() )
            {
                uno::Reference< lang::XUnoTunnel >  xObjShellTunnel( xModel, uno::UNO_QUERY );
                SfxObjectShell* pFoundShell = reinterpret_cast<SfxObjectShell*>( xObjShellTunnel.is() ? xObjShellTunnel->getSomething(SfxObjectShell::getUnoTunnelId()) : 0 );
                if ( pFoundShell && ooo::vba::isAlienExcelDoc( *pFoundShell ) )
                {
                    xRowStylesPropertySetMapper = new XMLPropertySetMapper((XMLPropertyMapEntry*)aXMLScFromXLSRowStylesProperties, xScPropHdlFactory);
                    xRowStylesExportPropertySetMapper = new ScXMLRowExportPropertyMapper(xRowStylesPropertySetMapper);
                    GetAutoStylePool()->SetFamilyPropSetMapper( XML_STYLE_FAMILY_TABLE_ROW,
                        xRowStylesExportPropertySetMapper );
                }
            }
            CollectUserDefinedNamespaces(GetDocument()->GetPool(), ATTR_USERDEF);
            CollectUserDefinedNamespaces(GetDocument()->GetEditPool(), EE_PARA_XMLATTRIBS);
            CollectUserDefinedNamespaces(GetDocument()->GetEditPool(), EE_CHAR_XMLATTRIBS);
            ScDrawLayer* pDrawLayer = GetDocument()->GetDrawLayer();
            if (pDrawLayer)
            {
                CollectUserDefinedNamespaces(&pDrawLayer->GetItemPool(), EE_PARA_XMLATTRIBS);
                CollectUserDefinedNamespaces(&pDrawLayer->GetItemPool(), EE_CHAR_XMLATTRIBS);
                CollectUserDefinedNamespaces(&pDrawLayer->GetItemPool(), SDRATTR_XMLATTRIBUTES);
            }

            // sheet events use officeooo namespace
            if( (getExportFlags() & EXPORT_CONTENT) != 0 &&
                getDefaultVersion() >= SvtSaveOptions::ODFVER_012 )
            {
                bool bAnySheetEvents = false;
                SCTAB nTabCount = pDoc->GetTableCount();
                for (SCTAB nTab=0; nTab<nTabCount; ++nTab)
                    if (pDoc->GetSheetEvents(nTab))
                        bAnySheetEvents = true;
                if (bAnySheetEvents)
                    _GetNamespaceMap().Add(
                        GetXMLToken( XML_NP_OFFICE_EXT ),
                        GetXMLToken( XML_N_OFFICE_EXT ),
                        XML_NAMESPACE_OFFICE_EXT );
            }
        }
    }
    return SvXMLExport::exportDoc( eClass );
}

// XExporter
void SAL_CALL ScXMLExport::setSourceDocument( const uno::Reference<lang::XComponent>& xComponent )
                            throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SvXMLExport::setSourceDocument( xComponent );

    pDoc = ScXMLConverter::GetScDocument( GetModel() );
    OSL_ENSURE( pDoc, "ScXMLExport::setSourceDocument - no ScDocument!" );
    if (!pDoc)
        throw lang::IllegalArgumentException();

    // create ScChangeTrackingExportHelper after document is known
    pChangeTrackingExportHelper = new ScChangeTrackingExportHelper(*this);

    // Set the document's storage grammar corresponding to the ODF version that
    // is to be written.
    SvtSaveOptions::ODFDefaultVersion meODFDefaultVersion = getDefaultVersion();
    switch (meODFDefaultVersion)
    {
        // ODF 1.0 and 1.1 use GRAM_PODF, everything later or unspecified GRAM_ODFF
        case SvtSaveOptions::ODFVER_010:
        case SvtSaveOptions::ODFVER_011:
            pDoc->SetStorageGrammar( formula::FormulaGrammar::GRAM_PODF);
            break;
        default:
            pDoc->SetStorageGrammar( formula::FormulaGrammar::GRAM_ODFF);
    }
}

// XFilter
sal_Bool SAL_CALL ScXMLExport::filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
    throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDoc)
        pDoc->EnableIdle(false);
    bool bReturn(SvXMLExport::filter(aDescriptor));
    if (pDoc)
        pDoc->EnableIdle(true);
    return bReturn;
}

void SAL_CALL ScXMLExport::cancel()
    throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDoc)
        pDoc->EnableIdle(true);
    SvXMLExport::cancel();
}

// XInitialization
void SAL_CALL ScXMLExport::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SvXMLExport::initialize(aArguments);
}

// XServiceInfo
OUString SAL_CALL ScXMLExport::getImplementationName(  )
    throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_uInt16 nFlags = getExportFlags();
    if (nFlags & EXPORT_OASIS)
    {
        nFlags |= EXPORT_OASIS;
        switch( nFlags )
        {
            case EXPORT_ALL:
                return ScXMLOasisExport_getImplementationName();
            case (EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_FONTDECLS):
                return ScXMLOasisExport_Styles_getImplementationName();
            case (EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS):
                return ScXMLOasisExport_Content_getImplementationName();
            case EXPORT_META:
                return ScXMLOasisExport_Meta_getImplementationName();
            case EXPORT_SETTINGS:
                return ScXMLOasisExport_Settings_getImplementationName();
            default:
                // generic name for 'unknown' cases
                return ScXMLOasisExport_getImplementationName();
        }
    }
    else
    {
        switch( nFlags )
        {
            case EXPORT_ALL:
                return ScXMLOOoExport_getImplementationName();
            case (EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_FONTDECLS):
                return ScXMLOOoExport_Styles_getImplementationName();
            case (EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS):
                return ScXMLOOoExport_Content_getImplementationName();
            case EXPORT_META:
                return ScXMLOOoExport_Meta_getImplementationName();
            case EXPORT_SETTINGS:
                return ScXMLOOoExport_Settings_getImplementationName();
            default:
                // generic name for 'unknown' cases
                return ScXMLOOoExport_getImplementationName();
        }
    }
}

sal_Bool SAL_CALL ScXMLExport::supportsService( const OUString& ServiceName )
    throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return SvXMLExport::supportsService( ServiceName );
}

::com::sun::star::uno::Sequence< OUString > SAL_CALL ScXMLExport::getSupportedServiceNames(  )
    throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return SvXMLExport::getSupportedServiceNames();
}

// XUnoTunnel
sal_Int64 SAL_CALL ScXMLExport::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier )
    throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return SvXMLExport::getSomething(aIdentifier);
}

void ScXMLExport::DisposingModel()
{
    SvXMLExport::DisposingModel();
    pDoc = NULL;
    xCurrentTable = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
