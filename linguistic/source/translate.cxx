#include <linguistic/translate.hxx>
#include <sal/log.hxx>
#include <curl/curl.h>
#include <sal/log.hxx>
#include <rtl/string.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <curlinit.hxx>
#include <vcl/htmltransferable.hxx>
#include <tools/long.hxx>

namespace linguistic
{
OString Translate(const OString& rTargetLang, const OString& rAPIUrl, const OString& rAuthKey,
                  const OString& rData)
{
    constexpr tools::Long CURL_TIMEOUT = 10L;

    std::unique_ptr<CURL, std::function<void(CURL*)>> curl(curl_easy_init(),
                                                           [](CURL* p) { curl_easy_cleanup(p); });

    ::InitCurl_easy(curl.get());

    curl_easy_setopt(curl.get(), CURLOPT_URL, rAPIUrl.getStr());
    curl_easy_setopt(curl.get(), CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, CURL_TIMEOUT);

    std::string response_body;
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION,
                     +[](void* buffer, size_t size, size_t nmemb, void* userp) -> size_t {
                         if (!userp)
                             return 0;
                         std::string* response = static_cast<std::string*>(userp);
                         size_t real_size = size * nmemb;
                         response->append(static_cast<char*>(buffer), real_size);
                         return real_size;
                     });
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, static_cast<void*>(&response_body));
    OString aLang(curl_easy_escape(curl.get(), rTargetLang.getStr(), rTargetLang.getLength()));
    OString aAuthKey(curl_easy_escape(curl.get(), rAuthKey.getStr(), rAuthKey.getLength()));
    OString aData(curl_easy_escape(curl.get(), rData.getStr(), rData.getLength()));
    OString aPostData("auth_key=" + aAuthKey + "&target_lang=" + aLang + "&text=" + aData);

    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, aPostData.getStr());
    CURLcode cc = curl_easy_perform(curl.get());
    if (cc != CURLE_OK)
    {
        SAL_WARN("translatehelper",
                 "CURL perform returned with error: " << static_cast<sal_Int32>(cc));
        return {};
    }
    tools::Long nStatusCode;
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &nStatusCode);
    if (nStatusCode != 200)
    {
        SAL_WARN("translatehelper", "CURL request returned with status code: " << nStatusCode);
        return {};
    }
    // parse the response
    boost::property_tree::ptree root;
    std::stringstream aStream(response_body.data());
    boost::property_tree::read_json(aStream, root);
    boost::property_tree::ptree& translations = root.get_child("translations");
    size_t size = translations.size();
    if (size <= 0)
    {
        SAL_WARN("translatehelper", "API did not return any translations");
    }
    // take the first one
    const boost::property_tree::ptree& translation = translations.begin()->second;
    const std::string text = translation.get<std::string>("text");
    return OString(text);
}
}
