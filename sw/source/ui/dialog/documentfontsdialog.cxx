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

#include "documentfontsdialog.hxx"

#include <doc.hxx>
#include <docsh.hxx>

SfxTabPage* SwDocumentFontsPage::Create( Window* parent, const SfxItemSet& set )
{
    return new SwDocumentFontsPage( parent, set );
}

SwDocumentFontsPage::SwDocumentFontsPage( Window* parent, const SfxItemSet& set )
    : SfxTabPage( parent, "DocumentFontsPage", "modules/swriter/ui/documentfontspage.ui", set )
{
    get( embedFontsCheckbox, "embedFonts" );
}

void SwDocumentFontsPage::Reset( const SfxItemSet& )
{
    // I take it the SfxItemSet should be "officially" used, but the dialog
    // is modal (and SwDocStatPage does it too), so it should be safe, and
    // also simpler, to short-circuit directly to the setting.
    SwDocShell* docShell = static_cast< SwDocShell* >( SfxObjectShell::Current());
    SwDoc* doc = docShell->GetDoc();
    embedFontsCheckbox->Check( doc->get( IDocumentSettingAccess::EMBED_FONTS ));
}

sal_Bool SwDocumentFontsPage::FillItemSet( SfxItemSet& )
{
    SwDocShell* docShell = static_cast< SwDocShell* >( SfxObjectShell::Current());
    SwDoc* doc = docShell->GetDoc();
    doc->set( IDocumentSettingAccess::EMBED_FONTS, embedFontsCheckbox->IsChecked());
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
