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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#define ITEMID_FIELD EE_FEATURE_FIELD


#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif
#ifndef _SC_XMLSTYLE_HXX
#include "xmlstyle.hxx"
#endif
#ifndef SC_UNONAMES_HXX
#include "unonames.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_OUTLINETAB_HXX
#include "olinetab.hxx"
#endif
#ifndef SC_CELLSUNO_HXX
#include "cellsuno.hxx"
#endif
#ifndef SC_CELL_HXX
#include "cell.hxx"
#endif
#ifndef SC_RANGENAM_HXX
#include "rangenam.hxx"
#endif
#ifndef _SC_XMLTABLEMASTERPAGEEXPORT_HXX
#include "XMLTableMasterPageExport.hxx"
#endif
#ifndef SC_DRWLAYER_HXX
#include "drwlayer.hxx"
#endif
#ifndef _SC_XMLEXPORTDATAPILOT_HXX
#include "XMLExportDataPilot.hxx"
#endif
#ifndef SC_XMLEXPORTDATABASERANGES_HXX
#include "XMLExportDatabaseRanges.hxx"
#endif
#ifndef _SC_XMLEXPORTDDELINKS_HXX
#include "XMLExportDDELinks.hxx"
#endif
#ifndef _SC_XMLEXPORTITERATOR_HXX
#include "XMLExportIterator.hxx"
#endif
#ifndef _SC_XMLCOLUMNROWGROUPEXPORT_HXX
#include "XMLColumnRowGroupExport.hxx"
#endif
#ifndef _SC_XMLSTYLESEXPORTHELPER_HXX
#include "XMLStylesExportHelper.hxx"
#endif
#ifndef _SC_XMLCHANGETRACKINGEXPORTHELPER_HXX
#include "XMLChangeTrackingExportHelper.hxx"
#endif
#ifndef SC_DOCOPTIO_HXX
#include "docoptio.hxx"
#endif
#ifndef SC_XMLEXPORTSHAREDDATA_HXX
#include "XMLExportSharedData.hxx"
#endif
#ifndef SC_CHGVISET_HXX
#include "chgviset.hxx"
#endif
#ifndef SC_DOCUNO_HXX
#include "docuno.hxx"
#endif
#ifndef SC_CHARTLIS_HXX
#include "chartlis.hxx"
#endif
#ifndef SC_UNOGUARD_HXX
#include "unoguard.hxx"
#endif
#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#ifndef SC_SCDOCPOL_HXX
#include "docpool.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <bf_xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <bf_xmloff/nmspmap.hxx>
#endif
#ifndef XMLOFF_NUMEHELP_HXX
#include <bf_xmloff/numehelp.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <bf_xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_XMLCNITM_HXX
#include <bf_xmloff/xmlcnitm.hxx>
#endif


#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#ifndef _ZFORLIST_HXX
#include <bf_svtools/zforlist.hxx>
#endif
#ifndef _SVX_UNOSHAPE_HXX
#include <bf_svx/unoshape.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _EMBOBJ_HXX
#include <bf_so3/embobj.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XUSEDAREACURSOR_HPP_
#include <com/sun/star/sheet/XUsedAreaCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XAREALINKS_HPP_
#include <com/sun/star/sheet/XAreaLinks.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XAREALINK_HPP_
#include <com/sun/star/sheet/XAreaLink.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_CELLFLAGS_HPP_
#include <com/sun/star/sheet/CellFlags.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XLABELRANGES_HPP_
#include <com/sun/star/sheet/XLabelRanges.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XLABELRANGE_HPP_
#include <com/sun/star/sheet/XLabelRange.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XNAMEDRANGES_HPP_
#include <com/sun/star/sheet/XNamedRanges.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XNAMEDRANGE_HPP_
#include <com/sun/star/sheet/XNamedRange.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLRANGEREFERRER_HPP_
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_NAMEDRANGEFLAG_HPP_
#include <com/sun/star/sheet/NamedRangeFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMSUPPLIER_HPP_
#include <com/sun/star/form/XFormsSupplier.hpp>
#endif
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#endif
namespace binfilter {
//! not found in unonames.hxx
#define SC_STANDARDFORMAT "StandardFormat"
#define SC_LAYERID "LayerID"

#define SC_DEFAULT_TABLE_COUNT						3
#define SC_VIEWCHANGES_COUNT						13
#define SC_SHOW_CHANGES								0
#define SC_SHOW_ACCEPTED_CHANGES					1
#define SC_SHOW_REJECTED_CHANGES					2
#define SC_SHOW_CHANGES_BY_DATETIME					3
#define SC_SHOW_CHANGES_BY_DATETIME_MODE			4
#define SC_SHOW_CHANGES_BY_DATETIME_FIRST_DATETIME	5
#define SC_SHOW_CHANGES_BY_DATETIME_SECOND_DATETIME	6
#define SC_SHOW_CHANGES_BY_AUTHOR					7
#define SC_SHOW_CHANGES_BY_AUTHOR_NAME				8
#define SC_SHOW_CHANGES_BY_COMMENT					9
#define SC_SHOW_CHANGES_BY_COMMENT_TEXT				10
#define SC_SHOW_CHANGES_BY_RANGES					11
#define SC_SHOW_CHANGES_BY_RANGES_LIST				12

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

//----------------------------------------------------------------------------

OUString SAL_CALL ScXMLExport_getImplementationName() throw()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLExporter" ) );
}

uno::Sequence< ::rtl::OUString > SAL_CALL ScXMLExport_getSupportedServiceNames() throw()
{
    const ::rtl::OUString aServiceName( ScXMLExport_getImplementationName() );
    const uno::Sequence< ::rtl::OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLExport_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLExport(EXPORT_ALL);
    return (cppu::OWeakObject*)new ScXMLExport( rSMgr, EXPORT_ALL );
}

OUString SAL_CALL ScXMLExport_Meta_getImplementationName() throw()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLMetaExporter" ) );
}

uno::Sequence< ::rtl::OUString > SAL_CALL ScXMLExport_Meta_getSupportedServiceNames() throw()
{
    const ::rtl::OUString aServiceName( ScXMLExport_Meta_getImplementationName() );
    const uno::Sequence< ::rtl::OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLExport_Meta_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLExport(EXPORT_META);
    return (cppu::OWeakObject*)new ScXMLExport( rSMgr, EXPORT_META );
}

OUString SAL_CALL ScXMLExport_Styles_getImplementationName() throw()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLStylesExporter" ) );
}

uno::Sequence< ::rtl::OUString > SAL_CALL ScXMLExport_Styles_getSupportedServiceNames() throw()
{
    const ::rtl::OUString aServiceName( ScXMLExport_Styles_getImplementationName() );
    const uno::Sequence< ::rtl::OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLExport_Styles_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLExport(EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_FONTDECLS);
    return (cppu::OWeakObject*)new ScXMLExport( rSMgr, EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_FONTDECLS);
}

OUString SAL_CALL ScXMLExport_Content_getImplementationName() throw()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLContentExporter" ) );
}

uno::Sequence< ::rtl::OUString > SAL_CALL ScXMLExport_Content_getSupportedServiceNames() throw()
{
    const ::rtl::OUString aServiceName( ScXMLExport_Content_getImplementationName() );
    const uno::Sequence< ::rtl::OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLExport_Content_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLExport(EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS);
    return (cppu::OWeakObject*)new ScXMLExport( rSMgr, EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS);
}

OUString SAL_CALL ScXMLExport_Settings_getImplementationName() throw()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Calc.XMLSettingsExporter" ) );
}

uno::Sequence< ::rtl::OUString > SAL_CALL ScXMLExport_Settings_getSupportedServiceNames() throw()
{
    const ::rtl::OUString aServiceName( ScXMLExport_Settings_getImplementationName() );
    const uno::Sequence< ::rtl::OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL ScXMLExport_Settings_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr ) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new ScXMLExport(EXPORT_SETTINGS);
    return (cppu::OWeakObject*)new ScXMLExport( rSMgr, EXPORT_SETTINGS );
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
        sal_Int16 nLayerID;
        if( (xShapeProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SC_LAYERID ))) >>= nLayerID) && (nLayerID == SC_LAYER_BACK) )
            GetExport().AddAttribute(XML_NAMESPACE_TABLE, XML_TABLE_BACKGROUND, XML_TRUE);
    }
}

//----------------------------------------------------------------------------

sal_Int16 ScXMLExport::GetFieldUnit()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xProperties(
                ::legacy_binfilters::getLegacyProcessServiceFactory()->createInstance(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sheet.GlobalSheetSettings" )) ),
                ::com::sun::star::uno::UNO_QUERY);
    if (xProperties.is())
    {
        ::com::sun::star::uno::Any aAny = xProperties->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Metric")));
        sal_Int16 nFieldUnit;
        if (aAny >>= nFieldUnit)
            return nFieldUnit;
    }
    return 0;
}


// #110680#
ScXMLExport::ScXMLExport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    const sal_uInt16 nExportFlag) 
:	SvXMLExport( xServiceFactory, SvXMLUnitConverter::GetMapUnit(GetFieldUnit()), XML_SPREADSHEET, nExportFlag ),
    pDoc(NULL),
    mbShowProgress( sal_False ),
    pSharedData(NULL),
    pColumnStyles(NULL),
    pRowStyles(NULL),
    pCellStyles(NULL),
    pMergedRangesContainer(NULL),
    pValidationsContainer(NULL),
    pRowFormatRanges(NULL),
    pCellsItr(NULL),
    pChangeTrackingExportHelper(NULL),
    pDefaults(NULL),
    pNumberFormatAttributesExportHelper(NULL),
    pGroupColumns (NULL),
    pGroupRows (NULL),
    nOpenRow(-1),
    nCurrentTable(0),
    aTableStyles(),
    bHasRowHeader(sal_False),
    bRowHeaderOpen(sal_False),
    sLayerID(RTL_CONSTASCII_USTRINGPARAM( SC_LAYERID )),
    sCaptionShape(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.CaptionShape")),
    pChartListener(NULL)
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
    xCellStylesExportPropertySetMapper->ChainExportMapper(XMLTextParagraphExport::CreateCharExtPropMapper(*this));
    xColumnStylesExportPropertySetMapper = new ScXMLColumnExportPropertyMapper(xColumnStylesPropertySetMapper);
    xRowStylesExportPropertySetMapper = new ScXMLRowExportPropertyMapper(xRowStylesPropertySetMapper);
    xTableStylesExportPropertySetMapper = new ScXMLTableExportPropertyMapper(xTableStylesPropertySetMapper);

    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_CELL, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME)),
        xCellStylesExportPropertySetMapper, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX)));
    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_COLUMN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME)),
        xColumnStylesExportPropertySetMapper, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX)));
    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_ROW, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_ROW_STYLES_NAME)),
        xRowStylesExportPropertySetMapper, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_ROW_STYLES_PREFIX)));
    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_TABLE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME)),
        xTableStylesExportPropertySetMapper, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_TABLE_STYLES_PREFIX)));

    if( (getExportFlags() & (EXPORT_STYLES|EXPORT_AUTOSTYLES|EXPORT_MASTERSTYLES|EXPORT_CONTENT) ) != 0 )
    {
        sAttrName = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TABLE, GetXMLToken(XML_NAME));
        sAttrStyleName = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TABLE, GetXMLToken(XML_STYLE_NAME));
        sAttrColumnsRepeated = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TABLE, GetXMLToken(XML_NUMBER_COLUMNS_REPEATED));
        sAttrFormula = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TABLE, GetXMLToken(XML_FORMULA));
        sAttrStringValue = GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TABLE, GetXMLToken(XML_STRING_VALUE));
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

sal_Bool ScXMLExport::HasDrawPages(uno::Reference <sheet::XSpreadsheetDocument>& xDoc)
{
    sal_Bool bRet(sal_False);
    uno::Reference <beans::XPropertySet> xDocProps( xDoc, uno::UNO_QUERY );
    if (xDocProps.is())
    {
        uno::Any aAny(xDocProps->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_HASDRAWPAGES))));
        bRet = ::cppu::any2bool( aAny );
    }
    return bRet;
}

void ScXMLExport::CollectSharedData(sal_Int32& nTableCount, sal_Int32& nShapesCount, const sal_Int32 nCellCount)
{
    if (GetModel().is())
    {
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( GetModel(), uno::UNO_QUERY );
        if ( xSpreadDoc.is())
        {
            uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
            uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
            if ( xIndex.is() )
            {
                nTableCount = xIndex->getCount();
                if (!pSharedData)
                    CreateSharedData(nTableCount);
                pCellStyles->AddNewTable(nTableCount - 1);
                if (HasDrawPages(xSpreadDoc))
                {
                    ::rtl::OUString sCaptionPoint( RTL_CONSTASCII_USTRINGPARAM( "CaptionPoint" ));
                    for (sal_Int32 nTable = 0; nTable < nTableCount; nTable++)
                    {
                        uno::Any aTable = xIndex->getByIndex(nTable);
                        uno::Reference<sheet::XSpreadsheet> xTable;
                        if (aTable>>=xTable)
                        {
                            uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xTable, uno::UNO_QUERY);
                            if (xDrawPageSupplier.is())
                            {
                                uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
                                ScMyDrawPage aDrawPage;
                                aDrawPage.bHasForms = sal_False;
                                aDrawPage.xDrawPage = xDrawPage;
                                pSharedData->AddDrawPage(aDrawPage, nTable);
                                uno::Reference<container::XIndexAccess> xShapesIndex (xDrawPage, uno::UNO_QUERY);
                                if (xShapesIndex.is())
                                {
                                    sal_Int32 nShapes = xShapesIndex->getCount();
                                    for (sal_Int32 nShape = 0; nShape < nShapes; nShape++)
                                    {
                                        uno::Any aShape = xShapesIndex->getByIndex(nShape);
                                        uno::Reference<drawing::XShape> xShape;
                                        if (aShape >>= xShape)
                                        {
                                            uno::Reference< beans::XPropertySet > xShapeProp( xShape, uno::UNO_QUERY );
                                            if( xShapeProp.is() )
                                            {
                                                uno::Any aPropAny = xShapeProp->getPropertyValue(sLayerID);
                                                sal_Int16 nLayerID;
                                                if( aPropAny >>= nLayerID )
                                                {
                                                    if( nLayerID == SC_LAYER_INTERN )
                                                        CollectInternalShape( xShape );
                                                    else
                                                    {
                                                        nShapesCount++;
                                                        SvxShape* pShapeImp = SvxShape::getImplementation(xShape);
                                                        if (pShapeImp)
                                                        {
                                                            SdrObject *pSdrObj = pShapeImp->GetSdrObject();
                                                            if (pSdrObj)
                                                            {
                                                                if (ScDrawLayer::GetAnchor(pSdrObj) == SCA_CELL)
                                                                {
                                                                    if (pDoc)
                                                                    {

                                                                        awt::Point aPoint(xShape->getPosition());
                                                                        awt::Size aSize(xShape->getSize());
                                                                        ::rtl::OUString sType(xShape->getShapeType());
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
                                                                        ScRange aRange = pDoc->GetRange(static_cast<USHORT>(nTable), aRectangle);
                                                                        ScMyShape aMyShape;
                                                                        aMyShape.aAddress = aRange.aStart;
                                                                        aMyShape.aEndAddress = aRange.aEnd;
                                                                        aMyShape.xShape = xShape;
                                                                        aMyShape.nLayerID = nLayerID;
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
    }
    sal_Int32 nRef(nCellCount + (2 * nTableCount) + (2 * nShapesCount));
    GetProgressBarHelper()->SetReference(nRef);
    GetProgressBarHelper()->SetValue(0);
}

void ScXMLExport::CollectShapesAutoStyles(const sal_Int32 nTableCount)
{
    pSharedData->SortShapesContainer();
    const ScMyShapeList* pShapeList = NULL;
    ScMyShapeList::const_iterator aShapeItr;
    if (pSharedData->GetShapesContainer())
    {
        pShapeList = pSharedData->GetShapesContainer()->GetShapes();
        aShapeItr = pShapeList->begin();
    }
    if (pSharedData->HasDrawPage())
    {
        for (sal_Int32 nTable = 0; nTable < nTableCount; nTable++)
        {
            uno::Reference<drawing::XDrawPage> xDrawPage(pSharedData->GetDrawPage(nTable));
            uno::Reference<drawing::XShapes> xShapes (xDrawPage, uno::UNO_QUERY);
            if (xShapes.is())
            {
                GetShapeExport()->seekShapes(xShapes);
                uno::Reference< form::XFormsSupplier > xFormsSupplier( xDrawPage, uno::UNO_QUERY );
                if( xFormsSupplier.is() )
                {
                    uno::Reference< container::XNameContainer > xForms( xFormsSupplier->getForms() );
                    if( xForms.is() && xForms->hasElements() )
                    {
                        GetFormExport()->examineForms(xDrawPage);
                        pSharedData->SetDrawPageHasForms(nTable, sal_True);
                    }
                }
                ScMyTableShapes* pTableShapes = pSharedData->GetTableShapes();
                if (pTableShapes)
                {
                    ScMyTableXShapes::iterator aItr = (*pTableShapes)[nTable].begin();
                    while (aItr != (*pTableShapes)[nTable].end())
                    {
                        GetShapeExport()->collectShapeAutoStyles(*aItr);
                        GetProgressBarHelper()->Increment();
                        aItr++;
                    }
                }
                if (pShapeList)
                {
                    while (aShapeItr != pShapeList->end() && (static_cast<sal_Int32>(aShapeItr->aAddress.Tab()) == nTable))
                    {
                        GetShapeExport()->collectShapeAutoStyles(aShapeItr->xShape);
                        GetProgressBarHelper()->Increment();
                        aShapeItr++;
                    }
                }
            }
        }
    }
}

void ScXMLExport::_ExportMeta()
{
    SvXMLExport::_ExportMeta();
    sal_Int32 nCellCount(pDoc ? pDoc->GetCellCount() : 0);
    sal_Int32 nTableCount(0);
    sal_Int32 nShapesCount(0);
    GetAutoStylePool()->ClearEntries();
    CollectSharedData(nTableCount, nShapesCount, nCellCount);
    ::rtl::OUStringBuffer sBuffer;
    if (nTableCount)
    {
        GetMM100UnitConverter().convertNumber(sBuffer, nTableCount);
        AddAttribute(XML_NAMESPACE_META, XML_TABLE_COUNT, sBuffer.makeStringAndClear());
    }
    if (nCellCount)
    {
        GetMM100UnitConverter().convertNumber(sBuffer, nCellCount);
        AddAttribute(XML_NAMESPACE_META, XML_CELL_COUNT, sBuffer.makeStringAndClear());
    }
    if (nShapesCount)
    {
        GetMM100UnitConverter().convertNumber(sBuffer, nShapesCount);
        AddAttribute(XML_NAMESPACE_META, XML_OBJECT_COUNT, sBuffer.makeStringAndClear());
    }
    {
        SvXMLElementExport aElemStat(*this, XML_NAMESPACE_META, XML_DOCUMENT_STATISTIC, sal_True, sal_True);
    }
}

void ScXMLExport::_ExportFontDecls()
{
    GetFontAutoStylePool();	// make sure the pool is created
    SvXMLExport::_ExportFontDecls();
}

table::CellRangeAddress ScXMLExport::GetEndAddress(uno::Reference<sheet::XSpreadsheet>& xTable,const sal_uInt16 nTable)
{
    table::CellRangeAddress aCellAddress;
    uno::Reference<sheet::XSheetCellCursor> xCursor = xTable->createCursor();
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

    uno::Reference< sheet::XAreaLinks > xAreaLinks;
    uno::Any aAny( xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_AREALINKS ) ) ) );
    if( aAny >>= xAreaLinks )
    {
        uno::Reference< container::XIndexAccess > xLinksIAccess( xAreaLinks, uno::UNO_QUERY );
        if( xLinksIAccess.is() )
        {
            const OUString sFilter( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_FILTER ) );
            const OUString sFilterOpt( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_FILTOPT ) );
            const OUString sURL( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_LINKURL ) );
            const OUString sRefresh( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_REFDELAY ) );

            sal_Int32 nCount = xLinksIAccess->getCount();
            for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
            {
                uno::Reference< sheet::XAreaLink > xAreaLink;
                uno::Any aLinkAny( xLinksIAccess->getByIndex( nIndex ) );
                if( aLinkAny >>= xAreaLink )
                {
                    ScMyAreaLink aAreaLink;
                    aAreaLink.aDestRange = xAreaLink->getDestArea();
                    aAreaLink.sSourceStr = xAreaLink->getSourceArea();
                    uno::Reference< beans::XPropertySet > xLinkProp( xAreaLink, uno::UNO_QUERY );
                    if( xLinkProp.is() )
                    {
                        aLinkAny = xLinkProp->getPropertyValue( sFilter );
                        aLinkAny >>= aAreaLink.sFilter;
                        aLinkAny = xLinkProp->getPropertyValue( sFilterOpt );
                        aLinkAny >>= aAreaLink.sFilterOptions;
                        aLinkAny = xLinkProp->getPropertyValue( sURL );
                        aLinkAny >>= aAreaLink.sURL;
                        aLinkAny = xLinkProp->getPropertyValue( sRefresh );
                        aLinkAny >>= aAreaLink.nRefresh;
                    }
                    rAreaLinks.AddNewAreaLink( aAreaLink );
                }
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
        ScDetOpList* pOpList = pDoc->GetDetOpList();
        if( pOpList )
        {
            sal_uInt32 nCount = pOpList->Count();
            for( sal_uInt32 nIndex = 0; nIndex < nCount; nIndex++ )
            {
                ScDetOpData* pDetData = pOpList->GetObject( static_cast<USHORT>(nIndex) );
                if( pDetData )
                {
                    const ScAddress& rDetPos = pDetData->GetPos();
                    sal_uInt16 nTab = rDetPos.Tab();
                    if ( nTab < pDoc->GetTableCount() )
                    {
                        rDetOp.AddOperation( pDetData->GetOperation(), rDetPos, nIndex );

                        // #i61888# cells with detective operations are written even if empty
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
        OUString sOUEndCol = OUString::valueOf(static_cast <sal_Int32> (nRepeatColumns));
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
    for (sal_Int32 i = nColumn + 1; i < nColumn + nRepeatColumns; i++)
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
            nRepeat++;
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

void ScXMLExport::ExportColumns(const sal_uInt16 nTable, const table::CellRangeAddress& aColumnHeaderRange, const sal_Bool bHasColumnHeader)
{
    sal_Int32 nColsRepeated (1);
    ::rtl::OUString sParent;
    sal_Int32 nIndex;
    sal_Int32 nPrevColumn(0);
    sal_Bool bPrevIsVisible (sal_True);
    sal_Bool bWasHeader (sal_False);
    sal_Bool bIsHeader (sal_False);
    sal_Bool bIsClosed (sal_True);
    sal_Bool bIsFirst (sal_False);
    sal_Int32 nPrevIndex (-1);
    sal_Int32 nColumn;

    for (nColumn = 0; nColumn <= pSharedData->GetLastColumn(nTable); nColumn++)
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
            nColsRepeated++;
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
                    ::rtl::OUStringBuffer aBuf;
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
            ::rtl::OUStringBuffer aBuf;
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
        ::rtl::OUStringBuffer aBuf;
        GetMM100UnitConverter().convertNumber(aBuf, nEqualRows);
        AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_ROWS_REPEATED, aBuf.makeStringAndClear());
    }

    const ScMyDefaultStyleList& rRowDefaults = *pDefaults->GetRowDefaults();
    if ( nRow >= rRowDefaults.size() )
    {
        // #i61888# used to happen with detective operations - if there are more cases, use the last row's style
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

void ScXMLExport::OpenRow(const sal_uInt16 nTable, const sal_Int32 nStartRow, const sal_Int32 nRepeatRow)
{
    if (nRepeatRow > 1)
    {
        sal_Int32 nPrevIndex(0), nIndex;
        sal_Int8 nPrevFlag(0);
        sal_Int8 nFlag(0);
        sal_Int32 nEqualRows(1);
        sal_Int32 nEndRow(nStartRow + nRepeatRow);
        sal_Int32 nRow;

        for (nRow = nStartRow; nRow < nEndRow; nRow++)
        {
            if (nRow == nStartRow)
            {
                nPrevIndex = pRowStyles->GetStyleNameIndex(nTable, nRow);
                if (pDoc)
                    nPrevFlag = (pDoc->GetRowFlags(static_cast<USHORT>(nRow), nTable)) & (CR_HIDDEN | CR_FILTERED);
            }
            else
            {
                nIndex = pRowStyles->GetStyleNameIndex(nTable, nRow);
                if (pDoc)
                    nFlag = (pDoc->GetRowFlags(static_cast<USHORT>(nRow), nTable)) & (CR_HIDDEN | CR_FILTERED);
                if (nIndex == nPrevIndex && nFlag == nPrevFlag &&
                    !(bHasRowHeader && ((nRow == aRowHeaderRange.StartRow) || (nRow - 1 == aRowHeaderRange.EndRow))) &&
                    !(pGroupRows->IsGroupStart(nRow)) &&
                    !(pGroupRows->IsGroupEnd(nRow - 1)))
                    nEqualRows++;
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
            nFlag = (pDoc->GetRowFlags(static_cast<USHORT>(nStartRow), nTable)) & (CR_HIDDEN | CR_FILTERED);
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
    const sal_Int32 nEndCol, const sal_Int32 nEndRow, const sal_uInt16 nSheet)
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
            sal_Int32 nRows = 1;
            sal_Int32 nTotalRows = nEndRow - nStartRow + 1 - 1;
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
            sal_Int32 nRows = 0;
            sal_Int32 nTotalRows = nEndRow - nStartRow + 1 - 1;
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

sal_Bool ScXMLExport::GetColumnHeader(::com::sun::star::table::CellRangeAddress& aColumnHeaderRange) const
{
    sal_Bool bResult(sal_False);
    uno::Reference <sheet::XPrintAreas> xPrintAreas (xCurrentTable, uno::UNO_QUERY);
    if (xPrintAreas.is())
    {
        bResult = xPrintAreas->getPrintTitleColumns();
        aColumnHeaderRange = xPrintAreas->getTitleColumns();
    }
    return bResult;
}

sal_Bool ScXMLExport::GetRowHeader(::com::sun::star::table::CellRangeAddress& aRowHeaderRange) const
{
    sal_Bool bResult(sal_False);
    uno::Reference <sheet::XPrintAreas> xPrintAreas (xCurrentTable, uno::UNO_QUERY);
    if (xPrintAreas.is())
    {
        bResult = xPrintAreas->getPrintTitleRows();
        aRowHeaderRange = xPrintAreas->getTitleRows();
    }
    return bResult;
}

void ScXMLExport::FillFieldGroup(ScOutlineArray* pFields, ScMyOpenCloseColumnRowGroup* pGroups)
{
    sal_Int32 nDepth = pFields->GetDepth();
    for(sal_Int32 i = 0; i < nDepth; i++)
    {
        sal_Int32 nFields = pFields->GetCount(static_cast<USHORT>(i));
        for (sal_Int32 j = 0; j < nFields; j++)
        {
            ScMyColumnRowGroup aGroup;
            ScOutlineEntry* pEntry = pFields->GetEntry(static_cast<USHORT>(i), static_cast<USHORT>(j));
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
        ScOutlineTable* pOutlineTable = pDoc->GetOutlineTable( nCurrentTable, sal_False );
        if(pOutlineTable)
        {
            ScOutlineArray* pCols = pOutlineTable->GetColArray();
            ScOutlineArray* pRows = pOutlineTable->GetRowArray();
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
        ::rtl::OUStringBuffer aBuffer;
        SvXMLUnitConverter::encodeBase64(aBuffer, pDoc->GetDocPassword());
        if (aBuffer.getLength())
            AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY, aBuffer.makeStringAndClear());
    }
}

void ScXMLExport::_ExportContent()
{
    if (!pSharedData)
    {
        sal_Int32 nTableCount(0);
        sal_Int32 nShapesCount(0);
        sal_Int32 nCellCount(pDoc ? pDoc->GetCellCount() : 0);
        CollectSharedData(nTableCount, nShapesCount, nCellCount);
        DBG_ERROR("no shared data setted");
    }
    ScXMLExportDatabaseRanges aExportDatabaseRanges(*this);
    if (GetModel().is())
    {
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( GetModel(), uno::UNO_QUERY );
        if ( xSpreadDoc.is() )
        {
            uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
            uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
            if ( xIndex.is() )
            {
                //_GetNamespaceMap().ClearQNamesCache();
                pChangeTrackingExportHelper->CollectAndWriteChanges();
                WriteCalculationSettings(xSpreadDoc);
                sal_Int32 nTableCount = xIndex->getCount();
                ScMyAreaLinksContainer aAreaLinks;
                GetAreaLinks( xSpreadDoc, aAreaLinks );
                ScMyEmptyDatabaseRangesContainer aEmptyRanges = aExportDatabaseRanges.GetEmptyDatabaseRanges();
                ScMyDetectiveOpContainer aDetectiveOpContainer;
                GetDetectiveOpList( aDetectiveOpContainer );

                pCellStyles->Sort();
                pMergedRangesContainer->Sort();
                pSharedData->GetDetectiveObjContainer()->Sort();

                pCellsItr->Clear();
                pCellsItr->SetShapes( pSharedData->GetShapesContainer() );
                pCellsItr->SetMergedRanges( pMergedRangesContainer );
                pCellsItr->SetAreaLinks( &aAreaLinks );
                pCellsItr->SetEmptyDatabaseRanges( &aEmptyRanges );
                pCellsItr->SetDetectiveObj( pSharedData->GetDetectiveObjContainer() );
                pCellsItr->SetDetectiveOp( &aDetectiveOpContainer );

                if (nTableCount > 0)
                    pValidationsContainer->WriteValidations(*this);
                WriteTheLabelRanges( xSpreadDoc );
                for (sal_uInt16 nTable = 0; nTable < nTableCount; nTable++)
                {
                    uno::Any aTable = xIndex->getByIndex(nTable);
                    uno::Reference<sheet::XSpreadsheet> xTable;
                    if (aTable>>=xTable)
                    {
                        xCurrentTable = xTable;
                        uno::Reference<container::XNamed> xName (xTable, uno::UNO_QUERY );
                        if ( xName.is() )
                        {
                            nCurrentTable = nTable;
                            ::rtl::OUString sOUTableName = xName->getName();
                            AddAttribute(sAttrName, sOUTableName);
                            AddAttribute(sAttrStyleName, aTableStyles[nTable]);
                            uno::Reference<util::XProtectable> xProtectable (xTable, uno::UNO_QUERY);
                            if (xProtectable.is())
                                if (xProtectable->isProtected())
                                {
                                    AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTED, XML_TRUE);
                                    ::rtl::OUStringBuffer aBuffer;
                                    if (pDoc)
                                        SvXMLUnitConverter::encodeBase64(aBuffer, pDoc->GetTabPassword(nTable));
                                    if (aBuffer.getLength())
                                        AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY, aBuffer.makeStringAndClear());
                                }
                            ::rtl::OUString sPrintRanges( GetPrintRanges() );
                            if( sPrintRanges.getLength() )
                                AddAttribute( XML_NAMESPACE_TABLE, XML_PRINT_RANGES, sPrintRanges );
                            SvXMLElementExport aElemT(*this, sElemTab, sal_True, sal_True);
                            CheckAttrList();
                            WriteTableSource();
                            WriteScenario();
                            uno::Reference<drawing::XDrawPage> xDrawPage;
                            if (pSharedData->HasForm(nTable, xDrawPage) && xDrawPage.is())
                            {
                                ::binfilter::xmloff::OOfficeFormsExport aForms(*this);
                                GetFormExport()->exportForms( xDrawPage );
                                sal_Bool bRet = GetFormExport()->seekPage( xDrawPage );
                                DBG_ASSERT( bRet, "OFormLayerXMLExport::seekPage failed!" );
                            }
                            if (pSharedData->HasDrawPage())
                            {
                                GetShapeExport()->seekShapes(uno::Reference<drawing::XShapes>(pSharedData->GetDrawPage(nTable), uno::UNO_QUERY));
                                WriteTableShapes();
                            }
                            table::CellRangeAddress aRange = GetEndAddress(xTable, nTable);
                            pSharedData->SetLastColumn(nTable, aRange.EndColumn);
                            pSharedData->SetLastRow(nTable, aRange.EndRow);
                            pCellsItr->SetCurrentTable(nTable, xCurrentTable);
                            pGroupColumns->NewTable();
                            pGroupRows->NewTable();
                            FillColumnRowGroups();
                            table::CellRangeAddress aColumnHeaderRange;
                            sal_Bool bHasColumnHeader(GetColumnHeader(aColumnHeaderRange));
                            if (bHasColumnHeader)
                                pSharedData->SetLastColumn(nTable, aColumnHeaderRange.EndColumn);
                            bHasRowHeader = GetRowHeader(aRowHeaderRange);
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
                                            nEqualCells++;
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
                    GetProgressBarHelper()->Increment();
                }
            }
            WriteNamedExpressions(xSpreadDoc);
            aExportDatabaseRanges.WriteDatabaseRanges(xSpreadDoc);
            ScXMLExportDataPilot aExportDataPilot(*this);
            aExportDataPilot.WriteDataPilots(xSpreadDoc);
            WriteConsolidation();
            ScXMLExportDDELinks aExportDDELinks(*this);
            aExportDDELinks.WriteDDELinks(xSpreadDoc);
            GetProgressBarHelper()->SetValue(GetProgressBarHelper()->GetReference());
        }
    }
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
    ScXMLStyleExport aStylesExp(*this, ::rtl::OUString(), GetAutoStylePool().get());
    if (GetModel().is())
    {
        uno::Reference <lang::XMultiServiceFactory> xMultiServiceFactory(GetModel(), uno::UNO_QUERY);
        if (xMultiServiceFactory.is())
        {
            uno::Reference <uno::XInterface> xInterface = xMultiServiceFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.Defaults")));
            uno::Reference <beans::XPropertySet> xProperties(xInterface, uno::UNO_QUERY);
            if (xProperties.is())
                aStylesExp.exportDefaultStyle(xProperties, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME)), xCellStylesExportPropertySetMapper);
            if (pSharedData->HasShapes())
            {
                GetShapeExport()->ExportGraphicDefaults();
/*				xInterface = xMultiServiceFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Defaults")));
                uno::Reference <beans::XPropertySet> xDrawProperties(xInterface, uno::UNO_QUERY);
                if (xDrawProperties.is())
                    aStylesExp.exportDefaultStyle(xDrawProperties, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_GRAPHICS_NAME)), GetShapeExport()->CreateShapePropMapper(*this));*/
            }
        }
        uno::Reference <style::XStyleFamiliesSupplier> xStyleFamiliesSupplier (GetModel(), uno::UNO_QUERY);
        if (xStyleFamiliesSupplier.is())
        {
            uno::Reference <container::XNameAccess> aStylesFamilies = xStyleFamiliesSupplier->getStyleFamilies();
            if (aStylesFamilies.is())
            {
                uno::Any aStyleFamily = aStylesFamilies->getByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CellStyles")));
                uno::Reference <container::XIndexAccess> aCellStyles;
                if (aStyleFamily >>= aCellStyles)
                {
                    sal_Int32 nCount = aCellStyles->getCount();
                    ::rtl::OUString sNumberFormat(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_NUMFMT));
                    for (sal_Int32 i = 0; i < nCount; i++)
                    {
                        uno::Any aCellStyle = aCellStyles->getByIndex(i);
                        uno::Reference <beans::XPropertySet> xCellProperties;
                        if (aCellStyle >>= xCellProperties)
                        {
                            uno::Any aNumberFormat = xCellProperties->getPropertyValue(sNumberFormat);
                            sal_Int32 nNumberFormat;
                            if (aNumberFormat >>= nNumberFormat)
                                addDataStyle(nNumberFormat);
                        }
                    }
                }
            }
        }
    }
    exportDataStyles();

    aStylesExp.exportStyleFamily(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CellStyles")),
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME)), xCellStylesExportPropertySetMapper, FALSE, XML_STYLE_FAMILY_TABLE_CELL);

    SvXMLExport::_ExportStyles(bUsed);
}

void ScXMLExport::_ExportAutoStyles()
{
    if (GetModel().is())
    {
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( GetModel(), uno::UNO_QUERY );
        if ( xSpreadDoc.is() )
        {
            uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
            uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
            if ( xIndex.is() )
            {
                if (getExportFlags() & EXPORT_CONTENT)
                {
                    if (!pSharedData)
                    {
                        sal_Int32 nTableCount(0);
                        sal_Int32 nShapesCount(0);
                        sal_Int32 nCellCount(pDoc ? pDoc->GetCellCount() : 0);
                        CollectSharedData(nTableCount, nShapesCount, nCellCount);
                        //DBG_ERROR("no shared data setted");
                    }
                    ::rtl::OUString SC_SCOLUMNPREFIX(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX));
                    ::rtl::OUString SC_SROWPREFIX(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_ROW_STYLES_PREFIX));
                    ::rtl::OUString SC_SCELLPREFIX(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX));
                    ::rtl::OUString SC_NUMBERFORMAT(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_NUMFMT));
                    sal_Int32 nTableCount = xIndex->getCount();
                    pCellStyles->AddNewTable(nTableCount - 1);
                    CollectShapesAutoStyles(nTableCount);
                    for (sal_uInt16 nTable = 0; nTable < nTableCount; nTable++)
                    {
                        uno::Any aTable = xIndex->getByIndex(nTable);
                        uno::Reference<sheet::XSpreadsheet> xTable;
                        if (aTable>>=xTable)
                        {
                            uno::Reference<beans::XPropertySet> xTableProperties(xTable, uno::UNO_QUERY);
                            if (xTableProperties.is())
                            {
                                std::vector<XMLPropertyState> xPropStates = xTableStylesExportPropertySetMapper->Filter(xTableProperties);
                                if(xPropStates.size())
                                {
                                    ::rtl::OUString sParent;
                                    ::rtl::OUString sName;
                                    GetAutoStylePool()->Add(sName, XML_STYLE_FAMILY_TABLE_TABLE, sParent, xPropStates);
                                    aTableStyles.push_back(sName);
                                }
                            }
                            uno::Reference<sheet::XUniqueCellFormatRangesSupplier> xCellFormatRanges ( xTable, uno::UNO_QUERY );
                            if ( xCellFormatRanges.is() )
                            {
                                uno::Reference<container::XIndexAccess> xFormatRangesIndex = xCellFormatRanges->getUniqueCellFormatRanges();
                                if (xFormatRangesIndex.is())
                                {
                                    sal_Int32 nFormatRangesCount = xFormatRangesIndex->getCount();
                                    GetProgressBarHelper()->ChangeReference(GetProgressBarHelper()->GetReference() + nFormatRangesCount);
                                    for (sal_Int32 nFormatRange = 0; nFormatRange < nFormatRangesCount; nFormatRange++)
                                    {
                                        uno::Any aFormatRange = xFormatRangesIndex->getByIndex(nFormatRange);
                                        uno::Reference< sheet::XSheetCellRanges> xCellRanges;
                                        if (aFormatRange >>= xCellRanges)
                                        {
                                            uno::Reference <beans::XPropertySet> xProperties (xCellRanges, uno::UNO_QUERY);
                                            if (xProperties.is())
                                            {
                                                ::rtl::OUString sStyleName;
                                                sal_Int32 nNumberFormat(-1);
                                                sal_Int32 nValidationIndex(-1);
                                                std::vector< XMLPropertyState > xPropStates = xCellStylesExportPropertySetMapper->Filter( xProperties );
                                                std::vector< XMLPropertyState >::iterator aItr = xPropStates.begin();
                                                sal_Int32 nCount(0);
                                                while (aItr != xPropStates.end())
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
                                                            }
                                                            break;
                                                            case CTF_SC_CELLSTYLE :
                                                            {
                                                                aItr->maValue >>= sStyleName;
                                                                aItr->mnIndex = -1;
                                                                aItr++;
                                                                nCount++;
                                                            }
                                                            break;
                                                            case CTF_SC_NUMBERFORMAT :
                                                            {
                                                                if (aItr->maValue >>= nNumberFormat)
                                                                    addDataStyle(nNumberFormat);
                                                                aItr++;
                                                                nCount++;
                                                            }
                                                            break;
                                                            default:
                                                            {
                                                                aItr++;
                                                                nCount++;
                                                            }
                                                            break;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        aItr++;
                                                        nCount++;
                                                    }
                                                }
                                                if (nCount == 1) // this is the CellStyle and should be removed if alone
                                                    xPropStates.clear();
                                                if (nNumberFormat == -1)
                                                {
                                                    uno::Any aAny = xProperties->getPropertyValue(SC_NUMBERFORMAT);
                                                    aAny >>= nNumberFormat;
                                                }
                                                if (sStyleName.getLength())
                                                {
                                                    if (xPropStates.size())
                                                    {
                                                        sal_Int32 nIndex;
                                                        ::rtl::OUString sName;
                                                        sal_Bool bIsAutoStyle = sal_True;
                                                        if (GetAutoStylePool()->Add(sName, XML_STYLE_FAMILY_TABLE_CELL, sStyleName, xPropStates))
                                                        {
                                                            ::rtl::OUString* pTemp = new ::rtl::OUString(sName);
                                                            if (!pCellStyles->AddStyleName(pTemp, nIndex))
                                                                delete pTemp;
                                                        }
                                                        else
                                                            nIndex = pCellStyles->GetIndexOfStyleName(sName, SC_SCELLPREFIX, bIsAutoStyle);
                                                        uno::Sequence<table::CellRangeAddress> aAddresses = xCellRanges->getRangeAddresses();
                                                        table::CellRangeAddress* pAddresses = aAddresses.getArray();
                                                        sal_Bool bGetMerge(sal_True);
                                                        for (sal_Int32 i = 0; i < aAddresses.getLength(); i++, pAddresses++)
                                                        {
                                                            pSharedData->SetLastColumn(nTable, pAddresses->EndColumn);
                                                            pSharedData->SetLastRow(nTable, pAddresses->EndRow);
                                                            pCellStyles->AddRangeStyleName(*pAddresses, nIndex, bIsAutoStyle, nValidationIndex, nNumberFormat);
                                                            if (bGetMerge)
                                                                bGetMerge = GetMerged(pAddresses, xTable);
                                                        }
                                                    }
                                                    else
                                                    {
                                                        ::rtl::OUString* pTemp = new ::rtl::OUString(sStyleName);
                                                        sal_Int32 nIndex(0);
                                                        if (!pCellStyles->AddStyleName(pTemp, nIndex, sal_False))
                                                        {
                                                            delete pTemp;
                                                            pTemp = NULL;
                                                        }
                                                        uno::Sequence<table::CellRangeAddress> aAddresses = xCellRanges->getRangeAddresses();
                                                        table::CellRangeAddress* pAddresses = aAddresses.getArray();
                                                        sal_Bool bGetMerge(sal_True);
                                                        for (sal_Int32 i = 0; i < aAddresses.getLength(); i++, pAddresses++)
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
                                                GetProgressBarHelper()->Increment();
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
                                    uno::Reference<table::XTableColumns> xTableColumns = xColumnRowRange->getColumns();
                                    if (xTableColumns.is())
                                    {
                                        sal_Int32 nColumns = pDoc->GetLastChangedCol(nTable);
                                        pSharedData->SetLastColumn(nTable, nColumns);
                                        table::CellRangeAddress aCellAddress = GetEndAddress(xTable, nTable);
                                        if (aCellAddress.EndColumn > nColumns)
                                        {
                                            nColumns++;
                                            pColumnStyles->AddNewTable(nTable, aCellAddress.EndColumn);
                                        }
        //								else if (nColumns < MAXCOL)
        //									pColumnStyles->AddNewTable(nTable, ++nColumns);
                                        else
                                            pColumnStyles->AddNewTable(nTable, nColumns);
                                        sal_Int32 nColumn = 0;
                                        while (/*nColumn <= nColumns && */nColumn <= MAXCOL)
                                        {
                                            sal_Int32 nIndex(-1);
                                            sal_Bool bIsVisible(sal_True);
                                            uno::Any aColumn = xTableColumns->getByIndex(nColumn);
                                            uno::Reference<table::XCellRange> xTableColumn;
                                            if (aColumn >>= xTableColumn)
                                            {
                                                uno::Reference <beans::XPropertySet> xColumnProperties(xTableColumn, uno::UNO_QUERY);
                                                if (xColumnProperties.is())
                                                {
                                                    std::vector<XMLPropertyState> xPropStates = xColumnStylesExportPropertySetMapper->Filter(xColumnProperties);
                                                    if(xPropStates.size())
                                                    {
                                                        std::vector< XMLPropertyState >::iterator aItr = xPropStates.begin();
                                                        while (aItr != xPropStates.end())
                                                        {
                                                            if (xColumnStylesPropertySetMapper->GetEntryContextId(aItr->mnIndex) == CTF_SC_ISVISIBLE)
                                                            {
                                                                aItr->maValue >>= bIsVisible;
                                                                break;
                                                            }
                                                            aItr++;
                                                        }
                                                        ::rtl::OUString sParent;
                                                        ::rtl::OUString sName;
                                                        if (GetAutoStylePool()->Add(sName, XML_STYLE_FAMILY_TABLE_COLUMN, sParent, xPropStates))
                                                        {
                                                            ::rtl::OUString* pTemp = new ::rtl::OUString(sName);
                                                            nIndex = pColumnStyles->AddStyleName(pTemp);
                                                        }
                                                        else
                                                            nIndex = pColumnStyles->GetIndexOfStyleName(sName, SC_SCOLUMNPREFIX);
                                                        pColumnStyles->AddFieldStyleName(nTable, nColumn, nIndex, bIsVisible);
                                                    }
                                                }
                                            }
                                            sal_Int32 nOld = nColumn;
                                            nColumn = pDoc->GetNextDifferentChangedCol(nTable, static_cast<USHORT>(nColumn));
                                            if (nColumn == MAXCOL)
                                                nColumn++;
                                            for (sal_Int32 i = nOld + 1; i < nColumn; i++)
                                                pColumnStyles->AddFieldStyleName(nTable, i, nIndex, bIsVisible);
                                        }
                                        if (aCellAddress.EndColumn > nColumns)
                                        {
                                            sal_Bool bIsVisible(sal_True);
                                            sal_Int32 nIndex = pColumnStyles->GetStyleNameIndex(nTable, nColumns, bIsVisible);
                                            for (sal_Int32 i = nColumns + 1; i <= aCellAddress.EndColumn; i++)
                                                pColumnStyles->AddFieldStyleName(nTable, i, nIndex, bIsVisible);
                                        }
                                    }
                                    uno::Reference<table::XTableRows> xTableRows = xColumnRowRange->getRows();
                                    if (xTableRows.is())
                                    {
                                        sal_Int32 nRows = pDoc->GetLastChangedRow(nTable);
                                        pSharedData->SetLastRow(nTable, nRows);
                                        table::CellRangeAddress aCellAddress = GetEndAddress(xTable, nTable);
                                        if (aCellAddress.EndRow > nRows)
                                        {
                                            nRows++;
                                            pRowStyles->AddNewTable(nTable, aCellAddress.EndRow);
                                        }
        //								else if (nRows < MAXROW)
        //									pRowStyles->AddNewTable(nTable, ++nRows);
                                        else
                                            pRowStyles->AddNewTable(nTable, nRows);
                                        sal_Int32 nRow = 0;
                                        while ( /*nRow <= nRows && */nRow <= MAXROW)
                                        {
                                            sal_Int32 nIndex;
                                            uno::Any aRow = xTableRows->getByIndex(nRow);
                                            uno::Reference<table::XCellRange> xTableRow;
                                            if (aRow >>= xTableRow)
                                            {
                                                uno::Reference <beans::XPropertySet> xRowProperties(xTableRow, uno::UNO_QUERY);
                                                if(xRowProperties.is())
                                                {
                                                    std::vector<XMLPropertyState> xPropStates = xRowStylesExportPropertySetMapper->Filter(xRowProperties);
                                                    if(xPropStates.size())
                                                    {
                                                        ::rtl::OUString sParent;
                                                        ::rtl::OUString sName;
                                                        if (GetAutoStylePool()->Add(sName, XML_STYLE_FAMILY_TABLE_ROW, sParent, xPropStates))
                                                        {
                                                            ::rtl::OUString* pTemp = new ::rtl::OUString(sName);
                                                            nIndex = pRowStyles->AddStyleName(pTemp);
                                                        }
                                                        else
                                                            nIndex = pRowStyles->GetIndexOfStyleName(sName, SC_SROWPREFIX);
                                                        pRowStyles->AddFieldStyleName(nTable, nRow, nIndex);
                                                    }
                                                }
                                            }
                                            sal_Int32 nOld = nRow;
                                            nRow = pDoc->GetNextDifferentChangedRow(nTable, static_cast<USHORT>(nRow), false);
                                            if (nRow == MAXROW)
                                                nRow++;
                                            for (sal_Int32 i = nOld + 1; i < nRow; i++)
                                                pRowStyles->AddFieldStyleName(nTable, i, nIndex);
                                        }
                                        if (aCellAddress.EndRow > nRows)
                                        {
                                            sal_Int32 nIndex = pRowStyles->GetStyleNameIndex(nTable, nRows);
                                            for (sal_Int32 i = nRows + 1; i <= aCellAddress.EndRow; i++)
                                                pRowStyles->AddFieldStyleName(nTable, i, nIndex);
                                        }
                                    }
                                }
                            }
                            uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery (xTable, uno::UNO_QUERY);
                            if (xCellRangesQuery.is())
                            {
                                uno::Reference<sheet::XSheetCellRanges> xSheetCellRanges = xCellRangesQuery->queryContentCells(sheet::CellFlags::STRING);
                                uno::Reference<sheet::XSheetOperation> xSheetOperation(xSheetCellRanges, uno::UNO_QUERY);
                                if (xSheetCellRanges.is() && xSheetOperation.is())
                                {
                                    sal_uInt32 nCount(sal_uInt32(xSheetOperation->computeFunction(sheet::GeneralFunction_COUNT)));
                                    uno::Reference<container::XEnumerationAccess> xCellsAccess = xSheetCellRanges->getCells();
                                    if (xCellsAccess.is())
                                    {
                                        GetProgressBarHelper()->ChangeReference(GetProgressBarHelper()->GetReference() + nCount);
                                        uno::Reference<container::XEnumeration>xCells = xCellsAccess->createEnumeration();
                                        if (xCells.is())
                                        {
                                            sal_uInt32 nCount2(0);
                                            while (xCells->hasMoreElements())
                                            {
                                                uno::Any aCell = xCells->nextElement();
                                                uno::Reference<table::XCell> xCell;
                                                if (aCell >>= xCell)
                                                {
                                                    if (IsEditCell(xCell))
                                                    {
                                                        uno::Reference<text::XText> xText(xCell, uno::UNO_QUERY);
                                                        if (xText.is())
                                                            GetTextParagraphExport()->collectTextAutoStyles(xText, sal_False, sal_False);
                                                    }
                                                }
                                                nCount2++;
                                                GetProgressBarHelper()->Increment();
                                            }
                                            if(nCount2 > nCount)
                                                GetProgressBarHelper()->SetReference(GetProgressBarHelper()->GetReference() + nCount2 - nCount);
                                        }
                                    }
                                }
                            }
                        }
                        GetProgressBarHelper()->Increment();
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
                    GetTextParagraphExport()->exportTextAutoStyles();

                    GetShapeExport()->exportAutoStyles();
                    GetFormExport()->exportAutoStyles( );
                }
                if ((getExportFlags() & EXPORT_ALL) == EXPORT_ALL)
                    GetChartExport()->exportAutoStyles();
                if (getExportFlags() & EXPORT_MASTERSTYLES)
                {
                    GetPageExport()->collectAutoStyles(sal_True);
                    GetTextParagraphExport()->exportTextAutoStyles();
                      GetPageExport()->exportAutoStyles();
                }
            }
        }
    }
}

void ScXMLExport::_ExportMasterStyles()
{
    GetPageExport()->exportMasterStyles( sal_True );
}

void ScXMLExport::CollectInternalShape( uno::Reference< drawing::XShape > xShape )
{
    // detective objects
    SvxShape* pShapeImp = SvxShape::getImplementation( xShape );
    if( pShapeImp )
    {
        SdrObject *pObject = pShapeImp->GetSdrObject();
        if( pObject )
        {
            ScDetectiveFunc	aDetFunc( pDoc, nCurrentTable );
            ScAddress		aPosition;
            ScRange			aSourceRange;
            sal_Bool		bRedLine;
            ScDetectiveObjType eObjType = aDetFunc.GetDetectiveObjectType(
                pObject, aPosition, aSourceRange, bRedLine );
            pSharedData->GetDetectiveObjContainer()->AddObject( eObjType, aPosition, aSourceRange, bRedLine );
        }
    }
}

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
        uno::Reference<table::XCellRange> xCellRange = xTable->getCellRangeByPosition(
            nCol, nRow, nCol, nRow);
        if (xCellRange.is())
        {
            uno::Reference<sheet::XSheetCellRange> xSheetCellRange(xCellRange, uno::UNO_QUERY);
            if (xSheetCellRange.is())
            {
                uno::Reference<sheet::XSheetCellCursor> xCursor = xTable->createCursorByRange(xSheetCellRange);
                if(xCursor.is())
                {
                    uno::Reference<sheet::XCellRangeAddressable> xCellAddress (xCursor, uno::UNO_QUERY);
                    xCursor->collapseToMergedArea();
                    table::CellRangeAddress aCellAddress2 = xCellAddress->getRangeAddress();
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
        }
        if (!bReady)
        {
            if (bRowInc)
                nRow++;
            else
                nCol++;
        }
    }
    DBG_ASSERT(!(!bReady && nEndRow > nRow && nEndCol > nCol), "should not be possible");
    return !bReady;
}

sal_Bool ScXMLExport::IsMatrix (const uno::Reference <table::XCell>& xCell,
                            const uno::Reference <sheet::XSpreadsheet>& xTable,
                            const sal_Int32 nCol, const sal_Int32 nRow,
                            table::CellRangeAddress& aCellAddress, sal_Bool& bIsFirst) const
{
    bIsFirst = sal_False;
    uno::Reference <sheet::XArrayFormulaRange> xArrayFormulaRange (xCell, uno::UNO_QUERY);
    if (xArrayFormulaRange.is())
    {
        ::rtl::OUString sArrayFormula = xArrayFormulaRange->getArrayFormula();
        if (sArrayFormula.getLength())
        {
            uno::Reference<sheet::XSheetCellRange> xMatrixSheetCellRange (xCell, uno::UNO_QUERY);
            if (xMatrixSheetCellRange.is())
            {
                uno::Reference<sheet::XSheetCellCursor> xMatrixSheetCursor = xTable->createCursorByRange(xMatrixSheetCellRange);
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
    return sal_False;
}

sal_Bool ScXMLExport::GetCellText (ScMyCell& rMyCell) const
{
    if (rMyCell.bHasStringValue)
        return sal_True;
    else
    {
        if (!rMyCell.bHasXText)
        {
            rMyCell.xText = uno::Reference <text::XText>(rMyCell.xCell, uno::UNO_QUERY);
            rMyCell.bHasXText = sal_True;
        }
        if (rMyCell.xText.is())
        {
            rMyCell.sStringValue = rMyCell.xText->getString();
            rMyCell.bHasStringValue = sal_True;
            return sal_True;
        }
    }
    return sal_False;
}

OUString ScXMLExport::GetPrintRanges()
{
    ::rtl::OUString sPrintRanges;
    uno::Reference< sheet::XPrintAreas > xPrintAreas( xCurrentTable, uno::UNO_QUERY );
    if( xPrintAreas.is() )
    {
        uno::Sequence< table::CellRangeAddress > aRangeList( xPrintAreas->getPrintAreas() );
        ScXMLConverter::GetStringFromRangeList( sPrintRanges, aRangeList, pDoc );
    }
    return sPrintRanges;
}

void ScXMLExport::WriteCell (ScMyCell& aCell)
{
    if (aCell.nStyleIndex != -1)
        AddAttribute(sAttrStyleName, *pCellStyles->GetStyleNameByIndex(aCell.nStyleIndex, aCell.bIsAutoStyle));
    if (aCell.nValidationIndex > -1)
        AddAttribute(XML_NAMESPACE_TABLE, XML_CONTENT_VALIDATION_NAME, pValidationsContainer->GetValidationName(aCell.nValidationIndex));
    sal_Bool bIsMatrix(aCell.bIsMatrixBase || aCell.bIsMatrixCovered);
    sal_Bool bIsFirstMatrixCell(aCell.bIsMatrixBase);
    if (bIsFirstMatrixCell)
    {
        sal_Int32 nColumns = aCell.aMatrixRange.EndColumn - aCell.aMatrixRange.StartColumn + 1;
        sal_Int32 nRows = aCell.aMatrixRange.EndRow - aCell.aMatrixRange.StartRow + 1;
        ::rtl::OUStringBuffer sColumns;
        ::rtl::OUStringBuffer sRows;
        SvXMLUnitConverter::convertNumber(sColumns, nColumns);
        SvXMLUnitConverter::convertNumber(sRows, nRows);
        AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_MATRIX_COLUMNS_SPANNED, sColumns.makeStringAndClear());
        AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_MATRIX_ROWS_SPANNED, sRows.makeStringAndClear());
    }
    sal_Bool bIsEmpty = sal_False;
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
                aCell.fValue = aCell.xCell->getValue();
                aCell.bHasDoubleValue = sal_True;
            }
            GetNumberFormatAttributesExportHelper()->SetNumberFormatAttributes(
                aCell.nNumberFormat, aCell.fValue);
        }
        break;
    case table::CellContentType_TEXT :
        {
            if (GetCellText(aCell))
            {
                ::rtl::OUString sFormula(aCell.xCell->getFormula());
                if (sFormula[0] == '\'')
                    sFormula = sFormula.copy(1);
                GetNumberFormatAttributesExportHelper()->SetNumberFormatAttributes(
                    sFormula, aCell.sStringValue, sal_True, sal_False);
            }
        }
        break;
    case table::CellContentType_FORMULA :
        {
            ScAddress aCoreAddress(static_cast<sal_uInt16>(aCell.aCellAddress.Column),
                                static_cast<sal_uInt16>(aCell.aCellAddress.Row),
                                static_cast<sal_uInt16>(aCell.aCellAddress.Sheet));
            ScBaseCell* pBaseCell = GetDocument() ? GetDocument()->GetCell(aCoreAddress) : NULL;
            if (pBaseCell && pBaseCell->GetCellType() == CELLTYPE_FORMULA)
            {
                ::rtl::OUStringBuffer sFormula;
                ScFormulaCell* pFormulaCell = (ScFormulaCell*) pBaseCell;
                if (!bIsMatrix || (bIsMatrix && bIsFirstMatrixCell))
                {
                    pFormulaCell->GetEnglishFormula(sFormula, sal_True);
                    ::rtl::OUString sOUFormula(sFormula.makeStringAndClear());
                    if (!bIsMatrix)
                        AddAttribute(sAttrFormula, sOUFormula);
                    else
                    {
                        ::rtl::OUString sMatrixFormula = sOUFormula.copy(1, sOUFormula.getLength() - 2);
                        AddAttribute(sAttrFormula, sMatrixFormula);
                    }
                }
                if (pFormulaCell->IsValue())
                {
                    sal_Bool bIsStandard;
                    ::rtl::OUString sCurrency;
                    GetNumberFormatAttributesExportHelper()->GetCellType(aCell.nNumberFormat, sCurrency, bIsStandard);
                    if (bIsStandard)
                    {
                        if (pDoc)
                            GetNumberFormatAttributesExportHelper()->SetNumberFormatAttributes(
                                pFormulaCell->GetStandardFormat(*pDoc->GetFormatTable(), 0),
                                aCell.xCell->getValue());
                    }
                    else
                        GetNumberFormatAttributesExportHelper()->SetNumberFormatAttributes(
                            aCell.nNumberFormat, aCell.xCell->getValue());
                }
                else
                {
                    if (GetCellText(aCell))
                        if (aCell.sStringValue.getLength())
                            AddAttribute(sAttrStringValue, aCell.sStringValue);
                }
            }
        }
        break;
    }
    ::rtl::OUString* pCellString = &sElemCell;
    if (aCell.bIsCovered)
    {
        pCellString = &sElemCoveredCell;
    }
    else
    {
        if (aCell.bIsMergedBase)
        {
            sal_Int32 nColumns = aCell.aMergeRange.EndColumn - aCell.aMergeRange.StartColumn + 1;
            sal_Int32 nRows = aCell.aMergeRange.EndRow - aCell.aMergeRange.StartRow + 1;
            ::rtl::OUStringBuffer sColumns;
            ::rtl::OUStringBuffer sRows;
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
    if (!bIsEmpty)
    {
        if ((aCell.nType == table::CellContentType_TEXT) && IsEditCell(aCell))
        {
            if (!aCell.bHasXText)
            {
                aCell.xText = uno::Reference<text::XText>(aCell.xCell, uno::UNO_QUERY);
                aCell.bHasXText = sal_True;
            }
            if ( aCell.xText.is())
                GetTextParagraphExport()->exportText(aCell.xText, sal_False, sal_False);
        }
        else
        {
            SvXMLElementExport aElemC(*this, sElemP, sal_True, sal_False);
            sal_Bool bPrevCharWasSpace(sal_True);
              if (GetCellText(aCell))
                GetTextParagraphExport()->exportText(aCell.sStringValue, bPrevCharWasSpace);
        }
    }
    WriteShapes(aCell);
    if (!bIsEmpty)
        GetProgressBarHelper()->Increment();
}

void ScXMLExport::ExportShape(const uno::Reference < drawing::XShape >& xShape, awt::Point* pPoint)
{
    uno::Reference < beans::XPropertySet > xShapeProps ( xShape, uno::UNO_QUERY );
    sal_Bool bMemChart(sal_False);
    ::rtl::OUString sPropCLSID (RTL_CONSTASCII_USTRINGPARAM("CLSID"));
    ::rtl::OUString sPersistName (RTL_CONSTASCII_USTRINGPARAM("PersistName"));
    if (xShapeProps.is())
    {
        uno::Any aAny = xShapeProps->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ZOrder")));
        sal_Int32 nZOrder;
        if (aAny >>= nZOrder)
        {
            ::rtl::OUStringBuffer sBuffer;
            GetMM100UnitConverter().convertNumber(sBuffer, nZOrder);
            AddAttribute(XML_NAMESPACE_DRAW, XML_ZINDEX, sBuffer.makeStringAndClear());
        }
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo = xShapeProps->getPropertySetInfo();
        if( xPropSetInfo->hasPropertyByName( sPropCLSID ) )
        {
            uno::Any aAny = xShapeProps->getPropertyValue( sPropCLSID );
            ::rtl::OUString sCLSID;
            if (aAny >>= sCLSID)
            {
                if ( sCLSID.equalsIgnoreAsciiCase(GetChartExport()->getChartCLSID()) )
                {
                    aAny = xShapeProps->getPropertyValue(sPersistName);
                    ::rtl::OUString sOUName;
                    aAny >>= sOUName;
                    String sName(sOUName);
                    if (!pChartListener)
                    {
                        String aEmptyString;
                        ScRange aRange;
                        pChartListener = new ScChartListener ( aEmptyString, GetDocument(), aRange );
                    }
                    if(pChartListener)
                    {
                        USHORT nIndex(0);
                        pChartListener->SetString( sName );
                        if ( GetDocument() && GetDocument()->GetChartListenerCollection()->Search( pChartListener, nIndex ) )
                        {
                            const ScRangeListRef& rRangeListRef = ((ScChartListener*)
                                (GetDocument()->GetChartListenerCollection()->
                                At( nIndex )))->GetRangeList();
                            if (rRangeListRef.Is())
                            {
                                bMemChart = sal_True;
                                ::rtl::OUString sRanges;
                                ScXMLConverter::GetStringFromRangeList(sRanges, rRangeListRef, GetDocument());
                                if (sRanges.getLength())
                                    AddAttribute(XML_NAMESPACE_DRAW, XML_NOTIFY_ON_UPDATE_OF_RANGES, sRanges);
                                GetShapeExport()->exportShape(xShape, SEF_EXPORT_NO_CHART_DATA | SEF_DEFAULT, pPoint);
                            }
                        }
                        else
                        {
                            bMemChart = sal_True;
                            AddAttribute(XML_NAMESPACE_DRAW, XML_NOTIFY_ON_UPDATE_OF_RANGES, ::rtl::OUString());
                            GetShapeExport()->exportShape(xShape, SEF_EXPORT_NO_CHART_DATA | SEF_DEFAULT, pPoint);
                        }
                    }
/*					SchMemChart* pMemChart = pDoc->FindChartData(sName);
                    if (pMemChart && pMemChart->GetSeriesAddresses().getLength())
                    {
                        bMemChart = sal_True;
                        ::rtl::OUString sRanges(pMemChart->getXMLStringForChartRange());
                        if (sRanges.getLength())
                            AddAttribute(XML_NAMESPACE_DRAW, XML_NOTIFY_ON_UPDATE_OF_RANGES, sRanges);
                        GetShapeExport()->exportShape(xShape, SEF_EXPORT_NO_CHART_DATA | SEF_DEFAULT, pPoint);
                    }*/
                }
            }
        }
    }
    if (!bMemChart)
        GetShapeExport()->exportShape(xShape, SEF_DEFAULT, pPoint);
    GetProgressBarHelper()->Increment();
}

void ScXMLExport::WriteShapes(const ScMyCell& rMyCell)
{
    if( rMyCell.bHasShape && !rMyCell.aShapeList.empty() && pDoc )
    {
        awt::Point aPoint;
        Rectangle aRec = pDoc->GetMMRect(static_cast<USHORT>(rMyCell.aCellAddress.Column), static_cast<USHORT>(rMyCell.aCellAddress.Row),
            static_cast<USHORT>(rMyCell.aCellAddress.Column), static_cast<USHORT>(rMyCell.aCellAddress.Row), rMyCell.aCellAddress.Sheet);
        aPoint.X = aRec.Left();
        aPoint.Y = aRec.Top();
        awt::Point* pPoint = &aPoint;
        ScMyShapeList::const_iterator aItr = rMyCell.aShapeList.begin();
        while (aItr != rMyCell.aShapeList.end())
        {
            if (aItr->xShape.is())
            {
                if ( !aItr->xShape->getShapeType().equals(sCaptionShape) )
                {
                    awt::Point aEndPoint;
                    Rectangle aEndRec = pDoc->GetMMRect(aItr->aEndAddress.Col(), aItr->aEndAddress.Row(),
                        aItr->aEndAddress.Col(), aItr->aEndAddress.Row(), aItr->aEndAddress.Tab());
                    ::rtl::OUString sEndAddress;
                    ScXMLConverter::GetStringFromAddress(sEndAddress, aItr->aEndAddress, pDoc);
                    AddAttribute(XML_NAMESPACE_TABLE, XML_END_CELL_ADDRESS, sEndAddress);
                    aEndPoint.X = aEndRec.Left();
                    aEndPoint.Y = aEndRec.Top();
                    awt::Point aStartPoint = aItr->xShape->getPosition();
                    awt::Size aSize = aItr->xShape->getSize();
                    sal_Int32 nEndX = aStartPoint.X + aSize.Width - aEndPoint.X;
                    sal_Int32 nEndY = aStartPoint.Y + aSize.Height - aEndPoint.Y;
                    ::rtl::OUStringBuffer sBuffer;
                    GetMM100UnitConverter().convertMeasure(sBuffer, nEndX);
                    AddAttribute(XML_NAMESPACE_TABLE, XML_END_X, sBuffer.makeStringAndClear());
                    GetMM100UnitConverter().convertMeasure(sBuffer, nEndY);
                    AddAttribute(XML_NAMESPACE_TABLE, XML_END_Y, sBuffer.makeStringAndClear());
                }
                ExportShape(aItr->xShape, pPoint);
            }
            aItr++;
        }
    }
}

void ScXMLExport::WriteTableShapes()
{
    ScMyTableShapes* pTableShapes = pSharedData->GetTableShapes();
    if (pTableShapes && !(*pTableShapes)[nCurrentTable].empty())
    {
        DBG_ASSERT(pTableShapes->size() > static_cast<sal_uInt32>(nCurrentTable), "wrong Table");
        SvXMLElementExport aShapesElem(*this, XML_NAMESPACE_TABLE, XML_SHAPES, sal_True, sal_False);
        ScMyTableXShapes::iterator aItr = (*pTableShapes)[nCurrentTable].begin();
        while (aItr != (*pTableShapes)[nCurrentTable].end())
        {
            if (aItr->is())
            {
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

void ScXMLExport::WriteAnnotation(const ScMyCell& rMyCell)
{
    if( rMyCell.bHasAnnotation && rMyCell.xAnnotation.is())
    {
        ::rtl::OUString sAuthor(rMyCell.xAnnotation->getAuthor());
        if (sAuthor.getLength())
            AddAttribute(XML_NAMESPACE_OFFICE, XML_AUTHOR, sAuthor);
        String aDate(rMyCell.xAnnotation->getDate());
        if (pDoc)
        {
            SvNumberFormatter* pNumForm = pDoc->GetFormatTable();
            double fDate;
            sal_uInt32 nfIndex = pNumForm->GetFormatIndex(NF_DATE_SYS_DDMMYYYY, LANGUAGE_SYSTEM);
            if (pNumForm->IsNumberFormat(aDate, nfIndex, fDate))
            {
                ::rtl::OUStringBuffer sBuf;
                GetMM100UnitConverter().convertDateTime(sBuf, fDate);
                AddAttribute(XML_NAMESPACE_OFFICE, XML_CREATE_DATE, sBuf.makeStringAndClear());
            }
            else
                AddAttribute(XML_NAMESPACE_OFFICE, XML_CREATE_DATE_STRING, ::rtl::OUString(aDate));
        }
        else
            AddAttribute(XML_NAMESPACE_OFFICE, XML_CREATE_DATE_STRING, ::rtl::OUString(aDate));
        if (rMyCell.xAnnotation->getIsVisible())
            AddAttribute(XML_NAMESPACE_OFFICE, XML_DISPLAY, XML_TRUE);
        SvXMLElementExport aElemA(*this, XML_NAMESPACE_OFFICE, XML_ANNOTATION, sal_True, sal_True);
        sal_Int32 i = 0;
        ::rtl::OUStringBuffer sTemp;
        sal_Bool bPrevCharWasSpace(sal_True);
        String sText(rMyCell.sAnnotationText);
        ::rtl::OUString sOUText2 (sText.ConvertLineEnd(LINEEND_LF));
        while(i < sOUText2.getLength())
        {
            if (sOUText2[i] == '\n')
            {
                SvXMLElementExport aElemP(*this, sElemP, sal_True, sal_False);
                GetTextParagraphExport()->exportText(sTemp.makeStringAndClear(), bPrevCharWasSpace);
            }
            else
                sTemp.append(sOUText2[i]);
            i++;
        }
        if (sTemp.getLength())
        {
            SvXMLElementExport aElemP(*this, sElemP, sal_True, sal_False);
            GetTextParagraphExport()->exportText(sTemp.makeStringAndClear(), bPrevCharWasSpace);
        }
        CheckAttrList();
    }
}

void ScXMLExport::WriteDetective( const ScMyCell& rMyCell )
{
    if( rMyCell.bHasDetectiveObj || rMyCell.bHasDetectiveOp )
    {
        const ScMyDetectiveObjVec& rObjVec = rMyCell.aDetectiveObjVec;
        const ScMyDetectiveOpVec& rOpVec = rMyCell.aDetectiveOpVec;
        sal_Int32 nObjCount = rObjVec.size();
        sal_Int32 nOpCount = rOpVec.size();
        if( nObjCount || nOpCount )
        {
            SvXMLElementExport aDetElem( *this, XML_NAMESPACE_TABLE, XML_DETECTIVE, sal_True, sal_True );
            OUString sString;
            for( ScMyDetectiveObjVec::const_iterator aObjItr = rObjVec.begin(); aObjItr != rObjVec.end(); aObjItr++ )
            {
                if (aObjItr->eObjType != SC_DETOBJ_CIRCLE)
                {
                    if( (aObjItr->eObjType == SC_DETOBJ_ARROW) || (aObjItr->eObjType == SC_DETOBJ_TOOTHERTAB))
                    {
                        ScXMLConverter::GetStringFromRange( sString, aObjItr->aSourceRange, pDoc );
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
            }
            OUStringBuffer aBuffer;
            for( ScMyDetectiveOpVec::const_iterator aOpItr = rOpVec.begin(); aOpItr != rOpVec.end(); aOpItr++ )
            {
                OUString sString;
                ScXMLConverter::GetStringFromDetOpType( sString, aOpItr->eOpType );
                AddAttribute( XML_NAMESPACE_TABLE, XML_NAME, sString );
                SvXMLUnitConverter::convertNumber( aBuffer, aOpItr->nIndex );
                AddAttribute( XML_NAMESPACE_TABLE, XML_INDEX, aBuffer.makeStringAndClear() );
                SvXMLElementExport aRangeElem( *this, XML_NAMESPACE_TABLE, XML_OPERATION, sal_True, sal_True );
            }
        }
    }
}

void ScXMLExport::SetRepeatAttribute (const sal_Int32 nEqualCellCount)
{
    if (nEqualCellCount > 0)
    {
        sal_Int32 nTemp = nEqualCellCount + 1;
        OUString sOUEqualCellCount = OUString::valueOf(nTemp);
        AddAttribute(sAttrColumnsRepeated, sOUEqualCellCount);
        GetProgressBarHelper()->Increment(nEqualCellCount);
    }
}

sal_Bool ScXMLExport::IsCellTypeEqual (const ScMyCell& aCell1, const ScMyCell& aCell2) const
{
    return (aCell1.nType == aCell2.nType);
}

sal_Bool ScXMLExport::IsEditCell(const ::com::sun::star::table::CellAddress& aAddress) const
{
    ScAddress aCoreAddress(static_cast<sal_uInt16>(aAddress.Column),
                        static_cast<sal_uInt16>(aAddress.Row),
                        static_cast<sal_uInt16>(aAddress.Sheet));
    ScBaseCell* pBaseCell = GetDocument() ? GetDocument()->GetCell(aCoreAddress) : NULL;
    if (pBaseCell)
        return (pBaseCell->GetCellType() == CELLTYPE_EDIT);
    return sal_False;
}

sal_Bool ScXMLExport::IsEditCell(const ::com::sun::star::uno::Reference < ::com::sun::star::table::XCell>& xCell) const
{
    uno::Reference<sheet::XCellAddressable> xAddressable (xCell, uno::UNO_QUERY);
    if ( xAddressable.is() )
        return IsEditCell(xAddressable->getCellAddress());
    return sal_False;
}

sal_Bool ScXMLExport::IsEditCell(ScMyCell& rCell) const
{
    if (rCell.bKnowWhetherIsEditCell)
        return rCell.bIsEditCell;
    else
    {
        rCell.bIsEditCell = IsEditCell(rCell.aCellAddress);
        rCell.bKnowWhetherIsEditCell = sal_True;
        return rCell.bIsEditCell;
    }
}

sal_Bool ScXMLExport::IsAnnotationEqual(const uno::Reference<table::XCell>& xCell1,
                                        const uno::Reference<table::XCell>& xCell2)
{
    uno::Reference<sheet::XSheetAnnotationAnchor> xSheetAnnotationAnchor1(xCell1, uno::UNO_QUERY);
    uno::Reference<sheet::XSheetAnnotationAnchor> xSheetAnnotationAnchor2(xCell2, uno::UNO_QUERY);
    if (xSheetAnnotationAnchor1.is() && xSheetAnnotationAnchor2.is())
    {
        uno::Reference <sheet::XSheetAnnotation> xSheetAnnotation1 = xSheetAnnotationAnchor1->getAnnotation();
        uno::Reference <sheet::XSheetAnnotation> xSheetAnnotation2 = xSheetAnnotationAnchor2->getAnnotation();
        uno::Reference<text::XSimpleText> xSimpleText1(xSheetAnnotation1, uno::UNO_QUERY);
        uno::Reference<text::XSimpleText> xSimpleText2(xSheetAnnotation2, uno::UNO_QUERY);
        if (xSheetAnnotation1.is() && xSimpleText1.is() &&
            xSheetAnnotation2.is() && xSimpleText2.is())
        {
            ::rtl::OUString sText1 = xSimpleText1->getString();
            ::rtl::OUString sText2 = xSimpleText2->getString();
            sal_Int32 nLength1 = sText1.getLength();
            sal_Int32 nLength2 = sText2.getLength();
            if (nLength1 && nLength2)
                if (sText1 == sText2 &&
                    xSheetAnnotation1->getAuthor() == xSheetAnnotation2->getAuthor() &&
                    xSheetAnnotation1->getDate() == xSheetAnnotation2->getDate() &&
                    xSheetAnnotation1->getIsVisible() == xSheetAnnotation2->getIsVisible())
                    return sal_True;
                else
                    return sal_False;
            else
                if (nLength1 || nLength2)
                    return sal_False;
                else
                    return sal_True;
        }
    }
    return sal_False;
}

sal_Bool ScXMLExport::IsCellEqual (ScMyCell& aCell1, ScMyCell& aCell2)
{
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
            if (!aCell1.bHasAnnotation || (aCell1.bHasAnnotation && IsAnnotationEqual(aCell1.xCell, aCell2.xCell)))
            {
                if (((aCell1.nStyleIndex == aCell2.nStyleIndex) && (aCell1.bIsAutoStyle == aCell2.bIsAutoStyle) ||
                    (aCell1.nStyleIndex == aCell2.nStyleIndex) && (aCell1.nStyleIndex == -1)) &&
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
                                aCell1.fValue = aCell1.xCell->getValue();
                                aCell1.bHasDoubleValue = sal_True;
                            }
                            if (!aCell2.bHasDoubleValue)
                            {
                                aCell2.fValue = aCell2.xCell->getValue();
                                aCell2.bHasDoubleValue = sal_True;
                            }
                            bIsEqual = (aCell1.fValue == aCell2.fValue);
                        }
                        break;
                    case table::CellContentType_TEXT :
                        {
                            if (IsEditCell(aCell1) || IsEditCell(aCell2))
                                bIsEqual = sal_False;
                            else
                            {
                                if (GetCellText(aCell1) && GetCellText(aCell2))
                                {
                                    bIsEqual = (aCell1.sStringValue == aCell2.sStringValue) &&
                                               (aCell1.xCell->getFormula() == aCell2.xCell->getFormula());
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
        sal_Bool bCalcAsShown (::cppu::any2bool( xPropertySet->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_CALCASSHOWN))) ));
        sal_Bool bIgnoreCase (::cppu::any2bool( xPropertySet->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_IGNORECASE))) ));
        sal_Bool bLookUpLabels (::cppu::any2bool( xPropertySet->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_LOOKUPLABELS))) ));
        sal_Bool bMatchWholeCell (::cppu::any2bool( xPropertySet->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_MATCHWHOLE))) ));
        sal_Bool bUseRegularExpressions (::cppu::any2bool( xPropertySet->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_REGEXENABLED))) ));
        sal_Bool bIsIterationEnabled (::cppu::any2bool( xPropertySet->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ITERENABLED))) ));
        sal_uInt16 nYear2000 (pDoc ? pDoc->GetDocOptions().GetYear2000() : 0);
        uno::Any aAny = xPropertySet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ITERCOUNT)));
        sal_Int32 nIterationCount(100);
        aAny >>= nIterationCount;
        aAny = xPropertySet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_ITEREPSILON)));
        double fIterationEpsilon;
        aAny >>= fIterationEpsilon;
        aAny = xPropertySet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_NULLDATE)));
        util::Date aNullDate;
        aAny >>= aNullDate;
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
                ::rtl::OUStringBuffer sBuffer;
                GetMM100UnitConverter().convertNumber(sBuffer, nYear2000);
                AddAttribute(XML_NAMESPACE_TABLE, XML_NULL_YEAR, sBuffer.makeStringAndClear());
            }
            SvXMLElementExport aCalcSettings(*this, XML_NAMESPACE_TABLE, XML_CALCULATION_SETTINGS, sal_True, sal_True);
            {
                if (aNullDate.Day != 30 || aNullDate.Month != 12 || aNullDate.Year != 1899)
                {
                    ::rtl::OUStringBuffer sDate;
                    GetMM100UnitConverter().convertDateTime(sDate, 0.0, aNullDate);
                    AddAttribute(XML_NAMESPACE_TABLE, XML_DATE_VALUE, sDate.makeStringAndClear());
                    SvXMLElementExport aElemNullDate(*this, XML_NAMESPACE_TABLE, XML_NULL_DATE, sal_True, sal_True);
                }
                if (bIsIterationEnabled || nIterationCount != 100 || !::rtl::math::approxEqual(fIterationEpsilon, 0.001))
                {
                    ::rtl::OUStringBuffer sBuffer;
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
            ::rtl::OUString sLink (xLinkable->getLinkUrl());
            uno::Reference <beans::XPropertySet> xProps (GetModel(), uno::UNO_QUERY);
            if (xProps.is())
            {
                uno::Any aAny = xProps->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_SHEETLINKS)));
                uno::Reference <container::XIndexAccess> xIndex;
                if (aAny >>= xIndex)
                {
                    sal_Int32 nCount = xIndex->getCount();
                    if (nCount)
                    {
                        sal_Bool bFound(sal_False);
                        uno::Reference <beans::XPropertySet> xLinkProps;
                        for (sal_Int32 i = 0; (i < nCount) && !bFound; i++)
                        {
                            uno::Any aSheetLink = xIndex->getByIndex(i);
                            if (aSheetLink >>= xLinkProps)
                            {
                                aAny = xLinkProps->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_LINKURL)));
                                ::rtl::OUString sNewLink;
                                if (aAny >>= sNewLink)
                                    bFound = sLink.equals(sNewLink);
                            }
                        }
                        if (bFound && xLinkProps.is())
                        {
                            ::rtl::OUString sFilter;
                            ::rtl::OUString sFilterOptions;
                            ::rtl::OUString sTableName (xLinkable->getLinkSheetName());
                            sal_Int32 nRefresh(0);
                            aAny = xLinkProps->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_FILTER)));
                            aAny >>= sFilter;
                            aAny = xLinkProps->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_FILTOPT)));
                            aAny >>= sFilterOptions;
                            aAny = xLinkProps->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_REFDELAY)));
                            aAny >>= nRefresh;
                            if (sLink.getLength())
                            {
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
                                    ::rtl::OUStringBuffer sBuffer;
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
    if (pDoc && pDoc->IsScenario(nCurrentTable))
    {
        String		sComment;
        Color		aColor;
        sal_uInt16	nFlags;
        pDoc->GetScenarioData(nCurrentTable, sComment, aColor, nFlags);
        if (!(nFlags & SC_SCENARIO_SHOWFRAME))
            AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_BORDER, XML_FALSE);
        ::rtl::OUStringBuffer aBuffer;
        SvXMLUnitConverter::convertColor(aBuffer, aColor);
        AddAttribute(XML_NAMESPACE_TABLE, XML_BORDER_COLOR, aBuffer.makeStringAndClear());
        if (!(nFlags & SC_SCENARIO_TWOWAY))
            AddAttribute(XML_NAMESPACE_TABLE, XML_COPY_BACK, XML_FALSE);
        if (!(nFlags & SC_SCENARIO_ATTRIB))
            AddAttribute(XML_NAMESPACE_TABLE, XML_COPY_STYLES, XML_FALSE);
        if (nFlags & SC_SCENARIO_VALUE)
            AddAttribute(XML_NAMESPACE_TABLE, XML_COPY_FORMULAS, XML_FALSE);
        SvXMLUnitConverter::convertBool(aBuffer, pDoc->IsActiveScenario(nCurrentTable));
        AddAttribute(XML_NAMESPACE_TABLE, XML_IS_ACTIVE, aBuffer.makeStringAndClear());
        const ScRangeList* pRangeList = pDoc->GetScenarioRanges(nCurrentTable);
        ::rtl::OUString sRangeListStr;
        ScXMLConverter::GetStringFromRangeList( sRangeListStr, pRangeList, pDoc );
        AddAttribute(XML_NAMESPACE_TABLE, XML_SCENARIO_RANGES, sRangeListStr);
        if (sComment.Len())
            AddAttribute(XML_NAMESPACE_TABLE, XML_COMMENT, ::rtl::OUString(sComment));
        SvXMLElementExport aElem(*this, XML_NAMESPACE_TABLE, XML_SCENARIO, sal_True, sal_True);
    }
}

void ScXMLExport::WriteTheLabelRanges( const uno::Reference< sheet::XSpreadsheetDocument >& xSpreadDoc )
{
    uno::Reference< beans::XPropertySet > xDocProp( xSpreadDoc, uno::UNO_QUERY );
    if( !xDocProp.is() ) return;

    sal_Int32 nCount = 0;
    uno::Any aAny = xDocProp->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_COLLABELRNG ) ) );
    uno::Reference< sheet::XLabelRanges > xLabelRanges;
    uno::Reference< container::XIndexAccess > xColRangesIAccess;
    if( aAny >>= xLabelRanges )
        xColRangesIAccess = uno::Reference< container::XIndexAccess >( xLabelRanges, uno::UNO_QUERY );
    if( xColRangesIAccess.is() )
        nCount += xColRangesIAccess->getCount();

    aAny = xDocProp->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_ROWLABELRNG ) ) );
    uno::Reference< container::XIndexAccess > xRowRangesIAccess;
    if( aAny >>= xLabelRanges )
        xRowRangesIAccess = uno::Reference< container::XIndexAccess >( xLabelRanges, uno::UNO_QUERY );
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

    sal_Int32 nCount = xRangesIAccess->getCount();
    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        uno::Any aRangeAny = xRangesIAccess->getByIndex( nIndex );
        uno::Reference< sheet::XLabelRange > xRange;
        if( aRangeAny >>= xRange )
        {
            OUString sRangeStr;
            table::CellRangeAddress aCellRange( xRange->getLabelArea() );
            ScXMLConverter::GetStringFromRange( sRangeStr, aCellRange, pDoc );
            AddAttribute( XML_NAMESPACE_TABLE, XML_LABEL_CELL_RANGE_ADDRESS, sRangeStr );
            aCellRange = xRange->getDataArea();
            ScXMLConverter::GetStringFromRange( sRangeStr, aCellRange, pDoc );
            AddAttribute( XML_NAMESPACE_TABLE, XML_DATA_CELL_RANGE_ADDRESS, sRangeStr );
            AddAttribute( XML_NAMESPACE_TABLE, XML_ORIENTATION, bColumn ? XML_COLUMN : XML_ROW );
            SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE, XML_LABEL_RANGE, sal_True, sal_True );
        }
    }
}

void ScXMLExport::WriteNamedExpressions(const ::com::sun::star::uno::Reference < ::com::sun::star::sheet::XSpreadsheetDocument>& xSpreadDoc)
{
    uno::Reference <beans::XPropertySet> xPropertySet (xSpreadDoc, uno::UNO_QUERY);
    if (xPropertySet.is())
    {
        uno::Any aNamedRanges = xPropertySet->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_NAMEDRANGES)));
        uno::Reference <sheet::XNamedRanges> xNamedRanges;
        CheckAttrList();
        if (aNamedRanges >>= xNamedRanges)
        {
            uno::Sequence <rtl::OUString> aRangesNames = xNamedRanges->getElementNames();
            sal_Int32 nNamedRangesCount = aRangesNames.getLength();
            if (nNamedRangesCount > 0)
            {
                if (pDoc)
                {
                    ScRangeName* pNamedRanges = pDoc->GetRangeName();
                    SvXMLElementExport aElemNEs(*this, XML_NAMESPACE_TABLE, XML_NAMED_EXPRESSIONS, sal_True, sal_True);
                    for (sal_Int32 i = 0; i < nNamedRangesCount; i++)
                    {
                        CheckAttrList();
                        ::rtl::OUString sNamedRange = aRangesNames[i];
                        uno::Any aNamedRange = xNamedRanges->getByName(sNamedRange);
                        uno::Reference <sheet::XNamedRange> xNamedRange;
                        if (aNamedRange >>= xNamedRange)
                        {
                            uno::Reference <container::XNamed> xNamed (xNamedRange, uno::UNO_QUERY);
                            uno::Reference <sheet::XCellRangeReferrer> xCellRangeReferrer (xNamedRange, uno::UNO_QUERY);
                            if (xNamed.is() && xCellRangeReferrer.is())
                            {
                                ::rtl::OUString sOUName = xNamed->getName();
                                AddAttribute(sAttrName, sOUName);

                                OUString sOUBaseCellAddress;
                                ScXMLConverter::GetStringFromAddress( sOUBaseCellAddress,
                                    xNamedRange->getReferencePosition(), pDoc, sal_False, SCA_ABS_3D );
                                AddAttribute(XML_NAMESPACE_TABLE, XML_BASE_CELL_ADDRESS, sOUBaseCellAddress);

                                sal_uInt16 nRangeIndex;
                                String sName(sOUName);
                                pNamedRanges->SearchName(sName, nRangeIndex);
                                ScRangeData* pNamedRange = (*pNamedRanges)[nRangeIndex]; //should get directly and not with ScDocument
                                String sContent(xNamedRange->getContent());
                                pNamedRange->GetEnglishSymbol(sContent, sal_True);
                                ::rtl::OUString sOUTempContent(sContent);
                                uno::Reference <table::XCellRange> xCellRange = xCellRangeReferrer->getReferredCells();
                                if(xCellRange.is())
                                {
                                    ::rtl::OUString sOUContent = sOUTempContent.copy(1, sOUTempContent.getLength() - 2);
                                    AddAttribute(XML_NAMESPACE_TABLE, XML_CELL_RANGE_ADDRESS, sOUContent);
                                    sal_Int32 nRangeType = xNamedRange->getType();
                                    ::rtl::OUStringBuffer sBufferRangeType;
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
                                    ::rtl::OUString sRangeType = sBufferRangeType.makeStringAndClear();
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

// core implementation
void ScXMLExport::WriteConsolidation()
{
    if (pDoc)
    {
        const ScConsolidateParam* pCons = pDoc->GetConsolidateDlgData();
        if( pCons )
        {
            OUString sStrData;

            ScXMLConverter::GetStringFromFunction( sStrData, pCons->eFunction );
            AddAttribute( XML_NAMESPACE_TABLE, XML_FUNCTION, sStrData );

            sStrData = OUString();
            for( sal_Int32 nIndex = 0; nIndex < pCons->nDataAreaCount; nIndex++ )
                ScXMLConverter::GetStringFromArea( sStrData, *pCons->ppDataAreas[ nIndex ], pDoc, sal_True );
            AddAttribute( XML_NAMESPACE_TABLE, XML_SOURCE_CELL_RANGE_ADDRESSES, sStrData );

            ScXMLConverter::GetStringFromAddress( sStrData, ScAddress( pCons->nCol, pCons->nRow, pCons->nTab ), pDoc );
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
    ScChangeViewSettings* pViewSettings = GetDocument() ? GetDocument()->GetChangeViewSettings() : NULL;
    if (pViewSettings)
    {
        sal_Int32 nChangePos(rProps.getLength());
        rProps.realloc(nChangePos + 1);
        beans::PropertyValue* pProps = rProps.getArray();
        if (pProps)
        {
            uno::Sequence<beans::PropertyValue> aChangeProps(SC_VIEWCHANGES_COUNT);
            beans::PropertyValue* pChangeProps = aChangeProps.getArray();
            if (pChangeProps)
            {
                pChangeProps[SC_SHOW_CHANGES].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChanges"));
                pChangeProps[SC_SHOW_CHANGES].Value <<= pViewSettings->ShowChanges();
                pChangeProps[SC_SHOW_ACCEPTED_CHANGES].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowAcceptedChanges"));
                pChangeProps[SC_SHOW_ACCEPTED_CHANGES].Value <<= pViewSettings->IsShowAccepted();
                pChangeProps[SC_SHOW_REJECTED_CHANGES].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowRejectedChanges"));
                pChangeProps[SC_SHOW_REJECTED_CHANGES].Value <<= pViewSettings->IsShowRejected();
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByDatetime"));
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME].Value <<= pViewSettings->HasDate();
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_MODE].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByDatetimeMode"));
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_MODE].Value <<= static_cast<sal_Int16>(pViewSettings->GetTheDateMode());
                util::DateTime aDateTime;
                ScXMLConverter::ConvertCoreToAPIDateTime(pViewSettings->GetTheFirstDateTime(), aDateTime);
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_FIRST_DATETIME].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByDatetimeFirstDatetime"));
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_FIRST_DATETIME].Value <<= aDateTime;
                ScXMLConverter::ConvertCoreToAPIDateTime(pViewSettings->GetTheLastDateTime(), aDateTime);
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_SECOND_DATETIME].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByDatetimeSecondDatetime"));
                pChangeProps[SC_SHOW_CHANGES_BY_DATETIME_SECOND_DATETIME].Value <<= aDateTime;
                pChangeProps[SC_SHOW_CHANGES_BY_AUTHOR].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByAuthor"));
                pChangeProps[SC_SHOW_CHANGES_BY_AUTHOR].Value <<= pViewSettings->HasAuthor();
                pChangeProps[SC_SHOW_CHANGES_BY_AUTHOR_NAME].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByAuthorName"));
                pChangeProps[SC_SHOW_CHANGES_BY_AUTHOR_NAME].Value <<= ::rtl::OUString (pViewSettings->GetTheAuthorToShow());
                pChangeProps[SC_SHOW_CHANGES_BY_COMMENT].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByComment"));
                pChangeProps[SC_SHOW_CHANGES_BY_COMMENT].Value <<= pViewSettings->HasComment();
                pChangeProps[SC_SHOW_CHANGES_BY_COMMENT_TEXT].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByCommentText"));
                pChangeProps[SC_SHOW_CHANGES_BY_COMMENT_TEXT].Value <<= ::rtl::OUString (pViewSettings->GetTheComment());
                pChangeProps[SC_SHOW_CHANGES_BY_RANGES].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByRanges"));
                pChangeProps[SC_SHOW_CHANGES_BY_RANGES].Value <<= pViewSettings->HasRange();
                ::rtl::OUString sRangeList;
                ScXMLConverter::GetStringFromRangeList(sRangeList, &(pViewSettings->GetTheRangeList()), GetDocument());
                pChangeProps[SC_SHOW_CHANGES_BY_RANGES_LIST].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowChangesByRangesList"));
                pChangeProps[SC_SHOW_CHANGES_BY_RANGES_LIST].Value <<= sRangeList;

                pProps[nChangePos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TrackedChangesViewSettings"));
                pProps[nChangePos].Value <<= aChangeProps;
            }
        }
    }
}

void ScXMLExport::GetViewSettings(uno::Sequence<beans::PropertyValue>& rProps)
{
    rProps.realloc(4);
    beans::PropertyValue* pProps = rProps.getArray();
    if(pProps)
    {
        if (GetModel().is())
        {
            ScModelObj* pDocObj = ScModelObj::getImplementation( GetModel() );
            if (pDocObj)
            {
                SvEmbeddedObject* pEmbeddedObj = pDocObj->GetEmbeddedObject();
                if (pEmbeddedObj)
                {
                    Rectangle aRect(pEmbeddedObj->GetVisArea());
                    sal_uInt16 i(0);
                    pProps[i].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VisibleAreaTop"));
                    pProps[i++].Value <<= aRect.getY();
                    pProps[i].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VisibleAreaLeft"));
                    pProps[i++].Value <<= aRect.getX();
                    pProps[i].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VisibleAreaWidth"));
                    pProps[i++].Value <<= aRect.getWidth();
                    pProps[i].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VisibleAreaHeight"));
                    pProps[i++].Value <<= aRect.getHeight();
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
            uno::Reference <uno::XInterface> xInterface = xMultiServiceFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.SpreadsheetSettings")));
            uno::Reference <beans::XPropertySet> xProperties(xInterface, uno::UNO_QUERY);
            if (xProperties.is())
                SvXMLUnitConverter::convertPropertySet(rProps, xProperties);
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
        pNumberFormatAttributesExportHelper = new XMLNumberFormatAttributesExportHelper(GetNumberFormatsSupplier(), *this, XML_NAMESPACE_TABLE);
    return pNumberFormatAttributesExportHelper;
}

void ScXMLExport::CollectUserDefinedNamespaces(const SfxItemPool* pPool, sal_uInt16 nAttrib)
{
    const SfxPoolItem* pItem;
    sal_uInt16 nItems = pPool->GetItemCount( nAttrib );
    for( sal_uInt16 i = 0; i < nItems; ++i )
    {
        if( 0 != (pItem = pPool->GetItem( nAttrib, i ) ) )
        {
            const SvXMLAttrContainerItem *pUnknown =
                (const SvXMLAttrContainerItem *)pItem;
            if( (pUnknown->GetAttrCount() > 0) )
            {
                sal_uInt16 nIdx = pUnknown->GetFirstNamespaceIndex();
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
}

// XFilter
sal_Bool SAL_CALL ScXMLExport::filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
    throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return SvXMLExport::filter(aDescriptor);
}

void SAL_CALL ScXMLExport::cancel()
    throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
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
    switch( getExportFlags() )
    {
        case EXPORT_ALL:
            return ScXMLExport_getImplementationName();
            break;
        case (EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_FONTDECLS):
            return ScXMLExport_Styles_getImplementationName();
            break;
        case (EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_FONTDECLS):
            return ScXMLExport_Content_getImplementationName();
            break;
        case EXPORT_META:
            return ScXMLExport_Meta_getImplementationName();
            break;
        case EXPORT_SETTINGS:
            return ScXMLExport_Settings_getImplementationName();
            break;
        default:
            // generic name for 'unknown' cases
            return ScXMLExport_getImplementationName();
            break;
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

}
