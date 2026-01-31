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

    // Debug: dump raw JSON to file
    FILE* rawDbg = fopen("C:\\temp\\raw_json_debug.log", "w");
    if (rawDbg) {
        fprintf(rawDbg, "Raw JSON (%d bytes):\n%s\n", (int)json.length(), json.c_str());
        fclose(rawDbg);
    }

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
        fflush(dbg);
    }

    size_t autoEditsStart = json.find("\"auto_edits\":");
    if (dbg) { fprintf(dbg, "  autoEditsStart: %d\n", (int)autoEditsStart); fflush(dbg); }

    if (autoEditsStart != std::string::npos) {
        size_t arrayStart = json.find("[", autoEditsStart);
        // Find matching closing bracket (handle nested arrays)
        size_t arrayEnd = arrayStart;
        int bracketDepth = 1;
        for (size_t i = arrayStart + 1; i < json.length() && bracketDepth > 0; ++i) {
            if (json[i] == '[') bracketDepth++;
            else if (json[i] == ']') bracketDepth--;
            if (bracketDepth == 0) arrayEnd = i;
        }
        if (dbg) { fprintf(dbg, "  arrayStart: %d, arrayEnd: %d\n", (int)arrayStart, (int)arrayEnd); fflush(dbg); }

        if (arrayStart != std::string::npos && arrayEnd != std::string::npos && arrayEnd > arrayStart) {
            std::string arrayContent = json.substr(arrayStart, arrayEnd - arrayStart + 1);
            if (dbg) { fprintf(dbg, "  arrayContent length: %d\n", (int)arrayContent.length()); fflush(dbg); }

            // Parse each object in the array
            size_t objStart = 0;
            int objCount = 0;
            while ((objStart = arrayContent.find("{", objStart)) != std::string::npos) {
                // Find matching closing brace (handle nested structures)
                size_t objEnd = objStart;
                int braceDepth = 1;
                for (size_t i = objStart + 1; i < arrayContent.length() && braceDepth > 0; ++i) {
                    if (arrayContent[i] == '{') braceDepth++;
                    else if (arrayContent[i] == '}') braceDepth--;
                    if (braceDepth == 0) objEnd = i;
                }
                if (objEnd == objStart) break;

                std::string objStr = arrayContent.substr(objStart, objEnd - objStart + 1);
                if (dbg) { fprintf(dbg, "  object %d (len %d): parsing...\n", objCount, (int)objStr.length()); fflush(dbg); }

                AutoEditCommand cmd;
                if (dbg) { fprintf(dbg, "    extracting action...\n"); fflush(dbg); }
                cmd.action = OUString::fromUtf8(extractJsonString(objStr, "action").c_str());
                if (dbg) { fprintf(dbg, "    extracting find_text...\n"); fflush(dbg); }
                cmd.findText = OUString::fromUtf8(unescapeJson(extractJsonString(objStr, "find_text")).c_str());
                if (dbg) { fprintf(dbg, "    extracting new_text...\n"); fflush(dbg); }
                cmd.newText = OUString::fromUtf8(unescapeJson(extractJsonString(objStr, "new_text")).c_str());
                if (dbg) { fprintf(dbg, "    extracting position...\n"); fflush(dbg); }
                cmd.position = OUString::fromUtf8(extractJsonString(objStr, "position").c_str());
                if (dbg) { fprintf(dbg, "    extracting as_paragraph...\n"); fflush(dbg); }
                cmd.asParagraph = extractJsonBool(objStr, "as_paragraph");
                if (dbg) { fprintf(dbg, "    extracting bold...\n"); fflush(dbg); }
                cmd.bold = extractJsonBool(objStr, "bold");
                if (dbg) { fprintf(dbg, "    extracting italic...\n"); fflush(dbg); }
                cmd.italic = extractJsonBool(objStr, "italic");
                if (dbg) { fprintf(dbg, "    extracting underline...\n"); fflush(dbg); }
                cmd.underline = extractJsonBool(objStr, "underline");
                cmd.strikethrough = extractJsonBool(objStr, "strikethrough");
                cmd.superscript = extractJsonBool(objStr, "superscript");
                cmd.subscript = extractJsonBool(objStr, "subscript");
                if (dbg) { fprintf(dbg, "    extracting heading_level...\n"); fflush(dbg); }
                cmd.headingLevel = extractJsonInt(objStr, "heading_level");
                if (dbg) { fprintf(dbg, "    extracting font_color...\n"); fflush(dbg); }
                cmd.fontColor = OUString::fromUtf8(extractJsonString(objStr, "font_color").c_str());
                cmd.highlightColor = OUString::fromUtf8(extractJsonString(objStr, "highlight_color").c_str());
                cmd.fontName = OUString::fromUtf8(extractJsonString(objStr, "font_name").c_str());
                if (dbg) { fprintf(dbg, "    extracting font_size...\n"); fflush(dbg); }
                // Parse font_size as double from JSON number
                cmd.fontSize = 0;
                {
                    std::string searchKey = "\"font_size\":";
                    size_t keyPos = objStr.find(searchKey);
                    if (keyPos != std::string::npos) {
                        size_t valStart = keyPos + searchKey.length();
                        std::string numStr;
                        while (valStart < objStr.length() && (isdigit(objStr[valStart]) || objStr[valStart] == '.' || objStr[valStart] == '-')) {
                            numStr += objStr[valStart++];
                        }
                        if (!numStr.empty()) {
                            try { cmd.fontSize = std::stod(numStr); } catch (...) {}
                        }
                    }
                }
                // Paragraph formatting
                if (dbg) { fprintf(dbg, "    extracting alignment...\n"); fflush(dbg); }
                cmd.alignment = OUString::fromUtf8(extractJsonString(objStr, "alignment").c_str());
                if (dbg) { fprintf(dbg, "    extracting line_spacing...\n"); fflush(dbg); }
                // Parse line_spacing as double from JSON number (not string)
                cmd.lineSpacing = 0;
                {
                    std::string searchKey = "\"line_spacing\":";
                    size_t keyPos = objStr.find(searchKey);
                    if (keyPos != std::string::npos) {
                        size_t valStart = keyPos + searchKey.length();
                        std::string numStr;
                        while (valStart < objStr.length() && (isdigit(objStr[valStart]) || objStr[valStart] == '.' || objStr[valStart] == '-')) {
                            numStr += objStr[valStart++];
                        }
                        if (!numStr.empty()) {
                            try { cmd.lineSpacing = std::stod(numStr); } catch (...) {}
                        }
                    }
                }
                if (dbg) { fprintf(dbg, "    line_spacing = %f\n", cmd.lineSpacing); fflush(dbg); }
                cmd.spaceBefore = static_cast<double>(extractJsonInt(objStr, "space_before"));
                cmd.spaceAfter = static_cast<double>(extractJsonInt(objStr, "space_after"));
                // Parse indents as double from JSON number
                {
                    std::string searchKey = "\"indent_left\":";
                    size_t keyPos = objStr.find(searchKey);
                    if (keyPos != std::string::npos) {
                        size_t valStart = keyPos + searchKey.length();
                        std::string numStr;
                        while (valStart < objStr.length() && (isdigit(objStr[valStart]) || objStr[valStart] == '.' || objStr[valStart] == '-')) {
                            numStr += objStr[valStart++];
                        }
                        if (!numStr.empty()) {
                            try { cmd.indentLeft = std::stod(numStr); } catch (...) {}
                        }
                    }
                }
                {
                    std::string searchKey = "\"indent_right\":";
                    size_t keyPos = objStr.find(searchKey);
                    if (keyPos != std::string::npos) {
                        size_t valStart = keyPos + searchKey.length();
                        std::string numStr;
                        while (valStart < objStr.length() && (isdigit(objStr[valStart]) || objStr[valStart] == '.' || objStr[valStart] == '-')) {
                            numStr += objStr[valStart++];
                        }
                        if (!numStr.empty()) {
                            try { cmd.indentRight = std::stod(numStr); } catch (...) {}
                        }
                    }
                }
                {
                    std::string searchKey = "\"indent_first_line\":";
                    size_t keyPos = objStr.find(searchKey);
                    if (keyPos != std::string::npos) {
                        size_t valStart = keyPos + searchKey.length();
                        std::string numStr;
                        while (valStart < objStr.length() && (isdigit(objStr[valStart]) || objStr[valStart] == '.' || objStr[valStart] == '-')) {
                            numStr += objStr[valStart++];
                        }
                        if (!numStr.empty()) {
                            try { cmd.indentFirstLine = std::stod(numStr); } catch (...) {}
                        }
                    }
                }
                // Style application
                cmd.styleName = OUString::fromUtf8(extractJsonString(objStr, "style_name").c_str());
                // Undo/Redo steps
                cmd.undoSteps = extractJsonInt(objStr, "undo_steps");
                if (cmd.undoSteps <= 0) cmd.undoSteps = 1;  // Default to 1
                // List creation
                cmd.listType = OUString::fromUtf8(extractJsonString(objStr, "list_type").c_str());
                // Parse list_items array
                size_t listStart = objStr.find("\"list_items\":");
                if (listStart != std::string::npos) {
                    size_t arrStart = objStr.find("[", listStart);
                    size_t arrEnd = objStr.find("]", arrStart);
                    if (arrStart != std::string::npos && arrEnd != std::string::npos) {
                        std::string itemsStr = objStr.substr(arrStart + 1, arrEnd - arrStart - 1);
                        // Simple string array parsing
                        size_t pos = 0;
                        while ((pos = itemsStr.find("\"", pos)) != std::string::npos) {
                            size_t end = itemsStr.find("\"", pos + 1);
                            if (end != std::string::npos) {
                                std::string item = itemsStr.substr(pos + 1, end - pos - 1);
                                cmd.listItems.push_back(OUString::fromUtf8(unescapeJson(item).c_str()));
                                pos = end + 1;
                            } else {
                                break;
                            }
                        }
                    }
                }

                // Table creation
                cmd.tableRows = extractJsonInt(objStr, "table_rows");
                cmd.tableColumns = extractJsonInt(objStr, "table_columns");
                cmd.headerRow = extractJsonBool(objStr, "header_row");
                // Parse table_data 2D array: [["col1", "col2"], ["cell1", "cell2"]]
                {
                    size_t tableDataStart = objStr.find("\"table_data\":");
                    if (tableDataStart != std::string::npos) {
                        size_t outerArrStart = objStr.find("[", tableDataStart);
                        if (outerArrStart != std::string::npos) {
                            // Find matching closing bracket for outer array
                            size_t outerArrEnd = outerArrStart;
                            int tableBracketDepth = 1;
                            for (size_t i = outerArrStart + 1; i < objStr.length() && tableBracketDepth > 0; ++i) {
                                if (objStr[i] == '[') tableBracketDepth++;
                                else if (objStr[i] == ']') tableBracketDepth--;
                                if (tableBracketDepth == 0) outerArrEnd = i;
                            }
                            if (outerArrEnd > outerArrStart) {
                                // Parse each row (inner array)
                                size_t pos = outerArrStart + 1;
                                while (pos < outerArrEnd) {
                                    // Find start of row array
                                    size_t rowStart = objStr.find("[", pos);
                                    if (rowStart == std::string::npos || rowStart >= outerArrEnd) break;
                                    // Find end of row array
                                    size_t rowEnd = rowStart;
                                    int rowDepth = 1;
                                    for (size_t i = rowStart + 1; i < outerArrEnd && rowDepth > 0; ++i) {
                                        if (objStr[i] == '[') rowDepth++;
                                        else if (objStr[i] == ']') rowDepth--;
                                        if (rowDepth == 0) rowEnd = i;
                                    }
                                    if (rowEnd > rowStart) {
                                        // Parse strings in this row
                                        std::vector<OUString> rowData;
                                        std::string rowStr = objStr.substr(rowStart + 1, rowEnd - rowStart - 1);
                                        size_t strPos = 0;
                                        while ((strPos = rowStr.find("\"", strPos)) != std::string::npos) {
                                            size_t strEnd = rowStr.find("\"", strPos + 1);
                                            // Handle escaped quotes
                                            while (strEnd != std::string::npos && strEnd > 0 && rowStr[strEnd - 1] == '\\') {
                                                strEnd = rowStr.find("\"", strEnd + 1);
                                            }
                                            if (strEnd != std::string::npos) {
                                                std::string cellVal = rowStr.substr(strPos + 1, strEnd - strPos - 1);
                                                rowData.push_back(OUString::fromUtf8(unescapeJson(cellVal).c_str()));
                                                strPos = strEnd + 1;
                                            } else {
                                                break;
                                            }
                                        }
                                        if (!rowData.empty()) {
                                            cmd.tableData.push_back(rowData);
                                        }
                                        pos = rowEnd + 1;
                                    } else {
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                // Search & Replace
                cmd.caseSensitive = extractJsonBool(objStr, "case_sensitive");
                cmd.wholeWords = extractJsonBool(objStr, "whole_words");

                if (dbg) {
                    OString actionStr = cmd.action.toUtf8();
                    OString colorStr = cmd.fontColor.toUtf8();
                    OString alignStr = cmd.alignment.toUtf8();
                    fprintf(dbg, "    action='%s' bold=%d color='%s' asParagraph=%d fontSize=%.1f align='%s' lineSpacing=%.1f listItems=%d rows=%d cols=%d tableData=%d\n",
                            actionStr.getStr(), cmd.bold ? 1 : 0, colorStr.getStr(), cmd.asParagraph ? 1 : 0, cmd.fontSize, alignStr.getStr(), cmd.lineSpacing, (int)cmd.listItems.size(), cmd.tableRows, cmd.tableColumns, (int)cmd.tableData.size());
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
    }

    // Check for clarification flow
    response.needsClarification = extractJsonBool(json, "needs_clarification");

    if (response.needsClarification) {
        if (dbg) fprintf(dbg, "  needs_clarification: true, parsing questions\n");

        // Parse clarification_questions array
        size_t questionsStart = json.find("\"clarification_questions\":");
        if (questionsStart != std::string::npos) {
            size_t arrStart = json.find("[", questionsStart);
            if (arrStart != std::string::npos) {
                // Find matching closing bracket
                size_t arrEnd = arrStart;
                int depth = 1;
                for (size_t i = arrStart + 1; i < json.length() && depth > 0; ++i) {
                    if (json[i] == '[') depth++;
                    else if (json[i] == ']') depth--;
                    if (depth == 0) arrEnd = i;
                }

                if (arrEnd > arrStart) {
                    // Parse each question object
                    size_t objStart = arrStart;
                    while ((objStart = json.find("{", objStart + 1)) != std::string::npos && objStart < arrEnd) {
                        size_t objEnd = objStart;
                        int objDepth = 1;
                        for (size_t i = objStart + 1; i < arrEnd && objDepth > 0; ++i) {
                            if (json[i] == '{') objDepth++;
                            else if (json[i] == '}') objDepth--;
                            if (objDepth == 0) objEnd = i;
                        }
                        if (objEnd > objStart) {
                            std::string qStr = json.substr(objStart, objEnd - objStart + 1);

                            ClarificationQuestion q;
                            q.id = OUString::fromUtf8(extractJsonString(qStr, "id").c_str());
                            q.question = OUString::fromUtf8(extractJsonString(qStr, "question").c_str());
                            q.defaultValue = OUString::fromUtf8(extractJsonString(qStr, "default").c_str());

                            // Parse options array
                            size_t optStart = qStr.find("\"options\":");
                            if (optStart != std::string::npos && qStr.find("null", optStart) > optStart + 15) {
                                size_t optArrStart = qStr.find("[", optStart);
                                size_t optArrEnd = qStr.find("]", optArrStart);
                                if (optArrStart != std::string::npos && optArrEnd != std::string::npos) {
                                    std::string optStr = qStr.substr(optArrStart + 1, optArrEnd - optArrStart - 1);
                                    size_t pos = 0;
                                    while ((pos = optStr.find("\"", pos)) != std::string::npos) {
                                        size_t end = optStr.find("\"", pos + 1);
                                        if (end != std::string::npos) {
                                            std::string opt = optStr.substr(pos + 1, end - pos - 1);
                                            q.options.push_back(OUString::fromUtf8(opt.c_str()));
                                            pos = end + 1;
                                        } else {
                                            break;
                                        }
                                    }
                                }
                            }

                            response.clarificationQuestions.push_back(q);
                            objStart = objEnd;
                        } else {
                            break;
                        }
                    }
                }
            }
        }
        if (dbg) fprintf(dbg, "  Parsed %d clarification questions\n", (int)response.clarificationQuestions.size());
    }

    if (dbg) fclose(dbg);

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
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);  // 10 seconds to connect
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);  // 120 seconds for full request (LLM can be slow)
    
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

AgentResponse AgentConnection::sendMessageWithClarification(const OUString& message, const OUString& documentContent, const OUString& selection, const std::map<OUString, OUString>& clarificationAnswers) {
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

    // Build JSON payload with clarification answers
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
    jsonBuf.append("},");

    // Add clarification_answers object
    jsonBuf.append("\"clarification_answers\":{");
    bool first = true;
    for (const auto& pair : clarificationAnswers) {
        if (!first) jsonBuf.append(",");
        jsonBuf.append("\"");
        jsonBuf.append(escapeJsonString(pair.first));
        jsonBuf.append("\":\"");
        jsonBuf.append(escapeJsonString(pair.second));
        jsonBuf.append("\"");
        first = false;
    }
    jsonBuf.append("}");

    jsonBuf.append("}");

    std::string payload = jsonBuf.makeStringAndClear().toUtf8().getStr();
    std::string url = m_backendUrl + "/api/chat";

    // Debug: log the payload
    FILE* dbg = fopen("C:\\temp\\clarification_request.log", "w");
    if (dbg) {
        fprintf(dbg, "Sending clarification request:\n%s\n", payload.c_str());
        fclose(dbg);
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 180L);  // 180 seconds for document generation

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
