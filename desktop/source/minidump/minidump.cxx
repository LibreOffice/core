/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <desktop/minidump.hxx>
#include <sal/log.hxx>

#include <map>
#include <fstream>
#include <sstream>
#include <string>

#include <curl/curl.h>

#include <systools/curlinit.hxx>

#ifdef _WIN32
#include <memory>
#include <windows.h>
#endif

const char kUserAgent[] = "Breakpad/1.0 (Linux)";

static std::map<std::string, std::string> readStrings(std::istream& file)
{
    std::map<std::string, std::string> parameters;

    // when file is not readable, the status eof would not be set
    // better test of state is okay
    while (file)
    {
        std::string line;
        std::getline(file, line);
        int sep = line.find('=');
        if (sep >= 0)
        {
            std::string key = line.substr(0, sep);
            std::string value = line.substr(sep + 1);
            parameters[key] = value;
        }
    }

    return parameters;
}

// Callback to get the response data from server.
static size_t WriteCallback(void const *ptr, size_t size,
                            size_t nmemb, void *userp)
{
  if (!userp)
    return 0;

  std::string* response = static_cast<std::string *>(userp);
  size_t real_size = size * nmemb;
  response->append(static_cast<char const *>(ptr), real_size);
  return real_size;
}

static void getProperty(const std::string& key, std::string& value,
        std::map<std::string, std::string>& parameters)
{
    auto itr = parameters.find(key);
    if (itr != parameters.end())
    {
        value = itr->second;
        parameters.erase(itr);
    }
}

static std::string generate_json(const std::map<std::string, std::string>& parameters)
{
    std::ostringstream stream;
    stream << "{\n";
    bool first = true;
    for (auto itr = parameters.begin(), itrEnd = parameters.end(); itr != itrEnd; ++itr)
    {
        if (!first)
        {
            stream << ",\n";
        }
        first = false;
        stream << "\"" << itr->first << "\": \"" << itr->second << "\"";
    }
    stream << "\n}";

    return stream.str();
}

static bool uploadContent(std::map<std::string, std::string>& parameters, std::string& response)
{
    CURL* curl = curl_easy_init();
    if (!curl)
        return false;

    ::InitCurl_easy(curl);

    std::string proxy, proxy_user_pwd, ca_certificate_file, file, url, version;

    getProperty("Proxy", proxy, parameters);
    getProperty("ProxyUserPW", proxy_user_pwd, parameters);
    getProperty("CAFile", ca_certificate_file, parameters);

    getProperty("DumpFile", file, parameters);
    getProperty("URL", url, parameters);
    getProperty("Version", version, parameters);
    if (url.empty())
        return false;

    if (file.empty())
        return false;

    if (version.empty())
        return false;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, kUserAgent);
    // Set proxy information if necessary.
    if (!proxy.empty())
    {
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());

        curl_easy_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_ANYSAFE);

        if (!proxy_user_pwd.empty())
            curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, proxy_user_pwd.c_str());
        else
            curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, ":");
    }

    if (!ca_certificate_file.empty())
        curl_easy_setopt(curl, CURLOPT_CAINFO, ca_certificate_file.c_str());

    curl_mime* mime = curl_mime_init(curl);
    std::string additional_data = generate_json(parameters);
    curl_mimepart* part = curl_mime_addpart(mime);
    curl_mime_name(part, "AdditionalData");
    curl_mime_data(part, additional_data.c_str(), CURL_ZERO_TERMINATED);

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "Version");
    curl_mime_data(part, version.c_str(), CURL_ZERO_TERMINATED);

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "upload_file_minidump");
    curl_mime_filedata(part, file.c_str());

    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);


    // Disable 100-continue header.
    char buf[] = "Expect:";
    curl_slist* headerlist = nullptr;
    headerlist = curl_slist_append(headerlist, buf);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    std::string response_body;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,
            static_cast<void *>(&response_body));

    // Fail if 400+ is returned from the web server.
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

    CURLcode cc = curl_easy_perform(curl);
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    SAL_WARN_IF(cc != CURLE_OK, "desktop",
            "Failed to send http request to " <<
            url.c_str() <<
            ", error: " <<
            curl_easy_strerror(cc));

    if (headerlist != nullptr)
    {
        curl_slist_free_all(headerlist);
    }

    response = response_body;

    if( CURLE_OK != cc )
    {
        if (response.empty())
        {
            response = curl_easy_strerror(cc);
        }
        return false;
    }

    return true;
}

namespace crashreport {

bool readConfig(const std::string& iniPath, std::string * response)
{
#if defined _WIN32
    std::wstring iniPathW;
    const int nChars = MultiByteToWideChar(CP_UTF8, 0, iniPath.c_str(), -1, nullptr, 0);
    auto buf = std::make_unique<wchar_t[]>(nChars);
    if (MultiByteToWideChar(CP_UTF8, 0, iniPath.c_str(), -1, buf.get(), nChars) != 0)
        iniPathW = buf.get();

    std::ifstream file = iniPathW.empty() ? std::ifstream(iniPath) : std::ifstream(iniPathW);
#else
    std::ifstream file(iniPath);
#endif
    std::map<std::string, std::string> parameters = readStrings(file);

    // make sure that at least the mandatory parameters are in there
    if (parameters.find("DumpFile") == parameters.end())
    {
        if(response != nullptr)
            *response = "ini file needs to contain a key DumpFile!";
        return false;
    }

    if (parameters.find("Version") == parameters.end())
    {
        if (response != nullptr)
            *response = "ini file needs to contain a key Version!";
        return false;
    }

    if (parameters.find("URL") == parameters.end())
    {
        if (response != nullptr)
            *response = "ini file needs to contain a key URL!";
        return false;
    }

    if (response != nullptr)
        return uploadContent(parameters, *response);

    return true;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
