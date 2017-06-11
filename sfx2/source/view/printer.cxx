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

#include <vcl/virdev.hxx>
#include <vcl/metric.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/printwarningoptions.hxx>
#include <svtools/printoptions.hxx>

#include <utility>
#include <vector>

#include <sfx2/printer.hxx>
#include <sfx2/printopt.hxx>
#include "sfxtypes.hxx"
#include <sfx2/prnmon.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/sfxresid.hxx>

// struct SfxPrinter_Impl ------------------------------------------------

struct SfxPrinter_Impl
{
    bool            mbAll;
    bool            mbSelection;
    bool            mbFromTo;
    bool            mbRange;

    SfxPrinter_Impl() :
        mbAll       ( true ),
        mbSelection ( true ),
        mbFromTo    ( true ),
        mbRange     ( true ) {}
};

struct SfxPrintOptDlg_Impl
{
    bool        mbHelpDisabled;

    SfxPrintOptDlg_Impl() :
        mbHelpDisabled  ( false ) {}
};

// class SfxPrinter ------------------------------------------------------

VclPtr<SfxPrinter> SfxPrinter::Create( SvStream& rStream, std::unique_ptr<SfxItemSet>&& pOptions )

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
    ReadJobSetup( rStream, aFileJobSetup );

    // Get printers
    VclPtr<SfxPrinter> pPrinter = VclPtr<SfxPrinter>::Create( std::move(pOptions), aFileJobSetup );
    return pPrinter;
}


void SfxPrinter::Store( SvStream& rStream ) const

/*  [Description]

    Saves the used JobSetup of <SfxPrinter>s.
*/

{
    WriteJobSetup( rStream, GetJobSetup() );
}


SfxPrinter::SfxPrinter( std::unique_ptr<SfxItemSet>&& pTheOptions ) :

/*  [Description]

    This constructor creates a default printer.
*/
    pOptions( std::move(pTheOptions) ),
    pImpl( new SfxPrinter_Impl ),
    bKnown( true )
{
    assert(pOptions);
}


SfxPrinter::SfxPrinter( std::unique_ptr<SfxItemSet>&& pTheOptions,
                        const JobSetup& rTheOrigJobSetup ) :
    Printer( rTheOrigJobSetup.GetPrinterName() ),
    pOptions( std::move(pTheOptions) ),
    pImpl( new SfxPrinter_Impl )
{
    assert(pOptions);
    bKnown = GetName() == rTheOrigJobSetup.GetPrinterName();

    if ( bKnown )
        SetJobSetup( rTheOrigJobSetup );
}


SfxPrinter::SfxPrinter( std::unique_ptr<SfxItemSet>&& pTheOptions,
                        const OUString& rPrinterName ) :
    Printer( rPrinterName ),
    pOptions( std::move(pTheOptions) ),
    pImpl( new SfxPrinter_Impl ),
    bKnown( GetName() == rPrinterName )
{
    assert(pOptions);
}


SfxPrinter::SfxPrinter( const SfxPrinter& rPrinter ) :
    VclReferenceBase(),
    Printer( rPrinter.GetName() ),
    pOptions( rPrinter.GetOptions().Clone() ),
    pImpl( new SfxPrinter_Impl ),
    bKnown( rPrinter.IsKnown() )
{
    assert(pOptions);
    SetJobSetup( rPrinter.GetJobSetup() );
    SetPrinterProps( &rPrinter );
    SetMapMode( rPrinter.GetMapMode() );

    pImpl->mbAll = rPrinter.pImpl->mbAll;
    pImpl->mbSelection = rPrinter.pImpl->mbSelection;
    pImpl->mbFromTo = rPrinter.pImpl->mbFromTo;
    pImpl->mbRange = rPrinter.pImpl->mbRange;
}


VclPtr<SfxPrinter> SfxPrinter::Clone() const
{
    if ( IsDefPrinter() )
    {
        VclPtr<SfxPrinter> pNewPrinter = VclPtr<SfxPrinter>::Create( std::unique_ptr<SfxItemSet>(GetOptions().Clone()) );
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
        return VclPtr<SfxPrinter>::Create( *this );
}


SfxPrinter::~SfxPrinter()
{
    disposeOnce();
}

void SfxPrinter::dispose()
{
    pOptions.reset();
    pImpl.reset();
    Printer::dispose();
}


void SfxPrinter::SetOptions( const SfxItemSet &rNewOptions )
{
    pOptions->Set(rNewOptions);
}


SfxPrintOptionsDialog::SfxPrintOptionsDialog(vcl::Window *pParent,
                                              SfxViewShell *pViewShell,
                                              const SfxItemSet *pSet)

    : ModalDialog(pParent, "PrinterOptionsDialog",
        "sfx/ui/printeroptionsdialog.ui")
    , pDlgImpl(new SfxPrintOptDlg_Impl)
    , pViewSh(pViewShell)
    , pOptions(pSet->Clone())
{
    VclContainer *pVBox = get_content_area();

    // Insert TabPage
    pPage.reset(pViewSh->CreatePrintOptionsPage(pVBox, *pOptions));
    DBG_ASSERT( pPage, "CreatePrintOptions != SFX_VIEW_HAS_PRINTOPTIONS" );
    if( pPage )
    {
        pPage->Reset( pOptions );
        SetHelpId( pPage->GetHelpId() );
        pPage->Show();
    }
}


SfxPrintOptionsDialog::~SfxPrintOptionsDialog()
{
    disposeOnce();
}

void SfxPrintOptionsDialog::dispose()
{
    pDlgImpl.reset();
    pPage.disposeAndClear();
    delete pOptions;
    ModalDialog::dispose();
}


short SfxPrintOptionsDialog::Execute()
{
    if( ! pPage )
        return RET_CANCEL;

    short nRet = ModalDialog::Execute();
    if ( nRet == RET_OK )
        pPage->FillItemSet( pOptions );
    else
        pPage->Reset( pOptions );
    return nRet;
}


bool SfxPrintOptionsDialog::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        if ( rNEvt.GetKeyEvent()->GetKeyCode().GetCode() == KEY_F1 && pDlgImpl->mbHelpDisabled )
            return true; // help disabled -> <F1> does nothing
    }

    return ModalDialog::EventNotify( rNEvt );
}


void SfxPrintOptionsDialog::DisableHelp()
{
    pDlgImpl->mbHelpDisabled = true;

    get<HelpButton>("help")->Disable();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
