/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <osl/thread.h>

#include <unx/font/fontmanager.hxx>
#include <unx/gendata.hxx>
#include <unx/helper.hxx>

#include <tools/urlobj.hxx>

#include <sal/log.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <sys/times.h>
#include <stdio.h>
#endif

#ifdef CALLGRIND_COMPILE
#include <valgrind/callgrind.h>
#endif

using namespace psp;

/*
 *  one instance only
 */
FontConfigManager& FontConfigManager::get()
{
    GenericUnixSalData* const pSalData(GetGenericUnixSalData());
    assert(pSalData);
    return *pSalData->GetFontConfigManager();
}

/*
 *  the FontConfigManager
 */

FontConfigManager::FontConfigManager()
    : m_aFontInstallerTimer("FontConfigManager m_aFontInstallerTimer")
{
    m_aFontInstallerTimer.SetInvokeHandler(LINK(this, FontConfigManager, autoInstallFontLangSupport));
    m_aFontInstallerTimer.SetTimeout(5000);

    initFontconfig();

    // tell fontconfig about the fonts installed with the office, in
    // SAL_PRIVATE_FONTPATH; it is asked for substitutes before we check whether
    // a font is available, and would happily substitute fonts it doesn't know
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    const OUString& rSalPrivatePath = psp::getFontPath();
    if (!rSalPrivatePath.isEmpty())
    {
        OString aPath = OUStringToOString(rSalPrivatePath, aEncoding);
        sal_Int32 nIndex = 0;
        do
        {
            OString aToken = aPath.getToken(0, ';', nIndex);
            normPath(aToken);
            if (!aToken.isEmpty())
                addFontconfigDir(aToken);
        } while (nIndex >= 0);
    }

    // this also filters the fontconfig font set, so it has to run before anything
    // can match against it; GenericFontList takes the fonts off our hands
    collectSystemFonts();
}

FontConfigManager::~FontConfigManager()
{
    m_aFontInstallerTimer.Stop();
    deinitFontconfig();
}

std::vector<FontconfigFont> FontConfigManager::addFontFile(const OString& rFile)
{
    addFontconfigFile(rFile);
    return fontsFromFontconfigFile(rFile);
}

void FontConfigManager::removeFontFile(std::string_view rFile)
{
    removeFontconfigFile(rFile);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
