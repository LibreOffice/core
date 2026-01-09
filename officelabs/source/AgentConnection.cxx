#include <officelabs/AgentConnection.hxx>
#include <curl/curl.h>
#include <rtl/ustrbuf.hxx>

namespace officelabs {

size_t AgentConnection::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

AgentConnection::AgentConnection()
    : m_backendUrl("http://localhost:8765")
    , m_connected(false)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    checkConnection();
}

AgentConnection::~AgentConnection() {
    curl_global_cleanup();
}

bool AgentConnection::checkConnection() {
    CURL* curl = curl_easy_init();
    if (!curl) {
        m_connected = false;
        return false;
    }
    
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, m_backendUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L);
    
    CURLcode res = curl_easy_perform(curl);
    m_connected = (res == CURLE_OK);
    
    curl_easy_cleanup(curl);
    return m_connected;
}

void AgentConnection::setBackendUrl(const OUString& url) {
    m_backendUrl = url.toUtf8().getStr();
    checkConnection();
}

AgentResponse AgentConnection::parseResponse(const std::string& json) {
    AgentResponse response;
    
    // Simple JSON parsing (for production, use a proper JSON library)
    // For now, just extract the message
    size_t msgStart = json.find("\"message\":");
    if (msgStart != std::string::npos) {
        msgStart = json.find("\"", msgStart + 10);
        if (msgStart != std::string::npos) {
            size_t msgEnd = json.find("\"", msgStart + 1);
            if (msgEnd != std::string::npos) {
                std::string msg = json.substr(msgStart + 1, msgEnd - msgStart - 1);
                response.message = OUString::fromUtf8(msg.c_str());
            }
        }
    }
    
    // Check for patch
    response.hasPatch = (json.find("\"patch\":") != std::string::npos);
    
    if (response.hasPatch) {
        // Extract patch details (simplified)
        size_t typeStart = json.find("\"type\":");
        if (typeStart != std::string::npos) {
            typeStart = json.find("\"", typeStart + 7);
            if (typeStart != std::string::npos) {
                size_t typeEnd = json.find("\"", typeStart + 1);
                if (typeEnd != std::string::npos) {
                    std::string type = json.substr(typeStart + 1, typeEnd - typeStart - 1);
                    response.patchType = OUString::fromUtf8(type.c_str());
                }
            }
        }
        
        // Extract diff
        size_t diffStart = json.find("\"diff\":");
        if (diffStart != std::string::npos) {
            diffStart = json.find("\"", diffStart + 7);
            if (diffStart != std::string::npos) {
                size_t diffEnd = json.find("\"", diffStart + 1);
                if (diffEnd != std::string::npos) {
                    std::string diff = json.substr(diffStart + 1, diffEnd - diffStart - 1);
                    response.patchDiff = OUString::fromUtf8(diff.c_str());
                }
            }
        }
    }
    
    return response;
}

AgentResponse AgentConnection::sendMessage(const OUString& message, const OUString& documentContext) {
    AgentResponse response;
    
    if (!m_connected) {
        checkConnection();
        if (!m_connected) {
            response.message = "Error: Backend not available at " + OUString::fromUtf8(m_backendUrl.c_str());
            response.hasPatch = false;
            return response;
        }
    }
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        response.message = "Error: Failed to initialize CURL";
        response.hasPatch = false;
        return response;
    }
    
    std::string responseStr;
    
    // Build JSON payload
    OUStringBuffer jsonBuf;
    jsonBuf.append("{");
    jsonBuf.append("\"message\":\"");
    jsonBuf.append(message);
    jsonBuf.append("\",");
    jsonBuf.append("\"context\":{");
    jsonBuf.append("\"document\":\"");
    jsonBuf.append(documentContext);
    jsonBuf.append("\",");
    jsonBuf.append("\"selection\":\"\"");
    jsonBuf.append("}");
    jsonBuf.append("}");
    
    std::string payload = jsonBuf.makeStringAndClear().toUtf8().getStr();
    std::string url = m_backendUrl + "/api/chat";
    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        response.message = "Error: " + OUString::fromUtf8(curl_easy_strerror(res));
        response.hasPatch = false;
        return response;
    }
    
    return parseResponse(responseStr);
}

} // namespace officelabs
