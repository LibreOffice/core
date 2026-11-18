/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <Poco/DirectoryIterator.h>
#include <Poco/Exception.h>
#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Path.h>
#include <Poco/StreamCopier.h>

#include <config.h>

#include <common/Log.hpp>
#include <common/Uri.hpp>
#include <wsd/Extensions.hpp>

std::vector<std::pair<std::string, std::string>> Extensions::enumerateGasScripts(
    Poco::Path const & dir)
{
    std::vector<std::pair<std::string, std::string>> scripts;
    for (Poco::DirectoryIterator it(dir), end; it != end; ++it) {
        if (!it->isFile() || it->isLink()) {
            continue;
        }
        std::string const & name = it.name();
        if (name.starts_with('.') || !(name.ends_with(".gs") || name.ends_with(".js"))) {
            continue;
        }
        Poco::FileInputStream stream(it->path());
        std::string src;
        Poco::StreamCopier::copyToString(stream, src);
        scripts.emplace_back(name, std::move(src));
    }
    std::sort(scripts.begin(), scripts.end());
    return scripts;
}

static std::string jsonQuote(std::string const & s) {
    std::string out;
    out.reserve(s.size() + 2);
    out.push_back('"');
    for (char c : s) {
        switch (c) {
        case '"': out.append("\\\""); break;
        case '\\': out.append("\\\\"); break;
        case '\b': out.append("\\b"); break;
        case '\f': out.append("\\f"); break;
        case '\n': out.append("\\n"); break;
        case '\r': out.append("\\r"); break;
        case '\t': out.append("\\t"); break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                char buf[8];
                std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                out.append(buf);
            } else {
                out.push_back(c);
            }
            break;
        }
    }
    out.push_back('"');
    return out;
}

namespace {

bool isDigit(char c) { return c >= '0' && c <= '9'; }

// The characters that \w matches in a JavaScript regular expression:
bool isWordCharacter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || isDigit(c) || c == '_';
}

bool isIdentifierCharacter(char c) { return isWordCharacter(c) || c == '$'; }

// The ASCII characters that \s matches in a JavaScript regular expression:
bool isSpace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

// The names of the functions that the scripts declare (as "function name(", each name once, in the
// order of first occurrence):
std::vector<std::string> declaredFunctions(
    std::vector<std::pair<std::string, std::string>> const & scripts)
{
    std::vector<std::string> functions;
    std::set<std::string> seen;
    for (auto const & [name, src]: scripts) {
        std::string_view const keyword = "function";
        for (auto i = src.find(keyword); i != std::string::npos; i = src.find(keyword, i + 1)) {
            if (i != 0 && isWordCharacter(src[i - 1])) {
                continue;
            }
            auto j = i + keyword.size();
            auto const afterKeyword = j;
            while (j != src.size() && isSpace(src[j])) {
                ++j;
            }
            if (j == afterKeyword || j == src.size() || !isIdentifierCharacter(src[j])
                || isDigit(src[j]))
            {
                continue;
            }
            auto const start = j;
            while (j != src.size() && isIdentifierCharacter(src[j])) {
                ++j;
            }
            auto const end = j;
            while (j != src.size() && isSpace(src[j])) {
                ++j;
            }
            if (j == src.size() || src[j] != '(') {
                continue;
            }
            std::string function(src, start, end - start);
            if (seen.insert(function).second) {
                functions.push_back(std::move(function));
            }
        }
    }
    return functions;
}

}

std::string Extensions::synthesizeGasSidecar(
    std::vector<std::pair<std::string, std::string>> const & scripts)
{
    // Guess the add-on's target document types from the DocumentApp/SpreadsheetApp/SlidesApp
    // mentions:
    std::vector<std::string> supports;
    auto containsAnySource = [&scripts](std::string_view needle) {
        for (auto const & [name, src]: scripts) {
            if (src.find(needle) != std::string::npos) {
                return true;
            }
        }
        return false;
    };
    if (containsAnySource("DocumentApp")) {
        supports.push_back("text");
    }
    if (containsAnySource("SpreadsheetApp")) {
        supports.push_back("spreadsheet");
    }
    if (containsAnySource("SlidesApp")) {
        supports.push_back("presentation");
    }

    // Guess a display name from setTitle("..."), a NAME_TITLE = "..." constant, or the name an
    // addMenu("...", ...) call gives its own menu:
    std::string displayName;
    static const std::regex reSetTitle(R"RE(setTitle\s*\(\s*(?:'([^']+)'|"([^"]+)"))RE");
    static const std::regex reTitleConst(
        R"RE([A-Za-z_][A-Za-z0-9_]*_TITLE\s*=\s*(?:'([^']+)'|"([^"]+)"))RE");
    static const std::regex reMenuName(R"RE(addMenu\s*\(\s*(?:'([^']+)'|"([^"]+)"))RE");
    auto tryMatch = [&scripts](std::regex const & re) -> std::string {
        for (auto const & [name, src]: scripts) {
            std::smatch m;
            if (std::regex_search(src, m, re)) {
                return m[1].matched ? m[1].str() : m[2].str();
            }
        }
        return std::string();
    };
    displayName = tryMatch(reSetTitle);
    if (displayName.empty()) {
        displayName = tryMatch(reTitleConst);
    }
    if (displayName.empty()) {
        displayName = tryMatch(reMenuName);
    }

    std::string body = "{\"scripts\":[";
    bool firstScript = true;
    for (auto const & [name, src]: scripts) {
        if (!firstScript) {
            body.push_back(',');
        }
        firstScript = false;
        body.append(jsonQuote(name));
    }
    body.push_back(']');
    body.append(",\"functions\":[");
    bool firstFunction = true;
    for (auto const & function: declaredFunctions(scripts)) {
        if (!firstFunction) {
            body.push_back(',');
        }
        firstFunction = false;
        body.append(jsonQuote(function));
    }
    body.push_back(']');
    if (!displayName.empty()) {
        body.append(",\"name\":");
        body.append(jsonQuote(displayName));
    }
    if (!supports.empty()) {
        body.append(",\"supports\":[");
        bool firstSupport = true;
        for (auto const & s: supports) {
            if (!firstSupport) {
                body.push_back(',');
            }
            firstSupport = false;
            body.append(jsonQuote(s));
        }
        body.push_back(']');
    }
    body.push_back('}');
    return body;
}

namespace {

#if ENABLE_DEBUG
std::string builtinDirectory;
#endif

// A name of a file or directory in an extension, made of letters, digits, '.', '_' and '-' (so no
// ~new or ~old swap entry), not starting with '.' (so neither "." nor ".." nor a hidden file):
bool isPlainName(std::string_view name) {
    if (name.empty() || name.size() > 255 || name.front() == '.') {
        return false;
    }
    return std::all_of(name.begin(), name.end(), [](char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
            || c == '.' || c == '_' || c == '-';
    });
}

// A name made of ASCII letters, digits, '_' and '$', not starting with a digit:
bool isIdentifier(std::string_view name) {
    auto const isStart = [](char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '$';
    };
    if (name.empty() || name.size() > 255 || !isStart(name.front())) {
        return false;
    }
    return std::all_of(name.begin() + 1, name.end(), [&isStart](char c) {
        return isStart(c) || (c >= '0' && c <= '9');
    });
}

// The entry at the relative path (plain names separated by '/') within the directory dir, if it
// exists and is a directory or a file as asked, and neither it nor any directory on the way is a
// symbolic link:
std::optional<Poco::Path> entryWithin(
    Poco::Path const & dir, std::string_view relative, bool directory)
{
    Poco::Path current(dir);
    current.makeDirectory();
    for (;;) {
        auto const slash = relative.find('/');
        auto const segment = relative.substr(0, slash);
        if (!isPlainName(segment)) {
            return {};
        }
        Poco::Path entry(current);
        entry.setFileName(std::string(segment));
        Poco::File const file(entry);
        if (!file.exists() || file.isLink()) {
            return {};
        }
        if (slash == std::string_view::npos) {
            if (directory ? !file.isDirectory() : !file.isFile()) {
                return {};
            }
            return directory ? current.pushDirectory(std::string(segment)) : entry;
        }
        if (!file.isDirectory()) {
            return {};
        }
        current.pushDirectory(std::string(segment));
        relative.remove_prefix(slash + 1);
    }
}

std::string readFile(Poco::Path const & path) {
    Poco::FileInputStream stream(path.toString());
    std::string text;
    Poco::StreamCopier::copyToString(stream, text);
    return text;
}

// The JSON object in the file at the relative path within dir, or null:
Poco::JSON::Object::Ptr readJsonObject(Poco::Path const & dir, std::string_view relative) {
    auto const path = entryWithin(dir, relative, false);
    if (!path) {
        return nullptr;
    }
    try {
        return Poco::JSON::Parser().parse(readFile(*path)).extract<Poco::JSON::Object::Ptr>();
    } catch (Poco::Exception const & e) {
        LOG_WRN("Extensions: cannot read [" << path->toString() << "]: " << e.displayText());
        return nullptr;
    }
}

// The directory of the extension with the given ID, in the presets of the configuration configId
// (whose directory has the URI-encoded configId as its name), else among the built-in extensions
// that the built-in index.json lists (which only debug builds have):
std::optional<Poco::Path> findExtension(
    std::string const & childRoot, std::string const & configId, std::string const & id)
{
    if (!isPlainName(id)) {
        return {};
    }
    auto const configDirectory = Uri::encode(configId);
    if (!childRoot.empty() && !configDirectory.empty() && configDirectory != "."
        && configDirectory != ".." && configDirectory.find_first_of("/\\") == std::string::npos)
    {
        Poco::Path presets = Poco::Path::forDirectory(childRoot);
        presets.pushDirectory("tmp");
        presets.pushDirectory("sharedpresets");
        presets.pushDirectory(configDirectory);
        presets.pushDirectory("extensions");
        if (auto dir = entryWithin(presets, id, true)) {
            return dir;
        }
    }
#if ENABLE_DEBUG
    assert(!builtinDirectory.empty());
    Poco::Path const builtin = Poco::Path::forDirectory(builtinDirectory);
    auto const index = entryWithin(builtin, "index.json", false);
    if (!index) {
        return {};
    }
    try {
        auto const ids
            = Poco::JSON::Parser().parse(readFile(*index)).extract<Poco::JSON::Array::Ptr>();
        for (std::size_t i = 0; i != ids->size(); ++i) {
            if (ids->getElement<std::string>(i) == id) {
                return entryWithin(builtin, id, true);
            }
        }
    } catch (Poco::Exception const & e) {
        LOG_WRN("Extensions: cannot read [" << index->toString() << "]: " << e.displayText());
    }
#endif
    return {};
}

std::string jsonArray(std::vector<std::string> const & elements) {
    std::string out = "[";
    for (auto const & element: elements) {
        if (out.size() != 1) {
            out.push_back(',');
        }
        out.append(element);
    }
    out.push_back(']');
    return out;
}

}

std::pair<std::optional<std::vector<std::pair<std::string, std::string>>>, std::string>
Extensions::gasScripts(Poco::Path const & dir)
{
    try {
        auto const listing = readJsonObject(dir, "_cool-gas.json");
        auto const names = listing ? listing->getArray("scripts") : nullptr;
        if (!names) {
            return {Extensions::enumerateGasScripts(dir), ""};
        }
        std::vector<std::pair<std::string, std::string>> scripts;
        for (std::size_t i = 0; i != names->size(); ++i) {
            auto const name = names->getElement<std::string>(i);
            auto const path = entryWithin(dir, name, false);
            if (!path) {
                return {{}, "Apps Script file " + name + " is missing"};
            }
            scripts.emplace_back(name, readFile(*path));
        }
        return {std::move(scripts), ""};
    } catch (Poco::Exception const & e) {
        return {{}, e.displayText()};
    }
}

namespace {

// The JSON array of the libraries that the appsscript.json of an Apps Script add-on in root (or of
// one of its libraries) declares, each an object with its userSymbol, dir, scripts, sources and
// libraries, loaded from the add-on's _cool-gas-libraries directory together with the libraries
// that it declares in turn:
std::optional<std::string> gasLibraries(
    Poco::Path const & root, Poco::JSON::Object::Ptr const & appsscript, int depth,
    std::string & error)
{
    if (depth > 8) {
        error = "Apps Script libraries nest too deeply";
        return {};
    }
    auto const dependencies = appsscript->getObject("dependencies");
    auto const declared = dependencies ? dependencies->getArray("libraries") : nullptr;
    std::vector<std::string> libraries;
    for (std::size_t i = 0; declared && i != declared->size(); ++i) {
        auto const library = declared->getObject(i);
        if (!library) {
            error = "malformed Apps Script library declaration";
            return {};
        }
        auto const userSymbol = library->optValue<std::string>("userSymbol", "");
        auto const libraryId = library->optValue<std::string>("libraryId", "");
        std::string const version = library->optValue<bool>("developmentMode", false)
            ? std::string("HEAD")
            : library->has("version") ? library->get("version").toString() : std::string();
        if (!isIdentifier(userSymbol) || !isPlainName(libraryId) || !isPlainName(version)) {
            error = "malformed Apps Script library declaration";
            return {};
        }
        std::string const relative = "_cool-gas-libraries/" + libraryId + '/' + version;
        auto const dir = entryWithin(root, relative, true);
        if (!dir) {
            LOG_WRN("Extensions: Apps Script library " << userSymbol << " is missing from "
                                                       << relative);
            continue;
        }
        auto const manifest = readJsonObject(*dir, "appsscript.json");
        if (!manifest) {
            error = "Apps Script library " + userSymbol + " has no appsscript.json";
            return {};
        }
        auto const nested = gasLibraries(root, manifest, depth + 1, error);
        if (!nested) {
            return {};
        }
        auto const [files, filesError] = Extensions::gasScripts(*dir);
        if (!files) {
            error = filesError;
            return {};
        }
        std::vector<std::string> scripts;
        std::vector<std::string> sources;
        for (auto const & [name, source]: *files) {
            scripts.push_back(jsonQuote(name));
            sources.push_back(jsonQuote(source));
        }
        libraries.push_back(
            "{\"userSymbol\":" + jsonQuote(userSymbol) + ",\"dir\":" + jsonQuote(relative + '/')
            + ",\"scripts\":" + jsonArray(scripts) + ",\"sources\":" + jsonArray(sources)
            + ",\"libraries\":" + *nested + '}');
    }
    return jsonArray(libraries);
}

}

std::optional<Extensions::CallScript> Extensions::makeCallScript(
    std::string const & childRoot, std::string const & configId, std::string const & id,
    std::string const & kind, std::string const & name, std::string & error)
{
    try {
        auto const dir = findExtension(childRoot, configId, id);
        if (!dir) {
            error = "no extension " + id;
            return {};
        }
        if (kind == "remote") {
            auto const manifest = readJsonObject(*dir, "manifest.json");
            if (!manifest || manifest->optValue<std::string>("manifestVersion", "") != "0.2") {
                error = "extension " + id + " has no usable manifest.json";
                return {};
            }
            std::vector<std::string> files;
            auto const remote = manifest->getArray("remote");
            for (std::size_t i = 0; remote && i != remote->size(); ++i) {
                if (!remote->get(i).isString()) {
                    error = "extension " + id + " has a malformed remote list";
                    return {};
                }
                files.push_back(remote->getElement<std::string>(i));
            }
            if (files.empty()) {
                error = "extension " + id + " has no remote files";
                return {};
            }
            // Each file runs on its own (so the lines and columns of its frames are its own) and
            // adds its functions to the remote object, and the script's value is the function that
            // passes the call's arguments on to the function of the remote object under the call's
            // name:
            std::string text = "const remote = {};\n";
            for (auto const & file: files) {
                auto const path = entryWithin(*dir, file, false);
                if (!path) {
                    error = "extension " + id + " has no remote file " + file;
                    return {};
                }
                text += "$internal.evalWithSource(" + jsonQuote(readFile(*path)) + ", "
                    + jsonQuote(id + '/' + file) + ", 1);\n";
            }
            return CallScript{
                "<remote-call>", 1,
                text + "(function (name) { return function (...args) {"
                " if (!Object.prototype.hasOwnProperty.call(remote, name)"
                " || typeof remote[name] !== 'function') {"
                " throw new TypeError(name + ' is not a remote function of the extension'); }"
                " return remote[name].apply(null, args); }; })(" + jsonQuote(name) + ')'};
        }
        if (kind == "gas") {
            // A manifest.json makes the directory a native extension, also when it has an
            // appsscript.json:
            auto const appsscript = readJsonObject(*dir, "appsscript.json");
            if (!appsscript || Poco::File(Poco::Path(*dir, "manifest.json")).exists()) {
                error = "extension " + id + " is no Apps Script add-on";
                return {};
            }
            auto const libraries = gasLibraries(*dir, appsscript, 0, error);
            if (!libraries) {
                return {};
            }
            auto const [files, filesError] = gasScripts(*dir);
            if (!files) {
                error = filesError;
                return {};
            }
            std::vector<std::string> names;
            std::vector<std::string> sources;
            for (auto const & [file, source]: *files) {
                names.push_back(jsonQuote(file));
                sources.push_back(jsonQuote(source));
            }
            return CallScript{
                "<gas-call>", 1,
                "(function (proxyId, callArgs) { return $internal.runGasAddOn(proxyId, "
                + jsonArray(sources) + ", " + jsonArray(names) + ", " + jsonQuote(name)
                + ", callArgs, " + jsonQuote(id) + ", " + *libraries + "); })"};
        }
        if (kind == "listen" || kind == "unlisten") {
            if (name != "selectionChanged" && name != "modified") {
                error = "no document event " + name;
                return {};
            }
            return CallScript{
                "<document-event>", 1,
                "(function (proxyId) { $internal."
                + std::string(kind == "listen" ? "listenToDocument" : "stopListeningToDocument")
                + '(' + jsonQuote(name) + ", proxyId); })"};
        }
        error = "unknown kind of call " + kind;
        return {};
    } catch (Poco::Exception const & e) {
        error = "extension " + id + " cannot be read: " + e.displayText();
        return {};
    }
}

std::optional<Poco::Path> Extensions::servableFile(
    Poco::Path const & extension, std::string const & file)
{
    // The path names a file and has a single spelling (each segment is a name other than "." and
    // "..", without a NUL character), and neither the extension's directory nor anything on the way
    // to the file is a symbolic link:
    Poco::Path current = Poco::Path::forDirectory(extension.toString());
    auto directoryName = current.toString();
    while (directoryName.size() > 1 && directoryName.ends_with('/')) {
        directoryName.pop_back();
    }
    if (!Poco::File(directoryName).exists() || Poco::File(directoryName).isLink()) {
        return {};
    }
    std::vector<std::string> segments;
    Poco::Path entry;
    for (std::string_view rest = file;;) {
        auto const slash = rest.find('/');
        auto const segment = rest.substr(0, slash);
        if (segment.empty() || segment == "." || segment == ".."
            || segment.find('\0') != std::string_view::npos)
        {
            return {};
        }
        segments.emplace_back(segment);
        entry = current;
        entry.setFileName(segments.back());
        if (!Poco::File(entry).exists() || Poco::File(entry).isLink()) {
            return {};
        }
        if (slash == std::string_view::npos) {
            if (!Poco::File(entry).isFile()) {
                return {};
            }
            break;
        }
        current.pushDirectory(segments.back());
        rest.remove_prefix(slash + 1);
    }

    // The code of an Apps Script add-on (its .gs and .js files, and whatever the "scripts" of a
    // _cool-gas.json lists in its own directory or a directory above it), unless a manifest.json
    // makes it a native extension:
    if (Poco::File(Poco::Path(extension, "appsscript.json")).exists()
        && !Poco::File(Poco::Path(extension, "manifest.json")).exists())
    {
        if (file.ends_with(".gs") || file.ends_with(".js")) {
            return {};
        }
        std::string directory;
        for (std::size_t i = 0; i != segments.size(); ++i) {
            std::string relative;
            for (std::size_t j = i; j != segments.size(); ++j) {
                relative += (j == i ? "" : "/") + segments[j];
            }
            auto const listing = readJsonObject(extension, directory + "_cool-gas.json");
            auto const scripts = listing ? listing->getArray("scripts") : nullptr;
            for (std::size_t k = 0; scripts && k != scripts->size(); ++k) {
                if (scripts->get(k).isString() && scripts->getElement<std::string>(k) == relative) {
                    return {};
                }
            }
            directory += segments[i] + '/';
        }
    }

    // The files that the manifest.json of a native extension lists as "remote" (and, as the
    // extension is unusable without a manifest, everything but an unreadable manifest.json):
    if (Poco::File(Poco::Path(extension, "manifest.json")).exists()) {
        auto const manifest = readJsonObject(extension, "manifest.json");
        if (!manifest) {
            return file == "manifest.json" ? std::optional(entry) : std::nullopt;
        }
        auto const remote = manifest->getArray("remote");
        for (std::size_t i = 0; remote && i != remote->size(); ++i) {
            if (remote->get(i).isString() && remote->getElement<std::string>(i) == file) {
                return {};
            }
        }
    }

    return entry;
}

#if ENABLE_DEBUG

void Extensions::synthesizeBuiltinExtensionsIndex(std::string const & distExtensionsDir) {
    builtinDirectory = distExtensionsDir;
    Poco::File extDir(distExtensionsDir);
    if (!extDir.exists() || !extDir.isDirectory()) {
        return;
    }

    std::set<std::string> nativeIds;
    std::set<std::string> gasIds;
    try {
        for (Poco::DirectoryIterator it(distExtensionsDir), end; it != end; ++it) {
            if (!it->isDirectory()) {
                continue;
            }
            std::string const & id = it.name();
            if (id.empty() || id.front() == '.') {
                continue;
            }
            Poco::Path subdir(it->path());
            Poco::Path native(subdir, "manifest.json");
            Poco::Path gas(subdir, "appsscript.json");
            if (Poco::File(native).exists()) {
                nativeIds.insert(id);
            } else if (Poco::File(gas).exists()) {
                gasIds.insert(id);
            }
        }
    } catch (Poco::Exception const & e) {
        LOG_WRN(
            "Extensions: failed to enumerate [" << distExtensionsDir
            << "]: " << e.displayText());
        return;
    }

    std::vector<std::string> allIds;
    allIds.reserve(nativeIds.size() + gasIds.size());
    allIds.insert(allIds.end(), nativeIds.begin(), nativeIds.end());
    allIds.insert(allIds.end(), gasIds.begin(), gasIds.end());
    std::sort(allIds.begin(), allIds.end());

    std::string indexJson = "[";
    bool first = true;
    for (auto const & id: allIds) {
        if (!first) {
            indexJson.push_back(',');
        }
        first = false;
        indexJson.append(jsonQuote(id));
    }
    indexJson.push_back(']');
    Poco::Path indexPath(distExtensionsDir);
    indexPath.append("index.json");
    try {
        Poco::FileOutputStream out(indexPath.toString());
        out.write(indexJson.data(), indexJson.size());
    } catch (Poco::Exception const & e) {
        LOG_WRN(
            "Extensions: failed to write [" << indexPath.toString()
            << "]: " << e.displayText());
    }

    // Stash a <id>/_cool-gas.json sidecar listing each Apps Script directory's scripts:
    for (auto const & id: gasIds) {
        Poco::Path dir = Poco::Path::forDirectory(distExtensionsDir);
        dir.pushDirectory(id);
        std::vector<std::pair<std::string, std::string>> scripts;
        try {
            scripts = enumerateGasScripts(dir);
        } catch (Poco::Exception const & e) {
            LOG_WRN(
                "Extensions: failed to enumerate [" << dir.toString()
                << "]: " << e.displayText());
            continue;
        }
        Poco::Path sidecarPath(dir);
        sidecarPath.append("_cool-gas.json");
        try {
            Poco::FileOutputStream out(sidecarPath.toString());
            std::string const body = synthesizeGasSidecar(scripts);
            out.write(body.data(), body.size());
        } catch (Poco::Exception const & e) {
            LOG_WRN(
                "Extensions: failed to write [" << sidecarPath.toString()
                << "]: " << e.displayText());
        }
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
