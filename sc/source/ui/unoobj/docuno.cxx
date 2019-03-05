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

#include <config_features.h>

#include <boost/property_tree/json_parser.hpp>

#include <scitems.hxx>
#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <o3tl/any.hxx>
#include <svx/fmdpage.hxx>
#include <svx/fmview.hxx>
#include <svx/svditer.hxx>
#include <svx/svdpage.hxx>
#include <svx/svxids.hrc>
#include <svx/unoshape.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/propertysequence.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Calc.hxx>
#include <svl/numuno.hxx>
#include <svl/hint.hxx>
#include <unotools/moduleoptions.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <vcl/waitobj.hxx>
#include <unotools/charclass.hxx>
#include <tools/multisel.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <unotools/saveopt.hxx>
#include <sal/log.hxx>

#include <float.h>

#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/XLabelRanges.hpp>
#include <com/sun/star/sheet/XSelectedSheetsSupplier.hpp>
#include <com/sun/star/sheet/XUnnamedDatabaseRanges.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/document/IndexedPropertyValues.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/profilezone.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/string.hxx>
#include <cppuhelper/supportsservice.hxx>
#if HAVE_FEATURE_OPENCL
#include <opencl/platforminfo.hxx>
#endif
#include <sfx2/lokhelper.hxx>
#include <sfx2/lokcharthelper.hxx>

#include <cellsuno.hxx>
#include <columnspanset.hxx>
#include <convuno.hxx>
#include <datauno.hxx>
#include <docfunc.hxx>
#include <dociter.hxx>
#include <docoptio.hxx>
#include <docsh.hxx>
#include <docuno.hxx>
#include <drwlayer.hxx>
#include <forbiuno.hxx>
#include <formulacell.hxx>
#include <formulagroup.hxx>
#include <gridwin.hxx>
#include <hints.hxx>
#include <inputhdl.hxx>
#include <inputopt.hxx>
#include <interpre.hxx>
#include <linkuno.hxx>
#include <markdata.hxx>
#include <miscuno.hxx>
#include <nameuno.hxx>
#include <notesuno.hxx>
#include <optuno.hxx>
#include <pfuncache.hxx>
#include <postit.hxx>
#include <printfun.hxx>
#include <rangeutl.hxx>
#include <scmod.hxx>
#include <scresid.hxx>
#include <servuno.hxx>
#include <shapeuno.hxx>
#include <sheetevents.hxx>
#include <styleuno.hxx>
#include <tabvwsh.hxx>
#include <targuno.hxx>
#include <unonames.hxx>
#include <ViewSettingsSequenceDefines.hxx>
#include <viewuno.hxx>
#include <editsh.hxx>
#include <drawsh.hxx>
#include <drtxtob.hxx>
#include <transobj.hxx>
#include <chgtrack.hxx>
#include <table.hxx>
#include <appoptio.hxx>
#include <formulaopt.hxx>

#include <strings.hrc>

using namespace com::sun::star;

// #i111553# provides the name of the VBA constant for this document type (e.g. 'ThisExcelDoc' for Calc)
#define SC_UNO_VBAGLOBNAME "VBAGlobalConstantName"

//  no Which-ID here, map only for PropertySetInfo

//! rename this, those are no longer only options
static const SfxItemPropertyMapEntry* lcl_GetDocOptPropertyMap()
{
    static const SfxItemPropertyMapEntry aDocOptPropertyMap_Impl[] =
    {
        {OUString(SC_UNO_APPLYFMDES),              0, cppu::UnoType<bool>::get(),                                             0, 0},
        {OUString(SC_UNO_AREALINKS),               0, cppu::UnoType<sheet::XAreaLinks>::get(),               0, 0},
        {OUString(SC_UNO_AUTOCONTFOC),             0, cppu::UnoType<bool>::get(),                                             0, 0},
        {OUString(SC_UNO_BASICLIBRARIES),          0, cppu::UnoType<script::XLibraryContainer>::get(),     beans::PropertyAttribute::READONLY, 0},
        {OUString(SC_UNO_DIALOGLIBRARIES),         0, cppu::UnoType<script::XLibraryContainer>::get(),     beans::PropertyAttribute::READONLY, 0},
        {OUString(SC_UNO_VBAGLOBNAME),             0, cppu::UnoType<OUString>::get(),                  beans::PropertyAttribute::READONLY, 0},
        {OUString(SC_UNO_CALCASSHOWN),             PROP_UNO_CALCASSHOWN, cppu::UnoType<bool>::get(),                          0, 0},
        {OUString(SC_UNONAME_CLOCAL),              0, cppu::UnoType<lang::Locale>::get(),                                    0, 0},
        {OUString(SC_UNO_CJK_CLOCAL),              0, cppu::UnoType<lang::Locale>::get(),                                    0, 0},
        {OUString(SC_UNO_CTL_CLOCAL),              0, cppu::UnoType<lang::Locale>::get(),                                    0, 0},
        {OUString(SC_UNO_COLLABELRNG),             0, cppu::UnoType<sheet::XLabelRanges>::get(),             0, 0},
        {OUString(SC_UNO_DDELINKS),                0, cppu::UnoType<container::XNameAccess>::get(),          0, 0},
        {OUString(SC_UNO_DEFTABSTOP),              PROP_UNO_DEFTABSTOP, cppu::UnoType<sal_Int16>::get(),                     0, 0},
        {OUString(SC_UNO_EXTERNALDOCLINKS),        0, cppu::UnoType<sheet::XExternalDocLinks>::get(),        0, 0},
        {OUString(SC_UNO_FORBIDDEN),               0, cppu::UnoType<i18n::XForbiddenCharacters>::get(),      beans::PropertyAttribute::READONLY, 0},
        {OUString(SC_UNO_HASDRAWPAGES),            0, cppu::UnoType<bool>::get(),                                             beans::PropertyAttribute::READONLY, 0},
        {OUString(SC_UNO_IGNORECASE),              PROP_UNO_IGNORECASE, cppu::UnoType<bool>::get(),                           0, 0},
        {OUString(SC_UNO_ITERENABLED),             PROP_UNO_ITERENABLED, cppu::UnoType<bool>::get(),                          0, 0},
        {OUString(SC_UNO_ITERCOUNT),               PROP_UNO_ITERCOUNT, cppu::UnoType<sal_Int32>::get(),                      0, 0},
        {OUString(SC_UNO_ITEREPSILON),             PROP_UNO_ITEREPSILON, cppu::UnoType<double>::get(),                       0, 0},
        {OUString(SC_UNO_LOOKUPLABELS),            PROP_UNO_LOOKUPLABELS, cppu::UnoType<bool>::get(),                         0, 0},
        {OUString(SC_UNO_MATCHWHOLE),              PROP_UNO_MATCHWHOLE, cppu::UnoType<bool>::get(),                           0, 0},
        {OUString(SC_UNO_NAMEDRANGES),             0, cppu::UnoType<sheet::XNamedRanges>::get(),             0, 0},
        {OUString(SC_UNO_DATABASERNG),             0, cppu::UnoType<sheet::XDatabaseRanges>::get(),          0, 0},
        {OUString(SC_UNO_NULLDATE),                PROP_UNO_NULLDATE, cppu::UnoType<util::Date>::get(),                      0, 0},
        {OUString(SC_UNO_ROWLABELRNG),             0, cppu::UnoType<sheet::XLabelRanges>::get(),             0, 0},
        {OUString(SC_UNO_SHEETLINKS),              0, cppu::UnoType<container::XNameAccess>::get(),          0, 0},
        {OUString(SC_UNO_SPELLONLINE),             PROP_UNO_SPELLONLINE, cppu::UnoType<bool>::get(),                          0, 0},
        {OUString(SC_UNO_STANDARDDEC),             PROP_UNO_STANDARDDEC, cppu::UnoType<sal_Int16>::get(),                    0, 0},
        {OUString(SC_UNO_REGEXENABLED),            PROP_UNO_REGEXENABLED, cppu::UnoType<bool>::get(),                         0, 0},
        {OUString(SC_UNO_WILDCARDSENABLED),        PROP_UNO_WILDCARDSENABLED, cppu::UnoType<bool>::get(),                         0, 0},
        {OUString(SC_UNO_RUNTIMEUID),              0, cppu::UnoType<OUString>::get(),                  beans::PropertyAttribute::READONLY, 0},
        {OUString(SC_UNO_HASVALIDSIGNATURES),      0, cppu::UnoType<bool>::get(),                                             beans::PropertyAttribute::READONLY, 0},
        {OUString(SC_UNO_ISLOADED),                0, cppu::UnoType<bool>::get(),                                             0, 0},
        {OUString(SC_UNO_ISUNDOENABLED),           0, cppu::UnoType<bool>::get(),                                             0, 0},
        {OUString(SC_UNO_RECORDCHANGES),           0, cppu::UnoType<bool>::get(),                                             0, 0},
        {OUString(SC_UNO_ISRECORDCHANGESPROTECTED),0, cppu::UnoType<bool>::get(),            beans::PropertyAttribute::READONLY, 0},
        {OUString(SC_UNO_ISADJUSTHEIGHTENABLED),   0, cppu::UnoType<bool>::get(),                                             0, 0},
        {OUString(SC_UNO_ISEXECUTELINKENABLED),    0, cppu::UnoType<bool>::get(),                                             0, 0},
        {OUString(SC_UNO_ISCHANGEREADONLYENABLED), 0, cppu::UnoType<bool>::get(),                                             0, 0},
        {OUString(SC_UNO_REFERENCEDEVICE),         0, cppu::UnoType<awt::XDevice>::get(),                    beans::PropertyAttribute::READONLY, 0},
        {OUString("BuildId"),                      0, ::cppu::UnoType<OUString>::get(),                0, 0},
        {OUString(SC_UNO_CODENAME),                0, cppu::UnoType<OUString>::get(),                  0, 0},
        {OUString(SC_UNO_INTEROPGRABBAG),          0, cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get(), 0, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aDocOptPropertyMap_Impl;
}

//! StandardDecimals as property and from NumberFormatter ????????

static const SfxItemPropertyMapEntry* lcl_GetColumnsPropertyMap()
{
    static const SfxItemPropertyMapEntry aColumnsPropertyMap_Impl[] =
    {
        {OUString(SC_UNONAME_MANPAGE),  0,  cppu::UnoType<bool>::get(),          0, 0 },
        {OUString(SC_UNONAME_NEWPAGE),  0,  cppu::UnoType<bool>::get(),          0, 0 },
        {OUString(SC_UNONAME_CELLVIS),  0,  cppu::UnoType<bool>::get(),          0, 0 },
        {OUString(SC_UNONAME_OWIDTH),   0,  cppu::UnoType<bool>::get(),          0, 0 },
        {OUString(SC_UNONAME_CELLWID),  0,  cppu::UnoType<sal_Int32>::get(),    0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aColumnsPropertyMap_Impl;
}

static const SfxItemPropertyMapEntry* lcl_GetRowsPropertyMap()
{
    static const SfxItemPropertyMapEntry aRowsPropertyMap_Impl[] =
    {
        {OUString(SC_UNONAME_CELLHGT),  0,  cppu::UnoType<sal_Int32>::get(),    0, 0 },
        {OUString(SC_UNONAME_CELLFILT), 0,  cppu::UnoType<bool>::get(),          0, 0 },
        {OUString(SC_UNONAME_OHEIGHT),  0,  cppu::UnoType<bool>::get(),          0, 0 },
        {OUString(SC_UNONAME_MANPAGE),  0,  cppu::UnoType<bool>::get(),          0, 0 },
        {OUString(SC_UNONAME_NEWPAGE),  0,  cppu::UnoType<bool>::get(),          0, 0 },
        {OUString(SC_UNONAME_CELLVIS),  0,  cppu::UnoType<bool>::get(),          0, 0 },
        {OUString(SC_UNONAME_CELLBACK), ATTR_BACKGROUND, ::cppu::UnoType<sal_Int32>::get(), 0, MID_BACK_COLOR },
        {OUString(SC_UNONAME_CELLTRAN), ATTR_BACKGROUND, cppu::UnoType<bool>::get(), 0, MID_GRAPHIC_TRANSPARENT },
        // not sorted, not used with SfxItemPropertyMapEntry::GetByName
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aRowsPropertyMap_Impl;
}

using sc::HMMToTwips;
using sc::TwipsToHMM;

#define SCMODELOBJ_SERVICE          "com.sun.star.sheet.SpreadsheetDocument"
#define SCDOCSETTINGS_SERVICE       "com.sun.star.sheet.SpreadsheetDocumentSettings"
#define SCDOC_SERVICE               "com.sun.star.document.OfficeDocument"

SC_SIMPLE_SERVICE_INFO( ScAnnotationsObj, "ScAnnotationsObj", "com.sun.star.sheet.CellAnnotations" )
SC_SIMPLE_SERVICE_INFO( ScDrawPagesObj, "ScDrawPagesObj", "com.sun.star.drawing.DrawPages" )
SC_SIMPLE_SERVICE_INFO( ScScenariosObj, "ScScenariosObj", "com.sun.star.sheet.Scenarios" )
SC_SIMPLE_SERVICE_INFO( ScSpreadsheetSettingsObj, "ScSpreadsheetSettingsObj", "com.sun.star.sheet.SpreadsheetDocumentSettings" )
SC_SIMPLE_SERVICE_INFO( ScTableColumnsObj, "ScTableColumnsObj", "com.sun.star.table.TableColumns" )
SC_SIMPLE_SERVICE_INFO( ScTableRowsObj, "ScTableRowsObj", "com.sun.star.table.TableRows" )
SC_SIMPLE_SERVICE_INFO( ScTableSheetsObj, "ScTableSheetsObj", "com.sun.star.sheet.Spreadsheets" )

class ScPrintUIOptions : public vcl::PrinterOptionsHelper
{
public:
    ScPrintUIOptions();
    void SetDefaults();
};

ScPrintUIOptions::ScPrintUIOptions()
{
    const ScPrintOptions& rPrintOpt = SC_MOD()->GetPrintOptions();
    sal_Int32 nContent = rPrintOpt.GetAllSheets() ? 0 : 1;
    bool bSuppress = rPrintOpt.GetSkipEmpty();

    sal_Int32 nNumProps= 9, nIdx = 0;

    m_aUIProperties.resize(nNumProps);

    // load the writer PrinterOptions into the custom tab
    m_aUIProperties[nIdx].Name = "OptionsUIFile";
    m_aUIProperties[nIdx++].Value <<= OUString("modules/scalc/ui/printeroptions.ui");

    // create Section for spreadsheet (results in an extra tab page in dialog)
    SvtModuleOptions aOpt;
    OUString aAppGroupname( ScResId( SCSTR_PRINTOPT_PRODNAME ) );
    aAppGroupname = aAppGroupname.replaceFirst( "%s", aOpt.GetModuleName( SvtModuleOptions::EModule::CALC ) );
    m_aUIProperties[nIdx++].Value = setGroupControlOpt("tabcontrol-page2", aAppGroupname, OUString());

    // show subgroup for pages
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("pages", ScResId( SCSTR_PRINTOPT_PAGES ), OUString());

    // create a bool option for empty pages
    m_aUIProperties[nIdx++].Value = setBoolControlOpt("suppressemptypages", ScResId( SCSTR_PRINTOPT_SUPPRESSEMPTY ),
                                                  ".HelpID:vcl:PrintDialog:IsSuppressEmptyPages:CheckBox",
                                                  "IsSuppressEmptyPages",
                                                  bSuppress);
    // show Subgroup for print content
    vcl::PrinterOptionsHelper::UIControlOptions aPrintRangeOpt;
    aPrintRangeOpt.maGroupHint = "PrintRange";
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("printrange", ScResId( SCSTR_PRINTOPT_PAGES ),
                                                      OUString(),
                                                      aPrintRangeOpt);

    // create a choice for the content to create
    uno::Sequence< OUString > aChoices{
        ScResId( SCSTR_PRINTOPT_ALLSHEETS ),
        ScResId( SCSTR_PRINTOPT_SELECTEDSHEETS ),
        ScResId( SCSTR_PRINTOPT_SELECTEDCELLS )};
    uno::Sequence< OUString > aHelpIds{
        ".HelpID:vcl:PrintDialog:PrintContent:ListBox"};
    m_aUIProperties[nIdx++].Value = setChoiceListControlOpt( "printextrabox", OUString(),
                                                    aHelpIds, "PrintContent",
                                                    aChoices, nContent );

    // show Subgroup for print range
    aPrintRangeOpt.mbInternalOnly = true;
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("fromwhich", ScResId( SCSTR_PRINTOPT_FROMWHICH ),
                                                      OUString(),
                                                      aPrintRangeOpt);

    // create a choice for the range to print
    OUString aPrintRangeName( "PrintRange" );
    aChoices.realloc( 4 );
    aHelpIds.realloc( 4 );
    uno::Sequence< OUString > aWidgetIds( 4 );
    aChoices[0] = ScResId( SCSTR_PRINTOPT_PRINTALLPAGES );
    aHelpIds[0] = ".HelpID:vcl:PrintDialog:PrintRange:RadioButton:0";
    aWidgetIds[0] = "rbAllPages";
    aChoices[1] = ScResId( SCSTR_PRINTOPT_PRINTPAGES );
    aHelpIds[1] = ".HelpID:vcl:PrintDialog:PrintRange:RadioButton:1";
    aWidgetIds[1] = "rbRangePages";
    aChoices[2] = ScResId( SCSTR_PRINTOPT_PRINTEVENPAGES );
    aHelpIds[2] = ".HelpID:vcl:PrintDialog:PrintRange:RadioButton:2";
    aWidgetIds[2] = "rbEvenPages";
    aChoices[3] = ScResId( SCSTR_PRINTOPT_PRINTODDPAGES );
    aHelpIds[3] = ".HelpID:vcl:PrintDialog:PrintRange:RadioButton:3";
    aWidgetIds[3] = "rbOddPages";
    m_aUIProperties[nIdx++].Value = setChoiceRadiosControlOpt(aWidgetIds, OUString(),
                                                    aHelpIds,
                                                    aPrintRangeName,
                                                    aChoices,
                                                    0 );

    // create a an Edit dependent on "Pages" selected
    vcl::PrinterOptionsHelper::UIControlOptions aPageRangeOpt( aPrintRangeName, 1, true );
    m_aUIProperties[nIdx++].Value = setEditControlOpt("pagerange", OUString(),
                                                      ".HelpID:vcl:PrintDialog:PageRange:Edit",
                                                      "PageRange", OUString(), aPageRangeOpt);

    assert(nIdx == nNumProps);
}

void ScPrintUIOptions::SetDefaults()
{
    // re-initialize the default values from print options

    const ScPrintOptions& rPrintOpt = SC_MOD()->GetPrintOptions();
    sal_Int32 nContent = rPrintOpt.GetAllSheets() ? 0 : 1;
    bool bSuppress = rPrintOpt.GetSkipEmpty();

    for (beans::PropertyValue & rPropValue : m_aUIProperties)
    {
        uno::Sequence<beans::PropertyValue> aUIProp;
        if ( rPropValue.Value >>= aUIProp )
        {
            for (sal_Int32 nPropPos=0; nPropPos<aUIProp.getLength(); ++nPropPos)
            {
                OUString aName = aUIProp[nPropPos].Name;
                if ( aName == "Property" )
                {
                    beans::PropertyValue aPropertyValue;
                    if ( aUIProp[nPropPos].Value >>= aPropertyValue )
                    {
                        if ( aPropertyValue.Name == "PrintContent" )
                        {
                            aPropertyValue.Value <<= nContent;
                            aUIProp[nPropPos].Value <<= aPropertyValue;
                        }
                        else if ( aPropertyValue.Name == "IsSuppressEmptyPages" )
                        {
                            aPropertyValue.Value <<= bSuppress;
                            aUIProp[nPropPos].Value <<= aPropertyValue;
                        }
                    }
                }
            }
            rPropValue.Value <<= aUIProp;
        }
    }
}

void ScModelObj::CreateAndSet(ScDocShell* pDocSh)
{
    if (pDocSh)
        pDocSh->SetBaseModel( new ScModelObj(pDocSh) );
}

SdrModel& ScModelObj::getSdrModelFromUnoModel() const
{
    ScDocument& rDoc(pDocShell->GetDocument());

    if(!rDoc.GetDrawLayer())
    {
        rDoc.InitDrawLayer();
    }

    return *rDoc.GetDrawLayer(); // TTTT should be reference
}

ScModelObj::ScModelObj( ScDocShell* pDocSh ) :
    SfxBaseModel( pDocSh ),
    aPropSet( lcl_GetDocOptPropertyMap() ),
    pDocShell( pDocSh ),
    maChangesListeners( m_aMutex )
{
    // pDocShell may be NULL if this is the base of a ScDocOptionsObj
    if ( pDocShell )
    {
        pDocShell->GetDocument().AddUnoObject(*this);      // SfxModel is derived from SfxListener
    }
}

ScModelObj::~ScModelObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);

    if (xNumberAgg.is())
        xNumberAgg->setDelegator(uno::Reference<uno::XInterface>());

    pPrintFuncCache.reset();
    pPrinterOptions.reset();
}

uno::Reference< uno::XAggregation> const & ScModelObj::GetFormatter()
{
    // pDocShell may be NULL if this is the base of a ScDocOptionsObj
    if ( !xNumberAgg.is() && pDocShell )
    {
        // setDelegator changes RefCount, so we'd better hold the reference ourselves
        // (directly in m_refCount, so we don't delete ourselves with release())
        osl_atomic_increment( &m_refCount );
        // we need a reference to SvNumberFormatsSupplierObj during queryInterface,
        // otherwise it'll be deleted
        uno::Reference<util::XNumberFormatsSupplier> xFormatter(
            new SvNumberFormatsSupplierObj(pDocShell->GetDocument().GetThreadedContext().GetFormatTable() ));
        {
            xNumberAgg.set(uno::Reference<uno::XAggregation>( xFormatter, uno::UNO_QUERY ));
            // extra block to force deletion of the temporary before setDelegator
        }

        // during setDelegator no additional reference should exist
        xFormatter = nullptr;

        if (xNumberAgg.is())
            xNumberAgg->setDelegator( static_cast<cppu::OWeakObject*>(this) );
        osl_atomic_decrement( &m_refCount );
    } // if ( !xNumberAgg.is() )
    return xNumberAgg;
}

ScDocument* ScModelObj::GetDocument() const
{
    if (pDocShell)
        return &pDocShell->GetDocument();
    return nullptr;
}

SfxObjectShell* ScModelObj::GetEmbeddedObject() const
{
    return pDocShell;
}

void ScModelObj::UpdateAllRowHeights()
{
    if (pDocShell)
        pDocShell->UpdateAllRowHeights();
}

void ScModelObj::BeforeXMLLoading()
{
    if (pDocShell)
        pDocShell->BeforeXMLLoading();
}

void ScModelObj::AfterXMLLoading()
{
    if (pDocShell)
        pDocShell->AfterXMLLoading(true);
}

ScSheetSaveData* ScModelObj::GetSheetSaveData()
{
    if (pDocShell)
        return pDocShell->GetSheetSaveData();
    return nullptr;
}

ScFormatSaveData* ScModelObj::GetFormatSaveData()
{
    if (pDocShell)
        return pDocShell->GetFormatSaveData();
    return nullptr;
}

void ScModelObj::RepaintRange( const ScRange& rRange )
{
    if (pDocShell)
        pDocShell->PostPaint( rRange, PaintPartFlags::Grid );
}

void ScModelObj::RepaintRange( const ScRangeList& rRange )
{
    if (pDocShell)
        pDocShell->PostPaint( rRange, PaintPartFlags::Grid );
}

void ScModelObj::paintTile( VirtualDevice& rDevice,
                            int nOutputWidth, int nOutputHeight,
                            int nTilePosX, int nTilePosY,
                            long nTileWidth, long nTileHeight )
{
    // There seems to be no clear way of getting the grid window for this
    // particular document, hence we need to hope we get the right window.
    ScViewData* pViewData = ScDocShell::GetViewData();
    ScGridWindow* pGridWindow = pViewData->GetActiveWin();

    // update the size of the area we are painting
    // FIXME we want to use only the minimal necessary size, like the
    // following; but for the moment there is too many problems with that and
    // interaction with editeng used for the cell editing
    //Size aTileSize(nOutputWidth, nOutputHeight);
    //if (pGridWindow->GetOutputSizePixel() != aTileSize)
    //    pGridWindow->SetOutputSizePixel(Size(nOutputWidth, nOutputHeight));
    // so instead for now, set the viewport size to document size
    Size aDocSize = getDocumentSize();
    pGridWindow->SetOutputSizePixel(Size(aDocSize.Width() * pViewData->GetPPTX(), aDocSize.Height() * pViewData->GetPPTY()));

    pGridWindow->PaintTile( rDevice, nOutputWidth, nOutputHeight,
                            nTilePosX, nTilePosY, nTileWidth, nTileHeight );

    LokChartHelper::PaintAllChartsOnTile(rDevice, nOutputWidth, nOutputHeight,
                                         nTilePosX, nTilePosY, nTileWidth, nTileHeight);
}

void ScModelObj::setPart( int nPart )
{
    ScViewData* pViewData = ScDocShell::GetViewData();
    ScTabView* pTabView = dynamic_cast< ScTabView* >( pViewData->GetView() );

    if (pTabView)
        pTabView->SelectTabPage(nPart + 1);
}

int ScModelObj::getParts()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    return rDoc.GetTableCount();
}

int ScModelObj::getPart()
{
    ScViewData* pViewData = ScDocShell::GetViewData();
    return pViewData->GetViewShell()->getPart();
}

OUString ScModelObj::getPartInfo( int nPart )
{
    OUString aPartInfo;
    ScViewData* pViewData = ScDocShell::GetViewData();
    bool bIsVisible = pViewData->GetDocument()->IsVisible(nPart);

    aPartInfo += "{ \"visible\": \"";
    aPartInfo += OUString::number(static_cast<unsigned int>(bIsVisible));
    aPartInfo += "\" }";
    return aPartInfo;
}

OUString ScModelObj::getPartName( int nPart )
{
    OUString sTabName;
    ScViewData* pViewData = ScDocShell::GetViewData();
    pViewData->GetDocument()->GetName(nPart, sTabName);
    return sTabName;
}

OUString ScModelObj::getPartHash( int nPart )
{
    sal_Int64 nHashCode;
    ScViewData* pViewData = ScDocShell::GetViewData();
    return (pViewData->GetDocument()->GetHashCode(nPart, nHashCode) ? OUString::number(nHashCode) : OUString());
}

VclPtr<vcl::Window> ScModelObj::getDocWindow()
{
    SolarMutexGuard aGuard;

    // There seems to be no clear way of getting the grid window for this
    // particular document, hence we need to hope we get the right window.
    ScViewData* pViewData = ScDocShell::GetViewData();
    VclPtr<vcl::Window> pWindow;
    if (pViewData)
    {
        pWindow = pViewData->GetActiveWin();

        LokChartHelper aChartHelper(pViewData->GetViewShell());
        vcl::Window* pChartWindow = aChartHelper.GetWindow();
        if (pChartWindow)
            pWindow = pChartWindow;
    }

    return pWindow;
}

Size ScModelObj::getDocumentSize()
{
    Size aSize(10, 10); // minimum size

    ScViewData* pViewData = ScDocShell::GetViewData();
    if (!pViewData)
        return aSize;

    SCTAB nTab = pViewData->GetTabNo();
    SCCOL nEndCol = 0;
    SCROW nEndRow = 0;
    const ScDocument& rDoc = pDocShell->GetDocument();

    rDoc.GetTiledRenderingArea(nTab, nEndCol, nEndRow);

    const ScDocument* pThisDoc = &rDoc;

    auto GetColWidthPx = [pThisDoc, nTab](SCCOL nCol) {
        const sal_uInt16 nSize = pThisDoc->GetColWidth(nCol, nTab);
        return ScViewData::ToPixel(nSize, 1.0 / TWIPS_PER_PIXEL);
    };

    long nDocWidthPixel = pViewData->GetLOKWidthHelper().computePosition(nEndCol, GetColWidthPx);


    auto GetRowHeightPx = [pThisDoc, nTab](SCROW nRow) {
        const sal_uInt16 nSize = pThisDoc->GetRowHeight(nRow, nTab);
        return ScViewData::ToPixel(nSize, 1.0 / TWIPS_PER_PIXEL);
    };

    long nDocHeightPixel = pViewData->GetLOKHeightHelper().computePosition(nEndRow, GetRowHeightPx);


    if (nDocWidthPixel > 0 && nDocHeightPixel > 0)
    {
        // convert to twips
        aSize.setWidth(nDocWidthPixel * TWIPS_PER_PIXEL);
        aSize.setHeight(nDocHeightPixel * TWIPS_PER_PIXEL);
    }
    else
    {
        // convert to twips
        aSize.setWidth(rDoc.GetColWidth(0, nEndCol, nTab));
        aSize.setHeight(rDoc.GetRowHeight(0, nEndRow, nTab));
    }

    return aSize;
}


void ScModelObj::postKeyEvent(int nType, int nCharCode, int nKeyCode)
{
    SolarMutexGuard aGuard;
    SfxLokHelper::postKeyEventAsync(getDocWindow(), nType, nCharCode, nKeyCode);
}

void ScModelObj::postMouseEvent(int nType, int nX, int nY, int nCount, int nButtons, int nModifier)
{
    SolarMutexGuard aGuard;

    // There seems to be no clear way of getting the grid window for this
    // particular document, hence we need to hope we get the right window.
    ScViewData* pViewData = ScDocShell::GetViewData();
    ScGridWindow* pGridWindow = pViewData->GetActiveWin();

    if (!pGridWindow)
        return;

    // update the aLogicMode in ScViewData to something predictable
    pViewData->SetZoom(Fraction(mnTilePixelWidth * TWIPS_PER_PIXEL, mnTileTwipWidth),
                       Fraction(mnTilePixelHeight * TWIPS_PER_PIXEL, mnTileTwipHeight), true);

    // check if user hit a chart which is being edited by him
    ScTabViewShell * pTabViewShell = pViewData->GetViewShell();
    LokChartHelper aChartHelper(pTabViewShell);
    if (aChartHelper.postMouseEvent(nType, nX, nY,
                                    nCount, nButtons, nModifier,
                                    pViewData->GetPPTX(), pViewData->GetPPTY()))
        return;

    // check if the user hit a chart which is being edited by someone else
    // and, if so, skip current mouse event
    if (nType != LOK_MOUSEEVENT_MOUSEMOVE)
    {
        if (LokChartHelper::HitAny(Point(nX, nY)))
            return;
    }

    // Calc operates in pixels...
    const Point aPos(nX * pViewData->GetPPTX(), nY * pViewData->GetPPTY());
    SfxLokHelper::postMouseEventAsync(pGridWindow, nType, aPos, nCount,
                                      MouseEventModifiers::SIMPLECLICK,
                                      nButtons, nModifier);
}

void ScModelObj::setTextSelection(int nType, int nX, int nY)
{
    SolarMutexGuard aGuard;
    ScViewData* pViewData = ScDocShell::GetViewData();
    ScTabViewShell* pViewShell = pViewData->GetViewShell();

    LokChartHelper aChartHelper(pViewShell);
    if (aChartHelper.setTextSelection(nType, nX, nY))
        return;

    ScInputHandler* pInputHandler = SC_MOD()->GetInputHdl(pViewShell);
    ScDrawView* pDrawView = pViewData->GetScDrawView();

    // update the aLogicMode in ScViewData to something predictable
    pViewData->SetZoom(Fraction(mnTilePixelWidth * TWIPS_PER_PIXEL, mnTileTwipWidth),
                       Fraction(mnTilePixelHeight * TWIPS_PER_PIXEL, mnTileTwipHeight), true);

    bool bHandled = false;

    if (pInputHandler && pInputHandler->IsInputMode())
    {
        // forwarding to editeng - we are editing the cell content
        EditView* pTableView = pInputHandler->GetTableView();
        assert(pTableView);

        Point aPoint(convertTwipToMm100(nX), convertTwipToMm100(nY));

        if (pTableView->GetOutputArea().IsInside(aPoint))
        {
            switch (nType)
            {
                case LOK_SETTEXTSELECTION_START:
                    pTableView->SetCursorLogicPosition(aPoint, /*bPoint=*/false, /*bClearMark=*/false);
                    break;
                case LOK_SETTEXTSELECTION_END:
                    pTableView->SetCursorLogicPosition(aPoint, /*bPoint=*/true, /*bClearMark=*/false);
                    break;
                case LOK_SETTEXTSELECTION_RESET:
                    pTableView->SetCursorLogicPosition(aPoint, /*bPoint=*/true, /*bClearMark=*/true);
                    break;
                default:
                    assert(false);
                    break;
            }
            bHandled = true;
        }
    }
    else if (pDrawView && pDrawView->IsTextEdit())
    {
        // forwarding to editeng - we are editing the text in shape
        OutlinerView* pOutlinerView = pDrawView->GetTextEditOutlinerView();
        EditView& rEditView = pOutlinerView->GetEditView();

        Point aPoint(convertTwipToMm100(nX), convertTwipToMm100(nY));
        switch (nType)
        {
            case LOK_SETTEXTSELECTION_START:
                rEditView.SetCursorLogicPosition(aPoint, /*bPoint=*/false, /*bClearMark=*/false);
                break;
            case LOK_SETTEXTSELECTION_END:
                rEditView.SetCursorLogicPosition(aPoint, /*bPoint=*/true, /*bClearMark=*/false);
                break;
            case LOK_SETTEXTSELECTION_RESET:
                rEditView.SetCursorLogicPosition(aPoint, /*bPoint=*/true, /*bClearMark=*/true);
                break;
            default:
                assert(false);
                break;
        }
        bHandled = true;
    }

    if (!bHandled)
    {
        // just update the cell selection
        ScGridWindow* pGridWindow = pViewData->GetActiveWin();
        if (!pGridWindow)
            return;

        // move the cell selection handles
        pGridWindow->SetCellSelectionPixel(nType, nX * pViewData->GetPPTX(), nY * pViewData->GetPPTY());
    }
}

OString ScModelObj::getTextSelection(const char* pMimeType, OString& rUsedMimeType)
{
    SolarMutexGuard aGuard;

    TransferableDataHelper aDataHelper;
    ScViewData* pViewData = ScDocShell::GetViewData();
    uno::Reference<datatransfer::XTransferable> xTransferable;

    if ( ScEditShell * pShell = dynamic_cast<ScEditShell*>( pViewData->GetViewShell()->GetViewFrame()->GetDispatcher()->GetShell(0) ) )
        xTransferable = pShell->GetEditView()->GetTransferable();
    else if ( nullptr != dynamic_cast<ScDrawTextObjectBar*>( pViewData->GetViewShell()->GetViewFrame()->GetDispatcher()->GetShell(0) ))
    {
        ScDrawView* pView = pViewData->GetScDrawView();
        OutlinerView* pOutView = pView->GetTextEditOutlinerView();
        if (pOutView)
            xTransferable = pOutView->GetEditView().GetTransferable();
    }
    else if ( ScDrawShell * pDrawShell = dynamic_cast<ScDrawShell*>( pViewData->GetViewShell()->GetViewFrame()->GetDispatcher()->GetShell(0) ) )
        xTransferable = pDrawShell->GetDrawView()->CopyToTransferable();
    else
    {
        ScTransferObj* pObj = pViewData->GetViewShell()->CopyToTransferable();
        xTransferable.set( pObj );
    }

    if (!xTransferable.is())
        xTransferable.set( aDataHelper.GetTransferable() );

    // Take care of UTF-8 text here.
    OString aMimeType(pMimeType);
    bool bConvert = false;
    sal_Int32 nIndex = 0;
    if (aMimeType.getToken(0, ';', nIndex) == "text/plain")
    {
        if (aMimeType.getToken(0, ';', nIndex) == "charset=utf-8")
        {
            aMimeType = "text/plain;charset=utf-16";
            bConvert = true;
        }
    }

    datatransfer::DataFlavor aFlavor;
    aFlavor.MimeType = OUString::fromUtf8(aMimeType.getStr());
    if (aMimeType == "text/plain;charset=utf-16")
        aFlavor.DataType = cppu::UnoType<OUString>::get();
    else
        aFlavor.DataType = cppu::UnoType< uno::Sequence<sal_Int8> >::get();

    if (!xTransferable.is() || !xTransferable->isDataFlavorSupported(aFlavor))
        return OString();

    uno::Any aAny;
    try
    {
        aAny = xTransferable->getTransferData(aFlavor);
    }
    catch (const datatransfer::UnsupportedFlavorException& e)
    {
        SAL_WARN("sc", "Caught " << e);
        return OString();
    }
    catch (const css::uno::Exception& e)
    {
        SAL_WARN("sc", "Caught " << e);
        return OString();
    }

    OString aRet;
    if (aFlavor.DataType == cppu::UnoType<OUString>::get())
    {
        OUString aString;
        aAny >>= aString;
        if (bConvert)
            aRet = OUStringToOString(aString, RTL_TEXTENCODING_UTF8);
        else
            aRet = OString(reinterpret_cast<const sal_Char *>(aString.getStr()), aString.getLength() * sizeof(sal_Unicode));
    }
    else
    {
        uno::Sequence<sal_Int8> aSequence;
        aAny >>= aSequence;
        aRet = OString(reinterpret_cast<sal_Char*>(aSequence.getArray()), aSequence.getLength());
    }

    rUsedMimeType = pMimeType;
    return aRet;
}

void ScModelObj::setGraphicSelection(int nType, int nX, int nY)
{
    SolarMutexGuard aGuard;

    // There seems to be no clear way of getting the grid window for this
    // particular document, hence we need to hope we get the right window.
    ScViewData* pViewData = ScDocShell::GetViewData();
    ScGridWindow* pGridWindow = pViewData->GetActiveWin();

    // update the aLogicMode in ScViewData to something predictable
    pViewData->SetZoom(Fraction(mnTilePixelWidth * TWIPS_PER_PIXEL, mnTileTwipWidth),
                       Fraction(mnTilePixelHeight * TWIPS_PER_PIXEL, mnTileTwipHeight), true);
    double fPPTX = pViewData->GetPPTX();
    double fPPTY = pViewData->GetPPTY();

    ScTabViewShell* pViewShell = pViewData->GetViewShell();
    LokChartHelper aChartHelper(pViewShell);
    if (aChartHelper.setGraphicSelection(nType, nX, nY, fPPTX, fPPTY))
        return;

    int nPixelX = nX * fPPTX;
    int nPixelY = nY * fPPTY;

    switch (nType)
    {
    case LOK_SETGRAPHICSELECTION_START:
        {
            MouseEvent aClickEvent(Point(nPixelX, nPixelY), 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT);
            pGridWindow->MouseButtonDown(aClickEvent);
            MouseEvent aMoveEvent(Point(nPixelX, nPixelY), 0, MouseEventModifiers::SIMPLEMOVE, MOUSE_LEFT);
            pGridWindow->MouseMove(aMoveEvent);
        }
        break;
    case LOK_SETGRAPHICSELECTION_END:
        {
            MouseEvent aMoveEvent(Point(nPixelX, nPixelY), 0, MouseEventModifiers::SIMPLEMOVE, MOUSE_LEFT);
            pGridWindow->MouseMove(aMoveEvent);
            MouseEvent aClickEvent(Point(nPixelX, nPixelY), 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT);
            pGridWindow->MouseButtonUp(aClickEvent);
        }
        break;
    default:
        assert(false);
        break;
    }
}

void ScModelObj::resetSelection()
{
    SolarMutexGuard aGuard;

    ScViewData* pViewData = ScDocShell::GetViewData();
    ScTabViewShell* pViewShell = pViewData->GetViewShell();

    // deselect the shapes & texts
    ScDrawView* pDrawView = pViewShell->GetScDrawView();
    if (pDrawView)
    {
        pDrawView->ScEndTextEdit();
        pDrawView->UnmarkAll();
    }
    else
        pViewShell->Unmark();

    // and hide the cell and text selection
    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_TEXT_SELECTION, "");
    SfxLokHelper::notifyOtherViews(pViewShell, LOK_CALLBACK_TEXT_VIEW_SELECTION, "selection", "");
}

void ScModelObj::setClipboard(const uno::Reference<datatransfer::clipboard::XClipboard>& xClipboard)
{
    SolarMutexGuard aGuard;

    ScViewData* pViewData = ScDocShell::GetViewData();
    if (!pViewData)
        return;

    pViewData->GetActiveWin()->SetClipboard(xClipboard);
}

bool ScModelObj::isMimeTypeSupported()
{
    SolarMutexGuard aGuard;

    ScViewData* pViewData = ScDocShell::GetViewData();
    if (!pViewData)
        return false;


    TransferableDataHelper aDataHelper(TransferableDataHelper::CreateFromSystemClipboard(pViewData->GetActiveWin()));
    return EditEngine::HasValidData(aDataHelper.GetTransferable());
}

void ScModelObj::setClientZoom(int /*nTilePixelWidth_*/, int /*nTilePixelHeight_*/, int /*nTileTwipWidth_*/, int /*nTileTwipHeight_*/)
{
    mnTilePixelWidth = 256;
    mnTilePixelHeight = 256;
    mnTileTwipWidth = mnTilePixelWidth * TWIPS_PER_PIXEL;
    mnTileTwipHeight = mnTilePixelHeight * TWIPS_PER_PIXEL;
}

OUString ScModelObj::getRowColumnHeaders(const tools::Rectangle& rRectangle)
{
    ScViewData* pViewData = ScDocShell::GetViewData();

    if (!pViewData)
        return OUString();

    // update the aLogicMode in ScViewData to something predictable
    pViewData->SetZoom(Fraction(mnTilePixelWidth * TWIPS_PER_PIXEL, mnTileTwipWidth),
                       Fraction(mnTilePixelHeight * TWIPS_PER_PIXEL, mnTileTwipHeight), true);

    ScTabView* pTabView = pViewData->GetView();
    if (!pTabView)
        return OUString();

    return pTabView->getRowColumnHeaders(rRectangle);
}

OString ScModelObj::getCellCursor( int nOutputWidth, int nOutputHeight,
                                   long nTileWidth, long nTileHeight )
{
    SolarMutexGuard aGuard;

    ScViewData* pViewData = ScDocShell::GetViewData();

    if (!pViewData)
        return OString();

    ScGridWindow* pGridWindow = pViewData->GetActiveWin();
    if (!pGridWindow)
        return OString();

    return "{ \"commandName\": \".uno:CellCursor\", \"commandValues\": \"" + pGridWindow->getCellCursor( nOutputWidth, nOutputHeight, nTileWidth, nTileHeight ) + "\" }";
}

PointerStyle ScModelObj::getPointer()
{
    SolarMutexGuard aGuard;

    ScViewData* pViewData = ScDocShell::GetViewData();
    if (!pViewData)
        return PointerStyle::Arrow;

    ScGridWindow* pGridWindow = pViewData->GetActiveWin();
    if (!pGridWindow)
        return PointerStyle::Arrow;

    return pGridWindow->GetPointer();
}

OUString ScModelObj::getTrackedChanges()
{
    OUString aRet;

    if (pDocShell)
    {
        if (ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack())
            aRet = pChangeTrack->GetChangeTrackInfo();
    }

    return aRet;
}

void ScModelObj::setClientVisibleArea(const tools::Rectangle& rRectangle)
{
    ScViewData* pViewData = ScDocShell::GetViewData();
    if (!pViewData)
        return;

    // set the PgUp/PgDown offset
    pViewData->ForcePageUpDownOffset(rRectangle.GetHeight());
}

void ScModelObj::setOutlineState(bool bColumn, int nLevel, int nIndex, bool bHidden)
{
    ScViewData* pViewData = ScDocShell::GetViewData();

    if (!pViewData)
        return;

    ScDBFunc* pFunc = pViewData->GetView();

    if (pFunc)
        pFunc->SetOutlineState(bColumn, nLevel, nIndex, bHidden);
}

OUString ScModelObj::getPostIts()
{
    if (!pDocShell)
        return OUString();

    ScDocument& rDoc = pDocShell->GetDocument();
    std::vector<sc::NoteEntry> aNotes;
    rDoc.GetAllNoteEntries(aNotes);

    boost::property_tree::ptree aAnnotations;
    for (const sc::NoteEntry& aNote : aNotes)
    {
        boost::property_tree::ptree aAnnotation;

        aAnnotation.put("id", aNote.mpNote->GetId());
        aAnnotation.put("tab", aNote.maPos.Tab());
        aAnnotation.put("author", aNote.mpNote->GetAuthor());
        aAnnotation.put("dateTime", aNote.mpNote->GetDate());
        aAnnotation.put("text", aNote.mpNote->GetText());

        // Calculating the cell cursor position
        ScViewData* pViewData = ScDocShell::GetViewData();
        ScGridWindow* pGridWindow = pViewData->GetActiveWin();
        if (pGridWindow)
        {
            SCCOL nX = aNote.maPos.Col();
            SCROW nY = aNote.maPos.Row();
            Point aScrPos = pViewData->GetScrPos(nX, nY, pViewData->GetActivePart(), true);
            long nSizeXPix;
            long nSizeYPix;
            pViewData->GetMergeSizePixel(nX, nY, nSizeXPix, nSizeYPix);

            double fPPTX = pViewData->GetPPTX();
            double fPPTY = pViewData->GetPPTY();
            tools::Rectangle aRect(Point(aScrPos.getX() / fPPTX, aScrPos.getY() / fPPTY),
                            Size(nSizeXPix / fPPTX, nSizeYPix / fPPTY));

            aAnnotation.put("cellPos", aRect.toString());
        }

        aAnnotations.push_back(std::make_pair("", aAnnotation));
    }

    boost::property_tree::ptree aTree;
    aTree.add_child("comments", aAnnotations);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);

    return OUString::fromUtf8(aStream.str().c_str());
}

OUString ScModelObj::getPostItsPos()
{
    if (!pDocShell)
        return OUString();

    ScDocument& rDoc = pDocShell->GetDocument();
    std::vector<sc::NoteEntry> aNotes;
    rDoc.GetAllNoteEntries(aNotes);

    boost::property_tree::ptree aAnnotations;
    for (const sc::NoteEntry& aNote : aNotes)
    {
        boost::property_tree::ptree aAnnotation;

        aAnnotation.put("id", aNote.mpNote->GetId());
        aAnnotation.put("tab", aNote.maPos.Tab());

        // Calculating the cell cursor position
        ScViewData* pViewData = ScDocShell::GetViewData();
        ScGridWindow* pGridWindow = pViewData->GetActiveWin();
        if (pGridWindow)
        {
            SCCOL nX = aNote.maPos.Col();
            SCROW nY = aNote.maPos.Row();
            Point aScrPos = pViewData->GetScrPos(nX, nY, pViewData->GetActivePart(), true);
            long nSizeXPix;
            long nSizeYPix;
            pViewData->GetMergeSizePixel(nX, nY, nSizeXPix, nSizeYPix);

            double fPPTX = pViewData->GetPPTX();
            double fPPTY = pViewData->GetPPTY();
            tools::Rectangle aRect(Point(aScrPos.getX() / fPPTX, aScrPos.getY() / fPPTY),
                            Size(nSizeXPix / fPPTX, nSizeYPix / fPPTY));

            aAnnotation.put("cellPos", aRect.toString());
        }

        aAnnotations.push_back(std::make_pair("", aAnnotation));
    }

    boost::property_tree::ptree aTree;
    aTree.add_child("commentsPos", aAnnotations);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);

    return OUString::fromUtf8(aStream.str().c_str());
}

void ScModelObj::initializeForTiledRendering(const css::uno::Sequence<css::beans::PropertyValue>& /*rArguments*/)
{
    SolarMutexGuard aGuard;

    // disable word autocompletion
    ScAppOptions aAppOptions( SC_MOD()->GetAppOptions() );
    aAppOptions.SetAutoComplete(false);
    SC_MOD()->SetAppOptions(aAppOptions);

    // show us the text exactly
    ScInputOptions aInputOptions(SC_MOD()->GetInputOptions());
    aInputOptions.SetTextWysiwyg(true);
    aInputOptions.SetReplaceCellsWarn(false);
    SC_MOD()->SetInputOptions(aInputOptions);
    pDocShell->CalcOutputFactor();

    // when the "This document may contain formatting or content that cannot
    // be saved..." dialog appears, it is auto-cancelled with tiled rendering,
    // causing 'Save' being disabled; so let's always save to the original
    // format
    SvtSaveOptions().SetWarnAlienFormat(false);

    // default tile size in pixels
    mnTilePixelWidth = 256;
    mnTilePixelHeight = 256;
    // the default zoom level will be 1
    mnTileTwipWidth = mnTilePixelWidth * TWIPS_PER_PIXEL;
    mnTileTwipHeight = mnTilePixelHeight * TWIPS_PER_PIXEL;
}

uno::Any SAL_CALL ScModelObj::queryInterface( const uno::Type& rType )
{
    SC_QUERYINTERFACE( sheet::XSpreadsheetDocument )
    SC_QUERYINTERFACE( document::XActionLockable )
    SC_QUERYINTERFACE( sheet::XCalculatable )
    SC_QUERYINTERFACE( util::XProtectable )
    SC_QUERYINTERFACE( drawing::XDrawPagesSupplier )
    SC_QUERYINTERFACE( sheet::XGoalSeek )
    SC_QUERYINTERFACE( sheet::XConsolidatable )
    SC_QUERYINTERFACE( sheet::XDocumentAuditing )
    SC_QUERYINTERFACE( style::XStyleFamiliesSupplier )
    SC_QUERYINTERFACE( view::XRenderable )
    SC_QUERYINTERFACE( document::XLinkTargetSupplier )
    SC_QUERYINTERFACE( beans::XPropertySet )
    SC_QUERYINTERFACE( lang::XMultiServiceFactory )
    SC_QUERYINTERFACE( lang::XServiceInfo )
    SC_QUERYINTERFACE( util::XChangesNotifier )
    SC_QUERYINTERFACE( sheet::opencl::XOpenCLSelection )
    SC_QUERYINTERFACE( chart2::XDataProviderAccess )

    uno::Any aRet(SfxBaseModel::queryInterface( rType ));
    if ( !aRet.hasValue()
        && rType != cppu::UnoType<css::document::XDocumentEventBroadcaster>::get()
        && rType != cppu::UnoType<css::frame::XController>::get()
        && rType != cppu::UnoType<css::frame::XFrame>::get()
        && rType != cppu::UnoType<css::script::XInvocation>::get()
        && rType != cppu::UnoType<css::beans::XFastPropertySet>::get()
        && rType != cppu::UnoType<css::awt::XWindow>::get())
    {
        GetFormatter();
        if ( xNumberAgg.is() )
            aRet = xNumberAgg->queryAggregation( rType );
    }

    return aRet;
}

void SAL_CALL ScModelObj::acquire() throw()
{
    SfxBaseModel::acquire();
}

void SAL_CALL ScModelObj::release() throw()
{
    SfxBaseModel::release();
}

uno::Sequence<uno::Type> SAL_CALL ScModelObj::getTypes()
{
    static const uno::Sequence<uno::Type> aTypes = [&]()
    {
        uno::Sequence<uno::Type> aAggTypes;
        if ( GetFormatter().is() )
        {
            const uno::Type& rProvType = cppu::UnoType<lang::XTypeProvider>::get();
            uno::Any aNumProv(xNumberAgg->queryAggregation(rProvType));
            if(auto xNumProv
               = o3tl::tryAccess<uno::Reference<lang::XTypeProvider>>(aNumProv))
            {
                aAggTypes = (*xNumProv)->getTypes();
            }
        }
        return comphelper::concatSequences(
            SfxBaseModel::getTypes(),
            aAggTypes,
            uno::Sequence<uno::Type>
            {
                cppu::UnoType<sheet::XSpreadsheetDocument>::get(),
                cppu::UnoType<document::XActionLockable>::get(),
                cppu::UnoType<sheet::XCalculatable>::get(),
                cppu::UnoType<util::XProtectable>::get(),
                cppu::UnoType<drawing::XDrawPagesSupplier>::get(),
                cppu::UnoType<sheet::XGoalSeek>::get(),
                cppu::UnoType<sheet::XConsolidatable>::get(),
                cppu::UnoType<sheet::XDocumentAuditing>::get(),
                cppu::UnoType<style::XStyleFamiliesSupplier>::get(),
                cppu::UnoType<view::XRenderable>::get(),
                cppu::UnoType<document::XLinkTargetSupplier>::get(),
                cppu::UnoType<beans::XPropertySet>::get(),
                cppu::UnoType<lang::XMultiServiceFactory>::get(),
                cppu::UnoType<lang::XServiceInfo>::get(),
                cppu::UnoType<util::XChangesNotifier>::get(),
                cppu::UnoType<sheet::opencl::XOpenCLSelection>::get(),
            } );
    }();
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScModelObj::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

void ScModelObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    //  Not interested in reference update hints here

    const SfxHintId nId = rHint.GetId();
    if ( nId == SfxHintId::Dying )
    {
        pDocShell = nullptr;       // has become invalid
        if (xNumberAgg.is())
        {
            SvNumberFormatsSupplierObj* pNumFmt =
                SvNumberFormatsSupplierObj::getImplementation(
                        uno::Reference<util::XNumberFormatsSupplier>(xNumberAgg, uno::UNO_QUERY) );
            if ( pNumFmt )
                pNumFmt->SetNumberFormatter( nullptr );
        }

        pPrintFuncCache.reset();     // must be deleted because it has a pointer to the DocShell
        m_pPrintState.reset();
    }
    else if ( nId == SfxHintId::DataChanged )
    {
        //  cached data for rendering become invalid when contents change
        //  (if a broadcast is added to SetDrawModified, is has to be tested here, too)

        pPrintFuncCache.reset();
        m_pPrintState.reset();

        // handle "OnCalculate" sheet events (search also for VBA event handlers)
        if ( pDocShell )
        {
            ScDocument& rDoc = pDocShell->GetDocument();
            if ( rDoc.GetVbaEventProcessor().is() )
            {
                // If the VBA event processor is set, HasAnyCalcNotification is much faster than HasAnySheetEventScript
                if ( rDoc.HasAnyCalcNotification() && rDoc.HasAnySheetEventScript( ScSheetEventId::CALCULATE, true ) )
                    HandleCalculateEvents();
            }
            else
            {
                if ( rDoc.HasAnySheetEventScript( ScSheetEventId::CALCULATE ) )
                    HandleCalculateEvents();
            }
        }
    }

    // always call parent - SfxBaseModel might need to handle the same hints again
    SfxBaseModel::Notify( rBC, rHint );     // SfxBaseModel is derived from SfxListener
}

// XSpreadsheetDocument

uno::Reference<sheet::XSpreadsheets> SAL_CALL ScModelObj::getSheets()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return new ScTableSheetsObj(pDocShell);
    return nullptr;
}

css::uno::Reference< ::css::chart2::data::XDataProvider > SAL_CALL ScModelObj::createDataProvider()
{
    if (pDocShell)
    {
        return css::uno::Reference< ::css::chart2::data::XDataProvider > (
            ScServiceProvider::MakeInstance(ScServiceProvider::Type::CHDATAPROV, pDocShell), uno::UNO_QUERY);
    }
    return nullptr;
}

// XStyleFamiliesSupplier

uno::Reference<container::XNameAccess> SAL_CALL ScModelObj::getStyleFamilies()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return new ScStyleFamiliesObj(pDocShell);
    return nullptr;
}

// XRenderable

static OutputDevice* lcl_GetRenderDevice( const uno::Sequence<beans::PropertyValue>& rOptions )
{
    OutputDevice* pRet = nullptr;
    const beans::PropertyValue* pPropArray = rOptions.getConstArray();
    const long nPropCount = rOptions.getLength();
    for (long i = 0; i < nPropCount; i++)
    {
        const beans::PropertyValue& rProp = pPropArray[i];
        const OUString & rPropName = rProp.Name;

        if (rPropName == SC_UNONAME_RENDERDEV)
        {
            uno::Reference<awt::XDevice> xRenderDevice(rProp.Value, uno::UNO_QUERY);
            if ( xRenderDevice.is() )
            {
                VCLXDevice* pDevice = VCLXDevice::GetImplementation( xRenderDevice );
                if ( pDevice )
                {
                    pRet = pDevice->GetOutputDevice().get();
                    pRet->SetDigitLanguage( SC_MOD()->GetOptDigitLanguage() );
                }
            }
        }
    }
    return pRet;
}

static bool lcl_ParseTarget( const OUString& rTarget, ScRange& rTargetRange, tools::Rectangle& rTargetRect,
                        bool& rIsSheet, ScDocument* pDoc, SCTAB nSourceTab )
{
    // test in same order as in SID_CURRENTCELL execute

    ScAddress aAddress;
    ScRangeUtil aRangeUtil;
    SCTAB nNameTab;
    sal_Int32 nNumeric = 0;

    bool bRangeValid = false;
    bool bRectValid = false;

    if ( rTargetRange.Parse( rTarget, pDoc ) & ScRefFlags::VALID )
    {
        bRangeValid = true;             // range reference
    }
    else if ( aAddress.Parse( rTarget, pDoc ) & ScRefFlags::VALID )
    {
        rTargetRange = aAddress;
        bRangeValid = true;             // cell reference
    }
    else if ( ScRangeUtil::MakeRangeFromName( rTarget, pDoc, nSourceTab, rTargetRange ) ||
              ScRangeUtil::MakeRangeFromName( rTarget, pDoc, nSourceTab, rTargetRange, RUTL_DBASE ) )
    {
        bRangeValid = true;             // named range or database range
    }
    else if ( comphelper::string::isdigitAsciiString(rTarget) &&
              ( nNumeric = rTarget.toInt32() ) > 0 && nNumeric <= MAXROW+1 )
    {
        // row number is always mapped to cell A(row) on the same sheet
        rTargetRange = ScAddress( 0, static_cast<SCROW>(nNumeric-1), nSourceTab );     // target row number is 1-based
        bRangeValid = true;             // row number
    }
    else if ( pDoc->GetTable( rTarget, nNameTab ) )
    {
        rTargetRange = ScAddress(0,0,nNameTab);
        bRangeValid = true;             // sheet name
        rIsSheet = true;                // needs special handling (first page of the sheet)
    }
    else
    {
        // look for named drawing object

        ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
        if ( pDrawLayer )
        {
            SCTAB nTabCount = pDoc->GetTableCount();
            for (SCTAB i=0; i<nTabCount && !bRangeValid; i++)
            {
                SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(i));
                OSL_ENSURE(pPage,"Page ?");
                if (pPage)
                {
                    SdrObjListIter aIter( pPage, SdrIterMode::DeepWithGroups );
                    SdrObject* pObject = aIter.Next();
                    while (pObject && !bRangeValid)
                    {
                        if ( ScDrawLayer::GetVisibleName( pObject ) == rTarget )
                        {
                            rTargetRect = pObject->GetLogicRect();              // 1/100th mm
                            rTargetRange = pDoc->GetRange( i, rTargetRect );    // underlying cells
                            bRangeValid = bRectValid = true;                    // rectangle is valid
                        }
                        pObject = aIter.Next();
                    }
                }
            }
        }
    }
    if ( bRangeValid && !bRectValid )
    {
        //  get rectangle for cell range
        rTargetRect = pDoc->GetMMRect( rTargetRange.aStart.Col(), rTargetRange.aStart.Row(),
                                       rTargetRange.aEnd.Col(),   rTargetRange.aEnd.Row(),
                                       rTargetRange.aStart.Tab() );
    }

    return bRangeValid;
}

bool ScModelObj::FillRenderMarkData( const uno::Any& aSelection,
                                     const uno::Sequence< beans::PropertyValue >& rOptions,
                                     ScMarkData& rMark,
                                     ScPrintSelectionStatus& rStatus, OUString& rPagesStr,
                                     bool& rbRenderToGraphic ) const
{
    OSL_ENSURE( !rMark.IsMarked() && !rMark.IsMultiMarked(), "FillRenderMarkData: MarkData must be empty" );
    OSL_ENSURE( pDocShell, "FillRenderMarkData: DocShell must be set" );

    bool bDone = false;

    uno::Reference<frame::XController> xView;

    // defaults when no options are passed: all sheets, include empty pages
    bool bSelectedSheetsOnly = false;
    bool bSuppressEmptyPages = true;

    bool bHasPrintContent = false;
    sal_Int32 nPrintContent = 0;        // all sheets / selected sheets / selected cells
    sal_Int32 nPrintRange = 0;          // all pages / pages / even pages / odd pages
    OUString aPageRange;           // "pages" edit value

    for( sal_Int32 i = 0, nLen = rOptions.getLength(); i < nLen; i++ )
    {
        if ( rOptions[i].Name == "IsOnlySelectedSheets" )
        {
            rOptions[i].Value >>= bSelectedSheetsOnly;
        }
        else if ( rOptions[i].Name == "IsSuppressEmptyPages" )
        {
            rOptions[i].Value >>= bSuppressEmptyPages;
        }
        else if ( rOptions[i].Name == "PageRange" )
        {
            rOptions[i].Value >>= aPageRange;
        }
        else if ( rOptions[i].Name == "PrintRange" )
        {
            rOptions[i].Value >>= nPrintRange;
        }
        else if ( rOptions[i].Name == "PrintContent" )
        {
            bHasPrintContent = true;
            rOptions[i].Value >>= nPrintContent;
        }
        else if ( rOptions[i].Name == "View" )
        {
            rOptions[i].Value >>= xView;
        }
        else if ( rOptions[i].Name == "RenderToGraphic" )
        {
            rOptions[i].Value >>= rbRenderToGraphic;
        }
    }

    // "Print Content" selection wins over "Selected Sheets" option
    if ( bHasPrintContent )
        bSelectedSheetsOnly = ( nPrintContent != 0 );

    uno::Reference<uno::XInterface> xInterface(aSelection, uno::UNO_QUERY);
    if ( xInterface.is() )
    {
        ScCellRangesBase* pSelObj = ScCellRangesBase::getImplementation( xInterface );
        uno::Reference< drawing::XShapes > xShapes( xInterface, uno::UNO_QUERY );
        if ( pSelObj && pSelObj->GetDocShell() == pDocShell )
        {
            bool bSheet = ( ScTableSheetObj::getImplementation( xInterface ) != nullptr );
            bool bCursor = pSelObj->IsCursorOnly();
            const ScRangeList& rRanges = pSelObj->GetRangeList();

            rMark.MarkFromRangeList( rRanges, false );
            rMark.MarkToSimple();

            if ( rMark.IsMultiMarked() )
            {
                // #i115266# copy behavior of old printing:
                // treat multiple selection like a single selection with the enclosing range
                ScRange aMultiMarkArea;
                rMark.GetMultiMarkArea( aMultiMarkArea );
                rMark.ResetMark();
                rMark.SetMarkArea( aMultiMarkArea );
            }

            if ( rMark.IsMarked() && !rMark.IsMultiMarked() )
            {
                // a sheet object is treated like an empty selection: print the used area of the sheet

                if ( bCursor || bSheet )                // nothing selected -> use whole tables
                {
                    rMark.ResetMark();      // doesn't change table selection
                    rStatus.SetMode( SC_PRINTSEL_CURSOR );
                }
                else
                    rStatus.SetMode( SC_PRINTSEL_RANGE );

                rStatus.SetRanges( rRanges );
                bDone = true;
            }
            // multi selection isn't supported
        }
        else if( xShapes.is() )
        {
            //print a selected ole object
            uno::Reference< container::XIndexAccess > xIndexAccess( xShapes, uno::UNO_QUERY );
            if( xIndexAccess.is() )
            {
                // multi selection isn't supported yet
                uno::Reference< drawing::XShape > xShape( xIndexAccess->getByIndex(0), uno::UNO_QUERY );
                SvxShape* pShape = SvxShape::getImplementation( xShape );
                if( pShape )
                {
                    SdrObject *pSdrObj = pShape->GetSdrObject();
                    if( pDocShell )
                    {
                        ScDocument& rDoc = pDocShell->GetDocument();
                        if( pSdrObj )
                        {
                            tools::Rectangle aObjRect = pSdrObj->GetCurrentBoundRect();
                            SCTAB nCurrentTab = ScDocShell::GetCurTab();
                            ScRange aRange = rDoc.GetRange( nCurrentTab, aObjRect );
                            rMark.SetMarkArea( aRange );

                            if( rMark.IsMarked() && !rMark.IsMultiMarked() )
                            {
                                rStatus.SetMode( SC_PRINTSEL_RANGE_EXCLUSIVELY_OLE_AND_DRAW_OBJECTS );
                                bDone = true;
                            }
                        }
                    }
                }
            }
        }
        else if ( ScModelObj::getImplementation( xInterface ) == this )
        {
            //  render the whole document
            //  -> no selection, all sheets

            SCTAB nTabCount = pDocShell->GetDocument().GetTableCount();
            for (SCTAB nTab = 0; nTab < nTabCount; nTab++)
                rMark.SelectTable( nTab, true );
            rStatus.SetMode( SC_PRINTSEL_DOCUMENT );
            bDone = true;
        }
        // other selection types aren't supported
    }

    // restrict to selected sheets if a view is available
    uno::Reference<sheet::XSelectedSheetsSupplier> xSelectedSheets(xView, uno::UNO_QUERY);
    if (bSelectedSheetsOnly && xSelectedSheets.is())
    {
        uno::Sequence<sal_Int32> aSelected = xSelectedSheets->getSelectedSheets();
        ScMarkData::MarkedTabsType aSelectedTabs;
        SCTAB nMaxTab = pDocShell->GetDocument().GetTableCount() -1;
        for (sal_Int32 i = 0, n = aSelected.getLength(); i < n; ++i)
        {
            SCTAB nSelected = static_cast<SCTAB>(aSelected[i]);
            if (ValidTab(nSelected, nMaxTab))
                aSelectedTabs.insert(static_cast<SCTAB>(aSelected[i]));
        }
        rMark.SetSelectedTabs(aSelectedTabs);
    }

    ScPrintOptions aNewOptions;
    aNewOptions.SetSkipEmpty( bSuppressEmptyPages );
    aNewOptions.SetAllSheets( !bSelectedSheetsOnly );
    rStatus.SetOptions( aNewOptions );

    // "PrintRange" enables (1) or disables (0) the "PageRange" edit
    if ( nPrintRange == 1 )
        rPagesStr = aPageRange;
    else
        rPagesStr.clear();

    return bDone;
}

sal_Int32 SAL_CALL ScModelObj::getRendererCount(const uno::Any& aSelection,
    const uno::Sequence<beans::PropertyValue>& rOptions)
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
    {
        throw lang::DisposedException( OUString(),
                static_cast< sheet::XSpreadsheetDocument* >(this) );
    }

    ScMarkData aMark;
    ScPrintSelectionStatus aStatus;
    OUString aPagesStr;
    bool bRenderToGraphic = false;
    if ( !FillRenderMarkData( aSelection, rOptions, aMark, aStatus, aPagesStr, bRenderToGraphic ) )
        return 0;

    //  The same ScPrintFuncCache object in pPrintFuncCache is used as long as
    //  the same selection is used (aStatus) and the document isn't changed
    //  (pPrintFuncCache is cleared in Notify handler)

    if ( !pPrintFuncCache || !pPrintFuncCache->IsSameSelection( aStatus ) )
    {
        pPrintFuncCache.reset(new ScPrintFuncCache( pDocShell, aMark, aStatus ));
    }
    sal_Int32 nPages = pPrintFuncCache->GetPageCount();

    m_pPrintState.reset();
    maValidPages.clear();

    sal_Int32 nContent = 0;
    for ( const auto& rValue : rOptions)
    {
        if ( rValue.Name == "PrintRange" )
        {
            rValue.Value >>= nContent;
            break;
        }
    }

    bool bIsPrintEvenPages = nContent != 3;
    bool bIsPrintOddPages = nContent != 2;

    for ( sal_Int32 nPage = 1; nPage <= nPages; nPage++ )
    {
        if ( (bIsPrintEvenPages && IsOnEvenPage( nPage )) || (bIsPrintOddPages && !IsOnEvenPage( nPage )) )
            maValidPages.push_back( nPage );
    }

    sal_Int32 nSelectCount = static_cast<sal_Int32>( maValidPages.size() );

    if ( nContent == 2 || nContent == 3 ) // even pages / odd pages
        return nSelectCount;

    if ( !aPagesStr.isEmpty() )
    {
        StringRangeEnumerator aRangeEnum( aPagesStr, 0, nPages-1 );
        nSelectCount = aRangeEnum.size();
    }
    return (nSelectCount > 0) ? nSelectCount : 1;
}

static sal_Int32 lcl_GetRendererNum( sal_Int32 nSelRenderer, const OUString& rPagesStr, sal_Int32 nTotalPages )
{
    if ( rPagesStr.isEmpty() )
        return nSelRenderer;

    StringRangeEnumerator aRangeEnum( rPagesStr, 0, nTotalPages-1 );
    StringRangeEnumerator::Iterator aIter = aRangeEnum.begin();
    StringRangeEnumerator::Iterator aEnd  = aRangeEnum.end();
    for ( ; nSelRenderer > 0 && aIter != aEnd; --nSelRenderer )
        ++aIter;

    return *aIter; // returns -1 if reached the end
}

static bool lcl_renderSelectionToGraphic( bool bRenderToGraphic, const ScPrintSelectionStatus& rStatus )
{
    return bRenderToGraphic && rStatus.GetMode() == SC_PRINTSEL_RANGE;
}

uno::Sequence<beans::PropertyValue> SAL_CALL ScModelObj::getRenderer( sal_Int32 nSelRenderer,
                                    const uno::Any& aSelection, const uno::Sequence<beans::PropertyValue>& rOptions  )
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
    {
        throw lang::DisposedException( OUString(),
                static_cast< sheet::XSpreadsheetDocument* >(this) );
    }

    ScMarkData aMark;
    ScPrintSelectionStatus aStatus;
    OUString aPagesStr;
    // #i115266# if FillRenderMarkData fails, keep nTotalPages at 0, but still handle getRenderer(0) below
    long nTotalPages = 0;
    bool bRenderToGraphic = false;
    if ( FillRenderMarkData( aSelection, rOptions, aMark, aStatus, aPagesStr, bRenderToGraphic ) )
    {
        if ( !pPrintFuncCache || !pPrintFuncCache->IsSameSelection( aStatus ) )
        {
            pPrintFuncCache.reset(new ScPrintFuncCache( pDocShell, aMark, aStatus ));
        }
        nTotalPages = pPrintFuncCache->GetPageCount();
    }
    sal_Int32 nRenderer = lcl_GetRendererNum( nSelRenderer, aPagesStr, nTotalPages );
    if ( nRenderer < 0 )
    {
        if ( nSelRenderer != 0 )
            throw lang::IllegalArgumentException();

        // getRenderer(0) is used to query the settings, so it must always return something

        awt::Size aPageSize;
        if (lcl_renderSelectionToGraphic( bRenderToGraphic, aStatus))
        {
            assert( aMark.IsMarked());
            ScRange aRange;
            aMark.GetMarkArea( aRange );
            tools::Rectangle aMMRect( pDocShell->GetDocument().GetMMRect(
                    aRange.aStart.Col(), aRange.aStart.Row(),
                    aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aStart.Tab()));
            aPageSize.Width = aMMRect.GetWidth();
            aPageSize.Height = aMMRect.GetHeight();
        }
        else
        {
            SCTAB const nCurTab = 0;      //! use current sheet from view?
            ScPrintFunc aDefaultFunc( pDocShell, pDocShell->GetPrinter(), nCurTab );
            Size aTwips = aDefaultFunc.GetPageSize();
            aPageSize.Width = TwipsToHMM( aTwips.Width());
            aPageSize.Height = TwipsToHMM( aTwips.Height());
        }

        uno::Sequence<beans::PropertyValue> aSequence( comphelper::InitPropertySequence({
            { SC_UNONAME_PAGESIZE, uno::Any(aPageSize) }
        }));

        if( ! pPrinterOptions )
            pPrinterOptions.reset(new ScPrintUIOptions);
        else
            pPrinterOptions->SetDefaults();
        pPrinterOptions->appendPrintUIOptions( aSequence );
        return aSequence;

    }

    //  printer is used as device (just for page layout), draw view is not needed

    SCTAB nTab;
    if ( !maValidPages.empty() )
        nTab = pPrintFuncCache->GetTabForPage( maValidPages.at( nRenderer )-1 );
    else
        nTab = pPrintFuncCache->GetTabForPage( nRenderer );


    ScRange aRange;
    const ScRange* pSelRange = nullptr;
    if ( aMark.IsMarked() )
    {
        aMark.GetMarkArea( aRange );
        pSelRange = &aRange;
    }

    awt::Size aPageSize;
    bool bWasCellRange = false;
    ScRange aCellRange;
    if (lcl_renderSelectionToGraphic( bRenderToGraphic, aStatus))
    {
        bWasCellRange = true;
        aCellRange = aRange;
        tools::Rectangle aMMRect( pDocShell->GetDocument().GetMMRect(
                    aRange.aStart.Col(), aRange.aStart.Row(),
                    aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aStart.Tab()));
        aPageSize.Width = aMMRect.GetWidth();
        aPageSize.Height = aMMRect.GetHeight();
    }
    else
    {
        std::unique_ptr<ScPrintFunc, o3tl::default_delete<ScPrintFunc>> pPrintFunc;
        if (m_pPrintState && m_pPrintState->nPrintTab == nTab)
            pPrintFunc.reset(new ScPrintFunc(pDocShell, pDocShell->GetPrinter(), *m_pPrintState, &aStatus.GetOptions()));
        else
            pPrintFunc.reset(new ScPrintFunc(pDocShell, pDocShell->GetPrinter(), nTab,
                                             pPrintFuncCache->GetFirstAttr(nTab), nTotalPages, pSelRange, &aStatus.GetOptions()));
        pPrintFunc->SetRenderFlag( true );

        sal_Int32 nContent = 0;
        for ( const auto& rValue : rOptions)
        {
            if ( rValue.Name == "PrintRange" )
            {
                rValue.Value >>= nContent;
                break;
            }
        }

        MultiSelection aPage;
        if ( nContent == 2 || nContent == 3 ) // even pages or odd pages
        {
            aPage.SetTotalRange( Range(0,RANGE_MAX) );
            aPage.Select( maValidPages.at( nRenderer ) );
        }
        else
        {
            aPage.SetTotalRange( Range(0,RANGE_MAX) );
            aPage.Select( nRenderer+1 );
        }

        long nDisplayStart = pPrintFuncCache->GetDisplayStart( nTab );
        long nTabStart = pPrintFuncCache->GetTabStart( nTab );

        (void)pPrintFunc->DoPrint( aPage, nTabStart, nDisplayStart, false, nullptr );

        bWasCellRange = pPrintFunc->GetLastSourceRange( aCellRange );
        Size aTwips = pPrintFunc->GetPageSize();

        if (!m_pPrintState)
        {
            m_pPrintState.reset(new ScPrintState());
            pPrintFunc->GetPrintState(*m_pPrintState, true);
        }

        aPageSize.Width = TwipsToHMM( aTwips.Width());
        aPageSize.Height = TwipsToHMM( aTwips.Height());
    }

    long nPropCount = bWasCellRange ? 5 : 4;
    uno::Sequence<beans::PropertyValue> aSequence(nPropCount);
    beans::PropertyValue* pArray = aSequence.getArray();
    pArray[0].Name = SC_UNONAME_PAGESIZE;
    pArray[0].Value <<= aPageSize;
    // #i111158# all positions are relative to the whole page, including non-printable area
    pArray[1].Name = SC_UNONAME_INC_NP_AREA;
    pArray[1].Value <<= true;
    if ( bWasCellRange )
    {
        table::CellRangeAddress aRangeAddress( nTab,
                        aCellRange.aStart.Col(), aCellRange.aStart.Row(),
                        aCellRange.aEnd.Col(), aCellRange.aEnd.Row() );
        tools::Rectangle aMMRect( pDocShell->GetDocument().GetMMRect(
                    aCellRange.aStart.Col(), aCellRange.aStart.Row(),
                    aCellRange.aEnd.Col(), aCellRange.aEnd.Row(), aCellRange.aStart.Tab()));

        awt::Size aCalcPageSize ( aMMRect.GetSize().Width(),  aMMRect.GetSize().Height() );
        awt::Point aCalcPagePos( aMMRect.getX(), aMMRect.getY() );

        pArray[2].Name = SC_UNONAME_SOURCERANGE;
        pArray[2].Value <<= aRangeAddress;
        pArray[3].Name = SC_UNONAME_CALCPAGESIZE;
        pArray[3].Value <<= aCalcPageSize;
        pArray[4].Name = SC_UNONAME_CALCPAGEPOS;
        pArray[4].Value <<= aCalcPagePos;
    }

    if( ! pPrinterOptions )
        pPrinterOptions.reset(new ScPrintUIOptions);
    else
        pPrinterOptions->SetDefaults();
    pPrinterOptions->appendPrintUIOptions( aSequence );
    return aSequence;
}

void SAL_CALL ScModelObj::render( sal_Int32 nSelRenderer, const uno::Any& aSelection,
                                    const uno::Sequence<beans::PropertyValue>& rOptions )
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
    {
        throw lang::DisposedException( OUString(),
                static_cast< sheet::XSpreadsheetDocument* >(this) );
    }

    ScMarkData aMark;
    ScPrintSelectionStatus aStatus;
    OUString aPagesStr;
    bool bRenderToGraphic = false;
    if ( !FillRenderMarkData( aSelection, rOptions, aMark, aStatus, aPagesStr, bRenderToGraphic ) )
        throw lang::IllegalArgumentException();

    if ( !pPrintFuncCache || !pPrintFuncCache->IsSameSelection( aStatus ) )
    {
        pPrintFuncCache.reset(new ScPrintFuncCache( pDocShell, aMark, aStatus ));
    }
    long nTotalPages = pPrintFuncCache->GetPageCount();
    sal_Int32 nRenderer = lcl_GetRendererNum( nSelRenderer, aPagesStr, nTotalPages );
    if ( nRenderer < 0 )
        throw lang::IllegalArgumentException();

    OutputDevice* pDev = lcl_GetRenderDevice( rOptions );
    if ( !pDev )
        throw lang::IllegalArgumentException();

    ScDocument& rDoc = pDocShell->GetDocument();

    ScRange aRange;
    const ScRange* pSelRange = nullptr;
    if ( aMark.IsMarked() )
    {
        aMark.GetMarkArea( aRange );
        pSelRange = &aRange;
    }

    if (lcl_renderSelectionToGraphic( bRenderToGraphic, aStatus))
    {
        // Similar to as in and when calling ScTransferObj::PaintToDev()

        tools::Rectangle aBound( Point(), pDev->GetOutputSize());

        ScViewData aViewData(nullptr,nullptr);
        aViewData.InitData( &rDoc );

        aViewData.SetTabNo( aRange.aStart.Tab() );
        aViewData.SetScreen( aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(), aRange.aEnd.Row() );

        const double nPrintFactor = 1.0;    /* XXX: currently (2017-08-28) is not evaluated */
        // The bMetaFile argument maybe could be
        // pDev->GetConnectMetaFile() != nullptr
        // but for some yet unknown reason does not draw cell content if true.
        ScPrintFunc::DrawToDev( &rDoc, pDev, nPrintFactor, aBound, &aViewData, false /*bMetaFile*/ );

        return;
    }

    struct DrawViewKeeper
    {
        std::unique_ptr<FmFormView> mpDrawView;
        DrawViewKeeper() {}
        ~DrawViewKeeper()
        {
            if (mpDrawView)
            {
                mpDrawView->HideSdrPage();
                mpDrawView.reset();
            }
        }
    } aDrawViewKeeper;

    SCTAB nTab;
    if ( !maValidPages.empty() )
        nTab = pPrintFuncCache->GetTabForPage( maValidPages.at( nRenderer )-1 );
    else
        nTab = pPrintFuncCache->GetTabForPage( nRenderer );

    ScDrawLayer* pModel = rDoc.GetDrawLayer();

    if( pModel )
    {
        aDrawViewKeeper.mpDrawView.reset( new FmFormView(
            *pModel,
            pDev) );
        aDrawViewKeeper.mpDrawView->ShowSdrPage(aDrawViewKeeper.mpDrawView->GetModel()->GetPage(nTab));
        aDrawViewKeeper.mpDrawView->SetPrintPreview();
    }

    //  to increase performance, ScPrintState might be used here for subsequent
    //  pages of the same sheet


    std::unique_ptr<ScPrintFunc, o3tl::default_delete<ScPrintFunc>> pPrintFunc;
    if (m_pPrintState && m_pPrintState->nPrintTab == nTab
        && ! pSelRange) // tdf#120161 use selection to set required printed area
        pPrintFunc.reset(new ScPrintFunc(pDev, pDocShell, *m_pPrintState, &aStatus.GetOptions()));
    else
        pPrintFunc.reset(new ScPrintFunc(pDev, pDocShell, nTab, pPrintFuncCache->GetFirstAttr(nTab), nTotalPages, pSelRange, &aStatus.GetOptions()));

    pPrintFunc->SetDrawView( aDrawViewKeeper.mpDrawView.get() );
    pPrintFunc->SetRenderFlag( true );
    if( aStatus.GetMode() == SC_PRINTSEL_RANGE_EXCLUSIVELY_OLE_AND_DRAW_OBJECTS )
        pPrintFunc->SetExclusivelyDrawOleAndDrawObjects();

    sal_Int32 nContent = 0;
    for ( const auto& rValue : rOptions)
    {
        if ( rValue.Name == "PrintRange" )
        {
            rValue.Value >>= nContent;
            break;
        }
    }

    MultiSelection aPage;
    if ( nContent == 2 || nContent == 3 ) // even pages or odd pages
    {
        aPage.SetTotalRange( Range(0,RANGE_MAX) );
        aPage.Select( maValidPages.at( nRenderer ) );
    }
    else
    {
        aPage.SetTotalRange( Range(0,RANGE_MAX) );
        aPage.Select( nRenderer+1 );
    }

    long nDisplayStart = pPrintFuncCache->GetDisplayStart( nTab );
    long nTabStart = pPrintFuncCache->GetTabStart( nTab );

    vcl::PDFExtOutDevData* pPDFData = dynamic_cast< vcl::PDFExtOutDevData* >(pDev->GetExtOutDevData() );
    if ( nRenderer == nTabStart )
    {
        // first page of a sheet: add outline item for the sheet name

        if ( pPDFData && pPDFData->GetIsExportBookmarks() )
        {
            // the sheet starts at the top of the page
            tools::Rectangle aArea( pDev->PixelToLogic( tools::Rectangle( 0,0,0,0 ) ) );
            sal_Int32 nDestID = pPDFData->CreateDest( aArea );
            OUString aTabName;
            rDoc.GetName( nTab, aTabName );
            // top-level
            pPDFData->CreateOutlineItem( -1/*nParent*/, aTabName, nDestID );
        }
        // #i56629# add the named destination stuff
        if( pPDFData && pPDFData->GetIsExportNamedDestinations() )
        {
            tools::Rectangle aArea( pDev->PixelToLogic( tools::Rectangle( 0,0,0,0 ) ) );
            OUString aTabName;
            rDoc.GetName( nTab, aTabName );
            //need the PDF page number here
            pPDFData->CreateNamedDest( aTabName, aArea );
        }
    }

    (void)pPrintFunc->DoPrint( aPage, nTabStart, nDisplayStart, true, nullptr );

    if (!m_pPrintState)
    {
        m_pPrintState.reset(new ScPrintState());
        pPrintFunc->GetPrintState(*m_pPrintState, true);
    }

    //  resolve the hyperlinks for PDF export

    if ( pPDFData && !pPDFData->GetBookmarks().empty() )
    {
        //  iterate over the hyperlinks that were output for this page

        std::vector< vcl::PDFExtOutDevBookmarkEntry >& rBookmarks = pPDFData->GetBookmarks();
        for ( const auto& rBookmark : rBookmarks )
        {
            OUString aBookmark = rBookmark.aBookmark;
            if ( aBookmark.toChar() == '#' )
            {
                //  try to resolve internal link

                OUString aTarget( aBookmark.copy( 1 ) );

                ScRange aTargetRange;
                tools::Rectangle aTargetRect;      // 1/100th mm
                bool bIsSheet = false;
                bool bValid = lcl_ParseTarget( aTarget, aTargetRange, aTargetRect, bIsSheet, &rDoc, nTab );

                if ( bValid )
                {
                    sal_Int32 nPage = -1;
                    tools::Rectangle aArea;
                    if ( bIsSheet )
                    {
                        //  Get first page for sheet (if nothing from that sheet is printed,
                        //  this page can show a different sheet)
                        nPage = pPrintFuncCache->GetTabStart( aTargetRange.aStart.Tab() );
                        aArea = pDev->PixelToLogic( tools::Rectangle( 0,0,0,0 ) );
                    }
                    else
                    {
                        pPrintFuncCache->InitLocations( aMark, pDev );      // does nothing if already initialized

                        ScPrintPageLocation aLocation;
                        if ( pPrintFuncCache->FindLocation( aTargetRange.aStart, aLocation ) )
                        {
                            nPage = aLocation.nPage;

                            // get the rectangle of the page's cell range in 1/100th mm
                            ScRange aLocRange = aLocation.aCellRange;
                            tools::Rectangle aLocationMM = rDoc.GetMMRect(
                                       aLocRange.aStart.Col(), aLocRange.aStart.Row(),
                                       aLocRange.aEnd.Col(),   aLocRange.aEnd.Row(),
                                       aLocRange.aStart.Tab() );
                            tools::Rectangle aLocationPixel = aLocation.aRectangle;

                            // Scale and move the target rectangle from aLocationMM to aLocationPixel,
                            // to get the target rectangle in pixels.

                            Fraction aScaleX( aLocationPixel.GetWidth(), aLocationMM.GetWidth() );
                            Fraction aScaleY( aLocationPixel.GetHeight(), aLocationMM.GetHeight() );

                            long nX1 = aLocationPixel.Left() + static_cast<long>( Fraction( aTargetRect.Left() - aLocationMM.Left(), 1 ) * aScaleX );
                            long nX2 = aLocationPixel.Left() + static_cast<long>( Fraction( aTargetRect.Right() - aLocationMM.Left(), 1 ) * aScaleX );
                            long nY1 = aLocationPixel.Top() + static_cast<long>( Fraction( aTargetRect.Top() - aLocationMM.Top(), 1 ) * aScaleY );
                            long nY2 = aLocationPixel.Top() + static_cast<long>( Fraction( aTargetRect.Bottom() - aLocationMM.Top(), 1 ) * aScaleY );

                            if ( nX1 > aLocationPixel.Right() ) nX1 = aLocationPixel.Right();
                            if ( nX2 > aLocationPixel.Right() ) nX2 = aLocationPixel.Right();
                            if ( nY1 > aLocationPixel.Bottom() ) nY1 = aLocationPixel.Bottom();
                            if ( nY2 > aLocationPixel.Bottom() ) nY2 = aLocationPixel.Bottom();

                            // The link target area is interpreted using the device's MapMode at
                            // the time of the CreateDest call, so PixelToLogic can be used here,
                            // regardless of the MapMode that is actually selected.

                            aArea = pDev->PixelToLogic( tools::Rectangle( nX1, nY1, nX2, nY2 ) );
                        }
                    }

                    if ( nPage >= 0 )
                        pPDFData->SetLinkDest( rBookmark.nLinkId, pPDFData->CreateDest( aArea, nPage ) );
                }
            }
            else
            {
                //  external link, use as-is
                pPDFData->SetLinkURL( rBookmark.nLinkId, aBookmark );
            }
        }
        rBookmarks.clear();
    }
}

// XLinkTargetSupplier

uno::Reference<container::XNameAccess> SAL_CALL ScModelObj::getLinks()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return new ScLinkTargetTypesObj(pDocShell);
    return nullptr;
}

// XActionLockable

sal_Bool SAL_CALL ScModelObj::isActionLocked()
{
    SolarMutexGuard aGuard;
    bool bLocked = false;
    if (pDocShell)
        bLocked = ( pDocShell->GetLockCount() != 0 );
    return bLocked;
}

void SAL_CALL ScModelObj::addActionLock()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        pDocShell->LockDocument();
}

void SAL_CALL ScModelObj::removeActionLock()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        pDocShell->UnlockDocument();
}

void SAL_CALL ScModelObj::setActionLocks( sal_Int16 nLock )
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        pDocShell->SetLockCount(nLock);
}

sal_Int16 SAL_CALL ScModelObj::resetActionLocks()
{
    SolarMutexGuard aGuard;
    sal_uInt16 nRet = 0;
    if (pDocShell)
    {
        nRet = pDocShell->GetLockCount();
        pDocShell->SetLockCount(0);
    }
    return nRet;
}

void SAL_CALL ScModelObj::lockControllers()
{
    SolarMutexGuard aGuard;
    SfxBaseModel::lockControllers();
    if (pDocShell)
        pDocShell->LockPaint();
}

void SAL_CALL ScModelObj::unlockControllers()
{
    SolarMutexGuard aGuard;
    if (hasControllersLocked())
    {
        SfxBaseModel::unlockControllers();
        if (pDocShell)
            pDocShell->UnlockPaint();
    }
}

// XCalculate

void SAL_CALL ScModelObj::calculate()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        comphelper::ProfileZone aZone("calculate");
        pDocShell->DoRecalc(true);
    }
    else
    {
        OSL_FAIL("no DocShell");     //! throw exception?
    }
}

void SAL_CALL ScModelObj::calculateAll()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        comphelper::ProfileZone aZone("calculateAll");
        pDocShell->DoHardRecalc();
    }
    else
    {
        OSL_FAIL("no DocShell");     //! throw exception?
    }
}

sal_Bool SAL_CALL ScModelObj::isAutomaticCalculationEnabled()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return pDocShell->GetDocument().GetAutoCalc();

    OSL_FAIL("no DocShell");     //! throw exception?
    return false;
}

void SAL_CALL ScModelObj::enableAutomaticCalculation( sal_Bool bEnabledIn )
{
    bool bEnabled(bEnabledIn);
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        if ( rDoc.GetAutoCalc() != bEnabled )
        {
            rDoc.SetAutoCalc( bEnabled );
            pDocShell->SetDocumentModified();
        }
    }
    else
    {
        OSL_FAIL("no DocShell");     //! throw exception?
    }
}

// XProtectable

void SAL_CALL ScModelObj::protect( const OUString& aPassword )
{
    SolarMutexGuard aGuard;
    // #i108245# if already protected, don't change anything
    if ( pDocShell && !pDocShell->GetDocument().IsDocProtected() )
    {
        pDocShell->GetDocFunc().Protect( TABLEID_DOC, aPassword );
    }
}

void SAL_CALL ScModelObj::unprotect( const OUString& aPassword )
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        bool bDone = pDocShell->GetDocFunc().Unprotect( TABLEID_DOC, aPassword, true );
        if (!bDone)
            throw lang::IllegalArgumentException();
    }
}

sal_Bool SAL_CALL ScModelObj::isProtected()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return pDocShell->GetDocument().IsDocProtected();

    OSL_FAIL("no DocShell");     //! throw exception?
    return false;
}

// XDrawPagesSupplier

uno::Reference<drawing::XDrawPages> SAL_CALL ScModelObj::getDrawPages()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return new ScDrawPagesObj(pDocShell);

    OSL_FAIL("no DocShell");     //! throw exception?
    return nullptr;
}

// XGoalSeek

sheet::GoalResult SAL_CALL ScModelObj::seekGoal(
                                const table::CellAddress& aFormulaPosition,
                                const table::CellAddress& aVariablePosition,
                                const OUString& aGoalValue )
{
    SolarMutexGuard aGuard;
    sheet::GoalResult aResult;
    aResult.Divergence = DBL_MAX;       // not found
    if (pDocShell)
    {
        WaitObject aWait( ScDocShell::GetActiveDialogParent() );
        ScDocument& rDoc = pDocShell->GetDocument();
        double fValue = 0.0;
        bool bFound = rDoc.Solver(
                    static_cast<SCCOL>(aFormulaPosition.Column), static_cast<SCROW>(aFormulaPosition.Row), aFormulaPosition.Sheet,
                    static_cast<SCCOL>(aVariablePosition.Column), static_cast<SCROW>(aVariablePosition.Row), aVariablePosition.Sheet,
                    aGoalValue, fValue );
        aResult.Result = fValue;
        if (bFound)
            aResult.Divergence = 0.0;   //! this is a lie
    }
    return aResult;
}

// XConsolidatable

uno::Reference<sheet::XConsolidationDescriptor> SAL_CALL ScModelObj::createConsolidationDescriptor(
                                sal_Bool bEmpty )
{
    SolarMutexGuard aGuard;
    ScConsolidationDescriptor* pNew = new ScConsolidationDescriptor;
    if ( pDocShell && !bEmpty )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        const ScConsolidateParam* pParam = rDoc.GetConsolidateDlgData();
        if (pParam)
            pNew->SetParam( *pParam );
    }
    return pNew;
}

void SAL_CALL ScModelObj::consolidate(
    const uno::Reference<sheet::XConsolidationDescriptor>& xDescriptor )
{
    SolarMutexGuard aGuard;
    //  in theory, this could also be a different object, so use only
    //  public XConsolidationDescriptor interface to copy the data into
    //  ScConsolidationDescriptor object
    //! but if this already is ScConsolidationDescriptor, do it directly via getImplementation?

    rtl::Reference< ScConsolidationDescriptor > xImpl(new ScConsolidationDescriptor);
    xImpl->setFunction( xDescriptor->getFunction() );
    xImpl->setSources( xDescriptor->getSources() );
    xImpl->setStartOutputPosition( xDescriptor->getStartOutputPosition() );
    xImpl->setUseColumnHeaders( xDescriptor->getUseColumnHeaders() );
    xImpl->setUseRowHeaders( xDescriptor->getUseRowHeaders() );
    xImpl->setInsertLinks( xDescriptor->getInsertLinks() );

    if (pDocShell)
    {
        const ScConsolidateParam& rParam = xImpl->GetParam();
        pDocShell->DoConsolidate( rParam );
        pDocShell->GetDocument().SetConsolidateDlgData( std::unique_ptr<ScConsolidateParam>(new ScConsolidateParam(rParam)) );
    }
}

// XDocumentAuditing

void SAL_CALL ScModelObj::refreshArrows()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        pDocShell->GetDocFunc().DetectiveRefresh();
}

// XViewDataSupplier
uno::Reference< container::XIndexAccess > SAL_CALL ScModelObj::getViewData(  )
{
    uno::Reference < container::XIndexAccess > xRet( SfxBaseModel::getViewData() );

    if( !xRet.is() )
    {
        SolarMutexGuard aGuard;
        if (pDocShell && pDocShell->GetCreateMode() == SfxObjectCreateMode::EMBEDDED)
        {
            uno::Reference < container::XIndexContainer > xCont = document::IndexedPropertyValues::create( ::comphelper::getProcessComponentContext() );
            xRet.set( xCont, uno::UNO_QUERY_THROW );

            uno::Sequence< beans::PropertyValue > aSeq;
            aSeq.realloc(3);
            OUString sName;
            pDocShell->GetDocument().GetName( pDocShell->GetDocument().GetVisibleTab(), sName );
            OUString sOUName(sName);
            aSeq[0].Name = SC_ACTIVETABLE;
            aSeq[0].Value <<= sOUName;
            SCCOL nPosLeft = pDocShell->GetDocument().GetPosLeft();
            aSeq[1].Name = SC_POSITIONLEFT;
            aSeq[1].Value <<= nPosLeft;
            SCROW nPosTop = pDocShell->GetDocument().GetPosTop();
            aSeq[2].Name = SC_POSITIONTOP;
            aSeq[2].Value <<= nPosTop;
            xCont->insertByIndex( 0, uno::makeAny( aSeq ) );
        }
    }

    return xRet;
}

//  XPropertySet (Doc-Options)
//! provide them also to the application?

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScModelObj::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScModelObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;

    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        const ScDocOptions& rOldOpt = rDoc.GetDocOptions();
        ScDocOptions aNewOpt = rOldOpt;
        //  Don't recalculate while loading XML, when the formula text is stored
        //  Recalculation after loading is handled separately.
        bool bHardRecalc = !rDoc.IsImportingXML();

        bool bOpt = ScDocOptionsHelper::setPropertyValue( aNewOpt, aPropSet.getPropertyMap(), aPropertyName, aValue );
        if (bOpt)
        {
            // done...
            if ( aPropertyName == SC_UNO_IGNORECASE ||
                 aPropertyName == SC_UNONAME_REGEXP ||
                 aPropertyName == SC_UNONAME_WILDCARDS ||
                 aPropertyName == SC_UNO_LOOKUPLABELS )
                bHardRecalc = false;
        }
        else if ( aPropertyName == SC_UNONAME_CLOCAL )
        {
            lang::Locale aLocale;
            if ( aValue >>= aLocale )
            {
                LanguageType eLatin, eCjk, eCtl;
                rDoc.GetLanguage( eLatin, eCjk, eCtl );
                eLatin = ScUnoConversion::GetLanguage(aLocale);
                rDoc.SetLanguage( eLatin, eCjk, eCtl );
            }
        }
        else if ( aPropertyName == SC_UNO_CODENAME )
        {
            OUString sCodeName;
            if ( aValue >>= sCodeName )
                rDoc.SetCodeName( sCodeName );
        }
        else if ( aPropertyName == SC_UNO_CJK_CLOCAL )
        {
            lang::Locale aLocale;
            if ( aValue >>= aLocale )
            {
                LanguageType eLatin, eCjk, eCtl;
                rDoc.GetLanguage( eLatin, eCjk, eCtl );
                eCjk = ScUnoConversion::GetLanguage(aLocale);
                rDoc.SetLanguage( eLatin, eCjk, eCtl );
            }
        }
        else if ( aPropertyName == SC_UNO_CTL_CLOCAL )
        {
            lang::Locale aLocale;
            if ( aValue >>= aLocale )
            {
                LanguageType eLatin, eCjk, eCtl;
                rDoc.GetLanguage( eLatin, eCjk, eCtl );
                eCtl = ScUnoConversion::GetLanguage(aLocale);
                rDoc.SetLanguage( eLatin, eCjk, eCtl );
            }
        }
        else if ( aPropertyName == SC_UNO_APPLYFMDES )
        {
            //  model is created if not there
            ScDrawLayer* pModel = pDocShell->MakeDrawLayer();
            pModel->SetOpenInDesignMode( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );

            SfxBindings* pBindings = pDocShell->GetViewBindings();
            if (pBindings)
                pBindings->Invalidate( SID_FM_OPEN_READONLY );
        }
        else if ( aPropertyName == SC_UNO_AUTOCONTFOC )
        {
            //  model is created if not there
            ScDrawLayer* pModel = pDocShell->MakeDrawLayer();
            pModel->SetAutoControlFocus( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );

            SfxBindings* pBindings = pDocShell->GetViewBindings();
            if (pBindings)
                pBindings->Invalidate( SID_FM_AUTOCONTROLFOCUS );
        }
        else if ( aPropertyName == SC_UNO_ISLOADED )
        {
            pDocShell->SetEmpty( !ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        }
        else if ( aPropertyName == SC_UNO_ISUNDOENABLED )
        {
            bool bUndoEnabled = ScUnoHelpFunctions::GetBoolFromAny( aValue );
            rDoc.EnableUndo( bUndoEnabled );
            pDocShell->GetUndoManager()->SetMaxUndoActionCount(
                bUndoEnabled
                ? officecfg::Office::Common::Undo::Steps::get() : 0);
        }
        else if ( aPropertyName == SC_UNO_RECORDCHANGES )
        {
            bool bRecordChangesEnabled = ScUnoHelpFunctions::GetBoolFromAny( aValue );

            bool bChangeAllowed = true;
            if (!bRecordChangesEnabled)
                bChangeAllowed = !pDocShell->HasChangeRecordProtection();

            if (bChangeAllowed)
                pDocShell->SetChangeRecording(bRecordChangesEnabled);
        }
        else if ( aPropertyName == SC_UNO_ISADJUSTHEIGHTENABLED )
        {
            if( ScUnoHelpFunctions::GetBoolFromAny( aValue ) )
                rDoc.UnlockAdjustHeight();
            else
                rDoc.LockAdjustHeight();
        }
        else if ( aPropertyName == SC_UNO_ISEXECUTELINKENABLED )
        {
            rDoc.EnableExecuteLink( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        }
        else if ( aPropertyName == SC_UNO_ISCHANGEREADONLYENABLED )
        {
            rDoc.EnableChangeReadOnly( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        }
        else if ( aPropertyName == "BuildId" )
        {
            aValue >>= maBuildId;
        }
        else if ( aPropertyName == "SavedObject" )    // set from chart after saving
        {
            OUString aObjName;
            aValue >>= aObjName;
            if ( !aObjName.isEmpty() )
                rDoc.RestoreChartListener( aObjName );
        }
        else if ( aPropertyName == SC_UNO_INTEROPGRABBAG )
        {
            setGrabBagItem(aValue);
        }

        if ( aNewOpt != rOldOpt )
        {
            rDoc.SetDocOptions( aNewOpt );
            //! Recalc only for options that need it?
            if ( bHardRecalc )
                pDocShell->DoHardRecalc();
            pDocShell->SetDocumentModified();
        }
    }
}

uno::Any SAL_CALL ScModelObj::getPropertyValue( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;

    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        const ScDocOptions& rOpt = rDoc.GetDocOptions();
        aRet = ScDocOptionsHelper::getPropertyValue( rOpt, aPropSet.getPropertyMap(), aPropertyName );
        if ( aRet.hasValue() )
        {
            // done...
        }
        else if ( aPropertyName == SC_UNONAME_CLOCAL )
        {
            LanguageType eLatin, eCjk, eCtl;
            rDoc.GetLanguage( eLatin, eCjk, eCtl );

            lang::Locale aLocale;
            ScUnoConversion::FillLocale( aLocale, eLatin );
            aRet <<= aLocale;
        }
        else if ( aPropertyName == SC_UNO_CODENAME )
        {
            aRet <<= rDoc.GetCodeName();
        }

        else if ( aPropertyName == SC_UNO_CJK_CLOCAL )
        {
            LanguageType eLatin, eCjk, eCtl;
            rDoc.GetLanguage( eLatin, eCjk, eCtl );

            lang::Locale aLocale;
            ScUnoConversion::FillLocale( aLocale, eCjk );
            aRet <<= aLocale;
        }
        else if ( aPropertyName == SC_UNO_CTL_CLOCAL )
        {
            LanguageType eLatin, eCjk, eCtl;
            rDoc.GetLanguage( eLatin, eCjk, eCtl );

            lang::Locale aLocale;
            ScUnoConversion::FillLocale( aLocale, eCtl );
            aRet <<= aLocale;
        }
        else if ( aPropertyName == SC_UNO_NAMEDRANGES )
        {
            aRet <<= uno::Reference<sheet::XNamedRanges>(new ScGlobalNamedRangesObj( pDocShell ));
        }
        else if ( aPropertyName == SC_UNO_DATABASERNG )
        {
            aRet <<= uno::Reference<sheet::XDatabaseRanges>(new ScDatabaseRangesObj( pDocShell ));
        }
        else if ( aPropertyName == SC_UNO_UNNAMEDDBRNG )
        {
            aRet <<= uno::Reference<sheet::XUnnamedDatabaseRanges>(new ScUnnamedDatabaseRangesObj(pDocShell));
        }
        else if ( aPropertyName == SC_UNO_COLLABELRNG )
        {
            aRet <<= uno::Reference<sheet::XLabelRanges>(new ScLabelRangesObj( pDocShell, true ));
        }
        else if ( aPropertyName == SC_UNO_ROWLABELRNG )
        {
            aRet <<= uno::Reference<sheet::XLabelRanges>(new ScLabelRangesObj( pDocShell, false ));
        }
        else if ( aPropertyName == SC_UNO_AREALINKS )
        {
            aRet <<= uno::Reference<sheet::XAreaLinks>(new ScAreaLinksObj( pDocShell ));
        }
        else if ( aPropertyName == SC_UNO_DDELINKS )
        {
            aRet <<= uno::Reference<container::XNameAccess>(new ScDDELinksObj( pDocShell ));
        }
        else if ( aPropertyName == SC_UNO_EXTERNALDOCLINKS )
        {
            aRet <<= uno::Reference<sheet::XExternalDocLinks>(new ScExternalDocLinksObj(pDocShell));
        }
        else if ( aPropertyName == SC_UNO_SHEETLINKS )
        {
            aRet <<= uno::Reference<container::XNameAccess>(new ScSheetLinksObj( pDocShell ));
        }
        else if ( aPropertyName == SC_UNO_APPLYFMDES )
        {
            // default for no model is TRUE
            ScDrawLayer* pModel = rDoc.GetDrawLayer();
            bool bOpenInDesign = pModel == nullptr || pModel->GetOpenInDesignMode();
            aRet <<= bOpenInDesign;
        }
        else if ( aPropertyName == SC_UNO_AUTOCONTFOC )
        {
            // default for no model is FALSE
            ScDrawLayer* pModel = rDoc.GetDrawLayer();
            bool bAutoControlFocus = pModel && pModel->GetAutoControlFocus();
            aRet <<= bAutoControlFocus;
        }
        else if ( aPropertyName == SC_UNO_FORBIDDEN )
        {
            aRet <<= uno::Reference<i18n::XForbiddenCharacters>(new ScForbiddenCharsObj( pDocShell ));
        }
        else if ( aPropertyName == SC_UNO_HASDRAWPAGES )
        {
            aRet <<= (pDocShell->GetDocument().GetDrawLayer() != nullptr);
        }
        else if ( aPropertyName == SC_UNO_BASICLIBRARIES )
        {
            aRet <<= pDocShell->GetBasicContainer();
        }
        else if ( aPropertyName == SC_UNO_DIALOGLIBRARIES )
        {
            aRet <<= pDocShell->GetDialogContainer();
        }
        else if ( aPropertyName == SC_UNO_VBAGLOBNAME )
        {
            /*  #i111553# This property provides the name of the constant that
                will be used to store this model in the global Basic manager.
                That constant will be equivalent to 'ThisComponent' but for
                each application, so e.g. a 'ThisExcelDoc' and a 'ThisWordDoc'
                constant can co-exist, as required by VBA. */
            aRet <<= OUString( "ThisExcelDoc" );
        }
        else if ( aPropertyName == SC_UNO_RUNTIMEUID )
        {
            aRet <<= getRuntimeUID();
        }
        else if ( aPropertyName == SC_UNO_HASVALIDSIGNATURES )
        {
            aRet <<= hasValidSignatures();
        }
        else if ( aPropertyName == SC_UNO_ISLOADED )
        {
            aRet <<= !pDocShell->IsEmpty();
        }
        else if ( aPropertyName == SC_UNO_ISUNDOENABLED )
        {
            aRet <<= rDoc.IsUndoEnabled();
        }
        else if ( aPropertyName == SC_UNO_RECORDCHANGES )
        {
            aRet <<= pDocShell->IsChangeRecording();
        }
        else if ( aPropertyName == SC_UNO_ISRECORDCHANGESPROTECTED )
        {
            aRet <<= pDocShell->HasChangeRecordProtection();
        }
        else if ( aPropertyName == SC_UNO_ISADJUSTHEIGHTENABLED )
        {
            aRet <<= !( rDoc.IsAdjustHeightLocked() );
        }
        else if ( aPropertyName == SC_UNO_ISEXECUTELINKENABLED )
        {
            aRet <<= rDoc.IsExecuteLinkEnabled();
        }
        else if ( aPropertyName == SC_UNO_ISCHANGEREADONLYENABLED )
        {
            aRet <<= rDoc.IsChangeReadOnlyEnabled();
        }
        else if ( aPropertyName == SC_UNO_REFERENCEDEVICE )
        {
            VCLXDevice* pXDev = new VCLXDevice();
            pXDev->SetOutputDevice( rDoc.GetRefDevice() );
            aRet <<= uno::Reference< awt::XDevice >( pXDev );
        }
        else if ( aPropertyName == "BuildId" )
        {
            aRet <<= maBuildId;
        }
        else if ( aPropertyName == "InternalDocument" )
        {
            aRet <<= (pDocShell->GetCreateMode() == SfxObjectCreateMode::INTERNAL);
        }
        else if ( aPropertyName == SC_UNO_INTEROPGRABBAG )
        {
            getGrabBagItem(aRet);
        }
    }

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScModelObj )

// XMultiServiceFactory

css::uno::Reference<css::uno::XInterface> ScModelObj::create(
    OUString const & aServiceSpecifier,
    css::uno::Sequence<css::uno::Any> const * arguments)
{
    using ServiceType = ScServiceProvider::Type;

    uno::Reference<uno::XInterface> xRet;
    ServiceType nType = ScServiceProvider::GetProviderType(aServiceSpecifier);
    if ( nType != ServiceType::INVALID )
    {
        //  drawing layer tables must be kept as long as the model is alive
        //  return stored instance if already set
        switch ( nType )
        {
            case ServiceType::GRADTAB:    xRet.set(xDrawGradTab);     break;
            case ServiceType::HATCHTAB:   xRet.set(xDrawHatchTab);    break;
            case ServiceType::BITMAPTAB:  xRet.set(xDrawBitmapTab);   break;
            case ServiceType::TRGRADTAB:  xRet.set(xDrawTrGradTab);   break;
            case ServiceType::MARKERTAB:  xRet.set(xDrawMarkerTab);   break;
            case ServiceType::DASHTAB:    xRet.set(xDrawDashTab);     break;
            case ServiceType::CHDATAPROV: xRet.set(xChartDataProv);   break;
            case ServiceType::VBAOBJECTPROVIDER: xRet.set(xObjProvider); break;
            default: break;
        }

        // #i64497# If a chart is in a temporary document during clipoard paste,
        // there should be no data provider, so that own data is used
        bool bCreate =
            ! ( nType == ServiceType::CHDATAPROV &&
                ( pDocShell->GetCreateMode() == SfxObjectCreateMode::INTERNAL ));
        // this should never happen, i.e. the temporary document should never be
        // loaded, because this unlinks the data
        OSL_ASSERT( bCreate );

        if ( !xRet.is() && bCreate )
        {
            xRet.set(ScServiceProvider::MakeInstance( nType, pDocShell ));

            //  store created instance
            switch ( nType )
            {
                case ServiceType::GRADTAB:    xDrawGradTab.set(xRet);     break;
                case ServiceType::HATCHTAB:   xDrawHatchTab.set(xRet);    break;
                case ServiceType::BITMAPTAB:  xDrawBitmapTab.set(xRet);   break;
                case ServiceType::TRGRADTAB:  xDrawTrGradTab.set(xRet);   break;
                case ServiceType::MARKERTAB:  xDrawMarkerTab.set(xRet);   break;
                case ServiceType::DASHTAB:    xDrawDashTab.set(xRet);     break;
                case ServiceType::CHDATAPROV: xChartDataProv.set(xRet);   break;
                case ServiceType::VBAOBJECTPROVIDER: xObjProvider.set(xRet); break;
                default: break;
            }
        }
    }
    else
    {
        //  we offload everything we don't know to SvxFmMSFactory,
        //  it'll throw exception if this isn't okay ...

        try
        {
            xRet = arguments == nullptr
                ? SvxFmMSFactory::createInstance(aServiceSpecifier)
                : SvxFmMSFactory::createInstanceWithArguments(
                    aServiceSpecifier, *arguments);
            // extra block to force deletion of the temporary before ScShapeObj ctor (setDelegator)
        }
        catch ( lang::ServiceNotRegisteredException & )
        {
        }

        //  if the drawing factory created a shape, a ScShapeObj has to be used
        //  to support own properties like ImageMap:

        uno::Reference<drawing::XShape> xShape( xRet, uno::UNO_QUERY );
        if ( xShape.is() )
        {
            xRet.clear();               // for aggregation, xShape must be the object's only ref
            new ScShapeObj( xShape );   // aggregates object and modifies xShape
            xRet.set(xShape);
        }
    }
    return xRet;
}

uno::Reference<uno::XInterface> SAL_CALL ScModelObj::createInstance(
                                const OUString& aServiceSpecifier )
{
    SolarMutexGuard aGuard;
    return create(aServiceSpecifier, nullptr);
}

uno::Reference<uno::XInterface> SAL_CALL ScModelObj::createInstanceWithArguments(
                                const OUString& ServiceSpecifier,
                                const uno::Sequence<uno::Any>& aArgs )
{
    //! distinguish between own services and those of drawing layer?

    SolarMutexGuard aGuard;
    uno::Reference<uno::XInterface> xInt(create(ServiceSpecifier, &aArgs));

    if ( aArgs.getLength() )
    {
        //  used only for cell value binding so far - it can be initialized after creating

        uno::Reference<lang::XInitialization> xInit( xInt, uno::UNO_QUERY );
        if ( xInit.is() )
            xInit->initialize( aArgs );
    }

    return xInt;
}

uno::Sequence<OUString> SAL_CALL ScModelObj::getAvailableServiceNames()
{
    SolarMutexGuard aGuard;

    //! why are the parameters of concatServiceNames not const ???
    //! return concatServiceNames( ScServiceProvider::GetAllServiceNames(),
    //!                            SvxFmMSFactory::getAvailableServiceNames() );

    uno::Sequence<OUString> aMyServices(ScServiceProvider::GetAllServiceNames());
    uno::Sequence<OUString> aDrawServices(SvxFmMSFactory::getAvailableServiceNames());

    return concatServiceNames( aMyServices, aDrawServices );
}

// XServiceInfo
OUString SAL_CALL ScModelObj::getImplementationName()
{
    return OUString( "ScModelObj" );
    /* // Matching the .component information:
       return OUString( "com.sun.star.comp.Calc.SpreadsheetDocument" );
    */
}

sal_Bool SAL_CALL ScModelObj::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScModelObj::getSupportedServiceNames()
{
    return {SCMODELOBJ_SERVICE, SCDOCSETTINGS_SERVICE, SCDOC_SERVICE};
}

// XUnoTunnel

sal_Int64 SAL_CALL ScModelObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId )
{
    if ( rId.getLength() == 16 &&
          0 == memcmp( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }

    if ( rId.getLength() == 16 &&
        0 == memcmp( SfxObjectShell::getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(pDocShell ));
    }

    //  aggregated number formats supplier has XUnoTunnel, too
    //  interface from aggregated object must be obtained via queryAggregation

    sal_Int64 nRet = SfxBaseModel::getSomething( rId );
    if ( nRet )
        return nRet;

    if ( GetFormatter().is() )
    {
        const uno::Type& rTunnelType = cppu::UnoType<lang::XUnoTunnel>::get();
        uno::Any aNumTunnel(xNumberAgg->queryAggregation(rTunnelType));
        if(auto xTunnelAgg = o3tl::tryAccess<uno::Reference<lang::XUnoTunnel>>(
               aNumTunnel))
        {
            return (*xTunnelAgg)->getSomething( rId );
        }
    }

    return 0;
}

namespace
{
    class theScModelObjUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScModelObjUnoTunnelId> {};
}

const uno::Sequence<sal_Int8>& ScModelObj::getUnoTunnelId()
{
    return theScModelObjUnoTunnelId::get().getSeq();
}

ScModelObj* ScModelObj::getImplementation(const uno::Reference<uno::XInterface>& rObj)
{
    ScModelObj* pRet = nullptr;
    uno::Reference<lang::XUnoTunnel> xUT(rObj, uno::UNO_QUERY);
    if (xUT.is())
        pRet = reinterpret_cast<ScModelObj*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}

// XChangesNotifier

void ScModelObj::addChangesListener( const uno::Reference< util::XChangesListener >& aListener )
{
    SolarMutexGuard aGuard;
    maChangesListeners.addInterface( aListener );
}

void ScModelObj::removeChangesListener( const uno::Reference< util::XChangesListener >& aListener )
{
    SolarMutexGuard aGuard;
    maChangesListeners.removeInterface( aListener );
}

bool ScModelObj::HasChangesListeners() const
{
    if ( maChangesListeners.getLength() > 0 )
        return true;

    // "change" event set in any sheet?
    return pDocShell && pDocShell->GetDocument().HasAnySheetEventScript(ScSheetEventId::CHANGE);
}

void ScModelObj::NotifyChanges( const OUString& rOperation, const ScRangeList& rRanges,
    const uno::Sequence< beans::PropertyValue >& rProperties )
{
    if ( pDocShell && HasChangesListeners() )
    {
        util::ChangesEvent aEvent;
        aEvent.Source.set( static_cast< cppu::OWeakObject* >( this ) );
        aEvent.Base <<= aEvent.Source;

        size_t nRangeCount = rRanges.size();
        aEvent.Changes.realloc( static_cast< sal_Int32 >( nRangeCount ) );
        for ( size_t nIndex = 0; nIndex < nRangeCount; ++nIndex )
        {
            uno::Reference< table::XCellRange > xRangeObj;

            ScRange const & rRange = rRanges[ nIndex ];
            if ( rRange.aStart == rRange.aEnd )
            {
                xRangeObj.set( new ScCellObj( pDocShell, rRange.aStart ) );
            }
            else
            {
                xRangeObj.set( new ScCellRangeObj( pDocShell, rRange ) );
            }

            util::ElementChange& rChange = aEvent.Changes[ static_cast< sal_Int32 >( nIndex ) ];
            rChange.Accessor <<= rOperation;
            rChange.Element <<= rProperties;
            rChange.ReplacedElement <<= xRangeObj;
        }

        ::comphelper::OInterfaceIteratorHelper2 aIter( maChangesListeners );
        while ( aIter.hasMoreElements() )
        {
            try
            {
                static_cast< util::XChangesListener* >( aIter.next() )->changesOccurred( aEvent );
            }
            catch( uno::Exception& )
            {
            }
        }
    }

    // handle sheet events
    //! separate method with ScMarkData? Then change HasChangesListeners back.
    if ( rOperation == "cell-change" && pDocShell )
    {
        ScMarkData aMarkData;
        aMarkData.MarkFromRangeList( rRanges, false );
        ScDocument& rDoc = pDocShell->GetDocument();
        SCTAB nTabCount = rDoc.GetTableCount();
        for (const SCTAB& nTab : aMarkData)
        {
            if (nTab >= nTabCount)
                break;
            const ScSheetEvents* pEvents = rDoc.GetSheetEvents(nTab);
            if (pEvents)
            {
                const OUString* pScript = pEvents->GetScript(ScSheetEventId::CHANGE);
                if (pScript)
                {
                    ScRangeList aTabRanges;     // collect ranges on this sheet
                    size_t nRangeCount = rRanges.size();
                    for ( size_t nIndex = 0; nIndex < nRangeCount; ++nIndex )
                    {
                        ScRange const & rRange = rRanges[ nIndex ];
                        if ( rRange.aStart.Tab() == nTab )
                            aTabRanges.push_back( rRange );
                    }
                    size_t nTabRangeCount = aTabRanges.size();
                    if ( nTabRangeCount > 0 )
                    {
                        uno::Reference<uno::XInterface> xTarget;
                        if ( nTabRangeCount == 1 )
                        {
                            ScRange const & rRange = aTabRanges[ 0 ];
                            if ( rRange.aStart == rRange.aEnd )
                                xTarget.set( static_cast<cppu::OWeakObject*>( new ScCellObj( pDocShell, rRange.aStart ) ) );
                            else
                                xTarget.set( static_cast<cppu::OWeakObject*>( new ScCellRangeObj( pDocShell, rRange ) ) );
                        }
                        else
                            xTarget.set( static_cast<cppu::OWeakObject*>( new ScCellRangesObj( pDocShell, aTabRanges ) ) );

                        uno::Sequence<uno::Any> aParams(1);
                        aParams[0] <<= xTarget;

                        uno::Any aRet;
                        uno::Sequence<sal_Int16> aOutArgsIndex;
                        uno::Sequence<uno::Any> aOutArgs;

                        /*ErrCode eRet =*/ pDocShell->CallXScript( *pScript, aParams, aRet, aOutArgsIndex, aOutArgs );
                    }
                }
            }
        }
    }
}

void ScModelObj::HandleCalculateEvents()
{
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        // don't call events before the document is visible
        // (might also set a flag on SfxEventHintId::LoadFinished and only disable while loading)
        if ( rDoc.IsDocVisible() )
        {
            SCTAB nTabCount = rDoc.GetTableCount();
            for (SCTAB nTab = 0; nTab < nTabCount; nTab++)
            {
                if (rDoc.HasCalcNotification(nTab))
                {
                    if (const ScSheetEvents* pEvents = rDoc.GetSheetEvents( nTab ))
                    {
                        if (const OUString* pScript = pEvents->GetScript(ScSheetEventId::CALCULATE))
                        {
                            uno::Any aRet;
                            uno::Sequence<uno::Any> aParams;
                            uno::Sequence<sal_Int16> aOutArgsIndex;
                            uno::Sequence<uno::Any> aOutArgs;
                            pDocShell->CallXScript( *pScript, aParams, aRet, aOutArgsIndex, aOutArgs );
                        }
                    }

                    try
                    {
                        uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents( rDoc.GetVbaEventProcessor(), uno::UNO_SET_THROW );
                        uno::Sequence< uno::Any > aArgs( 1 );
                        aArgs[ 0 ] <<= nTab;
                        xVbaEvents->processVbaEvent( ScSheetEvents::GetVbaSheetEventId( ScSheetEventId::CALCULATE ), aArgs );
                    }
                    catch( uno::Exception& )
                    {
                    }
                }
            }
        }
        rDoc.ResetCalcNotifications();
    }
}

// XOpenCLSelection

sal_Bool ScModelObj::isOpenCLEnabled()
{
    return ScCalcConfig::isOpenCLEnabled();
}

void ScModelObj::enableOpenCL(sal_Bool bEnable)
{
    if (ScCalcConfig::isOpenCLEnabled() == static_cast<bool>(bEnable))
        return;
    if (ScCalcConfig::getForceCalculationType() != ForceCalculationNone)
        return;

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::UseOpenCL::set(bEnable, batch);
    batch->commit();

    ScCalcConfig aConfig = ScInterpreter::GetGlobalConfig();
    if (bEnable)
        aConfig.setOpenCLConfigToDefault();
    ScInterpreter::SetGlobalConfig(aConfig);

#if HAVE_FEATURE_OPENCL
    sc::FormulaGroupInterpreter::switchOpenCLDevice(OUString(), true);
#endif

    ScDocument* pDoc = GetDocument();
    pDoc->CheckVectorizationState();

}

void ScModelObj::enableAutomaticDeviceSelection(sal_Bool bForce)
{
    ScCalcConfig aConfig = ScInterpreter::GetGlobalConfig();
    aConfig.mbOpenCLAutoSelect = true;
    ScInterpreter::SetGlobalConfig(aConfig);
    ScFormulaOptions aOptions = SC_MOD()->GetFormulaOptions();
    aOptions.SetCalcConfig(aConfig);
    SC_MOD()->SetFormulaOptions(aOptions);
#if !HAVE_FEATURE_OPENCL
    (void) bForce;
#else
    sc::FormulaGroupInterpreter::switchOpenCLDevice(OUString(), true, bForce);
#endif
}

void ScModelObj::disableAutomaticDeviceSelection()
{
    ScCalcConfig aConfig = ScInterpreter::GetGlobalConfig();
    aConfig.mbOpenCLAutoSelect = false;
    ScInterpreter::SetGlobalConfig(aConfig);
    ScFormulaOptions aOptions = SC_MOD()->GetFormulaOptions();
    aOptions.SetCalcConfig(aConfig);
    SC_MOD()->SetFormulaOptions(aOptions);
}

void ScModelObj::selectOpenCLDevice( sal_Int32 nPlatform, sal_Int32 nDevice )
{
    if(nPlatform < 0 || nDevice < 0)
        throw uno::RuntimeException();

#if !HAVE_FEATURE_OPENCL
    throw uno::RuntimeException();
#else
    std::vector<OpenCLPlatformInfo> aPlatformInfo;
    sc::FormulaGroupInterpreter::fillOpenCLInfo(aPlatformInfo);
    if(size_t(nPlatform) >= aPlatformInfo.size())
        throw uno::RuntimeException();

    if(size_t(nDevice) >= aPlatformInfo[nPlatform].maDevices.size())
        throw uno::RuntimeException();

    OUString aDeviceString = aPlatformInfo[nPlatform].maVendor + " " + aPlatformInfo[nPlatform].maDevices[nDevice].maName;
    sc::FormulaGroupInterpreter::switchOpenCLDevice(aDeviceString, false);
#endif
}

sal_Int32 ScModelObj::getPlatformID()
{
#if !HAVE_FEATURE_OPENCL
    return -1;
#else
    sal_Int32 nPlatformId;
    sal_Int32 nDeviceId;
    sc::FormulaGroupInterpreter::getOpenCLDeviceInfo(nDeviceId, nPlatformId);
    return nPlatformId;
#endif
}

sal_Int32 ScModelObj::getDeviceID()
{
#if !HAVE_FEATURE_OPENCL
    return -1;
#else
    sal_Int32 nPlatformId;
    sal_Int32 nDeviceId;
    sc::FormulaGroupInterpreter::getOpenCLDeviceInfo(nDeviceId, nPlatformId);
    return nDeviceId;
#endif
}

uno::Sequence< sheet::opencl::OpenCLPlatform > ScModelObj::getOpenCLPlatforms()
{
#if !HAVE_FEATURE_OPENCL
    return uno::Sequence<sheet::opencl::OpenCLPlatform>();
#else
    std::vector<OpenCLPlatformInfo> aPlatformInfo;
    sc::FormulaGroupInterpreter::fillOpenCLInfo(aPlatformInfo);

    uno::Sequence<sheet::opencl::OpenCLPlatform> aRet(aPlatformInfo.size());
    for(size_t i = 0; i < aPlatformInfo.size(); ++i)
    {
        aRet[i].Name = aPlatformInfo[i].maName;
        aRet[i].Vendor = aPlatformInfo[i].maVendor;

        aRet[i].Devices.realloc(aPlatformInfo[i].maDevices.size());
        for(size_t j = 0; j < aPlatformInfo[i].maDevices.size(); ++j)
        {
            const OpenCLDeviceInfo& rDevice = aPlatformInfo[i].maDevices[j];
            aRet[i].Devices[j].Name = rDevice.maName;
            aRet[i].Devices[j].Vendor = rDevice.maVendor;
            aRet[i].Devices[j].Driver = rDevice.maDriver;
        }
    }

    return aRet;
#endif
}

namespace {

/// @throws css::uno::RuntimeException
void setOpcodeSubsetTest(bool bFlag)
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Calc::Formula::Calculation::OpenCLSubsetOnly::set(bFlag, batch);
    batch->commit();
}

}

void ScModelObj::enableOpcodeSubsetTest()
{
    setOpcodeSubsetTest(true);
}

void ScModelObj::disableOpcodeSubsetTest()
{
    setOpcodeSubsetTest(false);
}

sal_Bool ScModelObj::isOpcodeSubsetTested()
{
    return officecfg::Office::Calc::Formula::Calculation::OpenCLSubsetOnly::get();
}

void ScModelObj::setFormulaCellNumberLimit( sal_Int32 number )
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Calc::Formula::Calculation::OpenCLMinimumDataSize::set(number, batch);
    batch->commit();
}

sal_Int32 ScModelObj::getFormulaCellNumberLimit()
{
    return officecfg::Office::Calc::Formula::Calculation::OpenCLMinimumDataSize::get().get();
}

ScDrawPagesObj::ScDrawPagesObj(ScDocShell* pDocSh) :
    pDocShell( pDocSh )
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScDrawPagesObj::~ScDrawPagesObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScDrawPagesObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //  we don't care about update of references here

    if ( rHint.GetId() == SfxHintId::Dying )
    {
        pDocShell = nullptr;       // became invalid
    }
}

uno::Reference<drawing::XDrawPage> ScDrawPagesObj::GetObjectByIndex_Impl(sal_Int32 nIndex) const
{
    if (pDocShell)
    {
        ScDrawLayer* pDrawLayer = pDocShell->MakeDrawLayer();
        OSL_ENSURE(pDrawLayer,"Cannot create Draw-Layer");
        if ( pDrawLayer && nIndex >= 0 && nIndex < pDocShell->GetDocument().GetTableCount() )
        {
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nIndex));
            OSL_ENSURE(pPage,"Draw-Page not found");
            if (pPage)
            {
                return uno::Reference<drawing::XDrawPage> (pPage->getUnoPage(), uno::UNO_QUERY);
            }
        }
    }
    return nullptr;
}

// XDrawPages

uno::Reference<drawing::XDrawPage> SAL_CALL ScDrawPagesObj::insertNewByIndex( sal_Int32 nPos )
{
    SolarMutexGuard aGuard;
    uno::Reference<drawing::XDrawPage> xRet;
    if (pDocShell)
    {
        OUString aNewName;
        pDocShell->GetDocument().CreateValidTabName(aNewName);
        if ( pDocShell->GetDocFunc().InsertTable( static_cast<SCTAB>(nPos),
                                                  aNewName, true, true ) )
            xRet.set(GetObjectByIndex_Impl( nPos ));
    }
    return xRet;
}

void SAL_CALL ScDrawPagesObj::remove( const uno::Reference<drawing::XDrawPage>& xPage )
{
    SolarMutexGuard aGuard;
    SvxDrawPage* pImp = SvxDrawPage::getImplementation( xPage );
    if ( pDocShell && pImp )
    {
        SdrPage* pPage = pImp->GetSdrPage();
        if (pPage)
        {
            SCTAB nPageNum = static_cast<SCTAB>(pPage->GetPageNum());
            pDocShell->GetDocFunc().DeleteTable( nPageNum, true );
        }
    }
}

// XIndexAccess

sal_Int32 SAL_CALL ScDrawPagesObj::getCount()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return pDocShell->GetDocument().GetTableCount();
    return 0;
}

uno::Any SAL_CALL ScDrawPagesObj::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    uno::Reference<drawing::XDrawPage> xPage(GetObjectByIndex_Impl(nIndex));
    if (!xPage.is())
        throw lang::IndexOutOfBoundsException();

    return uno::makeAny(xPage);
}

uno::Type SAL_CALL ScDrawPagesObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<drawing::XDrawPage>::get();
}

sal_Bool SAL_CALL ScDrawPagesObj::hasElements()
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

ScTableSheetsObj::ScTableSheetsObj(ScDocShell* pDocSh) :
    pDocShell( pDocSh )
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScTableSheetsObj::~ScTableSheetsObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScTableSheetsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    // we don't care about update of references here

    if ( rHint.GetId() == SfxHintId::Dying )
    {
        pDocShell = nullptr;       // became invalid
    }
}

// XSpreadsheets

ScTableSheetObj* ScTableSheetsObj::GetObjectByIndex_Impl(sal_Int32 nIndex) const
{
    if ( pDocShell && nIndex >= 0 && nIndex < pDocShell->GetDocument().GetTableCount() )
        return new ScTableSheetObj( pDocShell, static_cast<SCTAB>(nIndex) );

    return nullptr;
}

ScTableSheetObj* ScTableSheetsObj::GetObjectByName_Impl(const OUString& aName) const
{
    if (pDocShell)
    {
        SCTAB nIndex;
        if ( pDocShell->GetDocument().GetTable( aName, nIndex ) )
            return new ScTableSheetObj( pDocShell, nIndex );
    }
    return nullptr;
}

void SAL_CALL ScTableSheetsObj::insertNewByName( const OUString& aName, sal_Int16 nPosition )
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    if (pDocShell)
    {
        bDone = pDocShell->GetDocFunc().InsertTable( nPosition, aName, true, true );
    }
    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

void SAL_CALL ScTableSheetsObj::moveByName( const OUString& aName, sal_Int16 nDestination )
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    if (pDocShell)
    {
        SCTAB nSource;
        if ( pDocShell->GetDocument().GetTable( aName, nSource ) )
            bDone = pDocShell->MoveTable( nSource, nDestination, false, true );
    }
    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

void SAL_CALL ScTableSheetsObj::copyByName( const OUString& aName,
    const OUString& aCopy, sal_Int16 nDestination )
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    if (pDocShell)
    {
        SCTAB nSource;
        if ( pDocShell->GetDocument().GetTable( aName, nSource ) )
        {
            bDone = pDocShell->MoveTable( nSource, nDestination, true, true );
            if (bDone)
            {
                // #i92477# any index past the last sheet means "append" in MoveTable
                SCTAB nResultTab = static_cast<SCTAB>(nDestination);
                SCTAB nTabCount = pDocShell->GetDocument().GetTableCount();    // count after copying
                if (nResultTab >= nTabCount)
                    nResultTab = nTabCount - 1;

                bDone = pDocShell->GetDocFunc().RenameTable( nResultTab, aCopy,
                                                             true, true );
            }
        }
    }
    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

void SAL_CALL ScTableSheetsObj::insertByName( const OUString& aName, const uno::Any& aElement )
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    bool bIllArg = false;

    //! Type of aElement can be some specific interface instead of XInterface

    if ( pDocShell )
    {
        uno::Reference<uno::XInterface> xInterface(aElement, uno::UNO_QUERY);
        if ( xInterface.is() )
        {
            ScTableSheetObj* pSheetObj = ScTableSheetObj::getImplementation( xInterface );
            if ( pSheetObj && !pSheetObj->GetDocShell() )   // not inserted yet?
            {
                ScDocument& rDoc = pDocShell->GetDocument();
                SCTAB nDummy;
                if ( rDoc.GetTable( aName, nDummy ) )
                {
                    //  name already exists
                    throw container::ElementExistException();
                }
                SCTAB nPosition = rDoc.GetTableCount();
                bDone = pDocShell->GetDocFunc().InsertTable( nPosition, aName,
                                                             true, true );
                if (bDone)
                    pSheetObj->InitInsertSheet( pDocShell, nPosition );
                //  set document and new range in the object
            }
            else
                bIllArg = true;
        }
        else
            bIllArg = true;
    }

    if (!bDone)
    {
        if (bIllArg)
            throw lang::IllegalArgumentException();
        else
            throw uno::RuntimeException();      // ElementExistException is handled above
    }
}

void SAL_CALL ScTableSheetsObj::replaceByName( const OUString& aName, const uno::Any& aElement )
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    bool bIllArg = false;

    //! Type of aElement can be some specific interface instead of XInterface

    if ( pDocShell )
    {
        uno::Reference<uno::XInterface> xInterface(aElement, uno::UNO_QUERY);
        if ( xInterface.is() )
        {
            ScTableSheetObj* pSheetObj = ScTableSheetObj::getImplementation( xInterface );
            if ( pSheetObj && !pSheetObj->GetDocShell() )   // not inserted yet?
            {
                SCTAB nPosition;
                if ( !pDocShell->GetDocument().GetTable( aName, nPosition ) )
                {
                    //  not found
                    throw container::NoSuchElementException();
                }

                if ( pDocShell->GetDocFunc().DeleteTable( nPosition, true ) )
                {
                    //  InsertTable can't really go wrong now
                    bDone = pDocShell->GetDocFunc().InsertTable( nPosition, aName, true, true );
                    if (bDone)
                        pSheetObj->InitInsertSheet( pDocShell, nPosition );
                }

            }
            else
                bIllArg = true;
        }
        else
            bIllArg = true;
    }

    if (!bDone)
    {
        if (bIllArg)
            throw lang::IllegalArgumentException();
        else
            throw uno::RuntimeException();      // NoSuchElementException is handled above
    }
}

void SAL_CALL ScTableSheetsObj::removeByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    if (pDocShell)
    {
        SCTAB nIndex;
        if ( !pDocShell->GetDocument().GetTable( aName, nIndex ) )
            throw container::NoSuchElementException(); // not found
        bDone = pDocShell->GetDocFunc().DeleteTable( nIndex, true );
    }

    if (!bDone)
        throw uno::RuntimeException();      // NoSuchElementException is handled above
}

sal_Int32 ScTableSheetsObj::importSheet(
    const uno::Reference < sheet::XSpreadsheetDocument > & xDocSrc,
    const OUString& srcName, const sal_Int32 nDestPosition )
{
    //pDocShell is the destination
    ScDocument& rDocDest = pDocShell->GetDocument();

    // Source document docShell
    if ( !xDocSrc.is() )
        throw uno::RuntimeException();
    ScModelObj* pObj = ScModelObj::getImplementation(xDocSrc);
    ScDocShell* pDocShellSrc = static_cast<ScDocShell*>(pObj->GetEmbeddedObject());

    // SourceSheet Position and does srcName exists ?
    SCTAB nIndexSrc;
    if ( !pDocShellSrc->GetDocument().GetTable( srcName, nIndexSrc ) )
        throw lang::IllegalArgumentException();

    // Check the validity of destination index.
    SCTAB nCount = rDocDest.GetTableCount();
    SCTAB nIndexDest = static_cast<SCTAB>(nDestPosition);
    if (nIndexDest > nCount || nIndexDest < 0)
        throw lang::IndexOutOfBoundsException();

    // Transfer Tab
    pDocShell->TransferTab(
        *pDocShellSrc, nIndexSrc, nIndexDest, true/*bInsertNew*/, true/*bNotifyAndPaint*/ );

    return nIndexDest;
}

// XCellRangesAccess

uno::Reference< table::XCell > SAL_CALL ScTableSheetsObj::getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow, sal_Int32 nSheet )
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XCellRange> xSheet(static_cast<ScCellRangeObj*>(GetObjectByIndex_Impl(static_cast<sal_uInt16>(nSheet))));
    if (! xSheet.is())
        throw lang::IndexOutOfBoundsException();

    return xSheet->getCellByPosition(nColumn, nRow);
}

uno::Reference< table::XCellRange > SAL_CALL ScTableSheetsObj::getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom, sal_Int32 nSheet )
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XCellRange> xSheet(static_cast<ScCellRangeObj*>(GetObjectByIndex_Impl(static_cast<sal_uInt16>(nSheet))));
    if (! xSheet.is())
        throw lang::IndexOutOfBoundsException();

    return xSheet->getCellRangeByPosition(nLeft, nTop, nRight, nBottom);
}

uno::Sequence < uno::Reference< table::XCellRange > > SAL_CALL ScTableSheetsObj::getCellRangesByName( const OUString& aRange )
{
    SolarMutexGuard aGuard;
    uno::Sequence < uno::Reference < table::XCellRange > > xRet;

    ScRangeList aRangeList;
    ScDocument& rDoc = pDocShell->GetDocument();
    if (!ScRangeStringConverter::GetRangeListFromString( aRangeList, aRange, &rDoc, ::formula::FormulaGrammar::CONV_OOO, ';' ))
        throw lang::IllegalArgumentException();

    size_t nCount = aRangeList.size();
    if (!nCount)
        throw lang::IllegalArgumentException();

    xRet.realloc(nCount);
    for( size_t nIndex = 0; nIndex < nCount; nIndex++ )
    {
        const ScRange & rRange = aRangeList[ nIndex ];
        xRet[nIndex] = new ScCellRangeObj(pDocShell, rRange);
    }

    return xRet;
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScTableSheetsObj::createEnumeration()
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, "com.sun.star.sheet.SpreadsheetsEnumeration");
}

// XIndexAccess

sal_Int32 SAL_CALL ScTableSheetsObj::getCount()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return pDocShell->GetDocument().GetTableCount();
    return 0;
}

uno::Any SAL_CALL ScTableSheetsObj::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XSpreadsheet> xSheet(GetObjectByIndex_Impl(nIndex));
    if (!xSheet.is())
        throw lang::IndexOutOfBoundsException();

    return uno::makeAny(xSheet);

//    return uno::Any();
}

uno::Type SAL_CALL ScTableSheetsObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<sheet::XSpreadsheet>::get();
}

sal_Bool SAL_CALL ScTableSheetsObj::hasElements()
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

// XNameAccess

uno::Any SAL_CALL ScTableSheetsObj::getByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XSpreadsheet> xSheet(GetObjectByName_Impl(aName));
    if (!xSheet.is())
        throw container::NoSuchElementException();

    return uno::makeAny(xSheet);
}

uno::Sequence<OUString> SAL_CALL ScTableSheetsObj::getElementNames()
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        SCTAB nCount = rDoc.GetTableCount();
        OUString aName;
        uno::Sequence<OUString> aSeq(nCount);
        OUString* pAry = aSeq.getArray();
        for (SCTAB i=0; i<nCount; i++)
        {
            rDoc.GetName( i, aName );
            pAry[i] = aName;
        }
        return aSeq;
    }
    return uno::Sequence<OUString>();
}

sal_Bool SAL_CALL ScTableSheetsObj::hasByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        SCTAB nIndex;
        if ( pDocShell->GetDocument().GetTable( aName, nIndex ) )
            return true;
    }
    return false;
}

ScTableColumnsObj::ScTableColumnsObj(ScDocShell* pDocSh, SCTAB nT, SCCOL nSC, SCCOL nEC) :
    pDocShell( pDocSh ),
    nTab     ( nT ),
    nStartCol( nSC ),
    nEndCol  ( nEC )
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScTableColumnsObj::~ScTableColumnsObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScTableColumnsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( dynamic_cast<const ScUpdateRefHint*>(&rHint) )
    {
        //! update of references for sheet and its start/end
    }
    else if ( rHint.GetId() == SfxHintId::Dying )
    {
        pDocShell = nullptr;       // became invalid
    }
}

// XTableColumns

ScTableColumnObj* ScTableColumnsObj::GetObjectByIndex_Impl(sal_Int32 nIndex) const
{
    SCCOL nCol = static_cast<SCCOL>(nIndex) + nStartCol;
    if ( pDocShell && nCol <= nEndCol )
        return new ScTableColumnObj( pDocShell, nCol, nTab );

    return nullptr;    // wrong index
}

ScTableColumnObj* ScTableColumnsObj::GetObjectByName_Impl(const OUString& aName) const
{
    SCCOL nCol = 0;
    if ( ::AlphaToCol( nCol, aName) )
        if ( pDocShell && nCol >= nStartCol && nCol <= nEndCol )
            return new ScTableColumnObj( pDocShell, nCol, nTab );

    return nullptr;
}

void SAL_CALL ScTableColumnsObj::insertByIndex( sal_Int32 nPosition, sal_Int32 nCount )
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    if ( pDocShell && nCount > 0 && nPosition >= 0 && nStartCol+nPosition <= nEndCol &&
            nStartCol+nPosition+nCount-1 <= MAXCOL )
    {
        ScRange aRange( static_cast<SCCOL>(nStartCol+nPosition), 0, nTab,
                        static_cast<SCCOL>(nStartCol+nPosition+nCount-1), MAXROW, nTab );
        bDone = pDocShell->GetDocFunc().InsertCells( aRange, nullptr, INS_INSCOLS_BEFORE, true, true );
    }
    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

void SAL_CALL ScTableColumnsObj::removeByIndex( sal_Int32 nIndex, sal_Int32 nCount )
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    //  the range to be deleted has to lie within the object
    if ( pDocShell && nCount > 0 && nIndex >= 0 && nStartCol+nIndex+nCount-1 <= nEndCol )
    {
        ScRange aRange( static_cast<SCCOL>(nStartCol+nIndex), 0, nTab,
                        static_cast<SCCOL>(nStartCol+nIndex+nCount-1), MAXROW, nTab );
        bDone = pDocShell->GetDocFunc().DeleteCells( aRange, nullptr, DelCellCmd::Cols, true );
    }
    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScTableColumnsObj::createEnumeration()
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, "com.sun.star.table.TableColumnsEnumeration");
}

// XIndexAccess

sal_Int32 SAL_CALL ScTableColumnsObj::getCount()
{
    SolarMutexGuard aGuard;
    return nEndCol - nStartCol + 1;
}

uno::Any SAL_CALL ScTableColumnsObj::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XCellRange> xColumn(GetObjectByIndex_Impl(nIndex));
    if (!xColumn.is())
        throw lang::IndexOutOfBoundsException();

    return uno::makeAny(xColumn);

}

uno::Type SAL_CALL ScTableColumnsObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<table::XCellRange>::get();
}

sal_Bool SAL_CALL ScTableColumnsObj::hasElements()
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

uno::Any SAL_CALL ScTableColumnsObj::getByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XCellRange> xColumn(GetObjectByName_Impl(aName));
    if (!xColumn.is())
        throw container::NoSuchElementException();

    return uno::makeAny(xColumn);
}

uno::Sequence<OUString> SAL_CALL ScTableColumnsObj::getElementNames()
{
    SolarMutexGuard aGuard;
    SCCOL nCount = nEndCol - nStartCol + 1;
    uno::Sequence<OUString> aSeq(nCount);
    OUString* pAry = aSeq.getArray();
    for (SCCOL i=0; i<nCount; i++)
        pAry[i] = ::ScColToAlpha( nStartCol + i );

    return aSeq;
}

sal_Bool SAL_CALL ScTableColumnsObj::hasByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    SCCOL nCol = 0;
    if ( ::AlphaToCol( nCol, aName) )
        if ( pDocShell && nCol >= nStartCol && nCol <= nEndCol )
            return true;

    return false;       // not found
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScTableColumnsObj::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( lcl_GetColumnsPropertyMap() ));
    return aRef;
}

void SAL_CALL ScTableColumnsObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
        throw uno::RuntimeException();

    std::vector<sc::ColRowSpan> aColArr(1, sc::ColRowSpan(nStartCol,nEndCol));
    ScDocFunc& rFunc = pDocShell->GetDocFunc();

    if ( aPropertyName == SC_UNONAME_CELLWID )
    {
        sal_Int32 nNewWidth = 0;
        if ( aValue >>= nNewWidth )
            rFunc.SetWidthOrHeight(
                true, aColArr, nTab, SC_SIZE_ORIGINAL, static_cast<sal_uInt16>(HMMToTwips(nNewWidth)), true, true);
    }
    else if ( aPropertyName == SC_UNONAME_CELLVIS )
    {
        bool bVis = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        ScSizeMode eMode = bVis ? SC_SIZE_SHOW : SC_SIZE_DIRECT;
        rFunc.SetWidthOrHeight(true, aColArr, nTab, eMode, 0, true, true);
        //  SC_SIZE_DIRECT with size 0: hide
    }
    else if ( aPropertyName == SC_UNONAME_OWIDTH )
    {
        bool bOpt = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        if (bOpt)
            rFunc.SetWidthOrHeight(
                true, aColArr, nTab, SC_SIZE_OPTIMAL, STD_EXTRA_WIDTH, true, true);
        // sal_False for columns currently has no effect
    }
    else if ( aPropertyName == SC_UNONAME_NEWPAGE || aPropertyName == SC_UNONAME_MANPAGE )
    {
        //! single function to set/remove all breaks?
        bool bSet = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        for (SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++)
            if (bSet)
                rFunc.InsertPageBreak( true, ScAddress(nCol,0,nTab), true, true );
            else
                rFunc.RemovePageBreak( true, ScAddress(nCol,0,nTab), true, true );
    }
}

uno::Any SAL_CALL ScTableColumnsObj::getPropertyValue( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
        throw uno::RuntimeException();

    ScDocument& rDoc = pDocShell->GetDocument();
    uno::Any aAny;

    //! loop over all columns for current state?

    if ( aPropertyName == SC_UNONAME_CELLWID )
    {
        // for hidden column, return original height
        sal_uInt16 nWidth = rDoc.GetOriginalWidth( nStartCol, nTab );
        aAny <<= static_cast<sal_Int32>(TwipsToHMM(nWidth));
    }
    else if ( aPropertyName == SC_UNONAME_CELLVIS )
    {
        bool bVis = !rDoc.ColHidden(nStartCol, nTab);
        aAny <<= bVis;
    }
    else if ( aPropertyName == SC_UNONAME_OWIDTH )
    {
        bool bOpt = !(rDoc.GetColFlags( nStartCol, nTab ) & CRFlags::ManualSize);
        aAny <<= bOpt;
    }
    else if ( aPropertyName == SC_UNONAME_NEWPAGE )
    {
        ScBreakType nBreak = rDoc.HasColBreak(nStartCol, nTab);
        aAny <<= (nBreak != ScBreakType::NONE);
    }
    else if ( aPropertyName == SC_UNONAME_MANPAGE )
    {
        ScBreakType nBreak = rDoc.HasColBreak(nStartCol, nTab);
        aAny <<= bool(nBreak & ScBreakType::Manual);
    }

    return aAny;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScTableColumnsObj )

ScTableRowsObj::ScTableRowsObj(ScDocShell* pDocSh, SCTAB nT, SCROW nSR, SCROW nER) :
    pDocShell( pDocSh ),
    nTab     ( nT ),
    nStartRow( nSR ),
    nEndRow  ( nER )
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScTableRowsObj::~ScTableRowsObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScTableRowsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( dynamic_cast<const ScUpdateRefHint*>(&rHint) )
    {
        //! update of references for sheet and its start/end
    }
    else if ( rHint.GetId() == SfxHintId::Dying )
    {
        pDocShell = nullptr;       // became invalid
    }
}

// XTableRows

ScTableRowObj* ScTableRowsObj::GetObjectByIndex_Impl(sal_Int32 nIndex) const
{
    SCROW nRow = static_cast<SCROW>(nIndex) + nStartRow;
    if ( pDocShell && nRow <= nEndRow )
        return new ScTableRowObj( pDocShell, nRow, nTab );

    return nullptr;    // wrong index
}

void SAL_CALL ScTableRowsObj::insertByIndex( sal_Int32 nPosition, sal_Int32 nCount )
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    if ( pDocShell && nCount > 0 && nPosition >= 0 && nStartRow+nPosition <= nEndRow &&
            nStartRow+nPosition+nCount-1 <= MAXROW )
    {
        ScRange aRange( 0, static_cast<SCROW>(nStartRow+nPosition), nTab,
                        MAXCOL, static_cast<SCROW>(nStartRow+nPosition+nCount-1), nTab );
        bDone = pDocShell->GetDocFunc().InsertCells( aRange, nullptr, INS_INSROWS_BEFORE, true, true );
    }
    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

void SAL_CALL ScTableRowsObj::removeByIndex( sal_Int32 nIndex, sal_Int32 nCount )
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    // the range to be deleted has to lie within the object
    if ( pDocShell && nCount > 0 && nIndex >= 0 && nStartRow+nIndex+nCount-1 <= nEndRow )
    {
        ScRange aRange( 0, static_cast<SCROW>(nStartRow+nIndex), nTab,
                        MAXCOL, static_cast<SCROW>(nStartRow+nIndex+nCount-1), nTab );
        bDone = pDocShell->GetDocFunc().DeleteCells( aRange, nullptr, DelCellCmd::Rows, true );
    }
    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScTableRowsObj::createEnumeration()
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, "com.sun.star.table.TableRowsEnumeration");
}

// XIndexAccess

sal_Int32 SAL_CALL ScTableRowsObj::getCount()
{
    SolarMutexGuard aGuard;
    return nEndRow - nStartRow + 1;
}

uno::Any SAL_CALL ScTableRowsObj::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XCellRange> xRow(GetObjectByIndex_Impl(nIndex));
    if (!xRow.is())
        throw lang::IndexOutOfBoundsException();

    return uno::makeAny(xRow);
}

uno::Type SAL_CALL ScTableRowsObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<table::XCellRange>::get();
}

sal_Bool SAL_CALL ScTableRowsObj::hasElements()
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScTableRowsObj::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( lcl_GetRowsPropertyMap() ));
    return aRef;
}

void SAL_CALL ScTableRowsObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
        throw uno::RuntimeException();

    ScDocFunc& rFunc = pDocShell->GetDocFunc();
    ScDocument& rDoc = pDocShell->GetDocument();
    std::vector<sc::ColRowSpan> aRowArr(1, sc::ColRowSpan(nStartRow,nEndRow));

    if ( aPropertyName == SC_UNONAME_OHEIGHT )
    {
        sal_Int32 nNewHeight = 0;
        if ( rDoc.IsImportingXML() && ( aValue >>= nNewHeight ) )
        {
            // used to set the stored row height for rows with optimal height when loading.

            // TODO: It's probably cleaner to use a different property name
            // for this.
            rDoc.SetRowHeightOnly( nStartRow, nEndRow, nTab, static_cast<sal_uInt16>(HMMToTwips(nNewHeight)) );
        }
        else
        {
            bool bOpt = ScUnoHelpFunctions::GetBoolFromAny( aValue );
            if (bOpt)
                rFunc.SetWidthOrHeight(false, aRowArr, nTab, SC_SIZE_OPTIMAL, 0, true, true);
            else
            {
                //! manually set old heights again?
            }
        }
    }
    else if ( aPropertyName == SC_UNONAME_CELLHGT )
    {
        sal_Int32 nNewHeight = 0;
        if ( aValue >>= nNewHeight )
        {
            if (rDoc.IsImportingXML())
            {
                // TODO: This is a band-aid fix.  Eventually we need to
                // re-work ods' style import to get it to set styles to
                // ScDocument directly.
                rDoc.SetRowHeightOnly( nStartRow, nEndRow, nTab, static_cast<sal_uInt16>(HMMToTwips(nNewHeight)) );
                rDoc.SetManualHeight( nStartRow, nEndRow, nTab, true );
            }
            else
                rFunc.SetWidthOrHeight(
                    false, aRowArr, nTab, SC_SIZE_ORIGINAL, static_cast<sal_uInt16>(HMMToTwips(nNewHeight)), true, true);
        }
    }
    else if ( aPropertyName == SC_UNONAME_CELLVIS )
    {
        bool bVis = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        ScSizeMode eMode = bVis ? SC_SIZE_SHOW : SC_SIZE_DIRECT;
        rFunc.SetWidthOrHeight(false, aRowArr, nTab, eMode, 0, true, true);
        //  SC_SIZE_DIRECT with size 0: hide
    }
    else if ( aPropertyName == SC_UNONAME_VISFLAG )
    {
        // #i116460# Shortcut to only set the flag, without drawing layer update etc.
        // Should only be used from import filters.
        rDoc.SetRowHidden(nStartRow, nEndRow, nTab, !ScUnoHelpFunctions::GetBoolFromAny( aValue ));
    }
    else if ( aPropertyName == SC_UNONAME_CELLFILT )
    {
        //! undo etc.
        if (ScUnoHelpFunctions::GetBoolFromAny( aValue ))
            rDoc.SetRowFiltered(nStartRow, nEndRow, nTab, true);
        else
            rDoc.SetRowFiltered(nStartRow, nEndRow, nTab, false);
    }
    else if ( aPropertyName == SC_UNONAME_NEWPAGE || aPropertyName == SC_UNONAME_MANPAGE )
    {
        //! single function to set/remove all breaks?
        bool bSet = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        for (SCROW nRow=nStartRow; nRow<=nEndRow; nRow++)
            if (bSet)
                rFunc.InsertPageBreak( false, ScAddress(0,nRow,nTab), true, true );
            else
                rFunc.RemovePageBreak( false, ScAddress(0,nRow,nTab), true, true );
    }
    else if ( aPropertyName == SC_UNONAME_CELLBACK || aPropertyName == SC_UNONAME_CELLTRAN )
    {
        // #i57867# Background color is specified for row styles in the file format,
        // so it has to be supported along with the row properties (import only).

        // Use ScCellRangeObj to set the property for all cells in the rows
        // (this means, the "row attribute" must be set before individual cell attributes).

        ScRange aRange( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab );
        uno::Reference<beans::XPropertySet> xRangeObj = new ScCellRangeObj( pDocShell, aRange );
        xRangeObj->setPropertyValue( aPropertyName, aValue );
    }
}

uno::Any SAL_CALL ScTableRowsObj::getPropertyValue( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
        throw uno::RuntimeException();

    ScDocument& rDoc = pDocShell->GetDocument();
    uno::Any aAny;

    //! loop over all rows for current state?

    if ( aPropertyName == SC_UNONAME_CELLHGT )
    {
        // for hidden row, return original height
        sal_uInt16 nHeight = rDoc.GetOriginalHeight( nStartRow, nTab );
        aAny <<= static_cast<sal_Int32>(TwipsToHMM(nHeight));
    }
    else if ( aPropertyName == SC_UNONAME_CELLVIS )
    {
        SCROW nLastRow;
        bool bVis = !rDoc.RowHidden(nStartRow, nTab, nullptr, &nLastRow);
        aAny <<= bVis;
    }
    else if ( aPropertyName == SC_UNONAME_CELLFILT )
    {
        bool bVis = rDoc.RowFiltered(nStartRow, nTab);
        aAny <<= bVis;
    }
    else if ( aPropertyName == SC_UNONAME_OHEIGHT )
    {
        bool bOpt = !(rDoc.GetRowFlags( nStartRow, nTab ) & CRFlags::ManualSize);
        aAny <<= bOpt;
    }
    else if ( aPropertyName == SC_UNONAME_NEWPAGE )
    {
        ScBreakType nBreak = rDoc.HasRowBreak(nStartRow, nTab);
        aAny <<= (nBreak != ScBreakType::NONE);
    }
    else if ( aPropertyName == SC_UNONAME_MANPAGE )
    {
        ScBreakType nBreak = rDoc.HasRowBreak(nStartRow, nTab);
        aAny <<= bool(nBreak & ScBreakType::Manual);
    }
    else if ( aPropertyName == SC_UNONAME_CELLBACK || aPropertyName == SC_UNONAME_CELLTRAN )
    {
        // Use ScCellRangeObj to get the property from the cell range
        // (for completeness only, this is not used by the XML filter).

        ScRange aRange( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab );
        uno::Reference<beans::XPropertySet> xRangeObj = new ScCellRangeObj( pDocShell, aRange );
        aAny = xRangeObj->getPropertyValue( aPropertyName );
    }

    return aAny;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScTableRowsObj )

ScSpreadsheetSettingsObj::~ScSpreadsheetSettingsObj()
{
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScSpreadsheetSettingsObj::getPropertySetInfo()
{
    return nullptr;
}

void SAL_CALL ScSpreadsheetSettingsObj::setPropertyValue(
                        const OUString& /* aPropertyName */, const uno::Any& /* aValue */ )
{
}

uno::Any SAL_CALL ScSpreadsheetSettingsObj::getPropertyValue( const OUString& /* aPropertyName */ )
{
    return uno::Any();
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScSpreadsheetSettingsObj )

ScAnnotationsObj::ScAnnotationsObj(ScDocShell* pDocSh, SCTAB nT) :
    pDocShell( pDocSh ),
    nTab( nT )
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScAnnotationsObj::~ScAnnotationsObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScAnnotationsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //! adjust nTab when updating references!!!

    if ( rHint.GetId() == SfxHintId::Dying )
    {
        pDocShell = nullptr;       // became invalid
    }
}

bool ScAnnotationsObj::GetAddressByIndex_Impl( sal_Int32 nIndex, ScAddress& rPos ) const
{
    if (!pDocShell)
        return false;

    ScDocument& rDoc = pDocShell->GetDocument();
    rPos = rDoc.GetNotePosition(nIndex, nTab);
    return rPos.IsValid();
}

ScAnnotationObj* ScAnnotationsObj::GetObjectByIndex_Impl( sal_Int32 nIndex ) const
{
    if (pDocShell)
    {
        ScAddress aPos;
        if ( GetAddressByIndex_Impl( nIndex, aPos ) )
            return new ScAnnotationObj( pDocShell, aPos );
    }
    return nullptr;
}

// XSheetAnnotations

void SAL_CALL ScAnnotationsObj::insertNew(
        const table::CellAddress& aPosition, const OUString& rText )
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        OSL_ENSURE( aPosition.Sheet == nTab, "addAnnotation with a wrong Sheet" );
        ScAddress aPos( static_cast<SCCOL>(aPosition.Column), static_cast<SCROW>(aPosition.Row), nTab );
        pDocShell->GetDocFunc().ReplaceNote( aPos, rText, nullptr, nullptr, true );
    }
}

void SAL_CALL ScAnnotationsObj::removeByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        ScAddress aPos;
        if ( GetAddressByIndex_Impl( nIndex, aPos ) )
        {
            ScMarkData aMarkData;
            aMarkData.SelectTable( aPos.Tab(), true );
            aMarkData.SetMultiMarkArea( ScRange(aPos) );

            pDocShell->GetDocFunc().DeleteContents( aMarkData, InsertDeleteFlags::NOTE, true, true );
        }
    }
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScAnnotationsObj::createEnumeration()
{
    //! iterate directly (more efficiently)?

    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, "com.sun.star.sheet.CellAnnotationsEnumeration");
}

// XIndexAccess

sal_Int32 SAL_CALL ScAnnotationsObj::getCount()
{
    SolarMutexGuard aGuard;
    sal_Int32 nCount = 0;
    if (pDocShell)
    {
        const ScDocument& rDoc = pDocShell->GetDocument();
        for (SCCOL nCol : rDoc.GetColumnsRange(nTab, 0, MAXCOL))
            nCount += rDoc.GetNoteCount(nTab, nCol);
    }
    return nCount;
}

uno::Any SAL_CALL ScAnnotationsObj::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XSheetAnnotation> xAnnotation(GetObjectByIndex_Impl(nIndex));
    if (!xAnnotation.is())
        throw lang::IndexOutOfBoundsException();

    return uno::makeAny(xAnnotation);
}

uno::Type SAL_CALL ScAnnotationsObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<sheet::XSheetAnnotation>::get();
}

sal_Bool SAL_CALL ScAnnotationsObj::hasElements()
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

ScScenariosObj::ScScenariosObj(ScDocShell* pDocSh, SCTAB nT) :
    pDocShell( pDocSh ),
    nTab     ( nT )
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScScenariosObj::~ScScenariosObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScScenariosObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( dynamic_cast<const ScUpdateRefHint*>(&rHint) )
    {
        //! update of references for sheet and its start/end
    }
    else if ( rHint.GetId() == SfxHintId::Dying )
    {
        pDocShell = nullptr;       // became invalid
    }
}

// XScenarios

bool ScScenariosObj::GetScenarioIndex_Impl( const OUString& rName, SCTAB& rIndex )
{
    //! Case-insensitive ????

    if ( pDocShell )
    {
        OUString aTabName;
        ScDocument& rDoc = pDocShell->GetDocument();
        SCTAB nCount = static_cast<SCTAB>(getCount());
        for (SCTAB i=0; i<nCount; i++)
            if (rDoc.GetName( nTab+i+1, aTabName ))
                if (aTabName == rName)
                {
                    rIndex = i;
                    return true;
                }
    }

    return false;
}

ScTableSheetObj* ScScenariosObj::GetObjectByIndex_Impl(sal_Int32 nIndex)
{
    sal_uInt16 nCount = static_cast<sal_uInt16>(getCount());
    if ( pDocShell && nIndex >= 0 && nIndex < nCount )
        return new ScTableSheetObj( pDocShell, nTab+static_cast<SCTAB>(nIndex)+1 );

    return nullptr;    // no document or wrong index
}

ScTableSheetObj* ScScenariosObj::GetObjectByName_Impl(const OUString& aName)
{
    SCTAB nIndex;
    if ( pDocShell && GetScenarioIndex_Impl( aName, nIndex ) )
        return new ScTableSheetObj( pDocShell, nTab+nIndex+1 );

    return nullptr;    // not found
}

void SAL_CALL ScScenariosObj::addNewByName( const OUString& aName,
                                const uno::Sequence<table::CellRangeAddress>& aRanges,
                                const OUString& aComment )
{
    SolarMutexGuard aGuard;
    if ( pDocShell )
    {
        ScMarkData aMarkData;
        aMarkData.SelectTable( nTab, true );

        sal_uInt16 nRangeCount = static_cast<sal_uInt16>(aRanges.getLength());
        if (nRangeCount)
        {
            const table::CellRangeAddress* pAry = aRanges.getConstArray();
            for (sal_uInt16 i=0; i<nRangeCount; i++)
            {
                OSL_ENSURE( pAry[i].Sheet == nTab, "addScenario with a wrong Tab" );
                ScRange aRange( static_cast<SCCOL>(pAry[i].StartColumn), static_cast<SCROW>(pAry[i].StartRow), nTab,
                                static_cast<SCCOL>(pAry[i].EndColumn),   static_cast<SCROW>(pAry[i].EndRow),   nTab );

                aMarkData.SetMultiMarkArea( aRange );
            }
        }

        ScScenarioFlags const nFlags = ScScenarioFlags::ShowFrame | ScScenarioFlags::PrintFrame
                                     | ScScenarioFlags::TwoWay    | ScScenarioFlags::Protected;

        pDocShell->MakeScenario( nTab, aName, aComment, COL_LIGHTGRAY, nFlags, aMarkData );
    }
}

void SAL_CALL ScScenariosObj::removeByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    SCTAB nIndex;
    if ( pDocShell && GetScenarioIndex_Impl( aName, nIndex ) )
        pDocShell->GetDocFunc().DeleteTable( nTab+nIndex+1, true );
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScScenariosObj::createEnumeration()
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, "com.sun.star.sheet.ScenariosEnumeration");
}

// XIndexAccess

sal_Int32 SAL_CALL ScScenariosObj::getCount()
{
    SolarMutexGuard aGuard;
    SCTAB nCount = 0;
    if ( pDocShell )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        if (!rDoc.IsScenario(nTab))
        {
            SCTAB nTabCount = rDoc.GetTableCount();
            SCTAB nNext = nTab + 1;
            while (nNext < nTabCount && rDoc.IsScenario(nNext))
            {
                ++nCount;
                ++nNext;
            }
        }
    }
    return nCount;
}

uno::Any SAL_CALL ScScenariosObj::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XScenario> xScen(GetObjectByIndex_Impl(nIndex));
    if (!xScen.is())
        throw lang::IndexOutOfBoundsException();

    return uno::makeAny(xScen);
}

uno::Type SAL_CALL ScScenariosObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<sheet::XScenario>::get();
}

sal_Bool SAL_CALL ScScenariosObj::hasElements()
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

uno::Any SAL_CALL ScScenariosObj::getByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XScenario> xScen(GetObjectByName_Impl(aName));
    if (!xScen.is())
        throw container::NoSuchElementException();

    return uno::makeAny(xScen);
}

uno::Sequence<OUString> SAL_CALL ScScenariosObj::getElementNames()
{
    SolarMutexGuard aGuard;
    SCTAB nCount = static_cast<SCTAB>(getCount());
    uno::Sequence<OUString> aSeq(nCount);

    if ( pDocShell )    // otherwise Count = 0
    {
        OUString aTabName;
        ScDocument& rDoc = pDocShell->GetDocument();
        OUString* pAry = aSeq.getArray();
        for (SCTAB i=0; i<nCount; i++)
            if (rDoc.GetName( nTab+i+1, aTabName ))
                pAry[i] = aTabName;
    }

    return aSeq;
}

sal_Bool SAL_CALL ScScenariosObj::hasByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    SCTAB nIndex;
    return GetScenarioIndex_Impl( aName, nIndex );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
