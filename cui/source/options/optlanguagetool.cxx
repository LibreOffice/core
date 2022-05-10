#include "optlanguagetool.hxx"
#include <svtools/languagetoolcfg.hxx>
#include <sal/log.hxx>

OptLanguageToolTabPage::OptLanguageToolTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "cui/ui/langtoolconfigpage.ui", "OptLangToolPage", &rSet)
    , m_xBaseURLED(m_xBuilder->weld_entry("baseurl"))
    , m_xUsernameED(m_xBuilder->weld_entry("username"))
    , m_xApiKeyED(m_xBuilder->weld_entry("apikey"))
{

}

OptLanguageToolTabPage::~OptLanguageToolTabPage()
{
}

void OptLanguageToolTabPage::Reset( const SfxItemSet* )
{
    SvxLanguageToolOptions& rLanguageOpts = SvxLanguageToolOptions::Get();
    m_xBaseURLED->set_text( rLanguageOpts.getBaseURL() );
    m_xUsernameED->set_text( rLanguageOpts.getLocaleListURL() );
    m_xApiKeyED->set_text( rLanguageOpts.getApiKey() );
}

bool OptLanguageToolTabPage::FillItemSet( SfxItemSet* )
{
    SvxLanguageToolOptions& rLanguageOpts = SvxLanguageToolOptions::Get();
    rLanguageOpts.setBaseURL( m_xBaseURLED->get_text() );
    rLanguageOpts.setUsername( m_xUsernameED->get_text() );
    rLanguageOpts.setApiKey( m_xApiKeyED->get_text() );
    return false;
}

std::unique_ptr<SfxTabPage> OptLanguageToolTabPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                           const SfxItemSet* rAttrSet )
{
    return std::make_unique<OptLanguageToolTabPage>(pPage, pController, *rAttrSet);
}