/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>


namespace comphelper
{

namespace LibreOfficeKit
{

static bool g_bActive(false);

static bool g_bPartInInvalidation(false);

static bool g_bTiledPainting(false);

static bool g_bDialogPainting(false);

static bool g_bTiledAnnotations(true);

void setActive(bool bActive)
{
    g_bActive = bActive;
}

bool isActive()
{
    return g_bActive;
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

void setTiledAnnotations(bool bTiledAnnotations)
{
    g_bTiledAnnotations = bTiledAnnotations;
}

bool isTiledAnnotations()
{
    return g_bTiledAnnotations;
}

static bool g_bLocalRendering(false);

void setLocalRendering(bool bLocalRendering)
{
    g_bLocalRendering = bLocalRendering;
}

bool isLocalRendering()
{
    return g_bLocalRendering;
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

css::uno::Sequence< css::lang::Locale > getSpellLanguages()
{
    css::uno::Reference< css::uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    css::uno::Reference< css::linguistic2::XLinguServiceManager2 > xLangSrv = css::linguistic2::LinguServiceManager::create(xContext);
    css::uno::Reference< css::linguistic2::XSpellChecker > xSpell(xLangSrv.is() ? xLangSrv->getSpellChecker() : nullptr, css::uno::UNO_QUERY);
    css::uno::Reference< css::linguistic2::XSupportedLocales > xLocales(xSpell, css::uno::UNO_QUERY);
    css::uno::Sequence< css::lang::Locale > aLocales(xLocales.is() ? xLocales->getLocales() : css::uno::Sequence< css::lang::Locale >());
    return aLocales;
}

} // namespace LibreOfficeKit

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
