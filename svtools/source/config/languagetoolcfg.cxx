#include <sal/log.hxx>
#include <sal/config.h>
#include <svtools/languagetoolcfg.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/debug.hxx>

using namespace utl;
using namespace com::sun::star::uno;

struct LanguageToolOptions_Impl
{
    OUString sBaseURL;
    OUString sUsername;
    OUString sApiKey;
};

const Sequence<OUString>& SvxLanguageToolOptions::GetPropertyNames()
{
    static Sequence<OUString> const aNames
    {
            "LanguageTool/BaseURL",
            "LanguageTool/Username",
            "LanguageTool/ApiKey"
    };
    return aNames;
}

const OUString SvxLanguageToolOptions::getBaseURL() const
{
    return pImpl->sBaseURL;
}

void SvxLanguageToolOptions::setBaseURL(const OUString& rVal)
{
    pImpl->sBaseURL = rVal;
    SetModified();
}

const OUString SvxLanguageToolOptions::getUsername() const
{
    return pImpl->sUsername;
}

void SvxLanguageToolOptions::setUsername(const OUString& rVal)
{
    pImpl->sUsername = rVal;
    SetModified();
}

const OUString SvxLanguageToolOptions::getLocaleListURL() const
{
    return pImpl->sBaseURL + "/languages";
}

const OUString SvxLanguageToolOptions::getCheckerURL() const
{
    return pImpl->sBaseURL + "/check";
}

const OUString SvxLanguageToolOptions::getApiKey() const
{
    return pImpl->sApiKey;
}

void SvxLanguageToolOptions::setApiKey(const OUString& rVal)
{
    pImpl->sApiKey = rVal;
    SetModified();
}

namespace
{
    class theSvxLanguageToolOptions : public rtl::Static<SvxLanguageToolOptions, theSvxLanguageToolOptions> {};
}

SvxLanguageToolOptions& SvxLanguageToolOptions::Get()
{
    return theSvxLanguageToolOptions::get();
}


SvxLanguageToolOptions::SvxLanguageToolOptions()
    : ConfigItem("Office.Linguistic/GrammarChecking")
    , pImpl(new LanguageToolOptions_Impl)
{
    Load( GetPropertyNames() );
}

SvxLanguageToolOptions::~SvxLanguageToolOptions()
{

}
void SvxLanguageToolOptions::Notify( const css::uno::Sequence< OUString >& )
{
    Load( GetPropertyNames() );
}

void SvxLanguageToolOptions::Load( const css::uno::Sequence< OUString >& aNames )
{
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() != aNames.getLength())
        return;
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        if(!pValues[nProp].hasValue()) continue;
        switch(nProp)
        {
            case  0:
                pValues[nProp] >>= pImpl->sBaseURL;
            break;
            case  1:
                pValues[nProp] >>= pImpl->sUsername;
            break;
            case  2:
                pValues[nProp] >>= pImpl->sApiKey;
            break;
            default:
            break;
        }
    }
}

void SvxLanguageToolOptions::ImplCommit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch (nProp)
        {
            case 0:
                pValues[nProp] <<= pImpl->sBaseURL;
                break;
            case 1:
                pValues[nProp] <<= pImpl->sUsername;
                break;
            case 2:
                pValues[nProp] <<= pImpl->sApiKey;
                break;
            default:
                break;
        }
    }
    PutProperties(aNames, aValues);
}