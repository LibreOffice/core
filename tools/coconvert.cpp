// -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*-
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <locale>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <vector>

#include <Poco/Path.h>
#include <Poco/URI.h>

#include <COKit/COKit.hxx>
#include <COKit/COKitInit.h>

#if defined _WIN32
#include <Windows.h>
#include <common/Util.hpp>
#endif

namespace
{
// What to convert the input files to, and where to put the results.
struct ConversionOptions
{
    // The format (extension) to convert to, eg. pdf or odt.
    std::string format;
    // The options of the export filter, for example a JSON object, or empty when there are none.
    std::string filterOptions;
    // The folder of the output files, or empty for the current folder.
    std::string outputDirectory;
};

// Read the format argument, format[:filter][:options], into the output format and the filter
// options. Filter and options are optional, if there are only two parameters, and the second
// parameter is JSON, it is interpreted as options, otherwise as filter.
void parseFormatArgument(std::string_view argument, ConversionOptions& options)
{
    const std::size_t formatEnd = argument.find(':');
    options.format = argument.substr(0, formatEnd);
    options.filterOptions.clear();
    if (formatEnd == std::string_view::npos)
        return;

    const std::string_view rest = argument.substr(formatEnd + 1);
    if (rest.starts_with('{') && rest.ends_with('}'))
    {
        options.filterOptions = rest;
        return;
    }

    const std::size_t filterEnd = rest.find(':');
    if (filterEnd != std::string_view::npos)
        options.filterOptions = rest.substr(filterEnd + 1);
}

// A regular expression that matches file names in the form the file system stores them: UTF-16 on
// Windows and UTF-8 bytes elsewhere.
using FileNameRegex = std::basic_regex<std::filesystem::path::value_type>;

// Turn a file name pattern into a regular expression where '*' stands for any run of characters and
// '?' for any single character. Every other character matches only itself.
FileNameRegex wildcardToRegex(const std::filesystem::path::string_type& pattern)
{
    std::filesystem::path::string_type expression;
    for (const auto character : pattern)
    {
        if (character == '*')
        {
            expression += '.';
            expression += '*';
        }
        else if (character == '?')
        {
#ifdef _WIN32
            expression += '.';
#else
            // One UTF-8 character is a byte that is not a continuation byte, then any continuation
            // bytes.
            expression += "[^\\x80-\\xBF][\\x80-\\xBF]*";
#endif
        }
        else
        {
            const auto code =
                static_cast<std::make_unsigned_t<std::filesystem::path::value_type>>(character);
            if (code < 0x80 && std::string_view("\\^$.|+()[]{}").find(static_cast<char>(code)) !=
                                   std::string_view::npos)
                expression += '\\';
            expression += character;
        }
    }
#ifdef _WIN32
    // File names on Windows ignore case. The case folding follows the user's locale, so it covers
    // letters beyond ASCII too.
    FileNameRegex regex;
    regex.imbue(std::locale(""));
    regex.assign(expression, std::regex::ECMAScript | std::regex::icase);
    return regex;
#else
    return FileNameRegex(expression);
#endif
}

// Expand a file name pattern such as *.odt into the sorted list of the files it matches. The
// wildcards work in the last part of the path only. Hidden files match only a pattern that starts
// with a dot, as in a shell. The Windows command line hands patterns over as they are typed, so
// this expansion is what makes them work there.
std::vector<std::string> expandPattern(const std::string& argument)
{
    const std::filesystem::path path(argument);
    const std::filesystem::path::string_type pattern = path.filename().native();
    std::error_code error;
    if ((pattern.find('*') == pattern.npos && pattern.find('?') == pattern.npos) ||
        std::filesystem::exists(path, error))
        return { argument };

    const FileNameRegex expression = wildcardToRegex(pattern);
    const bool hasDirectory = path.has_parent_path();
    std::vector<std::string> matches;
    for (const auto& entry :
         std::filesystem::directory_iterator(hasDirectory ? path.parent_path() : ".", error))
    {
        const std::filesystem::path name = entry.path().filename();
        if ((name.native()[0] == '.' && pattern[0] != '.') || !entry.is_regular_file(error) ||
            !std::regex_match(name.native(), expression))
            continue;
        matches.push_back((hasDirectory ? entry.path() : name).string<char>());
    }
    std::sort(matches.begin(), matches.end());
    return matches;
}

struct Conversion
{
    std::string input;
    std::string output;
};

// Return whether two paths name the same file. The file system answers this, so case folding, links
// and case-sensitive folders all follow its own rules. When neither path exists, the file system
// has no answer, and two paths that are the same after normalization count as the same file.
bool isSameFile(const std::filesystem::path& first, const std::filesystem::path& second)
{
    std::error_code error;
    const bool same = std::filesystem::equivalent(first, second, error);
    if (!error)
        return same;
    std::error_code firstError;
    std::error_code secondError;
    return std::filesystem::absolute(first, firstError).lexically_normal() ==
           std::filesystem::absolute(second, secondError).lexically_normal();
}

// Pair each input file with its output file. An input that is listed more than once is converted
// once. A conversion is left out with an error when its output is one of the input files, so a
// pattern such as a* never converts a file that the same run writes.
std::vector<Conversion> planConversions(const std::vector<std::string>& inputs,
                                        const ConversionOptions& options, bool& failed)
{
    std::vector<std::string> uniqueInputs;
    for (const std::string& input : inputs)
    {
        if (std::none_of(uniqueInputs.begin(), uniqueInputs.end(),
                         [&input](const std::string& unique) { return isSameFile(input, unique); }))
            uniqueInputs.push_back(input);
    }

    std::vector<Conversion> conversions;
    for (const std::string& input : uniqueInputs)
    {
        std::filesystem::path outputPath =
            std::filesystem::path(options.outputDirectory) / std::filesystem::path(input).stem();
        outputPath += "." + options.format;
        const std::string output = outputPath.string<char>();

        std::error_code error;
        if (std::filesystem::exists(outputPath, error) &&
            std::any_of(uniqueInputs.begin(), uniqueInputs.end(),
                        [&outputPath](const std::string& other)
                        { return isSameFile(outputPath, other); }))
        {
            std::cerr << "Error: skipping " << input << ", because its output " << output
                      << " is one of the input files\n";
            failed = true;
            continue;
        }

        conversions.push_back({ input, output });
    }
    return conversions;
}

// Return the first conversion before the one at index that has the same output file, or nullptr
// when there is none. The file system compares only outputs that exist. An output that does not
// exist yet matches only an output with the same normalized path.
const Conversion* findEarlierOutput(const std::vector<Conversion>& conversions, std::size_t index)
{
    for (std::size_t earlier = 0; earlier < index; ++earlier)
    {
        if (isSameFile(conversions[index].output, conversions[earlier].output))
            return &conversions[earlier];
    }
    return nullptr;
}

bool convert(COKit* office, const Conversion& conversion, const ConversionOptions& options)
{
    const std::string& input = conversion.input;
    const std::string& output = conversion.output;

    std::cout << "convert " << input << " -> " << output << "\n";

    try
    {
        std::unique_ptr<COKitDocument> doc(office->documentLoadWithOptions(
            Poco::URI(Poco::Path(input)).toString().c_str(), nullptr /* options */));
        if (!doc)
        {
            std::cerr << "Error: could not load document: " << office->getError().c_str() << "\n";
            return false;
        }

        if (!doc->saveAs(Poco::URI(Poco::Path(output)).toString().c_str(), options.format.c_str(),
                         options.filterOptions.empty() ? nullptr : options.filterOptions.c_str()))
        {
            std::cerr << "Error: could not export document as " << options.format << ": "
                      << office->getError().c_str() << "\n";
            return false;
        }
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: COKit exception while converting " << input << ": " << e.what()
                  << "\n";
        return false;
    }
}

void printUsage(const std::string& programName)
{
#ifdef _WIN32
    const char* const outputDirectoryExample = R"(C:\Users\user\Documents)";
    const char* const filterOptionsExample =
        R"("pdf:writer_pdf_Export:{\"PageRange\":{\"type\":\"string\",\"value\":\"2-3\"}}")";
    const char* const jsonOnlyExample =
        R"("pdf:{\"PageRange\":{\"type\":\"string\",\"value\":\"2-3\"}}")";
    const char* const quote = "double";
#else
    const char* const outputDirectoryExample = "/home/user";
    const char* const filterOptionsExample =
        R"('pdf:writer_pdf_Export:{"PageRange":{"type":"string","value":"2-3"}}')";
    const char* const jsonOnlyExample = R"('pdf:{"PageRange":{"type":"string","value":"2-3"}}')";
    const char* const quote = "single";
#endif

    std::cout
        << "Usage: " << programName
        << " OutputFileExtension[:OutputFilterName][:FilterOptions] \\\n"
        << std::string(programName.size() + 8, ' ') << "[--outdir output_dir] file ...\n"
           "\n"
           "Convert each file to the format that OutputFileExtension names, for example pdf, docx\n"
           "or ods. A file can also be a pattern such as *.docx.\n"
           "\n"
           "OutputFilterName is accepted, but has no effect for now: the filter is chosen from\n"
           "the extension and the type of the document. FilterOptions are passed to the export\n"
           "filter. When they are a JSON object, the filter name and its colon can be left out.\n"
           "Put "
        << quote
        << " quotes around the whole argument when it contains spaces or JSON.\n"
           "\n"
           "  --outdir output_dir  The folder for the output files. It is created if it is\n"
           "                       missing. Without it, the current folder is used. If it is\n"
           "                       given more than once, only its last value is effective.\n"
           "  --convert-to, --convert-images-to mime_type\n"
           "                       Accepted for compatibility with soffice, and ignored. The\n"
           "                       output format is the first argument that is not an option.\n"
           "\n"
           "For example:\n"
           "  " << programName << " pptx *.odp\n"
           "  " << programName << " docx --outdir " << outputDirectoryExample << " *.doc\n"
           "  " << programName << " " << filterOptionsExample << " MyDocument.odt\n"
           "  " << programName << " " << jsonOnlyExample << " MyDocument.odt\n";
}

// Read the value of the option at argv[index], either from the rest of the same argument, as in
// --outdir=output, or from the next argument, in which case index moves past it.
std::optional<std::string> readOptionValue(int argc, char** argv, int& index,
                                           std::string_view longName)
{
    const std::string_view argument = argv[index];
    if (argument == longName)
    {
        if (index + 1 == argc)
        {
            std::cerr << "Error: " << argument << " needs a value\n";
            std::exit(1);
        }
        return std::string(argv[++index]);
    }
    if (argument.starts_with(longName) && argument.size() > longName.size() &&
        argument[longName.size()] == '=')
        return std::string(argument.substr(longName.size() + 1));
    return std::nullopt;
}

#ifdef _WIN32
// The console output code page from before the program changed it, or 0 when the program left it
// as it was. The console belongs to the shell that started the program too.
UINT previousConsoleOutputCodePage = 0;

void restoreConsoleOutputCodePage()
{
    if (previousConsoleOutputCodePage)
        SetConsoleOutputCP(previousConsoleOutputCodePage);
}

// Put the code page back when Ctrl+C, Ctrl+Break or closing the console ends the program, and then
// let the default handler end it.
BOOL WINAPI handleConsoleControl(DWORD)
{
    restoreConsoleOutputCodePage();
    return FALSE;
}
#endif

} // namespace

// On Windows, we deploy the .exe file with the magic that makes the program use CP_UTF8 as its
// codepage, so command-line arguments come in as UTF-8, which nicely matches what happens on Linux
// and macOS.

int main(int argc, char** argv)
{
#if defined _WIN32
    // File names in the messages are UTF-8. The console shows them correctly only when its output
    // code page is UTF-8 too. The earlier code page comes back when main returns, when std::exit
    // ends the program, and when the console stops it. GetConsoleOutputCP returns 0 when there is
    // no console.
    const UINT consoleOutputCodePage = GetConsoleOutputCP();
    if (consoleOutputCodePage && consoleOutputCodePage != CP_UTF8)
    {
        previousConsoleOutputCodePage = consoleOutputCodePage;
        SetConsoleOutputCP(CP_UTF8);
        std::atexit(restoreConsoleOutputCodePage);
        SetConsoleCtrlHandler(handleConsoleControl, TRUE);
    }

#elif defined __APPLE__

#else

#endif

    const std::string programName
        = argc > 0 ? std::filesystem::path(argv[0]).filename().string<char>() : "coconvert";
    ConversionOptions options;
    bool hasFormatArgument = false;
    std::vector<std::string> inputs;
    bool hasFileArgument = false;
    bool failed = false;
    bool optionsEnded = false;
    for (int i = 1; i < argc; i++)
    {
        const std::string argument = argv[i];
        if (!optionsEnded && argument.starts_with("-"))
        {
            if (argument == "--")
                optionsEnded = true;
            else if (auto outputDirectory = readOptionValue(argc, argv, i, "--outdir"))
                options.outputDirectory = *outputDirectory;
            // These two options of soffice are accepted and ignored. The value after --convert-to
            // is the first argument that is not an option, so it becomes the format argument.
            else if (argument == "--convert-to")
                std::cerr << "Note: ignoring --convert-to, the output format is the first argument"
                             " that is not an option\n";
            else if (auto imageType = readOptionValue(argc, argv, i, "--convert-images-to"))
                std::cerr << "Note: ignoring --convert-images-to " << *imageType << "\n";
            else if (argument == "-h" || argument == "--help")
            {
                printUsage(programName);
                return 0;
            }
            else
            {
                std::cerr << "Error: unknown option " << argument << "\n";
                printUsage(programName);
                return 1;
            }
            continue;
        }

        // The first argument that is not an option is the format, as in soffice --convert-to.
        if (!hasFormatArgument)
        {
            hasFormatArgument = true;
            parseFormatArgument(argument, options);
            if (options.format.empty())
            {
                std::cerr << "Error: no output format before the first colon in " << argument
                          << "\n";
                return 1;
            }
            continue;
        }

        if (argument.empty())
        {
            std::cerr << "Error: a file name cannot be empty\n";
            return 1;
        }

        hasFileArgument = true;
        const std::vector<std::string> matches = expandPattern(argument);
        if (matches.empty())
        {
            std::cerr << "Error: no file matches " << argument << "\n";
            failed = true;
        }
        inputs.insert(inputs.end(), matches.begin(), matches.end());
    }

    if (!hasFormatArgument || !hasFileArgument)
    {
        printUsage(programName);
        return 1;
    }

    const std::vector<Conversion> conversions = planConversions(inputs, options, failed);
    if (conversions.empty())
        return 1;

    if (!options.outputDirectory.empty())
    {
        std::error_code error;
        std::filesystem::create_directories(options.outputDirectory, error);
        if (error)
        {
            std::cerr << "Error: could not create " << options.outputDirectory << ": "
                      << error.message() << "\n";
            return 1;
        }
    }

    std::unique_ptr<COKit> office;
    try
    {
        std::string kit_path;

#if defined _WIN32
        wchar_t fileName[1000];
        GetModuleFileNameW(NULL, fileName, sizeof(fileName) / sizeof(fileName[0]));

        // CoConvert.exe is put in the "program" folder so kit_path can be that folder.

        kit_path = Util::wide_string_to_string(std::wstring(fileName));
        kit_path.resize(kit_path.find_last_of('\\') + 1);

#elif defined __APPLE__

#else

        // The engine installation configured at build time, the "program"
        // folder of which holds the engine libraries.
        kit_path = LO_PATH "/program";

#endif

        office.reset(kit::kit_cpp_init(kit_path.c_str()));
        if (!office)
        {
            std::cerr << "Error: could not initialize COKit\n";
            return 1;
        }

        // Each output is compared with the earlier outputs just before its conversion, when the
        // earlier outputs exist and the file system can compare them. Two inputs that differ only
        // in extension, or only in case on a file system that ignores case, then do not overwrite
        // each other.
        for (std::size_t i = 0; i < conversions.size(); ++i)
        {
            if (const Conversion* earlier = findEarlierOutput(conversions, i))
            {
                std::cerr << "Error: skipping " << conversions[i].input << ", because "
                          << earlier->input << " is converted to the same output "
                          << conversions[i].output << "\n";
                failed = true;
                continue;
            }
            if (!convert(office.get(), conversions[i], options))
                failed = true;
        }

        // Shut the engine down while its libraries are still fully loaded.
        office.reset();

        // The engine libraries register exit-time destructors that expect a
        // kit session, which a command-line conversion never creates.
        // std::_Exit ends the process without running them.
        std::cout.flush();
#if defined _WIN32
        restoreConsoleOutputCodePage();
#endif
        std::_Exit(failed ? 1 : 0);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: COKit exception: " << e.what() << "\n";
        return 1;
    }
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
