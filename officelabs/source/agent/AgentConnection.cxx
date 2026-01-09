#include <officelabs/AgentConnection.hxx>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <curl/curl.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sal/types.h>

#include <cstdlib>
#include <mutex>
#include <sstream>
#include <string>

namespace {

std::once_flag gCurlInitOnce;

std::string NormalizeBackendUrl(std::string url)
{
    if (!url.empty() && url.back() == '/')
        url.pop_back();
    return url;
}

std::string ToUtf8String(const OUString& value)
{
    OString utf8 = value.toUtf8();
    return std::string(utf8.getStr(), utf8.getLength());
}

void EnsureCurlInit()
{
    std::call_once(gCurlInitOnce, []() { curl_global_init(CURL_GLOBAL_DEFAULT); });
}

} // namespace

namespace officelabs {

size_t AgentConnection::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    static_cast<std::string*>(userp)->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

AgentConnection::AgentConnection()
    : m_backendUrl("http://localhost:8765")
    , m_connected(false)
{
    if (const char* envUrl = std::getenv("OFFICELABS_BACKEND_URL")) {
        if (*envUrl) {
            m_backendUrl = envUrl;
        }
    }

    m_backendUrl = NormalizeBackendUrl(m_backendUrl);
    EnsureCurlInit();
    checkConnection();
}

AgentConnection::~AgentConnection() = default;

bool AgentConnection::checkConnection()
{
    CURL* curl = curl_easy_init();
    if (!curl) {
        m_connected = false;
        return false;
    }

    std::string response;
    std::string url = m_backendUrl + "/api/health";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    CURLcode res = curl_easy_perform(curl);
    long responseCode = 0;
    if (res == CURLE_OK)
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

    curl_easy_cleanup(curl);

    m_connected = (res == CURLE_OK && responseCode >= 200 && responseCode < 300);
    return m_connected;
}

void AgentConnection::setBackendUrl(const OUString& url)
{
    m_backendUrl = NormalizeBackendUrl(ToUtf8String(url));
    checkConnection();
}

AgentResponse AgentConnection::parseResponse(const std::string& json)
{
    AgentResponse response;
    response.hasPatch = false;

    try {
        boost::property_tree::ptree tree;
        std::istringstream stream(json);
        boost::property_tree::read_json(stream, tree);

        const std::string message = tree.get<std::string>("message", "");
        response.message = OUString::fromUtf8(message);

        const auto patchNode = tree.get_child_optional("patch");
        if (patchNode) {
            response.hasPatch = true;
            response.patchType = OUString::fromUtf8(patchNode->get<std::string>("type", ""));
            response.patchTarget = OUString::fromUtf8(patchNode->get<std::string>("target", ""));
            response.patchOldValue = OUString::fromUtf8(patchNode->get<std::string>("old_value", ""));
            response.patchNewValue = OUString::fromUtf8(patchNode->get<std::string>("new_value", ""));
            response.patchDiff = OUString::fromUtf8(patchNode->get<std::string>("diff", ""));
        }
    } catch (const boost::property_tree::json_parser_error& e) {
        SAL_WARN("officelabs", "Failed to parse backend response: " << e.message());
        response.message = u"Error: Invalid response from backend"_ustr;
        response.hasPatch = false;
    }

    return response;
}

AgentResponse AgentConnection::sendMessage(const OUString& message, const OUString& documentContext)
{
    AgentResponse response;

    if (!m_connected) {
        checkConnection();
        if (!m_connected) {
            response.message = u"Error: Backend not available at "_ustr
                               + OUString::fromUtf8(m_backendUrl.c_str());
            response.hasPatch = false;
            return response;
        }
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        response.message = u"Error: Failed to initialize CURL"_ustr;
        response.hasPatch = false;
        m_connected = false;
        return response;
    }

    boost::property_tree::ptree root;
    root.put("message", ToUtf8String(message));
    boost::property_tree::ptree context;
    context.put("document", ToUtf8String(documentContext));
    context.put("selection", "");
    root.add_child("context", context);

    std::ostringstream payloadStream;
    boost::property_tree::write_json(payloadStream, root, false);
    std::string payload = payloadStream.str();

    std::string responseStr;
    std::string url = m_backendUrl + "/api/chat";

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(payload.size()));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    CURLcode res = curl_easy_perform(curl);
    long responseCode = 0;
    if (res == CURLE_OK)
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        response.message = u"Error: "_ustr + OUString::fromUtf8(curl_easy_strerror(res));
        response.hasPatch = false;
        m_connected = false;
        return response;
    }

    if (responseCode < 200 || responseCode >= 300) {
        response.message = u"Error: Backend returned HTTP "_ustr
                           + OUString::number(static_cast<sal_Int64>(responseCode));
        response.hasPatch = false;
        m_connected = false;
        return response;
    }

    m_connected = true;
    return parseResponse(responseStr);
}

} // namespace officelabs
