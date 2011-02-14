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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include <editeng/eeitem.hxx>

#include "xmlexprt.hxx"
#include "XMLConverter.hxx"
#include "xmlstyle.hxx"
#include "unonames.hxx"
#include "document.hxx"
#include "olinetab.hxx"
#include "cellsuno.hxx"
#include "cell.hxx"
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
#include "unoguard.hxx"
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

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/families.hxx>
#include <xmloff/numehelp.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/txtparae.hxx>
#include <xmloff/xmlcnitm.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/XMLEventExport.hxx>

#include <rtl/ustring.hxx>

#include <tools/debug.hxx>
#include "tools/color.hxx"
#include <rtl/math.hxx>
#include <svl/zforlist.hxx>
#include <svx/unoshape.hxx>
#include <comphelper/extract.hxx>
#include <editeng/eeitem.hxx>
#include <toolkit/helper/convert.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdocapt.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svditer.hxx>
#include <svx/svdpage.hxx>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/sheet/XUsedAreaCursor.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XAreaLinks.hpp>
#include <com/sun/star/sheet/XAreaLink.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/sheet/XPrintAreas.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/util/XProtectable.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/sheet/XUniqueCellFormatRangesSupplier.hpp>
#include <com/sun/star/sheet/XCellRangesQuery.hpp>
#include <com/sun/star/sheet/CellFlags.hpp>
#include <com/sun/star/util/XMergeable.hpp>
#include <com/sun/star/sheet/XArrayFormulaRange.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/sheet/XLabelRanges.hpp>
#include <com/sun/star/sheet/XLabelRange.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/sheet/XSheetLinkable.hpp>
#include <com/sun/star/form/XFormsSupplier2.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XRangeXMLConversion.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include "XMLCodeNameProvider.hxx"

#include <sfx2/objsh.hxx>

#include <vector>

//! not found in unonames.hxx
#define SC_STANDARDFORMAT "StandardFormat"
#define SC_LAYERID "LayerID"

#define SC_DEFAULT_TABLE_COUNT                      3
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

using namespace rtl;
using namespace formula;
using namespace com::sun::star;
using namespace xmloff::token;
using ::std::vector;
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

    String aVal;        // document uses tools-strings
    if (pDoc)
    {
        ScBaseCell* pCell = pDoc->GetCell( rPos );
        if (pCell)
        {
            CellType eType = pCell->GetCellType();
            if ( eType == CELLTYPE_STRING )
                static_cast<ScStringCell*>(pCell)->GetString(aVal);     // string cell: content
            else if ( eType == CELLTYPE_EDIT )
            {
                // edit cell: text with line breaks
                const EditTextObject* pData = static_cast<ScEditCell*>(pCell)->GetData();
                if (pData)
                {
                    EditEngine& rEngine = pDoc->GetEditEngine();
                    rEngine.SetText( *pData );
                    aVal = rEngine.GetText( LINEEND_LF );
                }
            }
        }
    }
    return aVal;
}
} // anonymous namespace

//----------------------------------------------------------------------------

OUString SAL_CALL ScXMLOOoExport_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLExporter" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL ScXMLOOoExport_getSupportedServiceNames() throw()
{
    const rtl::OUString aServiceName( ScXMLOOoExport_getImplementationName() );
    return uno::Sequence< rtl::OUString >( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLExport(EXPORT_ALL);
    return (cppu::OWeakObject*)new ScXMLExport( rSMgr, EXPORT_ALL );
}

OUString SAL_CALL ScXMLOOoExport_Meta_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLMetaExporter" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL ScXMLOOoExport_Meta_getSupportedServiceNames() throw()
{
    const rtl::OUString aServiceName( ScXMLOOoExport_Meta_getImplementationName() );
    return uno::Sequence< rtl::OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_Meta_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLExport(EXPORT_META);
    return (cppu::OWeakObject*)new ScXMLExport( rSMgr, EXPORT_META );
}

OUString SAL_CALL ScXMLOOoExport_Styles_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLStylesExporter" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL ScXMLOOoExport_Styles_getSupportedServiceNames() throw()
{
    const rtl::OUString aServiceName( ScXMLOOoExport_Styles_getImplementationName() );
    return uno::Sequence< rtl::OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_Styles_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLExport(EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_FONTDECLS);
    return (cppu::OWeakObject*)new ScXMLExport( rSMgr, EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_FONTDECLS);
}

OUString SAL_CALL ScXMLOOoExport_Content_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLContentExporter" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL ScXMLOOoExport_Content_getSupportedServiceNames() throw()
{
    const rtl::OUString aServiceName( ScXMLOOoExport_Content_getImplementationName() );
    return uno::Sequence< rtl::OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_Content_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLExport(EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS);
    return (cppu::OWeakObject*)new ScXMLExport( rSMgr, EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS);
}

OUString SAL_CALL ScXMLOOoExport_Settings_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLSettingsExporter" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL ScXMLOOoExport_Settings_getSupportedServiceNames() throw()
{
    const rtl::OUString aServiceName( ScXMLOOoExport_Settings_getImplementationName() );
    return uno::Sequence< rtl::OUString > ( &aServiceName, 1 );
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOOoExport_Settings_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLExport(EXPORT_SETTINGS);
    return (cppu::OWeakObject*)new ScXMLExport( rSMgr, EXPORT_SETTINGS );
}

// Oasis Filter

OUString SAL_CALL ScXMLOasisExport_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLOasisExporter" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL ScXMLOasisExport_getSupportedServiceNames() throw()
{
    const rtl::OUString aServiceName( ScXMLOasisExport_getImplementationName() );
    const uno::Sequence< rtl::OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new ScXMLExport(rSMgr, EXPORT_ALL|EXPORT_OASIS);
}

OUString SAL_CALL ScXMLOasisExport_Meta_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLOasisMetaExporter" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL ScXMLOasisExport_Meta_getSupportedServiceNames() throw()
{
    const rtl::OUString aServiceName( ScXMLOasisExport_Meta_getImplementationName() );
    const uno::Sequence< rtl::OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_Meta_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new ScXMLExport(rSMgr, EXPORT_META|EXPORT_OASIS);
}

OUString SAL_CALL ScXMLOasisExport_Styles_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLOasisStylesExporter" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL ScXMLOasisExport_Styles_getSupportedServiceNames() throw()
{
    const rtl::OUString aServiceName( ScXMLOasisExport_Styles_getImplementationName() );
    const uno::Sequence< rtl::OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_Styles_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new ScXMLExport(rSMgr, EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_FONTDECLS|EXPORT_OASIS);
}

OUString SAL_CALL ScXMLOasisExport_Content_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLOasisContentExporter" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL ScXMLOasisExport_Content_getSupportedServiceNames() throw()
{
    const rtl::OUString aServiceName( ScXMLOasisExport_Content_getImplementationName() );
    const uno::Sequence< rtl::OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_Content_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new ScXMLExport(rSMgr, EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS|EXPORT_OASIS);
}

OUString SAL_CALL ScXMLOasisExport_Settings_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLOasisSettingsExporter" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL ScXMLOasisExport_Settings_getSupportedServiceNames() throw()
{
    const rtl::OUString aServiceName( ScXMLOasisExport_Settings_getImplementationName() );
    const uno::Sequence< rtl::OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLOasisExport_Settings_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new ScXMLExport(rSMgr, EXPORT_SETTINGS|EXPORT_OASIS);
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
        if( (xShapeProp->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_LAYERID ))) >>= nLayerID) && (nLayerID == SC_LAYER_BACK) )
            GetExport().AddAttribute(XML_NAMESPACE_TABLE, XML_TABLE_BACKGROUND, XML_TRUE);
    }
}

//----------------------------------------------------------------------------

sal_Int16 ScXMLExport::GetFieldUnit()
{
    com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> xProperties(
                comphelper::getProcessServiceFactory()->createInstance(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sheet.GlobalSheetSettings" )) ),
                com::sun::star::uno::UNO_QUERY);
    if (xProperties.is())
    {
        sal_Int16 nFieldUnit = 0;
        if (xProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Metric"))) >>= nFieldUnit)
            return nFieldUnit;
    }
    return 0;
}


// #110680#
ScXMLExport::ScXMLExport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    const sal_uInt16 nExportFlag)
:   SvXMLExport( xServiceFactory, SvXMLUnitConverter::GetMapUnit(GetFieldUnit()), XML_SPREADSHEET, nExportFlag ),
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
    sLayerID(RTL_CONSTASCII_USTRINGPARAM( SC_LAYERID )),
    sCaptionShape(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.CaptionShape")),
    nOpenRow(-1),
    nProgressCount(0),
    nCurrentTable(0),
    bHasRowHeader(sal_False),
    bRowHeaderOpen(sal_False),
    mbShowProgress( sal_False )
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

    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_CELL, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME)),
        xCellStylesExportPropertySetMapper, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX)));
    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_COLUMN, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME)),
        xColumnStylesExportPropertySetMapper, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX)));
    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_ROW, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_ROW_STYLES_NAME)),
        xRowStylesExportPropertySetMapper, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_ROW_STYLES_PREFIX)));
    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_TABLE, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME)),
        xTableStylesExportPropertySetMapper, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_TABLE_STYLES_PREFIX)));

    if( (getExportFlags() & (EXPORT_STYLES|EXPORT_AUTOSTYLES|EXPORT_MASTERSTYLES|EXPORT_CONTENT) ) != 0 )
    {
        // This name is reserved for the external ref cache tables.  This
        // should not conflict with user-defined styles since this name is
        // used for a table style which is not available in the UI.
        sExternalRefTabStyleName = rtl::OUString::createFromAscii("ta_extref");
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
    if (pGroupColumns)
        delete pGroupColumns;
    if (pGroupRows)
        delete pGroupRows;
    if (pColumnStyles)
        delete pColumnStyles;
    if (pRowStyles)
        delete pRowStyles;
    if (pCellStyles)
        delete pCellStyles;
    if (pRowFormatRanges)
        delete pRowFormatRanges;
    if (pMergedRangesContainer)
        delete pMergedRangesContainer;
    if (pValidationsContainer)
        delete pValidationsContainer;
    if (pChangeTrackingExportHelper)
        delete pChangeTrackingExportHelper;
    if (pChartListener)
        delete pChartListener;
    if (pCellsItr)
        delete pCellsItr;
    if (pDefaults)
        delete pDefaults;
    if (pNumberFormatAttributesExportHelper)
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

        if ( nRead != nLen || rtl_compareMemory( aFileStart.getConstArray(), pXmlHeader, nLen ) != 0 )
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

sal_Bool ScXMLExport::HasDrawPages(uno::Reference <sheet::XSpreadsheetDocument>& xDoc)
{
    uno::Reference <beans::XPropertySet> xDocProps( xDoc, uno::UNO_QUERY );
    return (xDocProps.is() && ::cppu::any2bool( xDocProps->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_HASDRAWPAGES))) ));
}

void ScXMLExport::CollectSharedData(sal_Int32& nTableCount, sal_Int32& nShapesCount, const sal_Int32 nCellCount)
{
    if (GetModel().is())
    {
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( GetModel(), uno::UNO_QUERY );
        if ( xSpreadDoc.is())
        {
            uno::Reference<container::XIndexAccess> xIndex( xSpreadDoc->getSheets(), uno::UNO_QUERY );
            if ( xIndex.is() )
            {
                nTableCount = xIndex->getCount();
                if (!pSharedData)
                    CreateSharedData(nTableCount);
                pCellStyles->AddNewTable(nTableCount - 1);
                pDoc->InitializeAllNoteCaptions( true );
                if (HasDrawPages(xSpreadDoc))
                {
                    rtl::OUString sCaptionPoint( RTL_CONSTASCII_USTRINGPARAM( "CaptionPoint" ));
                    for (SCTAB nTable = 0; nTable < nTableCount; ++nTable)
                    {
                        nCurrentTable = sal::static_int_cast<sal_uInt16>( nTable );
                        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xIndex->getByIndex(nTable), uno::UNO_QUERY);
                        if (xDrawPageSupplier.is())
                        {
                            uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage());
                            ScMyDrawPage aDrawPage;
                            aDrawPage.bHasForms = sal_False;
                            aDrawPage.xDrawPage.set(xDrawPage);
                            pSharedData->AddDrawPage(aDrawPage, nTable);
                            uno::Reference<container::XIndexAccess> xShapesIndex (xDrawPage, uno::UNO_QUERY);
                            if (xShapesIndex.is())
                            {
                                sal_Int32 nShapes(xShapesIndex->getCount());
                                for (sal_Int32 nShape = 0; nShape < nShapes; ++nShape)
                                {
                                    uno::Reference<drawing::XShape> xShape(xShapesIndex->getByIndex(nShape), uno::UNO_QUERY);
                                    if (xShape.is())
                                    {
                                        uno::Reference< beans::XPropertySet > xShapeProp( xShape, uno::UNO_QUERY );
                                        if( xShapeProp.is() )
                                        {
                                            sal_Int16 nLayerID = 0;
                                            if( xShapeProp->getPropertyValue(sLayerID) >>= nLayerID )
                                            {
                                                if( (nLayerID == SC_LAYER_INTERN) || (nLayerID == SC_LAYER_HIDDEN) )
                                                    CollectInternalShape( xShape );
                                                else
                                                {
                                                    ++nShapesCount;
                                                    SvxShape* pShapeImp(SvxShape::getImplementation(xShape));
                                                    if (pShapeImp)
                                                    {
                                                        SdrObject *pSdrObj(pShapeImp->GetSdrObject());
                                                        if (pSdrObj)
                                                        {
                                                            if (ScDrawLayer::GetAnchor(pSdrObj) == SCA_CELL)
                                                            {
                                                                if (pDoc)
                                                                {

                                                                    awt::Point aPoint(xShape->getPosition());
                                                                    awt::Size aSize(xShape->getSize());
                                                                    rtl::OUString sType(xShape->getShapeType());
                                                                    Rectangle aRectangle(aPoint.X, aPoint.Y, aPoint.X + aSize.Width, aPoint.Y + aSize.Height);
                                                                    if ( sType.equals(sCaptionShape) )
                                                                    {
                                                                        awt::Point aRelativeCaptionPoint;
                                                                        xShapeProp->getPropertyValue( sCaptionPoint ) >>= aRelativeCaptionPoint;
                                                                        Point aCoreRelativeCaptionPoint(aRelativeCaptionPoint.X, aRelativeCaptionPoint.Y);
                                                                        Point aCoreAbsoluteCaptionPoint(aPoint.X, aPoint.Y);
                                                                        aCoreAbsoluteCaptionPoint += aCoreRelativeCaptionPoint;
                                                                        aRectangle.Union(Rectangle(aCoreAbsoluteCaptionPoint, aCoreAbsoluteCaptionPoint));
                                                                    }
                                                                    ScRange aRange(pDoc->GetRange(static_cast<SCTAB>(nTable), aRectangle));
                                                                    ScMyShape aMyShape;
                                                                    aMyShape.aAddress = aRange.aStart;
                                                                    aMyShape.aEndAddress = aRange.aEnd;
                                                                    aMyShape.xShape = xShape;
                                                                    pSharedData->AddNewShape(aMyShape);
                                                                    pSharedData->SetLastColumn(nTable, aRange.aStart.Col());
                                                                    pSharedData->SetLastRow(nTable, aRange.aStart.Row());
                                                                }
                                                            }
                                                            else
                                                                pSharedData->AddTableShape(nTable, xShape);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    sal_Int32 nRef(nCellCount + (2 * nTableCount) + (2 * nShapesCount));
    GetProgressBarHelper()->SetReference(nRef);
    GetProgressBarHelper()->SetValue(0);
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
                    pSharedData->SetDrawPageHasForms(nTable, sal_True);
                }
                ScMyTableShapes* pTableShapes(pSharedData->GetTableShapes());
                if (pTableShapes)
                {
                    ScMyTableXShapes::iterator aItr((*pTableShapes)[nTable].begin());
                    ScMyTableXShapes::iterator aEndItr((*pTableShapes)[nTable].end());
                    while (aItr != aEndItr)
                    {
                        GetShapeExport()->collectShapeAutoStyles(*aItr);
                        IncrementProgressBar(sal_False);
                        ++aItr;
                    }
                }
                if (pShapeList)
                {
                    ScMyShapeList::const_iterator aEndItr(pShapeList->end());
                    while (aShapeItr != aEndItr && (static_cast<sal_Int32>(aShapeItr->aAddress.Tab()) == nTable))
                    {
                        GetShapeExport()->collectShapeAutoStyles(aShapeItr->xShape);
                        IncrementProgressBar(sal_False);
                        ++aShapeItr;
                    }
                }
                const ScMyNoteShapeList* pNoteShapes = NULL;
                ScMyNoteShapeList::const_iterator aNoteShapeItr;
                ScMyNoteShapeList::const_iterator aNoteShapeEndItr;
                if (pSharedData->GetNoteShapes())
                {
                    pNoteShapes = pSharedData->GetNoteShapes()->GetNotes();
                    if (pNoteShapes)
                    {
                        aNoteShapeItr = pNoteShapes->begin();
                        aNoteShapeEndItr = pNoteShapes->end();
                    }
                }
                if (pNoteShapes)
                {
                    while (aNoteShapeItr != aNoteShapeEndItr)
                    {
                        if (static_cast<sal_Int32>(aNoteShapeItr->aPos.Tab()) == nTable)
                            GetShapeExport()->collectShapeAutoStyles(aNoteShapeItr->xShape);
                        ++aNoteShapeItr;
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
    CollectSharedData(nTableCount, nShapesCount, nCellCount);

    uno::Sequence<beans::NamedValue> stats(3);
    stats[0] = beans::NamedValue(::rtl::OUString::createFromAscii("TableCount"),
                uno::makeAny(nTableCount));
    stats[1] = beans::NamedValue(::rtl::OUString::createFromAscii("CellCount"),
                uno::makeAny(nCellCount));
    stats[2] = beans::NamedValue(::rtl::OUString::createFromAscii("ObjectCount"),
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
        xUsedArea->gotoEndOfUsedArea(sal_True);
        aCellAddress = xCellAddress->getRangeAddress();
    }
    return aCellAddress;
}

void ScXMLExport::GetAreaLinks( uno::Reference< sheet::XSpreadsheetDocument>& xSpreadDoc,
                                ScMyAreaLinksContainer& rAreaLinks )
{
    uno::Reference< beans::XPropertySet > xPropSet( xSpreadDoc, uno::UNO_QUERY );
    if( !xPropSet.is() ) return;

    uno::Reference< container::XIndexAccess > xLinksIAccess( xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_AREALINKS ) ) ), uno::UNO_QUERY);
    if( xLinksIAccess.is() )
    {
        const OUString sFilter( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_FILTER ) );
        const OUString sFilterOpt( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_FILTOPT ) );
        const OUString sURL( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_LINKURL ) );
        const OUString sRefresh( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_REFDELAY ) );

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
            sal_uInt32 nCount(pOpList->Count());
            for( sal_uInt32 nIndex = 0; nIndex < nCount; ++nIndex )
            {
                ScDetOpData* pDetData(pOpList->GetObject( static_cast<sal_uInt16>(nIndex) ));
                if( pDetData )
                {
                    const ScAddress& rDetPos = pDetData->GetPos();
                    SCTAB nTab = rDetPos.Tab();
                    if ( nTab < pDoc->GetTableCount() )
                    {
                        rDetOp.AddOperation( pDetData->GetOperation(), rDetPos, nIndex );

                        // #123981# cells with detective operations are written even if empty
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
    const sal_Int32 nIndex, const sal_Bool bIsAutoStyle, const sal_Bool bIsVisible)
{
    CheckAttrList();
    AddAttribute(sAttrStyleName, *pColumnStyles->GetStyleNameByIndex(nStyleIndex));
    if (!bIsVisible)
        AddAttribute(XML_NAMESPACE_TABLE, XML_VISIBILITY, XML_COLLAPSE);
    if (nRepeatColumns > 1)
    {
        OUString sOUEndCol(OUString::valueOf(static_cast <sal_Int32> (nRepeatColumns)));
        AddAttribute(sAttrColumnsRepeated, sOUEndCol);
    }
    if (nIndex != -1)
        AddAttribute(XML_NAMESPACE_TABLE, XML_DEFAULT_CELL_STYLE_NAME, *pCellStyles->GetStyleNameByIndex(nIndex, bIsAutoStyle));
    SvXMLElementExport aElemC(*this, sElemCol, sal_True, sal_True);
}

void ScXMLExport::WriteColumn(const sal_Int32 nColumn, const sal_Int32 nRepeatColumns,
    const sal_Int32 nStyleIndex, const sal_Bool bIsVisible)
{
    sal_Int32 nRepeat(1);
    sal_Int32 nPrevIndex((*pDefaults->GetColDefaults())[nColumn].nIndex);
    sal_Bool bPrevAutoStyle((*pDefaults->GetColDefaults())[nColumn].bIsAutoStyle);
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
    StartElement( XML_NAMESPACE_TABLE, XML_TABLE_HEADER_COLUMNS, sal_True );
}

void ScXMLExport::CloseHeaderColumn()
{
    EndElement(XML_NAMESPACE_TABLE, XML_TABLE_HEADER_COLUMNS, sal_True);
}

void ScXMLExport::ExportColumns(const sal_Int32 nTable, const table::CellRangeAddress& aColumnHeaderRange, const sal_Bool bHasColumnHeader)
{
    sal_Int32 nColsRepeated (1);
    rtl::OUString sParent;
    sal_Int32 nIndex;
    sal_Int32 nPrevColumn(0);
    sal_Bool bPrevIsVisible (sal_True);
    sal_Bool bWasHeader (sal_False);
    sal_Bool bIsHeader (sal_False);
    sal_Bool bIsClosed (sal_True);
    sal_Bool bIsFirst (sal_False);
    sal_Int32 nPrevIndex (-1);
    sal_Int32 nColumn;
    for (nColumn = 0; nColumn <= pSharedData->GetLastColumn(nTable); ++nColumn)
    {
        CheckAttrList();
        sal_Bool bIsVisible(sal_True);
        nIndex = pColumnStyles->GetStyleNameIndex(nTable, nColumn, bIsVisible);

        bIsHeader = bHasColumnHeader && (aColumnHeaderRange.StartColumn <= nColumn) && (nColumn <= aColumnHeaderRange.EndColumn);
        if (bIsHeader != bWasHeader)
        {
            if (bIsHeader)
            {
                bIsFirst = sal_False;
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
                bIsFirst = sal_True;
                if(pGroupColumns->IsGroupStart(nColumn))
                    pGroupColumns->OpenGroups(nColumn);
                OpenHeaderColumn();
                bWasHeader = sal_True;
                bIsClosed = sal_False;
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
                bWasHeader = sal_False;
                bIsClosed = sal_True;
            }
        }
        else if (nColumn == 0)
        {
            if (pGroupColumns->IsGroupStart(nColumn))
                pGroupColumns->OpenGroups(nColumn);
            bPrevIsVisible = bIsVisible;
            nPrevIndex = nIndex;
            bIsFirst = sal_True;
        }
        else if ((bIsVisible == bPrevIsVisible) && (nIndex == nPrevIndex) &&
            !pGroupColumns->IsGroupStart(nColumn) && !pGroupColumns->IsGroupEnd(nColumn - 1))
            ++nColsRepeated;
        else
        {
            bIsFirst = sal_False;
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
    //if (nColsRepeated > 1 || bIsFirst)
        WriteColumn(nPrevColumn, nColsRepeated, nPrevIndex, bPrevIsVisible);
    if (!bIsClosed)
        CloseHeaderColumn();
    if (pGroupColumns->IsGroupEnd(nColumn - 1))
        pGroupColumns->CloseGroups(nColumn - 1);
}

void ScXMLExport::ExportExternalRefCacheStyles()
{
    sal_Int32 nEntryIndex = GetCellStylesPropertySetMapper()->FindEntryIndex(
        "NumberFormat", XML_NAMESPACE_STYLE, OUString::createFromAscii("data-style-name"));

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
    const OUString aDefaultStyle = OUString::createFromAscii("Default").intern();
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
            sal_Bool bIsAuto;
            nIndex = pCellStyles->GetIndexOfStyleName(
                aName, OUString::createFromAscii(XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX), bIsAuto);
        }

        // store the number format to index mapping for later use.
        aNumFmtIndexMap.insert(NumberFormatIndexMap::value_type(nNumFmt, nIndex));
    }
}

void ScXMLExport::WriteRowContent()
{
    ScMyRowFormatRange aRange;
    sal_Int32 nIndex(-1);
#ifdef DBG_UTIL
    sal_Int32 nPrevCol(0);
#endif
    sal_Int32 nCols(0);
    sal_Int32 nPrevValidationIndex(-1);
    sal_Bool bIsAutoStyle(sal_True);
    sal_Bool bIsFirst(sal_True);
    while (pRowFormatRanges->GetNext(aRange))
    {
#ifdef DBG_UTIL
        DBG_ASSERT(bIsFirst || (!bIsFirst && (nPrevCol + nCols == aRange.nStartColumn)), "here are some columns missing");
#endif
        if (bIsFirst)
        {
            nIndex = aRange.nIndex;
            nPrevValidationIndex = aRange.nValidationIndex;
            bIsAutoStyle = aRange.bIsAutoStyle;
            nCols = aRange.nRepeatColumns;
            bIsFirst = sal_False;
#ifdef DBG_UTIL
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
                    rtl::OUStringBuffer aBuf;
                    GetMM100UnitConverter().convertNumber(aBuf, nCols);
                    AddAttribute(sAttrColumnsRepeated, aBuf.makeStringAndClear());
                }
                SvXMLElementExport aElemC(*this, sElemCell, sal_True, sal_True);
                nIndex = aRange.nIndex;
                bIsAutoStyle = aRange.bIsAutoStyle;
                nCols = aRange.nRepeatColumns;
                nPrevValidationIndex = aRange.nValidationIndex;
#ifdef DBG_UTIL
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
            rtl::OUStringBuffer aBuf;
            GetMM100UnitConverter().convertNumber(aBuf, nCols);
            AddAttribute(sAttrColumnsRepeated, aBuf.makeStringAndClear());
        }
        SvXMLElementExport aElemC(*this, sElemCell, sal_True, sal_True);
    }
}

void ScXMLExport::WriteRowStartTag(sal_Int32 nRow, const sal_Int32 nIndex,
    const sal_Int8 nFlag, const sal_Int32 nEqualRows)
{
    AddAttribute(sAttrStyleName, *pRowStyles->GetStyleNameByIndex(nIndex));
    if (nFlag)
        if (nFlag & CR_HIDDEN)
        {
            if (nFlag & CR_FILTERED)
                AddAttribute(XML_NAMESPACE_TABLE, XML_VISIBILITY, XML_FILTER);
            else
                AddAttribute(XML_NAMESPACE_TABLE, XML_VISIBILITY, XML_COLLAPSE);
        }
    if (nEqualRows > 1)
    {
        rtl::OUStringBuffer aBuf;
        GetMM100UnitConverter().convertNumber(aBuf, nEqualRows);
        AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_ROWS_REPEATED, aBuf.makeStringAndClear());
    }

    const ScMyDefaultStyleList& rRowDefaults = *pDefaults->GetRowDefaults();
    if ( nRow >= sal::static_int_cast<sal_Int32>( rRowDefaults.size() ) )
    {
        // #123981# used to happen with detective operations - if there are more cases, use the last row's style
        DBG_ERRORFILE("WriteRowStartTag: not enough defaults");
        nRow = rRowDefaults.size() - 1;
    }
    sal_Int32 nCellStyleIndex(rRowDefaults[nRow].nIndex);
    if (nCellStyleIndex != -1)
        AddAttribute(XML_NAMESPACE_TABLE, XML_DEFAULT_CELL_STYLE_NAME,
            *pCellStyles->GetStyleNameByIndex(nCellStyleIndex,
                (*pDefaults->GetRowDefaults())[nRow].bIsAutoStyle));
    StartElement( sElemRow, sal_True);
}

void ScXMLExport::OpenHeaderRows()
{
    StartElement( XML_NAMESPACE_TABLE, XML_TABLE_HEADER_ROWS, sal_True);
    bRowHeaderOpen = sal_True;
}

void ScXMLExport::CloseHeaderRows()
{
    EndElement(XML_NAMESPACE_TABLE, XML_TABLE_HEADER_ROWS, sal_True);
}

void ScXMLExport::OpenNewRow(const sal_Int32 nIndex, const sal_Int8 nFlag, const sal_Int32 nStartRow, const sal_Int32 nEqualRows)
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
        WriteRowStartTag(nStartRow, nIndex, nFlag, nEquals);
        nOpenRow = nStartRow + nEquals - 1;
        if (nEquals < nEqualRows)
        {
            CloseRow(nStartRow + nEquals - 1);
            WriteRowStartTag(nStartRow, nIndex, nFlag, nEqualRows - nEquals);
            nOpenRow = nStartRow + nEqualRows - 1;
        }
    }
    else
        WriteRowStartTag(nStartRow, nIndex, nFlag, nEqualRows);
}

void ScXMLExport::OpenAndCloseRow(const sal_Int32 nIndex, const sal_Int8 nFlag,
    const sal_Int32 nStartRow, const sal_Int32 nEqualRows)
{
    OpenNewRow(nIndex, nFlag, nStartRow, nEqualRows);
    WriteRowContent();
    CloseRow(nStartRow + nEqualRows - 1);
    pRowFormatRanges->Clear();
}

void ScXMLExport::OpenRow(const sal_Int32 nTable, const sal_Int32 nStartRow, const sal_Int32 nRepeatRow)
{
    if (nRepeatRow > 1)
    {
        sal_Int32 nPrevIndex(0), nIndex;
        sal_Int8 nPrevFlag(0);
        sal_Int8 nFlag(0);
        sal_Int32 nEqualRows(1);
        sal_Int32 nEndRow(nStartRow + nRepeatRow);
        sal_Int32 nRow;
        for (nRow = nStartRow; nRow < nEndRow; ++nRow)
        {
            if (nRow == nStartRow)
            {
                nPrevIndex = pRowStyles->GetStyleNameIndex(nTable, nRow);
                if (pDoc)
                    nPrevFlag = (pDoc->GetRowFlags(static_cast<SCROW>(nRow), static_cast<SCTAB>(nTable))) & (CR_HIDDEN | CR_FILTERED);
            }
            else
            {
                nIndex = pRowStyles->GetStyleNameIndex(nTable, nRow);
                if (pDoc)
                    nFlag = (pDoc->GetRowFlags(static_cast<SCROW>(nRow), static_cast<SCTAB>(nTable))) & (CR_HIDDEN | CR_FILTERED);
                if (nIndex == nPrevIndex && nFlag == nPrevFlag &&
                    !(bHasRowHeader && ((nRow == aRowHeaderRange.StartRow) || (nRow - 1 == aRowHeaderRange.EndRow))) &&
                    !(pGroupRows->IsGroupStart(nRow)) &&
                    !(pGroupRows->IsGroupEnd(nRow - 1)))
                    ++nEqualRows;
                else
                {
                    if (nRow < nEndRow)
                    {
                        ScRowFormatRanges* pTempRowFormatRanges = new ScRowFormatRanges(pRowFormatRanges);
                        OpenAndCloseRow(nPrevIndex, nPrevFlag, nRow - nEqualRows, nEqualRows);
                        delete pRowFormatRanges;
                        pRowFormatRanges = pTempRowFormatRanges;
                    }
                    else
                        OpenAndCloseRow(nPrevIndex, nPrevFlag, nRow - nEqualRows, nEqualRows);
                    nEqualRows = 1;
                    nPrevIndex = nIndex;
                    nPrevFlag = nFlag;
                }
            }
        }
        OpenNewRow(nPrevIndex, nPrevFlag, nRow - nEqualRows, nEqualRows);
    }
    else
    {
        sal_Int32 nIndex = pRowStyles->GetStyleNameIndex(nTable, nStartRow);
        sal_Int8 nFlag(0);
        if (pDoc)
            nFlag = (pDoc->GetRowFlags(static_cast<SCROW>(nStartRow), static_cast<SCTAB>(nTable))) & (CR_HIDDEN | CR_FILTERED);
        OpenNewRow(nIndex, nFlag, nStartRow, 1);
    }
    nOpenRow = nStartRow + nRepeatRow - 1;
}

void ScXMLExport::CloseRow(const sal_Int32 nRow)
{
    if (nOpenRow > -1)
    {
        EndElement(sElemRow, sal_True);
        if (bHasRowHeader && nRow == aRowHeaderRange.EndRow)
        {
            CloseHeaderRows();
            bRowHeaderOpen = sal_False;
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
    if (nStartRow == nEndRow)
    {
        pCellStyles->GetFormatRanges(nStartCol, nEndCol, nStartRow, nSheet, pRowFormatRanges);
        if (nOpenRow == - 1)
            OpenRow(nSheet, nStartRow, 1);
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
                DBG_ASSERT(nMaxRows, "something wents wrong");
                if (nMaxRows >= nTotalRows - nRows)
                {
                    OpenRow(nSheet, nStartRow + nRows, nTotalRows - nRows);
                    nRows += nTotalRows - nRows;
                }
                else
                {
                    OpenRow(nSheet, nStartRow + nRows, nMaxRows);
                    nRows += nMaxRows;
                }
                if (!pRowFormatRanges->GetSize())
                    pCellStyles->GetFormatRanges(0, pSharedData->GetLastColumn(nSheet), nStartRow + nRows, nSheet, pRowFormatRanges);
                WriteRowContent();
                CloseRow(nStartRow + nRows - 1);
            }
            if (nTotalRows == 1)
                CloseRow(nStartRow);
            OpenRow(nSheet, nEndRow, 1);
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
                    OpenRow(nSheet, nStartRow + nRows, nTotalRows - nRows);
                    nRows += nTotalRows - nRows;
                }
                else
                {
                    OpenRow(nSheet, nStartRow + nRows, nMaxRows);
                    nRows += nMaxRows;
                }
                if (!pRowFormatRanges->GetSize())
                    pCellStyles->GetFormatRanges(0, pSharedData->GetLastColumn(nSheet), nStartRow + nRows, nSheet, pRowFormatRanges);
                WriteRowContent();
                CloseRow(nStartRow + nRows - 1);
            }
            OpenRow(nSheet, nEndRow, 1);
            pRowFormatRanges->Clear();
            pCellStyles->GetFormatRanges(0, nEndCol, nEndRow, nSheet, pRowFormatRanges);
            WriteRowContent();
        }
    }
}

void ScXMLExport::GetColumnRowHeader(sal_Bool& rHasColumnHeader, table::CellRangeAddress& rColumnHeaderRange,
                                     sal_Bool& rHasRowHeader, table::CellRangeAddress& rRowHeaderRange,
                                     rtl::OUString& rPrintRanges) const
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
    sal_Int32 nDepth(pFields->GetDepth());
    for(sal_Int32 i = 0; i < nDepth; ++i)
    {
        sal_Int32 nFields = pFields->GetCount(static_cast<sal_uInt16>(i));
        for (sal_Int32 j = 0; j < nFields; ++j)
        {
            ScMyColumnRowGroup aGroup;
            ScOutlineEntry* pEntry(pFields->GetEntry(static_cast<sal_uInt16>(i), static_cast<sal_uInt16>(j)));
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
        ScOutlineTable* pOutlineTable = pDoc->GetOutlineTable( static_cast<SCTAB>(nCurrentTable), sal_False );
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
        rtl::OUStringBuffer aBuffer;
        uno::Sequence<sal_Int8> aPassHash;
        const ScDocProtection* p = pDoc->GetDocProtection();
        if (p)
            aPassHash = p->getPasswordHash(PASSHASH_OOO);
        SvXMLUnitConverter::encodeBase64(aBuffer, aPassHash);
        if (aBuffer.getLength())
            AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY, aBuffer.makeStringAndClear());
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
                ByteString aOutStr("\n   ");
                uno::Sequence<sal_Int8> aOutSeq( (sal_Int8*)aOutStr.GetBuffer(), aOutStr.Len() );
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

void ScXMLExport::_ExportContent()
{
    nCurrentTable = 0;
    if (!pSharedData)
    {
        sal_Int32 nTableCount(0);
        sal_Int32 nShapesCount(0);
        sal_Int32 nCellCount(pDoc ? pDoc->GetCellCount() : 0);
        CollectSharedData(nTableCount, nShapesCount, nCellCount);
        DBG_ERROR("no shared data setted");
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
            if (pSheetData && pDoc && pDoc->IsStreamValid((SCTAB)nTable))
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
                if (xTable.is())
                {
                    xCurrentTable.set(xTable);
                    xCurrentTableCellRange.set(xTable, uno::UNO_QUERY);
                    uno::Reference<container::XNamed> xName (xTable, uno::UNO_QUERY );
                    if ( xName.is() )
                    {
                        nCurrentTable = sal::static_int_cast<sal_uInt16>( nTable );
                        rtl::OUString sOUTableName(xName->getName());
                        AddAttribute(sAttrName, sOUTableName);
                        AddAttribute(sAttrStyleName, aTableStyles[nTable]);

                        uno::Reference<util::XProtectable> xProtectable (xTable, uno::UNO_QUERY);
                        if (xProtectable.is() && xProtectable->isProtected())
                        {
                            AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTED, XML_TRUE);
                            rtl::OUStringBuffer aBuffer;
                            if (pDoc)
                            {
                                ScTableProtection* pProtect = pDoc->GetTabProtection(static_cast<SCTAB>(nTable));
                                if (pProtect)
                                    SvXMLUnitConverter::encodeBase64(aBuffer, pProtect->getPasswordHash(PASSHASH_OOO));
                            }
                            if (aBuffer.getLength())
                                AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY, aBuffer.makeStringAndClear());
                        }
                        rtl::OUString sPrintRanges;
                        table::CellRangeAddress aColumnHeaderRange;
                        sal_Bool bHasColumnHeader;
                        GetColumnRowHeader(bHasColumnHeader, aColumnHeaderRange, bHasRowHeader, aRowHeaderRange, sPrintRanges);
                        if( sPrintRanges.getLength() )
                            AddAttribute( XML_NAMESPACE_TABLE, XML_PRINT_RANGES, sPrintRanges );
                        else if (!pDoc->IsPrintEntireSheet(static_cast<SCTAB>(nTable)))
                            AddAttribute( XML_NAMESPACE_TABLE, XML_PRINT, XML_FALSE);
                        SvXMLElementExport aElemT(*this, sElemTab, sal_True, sal_True);
                        CheckAttrList();

                        if ( pDoc && pDoc->GetSheetEvents( static_cast<SCTAB>(nTable) ) &&
                             getDefaultVersion() == SvtSaveOptions::ODFVER_LATEST )
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
                            sal_Bool bRet(GetFormExport()->seekPage( xDrawPage ));
                            DBG_ASSERT( bRet, "OFormLayerXMLExport::seekPage failed!" );
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
                        sal_Bool bIsFirst(sal_True);
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
                                        SetRepeatAttribute(nEqualCells);
                                        WriteCell(aPrevCell);
                                        nEqualCells = 0;
                                        aPrevCell = aCell;
                                    }
                                }
                                else
                                {
                                    SetRepeatAttribute(nEqualCells);
                                    WriteCell(aPrevCell);
                                    ExportFormatRanges(aPrevCell.aCellAddress.Column + nEqualCells + 1, aPrevCell.aCellAddress.Row,
                                        aCell.aCellAddress.Column - 1, aCell.aCellAddress.Row, nTable);
                                    nEqualCells = 0;
                                    aPrevCell = aCell;
                                }
                            }
                        }
                        if (!bIsFirst)
                        {
                            SetRepeatAttribute(nEqualCells);
                            WriteCell(aPrevCell);
                            ExportFormatRanges(aPrevCell.aCellAddress.Column + nEqualCells + 1, aPrevCell.aCellAddress.Row,
                                pSharedData->GetLastColumn(nTable), pSharedData->GetLastRow(nTable), nTable);
                        }
                        else
                            ExportFormatRanges(0, 0, pSharedData->GetLastColumn(nTable), pSharedData->GetLastRow(nTable), nTable);
                        CloseRow(pSharedData->GetLastRow(nTable));
                        nEqualCells = 0;
                    }
                }
            }
            IncrementProgressBar(sal_False);
        }
    }
    WriteExternalRefCaches();
    WriteNamedExpressions(xSpreadDoc);
    aExportDatabaseRanges.WriteDatabaseRanges(xSpreadDoc);
    ScXMLExportDataPilot aExportDataPilot(*this);
    aExportDataPilot.WriteDataPilots(xSpreadDoc);
    WriteConsolidation();
    ScXMLExportDDELinks aExportDDELinks(*this);
    aExportDDELinks.WriteDDELinks(xSpreadDoc);
    IncrementProgressBar(sal_True, 0);
    GetProgressBarHelper()->SetValue(GetProgressBarHelper()->GetReference());
}

void ScXMLExport::_ExportStyles( sal_Bool bUsed )
{
    if (!pSharedData)
    {
        sal_Int32 nTableCount(0);
        sal_Int32 nShapesCount(0);
        sal_Int32 nCellCount(pDoc ? pDoc->GetCellCount() : 0);
        CollectSharedData(nTableCount, nShapesCount, nCellCount);
        //DBG_ERROR("no shared data setted");
    }
    ScXMLStyleExport aStylesExp(*this, rtl::OUString(), GetAutoStylePool().get());
    if (GetModel().is())
    {
        uno::Reference <lang::XMultiServiceFactory> xMultiServiceFactory(GetModel(), uno::UNO_QUERY);
        if (xMultiServiceFactory.is())
        {
            uno::Reference <beans::XPropertySet> xProperties(xMultiServiceFactory->createInstance(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.Defaults"))), uno::UNO_QUERY);
            if (xProperties.is())
                aStylesExp.exportDefaultStyle(xProperties, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME)), xCellStylesExportPropertySetMapper);
            if (pSharedData->HasShapes())
            {
                GetShapeExport()->ExportGraphicDefaults();
/*              xInterface = xMultiServiceFactory->createInstance(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Defaults")));
                uno::Reference <beans::XPropertySet> xDrawProperties(xInterface, uno::UNO_QUERY);
                if (xDrawProperties.is())
                    aStylesExp.exportDefaultStyle(xDrawProperties, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_GRAPHICS_NAME)), GetShapeExport()->CreateShapePropMapper(*this));*/
            }
        }
        uno::Reference <style::XStyleFamiliesSupplier> xStyleFamiliesSupplier (GetModel(), uno::UNO_QUERY);
        if (xStyleFamiliesSupplier.is())
        {
            uno::Reference <container::XNameAccess> xStylesFamilies(xStyleFamiliesSupplier->getStyleFamilies());
            if (xStylesFamilies.is())
            {
                uno::Reference <container::XIndexAccess> xCellStyles(xStylesFamilies->getByName(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CellStyles"))), uno::UNO_QUERY);
                if (xCellStyles.is())
                {
                    sal_Int32 nCount(xCellStyles->getCount());
                    rtl::OUString sNumberFormat(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_NUMFMT));
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

    aStylesExp.exportStyleFamily(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CellStyles")),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME)), xCellStylesExportPropertySetMapper, sal_False, XML_STYLE_FAMILY_TABLE_CELL);

    SvXMLExport::_ExportStyles(bUsed);
}

void ScXMLExport::AddStyleFromCells(const uno::Reference<beans::XPropertySet>& xProperties,
                                    const uno::Reference<sheet::XSpreadsheet>& xTable,
                                    sal_Int32 nTable, const rtl::OUString* pOldName)
{
    //! pass xCellRanges instead
    uno::Reference<sheet::XSheetCellRanges> xCellRanges( xProperties, uno::UNO_QUERY );

    rtl::OUString SC_SCELLPREFIX(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX));
    rtl::OUString SC_NUMBERFORMAT(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_NUMFMT));

    rtl::OUString sStyleName;
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
                    aEndItr = xPropStates.end();    // #120346# old aEndItr is invalidated!
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
    if (sStyleName.getLength())
    {
        if (xPropStates.size())
        {
            sal_Int32 nIndex;
            if (pOldName)
            {
                if (GetAutoStylePool()->AddNamed(*pOldName, XML_STYLE_FAMILY_TABLE_CELL, sStyleName, xPropStates))
                {
                    GetAutoStylePool()->RegisterName(XML_STYLE_FAMILY_TABLE_CELL, *pOldName);
                    // add to pCellStyles, so the name is found for normal sheets
                    rtl::OUString* pTemp(new rtl::OUString(*pOldName));
                    if (!pCellStyles->AddStyleName(pTemp, nIndex))
                        delete pTemp;
                }
            }
            else
            {
                rtl::OUString sName;
                sal_Bool bIsAutoStyle(sal_True);
                if (GetAutoStylePool()->Add(sName, XML_STYLE_FAMILY_TABLE_CELL, sStyleName, xPropStates))
                {
                    rtl::OUString* pTemp(new rtl::OUString(sName));
                    if (!pCellStyles->AddStyleName(pTemp, nIndex))
                        delete pTemp;
                }
                else
                    nIndex = pCellStyles->GetIndexOfStyleName(sName, SC_SCELLPREFIX, bIsAutoStyle);

                uno::Sequence<table::CellRangeAddress> aAddresses(xCellRanges->getRangeAddresses());
                table::CellRangeAddress* pAddresses(aAddresses.getArray());
                sal_Bool bGetMerge(sal_True);
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
            rtl::OUString* pTemp(new rtl::OUString(EncodeStyleName(sStyleName)));
            sal_Int32 nIndex(0);
            if (!pCellStyles->AddStyleName(pTemp, nIndex, sal_False))
            {
                delete pTemp;
                pTemp = NULL;
            }
            if ( !pOldName )
            {
                uno::Sequence<table::CellRangeAddress> aAddresses(xCellRanges->getRangeAddresses());
                table::CellRangeAddress* pAddresses(aAddresses.getArray());
                sal_Bool bGetMerge(sal_True);
                for (sal_Int32 i = 0; i < aAddresses.getLength(); ++i, ++pAddresses)
                {
                    if (bGetMerge)
                        bGetMerge = GetMerged(pAddresses, xTable);
                    pCellStyles->AddRangeStyleName(*pAddresses, nIndex, sal_False, nValidationIndex, nNumberFormat);
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
                                     const rtl::OUString* pOldName, sal_Int32& rIndex, sal_Bool& rIsVisible)
{
    rtl::OUString SC_SCOLUMNPREFIX(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX));

    std::vector<XMLPropertyState> xPropStates(xColumnStylesExportPropertySetMapper->Filter(xColumnProperties));
    if(xPropStates.size())
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

        rtl::OUString sParent;
        if (pOldName)
        {
            if (GetAutoStylePool()->AddNamed(*pOldName, XML_STYLE_FAMILY_TABLE_COLUMN, sParent, xPropStates))
            {
                GetAutoStylePool()->RegisterName(XML_STYLE_FAMILY_TABLE_COLUMN, *pOldName);
                // add to pColumnStyles, so the name is found for normal sheets
                rtl::OUString* pTemp(new rtl::OUString(*pOldName));
                rIndex = pColumnStyles->AddStyleName(pTemp);
            }
        }
        else
        {
            rtl::OUString sName;
            if (GetAutoStylePool()->Add(sName, XML_STYLE_FAMILY_TABLE_COLUMN, sParent, xPropStates))
            {
                rtl::OUString* pTemp(new rtl::OUString(sName));
                rIndex = pColumnStyles->AddStyleName(pTemp);
            }
            else
                rIndex = pColumnStyles->GetIndexOfStyleName(sName, SC_SCOLUMNPREFIX);
        }
    }
}

void ScXMLExport::AddStyleFromRow(const uno::Reference<beans::XPropertySet>& xRowProperties,
                                  const rtl::OUString* pOldName, sal_Int32& rIndex)
{
    rtl::OUString SC_SROWPREFIX(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_ROW_STYLES_PREFIX));

    std::vector<XMLPropertyState> xPropStates(xRowStylesExportPropertySetMapper->Filter(xRowProperties));
    if(xPropStates.size())
    {
        rtl::OUString sParent;
        if (pOldName)
        {
            if (GetAutoStylePool()->AddNamed(*pOldName, XML_STYLE_FAMILY_TABLE_ROW, sParent, xPropStates))
            {
                GetAutoStylePool()->RegisterName(XML_STYLE_FAMILY_TABLE_ROW, *pOldName);
                // add to pRowStyles, so the name is found for normal sheets
                rtl::OUString* pTemp(new rtl::OUString(*pOldName));
                rIndex = pRowStyles->AddStyleName(pTemp);
            }
        }
        else
        {
            rtl::OUString sName;
            if (GetAutoStylePool()->Add(sName, XML_STYLE_FAMILY_TABLE_ROW, sParent, xPropStates))
            {
                rtl::OUString* pTemp(new rtl::OUString(sName));
                rIndex = pRowStyles->AddStyleName(pTemp);
            }
            else
                rIndex = pRowStyles->GetIndexOfStyleName(sName, SC_SROWPREFIX);
        }
    }
}

uno::Any lcl_GetEnumerated( uno::Reference<container::XEnumerationAccess> xEnumAccess, sal_Int32 nIndex )
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

    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( GetModel(), uno::UNO_QUERY );
    if (!xSpreadDoc.is())
        return;

    uno::Reference<container::XIndexAccess> xIndex( xSpreadDoc->getSheets(), uno::UNO_QUERY );
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
                {
                    ScCellIterator aIter( pDoc, 0,0,nTab, MAXCOL,MAXROW,nTab );
                    ScBaseCell* pCell = aIter.GetFirst();
                    while (pCell)
                    {
                        if (pCell->GetCellType() == CELLTYPE_FORMULA)
                            static_cast<ScFormulaCell*>(pCell)->IsValue();      // interpret if dirty
                        pCell = aIter.GetNext();
                    }
                }

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
                    uno::Reference <sheet::XSpreadsheet> xTable(xIndex->getByIndex(nTable), uno::UNO_QUERY);
                    uno::Reference <beans::XPropertySet> xProperties(
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
                    uno::Reference<table::XColumnRowRange> xColumnRowRange(xIndex->getByIndex(nTable), uno::UNO_QUERY);
                    uno::Reference<table::XTableColumns> xTableColumns(xColumnRowRange->getColumns());
                    uno::Reference<beans::XPropertySet> xColumnProperties(xTableColumns->getByIndex( aPos.Col() ), uno::UNO_QUERY);

                    sal_Int32 nIndex(-1);
                    sal_Bool bIsVisible(sal_True);
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
                    uno::Reference<table::XColumnRowRange> xColumnRowRange(xIndex->getByIndex(nTable), uno::UNO_QUERY);
                    uno::Reference<table::XTableRows> xTableRows(xColumnRowRange->getRows());
                    uno::Reference<beans::XPropertySet> xRowProperties(xTableRows->getByIndex( aPos.Row() ), uno::UNO_QUERY);

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
                    uno::Reference<beans::XPropertySet> xTableProperties(xIndex->getByIndex(nTable), uno::UNO_QUERY);
                    if (xTableProperties.is())
                    {
                        std::vector<XMLPropertyState> xPropStates(xTableStylesExportPropertySetMapper->Filter(xTableProperties));
                        rtl::OUString sParent;
                        rtl::OUString sName( aTableIter->maName );
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
                sal_Int32 nTable = aPos.Tab();
                bool bCopySheet = pDoc->IsStreamValid( static_cast<SCTAB>(nTable) );
                if (bCopySheet)
                {
                    //! separate method AddStyleFromNote needed?

                    ScPostIt* pNote = pDoc->GetNote( aPos );
                    DBG_ASSERT( pNote, "note not found" );
                    if (pNote)
                    {
                        SdrCaptionObj* pDrawObj = pNote->GetOrCreateCaption( aPos );
                        // all uno shapes are created anyway in CollectSharedData
                        uno::Reference<beans::XPropertySet> xShapeProperties( pDrawObj->getUnoShape(), uno::UNO_QUERY );
                        if (xShapeProperties.is())
                        {
                            if ( aNoteIter->maStyleName.getLength() )
                            {
                                std::vector<XMLPropertyState> xPropStates(xShapeMapper->Filter(xShapeProperties));
                                rtl::OUString sParent;
                                rtl::OUString sName( aNoteIter->maStyleName );
                                GetAutoStylePool()->AddNamed(sName, XML_STYLE_FAMILY_SD_GRAPHICS_ID, sParent, xPropStates);
                                GetAutoStylePool()->RegisterName(XML_STYLE_FAMILY_SD_GRAPHICS_ID, sName);
                            }
                            if ( aNoteIter->maTextStyle.getLength() )
                            {
                                std::vector<XMLPropertyState> xPropStates(
                                    GetTextParagraphExport()->GetParagraphPropertyMapper()->Filter(xShapeProperties));
                                rtl::OUString sParent;
                                rtl::OUString sName( aNoteIter->maTextStyle );
                                GetAutoStylePool()->AddNamed(sName, XML_STYLE_FAMILY_TEXT_PARAGRAPH, sParent, xPropStates);
                                GetAutoStylePool()->RegisterName(XML_STYLE_FAMILY_TEXT_PARAGRAPH, sName);
                            }
                        }
                    }
                }
                ++aNoteIter;
            }

            // note paragraph styles

            //UniReference<SvXMLExportPropertyMapper> xParaPropMapper = XMLTextParagraphExport::CreateParaExtPropMapper( *this );
            UniReference<SvXMLExportPropertyMapper> xParaPropMapper = GetTextParagraphExport()->GetParagraphPropertyMapper();

            const std::vector<ScTextStyleEntry>& rNoteParaEntries = pSheetData->GetNoteParaStyles();
            std::vector<ScTextStyleEntry>::const_iterator aNoteParaIter = rNoteParaEntries.begin();
            std::vector<ScTextStyleEntry>::const_iterator aNoteParaEnd = rNoteParaEntries.end();
            while (aNoteParaIter != aNoteParaEnd)
            {
                ScAddress aPos = aNoteParaIter->maCellPos;
                sal_Int32 nTable = aPos.Tab();
                bool bCopySheet = pDoc->IsStreamValid( static_cast<SCTAB>(nTable) );
                if (bCopySheet)
                {
                    ScPostIt* pNote = pDoc->GetNote( aPos );
                    DBG_ASSERT( pNote, "note not found" );
                    if (pNote)
                    {
                        SdrCaptionObj* pDrawObj = pNote->GetOrCreateCaption( aPos );
                        uno::Reference<container::XEnumerationAccess> xCellText(pDrawObj->getUnoShape(), uno::UNO_QUERY);
                        uno::Reference<beans::XPropertySet> xParaProp(
                            lcl_GetEnumerated( xCellText, aNoteParaIter->maSelection.nStartPara ), uno::UNO_QUERY );
                        if ( xParaProp.is() )
                        {
                            std::vector<XMLPropertyState> xPropStates(xParaPropMapper->Filter(xParaProp));
                            rtl::OUString sParent;
                            rtl::OUString sName( aNoteParaIter->maName );
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
                sal_Int32 nTable = aPos.Tab();
                bool bCopySheet = pDoc->IsStreamValid( static_cast<SCTAB>(nTable) );
                if (bCopySheet)
                {
                    ScPostIt* pNote = pDoc->GetNote( aPos );
                    DBG_ASSERT( pNote, "note not found" );
                    if (pNote)
                    {
                        SdrCaptionObj* pDrawObj = pNote->GetOrCreateCaption( aPos );
                        uno::Reference<text::XSimpleText> xCellText(pDrawObj->getUnoShape(), uno::UNO_QUERY);
                        uno::Reference<beans::XPropertySet> xCursorProp(xCellText->createTextCursor(), uno::UNO_QUERY);
                        ScDrawTextCursor* pCursor = ScDrawTextCursor::getImplementation( xCursorProp );
                        if (pCursor)
                        {
                            pCursor->SetSelection( aNoteTextIter->maSelection );

                            std::vector<XMLPropertyState> xPropStates(xTextPropMapper->Filter(xCursorProp));
                            rtl::OUString sParent;
                            rtl::OUString sName( aNoteTextIter->maName );
                            GetAutoStylePool()->AddNamed(sName, XML_STYLE_FAMILY_TEXT_TEXT, sParent, xPropStates);
                            GetAutoStylePool()->RegisterName(XML_STYLE_FAMILY_TEXT_TEXT, sName);
                        }
                    }
                }
                ++aNoteTextIter;
            }

            // stored text styles

            //UniReference<SvXMLExportPropertyMapper> xTextPropMapper = XMLTextParagraphExport::CreateCharExtPropMapper( *this );

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

                    uno::Reference<table::XCellRange> xCellRange(xIndex->getByIndex(nTable), uno::UNO_QUERY);
                    uno::Reference<text::XSimpleText> xCellText(xCellRange->getCellByPosition(aPos.Col(), aPos.Row()), uno::UNO_QUERY);
                    uno::Reference<beans::XPropertySet> xCursorProp(xCellText->createTextCursor(), uno::UNO_QUERY);
                    ScCellTextCursor* pCursor = ScCellTextCursor::getImplementation( xCursorProp );
                    if (pCursor)
                    {
                        pCursor->SetSelection( aTextIter->maSelection );

                        std::vector<XMLPropertyState> xPropStates(xTextPropMapper->Filter(xCursorProp));
                        rtl::OUString sParent;
                        rtl::OUString sName( aTextIter->maName );
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
            sal_Int32 nCellCount(pDoc ? pDoc->GetCellCount() : 0);
            CollectSharedData(nTableCount, nShapesCount, nCellCount);
            //DBG_ERROR("no shared data setted");
        }
        sal_Int32 nTableCount(xIndex->getCount());
        pCellStyles->AddNewTable(nTableCount - 1);
        CollectShapesAutoStyles(nTableCount);
        for (sal_Int32 nTable = 0; nTable < nTableCount; ++nTable)
        {
            bool bUseStream = pSheetData && pDoc && pDoc->IsStreamValid((SCTAB)nTable) &&
                              pSheetData->HasStreamPos(nTable) && xSourceStream.is();

            uno::Reference <sheet::XSpreadsheet> xTable(xIndex->getByIndex(nTable), uno::UNO_QUERY);
            if (xTable.is())
            {
                // table styles array must be complete, including copied tables - Add should find the stored style
                uno::Reference<beans::XPropertySet> xTableProperties(xTable, uno::UNO_QUERY);
                if (xTableProperties.is())
                {
                    std::vector<XMLPropertyState> xPropStates(xTableStylesExportPropertySetMapper->Filter(xTableProperties));
                    if(xPropStates.size())
                    {
                        rtl::OUString sParent;
                        rtl::OUString sName;
                        GetAutoStylePool()->Add(sName, XML_STYLE_FAMILY_TABLE_TABLE, sParent, xPropStates);
                        aTableStyles.push_back(sName);
                    }
                }
            }
            // collect other auto-styles only for non-copied sheets
            if (xTable.is() && !bUseStream)
            {
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
                                    AddStyleFromCells(xProperties, xTable, nTable, NULL);
                                    IncrementProgressBar(sal_False);
                                }
                            }
                        }
                    }
                }
                uno::Reference<table::XColumnRowRange> xColumnRowRange (xTable, uno::UNO_QUERY);
                if (xColumnRowRange.is())
                {
                    if (pDoc)
                    {
                        pDoc->SyncColRowFlags();
                        uno::Reference<table::XTableColumns> xTableColumns(xColumnRowRange->getColumns());
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
//                              else if (nColumns < MAXCOL)
//                                  pColumnStyles->AddNewTable(nTable, ++nColumns);
                            else
                                pColumnStyles->AddNewTable(nTable, nColumns);
                            sal_Int32 nColumn = 0;
                            while (/*nColumn <= nColumns && */nColumn <= MAXCOL)
                            {
                                sal_Int32 nIndex(-1);
                                sal_Bool bIsVisible(sal_True);
                                uno::Reference <beans::XPropertySet> xColumnProperties(xTableColumns->getByIndex(nColumn), uno::UNO_QUERY);
                                if (xColumnProperties.is())
                                {
                                    AddStyleFromColumn( xColumnProperties, NULL, nIndex, bIsVisible );
                                    //if(xPropStates.size())
                                    pColumnStyles->AddFieldStyleName(nTable, nColumn, nIndex, bIsVisible);
                                }
                                sal_Int32 nOld(nColumn);
                                nColumn = pDoc->GetNextDifferentChangedCol(sal::static_int_cast<SCTAB>(nTable), static_cast<SCCOL>(nColumn));
                                for (sal_Int32 i = nOld + 1; i < nColumn; ++i)
                                    pColumnStyles->AddFieldStyleName(nTable, i, nIndex, bIsVisible);
                            }
                            if (aCellAddress.EndColumn > nColumns)
                            {
                                sal_Bool bIsVisible(sal_True);
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
                            table::CellRangeAddress aCellAddress(GetEndAddress(xTable, nTable));
                            if (aCellAddress.EndRow > nRows)
                            {
                                ++nRows;
                                pRowStyles->AddNewTable(nTable, aCellAddress.EndRow);
                            }
//                              else if (nRows < MAXROW)
//                                  pRowStyles->AddNewTable(nTable, ++nRows);
                            else
                                pRowStyles->AddNewTable(nTable, nRows);
                            sal_Int32 nRow = 0;
                            while (nRow <= nRows && nRow <= MAXROW)
                            {
                                sal_Int32 nIndex = 0;
                                uno::Reference <beans::XPropertySet> xRowProperties(xTableRows->getByIndex(nRow), uno::UNO_QUERY);
                                if(xRowProperties.is())
                                {
                                    AddStyleFromRow( xRowProperties, NULL, nIndex );
                                    //if(xPropStates.size())
                                    pRowStyles->AddFieldStyleName(nTable, nRow, nIndex);
                                }
                                sal_Int32 nOld(nRow);
                                nRow = pDoc->GetNextDifferentChangedRow(sal::static_int_cast<SCTAB>(nTable), static_cast<SCROW>(nRow), false);
                                if (nRow > nOld + 1)
                                    pRowStyles->AddFieldStyleName(nTable, nOld + 1, nIndex, nRow - 1);
                            }
                            if (aCellAddress.EndRow > nRows)
                            {
                                sal_Int32 nIndex(pRowStyles->GetStyleNameIndex(nTable, nRows));
                                pRowStyles->AddFieldStyleName(nTable, nRows + 1, nIndex, aCellAddress.EndRow);
                            }
                        }
                    }
                }
                uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery (xTable, uno::UNO_QUERY);
                if (xCellRangesQuery.is())
                {
                    uno::Reference<sheet::XSheetCellRanges> xSheetCellRanges(xCellRangesQuery->queryContentCells(sheet::CellFlags::FORMATTED));
                    uno::Reference<sheet::XSheetOperation> xSheetOperation(xSheetCellRanges, uno::UNO_QUERY);
                    if (xSheetCellRanges.is() && xSheetOperation.is())
                    {
                        sal_uInt32 nCount(sal_uInt32(xSheetOperation->computeFunction(sheet::GeneralFunction_COUNT)));
                        uno::Reference<container::XEnumerationAccess> xCellsAccess(xSheetCellRanges->getCells());
                        if (xCellsAccess.is())
                        {
                            GetProgressBarHelper()->ChangeReference(GetProgressBarHelper()->GetReference() + nCount);
                            uno::Reference<container::XEnumeration> xCells(xCellsAccess->createEnumeration());
                            if (xCells.is())
                            {
                                sal_uInt32 nCount2(0);
                                while (xCells->hasMoreElements())
                                {
                                    uno::Reference<text::XText> xText(xCells->nextElement(), uno::UNO_QUERY);
                                    if (xText.is())
                                        GetTextParagraphExport()->collectTextAutoStyles(xText, sal_False, sal_False);
                                    ++nCount2;
                                    IncrementProgressBar(sal_False);
                                }
                                if(nCount2 > nCount)
                                    GetProgressBarHelper()->SetReference(GetProgressBarHelper()->GetReference() + nCount2 - nCount);
                            }
                        }
                    }
                }
            }
            IncrementProgressBar(sal_False);
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
                SvXMLElementExport aElemStyle(*this, XML_NAMESPACE_STYLE, XML_STYLE, sal_True, sal_True);
                AddAttribute(XML_NAMESPACE_TABLE,  XML_DISPLAY, XML_FALSE);
                SvXMLElementExport aElemStyleTabProps(*this, XML_NAMESPACE_STYLE, XML_TABLE_PROPERTIES, sal_True, sal_True);
            }
        }
    }

    if (getExportFlags() & EXPORT_MASTERSTYLES)
    {
        GetPageExport()->collectAutoStyles(sal_True);
        GetPageExport()->exportAutoStyles();
    }

    // #i30251#; only write Text Styles once

    if ((getExportFlags() & EXPORT_CONTENT) || (getExportFlags() & EXPORT_MASTERSTYLES))
        GetTextParagraphExport()->exportTextAutoStyles();
}

void ScXMLExport::_ExportMasterStyles()
{
    GetPageExport()->exportMasterStyles( sal_True );
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
                pSharedData->AddNoteObj( xShape, pCaptData->maStart );

                // #i60851# When the file is saved while editing a new note,
                // the cell is still empty -> last column/row must be updated
                DBG_ASSERT( pCaptData->maStart.Tab() == nCurrentTable, "invalid table in object data" );
                pSharedData->SetLastColumn( nCurrentTable, pCaptData->maStart.Col() );
                pSharedData->SetLastRow( nCurrentTable, pCaptData->maStart.Row() );
            }
            // other objects from internal layer only (detective)
            else if( pObject->GetLayer() == SC_LAYER_INTERN )
            {
                ScDetectiveFunc aDetFunc( pDoc, static_cast<SCTAB>(nCurrentTable) );
                ScAddress       aPosition;
                ScRange         aSourceRange;
                sal_Bool        bRedLine;
                ScDetectiveObjType eObjType = aDetFunc.GetDetectiveObjectType(
                    pObject, nCurrentTable, aPosition, aSourceRange, bRedLine );
                pSharedData->GetDetectiveObjContainer()->AddObject( eObjType, static_cast<SCTAB>(nCurrentTable), aPosition, aSourceRange, bRedLine );
            }
        }
    }
}

//UNUSED2008-05  sal_Bool ScXMLExport::GetMerge (const uno::Reference <sheet::XSpreadsheet>& xTable,
//UNUSED2008-05                                  const sal_Int32 nCol, const sal_Int32 nRow,
//UNUSED2008-05                                  table::CellRangeAddress& aCellAddress)
//UNUSED2008-05  {
//UNUSED2008-05      uno::Reference<sheet::XSheetCellRange> xSheetCellRange(xTable->getCellRangeByPosition(nCol, nRow, nCol, nRow), uno::UNO_QUERY);
//UNUSED2008-05      if (xSheetCellRange.is())
//UNUSED2008-05      {
//UNUSED2008-05          uno::Reference<sheet::XSheetCellCursor> xCursor(xTable->createCursorByRange(xSheetCellRange));
//UNUSED2008-05          if (xCursor.is())
//UNUSED2008-05          {
//UNUSED2008-05              uno::Reference<sheet::XCellRangeAddressable> xCellAddress (xCursor, uno::UNO_QUERY);
//UNUSED2008-05              xCursor->collapseToMergedArea();
//UNUSED2008-05              aCellAddress = xCellAddress->getRangeAddress();
//UNUSED2008-05              return sal_True;
//UNUSED2008-05          }
//UNUSED2008-05      }
//UNUSED2008-05      return sal_False;
//UNUSED2008-05  }

sal_Bool ScXMLExport::GetMerged (const table::CellRangeAddress* pCellAddress,
                            const uno::Reference <sheet::XSpreadsheet>& xTable)
{
    sal_Bool bReady(sal_False);
    sal_Int32 nRow(pCellAddress->StartRow);
    sal_Int32 nCol(pCellAddress->StartColumn);
    sal_Int32 nEndRow(pCellAddress->EndRow);
    sal_Int32 nEndCol(pCellAddress->EndColumn);
    sal_Bool bRowInc(nEndRow > nRow);
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
                    bReady = sal_True;
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
    DBG_ASSERT(!(!bReady && nEndRow > nRow && nEndCol > nCol), "should not be possible");
    return !bReady;
}

//UNUSED2008-05  sal_Bool ScXMLExport::IsMatrix (const uno::Reference <table::XCellRange>& xCellRange,
//UNUSED2008-05                                  const uno::Reference <sheet::XSpreadsheet>& xTable,
//UNUSED2008-05                                  const sal_Int32 nCol, const sal_Int32 nRow,
//UNUSED2008-05                                  table::CellRangeAddress& aCellAddress, sal_Bool& bIsFirst) const
//UNUSED2008-05  {
//UNUSED2008-05      bIsFirst = sal_False;
//UNUSED2008-05      uno::Reference <sheet::XArrayFormulaRange> xArrayFormulaRange (xCellRange->getCellRangeByPosition(nCol,nRow,nCol,nRow), uno::UNO_QUERY);
//UNUSED2008-05      if (xArrayFormulaRange.is())
//UNUSED2008-05      {
//UNUSED2008-05          rtl::OUString sArrayFormula(xArrayFormulaRange->getArrayFormula());
//UNUSED2008-05          if (sArrayFormula.getLength())
//UNUSED2008-05          {
//UNUSED2008-05              uno::Reference<sheet::XSheetCellRange> xMatrixSheetCellRange (xArrayFormulaRange, uno::UNO_QUERY);
//UNUSED2008-05              if (xMatrixSheetCellRange.is())
//UNUSED2008-05              {
//UNUSED2008-05                  uno::Reference<sheet::XSheetCellCursor> xMatrixSheetCursor(xTable->createCursorByRange(xMatrixSheetCellRange));
//UNUSED2008-05                  if (xMatrixSheetCursor.is())
//UNUSED2008-05                  {
//UNUSED2008-05                      xMatrixSheetCursor->collapseToCurrentArray();
//UNUSED2008-05                      uno::Reference<sheet::XCellRangeAddressable> xMatrixCellAddress (xMatrixSheetCursor, uno::UNO_QUERY);
//UNUSED2008-05                      if (xMatrixCellAddress.is())
//UNUSED2008-05                      {
//UNUSED2008-05                          aCellAddress = xMatrixCellAddress->getRangeAddress();
//UNUSED2008-05                          if ((aCellAddress.StartColumn == nCol && aCellAddress.StartRow == nRow) &&
//UNUSED2008-05                              (aCellAddress.EndColumn > nCol || aCellAddress.EndRow > nRow))
//UNUSED2008-05                          {
//UNUSED2008-05                              bIsFirst = sal_True;
//UNUSED2008-05                              return sal_True;
//UNUSED2008-05                          }
//UNUSED2008-05                          else if (aCellAddress.StartColumn != nCol || aCellAddress.StartRow != nRow ||
//UNUSED2008-05                                   aCellAddress.EndColumn != nCol || aCellAddress.EndRow != nRow)
//UNUSED2008-05                              return sal_True;
//UNUSED2008-05                          else
//UNUSED2008-05                          {
//UNUSED2008-05                              bIsFirst = sal_True;
//UNUSED2008-05                              return sal_True;
//UNUSED2008-05                          }
//UNUSED2008-05                      }
//UNUSED2008-05                  }
//UNUSED2008-05              }
//UNUSED2008-05          }
//UNUSED2008-05      }
//UNUSED2008-05      return sal_False;
//UNUSED2008-05  }

sal_Bool ScXMLExport::IsMatrix (const ScAddress& aCell,
                            table::CellRangeAddress& aCellAddress, sal_Bool& bIsFirst) const
{
    bIsFirst = sal_False;

    ScRange aMatrixRange;

    if (pDoc && pDoc->GetMatrixFormulaRange(aCell, aMatrixRange))
    {
        ScUnoConversion::FillApiRange( aCellAddress, aMatrixRange );
        if ((aCellAddress.StartColumn == aCell.Col() && aCellAddress.StartRow == aCell.Row()) &&
            (aCellAddress.EndColumn > aCell.Col() || aCellAddress.EndRow > aCell.Row()))
        {
            bIsFirst = sal_True;
            return sal_True;
        }
        else if (aCellAddress.StartColumn != aCell.Col() || aCellAddress.StartRow != aCell.Row() ||
            aCellAddress.EndColumn != aCell.Col() || aCellAddress.EndRow != aCell.Row())
            return sal_True;
        else
        {
            bIsFirst = sal_True;
            return sal_True;
        }
    }

    return sal_False;

/*  uno::Reference <sheet::XArrayFormulaRange> xArrayFormulaRange (xCell, uno::UNO_QUERY);
    if (xArrayFormulaRange.is())
    {
        rtl::OUString sArrayFormula(xArrayFormulaRange->getArrayFormula());
        if (sArrayFormula.getLength())
        {
            uno::Reference<sheet::XSheetCellRange> xMatrixSheetCellRange (xCell, uno::UNO_QUERY);
            if (xMatrixSheetCellRange.is())
            {
                uno::Reference<sheet::XSheetCellCursor> xMatrixSheetCursor(xTable->createCursorByRange(xMatrixSheetCellRange));
                if (xMatrixSheetCursor.is())
                {
                    xMatrixSheetCursor->collapseToCurrentArray();
                    uno::Reference<sheet::XCellRangeAddressable> xMatrixCellAddress (xMatrixSheetCursor, uno::UNO_QUERY);
                    if (xMatrixCellAddress.is())
                    {
                        aCellAddress = xMatrixCellAddress->getRangeAddress();
                        if ((aCellAddress.StartColumn == nCol && aCellAddress.StartRow == nRow) &&
                            (aCellAddress.EndColumn > nCol || aCellAddress.EndRow > nRow))
                        {
                            bIsFirst = sal_True;
                            return sal_True;
                        }
                        else if (aCellAddress.StartColumn != nCol || aCellAddress.StartRow != nRow ||
                            aCellAddress.EndColumn != nCol || aCellAddress.EndRow != nRow)
                            return sal_True;
                        else
                        {
                            bIsFirst = sal_True;
                            return sal_True;
                        }
                    }
                }
            }
        }
    }
    return sal_False;*/
}

sal_Bool ScXMLExport::GetCellText (ScMyCell& rMyCell, const ScAddress& aPos) const
{
    if (rMyCell.bHasStringValue)
        return sal_True;
    else
    {
/*      if (!rMyCell.bHasXText)
        {
            rMyCell.xText.set(xCurrentTableCellRange->getCellByPosition(rMyCell.aCellAddress.Column, rMyCell.aCellAddress.Row), uno::UNO_QUERY);
            rMyCell.bHasXText = sal_True;
        }*/
//      if (rMyCell.xText.is())
//      {
            rMyCell.sStringValue = ScCellObj::GetOutputString_Impl(pDoc, aPos);
            rMyCell.bHasStringValue = sal_True;
            return sal_True;
//      }
    }
}

void ScXMLExport::WriteCell (ScMyCell& aCell)
{
    ScAddress aCellPos;
    ScUnoConversion::FillScAddress( aCellPos, aCell.aCellAddress );
    if (aCell.nStyleIndex != -1)
        AddAttribute(sAttrStyleName, *pCellStyles->GetStyleNameByIndex(aCell.nStyleIndex, aCell.bIsAutoStyle));
    if (aCell.nValidationIndex > -1)
        AddAttribute(XML_NAMESPACE_TABLE, XML_CONTENT_VALIDATION_NAME, pValidationsContainer->GetValidationName(aCell.nValidationIndex));
    sal_Bool bIsMatrix(aCell.bIsMatrixBase || aCell.bIsMatrixCovered);
    sal_Bool bIsFirstMatrixCell(aCell.bIsMatrixBase);
    if (bIsFirstMatrixCell)
    {
        sal_Int32 nColumns(aCell.aMatrixRange.EndColumn - aCell.aMatrixRange.StartColumn + 1);
        sal_Int32 nRows(aCell.aMatrixRange.EndRow - aCell.aMatrixRange.StartRow + 1);
        rtl::OUStringBuffer sColumns;
        rtl::OUStringBuffer sRows;
        SvXMLUnitConverter::convertNumber(sColumns, nColumns);
        SvXMLUnitConverter::convertNumber(sRows, nRows);
        AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_MATRIX_COLUMNS_SPANNED, sColumns.makeStringAndClear());
        AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_MATRIX_ROWS_SPANNED, sRows.makeStringAndClear());
    }
    sal_Bool bIsEmpty(sal_False);
    switch (aCell.nType)
    {
    case table::CellContentType_EMPTY :
        {
            bIsEmpty = sal_True;
        }
        break;
    case table::CellContentType_VALUE :
        {
            if (!aCell.bHasDoubleValue)
            {
                aCell.fValue = pDoc->GetValue( aCellPos );
                aCell.bHasDoubleValue = sal_True;
            }
            GetNumberFormatAttributesExportHelper()->SetNumberFormatAttributes(
                aCell.nNumberFormat, aCell.fValue);
        }
        break;
    case table::CellContentType_TEXT :
        {
            if (GetCellText(aCell, aCellPos))
            {
                rtl::OUString sFormula(lcl_GetRawString(pDoc, aCellPos));
                GetNumberFormatAttributesExportHelper()->SetNumberFormatAttributes(
                    sFormula, aCell.sStringValue, sal_True, sal_True);
            }
        }
        break;
    case table::CellContentType_FORMULA :
        {
            ScBaseCell* pBaseCell = pDoc ? pDoc->GetCell(aCellPos) : NULL;
            if (pBaseCell && pBaseCell->GetCellType() == CELLTYPE_FORMULA)
            {
                rtl::OUStringBuffer sFormula;
                ScFormulaCell* pFormulaCell((ScFormulaCell*) pBaseCell);
                if (!bIsMatrix || (bIsMatrix && bIsFirstMatrixCell))
                {
                    const formula::FormulaGrammar::Grammar eGrammar = pDoc->GetStorageGrammar();
                    sal_uInt16 nNamespacePrefix = (eGrammar == formula::FormulaGrammar::GRAM_ODFF ? XML_NAMESPACE_OF : XML_NAMESPACE_OOOC);
                    pFormulaCell->GetFormula(sFormula, eGrammar);
                    rtl::OUString sOUFormula(sFormula.makeStringAndClear());
                    if (!bIsMatrix)
                    {
                        AddAttribute(sAttrFormula, GetNamespaceMap().GetQNameByKey( nNamespacePrefix, sOUFormula, sal_False ));
                    }
                    else
                    {
                        AddAttribute(sAttrFormula, GetNamespaceMap().GetQNameByKey( nNamespacePrefix, sOUFormula.copy(1, sOUFormula.getLength() - 2), sal_False ));
                    }
                }
                if (pFormulaCell->IsValue())
                {
                    sal_Bool bIsStandard;
                    rtl::OUString sCurrency;
                    GetNumberFormatAttributesExportHelper()->GetCellType(aCell.nNumberFormat, sCurrency, bIsStandard);
                    if (bIsStandard)
                    {
                        if (pDoc)
                            GetNumberFormatAttributesExportHelper()->SetNumberFormatAttributes(
                                pFormulaCell->GetStandardFormat(*pDoc->GetFormatTable(), 0),
                                pDoc->GetValue( aCellPos ));
                    }
                    else
                        GetNumberFormatAttributesExportHelper()->SetNumberFormatAttributes(
                            aCell.nNumberFormat, pDoc->GetValue( aCellPos ));
                }
                else
                {
                    if (GetCellText(aCell, aCellPos))
                        if (aCell.sStringValue.getLength())
                        {
                            AddAttribute(sAttrValueType, XML_STRING);
                            AddAttribute(sAttrStringValue, aCell.sStringValue);
                        }
                }
            }
        }
        break;
        default:
        {
            // added to avoid warnings
        }
    }
    rtl::OUString* pCellString(&sElemCell);
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
            rtl::OUStringBuffer sColumns;
            rtl::OUStringBuffer sRows;
            SvXMLUnitConverter::convertNumber(sColumns, nColumns);
            SvXMLUnitConverter::convertNumber(sRows, nRows);
            AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_SPANNED, sColumns.makeStringAndClear());
            AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_ROWS_SPANNED, sRows.makeStringAndClear());
        }
    }
    SvXMLElementExport aElemC(*this, *pCellString, sal_True, sal_True);
    CheckAttrList();
    WriteAreaLink(aCell);
    WriteAnnotation(aCell);
    WriteDetective(aCell);

    sal_Bool bEditCell = sal_False;

    if (!bIsEmpty)
    {
        if ((aCell.nType == table::CellContentType_TEXT && IsEditCell(aCell)) ||
            (aCell.nType == table::CellContentType_FORMULA && IsMultiLineFormulaCell(aCell)))
        {
            bEditCell = sal_True;
            uno::Reference<text::XText> xText(xCurrentTableCellRange->getCellByPosition(aCell.aCellAddress.Column, aCell.aCellAddress.Row), uno::UNO_QUERY);
            if ( xText.is())
                GetTextParagraphExport()->exportText(xText, sal_False, sal_False);
        }
        else
        {
            SvXMLElementExport aElemP(*this, sElemP, sal_True, sal_False);
            sal_Bool bPrevCharWasSpace(sal_True);
              if (GetCellText(aCell, aCellPos))
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
//BM    sal_Bool bMemChart(sal_False);  // das muss man jetzt umbenennen :-)
    bool bIsChart( false );
    rtl::OUString sPropCLSID (RTL_CONSTASCII_USTRINGPARAM("CLSID"));
    rtl::OUString sPropModel (RTL_CONSTASCII_USTRINGPARAM("Model"));
    rtl::OUString sPersistName (RTL_CONSTASCII_USTRINGPARAM("PersistName"));
    if (xShapeProps.is())
    {
        sal_Int32 nZOrder = 0;
        if (xShapeProps->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ZOrder"))) >>= nZOrder)
        {
            rtl::OUStringBuffer sBuffer;
            GetMM100UnitConverter().convertNumber(sBuffer, nZOrder);
            AddAttribute(XML_NAMESPACE_DRAW, XML_ZINDEX, sBuffer.makeStringAndClear());
        }
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo = xShapeProps->getPropertySetInfo();
        if( xPropSetInfo->hasPropertyByName( sPropCLSID ) )
        {
            rtl::OUString sCLSID;
            if (xShapeProps->getPropertyValue( sPropCLSID ) >>= sCLSID)
            {
                if ( sCLSID.equalsIgnoreAsciiCase(GetChartExport()->getChartCLSID()) )
                {
                    // we have a chart
                    ::rtl::OUString sRanges;
                    if ( pDoc )
                    {
                        ::rtl::OUString aChartName;
                        xShapeProps->getPropertyValue( sPersistName ) >>= aChartName;
                        ScRange aEmptyRange;
                        ScChartListener aSearcher( aChartName, pDoc, aEmptyRange );
                        sal_uInt16 nIndex = 0;
                        ScChartListenerCollection* pCollection = pDoc->GetChartListenerCollection();
                        if ( pCollection && pCollection->Search( &aSearcher, nIndex ) )
                        {
                            ScChartListener* pListener = static_cast< ScChartListener* >( pCollection->At( nIndex ) );
                            if ( pListener )
                            {
                                const ScRangeListRef& rRangeList = pListener->GetRangeList();
                                if ( rRangeList.Is() )
                                {
                                    ScRangeStringConverter::GetStringFromRangeList( sRanges, rRangeList, pDoc, FormulaGrammar::CONV_OOO );
                                    if ( sRanges.getLength() > 0 )
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

                    if ( sRanges.getLength() == 0 )
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
                                uno::Sequence< ::rtl::OUString > aRepresentations(
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

//BM                    rtl::OUString sOUName;
//BM                    xShapeProps->getPropertyValue(sPersistName) >>= sOUName;
//BM                    String sName(sOUName);
//BM                    if (!pChartListener)
//BM                    {
//BM                        String aEmptyString;
//BM                        ScRange aRange;
//BM                        pChartListener = new ScChartListener ( aEmptyString, GetDocument(), aRange );
//BM                    }
//BM                    if(pChartListener)
//BM                    {
//BM                        sal_uInt16 nIndex(0);
//BM                        pChartListener->SetString( sName );
//BM                        if ( GetDocument() && GetDocument()->GetChartListenerCollection()->Search( pChartListener, nIndex ) )
//BM                        {
//BM                            const ScRangeListRef& rRangeListRef(((ScChartListener*)
//BM                                (GetDocument()->GetChartListenerCollection()->
//BM                                At( nIndex )))->GetRangeList());
//BM                            if (rRangeListRef.Is())
//BM                            {
//BM                                bMemChart = sal_True;
//BM                                rtl::OUString sRanges;
//BM                                ScRangeStringConverter::GetStringFromRangeList(sRanges, rRangeListRef, GetDocument());
//BM                                 SvXMLAttributeList* pAttrList = NULL;
//BM                                if (sRanges.getLength())
//BM                                 {
//BM                                     pAttrList = new SvXMLAttributeList();
//BM                                     pAttrList->AddAttribute(
//BM                                         GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_DRAW, GetXMLToken(XML_NOTIFY_ON_UPDATE_OF_RANGES) ), sRanges );
//BM                                 }
//BM                                GetShapeExport()->exportShape(xShape, SEF_EXPORT_NO_CHART_DATA | SEF_DEFAULT, pPoint, pAttrList);
//BM                            }
//BM                        }
//BM                        else
//BM                        {
//BM                            bMemChart = sal_True;
//BM                             SvXMLAttributeList* pAttrList = new SvXMLAttributeList();
//BM                             pAttrList->AddAttribute(
//BM                                 GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_DRAW, GetXMLToken(XML_NOTIFY_ON_UPDATE_OF_RANGES) ), rtl::OUString() );
//BM                            GetShapeExport()->exportShape(xShape, SEF_EXPORT_NO_CHART_DATA | SEF_DEFAULT, pPoint, pAttrList);
//BM                        }
//BM                    }

/*                  SchMemChart* pMemChart = pDoc->FindChartData(sName);
                    if (pMemChart && pMemChart->GetSeriesAddresses().getLength())
                    {
                        bMemChart = sal_True;
                        rtl::OUString sRanges(pMemChart->getXMLStringForChartRange());
                        if (sRanges.getLength())
                            AddAttribute(XML_NAMESPACE_DRAW, XML_NOTIFY_ON_UPDATE_OF_RANGES, sRanges);
                        GetShapeExport()->exportShape(xShape, SEF_EXPORT_NO_CHART_DATA | SEF_DEFAULT, pPoint);
                    }*/
                }
            }
        }
    }
    if (!bIsChart)
        GetShapeExport()->exportShape(xShape, SEF_DEFAULT, pPoint);
    IncrementProgressBar(sal_False);
}

void ScXMLExport::WriteShapes(const ScMyCell& rMyCell)
{
    if( rMyCell.bHasShape && !rMyCell.aShapeList.empty() && pDoc )
    {
        awt::Point aPoint;
        Rectangle aRec = pDoc->GetMMRect(static_cast<SCCOL>(rMyCell.aCellAddress.Column), static_cast<SCROW>(rMyCell.aCellAddress.Row),
            static_cast<SCCOL>(rMyCell.aCellAddress.Column), static_cast<SCROW>(rMyCell.aCellAddress.Row), static_cast<SCTAB>(rMyCell.aCellAddress.Sheet));
        sal_Bool bNegativePage(pDoc->IsNegativePage(rMyCell.aCellAddress.Sheet));
        if (bNegativePage)
            aPoint.X = aRec.Right();
        else
            aPoint.X = aRec.Left();
        aPoint.Y = aRec.Top();
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
                    awt::Point aEndPoint;
                    Rectangle aEndRec(pDoc->GetMMRect(aItr->aEndAddress.Col(), aItr->aEndAddress.Row(),
                        aItr->aEndAddress.Col(), aItr->aEndAddress.Row(), aItr->aEndAddress.Tab()));
                    rtl::OUString sEndAddress;
                    ScRangeStringConverter::GetStringFromAddress(sEndAddress, aItr->aEndAddress, pDoc, FormulaGrammar::CONV_OOO);
                    AddAttribute(XML_NAMESPACE_TABLE, XML_END_CELL_ADDRESS, sEndAddress);
                    if (bNegativePage)
                        aEndPoint.X = -aEndRec.Right();
                    else
                        aEndPoint.X = aEndRec.Left();
                    aEndPoint.Y = aEndRec.Top();
                    awt::Point aStartPoint(aItr->xShape->getPosition());
                    awt::Size aSize(aItr->xShape->getSize());
                    sal_Int32 nEndX;
                    if (bNegativePage)
                        nEndX = -aStartPoint.X - aEndPoint.X;
                    else
                        nEndX = aStartPoint.X + aSize.Width - aEndPoint.X;
                    sal_Int32 nEndY(aStartPoint.Y + aSize.Height - aEndPoint.Y);
                    rtl::OUStringBuffer sBuffer;
                    GetMM100UnitConverter().convertMeasure(sBuffer, nEndX);
                    AddAttribute(XML_NAMESPACE_TABLE, XML_END_X, sBuffer.makeStringAndClear());
                    GetMM100UnitConverter().convertMeasure(sBuffer, nEndY);
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
        DBG_ASSERT(pTableShapes->size() > static_cast<size_t>(nCurrentTable), "wrong Table");
        SvXMLElementExport aShapesElem(*this, XML_NAMESPACE_TABLE, XML_SHAPES, sal_True, sal_False);
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
        if( rAreaLink.sFilterOptions.getLength() )
            AddAttribute( XML_NAMESPACE_TABLE, XML_FILTER_OPTIONS, rAreaLink.sFilterOptions );
        OUStringBuffer sValue;
        SvXMLUnitConverter::convertNumber( sValue, rAreaLink.GetColCount() );
        AddAttribute( XML_NAMESPACE_TABLE, XML_LAST_COLUMN_SPANNED, sValue.makeStringAndClear() );
        SvXMLUnitConverter::convertNumber( sValue, rAreaLink.GetRowCount() );
        AddAttribute( XML_NAMESPACE_TABLE, XML_LAST_ROW_SPANNED, sValue.makeStringAndClear() );
        if( rAreaLink.nRefresh )
        {
            SvXMLUnitConverter::convertTime( sValue, (double)rAreaLink.nRefresh / 86400 );
            AddAttribute( XML_NAMESPACE_TABLE, XML_REFRESH_DELAY, sValue.makeStringAndClear() );
        }
        SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE, XML_CELL_RANGE_SOURCE, sal_True, sal_True );
    }
}

void ScXMLExport::exportAnnotationMeta( const uno::Reference < drawing::XShape >& xShape)
{
    if (pCurrentCell && pCurrentCell->xNoteShape.is() && pCurrentCell->xNoteShape.get() == xShape.get() && pCurrentCell->xAnnotation.is())
    {
        rtl::OUString sAuthor(pCurrentCell->xAnnotation->getAuthor());
        if (sAuthor.getLength())
        {
            SvXMLElementExport aCreatorElem( *this, XML_NAMESPACE_DC,
                                                XML_CREATOR, sal_True,
                                                sal_False );
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
                rtl::OUStringBuffer sBuf;
                GetMM100UnitConverter().convertDateTime(sBuf, fDate,sal_True);
                SvXMLElementExport aDateElem( *this, XML_NAMESPACE_DC,
                                                XML_DATE, sal_True,
                                                sal_False );
                Characters(sBuf.makeStringAndClear());
            }
            else
            {
                SvXMLElementExport aDateElem( *this, XML_NAMESPACE_META,
                                                XML_DATE_STRING, sal_True,
                                                sal_False );
                Characters(rtl::OUString(aDate));
            }
        }
        else
        {
            SvXMLElementExport aDateElem( *this, XML_NAMESPACE_META,
                                            XML_DATE_STRING, sal_True,
                                            sal_False );
            Characters(rtl::OUString(aDate));
        }
    }
}

void ScXMLExport::WriteAnnotation(ScMyCell& rMyCell)
{
    if( rMyCell.bHasAnnotation && rMyCell.xAnnotation.is())
    {
/*      rtl::OUString sAuthor(rMyCell.xAnnotation->getAuthor());
        if (sAuthor.getLength())
        {
            SvXMLElementExport aCreatorElem( *this, XML_NAMESPACE_DC,
                                                XML_CREATOR, sal_True,
                                                sal_False );
            rtl::OUString sAuthor(sAuthor);
            Characters(sAuthor);
        }

        String aDate(rMyCell.xAnnotation->getDate());
        if (pDoc)
        {
            SvNumberFormatter* pNumForm(pDoc->GetFormatTable());
            double fDate;
            sal_uInt32 nfIndex = pNumForm->GetFormatIndex(NF_DATE_SYS_DDMMYYYY, LANGUAGE_SYSTEM);
            if (pNumForm->IsNumberFormat(aDate, nfIndex, fDate))
            {
                rtl::OUStringBuffer sBuf;
                GetMM100UnitConverter().convertDateTime(sBuf, fDate);
                SvXMLElementExport aDateElem( *this, XML_NAMESPACE_DC,
                                                XML_DATE, sal_True,
                                                sal_False );
                Characters(sBuf.makeStringAndClear());
            }
            else
            {
                SvXMLElementExport aDateElem( *this, XML_NAMESPACE_META,
                                                XML_DATE_STRING, sal_True,
                                                sal_False );
                Characters(rtl::OUString(aDate));
            }
        }
        else
        {
            SvXMLElementExport aDateElem( *this, XML_NAMESPACE_META,
                                            XML_DATE_STRING, sal_True,
                                            sal_False );
            Characters(rtl::OUString(aDate));
        }*/

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
            SvXMLElementExport aDetElem( *this, XML_NAMESPACE_TABLE, XML_DETECTIVE, sal_True, sal_True );
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
                SvXMLElementExport aRangeElem( *this, XML_NAMESPACE_TABLE, XML_HIGHLIGHTED_RANGE, sal_True, sal_True );
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
                SvXMLUnitConverter::convertNumber( aBuffer, aOpItr->nIndex );
                AddAttribute( XML_NAMESPACE_TABLE, XML_INDEX, aBuffer.makeStringAndClear() );
                SvXMLElementExport aRangeElem( *this, XML_NAMESPACE_TABLE, XML_OPERATION, sal_True, sal_True );
                ++aOpItr;
            }
        }
    }
}

void ScXMLExport::SetRepeatAttribute (const sal_Int32 nEqualCellCount)
{
    if (nEqualCellCount > 0)
    {
        sal_Int32 nTemp(nEqualCellCount + 1);
        OUString sOUEqualCellCount(OUString::valueOf(nTemp));
        AddAttribute(sAttrColumnsRepeated, sOUEqualCellCount);
        IncrementProgressBar(sal_False, nEqualCellCount);
    }
}

sal_Bool ScXMLExport::IsCellTypeEqual (const ScMyCell& aCell1, const ScMyCell& aCell2) const
{
    return (aCell1.nType == aCell2.nType);
}

sal_Bool ScXMLExport::IsEditCell(const com::sun::star::table::CellAddress& aAddress, ScMyCell* pMyCell) const
{
    ScAddress aCoreAddress(static_cast<SCCOL>(aAddress.Column),
                        static_cast<SCROW>(aAddress.Row),
                        static_cast<SCTAB>(aAddress.Sheet));
    ScBaseCell* pBaseCell = GetDocument() ? GetDocument()->GetCell(aCoreAddress) : NULL;
    if (pMyCell)
        pMyCell->pBaseCell = pBaseCell;

    if (pBaseCell)
        return (pBaseCell->GetCellType() == CELLTYPE_EDIT);
    return sal_False;
}

//UNUSED2008-05  sal_Bool ScXMLExport::IsEditCell(const com::sun::star::uno::Reference <com::sun::star::table::XCell>& xCell) const
//UNUSED2008-05  {
//UNUSED2008-05      uno::Reference<sheet::XCellAddressable> xAddressable (xCell, uno::UNO_QUERY);
//UNUSED2008-05      if ( xAddressable.is() )
//UNUSED2008-05          return IsEditCell(xAddressable->getCellAddress());
//UNUSED2008-05      return sal_False;
//UNUSED2008-05  }

sal_Bool ScXMLExport::IsEditCell(ScMyCell& rCell) const
{
    if (rCell.bKnowWhetherIsEditCell)
        return rCell.bIsEditCell;
    else
    {
         rCell.bIsEditCell = IsEditCell(rCell.aCellAddress, &rCell);
        rCell.bKnowWhetherIsEditCell = sal_True;
        return rCell.bIsEditCell;
    }
}

sal_Bool ScXMLExport::IsMultiLineFormulaCell(ScMyCell& rCell) const
{
    if (rCell.pBaseCell)
    {
        if (rCell.pBaseCell->GetCellType() != CELLTYPE_FORMULA)
            return false;

        return static_cast<ScFormulaCell*>(rCell.pBaseCell)->IsMultilineResult();
    }

    ScAddress aAddr(static_cast<SCCOL>(rCell.aCellAddress.Column),
                    static_cast<SCROW>(rCell.aCellAddress.Row),
                    static_cast<SCTAB>(rCell.aCellAddress.Sheet));
    ScBaseCell* pBaseCell = pDoc ? pDoc->GetCell(aAddr) : NULL;
    if (!pBaseCell)
        return false;

    rCell.pBaseCell = pBaseCell;
    if (rCell.pBaseCell->GetCellType() != CELLTYPE_FORMULA)
        return false;

    return static_cast<ScFormulaCell*>(rCell.pBaseCell)->IsMultilineResult();
}

//UNUSED2008-05  sal_Bool ScXMLExport::IsAnnotationEqual(const uno::Reference<table::XCell>& /* xCell1 */,
//UNUSED2008-05                                          const uno::Reference<table::XCell>& /* xCell2 */)
//UNUSED2008-05  {
//UNUSED2008-05      // no longer compareable, because the position and size and other attributes can also differ
//UNUSED2008-05
//UNUSED2008-05  /* uno::Reference<sheet::XSheetAnnotationAnchor> xSheetAnnotationAnchor1(xCell1, uno::UNO_QUERY);
//UNUSED2008-05      uno::Reference<sheet::XSheetAnnotationAnchor> xSheetAnnotationAnchor2(xCell2, uno::UNO_QUERY);
//UNUSED2008-05      if (xSheetAnnotationAnchor1.is() && xSheetAnnotationAnchor2.is())
//UNUSED2008-05      {
//UNUSED2008-05          uno::Reference <sheet::XSheetAnnotation> xSheetAnnotation1(xSheetAnnotationAnchor1->getAnnotation());
//UNUSED2008-05          uno::Reference <sheet::XSheetAnnotation> xSheetAnnotation2(xSheetAnnotationAnchor2->getAnnotation());
//UNUSED2008-05          uno::Reference<text::XSimpleText> xSimpleText1(xSheetAnnotation1, uno::UNO_QUERY);
//UNUSED2008-05          uno::Reference<text::XSimpleText> xSimpleText2(xSheetAnnotation2, uno::UNO_QUERY);
//UNUSED2008-05          if (xSheetAnnotation1.is() && xSimpleText1.is() &&
//UNUSED2008-05              xSheetAnnotation2.is() && xSimpleText2.is())
//UNUSED2008-05          {
//UNUSED2008-05              rtl::OUString sText1(xSimpleText1->getString());
//UNUSED2008-05              rtl::OUString sText2(xSimpleText2->getString());
//UNUSED2008-05              sal_Int32 nLength1(sText1.getLength());
//UNUSED2008-05              sal_Int32 nLength2(sText2.getLength());
//UNUSED2008-05              if (nLength1 && nLength2)
//UNUSED2008-05                  if (sText1 == sText2 &&
//UNUSED2008-05                      xSheetAnnotation1->getAuthor() == xSheetAnnotation2->getAuthor() &&
//UNUSED2008-05                      xSheetAnnotation1->getDate() == xSheetAnnotation2->getDate() &&
//UNUSED2008-05                      xSheetAnnotation1->getIsVisible() == xSheetAnnotation2->getIsVisible())
//UNUSED2008-05                      return sal_True;
//UNUSED2008-05                  else
//UNUSED2008-05                      return sal_False;
//UNUSED2008-05              else
//UNUSED2008-05                  if (nLength1 || nLength2)
//UNUSED2008-05                      return sal_False;
//UNUSED2008-05                  else
//UNUSED2008-05                      return sal_True;
//UNUSED2008-05          }
//UNUSED2008-05      }*/
//UNUSED2008-05      return sal_False;
//UNUSED2008-05  }

sal_Bool ScXMLExport::IsCellEqual (ScMyCell& aCell1, ScMyCell& aCell2)
{
    ScAddress aCellPos1;
    ScUnoConversion::FillScAddress( aCellPos1, aCell1.aCellAddress );
    ScAddress aCellPos2;
    ScUnoConversion::FillScAddress( aCellPos2, aCell2.aCellAddress );
    sal_Bool bIsEqual = sal_False;
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
            if (!aCell1.bHasAnnotation || (aCell1.bHasAnnotation && sal_False/*IsAnnotationEqual(aCell1.xCell, aCell2.xCell)*/)) // no longer compareable
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
                            bIsEqual = sal_True;
                        }
                        break;
                    case table::CellContentType_VALUE :
                        {
                            if(!aCell1.bHasDoubleValue)
                            {
                                aCell1.fValue = pDoc->GetValue( aCellPos1 );
                                aCell1.bHasDoubleValue = sal_True;
                            }
                            if (!aCell2.bHasDoubleValue)
                            {
                                aCell2.fValue = pDoc->GetValue( aCellPos2 );
                                aCell2.bHasDoubleValue = sal_True;
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
                                bIsEqual = sal_False;
                            else
                            {
                                if (GetCellText(aCell1, aCellPos1) && GetCellText(aCell2, aCellPos2))
                                {
                                    bIsEqual = (aCell1.sStringValue == aCell2.sStringValue) &&
                                               (lcl_GetRawString(pDoc, aCellPos1) == lcl_GetRawString(pDoc, aCellPos2));
                                }
                                else
                                    bIsEqual = sal_False;
                            }
                        }
                        break;
                    case table::CellContentType_FORMULA :
                        {
                            bIsEqual = sal_False;
                        }
                        break;
                    default :
                        {
                            bIsEqual = sal_False;
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
        sal_Bool bCalcAsShown (::cppu::any2bool( xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_CALCASSHOWN))) ));
        sal_Bool bIgnoreCase (::cppu::any2bool( xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_IGNORECASE))) ));
        sal_Bool bLookUpLabels (::cppu::any2bool( xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_LOOKUPLABELS))) ));
        sal_Bool bMatchWholeCell (::cppu::any2bool( xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_MATCHWHOLE))) ));
        sal_Bool bUseRegularExpressions (::cppu::any2bool( xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_REGEXENABLED))) ));
        sal_Bool bIsIterationEnabled (::cppu::any2bool( xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ITERENABLED))) ));
        sal_uInt16 nYear2000 (pDoc ? pDoc->GetDocOptions().GetYear2000() : 0);
        sal_Int32 nIterationCount(100);
        xPropertySet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ITERCOUNT))) >>= nIterationCount;
        double fIterationEpsilon = 0;
        xPropertySet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ITEREPSILON))) >>= fIterationEpsilon;
        util::Date aNullDate;
        xPropertySet->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_NULLDATE))) >>= aNullDate;
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
                rtl::OUStringBuffer sBuffer;
                GetMM100UnitConverter().convertNumber(sBuffer, nYear2000);
                AddAttribute(XML_NAMESPACE_TABLE, XML_NULL_YEAR, sBuffer.makeStringAndClear());
            }
            SvXMLElementExport aCalcSettings(*this, XML_NAMESPACE_TABLE, XML_CALCULATION_SETTINGS, sal_True, sal_True);
            {
                if (aNullDate.Day != 30 || aNullDate.Month != 12 || aNullDate.Year != 1899)
                {
                    rtl::OUStringBuffer sDate;
                    GetMM100UnitConverter().convertDateTime(sDate, 0.0, aNullDate);
                    AddAttribute(XML_NAMESPACE_TABLE, XML_DATE_VALUE, sDate.makeStringAndClear());
                    SvXMLElementExport aElemNullDate(*this, XML_NAMESPACE_TABLE, XML_NULL_DATE, sal_True, sal_True);
                }
                if (bIsIterationEnabled || nIterationCount != 100 || !::rtl::math::approxEqual(fIterationEpsilon, 0.001))
                {
                    rtl::OUStringBuffer sBuffer;
                    if (bIsIterationEnabled)
                        AddAttribute(XML_NAMESPACE_TABLE, XML_STATUS, XML_ENABLE);
                    if (nIterationCount != 100)
                    {
                        GetMM100UnitConverter().convertNumber(sBuffer, nIterationCount);
                        AddAttribute(XML_NAMESPACE_TABLE, XML_STEPS, sBuffer.makeStringAndClear());
                    }
                    if (!::rtl::math::approxEqual(fIterationEpsilon, 0.001))
                    {
                        GetMM100UnitConverter().convertDouble(sBuffer, fIterationEpsilon);
                        AddAttribute(XML_NAMESPACE_TABLE, XML_MAXIMUM_DIFFERENCE, sBuffer.makeStringAndClear());
                    }
                    SvXMLElementExport aElemIteration(*this, XML_NAMESPACE_TABLE, XML_ITERATION, sal_True, sal_True);
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
            rtl::OUString sLink (xLinkable->getLinkUrl());
            uno::Reference <beans::XPropertySet> xProps (GetModel(), uno::UNO_QUERY);
            if (xProps.is())
            {
                uno::Reference <container::XIndexAccess> xIndex(xProps->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_SHEETLINKS))), uno::UNO_QUERY);
                if (xIndex.is())
                {
                    sal_Int32 nCount(xIndex->getCount());
                    if (nCount)
                    {
                        sal_Bool bFound(sal_False);
                        uno::Reference <beans::XPropertySet> xLinkProps;
                        for (sal_Int32 i = 0; (i < nCount) && !bFound; ++i)
                        {
                            xLinkProps.set(xIndex->getByIndex(i), uno::UNO_QUERY);
                            if (xLinkProps.is())
                            {
                                rtl::OUString sNewLink;
                                if (xLinkProps->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_LINKURL))) >>= sNewLink)
                                    bFound = sLink.equals(sNewLink);
                            }
                        }
                        if (bFound && xLinkProps.is())
                        {
                            rtl::OUString sFilter;
                            rtl::OUString sFilterOptions;
                            rtl::OUString sTableName (xLinkable->getLinkSheetName());
                            sal_Int32 nRefresh(0);
                            xLinkProps->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_FILTER))) >>= sFilter;
                            xLinkProps->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_FILTOPT))) >>= sFilterOptions;
                            xLinkProps->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_REFDELAY))) >>= nRefresh;
                            if (sLink.getLength())
                            {
                                AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE);
                                AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, GetRelativeReference(sLink));
                                if (sTableName.getLength())
                                    AddAttribute(XML_NAMESPACE_TABLE, XML_TABLE_NAME, sTableName);
                                if (sFilter.getLength())
                                    AddAttribute(XML_NAMESPACE_TABLE, XML_FILTER_NAME, sFilter);
                                if (sFilterOptions.getLength())
                                    AddAttribute(XML_NAMESPACE_TABLE, XML_FILTER_OPTIONS, sFilterOptions);
                                if (nMode != sheet::SheetLinkMode_NORMAL)
                                    AddAttribute(XML_NAMESPACE_TABLE, XML_MODE, XML_COPY_RESULTS_ONLY);
                                if( nRefresh )
                                {
                                    rtl::OUStringBuffer sBuffer;
                                    SvXMLUnitConverter::convertTime( sBuffer, (double)nRefresh / 86400 );
                                    AddAttribute( XML_NAMESPACE_TABLE, XML_REFRESH_DELAY, sBuffer.makeStringAndClear() );
                                }
                                SvXMLElementExport aSourceElem(*this, XML_NAMESPACE_TABLE, XML_TABLE_SOURCE, sal_True, sal_True);
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
        String      sComment;
        Color       aColor;
        sal_uInt16  nFlags;
        pDoc->GetScenarioData(static_cast<SCTAB>(nCurrentTable), sComment, aColor, nFlags);
        if (!(nFlags & SC_SCENARIO_SHOWFRAME))
            AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_BORDER, XML_FALSE);
        rtl::OUStringBuffer aBuffer;
        SvXMLUnitConverter::convertColor(aBuffer, aColor);
        AddAttribute(XML_NAMESPACE_TABLE, XML_BORDER_COLOR, aBuffer.makeStringAndClear());
        if (!(nFlags & SC_SCENARIO_TWOWAY))
            AddAttribute(XML_NAMESPACE_TABLE, XML_COPY_BACK, XML_FALSE);
        if (!(nFlags & SC_SCENARIO_ATTRIB))
            AddAttribute(XML_NAMESPACE_TABLE, XML_COPY_STYLES, XML_FALSE);
        if (nFlags & SC_SCENARIO_VALUE)
            AddAttribute(XML_NAMESPACE_TABLE, XML_COPY_FORMULAS, XML_FALSE);
        if (nFlags & SC_SCENARIO_PROTECT)
            AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTED, XML_TRUE);
        SvXMLUnitConverter::convertBool(aBuffer, pDoc->IsActiveScenario(static_cast<SCTAB>(nCurrentTable)));
        AddAttribute(XML_NAMESPACE_TABLE, XML_IS_ACTIVE, aBuffer.makeStringAndClear());
        const ScRangeList* pRangeList = pDoc->GetScenarioRanges(static_cast<SCTAB>(nCurrentTable));
        rtl::OUString sRangeListStr;
        ScRangeStringConverter::GetStringFromRangeList( sRangeListStr, pRangeList, pDoc, FormulaGrammar::CONV_OOO );
        AddAttribute(XML_NAMESPACE_TABLE, XML_SCENARIO_RANGES, sRangeListStr);
        if (sComment.Len())
            AddAttribute(XML_NAMESPACE_TABLE, XML_COMMENT, rtl::OUString(sComment));
        SvXMLElementExport aElem(*this, XML_NAMESPACE_TABLE, XML_SCENARIO, sal_True, sal_True);
    }
}

void ScXMLExport::WriteTheLabelRanges( const uno::Reference< sheet::XSpreadsheetDocument >& xSpreadDoc )
{
    uno::Reference< beans::XPropertySet > xDocProp( xSpreadDoc, uno::UNO_QUERY );
    if( !xDocProp.is() ) return;

    sal_Int32 nCount(0);
    uno::Reference< container::XIndexAccess > xColRangesIAccess(xDocProp->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_COLLABELRNG ) ) ), uno::UNO_QUERY);
    if( xColRangesIAccess.is() )
        nCount += xColRangesIAccess->getCount();

    uno::Reference< container::XIndexAccess > xRowRangesIAccess(xDocProp->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_ROWLABELRNG ) ) ), uno::UNO_QUERY);
    if( xRowRangesIAccess.is() )
        nCount += xRowRangesIAccess->getCount();

    if( nCount )
    {
        SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE, XML_LABEL_RANGES, sal_True, sal_True );
        WriteLabelRanges( xColRangesIAccess, sal_True );
        WriteLabelRanges( xRowRangesIAccess, sal_False );
    }
}

void ScXMLExport::WriteLabelRanges( const uno::Reference< container::XIndexAccess >& xRangesIAccess, sal_Bool bColumn )
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
            SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE, XML_LABEL_RANGE, sal_True, sal_True );
        }
    }
}

void ScXMLExport::WriteNamedExpressions(const com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheetDocument>& xSpreadDoc)
{
    uno::Reference <beans::XPropertySet> xPropertySet (xSpreadDoc, uno::UNO_QUERY);
    if (xPropertySet.is())
    {
        uno::Reference <sheet::XNamedRanges> xNamedRanges(xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_NAMEDRANGES))), uno::UNO_QUERY);
        CheckAttrList();
        if (xNamedRanges.is())
        {
            uno::Sequence <rtl::OUString> aRangesNames(xNamedRanges->getElementNames());
            sal_Int32 nNamedRangesCount(aRangesNames.getLength());
            if (nNamedRangesCount > 0)
            {
                if (pDoc)
                {
                    ScRangeName* pNamedRanges(pDoc->GetRangeName());
                    SvXMLElementExport aElemNEs(*this, XML_NAMESPACE_TABLE, XML_NAMED_EXPRESSIONS, sal_True, sal_True);
                    for (sal_Int32 i = 0; i < nNamedRangesCount; ++i)
                    {
                        CheckAttrList();
                        rtl::OUString sNamedRange(aRangesNames[i]);
                        uno::Reference <sheet::XNamedRange> xNamedRange(xNamedRanges->getByName(sNamedRange), uno::UNO_QUERY);
                        if (xNamedRange.is())
                        {
                            uno::Reference <container::XNamed> xNamed (xNamedRange, uno::UNO_QUERY);
                            uno::Reference <sheet::XCellRangeReferrer> xCellRangeReferrer (xNamedRange, uno::UNO_QUERY);
                            if (xNamed.is() && xCellRangeReferrer.is())
                            {
                                rtl::OUString sOUName(xNamed->getName());
                                AddAttribute(sAttrName, sOUName);

                                OUString sOUBaseCellAddress;
                                ScRangeStringConverter::GetStringFromAddress( sOUBaseCellAddress,
                                    xNamedRange->getReferencePosition(), pDoc, FormulaGrammar::CONV_OOO, ' ', sal_False, SCA_ABS_3D );
                                AddAttribute(XML_NAMESPACE_TABLE, XML_BASE_CELL_ADDRESS, sOUBaseCellAddress);

                                sal_uInt16 nRangeIndex;
                                String sName(sOUName);
                                pNamedRanges->SearchName(sName, nRangeIndex);
                                ScRangeData* pNamedRange((*pNamedRanges)[nRangeIndex]); //should get directly and not with ScDocument
                                String sContent;
                                pNamedRange->GetSymbol(sContent, pDoc->GetStorageGrammar());
                                rtl::OUString sOUTempContent(sContent);
                                uno::Reference <table::XCellRange> xCellRange(xCellRangeReferrer->getReferredCells());
                                if(xCellRange.is())
                                {
                                    rtl::OUString sOUContent(sOUTempContent.copy(1, sOUTempContent.getLength() - 2));
                                    AddAttribute(XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, sOUContent);
                                    sal_Int32 nRangeType(xNamedRange->getType());
                                    rtl::OUStringBuffer sBufferRangeType;
                                    if ((nRangeType & sheet::NamedRangeFlag::COLUMN_HEADER) == sheet::NamedRangeFlag::COLUMN_HEADER)
                                        sBufferRangeType.append(GetXMLToken(XML_REPEAT_COLUMN));
                                    if ((nRangeType & sheet::NamedRangeFlag::ROW_HEADER) == sheet::NamedRangeFlag::ROW_HEADER)
                                    {
                                        if (sBufferRangeType.getLength() > 0)
                                            sBufferRangeType.appendAscii(" ");
                                        sBufferRangeType.append(GetXMLToken(XML_REPEAT_ROW));
                                    }
                                    if ((nRangeType & sheet::NamedRangeFlag::FILTER_CRITERIA) == sheet::NamedRangeFlag::FILTER_CRITERIA)
                                    {
                                        if (sBufferRangeType.getLength() > 0)
                                            sBufferRangeType.appendAscii(" ");
                                        sBufferRangeType.append(GetXMLToken(XML_FILTER));
                                    }
                                    if ((nRangeType & sheet::NamedRangeFlag::PRINT_AREA) == sheet::NamedRangeFlag::PRINT_AREA)
                                    {
                                        if (sBufferRangeType.getLength() > 0)
                                            sBufferRangeType.appendAscii(" ");
                                        sBufferRangeType.append(GetXMLToken(XML_PRINT_RANGE));
                                    }
                                    rtl::OUString sRangeType = sBufferRangeType.makeStringAndClear();
                                    if (sRangeType.getLength())
                                        AddAttribute(XML_NAMESPACE_TABLE, XML_RANGE_USABLE_AS, sRangeType);
                                    SvXMLElementExport aElemNR(*this, XML_NAMESPACE_TABLE, XML_NAMED_RANGE, sal_True, sal_True);
                                }
                                else
                                {
                                    AddAttribute(XML_NAMESPACE_TABLE, XML_EXPRESSION, sOUTempContent);
                                    SvXMLElementExport aElemNE(*this, XML_NAMESPACE_TABLE, XML_NAMED_EXPRESSION, sal_True, sal_True);
                                }
                            }
                        }
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
        const String* pUrl = pRefMgr->getExternalFileName(nFileId);
        if (!pUrl)
            continue;

        vector<String> aTabNames;
        pRefMgr->getAllCachedTableNames(nFileId, aTabNames);
        if (aTabNames.empty())
            continue;

        for (vector<String>::const_iterator itr = aTabNames.begin(), itrEnd = aTabNames.end();
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
            SvXMLElementExport aElemTable(*this, XML_NAMESPACE_TABLE, XML_TABLE, sal_True, sal_True);
            {
                const ScExternalRefManager::SrcFileData* pExtFileData = pRefMgr->getExternalFileData(nFileId);
                if (pExtFileData)
                {
                    String aRelUrl;
                    if (pExtFileData->maRelativeName.Len())
                        aRelUrl = pExtFileData->maRelativeName;
                    else
                        aRelUrl = GetRelativeReference(pExtFileData->maRelativeName);
                    AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE);
                    AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, aRelUrl);
                    AddAttribute(XML_NAMESPACE_TABLE, XML_TABLE_NAME, *itr);
                    if (pExtFileData->maFilterName.Len())
                        AddAttribute(XML_NAMESPACE_TABLE, XML_FILTER_NAME, pExtFileData->maFilterName);
                    if (pExtFileData->maFilterOptions.Len())
                        AddAttribute(XML_NAMESPACE_TABLE, XML_FILTER_OPTIONS, pExtFileData->maFilterOptions);
                    AddAttribute(XML_NAMESPACE_TABLE, XML_MODE, XML_COPY_RESULTS_ONLY);
                }
                SvXMLElementExport aElemTableSource(*this, XML_NAMESPACE_TABLE, XML_TABLE_SOURCE, sal_True, sal_True);
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
                                    OUString::valueOf(static_cast<sal_Int32>(nMaxColsUsed)));
                SvXMLElementExport aElemColumn(*this, XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, sal_True, sal_True);
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
                        SvXMLElementExport aElemRow(*this, XML_NAMESPACE_TABLE, XML_TABLE_ROW, sal_True, sal_True);
                        OUStringBuffer aVal;
                        aVal.append(static_cast<sal_Int32>(nMaxColsUsed));
                        AddAttribute(XML_NAMESPACE_TABLE,  XML_NUMBER_COLUMNS_REPEATED, aVal.makeStringAndClear());
                        SvXMLElementExport aElemCell(*this, XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True);
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
                        SvXMLElementExport aElemRow(*this, XML_NAMESPACE_TABLE, XML_TABLE_ROW, sal_True, sal_True);
                        OUStringBuffer aVal;
                        aVal.append(static_cast<sal_Int32>(nMaxColsUsed));
                        AddAttribute(XML_NAMESPACE_TABLE,  XML_NUMBER_COLUMNS_REPEATED, aVal.makeStringAndClear());
                        SvXMLElementExport aElemCell(*this, XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True);
                    }
                }
                SvXMLElementExport aElemRow(*this, XML_NAMESPACE_TABLE, XML_TABLE_ROW, sal_True, sal_True);

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
                            SvXMLElementExport aElemCell(*this, XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True);
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
                            SvXMLElementExport aElemCell(*this, XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True);
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
                    SvXMLElementExport aElemCell(*this, XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True);
                    SvXMLElementExport aElemText(*this, XML_NAMESPACE_TEXT, XML_P, sal_True, sal_False);
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
                ScRangeStringConverter::GetStringFromArea( sStrData, *pCons->ppDataAreas[ nIndex ], pDoc, FormulaGrammar::CONV_OOO, sal_True );
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

            SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE, XML_CONSOLIDATION, sal_True, sal_True );
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
                pChangeProps[SC_SHOW_CHANGES].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChanges"));
                pChangeProps[SC_SHOW_CHANGES].Value <<= pViewSettings->ShowChanges();
                pChangeProps[SC_SHOW_ACCEPTED_CHANGES].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowAcceptedChanges"));
                pChangeProps[SC_SHOW_ACCEPTED_CHANGES].Value <<= pViewSettings->IsShowAccepted();
                pChangeProps[SC_SHOW_REJECTED_CHANGES].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowRejectedChanges"));
                pChangeProps[SC_SHOW_REJECTED_CHANGES].Value <<= pViewSettings->IsShowRejected();
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByDatetime"));
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME].Value <<= pViewSettings->HasDate();
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_MODE].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByDatetimeMode"));
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_MODE].Value <<= static_cast<sal_Int16>(pViewSettings->GetTheDateMode());
                util::DateTime aDateTime;
                ScXMLConverter::ConvertCoreToAPIDateTime(pViewSettings->GetTheFirstDateTime(), aDateTime);
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_FIRST_DATETIME].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByDatetimeFirstDatetime"));
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_FIRST_DATETIME].Value <<= aDateTime;
                ScXMLConverter::ConvertCoreToAPIDateTime(pViewSettings->GetTheLastDateTime(), aDateTime);
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_SECOND_DATETIME].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByDatetimeSecondDatetime"));
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_SECOND_DATETIME].Value <<= aDateTime;
                pChangeProps[SC_SHOW_CHANGES_BY_AUTHOR].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByAuthor"));
                pChangeProps[SC_SHOW_CHANGES_BY_AUTHOR].Value <<= pViewSettings->HasAuthor();
                pChangeProps[SC_SHOW_CHANGES_BY_AUTHOR_NAME].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByAuthorName"));
                pChangeProps[SC_SHOW_CHANGES_BY_AUTHOR_NAME].Value <<= rtl::OUString (pViewSettings->GetTheAuthorToShow());
                pChangeProps[SC_SHOW_CHANGES_BY_COMMENT].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByComment"));
                pChangeProps[SC_SHOW_CHANGES_BY_COMMENT].Value <<= pViewSettings->HasComment();
                pChangeProps[SC_SHOW_CHANGES_BY_COMMENT_TEXT].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByCommentText"));
                pChangeProps[SC_SHOW_CHANGES_BY_COMMENT_TEXT].Value <<= rtl::OUString (pViewSettings->GetTheComment());
                pChangeProps[SC_SHOW_CHANGES_BY_RANGES].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByRanges"));
                pChangeProps[SC_SHOW_CHANGES_BY_RANGES].Value <<= pViewSettings->HasRange();
                rtl::OUString sRangeList;
                ScRangeStringConverter::GetStringFromRangeList(sRangeList, &(pViewSettings->GetTheRangeList()), GetDocument(), FormulaGrammar::CONV_OOO);
                pChangeProps[SC_SHOW_CHANGES_BY_RANGES_LIST].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByRangesList"));
                pChangeProps[SC_SHOW_CHANGES_BY_RANGES_LIST].Value <<= sRangeList;

                pProps[nChangePos].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TrackedChangesViewSettings"));
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
                    pProps[i].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VisibleAreaTop"));
                    pProps[i].Value <<= static_cast<sal_Int32>(aRect.getY());
                    pProps[++i].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VisibleAreaLeft"));
                    pProps[i].Value <<= static_cast<sal_Int32>(aRect.getX());
                    pProps[++i].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VisibleAreaWidth"));
                    pProps[i].Value <<= static_cast<sal_Int32>(aRect.getWidth());
                    pProps[++i].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VisibleAreaHeight"));
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
            uno::Reference <beans::XPropertySet> xProperties(xMultiServiceFactory->createInstance(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.SpreadsheetSettings"))), uno::UNO_QUERY);
            if (xProperties.is())
                SvXMLUnitConverter::convertPropertySet(rProps, xProperties);

            sal_Int32 nPropsToAdd = 0;
            rtl::OUStringBuffer aTrackedChangesKey;
            if (GetDocument() && GetDocument()->GetChangeTrack() && GetDocument()->GetChangeTrack()->IsProtected())
            {
                SvXMLUnitConverter::encodeBase64(aTrackedChangesKey, GetDocument()->GetChangeTrack()->GetProtection());
                if (aTrackedChangesKey.getLength())
                    ++nPropsToAdd;
            }

            bool bVBACompat = false;
            uno::Reference <container::XNameAccess> xCodeNameAccess;
            DBG_ASSERT( pDoc, "ScXMLExport::GetConfigurationSettings - no ScDocument!" );
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
                if (aTrackedChangesKey.getLength())
                {
                    rProps[nCount].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TrackedChangesProtectionKey"));
                    rProps[nCount].Value <<= aTrackedChangesKey.makeStringAndClear();
                    ++nCount;
                }
                if( bVBACompat )
                {
                    rProps[nCount].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VBACompatibilityMode"));
                    rProps[nCount].Value <<= bVBACompat;
                    ++nCount;
                }
                if( xCodeNameAccess.is() )
                {
                    rProps[nCount].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScriptConfiguration"));
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

void ScXMLExport::IncrementProgressBar(sal_Bool bEditCell, sal_Int32 nInc)
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
                getDefaultVersion() == SvtSaveOptions::ODFVER_LATEST )
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
    ScUnoGuard aGuard;
    SvXMLExport::setSourceDocument( xComponent );

    pDoc = ScXMLConverter::GetScDocument( GetModel() );
    DBG_ASSERT( pDoc, "ScXMLExport::setSourceDocument - no ScDocument!" );
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
    ScUnoGuard aGuard;
    if (pDoc)
        pDoc->DisableIdle(sal_True);
    sal_Bool bReturn(SvXMLExport::filter(aDescriptor));
    if (pDoc)
        pDoc->DisableIdle(sal_False);
    return bReturn;
}

void SAL_CALL ScXMLExport::cancel()
    throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pDoc)
        pDoc->DisableIdle(sal_False);
    SvXMLExport::cancel();
}

// XInitialization
void SAL_CALL ScXMLExport::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    SvXMLExport::initialize(aArguments);
}

// XServiceInfo
::rtl::OUString SAL_CALL ScXMLExport::getImplementationName(  )
    throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;

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
    return SvXMLExport::getImplementationName();
}

sal_Bool SAL_CALL ScXMLExport::supportsService( const ::rtl::OUString& ServiceName )
    throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return SvXMLExport::supportsService( ServiceName );
}

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL ScXMLExport::getSupportedServiceNames(  )
    throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return SvXMLExport::getSupportedServiceNames();
}

// XUnoTunnel
sal_Int64 SAL_CALL ScXMLExport::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier )
    throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return SvXMLExport::getSomething(aIdentifier);
}

void ScXMLExport::DisposingModel()
{
    SvXMLExport::DisposingModel();
    pDoc = NULL;
    xCurrentTable = 0;
}

