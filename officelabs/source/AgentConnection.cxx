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

// Helper to extract a JSON string value after a key
static std::string extractJsonString(const std::string& json, const std::string& key, size_t startPos = 0) {
    std::string searchKey = "\"" + key + "\":";
    size_t keyPos = json.find(searchKey, startPos);
    if (keyPos == std::string::npos) return "";

    size_t valueStart = json.find("\"", keyPos + searchKey.length());
    if (valueStart == std::string::npos) return "";

    // Handle escaped quotes in the value
    size_t valueEnd = valueStart + 1;
    while (valueEnd < json.length()) {
        if (json[valueEnd] == '"' && json[valueEnd - 1] != '\\') {
            break;
        }
        valueEnd++;
    }

    return json.substr(valueStart + 1, valueEnd - valueStart - 1);
}

// Helper to unescape JSON string
static std::string unescapeJson(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            char next = str[i + 1];
            if (next == 'n') { result += '\n'; i++; }
            else if (next == 'r') { result += '\r'; i++; }
            else if (next == 't') { result += '\t'; i++; }
            else if (next == '"') { result += '"'; i++; }
            else if (next == '\\') { result += '\\'; i++; }
            else { result += str[i]; }
        } else {
            result += str[i];
        }
    }
    return result;
}

// Helper to extract a JSON boolean value after a key
static bool extractJsonBool(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\":";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return false;

    size_t valueStart = keyPos + searchKey.length();
    // Skip whitespace
    while (valueStart < json.length() && (json[valueStart] == ' ' || json[valueStart] == '\t'))
        valueStart++;

    if (valueStart >= json.length()) return false;

    // Check for true/false
    if (json.substr(valueStart, 4) == "true") return true;
    return false;
}

// Helper to extract a JSON integer value after a key
static int extractJsonInt(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\":";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return 0;

    size_t valueStart = keyPos + searchKey.length();
    // Skip whitespace
    while (valueStart < json.length() && (json[valueStart] == ' ' || json[valueStart] == '\t'))
        valueStart++;

    if (valueStart >= json.length()) return 0;

    // Parse integer
    std::string numStr;
    while (valueStart < json.length() && (isdigit(json[valueStart]) || json[valueStart] == '-')) {
        numStr += json[valueStart];
        valueStart++;
    }

    if (numStr.empty()) return 0;
    return std::stoi(numStr);
}

AgentResponse AgentConnection::parseResponse(const std::string& json) {
    AgentResponse response;

    // Extract message
    std::string msg = extractJsonString(json, "message");
    response.message = OUString::fromUtf8(unescapeJson(msg).c_str());

    // Check for patch
    response.hasPatch = (json.find("\"patch\":") != std::string::npos &&
                         json.find("\"patch\":null") == std::string::npos);

    if (response.hasPatch) {
        response.patchType = OUString::fromUtf8(extractJsonString(json, "type").c_str());
        response.patchTarget = OUString::fromUtf8(extractJsonString(json, "target").c_str());
        response.patchNewValue = OUString::fromUtf8(unescapeJson(extractJsonString(json, "new_value")).c_str());
        response.patchOldValue = OUString::fromUtf8(unescapeJson(extractJsonString(json, "old_value")).c_str());
        response.patchDiff = OUString::fromUtf8(unescapeJson(extractJsonString(json, "diff")).c_str());
    }

    // Parse auto_edits array
    FILE* dbg = fopen("C:\\temp\\parse_response_debug.log", "a");
    if (dbg) {
        fprintf(dbg, "parseResponse: looking for auto_edits\n");
        fprintf(dbg, "  json length: %d\n", (int)json.length());
    }

    size_t autoEditsStart = json.find("\"auto_edits\":");
    if (dbg) fprintf(dbg, "  autoEditsStart: %d\n", (int)autoEditsStart);

    if (autoEditsStart != std::string::npos) {
        size_t arrayStart = json.find("[", autoEditsStart);
        size_t arrayEnd = json.find("]", arrayStart);
        if (dbg) fprintf(dbg, "  arrayStart: %d, arrayEnd: %d\n", (int)arrayStart, (int)arrayEnd);

        if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
            std::string arrayContent = json.substr(arrayStart, arrayEnd - arrayStart + 1);
            if (dbg) fprintf(dbg, "  arrayContent length: %d\n", (int)arrayContent.length());

            // Parse each object in the array
            size_t objStart = 0;
            int objCount = 0;
            while ((objStart = arrayContent.find("{", objStart)) != std::string::npos) {
                size_t objEnd = arrayContent.find("}", objStart);
                if (objEnd == std::string::npos) break;

                std::string objStr = arrayContent.substr(objStart, objEnd - objStart + 1);
                if (dbg) fprintf(dbg, "  object %d: %s\n", objCount, objStr.c_str());

                AutoEditCommand cmd;
                cmd.action = OUString::fromUtf8(extractJsonString(objStr, "action").c_str());
                cmd.findText = OUString::fromUtf8(unescapeJson(extractJsonString(objStr, "find_text")).c_str());
                cmd.newText = OUString::fromUtf8(unescapeJson(extractJsonString(objStr, "new_text")).c_str());
                cmd.position = OUString::fromUtf8(extractJsonString(objStr, "position").c_str());
                // Extract formatting flags
                cmd.bold = extractJsonBool(objStr, "bold");
                cmd.italic = extractJsonBool(objStr, "italic");
                cmd.underline = extractJsonBool(objStr, "underline");
                cmd.headingLevel = extractJsonInt(objStr, "heading_level");
                cmd.fontColor = OUString::fromUtf8(extractJsonString(objStr, "font_color").c_str());

                if (dbg) {
                    OString actionStr = cmd.action.toUtf8();
                    OString colorStr = cmd.fontColor.toUtf8();
                    fprintf(dbg, "    action='%s' bold=%d color='%s'\n", actionStr.getStr(), cmd.bold ? 1 : 0, colorStr.getStr());
                }

                if (!cmd.action.isEmpty()) {
                    response.autoEdits.push_back(cmd);
                }

                objStart = objEnd + 1;
                objCount++;
            }
            if (dbg) fprintf(dbg, "  Total objects parsed: %d\n", objCount);
        }
    } else {
        if (dbg) fprintf(dbg, "  auto_edits NOT FOUND in JSON\n");
    }

    if (dbg) {
        fprintf(dbg, "  Final autoEdits count: %d\n", (int)response.autoEdits.size());
        fclose(dbg);
    }

    return response;
}

static OUString escapeJsonString(const OUString& str) {
    OUStringBuffer result;
    for (sal_Int32 i = 0; i < str.getLength(); ++i) {
        sal_Unicode c = str[i];
        switch (c) {
            case '"': result.append("\\\""); break;
            case '\\': result.append("\\\\"); break;
            case '\n': result.append("\\n"); break;
            case '\r': result.append("\\r"); break;
            case '\t': result.append("\\t"); break;
            default:
                if (c < 32) {
                    // Escape other control characters
                    result.append("\\u00");
                    result.append(static_cast<sal_Unicode>((c >> 4) < 10 ? '0' + (c >> 4) : 'a' + (c >> 4) - 10));
                    result.append(static_cast<sal_Unicode>((c & 0xF) < 10 ? '0' + (c & 0xF) : 'a' + (c & 0xF) - 10));
                } else {
                    result.append(c);
                }
                break;
        }
    }
    return result.makeStringAndClear();
}

AgentResponse AgentConnection::sendMessage(const OUString& message, const OUString& documentContent, const OUString& selection) {
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

    // Build JSON payload with proper escaping
    OUStringBuffer jsonBuf;
    jsonBuf.append("{");
    jsonBuf.append("\"message\":\"");
    jsonBuf.append(escapeJsonString(message));
    jsonBuf.append("\",");
    jsonBuf.append("\"context\":{");
    jsonBuf.append("\"document\":\"");
    jsonBuf.append(escapeJsonString(documentContent));
    jsonBuf.append("\",");
    jsonBuf.append("\"selection\":\"");
    jsonBuf.append(escapeJsonString(selection));
    jsonBuf.append("\"");
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
