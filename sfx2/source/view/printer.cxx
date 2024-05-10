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

#include <tools/debug.hxx>

#include <utility>

#include <sfx2/printer.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/tabdlg.hxx>
#include "prnmon.hxx"

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
    bKnown( true )
{
    assert(pOptions);
}


SfxPrinter::SfxPrinter( std::unique_ptr<SfxItemSet>&& pTheOptions,
                        const JobSetup& rTheOrigJobSetup ) :
    Printer( rTheOrigJobSetup.GetPrinterName() ),
    pOptions( std::move(pTheOptions) )
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
    bKnown( GetName() == rPrinterName )
{
    assert(pOptions);
}


SfxPrinter::SfxPrinter( const SfxPrinter& rPrinter ) :
    VclReferenceBase(),
    Printer( rPrinter.GetName() ),
    pOptions( rPrinter.GetOptions().Clone() ),
    bKnown( rPrinter.IsKnown() )
{
    assert(pOptions);
    SetJobSetup( rPrinter.GetJobSetup() );
    SetPrinterProps( &rPrinter );
    SetMapMode( rPrinter.GetMapMode() );
}


VclPtr<SfxPrinter> SfxPrinter::Clone() const
{
    if ( IsDefPrinter() )
    {
        VclPtr<SfxPrinter> pNewPrinter = VclPtr<SfxPrinter>::Create( GetOptions().Clone() );
        pNewPrinter->SetJobSetup( GetJobSetup() );
        pNewPrinter->SetPrinterProps( this );
        pNewPrinter->SetMapMode( GetMapMode() );
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
    Printer::dispose();
}


void SfxPrinter::SetOptions( const SfxItemSet &rNewOptions )
{
    pOptions->Set(rNewOptions);
}


SfxPrintOptionsDialog::SfxPrintOptionsDialog(weld::Window *pParent,
                                             SfxViewShell *pViewShell,
                                             const SfxItemSet *pSet)
    : GenericDialogController(pParent, u"sfx/ui/printeroptionsdialog.ui"_ustr, u"PrinterOptionsDialog"_ustr)
    , pOptions(pSet->Clone())
    , m_xHelpBtn(m_xBuilder->weld_widget(u"help"_ustr))
    , m_xContainer(m_xDialog->weld_content_area())
    , m_xPage(pViewShell->CreatePrintOptionsPage(m_xContainer.get(), this, *pOptions)) // Insert TabPage
{
    DBG_ASSERT( m_xPage, "CreatePrintOptions != SFX_VIEW_HAS_PRINTOPTIONS" );
    if (m_xPage)
    {
        m_xPage->Reset( pOptions.get() );
        m_xDialog->set_help_id(m_xPage->GetHelpId());
    }
}

SfxPrintOptionsDialog::~SfxPrintOptionsDialog()
{
}

short SfxPrintOptionsDialog::run()
{
    if (!m_xPage)
        return RET_CANCEL;

    short nRet = GenericDialogController::run();

    if (nRet == RET_OK)
        m_xPage->FillItemSet( pOptions.get() );
    else
        m_xPage->Reset( pOptions.get() );
    return nRet;
}

void SfxPrintOptionsDialog::DisableHelp()
{
    m_xHelpBtn->set_sensitive(false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
