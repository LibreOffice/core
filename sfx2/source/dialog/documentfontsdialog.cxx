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

#include <sfx2/objsh.hxx>

using namespace ::com::sun::star;

SfxTabPage* SfxDocumentFontsPage::Create( Window* parent, const SfxItemSet& set )
{
    return new SfxDocumentFontsPage( parent, set );
}

SfxDocumentFontsPage::SfxDocumentFontsPage( Window* parent, const SfxItemSet& set )
    : SfxTabPage( parent, "DocumentFontsPage", "sfx/ui/documentfontspage.ui", set )
{
    get( embedFontsCheckbox, "embedFonts" );
}

void SfxDocumentFontsPage::Reset( const SfxItemSet& )
{
    bool bVal = false;
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    if ( pDocSh )
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xFac( pDocSh->GetModel(), uno::UNO_QUERY_THROW );
            uno::Reference< beans::XPropertySet > xProps( xFac->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY_THROW );
            xProps->getPropertyValue("EmbedFonts") >>= bVal;
        }
        catch( uno::Exception& )
        {
        }
    }
    embedFontsCheckbox->Check( bVal );
}

sal_Bool SfxDocumentFontsPage::FillItemSet( SfxItemSet& )
{
    bool bVal = embedFontsCheckbox->IsChecked();
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    if ( pDocSh )
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xFac( pDocSh->GetModel(), uno::UNO_QUERY_THROW );
            uno::Reference< beans::XPropertySet > xProps( xFac->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY_THROW );
            xProps->setPropertyValue("EmbedFonts", uno::makeAny( bVal ) );
        }
        catch( uno::Exception& )
        {
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
