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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

using namespace ::com::sun::star;

std::unique_ptr<SfxTabPage> SfxDocumentFontsPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* set)
{
    return std::make_unique<SfxDocumentFontsPage>(pPage, pController, *set);
}

SfxDocumentFontsPage::SfxDocumentFontsPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& set)
    : SfxTabPage(pPage, pController, u"sfx/ui/documentfontspage.ui"_ustr, u"DocumentFontsPage"_ustr, &set)
    , embedFontsCheckbox(m_xBuilder->weld_check_button(u"embedFonts"_ustr))
    , embedUsedFontsCheckbox(m_xBuilder->weld_check_button(u"embedUsedFonts"_ustr))
    , embedLatinScriptFontsCheckbox(m_xBuilder->weld_check_button(u"embedLatinScriptFonts"_ustr))
    , embedAsianScriptFontsCheckbox(m_xBuilder->weld_check_button(u"embedAsianScriptFonts"_ustr))
    , embedComplexScriptFontsCheckbox(m_xBuilder->weld_check_button(u"embedComplexScriptFonts"_ustr))
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
            uno::Reference< beans::XPropertySet > xProps( xFac->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW );

            xProps->getPropertyValue(u"EmbedFonts"_ustr) >>= bEmbedFonts;
            xProps->getPropertyValue(u"EmbedOnlyUsedFonts"_ustr) >>= bEmbedUsedFonts;
            xProps->getPropertyValue(u"EmbedLatinScriptFonts"_ustr) >>= bEmbedLatinScriptFonts;
            xProps->getPropertyValue(u"EmbedAsianScriptFonts"_ustr) >>= bEmbedAsianScriptFonts;
            xProps->getPropertyValue(u"EmbedComplexScriptFonts"_ustr) >>= bEmbedComplexScriptFonts;
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
            uno::Reference< beans::XPropertySet > xProps( xFac->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW );
            xProps->setPropertyValue(u"EmbedFonts"_ustr, uno::Any(bEmbedFonts));
            xProps->setPropertyValue(u"EmbedOnlyUsedFonts"_ustr, uno::Any(bEmbedUsedFonts));
            xProps->setPropertyValue(u"EmbedLatinScriptFonts"_ustr, uno::Any(bEmbedLatinScriptFonts));
            xProps->setPropertyValue(u"EmbedAsianScriptFonts"_ustr, uno::Any(bEmbedAsianScriptFonts));
            xProps->setPropertyValue(u"EmbedComplexScriptFonts"_ustr, uno::Any(bEmbedComplexScriptFonts));
        }
        catch( uno::Exception& )
        {
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
