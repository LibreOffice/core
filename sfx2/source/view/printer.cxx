/*************************************************************************
 *
 *  $RCSfile: printer.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif
#ifndef _SV_METRIC_HXX //autogen
#include <vcl/metric.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_PRINTDLG_HXX_ //autogen
#include <svtools/printdlg.hxx>
#endif
#pragma hdrstop

#include "printer.hxx"
#include "sfxtypes.hxx"
#include "prnmon.hxx"
#include "viewsh.hxx"
#include "tabdlg.hxx"
#include "sfxresid.hxx"
#include "view.hrc"

#ifdef MSC
// der ist buggy
#define NEW_OBJECTS(Class, nCount) ((Class*) new char[ sizeof(Class) * (nCount) ])
#else
#define NEW_OBJECTS(Class, nCount) (new Class[nCount])
#endif


USHORT SfxFontSizeInfo::pStaticSizes[] =
{
#ifdef MAC
    90,
#else
    60,
    80,
#endif
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

        // verfuegbare Groessen in die Liste eintragen, Groesse in 10tel Punkt
        USHORT nSizeCount = rDev.GetDevFontSizeCount(aFont);
        pSizes = NEW_OBJECTS(Size, nSizeCount);
        const MapMode aOldMapMode = rDev.GetMapMode();
        MapMode aMap(aOldMapMode);
        aMap.SetMapUnit(MAP_POINT);
        const Fraction aTen(1, 10);
        aMap.SetScaleX(aTen);
        aMap.SetScaleY(aTen);
        rDev.SetMapMode(aMap);

        // Es gibt Fonts mit Bitmaps und skalierbaren Groessen
        // In diesem Fall wird der Fonts als skalierbar behandelt.
        BOOL bFoundScalable = FALSE;
        for ( USHORT i = 0; i < nSizeCount; ++i )
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
            // statische Font-Sizes verwenden
            __DELETE(nSizeCount) pSizes;
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
    __DELETE(nSizes) pSizes;
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

/*  [Beschreibung]

    Erzeugt einen <SfxPrinter> aus dem Stream. Geladen wird genaugenommen
    nur ein JobSetup. Falls ein solcher Drucker auf dem System nicht
    verf"augbar ist, wird das Original als Orig-JobSetup gemerkt und
    ein "anhlicher exisitierender Drucker genommen.

    Die 'pOptions' werden in den erzeugten SfxPrinter "ubernommen,
    der Returnwert geh"ort dem Caller.
*/

{
    // JobSetup laden
    JobSetup aFileJobSetup;
    rStream >> aFileJobSetup;

    // Drucker erzeugen
    SfxPrinter *pPrinter = new SfxPrinter( pOptions, aFileJobSetup );
    return pPrinter;
}

//--------------------------------------------------------------------

SvStream& SfxPrinter::Store( SvStream& rStream ) const

/*  [Beschreibung]

    Speichert das verwendete JobSetup des <SfxPrinter>s.
*/

{
    return ( rStream << GetJobSetup() );
}

//--------------------------------------------------------------------

SfxPrinter::SfxPrinter( SfxItemSet* pTheOptions ) :

/*  [Beschreibung]

    Dieser Ctor erzeugt einen Standard-Drucker.
*/

    pOptions( pTheOptions )

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

        // falls kein Drucker gefunden werden konnte, ein
        // temp. Device erzeugen fuer das Erfragen der Fonts
    if( !IsValid() )
        pOut = pVirDev = new VirtualDevice;

    const USHORT nCount = pOut->GetDevFontCount();
    FONTS() =  new SfxFontArr_Impl((BYTE)nCount);
    for(USHORT i = 0;i < nCount;++i)
    {
        Font aFont(pOut->GetDevFont(i));
        if ( FONTS()->Count() == 0 ||
             (*FONTS())[FONTS()->Count()-1]->GetName() != aFont.GetName() )
        {
            DBG_ASSERT(0 == SfxFindFont_Impl(*FONTS(), aFont.GetName()), "Doppelte Fonts vom SV-Device!");
            SfxFont* pTmp = new SfxFont( aFont.GetFamily(), aFont.GetName(),
                                         aFont.GetPitch(), aFont.GetCharSet() );
            FONTS()->C40_INSERT(SfxFont, pTmp, FONTS()->Count());
        }
    }
    delete pVirDev;
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
    DBG_ASSERT( FONTS(), "bitte erst GetFontCount() abfragen!" );
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

SfxPrintOptionsDialog::SfxPrintOptionsDialog( Window *pParent,
                                              SfxViewShell *pViewShell,
                                              const SfxItemSet *pSet ):
    ModalDialog( pParent, WinBits( WB_STDMODAL | WB_3DLOOK ) ),
    aOkBtn( this ),
    aCancelBtn( this ),
    pViewSh( pViewShell ),
    pOptions( pSet->Clone() ),
    pPage( 0 )
{
    pHelpBtn = new HelpButton(this);
    SetText( SfxResId( STR_PRINT_OPTIONS_TITLE ) );

    // TabPage einh"angen
    pPage = pViewSh->CreatePrintOptionsPage( this, *pOptions );
    DBG_ASSERT( pPage, "CreatePrintOptions != SFX_VIEW_HAS_PRINTOPTIONS" );
    pPage->Reset( *pOptions );
    SetHelpId( pPage->GetHelpId() );
    pPage->Show();

    // Dialoggr"o\se bestimmen
    Size aOutSz( pPage->GetSizePixel() );
    aOutSz.Height() += 6;
    aOutSz.Width() += 108;
    if ( aOutSz.Height() < 90 )
        // mindestens die H"ohe der 3 Buttons
        aOutSz.Height() = 90;
    SetOutputSizePixel( aOutSz );

    // Buttons positionieren
    aOkBtn.SetPosSizePixel( Point( aOutSz.Width()-102, 6 ), Size( 96, 24 ) );
#ifdef MAC
    aCancelBtn.SetPosSizePixel( Point( aOutSz.Width()-102, 37 ), Size( 96, 24 ) );
    pHelpBtn->SetPosSizePixel( Point( aOutSz.Width()-102, 64 ), Size( 96, 24 ) );
#else
    aCancelBtn.SetPosSizePixel( Point( aOutSz.Width()-102, 33 ), Size( 96, 24 ) );
    pHelpBtn->SetPosSizePixel( Point( aOutSz.Width()-102, 60 ), Size( 96, 24 ) );
#endif
    aCancelBtn.Show();
    aOkBtn.Show();
    pHelpBtn->Show();
}

//--------------------------------------------------------------------

SfxPrintOptionsDialog::~SfxPrintOptionsDialog()
{
    delete pHelpBtn;
    delete pPage;
    delete pOptions;
}

//--------------------------------------------------------------------

short SfxPrintOptionsDialog::Execute()
{
    short nRet = ModalDialog::Execute();
    if ( nRet == RET_OK )
        pPage->FillItemSet( *pOptions );
    else
        pPage->Reset( *pOptions );
    return nRet;
}


