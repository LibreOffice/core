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
#include <unotxdoc.hxx>
#include <wdocsh.hxx>
#include <viewsh.hxx>
#include <docfld.hxx>

#include <svl/languageoptions.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <tools/string.hxx>
#include <tools/resary.hxx>
#include <unotools/moduleoptions.hxx>
#include <vcl/outdev.hxx>

extern bool sw_GetPostIts( IDocumentFieldsAccess* pIDFA, _SetGetExpFlds * pSrtLst );

using namespace ::com::sun::star;

// SwRenderData

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
    m_pPostItFields.reset(new _SetGetExpFlds);
    sw_GetPostIts( pDoc, m_pPostItFields.get() );

    //!! Disable spell and grammar checking in the temporary document.
    //!! Otherwise the grammar checker might process it and crash if we later on
    //!! simply delete this document while he is still at it.
    SwViewOption  aViewOpt( *pViewOpt );
    aViewOpt.SetOnlineSpell( sal_False );

    m_pPostItShell.reset(new ViewShell(*new SwDoc, 0, &aViewOpt, pOutDev));
}

void SwRenderData::DeletePostItData()
{
    if (HasPostItData())
    {
        // printer needs to remain at the real document
        m_pPostItShell->GetDoc()->setPrinter( 0, false, false );
        m_pPostItShell.reset();
        m_pPostItFields.reset();
    }
}

SfxObjectShellLock const& SwRenderData::GetTempDocShell() const
{
    return m_xTempDocShell;
}

void SwRenderData::SetTempDocShell(SfxObjectShellLock const& xShell)
{
    m_xTempDocShell = xShell;
}

bool SwRenderData::NeedNewViewOptionAdjust( const ViewShell& rCompare ) const
{
    return m_pViewOptionAdjust ? ! m_pViewOptionAdjust->checkShell( rCompare ) : true;
}

void SwRenderData::ViewOptionAdjustStart(
        ViewShell &rSh, const SwViewOption &rViewOptions)
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
    const TypeId aSwWebDocShellTypeId = TYPE(SwWebDocShell);
    sal_Bool bWeb = pDocShell->IsA( aSwWebDocShellTypeId );
    ::sw::InitPrintOptionsFromApplication(rOptions, bWeb);

    // get print options to use from provided properties
    rOptions.bPrintGraphic          = pOpt->IsPrintGraphics();
    rOptions.bPrintTable            = pOpt->IsPrintTables();
    rOptions.bPrintDraw             = pOpt->IsPrintDrawings();
    rOptions.bPrintControl          = pOpt->IsPrintFormControls();
    rOptions.bPrintLeftPages        = pOpt->IsPrintLeftPages();
    rOptions.bPrintRightPages       = pOpt->IsPrintRightPages();
    rOptions.bPrintPageBackground   = pOpt->IsPrintPageBackground();
    rOptions.bPrintEmptyPages       = pOpt->IsPrintEmptyPages( bIsPDFExport );
    // bUpdateFieldsInPrinting  <-- not set here; mail merge only
    rOptions.bPaperFromSetup        = pOpt->IsPaperFromSetup();
    rOptions.bPrintReverse          = pOpt->IsPrintReverse();
    rOptions.bPrintProspect         = pOpt->IsPrintProspect();
    rOptions.bPrintProspectRTL      = pOpt->IsPrintProspectRTL();
    // bPrintSingleJobs         <-- not set here; mail merge and or configuration
    // bModified                <-- not set here; mail merge only
    rOptions.bPrintBlackFont        = pOpt->IsPrintWithBlackTextColor();
    rOptions.bPrintHiddenText       = pOpt->IsPrintHiddenText();
    rOptions.bPrintTextPlaceholder  = pOpt->IsPrintTextPlaceholders();
    rOptions.nPrintPostIts          = pOpt->GetPrintPostItsType();

    //! needs to be set after MakeOptions since the assignment operation in that
    //! function will destroy the pointers
    rOptions.SetPrintUIOptions( pOpt );
    rOptions.SetRenderData( this );
}

// SwPrintUIOptions

SwPrintUIOptions::SwPrintUIOptions(
    sal_uInt16 nCurrentPage,
    bool bWeb,
    bool bSwSrcView,
    bool bHasSelection,
    bool bHasPostIts,
    const SwPrintData &rDefaultPrintData ) :
    m_pLast( NULL ),
    m_rDefaultPrintData( rDefaultPrintData )
{
    ResStringArray aLocalizedStrings( SW_RES( STR_PRINTOPTUI ) );

    OSL_ENSURE( aLocalizedStrings.Count() >= 30, "resource incomplete" );
    if( aLocalizedStrings.Count() < 30 ) // bad resource ?
        return;

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
    m_aUIProperties[nIdx].Name = OUString("OptionsUIFile");
    m_aUIProperties[nIdx++].Value <<= OUString("modules/swriter/ui/printeroptions.ui");

    // create "writer" section (new tab page in dialog)
    SvtModuleOptions aModOpt;
    String aAppGroupname( aLocalizedStrings.GetString( 0 ) );
    aAppGroupname.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s" ) ),
                                    aModOpt.GetModuleName( SvtModuleOptions::E_SWRITER ) );
    m_aUIProperties[ nIdx++ ].Value = setGroupControlOpt("tabcontrol-page2", aAppGroupname, ".HelpID:vcl:PrintDialog:TabPage:AppPage");

    // create sub section for Contents
    m_aUIProperties[ nIdx++ ].Value = setSubgroupControlOpt("contents", aLocalizedStrings.GetString(1), OUString());

    // create a bool option for background
    bool bDefaultVal = rDefaultPrintData.IsPrintPageBackground();
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("pagebackground", aLocalizedStrings.GetString( 2 ),
                                                        ".HelpID:vcl:PrintDialog:PrintPageBackground:CheckBox",
                                                        "PrintPageBackground",
                                                        bDefaultVal);

    // create a bool option for pictures/graphics AND OLE and drawing objects as well
    bDefaultVal = rDefaultPrintData.IsPrintGraphic() || rDefaultPrintData.IsPrintDraw();
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("pictures", aLocalizedStrings.GetString( 3 ),
                                                        ".HelpID:vcl:PrintDialog:PrintPicturesAndObjects:CheckBox",
                                                        "PrintPicturesAndObjects",
                                                         bDefaultVal);
    if (!bWeb)
    {
        // create a bool option for hidden text
        bDefaultVal = rDefaultPrintData.IsPrintHiddenText();
        m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("hiddentext", aLocalizedStrings.GetString( 4 ),
                                                            ".HelpID:vcl:PrintDialog:PrintHiddenText:CheckBox",
                                                            "PrintHiddenText",
                                                            bDefaultVal);

        // create a bool option for place holder
        bDefaultVal = rDefaultPrintData.IsPrintTextPlaceholder();
        m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("placeholders", aLocalizedStrings.GetString( 5 ),
                                                            ".HelpID:vcl:PrintDialog:PrintTextPlaceholder:CheckBox",
                                                            "PrintTextPlaceholder",
                                                            bDefaultVal);
    }

    // create a bool option for controls
    bDefaultVal = rDefaultPrintData.IsPrintControl();
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("formcontrols", aLocalizedStrings.GetString( 6 ),
                                                        ".HelpID:vcl:PrintDialog:PrintControls:CheckBox",
                                                        "PrintControls",
                                                        bDefaultVal);

    // create sub section for Color
    m_aUIProperties[ nIdx++ ].Value = setSubgroupControlOpt("color", aLocalizedStrings.GetString(7), OUString());

    // create a bool option for printing text with black font color
    bDefaultVal = rDefaultPrintData.IsPrintBlackFont();
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("textinblack", aLocalizedStrings.GetString( 8 ),
                                                        ".HelpID:vcl:PrintDialog:PrintBlackFonts:CheckBox",
                                                        "PrintBlackFonts",
                                                        bDefaultVal);

    if (!bWeb)
    {
        // create subgroup for misc options
        m_aUIProperties[ nIdx++ ].Value = setSubgroupControlOpt("pages", OUString(aLocalizedStrings.GetString(9)), OUString());

        // create a bool option for printing automatically inserted blank pages
        bDefaultVal = rDefaultPrintData.IsPrintEmptyPages();
        m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("autoblankpages", aLocalizedStrings.GetString( 10 ),
                                                            ".HelpID:vcl:PrintDialog:PrintEmptyPages:CheckBox",
                                                            "PrintEmptyPages",
                                                            bDefaultVal);
    }

    // create a bool option for paper tray
    bDefaultVal = rDefaultPrintData.IsPaperFromSetup();
    vcl::PrinterOptionsHelper::UIControlOptions aPaperTrayOpt;
    aPaperTrayOpt.maGroupHint = OUString( "OptionsPageOptGroup" );
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("printpaperfromsetup", aLocalizedStrings.GetString( 11 ),
                                                        ".HelpID:vcl:PrintDialog:PrintPaperFromSetup:CheckBox",
                                                        "PrintPaperFromSetup",
                                                        bDefaultVal,
                                                        aPaperTrayOpt);

    // print range selection
    vcl::PrinterOptionsHelper::UIControlOptions aPrintRangeOpt;
    aPrintRangeOpt.maGroupHint = OUString( "PrintRange" );
    aPrintRangeOpt.mbInternalOnly = sal_True;
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("printrange", OUString(aLocalizedStrings.GetString(26)),
                                                           OUString(),
                                                           aPrintRangeOpt);

    // create a choice for the content to create
    OUString aPrintRangeName( "PrintContent" );
    uno::Sequence< OUString > aChoices( 3 );
    uno::Sequence< sal_Bool > aChoicesDisabled( 3 );
    uno::Sequence< OUString > aHelpIds( 3 );
    uno::Sequence< OUString > aWidgetIds( 3 );
    aChoices[0] = aLocalizedStrings.GetString( 27 );
    aChoicesDisabled[0] = sal_False;
    aHelpIds[0] = OUString( ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:0" );
    aWidgetIds[0] = "printallpages";
    aChoices[1] = aLocalizedStrings.GetString( 28 );
    aChoicesDisabled[1] = sal_False;
    aHelpIds[1] = OUString( ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:1" );
    aWidgetIds[1] = "printpages";
    aChoices[2] = aLocalizedStrings.GetString( 29 );
    aChoicesDisabled[2] = sal_Bool(! bHasSelection);
    aHelpIds[2] = OUString( ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:2" );
    aWidgetIds[2] = "printselection";
    m_aUIProperties[nIdx++].Value = setChoiceRadiosControlOpt(aWidgetIds, OUString(),
                                                        aHelpIds, aPrintRangeName,
                                                        aChoices, 0 /* always default to 'All pages' */,
                                                        aChoicesDisabled);
    // show an Edit dependent on "Pages" selected
    vcl::PrinterOptionsHelper::UIControlOptions aPageRangeOpt( aPrintRangeName, 1, sal_True );
    m_aUIProperties[nIdx++].Value = setEditControlOpt("pagerange", OUString(),
                                                      ".HelpID:vcl:PrintDialog:PageRange:Edit",
                                                      "PageRange",
                                                      OUString::number( nCurrentPage ) /* set text box to current page number */,
                                                      aPageRangeOpt);
    // print content selection
    vcl::PrinterOptionsHelper::UIControlOptions aContentsOpt;
    aContentsOpt.maGroupHint = OUString( "JobPage" );
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("extrawriterprintoptions", OUString(aLocalizedStrings.GetString(12)),
                                                           OUString(), aContentsOpt);
    // create a list box for notes content
    const sal_Int16 nPrintPostIts = rDefaultPrintData.GetPrintPostIts();
    aChoices.realloc( 4 );
    aChoices[0] = aLocalizedStrings.GetString( 13 );
    aChoices[1] = aLocalizedStrings.GetString( 14 );
    aChoices[2] = aLocalizedStrings.GetString( 15 );
    aChoices[3] = aLocalizedStrings.GetString( 16 );
    aHelpIds.realloc( 2 );
    aHelpIds[0] = OUString( ".HelpID:vcl:PrintDialog:PrintAnnotationMode:FixedText" );
    aHelpIds[1] = OUString( ".HelpID:vcl:PrintDialog:PrintAnnotationMode:ListBox" );
    vcl::PrinterOptionsHelper::UIControlOptions aAnnotOpt( OUString( "PrintProspect" ), 0, sal_False );
    aAnnotOpt.mbEnabled = bHasPostIts;
    m_aUIProperties[ nIdx++ ].Value = setChoiceListControlOpt("writercomments",
                                                           aLocalizedStrings.GetString( 17 ),
                                                           aHelpIds,
                                                           OUString( "PrintAnnotationMode" ),
                                                           aChoices,
                                                           nPrintPostIts,
                                                           uno::Sequence< sal_Bool >(),
                                                           aAnnotOpt);

    // create subsection for Page settings
    vcl::PrinterOptionsHelper::UIControlOptions aPageSetOpt;
    aPageSetOpt.maGroupHint = OUString( "LayoutPage" );

    if (!bWeb)
    {
        m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("pagesides", OUString(aLocalizedStrings.GetString(18)),
                                                               OUString(), aPageSetOpt);
        uno::Sequence< OUString > aRLChoices( 3 );
        aRLChoices[0] = aLocalizedStrings.GetString( 19 );
        aRLChoices[1] = aLocalizedStrings.GetString( 20 );
        aRLChoices[2] = aLocalizedStrings.GetString( 21 );
        uno::Sequence< OUString > aRLHelp( 1 );
        aRLHelp[0] = OUString( ".HelpID:vcl:PrintDialog:PrintLeftRightPages:ListBox" );
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
                                                   aLocalizedStrings.GetString(22),
                                                   aRLHelp,
                                                   "PrintLeftRightPages",
                                                   aRLChoices,
                                                   nPagesChoice);
    }

    // create a bool option for brochure
    bDefaultVal = rDefaultPrintData.IsPrintProspect();
    OUString aBrochurePropertyName( "PrintProspect" );
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("brochure", aLocalizedStrings.GetString( 23 ),
                                                        ".HelpID:vcl:PrintDialog:PrintProspect:CheckBox",
                                                        aBrochurePropertyName,
                                                        bDefaultVal,
                                                        aPageSetOpt);

    if (bCTL)
    {
        // create a bool option for brochure RTL dependent on brochure
        uno::Sequence< OUString > aBRTLChoices( 2 );
        aBRTLChoices[0] = aLocalizedStrings.GetString( 24 );
        aBRTLChoices[1] = aLocalizedStrings.GetString( 25 );
        vcl::PrinterOptionsHelper::UIControlOptions aBrochureRTLOpt( aBrochurePropertyName, -1, sal_True );
        uno::Sequence< OUString > aBRTLHelpIds( 1 );
        aBRTLHelpIds[0] = OUString( ".HelpID:vcl:PrintDialog:PrintProspectRTL:ListBox" );
        aBrochureRTLOpt.maGroupHint = OUString( "LayoutPage" );
        // RTL brochure choices
        //      0 : left-to-right
        //      1 : right-to-left
        const sal_Int16 nBRTLChoice = rDefaultPrintData.IsPrintProspectRTL() ? 1 : 0;
        m_aUIProperties[ nIdx++ ].Value = setChoiceListControlOpt("scriptdirection",
                                                               OUString(),
                                                               aBRTLHelpIds,
                                                               OUString( "PrintProspectRTL" ),
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
            !getBoolValue( "IsSkipEmptyPages", sal_True ) :
            getBoolValue( "PrintEmptyPages", sal_True );
    return bRes;
}

bool SwPrintUIOptions::IsPrintTables() const
{
    // take care of different property names currently in use for this option.
    // for compatibility the old name should win (may still be used for PDF export or via Uno API)

    // for now it was decided that tables should always be printed
    return true;
}

bool SwPrintUIOptions::IsPrintGraphics() const
{
    // take care of different property names for the option.
    // for compatibility the old name should win (may still be used for PDF export or via Uno API)

    bool bRes = getBoolValue( "PrintPicturesAndObjects", sal_True );
    bRes = getBoolValue( "PrintGraphics", bRes );
    return bRes;
}

bool SwPrintUIOptions::IsPrintDrawings() const
{
    // take care of different property names for the option.
    // for compatibility the old name should win (may still be used for PDF export or via Uno API)

    bool bRes = getBoolValue( "PrintPicturesAndObjects", sal_True );
    bRes = getBoolValue( "PrintDrawings", bRes );
    return bRes;
}

bool SwPrintUIOptions::processPropertiesAndCheckFormat( const uno::Sequence< beans::PropertyValue >& i_rNewProp )
{
    bool bChanged = processProperties( i_rNewProp );

    uno::Reference< awt::XDevice >  xRenderDevice;
    uno::Any aVal( getValue( OUString( "RenderDevice" ) ) );
    aVal >>= xRenderDevice;

    OutputDevice* pOut = 0;
    if (xRenderDevice.is())
    {
        VCLXDevice*     pDevice = VCLXDevice::GetImplementation( xRenderDevice );
        pOut = pDevice ? pDevice->GetOutputDevice() : 0;
    }
    bChanged = bChanged || (pOut != m_pLast);
    if( pOut )
        m_pLast = pOut;

    return bChanged;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
