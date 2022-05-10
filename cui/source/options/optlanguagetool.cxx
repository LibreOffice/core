#include "optlanguagetool.hxx"
#include <svtools/languagetoolcfg.hxx>
#include <sal/log.hxx>

OptLanguageToolTabPage::OptLanguageToolTabPage(weld::Container* pPage,
                                               weld::DialogController* pController,
                                               const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "cui/ui/langtoolconfigpage.ui", "OptLangToolPage", &rSet)
    , m_xBaseURLED(m_xBuilder->weld_entry("baseurl"))
    , m_xUsernameED(m_xBuilder->weld_entry("username"))
    , m_xApiKeyED(m_xBuilder->weld_entry("apikey"))
    , m_xActivateBox(m_xBuilder->weld_check_button("activate"))
    , m_xApiSettingsFrame(m_xBuilder->weld_frame("apisettings"))
{
    m_xActivateBox->connect_toggled(LINK(this, OptLanguageToolTabPage, CheckHdl));
    SvxLanguageToolOptions& rLanguageOpts = SvxLanguageToolOptions::Get();
    EnableControls(rLanguageOpts.getEnabled());
}

OptLanguageToolTabPage::~OptLanguageToolTabPage() {}

void OptLanguageToolTabPage::EnableControls(bool bEnable)
{
    SvxLanguageToolOptions& rLanguageOpts = SvxLanguageToolOptions::Get();
    rLanguageOpts.setEnabled(bEnable);
    m_xApiSettingsFrame->set_visible(bEnable);
    m_xActivateBox->set_active(bEnable);
}

IMPL_LINK_NOARG(OptLanguageToolTabPage, CheckHdl, weld::ToggleButton&, void)
{
    EnableControls(m_xActivateBox->get_active());
}

void OptLanguageToolTabPage::Reset(const SfxItemSet*)
{
    SvxLanguageToolOptions& rLanguageOpts = SvxLanguageToolOptions::Get();
    m_xBaseURLED->set_text(rLanguageOpts.getBaseURL());
    m_xUsernameED->set_text(rLanguageOpts.getUsername());
    m_xApiKeyED->set_text(rLanguageOpts.getApiKey());
}

bool OptLanguageToolTabPage::FillItemSet(SfxItemSet*)
{
    SvxLanguageToolOptions& rLanguageOpts = SvxLanguageToolOptions::Get();
    rLanguageOpts.setBaseURL(m_xBaseURLED->get_text());
    rLanguageOpts.setUsername(m_xUsernameED->get_text());
    rLanguageOpts.setApiKey(m_xApiKeyED->get_text());
    return false;
}

std::unique_ptr<SfxTabPage> OptLanguageToolTabPage::Create(weld::Container* pPage,
                                                           weld::DialogController* pController,
                                                           const SfxItemSet* rAttrSet)
{
    return std::make_unique<OptLanguageToolTabPage>(pPage, pController, *rAttrSet);
}