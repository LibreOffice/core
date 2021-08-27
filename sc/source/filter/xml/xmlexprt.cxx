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

#include <sal/config.h>
#include <sal/log.hxx>

#include "xmlexprt.hxx"
#include "XMLConverter.hxx"
#include "xmlstyle.hxx"
#include <unonames.hxx>
#include <document.hxx>
#include <olinetab.hxx>
#include <formulacell.hxx>
#include <rangenam.hxx>
#include "XMLTableMasterPageExport.hxx"
#include <drwlayer.hxx>
#include "XMLExportDataPilot.hxx"
#include "XMLExportDatabaseRanges.hxx"
#include "XMLExportDDELinks.hxx"
#include "XMLExportIterator.hxx"
#include "XMLColumnRowGroupExport.hxx"
#include "XMLStylesExportHelper.hxx"
#include "XMLChangeTrackingExportHelper.hxx"
#include <sheetdata.hxx>
#include <docoptio.hxx>
#include "XMLExportSharedData.hxx"
#include <chgviset.hxx>
#include <docuno.hxx>
#include <textuno.hxx>
#include <chartlis.hxx>
#include <scitems.hxx>
#include <docpool.hxx>
#include <userdat.hxx>
#include <chgtrack.hxx>
#include <rangeutl.hxx>
#include <postit.hxx>
#include <externalrefmgr.hxx>
#include <editutil.hxx>
#include <tabprotection.hxx>
#include "cachedattraccess.hxx"
#include <colorscale.hxx>
#include <conditio.hxx>
#include <cellvalue.hxx>
#include <stylehelper.hxx>
#include <edittextiterator.hxx>
#include "editattributemap.hxx"
#include <arealink.hxx>
#include <datastream.hxx>
#include <documentlinkmgr.hxx>
#include <tokenstringcontext.hxx>
#include <cellform.hxx>
#include <datamapper.hxx>
#include <datatransformation.hxx>

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/families.hxx>
#include <xmloff/numehelp.hxx>
#include <xmloff/txtparae.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/section.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/xmlcnitm.hxx>
#include <editeng/flditem.hxx>
#include <editeng/eeitem.hxx>
#include <formula/errorcodes.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/XMLEventExport.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include <xmloff/table/XMLTableExport.hxx>

#include <sax/tools/converter.hxx>
#include <tools/fldunit.hxx>

#include <rtl/ustring.hxx>

#include <tools/color.hxx>
#include <rtl/math.hxx>
#include <svl/zforlist.hxx>
#include <svx/unoshape.hxx>
#include <comphelper/base64.hxx>
#include <comphelper/extract.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdocapt.hxx>
#include <vcl/svapp.hxx>
#include <svx/unoapi.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/form/XFormsSupplier2.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/sheet/XUsedAreaCursor.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XPrintAreas.hpp>
#include <com/sun/star/sheet/XUniqueCellFormatRangesSupplier.hpp>
#include <com/sun/star/sheet/XLabelRange.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>
#include <com/sun/star/sheet/XSheetCellCursor.hpp>
#include <com/sun/star/sheet/XSheetCellRanges.hpp>
#include <com/sun/star/sheet/XSheetLinkable.hpp>
#include <com/sun/star/sheet/GlobalSheetSettings.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/util/XProtectable.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include "XMLCodeNameProvider.hxx"

#include <sfx2/linkmgr.hxx>
#include <sfx2/objsh.hxx>

#include <memory>
#include <vector>
#include <vbahelper/vbaaccesshelper.hxx>
#include <officecfg/Office/Common.hxx>

namespace com::sun::star::uno { class XComponentContext; }



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
using ::com::sun::star::uno::UNO_QUERY;

namespace
{
OUString lcl_RangeSequenceToString(
    const uno::Sequence< OUString > & rRanges,
    const uno::Reference< chart2::data::XRangeXMLConversion > & xFormatConverter )
{
    OUStringBuffer aResult;
    const sal_Int32 nMaxIndex( rRanges.getLength() - 1 );
    const sal_Unicode cSep(' ');
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

OUString lcl_GetFormattedString(ScDocument* pDoc, const ScRefCellValue& rCell, const ScAddress& rAddr)
{
    // return text/edit cell string content, with line feeds in edit cells

    if (!pDoc)
        return EMPTY_OUSTRING;

    switch (rCell.meType)
    {
        case CELLTYPE_STRING:
        {
            OUString aStr;
            const Color* pColor;
            SvNumberFormatter* pFormatter = pDoc->GetFormatTable();

            sal_uInt32 nFormat = pDoc->GetNumberFormat(rAddr);
            ScCellFormat::GetString(rCell, nFormat, aStr, &pColor, *pFormatter, *pDoc);
            return aStr;
        }
        case CELLTYPE_EDIT:
        {
            const EditTextObject* pData = rCell.mpEditText;
            if (!pData)
                return EMPTY_OUSTRING;

            EditEngine& rEngine = pDoc->GetEditEngine();
            rEngine.SetText(*pData);
            return rEngine.GetText();
        }
        break;
        default:
            ;
    }

    return EMPTY_OUSTRING;
}

} // anonymous namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Calc_XMLExporter_get_implementation(css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ScXMLExport(context, "com.sun.star.comp.Calc.XMLExporter", SvXMLExportFlags::ALL));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Calc_XMLMetaExporter_get_implementation(css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ScXMLExport(context, "com.sun.star.comp.Calc.XMLMetaExporter", SvXMLExportFlags::META));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Calc_XMLStylesExporter_get_implementation(css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ScXMLExport(context, "com.sun.star.comp.Calc.XMLStylesExporter", SvXMLExportFlags::STYLES|SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::FONTDECLS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Calc_XMLContentExporter_get_implementation(css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ScXMLExport(context, "com.sun.star.comp.Calc.XMLContentExporter", SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::CONTENT|SvXMLExportFlags::SCRIPTS|SvXMLExportFlags::FONTDECLS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Calc_XMLSettingsExporter_get_implementation(css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ScXMLExport(context, "com.sun.star.comp.Calc.XMLSettingsExporter", SvXMLExportFlags::SETTINGS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Calc_XMLOasisExporter_get_implementation(css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ScXMLExport(context, "com.sun.star.comp.Calc.XMLOasisExporter", SvXMLExportFlags::ALL|SvXMLExportFlags::OASIS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Calc_XMLOasisMetaExporter_get_implementation(css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ScXMLExport(context, "com.sun.star.comp.Calc.XMLOasisMetaExporter", SvXMLExportFlags::META|SvXMLExportFlags::OASIS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Calc_XMLOasisStylesExporter_get_implementation(css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ScXMLExport(context, "com.sun.star.comp.Calc.XMLOasisStylesExporter", SvXMLExportFlags::STYLES|SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::FONTDECLS|SvXMLExportFlags::OASIS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Calc_XMLOasisContentExporter_get_implementation(css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ScXMLExport(context, "com.sun.star.comp.Calc.XMLOasisContentExporter", SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::CONTENT|SvXMLExportFlags::SCRIPTS|SvXMLExportFlags::FONTDECLS|SvXMLExportFlags::OASIS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Calc_XMLOasisSettingsExporter_get_implementation(css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ScXMLExport(context, "com.sun.star.comp.Calc.XMLOasisSettingsExporter", SvXMLExportFlags::SETTINGS|SvXMLExportFlags::OASIS));
}

namespace {

class ScXMLShapeExport : public XMLShapeExport
{
public:
    explicit ScXMLShapeExport(SvXMLExport& rExp) : XMLShapeExport(rExp) {}

    /** is called before a shape element for the given XShape is exported */
    virtual void onExport( const uno::Reference < drawing::XShape >& xShape ) override;
};

}

void ScXMLShapeExport::onExport( const uno::Reference < drawing::XShape >& xShape )
{
    uno::Reference< beans::XPropertySet > xShapeProp( xShape, uno::UNO_QUERY );
    if( xShapeProp.is() )
    {
        sal_Int16 nLayerID = 0;
        if( (xShapeProp->getPropertyValue( SC_LAYERID ) >>= nLayerID) && (SdrLayerID(nLayerID) == SC_LAYER_BACK) )
            GetExport().AddAttribute(XML_NAMESPACE_TABLE, XML_TABLE_BACKGROUND, XML_TRUE);
    }
}

sal_Int16 ScXMLExport::GetMeasureUnit()
{
    css::uno::Reference<css::sheet::XGlobalSheetSettings> xProperties =
                css::sheet::GlobalSheetSettings::create( comphelper::getProcessComponentContext() );
    const FieldUnit eFieldUnit = static_cast<FieldUnit>(xProperties->getMetric());
    return SvXMLUnitConverter::GetMeasureUnit(eFieldUnit);
}

constexpr OUStringLiteral gsLayerID( u"" SC_LAYERID );

ScXMLExport::ScXMLExport(
    const css::uno::Reference< css::uno::XComponentContext >& rContext,
    OUString const & implementationName, SvXMLExportFlags nExportFlag)
:   SvXMLExport(
        rContext, implementationName, GetMeasureUnit(), XML_SPREADSHEET, nExportFlag ),
    pDoc(nullptr),
    nSourceStreamPos(0),
    aTableStyles(),
    pCurrentCell(nullptr),
    nOpenRow(-1),
    nProgressCount(0),
    nCurrentTable(0),
    bHasRowHeader(false),
    bRowHeaderOpen(false)
{
    if (getExportFlags() & SvXMLExportFlags::CONTENT)
    {
        pGroupColumns.reset( new ScMyOpenCloseColumnRowGroup(*this, XML_TABLE_COLUMN_GROUP) );
        pGroupRows.reset( new ScMyOpenCloseColumnRowGroup(*this, XML_TABLE_ROW_GROUP) );
        pColumnStyles.reset( new ScColumnStyles() );
        pRowStyles.reset( new ScRowStyles() );
        pRowFormatRanges.reset( new ScRowFormatRanges() );
        pMergedRangesContainer.reset( new ScMyMergedRangesContainer() );
        pValidationsContainer.reset( new ScMyValidationsContainer() );
        mpCellsItr.reset(new ScMyNotEmptyCellsIterator(*this));
        pDefaults.reset( new ScMyDefaultStyles );
    }
    pCellStyles.reset( new ScFormatRangeStyles() );

    // document is not set here - create ScChangeTrackingExportHelper later

    xScPropHdlFactory = new XMLScPropHdlFactory;
    xCellStylesPropertySetMapper = new XMLPropertySetMapper(aXMLScCellStylesProperties, xScPropHdlFactory, true);
    xColumnStylesPropertySetMapper = new XMLPropertySetMapper(aXMLScColumnStylesProperties, xScPropHdlFactory, true);
    xRowStylesPropertySetMapper = new XMLPropertySetMapper(aXMLScRowStylesProperties, xScPropHdlFactory, true);
    xTableStylesPropertySetMapper = new XMLPropertySetMapper(aXMLScTableStylesProperties, xScPropHdlFactory, true);
    xCellStylesExportPropertySetMapper = new ScXMLCellExportPropertyMapper(xCellStylesPropertySetMapper);
    xCellStylesExportPropertySetMapper->ChainExportMapper(XMLTextParagraphExport::CreateParaExtPropMapper(*this));
    xColumnStylesExportPropertySetMapper = new ScXMLColumnExportPropertyMapper(xColumnStylesPropertySetMapper);
    xRowStylesExportPropertySetMapper = new ScXMLRowExportPropertyMapper(xRowStylesPropertySetMapper);
    xTableStylesExportPropertySetMapper = new ScXMLTableExportPropertyMapper(xTableStylesPropertySetMapper);

    GetAutoStylePool()->AddFamily(XmlStyleFamily::TABLE_CELL, XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME,
        xCellStylesExportPropertySetMapper, XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX);
    GetAutoStylePool()->AddFamily(XmlStyleFamily::TABLE_COLUMN, XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME,
        xColumnStylesExportPropertySetMapper, XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX);
    GetAutoStylePool()->AddFamily(XmlStyleFamily::TABLE_ROW, XML_STYLE_FAMILY_TABLE_ROW_STYLES_NAME,
        xRowStylesExportPropertySetMapper, XML_STYLE_FAMILY_TABLE_ROW_STYLES_PREFIX);
    GetAutoStylePool()->AddFamily(XmlStyleFamily::TABLE_TABLE, XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME,
        xTableStylesExportPropertySetMapper, XML_STYLE_FAMILY_TABLE_TABLE_STYLES_PREFIX);

    if( !(getExportFlags() & (SvXMLExportFlags::STYLES|SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::CONTENT)) )
        return;

    // This name is reserved for the external ref cache tables.  This
    // should not conflict with user-defined styles since this name is
    // used for a table style which is not available in the UI.
    sExternalRefTabStyleName = "ta_extref";
    GetAutoStylePool()->RegisterName(XmlStyleFamily::TABLE_TABLE, sExternalRefTabStyleName);

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

ScXMLExport::~ScXMLExport()
{
    pGroupColumns.reset();
    pGroupRows.reset();
    pColumnStyles.reset();
    pRowStyles.reset();
    pCellStyles.reset();
    pRowFormatRanges.reset();
    pMergedRangesContainer.reset();
    pValidationsContainer.reset();
    pChangeTrackingExportHelper.reset();
    pDefaults.reset();
    pNumberFormatAttributesExportHelper.reset();
}

void ScXMLExport::SetSourceStream( const uno::Reference<io::XInputStream>& xNewStream )
{
    xSourceStream = xNewStream;

    if ( !xSourceStream.is() )
        return;

    // make sure it's a plain UTF-8 stream as written by OOo itself

    const char pXmlHeader[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
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

        const ScSheetSaveData* pSheetData = comphelper::getUnoTunnelImplementation<ScModelObj>(GetModel())->GetSheetSaveData();
        if (pSheetData)
        {
            // add the loaded namespaces to the name space map

            if ( !pSheetData->AddLoadedNamespaces( GetNamespaceMap_() ) )
            {
                // conflicts in the namespaces - ignore the stream, save normally
                xSourceStream.clear();
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

void ScXMLExport::CollectSharedData(SCTAB& nTableCount, sal_Int32& nShapesCount)
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
        pSharedData.reset(new ScMySharedData(nTableCount));

    pCellStyles->AddNewTable(nTableCount - 1);

    for (SCTAB nTable = 0; nTable < nTableCount; ++nTable)
    {
        nCurrentTable = sal::static_int_cast<sal_uInt16>(nTable);
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xIndex->getByIndex(nTable), uno::UNO_QUERY);
        if (!xDrawPageSupplier.is())
            continue;

        uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage());
        ScMyDrawPage aDrawPage;
        aDrawPage.bHasForms = false;
        aDrawPage.xDrawPage.set(xDrawPage);
        pSharedData->AddDrawPage(aDrawPage, nTable);
        if (!xDrawPage.is())
            continue;

        sal_Int32 nShapes = xDrawPage->getCount();
        for (sal_Int32 nShape = 0; nShape < nShapes; ++nShape)
        {
            uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(nShape), uno::UNO_QUERY);
            if (!xShape.is())
                continue;

            uno::Reference<beans::XPropertySet> xShapeProp(xShape, uno::UNO_QUERY);
            if (!xShapeProp.is())
                continue;

            sal_Int16 nLayerID = 0;
            bool bExtracted = xShapeProp->getPropertyValue(gsLayerID) >>= nLayerID;
            if (!bExtracted)
                continue;

            if ((SdrLayerID(nLayerID) == SC_LAYER_INTERN) || (SdrLayerID(nLayerID) == SC_LAYER_HIDDEN))
            {
                CollectInternalShape(xShape);
                continue;
            }

            ++nShapesCount;

            SvxShape* pShapeImp = comphelper::getUnoTunnelImplementation<SvxShape>(xShape);
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
                aMyShape.bResizeWithCell = ScDrawLayer::IsResizeWithCell(*pSdrObj);
                pSharedData->AddNewShape(aMyShape);
                pSharedData->SetLastColumn(nTable, pAnchor->maStart.Col());
                pSharedData->SetLastRow(nTable, pAnchor->maStart.Row());
            }
            else
                pSharedData->AddTableShape(nTable, xShape);
        }
    }
}

void ScXMLExport::CollectShapesAutoStyles(SCTAB nTableCount)
{
    // #i84077# To avoid compiler warnings about uninitialized aShapeItr,
    // it's initialized using this dummy list. The iterator contains shapes
    // from all sheets, so it can't be declared inside the nTable loop where
    // it is used.
    ScMyShapeList aDummyInitList;

    pSharedData->SortShapesContainer();
    pSharedData->SortNoteShapes();
    const ScMyShapeList* pShapeList(nullptr);
    ScMyShapeList::const_iterator aShapeItr = aDummyInitList.end();
    if (pSharedData->GetShapesContainer())
    {
        pShapeList = &pSharedData->GetShapesContainer()->GetShapes();
        aShapeItr = pShapeList->begin();
    }
    if (pSharedData->HasDrawPage())
    {
        for (SCTAB nTable = 0; nTable < nTableCount; ++nTable)
        {
            uno::Reference<drawing::XDrawPage> xDrawPage(pSharedData->GetDrawPage(nTable));

            if (xDrawPage.is())
            {
                GetShapeExport()->seekShapes(xDrawPage);
                uno::Reference< form::XFormsSupplier2 > xFormsSupplier( xDrawPage, uno::UNO_QUERY );
                if( xFormsSupplier.is() && xFormsSupplier->hasForms() )
                {
                    GetFormExport()->examineForms(xDrawPage);
                    pSharedData->SetDrawPageHasForms(nTable, true);
                }
                ScMyTableShapes* pTableShapes(pSharedData->GetTableShapes());
                if (pTableShapes)
                {
                    for (const auto& rxShape : (*pTableShapes)[nTable])
                    {
                        GetShapeExport()->collectShapeAutoStyles(rxShape);
                        IncrementProgressBar(false);
                    }
                }
                if (pShapeList)
                {
                    ScMyShapeList::const_iterator aEndItr(pShapeList->end());
                    while ( aShapeItr != aEndItr && ( aShapeItr->aAddress.Tab() == nTable ) )
                    {
                        GetShapeExport()->collectShapeAutoStyles(aShapeItr->xShape);
                        IncrementProgressBar(false);
                        ++aShapeItr;
                    }
                }
                if (pSharedData->GetNoteShapes())
                {
                    const ScMyNoteShapeList& rNoteShapes = pSharedData->GetNoteShapes()->GetNotes();
                    for (const auto& rNoteShape : rNoteShapes)
                    {
                        if ( rNoteShape.aPos.Tab() == nTable )
                            GetShapeExport()->collectShapeAutoStyles(rNoteShape.xShape);
                    }
                }
            }
        }
    }
    pSharedData->SortNoteShapes(); // sort twice, because some more shapes are added
}

void ScXMLExport::ExportMeta_()
{
    sal_Int32 nCellCount(pDoc ? pDoc->GetCellCount() : 0);
    SCTAB nTableCount(0);
    sal_Int32 nShapesCount(0);
    GetAutoStylePool()->ClearEntries();
    CollectSharedData(nTableCount, nShapesCount);

    uno::Sequence<beans::NamedValue> stats
    {
        { "TableCount",  uno::makeAny(static_cast<sal_Int32>(nTableCount)) },
        { "CellCount",   uno::makeAny(nCellCount) },
        { "ObjectCount", uno::makeAny(nShapesCount) }
    };

    // update document statistics at the model
    uno::Reference<document::XDocumentPropertiesSupplier> xPropSup(GetModel(),
        uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps(
        xPropSup->getDocumentProperties());
    if (xDocProps.is()) {
        xDocProps->setDocumentStatistics(stats);
    }

    // export document properties
    SvXMLExport::ExportMeta_();
}

void ScXMLExport::ExportFontDecls_()
{
    GetFontAutoStylePool(); // make sure the pool is created
    SvXMLExport::ExportFontDecls_();
}

table::CellRangeAddress ScXMLExport::GetEndAddress(const uno::Reference<sheet::XSpreadsheet>& xTable)
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

void ScXMLExport::GetAreaLinks( ScMyAreaLinksContainer& rAreaLinks )
{
    if (pDoc->GetLinkManager())
    {
        const sfx2::SvBaseLinks& rLinks = pDoc->GetLinkManager()->GetLinks();
        for (const auto & rLink : rLinks)
        {
            ScAreaLink *pLink = dynamic_cast<ScAreaLink*>(rLink.get());
            if (pLink)
            {
                ScMyAreaLink aAreaLink;
                aAreaLink.aDestRange = pLink->GetDestArea();
                aAreaLink.sSourceStr = pLink->GetSource();
                aAreaLink.sFilter = pLink->GetFilter();
                aAreaLink.sFilterOptions = pLink->GetOptions();
                aAreaLink.sURL = pLink->GetFile();
                aAreaLink.nRefresh = pLink->GetRefreshDelay();
                rAreaLinks.AddNewAreaLink( aAreaLink );
            }
        }
    }
    rAreaLinks.Sort();
}

// core implementation
void ScXMLExport::GetDetectiveOpList( ScMyDetectiveOpContainer& rDetOp )
{
    if (!pDoc)
        return;

    ScDetOpList* pOpList(pDoc->GetDetOpList());
    if( !pOpList )
        return;

    size_t nCount = pOpList->Count();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex )
    {
        const ScDetOpData& rDetData = pOpList->GetObject( nIndex);
        const ScAddress& rDetPos = rDetData.GetPos();
        SCTAB nTab = rDetPos.Tab();
        if ( nTab < pDoc->GetTableCount() )
        {
            rDetOp.AddOperation( rDetData.GetOperation(), rDetPos, static_cast<sal_uInt32>( nIndex) );

            // cells with detective operations are written even if empty
            pSharedData->SetLastColumn( nTab, rDetPos.Col() );
            pSharedData->SetLastRow( nTab, rDetPos.Row() );
        }
    }
    rDetOp.Sort();
}

void ScXMLExport::WriteSingleColumn(const sal_Int32 nRepeatColumns, const sal_Int32 nStyleIndex,
    const sal_Int32 nIndex, const bool bIsAutoStyle, const bool bIsVisible)
{
    CheckAttrList();
    // tdf#138466
    if (nStyleIndex != -1)
        AddAttribute(sAttrStyleName, pColumnStyles->GetStyleNameByIndex(nStyleIndex));
    if (!bIsVisible)
        AddAttribute(XML_NAMESPACE_TABLE, XML_VISIBILITY, XML_COLLAPSE);
    if (nRepeatColumns > 1)
    {
        OUString sOUEndCol(OUString::number(nRepeatColumns));
        AddAttribute(sAttrColumnsRepeated, sOUEndCol);
    }
    if (nIndex != -1)
        AddAttribute(XML_NAMESPACE_TABLE, XML_DEFAULT_CELL_STYLE_NAME, pCellStyles->GetStyleNameByIndex(nIndex, bIsAutoStyle));
    SvXMLElementExport aElemC(*this, sElemCol, true, true);
}

void ScXMLExport::WriteColumn(const sal_Int32 nColumn, const sal_Int32 nRepeatColumns,
    const sal_Int32 nStyleIndex, const bool bIsVisible)
{
    sal_Int32 nRepeat(1);
    sal_Int32 nPrevIndex(pDefaults->GetColDefaults()[nColumn].nIndex);
    bool bPrevAutoStyle(pDefaults->GetColDefaults()[nColumn].bIsAutoStyle);
    for (sal_Int32 i = nColumn + 1; i < nColumn + nRepeatColumns; ++i)
    {
        if ((pDefaults->GetColDefaults()[i].nIndex != nPrevIndex) ||
            (pDefaults->GetColDefaults()[i].bIsAutoStyle != bPrevAutoStyle))
        {
            WriteSingleColumn(nRepeat, nStyleIndex, nPrevIndex, bPrevAutoStyle, bIsVisible);
            nPrevIndex = pDefaults->GetColDefaults()[i].nIndex;
            bPrevAutoStyle = pDefaults->GetColDefaults()[i].bIsAutoStyle;
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

void ScXMLExport::ExportColumns(const sal_Int32 nTable, const ScRange& aColumnHeaderRange, const bool bHasColumnHeader)
{
    sal_Int32 nColsRepeated (1);
    sal_Int32 nIndex;
    sal_Int32 nPrevColumn(0);
    bool bPrevIsVisible (true);
    bool bWasHeader (false);
    bool bIsClosed (true);
    sal_Int32 nPrevIndex (-1);
    sal_Int32 nColumn;
    for (nColumn = 0; nColumn <= pSharedData->GetLastColumn(nTable); ++nColumn)
    {
        CheckAttrList();
        bool bIsVisible(true);
        nIndex = pColumnStyles->GetStyleNameIndex(nTable, nColumn, bIsVisible);

        const bool bIsHeader = bHasColumnHeader && (aColumnHeaderRange.aStart.Col() <= nColumn) && (nColumn <= aColumnHeaderRange.aEnd.Col());
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
        "NumberFormat", XML_NAMESPACE_STYLE, "data-style-name");

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
    for (const auto& rNumFmt : aNumFmts)
    {
        sal_Int32 nNumFmt = static_cast<sal_Int32>(rNumFmt);

        addDataStyle(nNumFmt);

        uno::Any aVal;
        aVal <<= nNumFmt;
        vector<XMLPropertyState> aProps;
        aVal <<= aDefaultStyle;
        aProps.emplace_back(nEntryIndex, aVal);

        OUString aName;
        sal_Int32 nIndex;
        if (GetAutoStylePool()->Add(aName, XmlStyleFamily::TABLE_CELL, aDefaultStyle, aProps))
        {
            pCellStyles->AddStyleName(aName, nIndex);
        }
        else
        {
            bool bIsAuto;
            nIndex = pCellStyles->GetIndexOfStyleName(
                aName, XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX, bIsAuto);
        }

        // store the number format to index mapping for later use.
        aNumFmtIndexMap.emplace(nNumFmt, nIndex);
    }
}

namespace {

void handleFont(
    std::vector<XMLPropertyState>& rPropStates,
    const SfxPoolItem* p, const rtl::Reference<XMLPropertySetMapper>& xMapper, const OUString& rXMLName )
{
    sal_Int32 nEntryCount = xMapper->GetEntryCount();

    // Apparently font info needs special handling.
    const SvxFontItem* pItem = static_cast<const SvxFontItem*>(p);

    sal_Int32 nIndexFontName = xMapper->GetEntryIndex(XML_NAMESPACE_STYLE, rXMLName, 0);

    if (nIndexFontName == -1 || nIndexFontName >= nEntryCount)
        return;

    uno::Any aAny;
    if (!pItem->QueryValue(aAny, MID_FONT_FAMILY_NAME))
        return;

    rPropStates.emplace_back(nIndexFontName, aAny);
}

const SvxFieldData* toXMLPropertyStates(
    std::vector<XMLPropertyState>& rPropStates, const std::vector<const SfxPoolItem*>& rSecAttrs,
    const rtl::Reference<XMLPropertySetMapper>& xMapper, const ScXMLEditAttributeMap& rAttrMap )
{
    const SvxFieldData* pField = nullptr;
    sal_Int32 nEntryCount = xMapper->GetEntryCount();
    rPropStates.reserve(rSecAttrs.size());
    for (const SfxPoolItem* p : rSecAttrs)
    {
        if (p->Which() == EE_FEATURE_FIELD)
        {
            pField = static_cast<const SvxFieldItem*>(p)->GetField();
            continue;
        }

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
                handleFont(rPropStates, p, xMapper, "font-name");
            break;
            case EE_CHAR_FONTINFO_CJK:
                handleFont(rPropStates, p, xMapper, "font-name-asian");
            break;
            case EE_CHAR_FONTINFO_CTL:
                handleFont(rPropStates, p, xMapper, "font-name-complex");
            break;
            case EE_CHAR_WEIGHT:
            case EE_CHAR_WEIGHT_CJK:
            case EE_CHAR_WEIGHT_CTL:
            {
                if (!static_cast<const SvxWeightItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                    continue;

                rPropStates.emplace_back(nIndex, aAny);
            }
            break;
            case EE_CHAR_FONTHEIGHT:
            case EE_CHAR_FONTHEIGHT_CJK:
            case EE_CHAR_FONTHEIGHT_CTL:
            {
                if (!static_cast<const SvxFontHeightItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                    continue;

                rPropStates.emplace_back(nIndex, aAny);
            }
            break;
            case EE_CHAR_ITALIC:
            case EE_CHAR_ITALIC_CJK:
            case EE_CHAR_ITALIC_CTL:
            {
                if (!static_cast<const SvxPostureItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                    continue;

                rPropStates.emplace_back(nIndex, aAny);
            }
            break;
            case EE_CHAR_UNDERLINE:
            {
                // Underline attribute needs to export multiple entries.
                sal_Int32 nIndexStyle = xMapper->GetEntryIndex(XML_NAMESPACE_STYLE, "text-underline-style", 0);
                if (nIndexStyle == -1 || nIndexStyle > nEntryCount)
                    break;

                sal_Int32 nIndexWidth = xMapper->GetEntryIndex(XML_NAMESPACE_STYLE, "text-underline-width", 0);
                if (nIndexWidth == -1 || nIndexWidth > nEntryCount)
                    break;

                sal_Int32 nIndexType = xMapper->GetEntryIndex(XML_NAMESPACE_STYLE, "text-underline-type", 0);
                if (nIndexType == -1 || nIndexType > nEntryCount)
                    break;

                sal_Int32 nIndexColor = xMapper->FindEntryIndex("CharUnderlineColor", XML_NAMESPACE_STYLE, "text-underline-color");
                if (nIndexColor == -1 || nIndexColor > nEntryCount)
                    break;

                sal_Int32 nIndexHasColor = xMapper->FindEntryIndex("CharUnderlineHasColor", XML_NAMESPACE_STYLE, "text-underline-color");
                if (nIndexHasColor == -1 || nIndexHasColor > nEntryCount)
                    break;

                const SvxUnderlineItem* pUL = static_cast<const SvxUnderlineItem*>(p);
                pUL->QueryValue(aAny, MID_TL_STYLE);
                rPropStates.emplace_back(nIndexStyle, aAny);
                rPropStates.emplace_back(nIndexType,  aAny);
                rPropStates.emplace_back(nIndexWidth, aAny);

                pUL->QueryValue(aAny, MID_TL_COLOR);
                rPropStates.emplace_back(nIndexColor, aAny);

                pUL->QueryValue(aAny, MID_TL_HASCOLOR);
                rPropStates.emplace_back(nIndexHasColor, aAny);
            }
            break;
            case EE_CHAR_OVERLINE:
            {
                // Same with overline.  Do just as we do with underline attributes.
                sal_Int32 nIndexStyle = xMapper->GetEntryIndex(XML_NAMESPACE_STYLE, "text-overline-style", 0);
                if (nIndexStyle == -1 || nIndexStyle > nEntryCount)
                    break;

                sal_Int32 nIndexWidth = xMapper->GetEntryIndex(XML_NAMESPACE_STYLE, "text-overline-width", 0);
                if (nIndexWidth == -1 || nIndexWidth > nEntryCount)
                    break;

                sal_Int32 nIndexType = xMapper->GetEntryIndex(XML_NAMESPACE_STYLE, "text-overline-type", 0);
                if (nIndexType == -1 || nIndexType > nEntryCount)
                    break;

                sal_Int32 nIndexColor = xMapper->FindEntryIndex("CharOverlineColor", XML_NAMESPACE_STYLE, "text-overline-color");
                if (nIndexColor == -1 || nIndexColor > nEntryCount)
                    break;

                sal_Int32 nIndexHasColor = xMapper->FindEntryIndex("CharOverlineHasColor", XML_NAMESPACE_STYLE, "text-overline-color");
                if (nIndexHasColor == -1 || nIndexHasColor > nEntryCount)
                    break;

                const SvxOverlineItem* pOL = static_cast<const SvxOverlineItem*>(p);
                pOL->QueryValue(aAny, MID_TL_STYLE);
                rPropStates.emplace_back(nIndexStyle, aAny);
                rPropStates.emplace_back(nIndexType,  aAny);
                rPropStates.emplace_back(nIndexWidth, aAny);

                pOL->QueryValue(aAny, MID_TL_COLOR);
                rPropStates.emplace_back(nIndexColor, aAny);

                pOL->QueryValue(aAny, MID_TL_HASCOLOR);
                rPropStates.emplace_back(nIndexHasColor, aAny);
            }
            break;
            case EE_CHAR_COLOR:
            {
                if (!static_cast<const SvxColorItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                    continue;

                ::Color nColor;
                if ( aAny >>= nColor )
                {
                    sal_Int32 nIndexColor = ( nColor == COL_AUTO ) ? xMapper->GetEntryIndex(
                        XML_NAMESPACE_STYLE, GetXMLToken( XML_USE_WINDOW_FONT_COLOR ), 0 ) : nIndex;
                    rPropStates.emplace_back( nIndexColor, aAny );
                }
            }
            break;
            case EE_CHAR_WLM:
            {
                if (!static_cast<const SvxWordLineModeItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                    continue;

                rPropStates.emplace_back(nIndex, aAny);
            }
            break;
            case EE_CHAR_STRIKEOUT:
            {
                if (!static_cast<const SvxCrossedOutItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                    continue;

                rPropStates.emplace_back(nIndex, aAny);
            }
            break;
            case EE_CHAR_RELIEF:
            {
                if (!static_cast<const SvxCharReliefItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                    continue;

                rPropStates.emplace_back(nIndex, aAny);
            }
            break;
            case EE_CHAR_OUTLINE:
            {
                if (!static_cast<const SvxContourItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                    continue;

                rPropStates.emplace_back(nIndex, aAny);
            }
            break;
            case EE_CHAR_SHADOW:
            {
                if (!static_cast<const SvxShadowedItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                    continue;

                rPropStates.emplace_back(nIndex, aAny);
            }
            break;
            case EE_CHAR_KERNING:
            {
                if (!static_cast<const SvxKerningItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                    continue;

                rPropStates.emplace_back(nIndex, aAny);
            }
            break;
            case EE_CHAR_PAIRKERNING:
            {
                if (!static_cast<const SvxAutoKernItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                    continue;

                rPropStates.emplace_back(nIndex, aAny);
            }
            break;
            case EE_CHAR_FONTWIDTH:
            {
                if (!static_cast<const SvxCharScaleWidthItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                    continue;

                rPropStates.emplace_back(nIndex, aAny);
            }
            break;
            case EE_CHAR_ESCAPEMENT:
            {
                sal_Int32 nIndexEsc = xMapper->FindEntryIndex("CharEscapement", XML_NAMESPACE_STYLE, "text-position");
                if (nIndexEsc == -1 || nIndexEsc > nEntryCount)
                    break;

                sal_Int32 nIndexEscHeight = xMapper->FindEntryIndex("CharEscapementHeight", XML_NAMESPACE_STYLE, "text-position");
                if (nIndexEscHeight == -1 || nIndexEscHeight > nEntryCount)
                    break;

                const SvxEscapementItem* pEsc = static_cast<const SvxEscapementItem*>(p);

                pEsc->QueryValue(aAny);
                rPropStates.emplace_back(nIndexEsc, aAny);

                pEsc->QueryValue(aAny, MID_ESC_HEIGHT);
                rPropStates.emplace_back(nIndexEscHeight, aAny);

            }
            break;
            case EE_CHAR_EMPHASISMARK:
            {
                if (!static_cast<const SvxEmphasisMarkItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                    continue;

                rPropStates.emplace_back(nIndex, aAny);
            }
            break;
            case EE_CHAR_LANGUAGE:
            case EE_CHAR_LANGUAGE_CJK:
            case EE_CHAR_LANGUAGE_CTL:
            {
                if (!static_cast<const SvxLanguageItem*>(p)->QueryValue(aAny, pEntry->mnFlag))
                    continue;

                // Export multiple entries.
                sal_Int32 nIndexLanguage, nIndexCountry, nIndexScript, nIndexTag;
                switch (p->Which())
                {
                    case EE_CHAR_LANGUAGE:
                        nIndexLanguage = xMapper->GetEntryIndex( XML_NAMESPACE_FO, "language", 0);
                        nIndexCountry = xMapper->GetEntryIndex( XML_NAMESPACE_FO, "country", 0);
                        nIndexScript = xMapper->GetEntryIndex( XML_NAMESPACE_FO, "script", 0);
                        nIndexTag = xMapper->GetEntryIndex( XML_NAMESPACE_STYLE, "rfc-language-tag", 0);
                    break;
                    case EE_CHAR_LANGUAGE_CJK:
                        nIndexLanguage = xMapper->GetEntryIndex( XML_NAMESPACE_STYLE, "language-asian", 0);
                        nIndexCountry = xMapper->GetEntryIndex( XML_NAMESPACE_STYLE, "country-asian", 0);
                        nIndexScript = xMapper->GetEntryIndex( XML_NAMESPACE_STYLE, "script-asian", 0);
                        nIndexTag = xMapper->GetEntryIndex( XML_NAMESPACE_STYLE, "rfc-language-tag-asian", 0);
                    break;
                    case EE_CHAR_LANGUAGE_CTL:
                        nIndexLanguage = xMapper->GetEntryIndex( XML_NAMESPACE_STYLE, "language-complex", 0);
                        nIndexCountry = xMapper->GetEntryIndex( XML_NAMESPACE_STYLE, "country-complex", 0);
                        nIndexScript = xMapper->GetEntryIndex( XML_NAMESPACE_STYLE, "script-complex", 0);
                        nIndexTag = xMapper->GetEntryIndex( XML_NAMESPACE_STYLE, "rfc-language-tag-complex", 0);
                    break;
                    default:
                        nIndexLanguage = nIndexCountry = nIndexScript = nIndexTag = -1;
                }
                assert( nIndexLanguage >= 0 && nIndexCountry >= 0 && nIndexScript >= 0 && nIndexTag >= 0);
                rPropStates.emplace_back( nIndexLanguage, aAny);
                rPropStates.emplace_back( nIndexCountry, aAny);
                rPropStates.emplace_back( nIndexScript, aAny);
                rPropStates.emplace_back( nIndexTag, aAny);
            }
            break;
            default:
                continue;
        }
    }

    return pField;
}

}

void ScXMLExport::ExportCellTextAutoStyles(sal_Int32 nTable)
{
    if (!ValidTab(nTable))
        return;

    rtl::Reference<XMLPropertySetMapper> xMapper = GetTextParagraphExport()->GetTextPropMapper()->getPropertySetMapper();
    rtl::Reference<SvXMLAutoStylePoolP> xStylePool = GetAutoStylePool();
    const ScXMLEditAttributeMap& rAttrMap = GetEditAttributeMap();

    sc::EditTextIterator aIter(*pDoc, nTable);
    sal_Int32 nCellCount = 0;
    for (const EditTextObject* pEdit = aIter.first(); pEdit; pEdit = aIter.next(), ++nCellCount)
    {
        std::vector<editeng::Section> aAttrs;
        pEdit->GetAllSections(aAttrs);
        if (aAttrs.empty())
            continue;

        for (const auto& rSec : aAttrs)
        {
            const std::vector<const SfxPoolItem*>& rSecAttrs = rSec.maAttributes;
            if (rSecAttrs.empty())
                // No formats applied to this section. Skip it.
                continue;

            std::vector<XMLPropertyState> aPropStates;
            toXMLPropertyStates(aPropStates, rSecAttrs, xMapper, rAttrMap);
            if (!aPropStates.empty())
                xStylePool->Add(XmlStyleFamily::TEXT_TEXT, OUString(), aPropStates);
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
                    AddAttribute(sAttrStyleName, pCellStyles->GetStyleNameByIndex(nIndex, bIsAutoStyle));
                if (nPrevValidationIndex > -1)
                    AddAttribute(XML_NAMESPACE_TABLE, XML_CONTENT_VALIDATION_NAME, pValidationsContainer->GetValidationName(nPrevValidationIndex));
                if (nCols > 1)
                {
                    AddAttribute(sAttrColumnsRepeated, OUString::number(nCols));
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
        if (nIndex != -1)
            AddAttribute(sAttrStyleName, pCellStyles->GetStyleNameByIndex(nIndex, bIsAutoStyle));
        if (nPrevValidationIndex > -1)
            AddAttribute(XML_NAMESPACE_TABLE, XML_CONTENT_VALIDATION_NAME, pValidationsContainer->GetValidationName(nPrevValidationIndex));
        if (nCols > 1)
        {
            AddAttribute(sAttrColumnsRepeated, OUString::number(nCols));
        }
        SvXMLElementExport aElemC(*this, sElemCell, true, true);
    }
}

void ScXMLExport::WriteRowStartTag(
    const sal_Int32 nIndex, const sal_Int32 nEqualRows,
    bool bHidden, bool bFiltered)
{
    // tdf#143940
    if (nIndex != -1)
        AddAttribute(sAttrStyleName, pRowStyles->GetStyleNameByIndex(nIndex));
    if (bHidden)
    {
        if (bFiltered)
            AddAttribute(XML_NAMESPACE_TABLE, XML_VISIBILITY, XML_FILTER);
        else
            AddAttribute(XML_NAMESPACE_TABLE, XML_VISIBILITY, XML_COLLAPSE);
    }
    if (nEqualRows > 1)
    {
        AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_ROWS_REPEATED, OUString::number(nEqualRows));
    }

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
    if (bHasRowHeader && !bRowHeaderOpen && nStartRow >= aRowHeaderRange.aStart.Row() && nStartRow <= aRowHeaderRange.aEnd.Row())
    {
        if (nStartRow == aRowHeaderRange.aStart.Row())
            OpenHeaderRows();
        sal_Int32 nEquals;
        if (aRowHeaderRange.aEnd.Row() < nStartRow + nEqualRows - 1)
            nEquals = aRowHeaderRange.aEnd.Row() - nStartRow + 1;
        else
            nEquals = nEqualRows;
        WriteRowStartTag(nIndex, nEquals, bHidden, bFiltered);
        nOpenRow = nStartRow + nEquals - 1;
        if (nEquals < nEqualRows)
        {
            CloseRow(nStartRow + nEquals - 1);
            WriteRowStartTag(nIndex, nEqualRows - nEquals, bHidden, bFiltered);
            nOpenRow = nStartRow + nEqualRows - 1;
        }
    }
    else
        WriteRowStartTag(nIndex, nEqualRows, bHidden, bFiltered);
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
        sal_Int32 nEndRowHidden = nStartRow - 1;
        sal_Int32 nEndRowFiltered = nStartRow - 1;
        sal_Int32 nRow;
        for (nRow = nStartRow; nRow < nEndRow; ++nRow)
        {
            if (nRow == nStartRow)
            {
                nPrevIndex = pRowStyles->GetStyleNameIndex(nTable, nRow);
                if (pDoc)
                {
                    if (nRow > nEndRowHidden)
                    {
                        bPrevHidden = rRowAttr.rowHidden(nTable, nRow, nEndRowHidden);
                        bHidden = bPrevHidden;
                    }
                    if (nRow > nEndRowFiltered)
                    {
                        bPrevFiltered = rRowAttr.rowFiltered(nTable, nRow, nEndRowFiltered);
                        bFiltered = bPrevFiltered;
                    }
                }

            }
            else
            {
                nIndex = pRowStyles->GetStyleNameIndex(nTable, nRow);
                if (pDoc)
                {
                    if (nRow > nEndRowHidden)
                        bHidden = rRowAttr.rowHidden(nTable, nRow, nEndRowHidden);
                    if (nRow > nEndRowFiltered)
                        bFiltered = rRowAttr.rowFiltered(nTable, nRow, nEndRowFiltered);
                }
                if (nIndex == nPrevIndex && bHidden == bPrevHidden && bFiltered == bPrevFiltered &&
                    !(bHasRowHeader && ((nRow == aRowHeaderRange.aStart.Row()) || (nRow - 1 == aRowHeaderRange.aEnd.Row()))) &&
                    !(pGroupRows->IsGroupStart(nRow)) &&
                    !(pGroupRows->IsGroupEnd(nRow - 1)))
                    ++nEqualRows;
                else
                {
                    assert(nPrevIndex >= 0 && "coverity#1438402");
                    ScRowFormatRanges* pTempRowFormatRanges = new ScRowFormatRanges(pRowFormatRanges.get());
                    OpenAndCloseRow(nPrevIndex, nRow - nEqualRows, nEqualRows, bPrevHidden, bPrevFiltered);
                    pRowFormatRanges.reset(pTempRowFormatRanges);
                    nEqualRows = 1;
                    nPrevIndex = nIndex;
                    bPrevHidden = bHidden;
                    bPrevFiltered = bFiltered;
                }
            }
        }
        assert(nPrevIndex >= 0 && "coverity#1438402");
        OpenNewRow(nPrevIndex, nRow - nEqualRows, nEqualRows, bPrevHidden, bPrevFiltered);
    }
    else
    {
        sal_Int32 nIndex = pRowStyles->GetStyleNameIndex(nTable, nStartRow);
        bool bHidden = false;
        bool bFiltered = false;
        if (pDoc)
        {
            sal_Int32 nEndRowHidden;
            sal_Int32 nEndRowFiltered;
            bHidden = rRowAttr.rowHidden(nTable, nStartRow, nEndRowHidden);
            bFiltered = rRowAttr.rowFiltered(nTable, nStartRow, nEndRowFiltered);
        }
        assert(nIndex >= 0 && "coverity#1438402");
        OpenNewRow(nIndex, nStartRow, 1, bHidden, bFiltered);
    }
    nOpenRow = nStartRow + nRepeatRow - 1;
}

void ScXMLExport::CloseRow(const sal_Int32 nRow)
{
    if (nOpenRow > -1)
    {
        EndElement(sElemRow, true);
        if (bHasRowHeader && nRow == aRowHeaderRange.aEnd.Row())
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
        pCellStyles->GetFormatRanges(nStartCol, nEndCol, nStartRow, nSheet, pRowFormatRanges.get());
        if (nOpenRow == - 1)
            OpenRow(nSheet, nStartRow, 1, aRowAttr);
        WriteRowContent();
        pRowFormatRanges->Clear();
    }
    else
    {
        if (nOpenRow > -1)
        {
            pCellStyles->GetFormatRanges(nStartCol, pSharedData->GetLastColumn(nSheet), nStartRow, nSheet, pRowFormatRanges.get());
            WriteRowContent();
            CloseRow(nStartRow);
            sal_Int32 nRows(1);
            sal_Int32 nTotalRows(nEndRow - nStartRow + 1 - 1);
            while (nRows < nTotalRows)
            {
                pRowFormatRanges->Clear();
                pCellStyles->GetFormatRanges(0, pSharedData->GetLastColumn(nSheet), nStartRow + nRows, nSheet, pRowFormatRanges.get());
                sal_Int32 nMaxRows = pRowFormatRanges->GetMaxRows();
                OSL_ENSURE(nMaxRows, "something went wrong");
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
                    pCellStyles->GetFormatRanges(0, pSharedData->GetLastColumn(nSheet), nStartRow + nRows, nSheet, pRowFormatRanges.get());
                WriteRowContent();
                CloseRow(nStartRow + nRows - 1);
            }
            if (nTotalRows == 1)
                CloseRow(nStartRow);
            OpenRow(nSheet, nEndRow, 1, aRowAttr);
            pRowFormatRanges->Clear();
            pCellStyles->GetFormatRanges(0, nEndCol, nEndRow, nSheet, pRowFormatRanges.get());
            WriteRowContent();
        }
        else
        {
            sal_Int32 nRows(0);
            sal_Int32 nTotalRows(nEndRow - nStartRow + 1 - 1);
            while (nRows < nTotalRows)
            {
                pCellStyles->GetFormatRanges(0, pSharedData->GetLastColumn(nSheet), nStartRow + nRows, nSheet, pRowFormatRanges.get());
                sal_Int32 nMaxRows = pRowFormatRanges->GetMaxRows();
                OSL_ENSURE(nMaxRows, "something went wrong");
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
                    pCellStyles->GetFormatRanges(0, pSharedData->GetLastColumn(nSheet), nStartRow + nRows, nSheet, pRowFormatRanges.get());
                WriteRowContent();
                CloseRow(nStartRow + nRows - 1);
            }
            OpenRow(nSheet, nEndRow, 1, aRowAttr);
            pRowFormatRanges->Clear();
            pCellStyles->GetFormatRanges(0, nEndCol, nEndRow, nSheet, pRowFormatRanges.get());
            WriteRowContent();
        }
    }
}

void ScXMLExport::GetColumnRowHeader(bool& rHasColumnHeader, ScRange& rColumnHeaderRange,
                                     bool& rHasRowHeader, ScRange& rRowHeaderRange,
                                     OUString& rPrintRanges) const
{
    uno::Reference <sheet::XPrintAreas> xPrintAreas (xCurrentTable, uno::UNO_QUERY);
    if (!xPrintAreas.is())
        return;

    rHasRowHeader = xPrintAreas->getPrintTitleRows();
    rHasColumnHeader = xPrintAreas->getPrintTitleColumns();
    table::CellRangeAddress rTempRowHeaderRange = xPrintAreas->getTitleRows();
    rRowHeaderRange = ScRange(rTempRowHeaderRange.StartColumn,
                              rTempRowHeaderRange.StartRow,
                              rTempRowHeaderRange.Sheet,
                              rTempRowHeaderRange.EndColumn,
                              rTempRowHeaderRange.EndRow,
                              rTempRowHeaderRange.Sheet);
    table::CellRangeAddress rTempColumnHeaderRange = xPrintAreas->getTitleColumns();
    rColumnHeaderRange = ScRange(rTempColumnHeaderRange.StartColumn,
                              rTempColumnHeaderRange.StartRow,
                              rTempColumnHeaderRange.Sheet,
                              rTempColumnHeaderRange.EndColumn,
                              rTempColumnHeaderRange.EndRow,
                              rTempColumnHeaderRange.Sheet);
    uno::Sequence< table::CellRangeAddress > aRangeList( xPrintAreas->getPrintAreas() );
    ScRangeStringConverter::GetStringFromRangeList( rPrintRanges, aRangeList, pDoc, FormulaGrammar::CONV_OOO );
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
    if (!pDoc)
        return;

    ScOutlineTable* pOutlineTable = pDoc->GetOutlineTable( static_cast<SCTAB>(nCurrentTable) );
    if(pOutlineTable)
    {
        ScOutlineArray& rCols(pOutlineTable->GetColArray());
        ScOutlineArray& rRows(pOutlineTable->GetRowArray());
        FillFieldGroup(&rCols, pGroupColumns.get());
        FillFieldGroup(&rRows, pGroupRows.get());
        pSharedData->SetLastColumn(nCurrentTable, pGroupColumns->GetLast());
        pSharedData->SetLastRow(nCurrentTable, pGroupRows->GetLast());
    }
}

void ScXMLExport::SetBodyAttributes()
{
    if (!(pDoc && pDoc->IsDocProtected()))
        return;

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
        else if (p->hasPasswordHash(PASSHASH_SHA256))
        {
            aPassHash = p->getPasswordHash(PASSHASH_SHA256);
            eHashUsed = PASSHASH_SHA256;
        }
        else if (p->hasPasswordHash(PASSHASH_XL, PASSHASH_SHA1))
        {
            aPassHash = p->getPasswordHash(PASSHASH_XL, PASSHASH_SHA1);
            eHashUsed = PASSHASH_XL;
        }
    }
    ::comphelper::Base64::encode(aBuffer, aPassHash);
    if (aBuffer.isEmpty())
        return;

    AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY, aBuffer.makeStringAndClear());
    if (getSaneDefaultVersion() < SvtSaveOptions::ODFSVER_012)
        return;

    if (eHashUsed == PASSHASH_XL)
    {
        AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY_DIGEST_ALGORITHM,
                     ScPassHashHelper::getHashURI(PASSHASH_XL));
        if (getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
            AddAttribute(XML_NAMESPACE_LO_EXT, XML_PROTECTION_KEY_DIGEST_ALGORITHM_2,
                    ScPassHashHelper::getHashURI(PASSHASH_SHA1));
    }
    else if (eHashUsed == PASSHASH_SHA1)
    {
        AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY_DIGEST_ALGORITHM,
                     ScPassHashHelper::getHashURI(PASSHASH_SHA1));
    }
    else if (eHashUsed == PASSHASH_SHA256)
    {
        AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY_DIGEST_ALGORITHM,
                     ScPassHashHelper::getHashURI(PASSHASH_SHA256));
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
    if ( !xDestSource.is() )
        return;

    uno::Reference<io::XOutputStream> xDestStream = xDestSource->getOutputStream();
    uno::Reference<io::XSeekable> xDestSeek( xDestStream, uno::UNO_QUERY );
    if ( !xDestSeek.is() )
        return;

    // temporary: set same stream again to clear buffer
    xDestSource->setOutputStream( xDestStream );

    if ( getExportFlags() & SvXMLExportFlags::PRETTY )
    {
        const OString aOutStr("\n   ");
        uno::Sequence<sal_Int8> aOutSeq( reinterpret_cast<sal_Int8 const *>(aOutStr.getStr()), aOutStr.getLength() );
        xDestStream->writeBytes( aOutSeq );
    }

    rNewStart = static_cast<sal_Int32>(xDestSeek->getPosition());

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

    rNewEnd = static_cast<sal_Int32>(xDestSeek->getPosition());
}

const ScXMLEditAttributeMap& ScXMLExport::GetEditAttributeMap() const
{
    if (!mpEditAttrMap)
        mpEditAttrMap.reset(new ScXMLEditAttributeMap);
    return *mpEditAttrMap;
}

void ScXMLExport::RegisterDefinedStyleNames( const uno::Reference< css::sheet::XSpreadsheetDocument > & xSpreadDoc )
{
    ScFormatSaveData* pFormatData = comphelper::getUnoTunnelImplementation<ScModelObj>(xSpreadDoc)->GetFormatSaveData();
    auto xAutoStylePool = GetAutoStylePool();
    for (const auto& rFormatInfo : pFormatData->maIDToName)
    {
        xAutoStylePool->RegisterDefinedName(XmlStyleFamily::TABLE_CELL, rFormatInfo.second);
    }
}

void ScXMLExport::ExportContent_()
{
    nCurrentTable = 0;
    if (!pSharedData)
    {
        SCTAB nTableCount(0);
        sal_Int32 nShapesCount(0);
        CollectSharedData(nTableCount, nShapesCount);
        OSL_FAIL("no shared data set");
        if (!pSharedData)
            return;
    }
    ScXMLExportDatabaseRanges aExportDatabaseRanges(*this);
    if (!GetModel().is())
        return;

    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( GetModel(), uno::UNO_QUERY );
    if ( !xSpreadDoc.is() )
        return;

    ScSheetSaveData* pSheetData = comphelper::getUnoTunnelImplementation<ScModelObj>(xSpreadDoc)->GetSheetSaveData();
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
        GetAreaLinks( aAreaLinks );
        ScMyEmptyDatabaseRangesContainer aEmptyRanges(aExportDatabaseRanges.GetEmptyDatabaseRanges());
        ScMyDetectiveOpContainer aDetectiveOpContainer;
        GetDetectiveOpList( aDetectiveOpContainer );

        pCellStyles->Sort();
        pMergedRangesContainer->Sort();
        pSharedData->GetDetectiveObjContainer()->Sort();

        mpCellsItr->Clear();
        mpCellsItr->SetShapes( pSharedData->GetShapesContainer() );
        mpCellsItr->SetNoteShapes( pSharedData->GetNoteShapes() );
        mpCellsItr->SetMergedRanges( pMergedRangesContainer.get() );
        mpCellsItr->SetAreaLinks( &aAreaLinks );
        mpCellsItr->SetEmptyDatabaseRanges( &aEmptyRanges );
        mpCellsItr->SetDetectiveObj( pSharedData->GetDetectiveObjContainer() );
        mpCellsItr->SetDetectiveOp( &aDetectiveOpContainer );

        if (nTableCount > 0)
            pValidationsContainer->WriteValidations(*this);
        WriteTheLabelRanges( xSpreadDoc );
        for (sal_Int32 nTable = 0; nTable < nTableCount; ++nTable)
        {
            sal_Int32 nStartOffset = -1;
            sal_Int32 nEndOffset = -1;
            if (pSheetData && pDoc && pDoc->IsStreamValid(static_cast<SCTAB>(nTable)) && !pDoc->GetChangeTrack())
                pSheetData->GetStreamPos( nTable, nStartOffset, nEndOffset );

            if ( nStartOffset >= 0 && nEndOffset >= 0 && xSourceStream.is() )
            {
                sal_Int32 nNewStart = -1;
                sal_Int32 nNewEnd = -1;
                CopySourceStream( nStartOffset, nEndOffset, nNewStart, nNewEnd );

                // store position of copied sheet in output
                pSheetData->AddSavePos( nTable, nNewStart, nNewEnd );

                // skip iterator entries for this sheet
                mpCellsItr->SkipTable(static_cast<SCTAB>(nTable));
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
    WriteDataStream();
    aExportDatabaseRanges.WriteDatabaseRanges();
    WriteExternalDataMapping();
    ScXMLExportDataPilot aExportDataPilot(*this);
    aExportDataPilot.WriteDataPilots();
    WriteConsolidation();
    ScXMLExportDDELinks aExportDDELinks(*this);
    aExportDDELinks.WriteDDELinks(xSpreadDoc);
    IncrementProgressBar(true, 0);
    GetProgressBarHelper()->SetValue(GetProgressBarHelper()->GetReference());
}

void ScXMLExport::ExportStyles_( bool bUsed )
{
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( GetModel(), uno::UNO_QUERY );
    if (xSpreadDoc.is())
        RegisterDefinedStyleNames( xSpreadDoc);

    if (!pSharedData)
    {
        SCTAB nTableCount(0);
        sal_Int32 nShapesCount(0);
        CollectSharedData(nTableCount, nShapesCount);
    }
    rtl::Reference<XMLCellStyleExport> aStylesExp(new XMLCellStyleExport(*this, GetAutoStylePool().get()));
    if (GetModel().is())
    {
        uno::Reference <lang::XMultiServiceFactory> xMultiServiceFactory(GetModel(), uno::UNO_QUERY);
        if (xMultiServiceFactory.is())
        {
            uno::Reference <beans::XPropertySet> xProperties(xMultiServiceFactory->createInstance("com.sun.star.sheet.Defaults"), uno::UNO_QUERY);
            if (xProperties.is())
                aStylesExp->exportDefaultStyle(xProperties, XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME, xCellStylesExportPropertySetMapper);
            if (pSharedData->HasShapes())
            {
                GetShapeExport()->ExportGraphicDefaults();
            }
        }
        collectDataStyles(false);
    }
    exportDataStyles();

    aStylesExp->exportStyleFamily(OUString("CellStyles"),
        OUString(XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME), xCellStylesExportPropertySetMapper, false, XmlStyleFamily::TABLE_CELL);

    SvXMLExport::ExportStyles_(bUsed);
}

void ScXMLExport::AddStyleFromCells(const uno::Reference<beans::XPropertySet>& xProperties,
                                    const uno::Reference<sheet::XSpreadsheet>& xTable,
                                    sal_Int32 nTable, const OUString* pOldName)
{
    css::uno::Any aAny = xProperties->getPropertyValue("FormatID");
    sal_uInt64 nKey = 0;
    aAny >>= nKey;

    //! pass xCellRanges instead
    uno::Reference<sheet::XSheetCellRanges> xCellRanges( xProperties, uno::UNO_QUERY );

    OUString sStyleName;
    sal_Int32 nNumberFormat(-1);
    sal_Int32 nValidationIndex(-1);
    std::vector<XMLPropertyState> aPropStates(xCellStylesExportPropertySetMapper->Filter(*this, xProperties));
    std::vector< XMLPropertyState >::iterator aItr(aPropStates.begin());
    std::vector< XMLPropertyState >::iterator aEndItr(aPropStates.end());
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
                    aItr = aPropStates.erase(aItr);
                    aEndItr = aPropStates.end();    // old aEndItr is invalidated!
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
        aPropStates.clear();
    if (nNumberFormat == -1)
        xProperties->getPropertyValue(SC_UNONAME_NUMFMT) >>= nNumberFormat;
    if (sStyleName.isEmpty())
        return;

    if (!aPropStates.empty())
    {
        sal_Int32 nIndex;
        if (pOldName)
        {
            if (GetAutoStylePool()->AddNamed(*pOldName, XmlStyleFamily::TABLE_CELL, sStyleName, aPropStates))
            {
                GetAutoStylePool()->RegisterName(XmlStyleFamily::TABLE_CELL, *pOldName);
                // add to pCellStyles, so the name is found for normal sheets
                pCellStyles->AddStyleName(*pOldName, nIndex);
            }
        }
        else
        {
            OUString sName;
            bool bAdded = false;
            if (nKey)
            {
                uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( GetModel(), uno::UNO_QUERY );
                ScFormatSaveData* pFormatData = comphelper::getUnoTunnelImplementation<ScModelObj>(xSpreadDoc)->GetFormatSaveData();
                auto itr = pFormatData->maIDToName.find(nKey);
                if (itr != pFormatData->maIDToName.end())
                {
                    sName = itr->second;
                    bAdded = GetAutoStylePool()->AddNamed(sName, XmlStyleFamily::TABLE_CELL, sStyleName, aPropStates);
                    if (bAdded)
                        GetAutoStylePool()->RegisterName(XmlStyleFamily::TABLE_CELL, sName);
                }
            }
            bool bIsAutoStyle(true);
            if (bAdded || GetAutoStylePool()->Add(sName, XmlStyleFamily::TABLE_CELL, sStyleName, aPropStates))
            {
                pCellStyles->AddStyleName(sName, nIndex);
            }
            else
                nIndex = pCellStyles->GetIndexOfStyleName(sName, XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX, bIsAutoStyle);

            const uno::Sequence<table::CellRangeAddress> aAddresses(xCellRanges->getRangeAddresses());
            bool bGetMerge(true);
            for (table::CellRangeAddress const & address : aAddresses)
            {
                pSharedData->SetLastColumn(nTable, address.EndColumn);
                pSharedData->SetLastRow(nTable, address.EndRow);
                pCellStyles->AddRangeStyleName(address, nIndex, bIsAutoStyle, nValidationIndex, nNumberFormat);
                if (bGetMerge)
                    bGetMerge = GetMerged(&address, xTable);
            }
        }
    }
    else
    {
        OUString sEncodedStyleName(EncodeStyleName(sStyleName));
        sal_Int32 nIndex(0);
        pCellStyles->AddStyleName(sEncodedStyleName, nIndex, false);
        if ( !pOldName )
        {
            const uno::Sequence<table::CellRangeAddress> aAddresses(xCellRanges->getRangeAddresses());
            bool bGetMerge(true);
            for (table::CellRangeAddress const & address : aAddresses)
            {
                if (bGetMerge)
                    bGetMerge = GetMerged(&address, xTable);
                pCellStyles->AddRangeStyleName(address, nIndex, false, nValidationIndex, nNumberFormat);
                if( sStyleName != "Default" || nValidationIndex != -1 )
                {
                    pSharedData->SetLastColumn(nTable, address.EndColumn);
                    pSharedData->SetLastRow(nTable, address.EndRow);
                }
            }
        }
    }
}

void ScXMLExport::AddStyleFromColumn(const uno::Reference<beans::XPropertySet>& xColumnProperties,
                                     const OUString* pOldName, sal_Int32& rIndex, bool& rIsVisible)
{
    std::vector<XMLPropertyState> aPropStates(xColumnStylesExportPropertySetMapper->Filter(*this, xColumnProperties));
    if(aPropStates.empty())
        return;

    auto aItr = std::find_if(aPropStates.begin(), aPropStates.end(),
        [this](const XMLPropertyState& rPropState) {
            return xColumnStylesPropertySetMapper->GetEntryContextId(rPropState.mnIndex) == CTF_SC_ISVISIBLE; });
    if (aItr != aPropStates.end())
    {
        aItr->maValue >>= rIsVisible;
    }

    const OUString sParent;
    if (pOldName)
    {
        if (GetAutoStylePool()->AddNamed(*pOldName, XmlStyleFamily::TABLE_COLUMN, sParent, aPropStates))
        {
            GetAutoStylePool()->RegisterName(XmlStyleFamily::TABLE_COLUMN, *pOldName);
            // add to pColumnStyles, so the name is found for normal sheets
            rIndex = pColumnStyles->AddStyleName(*pOldName);
        }
    }
    else
    {
        OUString sName;
        if (GetAutoStylePool()->Add(sName, XmlStyleFamily::TABLE_COLUMN, sParent, aPropStates))
        {
            rIndex = pColumnStyles->AddStyleName(sName);
        }
        else
            rIndex = pColumnStyles->GetIndexOfStyleName(sName, XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX);
    }
}

void ScXMLExport::AddStyleFromRow(const uno::Reference<beans::XPropertySet>& xRowProperties,
                                  const OUString* pOldName, sal_Int32& rIndex)
{
    std::vector<XMLPropertyState> aPropStates(xRowStylesExportPropertySetMapper->Filter(*this, xRowProperties));
    if(aPropStates.empty())
        return;

    const OUString sParent;
    if (pOldName)
    {
        if (GetAutoStylePool()->AddNamed(*pOldName, XmlStyleFamily::TABLE_ROW, sParent, aPropStates))
        {
            GetAutoStylePool()->RegisterName(XmlStyleFamily::TABLE_ROW, *pOldName);
            // add to pRowStyles, so the name is found for normal sheets
            rIndex = pRowStyles->AddStyleName(*pOldName);
        }
    }
    else
    {
        OUString sName;
        if (GetAutoStylePool()->Add(sName, XmlStyleFamily::TABLE_ROW, sParent, aPropStates))
        {
            rIndex = pRowStyles->AddStyleName(sName);
        }
        else
            rIndex = pRowStyles->GetIndexOfStyleName(sName, XML_STYLE_FAMILY_TABLE_ROW_STYLES_PREFIX);
    }
}

static uno::Any lcl_GetEnumerated( uno::Reference<container::XEnumerationAccess> const & xEnumAccess, sal_Int32 nIndex )
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

void ScXMLExport::collectAutoStyles()
{
    SvXMLExport::collectAutoStyles();

    if (mbAutoStylesCollected)
        return;

    if (!GetModel().is())
        return;

    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( GetModel(), uno::UNO_QUERY );
    if (!xSpreadDoc.is())
        return;

    uno::Reference<container::XIndexAccess> xIndex( xSpreadDoc->getSheets(), uno::UNO_QUERY );
    if (!xIndex.is())
        return;

    if (getExportFlags() & SvXMLExportFlags::CONTENT)
    {
        // Reserve the loaded cell style names.
        RegisterDefinedStyleNames( xSpreadDoc);

        //  re-create automatic styles with old names from stored data
        ScSheetSaveData* pSheetData = comphelper::getUnoTunnelImplementation<ScModelObj>(xSpreadDoc)->GetSheetSaveData();
        if (pSheetData && pDoc)
        {
            // formulas have to be calculated now, to detect changed results
            // (during normal save, they will be calculated anyway)
            SCTAB nTabCount = pDoc->GetTableCount();
            for (SCTAB nTab=0; nTab<nTabCount; ++nTab)
                if (pDoc->IsStreamValid(nTab))
                    pDoc->InterpretDirtyCells(ScRange(0, 0, nTab, pDoc->MaxCol(), pDoc->MaxRow(), nTab));

            // stored cell styles
            const std::vector<ScCellStyleEntry>& rCellEntries = pSheetData->GetCellStyles();
            for (const auto& rCellEntry : rCellEntries)
            {
                ScAddress aPos = rCellEntry.maCellPos;
                sal_Int32 nTable = aPos.Tab();
                bool bCopySheet = pDoc->IsStreamValid( static_cast<SCTAB>(nTable) );
                if (bCopySheet)
                {
                    uno::Reference <sheet::XSpreadsheet> xTable(xIndex->getByIndex(nTable), uno::UNO_QUERY);
                    uno::Reference <beans::XPropertySet> xProperties(
                        xTable->getCellByPosition( aPos.Col(), aPos.Row() ), uno::UNO_QUERY );

                    AddStyleFromCells(xProperties, xTable, nTable, &rCellEntry.maName);
                }
            }

            // stored column styles
            const std::vector<ScCellStyleEntry>& rColumnEntries = pSheetData->GetColumnStyles();
            for (const auto& rColumnEntry : rColumnEntries)
            {
                ScAddress aPos = rColumnEntry.maCellPos;
                sal_Int32 nTable = aPos.Tab();
                bool bCopySheet = pDoc->IsStreamValid( static_cast<SCTAB>(nTable) );
                if (bCopySheet)
                {
                    uno::Reference<table::XColumnRowRange> xColumnRowRange(xIndex->getByIndex(nTable), uno::UNO_QUERY);
                    uno::Reference<table::XTableColumns> xTableColumns(xColumnRowRange->getColumns());
                    uno::Reference<beans::XPropertySet> xColumnProperties(xTableColumns->getByIndex( aPos.Col() ), uno::UNO_QUERY);

                    sal_Int32 nIndex(-1);
                    bool bIsVisible(true);
                    AddStyleFromColumn( xColumnProperties, &rColumnEntry.maName, nIndex, bIsVisible );
                }
            }

            // stored row styles
            const std::vector<ScCellStyleEntry>& rRowEntries = pSheetData->GetRowStyles();
            for (const auto& rRowEntry : rRowEntries)
            {
                ScAddress aPos = rRowEntry.maCellPos;
                sal_Int32 nTable = aPos.Tab();
                bool bCopySheet = pDoc->IsStreamValid( static_cast<SCTAB>(nTable) );
                if (bCopySheet)
                {
                    uno::Reference<table::XColumnRowRange> xColumnRowRange(xIndex->getByIndex(nTable), uno::UNO_QUERY);
                    uno::Reference<table::XTableRows> xTableRows(xColumnRowRange->getRows());
                    uno::Reference<beans::XPropertySet> xRowProperties(xTableRows->getByIndex( aPos.Row() ), uno::UNO_QUERY);

                    sal_Int32 nIndex(-1);
                    AddStyleFromRow( xRowProperties, &rRowEntry.maName, nIndex );
                }
            }

            // stored table styles
            const std::vector<ScCellStyleEntry>& rTableEntries = pSheetData->GetTableStyles();
            for (const auto& rTableEntry : rTableEntries)
            {
                ScAddress aPos = rTableEntry.maCellPos;
                sal_Int32 nTable = aPos.Tab();
                bool bCopySheet = pDoc->IsStreamValid( static_cast<SCTAB>(nTable) );
                if (bCopySheet)
                {
                    //! separate method AddStyleFromTable needed?
                    uno::Reference<beans::XPropertySet> xTableProperties(xIndex->getByIndex(nTable), uno::UNO_QUERY);
                    if (xTableProperties.is())
                    {
                        std::vector<XMLPropertyState> aPropStates(xTableStylesExportPropertySetMapper->Filter(*this, xTableProperties));
                        OUString sName( rTableEntry.maName );
                        GetAutoStylePool()->AddNamed(sName, XmlStyleFamily::TABLE_TABLE, OUString(), aPropStates);
                        GetAutoStylePool()->RegisterName(XmlStyleFamily::TABLE_TABLE, sName);
                    }
                }
            }

            // stored styles for notes

            rtl::Reference<SvXMLExportPropertyMapper> xShapeMapper = XMLShapeExport::CreateShapePropMapper( *this );
            GetShapeExport(); // make sure the graphics styles family is added

            const std::vector<ScNoteStyleEntry>& rNoteEntries = pSheetData->GetNoteStyles();
            for (const auto& rNoteEntry : rNoteEntries)
            {
                ScAddress aPos = rNoteEntry.maCellPos;
                SCTAB nTable = aPos.Tab();
                bool bCopySheet = pDoc->IsStreamValid( nTable );
                if (bCopySheet)
                {
                    //! separate method AddStyleFromNote needed?

                    ScPostIt* pNote = pDoc->GetNote(aPos);
                    OSL_ENSURE( pNote, "note not found" );
                    if (pNote)
                    {
                        SdrCaptionObj* pDrawObj = pNote->GetOrCreateCaption( aPos );
                        // all uno shapes are created anyway in CollectSharedData
                        uno::Reference<beans::XPropertySet> xShapeProperties( pDrawObj->getUnoShape(), uno::UNO_QUERY );
                        if (xShapeProperties.is())
                        {
                            if ( !rNoteEntry.maStyleName.isEmpty() )
                            {
                                std::vector<XMLPropertyState> aPropStates(xShapeMapper->Filter(*this, xShapeProperties));
                                OUString sName( rNoteEntry.maStyleName );
                                GetAutoStylePool()->AddNamed(sName, XmlStyleFamily::SD_GRAPHICS_ID, OUString(), aPropStates);
                                GetAutoStylePool()->RegisterName(XmlStyleFamily::SD_GRAPHICS_ID, sName);
                            }
                            if ( !rNoteEntry.maTextStyle.isEmpty() )
                            {
                                std::vector<XMLPropertyState> aPropStates(
                                    GetTextParagraphExport()->GetParagraphPropertyMapper()->Filter(*this, xShapeProperties));
                                OUString sName( rNoteEntry.maTextStyle );
                                GetAutoStylePool()->AddNamed(sName, XmlStyleFamily::TEXT_PARAGRAPH, OUString(), aPropStates);
                                GetAutoStylePool()->RegisterName(XmlStyleFamily::TEXT_PARAGRAPH, sName);
                            }
                        }
                    }
                }
            }

            // note paragraph styles

            rtl::Reference<SvXMLExportPropertyMapper> xParaPropMapper = GetTextParagraphExport()->GetParagraphPropertyMapper();

            const std::vector<ScTextStyleEntry>& rNoteParaEntries = pSheetData->GetNoteParaStyles();
            for (const auto& rNoteParaEntry : rNoteParaEntries)
            {
                ScAddress aPos = rNoteParaEntry.maCellPos;
                SCTAB nTable = aPos.Tab();
                bool bCopySheet = pDoc->IsStreamValid( nTable );
                if (bCopySheet)
                {
                    ScPostIt* pNote = pDoc->GetNote( aPos );
                    OSL_ENSURE( pNote, "note not found" );
                    if (pNote)
                    {
                        SdrCaptionObj* pDrawObj = pNote->GetOrCreateCaption( aPos );
                        uno::Reference<container::XEnumerationAccess> xCellText(pDrawObj->getUnoShape(), uno::UNO_QUERY);
                        uno::Reference<beans::XPropertySet> xParaProp(
                            lcl_GetEnumerated( xCellText, rNoteParaEntry.maSelection.nStartPara ), uno::UNO_QUERY );
                        if ( xParaProp.is() )
                        {
                            std::vector<XMLPropertyState> aPropStates(xParaPropMapper->Filter(*this, xParaProp));
                            OUString sName( rNoteParaEntry.maName );
                            GetAutoStylePool()->AddNamed(sName, XmlStyleFamily::TEXT_PARAGRAPH, OUString(), aPropStates);
                            GetAutoStylePool()->RegisterName(XmlStyleFamily::TEXT_PARAGRAPH, sName);
                        }
                    }
                }
            }

            // note text styles

            rtl::Reference<SvXMLExportPropertyMapper> xTextPropMapper = XMLTextParagraphExport::CreateCharExtPropMapper( *this );

            const std::vector<ScTextStyleEntry>& rNoteTextEntries = pSheetData->GetNoteTextStyles();
            for (const auto& rNoteTextEntry : rNoteTextEntries)
            {
                ScAddress aPos = rNoteTextEntry.maCellPos;
                SCTAB nTable = aPos.Tab();
                bool bCopySheet = pDoc->IsStreamValid( nTable );
                if (bCopySheet)
                {
                    ScPostIt* pNote = pDoc->GetNote( aPos );
                    OSL_ENSURE( pNote, "note not found" );
                    if (pNote)
                    {
                        SdrCaptionObj* pDrawObj = pNote->GetOrCreateCaption( aPos );
                        uno::Reference<text::XSimpleText> xCellText(pDrawObj->getUnoShape(), uno::UNO_QUERY);
                        uno::Reference<beans::XPropertySet> xCursorProp(xCellText->createTextCursor(), uno::UNO_QUERY);
                        ScDrawTextCursor* pCursor = comphelper::getUnoTunnelImplementation<ScDrawTextCursor>( xCursorProp );
                        if (pCursor)
                        {
                            pCursor->SetSelection( rNoteTextEntry.maSelection );

                            std::vector<XMLPropertyState> aPropStates(xTextPropMapper->Filter(*this, xCursorProp));
                            OUString sName( rNoteTextEntry.maName );
                            GetAutoStylePool()->AddNamed(sName, XmlStyleFamily::TEXT_TEXT, OUString(), aPropStates);
                            GetAutoStylePool()->RegisterName(XmlStyleFamily::TEXT_TEXT, sName);
                        }
                    }
                }
            }

            // stored text styles

            // Calling createTextCursor fires up editeng, which is very slow, and often subsequent style entries
            // refer to the same cell, so cache it.
            ScAddress aPrevPos;
            uno::Reference<beans::XPropertySet> xPrevCursorProp;
            const std::vector<ScTextStyleEntry>& rTextEntries = pSheetData->GetTextStyles();
            for (const auto& rTextEntry : rTextEntries)
            {
                ScAddress aPos = rTextEntry.maCellPos;
                sal_Int32 nTable = aPos.Tab();
                bool bCopySheet = pDoc->IsStreamValid( static_cast<SCTAB>(nTable) );
                if (!bCopySheet)
                    continue;

                //! separate method AddStyleFromText needed?
                //! cache sheet object

                uno::Reference<beans::XPropertySet> xCursorProp;
                if (xPrevCursorProp && aPrevPos == aPos)
                    xCursorProp = xPrevCursorProp;
                else
                {
                    uno::Reference<table::XCellRange> xCellRange(xIndex->getByIndex(nTable), uno::UNO_QUERY);
                    uno::Reference<text::XSimpleText> xCellText(xCellRange->getCellByPosition(aPos.Col(), aPos.Row()), uno::UNO_QUERY);
                    xCursorProp.set(xCellText->createTextCursor(), uno::UNO_QUERY);
                }
                ScCellTextCursor* pCursor = comphelper::getUnoTunnelImplementation<ScCellTextCursor>( xCursorProp );
                if (!pCursor)
                    continue;
                pCursor->SetSelection( rTextEntry.maSelection );

                std::vector<XMLPropertyState> aPropStates(xTextPropMapper->Filter(*this, xCursorProp));
                OUString sName( rTextEntry.maName );
                GetAutoStylePool()->AddNamed(sName, XmlStyleFamily::TEXT_TEXT, OUString(), aPropStates);
                GetAutoStylePool()->RegisterName(XmlStyleFamily::TEXT_TEXT, sName);
                xPrevCursorProp = xCursorProp;
                aPrevPos = aPos;
            }
        }

        ExportExternalRefCacheStyles();

        if (!pSharedData)
        {
            SCTAB nTableCount(0);
            sal_Int32 nShapesCount(0);
            CollectSharedData(nTableCount, nShapesCount);
        }
        sal_Int32 nTableCount(xIndex->getCount());
        pCellStyles->AddNewTable(nTableCount - 1);
        CollectShapesAutoStyles(nTableCount);
        for (sal_Int32 nTable = 0; nTable < nTableCount; ++nTable, IncrementProgressBar(false))
        {
            uno::Reference <sheet::XSpreadsheet> xTable(xIndex->getByIndex(nTable), uno::UNO_QUERY);
            if (!xTable.is())
                continue;

            // table styles array must be complete, including copied tables - Add should find the stored style
            uno::Reference<beans::XPropertySet> xTableProperties(xTable, uno::UNO_QUERY);
            if (xTableProperties.is())
            {
                std::vector<XMLPropertyState> aPropStates(xTableStylesExportPropertySetMapper->Filter(*this, xTableProperties));
                if(!aPropStates.empty())
                {
                    OUString sName;
                    GetAutoStylePool()->Add(sName, XmlStyleFamily::TABLE_TABLE, OUString(), aPropStates);
                    aTableStyles.push_back(sName);
                }
            }

            // collect other auto-styles only for non-copied sheets
            uno::Reference<sheet::XUniqueCellFormatRangesSupplier> xCellFormatRanges ( xTable, uno::UNO_QUERY );
            if ( xCellFormatRanges.is() )
            {
                uno::Reference<container::XIndexAccess> xFormatRangesIndex(xCellFormatRanges->getUniqueCellFormatRanges());
                if (xFormatRangesIndex.is())
                {
                    sal_Int32 nFormatRangesCount(xFormatRangesIndex->getCount());
                    GetProgressBarHelper()->ChangeReference(GetProgressBarHelper()->GetReference() + nFormatRangesCount);
                    for (sal_Int32 nFormatRange = 0; nFormatRange < nFormatRangesCount; ++nFormatRange)
                    {
                        uno::Reference< sheet::XSheetCellRanges> xCellRanges(xFormatRangesIndex->getByIndex(nFormatRange), uno::UNO_QUERY);
                        if (xCellRanges.is())
                        {
                            uno::Reference <beans::XPropertySet> xProperties (xCellRanges, uno::UNO_QUERY);
                            if (xProperties.is())
                            {
                                AddStyleFromCells(xProperties, xTable, nTable, nullptr);
                                IncrementProgressBar(false);
                            }
                        }
                    }
                }
            }
            uno::Reference<table::XColumnRowRange> xColumnRowRange (xTable, uno::UNO_QUERY);
            if (xColumnRowRange.is() && pDoc)
            {
                pDoc->SyncColRowFlags();
                uno::Reference<table::XTableColumns> xTableColumns(xColumnRowRange->getColumns());
                if (xTableColumns.is())
                {
                    sal_Int32 nColumns(pDoc->GetLastChangedCol(sal::static_int_cast<SCTAB>(nTable)));
                    pSharedData->SetLastColumn(nTable, nColumns);
                    table::CellRangeAddress aCellAddress(GetEndAddress(xTable));
                    if (aCellAddress.EndColumn > nColumns)
                    {
                        ++nColumns;
                        pColumnStyles->AddNewTable(nTable, aCellAddress.EndColumn);
                    }
                    else
                        pColumnStyles->AddNewTable(nTable, nColumns);
                    sal_Int32 nColumn = 0;
                    while (nColumn <= pDoc->MaxCol())
                    {
                        sal_Int32 nIndex(-1);
                        bool bIsVisible(true);
                        uno::Reference <beans::XPropertySet> xColumnProperties(xTableColumns->getByIndex(nColumn), uno::UNO_QUERY);
                        if (xColumnProperties.is())
                        {
                            AddStyleFromColumn( xColumnProperties, nullptr, nIndex, bIsVisible );
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
                uno::Reference<table::XTableRows> xTableRows(xColumnRowRange->getRows());
                if (xTableRows.is())
                {
                    sal_Int32 nRows(pDoc->GetLastChangedRow(sal::static_int_cast<SCTAB>(nTable)));
                    pSharedData->SetLastRow(nTable, nRows);

                    pRowStyles->AddNewTable(nTable, pDoc->MaxRow());
                    sal_Int32 nRow = 0;
                    while (nRow <= pDoc->MaxRow())
                    {
                        sal_Int32 nIndex = 0;
                        uno::Reference <beans::XPropertySet> xRowProperties(xTableRows->getByIndex(nRow), uno::UNO_QUERY);
                        if(xRowProperties.is())
                        {
                            AddStyleFromRow( xRowProperties, nullptr, nIndex );
                            pRowStyles->AddFieldStyleName(nTable, nRow, nIndex);
                        }
                        sal_Int32 nOld(nRow);
                        nRow = pDoc->GetNextDifferentChangedRow(sal::static_int_cast<SCTAB>(nTable), static_cast<SCROW>(nRow));
                        if (nRow > nOld + 1)
                            pRowStyles->AddFieldStyleName(nTable, nOld + 1, nIndex, nRow - 1);
                    }
                }
            }
            ExportCellTextAutoStyles(nTable);
        }

        pChangeTrackingExportHelper->CollectAutoStyles();
    }

    if (getExportFlags() & SvXMLExportFlags::MASTERSTYLES)
        GetPageExport()->collectAutoStyles(true);

    mbAutoStylesCollected = true;
}

void ScXMLExport::ExportAutoStyles_()
{
    if (!GetModel().is())
        return;

    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( GetModel(), uno::UNO_QUERY );
    if (!xSpreadDoc.is())
        return;

    uno::Reference<container::XIndexAccess> xIndex( xSpreadDoc->getSheets(), uno::UNO_QUERY );
    if (!xIndex.is())
        return;

    collectAutoStyles();

    if (getExportFlags() & SvXMLExportFlags::CONTENT)
    {
        GetAutoStylePool()->exportXML(XmlStyleFamily::TABLE_COLUMN);
        GetAutoStylePool()->exportXML(XmlStyleFamily::TABLE_ROW);
        GetAutoStylePool()->exportXML(XmlStyleFamily::TABLE_TABLE);
        exportAutoDataStyles();
        GetAutoStylePool()->exportXML(XmlStyleFamily::TABLE_CELL);

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

    if (getExportFlags() & SvXMLExportFlags::MASTERSTYLES)
    {
        exportAutoDataStyles();
        GetPageExport()->exportAutoStyles();
    }

    // #i30251#; only write Text Styles once

    if ((getExportFlags() & SvXMLExportFlags::CONTENT) || (getExportFlags() & SvXMLExportFlags::MASTERSTYLES))
        GetTextParagraphExport()->exportTextAutoStyles();
}

void ScXMLExport::ExportMasterStyles_()
{
    GetPageExport()->exportMasterStyles( true );
}

void ScXMLExport::CollectInternalShape( uno::Reference< drawing::XShape > const & xShape )
{
    // detective objects and notes
    SvxShape* pShapeImp = comphelper::getUnoTunnelImplementation<SvxShape>( xShape );
    if( !pShapeImp )
        return;

    SdrObject* pObject = pShapeImp->GetSdrObject();
    if( !pObject )
        return;

    // collect note caption objects from all layers (internal or hidden)
    if( ScDrawObjData* pCaptData = ScDrawLayer::GetNoteCaptionData( pObject, static_cast< SCTAB >( nCurrentTable ) ) )
    {
        if(pDoc->GetNote(pCaptData->maStart))
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
        ScDetectiveFunc aDetFunc( *pDoc, static_cast<SCTAB>(nCurrentTable) );
        ScAddress       aPosition;
        ScRange         aSourceRange;
        bool            bRedLine;
        ScDetectiveObjType eObjType = aDetFunc.GetDetectiveObjectType(
            pObject, nCurrentTable, aPosition, aSourceRange, bRedLine );
        pSharedData->GetDetectiveObjContainer()->AddObject( eObjType, static_cast<SCTAB>(nCurrentTable), aPosition, aSourceRange, bRedLine );
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
                ScRange aScRange( aCellAddress2.StartColumn, aCellAddress2.StartRow, aCellAddress2.Sheet,
                                  aCellAddress2.EndColumn, aCellAddress2.EndRow, aCellAddress2.Sheet );

                if ((aScRange.aEnd.Row() > nRow ||
                    aScRange.aEnd.Col() > nCol) &&
                    aScRange.aStart.Row() == nRow &&
                    aScRange.aStart.Col() == nCol)
                {
                    pMergedRangesContainer->AddRange(aScRange);
                    pSharedData->SetLastColumn(aScRange.aEnd.Tab(), aScRange.aEnd.Col());
                    pSharedData->SetLastRow(aScRange.aEnd.Tab(), aScRange.aEnd.Row());
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
                            ScRange& aCellAddress, bool& bIsFirst) const
{
    bIsFirst = false;

    ScRange aMatrixRange;

    if (pDoc && pDoc->GetMatrixFormulaRange(aCell, aMatrixRange))
    {
        aCellAddress = aMatrixRange;
        if ((aCellAddress.aStart.Col() == aCell.Col() && aCellAddress.aStart.Row() == aCell.Row()) &&
            (aCellAddress.aEnd.Col() > aCell.Col() || aCellAddress.aEnd.Row() > aCell.Row()))
        {
            bIsFirst = true;
            return true;
        }
        else if (aCellAddress.aStart.Col() != aCell.Col() || aCellAddress.aStart.Row() != aCell.Row() ||
            aCellAddress.aEnd.Col() != aCell.Col() || aCellAddress.aEnd.Row()!= aCell.Row())
            return true;
        else
        {
            bIsFirst = true;
            return true;
        }
    }

    return false;
}

void ScXMLExport::WriteTable(sal_Int32 nTable, const uno::Reference<sheet::XSpreadsheet>& xTable)
{
    if (!xTable.is())
        return;

    xCurrentTable.set(xTable);
    uno::Reference<container::XNamed> xName (xTable, uno::UNO_QUERY );
    if (!xName.is())
        return;

    nCurrentTable = sal::static_int_cast<sal_uInt16>( nTable );
    OUString sOUTableName(xName->getName());
    AddAttribute(sAttrName, sOUTableName);
    AddAttribute(sAttrStyleName, aTableStyles[nTable]);

    uno::Reference<util::XProtectable> xProtectable (xTable, uno::UNO_QUERY);
    ScTableProtection* pProtect = nullptr;
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
                    ::comphelper::Base64::encode(aBuffer,
                        pProtect->getPasswordHash(PASSHASH_SHA1));
                    eHashUsed = PASSHASH_SHA1;
                }
                else if (pProtect->hasPasswordHash(PASSHASH_SHA256))
                {
                    ::comphelper::Base64::encode(aBuffer,
                        pProtect->getPasswordHash(PASSHASH_SHA256));
                    eHashUsed = PASSHASH_SHA256;
                }
                else if (pProtect->hasPasswordHash(PASSHASH_XL, PASSHASH_SHA1))
                {
                    // Double-hash this by SHA1 on top of the legacy xls hash.
                    uno::Sequence<sal_Int8> aHash = pProtect->getPasswordHash(PASSHASH_XL, PASSHASH_SHA1);
                    ::comphelper::Base64::encode(aBuffer, aHash);
                    eHashUsed = PASSHASH_XL;
                }
                if (!aBuffer.isEmpty())
                {
                    AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY, aBuffer.makeStringAndClear());
                    if (getSaneDefaultVersion() >= SvtSaveOptions::ODFSVER_012)
                    {
                        if (eHashUsed == PASSHASH_XL)
                        {
                            AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY_DIGEST_ALGORITHM,
                                         ScPassHashHelper::getHashURI(PASSHASH_XL));
                            if (getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
                                AddAttribute(XML_NAMESPACE_LO_EXT, XML_PROTECTION_KEY_DIGEST_ALGORITHM_2,
                                        ScPassHashHelper::getHashURI(PASSHASH_SHA1));
                        }
                        else if (eHashUsed == PASSHASH_SHA1)
                        {
                            AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY_DIGEST_ALGORITHM,
                                         ScPassHashHelper::getHashURI(PASSHASH_SHA1));
                        }
                        else if (eHashUsed == PASSHASH_SHA256)
                        {
                            AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY_DIGEST_ALGORITHM,
                                         ScPassHashHelper::getHashURI(PASSHASH_SHA256));
                        }
                    }
                }
            }
        }
    }
    OUString sPrintRanges;
    ScRange aColumnHeaderRange;
    bool bHasColumnHeader;
    GetColumnRowHeader(bHasColumnHeader, aColumnHeaderRange, bHasRowHeader, aRowHeaderRange, sPrintRanges);
    if( !sPrintRanges.isEmpty() )
        AddAttribute( XML_NAMESPACE_TABLE, XML_PRINT_RANGES, sPrintRanges );
    else if (pDoc && !pDoc->IsPrintEntireSheet(static_cast<SCTAB>(nTable)))
        AddAttribute( XML_NAMESPACE_TABLE, XML_PRINT, XML_FALSE);
    SvXMLElementExport aElemT(*this, sElemTab, true, true);

    if (pProtect && pProtect->isProtected() && getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
    {
        if (pProtect->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS))
            AddAttribute(XML_NAMESPACE_LO_EXT, XML_SELECT_PROTECTED_CELLS, XML_TRUE);
        if (pProtect->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS))
            AddAttribute(XML_NAMESPACE_LO_EXT, XML_SELECT_UNPROTECTED_CELLS, XML_TRUE);

        if (pProtect->isOptionEnabled(ScTableProtection::INSERT_COLUMNS))
            AddAttribute(XML_NAMESPACE_LO_EXT, XML_INSERT_COLUMNS, XML_TRUE);
        if (pProtect->isOptionEnabled(ScTableProtection::INSERT_ROWS))
            AddAttribute(XML_NAMESPACE_LO_EXT, XML_INSERT_ROWS, XML_TRUE);

        if (pProtect->isOptionEnabled(ScTableProtection::DELETE_COLUMNS))
            AddAttribute(XML_NAMESPACE_LO_EXT, XML_DELETE_COLUMNS, XML_TRUE);
        if (pProtect->isOptionEnabled(ScTableProtection::DELETE_ROWS))
            AddAttribute(XML_NAMESPACE_LO_EXT, XML_DELETE_ROWS, XML_TRUE);

        OUString aElemName = GetNamespaceMap().GetQNameByKey(
            XML_NAMESPACE_LO_EXT, GetXMLToken(XML_TABLE_PROTECTION));

        SvXMLElementExport aElemProtected(*this, aElemName, true, true);
    }

    CheckAttrList();

    if ( pDoc && pDoc->GetSheetEvents( static_cast<SCTAB>(nTable) ) &&
        getSaneDefaultVersion() >= SvtSaveOptions::ODFSVER_012)
    {
        // store sheet events
        uno::Reference<document::XEventsSupplier> xSupplier(xTable, uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xEvents = xSupplier->getEvents();
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
    }
    if (pSharedData->HasDrawPage())
    {
        GetShapeExport()->seekShapes(pSharedData->GetDrawPage(nTable));
        WriteTableShapes();
    }
    table::CellRangeAddress aRange(GetEndAddress(xTable));
    pSharedData->SetLastColumn(nTable, aRange.EndColumn);
    pSharedData->SetLastRow(nTable, aRange.EndRow);
    mpCellsItr->SetCurrentTable(static_cast<SCTAB>(nTable), xCurrentTable);
    pGroupColumns->NewTable();
    pGroupRows->NewTable();
    FillColumnRowGroups();
    if (bHasColumnHeader)
        pSharedData->SetLastColumn(nTable, aColumnHeaderRange.aEnd.Col());
    bRowHeaderOpen = false;
    if (bHasRowHeader)
        pSharedData->SetLastRow(nTable, aRowHeaderRange.aEnd.Row());
    pDefaults->FillDefaultStyles(nTable, pSharedData->GetLastRow(nTable),
        pSharedData->GetLastColumn(nTable), pCellStyles.get(), pDoc);
    pRowFormatRanges->SetColDefaults(&pDefaults->GetColDefaults());
    pCellStyles->SetColDefaults(&pDefaults->GetColDefaults());
    ExportColumns(nTable, aColumnHeaderRange, bHasColumnHeader);
    bool bIsFirst(true);
    sal_Int32 nEqualCells(0);
    ScMyCell aCell;
    ScMyCell aPrevCell;
    while (mpCellsItr->GetNext(aCell, pCellStyles.get()))
    {
        if (bIsFirst)
        {
            ExportFormatRanges(0, 0, aCell.maCellAddress.Col()-1, aCell.maCellAddress.Row(), nTable);
            aPrevCell = aCell;
            bIsFirst = false;
        }
        else
        {
            if ((aPrevCell.maCellAddress.Row() == aCell.maCellAddress.Row()) &&
                (aPrevCell.maCellAddress.Col() + nEqualCells + 1 == aCell.maCellAddress.Col()))
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
                ExportFormatRanges(aPrevCell.maCellAddress.Col() + nEqualCells + 1, aPrevCell.maCellAddress.Row(),
                    aCell.maCellAddress.Col()-1, aCell.maCellAddress.Row(), nTable);
                nEqualCells = 0;
                aPrevCell = aCell;
            }
        }
    }
    if (!bIsFirst)
    {
        WriteCell(aPrevCell, nEqualCells);
        ExportFormatRanges(aPrevCell.maCellAddress.Col() + nEqualCells + 1, aPrevCell.maCellAddress.Row(),
            pSharedData->GetLastColumn(nTable), pSharedData->GetLastRow(nTable), nTable);
    }
    else
        ExportFormatRanges(0, 0, pSharedData->GetLastColumn(nTable), pSharedData->GetLastRow(nTable), nTable);

    CloseRow(pSharedData->GetLastRow(nTable));

    if (!pDoc)
        return;

    // Export sheet-local named ranges.
    ScRangeName* pRangeName = pDoc->GetRangeName(nTable);
    if (pRangeName && !pRangeName->empty())
    {
        WriteNamedRange(pRangeName);
    }

    if (getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
    {
        //export new conditional format information
        ExportConditionalFormat(nTable);
    }
}

namespace {

void writeContent(
    ScXMLExport& rExport, const OUString& rStyleName, const OUString& rContent, const SvxFieldData* pField )
{
    std::unique_ptr<SvXMLElementExport> pElem;
    if (!rStyleName.isEmpty())
    {
        // Formatted section with automatic style.
        rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_STYLE_NAME, rStyleName);
        OUString aElemName = rExport.GetNamespaceMap().GetQNameByKey(
            XML_NAMESPACE_TEXT, GetXMLToken(XML_SPAN));
        pElem.reset(new SvXMLElementExport(rExport, aElemName, false, false));
    }

    if (pField)
    {
        // Write a field item.
        OUString aFieldVal = ScEditUtil::GetCellFieldValue(*pField, rExport.GetDocument(), nullptr);
        switch (pField->GetClassId())
        {
            case text::textfield::Type::URL:
            {
                // <text:a xlink:href="url" xlink:type="simple">value</text:a>

                const SvxURLField* pURLField = static_cast<const SvxURLField*>(pField);
                const OUString& aURL = pURLField->GetURL();
                rExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, rExport.GetRelativeReference(aURL));
                rExport.AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, "simple");
                const OUString& aTargetFrame = pURLField->GetTargetFrame();
                if (!aTargetFrame.isEmpty())
                    rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_TARGET_FRAME_NAME, aTargetFrame);

                OUString aElemName = rExport.GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_TEXT, GetXMLToken(XML_A));
                SvXMLElementExport aElem(rExport, aElemName, false, false);
                rExport.Characters(aFieldVal);
            }
            break;
            case text::textfield::Type::DATE:
            {
                // <text:date style:data-style-name="N2" text:date-value="YYYY-MM-DD">value</text:date>

                Date aDate(Date::SYSTEM);
                OUStringBuffer aBuf;
                sal_Int32 nVal = aDate.GetYear();
                aBuf.append(nVal);
                aBuf.append('-');
                nVal = aDate.GetMonth();
                if (nVal < 10)
                    aBuf.append('0');
                aBuf.append(nVal);
                aBuf.append('-');
                nVal = aDate.GetDay();
                if (nVal < 10)
                    aBuf.append('0');
                aBuf.append(nVal);
                rExport.AddAttribute(XML_NAMESPACE_STYLE, XML_DATA_STYLE_NAME, "N2");
                rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_DATE_VALUE, aBuf.makeStringAndClear());

                OUString aElemName = rExport.GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_TEXT, GetXMLToken(XML_DATE));
                SvXMLElementExport aElem(rExport, aElemName, false, false);
                rExport.Characters(aFieldVal);
            }
            break;
            case text::textfield::Type::DOCINFO_TITLE:
            {
                // <text:title>value</text:title>

                OUString aElemName = rExport.GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_TEXT, GetXMLToken(XML_TITLE));
                SvXMLElementExport aElem(rExport, aElemName, false, false);
                rExport.Characters(aFieldVal);
            }
            break;
            case text::textfield::Type::TABLE:
            {
                // <text:sheet-name>value</text:sheet-name>

                OUString aElemName = rExport.GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_TEXT, GetXMLToken(XML_SHEET_NAME));
                SvXMLElementExport aElem(rExport, aElemName, false, false);
                rExport.Characters(aFieldVal);
            }
            break;
            default:
                rExport.Characters(aFieldVal);
        }
    }
    else
        rExport.Characters(rContent);
}

void flushParagraph(
    ScXMLExport& rExport, const OUString& rParaText,
    rtl::Reference<XMLPropertySetMapper> const & xMapper, rtl::Reference<SvXMLAutoStylePoolP> const & xStylePool,
    const ScXMLEditAttributeMap& rAttrMap,
    std::vector<editeng::Section>::const_iterator it, std::vector<editeng::Section>::const_iterator const & itEnd )
{
    OUString aElemName = rExport.GetNamespaceMap().GetQNameByKey(
        XML_NAMESPACE_TEXT, GetXMLToken(XML_P));
    SvXMLElementExport aElemP(rExport, aElemName, false, false);

    for (; it != itEnd; ++it)
    {
        const editeng::Section& rSec = *it;

        OUString aContent(rParaText.copy(rSec.mnStart, rSec.mnEnd - rSec.mnStart));

        std::vector<XMLPropertyState> aPropStates;
        const SvxFieldData* pField = toXMLPropertyStates(aPropStates, rSec.maAttributes, xMapper, rAttrMap);
        OUString aStyleName = xStylePool->Find(XmlStyleFamily::TEXT_TEXT, OUString(), aPropStates);
        writeContent(rExport, aStyleName, aContent, pField);
    }
}

}

void ScXMLExport::WriteCell(ScMyCell& aCell, sal_Int32 nEqualCellCount)
{
    // nEqualCellCount is the number of additional cells
    SetRepeatAttribute(nEqualCellCount, (aCell.nType != table::CellContentType_EMPTY));

    if (aCell.nStyleIndex != -1)
        AddAttribute(sAttrStyleName, pCellStyles->GetStyleNameByIndex(aCell.nStyleIndex, aCell.bIsAutoStyle));
    if (aCell.nValidationIndex > -1)
        AddAttribute(XML_NAMESPACE_TABLE, XML_CONTENT_VALIDATION_NAME, pValidationsContainer->GetValidationName(aCell.nValidationIndex));
    const bool bIsFirstMatrixCell(aCell.bIsMatrixBase);
    if (bIsFirstMatrixCell)
    {
        SCCOL nColumns( aCell.aMatrixRange.aEnd.Col() - aCell.aMatrixRange.aStart.Col() + 1 );
        SCROW nRows( aCell.aMatrixRange.aEnd.Row() - aCell.aMatrixRange.aStart.Row() + 1 );
        AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_MATRIX_COLUMNS_SPANNED, OUString::number(nColumns));
        AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_MATRIX_ROWS_SPANNED, OUString::number(nRows));
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
                GetNumberFormatAttributesExportHelper()->SetNumberFormatAttributes(
                    aCell.nNumberFormat, aCell.maBaseCell.mfValue);
                if (getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
                    GetNumberFormatAttributesExportHelper()->SetNumberFormatAttributes(
                            aCell.nNumberFormat, aCell.maBaseCell.mfValue, false, XML_NAMESPACE_CALC_EXT, false);
            }
            break;
        case table::CellContentType_TEXT :
            {
                OUString sFormattedString(lcl_GetFormattedString(pDoc, aCell.maBaseCell, aCell.maCellAddress));
                OUString sCellString = aCell.maBaseCell.getString(pDoc);
                GetNumberFormatAttributesExportHelper()->SetNumberFormatAttributes(
                        sCellString, sFormattedString);
                if (getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
                    GetNumberFormatAttributesExportHelper()->SetNumberFormatAttributes(
                            sCellString, sFormattedString, false, XML_NAMESPACE_CALC_EXT);
            }
            break;
        case table::CellContentType_FORMULA :
            {
                if (aCell.maBaseCell.meType == CELLTYPE_FORMULA)
                {
                    const bool bIsMatrix(bIsFirstMatrixCell || aCell.bIsMatrixCovered);
                    ScFormulaCell* pFormulaCell = aCell.maBaseCell.mpFormula;
                    if (!bIsMatrix || bIsFirstMatrixCell)
                    {
                        if (!mpCompileFormulaCxt)
                        {
                            const formula::FormulaGrammar::Grammar eGrammar = pDoc->GetStorageGrammar();
                            mpCompileFormulaCxt.reset(new sc::CompileFormulaContext(*pDoc, eGrammar));
                        }

                        OUString aFormula = pFormulaCell->GetFormula(*mpCompileFormulaCxt);
                        sal_uInt16 nNamespacePrefix =
                            (mpCompileFormulaCxt->getGrammar() == formula::FormulaGrammar::GRAM_ODFF ? XML_NAMESPACE_OF : XML_NAMESPACE_OOOC);

                        if (!bIsMatrix)
                        {
                            AddAttribute(sAttrFormula, GetNamespaceMap().GetQNameByKey(nNamespacePrefix, aFormula, false));
                        }
                        else
                        {
                            AddAttribute(sAttrFormula, GetNamespaceMap().GetQNameByKey(nNamespacePrefix, aFormula.copy(1, aFormula.getLength()-2), false));
                        }
                    }
                    if (pFormulaCell->GetErrCode() != FormulaError::NONE)
                    {
                        AddAttribute(sAttrValueType, XML_STRING);
                        AddAttribute(sAttrStringValue, aCell.maBaseCell.getString(pDoc));
                        if (getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
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
                                    aCell.nNumberFormat, pDoc->GetValue(aCell.maCellAddress));
                            if (getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
                            {
                                GetNumberFormatAttributesExportHelper()->SetNumberFormatAttributes(
                                        aCell.nNumberFormat, pDoc->GetValue(aCell.maCellAddress), false, XML_NAMESPACE_CALC_EXT, false );
                            }
                        }
                    }
                    else
                    {
                        if (!aCell.maBaseCell.getString(pDoc).isEmpty())
                        {
                            AddAttribute(sAttrValueType, XML_STRING);
                            AddAttribute(sAttrStringValue, aCell.maBaseCell.getString(pDoc));
                            if (getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
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
            SCCOL nColumns( aCell.aMergeRange.aEnd.Col() - aCell.aMergeRange.aStart.Col() + 1 );
            SCROW nRows( aCell.aMergeRange.aEnd.Row() - aCell.aMergeRange.aStart.Row() + 1 );
            AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_SPANNED, OUString::number(nColumns));
            AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_ROWS_SPANNED, OUString::number(nRows));
        }
    }
    SvXMLElementExport aElemC(*this, *pCellString, true, true);
    CheckAttrList();
    WriteAreaLink(aCell);
    WriteAnnotation(aCell);
    WriteDetective(aCell);

    if (!bIsEmpty)
    {
        if (aCell.maBaseCell.meType == CELLTYPE_EDIT)
        {
            WriteEditCell(aCell.maBaseCell.mpEditText);
        }
        else if (aCell.maBaseCell.meType == CELLTYPE_FORMULA && aCell.maBaseCell.mpFormula->IsMultilineResult())
        {
            WriteMultiLineFormulaResult(aCell.maBaseCell.mpFormula);
        }
        else
        {
            SvXMLElementExport aElemP(*this, sElemP, true, false);

            OUString aParaStr =
                ScCellFormat::GetOutputString(*pDoc, aCell.maCellAddress, aCell.maBaseCell);

            bool bPrevCharWasSpace = true;
            GetTextParagraphExport()->exportCharacterData(aParaStr, bPrevCharWasSpace);
        }
    }
    WriteShapes(aCell);
    if (!bIsEmpty)
        IncrementProgressBar(false);
}

void ScXMLExport::WriteEditCell(const EditTextObject* pText)
{
    rtl::Reference<XMLPropertySetMapper> xMapper = GetTextParagraphExport()->GetTextPropMapper()->getPropertySetMapper();
    rtl::Reference<SvXMLAutoStylePoolP> xStylePool = GetAutoStylePool();
    const ScXMLEditAttributeMap& rAttrMap = GetEditAttributeMap();

    // Get raw paragraph texts first.
    std::vector<OUString> aParaTexts;
    sal_Int32 nParaCount = pText->GetParagraphCount();
    aParaTexts.reserve(nParaCount);
    for (sal_Int32 i = 0; i < nParaCount; ++i)
        aParaTexts.push_back(pText->GetText(i));

    // Get all section data and iterate through them.
    std::vector<editeng::Section> aAttrs;
    pText->GetAllSections(aAttrs);
    std::vector<editeng::Section>::const_iterator itSec = aAttrs.begin(), itSecEnd = aAttrs.end();
    std::vector<editeng::Section>::const_iterator itPara = itSec;
    sal_Int32 nCurPara = 0; // current paragraph
    for (; itSec != itSecEnd; ++itSec)
    {
        const editeng::Section& rSec = *itSec;
        if (nCurPara == rSec.mnParagraph)
            // Still in the same paragraph.
            continue;

        // Start of a new paragraph. Flush the old paragraph.
        flushParagraph(*this, aParaTexts[nCurPara], xMapper, xStylePool, rAttrMap, itPara, itSec);
        nCurPara = rSec.mnParagraph;
        itPara = itSec;
    }

    flushParagraph(*this, aParaTexts[nCurPara], xMapper, xStylePool, rAttrMap, itPara, itSecEnd);
}

void ScXMLExport::WriteMultiLineFormulaResult(const ScFormulaCell* pCell)
{
    OUString aElemName = GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TEXT, GetXMLToken(XML_P));

    OUString aResStr = pCell->GetResultString().getString();
    const sal_Unicode* p = aResStr.getStr();
    const sal_Unicode* pEnd = p + static_cast<size_t>(aResStr.getLength());
    const sal_Unicode* pPara = p; // paragraph head.
    for (; p != pEnd; ++p)
    {
        if (*p != '\n')
            continue;

        // flush the paragraph.
        OUString aContent;
        if (*pPara == '\n')
            ++pPara;
        if (p > pPara)
            aContent = OUString(pPara, p-pPara);

        SvXMLElementExport aElem(*this, aElemName, false, false);
        Characters(aContent);

        pPara = p;
    }

    OUString aContent;
    if (*pPara == '\n')
        ++pPara;
    if (pEnd > pPara)
        aContent = OUString(pPara, pEnd-pPara);

    SvXMLElementExport aElem(*this, aElemName, false, false);
    Characters(aContent);
}

void ScXMLExport::ExportShape(const uno::Reference < drawing::XShape >& xShape, awt::Point* pPoint)
{
    uno::Reference < beans::XPropertySet > xShapeProps ( xShape, uno::UNO_QUERY );
    bool bIsChart( false );
    if (xShapeProps.is())
    {
        sal_Int32 nZOrder = 0;
        if (xShapeProps->getPropertyValue("ZOrder") >>= nZOrder)
        {
            AddAttribute(XML_NAMESPACE_DRAW, XML_ZINDEX, OUString::number(nZOrder));
        }
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo = xShapeProps->getPropertySetInfo();
        OUString sPropCLSID ("CLSID");
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
                        xShapeProps->getPropertyValue( "PersistName" ) >>= aChartName;
                        ScChartListenerCollection* pCollection = pDoc->GetChartListenerCollection();
                        if (pCollection)
                        {
                            ScChartListener* pListener = pCollection->findByName(aChartName);
                            if (pListener)
                            {
                                const ScRangeListRef& rRangeList = pListener->GetRangeList();
                                if ( rRangeList.is() )
                                {
                                    ScRangeStringConverter::GetStringFromRangeList( sRanges, rRangeList.get(), pDoc, FormulaGrammar::CONV_OOO );
                                    if ( !sRanges.isEmpty() )
                                    {
                                        bIsChart = true;
                                        SvXMLAttributeList* pAttrList = new SvXMLAttributeList();
                                        pAttrList->AddAttribute(
                                            GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_DRAW, GetXMLToken( XML_NOTIFY_ON_UPDATE_OF_RANGES ) ), sRanges );
                                        GetShapeExport()->exportShape( xShape, SEF_DEFAULT, pPoint, pAttrList );
                                    }
                                }
                            }
                        }
                    }

                    if ( sRanges.isEmpty() )
                    {
                        uno::Reference< frame::XModel > xChartModel;
                        if( ( xShapeProps->getPropertyValue( "Model" ) >>= xChartModel ) &&
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
                                SvXMLAttributeList* pAttrList = nullptr;
                                if(aRepresentations.hasElements())
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
                                GetShapeExport()->exportShape(xShape, SEF_DEFAULT, pPoint, pAttrList);
                            }
                        }
                    }
                }
            }
        }
    }
    if (!bIsChart)
    {
        OUString sHlink;
        try
        {
            uno::Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );
            if ( xProps.is() )
                xProps->getPropertyValue( SC_UNONAME_HYPERLINK ) >>= sHlink;
        }
        catch ( const beans::UnknownPropertyException& )
        {
            // no hyperlink property
        }

        std::unique_ptr< SvXMLElementExport > pDrawA;
        // enclose shapes with <draw:a> element only if sHlink contains something
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
    if( !(rMyCell.bHasShape && !rMyCell.aShapeList.empty() && pDoc) )
        return;

    // Reference point to turn absolute coordinates in reference point + offset. That happens in most
    // cases in XMLShapeExport::ImpExportNewTrans_DecomposeAndRefPoint, which gets the absolute
    // coordinates as translation from matrix in property "Transformation". For cell anchored shapes
    // the reference point is left-top (in LTR mode) of that cell, which contains the shape.
    tools::Rectangle aCellRectFull = pDoc->GetMMRect(
        rMyCell.maCellAddress.Col(), rMyCell.maCellAddress.Row(), rMyCell.maCellAddress.Col(),
        rMyCell.maCellAddress.Row(), rMyCell.maCellAddress.Tab(), false /*bHiddenAsZero*/);
    awt::Point aPoint;
    bool bNegativePage = pDoc->IsNegativePage(rMyCell.maCellAddress.Tab());
    if (bNegativePage)
        aPoint.X = aCellRectFull.Right();
    else
        aPoint.X = aCellRectFull.Left();
    aPoint.Y = aCellRectFull.Top();

    for (const auto& rShape : rMyCell.aShapeList)
    {
        if (rShape.xShape.is())
        {
            // The current object geometry is based on bHiddenAsZero=true, but ODF file format
            // needs it as if there were no hidden rows or columns. We manipulate the geometry
            // accordingly for writing xml markup and restore geometry later.
            bool bNeedsRestore = false;
            SdrObject* pObj = GetSdrObjectFromXShape(rShape.xShape);
            // Remember original geometry
            SdrObjGeoData* pGeoData = nullptr;
            if (pObj)
                pGeoData = pObj->GetGeoData();

            // Hiding row or column affects the shape based on its snap rect. So we need start and
            // end cell address of snap rect. In case of a transformed shape, it is not in rMyCell.
            ScAddress aSnapStartAddress = rMyCell.maCellAddress;
            ScDrawObjData* pObjData = nullptr;
            bool bIsShapeTransformed = false;
            if (pObj)
            {
                pObjData = ScDrawLayer::GetObjData(pObj);
                bIsShapeTransformed = pObj->GetRotateAngle() != 0 || pObj->GetShearAngle() != 0;
            }
            if (bIsShapeTransformed && pObjData)
                aSnapStartAddress = pObjData->maStart;

            // In case rows or columns are hidden above or before the snap rect, move the shape to the
            // position it would have, if these rows and columns are visible.
            tools::Rectangle aRectFull = pDoc->GetMMRect(
                aSnapStartAddress.Col(), aSnapStartAddress.Row(), aSnapStartAddress.Col(),
                aSnapStartAddress.Row(), aSnapStartAddress.Tab(), false /*bHiddenAsZero*/);
            tools::Rectangle aRectReduced = pDoc->GetMMRect(
                aSnapStartAddress.Col(), aSnapStartAddress.Row(), aSnapStartAddress.Col(),
                aSnapStartAddress.Row(), aSnapStartAddress.Tab(), true /*bHiddenAsZero*/);
            const tools::Long nLeftDiff(aRectFull.Left() - aRectReduced.Left());
            const tools::Long nTopDiff(aRectFull.Top() - aRectReduced.Top());
            if (pObj && (abs(nLeftDiff) > 1 || abs(nTopDiff) > 1))
            {
                bNeedsRestore = true;
                pObj->NbcMove(Size(nLeftDiff, nTopDiff));
            }

            // tdf#137033 In case the shape is anchored "To Cell (resize with cell)" hiding rows or
            // columns inside the snap rect has not only changed size of the shape but rotate and shear
            // angle too. We resize the shape to full size. That will recover the original angles too.
            if (rShape.bResizeWithCell && pObjData && pObj)
            {
                // Get original size from anchor
                const Point aSnapStartOffset = pObjData->maStartOffset;
                // In case of 'resize with cell' maEnd and maEndOffset should be valid.
                const ScAddress aSnapEndAddress(pObjData->maEnd);
                const Point aSnapEndOffset = pObjData->maEndOffset;
                const tools::Rectangle aStartCellRect = pDoc->GetMMRect(
                    aSnapStartAddress.Col(), aSnapStartAddress.Row(), aSnapStartAddress.Col(),
                    aSnapStartAddress.Row(), aSnapStartAddress.Tab(), false /*bHiddenAsZero*/);
                const tools::Rectangle aEndCellRect = pDoc->GetMMRect(
                    aSnapEndAddress.Col(), aSnapEndAddress.Row(), aSnapEndAddress.Col(),
                    aSnapEndAddress.Row(), aSnapEndAddress.Tab(), false /*bHiddenAsZero*/);
                if (bNegativePage)
                {
                    aRectFull.SetLeft(aEndCellRect.Right() - aSnapEndOffset.X());
                    aRectFull.SetRight(aStartCellRect.Right() - aSnapStartOffset.X());
                }
                else
                {
                    aRectFull.SetLeft(aStartCellRect.Left() + aSnapStartOffset.X());
                    aRectFull.SetRight(aEndCellRect.Left() + aSnapEndOffset.X());
                }
                aRectFull.SetTop(aStartCellRect.Top() + aSnapStartOffset.Y());
                aRectFull.SetBottom(aEndCellRect.Top() + aSnapEndOffset.Y());
                aRectReduced = pObjData->getShapeRect();
                if(abs(aRectFull.getWidth() - aRectReduced.getWidth()) > 1
                   || abs(aRectFull.getHeight() - aRectReduced.getHeight()) > 1)
                {
                    bNeedsRestore = true;
                    Fraction aScaleWidth(aRectFull.getWidth(), aRectReduced.getWidth());
                    if (!aScaleWidth.IsValid())
                        aScaleWidth = Fraction(1.0);
                    Fraction aScaleHeight(aRectFull.getHeight(), aRectReduced.getHeight());
                    if (!aScaleHeight.IsValid())
                        aScaleHeight = Fraction(1.0);
                    pObj->NbcResize(pObj->GetRelativePos(), aScaleWidth, aScaleHeight);
                }
            }

            // We only write the end address if we want the shape to resize with the cell
            if ( rShape.bResizeWithCell &&
                rShape.xShape->getShapeType() != "com.sun.star.drawing.CaptionShape" )
            {
                OUString sEndAddress;
                ScRangeStringConverter::GetStringFromAddress(sEndAddress, rShape.aEndAddress, pDoc, FormulaGrammar::CONV_OOO);
                AddAttribute(XML_NAMESPACE_TABLE, XML_END_CELL_ADDRESS, sEndAddress);
                OUStringBuffer sBuffer;
                GetMM100UnitConverter().convertMeasureToXML(
                        sBuffer, rShape.nEndX);
                AddAttribute(XML_NAMESPACE_TABLE, XML_END_X, sBuffer.makeStringAndClear());
                GetMM100UnitConverter().convertMeasureToXML(
                        sBuffer, rShape.nEndY);
                AddAttribute(XML_NAMESPACE_TABLE, XML_END_Y, sBuffer.makeStringAndClear());
            }

            // Correct above calculated reference point for some cases:
            // a) For a RTL-sheet translate from matrix is not suitable, because the shape
            // from xml (which is always LTR) is not mirrored to negative page but shifted.
            // b) In case of horizontal mirrored, 'resize with cell' anchored custom shape, translate
            // has wrong values. FixMe: Why is translate wrong?
            // c) Measure lines do not use transformation matrix but use start and end point directly.
            ScDrawObjData* pNRObjData = nullptr;
            if (pObj && bNegativePage
                && rShape.xShape->getShapeType() == "com.sun.star.drawing.MeasureShape")
            {
                // invers of shift when import
                tools::Rectangle aSnapRect = pObj->GetSnapRect();
                aPoint.X = aSnapRect.Left() + aSnapRect.Right() - aPoint.X;
            }
            else if (pObj && (pNRObjData = ScDrawLayer::GetNonRotatedObjData(pObj))
                     && ((rShape.bResizeWithCell && pObj->GetObjIdentifier() == OBJ_CUSTOMSHAPE
                          && static_cast<SdrObjCustomShape*>(pObj)->IsMirroredX())
                         || bNegativePage))
            {
                //In these cases we set reference Point = matrix translate - startOffset.
                awt::Point aMatrixTranslate = rShape.xShape->getPosition();
                aPoint.X = aMatrixTranslate.X - pNRObjData->maStartOffset.X();
                aPoint.Y = aMatrixTranslate.Y - pNRObjData->maStartOffset.Y();
            }

            ExportShape(rShape.xShape, &aPoint);

            // Restore object geometry
            if (bNeedsRestore && pObj && pGeoData)
                pObj->SetGeoData(*pGeoData);
        }
    }
}

void ScXMLExport::WriteTableShapes()
{
    ScMyTableShapes* pTableShapes(pSharedData->GetTableShapes());
    if (!pTableShapes || (*pTableShapes)[nCurrentTable].empty())
        return;

    OSL_ENSURE(pTableShapes->size() > static_cast<size_t>(nCurrentTable), "wrong Table");
    SvXMLElementExport aShapesElem(*this, XML_NAMESPACE_TABLE, XML_SHAPES, true, false);
    for (const auto& rxShape : (*pTableShapes)[nCurrentTable])
    {
        if (rxShape.is())
        {
            if (pDoc->IsNegativePage(static_cast<SCTAB>(nCurrentTable)))
            {
                // RTL-mirroring refers to snap rectangle, not to logic rectangle, therefore cannot use
                // getPosition() and getSize(), but need property "FrameRect" from rxShape or
                // GetSnapRect() from associated SdrObject.
                uno::Reference<beans::XPropertySet> xShapeProp(rxShape, uno::UNO_QUERY);
                awt::Rectangle aFrameRect;
                if (xShapeProp.is() && (xShapeProp->getPropertyValue("FrameRect") >>= aFrameRect))
                {
                    // file format uses shape in LTR mode. newLeft = - oldRight = - (oldLeft + width).
                    // newTranslate = oldTranslate - refPoint, oldTranslate from transformation matrix,
                    // calculated in XMLShapeExport::exportShape common for all modules.
                    // oldTranslate.X = oldLeft ==> refPoint.X = 2 * oldLeft + width
                    awt::Point aRefPoint;
                    aRefPoint.X = 2 * aFrameRect.X + aFrameRect.Width - 1;
                    aRefPoint.Y = 0;
                    ExportShape(rxShape, &aRefPoint);
                }
                // else should not happen
            }
            else
                ExportShape(rxShape, nullptr);
        }
    }
    (*pTableShapes)[nCurrentTable].clear();
}

void ScXMLExport::WriteAreaLink( const ScMyCell& rMyCell )
{
    if( !rMyCell.bHasAreaLink )
        return;

    const ScMyAreaLink& rAreaLink = rMyCell.aAreaLink;
    AddAttribute( XML_NAMESPACE_TABLE, XML_NAME, rAreaLink.sSourceStr );
    AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
    AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, GetRelativeReference(rAreaLink.sURL) );
    AddAttribute( XML_NAMESPACE_TABLE, XML_FILTER_NAME, rAreaLink.sFilter );
    if( !rAreaLink.sFilterOptions.isEmpty() )
        AddAttribute( XML_NAMESPACE_TABLE, XML_FILTER_OPTIONS, rAreaLink.sFilterOptions );
    AddAttribute( XML_NAMESPACE_TABLE, XML_LAST_COLUMN_SPANNED, OUString::number(rAreaLink.GetColCount()) );
    AddAttribute( XML_NAMESPACE_TABLE, XML_LAST_ROW_SPANNED, OUString::number(rAreaLink.GetRowCount()) );
    if( rAreaLink.nRefresh )
    {
        OUStringBuffer sValue;
        ::sax::Converter::convertDuration( sValue,
                static_cast<double>(rAreaLink.nRefresh) / 86400 );
        AddAttribute( XML_NAMESPACE_TABLE, XML_REFRESH_DELAY, sValue.makeStringAndClear() );
    }
    SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE, XML_CELL_RANGE_SOURCE, true, true );
}

void ScXMLExport::exportAnnotationMeta( const uno::Reference < drawing::XShape >& xShape)
{
    ScPostIt* pNote = pCurrentCell->pNote;

    if (!pNote)
        return;

    // TODO : notes
    //is it still useful, as this call back is only called from ScXMLExport::WriteAnnotation
    // and should be in sync with pCurrentCell
    SdrCaptionObj* pNoteCaption = pNote->GetOrCreateCaption(pCurrentCell->maCellAddress);
    uno::Reference<drawing::XShape> xCurrentShape( pNoteCaption->getUnoShape(), uno::UNO_QUERY );
    if (xCurrentShape.get()!=xShape.get())
        return;

    const OUString& sAuthor(pNote->GetAuthor());
    if (!sAuthor.isEmpty())
    {
        SvXMLElementExport aCreatorElem( *this, XML_NAMESPACE_DC,
                                            XML_CREATOR, true,
                                            false );
        Characters(sAuthor);
    }

    const OUString& aDate(pNote->GetDate());
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
            Characters(aDate);
        }
    }
    else
    {
        SvXMLElementExport aDateElem( *this, XML_NAMESPACE_META,
                                        XML_DATE_STRING, true,
                                        false );
        Characters(aDate);
    }
}

void ScXMLExport::WriteAnnotation(ScMyCell& rMyCell)
{
    ScPostIt* pNote = pDoc->GetNote(rMyCell.maCellAddress);
    if (!pNote)
        return;

    if (pNote->IsCaptionShown())
        AddAttribute(XML_NAMESPACE_OFFICE, XML_DISPLAY, XML_TRUE);

    pCurrentCell = &rMyCell;

    SdrCaptionObj* pNoteCaption = pNote->GetOrCreateCaption(rMyCell.maCellAddress);
    if (pNoteCaption)
    {
        uno::Reference<drawing::XShape> xShape( pNoteCaption->getUnoShape(), uno::UNO_QUERY );
        if (xShape.is())
            GetShapeExport()->exportShape(xShape, SEF_DEFAULT|XMLShapeExportFlags::ANNOTATION);
    }

    pCurrentCell = nullptr;
}

void ScXMLExport::WriteDetective( const ScMyCell& rMyCell )
{
    if( !(rMyCell.bHasDetectiveObj || rMyCell.bHasDetectiveOp) )
        return;

    const ScMyDetectiveObjVec& rObjVec = rMyCell.aDetectiveObjVec;
    const ScMyDetectiveOpVec& rOpVec = rMyCell.aDetectiveOpVec;
    sal_Int32 nObjCount(rObjVec.size());
    sal_Int32 nOpCount(rOpVec.size());
    if( !(nObjCount || nOpCount) )
        return;

    SvXMLElementExport aDetElem( *this, XML_NAMESPACE_TABLE, XML_DETECTIVE, true, true );
    OUString sString;
    for(const auto& rObj : rObjVec)
    {
        if (rObj.eObjType != SC_DETOBJ_CIRCLE)
        {
            if( (rObj.eObjType == SC_DETOBJ_ARROW) || (rObj.eObjType == SC_DETOBJ_TOOTHERTAB))
            {
                ScRangeStringConverter::GetStringFromRange( sString, rObj.aSourceRange, pDoc, FormulaGrammar::CONV_OOO );
                AddAttribute( XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, sString );
            }
            ScXMLConverter::GetStringFromDetObjType( sString, rObj.eObjType );
            AddAttribute( XML_NAMESPACE_TABLE, XML_DIRECTION, sString );
            if( rObj.bHasError )
                AddAttribute( XML_NAMESPACE_TABLE, XML_CONTAINS_ERROR, XML_TRUE );
        }
        else
            AddAttribute( XML_NAMESPACE_TABLE, XML_MARKED_INVALID, XML_TRUE );
        SvXMLElementExport aRangeElem( *this, XML_NAMESPACE_TABLE, XML_HIGHLIGHTED_RANGE, true, true );
    }
    for(const auto& rOp : rOpVec)
    {
        OUString sOpString;
        ScXMLConverter::GetStringFromDetOpType( sOpString, rOp.eOpType );
        AddAttribute( XML_NAMESPACE_TABLE, XML_NAME, sOpString );
        AddAttribute( XML_NAMESPACE_TABLE, XML_INDEX, OUString::number(rOp.nIndex) );
        SvXMLElementExport aRangeElem( *this, XML_NAMESPACE_TABLE, XML_OPERATION, true, true );
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
            IncrementProgressBar(false, nEqualCellCount);
    }
}

bool ScXMLExport::IsEditCell(const ScMyCell& rCell)
{
    return rCell.maBaseCell.meType == CELLTYPE_EDIT;
}

bool ScXMLExport::IsCellEqual (const ScMyCell& aCell1, const ScMyCell& aCell2)
{
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
            if (!aCell1.bHasAnnotation)
            {
                if ((((aCell1.nStyleIndex == aCell2.nStyleIndex) && (aCell1.bIsAutoStyle == aCell2.bIsAutoStyle)) ||
                     ((aCell1.nStyleIndex == aCell2.nStyleIndex) && (aCell1.nStyleIndex == -1))) &&
                    aCell1.nValidationIndex == aCell2.nValidationIndex &&
                    aCell1.nType == aCell2.nType)
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
                            // #i29101# number format may be different from column default styles,
                            // but can lead to different value types, so it must also be compared
                            bIsEqual = (aCell1.nNumberFormat == aCell2.nNumberFormat) &&
                                       (aCell1.maBaseCell.mfValue == aCell2.maBaseCell.mfValue);
                        }
                        break;
                    case table::CellContentType_TEXT :
                        {
                            if (IsEditCell(aCell1) || IsEditCell(aCell2))
                                bIsEqual = false;
                            else
                            {
                                bIsEqual = (aCell1.maBaseCell.getString(pDoc) == aCell2.maBaseCell.getString(pDoc));
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
    if (!xPropertySet.is())
        return;

    bool bCalcAsShown (::cppu::any2bool( xPropertySet->getPropertyValue(SC_UNO_CALCASSHOWN) ));
    bool bIgnoreCase (::cppu::any2bool( xPropertySet->getPropertyValue(SC_UNO_IGNORECASE) ));
    bool bLookUpLabels (::cppu::any2bool( xPropertySet->getPropertyValue(SC_UNO_LOOKUPLABELS) ));
    bool bMatchWholeCell (::cppu::any2bool( xPropertySet->getPropertyValue(SC_UNO_MATCHWHOLE) ));
    bool bUseRegularExpressions (::cppu::any2bool( xPropertySet->getPropertyValue(SC_UNO_REGEXENABLED) ));
    bool bUseWildcards (::cppu::any2bool( xPropertySet->getPropertyValue(SC_UNO_WILDCARDSENABLED) ));
    if (bUseWildcards && bUseRegularExpressions)
        bUseRegularExpressions = false;     // mutually exclusive, wildcards take precedence
    bool bIsIterationEnabled (::cppu::any2bool( xPropertySet->getPropertyValue(SC_UNO_ITERENABLED) ));
    sal_uInt16 nYear2000 (pDoc ? pDoc->GetDocOptions().GetYear2000() : 0);
    sal_Int32 nIterationCount(100);
    xPropertySet->getPropertyValue( SC_UNO_ITERCOUNT ) >>= nIterationCount;
    double fIterationEpsilon = 0;
    xPropertySet->getPropertyValue( SC_UNO_ITEREPSILON ) >>= fIterationEpsilon;
    util::Date aNullDate;
    xPropertySet->getPropertyValue( SC_UNO_NULLDATE ) >>= aNullDate;
    if (!(bCalcAsShown || bIgnoreCase || !bLookUpLabels || !bMatchWholeCell || !bUseRegularExpressions ||
            bUseWildcards ||
            bIsIterationEnabled || nIterationCount != 100 || !::rtl::math::approxEqual(fIterationEpsilon, 0.001) ||
            aNullDate.Day != 30 || aNullDate.Month != 12 || aNullDate.Year != 1899 || nYear2000 != 1930))
        return;

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
    if (bUseWildcards)
        AddAttribute(XML_NAMESPACE_TABLE, XML_USE_WILDCARDS, XML_TRUE);
    if (nYear2000 != 1930)
    {
        AddAttribute(XML_NAMESPACE_TABLE, XML_NULL_YEAR, OUString::number(nYear2000));
    }
    SvXMLElementExport aCalcSettings(*this, XML_NAMESPACE_TABLE, XML_CALCULATION_SETTINGS, true, true);
    {
        if (aNullDate.Day != 30 || aNullDate.Month != 12 || aNullDate.Year != 1899)
        {
            OUStringBuffer sDate;
            SvXMLUnitConverter::convertDateTime(sDate, 0.0, aNullDate);
            AddAttribute(XML_NAMESPACE_TABLE, XML_DATE_VALUE, sDate.makeStringAndClear());
            SvXMLElementExport aElemNullDate(*this, XML_NAMESPACE_TABLE, XML_NULL_DATE, true, true);
        }
        if (bIsIterationEnabled || nIterationCount != 100 || !::rtl::math::approxEqual(fIterationEpsilon, 0.001))
        {
            if (bIsIterationEnabled)
                AddAttribute(XML_NAMESPACE_TABLE, XML_STATUS, XML_ENABLE);
            if (nIterationCount != 100)
            {
                AddAttribute(XML_NAMESPACE_TABLE, XML_STEPS, OUString::number(nIterationCount));
            }
            if (!::rtl::math::approxEqual(fIterationEpsilon, 0.001))
            {
                OUStringBuffer sBuffer;
                ::sax::Converter::convertDouble(sBuffer,
                        fIterationEpsilon);
                AddAttribute(XML_NAMESPACE_TABLE, XML_MAXIMUM_DIFFERENCE, sBuffer.makeStringAndClear());
            }
            SvXMLElementExport aElemIteration(*this, XML_NAMESPACE_TABLE, XML_ITERATION, true, true);
        }
    }
}

void ScXMLExport::WriteTableSource()
{
    uno::Reference <sheet::XSheetLinkable> xLinkable (xCurrentTable, uno::UNO_QUERY);
    if (!(xLinkable.is() && GetModel().is()))
        return;

    sheet::SheetLinkMode nMode (xLinkable->getLinkMode());
    if (nMode == sheet::SheetLinkMode_NONE)
        return;

    OUString sLink (xLinkable->getLinkUrl());
    uno::Reference <beans::XPropertySet> xProps (GetModel(), uno::UNO_QUERY);
    if (!xProps.is())
        return;

    uno::Reference <container::XIndexAccess> xIndex(xProps->getPropertyValue(SC_UNO_SHEETLINKS), uno::UNO_QUERY);
    if (!xIndex.is())
        return;

    sal_Int32 nCount(xIndex->getCount());
    if (!nCount)
        return;

    bool bFound(false);
    uno::Reference <beans::XPropertySet> xLinkProps;
    for (sal_Int32 i = 0; (i < nCount) && !bFound; ++i)
    {
        xLinkProps.set(xIndex->getByIndex(i), uno::UNO_QUERY);
        if (xLinkProps.is())
        {
            OUString sNewLink;
            if (xLinkProps->getPropertyValue(SC_UNONAME_LINKURL) >>= sNewLink)
                bFound = sLink == sNewLink;
        }
    }
    if (!(bFound && xLinkProps.is()))
        return;

    OUString sFilter;
    OUString sFilterOptions;
    OUString sTableName (xLinkable->getLinkSheetName());
    sal_Int32 nRefresh(0);
    xLinkProps->getPropertyValue(SC_UNONAME_FILTER) >>= sFilter;
    xLinkProps->getPropertyValue(SC_UNONAME_FILTOPT) >>= sFilterOptions;
    xLinkProps->getPropertyValue(SC_UNONAME_REFDELAY) >>= nRefresh;
    if (sLink.isEmpty())
        return;

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
                static_cast<double>(nRefresh) / 86400 );
        AddAttribute( XML_NAMESPACE_TABLE, XML_REFRESH_DELAY, sBuffer.makeStringAndClear() );
    }
    SvXMLElementExport aSourceElem(*this, XML_NAMESPACE_TABLE, XML_TABLE_SOURCE, true, true);
}

// core implementation
void ScXMLExport::WriteScenario()
{
    if (!(pDoc && pDoc->IsScenario(static_cast<SCTAB>(nCurrentTable))))
        return;

    OUString sComment;
    Color       aColor;
    ScScenarioFlags nFlags;
    pDoc->GetScenarioData(static_cast<SCTAB>(nCurrentTable), sComment, aColor, nFlags);
    if (!(nFlags & ScScenarioFlags::ShowFrame))
        AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_BORDER, XML_FALSE);
    OUStringBuffer aBuffer;
    ::sax::Converter::convertColor(aBuffer, aColor);
    AddAttribute(XML_NAMESPACE_TABLE, XML_BORDER_COLOR, aBuffer.makeStringAndClear());
    if (!(nFlags & ScScenarioFlags::TwoWay))
        AddAttribute(XML_NAMESPACE_TABLE, XML_COPY_BACK, XML_FALSE);
    if (!(nFlags & ScScenarioFlags::Attrib))
        AddAttribute(XML_NAMESPACE_TABLE, XML_COPY_STYLES, XML_FALSE);
    if (nFlags & ScScenarioFlags::Value)
        AddAttribute(XML_NAMESPACE_TABLE, XML_COPY_FORMULAS, XML_FALSE);
    if (nFlags & ScScenarioFlags::Protected)
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
    SvXMLElementExport aElem(*this, XML_NAMESPACE_TABLE, XML_SCENARIO, true, true);
}

void ScXMLExport::WriteTheLabelRanges( const uno::Reference< sheet::XSpreadsheetDocument >& xSpreadDoc )
{
    uno::Reference< beans::XPropertySet > xDocProp( xSpreadDoc, uno::UNO_QUERY );
    if( !xDocProp.is() ) return;

    sal_Int32 nCount(0);
    uno::Reference< container::XIndexAccess > xColRangesIAccess(xDocProp->getPropertyValue( SC_UNO_COLLABELRNG ), uno::UNO_QUERY);
    if( xColRangesIAccess.is() )
        nCount += xColRangesIAccess->getCount();

    uno::Reference< container::XIndexAccess > xRowRangesIAccess(xDocProp->getPropertyValue( SC_UNO_ROWLABELRNG ), uno::UNO_QUERY);
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

void ScXMLExport::WriteExternalDataMapping()
{
    if (!pDoc)
        return;

    if ((getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED) == 0)
        // Export this only for 1.2 extended and above.
        return;

    sc::ExternalDataMapper& rDataMapper = pDoc->GetExternalDataMapper();
    auto& rDataSources = rDataMapper.getDataSources();

    if (rDataSources.empty())
        return;

    SvXMLElementExport aMappings(*this, XML_NAMESPACE_CALC_EXT, XML_DATA_MAPPINGS, true, true);
    for (const auto& itr : rDataSources)
    {
        AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, itr.getURL());
        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_PROVIDER, itr.getProvider());
        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_DATA_FREQUENCY, OUString::number(sc::ExternalDataSource::getUpdateFrequency()));
        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_ID, itr.getID());
        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_DATABASE_NAME, itr.getDBName());

        SvXMLElementExport aMapping(*this, XML_NAMESPACE_CALC_EXT, XML_DATA_MAPPING, true, true);
        // Add the data transformations
        WriteExternalDataTransformations(itr.getDataTransformation());
    }
}

void ScXMLExport::WriteExternalDataTransformations(const std::vector<std::shared_ptr<sc::DataTransformation>>& aDataTransformations)
{
    SvXMLElementExport aTransformations(*this, XML_NAMESPACE_CALC_EXT, XML_DATA_TRANSFORMATIONS, true, true);
    for (auto& itr : aDataTransformations)
    {
        sc::TransformationType aTransformationType = itr->getTransformationType();

        switch(aTransformationType)
        {
            case sc::TransformationType::DELETE_TRANSFORMATION:
            {
                // Delete Columns Transformation
                std::shared_ptr<sc::ColumnRemoveTransformation> aDeleteTransformation = std::dynamic_pointer_cast<sc::ColumnRemoveTransformation>(itr);
                std::set<SCCOL> aColumns = aDeleteTransformation->getColumns();
                SvXMLElementExport aTransformation(*this, XML_NAMESPACE_CALC_EXT, XML_COLUMN_REMOVE_TRANSFORMATION, true, true);
                for(auto& col : aColumns)
                {
                    // Add Columns
                    AddAttribute(XML_NAMESPACE_CALC_EXT, XML_COLUMN, OUString::number(col));
                    SvXMLElementExport aCol(*this, XML_NAMESPACE_CALC_EXT, XML_COLUMN, true, true);
                }
            }
            break;
            case sc::TransformationType::SPLIT_TRANSFORMATION:
            {
                std::shared_ptr<sc::SplitColumnTransformation> aSplitTransformation = std::dynamic_pointer_cast<sc::SplitColumnTransformation>(itr);

                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_COLUMN, OUString::number(aSplitTransformation->getColumn()));
                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_SEPARATOR, OUString::number(aSplitTransformation->getSeparator()));
                SvXMLElementExport aTransformation(*this, XML_NAMESPACE_CALC_EXT, XML_COLUMN_SPLIT_TRANSFORMATION, true, true);
            }
            break;
            case sc::TransformationType::MERGE_TRANSFORMATION:
            {
                // Merge Transformation
                std::shared_ptr<sc::MergeColumnTransformation> aMergeTransformation = std::dynamic_pointer_cast<sc::MergeColumnTransformation>(itr);
                std::set<SCCOL> aColumns = aMergeTransformation->getColumns();

                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_MERGE_STRING, aMergeTransformation->getMergeString());
                SvXMLElementExport aTransformation(*this, XML_NAMESPACE_CALC_EXT, XML_COLUMN_MERGE_TRANSFORMATION, true, true);

                for(auto& col : aColumns)
                {
                    // Columns
                    AddAttribute(XML_NAMESPACE_CALC_EXT, XML_COLUMN, OUString::number(col));
                    SvXMLElementExport aCol(*this, XML_NAMESPACE_CALC_EXT, XML_COLUMN, true, true);
                }
            }
            break;
            case sc::TransformationType::SORT_TRANSFORMATION:
            {
                // Sort Transformation
                std::shared_ptr<sc::SortTransformation> aSortTransformation = std::dynamic_pointer_cast<sc::SortTransformation>(itr);
                ScSortParam aSortParam = aSortTransformation->getSortParam();
                const sc::DocumentLinkManager& rMgr = pDoc->GetDocLinkManager();
                const sc::DataStream* pStrm = rMgr.getDataStream();
                if (!pStrm)
                    // No data stream.
                    return;

                // Streamed range
                ScRange aRange = pStrm->GetRange();

                SvXMLElementExport aTransformation(*this, XML_NAMESPACE_CALC_EXT, XML_COLUMN_SORT_TRANSFORMATION, true, true);

                writeSort(*this, aSortParam, aRange, pDoc);
            }
            break;
            case sc::TransformationType::TEXT_TRANSFORMATION:
            {
                // Text Transformation
                std::shared_ptr<sc::TextTransformation> aTextTransformation = std::dynamic_pointer_cast<sc::TextTransformation>(itr);

                sc::TEXT_TRANSFORM_TYPE aTextTransformType = aTextTransformation->getTextTransformationType();

                switch ( aTextTransformType )
                {
                    case sc::TEXT_TRANSFORM_TYPE::TO_LOWER:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_CASEMAP_LOWERCASE);
                    break;
                    case sc::TEXT_TRANSFORM_TYPE::TO_UPPER:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_CASEMAP_UPPERCASE);
                    break;
                    case sc::TEXT_TRANSFORM_TYPE::CAPITALIZE:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_CASEMAP_CAPITALIZE);
                    break;
                    case sc::TEXT_TRANSFORM_TYPE::TRIM:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_TRIM);
                    break;
                }

                std::set<SCCOL> aColumns = aTextTransformation->getColumns();

                SvXMLElementExport aTransformation(*this, XML_NAMESPACE_CALC_EXT, XML_COLUMN_TEXT_TRANSFORMATION, true, true);

                for(auto& col : aColumns)
                {
                    // Columns
                    AddAttribute(XML_NAMESPACE_CALC_EXT, XML_COLUMN, OUString::number(col));
                    SvXMLElementExport aCol(*this, XML_NAMESPACE_CALC_EXT, XML_COLUMN, true, true);
                }
            }
            break;
            case sc::TransformationType::AGGREGATE_FUNCTION:
            {
                // Aggregate Transformation
                std::shared_ptr<sc::AggregateFunction> aAggregateFunction = std::dynamic_pointer_cast<sc::AggregateFunction>(itr);
                std::set<SCCOL> aColumns = aAggregateFunction->getColumns();

                sc::AGGREGATE_FUNCTION aAggregateType = aAggregateFunction->getAggregateType();

                switch (aAggregateType)
                {
                    case sc::AGGREGATE_FUNCTION::SUM:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_SUM);
                    break;
                    case sc::AGGREGATE_FUNCTION::AVERAGE:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_AVERAGE);
                    break;
                    case sc::AGGREGATE_FUNCTION::MIN:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_MIN);
                    break;
                    case sc::AGGREGATE_FUNCTION::MAX:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_MAX);
                    break;
                }

                SvXMLElementExport aTransformation(*this, XML_NAMESPACE_CALC_EXT,XML_COLUMN_AGGREGATE_TRANSFORMATION, true, true);

                for(auto& col : aColumns)
                {
                    // Columns
                    AddAttribute(XML_NAMESPACE_CALC_EXT, XML_COLUMN, OUString::number(col));
                    SvXMLElementExport aCol(*this, XML_NAMESPACE_CALC_EXT, XML_COLUMN, true, true);
                }
            }
            break;
            case sc::TransformationType::NUMBER_TRANSFORMATION:
            {
                // Number Transformation
                std::shared_ptr<sc::NumberTransformation> aNumberTransformation = std::dynamic_pointer_cast<sc::NumberTransformation>(itr);

                sc::NUMBER_TRANSFORM_TYPE aNumberTransformType = aNumberTransformation->getNumberTransformationType();

                switch ( aNumberTransformType )
                {
                    case sc::NUMBER_TRANSFORM_TYPE::ROUND:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_ROUND);
                    break;
                    case sc::NUMBER_TRANSFORM_TYPE::ROUND_UP:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_ROUND_UP);
                    break;
                    case sc::NUMBER_TRANSFORM_TYPE::ROUND_DOWN:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_ROUND_DOWN);
                    break;
                    case sc::NUMBER_TRANSFORM_TYPE::ABSOLUTE:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_ABS);
                    break;
                    case sc::NUMBER_TRANSFORM_TYPE::LOG_E:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_LOG);
                    break;
                    case sc::NUMBER_TRANSFORM_TYPE::LOG_10:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_LOG_10);
                    break;
                    case sc::NUMBER_TRANSFORM_TYPE::CUBE:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_CUBE);
                    break;
                    case sc::NUMBER_TRANSFORM_TYPE::SQUARE:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_SQUARE);
                    break;
                    case sc::NUMBER_TRANSFORM_TYPE::SQUARE_ROOT:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_SQUARE_ROOT);
                    break;
                    case sc::NUMBER_TRANSFORM_TYPE::EXPONENT:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_EXPONENTIAL);
                    break;
                    case sc::NUMBER_TRANSFORM_TYPE::IS_EVEN:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_EVEN);
                    break;
                    case sc::NUMBER_TRANSFORM_TYPE::IS_ODD:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_ODD);
                    break;
                    case sc::NUMBER_TRANSFORM_TYPE::SIGN:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_SIGN);
                    break;
                }

                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_PRECISION, OUString::number(aNumberTransformation->getPrecision()));
                SvXMLElementExport aTransformation(*this, XML_NAMESPACE_CALC_EXT, XML_COLUMN_NUMBER_TRANSFORMATION, true, true);

                std::set<SCCOL> aColumns = aNumberTransformation->getColumn();
                for(auto& col : aColumns)
                {
                    // Columns
                    AddAttribute(XML_NAMESPACE_CALC_EXT, XML_COLUMN, OUString::number(col));
                    SvXMLElementExport aCol(*this, XML_NAMESPACE_CALC_EXT, XML_COLUMN, true, true);
                }
            }
            break;
            case sc::TransformationType::REMOVE_NULL_TRANSFORMATION:
            {
                // Replace Null Transformation
                std::shared_ptr<sc::ReplaceNullTransformation> aReplaceNullTransformation = std::dynamic_pointer_cast<sc::ReplaceNullTransformation>(itr);
                std::set<SCCOL> aColumns = aReplaceNullTransformation->getColumn();

                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_REPLACE_STRING, aReplaceNullTransformation->getReplaceString());
                SvXMLElementExport aTransformation(*this, XML_NAMESPACE_CALC_EXT, XML_COLUMN_REPLACENULL_TRANSFORMATION, true, true);

                for(auto& col : aColumns)
                {
                    // Columns
                    AddAttribute(XML_NAMESPACE_CALC_EXT, XML_COLUMN, OUString::number(col));
                    SvXMLElementExport aCol(*this, XML_NAMESPACE_CALC_EXT, XML_COLUMN, true, true);
                }
            }
            break;
            case sc::TransformationType::DATETIME_TRANSFORMATION:
            {
                // Number Transformation
                std::shared_ptr<sc::DateTimeTransformation> aDateTimeTransformation = std::dynamic_pointer_cast<sc::DateTimeTransformation>(itr);

                sc::DATETIME_TRANSFORMATION_TYPE aDateTimeTransformationType = aDateTimeTransformation->getDateTimeTransformationType();

                switch ( aDateTimeTransformationType )
                {
                    case sc::DATETIME_TRANSFORMATION_TYPE::DATE_STRING:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_DATE_STRING);
                    break;
                    case sc::DATETIME_TRANSFORMATION_TYPE::YEAR:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_YEAR);
                    break;
                    case sc::DATETIME_TRANSFORMATION_TYPE::START_OF_YEAR:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_START_OF_YEAR);
                    break;
                    case sc::DATETIME_TRANSFORMATION_TYPE::END_OF_YEAR:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_END_OF_YEAR);
                    break;
                    case sc::DATETIME_TRANSFORMATION_TYPE::MONTH:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_MONTH);
                    break;
                    case sc::DATETIME_TRANSFORMATION_TYPE::MONTH_NAME:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_MONTH_NAME);
                    break;
                    case sc::DATETIME_TRANSFORMATION_TYPE::START_OF_MONTH:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_START_OF_MONTH);
                    break;
                    case sc::DATETIME_TRANSFORMATION_TYPE::END_OF_MONTH:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_END_OF_MONTH);
                    break;
                    case sc::DATETIME_TRANSFORMATION_TYPE::DAY:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_DAY);
                    break;
                    case sc::DATETIME_TRANSFORMATION_TYPE::DAY_OF_WEEK:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_DAY_OF_WEEK);
                    break;
                    case sc::DATETIME_TRANSFORMATION_TYPE::DAY_OF_YEAR:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_DAY_OF_YEAR);
                    break;
                    case sc::DATETIME_TRANSFORMATION_TYPE::QUARTER:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_QUARTER);
                    break;
                    case sc::DATETIME_TRANSFORMATION_TYPE::START_OF_QUARTER:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_START_OF_QUARTER);
                    break;
                    case sc::DATETIME_TRANSFORMATION_TYPE::END_OF_QUARTER:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_END_OF_QUARTER);
                    break;
                    case sc::DATETIME_TRANSFORMATION_TYPE::TIME:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_TIME);
                    break;
                    case sc::DATETIME_TRANSFORMATION_TYPE::HOUR:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_HOUR);
                    break;
                    case sc::DATETIME_TRANSFORMATION_TYPE::MINUTE:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_MINUTE);
                    break;
                    case sc::DATETIME_TRANSFORMATION_TYPE::SECOND:
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, XML_SECONDS);
                    break;
                }

                SvXMLElementExport aTransformation(*this, XML_NAMESPACE_CALC_EXT, XML_COLUMN_DATETIME_TRANSFORMATION, true, true);

                std::set<SCCOL> aColumns = aDateTimeTransformation->getColumn();
                for(auto& col : aColumns)
                {
                    // Columns
                    AddAttribute(XML_NAMESPACE_CALC_EXT, XML_COLUMN, OUString::number(col));
                    SvXMLElementExport aCol(*this, XML_NAMESPACE_CALC_EXT, XML_COLUMN, true, true);
                }
            }
            break;
            default:
            break;
        }
    }
}

void ScXMLExport::WriteDataStream()
{
    if (!pDoc)
        return;

    if (!officecfg::Office::Common::Misc::ExperimentalMode::get())
        // Export this only in experimental mode.
        return;

    if ((getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED) == 0)
        // Export this only for 1.2 extended and above.
        return;

    const sc::DocumentLinkManager& rMgr = pDoc->GetDocLinkManager();
    const sc::DataStream* pStrm = rMgr.getDataStream();
    if (!pStrm)
        // No data stream.
        return;

    // Source URL
    AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, GetRelativeReference(pStrm->GetURL()));

    // Streamed range
    ScRange aRange = pStrm->GetRange();
    OUString aRangeStr;
    ScRangeStringConverter::GetStringFromRange(
        aRangeStr, aRange, pDoc, formula::FormulaGrammar::CONV_OOO);
    AddAttribute(XML_NAMESPACE_TABLE, XML_TARGET_RANGE_ADDRESS, aRangeStr);

    // Empty line refresh option.
    AddAttribute(XML_NAMESPACE_CALC_EXT, XML_EMPTY_LINE_REFRESH, pStrm->IsRefreshOnEmptyLine() ? XML_TRUE : XML_FALSE);

    // New data insertion position. Either top of bottom. Default to bottom.
    xmloff::token::XMLTokenEnum eInsertPosition = XML_BOTTOM;
    if (pStrm->GetMove() == sc::DataStream::MOVE_DOWN)
        eInsertPosition = XML_TOP;

    AddAttribute(XML_NAMESPACE_CALC_EXT, XML_INSERTION_POSITION, eInsertPosition);

    SvXMLElementExport aElem(*this, XML_NAMESPACE_CALC_EXT, XML_DATA_STREAM_SOURCE, true, true);
}

void ScXMLExport::WriteNamedRange(ScRangeName* pRangeName)
{
    //write a global or local ScRangeName
    SvXMLElementExport aElemNEs(*this, XML_NAMESPACE_TABLE, XML_NAMED_EXPRESSIONS, true, true);
    for (const auto& rxEntry : *pRangeName)
    {
        AddAttribute(sAttrName, rxEntry.second->GetName());

        OUString sBaseCellAddress;
        rxEntry.second->ValidateTabRefs();
        ScRangeStringConverter::GetStringFromAddress( sBaseCellAddress, rxEntry.second->GetPos(), pDoc,
                            FormulaGrammar::CONV_OOO, ' ', false, ScRefFlags::ADDR_ABS_3D);
        AddAttribute(XML_NAMESPACE_TABLE, XML_BASE_CELL_ADDRESS, sBaseCellAddress);

        OUString sSymbol;
        rxEntry.second->GetSymbol(sSymbol, pDoc->GetStorageGrammar());
        OUString sTempSymbol(sSymbol);
        ScRange aRange;
        if (rxEntry.second->IsReference(aRange))
        {

            OUString sContent(sTempSymbol.copy(1, sTempSymbol.getLength() -2 ));
            AddAttribute(XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, sContent);

            sal_Int32 nRangeType = rxEntry.second->GetUnoType();
            OUStringBuffer sBufferRangeType;
            if ((nRangeType & sheet::NamedRangeFlag::COLUMN_HEADER) == sheet::NamedRangeFlag::COLUMN_HEADER)
                sBufferRangeType.append(GetXMLToken(XML_REPEAT_COLUMN));
            if ((nRangeType & sheet::NamedRangeFlag::ROW_HEADER) == sheet::NamedRangeFlag::ROW_HEADER)
            {
                if (!sBufferRangeType.isEmpty())
                    sBufferRangeType.append(" ");
                sBufferRangeType.append(GetXMLToken(XML_REPEAT_ROW));
            }
            if ((nRangeType & sheet::NamedRangeFlag::FILTER_CRITERIA) == sheet::NamedRangeFlag::FILTER_CRITERIA)
            {
                if (!sBufferRangeType.isEmpty())
                    sBufferRangeType.append(" ");
                sBufferRangeType.append(GetXMLToken(XML_FILTER));
            }
            if ((nRangeType & sheet::NamedRangeFlag::PRINT_AREA) == sheet::NamedRangeFlag::PRINT_AREA)
            {
                if (!sBufferRangeType.isEmpty())
                    sBufferRangeType.append(" ");
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
            return "minimum";
        case COLORSCALE_MAX:
            return "maximum";
        case COLORSCALE_PERCENT:
            return "percent";
        case COLORSCALE_PERCENTILE:
            return "percentile";
        case COLORSCALE_FORMULA:
            return "formula";
        case COLORSCALE_VALUE:
            return "number";
        case COLORSCALE_AUTO:
            // only important for data bars
            if(bFirst)
                return "auto-minimum";
            else
                return "auto-maximum";
    }
    return OUString();
}

OUString getDateStringForType(condformat::ScCondFormatDateType eType)
{
    switch(eType)
    {
        case condformat::TODAY:
            return "today";
        case condformat::YESTERDAY:
            return "yesterday";
        case condformat::TOMORROW:
            return "tomorrow";
        case condformat::LAST7DAYS:
            return "last-7-days";
        case condformat::THISWEEK:
            return "this-week";
        case condformat::LASTWEEK:
            return "last-week";
        case condformat::NEXTWEEK:
            return "next-week";
        case condformat::THISMONTH:
            return "this-month";
        case condformat::LASTMONTH:
            return "last-month";
        case condformat::NEXTMONTH:
            return "next-month";
        case condformat::THISYEAR:
            return "this-year";
        case condformat::LASTYEAR:
            return "last-year";
        case condformat::NEXTYEAR:
            return "next-year";
    }

    return OUString();
}

}

void ScXMLExport::ExportConditionalFormat(SCTAB nTab)
{
    ScConditionalFormatList* pCondFormatList = pDoc->GetCondFormList(nTab);
    if(!pCondFormatList)
        return;

    if (pCondFormatList->empty())
        return;

    SvXMLElementExport aElementCondFormats(*this, XML_NAMESPACE_CALC_EXT, XML_CONDITIONAL_FORMATS, true, true);

    for(const auto& rxCondFormat : *pCondFormatList)
    {
        OUString sRanges;
        const ScRangeList& rRangeList = rxCondFormat->GetRange();
        ScRangeStringConverter::GetStringFromRangeList( sRanges, &rRangeList, pDoc, formula::FormulaGrammar::CONV_OOO );
        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TARGET_RANGE_ADDRESS, sRanges);
        SvXMLElementExport aElementCondFormat(*this, XML_NAMESPACE_CALC_EXT, XML_CONDITIONAL_FORMAT, true, true);
        size_t nEntries = rxCondFormat->size();
        for(size_t i = 0; i < nEntries; ++i)
        {
            const ScFormatEntry* pFormatEntry = rxCondFormat->GetEntry(i);
            if(pFormatEntry->GetType()==ScFormatEntry::Type::Condition)
            {
                const ScCondFormatEntry* pEntry = static_cast<const ScCondFormatEntry*>(pFormatEntry);
                OUStringBuffer aCond;
                ScAddress aPos = pEntry->GetSrcPos();
                switch(pEntry->GetOperation())
                {
                    case ScConditionMode::Equal:
                        aCond.append('=');
                        aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                        break;
                    case ScConditionMode::Less:
                        aCond.append('<');
                        aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                        break;
                    case ScConditionMode::Greater:
                        aCond.append('>');
                        aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                        break;
                    case ScConditionMode::EqLess:
                        aCond.append("<=");
                        aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                        break;
                    case ScConditionMode::EqGreater:
                        aCond.append(">=");
                        aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                        break;
                    case ScConditionMode::NotEqual:
                        aCond.append("!=");
                        aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                        break;
                    case ScConditionMode::Between:
                        aCond.append("between(");
                        aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                        aCond.append(',');
                        aCond.append(pEntry->GetExpression(aPos, 1, 0, formula::FormulaGrammar::GRAM_ODFF));
                        aCond.append(')');
                        break;
                    case ScConditionMode::NotBetween:
                        aCond.append("not-between(");
                        aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                        aCond.append(',');
                        aCond.append(pEntry->GetExpression(aPos, 1, 0, formula::FormulaGrammar::GRAM_ODFF));
                        aCond.append(')');
                        break;
                    case ScConditionMode::Duplicate:
                        aCond.append("duplicate");
                        break;
                    case ScConditionMode::NotDuplicate:
                        aCond.append("unique");
                        break;
                    case ScConditionMode::Direct:
                        aCond.append("formula-is(");
                        aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                        aCond.append(')');
                        break;
                    case ScConditionMode::Top10:
                        aCond.append("top-elements(");
                        aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                        aCond.append(")");
                        break;
                    case ScConditionMode::Bottom10:
                        aCond.append("bottom-elements(");
                        aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                        aCond.append(")");
                        break;
                    case ScConditionMode::TopPercent:
                        aCond.append("top-percent(");
                        aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                        aCond.append(")");
                        break;
                    case ScConditionMode::BottomPercent:
                        aCond.append("bottom-percent(");
                        aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                        aCond.append(")");
                        break;
                    case ScConditionMode::AboveAverage:
                        aCond.append("above-average");
                        break;
                    case ScConditionMode::BelowAverage:
                        aCond.append("below-average");
                        break;
                    case ScConditionMode::AboveEqualAverage:
                        aCond.append("above-equal-average");
                        break;
                    case ScConditionMode::BelowEqualAverage:
                        aCond.append("below-equal-average");
                        break;
                    case ScConditionMode::Error:
                        aCond.append("is-error");
                        break;
                    case ScConditionMode::NoError:
                        aCond.append("is-no-error");
                        break;
                    case ScConditionMode::BeginsWith:
                        aCond.append("begins-with(");
                        aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                        aCond.append(")");
                        break;
                    case ScConditionMode::EndsWith:
                        aCond.append("ends-with(");
                        aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                        aCond.append(")");
                        break;
                    case ScConditionMode::ContainsText:
                        aCond.append("contains-text(");
                        aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                        aCond.append(")");
                        break;
                    case ScConditionMode::NotContainsText:
                        aCond.append("not-contains-text(");
                        aCond.append(pEntry->GetExpression(aPos, 0, 0, formula::FormulaGrammar::GRAM_ODFF));
                        aCond.append(")");
                        break;
                    case ScConditionMode::NONE:
                        continue;
                    default:
                        SAL_WARN("sc", "unimplemented conditional format export");
                }
                OUString sStyle = ScStyleNameConversion::DisplayToProgrammaticName(pEntry->GetStyle(), SfxStyleFamily::Para);
                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_APPLY_STYLE_NAME, sStyle);
                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_VALUE, aCond.makeStringAndClear());

                OUString sBaseAddress;
                ScRangeStringConverter::GetStringFromAddress( sBaseAddress, aPos, pDoc,formula::FormulaGrammar::CONV_ODF );
                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_BASE_CELL_ADDRESS, sBaseAddress);
                SvXMLElementExport aElementCondEntry(*this, XML_NAMESPACE_CALC_EXT, XML_CONDITION, true, true);
            }
            else if(pFormatEntry->GetType() == ScFormatEntry::Type::Colorscale)
            {
                SvXMLElementExport aElementColorScale(*this, XML_NAMESPACE_CALC_EXT, XML_COLOR_SCALE, true, true);
                const ScColorScaleFormat& rColorScale = static_cast<const ScColorScaleFormat&>(*pFormatEntry);
                for(const auto& rxItem : rColorScale)
                {
                    if(rxItem->GetType() == COLORSCALE_FORMULA)
                    {
                        OUString sFormula = rxItem->GetFormula(formula::FormulaGrammar::GRAM_ODFF);
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_VALUE, sFormula);
                    }
                    else
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_VALUE, OUString::number(rxItem->GetValue()));

                    AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, getCondFormatEntryType(*rxItem));
                    OUStringBuffer aBuffer;
                    ::sax::Converter::convertColor(aBuffer, rxItem->GetColor());
                    AddAttribute(XML_NAMESPACE_CALC_EXT, XML_COLOR, aBuffer.makeStringAndClear());
                    SvXMLElementExport aElementColorScaleEntry(*this, XML_NAMESPACE_CALC_EXT, XML_COLOR_SCALE_ENTRY, true, true);
                }
            }
            else if(pFormatEntry->GetType() == ScFormatEntry::Type::Databar)
            {
                const ScDataBarFormatData* pFormatData = static_cast<const ScDataBarFormat&>(*pFormatEntry).GetDataBarData();
                if(!pFormatData->mbGradient)
                    AddAttribute(XML_NAMESPACE_CALC_EXT, XML_GRADIENT, XML_FALSE);
                if(pFormatData->mbOnlyBar)
                    AddAttribute(XML_NAMESPACE_CALC_EXT, XML_SHOW_VALUE, XML_FALSE);

                if (pFormatData->mnMinLength != 0.0)
                    AddAttribute(XML_NAMESPACE_CALC_EXT, XML_MIN_LENGTH, OUString::number(pFormatData->mnMinLength));

                if (pFormatData->mnMaxLength != 0.0)
                    AddAttribute(XML_NAMESPACE_CALC_EXT, XML_MAX_LENGTH, OUString::number(pFormatData->mnMaxLength));

                if(pFormatData->mbNeg)
                {
                    if(pFormatData->mxNegativeColor)
                    {
                        OUStringBuffer aBuffer;
                        ::sax::Converter::convertColor(aBuffer, *pFormatData->mxNegativeColor);
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_NEGATIVE_COLOR, aBuffer.makeStringAndClear());
                    }
                    else
                    {
                        OUStringBuffer aBuffer;
                        ::sax::Converter::convertColor(aBuffer, COL_LIGHTRED);
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
                ::sax::Converter::convertColor(aBuffer, pFormatData->maPositiveColor);
                AddAttribute(XML_NAMESPACE_CALC_EXT, XML_POSITIVE_COLOR, aBuffer.makeStringAndClear());

                aBuffer.truncate();
                ::sax::Converter::convertColor(aBuffer, pFormatData->maAxisColor);
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
                    AddAttribute(XML_NAMESPACE_CALC_EXT, XML_TYPE, getCondFormatEntryType(*pFormatData->mpLowerLimit));
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
            else if(pFormatEntry->GetType() == ScFormatEntry::Type::Iconset)
            {
                const ScIconSetFormat& rIconSet = static_cast<const ScIconSetFormat&>(*pFormatEntry);
                OUString aIconSetName = OUString::createFromAscii(ScIconSetFormat::getIconSetName(rIconSet.GetIconSetData()->eIconSetType));
                AddAttribute( XML_NAMESPACE_CALC_EXT, XML_ICON_SET_TYPE, aIconSetName );
                if (rIconSet.GetIconSetData()->mbCustom)
                    AddAttribute(XML_NAMESPACE_CALC_EXT, XML_CUSTOM, OUString::boolean(true));

                SvXMLElementExport aElementColorScale(*this, XML_NAMESPACE_CALC_EXT, XML_ICON_SET, true, true);

                if (rIconSet.GetIconSetData()->mbCustom)
                {
                    for (const auto& [rType, rIndex] : rIconSet.GetIconSetData()->maCustomVector)
                    {
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_CUSTOM_ICONSET_NAME, OUString::createFromAscii(ScIconSetFormat::getIconSetName(rType)));
                        AddAttribute(XML_NAMESPACE_CALC_EXT, XML_CUSTOM_ICONSET_INDEX, OUString::number(rIndex));
                        SvXMLElementExport aCustomIcon(*this, XML_NAMESPACE_CALC_EXT, XML_CUSTOM_ICONSET, true, true);
                    }

                }

                if(!rIconSet.GetIconSetData()->mbShowValue)
                    AddAttribute(XML_NAMESPACE_CALC_EXT, XML_SHOW_VALUE, XML_FALSE);
                for (auto const& it : rIconSet)
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
            else if(pFormatEntry->GetType() == ScFormatEntry::Type::Date)
            {
                const ScCondDateFormatEntry& rDateFormat = static_cast<const ScCondDateFormatEntry&>(*pFormatEntry);
                OUString aDateType = getDateStringForType(rDateFormat.GetDateType());
                OUString aStyleName = ScStyleNameConversion::DisplayToProgrammaticName(rDateFormat.GetStyleName(), SfxStyleFamily::Para );
                AddAttribute( XML_NAMESPACE_CALC_EXT, XML_STYLE, aStyleName);
                AddAttribute( XML_NAMESPACE_CALC_EXT, XML_DATE, aDateType);
                SvXMLElementExport aElementDateFormat(*this, XML_NAMESPACE_CALC_EXT, XML_DATE_IS, true, true);
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

        for (const auto& rTabName : aTabNames)
        {
            ScExternalRefCache::TableTypeRef pTable = pRefMgr->getCacheTable(nFileId, rTabName, false);
            if (!pTable || !pTable->isReferenced())
                continue;

            AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, "'" + *pUrl + "'#" + rTabName);
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
                    AddAttribute(XML_NAMESPACE_TABLE, XML_TABLE_NAME, rTabName);
                    if (!pExtFileData->maFilterName.isEmpty())
                        AddAttribute(XML_NAMESPACE_TABLE, XML_FILTER_NAME, pExtFileData->maFilterName);
                    if (!pExtFileData->maFilterOptions.isEmpty())
                        AddAttribute(XML_NAMESPACE_TABLE, XML_FILTER_OPTIONS, pExtFileData->maFilterOptions);
                    AddAttribute(XML_NAMESPACE_TABLE, XML_MODE, XML_COPY_RESULTS_ONLY);
                }
                SvXMLElementExport aElemTableSource(*this, XML_NAMESPACE_TABLE, XML_TABLE_SOURCE, true, true);
            }

            // Determine maximum column count of used area, for repeated cells.
            SCCOL nMaxColsUsed = 1;     // assume that there is at least one cell somewhere...
            vector<SCROW> aRows;
            pTable->getAllRows(aRows);
            for (SCROW nRow : aRows)
            {
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
            for (SCROW nRow : aRows)
            {
                if (bFirstRow)
                {
                    if (nRow > 0)
                    {
                        if (nRow > 1)
                        {
                            OUString aVal = OUString::number(nRow);
                            AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_ROWS_REPEATED, aVal);
                        }
                        SvXMLElementExport aElemRow(*this, XML_NAMESPACE_TABLE, XML_TABLE_ROW, true, true);
                        OUString aVal = OUString::number(static_cast<sal_Int32>(nMaxColsUsed));
                        AddAttribute(XML_NAMESPACE_TABLE,  XML_NUMBER_COLUMNS_REPEATED, aVal);
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
                            OUString aVal = OUString::number(static_cast<sal_Int32>(nRowGap-1));
                            AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_ROWS_REPEATED, aVal);
                        }
                        SvXMLElementExport aElemRow(*this, XML_NAMESPACE_TABLE, XML_TABLE_ROW, true, true);
                        OUString aVal = OUString::number(static_cast<sal_Int32>(nMaxColsUsed));
                        AddAttribute(XML_NAMESPACE_TABLE,  XML_NUMBER_COLUMNS_REPEATED, aVal);
                        SvXMLElementExport aElemCell(*this, XML_NAMESPACE_TABLE, XML_TABLE_CELL, true, true);
                    }
                }
                SvXMLElementExport aElemRow(*this, XML_NAMESPACE_TABLE, XML_TABLE_ROW, true, true);

                vector<SCCOL> aCols;
                pTable->getAllCols(nRow, aCols);
                SCCOL nLastCol = 0;
                bool bFirstCol = true;
                for (SCCOL nCol : aCols)
                {
                    if (bFirstCol)
                    {
                        if (nCol > 0)
                        {
                            if (nCol > 1)
                            {
                                OUString aVal = OUString::number(static_cast<sal_Int32>(nCol));
                                AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED, aVal);
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
                                OUString aVal = OUString::number(static_cast<sal_Int32>(nColGap-1));
                                AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED, aVal);
                            }
                            SvXMLElementExport aElemCell(*this, XML_NAMESPACE_TABLE, XML_TABLE_CELL, true, true);
                        }
                    }

                    // Write out this cell.
                    sal_uInt32 nNumFmt = 0;
                    ScExternalRefCache::TokenRef pToken = pTable->getCell(nCol, nRow, &nNumFmt);
                    OUString aStrVal;
                    if (pToken)
                    {
                        sal_Int32 nIndex = GetNumberFormatStyleIndex(nNumFmt);
                        if (nIndex >= 0)
                        {
                            const OUString & aStyleName = pCellStyles->GetStyleNameByIndex(nIndex, true);
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
                                aStrVal = pToken->GetString().getString();
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
    if (!pDoc)
        return;

    const ScConsolidateParam* pCons(pDoc->GetConsolidateDlgData());
    if( !pCons )
        return;

    OUString sStrData;

    ScXMLConverter::GetStringFromFunction( sStrData, pCons->eFunction );
    AddAttribute( XML_NAMESPACE_TABLE, XML_FUNCTION, sStrData );

    sStrData.clear();
    for( sal_Int32 nIndex = 0; nIndex < pCons->nDataAreaCount; ++nIndex )
        ScRangeStringConverter::GetStringFromArea( sStrData, pCons->pDataAreas[ nIndex ], pDoc, FormulaGrammar::CONV_OOO, ' ', true );
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
    ScChangeViewSettings* pViewSettings(GetDocument() ? GetDocument()->GetChangeViewSettings() : nullptr);
    if (!pViewSettings)
        return;

    sal_Int32 nChangePos(rProps.getLength());
    rProps.realloc(nChangePos + 1);
    beans::PropertyValue* pProps(rProps.getArray());

    uno::Sequence<beans::PropertyValue> aChangeProps(SC_VIEWCHANGES_COUNT);
    beans::PropertyValue* pChangeProps(aChangeProps.getArray());
    pChangeProps[SC_SHOW_CHANGES].Name = "ShowChanges";
    pChangeProps[SC_SHOW_CHANGES].Value <<= pViewSettings->ShowChanges();
    pChangeProps[SC_SHOW_ACCEPTED_CHANGES].Name = "ShowAcceptedChanges";
    pChangeProps[SC_SHOW_ACCEPTED_CHANGES].Value <<= pViewSettings->IsShowAccepted();
    pChangeProps[SC_SHOW_REJECTED_CHANGES].Name = "ShowRejectedChanges";
    pChangeProps[SC_SHOW_REJECTED_CHANGES].Value <<= pViewSettings->IsShowRejected();
    pChangeProps[SC_SHOW_CHANGES_BY_DATETIME].Name = "ShowChangesByDatetime";
    pChangeProps[SC_SHOW_CHANGES_BY_DATETIME].Value <<= pViewSettings->HasDate();
    pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_MODE].Name = "ShowChangesByDatetimeMode";
    pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_MODE].Value <<= static_cast<sal_Int16>(pViewSettings->GetTheDateMode());
    pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_FIRST_DATETIME].Name = "ShowChangesByDatetimeFirstDatetime";
    pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_FIRST_DATETIME].Value <<= pViewSettings->GetTheFirstDateTime().GetUNODateTime();
    pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_SECOND_DATETIME].Name = "ShowChangesByDatetimeSecondDatetime";
    pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_SECOND_DATETIME].Value <<= pViewSettings->GetTheLastDateTime().GetUNODateTime();
    pChangeProps[SC_SHOW_CHANGES_BY_AUTHOR].Name = "ShowChangesByAuthor";
    pChangeProps[SC_SHOW_CHANGES_BY_AUTHOR].Value <<= pViewSettings->HasAuthor();
    pChangeProps[SC_SHOW_CHANGES_BY_AUTHOR_NAME].Name = "ShowChangesByAuthorName";
    pChangeProps[SC_SHOW_CHANGES_BY_AUTHOR_NAME].Value <<= pViewSettings->GetTheAuthorToShow();
    pChangeProps[SC_SHOW_CHANGES_BY_COMMENT].Name = "ShowChangesByComment";
    pChangeProps[SC_SHOW_CHANGES_BY_COMMENT].Value <<= pViewSettings->HasComment();
    pChangeProps[SC_SHOW_CHANGES_BY_COMMENT_TEXT].Name = "ShowChangesByCommentText";
    pChangeProps[SC_SHOW_CHANGES_BY_COMMENT_TEXT].Value <<= pViewSettings->GetTheComment();
    pChangeProps[SC_SHOW_CHANGES_BY_RANGES].Name = "ShowChangesByRanges";
    pChangeProps[SC_SHOW_CHANGES_BY_RANGES].Value <<= pViewSettings->HasRange();
    OUString sRangeList;
    ScRangeStringConverter::GetStringFromRangeList(sRangeList, &(pViewSettings->GetTheRangeList()), GetDocument(), FormulaGrammar::CONV_OOO);
    pChangeProps[SC_SHOW_CHANGES_BY_RANGES_LIST].Name = "ShowChangesByRangesList";
    pChangeProps[SC_SHOW_CHANGES_BY_RANGES_LIST].Value <<= sRangeList;

    pProps[nChangePos].Name = "TrackedChangesViewSettings";
    pProps[nChangePos].Value <<= aChangeProps;
}

void ScXMLExport::GetViewSettings(uno::Sequence<beans::PropertyValue>& rProps)
{
    if (GetModel().is())
    {
        rProps.realloc(4);
        beans::PropertyValue* pProps(rProps.getArray());
        ScModelObj* pDocObj(comphelper::getUnoTunnelImplementation<ScModelObj>( GetModel() ));
        if (pDocObj)
        {
            SfxObjectShell* pEmbeddedObj = pDocObj->GetEmbeddedObject();
            if (pEmbeddedObj)
            {
                tools::Rectangle aRect(pEmbeddedObj->GetVisArea());
                sal_uInt16 i(0);
                pProps[i].Name = "VisibleAreaTop";
                pProps[i].Value <<= static_cast<sal_Int32>(aRect.getY());
                pProps[++i].Name = "VisibleAreaLeft";
                pProps[i].Value <<= static_cast<sal_Int32>(aRect.getX());
                pProps[++i].Name = "VisibleAreaWidth";
                pProps[i].Value <<= static_cast<sal_Int32>(aRect.getWidth());
                pProps[++i].Name = "VisibleAreaHeight";
                pProps[i].Value <<= static_cast<sal_Int32>(aRect.getHeight());
            }
        }
    }
    GetChangeTrackViewSettings(rProps);
}

void ScXMLExport::GetConfigurationSettings(uno::Sequence<beans::PropertyValue>& rProps)
{
    if (!GetModel().is())
        return;

    uno::Reference <lang::XMultiServiceFactory> xMultiServiceFactory(GetModel(), uno::UNO_QUERY);
    if (!xMultiServiceFactory.is())
        return;

    uno::Reference <beans::XPropertySet> xProperties(xMultiServiceFactory->createInstance("com.sun.star.comp.SpreadsheetSettings"), uno::UNO_QUERY);
    if (xProperties.is())
        SvXMLUnitConverter::convertPropertySet(rProps, xProperties);

    sal_Int32 nPropsToAdd = 0;
    OUStringBuffer aTrackedChangesKey;
    if (GetDocument() && GetDocument()->GetChangeTrack() && GetDocument()->GetChangeTrack()->IsProtected())
    {
        ::comphelper::Base64::encode(aTrackedChangesKey,
                GetDocument()->GetChangeTrack()->GetProtection());
        if (!aTrackedChangesKey.isEmpty())
            ++nPropsToAdd;
    }

    bool bVBACompat = false;
    uno::Reference <container::XNameAccess> xCodeNameAccess;
    OSL_ENSURE( pDoc, "ScXMLExport::GetConfigurationSettings - no ScDocument!" );
    // tdf#71271 - add code names regardless of VBA compatibility mode
    if (pDoc)
    {
        // VBA compatibility mode
        if (bVBACompat = pDoc->IsInVBAMode(); bVBACompat)
            ++nPropsToAdd;

        // code names
        xCodeNameAccess = new XMLCodeNameProvider( pDoc );
        if( xCodeNameAccess->hasElements() )
            ++nPropsToAdd;
        else
            xCodeNameAccess.clear();
    }

    if( nPropsToAdd <= 0 )
        return;

    sal_Int32 nCount(rProps.getLength());
    rProps.realloc(nCount + nPropsToAdd);
    if (!aTrackedChangesKey.isEmpty())
    {
        rProps[nCount].Name = "TrackedChangesProtectionKey";
        rProps[nCount].Value <<= aTrackedChangesKey.makeStringAndClear();
        ++nCount;
    }
    if( bVBACompat )
    {
        rProps[nCount].Name = "VBACompatibilityMode";
        rProps[nCount].Value <<= bVBACompat;
        ++nCount;
    }
    if( xCodeNameAccess.is() )
    {
        rProps[nCount].Name = "ScriptConfiguration";
        rProps[nCount].Value <<= xCodeNameAccess;
        ++nCount;
    }
}

XMLShapeExport* ScXMLExport::CreateShapeExport()
{
    return new ScXMLShapeExport(*this);
}

XMLNumberFormatAttributesExportHelper* ScXMLExport::GetNumberFormatAttributesExportHelper()
{
    if (!pNumberFormatAttributesExportHelper)
        pNumberFormatAttributesExportHelper.reset(new XMLNumberFormatAttributesExportHelper(GetNumberFormatsSupplier(), *this ));
    return pNumberFormatAttributesExportHelper.get();
}

void ScXMLExport::CollectUserDefinedNamespaces(const SfxItemPool* pPool, sal_uInt16 nAttrib)
{
    for (const SfxPoolItem* pItem : pPool->GetItemSurrogates(nAttrib))
    {
        const SvXMLAttrContainerItem *pUnknown(static_cast<const SvXMLAttrContainerItem *>(pItem));
        if( pUnknown->GetAttrCount() > 0 )
        {
            sal_uInt16 nIdx(pUnknown->GetFirstNamespaceIndex());
            while( USHRT_MAX != nIdx )
            {
                if( (XML_NAMESPACE_UNKNOWN_FLAG & nIdx) != 0 )
                {
                    const OUString& rPrefix = pUnknown->GetPrefix( nIdx );
                    // Add namespace declaration for unknown attributes if
                    // there aren't existing ones for the prefix used by the
                    // attributes
                    GetNamespaceMap_().Add( rPrefix,
                                            pUnknown->GetNamespace( nIdx ) );
                }
                nIdx = pUnknown->GetNextNamespaceIndex( nIdx );
            }
        }
    }

    // #i66550# needed for 'presentation:event-listener' element for URLs in shapes
    GetNamespaceMap_().Add(
        GetXMLToken( XML_NP_PRESENTATION ),
        GetXMLToken( XML_N_PRESENTATION ),
        XML_NAMESPACE_PRESENTATION );
}

void ScXMLExport::IncrementProgressBar(bool bFlush, sal_Int32 nInc)
{
    nProgressCount += nInc;
    if (bFlush || nProgressCount > 100)
    {
        GetProgressBarHelper()->Increment(nProgressCount);
        nProgressCount = 0;
    }
}

ErrCode ScXMLExport::exportDoc( enum XMLTokenEnum eClass )
{
    if( getExportFlags() & (SvXMLExportFlags::FONTDECLS|SvXMLExportFlags::STYLES|
                             SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::CONTENT) )
    {
        if (GetDocument())
        {
            // if source doc was Excel then
            uno::Reference< frame::XModel > xModel = GetModel();
            if ( xModel.is() )
            {
                auto pFoundShell = comphelper::getUnoTunnelImplementation<SfxObjectShell>(xModel);
                if ( pFoundShell && ooo::vba::isAlienExcelDoc( *pFoundShell ) )
                {
                    xRowStylesPropertySetMapper = new XMLPropertySetMapper(aXMLScFromXLSRowStylesProperties, xScPropHdlFactory, true);
                    xRowStylesExportPropertySetMapper = new ScXMLRowExportPropertyMapper(xRowStylesPropertySetMapper);
                    GetAutoStylePool()->SetFamilyPropSetMapper( XmlStyleFamily::TABLE_ROW,
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
            if( (getExportFlags() & SvXMLExportFlags::CONTENT) &&
                getSaneDefaultVersion() >= SvtSaveOptions::ODFSVER_012)
            {
                bool bAnySheetEvents = false;
                SCTAB nTabCount = pDoc->GetTableCount();
                for (SCTAB nTab=0; nTab<nTabCount; ++nTab)
                    if (pDoc->GetSheetEvents(nTab))
                        bAnySheetEvents = true;
                if (bAnySheetEvents)
                    GetNamespaceMap_().Add(
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
{
    SolarMutexGuard aGuard;
    SvXMLExport::setSourceDocument( xComponent );

    pDoc = ScXMLConverter::GetScDocument( GetModel() );
    OSL_ENSURE( pDoc, "ScXMLExport::setSourceDocument - no ScDocument!" );
    if (!pDoc)
        throw lang::IllegalArgumentException();

    // create ScChangeTrackingExportHelper after document is known
    pChangeTrackingExportHelper.reset(new ScChangeTrackingExportHelper(*this));

    // Set the document's storage grammar corresponding to the ODF version that
    // is to be written.
    SvtSaveOptions::ODFSaneDefaultVersion meODFDefaultVersion = getSaneDefaultVersion();
    switch (meODFDefaultVersion)
    {
        // ODF 1.0 and 1.1 use GRAM_PODF, everything later or unspecified GRAM_ODFF
        case SvtSaveOptions::ODFSVER_010:
        case SvtSaveOptions::ODFSVER_011:
            pDoc->SetStorageGrammar( formula::FormulaGrammar::GRAM_PODF);
            break;
        default:
            pDoc->SetStorageGrammar( formula::FormulaGrammar::GRAM_ODFF);
    }
}

// XFilter
sal_Bool SAL_CALL ScXMLExport::filter( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor )
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
{
    SolarMutexGuard aGuard;
    if (pDoc)
        pDoc->EnableIdle(true);
    SvXMLExport::cancel();
}

// XInitialization
void SAL_CALL ScXMLExport::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    SolarMutexGuard aGuard;
    SvXMLExport::initialize(aArguments);
}

// XUnoTunnel
sal_Int64 SAL_CALL ScXMLExport::getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier )
{
    SolarMutexGuard aGuard;
    return SvXMLExport::getSomething(aIdentifier);
}

void ScXMLExport::DisposingModel()
{
    SvXMLExport::DisposingModel();
    pDoc = nullptr;
    xCurrentTable = nullptr;
}

void ScXMLExport::SetSharedData(std::unique_ptr<ScMySharedData> pTemp) { pSharedData = std::move(pTemp); }

std::unique_ptr<ScMySharedData> ScXMLExport::ReleaseSharedData() { return std::move(pSharedData); }
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
