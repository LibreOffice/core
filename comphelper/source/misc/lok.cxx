/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/lok.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <sal/log.hxx>

#include <iostream>
#include <map>

namespace comphelper
{

namespace LibreOfficeKit
{

static bool g_bActive(false);

static bool g_bPartInInvalidation(false);

static bool g_bTiledPainting(false);

static bool g_bDialogPainting(false);

static bool g_bTiledAnnotations(true);

static bool g_bRangeHeaders(false);

static bool g_bViewIdForVisCursorInvalidation(false);

static bool g_bLocalRendering(false);

static Compat g_eCompatFlags(Compat::none);

static LanguageTag g_aLanguageTag("en-US", true);

/// Scaling of the cairo canvas painting for hi-dpi
static double g_fDPIScale(1.0);

/// Which views are on mobile phones?
static std::map<int, bool> g_vIsViewMobilePhone;

/// Which views are on tablets?
static std::map<int, bool> g_vIsViewTablet;

void setActive(bool bActive)
{
    g_bActive = bActive;
}

bool isActive()
{
    return g_bActive;
}

void setMobilePhone(int nViewId, bool bIsMobilePhone)
{
    g_vIsViewMobilePhone[nViewId] = bIsMobilePhone;
}

bool isMobilePhone(int nViewId)
{
    if (g_vIsViewMobilePhone.find(nViewId) != g_vIsViewMobilePhone.end())
        return g_vIsViewMobilePhone[nViewId];
    else
        return false;
}

void setTablet(int nViewId, bool bIsTablet)
{
    g_vIsViewTablet[nViewId] = bIsTablet;
}

bool isTablet(int nViewId)
{
    if (g_vIsViewTablet.find(nViewId) != g_vIsViewTablet.end())
        return g_vIsViewTablet[nViewId];
    else
        return false;
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

void setLanguageTag(const LanguageTag& languageTag)
{
    if (g_aLanguageTag != languageTag)
    {
        SAL_INFO("comphelper.lok", "setLanguageTag: from " << g_aLanguageTag.getBcp47() << " to " << languageTag.getBcp47());
        g_aLanguageTag = languageTag;
    }
}

const LanguageTag& getLanguageTag()
{
    return g_aLanguageTag;
}

bool isWhitelistedLanguage(const OUString& lang)
{
    if (!isActive())
        return true;

    static bool bInitialized = false;
    static std::vector<OUString> aWhitelist;
    if (!bInitialized)
    {
        // coverity[tainted_data] - we trust the contents of this variable
        const char* pWhitelist = getenv("LOK_WHITELIST_LANGUAGES");
        if (pWhitelist)
        {
            std::stringstream stream(pWhitelist);
            std::string s;

            std::cerr << "Whitelisted languages: ";
            while (getline(stream, s, ' ')) {
                if (s.length() == 0)
                    continue;

                std::cerr << s << " ";
                aWhitelist.emplace_back(OStringToOUString(s.c_str(), RTL_TEXTENCODING_UTF8));
            }
            std::cerr << std::endl;
        }

        if (aWhitelist.empty())
            std::cerr << "No language whitelisted, turning off the language support." << std::endl;

        bInitialized = true;
    }

    if (aWhitelist.empty())
        return false;

    for (const auto& entry : aWhitelist)
    {
        if (lang.startsWith(entry))
            return true;
        if (lang.startsWith(entry.replace('_', '-')))
            return true;
    }

    return false;
}

static void (*pStatusIndicatorCallback)(void *data, statusIndicatorCallbackType type, int percent)(nullptr);
static void *pStatusIndicatorCallbackData(nullptr);

void setStatusIndicatorCallback(void (*callback)(void *data, statusIndicatorCallbackType type, int percent), void *data)
{
    pStatusIndicatorCallback = callback;
    pStatusIndicatorCallbackData = data;
}

void statusIndicatorStart()
{
    if (pStatusIndicatorCallback)
        pStatusIndicatorCallback(pStatusIndicatorCallbackData, statusIndicatorCallbackType::Start, 0);
}

void statusIndicatorSetValue(int percent)
{
    if (pStatusIndicatorCallback)
        pStatusIndicatorCallback(pStatusIndicatorCallbackData, statusIndicatorCallbackType::SetValue, percent);
}

void statusIndicatorFinish()
{
    if (pStatusIndicatorCallback)
        pStatusIndicatorCallback(pStatusIndicatorCallbackData, statusIndicatorCallbackType::Finish, 0);
}

} // namespace LibreOfficeKit

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
