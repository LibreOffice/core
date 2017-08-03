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

#include <config_folders.h>

#include <sal/config.h>

#include <cassert>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <unotools/resmgr.hxx>
#include <osl/endian.h>
#include <osl/process.h>
#include <osl/thread.h>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <osl/signal.h>
#include <rtl/crc.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <rtl/instance.hxx>
#include <rtl/bootstrap.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>

#include <boost/locale.hpp>
#include <boost/locale/gnu_gettext.hpp>

#include <algorithm>
#include <list>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <memory>

namespace
{
    OUString createFromUtf8(const char* data, size_t size)
    {
        OUString aTarget;
        bool bSuccess = rtl_convertStringToUString(&aTarget.pData,
                                                   data,
                                                   size,
                                                   RTL_TEXTENCODING_UTF8,
                                                   RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR|RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR|RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR);
        (void) bSuccess;
        assert(bSuccess);
        return aTarget;
    }

    OString genKeyId(const OString& rGenerator)
    {
        sal_uInt32 nCRC = rtl_crc32(0, rGenerator.getStr(), rGenerator.getLength());
        // Use simple ASCII characters, exclude I, l, 1 and O, 0 to avoid confusing IDs
        static const char sSymbols[] =
            "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789";
        char sKeyId[6];
        for (short nKeyInd = 0; nKeyInd < 5; ++nKeyInd)
        {
            sKeyId[nKeyInd] = sSymbols[(nCRC & 63) % strlen(sSymbols)];
            nCRC >>= 6;
        }
        sKeyId[5] = '\0';
        return OString(sKeyId);
    }
}

namespace Translate
{
    std::locale Create(const sal_Char* pPrefixName, const LanguageTag& rLocale)
    {
        static std::unordered_map<OString, std::locale, OStringHash> aCache;
        OString sIdentifier = rLocale.getGlibcLocaleString(".UTF-8").toUtf8();
        OString sUnique = sIdentifier + OString(pPrefixName);
        auto aFind = aCache.find(sUnique);
        if (aFind != aCache.end())
            return aFind->second;
        boost::locale::generator gen;
        gen.characters(boost::locale::char_facet);
        gen.categories(boost::locale::message_facet | boost::locale::information_facet);
        OUString uri("$BRAND_BASE_DIR/$BRAND_SHARE_RESOURCE_SUBDIR/");
        rtl::Bootstrap::expandMacros(uri);
        OUString path;
        osl::File::getSystemPathFromFileURL(uri, path);
        OString sPath(OUStringToOString(path, osl_getThreadTextEncoding()));
        gen.add_messages_path(sPath.getStr());
#ifdef LINUX
        bindtextdomain(pPrefixName, sPath.getStr());
#endif
        gen.add_messages_domain(pPrefixName);
        std::locale aRet(gen(sIdentifier.getStr()));
        aCache[sUnique] = aRet;
        return aRet;
    }

    OUString get(const char* pContextAndId, const std::locale &loc)
    {
        OString sContext;
        const char *pId = strchr(pContextAndId, '\004');
        if (!pId)
            pId = pContextAndId;
        else
        {
            sContext = OString(pContextAndId, pId - pContextAndId);
            ++pId;
        }

        //if its a key id locale, generate it here
        if (std::use_facet<boost::locale::info>(loc).language() == "qtz")
        {
            OString sKeyId(genKeyId(OString(pContextAndId).replace('\004', '|')));
            return OUString::fromUtf8(sKeyId) + OUStringLiteral1(0x2016) + createFromUtf8(pId, strlen(pId));
        }

        //otherwise translate it
        const std::string ret = boost::locale::pgettext(sContext.getStr(), pId, loc);
        return ExpandVariables(createFromUtf8(ret.data(), ret.size()));
    }

    static ResHookProc pImplResHookProc = nullptr;

    OUString ExpandVariables(const OUString& rString)
    {
        if (pImplResHookProc)
            return pImplResHookProc(rString);
        return rString;
    }

    void SetReadStringHook( ResHookProc pProc )
    {
        pImplResHookProc = pProc;
    }

    ResHookProc GetReadStringHook()
    {
        return pImplResHookProc;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
