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

#include <sal/config.h>

#include <sfx2/app.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/objface.hxx>

#include "unomodel.hxx"

#include "basdoc.hxx"
#define ShellClass_basctl_DocShell
#include <basslots.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/itemset.hxx>
#include <svx/svxids.hrc>
#include <tools/globname.hxx>
#include <tools/debug.hxx>

namespace basctl
{


SFX_IMPL_OBJECTFACTORY( DocShell, SvGlobalName(), "sbasic" )

SFX_IMPL_SUPERCLASS_INTERFACE(basctl_DocShell, SfxObjectShell)

void basctl_DocShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterStatusBar(StatusBarId::BasicIdeStatusBar);
}

DocShell::DocShell()
    :SfxObjectShell( SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS | SfxModelFlags::DISABLE_DOCUMENT_RECOVERY )
{
    SetPool( &SfxGetpApp()->GetPool() );
    SetBaseModel( new SIDEModel(this) );
}

DocShell::~DocShell()
{
    pPrinter.disposeAndClear();
}

SfxPrinter* DocShell::GetPrinter( bool bCreate )
{
    if ( !pPrinter && bCreate )
        pPrinter.disposeAndReset(VclPtr<SfxPrinter>::Create(std::make_unique<SfxItemSet>(
            GetPool(), svl::Items<SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN>{}
        )));

    return pPrinter.get();
}

void DocShell::SetPrinter( SfxPrinter* pPr )
{
    if (pPr != pPrinter.get())
    {
        pPrinter.disposeAndReset(pPr);
    }
}

void DocShell::FillClass( SvGlobalName*, SotClipboardFormatId*, OUString*, OUString*, OUString*, sal_Int32, bool bTemplate) const
{
    DBG_ASSERT( !bTemplate, "No template for Basic" );
}

void DocShell::Draw( OutputDevice *, const JobSetup &, sal_uInt16 )
{}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
