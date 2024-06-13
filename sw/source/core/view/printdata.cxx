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

#include <strings.hrc>
#include <doc.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <unotxdoc.hxx>
#include <wdocsh.hxx>
#include <viewsh.hxx>
#include <docfld.hxx>

#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <comphelper/configuration.hxx>
#include <unotools/moduleoptions.hxx>
#include <vcl/outdev.hxx>
#include <osl/diagnose.h>

using namespace ::com::sun::star;

SwRenderData::SwRenderData()
{
}

SwRenderData::~SwRenderData()
{
    OSL_ENSURE( !m_pPostItShell, "m_pPostItShell should already have been deleted" );
    OSL_ENSURE( !m_pPostItFields, " should already have been deleted" );
}

void SwRenderData::CreatePostItData( SwDoc& rDoc, const SwViewOption *pViewOpt, OutputDevice *pOutDev )
{
    DeletePostItData();
    m_pPostItFields.reset(new SetGetExpFields);
    sw_GetPostIts( rDoc.getIDocumentFieldsAccess(), m_pPostItFields.get() );

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
        {   // avoid destroying layout from SwDoc dtor
            rtl::Reference<SwDoc> const xKeepAlive(m_pPostItShell->GetDoc());
            m_pPostItShell.reset();
        }
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

void SwRenderData::ViewOptionAdjust(SwPrintData const*const pPrtOptions, bool setShowPlaceHoldersInPDF)
{
    m_pViewOptionAdjust->AdjustViewOptions( pPrtOptions, setShowPlaceHoldersInPDF );
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
    rOptions.m_bPrintControl          = pOpt->IsPrintFormControls();
    rOptions.m_bPrintLeftPages        = pOpt->IsPrintLeftPages();
    rOptions.m_bPrintRightPages       = pOpt->IsPrintRightPages();
    rOptions.m_bPrintPageBackground   = pOpt->IsPrintPageBackground();
    rOptions.m_bPrintEmptyPages       = pOpt->IsPrintEmptyPages( bIsPDFExport );
    // bUpdateFieldsInPrinting  <-- not set here; mail merge only
    rOptions.m_bPaperFromSetup        = pOpt->IsPaperFromSetup();
    rOptions.m_bPrintProspect         = pOpt->IsPrintProspect();
    rOptions.m_bPrintProspectRTL      = pOpt->IsPrintProspectRTL();
    // bModified                <-- not set here; mail merge only
    rOptions.m_bPrintBlackFont        = pOpt->IsPrintWithBlackTextColor();
    rOptions.m_bPrintHiddenText       = pOpt->IsPrintHiddenText();
    rOptions.m_bPrintTextPlaceholder  = pOpt->IsPrintTextPlaceholders();
    rOptions.m_nPrintPostIts          = pOpt->GetPrintPostItsType();

    //! needs to be set after MakeOptions since the assignment operation in that
    //! function will destroy the pointers
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
    // It's just the source code that gets printed...
    if (bSwSrcView || comphelper::IsFuzzing())
    {
        m_aUIProperties.clear();
        return;
    }

    // check if either CJK or CTL is enabled
    bool bRTL = SvtCJKOptions::IsCJKFontEnabled() || SvtCTLOptions::IsCTLFontEnabled();

    // create sequence of print UI options
    // (5 options are not available for Writer-Web)
    const int nRTLOpts = bRTL ? 1 : 0;
    const int nNumProps = nRTLOpts + (bWeb ? 15 : 19);
    m_aUIProperties.resize( nNumProps);
    int nIdx = 0;

    // load the writer PrinterOptions into the custom tab
    m_aUIProperties[nIdx].Name = "OptionsUIFile";
    m_aUIProperties[nIdx++].Value <<= u"modules/swriter/ui/printeroptions.ui"_ustr;

    // create "writer" section (new tab page in dialog)
    SvtModuleOptions aModOpt;
    OUString aAppGroupname( SwResId( STR_PRINTOPTUI_PRODUCTNAME) );
    aAppGroupname = aAppGroupname.replaceFirst( "%s", aModOpt.GetModuleName( SvtModuleOptions::EModule::WRITER ) );
    m_aUIProperties[ nIdx++ ].Value = setGroupControlOpt(u"tabcontrol-page2"_ustr, aAppGroupname, u".HelpID:vcl:PrintDialog:TabPage:AppPage"_ustr);

    // create sub section for Contents
    m_aUIProperties[ nIdx++ ].Value = setSubgroupControlOpt(u"contents"_ustr, SwResId( STR_PRINTOPTUI_CONTENTS), OUString());

    // create a bool option for background
    bool bDefaultVal = rDefaultPrintData.IsPrintPageBackground();
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt(u"pagebackground"_ustr, SwResId( STR_PRINTOPTUI_PAGE_BACKGROUND),
                                                        u".HelpID:vcl:PrintDialog:PrintPageBackground:CheckBox"_ustr,
                                                        u"PrintPageBackground"_ustr,
                                                        bDefaultVal);

    // create a bool option for pictures/graphics AND OLE and drawing objects as well
    bDefaultVal = rDefaultPrintData.IsPrintGraphic();
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt(u"pictures"_ustr, SwResId( STR_PRINTOPTUI_PICTURES),
                                                        u".HelpID:vcl:PrintDialog:PrintPicturesAndObjects:CheckBox"_ustr,
                                                        u"PrintPicturesAndObjects"_ustr,
                                                         bDefaultVal);
    if (!bWeb)
    {
        // create a bool option for hidden text
        bDefaultVal = rDefaultPrintData.IsPrintHiddenText();
        m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt(u"hiddentext"_ustr, SwResId( STR_PRINTOPTUI_HIDDEN),
                                                            u".HelpID:vcl:PrintDialog:PrintHiddenText:CheckBox"_ustr,
                                                            u"PrintHiddenText"_ustr,
                                                            bDefaultVal);

        // create a bool option for place holder
        bDefaultVal = rDefaultPrintData.IsPrintTextPlaceholder();
        m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt(u"placeholders"_ustr, SwResId( STR_PRINTOPTUI_TEXT_PLACEHOLDERS),
                                                            u".HelpID:vcl:PrintDialog:PrintTextPlaceholder:CheckBox"_ustr,
                                                            u"PrintTextPlaceholder"_ustr,
                                                            bDefaultVal);
    }

    // create a bool option for controls
    bDefaultVal = rDefaultPrintData.IsPrintControl();
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt(u"formcontrols"_ustr, SwResId( STR_PRINTOPTUI_FORM_CONTROLS),
                                                        u".HelpID:vcl:PrintDialog:PrintControls:CheckBox"_ustr,
                                                        u"PrintControls"_ustr,
                                                        bDefaultVal);

    // create sub section for Color
    m_aUIProperties[ nIdx++ ].Value = setSubgroupControlOpt(u"color"_ustr, SwResId( STR_PRINTOPTUI_COLOR), OUString());

    // create a bool option for printing text with black font color
    bDefaultVal = rDefaultPrintData.IsPrintBlackFont();
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt(u"textinblack"_ustr, SwResId( STR_PRINTOPTUI_PRINT_BLACK),
                                                        u".HelpID:vcl:PrintDialog:PrintBlackFonts:CheckBox"_ustr,
                                                        u"PrintBlackFonts"_ustr,
                                                        bDefaultVal);

    if (!bWeb)
    {
        // create subgroup for misc options
        m_aUIProperties[ nIdx++ ].Value = setSubgroupControlOpt(u"pages"_ustr, SwResId( STR_PRINTOPTUI_PAGES_TEXT), OUString());

        // create a bool option for printing automatically inserted blank pages
        bDefaultVal = rDefaultPrintData.IsPrintEmptyPages();
        m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt(u"autoblankpages"_ustr, SwResId( STR_PRINTOPTUI_PRINT_BLANK),
                                                            u".HelpID:vcl:PrintDialog:PrintEmptyPages:CheckBox"_ustr,
                                                            u"PrintEmptyPages"_ustr,
                                                            bDefaultVal);
    }

    // create a bool option for paper tray
    bDefaultVal = rDefaultPrintData.IsPaperFromSetup();
    vcl::PrinterOptionsHelper::UIControlOptions aPaperTrayOpt;
    aPaperTrayOpt.maGroupHint = "OptionsPageOptGroup";
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt(u"printpaperfromsetup"_ustr, SwResId( STR_PRINTOPTUI_ONLY_PAPER),
                                                        u".HelpID:vcl:PrintDialog:PrintPaperFromSetup:CheckBox"_ustr,
                                                        u"PrintPaperFromSetup"_ustr,
                                                        bDefaultVal,
                                                        aPaperTrayOpt);

    // print range selection
    vcl::PrinterOptionsHelper::UIControlOptions aPrintRangeOpt;
    aPrintRangeOpt.maGroupHint = "PrintRange";
    aPrintRangeOpt.mbInternalOnly = true;
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt( u"printrange"_ustr,
                                                          SwResId( STR_PRINTOPTUI_PAGES_TEXT ),
                                                          OUString(),
                                                          aPrintRangeOpt );

    // create a choice for the content to create
    static constexpr OUString aPrintRangeName( u"PrintContent"_ustr );
    uno::Sequence< OUString > aChoices{ SwResId( STR_PRINTOPTUI_PRINTALLPAGES ),
                                        SwResId( STR_PRINTOPTUI_PRINTPAGES ),
                                        SwResId( STR_PRINTOPTUI_PRINTSELECTION ) };
    uno::Sequence< sal_Bool > aChoicesDisabled{ false, false, !bHasSelection };
    uno::Sequence< OUString > aHelpIds{ u".HelpID:vcl:PrintDialog:PrintContent:RadioButton:0"_ustr,
                                        u".HelpID:vcl:PrintDialog:PrintContent:RadioButton:1"_ustr,
                                        u".HelpID:vcl:PrintDialog:PrintContent:RadioButton:2"_ustr };
    uno::Sequence< OUString > aWidgetIds{ u"rbAllPages"_ustr, u"rbRangePages"_ustr, u"rbRangeSelection"_ustr };
    m_aUIProperties[nIdx++].Value = setChoiceRadiosControlOpt(aWidgetIds, OUString(),
                                                        aHelpIds, aPrintRangeName,
                                                        aChoices, 0 /* always default to 'All pages' */,
                                                        aChoicesDisabled);

    // show an Edit dependent on "Pages" selected
    vcl::PrinterOptionsHelper::UIControlOptions aPageRangeOpt( aPrintRangeName, 1, true );
    m_aUIProperties[nIdx++].Value = setEditControlOpt(u"pagerange"_ustr, OUString(),
                                                      u".HelpID:vcl:PrintDialog:PageRange:Edit"_ustr,
                                                      u"PageRange"_ustr,
                                                      OUString::number( nCurrentPage ) /* set text box to current page number */,
                                                      aPageRangeOpt);

    vcl::PrinterOptionsHelper::UIControlOptions aEvenOddOpt(aPrintRangeName, -1, true);
    m_aUIProperties[ nIdx++ ].Value = setChoiceListControlOpt(u"evenoddbox"_ustr,
                                                           OUString(),
                                                           uno::Sequence<OUString>(),
                                                           u"EvenOdd"_ustr,
                                                           uno::Sequence<OUString>(),
                                                           0,
                                                           uno::Sequence< sal_Bool >(),
                                                           aEvenOddOpt);

    // create a list box for notes content
    const SwPostItMode nPrintPostIts = rDefaultPrintData.GetPrintPostIts();
    aChoices = { SwResId( STR_PRINTOPTUI_NONE),
                 SwResId( STR_PRINTOPTUI_COMMENTS_ONLY),
                 SwResId( STR_PRINTOPTUI_PLACE_END),
                 SwResId( STR_PRINTOPTUI_PLACE_PAGE),
                 SwResId( STR_PRINTOPTUI_PLACE_MARGINS) };
    aHelpIds = { u".HelpID:vcl:PrintDialog:PrintAnnotationMode:FixedText"_ustr,
                 u".HelpID:vcl:PrintDialog:PrintAnnotationMode:ListBox"_ustr };
    vcl::PrinterOptionsHelper::UIControlOptions aAnnotOpt( u"PrintProspect"_ustr, 0, false );
    aAnnotOpt.mbEnabled = bHasPostIts;
    m_aUIProperties[ nIdx++ ].Value = setChoiceListControlOpt(u"writercomments"_ustr,
                                                           SwResId( STR_PRINTOPTUI_COMMENTS),
                                                           aHelpIds,
                                                           u"PrintAnnotationMode"_ustr,
                                                           aChoices,
                                                           bHasPostIts ? static_cast<sal_uInt16>(nPrintPostIts) : 0,
                                                           uno::Sequence< sal_Bool >(),
                                                           aAnnotOpt);

    // create subsection for Page settings
    vcl::PrinterOptionsHelper::UIControlOptions aPageSetOpt;
    aPageSetOpt.maGroupHint = "LayoutPage";

    // create a bool option for brochure
    bDefaultVal = rDefaultPrintData.IsPrintProspect();
    static constexpr OUString aBrochurePropertyName( u"PrintProspect"_ustr );
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt(u"brochure"_ustr, SwResId( STR_PRINTOPTUI_BROCHURE),
                                                        u".HelpID:vcl:PrintDialog:PrintProspect:CheckBox"_ustr,
                                                        aBrochurePropertyName,
                                                        bDefaultVal,
                                                        aPageSetOpt);

    if (bRTL)
    {
        // create a bool option for brochure RTL dependent on brochure
        uno::Sequence< OUString > aBRTLChoices{ SwResId( STR_PRINTOPTUI_LEFT_SCRIPT),
                                                SwResId( STR_PRINTOPTUI_RIGHT_SCRIPT) };
        vcl::PrinterOptionsHelper::UIControlOptions aBrochureRTLOpt( aBrochurePropertyName, -1, true );
        uno::Sequence<OUString> aBRTLHelpIds { u".HelpID:vcl:PrintDialog:PrintProspectRTL:ListBox"_ustr };
        aBrochureRTLOpt.maGroupHint = "LayoutPage";
        // RTL brochure choices
        //      0 : left-to-right
        //      1 : right-to-left
        const sal_Int16 nBRTLChoice = rDefaultPrintData.IsPrintProspectRTL() ? 1 : 0;
        m_aUIProperties[ nIdx++ ].Value = setChoiceListControlOpt(u"scriptdirection"_ustr,
                                                               OUString(),
                                                               aBRTLHelpIds,
                                                               u"PrintProspectRTL"_ustr,
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
    sal_Int64 nEOPages = getIntValue( "EvenOdd", 0 /* default: all */ );
    bool bRes = nEOPages != 1;
    bRes = getBoolValue( "EvenOdd", bRes /* <- default value if property is not found */ );
    return bRes;
}

bool SwPrintUIOptions::IsPrintRightPages() const
{
    // take care of different property names for the option.
    // for compatibility the old name should win (may still be used for PDF export or via Uno API)

    sal_Int64 nEOPages = getIntValue( "EvenOdd", 0 /* default: all */ );
    bool bRes = nEOPages != 2;
    bRes = getBoolValue( "EvenOdd", bRes /* <- default value if property is not found */ );
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

bool SwPrintUIOptions::processPropertiesAndCheckFormat( const uno::Sequence< beans::PropertyValue >& i_rNewProp )
{
    bool bChanged = processProperties( i_rNewProp );

    uno::Reference< awt::XDevice >  xRenderDevice;
    uno::Any aVal( getValue( u"RenderDevice"_ustr ) );
    aVal >>= xRenderDevice;

    VclPtr< OutputDevice > pOut;
    if (xRenderDevice.is())
    {
        VCLXDevice* pDevice = dynamic_cast<VCLXDevice*>( xRenderDevice.get() );
        if (pDevice)
            pOut = pDevice->GetOutputDevice();
    }
    bChanged = bChanged || (pOut.get() != m_pLast.get());
    if( pOut )
        m_pLast = pOut;

    return bChanged;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
