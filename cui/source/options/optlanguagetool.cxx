#include "optlanguagetool.hxx"

OptLanguageToolTabPage::OptLanguageToolTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
: SfxTabPage(pPage, pController, "cui/ui/langtoolconfigpage.ui", "OptLangToolPage", &rSet)
{

}

OptLanguageToolTabPage::~OptLanguageToolTabPage()
{
}

void OptLanguageToolTabPage::Reset( const SfxItemSet* )
{

}

bool OptLanguageToolTabPage::FillItemSet( SfxItemSet* )
{
    return false;
}

std::unique_ptr<SfxTabPage> OptLanguageToolTabPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                           const SfxItemSet* rAttrSet )
{
    return std::make_unique<OptLanguageToolTabPage>(pPage, pController, *rAttrSet);
}