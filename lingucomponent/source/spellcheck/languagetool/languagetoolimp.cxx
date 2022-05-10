
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include "languagetoolimp.hxx"


#include <i18nlangtag/languagetag.hxx>
#include <svtools/strings.hrc>
#include <unotools/resmgr.hxx>

#include <vector>
#include <set>
#include <string.h>
#include <sal/log.hxx>

#include <curl/curl.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <algorithm>
#include <string_view>
#include <sal/log.hxx>


using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

LanguageToolGrammarChecker::LanguageToolGrammarChecker()
: mCachedResults(MAX_CACHE_SIZE)
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
    long statusCode = 0;
    std::string response = makeHttpRequest("https://api.languagetoolplus.com/v2/languages", HTTP_METHOD::HTTP_GET, OString(), statusCode);
    boost::property_tree::ptree root;
    std::stringstream aStream(response);
    boost::property_tree::read_json(aStream, root);

    size_t length = root.size();
    m_aSuppLocales.realloc(length);
    int i = 0;
    for (auto it = root.begin(); it != root.end(); it++, i++)
    {
        boost::property_tree::ptree& localeItem = it->second;
        const std::string longCode = localeItem.get<std::string>("longCode");
        Locale aLocale = LanguageTag::convertToLocale(OUString(longCode.c_str(), longCode.length(), RTL_TEXTENCODING_UTF8));
        m_aSuppLocales[i] = aLocale;
    }
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

ProofreadingResult SAL_CALL LanguageToolGrammarChecker::doProofreading( const OUString& aDocumentIdentifier, const OUString& aText, const Locale& aLocale, sal_Int32 nStartOfSentencePosition, sal_Int32 nSuggestedBehindEndOfSentencePosition, const Sequence< PropertyValue >& aProperties )
{
    // ProofreadingResult declared here instead of parseHttpJSONResponse because of the early exists.
    ProofreadingResult xRes;
    xRes.aDocumentIdentifier = aDocumentIdentifier;
    xRes.aText = aText;
    xRes.aLocale = aLocale;
    xRes.nStartOfSentencePosition = nStartOfSentencePosition;
    xRes.nBehindEndOfSentencePosition = nSuggestedBehindEndOfSentencePosition;
    xRes.aProperties = Sequence< PropertyValue >();
    xRes.xProofreader = this;
    xRes.aErrors = Sequence< SingleProofreadingError >();

    if (aText.isEmpty())
    {
        return xRes;
    }

    if (nStartOfSentencePosition != 0)
    {
        return xRes;
    }

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
        xRes.nStartOfNextSentencePosition = std::min(nSuggestedBehindEndOfSentencePosition + 1, aText.getLength());
    }
    xRes.nBehindEndOfSentencePosition = xRes.nStartOfNextSentencePosition;

    auto cachedResult = mCachedResults.find(aText);
    if (cachedResult != mCachedResults.end())
    {
        xRes.aErrors = cachedResult->second;
        return xRes;
    }

    long http_code = 0;
    OUString langTag(aLocale.Language + "-" + aLocale.Country);
    OString postData(OUStringToOString(OUString("text=" + aText + "&language=" + langTag), RTL_TEXTENCODING_UTF8));
    const std::string response_body = makeHttpRequest("https://api.languagetool.org/v2/check", HTTP_METHOD::HTTP_POST, postData, http_code);

    if (http_code != 200)
    {
        return xRes;
    }

    if (response_body.length() <= 0)
    {
        return xRes;
    }

    parseProofreadingJSONResponse(xRes, response_body);
    // cache the result
    mCachedResults.insert(std::pair<OUString, Sequence<SingleProofreadingError>>(aText, xRes.aErrors));
    return xRes;
}

/*
    rResult is both input and output
    aJSONBody is the response body from the HTTP Request to LanguageTool API
*/
void LanguageToolGrammarChecker::parseProofreadingJSONResponse(ProofreadingResult& rResult, std::string_view aJSONBody)
{
    boost::property_tree::ptree root;
    std::stringstream aStream(aJSONBody.data());
    boost::property_tree::read_json(aStream, root);
    boost::property_tree::ptree& matches = root.get_child("matches");
    size_t matchSize = matches.size();

    if (matchSize <= 0)
    {
        return;
    }
    Sequence< SingleProofreadingError > aErrors(matchSize);
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
        aErrors[i].nErrorType = PROOFREADING_ERROR;
        aErrors[i].aShortComment = aShortComment;
        aErrors[i].aFullComment = aFullComment;
        // aErrors[i].aRuleIdentifier = aShortComment;

        const boost::property_tree::ptree& replacements = match.get_child("replacements");
        int suggestionSize = replacements.size();

        if (suggestionSize <= 0)
        {
            continue;
        }
        Sequence< OUString > aSuggestions(std::min(suggestionSize, MAX_SUGGESTIONS_SIZE));
        // Limit suggestions to avoid crash on context menu popup:
        // (soffice:17251): Gdk-CRITICAL **: 17:00:21.277: ../../../../../gdk/wayland/gdkdisplay-wayland.c:1399: Unable to create Cairo image
        // surface: invalid value (typically too big) for the size of the input (surface, pattern, etc.)
        int j = 0;
        for (auto it2 = replacements.begin(); it2 != replacements.end() && j < MAX_SUGGESTIONS_SIZE; it2++, j++)
        {
            const boost::property_tree::ptree& replacement = it2->second;
            std::string replacementStr = replacement.get<std::string>("value");
            aSuggestions[j] = OUString(replacementStr.c_str(), replacementStr.length(), RTL_TEXTENCODING_UTF8);
        }
        aErrors[i].aSuggestions = aSuggestions;
    }
    rResult.aErrors = aErrors;
}

const std::string LanguageToolGrammarChecker::makeHttpRequest(std::string_view aURL, HTTP_METHOD method, const OString& aPostData, long& nStatusCode)
{
    std::unique_ptr<CURL, std::function<void(CURL *)>> curl(
    curl_easy_init(), [](CURL * p) { curl_easy_cleanup(p); });
    if (!curl)
        return {}; // empty string

    std::string response_body;
    curl_easy_setopt(curl.get(), CURLOPT_URL, aURL.data());

    curl_easy_setopt(curl.get(), CURLOPT_FAILONERROR, 1L);
    // curl_easy_setopt(curl.get(), CURLOPT_VERBOSE, 1L);

    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, static_cast<void *>(&response_body));
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, false);
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, CURL_TIMEOUT);

    if (method == HTTP_METHOD::HTTP_POST)
    {
        curl_easy_setopt(curl.get(), CURLOPT_POST, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, aPostData.getStr());
    }

    /*CURLcode cc = */
    curl_easy_perform(curl.get());
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &nStatusCode);
    return response_body;
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