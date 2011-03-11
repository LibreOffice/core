/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sfx2.hxx"
#include <vcl/virdev.hxx>
#include <vcl/metric.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/printdlg.hxx>
#include <unotools/printwarningoptions.hxx>
#include <svtools/printoptions.hxx>
#include <vector>

#include <sfx2/printer.hxx>
#include <sfx2/printopt.hxx>
#include "sfxtypes.hxx"
#include <sfx2/prnmon.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/tabdlg.hxx>
#include "sfxresid.hxx"
#include "view.hrc"

#ifdef MSC
// this one is buggy
#define NEW_OBJECTS(Class, nCount) ((Class*) new char[ sizeof(Class) * (nCount) ])
#else
#define NEW_OBJECTS(Class, nCount) (new Class[nCount])
#endif


USHORT SfxFontSizeInfo::pStaticSizes[] =
{
    60,
    80,
    100,
    120,
    140,
    180,
    240,
    360,
    480,
    600,
    720
};

//--------------------------------------------------------------------

SV_DECL_PTRARR_DEL(SfxFontArr_Impl,SfxFont*,10,5)

// struct SfxPrinter_Impl ------------------------------------------------

struct SfxPrinter_Impl
{
    SfxFontArr_Impl*    mpFonts;
    BOOL                mbAll;
    BOOL                mbSelection;
    BOOL                mbFromTo;
    BOOL                mbRange;

    SfxPrinter_Impl() :
        mpFonts     ( NULL ),
        mbAll       ( TRUE ),
        mbSelection ( TRUE ),
        mbFromTo    ( TRUE ),
        mbRange     ( TRUE ) {}
    ~SfxPrinter_Impl() { delete mpFonts; }
};

#define FONTS() pImpl->mpFonts

struct SfxPrintOptDlg_Impl
{
    sal_Bool        mbHelpDisabled;

    SfxPrintOptDlg_Impl() :
        mbHelpDisabled  ( sal_False ) {}
};

//--------------------------------------------------------------------

SfxFontSizeInfo::SfxFontSizeInfo( const SfxFont &rFont,
                                  const OutputDevice &rDevice ) :

    pSizes(0),
    nSizes(0),
    bScalable(TRUE)

{
    if ( 0 == rDevice.GetDevFontCount() )
        bScalable = FALSE;
    else
    {
        OutputDevice &rDev = (OutputDevice&) rDevice;
        Font aFont(rFont.GetName(), Size(0,12));
        aFont.SetFamily(rFont.GetFamily());
        aFont.SetPitch(rFont.GetPitch());
        aFont.SetCharSet(rFont.GetCharSet());

        // Add available sizes to the list, size in tenths of a point
        int nSizeCount = rDev.GetDevFontSizeCount(aFont);
        pSizes = NEW_OBJECTS(Size, nSizeCount);
        const MapMode aOldMapMode = rDev.GetMapMode();
        MapMode aMap(aOldMapMode);
        aMap.SetMapUnit(MAP_POINT);
        const Fraction aTen(1, 10);
        aMap.SetScaleX(aTen);
        aMap.SetScaleY(aTen);
        rDev.SetMapMode(aMap);

        // There are fonts with bitmaps and scalable sizes
        // In this case the fonts arehandled as scalable
        BOOL bFoundScalable = FALSE;
        for ( int i = 0; i < nSizeCount; ++i )
        {
            const Size aSize( rDev.GetDevFontSize(aFont, i) );
            if ( aSize.Height() != 0 )
                pSizes[nSizes++] = aSize;
            else
                bFoundScalable |= TRUE;
        }
        if( !bFoundScalable )
            bScalable = FALSE;
        else
        {
            // Static Font-Sizes are used
            delete [] pSizes;
            nSizes = 0;
        }
        rDev.SetMapMode(aOldMapMode);
    }

    if ( 0 == nSizes )
    {
        nSizes = sizeof(pStaticSizes) / sizeof(USHORT);
        pSizes = NEW_OBJECTS(Size, nSizes);
        for ( USHORT nPos = 0; nPos <nSizes; ++nPos )
           pSizes[nPos] = Size( 0, pStaticSizes[nPos] );
    }
}

//--------------------------------------------------------------------

SfxFontSizeInfo::~SfxFontSizeInfo()
{
    delete [] pSizes;
}

//--------------------------------------------------------------------

BOOL SfxFontSizeInfo::HasSize(const Size &rSize) const
{
    if ( bScalable )
        return TRUE;
    for ( USHORT i = 0; i < nSizes; ++i)
        if ( pSizes[i] == rSize )
            return TRUE;
    return FALSE;
}

//--------------------------------------------------------------------

SfxFont::SfxFont( const FontFamily eFontFamily, const String& aFontName,
                  const FontPitch eFontPitch, const CharSet eFontCharSet ):
    aName( aFontName ),
    eFamily( eFontFamily ),
        ePitch( eFontPitch ),
    eCharSet( eFontCharSet )
{
}

// class SfxPrinter ------------------------------------------------------

SfxPrinter* SfxPrinter::Create( SvStream& rStream, SfxItemSet* pOptions )

/*  [Description]

    Creates a <SfxPrinter> from the stream. Loading is really only a jobsetup.
    If such a printer is not available on the system, then the original is
    marked as the original Job-setup and a comparable printer is selected from
    existing ones.

    The 'pOptions' are taken over in the generated SfxPrinter, the return
    value belongs to the caller.
*/

{
    // Load JobSetup
    JobSetup aFileJobSetup;
    rStream >> aFileJobSetup;

    // Get printers
    SfxPrinter *pPrinter = new SfxPrinter( pOptions, aFileJobSetup );
    return pPrinter;
}

//--------------------------------------------------------------------

SvStream& SfxPrinter::Store( SvStream& rStream ) const

/*  [Description]

    Saves the used JobSetup of <SfxPrinter>s.
*/

{
    return ( rStream << GetJobSetup() );
}

//--------------------------------------------------------------------

SfxPrinter::SfxPrinter( SfxItemSet* pTheOptions ) :

/*  [Description]

    This constructor creates a default printer.
*/

    pOptions( pTheOptions ),
    bKnown(sal_True)

{
    pImpl = new SfxPrinter_Impl;
}

//--------------------------------------------------------------------

SfxPrinter::SfxPrinter( SfxItemSet* pTheOptions,
                        const JobSetup& rTheOrigJobSetup ) :

    Printer         ( rTheOrigJobSetup.GetPrinterName() ),
    pOptions        ( pTheOptions )

{
    pImpl = new SfxPrinter_Impl;
    bKnown = GetName() == rTheOrigJobSetup.GetPrinterName();

    if ( bKnown )
        SetJobSetup( rTheOrigJobSetup );
}

//--------------------------------------------------------------------

SfxPrinter::SfxPrinter( SfxItemSet* pTheOptions,
                        const String& rPrinterName ) :

    Printer         ( rPrinterName ),
    pOptions        ( pTheOptions ),
    bKnown          ( GetName() == rPrinterName )

{
    pImpl = new SfxPrinter_Impl;
}

//--------------------------------------------------------------------

SfxPrinter::SfxPrinter( const SfxPrinter& rPrinter ) :

    Printer ( rPrinter.GetName() ),
    pOptions( rPrinter.GetOptions().Clone() ),
    bKnown  ( rPrinter.IsKnown() )
{
    SetJobSetup( rPrinter.GetJobSetup() );
    SetPrinterProps( &rPrinter );
    SetMapMode( rPrinter.GetMapMode() );

    pImpl = new SfxPrinter_Impl;
    pImpl->mbAll = rPrinter.pImpl->mbAll;
    pImpl->mbSelection = rPrinter.pImpl->mbSelection;
    pImpl->mbFromTo = rPrinter.pImpl->mbFromTo;
    pImpl->mbRange = rPrinter.pImpl->mbRange;
}

//--------------------------------------------------------------------

SfxPrinter* SfxPrinter::Clone() const
{
    if ( IsDefPrinter() )
    {
        SfxPrinter *pNewPrinter;
        pNewPrinter = new SfxPrinter( GetOptions().Clone() );
        pNewPrinter->SetJobSetup( GetJobSetup() );
        pNewPrinter->SetPrinterProps( this );
        pNewPrinter->SetMapMode( GetMapMode() );
        pNewPrinter->pImpl->mbAll = pImpl->mbAll;
        pNewPrinter->pImpl->mbSelection =pImpl->mbSelection;
        pNewPrinter->pImpl->mbFromTo = pImpl->mbFromTo;
        pNewPrinter->pImpl->mbRange =pImpl->mbRange;
        return pNewPrinter;
    }
    else
        return new SfxPrinter( *this );
}

//--------------------------------------------------------------------

SfxPrinter::~SfxPrinter()
{
    delete pOptions;
    delete pImpl;
}

//--------------------------------------------------------------------

void SfxPrinter::SetOptions( const SfxItemSet &rNewOptions )
{
    pOptions->Set(rNewOptions);
}

//--------------------------------------------------------------------

void SfxPrinter::EnableRange( USHORT nRange )
{
    PrintDialogRange eRange = (PrintDialogRange)nRange;

    if ( eRange == PRINTDIALOG_ALL )
        pImpl->mbAll = TRUE;
    else if ( eRange == PRINTDIALOG_SELECTION )
        pImpl->mbSelection = TRUE;
    else if ( eRange == PRINTDIALOG_FROMTO )
        pImpl->mbFromTo = TRUE;
    else if ( eRange == PRINTDIALOG_RANGE )
        pImpl->mbRange = TRUE;
}

//--------------------------------------------------------------------

void SfxPrinter::DisableRange( USHORT nRange )
{
    PrintDialogRange eRange = (PrintDialogRange)nRange;

    if ( eRange == PRINTDIALOG_ALL )
        pImpl->mbAll = FALSE;
    else if ( eRange == PRINTDIALOG_SELECTION )
        pImpl->mbSelection = FALSE;
    else if ( eRange == PRINTDIALOG_FROMTO )
        pImpl->mbFromTo = FALSE;
    else if ( eRange == PRINTDIALOG_RANGE )
        pImpl->mbRange = FALSE;
}

//--------------------------------------------------------------------

BOOL SfxPrinter::IsRangeEnabled( USHORT nRange ) const
{
    PrintDialogRange eRange = (PrintDialogRange)nRange;
    BOOL bRet = FALSE;

    if ( eRange == PRINTDIALOG_ALL )
        bRet = pImpl->mbAll;
    else if ( eRange == PRINTDIALOG_SELECTION )
        bRet = pImpl->mbSelection;
    else if ( eRange == PRINTDIALOG_FROMTO )
        bRet = pImpl->mbFromTo;
    else if ( eRange == PRINTDIALOG_RANGE )
        bRet = pImpl->mbRange;

    return bRet;
}

//--------------------------------------------------------------------

SV_IMPL_PTRARR(SfxFontArr_Impl,SfxFont*)

//--------------------------------------------------------------------

const SfxFont* SfxFindFont_Impl( const SfxFontArr_Impl& rArr,
                                 const String& rName )
{
    const USHORT nCount = rArr.Count();
    for ( USHORT i = 0; i < nCount; ++i )
    {
        const SfxFont *pFont = rArr[i];
        if ( pFont->GetName() == rName )
            return pFont;
    }
    return NULL;
}

//--------------------------------------------------------------------

void SfxPrinter::UpdateFonts_Impl()
{
    VirtualDevice *pVirDev = 0;
    const OutputDevice *pOut = this;

    // If no printer was found, a temporay device is created
    // for queries about fonts
    if( !IsValid() )
        pOut = pVirDev = new VirtualDevice;

    int nCount = pOut->GetDevFontCount();
    FONTS() =  new SfxFontArr_Impl((BYTE)nCount);

    std::vector< Font > aNonRegularFonts;
    for(int i = 0;i < nCount;++i)
    {
        Font aFont(pOut->GetDevFont(i));
        if ( (aFont.GetItalic() != ITALIC_NONE) ||
             (aFont.GetWeight() != WEIGHT_MEDIUM) )
        {
            // First: Don't add non-regular fonts. The font name is not unique so we have
            // to filter the device font list.
            aNonRegularFonts.push_back( aFont );
        }
        else if ( FONTS()->Count() == 0 ||
             (*FONTS())[FONTS()->Count()-1]->GetName() != aFont.GetName() )
        {
            DBG_ASSERT(0 == SfxFindFont_Impl(*FONTS(), aFont.GetName()), "Double Fonts from SV-Device!");
            SfxFont* pTmp = new SfxFont( aFont.GetFamily(), aFont.GetName(),
                                         aFont.GetPitch(), aFont.GetCharSet() );
            FONTS()->C40_INSERT(SfxFont, pTmp, FONTS()->Count());
        }
    }
    delete pVirDev;

    // Try to add all non-regular fonts. It could be that there was no regular font
    // with the same name added.
    std::vector< Font >::const_iterator pIter;
    for ( pIter = aNonRegularFonts.begin(); pIter != aNonRegularFonts.end(); ++pIter )
    {
        if ( SfxFindFont_Impl( *FONTS(), pIter->GetName() ) == 0 )
        {
            SfxFont* pTmp = new SfxFont( pIter->GetFamily(), pIter->GetName(),
                                         pIter->GetPitch(), pIter->GetCharSet() );
            FONTS()->C40_INSERT( SfxFont, pTmp, FONTS()->Count() );
        }
    }
}

//--------------------------------------------------------------------

USHORT SfxPrinter::GetFontCount()
{
    if ( !FONTS() )
        UpdateFonts_Impl();
    return FONTS()->Count();
}

//--------------------------------------------------------------------

const SfxFont* SfxPrinter::GetFont( USHORT nNo ) const
{
    DBG_ASSERT( FONTS(), "First, please check GetFontCount()!" );
    return (*FONTS())[ nNo ];
}

//--------------------------------------------------------------------

const SfxFont* SfxPrinter::GetFontByName( const String &rFontName )
{
    if ( !FONTS() )
        UpdateFonts_Impl();
    return SfxFindFont_Impl(*FONTS(), rFontName);
}

//--------------------------------------------------------------------

BOOL SfxPrinter::InitJob( Window* pUIParent, BOOL bAskAboutTransparentObjects )
{
    const SvtPrinterOptions     aPrinterOpt;
    const SvtPrintFileOptions   aPrintFileOpt;
    const SvtBasePrintOptions*  pPrinterOpt = &aPrinterOpt;
    const SvtBasePrintOptions*  pPrintFileOpt = &aPrintFileOpt;
    PrinterOptions              aNewPrinterOptions;
    BOOL                        bRet = TRUE;

    ( ( IsPrintFileEnabled() && GetPrintFile().Len() ) ? pPrintFileOpt : pPrinterOpt )->GetPrinterOptions( aNewPrinterOptions );

    if( bAskAboutTransparentObjects && !aNewPrinterOptions.IsReduceTransparency() )
    {
        if ( !Application::IsHeadlessModeEnabled() )
        {
            SvtPrintWarningOptions aWarnOpt;

            if( aWarnOpt.IsTransparency() )
            {
                TransparencyPrintWarningBox aWarnBox( pUIParent );
                const USHORT                nRet = aWarnBox.Execute();

                if( nRet == RET_CANCEL )
                    bRet = FALSE;
                else
                {
                    aNewPrinterOptions.SetReduceTransparency( nRet != RET_NO );
                    aWarnOpt.SetTransparency( !aWarnBox.IsNoWarningChecked() );
                }
            }
        }
    }

    if( bRet )
        SetPrinterOptions( aNewPrinterOptions );

    return bRet;
}

//--------------------------------------------------------------------

SfxPrintOptionsDialog::SfxPrintOptionsDialog( Window *pParent,
                                              SfxViewShell *pViewShell,
                                              const SfxItemSet *pSet ) :

    ModalDialog( pParent, WinBits( WB_STDMODAL | WB_3DLOOK ) ),

    aOkBtn      ( this ),
    aCancelBtn  ( this ),
    aHelpBtn    ( this ),
    pDlgImpl    ( new SfxPrintOptDlg_Impl ),
    pViewSh     ( pViewShell ),
    pOptions    ( pSet->Clone() ),
    pPage       ( NULL )

{
    SetText( SfxResId( STR_PRINT_OPTIONS_TITLE ) );

    // Insert TabPage
    pPage = pViewSh->CreatePrintOptionsPage( this, *pOptions );
    DBG_ASSERT( pPage, "CreatePrintOptions != SFX_VIEW_HAS_PRINTOPTIONS" );
    if( pPage )
    {
        pPage->Reset( *pOptions );
        SetHelpId( pPage->GetHelpId() );
        pPage->Show();
    }

    // Set dialog size
    Size a6Sz = LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
    Size aBtnSz = LogicToPixel( Size( 50, 14 ), MAP_APPFONT );
    Size aOutSz( pPage ? pPage->GetSizePixel() : Size() );
    aOutSz.Height() += 6;
    long nWidth = aBtnSz.Width();
    nWidth += a6Sz.Width();
    aOutSz.Width() += nWidth;
    if ( aOutSz.Height() < 90 )
        // at least the height of the 3 buttons
        aOutSz.Height() = 90;
    SetOutputSizePixel( aOutSz );

    // set position and size of the buttons
    Point aBtnPos( aOutSz.Width() - aBtnSz.Width() - a6Sz.Width(), a6Sz.Height() );
    aOkBtn.SetPosSizePixel( aBtnPos, aBtnSz );
    aBtnPos.Y() += aBtnSz.Height() + ( a6Sz.Height() / 2 );
    aCancelBtn.SetPosSizePixel( aBtnPos, aBtnSz );
    aBtnPos.Y() += aBtnSz.Height() + a6Sz.Height();
    aHelpBtn.SetPosSizePixel( aBtnPos, aBtnSz );

    aCancelBtn.Show();
    aOkBtn.Show();
    aHelpBtn.Show();
}

//--------------------------------------------------------------------

SfxPrintOptionsDialog::~SfxPrintOptionsDialog()
{
    delete pDlgImpl;
    delete pPage;
    delete pOptions;
}

//--------------------------------------------------------------------

short SfxPrintOptionsDialog::Execute()
{
    if( ! pPage )
        return RET_CANCEL;

    short nRet = ModalDialog::Execute();
    if ( nRet == RET_OK )
        pPage->FillItemSet( *pOptions );
    else
        pPage->Reset( *pOptions );
    return nRet;
}

//--------------------------------------------------------------------

long SfxPrintOptionsDialog::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        if ( rNEvt.GetKeyEvent()->GetKeyCode().GetCode() == KEY_F1 && pDlgImpl->mbHelpDisabled )
            return 1; // help disabled -> <F1> does nothing
    }

    return ModalDialog::Notify( rNEvt );
}

//--------------------------------------------------------------------

void SfxPrintOptionsDialog::DisableHelp()
{
    pDlgImpl->mbHelpDisabled = sal_True;

    aHelpBtn.Disable();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
