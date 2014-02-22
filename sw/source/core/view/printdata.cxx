/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <tools/resary.hxx>
#include <unotools/moduleoptions.hxx>
#include <vcl/outdev.hxx>

extern bool sw_GetPostIts( IDocumentFieldsAccess* pIDFA, _SetGetExpFlds * pSrtLst );

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
    m_pPostItFields.reset(new _SetGetExpFlds);
    sw_GetPostIts( pDoc, m_pPostItFields.get() );

    
    
    
    SwViewOption  aViewOpt( *pViewOpt );
    aViewOpt.SetOnlineSpell( sal_False );

    m_pPostItShell.reset(new SwViewShell(*new SwDoc, 0, &aViewOpt, pOutDev));
}

void SwRenderData::DeletePostItData()
{
    if (HasPostItData())
    {
        
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

bool SwRenderData::NeedNewViewOptionAdjust( const SwViewShell& rCompare ) const
{
    return m_pViewOptionAdjust ? ! m_pViewOptionAdjust->checkShell( rCompare ) : true;
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

    
    const TypeId aSwWebDocShellTypeId = TYPE(SwWebDocShell);
    sal_Bool bWeb = pDocShell->IsA( aSwWebDocShellTypeId );
    ::sw::InitPrintOptionsFromApplication(rOptions, bWeb);

    
    rOptions.bPrintGraphic          = pOpt->IsPrintGraphics();
    rOptions.bPrintTable            = pOpt->IsPrintTables();
    rOptions.bPrintDraw             = pOpt->IsPrintDrawings();
    rOptions.bPrintControl          = pOpt->IsPrintFormControls();
    rOptions.bPrintLeftPages        = pOpt->IsPrintLeftPages();
    rOptions.bPrintRightPages       = pOpt->IsPrintRightPages();
    rOptions.bPrintPageBackground   = pOpt->IsPrintPageBackground();
    rOptions.bPrintEmptyPages       = pOpt->IsPrintEmptyPages( bIsPDFExport );
    
    rOptions.bPaperFromSetup        = pOpt->IsPaperFromSetup();
    rOptions.bPrintReverse          = pOpt->IsPrintReverse();
    rOptions.bPrintProspect         = pOpt->IsPrintProspect();
    rOptions.bPrintProspectRTL      = pOpt->IsPrintProspectRTL();
    
    
    rOptions.bPrintBlackFont        = pOpt->IsPrintWithBlackTextColor();
    rOptions.bPrintHiddenText       = pOpt->IsPrintHiddenText();
    rOptions.bPrintTextPlaceholder  = pOpt->IsPrintTextPlaceholders();
    rOptions.nPrintPostIts          = pOpt->GetPrintPostItsType();

    
    
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
    m_pLast( NULL ),
    m_rDefaultPrintData( rDefaultPrintData )
{
    ResStringArray aLocalizedStrings( SW_RES( STR_PRINTOPTUI ) );

    OSL_ENSURE( aLocalizedStrings.Count() >= 30, "resource incomplete" );
    if( aLocalizedStrings.Count() < 30 ) 
        return;

    
    
    if (bSwSrcView)
    {
        m_aUIProperties.realloc( 0 );
        return;
    }

    
    SvtLanguageOptions aLangOpt;
    bool bCTL = aLangOpt.IsCTLFontEnabled();

    
    
    const int nCTLOpts = bCTL ? 1 : 0;
    const int nNumProps = nCTLOpts + (bWeb ? 15 : 21);
    m_aUIProperties.realloc( nNumProps );
    int nIdx = 0;

    
    m_aUIProperties[nIdx].Name = "OptionsUIFile";
    m_aUIProperties[nIdx++].Value <<= OUString("modules/swriter/ui/printeroptions.ui");

    
    SvtModuleOptions aModOpt;
    OUString aAppGroupname( aLocalizedStrings.GetString( 0 ) );
    aAppGroupname = aAppGroupname.replaceFirst( "%s", aModOpt.GetModuleName( SvtModuleOptions::E_SWRITER ) );
    m_aUIProperties[ nIdx++ ].Value = setGroupControlOpt("tabcontrol-page2", aAppGroupname, ".HelpID:vcl:PrintDialog:TabPage:AppPage");

    
    m_aUIProperties[ nIdx++ ].Value = setSubgroupControlOpt("contents", aLocalizedStrings.GetString(1), OUString());

    
    bool bDefaultVal = rDefaultPrintData.IsPrintPageBackground();
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("pagebackground", aLocalizedStrings.GetString( 2 ),
                                                        ".HelpID:vcl:PrintDialog:PrintPageBackground:CheckBox",
                                                        "PrintPageBackground",
                                                        bDefaultVal);

    
    bDefaultVal = rDefaultPrintData.IsPrintGraphic() || rDefaultPrintData.IsPrintDraw();
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("pictures", aLocalizedStrings.GetString( 3 ),
                                                        ".HelpID:vcl:PrintDialog:PrintPicturesAndObjects:CheckBox",
                                                        "PrintPicturesAndObjects",
                                                         bDefaultVal);
    if (!bWeb)
    {
        
        bDefaultVal = rDefaultPrintData.IsPrintHiddenText();
        m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("hiddentext", aLocalizedStrings.GetString( 4 ),
                                                            ".HelpID:vcl:PrintDialog:PrintHiddenText:CheckBox",
                                                            "PrintHiddenText",
                                                            bDefaultVal);

        
        bDefaultVal = rDefaultPrintData.IsPrintTextPlaceholder();
        m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("placeholders", aLocalizedStrings.GetString( 5 ),
                                                            ".HelpID:vcl:PrintDialog:PrintTextPlaceholder:CheckBox",
                                                            "PrintTextPlaceholder",
                                                            bDefaultVal);
    }

    
    bDefaultVal = rDefaultPrintData.IsPrintControl();
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("formcontrols", aLocalizedStrings.GetString( 6 ),
                                                        ".HelpID:vcl:PrintDialog:PrintControls:CheckBox",
                                                        "PrintControls",
                                                        bDefaultVal);

    
    m_aUIProperties[ nIdx++ ].Value = setSubgroupControlOpt("color", aLocalizedStrings.GetString(7), OUString());

    
    bDefaultVal = rDefaultPrintData.IsPrintBlackFont();
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("textinblack", aLocalizedStrings.GetString( 8 ),
                                                        ".HelpID:vcl:PrintDialog:PrintBlackFonts:CheckBox",
                                                        "PrintBlackFonts",
                                                        bDefaultVal);

    if (!bWeb)
    {
        
        m_aUIProperties[ nIdx++ ].Value = setSubgroupControlOpt("pages", OUString(aLocalizedStrings.GetString(9)), OUString());

        
        bDefaultVal = rDefaultPrintData.IsPrintEmptyPages();
        m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("autoblankpages", aLocalizedStrings.GetString( 10 ),
                                                            ".HelpID:vcl:PrintDialog:PrintEmptyPages:CheckBox",
                                                            "PrintEmptyPages",
                                                            bDefaultVal);
    }

    
    bDefaultVal = rDefaultPrintData.IsPaperFromSetup();
    vcl::PrinterOptionsHelper::UIControlOptions aPaperTrayOpt;
    aPaperTrayOpt.maGroupHint = "OptionsPageOptGroup";
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("printpaperfromsetup", aLocalizedStrings.GetString( 11 ),
                                                        ".HelpID:vcl:PrintDialog:PrintPaperFromSetup:CheckBox",
                                                        "PrintPaperFromSetup",
                                                        bDefaultVal,
                                                        aPaperTrayOpt);

    
    vcl::PrinterOptionsHelper::UIControlOptions aPrintRangeOpt;
    aPrintRangeOpt.maGroupHint = "PrintRange";
    aPrintRangeOpt.mbInternalOnly = true;
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("printrange", OUString(aLocalizedStrings.GetString(26)),
                                                           OUString(),
                                                           aPrintRangeOpt);

    
    OUString aPrintRangeName( "PrintContent" );
    uno::Sequence< OUString > aChoices( 3 );
    uno::Sequence< sal_Bool > aChoicesDisabled( 3 );
    uno::Sequence< OUString > aHelpIds( 3 );
    uno::Sequence< OUString > aWidgetIds( 3 );
    aChoices[0] = aLocalizedStrings.GetString( 27 );
    aChoicesDisabled[0] = sal_False;
    aHelpIds[0] = ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:0";
    aWidgetIds[0] = "printallpages";
    aChoices[1] = aLocalizedStrings.GetString( 28 );
    aChoicesDisabled[1] = sal_False;
    aHelpIds[1] = ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:1";
    aWidgetIds[1] = "printpages";
    aChoices[2] = aLocalizedStrings.GetString( 29 );
    aChoicesDisabled[2] = sal_Bool(! bHasSelection);
    aHelpIds[2] = ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:2";
    aWidgetIds[2] = "printselection";
    m_aUIProperties[nIdx++].Value = setChoiceRadiosControlOpt(aWidgetIds, OUString(),
                                                        aHelpIds, aPrintRangeName,
                                                        aChoices, 0 /* always default to 'All pages' */,
                                                        aChoicesDisabled);
    
    vcl::PrinterOptionsHelper::UIControlOptions aPageRangeOpt( aPrintRangeName, 1, true );
    m_aUIProperties[nIdx++].Value = setEditControlOpt("pagerange", OUString(),
                                                      ".HelpID:vcl:PrintDialog:PageRange:Edit",
                                                      "PageRange",
                                                      OUString::number( nCurrentPage ) /* set text box to current page number */,
                                                      aPageRangeOpt);
    
    vcl::PrinterOptionsHelper::UIControlOptions aContentsOpt;
    aContentsOpt.maGroupHint = "JobPage";
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("extrawriterprintoptions", OUString(aLocalizedStrings.GetString(12)),
                                                           OUString(), aContentsOpt);
    
    const sal_Int16 nPrintPostIts = rDefaultPrintData.GetPrintPostIts();
    aChoices.realloc( 4 );
    aChoices[0] = aLocalizedStrings.GetString( 13 );
    aChoices[1] = aLocalizedStrings.GetString( 14 );
    aChoices[2] = aLocalizedStrings.GetString( 15 );
    aChoices[3] = aLocalizedStrings.GetString( 16 );
    aHelpIds.realloc( 2 );
    aHelpIds[0] = ".HelpID:vcl:PrintDialog:PrintAnnotationMode:FixedText";
    aHelpIds[1] = ".HelpID:vcl:PrintDialog:PrintAnnotationMode:ListBox";
    vcl::PrinterOptionsHelper::UIControlOptions aAnnotOpt( OUString( "PrintProspect" ), 0, false );
    aAnnotOpt.mbEnabled = bHasPostIts;
    m_aUIProperties[ nIdx++ ].Value = setChoiceListControlOpt("writercomments",
                                                           aLocalizedStrings.GetString( 17 ),
                                                           aHelpIds,
                                                           OUString( "PrintAnnotationMode" ),
                                                           aChoices,
                                                           nPrintPostIts,
                                                           uno::Sequence< sal_Bool >(),
                                                           aAnnotOpt);

    
    vcl::PrinterOptionsHelper::UIControlOptions aPageSetOpt;
    aPageSetOpt.maGroupHint = "LayoutPage";

    if (!bWeb)
    {
        m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("pagesides", OUString(aLocalizedStrings.GetString(18)),
                                                               OUString(), aPageSetOpt);
        uno::Sequence< OUString > aRLChoices( 3 );
        aRLChoices[0] = aLocalizedStrings.GetString( 19 );
        aRLChoices[1] = aLocalizedStrings.GetString( 20 );
        aRLChoices[2] = aLocalizedStrings.GetString( 21 );
        uno::Sequence< OUString > aRLHelp( 1 );
        aRLHelp[0] = ".HelpID:vcl:PrintDialog:PrintLeftRightPages:ListBox";
        
        
        
        
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

    
    bDefaultVal = rDefaultPrintData.IsPrintProspect();
    OUString aBrochurePropertyName( "PrintProspect" );
    m_aUIProperties[ nIdx++ ].Value = setBoolControlOpt("brochure", aLocalizedStrings.GetString( 23 ),
                                                        ".HelpID:vcl:PrintDialog:PrintProspect:CheckBox",
                                                        aBrochurePropertyName,
                                                        bDefaultVal,
                                                        aPageSetOpt);

    if (bCTL)
    {
        
        uno::Sequence< OUString > aBRTLChoices( 2 );
        aBRTLChoices[0] = aLocalizedStrings.GetString( 24 );
        aBRTLChoices[1] = aLocalizedStrings.GetString( 25 );
        vcl::PrinterOptionsHelper::UIControlOptions aBrochureRTLOpt( aBrochurePropertyName, -1, true );
        uno::Sequence< OUString > aBRTLHelpIds( 1 );
        aBRTLHelpIds[0] = ".HelpID:vcl:PrintDialog:PrintProspectRTL:ListBox";
        aBrochureRTLOpt.maGroupHint = "LayoutPage";
        
        
        
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
    
    

    
    
    
    sal_Int64 nLRPages = getIntValue( "PrintLeftRightPages", 0 /* default: all */ );
    bool bRes = nLRPages == 0 || nLRPages == 1;
    bRes = getBoolValue( "PrintLeftPages", bRes /* <- default value if property is not found */ );
    return bRes;
}

bool SwPrintUIOptions::IsPrintRightPages() const
{
    
    

    sal_Int64 nLRPages = getIntValue( "PrintLeftRightPages", 0 /* default: all */ );
    bool bRes = nLRPages == 0 || nLRPages == 2;
    bRes = getBoolValue( "PrintRightPages", bRes /* <- default value if property is not found */ );
    return bRes;
}

bool SwPrintUIOptions::IsPrintEmptyPages( bool bIsPDFExport ) const
{
    

    bool bRes = bIsPDFExport ?
            !getBoolValue( "IsSkipEmptyPages", true ) :
            getBoolValue( "PrintEmptyPages", true );
    return bRes;
}

bool SwPrintUIOptions::IsPrintTables() const
{
    
    

    
    return true;
}

bool SwPrintUIOptions::IsPrintGraphics() const
{
    
    

    bool bRes = getBoolValue( "PrintPicturesAndObjects", true );
    bRes = getBoolValue( "PrintGraphics", bRes );
    return bRes;
}

bool SwPrintUIOptions::IsPrintDrawings() const
{
    
    

    bool bRes = getBoolValue( "PrintPicturesAndObjects", true );
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
