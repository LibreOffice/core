/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_folders.h>

#include "personalization.hxx"

#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Common.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/strbuf.hxx>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <vcl/button.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/virdev.hxx>
#include <dialmgr.hxx>
#include <strings.hrc>

#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <ucbhelper/content.hxx>
#include <comphelper/simplefileaccessinteraction.hxx>

#include <vector>

using namespace com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;

SvxPersonalizationTabPage::SvxPersonalizationTabPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "PersonalizationTabPage", "cui/ui/personalization_tab.ui", &rSet)
{
    // persona
    get(m_pNoPersona, "no_persona");
    get(m_pDefaultPersona, "default_persona");

    for (sal_uInt32 i = 0; i < MAX_DEFAULT_PERSONAS; ++i)
    {
        OUString sDefaultId("default" + OUString::number(i));
        get(m_vDefaultPersonaImages[i], OUStringToOString(sDefaultId, RTL_TEXTENCODING_UTF8));
        m_vDefaultPersonaImages[i]->SetClickHdl(
            LINK(this, SvxPersonalizationTabPage, DefaultPersona));
    }

    LoadDefaultImages();
}

SvxPersonalizationTabPage::~SvxPersonalizationTabPage() { disposeOnce(); }

void SvxPersonalizationTabPage::dispose()
{
    m_pNoPersona.clear();
    m_pDefaultPersona.clear();
    for (VclPtr<PushButton>& i : m_vDefaultPersonaImages)
        i.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SvxPersonalizationTabPage::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<SvxPersonalizationTabPage>::Create(pParent.pParent, *rSet);
}

bool SvxPersonalizationTabPage::FillItemSet(SfxItemSet*)
{
    // persona
    OUString aPersona("default");
    if (m_pNoPersona->IsChecked())
        aPersona = "no";

    bool bModified = false;
    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    if (xContext.is()
        && (aPersona != officecfg::Office::Common::Misc::Persona::get(xContext)
            || m_aPersonaSettings
                   != officecfg::Office::Common::Misc::PersonaSettings::get(xContext)))
    {
        bModified = true;
    }

    // write
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
        comphelper::ConfigurationChanges::create());
    if (aPersona == "no")
        m_aPersonaSettings.clear();
    officecfg::Office::Common::Misc::Persona::set(aPersona, batch);
    officecfg::Office::Common::Misc::PersonaSettings::set(m_aPersonaSettings, batch);
    batch->commit();

    if (bModified)
    {
        // broadcast the change
        DataChangedEvent aDataChanged(DataChangedEventType::SETTINGS, nullptr,
                                      AllSettingsFlags::STYLE);
        Application::NotifyAllWindows(aDataChanged);
    }

    return bModified;
}

void SvxPersonalizationTabPage::Reset(const SfxItemSet*)
{
    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());

    // persona
    OUString aPersona("default");
    if (xContext.is())
    {
        aPersona = officecfg::Office::Common::Misc::Persona::get(xContext);
        m_aPersonaSettings = officecfg::Office::Common::Misc::PersonaSettings::get(xContext);
    }

    if (aPersona == "no")
        m_pNoPersona->Check();
    else
        m_pDefaultPersona->Check();
}

void SvxPersonalizationTabPage::LoadDefaultImages()
{
    // Load the pre saved personas

    OUString gallery = "$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/gallery/personas/";
    rtl::Bootstrap::expandMacros(gallery);
    OUString aPersonasList = gallery + "personas_list.txt";
    SvFileStream aStream(aPersonasList, StreamMode::READ);
    GraphicFilter aFilter;
    Graphic aGraphic;
    sal_Int32 nIndex = 0;
    bool foundOne = false;

    while (aStream.IsOpen() && !aStream.eof() && nIndex < MAX_DEFAULT_PERSONAS)
    {
        OString aLine;
        OUString aPersonaSetting, aPreviewFile, aName;
        sal_Int32 nParseIndex = 0;

        aStream.ReadLine(aLine);
        aPersonaSetting = OStringToOUString(aLine, RTL_TEXTENCODING_UTF8);
        aName = aPersonaSetting.getToken(1, ';', nParseIndex);
        aPreviewFile = aPersonaSetting.getToken(0, ';', nParseIndex);

        if (aPreviewFile.isEmpty())
            break;

        m_vDefaultPersonaSettings.push_back(aPersonaSetting);

        INetURLObject aURLObj(gallery + aPreviewFile);
        aFilter.ImportGraphic(aGraphic, aURLObj);
        BitmapEx aBmp = aGraphic.GetBitmapEx();
        m_vDefaultPersonaImages[nIndex]->SetModeImage(Image(aBmp));
        m_vDefaultPersonaImages[nIndex]->SetQuickHelpText(aName);
        m_vDefaultPersonaImages[nIndex++]->Show();
        foundOne = true;
    }

    m_pDefaultPersona->Enable(foundOne);
}

IMPL_LINK(SvxPersonalizationTabPage, DefaultPersona, Button*, pButton, void)
{
    m_pDefaultPersona->Check();
    for (sal_Int32 nIndex = 0; nIndex < MAX_DEFAULT_PERSONAS; ++nIndex)
    {
        if (pButton == m_vDefaultPersonaImages[nIndex])
            m_aPersonaSettings = m_vDefaultPersonaSettings[nIndex];
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
