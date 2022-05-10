#include <com/sun/star/uno/Reference.h>

#include <com/sun/star/linguistic2/SpellFailure.hpp>
#include <com/sun/star/linguistic2/XLinguProperties.hpp>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <tools/debug.hxx>
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include "languagetoolimp.hxx"

#include <lingutil.hxx>

#include <linguistic/lngprops.hxx>
#include <linguistic/spelldta.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <svtools/strings.hrc>
#include <unotools/pathoptions.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/resmgr.hxx>
#include <unotools/useroptions.hxx>
#include <osl/file.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/textenc.h>
#include <sal/log.hxx>

#include <numeric>
#include <utility>
#include <vector>
#include <set>
#include <string.h>
#include <sal/log.hxx>

#include <thread>
#include <chrono>

#include <curl/curl.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <algorithm>


using namespace utl;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

LanguageToolGrammarChecker::LanguageToolGrammarChecker()
{
}

LanguageToolGrammarChecker::~LanguageToolGrammarChecker()
{

}

sal_Bool SAL_CALL LanguageToolGrammarChecker::isSpellChecker()
{
    return false;
}

sal_Bool SAL_CALL LanguageToolGrammarChecker::hasLocale( const Locale& rLocale ) {
    bool bRes = false;
    if (!m_aSuppLocales.hasElements())
        getLocales();

    for (auto const& suppLocale : std::as_const(m_aSuppLocales))
    {
        if (rLocale == suppLocale)
        {
            bRes = true;
            break;
        }
    }

    return bRes;
}

Sequence< Locale > SAL_CALL LanguageToolGrammarChecker::getLocales()
{
    if (m_aSuppLocales.hasElements())
            return m_aSuppLocales;

    // LanguageType nLang = SvtLanguageTable::GetLanguageType( "en-US" );
    Locale aLocale = LanguageTag::convertToLocale("en-US");

    // https://api.languagetoolplus.com/v2/languages
    m_aSuppLocales.realloc(1);
    m_aSuppLocales[0] = aLocale;
    //LanguageTag::convertToLocale(localeName)
    return m_aSuppLocales;
}

// Callback to get the response data from server.
static size_t WriteCallback(void *ptr, size_t size,
                            size_t nmemb, void *userp)
{
  if (!userp)
    return 0;

  std::string* response = static_cast<std::string *>(userp);
  size_t real_size = size * nmemb;
  response->append(static_cast<char *>(ptr), real_size);
  return real_size;
}

::css::linguistic2::ProofreadingResult SAL_CALL LanguageToolGrammarChecker::doProofreading( const ::rtl::OUString& aDocumentIdentifier, const ::rtl::OUString& aText, const ::css::lang::Locale& aLocale, ::sal_Int32 nStartOfSentencePosition, ::sal_Int32 nSuggestedBehindEndOfSentencePosition, const ::css::uno::Sequence< ::css::beans::PropertyValue >& aProperties )
{
    ProofreadingResult xRes;
    Sequence< PropertyValue > aProps(0);
    Sequence< SingleProofreadingError > aErrors(0);
    xRes.aDocumentIdentifier = aDocumentIdentifier;
    xRes.aText = aText;
    xRes.aLocale = aLocale;
    xRes.nStartOfSentencePosition = nStartOfSentencePosition;
    xRes.nStartOfNextSentencePosition = nSuggestedBehindEndOfSentencePosition;
    xRes.aProperties = aProps;
    xRes.xProofreader = this;
    xRes.aErrors = aErrors;

    if (aText.isEmpty()) {
        return xRes;
    }

    xRes.nStartOfSentencePosition = nStartOfSentencePosition;
    xRes.nStartOfNextSentencePosition = nSuggestedBehindEndOfSentencePosition;
    xRes.nStartOfNextSentencePosition = aText.getLength();

    if (aProperties.getLength() > 0 && aProperties[0].Name == "Update")
    {
        // locale changed
        xRes.aText = "";
        return xRes;
    }

    sal_Int32 spaceIndex = std::min(xRes.nStartOfNextSentencePosition, aText.getLength() - 1);
    while (spaceIndex < aText.getLength() && aText[spaceIndex] == ' ')
    {
        xRes.nStartOfNextSentencePosition += 1;
        spaceIndex = xRes.nStartOfNextSentencePosition;
    }
    if (xRes.nStartOfNextSentencePosition == nSuggestedBehindEndOfSentencePosition && spaceIndex < aText.getLength())
    {
        xRes.nStartOfNextSentencePosition = nSuggestedBehindEndOfSentencePosition + 1;
    }
    xRes.nBehindEndOfSentencePosition = xRes.nStartOfNextSentencePosition;

    std::unique_ptr<CURL, std::function<void(CURL *)>> curl(
    curl_easy_init(), [](CURL * p) { curl_easy_cleanup(p); });
    if (!curl)
        return xRes;
    std::string response_body;
    curl_easy_setopt(curl.get(), CURLOPT_URL, "https://api.languagetool.org/v2/check");
    curl_easy_setopt(curl.get(), CURLOPT_POST, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_FAILONERROR, 1L);
    // curl_easy_setopt(curl.get(), CURLOPT_VERBOSE, 1L);

    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, static_cast<void *>(&response_body));
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, false);
    OString postData(OUStringToOString(OUString("text=" + aText + "&language=en-US"), RTL_TEXTENCODING_UTF8));
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, postData.getStr());

    /*CURLcode cc = */
    curl_easy_perform(curl.get());
    long http_code = 0;
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200)
    {
        return xRes;
    }

    if (response_body.length() <= 0)
    {
        return xRes;
    }

    boost::property_tree::ptree root;
    std::stringstream aStream(response_body);
    boost::property_tree::read_json(aStream, root);
    boost::property_tree::ptree& matches = root.get_child("matches");
    size_t matchSize = matches.size();
    if (matchSize <= 0)
    {
        return xRes;
    }

    aErrors.realloc(matchSize);
    size_t i = 0;
    for (auto it1 = matches.begin(); it1 != matches.end(); it1++, i++)
    {
        const boost::property_tree::ptree& match = it1->second;
        int offset = match.get<int>("offset");
        int length = match.get<int>("length");
        const std::string shortMessage = match.get<std::string>("message");
        const std::string message = match.get<std::string>("shortMessage");
        OUString aShortComment = OUString(shortMessage.c_str(), shortMessage.length(), RTL_TEXTENCODING_UTF8);
        OUString aFullComment = OUString(message.c_str(), message.length(), RTL_TEXTENCODING_UTF8);

        aErrors[i].nErrorStart = offset;
        aErrors[i].nErrorLength = length;
        aErrors[i].nErrorType = 2; // PROOFREADING
        aErrors[i].aShortComment = aShortComment;
        aErrors[i].aFullComment = aFullComment;

        const boost::property_tree::ptree& replacements = match.get_child("replacements");
        size_t suggestionSize = replacements.size();

        if (suggestionSize <= 0)
        {
            continue;
        }
        Sequence< OUString > aSuggestions(suggestionSize);
        int j = 0;
        for (auto it2 = replacements.begin(); it2 != replacements.end(); it2++, j++)
        {
            // const auto& x = replacements.get(0);
            const boost::property_tree::ptree& replacement = it2->second;
            std::string replacementStr = replacement.get<std::string>("value");
            aSuggestions[j] = OUString(replacementStr.c_str(), replacementStr.length(), RTL_TEXTENCODING_UTF8);
        }
        aErrors[i].aSuggestions = aSuggestions;
    }
    xRes.aErrors = aErrors;
    return xRes;
}

void SAL_CALL LanguageToolGrammarChecker::ignoreRule(
        const OUString& /*aRuleIdentifier*/,
        const Locale& /*aLocale*/
    )
{
}
void SAL_CALL LanguageToolGrammarChecker::resetIgnoreRules()
{
}

OUString SAL_CALL LanguageToolGrammarChecker::getServiceDisplayName(const Locale& rLocale)
{
    std::locale loc(Translate::Create("svt", LanguageTag(rLocale)));
    return Translate::get("LanguageTool Grammar Checker", loc);
}

OUString SAL_CALL LanguageToolGrammarChecker::getImplementationName()
{
    return "org.openoffice.lingu.LanguageToolGrammarChecker";
}

sal_Bool SAL_CALL LanguageToolGrammarChecker::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL LanguageToolGrammarChecker::getSupportedServiceNames()
{
    return { SN_GRAMMARCHECKER };
}

void SAL_CALL LanguageToolGrammarChecker::initialize( const Sequence< Any >& /*rArguments*/ )
{
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
lingucomponent_LanguageToolGrammarChecker_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(static_cast<cppu::OWeakObject*>(new LanguageToolGrammarChecker()));
}