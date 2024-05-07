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

#include <dialmgr.hxx>

#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Common.hxx>
#include <rtl/bootstrap.hxx>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/virdev.hxx>
#include <personas.hrc>

using namespace com::sun::star;
using namespace ::com::sun::star::beans;

// persona
SvxPersonalizationTabPage::SvxPersonalizationTabPage(weld::Container* pPage,
                                                     weld::DialogController* pController,
                                                     const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"cui/ui/personalization_tab.ui"_ustr,
                 u"PersonalizationTabPage"_ustr, &rSet)
    , m_xNoPersona(m_xBuilder->weld_radio_button(u"no_persona"_ustr))
    , m_xPersonaImg(m_xBuilder->weld_widget(u"lockpersona"_ustr))
    , m_xDefaultPersona(m_xBuilder->weld_radio_button(u"default_persona"_ustr))
    , m_xContentGrid(m_xBuilder->weld_container(u"gridpersonasetting"_ustr))
{
    for (sal_uInt32 i = 0; i < MAX_DEFAULT_PERSONAS; ++i)
    {
        OUString sDefaultId("default" + OUString::number(i));
        m_vDefaultPersonaImages[i] = m_xBuilder->weld_toggle_button(sDefaultId);
        m_vDefaultPersonaImages[i]->connect_clicked(
            LINK(this, SvxPersonalizationTabPage, DefaultPersona));
    }

    LoadDefaultImages();
}

SvxPersonalizationTabPage::~SvxPersonalizationTabPage() {}

std::unique_ptr<SfxTabPage> SvxPersonalizationTabPage::Create(weld::Container* pPage,
                                                              weld::DialogController* pController,
                                                              const SfxItemSet* rSet)
{
    return std::make_unique<SvxPersonalizationTabPage>(pPage, pController, *rSet);
}

OUString SvxPersonalizationTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString radioButton[] = { u"no_persona"_ustr, u"default_persona"_ustr };

    for (const auto& radio : radioButton)
    {
        if (const auto& pString = m_xBuilder->weld_radio_button(radio))
            sAllStrings += pString->get_label() + " ";
    }

    if (const auto& pString = m_xBuilder->weld_label(u"personas_label"_ustr))
        sAllStrings += pString->get_label() + " ";

    return sAllStrings.replaceAll("_", "");
}

bool SvxPersonalizationTabPage::FillItemSet(SfxItemSet*)
{
    // persona
    OUString aPersona(u"default"_ustr);
    if (m_xNoPersona->get_active())
        aPersona = "no";

    bool bModified = false;
    if (aPersona != officecfg::Office::Common::Misc::Persona::get()
        || m_aPersonaSettings != officecfg::Office::Common::Misc::PersonaSettings::get())
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
    // persona
    OUString aPersona = officecfg::Office::Common::Misc::Persona::get();
    m_aPersonaSettings = officecfg::Office::Common::Misc::PersonaSettings::get();

    if (aPersona == "no")
        m_xNoPersona->set_active(true);
    else
        m_xDefaultPersona->set_active(true);

    if (officecfg::Office::Common::Misc::Persona::isReadOnly())
    {
        m_xNoPersona->set_sensitive(false);
        m_xDefaultPersona->set_sensitive(false);
        m_xPersonaImg->set_visible(true);
    }

    if (officecfg::Office::Common::Misc::PersonaSettings::isReadOnly())
        m_xContentGrid->set_sensitive(false);
}

void SvxPersonalizationTabPage::LoadDefaultImages()
{
    // Load the pre saved personas

    OUString gallery = u"$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/gallery/personas/"_ustr;
    rtl::Bootstrap::expandMacros(gallery);
    OUString aPersonasList = gallery + "personas_list.txt";
    SvFileStream aStream(aPersonasList, StreamMode::READ);
    GraphicFilter aFilter;
    Graphic aGraphic;
    sal_Int32 nIndex = 0;
    bool foundOne = false;

    OStringBuffer aLine;
    int nLineNumberFilePersona = 0;
    while (aStream.IsOpen() && !aStream.eof() && nIndex < MAX_DEFAULT_PERSONAS)
    {
        OUString aPersonaSetting, aPreviewFile, aName;
        sal_Int32 nParseIndex = 0;

        aStream.ReadLine(aLine);
        aPersonaSetting = OStringToOUString(aLine, RTL_TEXTENCODING_UTF8);
        aName = CuiResId(RID_PERSONAS_COLOR[nLineNumberFilePersona].first);
        aPreviewFile = aPersonaSetting.getToken(2, ';', nParseIndex);

        if (aPreviewFile.isEmpty())
            break;

        m_vDefaultPersonaSettings.push_back(aPersonaSetting);

        INetURLObject aURLObj(rtl::Concat2View(gallery + aPreviewFile));
        aFilter.ImportGraphic(aGraphic, aURLObj);

        Size aSize(aGraphic.GetSizePixel());
        aSize.setWidth(aSize.Width() / 4);
        aSize.setHeight(aSize.Height() / 1.5);
        ScopedVclPtr<VirtualDevice> xVirDev
            = m_vDefaultPersonaImages[nIndex]->create_virtual_device();
        xVirDev->SetOutputSizePixel(aSize);
        aGraphic.Draw(*xVirDev, Point(0, 0));
        m_vDefaultPersonaImages[nIndex]->set_image(xVirDev.get());
        xVirDev.disposeAndClear();

        m_vDefaultPersonaImages[nIndex]->set_tooltip_text(aName);
        m_vDefaultPersonaImages[nIndex++]->show();
        foundOne = true;
        ++nLineNumberFilePersona;
    }

    m_xDefaultPersona->set_sensitive(foundOne);
}

IMPL_LINK(SvxPersonalizationTabPage, DefaultPersona, weld::Button&, rButton, void)
{
    m_xDefaultPersona->set_active(true);
    for (sal_Int32 nIndex = 0; nIndex < MAX_DEFAULT_PERSONAS; ++nIndex)
    {
        if (&rButton == m_vDefaultPersonaImages[nIndex].get())
            m_aPersonaSettings = m_vDefaultPersonaSettings[nIndex];
        else
            m_vDefaultPersonaImages[nIndex]->set_active(false);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
