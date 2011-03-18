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
#include <unotools/printwarningoptions.hxx>
#include <svtools/printoptions.hxx>
#include <vector>

#include <sfx2/printer.hxx>
#include <sfx2/printopt.hxx>
#include "sfxtypes.hxx"
#include <sfx2/prnmon.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/tabdlg.hxx>
#include "sfx2/sfxresid.hxx"
#include "view.hrc"

// struct SfxPrinter_Impl ------------------------------------------------

struct SfxPrinter_Impl
{
    sal_Bool            mbAll;
    sal_Bool            mbSelection;
    sal_Bool            mbFromTo;
    sal_Bool            mbRange;

    SfxPrinter_Impl() :
        mbAll       ( sal_True ),
        mbSelection ( sal_True ),
        mbFromTo    ( sal_True ),
        mbRange     ( sal_True ) {}
    ~SfxPrinter_Impl() {}
};

struct SfxPrintOptDlg_Impl
{
    sal_Bool        mbHelpDisabled;

    SfxPrintOptDlg_Impl() :
        mbHelpDisabled  ( sal_False ) {}
};

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
