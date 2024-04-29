/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/lok.hxx>
#include <osl/process.h>
#include <i18nlangtag/languagetag.hxx>
#include <sal/log.hxx>

#include <iostream>

namespace comphelper::LibreOfficeKit
{

static bool g_bActive(false);

static bool g_bForkedChild(false);

static bool g_bPartInInvalidation(false);

static bool g_bTiledPainting(false);

static bool g_bDialogPainting(false);

static bool g_bTiledAnnotations(true);

static bool g_bRangeHeaders(false);

static bool g_bViewIdForVisCursorInvalidation(false);

static bool g_bLocalRendering(false);

static Compat g_eCompatFlags(Compat::none);

namespace
{

class LanguageAndLocale
{
private:
    LanguageTag maLanguageTag;
    LanguageTag maLocaleLanguageTag;

public:

    LanguageAndLocale()
        : maLanguageTag(LANGUAGE_NONE)
        , maLocaleLanguageTag(LANGUAGE_NONE)
    {}

    const LanguageTag& getLanguage() const
    {
        return maLanguageTag;
    }

    void setLanguage(const LanguageTag& rLanguageTag)
    {
        if (maLanguageTag != rLanguageTag)
        {
            SAL_INFO("comphelper.lok", "Setting language from " << maLanguageTag.getBcp47() << " to " << rLanguageTag.getBcp47());
            maLanguageTag = rLanguageTag;
        }
    }

    const LanguageTag& getLocale() const
    {
        return maLocaleLanguageTag;
    }

    void setLocale(const LanguageTag& rLocaleLanguageTag)
    {
        if (maLocaleLanguageTag != rLocaleLanguageTag)
        {
            SAL_INFO("comphelper.lok", "Setting locale from " << maLocaleLanguageTag.getBcp47() << " to " << rLocaleLanguageTag.getBcp47());
            maLocaleLanguageTag = rLocaleLanguageTag;
        }
    }

};

}

static LanguageAndLocale g_aLanguageAndLocale;

/// Scaling of the cairo canvas painting for hi-dpi
static double g_fDPIScale(1.0);

void setActive(bool bActive)
{
    g_bActive = bActive;
}

bool isActive()
{
    return g_bActive;
}

void setForkedChild(bool bIsChild)
{
    g_bForkedChild = bIsChild;
}

bool isForkedChild()
{
    return g_bForkedChild;
}

void setPartInInvalidation(bool bPartInInvalidation)
{
    g_bPartInInvalidation = bPartInInvalidation;
}

bool isPartInInvalidation()
{
    return g_bPartInInvalidation;
}

void setTiledPainting(bool bTiledPainting)
{
    g_bTiledPainting = bTiledPainting;
}

bool isTiledPainting()
{
    return g_bTiledPainting;
}

void setDialogPainting(bool bDialogPainting)
{
    g_bDialogPainting = bDialogPainting;
}

bool isDialogPainting()
{
    return g_bDialogPainting;
}

void setDPIScale(double fDPIScale)
{
    g_fDPIScale = fDPIScale;
}

double getDPIScale()
{
    return g_fDPIScale;
}

void setTiledAnnotations(bool bTiledAnnotations)
{
    g_bTiledAnnotations = bTiledAnnotations;
}

bool isTiledAnnotations()
{
    return g_bTiledAnnotations;
}

void setRangeHeaders(bool bRangeHeaders)
{
    g_bRangeHeaders = bRangeHeaders;
}

void setViewIdForVisCursorInvalidation(bool bViewIdForVisCursorInvalidation)
{
    g_bViewIdForVisCursorInvalidation = bViewIdForVisCursorInvalidation;
}

bool isViewIdForVisCursorInvalidation()
{
    return g_bViewIdForVisCursorInvalidation;
}

bool isRangeHeaders()
{
    return g_bRangeHeaders;
}

void setLocalRendering(bool bLocalRendering)
{
    g_bLocalRendering = bLocalRendering;
}

bool isLocalRendering()
{
    return g_bLocalRendering;
}

void setCompatFlag(Compat flag) { g_eCompatFlags = static_cast<Compat>(g_eCompatFlags | flag); }

bool isCompatFlagSet(Compat flag) { return (g_eCompatFlags & flag) == flag; }

void resetCompatFlag() { g_eCompatFlags = Compat::none; }

void setLocale(const LanguageTag& rLanguageTag)
{
    g_aLanguageAndLocale.setLocale(rLanguageTag);
}

const LanguageTag& getLocale()
{
    const LanguageTag& rLocale = g_aLanguageAndLocale.getLocale();
    SAL_INFO_IF(rLocale.getLanguageType() == LANGUAGE_NONE, "comphelper.lok", "Locale not set");
    return rLocale;
}

void setLanguageTag(const LanguageTag& rLanguageTag)
{
    g_aLanguageAndLocale.setLanguage(rLanguageTag);
}

const LanguageTag& getLanguageTag()
{
    const LanguageTag& rLanguage = g_aLanguageAndLocale.getLanguage();
    SAL_INFO_IF(rLanguage.getLanguageType() == LANGUAGE_NONE, "comphelper.lok", "Language not set");
    return rLanguage;
}

bool isAllowlistedLanguage(const OUString& lang)
{
    if (!isActive())
        return true;

#if defined ANDROID || defined IOS
    (void) lang;
    return true;
#else
    static const std::vector<OUString> aAllowlist = [] {
        std::vector<OUString> aList;
        // coverity[tainted_data] - we trust the contents of this variable
        const char* pAllowlist = getenv("LOK_ALLOWLIST_LANGUAGES");
        if (pAllowlist)
        {
            std::stringstream stream(pAllowlist);
            std::string s;

            std::cerr << "Allowlisted languages: ";
            while (getline(stream, s, ' ')) {
                if (s.length() == 0)
                    continue;

                std::cerr << s << " ";
                aList.emplace_back(OStringToOUString(s, RTL_TEXTENCODING_UTF8));
            }
            std::cerr << std::endl;
        }

        if (aList.empty())
            std::cerr << "No language allowlisted, turning off the language support." << std::endl;

        return aList;
    }();

    if (aAllowlist.empty())
        return false;

    for (const auto& entry : aAllowlist)
    {
        if (lang.startsWith(entry))
            return true;
        if (lang.startsWith(entry.replace('_', '-')))
            return true;
    }

    return false;
#endif
}

void setTimezone(bool isSet, const OUString& rTimezone)
{
    if (isSet)
    {
        // Set the given timezone, even if empty.
        osl_setEnvironment(u"TZ"_ustr.pData, rTimezone.pData);
    }
    else
    {
        // Unset and empty aren't the same.
        // When unset, it means default to the system configured timezone.
        osl_clearEnvironment(u"TZ"_ustr.pData);
    }

    // Update the timezone data.
    ::tzset();
}

static void (*pStatusIndicatorCallback)(void *data, statusIndicatorCallbackType type, int percent, const char* pText)(nullptr);
static void *pStatusIndicatorCallbackData(nullptr);

void setStatusIndicatorCallback(void (*callback)(void *data, statusIndicatorCallbackType type, int percent, const char* pText), void *data)
{
    pStatusIndicatorCallback = callback;
    pStatusIndicatorCallbackData = data;
}

void statusIndicatorStart(const OUString& sText)
{
    if (pStatusIndicatorCallback)
        pStatusIndicatorCallback(pStatusIndicatorCallbackData, statusIndicatorCallbackType::Start, 0, sText.toUtf8().getStr());
}

void statusIndicatorSetValue(int percent)
{
    if (pStatusIndicatorCallback)
        pStatusIndicatorCallback(pStatusIndicatorCallbackData, statusIndicatorCallbackType::SetValue, percent, nullptr);
}

void statusIndicatorFinish()
{
    if (pStatusIndicatorCallback)
        pStatusIndicatorCallback(pStatusIndicatorCallbackData, statusIndicatorCallbackType::Finish, 0, nullptr);
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
