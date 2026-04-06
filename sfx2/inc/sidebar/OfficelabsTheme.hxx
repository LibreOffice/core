/* OfficeLabs theme helper — reads theme from config file */
#pragma once

#include <tools/color.hxx>
#include <fstream>
#include <string>

#ifdef _WIN32
#include <shlobj.h>
#include <windows.h>
#endif

namespace sfx2::sidebar {

enum class OLTheme { Light, MidnightBlue, Dark };

inline OLTheme GetOLTheme()
{
    // Read from <instdir>/share/officelabs_theme.txt
    // Fall back to midnight-blue if file missing
    std::string theme = "midnight-blue";

#ifdef _WIN32
    // Get the path to soffice.exe, go up to instdir
    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    std::wstring ws(exePath);
    auto pos = ws.rfind(L"\\program\\");
    if (pos != std::wstring::npos)
    {
        std::wstring confPath = ws.substr(0, pos) + L"\\share\\officelabs_theme.txt";
        std::ifstream f(confPath);
        if (f.is_open())
        {
            std::getline(f, theme);
            // Trim whitespace
            while (!theme.empty() && (theme.back() == '\r' || theme.back() == '\n' || theme.back() == ' '))
                theme.pop_back();
        }
    }
#endif

    if (theme == "light")
        return OLTheme::Light;
    if (theme == "dark")
        return OLTheme::Dark;
    return OLTheme::MidnightBlue;
}

struct OLColors
{
    Color bg;
    Color surface;
    Color border;
    Color text;
    Color subtext;
};

inline OLColors GetOLColors()
{
    switch (GetOLTheme())
    {
        case OLTheme::Light:
            return { Color(0xFA, 0xFA, 0xFA),   // bg
                     Color(0xF0, 0xF1, 0xF3),   // surface
                     Color(0xD0, 0xD0, 0xD0),   // border — darker for ruler contrast
                     Color(0x1A, 0x1A, 0x1A),   // text
                     Color(0x8B, 0x8B, 0x8B) };  // subtext
        case OLTheme::Dark:
            return { Color(0x1A, 0x1A, 0x1A),   // bg
                     Color(0x2A, 0x2A, 0x2A),   // surface
                     Color(0x3A, 0x3A, 0x3A),   // border
                     Color(0xFF, 0xFF, 0xFF),   // text
                     Color(0x77, 0x77, 0x77) };  // subtext
        case OLTheme::MidnightBlue:
        default:
            return { Color(0x21, 0x22, 0x2C),   // bg — VS Code Dracula sidebar
                     Color(0x28, 0x2A, 0x36),   // surface — Dracula editor bg
                     Color(0x44, 0x47, 0x5A),   // border
                     Color(0xF8, 0xF8, 0xF2),   // text
                     Color(0x62, 0x72, 0xA4) };  // subtext
    }
}

} // namespace
