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

#include "scitems.hxx"
#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <svx/fmdpage.hxx>
#include <svx/fmview.hxx>
#include <svx/svditer.hxx>
#include <svx/svdpage.hxx>
#include <svx/svxids.hrc>
#include <svx/unoshape.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Calc.hxx>
#include <svl/numuno.hxx>
#include <svl/smplhint.hxx>
#include <unotools/moduleoptions.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <vcl/waitobj.hxx>
#include <unotools/charclass.hxx>
#include <tools/multisel.hxx>
#include <tools/resary.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <unotools/saveopt.hxx>

#include <ctype.h>
#include <float.h>

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
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/string.hxx>
#include <cppuhelper/supportsservice.hxx>
#if HAVE_FEATURE_OPENCL
#include <opencl/platforminfo.hxx>
#endif
#include <sfx2/lokhelper.hxx>

#include "cellsuno.hxx"
#include <columnspanset.hxx>
#include "convuno.hxx"
#include "datauno.hxx"
#include "docfunc.hxx"
#include "dociter.hxx"
#include "docoptio.hxx"
#include "docsh.hxx"
#include "docuno.hxx"
#include "drwlayer.hxx"
#include "forbiuno.hxx"
#include "formulacell.hxx"
#include "formulagroup.hxx"
#include "gridwin.hxx"
#include "hints.hxx"
#include <inputhdl.hxx>
#include <inputopt.hxx>
#include "interpre.hxx"
#include "linkuno.hxx"
#include "markdata.hxx"
#include "miscuno.hxx"
#include "nameuno.hxx"
#include "notesuno.hxx"
#include "optuno.hxx"
#include "pfuncache.hxx"
#include "postit.hxx"
#include "printfun.hxx"
#include "rangeutl.hxx"
#include "scmod.hxx"
#include "scresid.hxx"
#include "servuno.hxx"
#include "shapeuno.hxx"
#include "sheetevents.hxx"
#include "styleuno.hxx"
#include "tabvwsh.hxx"
#include "targuno.hxx"
#include "unonames.hxx"
#include "ViewSettingsSequenceDefines.hxx"
#include "viewuno.hxx"
#include "editsh.hxx"
#include "drawsh.hxx"
#include "drtxtob.hxx"
#include "transobj.hxx"

#include "sc.hrc"

using namespace com::sun::star;

// #i111553# provides the name of the VBA constant for this document type (e.g. 'ThisExcelDoc' for Calc)
#define SC_UNO_VBAGLOBNAME "VBAGlobalConstantName"

//  alles ohne Which-ID, Map nur fuer PropertySetInfo

//! umbenennen, sind nicht mehr nur Options
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

//! StandardDecimals als Property und vom NumberFormatter ????????

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

    ResStringArray aStrings( ScResId( SCSTR_PRINT_OPTIONS ) );
    OSL_ENSURE( aStrings.Count() >= 10, "resource incomplete" );
    if( aStrings.Count() < 10 ) // bad resource ?
        return;

    sal_Int32 nNumProps= 9, nIdx = 0;

    m_aUIProperties.realloc(nNumProps);

    // load the writer PrinterOptions into the custom tab
    m_aUIProperties[nIdx].Name = "OptionsUIFile";
    m_aUIProperties[nIdx++].Value <<= OUString("modules/scalc/ui/printeroptions.ui");

    // create Section for spreadsheet (results in an extra tab page in dialog)
    SvtModuleOptions aOpt;
    OUString aAppGroupname( aStrings.GetString( 9 ) );
    aAppGroupname = aAppGroupname.replaceFirst( "%s", aOpt.GetModuleName( SvtModuleOptions::EModule::CALC ) );
    m_aUIProperties[nIdx++].Value = setGroupControlOpt("tabcontrol-page2", aAppGroupname, OUString());

    // show subgroup for pages
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("pages", OUString(aStrings.GetString(0)), OUString());

    // create a bool option for empty pages
    m_aUIProperties[nIdx++].Value = setBoolControlOpt("suppressemptypages", OUString( aStrings.GetString( 1 ) ),
                                                  ".HelpID:vcl:PrintDialog:IsSuppressEmptyPages:CheckBox",
                                                  "IsSuppressEmptyPages",
                                                  bSuppress);
    // show Subgroup for print content
    vcl::PrinterOptionsHelper::UIControlOptions aPrintRangeOpt;
    aPrintRangeOpt.maGroupHint = "PrintRange";
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("printrange", OUString(aStrings.GetString(2)),
                                                      OUString(),
                                                      aPrintRangeOpt);

    // create a choice for the content to create
    uno::Sequence< OUString > aChoices( 3 ), aHelpIds( 3 ), aWidgetIds( 3 );
    aChoices[0] = aStrings.GetString( 3 );
    aHelpIds[0] = ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:0";
    aWidgetIds[0] = "printallsheets";
    aChoices[1] = aStrings.GetString( 4 );
    aHelpIds[1] = ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:1";
    aWidgetIds[1] = "printselectedsheets";
    aChoices[2] = aStrings.GetString( 5 );
    aHelpIds[2] = ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:2";
    aWidgetIds[2] = "printselectedcells";
    m_aUIProperties[nIdx++].Value = setChoiceRadiosControlOpt(aWidgetIds, OUString(),
                                                    aHelpIds, "PrintContent",
                                                    aChoices, nContent );

    // show Subgroup for print range
    aPrintRangeOpt.mbInternalOnly = true;
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("fromwhich", OUString(aStrings.GetString(6)),
                                                      OUString(),
                                                      aPrintRangeOpt);

    // create a choice for the range to print
    OUString aPrintRangeName( "PrintRange" );
    aChoices.realloc( 2 );
    aHelpIds.realloc( 2 );
    aWidgetIds.realloc( 2 );
    aChoices[0] = aStrings.GetString( 7 );
    aHelpIds[0] = ".HelpID:vcl:PrintDialog:PrintRange:RadioButton:0";
    aWidgetIds[0] = "printallpages";
    aChoices[1] = aStrings.GetString( 8 );
    aHelpIds[1] = ".HelpID:vcl:PrintDialog:PrintRange:RadioButton:1";
    aWidgetIds[1] = "printpages";
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

    for (sal_Int32 nUIPos=0; nUIPos<m_aUIProperties.getLength(); ++nUIPos)
    {
        uno::Sequence<beans::PropertyValue> aUIProp;
        if ( m_aUIProperties[nUIPos].Value >>= aUIProp )
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
                            ScUnoHelpFunctions::SetBoolInAny( aPropertyValue.Value, bSuppress );
                            aUIProp[nPropPos].Value <<= aPropertyValue;
                        }
                    }
                }
            }
            m_aUIProperties[nUIPos].Value <<= aUIProp;
        }
    }
}

void ScModelObj::CreateAndSet(ScDocShell* pDocSh)
{
    if (pDocSh)
        pDocSh->SetBaseModel( new ScModelObj(pDocSh) );
}

ScModelObj::ScModelObj( ScDocShell* pDocSh ) :
    SfxBaseModel( pDocSh ),
    aPropSet( lcl_GetDocOptPropertyMap() ),
    pDocShell( pDocSh ),
    pPrintFuncCache( nullptr ),
    pPrinterOptions( nullptr ),
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

    delete pPrintFuncCache;
    delete pPrinterOptions;
}

uno::Reference< uno::XAggregation> ScModelObj::GetFormatter()
{
    // pDocShell may be NULL if this is the base of a ScDocOptionsObj
    if ( !xNumberAgg.is() && pDocShell )
    {
        // setDelegator veraendert den RefCount, darum eine Referenz selber halten
        // (direkt am m_refCount, um sich beim release nicht selbst zu loeschen)
        osl_atomic_increment( &m_refCount );
        // waehrend des queryInterface braucht man ein Ref auf das
        // SvNumberFormatsSupplierObj, sonst wird es geloescht.
        uno::Reference<util::XNumberFormatsSupplier> xFormatter(new SvNumberFormatsSupplierObj(pDocShell->GetDocument().GetFormatTable() ));
        {
            xNumberAgg.set(uno::Reference<uno::XAggregation>( xFormatter, uno::UNO_QUERY ));
            // extra block to force deletion of the temporary before setDelegator
        }

        // beim setDelegator darf die zusaetzliche Ref nicht mehr existieren
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

void ScModelObj::AfterXMLLoading(bool bRet)
{
    if (pDocShell)
        pDocShell->AfterXMLLoading(bRet);
}

ScSheetSaveData* ScModelObj::GetSheetSaveData()
{
    if (pDocShell)
        return pDocShell->GetSheetSaveData();
    return nullptr;
}

void ScModelObj::RepaintRange( const ScRange& rRange )
{
    if (pDocShell)
        pDocShell->PostPaint( rRange, PAINT_GRID );
}

void ScModelObj::RepaintRange( const ScRangeList& rRange )
{
    if (pDocShell)
        pDocShell->PostPaint( rRange, PAINT_GRID );
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

Size ScModelObj::getDocumentSize()
{
    Size aSize(10, 10); // minimum size

    const ScViewData* pViewData = ScDocShell::GetViewData();
    if (!pViewData)
        return aSize;

    SCTAB nTab = pViewData->GetTabNo();
    SCCOL nEndCol = 0;
    SCROW nEndRow = 0;
    const ScDocument& rDoc = pDocShell->GetDocument();

    rDoc.GetTiledRenderingArea(nTab, nEndCol, nEndRow);

    // convert to twips
    aSize.setWidth(rDoc.GetColWidth(0, nEndCol, nTab));
    aSize.setHeight(rDoc.GetRowHeight(0, nEndRow, nTab));

    return aSize;
}

void ScModelObj::postKeyEvent(int nType, int nCharCode, int nKeyCode)
{
    SolarMutexGuard aGuard;

    // There seems to be no clear way of getting the grid window for this
    // particular document, hence we need to hope we get the right window.
    ScViewData* pViewData = ScDocShell::GetViewData();
    ScGridWindow* pGridWindow = pViewData->GetActiveWin();

    if (!pGridWindow)
        return;

    KeyEvent aEvent(nCharCode, nKeyCode, 0);

    switch (nType)
    {
    case LOK_KEYEVENT_KEYINPUT:
        pGridWindow->KeyInput(aEvent);
        break;
    case LOK_KEYEVENT_KEYUP:
        pGridWindow->KeyUp(aEvent);
        break;
    default:
        assert(false);
        break;
    }
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

    // Calc operates in pixels...
    Point aPos(nX * pViewData->GetPPTX(), nY * pViewData->GetPPTY());
    MouseEvent aEvent(aPos, nCount,
            MouseEventModifiers::SIMPLECLICK, nButtons, nModifier);

    switch (nType)
    {
    case LOK_MOUSEEVENT_MOUSEBUTTONDOWN:
        pGridWindow->MouseButtonDown(aEvent);

        // Invoke the context menu
        if (nButtons & MOUSE_RIGHT)
        {
            const CommandEvent aCEvt(aPos, CommandEventId::ContextMenu, true, nullptr);
            pGridWindow->Command(aCEvt);
        }
        break;
    case LOK_MOUSEEVENT_MOUSEBUTTONUP:
        pGridWindow->MouseButtonUp(aEvent);

        // sometimes MouseButtonDown captures mouse and starts tracking, and VCL
        // will not take care of releasing that with tiled rendering
        if (pGridWindow->IsTracking())
            pGridWindow->EndTracking(TrackingEventFlags::DontCallHdl);

        break;
    case LOK_MOUSEEVENT_MOUSEMOVE:
        pGridWindow->MouseMove(aEvent);
        break;
    default:
        assert(false);
        break;
    }
}

void ScModelObj::setTextSelection(int nType, int nX, int nY)
{
    SolarMutexGuard aGuard;

    ScViewData* pViewData = ScDocShell::GetViewData();
    ScTabViewShell* pViewShell = pViewData->GetViewShell();
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

    ScEditShell* pShell;
    ScDrawShell* pDrawShell;
    TransferableDataHelper aDataHelper;
    ScViewData* pViewData = ScDocShell::GetViewData();
    uno::Reference<datatransfer::XTransferable> xTransferable;

    if (( pShell = dynamic_cast<ScEditShell*>( pViewData->GetViewShell()->GetViewFrame()->GetDispatcher()->GetShell(0) )) )
        xTransferable = pShell->GetEditView()->GetTransferable();
    else if ( nullptr != dynamic_cast<ScDrawTextObjectBar*>( pViewData->GetViewShell()->GetViewFrame()->GetDispatcher()->GetShell(0) ))
    {
        ScDrawView* pView = pViewData->GetScDrawView();
        OutlinerView* pOutView = pView->GetTextEditOutlinerView();
        if (pOutView)
            xTransferable = pOutView->GetEditView().GetTransferable();
    }
    else if (( pDrawShell = dynamic_cast<ScDrawShell*>( pViewData->GetViewShell()->GetViewFrame()->GetDispatcher()->GetShell(0) )) )
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
    catch (const datatransfer::UnsupportedFlavorException e)
    {
        OSL_TRACE("Caught UnsupportedFlavorException '%s'", OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
        return OString();
    }
    catch (const css::uno::Exception& e)
    {
        OSL_TRACE("Caught UNO Exception '%s'", OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
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

    int nPixelX = nX * pViewData->GetPPTX();
    int nPixelY = nY * pViewData->GetPPTY();

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

void ScModelObj::setClientZoom(int nTilePixelWidth_, int nTilePixelHeight_, int nTileTwipWidth_, int nTileTwipHeight_)
{
    mnTilePixelWidth = nTilePixelWidth_;
    mnTilePixelHeight = nTilePixelHeight_;
    mnTileTwipWidth = nTileTwipWidth_;
    mnTileTwipHeight = nTileTwipHeight_;
}

OUString ScModelObj::getRowColumnHeaders(const Rectangle& rRectangle)
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

Pointer ScModelObj::getPointer()
{
    SolarMutexGuard aGuard;

    ScViewData* pViewData = ScDocShell::GetViewData();
    if (!pViewData)
        return Pointer();

    ScGridWindow* pGridWindow = pViewData->GetActiveWin();
    if (!pGridWindow)
        return Pointer();

    return pGridWindow->GetPointer();
}

void ScModelObj::initializeForTiledRendering(const css::uno::Sequence<css::beans::PropertyValue>& /*rArguments*/)
{
    SolarMutexGuard aGuard;

    // show us the text exactly
    ScInputOptions aInputOptions(SC_MOD()->GetInputOptions());
    aInputOptions.SetTextWysiwyg(true);
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
                                                throw(uno::RuntimeException, std::exception)
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

uno::Sequence<uno::Type> SAL_CALL ScModelObj::getTypes() throw(uno::RuntimeException, std::exception)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        uno::Sequence<uno::Type> aParentTypes(SfxBaseModel::getTypes());
        long nParentLen = aParentTypes.getLength();
        const uno::Type* pParentPtr = aParentTypes.getConstArray();

        uno::Sequence<uno::Type> aAggTypes;
        if ( GetFormatter().is() )
        {
            const uno::Type& rProvType = cppu::UnoType<lang::XTypeProvider>::get();
            uno::Any aNumProv(xNumberAgg->queryAggregation(rProvType));
            if(aNumProv.getValueType() == rProvType)
            {
                uno::Reference<lang::XTypeProvider> xNumProv(
                    *static_cast<uno::Reference<lang::XTypeProvider> const *>(aNumProv.getValue()));
                aAggTypes = xNumProv->getTypes();
            }
        }
        long nAggLen = aAggTypes.getLength();
        const uno::Type* pAggPtr = aAggTypes.getConstArray();

        const long nThisLen = 16;
        aTypes.realloc( nParentLen + nAggLen + nThisLen );
        uno::Type* pPtr = aTypes.getArray();
        pPtr[nParentLen + 0] = cppu::UnoType<sheet::XSpreadsheetDocument>::get();
        pPtr[nParentLen + 1] = cppu::UnoType<document::XActionLockable>::get();
        pPtr[nParentLen + 2] = cppu::UnoType<sheet::XCalculatable>::get();
        pPtr[nParentLen + 3] = cppu::UnoType<util::XProtectable>::get();
        pPtr[nParentLen + 4] = cppu::UnoType<drawing::XDrawPagesSupplier>::get();
        pPtr[nParentLen + 5] = cppu::UnoType<sheet::XGoalSeek>::get();
        pPtr[nParentLen + 6] = cppu::UnoType<sheet::XConsolidatable>::get();
        pPtr[nParentLen + 7] = cppu::UnoType<sheet::XDocumentAuditing>::get();
        pPtr[nParentLen + 8] = cppu::UnoType<style::XStyleFamiliesSupplier>::get();
        pPtr[nParentLen + 9] = cppu::UnoType<view::XRenderable>::get();
        pPtr[nParentLen +10] = cppu::UnoType<document::XLinkTargetSupplier>::get();
        pPtr[nParentLen +11] = cppu::UnoType<beans::XPropertySet>::get();
        pPtr[nParentLen +12] = cppu::UnoType<lang::XMultiServiceFactory>::get();
        pPtr[nParentLen +13] = cppu::UnoType<lang::XServiceInfo>::get();
        pPtr[nParentLen +14] = cppu::UnoType<util::XChangesNotifier>::get();
        pPtr[nParentLen +15] = cppu::UnoType<sheet::opencl::XOpenCLSelection>::get();

        long i;
        for (i=0; i<nParentLen; i++)
            pPtr[i] = pParentPtr[i];                    // parent types first

        for (i=0; i<nAggLen; i++)
            pPtr[nParentLen+nThisLen+i] = pAggPtr[i];   // aggregated types last
    }
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL ScModelObj::getImplementationId()
                                                    throw(uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

void ScModelObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    //  Not interested in reference update hints here

    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint )
    {
        const sal_uInt32 nId = pSimpleHint->GetId();
        if ( nId == SFX_HINT_DYING )
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

            DELETEZ( pPrintFuncCache );     // must be deleted because it has a pointer to the DocShell
        }
        else if ( nId == SFX_HINT_DATACHANGED )
        {
            //  cached data for rendering become invalid when contents change
            //  (if a broadcast is added to SetDrawModified, is has to be tested here, too)

            DELETEZ( pPrintFuncCache );

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
    }
    else if ( dynamic_cast<const ScPointerChangedHint*>(&rHint) )
    {
        sal_uInt16 nFlags = static_cast<const ScPointerChangedHint&>(rHint).GetFlags();
        if (nFlags & SC_POINTERCHANGED_NUMFMT)
        {
            //  NumberFormatter-Pointer am Uno-Objekt neu setzen

            if (GetFormatter().is())
            {
                SvNumberFormatsSupplierObj* pNumFmt =
                    SvNumberFormatsSupplierObj::getImplementation(
                        uno::Reference<util::XNumberFormatsSupplier>(xNumberAgg, uno::UNO_QUERY) );
                if ( pNumFmt && pDocShell )
                    pNumFmt->SetNumberFormatter( pDocShell->GetDocument().GetFormatTable() );
            }
        }
    }

    // always call parent - SfxBaseModel might need to handle the same hints again
    SfxBaseModel::Notify( rBC, rHint );     // SfxBaseModel is derived from SfxListener
}

// XSpreadsheetDocument

uno::Reference<sheet::XSpreadsheets> SAL_CALL ScModelObj::getSheets() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return new ScTableSheetsObj(pDocShell);
    return nullptr;
}

// XStyleFamiliesSupplier

uno::Reference<container::XNameAccess> SAL_CALL ScModelObj::getStyleFamilies()
                                                throw(uno::RuntimeException, std::exception)
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
    long nPropCount = rOptions.getLength();
    for (long i = 0; i < nPropCount; i++)
    {
        const beans::PropertyValue& rProp = pPropArray[i];
        OUString aPropName(rProp.Name);

        if (aPropName == SC_UNONAME_RENDERDEV)
        {
            uno::Reference<awt::XDevice> xRenderDevice(rProp.Value, uno::UNO_QUERY);
            if ( xRenderDevice.is() )
            {
                VCLXDevice* pDevice = VCLXDevice::GetImplementation( xRenderDevice );
                if ( pDevice )
                {
                    pRet = pDevice->GetOutputDevice();
                    pRet->SetDigitLanguage( SC_MOD()->GetOptDigitLanguage() );
                }
            }
        }
    }
    return pRet;
}

static bool lcl_ParseTarget( const OUString& rTarget, ScRange& rTargetRange, Rectangle& rTargetRect,
                        bool& rIsSheet, ScDocument* pDoc, SCTAB nSourceTab )
{
    // test in same order as in SID_CURRENTCELL execute

    ScAddress aAddress;
    ScRangeUtil aRangeUtil;
    SCTAB nNameTab;
    sal_Int32 nNumeric = 0;

    bool bRangeValid = false;
    bool bRectValid = false;

    if ( rTargetRange.Parse( rTarget, pDoc ) & SCA_VALID )
    {
        bRangeValid = true;             // range reference
    }
    else if ( aAddress.Parse( rTarget, pDoc ) & SCA_VALID )
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
        rTargetRange = ScAddress( 0, (SCROW)(nNumeric-1), nSourceTab );     // target row number is 1-based
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
                    SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
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
                                     ScPrintSelectionStatus& rStatus, OUString& rPagesStr ) const
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
    sal_Int32 nPrintRange = 0;          // all pages / pages
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
                            Rectangle aObjRect = pSdrObj->GetCurrentBoundRect();
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
        throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
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
    if ( !FillRenderMarkData( aSelection, rOptions, aMark, aStatus, aPagesStr ) )
        return 0;

    //  The same ScPrintFuncCache object in pPrintFuncCache is used as long as
    //  the same selection is used (aStatus) and the document isn't changed
    //  (pPrintFuncCache is cleared in Notify handler)

    if ( !pPrintFuncCache || !pPrintFuncCache->IsSameSelection( aStatus ) )
    {
        delete pPrintFuncCache;
        pPrintFuncCache = new ScPrintFuncCache( pDocShell, aMark, aStatus );
    }
    sal_Int32 nPages = pPrintFuncCache->GetPageCount();

    sal_Int32 nSelectCount = nPages;
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

uno::Sequence<beans::PropertyValue> SAL_CALL ScModelObj::getRenderer( sal_Int32 nSelRenderer,
                                    const uno::Any& aSelection, const uno::Sequence<beans::PropertyValue>& rOptions  )
                                throw (lang::IllegalArgumentException,
                                       uno::RuntimeException,
                                       std::exception)
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
    if ( FillRenderMarkData( aSelection, rOptions, aMark, aStatus, aPagesStr ) )
    {
        if ( !pPrintFuncCache || !pPrintFuncCache->IsSameSelection( aStatus ) )
        {
            delete pPrintFuncCache;
            pPrintFuncCache = new ScPrintFuncCache( pDocShell, aMark, aStatus );
        }
        nTotalPages = pPrintFuncCache->GetPageCount();
    }
    sal_Int32 nRenderer = lcl_GetRendererNum( nSelRenderer, aPagesStr, nTotalPages );
    if ( nRenderer < 0 )
    {
        if ( nSelRenderer == 0 )
        {
            // getRenderer(0) is used to query the settings, so it must always return something

            SCTAB nCurTab = 0;      //! use current sheet from view?
            ScPrintFunc aDefaultFunc( pDocShell, pDocShell->GetPrinter(), nCurTab );
            Size aTwips = aDefaultFunc.GetPageSize();
            awt::Size aPageSize( TwipsToHMM( aTwips.Width() ), TwipsToHMM( aTwips.Height() ) );

            uno::Sequence<beans::PropertyValue> aSequence(1);
            beans::PropertyValue* pArray = aSequence.getArray();
            pArray[0].Name = SC_UNONAME_PAGESIZE;
            pArray[0].Value <<= aPageSize;

            if( ! pPrinterOptions )
                pPrinterOptions = new ScPrintUIOptions;
            else
                pPrinterOptions->SetDefaults();
            pPrinterOptions->appendPrintUIOptions( aSequence );
            return aSequence;
        }
        else
            throw lang::IllegalArgumentException();
    }

    //  printer is used as device (just for page layout), draw view is not needed

    SCTAB nTab = pPrintFuncCache->GetTabForPage( nRenderer );

    ScRange aRange;
    const ScRange* pSelRange = nullptr;
    if ( aMark.IsMarked() )
    {
        aMark.GetMarkArea( aRange );
        pSelRange = &aRange;
    }
    ScPrintFunc aFunc( pDocShell, pDocShell->GetPrinter(), nTab,
                        pPrintFuncCache->GetFirstAttr(nTab), nTotalPages, pSelRange, &aStatus.GetOptions() );
    aFunc.SetRenderFlag( true );

    Range aPageRange( nRenderer+1, nRenderer+1 );
    MultiSelection aPage( aPageRange );
    aPage.SetTotalRange( Range(0,RANGE_MAX) );
    aPage.Select( aPageRange );

    long nDisplayStart = pPrintFuncCache->GetDisplayStart( nTab );
    long nTabStart = pPrintFuncCache->GetTabStart( nTab );

    (void)aFunc.DoPrint( aPage, nTabStart, nDisplayStart, false, nullptr );

    ScRange aCellRange;
    bool bWasCellRange = aFunc.GetLastSourceRange( aCellRange );
    Size aTwips = aFunc.GetPageSize();
    awt::Size aPageSize( TwipsToHMM( aTwips.Width() ), TwipsToHMM( aTwips.Height() ) );

    long nPropCount = bWasCellRange ? 3 : 2;
    uno::Sequence<beans::PropertyValue> aSequence(nPropCount);
    beans::PropertyValue* pArray = aSequence.getArray();
    pArray[0].Name = SC_UNONAME_PAGESIZE;
    pArray[0].Value <<= aPageSize;
    // #i111158# all positions are relative to the whole page, including non-printable area
    pArray[1].Name = SC_UNONAME_INC_NP_AREA;
    pArray[1].Value = uno::makeAny( true );
    if ( bWasCellRange )
    {
        table::CellRangeAddress aRangeAddress( nTab,
                        aCellRange.aStart.Col(), aCellRange.aStart.Row(),
                        aCellRange.aEnd.Col(), aCellRange.aEnd.Row() );
        pArray[2].Name = SC_UNONAME_SOURCERANGE;
        pArray[2].Value <<= aRangeAddress;
    }

    if( ! pPrinterOptions )
        pPrinterOptions = new ScPrintUIOptions;
    else
        pPrinterOptions->SetDefaults();
    pPrinterOptions->appendPrintUIOptions( aSequence );
    return aSequence;
}

void SAL_CALL ScModelObj::render( sal_Int32 nSelRenderer, const uno::Any& aSelection,
                                    const uno::Sequence<beans::PropertyValue>& rOptions )
                                throw(lang::IllegalArgumentException,
                                      uno::RuntimeException,
                                      std::exception)
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
    if ( !FillRenderMarkData( aSelection, rOptions, aMark, aStatus, aPagesStr ) )
        throw lang::IllegalArgumentException();

    if ( !pPrintFuncCache || !pPrintFuncCache->IsSameSelection( aStatus ) )
    {
        delete pPrintFuncCache;
        pPrintFuncCache = new ScPrintFuncCache( pDocShell, aMark, aStatus );
    }
    long nTotalPages = pPrintFuncCache->GetPageCount();
    sal_Int32 nRenderer = lcl_GetRendererNum( nSelRenderer, aPagesStr, nTotalPages );
    if ( nRenderer < 0 )
        throw lang::IllegalArgumentException();

    OutputDevice* pDev = lcl_GetRenderDevice( rOptions );
    if ( !pDev )
        throw lang::IllegalArgumentException();

    SCTAB nTab = pPrintFuncCache->GetTabForPage( nRenderer );
    ScDocument& rDoc = pDocShell->GetDocument();

    FmFormView* pDrawView = nullptr;

    ScDrawLayer* pModel = rDoc.GetDrawLayer();

    if( pModel )
    {
        pDrawView = new FmFormView( pModel, pDev );
        pDrawView->ShowSdrPage(pDrawView->GetModel()->GetPage(nTab));
        pDrawView->SetPrintPreview();
    }

    ScRange aRange;
    const ScRange* pSelRange = nullptr;
    if ( aMark.IsMarked() )
    {
        aMark.GetMarkArea( aRange );
        pSelRange = &aRange;
    }

    //  to increase performance, ScPrintState might be used here for subsequent
    //  pages of the same sheet

    ScPrintFunc aFunc( pDev, pDocShell, nTab, pPrintFuncCache->GetFirstAttr(nTab), nTotalPages, pSelRange, &aStatus.GetOptions() );
    aFunc.SetDrawView( pDrawView );
    aFunc.SetRenderFlag( true );
    if( aStatus.GetMode() == SC_PRINTSEL_RANGE_EXCLUSIVELY_OLE_AND_DRAW_OBJECTS )
        aFunc.SetExclusivelyDrawOleAndDrawObjects();

    Range aPageRange( nRenderer+1, nRenderer+1 );
    MultiSelection aPage( aPageRange );
    aPage.SetTotalRange( Range(0,RANGE_MAX) );
    aPage.Select( aPageRange );

    long nDisplayStart = pPrintFuncCache->GetDisplayStart( nTab );
    long nTabStart = pPrintFuncCache->GetTabStart( nTab );

    vcl::PDFExtOutDevData* pPDFData = dynamic_cast< vcl::PDFExtOutDevData* >(pDev->GetExtOutDevData() );
    if ( nRenderer == nTabStart )
    {
        // first page of a sheet: add outline item for the sheet name

        if ( pPDFData && pPDFData->GetIsExportBookmarks() )
        {
            // the sheet starts at the top of the page
            Rectangle aArea( pDev->PixelToLogic( Rectangle( 0,0,0,0 ) ) );
            sal_Int32 nDestID = pPDFData->CreateDest( aArea );
            OUString aTabName;
            rDoc.GetName( nTab, aTabName );
            sal_Int32 nParent = -1;     // top-level
            pPDFData->CreateOutlineItem( nParent, aTabName, nDestID );
        }
        // #i56629# add the named destination stuff
        if( pPDFData && pPDFData->GetIsExportNamedDestinations() )
        {
            Rectangle aArea( pDev->PixelToLogic( Rectangle( 0,0,0,0 ) ) );
            OUString aTabName;
            rDoc.GetName( nTab, aTabName );
            //need the PDF page number here
            pPDFData->CreateNamedDest( aTabName, aArea );
        }
    }

    (void)aFunc.DoPrint( aPage, nTabStart, nDisplayStart, true, nullptr );

    //  resolve the hyperlinks for PDF export

    if ( pPDFData )
    {
        //  iterate over the hyperlinks that were output for this page

        std::vector< vcl::PDFExtOutDevBookmarkEntry >& rBookmarks = pPDFData->GetBookmarks();
        std::vector< vcl::PDFExtOutDevBookmarkEntry >::iterator aIter = rBookmarks.begin();
        std::vector< vcl::PDFExtOutDevBookmarkEntry >::iterator aIEnd = rBookmarks.end();
        while ( aIter != aIEnd )
        {
            OUString aBookmark = aIter->aBookmark;
            if ( aBookmark.toChar() == (sal_Unicode) '#' )
            {
                //  try to resolve internal link

                OUString aTarget( aBookmark.copy( 1 ) );

                ScRange aTargetRange;
                Rectangle aTargetRect;      // 1/100th mm
                bool bIsSheet = false;
                bool bValid = lcl_ParseTarget( aTarget, aTargetRange, aTargetRect, bIsSheet, &rDoc, nTab );

                if ( bValid )
                {
                    sal_Int32 nPage = -1;
                    Rectangle aArea;
                    if ( bIsSheet )
                    {
                        //  Get first page for sheet (if nothing from that sheet is printed,
                        //  this page can show a different sheet)
                        nPage = pPrintFuncCache->GetTabStart( aTargetRange.aStart.Tab() );
                        aArea = pDev->PixelToLogic( Rectangle( 0,0,0,0 ) );
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
                            Rectangle aLocationMM = rDoc.GetMMRect(
                                       aLocRange.aStart.Col(), aLocRange.aStart.Row(),
                                       aLocRange.aEnd.Col(),   aLocRange.aEnd.Row(),
                                       aLocRange.aStart.Tab() );
                            Rectangle aLocationPixel = aLocation.aRectangle;

                            // Scale and move the target rectangle from aLocationMM to aLocationPixel,
                            // to get the target rectangle in pixels.

                            Fraction aScaleX( aLocationPixel.GetWidth(), aLocationMM.GetWidth() );
                            Fraction aScaleY( aLocationPixel.GetHeight(), aLocationMM.GetHeight() );

                            long nX1 = aLocationPixel.Left() + (long)
                                ( Fraction( aTargetRect.Left() - aLocationMM.Left(), 1 ) * aScaleX );
                            long nX2 = aLocationPixel.Left() + (long)
                                ( Fraction( aTargetRect.Right() - aLocationMM.Left(), 1 ) * aScaleX );
                            long nY1 = aLocationPixel.Top() + (long)
                                ( Fraction( aTargetRect.Top() - aLocationMM.Top(), 1 ) * aScaleY );
                            long nY2 = aLocationPixel.Top() + (long)
                                ( Fraction( aTargetRect.Bottom() - aLocationMM.Top(), 1 ) * aScaleY );

                            if ( nX1 > aLocationPixel.Right() ) nX1 = aLocationPixel.Right();
                            if ( nX2 > aLocationPixel.Right() ) nX2 = aLocationPixel.Right();
                            if ( nY1 > aLocationPixel.Bottom() ) nY1 = aLocationPixel.Bottom();
                            if ( nY2 > aLocationPixel.Bottom() ) nY2 = aLocationPixel.Bottom();

                            // The link target area is interpreted using the device's MapMode at
                            // the time of the CreateDest call, so PixelToLogic can be used here,
                            // regardless of the MapMode that is actually selected.

                            aArea = pDev->PixelToLogic( Rectangle( nX1, nY1, nX2, nY2 ) );
                        }
                    }

                    if ( nPage >= 0 )
                        pPDFData->SetLinkDest( aIter->nLinkId, pPDFData->CreateDest( aArea, nPage ) );
                }
            }
            else
            {
                //  external link, use as-is
                pPDFData->SetLinkURL( aIter->nLinkId, aBookmark );
            }
            ++aIter;
        }
        rBookmarks.clear();
    }

    if ( pDrawView )
        pDrawView->HideSdrPage();
    delete pDrawView;
}

// XLinkTargetSupplier

uno::Reference<container::XNameAccess> SAL_CALL ScModelObj::getLinks() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return new ScLinkTargetTypesObj(pDocShell);
    return nullptr;
}

// XActionLockable

sal_Bool SAL_CALL ScModelObj::isActionLocked() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bLocked = false;
    if (pDocShell)
        bLocked = ( pDocShell->GetLockCount() != 0 );
    return bLocked;
}

void SAL_CALL ScModelObj::addActionLock() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        pDocShell->LockDocument();
}

void SAL_CALL ScModelObj::removeActionLock() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        pDocShell->UnlockDocument();
}

void SAL_CALL ScModelObj::setActionLocks( sal_Int16 nLock ) throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        pDocShell->SetLockCount(nLock);
}

sal_Int16 SAL_CALL ScModelObj::resetActionLocks() throw(uno::RuntimeException, std::exception)
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

void SAL_CALL ScModelObj::lockControllers() throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SfxBaseModel::lockControllers();
    if (pDocShell)
        pDocShell->LockPaint();
}

void SAL_CALL ScModelObj::unlockControllers() throw (css::uno::RuntimeException, std::exception)
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

void SAL_CALL ScModelObj::calculate() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        pDocShell->DoRecalc(true);
    else
    {
        OSL_FAIL("keine DocShell");     //! Exception oder so?
    }
}

void SAL_CALL ScModelObj::calculateAll() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        pDocShell->DoHardRecalc(true);
    else
    {
        OSL_FAIL("keine DocShell");     //! Exception oder so?
    }
}

sal_Bool SAL_CALL ScModelObj::isAutomaticCalculationEnabled() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return pDocShell->GetDocument().GetAutoCalc();

    OSL_FAIL("keine DocShell");     //! Exception oder so?
    return false;
}

void SAL_CALL ScModelObj::enableAutomaticCalculation( sal_Bool bEnabledIn )
                                                throw(uno::RuntimeException, std::exception)
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
        OSL_FAIL("keine DocShell");     //! Exception oder so?
    }
}

// XProtectable

void SAL_CALL ScModelObj::protect( const OUString& aPassword ) throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    // #i108245# if already protected, don't change anything
    if ( pDocShell && !pDocShell->GetDocument().IsDocProtected() )
    {
        OUString aString(aPassword);
        pDocShell->GetDocFunc().Protect( TABLEID_DOC, aString, true );
    }
}

void SAL_CALL ScModelObj::unprotect( const OUString& aPassword )
                        throw(lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        OUString aString(aPassword);
        bool bDone = pDocShell->GetDocFunc().Unprotect( TABLEID_DOC, aString, true );
        if (!bDone)
            throw lang::IllegalArgumentException();
    }
}

sal_Bool SAL_CALL ScModelObj::isProtected() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return pDocShell->GetDocument().IsDocProtected();

    OSL_FAIL("keine DocShell");     //! Exception oder so?
    return false;
}

// XDrawPagesSupplier

uno::Reference<drawing::XDrawPages> SAL_CALL ScModelObj::getDrawPages() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return new ScDrawPagesObj(pDocShell);

    OSL_FAIL("keine DocShell");     //! Exception oder so?
    return nullptr;
}

// XGoalSeek

sheet::GoalResult SAL_CALL ScModelObj::seekGoal(
                                const table::CellAddress& aFormulaPosition,
                                const table::CellAddress& aVariablePosition,
                                const OUString& aGoalValue )
                                    throw (uno::RuntimeException,
                                           std::exception)
{
    SolarMutexGuard aGuard;
    sheet::GoalResult aResult;
    aResult.Divergence = DBL_MAX;       // nichts gefunden
    if (pDocShell)
    {
        WaitObject aWait( ScDocShell::GetActiveDialogParent() );
        OUString aGoalString(aGoalValue);
        ScDocument& rDoc = pDocShell->GetDocument();
        double fValue = 0.0;
        bool bFound = rDoc.Solver(
                    (SCCOL)aFormulaPosition.Column, (SCROW)aFormulaPosition.Row, aFormulaPosition.Sheet,
                    (SCCOL)aVariablePosition.Column, (SCROW)aVariablePosition.Row, aVariablePosition.Sheet,
                    aGoalString, fValue );
        aResult.Result = fValue;
        if (bFound)
            aResult.Divergence = 0.0;   //! das ist gelogen
    }
    return aResult;
}

// XConsolidatable

uno::Reference<sheet::XConsolidationDescriptor> SAL_CALL ScModelObj::createConsolidationDescriptor(
                                sal_Bool bEmpty ) throw(uno::RuntimeException, std::exception)
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
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    //  das koennte theoretisch ein fremdes Objekt sein, also nur das
    //  oeffentliche XConsolidationDescriptor Interface benutzen, um
    //  die Daten in ein ScConsolidationDescriptor Objekt zu kopieren:
    //! wenn es schon ein ScConsolidationDescriptor ist, direkt per getImplementation?

    uno::Reference< ScConsolidationDescriptor > xImpl(new ScConsolidationDescriptor);
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
        pDocShell->GetDocument().SetConsolidateDlgData( &rParam );
    }
}

// XDocumentAuditing

void SAL_CALL ScModelObj::refreshArrows() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        pDocShell->GetDocFunc().DetectiveRefresh();
}

// XViewDataSupplier
uno::Reference< container::XIndexAccess > SAL_CALL ScModelObj::getViewData(  )
    throw (uno::RuntimeException, std::exception)
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
            aSeq.realloc(1);
            OUString sName;
            pDocShell->GetDocument().GetName( pDocShell->GetDocument().GetVisibleTab(), sName );
            OUString sOUName(sName);
            aSeq[0].Name = SC_ACTIVETABLE;
            aSeq[0].Value <<= sOUName;
            xCont->insertByIndex( 0, uno::makeAny( aSeq ) );
        }
    }

    return xRet;
}

//  XPropertySet (Doc-Optionen)
//! auch an der Applikation anbieten?

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScModelObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScModelObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException,
          lang::IllegalArgumentException, lang::WrappedTargetException,
          uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    OUString aString(aPropertyName);

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
            if ( aString == SC_UNO_IGNORECASE ||
                 aString == SC_UNONAME_REGEXP ||
                 aString == SC_UNO_LOOKUPLABELS )
                bHardRecalc = false;
        }
        else if ( aString == SC_UNONAME_CLOCAL )
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
        else if ( aString == SC_UNO_CODENAME )
        {
            OUString sCodeName;
            if ( aValue >>= sCodeName )
                rDoc.SetCodeName( sCodeName );
        }
        else if ( aString == SC_UNO_CJK_CLOCAL )
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
        else if ( aString == SC_UNO_CTL_CLOCAL )
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
        else if ( aString == SC_UNO_APPLYFMDES )
        {
            //  model is created if not there
            ScDrawLayer* pModel = pDocShell->MakeDrawLayer();
            pModel->SetOpenInDesignMode( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );

            SfxBindings* pBindings = pDocShell->GetViewBindings();
            if (pBindings)
                pBindings->Invalidate( SID_FM_OPEN_READONLY );
        }
        else if ( aString == SC_UNO_AUTOCONTFOC )
        {
            //  model is created if not there
            ScDrawLayer* pModel = pDocShell->MakeDrawLayer();
            pModel->SetAutoControlFocus( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );

            SfxBindings* pBindings = pDocShell->GetViewBindings();
            if (pBindings)
                pBindings->Invalidate( SID_FM_AUTOCONTROLFOCUS );
        }
        else if ( aString == SC_UNO_ISLOADED )
        {
            pDocShell->SetEmpty( !ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        }
        else if ( aString == SC_UNO_ISUNDOENABLED )
        {
            bool bUndoEnabled = ScUnoHelpFunctions::GetBoolFromAny( aValue );
            rDoc.EnableUndo( bUndoEnabled );
            pDocShell->GetUndoManager()->SetMaxUndoActionCount(
                bUndoEnabled
                ? officecfg::Office::Common::Undo::Steps::get() : 0);
        }
        else if ( aString == SC_UNO_RECORDCHANGES )
        {
            bool bRecordChangesEnabled = ScUnoHelpFunctions::GetBoolFromAny( aValue );

            bool bChangeAllowed = true;
            if (!bRecordChangesEnabled)
                bChangeAllowed = !pDocShell->HasChangeRecordProtection();

            if (bChangeAllowed)
                pDocShell->SetChangeRecording(bRecordChangesEnabled);
        }
        else if ( aString == SC_UNO_ISADJUSTHEIGHTENABLED )
        {
            bool bOldAdjustHeightEnabled = rDoc.IsAdjustHeightEnabled();
            bool bAdjustHeightEnabled = ScUnoHelpFunctions::GetBoolFromAny( aValue );
            if( bOldAdjustHeightEnabled != bAdjustHeightEnabled )
                rDoc.EnableAdjustHeight( bAdjustHeightEnabled );
        }
        else if ( aString == SC_UNO_ISEXECUTELINKENABLED )
        {
            rDoc.EnableExecuteLink( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        }
        else if ( aString == SC_UNO_ISCHANGEREADONLYENABLED )
        {
            rDoc.EnableChangeReadOnly( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        }
        else if ( aString == "BuildId" )
        {
            aValue >>= maBuildId;
        }
        else if ( aString == "SavedObject" )    // set from chart after saving
        {
            OUString aObjName;
            aValue >>= aObjName;
            if ( !aObjName.isEmpty() )
                rDoc.RestoreChartListener( aObjName );
        }
        else if ( aString == SC_UNO_INTEROPGRABBAG )
        {
            setGrabBagItem(aValue);
        }

        if ( aNewOpt != rOldOpt )
        {
            rDoc.SetDocOptions( aNewOpt );
            //! Recalc only for options that need it?
            if ( bHardRecalc )
                pDocShell->DoHardRecalc( true );
            pDocShell->SetDocumentModified();
        }
    }
}

uno::Any SAL_CALL ScModelObj::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    OUString aString(aPropertyName);
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
        else if ( aString == SC_UNONAME_CLOCAL )
        {
            LanguageType eLatin, eCjk, eCtl;
            rDoc.GetLanguage( eLatin, eCjk, eCtl );

            lang::Locale aLocale;
            ScUnoConversion::FillLocale( aLocale, eLatin );
            aRet <<= aLocale;
        }
        else if ( aString == SC_UNO_CODENAME )
        {
            OUString sCodeName = rDoc.GetCodeName();
            aRet <<= sCodeName;
        }

        else if ( aString == SC_UNO_CJK_CLOCAL )
        {
            LanguageType eLatin, eCjk, eCtl;
            rDoc.GetLanguage( eLatin, eCjk, eCtl );

            lang::Locale aLocale;
            ScUnoConversion::FillLocale( aLocale, eCjk );
            aRet <<= aLocale;
        }
        else if ( aString == SC_UNO_CTL_CLOCAL )
        {
            LanguageType eLatin, eCjk, eCtl;
            rDoc.GetLanguage( eLatin, eCjk, eCtl );

            lang::Locale aLocale;
            ScUnoConversion::FillLocale( aLocale, eCtl );
            aRet <<= aLocale;
        }
        else if ( aString == SC_UNO_NAMEDRANGES )
        {
            aRet <<= uno::Reference<sheet::XNamedRanges>(new ScGlobalNamedRangesObj( pDocShell ));
        }
        else if ( aString == SC_UNO_DATABASERNG )
        {
            aRet <<= uno::Reference<sheet::XDatabaseRanges>(new ScDatabaseRangesObj( pDocShell ));
        }
        else if ( aString == SC_UNO_UNNAMEDDBRNG )
        {
            aRet <<= uno::Reference<sheet::XUnnamedDatabaseRanges>(new ScUnnamedDatabaseRangesObj(pDocShell));
        }
        else if ( aString == SC_UNO_COLLABELRNG )
        {
            aRet <<= uno::Reference<sheet::XLabelRanges>(new ScLabelRangesObj( pDocShell, true ));
        }
        else if ( aString == SC_UNO_ROWLABELRNG )
        {
            aRet <<= uno::Reference<sheet::XLabelRanges>(new ScLabelRangesObj( pDocShell, false ));
        }
        else if ( aString == SC_UNO_AREALINKS )
        {
            aRet <<= uno::Reference<sheet::XAreaLinks>(new ScAreaLinksObj( pDocShell ));
        }
        else if ( aString == SC_UNO_DDELINKS )
        {
            aRet <<= uno::Reference<container::XNameAccess>(new ScDDELinksObj( pDocShell ));
        }
        else if ( aString == SC_UNO_EXTERNALDOCLINKS )
        {
            aRet <<= uno::Reference<sheet::XExternalDocLinks>(new ScExternalDocLinksObj(pDocShell));
        }
        else if ( aString == SC_UNO_SHEETLINKS )
        {
            aRet <<= uno::Reference<container::XNameAccess>(new ScSheetLinksObj( pDocShell ));
        }
        else if ( aString == SC_UNO_APPLYFMDES )
        {
            // default for no model is TRUE
            ScDrawLayer* pModel = rDoc.GetDrawLayer();
            bool bOpenInDesign = pModel == nullptr || pModel->GetOpenInDesignMode();
            ScUnoHelpFunctions::SetBoolInAny( aRet, bOpenInDesign );
        }
        else if ( aString == SC_UNO_AUTOCONTFOC )
        {
            // default for no model is FALSE
            ScDrawLayer* pModel = rDoc.GetDrawLayer();
            bool bAutoControlFocus = pModel && pModel->GetAutoControlFocus();
            ScUnoHelpFunctions::SetBoolInAny( aRet, bAutoControlFocus );
        }
        else if ( aString == SC_UNO_FORBIDDEN )
        {
            aRet <<= uno::Reference<i18n::XForbiddenCharacters>(new ScForbiddenCharsObj( pDocShell ));
        }
        else if ( aString == SC_UNO_HASDRAWPAGES )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, (pDocShell->GetDocument().GetDrawLayer() != nullptr) );
        }
        else if ( aString == SC_UNO_BASICLIBRARIES )
        {
            aRet <<= pDocShell->GetBasicContainer();
        }
        else if ( aString == SC_UNO_DIALOGLIBRARIES )
        {
            aRet <<= pDocShell->GetDialogContainer();
        }
        else if ( aString == SC_UNO_VBAGLOBNAME )
        {
            /*  #i111553# This property provides the name of the constant that
                will be used to store this model in the global Basic manager.
                That constant will be equivalent to 'ThisComponent' but for
                each application, so e.g. a 'ThisExcelDoc' and a 'ThisWordDoc'
                constant can co-exist, as required by VBA. */
            aRet <<= OUString( "ThisExcelDoc" );
        }
        else if ( aString == SC_UNO_RUNTIMEUID )
        {
            aRet <<= getRuntimeUID();
        }
        else if ( aString == SC_UNO_HASVALIDSIGNATURES )
        {
            aRet <<= hasValidSignatures();
        }
        else if ( aString == SC_UNO_ISLOADED )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, !pDocShell->IsEmpty() );
        }
        else if ( aString == SC_UNO_ISUNDOENABLED )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, rDoc.IsUndoEnabled() );
        }
        else if ( aString == SC_UNO_RECORDCHANGES )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, pDocShell->IsChangeRecording() );
        }
        else if ( aString == SC_UNO_ISRECORDCHANGESPROTECTED )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, pDocShell->HasChangeRecordProtection() );
        }
        else if ( aString == SC_UNO_ISADJUSTHEIGHTENABLED )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, rDoc.IsAdjustHeightEnabled() );
        }
        else if ( aString == SC_UNO_ISEXECUTELINKENABLED )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, rDoc.IsExecuteLinkEnabled() );
        }
        else if ( aString == SC_UNO_ISCHANGEREADONLYENABLED )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, rDoc.IsChangeReadOnlyEnabled() );
        }
        else if ( aString == SC_UNO_REFERENCEDEVICE )
        {
            VCLXDevice* pXDev = new VCLXDevice();
            pXDev->SetOutputDevice( rDoc.GetRefDevice() );
            aRet <<= uno::Reference< awt::XDevice >( pXDev );
        }
        else if ( aString == "BuildId" )
        {
            aRet <<= maBuildId;
        }
        else if ( aString == "InternalDocument" )
        {
            ScUnoHelpFunctions::SetBoolInAny( aRet, (pDocShell->GetCreateMode() == SfxObjectCreateMode::INTERNAL) );
        }
        else if ( aString == SC_UNO_INTEROPGRABBAG )
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
    OUString aNameStr(aServiceSpecifier);
    ServiceType nType = ScServiceProvider::GetProviderType(aNameStr);
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
        //  alles was ich nicht kenn, werf ich der SvxFmMSFactory an den Hals,
        //  da wird dann 'ne Exception geworfen, wenn's nicht passt...

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
                                throw(uno::Exception, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return create(aServiceSpecifier, nullptr);
}

uno::Reference<uno::XInterface> SAL_CALL ScModelObj::createInstanceWithArguments(
                                const OUString& ServiceSpecifier,
                                const uno::Sequence<uno::Any>& aArgs )
                                throw(uno::Exception, uno::RuntimeException, std::exception)
{
    //! unterscheiden zwischen eigenen Services und denen vom Drawing-Layer?

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
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    //! warum sind die Parameter bei concatServiceNames nicht const ???
    //! return concatServiceNames( ScServiceProvider::GetAllServiceNames(),
    //!                            SvxFmMSFactory::getAvailableServiceNames() );

    uno::Sequence<OUString> aMyServices(ScServiceProvider::GetAllServiceNames());
    uno::Sequence<OUString> aDrawServices(SvxFmMSFactory::getAvailableServiceNames());

    return concatServiceNames( aMyServices, aDrawServices );
}

// XServiceInfo
OUString SAL_CALL ScModelObj::getImplementationName() throw(uno::RuntimeException, std::exception)
{
    return OUString( "ScModelObj" );
    /* // Matching the .component information:
       return OUString( "com.sun.star.comp.Calc.SpreadsheetDocument" );
    */
}

sal_Bool SAL_CALL ScModelObj::supportsService( const OUString& rServiceName )
                                                    throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScModelObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence<OUString> aRet(3);
    aRet[0] = SCMODELOBJ_SERVICE;
    aRet[1] = SCDOCSETTINGS_SERVICE;
    aRet[2] = SCDOC_SERVICE;
    return aRet;
}

// XUnoTunnel

sal_Int64 SAL_CALL ScModelObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException, std::exception)
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
        if(aNumTunnel.getValueType() == rTunnelType)
        {
            uno::Reference<lang::XUnoTunnel> xTunnelAgg(
                *static_cast<uno::Reference<lang::XUnoTunnel> const *>(aNumTunnel.getValue()));
            return xTunnelAgg->getSomething( rId );
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
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    maChangesListeners.addInterface( aListener );
}

void ScModelObj::removeChangesListener( const uno::Reference< util::XChangesListener >& aListener )
    throw (uno::RuntimeException, std::exception)
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

            ScRange aRange( *rRanges[ nIndex ] );
            if ( aRange.aStart == aRange.aEnd )
            {
                xRangeObj.set( new ScCellObj( pDocShell, aRange.aStart ) );
            }
            else
            {
                xRangeObj.set( new ScCellRangeObj( pDocShell, aRange ) );
            }

            util::ElementChange& rChange = aEvent.Changes[ static_cast< sal_Int32 >( nIndex ) ];
            rChange.Accessor <<= rOperation;
            rChange.Element <<= rProperties;
            rChange.ReplacedElement <<= xRangeObj;
        }

        ::cppu::OInterfaceIteratorHelper aIter( maChangesListeners );
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
        ScMarkData::iterator itr = aMarkData.begin(), itrEnd = aMarkData.end();
        for (; itr != itrEnd && *itr < nTabCount; ++itr)
        {
            SCTAB nTab = *itr;
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
                        ScRange aRange( *rRanges[ nIndex ] );
                        if ( aRange.aStart.Tab() == nTab )
                            aTabRanges.Append( aRange );
                    }
                    size_t nTabRangeCount = aTabRanges.size();
                    if ( nTabRangeCount > 0 )
                    {
                        uno::Reference<uno::XInterface> xTarget;
                        if ( nTabRangeCount == 1 )
                        {
                            ScRange aRange( *aTabRanges[ 0 ] );
                            if ( aRange.aStart == aRange.aEnd )
                                xTarget.set( static_cast<cppu::OWeakObject*>( new ScCellObj( pDocShell, aRange.aStart ) ) );
                            else
                                xTarget.set( static_cast<cppu::OWeakObject*>( new ScCellRangeObj( pDocShell, aRange ) ) );
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
        // (might also set a flag on SFX_EVENT_LOADFINISHED and only disable while loading)
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
    throw (uno::RuntimeException, std::exception)
{
    return officecfg::Office::Common::Misc::UseOpenCL::get();
}

void ScModelObj::enableOpenCL(sal_Bool bEnable)
    throw (uno::RuntimeException, std::exception)
{
    if (ScCalcConfig::isOpenCLEnabled() == static_cast<bool>(bEnable))
        return;

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::UseOpenCL::set(bEnable, batch);
    batch->commit();

    ScCalcConfig aConfig = ScInterpreter::GetGlobalConfig();
    if (bEnable)
        aConfig.setOpenCLConfigToDefault();
    ScInterpreter::SetGlobalConfig(aConfig);

    sc::FormulaGroupInterpreter::switchOpenCLDevice(OUString(), true, false);

    ScDocument* pDoc = GetDocument();
    pDoc->CheckVectorizationState();

}

void ScModelObj::enableAutomaticDeviceSelection(sal_Bool bForce)
    throw (uno::RuntimeException, std::exception)
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
    throw (uno::RuntimeException, std::exception)
{
    ScCalcConfig aConfig = ScInterpreter::GetGlobalConfig();
    aConfig.mbOpenCLAutoSelect = false;
    ScInterpreter::SetGlobalConfig(aConfig);
    ScFormulaOptions aOptions = SC_MOD()->GetFormulaOptions();
    aOptions.SetCalcConfig(aConfig);
    SC_MOD()->SetFormulaOptions(aOptions);
}

void ScModelObj::selectOpenCLDevice( sal_Int32 nPlatform, sal_Int32 nDevice )
    throw (uno::RuntimeException, std::exception)
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
    throw (uno::RuntimeException, std::exception)
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
    throw (uno::RuntimeException, std::exception)
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
    throw (uno::RuntimeException, std::exception)
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

void setOpcodeSubsetTest(bool bFlag)
    throw (uno::RuntimeException, std::exception)
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Calc::Formula::Calculation::OpenCLSubsetOnly::set(bFlag, batch);
    batch->commit();
}

}

void ScModelObj::enableOpcodeSubsetTest()
    throw (uno::RuntimeException, std::exception)
{
    setOpcodeSubsetTest(true);
}

void ScModelObj::disableOpcodeSubsetTest()
    throw (uno::RuntimeException, std::exception)
{
    setOpcodeSubsetTest(false);
}

sal_Bool ScModelObj::isOpcodeSubsetTested()
    throw (uno::RuntimeException, std::exception)
{
    return officecfg::Office::Calc::Formula::Calculation::OpenCLSubsetOnly::get();
}

void ScModelObj::setFormulaCellNumberLimit( sal_Int32 number )
    throw (uno::RuntimeException, std::exception)
{
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Calc::Formula::Calculation::OpenCLMinimumDataSize::set(number, batch);
    batch->commit();
}

sal_Int32 ScModelObj::getFormulaCellNumberLimit()
    throw (uno::RuntimeException, std::exception)
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
    //  Referenz-Update interessiert hier nicht

    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
    {
        pDocShell = nullptr;       // ungueltig geworden
    }
}

uno::Reference<drawing::XDrawPage> ScDrawPagesObj::GetObjectByIndex_Impl(sal_Int32 nIndex) const
{
    if (pDocShell)
    {
        ScDrawLayer* pDrawLayer = pDocShell->MakeDrawLayer();
        OSL_ENSURE(pDrawLayer,"kann Draw-Layer nicht anlegen");
        if ( pDrawLayer && nIndex >= 0 && nIndex < pDocShell->GetDocument().GetTableCount() )
        {
            SdrPage* pPage = pDrawLayer->GetPage((sal_uInt16)nIndex);
            OSL_ENSURE(pPage,"Draw-Page nicht gefunden");
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
                                            throw(uno::RuntimeException, std::exception)
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
                                            throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SvxDrawPage* pImp = SvxDrawPage::getImplementation( xPage );
    if ( pDocShell && pImp )
    {
        SdrPage* pPage = pImp->GetSdrPage();
        if (pPage)
        {
            SCTAB nPageNum = static_cast<SCTAB>(pPage->GetPageNum());
            pDocShell->GetDocFunc().DeleteTable( nPageNum, true, true );
        }
    }
}

// XIndexAccess

sal_Int32 SAL_CALL ScDrawPagesObj::getCount() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return pDocShell->GetDocument().GetTableCount();
    return 0;
}

uno::Any SAL_CALL ScDrawPagesObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<drawing::XDrawPage> xPage(GetObjectByIndex_Impl(nIndex));
    if (xPage.is())
        return uno::makeAny(xPage);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScDrawPagesObj::getElementType() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<drawing::XDrawPage>::get();
}

sal_Bool SAL_CALL ScDrawPagesObj::hasElements() throw(uno::RuntimeException, std::exception)
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
    //  Referenz-Update interessiert hier nicht

    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
    {
        pDocShell = nullptr;       // ungueltig geworden
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
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    if (pDocShell)
    {
        OUString aNamStr(aName);
        bDone = pDocShell->GetDocFunc().InsertTable( nPosition, aNamStr, true, true );
    }
    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

void SAL_CALL ScTableSheetsObj::moveByName( const OUString& aName, sal_Int16 nDestination )
    throw(uno::RuntimeException, std::exception)
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
        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    if (pDocShell)
    {
        OUString aNewStr(aCopy);
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

                bDone = pDocShell->GetDocFunc().RenameTable( nResultTab, aNewStr,
                                                             true, true );
            }
        }
    }
    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

void SAL_CALL ScTableSheetsObj::insertByName( const OUString& aName, const uno::Any& aElement )
                            throw(lang::IllegalArgumentException, container::ElementExistException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
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
            if ( pSheetObj && !pSheetObj->GetDocShell() )   // noch nicht eingefuegt?
            {
                ScDocument& rDoc = pDocShell->GetDocument();
                OUString aNamStr(aName);
                SCTAB nDummy;
                if ( rDoc.GetTable( aNamStr, nDummy ) )
                {
                    //  name already exists
                    throw container::ElementExistException();
                }
                else
                {
                    SCTAB nPosition = rDoc.GetTableCount();
                    bDone = pDocShell->GetDocFunc().InsertTable( nPosition, aNamStr,
                                                                 true, true );
                    if (bDone)
                        pSheetObj->InitInsertSheet( pDocShell, nPosition );
                    //  Dokument und neuen Range am Objekt setzen
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
            throw uno::RuntimeException();      // ElementExistException is handled above
    }
}

void SAL_CALL ScTableSheetsObj::replaceByName( const OUString& aName, const uno::Any& aElement )
                            throw(lang::IllegalArgumentException, container::NoSuchElementException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
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
            if ( pSheetObj && !pSheetObj->GetDocShell() )   // noch nicht eingefuegt?
            {
                SCTAB nPosition;
                if ( pDocShell->GetDocument().GetTable( aName, nPosition ) )
                {
                    if ( pDocShell->GetDocFunc().DeleteTable( nPosition, true, true ) )
                    {
                        //  InsertTable kann jetzt eigentlich nicht schiefgehen...
                        OUString aNamStr(aName);
                        bDone = pDocShell->GetDocFunc().InsertTable( nPosition, aNamStr, true, true );
                        if (bDone)
                            pSheetObj->InitInsertSheet( pDocShell, nPosition );
                    }
                }
                else
                {
                    //  not found
                    throw container::NoSuchElementException();
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
                                throw(container::NoSuchElementException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    if (pDocShell)
    {
        SCTAB nIndex;
        if ( pDocShell->GetDocument().GetTable( aName, nIndex ) )
            bDone = pDocShell->GetDocFunc().DeleteTable( nIndex, true, true );
        else // not found
            throw container::NoSuchElementException();
    }

    if (!bDone)
        throw uno::RuntimeException();      // NoSuchElementException is handled above
}

sal_Int32 ScTableSheetsObj::importSheet(
    const uno::Reference < sheet::XSpreadsheetDocument > & xDocSrc,
    const OUString& srcName, const sal_Int32 nDestPosition )
        throw( lang::IllegalArgumentException, lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception )
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

    // Transfert Tab
    bool bInsertNew = true;
    bool bNotifyAndPaint = true;
    pDocShell->TransferTab(
        *pDocShellSrc, nIndexSrc, nIndexDest, bInsertNew, bNotifyAndPaint );

    return nIndexDest;
}

// XCellRangesAccess

uno::Reference< table::XCell > SAL_CALL ScTableSheetsObj::getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow, sal_Int32 nSheet )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XCellRange> xSheet(static_cast<ScCellRangeObj*>(GetObjectByIndex_Impl((sal_uInt16)nSheet)));
    if (! xSheet.is())
        throw lang::IndexOutOfBoundsException();

    return xSheet->getCellByPosition(nColumn, nRow);
}

uno::Reference< table::XCellRange > SAL_CALL ScTableSheetsObj::getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom, sal_Int32 nSheet )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XCellRange> xSheet(static_cast<ScCellRangeObj*>(GetObjectByIndex_Impl((sal_uInt16)nSheet)));
    if (! xSheet.is())
        throw lang::IndexOutOfBoundsException();

    return xSheet->getCellRangeByPosition(nLeft, nTop, nRight, nBottom);
}

uno::Sequence < uno::Reference< table::XCellRange > > SAL_CALL ScTableSheetsObj::getCellRangesByName( const OUString& aRange )
    throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Sequence < uno::Reference < table::XCellRange > > xRet;

    ScRangeList aRangeList;
    ScDocument& rDoc = pDocShell->GetDocument();
    if (ScRangeStringConverter::GetRangeListFromString( aRangeList, aRange, &rDoc, ::formula::FormulaGrammar::CONV_OOO, ';' ))
    {
        size_t nCount = aRangeList.size();
        if (nCount)
        {
            xRet.realloc(nCount);
            for( size_t nIndex = 0; nIndex < nCount; nIndex++ )
            {
                const ScRange* pRange = aRangeList[ nIndex ];
                if( pRange )
                    xRet[nIndex] = new ScCellRangeObj(pDocShell, *pRange);
            }
        }
        else
            throw lang::IllegalArgumentException();
    }
    else
        throw lang::IllegalArgumentException();
    return xRet;
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScTableSheetsObj::createEnumeration()
                                                    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.sheet.SpreadsheetsEnumeration"));
}

// XIndexAccess

sal_Int32 SAL_CALL ScTableSheetsObj::getCount() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return pDocShell->GetDocument().GetTableCount();
    return 0;
}

uno::Any SAL_CALL ScTableSheetsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XSpreadsheet> xSheet(GetObjectByIndex_Impl(nIndex));
    if (xSheet.is())
        return uno::makeAny(xSheet);
    else
        throw lang::IndexOutOfBoundsException();
//    return uno::Any();
}

uno::Type SAL_CALL ScTableSheetsObj::getElementType() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<sheet::XSpreadsheet>::get();
}

sal_Bool SAL_CALL ScTableSheetsObj::hasElements() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

// XNameAccess

uno::Any SAL_CALL ScTableSheetsObj::getByName( const OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XSpreadsheet> xSheet(GetObjectByName_Impl(aName));
    if (xSheet.is())
        return uno::makeAny(xSheet);
    else
        throw container::NoSuchElementException();
}

uno::Sequence<OUString> SAL_CALL ScTableSheetsObj::getElementNames()
                                                throw(uno::RuntimeException, std::exception)
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
                                        throw(uno::RuntimeException, std::exception)
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
        //! Referenz-Update fuer Tab und Start/Ende
    }
    else if ( dynamic_cast<const SfxSimpleHint*>(&rHint) &&
            static_cast<const SfxSimpleHint&>(rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = nullptr;       // ungueltig geworden
    }
}

// XTableColumns

ScTableColumnObj* ScTableColumnsObj::GetObjectByIndex_Impl(sal_Int32 nIndex) const
{
    SCCOL nCol = static_cast<SCCOL>(nIndex) + nStartCol;
    if ( pDocShell && nCol <= nEndCol )
        return new ScTableColumnObj( pDocShell, nCol, nTab );

    return nullptr;    // falscher Index
}

ScTableColumnObj* ScTableColumnsObj::GetObjectByName_Impl(const OUString& aName) const
{
    SCCOL nCol = 0;
    OUString aString(aName);
    if ( ::AlphaToCol( nCol, aString) )
        if ( pDocShell && nCol >= nStartCol && nCol <= nEndCol )
            return new ScTableColumnObj( pDocShell, nCol, nTab );

    return nullptr;
}

void SAL_CALL ScTableColumnsObj::insertByIndex( sal_Int32 nPosition, sal_Int32 nCount )
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    if ( pDocShell && nCount > 0 && nPosition >= 0 && nStartCol+nPosition <= nEndCol &&
            nStartCol+nPosition+nCount-1 <= MAXCOL )
    {
        ScRange aRange( (SCCOL)(nStartCol+nPosition), 0, nTab,
                        (SCCOL)(nStartCol+nPosition+nCount-1), MAXROW, nTab );
        bDone = pDocShell->GetDocFunc().InsertCells( aRange, nullptr, INS_INSCOLS_BEFORE, true, true );
    }
    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

void SAL_CALL ScTableColumnsObj::removeByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    //  Der zu loeschende Bereich muss innerhalb des Objekts liegen
    if ( pDocShell && nCount > 0 && nIndex >= 0 && nStartCol+nIndex+nCount-1 <= nEndCol )
    {
        ScRange aRange( (SCCOL)(nStartCol+nIndex), 0, nTab,
                        (SCCOL)(nStartCol+nIndex+nCount-1), MAXROW, nTab );
        bDone = pDocShell->GetDocFunc().DeleteCells( aRange, nullptr, DEL_DELCOLS, true, true );
    }
    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScTableColumnsObj::createEnumeration()
                                                    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.table.TableColumnsEnumeration"));
}

// XIndexAccess

sal_Int32 SAL_CALL ScTableColumnsObj::getCount() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return nEndCol - nStartCol + 1;
}

uno::Any SAL_CALL ScTableColumnsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XCellRange> xColumn(GetObjectByIndex_Impl(nIndex));
    if (xColumn.is())
        return uno::makeAny(xColumn);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScTableColumnsObj::getElementType() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<table::XCellRange>::get();
}

sal_Bool SAL_CALL ScTableColumnsObj::hasElements() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

uno::Any SAL_CALL ScTableColumnsObj::getByName( const OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XCellRange> xColumn(GetObjectByName_Impl(aName));
    if (xColumn.is())
        return uno::makeAny(xColumn);
    else
        throw container::NoSuchElementException();
}

uno::Sequence<OUString> SAL_CALL ScTableColumnsObj::getElementNames()
                                                throw(uno::RuntimeException, std::exception)
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
                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SCCOL nCol = 0;
    OUString aString(aName);
    if ( ::AlphaToCol( nCol, aString) )
        if ( pDocShell && nCol >= nStartCol && nCol <= nEndCol )
            return true;

    return false;       // nicht gefunden
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScTableColumnsObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( lcl_GetColumnsPropertyMap() ));
    return aRef;
}

void SAL_CALL ScTableColumnsObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
        throw uno::RuntimeException();

    std::vector<sc::ColRowSpan> aColArr(1, sc::ColRowSpan(nStartCol,nEndCol));
    OUString aNameString(aPropertyName);
    ScDocFunc& rFunc = pDocShell->GetDocFunc();

    if ( aNameString == SC_UNONAME_CELLWID )
    {
        sal_Int32 nNewWidth = 0;
        if ( aValue >>= nNewWidth )
            rFunc.SetWidthOrHeight(
                true, aColArr, nTab, SC_SIZE_ORIGINAL, (sal_uInt16)HMMToTwips(nNewWidth), true, true);
    }
    else if ( aNameString == SC_UNONAME_CELLVIS )
    {
        bool bVis = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        ScSizeMode eMode = bVis ? SC_SIZE_SHOW : SC_SIZE_DIRECT;
        rFunc.SetWidthOrHeight(true, aColArr, nTab, eMode, 0, true, true);
        //  SC_SIZE_DIRECT with size 0: hide
    }
    else if ( aNameString == SC_UNONAME_OWIDTH )
    {
        bool bOpt = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        if (bOpt)
            rFunc.SetWidthOrHeight(
                true, aColArr, nTab, SC_SIZE_OPTIMAL, STD_EXTRA_WIDTH, true, true);
        // sal_False for columns currently has no effect
    }
    else if ( aNameString == SC_UNONAME_NEWPAGE || aNameString == SC_UNONAME_MANPAGE )
    {
        //! single function to set/remove all breaks?
        bool bSet = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        for (SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++)
            if (bSet)
                rFunc.InsertPageBreak( true, ScAddress(nCol,0,nTab), true, true, true );
            else
                rFunc.RemovePageBreak( true, ScAddress(nCol,0,nTab), true, true, true );
    }
}

uno::Any SAL_CALL ScTableColumnsObj::getPropertyValue( const OUString& aPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException,
          uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
        throw uno::RuntimeException();

    ScDocument& rDoc = pDocShell->GetDocument();
    OUString aNameString(aPropertyName);
    uno::Any aAny;

    //! loop over all columns for current state?

    if ( aNameString == SC_UNONAME_CELLWID )
    {
        // for hidden column, return original height
        sal_uInt16 nWidth = rDoc.GetOriginalWidth( nStartCol, nTab );
        aAny <<= (sal_Int32)TwipsToHMM(nWidth);
    }
    else if ( aNameString == SC_UNONAME_CELLVIS )
    {
        bool bVis = !rDoc.ColHidden(nStartCol, nTab);
        ScUnoHelpFunctions::SetBoolInAny( aAny, bVis );
    }
    else if ( aNameString == SC_UNONAME_OWIDTH )
    {
        bool bOpt = !(rDoc.GetColFlags( nStartCol, nTab ) & CR_MANUALSIZE);
        ScUnoHelpFunctions::SetBoolInAny( aAny, bOpt );
    }
    else if ( aNameString == SC_UNONAME_NEWPAGE )
    {
        ScBreakType nBreak = rDoc.HasColBreak(nStartCol, nTab);
        ScUnoHelpFunctions::SetBoolInAny( aAny, nBreak != BREAK_NONE );
    }
    else if ( aNameString == SC_UNONAME_MANPAGE )
    {
        ScBreakType nBreak = rDoc.HasColBreak(nStartCol, nTab);
        ScUnoHelpFunctions::SetBoolInAny( aAny, (nBreak & BREAK_MANUAL) != 0 );
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
        //! Referenz-Update fuer Tab und Start/Ende
    }
    else if ( dynamic_cast<const SfxSimpleHint*>(&rHint) &&
            static_cast<const SfxSimpleHint&>(rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = nullptr;       // ungueltig geworden
    }
}

// XTableRows

ScTableRowObj* ScTableRowsObj::GetObjectByIndex_Impl(sal_Int32 nIndex) const
{
    SCROW nRow = static_cast<SCROW>(nIndex) + nStartRow;
    if ( pDocShell && nRow <= nEndRow )
        return new ScTableRowObj( pDocShell, nRow, nTab );

    return nullptr;    // falscher Index
}

void SAL_CALL ScTableRowsObj::insertByIndex( sal_Int32 nPosition, sal_Int32 nCount )
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    if ( pDocShell && nCount > 0 && nPosition >= 0 && nStartRow+nPosition <= nEndRow &&
            nStartRow+nPosition+nCount-1 <= MAXROW )
    {
        ScRange aRange( 0, (SCROW)(nStartRow+nPosition), nTab,
                        MAXCOL, (SCROW)(nStartRow+nPosition+nCount-1), nTab );
        bDone = pDocShell->GetDocFunc().InsertCells( aRange, nullptr, INS_INSROWS_BEFORE, true, true );
    }
    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

void SAL_CALL ScTableRowsObj::removeByIndex( sal_Int32 nIndex, sal_Int32 nCount )
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bDone = false;
    //  Der zu loeschende Bereich muss innerhalb des Objekts liegen
    if ( pDocShell && nCount > 0 && nIndex >= 0 && nStartRow+nIndex+nCount-1 <= nEndRow )
    {
        ScRange aRange( 0, (SCROW)(nStartRow+nIndex), nTab,
                        MAXCOL, (SCROW)(nStartRow+nIndex+nCount-1), nTab );
        bDone = pDocShell->GetDocFunc().DeleteCells( aRange, nullptr, DEL_DELROWS, true, true );
    }
    if (!bDone)
        throw uno::RuntimeException();      // no other exceptions specified
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScTableRowsObj::createEnumeration()
                                                    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.table.TableRowsEnumeration"));
}

// XIndexAccess

sal_Int32 SAL_CALL ScTableRowsObj::getCount() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return nEndRow - nStartRow + 1;
}

uno::Any SAL_CALL ScTableRowsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XCellRange> xRow(GetObjectByIndex_Impl(nIndex));
    if (xRow.is())
        return uno::makeAny(xRow);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScTableRowsObj::getElementType() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<table::XCellRange>::get();
}

sal_Bool SAL_CALL ScTableRowsObj::hasElements() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScTableRowsObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( lcl_GetRowsPropertyMap() ));
    return aRef;
}

void SAL_CALL ScTableRowsObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
        throw uno::RuntimeException();

    ScDocFunc& rFunc = pDocShell->GetDocFunc();
    ScDocument& rDoc = pDocShell->GetDocument();
    std::vector<sc::ColRowSpan> aRowArr(1, sc::ColRowSpan(nStartRow,nEndRow));
    OUString aNameString(aPropertyName);

    if ( aNameString == SC_UNONAME_OHEIGHT )
    {
        sal_Int32 nNewHeight = 0;
        if ( rDoc.IsImportingXML() && ( aValue >>= nNewHeight ) )
        {
            // used to set the stored row height for rows with optimal height when loading.

            // TODO: It's probably cleaner to use a different property name
            // for this.
            rDoc.SetRowHeightOnly( nStartRow, nEndRow, nTab, (sal_uInt16)HMMToTwips(nNewHeight) );
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
    else if ( aNameString == SC_UNONAME_CELLHGT )
    {
        sal_Int32 nNewHeight = 0;
        if ( aValue >>= nNewHeight )
        {
            if (rDoc.IsImportingXML())
            {
                // TODO: This is a band-aid fix.  Eventually we need to
                // re-work ods' style import to get it to set styles to
                // ScDocument directly.
                rDoc.SetRowHeightOnly( nStartRow, nEndRow, nTab, (sal_uInt16)HMMToTwips(nNewHeight) );
                rDoc.SetManualHeight( nStartRow, nEndRow, nTab, true );
            }
            else
                rFunc.SetWidthOrHeight(
                    false, aRowArr, nTab, SC_SIZE_ORIGINAL, (sal_uInt16)HMMToTwips(nNewHeight), true, true);
        }
    }
    else if ( aNameString == SC_UNONAME_CELLVIS )
    {
        bool bVis = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        ScSizeMode eMode = bVis ? SC_SIZE_SHOW : SC_SIZE_DIRECT;
        rFunc.SetWidthOrHeight(false, aRowArr, nTab, eMode, 0, true, true);
        //  SC_SIZE_DIRECT with size 0: hide
    }
    else if ( aNameString == SC_UNONAME_VISFLAG )
    {
        // #i116460# Shortcut to only set the flag, without drawing layer update etc.
        // Should only be used from import filters.
        rDoc.SetRowHidden(nStartRow, nEndRow, nTab, !ScUnoHelpFunctions::GetBoolFromAny( aValue ));
    }
    else if ( aNameString == SC_UNONAME_CELLFILT )
    {
        //! undo etc.
        if (ScUnoHelpFunctions::GetBoolFromAny( aValue ))
            rDoc.SetRowFiltered(nStartRow, nEndRow, nTab, true);
        else
            rDoc.SetRowFiltered(nStartRow, nEndRow, nTab, false);
    }
    else if ( aNameString == SC_UNONAME_NEWPAGE || aNameString == SC_UNONAME_MANPAGE )
    {
        //! single function to set/remove all breaks?
        bool bSet = ScUnoHelpFunctions::GetBoolFromAny( aValue );
        for (SCROW nRow=nStartRow; nRow<=nEndRow; nRow++)
            if (bSet)
                rFunc.InsertPageBreak( false, ScAddress(0,nRow,nTab), true, true, true );
            else
                rFunc.RemovePageBreak( false, ScAddress(0,nRow,nTab), true, true, true );
    }
    else if ( aNameString == SC_UNONAME_CELLBACK || aNameString == SC_UNONAME_CELLTRAN )
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
    throw(beans::UnknownPropertyException, lang::WrappedTargetException,
          uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (!pDocShell)
        throw uno::RuntimeException();

    ScDocument& rDoc = pDocShell->GetDocument();
    OUString aNameString(aPropertyName);
    uno::Any aAny;

    //! loop over all rows for current state?

    if ( aNameString == SC_UNONAME_CELLHGT )
    {
        // for hidden row, return original height
        sal_uInt16 nHeight = rDoc.GetOriginalHeight( nStartRow, nTab );
        aAny <<= (sal_Int32)TwipsToHMM(nHeight);
    }
    else if ( aNameString == SC_UNONAME_CELLVIS )
    {
        SCROW nLastRow;
        bool bVis = !rDoc.RowHidden(nStartRow, nTab, nullptr, &nLastRow);
        ScUnoHelpFunctions::SetBoolInAny( aAny, bVis );
    }
    else if ( aNameString == SC_UNONAME_CELLFILT )
    {
        bool bVis = rDoc.RowFiltered(nStartRow, nTab);
        ScUnoHelpFunctions::SetBoolInAny( aAny, bVis );
    }
    else if ( aNameString == SC_UNONAME_OHEIGHT )
    {
        bool bOpt = !(rDoc.GetRowFlags( nStartRow, nTab ) & CR_MANUALSIZE);
        ScUnoHelpFunctions::SetBoolInAny( aAny, bOpt );
    }
    else if ( aNameString == SC_UNONAME_NEWPAGE )
    {
        ScBreakType nBreak = rDoc.HasRowBreak(nStartRow, nTab);
        ScUnoHelpFunctions::SetBoolInAny( aAny, nBreak != BREAK_NONE );
    }
    else if ( aNameString == SC_UNONAME_MANPAGE )
    {
        ScBreakType nBreak = rDoc.HasRowBreak(nStartRow, nTab);
        ScUnoHelpFunctions::SetBoolInAny( aAny, (nBreak & BREAK_MANUAL) != 0 );
    }
    else if ( aNameString == SC_UNONAME_CELLBACK || aNameString == SC_UNONAME_CELLTRAN )
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
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScSpreadsheetSettingsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //  Referenz-Update interessiert hier nicht

    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
    {
        pDocShell = nullptr;       // ungueltig geworden
    }
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScSpreadsheetSettingsObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException, std::exception)
{
    //! muss noch
    return nullptr;
}

void SAL_CALL ScSpreadsheetSettingsObj::setPropertyValue(
                        const OUString& /* aPropertyName */, const uno::Any& /* aValue */ )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    //! muss noch
}

uno::Any SAL_CALL ScSpreadsheetSettingsObj::getPropertyValue( const OUString& /* aPropertyName */ )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    //! muss noch
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
    //! nTab bei Referenz-Update anpassen!!!

    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
    {
        pDocShell = nullptr;       // ungueltig geworden
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
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
    {
        OSL_ENSURE( aPosition.Sheet == nTab, "addAnnotation mit falschem Sheet" );
        ScAddress aPos( (SCCOL)aPosition.Column, (SCROW)aPosition.Row, nTab );
        pDocShell->GetDocFunc().ReplaceNote( aPos, rText, nullptr, nullptr, true );
    }
}

void SAL_CALL ScAnnotationsObj::removeByIndex( sal_Int32 nIndex ) throw(uno::RuntimeException, std::exception)
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
                                                    throw(uno::RuntimeException, std::exception)
{
    //! iterate directly (more efficiently)?

    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.sheet.CellAnnotationsEnumeration"));
}

// XIndexAccess

sal_Int32 SAL_CALL ScAnnotationsObj::getCount()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    sal_Int32 nCount = 0;
    if (pDocShell)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        const ScRangeList aRangeList( ScRange( 0, 0, nTab, MAXCOL, MAXROW, nTab) );
        std::vector<sc::NoteEntry> rNotes;
        rDoc.GetNotesInRange(aRangeList, rNotes);
        nCount = rNotes.size();
    }
    return nCount;
}

uno::Any SAL_CALL ScAnnotationsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XSheetAnnotation> xAnnotation(GetObjectByIndex_Impl(nIndex));
    if (xAnnotation.is())
        return uno::makeAny(xAnnotation);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScAnnotationsObj::getElementType() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<sheet::XSheetAnnotation>::get();
}

sal_Bool SAL_CALL ScAnnotationsObj::hasElements() throw(uno::RuntimeException, std::exception)
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
        //! Referenz-Update fuer Tab und Start/Ende
    }
    else if ( dynamic_cast<const SfxSimpleHint*>(&rHint) &&
            static_cast<const SfxSimpleHint&>(rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = nullptr;       // ungueltig geworden
    }
}

// XScenarios

bool ScScenariosObj::GetScenarioIndex_Impl( const OUString& rName, SCTAB& rIndex )
{
    //! Case-insensitiv ????

    if ( pDocShell )
    {
        OUString aTabName;
        ScDocument& rDoc = pDocShell->GetDocument();
        SCTAB nCount = (SCTAB)getCount();
        for (SCTAB i=0; i<nCount; i++)
            if (rDoc.GetName( nTab+i+1, aTabName ))
                if (aTabName.equals(rName))
                {
                    rIndex = i;
                    return true;
                }
    }

    return false;
}

ScTableSheetObj* ScScenariosObj::GetObjectByIndex_Impl(sal_Int32 nIndex)
{
    sal_uInt16 nCount = (sal_uInt16)getCount();
    if ( pDocShell && nIndex >= 0 && nIndex < nCount )
        return new ScTableSheetObj( pDocShell, nTab+static_cast<SCTAB>(nIndex)+1 );

    return nullptr;    // kein Dokument oder falscher Index
}

ScTableSheetObj* ScScenariosObj::GetObjectByName_Impl(const OUString& aName)
{
    SCTAB nIndex;
    if ( pDocShell && GetScenarioIndex_Impl( aName, nIndex ) )
        return new ScTableSheetObj( pDocShell, nTab+nIndex+1 );

    return nullptr;    // nicht gefunden
}

void SAL_CALL ScScenariosObj::addNewByName( const OUString& aName,
                                const uno::Sequence<table::CellRangeAddress>& aRanges,
                                const OUString& aComment )
                                    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if ( pDocShell )
    {
        ScMarkData aMarkData;
        aMarkData.SelectTable( nTab, true );

        sal_uInt16 nRangeCount = (sal_uInt16)aRanges.getLength();
        if (nRangeCount)
        {
            const table::CellRangeAddress* pAry = aRanges.getConstArray();
            for (sal_uInt16 i=0; i<nRangeCount; i++)
            {
                OSL_ENSURE( pAry[i].Sheet == nTab, "addScenario mit falscher Tab" );
                ScRange aRange( (SCCOL)pAry[i].StartColumn, (SCROW)pAry[i].StartRow, nTab,
                                (SCCOL)pAry[i].EndColumn,   (SCROW)pAry[i].EndRow,   nTab );

                aMarkData.SetMultiMarkArea( aRange );
            }
        }

        OUString aNameStr(aName);
        OUString aCommStr(aComment);

        Color aColor( COL_LIGHTGRAY );  // Default
        sal_uInt16 nFlags = SC_SCENARIO_SHOWFRAME | SC_SCENARIO_PRINTFRAME | SC_SCENARIO_TWOWAY | SC_SCENARIO_PROTECT;

        pDocShell->MakeScenario( nTab, aNameStr, aCommStr, aColor, nFlags, aMarkData );
    }
}

void SAL_CALL ScScenariosObj::removeByName( const OUString& aName )
                                            throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SCTAB nIndex;
    if ( pDocShell && GetScenarioIndex_Impl( aName, nIndex ) )
        pDocShell->GetDocFunc().DeleteTable( nTab+nIndex+1, true, true );
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScScenariosObj::createEnumeration()
                                                    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, OUString("com.sun.star.sheet.ScenariosEnumeration"));
}

// XIndexAccess

sal_Int32 SAL_CALL ScScenariosObj::getCount() throw(uno::RuntimeException, std::exception)
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
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XScenario> xScen(GetObjectByIndex_Impl(nIndex));
    if (xScen.is())
        return uno::makeAny(xScen);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScScenariosObj::getElementType() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<sheet::XScenario>::get();
}

sal_Bool SAL_CALL ScScenariosObj::hasElements() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

uno::Any SAL_CALL ScScenariosObj::getByName( const OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<sheet::XScenario> xScen(GetObjectByName_Impl(aName));
    if (xScen.is())
        return uno::makeAny(xScen);
    else
        throw container::NoSuchElementException();
}

uno::Sequence<OUString> SAL_CALL ScScenariosObj::getElementNames()
                                                throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SCTAB nCount = (SCTAB)getCount();
    uno::Sequence<OUString> aSeq(nCount);

    if ( pDocShell )    // sonst ist auch Count = 0
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
                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SCTAB nIndex;
    return GetScenarioIndex_Impl( aName, nIndex );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
