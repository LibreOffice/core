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

#include <printdata.hxx>

#include <globals.hrc>
#include <doc.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <unotxdoc.hxx>
#include <wdocsh.hxx>
#include <viewsh.hxx>
#include <docfld.hxx>

#include <svl/languageoptions.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <tools/resary.hxx>
#include <unotools/moduleoptions.hxx>
#include <vcl/outdev.hxx>

using namespace ::com::sun::star;

SwRenderData::SwRenderData()
{
}

SwRenderData::~SwRenderData()
{
    OSL_ENSURE( !m_pPostItShell, "m_pPostItShell should already have been deleted" );
    OSL_ENSURE( !m_pPostItFields, " should already have been deleted" );
}

void SwRenderData::CreatePostItData( SwDoc *pDoc, const SwViewOption *pViewOpt, OutputDevice *pOutDev )
{
    DeletePostItData();
    m_pPostItFields.reset(new _SetGetExpFields);
    sw_GetPostIts( &pDoc->getIDocumentFieldsAccess(), m_pPostItFields.get() );

    //!! Disable spell and grammar checking in the temporary document.
    //!! Otherwise the grammar checker might process it and crash if we later on
    //!! simply delete this document while he is still at it.
    SwViewOption  aViewOpt( *pViewOpt );
    aViewOpt.SetOnlineSpell( false );

    m_pPostItShell.reset(new SwViewShell(*new SwDoc, nullptr, &aViewOpt, pOutDev));
}

void SwRenderData::DeletePostItData()
{
    if (HasPostItData())
    {
        // printer needs to remain at the real document
        m_pPostItShell->GetDoc()->getIDocumentDeviceAccess().setPrinter( nullptr, false, false );
        m_pPostItShell.reset();
        m_pPostItFields.reset();
    }
}


void SwRenderData::SetTempDocShell(SfxObjectShellLock const& xShell)
{
    m_xTempDocShell = xShell;
}

bool SwRenderData::NeedNewViewOptionAdjust( const SwViewShell& rCompare ) const
{
    return !(m_pViewOptionAdjust && m_pViewOptionAdjust->checkShell( rCompare ));
}

void SwRenderData::ViewOptionAdjustStart(
        SwViewShell &rSh, const SwViewOption &rViewOptions)
{
    if (m_pViewOptionAdjust)
    {
        OSL_FAIL("error: there should be no ViewOptionAdjust active when calling this function" );
    }
    m_pViewOptionAdjust.reset(
            new SwViewOptionAdjust_Impl( rSh, rViewOptions ));
}

void SwRenderData::ViewOptionAdjust(SwPrintData const*const pPrtOptions)
{
    m_pViewOptionAdjust->AdjustViewOptions( pPrtOptions );
}

void SwRenderData::ViewOptionAdjustStop()
{
    m_pViewOptionAdjust.reset();
}

void SwRenderData::ViewOptionAdjustCrashPreventionKludge()
{
    m_pViewOptionAdjust->DontTouchThatViewShellItSmellsFunny();
}

void SwRenderData::MakeSwPrtOptions(
    SwDocShell const*const pDocShell,
    SwPrintUIOptions const*const pOpt,
    bool const bIsPDFExport)
{
    if (!pDocShell || !pOpt)
        return;

    m_pPrtOptions.reset(new SwPrintData);
    SwPrintData & rOptions(*m_pPrtOptions);

    // get default print options
    bool bWeb = dynamic_cast<const SwWebDocShell*>( pDocShell) !=  nullptr;
    ::sw::InitPrintOptionsFromApplication(rOptions, bWeb);

    // get print options to use from provided properties
    rOptions.m_bPrintGraphic          = pOpt->IsPrintGraphics();
    rOptions.m_bPrintTable            = true; // for now it was decided that tables should always be printed
    rOptions.m_bPrintDraw             = pOpt->IsPrintDrawings();
    rOptions.m_bPrintControl          = pOpt->IsPrintFormControls();
    rOptions.m_bPrintLeftPages        = pOpt->IsPrintLeftPages();
    rOptions.m_bPrintRightPages       = pOpt->IsPrintRightPages();
    rOptions.m_bPrintPageBackground   = pOpt->IsPrintPageBackground();
    rOptions.m_bPrintEmptyPages       = pOpt->IsPrintEmptyPages( bIsPDFExport );
    // bUpdateFieldsInPrinting  <-- not set here; mail merge only
    rOptions.m_bPaperFromSetup        = pOpt->IsPaperFromSetup();
    rOptions.m_bPrintReverse          = false; /*handled by print dialog now*/
    rOptions.m_bPrintProspect         = pOpt->IsPrintProspect();
    rOptions.m_bPrintProspectRTL      = pOpt->IsPrintProspectRTL();
    // bPrintSingleJobs         <-- not set here; mail merge and or configuration
    // bModified                <-- not set here; mail merge only
    rOptions.m_bPrintBlackFont        = pOpt->IsPrintWithBlackTextColor();
    rOptions.m_bPrintHiddenText       = pOpt->IsPrintHiddenText();
    rOptions.m_bPrintTextPlaceholder  = pOpt->IsPrintTextPlaceholders();
    rOptions.m_nPrintPostIts          = pOpt->GetPrintPostItsType();

    //! needs to be set after MakeOptions since the assignment operation in that
    //! function will destroy the pointers
    rOptions.SetPrintUIOptions( pOpt );
    rOptions.SetRenderData( this );
}

SwPrintUIOptions::SwPrintUIOptions(
    sal_uInt16 nCurrentPage,
    bool bWeb,
    bool bSwSrcView,
    bool bHasSelection,
    bool bHasPostIts,
    const SwPrintData &rDefaultPrintData ) :
    m_rDefaultPrintData( rDefaultPrintData )
{
    // printing HTML sources does not have any valid UI options.
    // Its just the source code that gets printed ...
    if (bSwSrcView)
    {
        m_aUIProperties.realloc( 0 );
        return;
    }

    // check if CTL is enabled
    SvtLanguageOptions aLangOpt;
    bool bCTL = aLangOpt.IsCTLFontEnabled();

    // create sequence of print UI options
    // (5 options are not available for Writer-Web)
    const int nCTLOpts = bCTL ? 1 : 0;
    const int nNumProps = nCTLOpts + (bWeb ? 15 : 21);
    m_aUIProperties.realloc( nNumProps );
    int nIdx = 0;

    // load the writer PrinterOptions into the custom tab
    m_aUIProperties[nIdx].Name = "OptionsUIFile";
    m_aUIProperties[nIdx++].Value <<= OUString("modules/swriter/ui/printeroptions.ui");

    // create "writer" section (new tab page in dialog)
    SvtModuleOptions aModOpt;
    OUString aAppGroupname( SW_RES( STR_PRINTOPTUI_PRODUCTNAME) );
    aAppGroupname = aAppGroupname.replaceFirst( "%s", aModOpt.GetModuleName( SvtModuleOptions::EModule::WRITER ) );
    m_aUIProperties[ nIdx++ ].Value = setGroupControlOpt("tabcontrol-page2", aAppGroupname, ".HelpID:vcl:PrintDialog:TabPage:AppPage");

    // create sub section for Contents
    m_aUIProperties[ nIdx++ ].Value = setSubgroupControlOpt("contents", SW_RES( STR_PRINTOPTUI_CONTENTS), OUString());

    // create a bool option for background
    bool bDefaultVal = rDefaultPrintData.IsPrintPageBackground();
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("pagebackground", SW_RES( STR_PRINTOPTUI_PAGE_BACKGROUND),
                                                        ".HelpID:vcl:PrintDialog:PrintPageBackground:CheckBox",
                                                        "PrintPageBackground",
                                                        bDefaultVal);

    // create a bool option for pictures/graphics AND OLE and drawing objects as well
    bDefaultVal = rDefaultPrintData.IsPrintGraphic() || rDefaultPrintData.IsPrintDraw();
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("pictures", SW_RES( STR_PRINTOPTUI_PICTURES),
                                                        ".HelpID:vcl:PrintDialog:PrintPicturesAndObjects:CheckBox",
                                                        "PrintPicturesAndObjects",
                                                         bDefaultVal);
    if (!bWeb)
    {
        // create a bool option for hidden text
        bDefaultVal = rDefaultPrintData.IsPrintHiddenText();
        m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("hiddentext", SW_RES( STR_PRINTOPTUI_HIDDEN),
                                                            ".HelpID:vcl:PrintDialog:PrintHiddenText:CheckBox",
                                                            "PrintHiddenText",
                                                            bDefaultVal);

        // create a bool option for place holder
        bDefaultVal = rDefaultPrintData.IsPrintTextPlaceholder();
        m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("placeholders", SW_RES( STR_PRINTOPTUI_TEXT_PLACEHOLDERS),
                                                            ".HelpID:vcl:PrintDialog:PrintTextPlaceholder:CheckBox",
                                                            "PrintTextPlaceholder",
                                                            bDefaultVal);
    }

    // create a bool option for controls
    bDefaultVal = rDefaultPrintData.IsPrintControl();
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("formcontrols", SW_RES( STR_PRINTOPTUI_FORM_CONTROLS),
                                                        ".HelpID:vcl:PrintDialog:PrintControls:CheckBox",
                                                        "PrintControls",
                                                        bDefaultVal);

    // create sub section for Color
    m_aUIProperties[ nIdx++ ].Value = setSubgroupControlOpt("color", SW_RES( STR_PRINTOPTUI_COLOR), OUString());

    // create a bool option for printing text with black font color
    bDefaultVal = rDefaultPrintData.IsPrintBlackFont();
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("textinblack", SW_RES( STR_PRINTOPTUI_PRINT_BLACK),
                                                        ".HelpID:vcl:PrintDialog:PrintBlackFonts:CheckBox",
                                                        "PrintBlackFonts",
                                                        bDefaultVal);

    if (!bWeb)
    {
        // create subgroup for misc options
        m_aUIProperties[ nIdx++ ].Value = setSubgroupControlOpt("pages", SW_RES( STR_PRINTOPTUI_PAGES_TEXT), OUString());

        // create a bool option for printing automatically inserted blank pages
        bDefaultVal = rDefaultPrintData.IsPrintEmptyPages();
        m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("autoblankpages", SW_RES( STR_PRINTOPTUI_PRINT_BLANK),
                                                            ".HelpID:vcl:PrintDialog:PrintEmptyPages:CheckBox",
                                                            "PrintEmptyPages",
                                                            bDefaultVal);
    }

    // create a bool option for paper tray
    bDefaultVal = rDefaultPrintData.IsPaperFromSetup();
    vcl::PrinterOptionsHelper::UIControlOptions aPaperTrayOpt;
    aPaperTrayOpt.maGroupHint = "OptionsPageOptGroup";
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("printpaperfromsetup", SW_RES( STR_PRINTOPTUI_ONLY_PAPER),
                                                        ".HelpID:vcl:PrintDialog:PrintPaperFromSetup:CheckBox",
                                                        "PrintPaperFromSetup",
                                                        bDefaultVal,
                                                        aPaperTrayOpt);

    // print range selection
    vcl::PrinterOptionsHelper::UIControlOptions aPrintRangeOpt;
    aPrintRangeOpt.maGroupHint = "PrintRange";
    aPrintRangeOpt.mbInternalOnly = true;
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("printrange", SW_RES( STR_PRINTOPTUI_RANGE_COPIES),
                                                           OUString(),
                                                           aPrintRangeOpt);

    // create a choice for the content to create
    const OUString aPrintRangeName( "PrintContent" );
    uno::Sequence< OUString > aChoices( 3 );
    uno::Sequence< sal_Bool > aChoicesDisabled( 3 );
    uno::Sequence< OUString > aHelpIds( 3 );
    uno::Sequence< OUString > aWidgetIds( 3 );
    aChoices[0] = SW_RES( STR_PRINTOPTUI_ALLPAGES);
    aChoicesDisabled[0] = sal_False;
    aHelpIds[0] = ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:0";
    aWidgetIds[0] = "printallpages";
    aChoices[1] = SW_RES( STR_PRINTOPTUI_SOMEPAGES);
    aChoicesDisabled[1] = sal_False;
    aHelpIds[1] = ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:1";
    aWidgetIds[1] = "printpages";
    aChoices[2] = SW_RES( STR_PRINTOPTUI_SELECTION);
    aChoicesDisabled[2] = ! bHasSelection;
    aHelpIds[2] = ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:2";
    aWidgetIds[2] = "printselection";
    m_aUIProperties[nIdx++].Value = setChoiceRadiosControlOpt(aWidgetIds, OUString(),
                                                        aHelpIds, aPrintRangeName,
                                                        aChoices, 0 /* always default to 'All pages' */,
                                                        aChoicesDisabled);
    // show an Edit dependent on "Pages" selected
    vcl::PrinterOptionsHelper::UIControlOptions aPageRangeOpt( aPrintRangeName, 1, true );
    m_aUIProperties[nIdx++].Value = setEditControlOpt("pagerange", OUString(),
                                                      ".HelpID:vcl:PrintDialog:PageRange:Edit",
                                                      "PageRange",
                                                      OUString::number( nCurrentPage ) /* set text box to current page number */,
                                                      aPageRangeOpt);
    // print content selection
    vcl::PrinterOptionsHelper::UIControlOptions aContentsOpt;
    aContentsOpt.maGroupHint = "JobPage";
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("extrawriterprintoptions",
                                                          SW_RES( STR_PRINTOPTUI_PRINT),
                                                          OUString(), aContentsOpt);
    // create a list box for notes content
    const SwPostItMode nPrintPostIts = rDefaultPrintData.GetPrintPostIts();
    aChoices.realloc( 5 );
    aChoices[0] = SW_RES( STR_PRINTOPTUI_NONE);
    aChoices[1] = SW_RES( STR_PRINTOPTUI_COMMENTS_ONLY);
    aChoices[2] = SW_RES( STR_PRINTOPTUI_PLACE_END);
    aChoices[3] = SW_RES( STR_PRINTOPTUI_PLACE_PAGE);
    aChoices[4] = SW_RES( STR_PRINTOPTUI_PLACE_MARGINS);
    aHelpIds.realloc( 2 );
    aHelpIds[0] = ".HelpID:vcl:PrintDialog:PrintAnnotationMode:FixedText";
    aHelpIds[1] = ".HelpID:vcl:PrintDialog:PrintAnnotationMode:ListBox";
    vcl::PrinterOptionsHelper::UIControlOptions aAnnotOpt( "PrintProspect", 0, false );
    aAnnotOpt.mbEnabled = bHasPostIts;
    m_aUIProperties[ nIdx++ ].Value = setChoiceListControlOpt("writercomments",
                                                           SW_RES( STR_PRINTOPTUI_COMMENTS),
                                                           aHelpIds,
                                                           "PrintAnnotationMode",
                                                           aChoices,
                                                           static_cast<sal_uInt16>(nPrintPostIts),
                                                           uno::Sequence< sal_Bool >(),
                                                           aAnnotOpt);

    // create subsection for Page settings
    vcl::PrinterOptionsHelper::UIControlOptions aPageSetOpt;
    aPageSetOpt.maGroupHint = "LayoutPage";

    if (!bWeb)
    {
        m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("pagesides",
                                                              SW_RES( STR_PRINTOPTUI_PAGE_SIDES),
                                                               OUString(), aPageSetOpt);
        uno::Sequence< OUString > aRLChoices( 3 );
        aRLChoices[0] = SW_RES( STR_PRINTOPTUI_ALL_PAGES);
        aRLChoices[1] = SW_RES( STR_PRINTOPTUI_BACK_PAGES);
        aRLChoices[2] = SW_RES( STR_PRINTOPTUI_FONT_PAGES);
        uno::Sequence<OUString> aRLHelp { ".HelpID:vcl:PrintDialog:PrintLeftRightPages:ListBox" };
        // create a choice option for all/left/right pages
        // 0 : all pages (left & right)
        // 1 : left pages
        // 2 : right pages
        OSL_ENSURE( rDefaultPrintData.IsPrintLeftPage() || rDefaultPrintData.IsPrintRightPage(),
                "unexpected value combination" );
        sal_Int16 nPagesChoice = 0;
        if (rDefaultPrintData.IsPrintLeftPage() && !rDefaultPrintData.IsPrintRightPage())
            nPagesChoice = 1;
        else if (!rDefaultPrintData.IsPrintLeftPage() && rDefaultPrintData.IsPrintRightPage())
            nPagesChoice = 2;
        m_aUIProperties[ nIdx++ ].Value = setChoiceListControlOpt("brochureinclude",
                                                   SW_RES( STR_PRINTOPTUI_INCLUDE),
                                                   aRLHelp,
                                                   "PrintLeftRightPages",
                                                   aRLChoices,
                                                   nPagesChoice);
    }

    // create a bool option for brochure
    bDefaultVal = rDefaultPrintData.IsPrintProspect();
    const OUString aBrochurePropertyName( "PrintProspect" );
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("brochure", SW_RES( STR_PRINTOPTUI_BROCHURE),
                                                        ".HelpID:vcl:PrintDialog:PrintProspect:CheckBox",
                                                        aBrochurePropertyName,
                                                        bDefaultVal,
                                                        aPageSetOpt);

    if (bCTL)
    {
        // create a bool option for brochure RTL dependent on brochure
        uno::Sequence< OUString > aBRTLChoices( 2 );
        aBRTLChoices[0] = SW_RES( STR_PRINTOPTUI_LEFT_SCRIPT);
        aBRTLChoices[1] = SW_RES( STR_PRINTOPTUI_RIGHT_SCRIPT);
        vcl::PrinterOptionsHelper::UIControlOptions aBrochureRTLOpt( aBrochurePropertyName, -1, true );
        uno::Sequence<OUString> aBRTLHelpIds { ".HelpID:vcl:PrintDialog:PrintProspectRTL:ListBox" };
        aBrochureRTLOpt.maGroupHint = "LayoutPage";
        // RTL brochure choices
        //      0 : left-to-right
        //      1 : right-to-left
        const sal_Int16 nBRTLChoice = rDefaultPrintData.IsPrintProspectRTL() ? 1 : 0;
        m_aUIProperties[ nIdx++ ].Value = setChoiceListControlOpt("scriptdirection",
                                                               OUString(),
                                                               aBRTLHelpIds,
                                                               "PrintProspectRTL",
                                                               aBRTLChoices,
                                                               nBRTLChoice,
                                                               uno::Sequence< sal_Bool >(),
                                                               aBrochureRTLOpt);
    }

    assert(nIdx == nNumProps);
}

SwPrintUIOptions::~SwPrintUIOptions()
{
}

bool SwPrintUIOptions::IsPrintLeftPages() const
{
    // take care of different property names for the option.
    // for compatibility the old name should win (may still be used for PDF export or via Uno API)

    // 0: left and right pages
    // 1: left pages only
    // 2: right pages only
    sal_Int64 nLRPages = getIntValue( "PrintLeftRightPages", 0 /* default: all */ );
    bool bRes = nLRPages == 0 || nLRPages == 1;
    bRes = getBoolValue( "PrintLeftPages", bRes /* <- default value if property is not found */ );
    return bRes;
}

bool SwPrintUIOptions::IsPrintRightPages() const
{
    // take care of different property names for the option.
    // for compatibility the old name should win (may still be used for PDF export or via Uno API)

    sal_Int64 nLRPages = getIntValue( "PrintLeftRightPages", 0 /* default: all */ );
    bool bRes = nLRPages == 0 || nLRPages == 2;
    bRes = getBoolValue( "PrintRightPages", bRes /* <- default value if property is not found */ );
    return bRes;
}

bool SwPrintUIOptions::IsPrintEmptyPages( bool bIsPDFExport ) const
{
    // take care of different property names for the option.

    bool bRes = bIsPDFExport ?
            !getBoolValue( "IsSkipEmptyPages", true ) :
            getBoolValue( "PrintEmptyPages", true );
    return bRes;
}

bool SwPrintUIOptions::IsPrintGraphics() const
{
    // take care of different property names for the option.
    // for compatibility the old name should win (may still be used for PDF export or via Uno API)

    bool bRes = getBoolValue( "PrintPicturesAndObjects", true );
    bRes = getBoolValue( "PrintGraphics", bRes );
    return bRes;
}

bool SwPrintUIOptions::IsPrintDrawings() const
{
    // take care of different property names for the option.
    // for compatibility the old name should win (may still be used for PDF export or via Uno API)

    bool bRes = getBoolValue( "PrintPicturesAndObjects", true );
    bRes = getBoolValue( "PrintDrawings", bRes );
    return bRes;
}

bool SwPrintUIOptions::processPropertiesAndCheckFormat( const uno::Sequence< beans::PropertyValue >& i_rNewProp )
{
    bool bChanged = processProperties( i_rNewProp );

    uno::Reference< awt::XDevice >  xRenderDevice;
    uno::Any aVal( getValue( "RenderDevice" ) );
    aVal >>= xRenderDevice;

    VclPtr< OutputDevice > pOut;
    if (xRenderDevice.is())
    {
        VCLXDevice*     pDevice = VCLXDevice::GetImplementation( xRenderDevice );
        if (pDevice)
            pOut = pDevice->GetOutputDevice();
    }
    bChanged = bChanged || (pOut.get() != m_pLast.get());
    if( pOut )
        m_pLast = pOut;

    return bChanged;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
