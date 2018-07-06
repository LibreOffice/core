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

#include <documentfontsdialog.hxx>

#include <sfx2/objsh.hxx>

using namespace ::com::sun::star;

VclPtr<SfxTabPage> SfxDocumentFontsPage::Create(TabPageParent pParent, const SfxItemSet* set)
{
    return VclPtr<SfxDocumentFontsPage>::Create(pParent, *set);
}

SfxDocumentFontsPage::SfxDocumentFontsPage(TabPageParent parent, const SfxItemSet& set)
    : SfxTabPage(parent, "sfx/ui/documentfontspage.ui", "DocumentFontsPage", &set)
    , embedFontsCheckbox(m_xBuilder->weld_check_button("embedFonts"))
    , embedUsedFontsCheckbox(m_xBuilder->weld_check_button("embedUsedFonts"))
    , embedLatinScriptFontsCheckbox(m_xBuilder->weld_check_button("embedLatinScriptFonts"))
    , embedAsianScriptFontsCheckbox(m_xBuilder->weld_check_button("embedAsianScriptFonts"))
    , embedComplexScriptFontsCheckbox(m_xBuilder->weld_check_button("embedComplexScriptFonts"))
{
}

SfxDocumentFontsPage::~SfxDocumentFontsPage()
{
}

void SfxDocumentFontsPage::Reset( const SfxItemSet* )
{
    bool bEmbedFonts = false;
    bool bEmbedUsedFonts = false;

    bool bEmbedLatinScriptFonts  = false;
    bool bEmbedAsianScriptFonts  = false;
    bool bEmbedComplexScriptFonts  = false;

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    if (pDocSh)
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xFac( pDocSh->GetModel(), uno::UNO_QUERY_THROW );
            uno::Reference< beans::XPropertySet > xProps( xFac->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY_THROW );

            xProps->getPropertyValue("EmbedFonts") >>= bEmbedFonts;
            xProps->getPropertyValue("EmbedOnlyUsedFonts") >>= bEmbedUsedFonts;
            xProps->getPropertyValue("EmbedLatinScriptFonts") >>= bEmbedLatinScriptFonts;
            xProps->getPropertyValue("EmbedAsianScriptFonts") >>= bEmbedAsianScriptFonts;
            xProps->getPropertyValue("EmbedComplexScriptFonts") >>= bEmbedComplexScriptFonts;
        }
        catch( uno::Exception& )
        {
        }
    }
    embedFontsCheckbox->set_active(bEmbedFonts);
    embedUsedFontsCheckbox->set_active(bEmbedUsedFonts);

    embedLatinScriptFontsCheckbox->set_active(bEmbedLatinScriptFonts);
    embedAsianScriptFontsCheckbox->set_active(bEmbedAsianScriptFonts);
    embedComplexScriptFontsCheckbox->set_active(bEmbedComplexScriptFonts);
}

bool SfxDocumentFontsPage::FillItemSet( SfxItemSet* )
{
    bool bEmbedFonts = embedFontsCheckbox->get_active();
    bool bEmbedUsedFonts = embedUsedFontsCheckbox->get_active();

    bool bEmbedLatinScriptFonts  = embedLatinScriptFontsCheckbox->get_active();
    bool bEmbedAsianScriptFonts  = embedAsianScriptFontsCheckbox->get_active();
    bool bEmbedComplexScriptFonts  = embedComplexScriptFontsCheckbox->get_active();

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    if ( pDocSh )
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xFac( pDocSh->GetModel(), uno::UNO_QUERY_THROW );
            uno::Reference< beans::XPropertySet > xProps( xFac->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY_THROW );
            xProps->setPropertyValue("EmbedFonts", uno::makeAny(bEmbedFonts));
            xProps->setPropertyValue("EmbedOnlyUsedFonts", uno::makeAny(bEmbedUsedFonts));
            xProps->setPropertyValue("EmbedLatinScriptFonts", uno::makeAny(bEmbedLatinScriptFonts));
            xProps->setPropertyValue("EmbedAsianScriptFonts", uno::makeAny(bEmbedAsianScriptFonts));
            xProps->setPropertyValue("EmbedComplexScriptFonts", uno::makeAny(bEmbedComplexScriptFonts));
        }
        catch( uno::Exception& )
        {
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
