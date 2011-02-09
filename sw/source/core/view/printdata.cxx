/**************************************************************************
 *
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vprint.cxx,v $
 * $Revision: 1.46 $
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
#include "precompiled_sw.hxx"

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
#include <tools/debug.hxx>
#include <tools/resary.hxx>
#include <unotools/moduleoptions.hxx>
#include <vcl/outdev.hxx>


extern bool lcl_GetPostIts( IDocumentFieldsAccess* pIDFA, _SetGetExpFlds * pSrtLst );


using namespace ::com::sun::star;
using ::rtl::OUString;


//////////////////////////////////////////////////////////////////////

SwRenderData::SwRenderData()
{
    m_pPostItFields   = 0;
    m_pPostItDoc      = 0;
    m_pPostItShell    = 0;

    m_pViewOptionAdjust = 0;
    m_pPrtOptions       = 0;
}


SwRenderData::~SwRenderData()
{
    delete m_pViewOptionAdjust;     m_pViewOptionAdjust = 0;
    delete m_pPrtOptions;           m_pPrtOptions = 0;
    DBG_ASSERT( !m_pPostItShell, "m_pPostItShell should already have been deleted" );
    DBG_ASSERT( !m_pPostItDoc, "m_pPostItDoc should already have been deleted" );
    DBG_ASSERT( !m_pPostItFields, " should already have been deleted" );
}


void SwRenderData::CreatePostItData( SwDoc *pDoc, const SwViewOption *pViewOpt, OutputDevice *pOutDev )
{
    DBG_ASSERT( !m_pPostItFields && !m_pPostItDoc && !m_pPostItShell, "some post-it data already exists" );
    m_pPostItFields = new _SetGetExpFlds;
    lcl_GetPostIts( pDoc, m_pPostItFields );
    m_pPostItDoc    = new SwDoc;

    //!! Disable spell and grammar checking in the temporary document.
    //!! Otherwise the grammar checker might process it and crash if we later on
    //!! simply delete this document while he is still at it.
    SwViewOption  aViewOpt( *pViewOpt );
    aViewOpt.SetOnlineSpell( sal_False );

    m_pPostItShell  = new ViewShell( *m_pPostItDoc, 0, &aViewOpt, pOutDev );
}


void SwRenderData::DeletePostItData()
{
    if (HasPostItData())
    {
        m_pPostItDoc->setPrinter( 0, false, false );  //damit am echten DOC der Drucker bleibt
        delete m_pPostItShell;        //Nimmt das PostItDoc mit ins Grab.
        delete m_pPostItFields;
        m_pPostItDoc    = 0;
        m_pPostItShell  = 0;
        m_pPostItFields = 0;
    }
}

bool SwRenderData::NeedNewViewOptionAdjust( const ViewShell& rCompare ) const
{
    return m_pViewOptionAdjust ? ! m_pViewOptionAdjust->checkShell( rCompare ) : true;
}


void SwRenderData::ViewOptionAdjustStart( ViewShell &rSh, const SwViewOption &rViewOptions )
{
    if (m_pViewOptionAdjust)
    {
        DBG_ASSERT( 0, "error: there should be no ViewOptionAdjust active when calling this function" );
    }
    m_pViewOptionAdjust = new SwViewOptionAdjust_Impl( rSh, rViewOptions );
}


void SwRenderData::ViewOptionAdjust(SwPrintData const*const pPrtOptions)
{
    m_pViewOptionAdjust->AdjustViewOptions( pPrtOptions );
}


void SwRenderData::ViewOptionAdjustStop()
{
    if (m_pViewOptionAdjust)
    {
        delete m_pViewOptionAdjust;
        m_pViewOptionAdjust = 0;
    }
}


void SwRenderData::MakeSwPrtOptions(
    SwPrintData & rOptions,
    const SwDocShell *pDocShell,
    const SwPrintUIOptions *pOpt,
    const SwRenderData *pData,
    bool bIsPDFExport )
{
    if (!pDocShell || !pOpt || !pData)
        return;

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
    rOptions.SetRenderData( pData );
}


//////////////////////////////////////////////////////////////////////

SwPrintUIOptions::SwPrintUIOptions(
    bool bWeb,
    bool bSwSrcView,
    bool bHasSelection,
    bool bHasPostIts,
    const SwPrintData &rDefaultPrintData ) :
    m_pLast( NULL ),
    m_rDefaultPrintData( rDefaultPrintData )
{
    ResStringArray aLocalizedStrings( SW_RES( STR_PRINTOPTUI ) );

    DBG_ASSERT( aLocalizedStrings.Count() >= 30, "resource incomplete" );
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
    const int nNumProps = nCTLOpts + (bWeb ? 14 : 20);
    m_aUIProperties.realloc( nNumProps );
    int nIdx = 0;

    // create "writer" section (new tab page in dialog)
    SvtModuleOptions aModOpt;
    String aAppGroupname( aLocalizedStrings.GetString( 0 ) );
    aAppGroupname.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s" ) ),
                                    aModOpt.GetModuleName( SvtModuleOptions::E_SWRITER ) );
    m_aUIProperties[ nIdx++ ].Value = getGroupControlOpt( aAppGroupname, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:TabPage:AppPage" ) ) );

    // create sub section for Contents
    m_aUIProperties[ nIdx++ ].Value = getSubgroupControlOpt( aLocalizedStrings.GetString( 1 ), rtl::OUString() );

    // create a bool option for background
    bool bDefaultVal = rDefaultPrintData.IsPrintPageBackground();
    m_aUIProperties[ nIdx++ ].Value = getBoolControlOpt( aLocalizedStrings.GetString( 2 ),
                                                         rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintPageBackground:CheckBox" ) ),
                                                         rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintPageBackground" ) ),
                                                         bDefaultVal );

    // create a bool option for pictures/graphics AND OLE and drawing objects as well
    bDefaultVal = rDefaultPrintData.IsPrintGraphic() || rDefaultPrintData.IsPrintDraw();
    m_aUIProperties[ nIdx++ ].Value = getBoolControlOpt( aLocalizedStrings.GetString( 3 ),
                                                         rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintPicturesAndObjects:CheckBox" ) ),
                                                         rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintPicturesAndObjects" ) ),
                                                         bDefaultVal );
    if (!bWeb)
    {
        // create a bool option for hidden text
        bDefaultVal = rDefaultPrintData.IsPrintHiddenText();
        m_aUIProperties[ nIdx++ ].Value = getBoolControlOpt( aLocalizedStrings.GetString( 4 ),
                                                             rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintHiddenText:CheckBox" ) ),
                                                             rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintHiddenText" ) ),
                                                             bDefaultVal );

        // create a bool option for place holder
        bDefaultVal = rDefaultPrintData.IsPrintTextPlaceholder();
        m_aUIProperties[ nIdx++ ].Value = getBoolControlOpt( aLocalizedStrings.GetString( 5 ),
                                                             rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintTextPlaceholder:CheckBox" ) ),
                                                             rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintTextPlaceholder" ) ),
                                                             bDefaultVal );
    }

    // create a bool option for controls
    bDefaultVal = rDefaultPrintData.IsPrintControl();
    m_aUIProperties[ nIdx++ ].Value = getBoolControlOpt( aLocalizedStrings.GetString( 6 ),
                                                         rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintControls:CheckBox" ) ),
                                                         rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintControls" ) ),
                                                         bDefaultVal );

    // create sub section for Color
    m_aUIProperties[ nIdx++ ].Value = getSubgroupControlOpt( aLocalizedStrings.GetString( 7 ), rtl::OUString() );

    // create a bool option for printing text with black font color
    bDefaultVal = rDefaultPrintData.IsPrintBlackFont();
    m_aUIProperties[ nIdx++ ].Value = getBoolControlOpt( aLocalizedStrings.GetString( 8 ),
                                                         rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintBlackFonts:CheckBox" ) ),
                                                         rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintBlackFonts" ) ),
                                                         bDefaultVal );

    if (!bWeb)
    {
        // create subgroup for misc options
        m_aUIProperties[ nIdx++ ].Value = getSubgroupControlOpt( rtl::OUString( aLocalizedStrings.GetString( 9 ) ), rtl::OUString() );

        // create a bool option for printing automatically inserted blank pages
        bDefaultVal = rDefaultPrintData.IsPrintEmptyPages();
        m_aUIProperties[ nIdx++ ].Value = getBoolControlOpt( aLocalizedStrings.GetString( 10 ),
                                                             rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintEmptyPages:CheckBox" ) ),
                                                             rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintEmptyPages" ) ),
                                                             bDefaultVal );
    }

    // create a bool option for paper tray
    bDefaultVal = rDefaultPrintData.IsPaperFromSetup();
    vcl::PrinterOptionsHelper::UIControlOptions aPaperTrayOpt;
    aPaperTrayOpt.maGroupHint = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OptionsPageOptGroup" ) );
    m_aUIProperties[ nIdx++ ].Value = getBoolControlOpt( aLocalizedStrings.GetString( 11 ),
                                                         rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintPaperFromSetup:CheckBox" ) ),
                                                         rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintPaperFromSetup" ) ),
                                                         bDefaultVal,
                                                         aPaperTrayOpt
                                                         );

    // print range selection
    vcl::PrinterOptionsHelper::UIControlOptions aPrintRangeOpt;
    aPrintRangeOpt.maGroupHint = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintRange" ) );
    aPrintRangeOpt.mbInternalOnly = sal_True;
    m_aUIProperties[nIdx++].Value = getSubgroupControlOpt( rtl::OUString( aLocalizedStrings.GetString( 26 ) ),
                                                           rtl::OUString(),
                                                           aPrintRangeOpt
                                                           );

    // create a choice for the content to create
    rtl::OUString aPrintRangeName( RTL_CONSTASCII_USTRINGPARAM( "PrintContent" ) );
    uno::Sequence< rtl::OUString > aChoices( 3 );
    uno::Sequence< sal_Bool > aChoicesDisabled( 3 );
    uno::Sequence< rtl::OUString > aHelpIds( 3 );
    aChoices[0] = aLocalizedStrings.GetString( 27 );
    aChoicesDisabled[0] = sal_False;
    aHelpIds[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:0" ) );
    aChoices[1] = aLocalizedStrings.GetString( 28 );
    aChoicesDisabled[1] = sal_False;
    aHelpIds[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:1" ) );
    aChoices[2] = aLocalizedStrings.GetString( 29 );
    aChoicesDisabled[2] = sal_Bool(! bHasSelection);
    aHelpIds[2] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:2" ) );
    m_aUIProperties[nIdx++].Value = getChoiceControlOpt( rtl::OUString(),
                                                         aHelpIds,
                                                         aPrintRangeName,
                                                         aChoices,
                                                         bHasSelection ? 2 /*enable 'Selection' radio button*/ : 0 /* enable 'All pages' */,
                                                         rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Radio" ) ),
                                                         aChoicesDisabled
                                                         );
    // create a an Edit dependent on "Pages" selected
    vcl::PrinterOptionsHelper::UIControlOptions aPageRangeOpt( aPrintRangeName, 1, sal_True );
    m_aUIProperties[nIdx++].Value = getEditControlOpt( rtl::OUString(),
                                                       rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PageRange:Edit" ) ),
                                                       rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PageRange" ) ),
                                                       rtl::OUString(),
                                                       aPageRangeOpt
                                                       );
    // print content selection
    vcl::PrinterOptionsHelper::UIControlOptions aContentsOpt;
    aContentsOpt.maGroupHint = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "JobPage" ) );
    m_aUIProperties[nIdx++].Value = getSubgroupControlOpt( rtl::OUString( aLocalizedStrings.GetString( 12 ) ),
                                                           rtl::OUString(),
                                                           aContentsOpt
                                                           );
    // create a list box for notes content
    const sal_Int16 nPrintPostIts = rDefaultPrintData.GetPrintPostIts();
    aChoices.realloc( 4 );
    aChoices[0] = aLocalizedStrings.GetString( 13 );
    aChoices[1] = aLocalizedStrings.GetString( 14 );
    aChoices[2] = aLocalizedStrings.GetString( 15 );
    aChoices[3] = aLocalizedStrings.GetString( 16 );
    aHelpIds.realloc( 2 );
    aHelpIds[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintAnnotationMode:FixedText" ) );
    aHelpIds[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintAnnotationMode:ListBox" ) );
    vcl::PrinterOptionsHelper::UIControlOptions aAnnotOpt( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintProspect" ) ), 0, sal_False );
    aAnnotOpt.mbEnabled = bHasPostIts;
    m_aUIProperties[ nIdx++ ].Value = getChoiceControlOpt( aLocalizedStrings.GetString( 17 ),
                                                           aHelpIds,
                                                           rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintAnnotationMode" ) ),
                                                           aChoices,
                                                           nPrintPostIts,
                                                           rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "List" ) ),
                                                           uno::Sequence< sal_Bool >(),
                                                           aAnnotOpt
                                                           );

    // create subsection for Page settings
    vcl::PrinterOptionsHelper::UIControlOptions aPageSetOpt;
    aPageSetOpt.maGroupHint = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutPage" ) );

    if (!bWeb)
    {
        m_aUIProperties[nIdx++].Value = getSubgroupControlOpt( rtl::OUString( aLocalizedStrings.GetString( 18 ) ),
                                                               rtl::OUString(),
                                                               aPageSetOpt
                                                               );
        uno::Sequence< rtl::OUString > aRLChoices( 3 );
        aRLChoices[0] = aLocalizedStrings.GetString( 19 );
        aRLChoices[1] = aLocalizedStrings.GetString( 20 );
        aRLChoices[2] = aLocalizedStrings.GetString( 21 );
        uno::Sequence< rtl::OUString > aRLHelp( 1 );
        aRLHelp[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintLeftRightPages:ListBox" ) );
        // create a choice option for all/left/right pages
        // 0 : all pages (left & right)
        // 1 : left pages
        // 2 : right pages
        DBG_ASSERT( rDefaultPrintData.IsPrintLeftPage() || rDefaultPrintData.IsPrintRightPage(),
                "unexpected value combination" );
        sal_Int16 nPagesChoice = 0;
        if (rDefaultPrintData.IsPrintLeftPage() && !rDefaultPrintData.IsPrintRightPage())
            nPagesChoice = 1;
        else if (!rDefaultPrintData.IsPrintLeftPage() && rDefaultPrintData.IsPrintRightPage())
            nPagesChoice = 2;
        m_aUIProperties[ nIdx++ ].Value = getChoiceControlOpt( aLocalizedStrings.GetString( 22 ),
                                                   aRLHelp,
                                                   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintLeftRightPages" ) ),
                                                   aRLChoices,
                                                   nPagesChoice,
                                                   rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "List" ) )
                                                   );
    }

    // create a bool option for brochure
    bDefaultVal = rDefaultPrintData.IsPrintProspect();
    rtl::OUString aBrochurePropertyName( RTL_CONSTASCII_USTRINGPARAM( "PrintProspect" ) );
    m_aUIProperties[ nIdx++ ].Value = getBoolControlOpt( aLocalizedStrings.GetString( 23 ),
                                                         rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintProspect:CheckBox" ) ),
                                                         aBrochurePropertyName,
                                                         bDefaultVal,
                                                         aPageSetOpt
                                                         );

    if (bCTL)
    {
        // create a bool option for brochure RTL dependent on brochure
        uno::Sequence< rtl::OUString > aBRTLChoices( 2 );
        aBRTLChoices[0] = aLocalizedStrings.GetString( 24 );
        aBRTLChoices[1] = aLocalizedStrings.GetString( 25 );
        vcl::PrinterOptionsHelper::UIControlOptions aBrochureRTLOpt( aBrochurePropertyName, -1, sal_True );
        uno::Sequence< rtl::OUString > aBRTLHelpIds( 1 );
        aBRTLHelpIds[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintProspectRTL:ListBox" ) );
        aBrochureRTLOpt.maGroupHint = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutPage" ) );
        // RTL brochure choices
        //      0 : left-to-right
        //      1 : right-to-left
        const sal_Int16 nBRTLChoice = rDefaultPrintData.IsPrintProspectRTL() ? 1 : 0;
        m_aUIProperties[ nIdx++ ].Value = getChoiceControlOpt( rtl::OUString(),
                                                               aBRTLHelpIds,
                                                               rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintProspectRTL" ) ),
                                                               aBRTLChoices,
                                                               nBRTLChoice,
                                                               rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "List" ) ),
                                                               uno::Sequence< sal_Bool >(),
                                                               aBrochureRTLOpt
                                                               );
    }


    DBG_ASSERT( nIdx == nNumProps, "number of added properties is not as expected" );
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

//    bool bRes = getBoolValue( "PrintTablesGraphicsAndDiagrams", sal_True );
//    bRes = getBoolValue( "PrintTables", bRes );
//    return bRes;
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
    uno::Any aVal( getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "RenderDevice" ) ) ) );
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

//////////////////////////////////////////////////////////////////////

