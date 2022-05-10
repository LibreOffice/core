#pragma once
#include <unotools/configitem.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <svtools/svtdllapi.h>

using namespace utl;
using namespace com::sun::star::uno;

struct LanguageToolOptions_Impl;

class SVT_DLLPUBLIC SvxLanguageToolOptions final : public utl::ConfigItem
{
public:
    SvxLanguageToolOptions();
    virtual ~SvxLanguageToolOptions() override;

    virtual void Notify( const css::uno::Sequence< OUString >& _rPropertyNames) override;
    static SvxLanguageToolOptions& Get();

    const OUString getBaseURL() const;
    void setBaseURL(const OUString& rVal);

    const OUString getUsername() const;
    void setUsername(const OUString& rVal);

    const OUString getLocaleListURL() const;
    const OUString getCheckerURL() const;

    const OUString getApiKey() const;
    void setApiKey(const OUString& rVal);

    bool getEnabled() const;
    void setEnabled(bool enabled);

private:
    std::unique_ptr<LanguageToolOptions_Impl> pImpl;
    void Load( const css::uno::Sequence< OUString >& rPropertyNames );
    virtual void ImplCommit() override;
    const Sequence<OUString>& GetPropertyNames();
};
