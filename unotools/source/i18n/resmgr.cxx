/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

// Needed when #include <boost/locale.hpp> below includes Boost 1.65.1
// workdir/UnpackedTarball/boost/boost/locale/format.hpp using "std::auto_ptr<data> d;", but must
// come very early here in case <memory> is already (indirectly) included earlier:
#include <config_libcxx.h>
#if HAVE_LIBCXX
#define _LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR
#elif defined _MSC_VER
#define _HAS_AUTO_PTR_ETC 1
#endif

#include <config_folders.h>

#include <sal/config.h>

#include <cassert>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#if defined UNX && !defined MACOSX && !defined IOS && !defined ANDROID
#   include <libintl.h>
#endif

#include <comphelper/lok.hxx>
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

#if defined(_WIN32) && defined(DBG_UTIL)
#include <o3tl/char16_t2wchar_t.hxx>
#include <prewin.h>
#include <crtdbg.h>
#include <postwin.h>
#endif

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

#if defined(_WIN32) && defined(DBG_UTIL)
static int IgnoringCrtReportHook(int reportType, wchar_t *message, int * /* returnValue */)
{
    OUString sType;
    if (reportType == _CRT_WARN)
        sType = "WARN";
    else if (reportType == _CRT_ERROR)
        sType = "ERROR";
    else if (reportType == _CRT_ASSERT)
        sType = "ASSERT";
    else
        sType = "?(" + OUString::number(reportType) + ")";

    SAL_WARN("unotools.i18n", "CRT Report Hook: " << sType << ": " << OUString(o3tl::toU(message)));

    return TRUE;
}
#endif


namespace Translate
{
    std::locale Create(const sal_Char* pPrefixName, const LanguageTag& rLocale)
    {
        static std::unordered_map<OString, std::locale> aCache;
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
#if defined UNX && !defined MACOSX && !defined IOS && !defined ANDROID
        bindtextdomain(pPrefixName, sPath.getStr());
#endif
        gen.add_messages_domain(pPrefixName);

#if defined(_WIN32) && defined(DBG_UTIL)
        // With a newer C++ debug runtime (in an --enable-dbgutil build), passing an invalid locale
        // name causes an attempt to display an error dialog. Which does not even show up, at least
        // for me, but instead the process (gengal, at least) just hangs. Which is far from ideal.

        // Passing a POSIX-style locale name to the std::locale constructor on Windows is a bit odd,
        // but apparently in the normal C++ runtime it "just" causes an exception to be thrown, that
        // boost catches (see the loadable(std::string name) in boost's
        // libs\locale\src\std\std_backend.cpp), and then instead uses the Windows style locale name
        // it knows how to construct. (Why does it even try the POSIX style name I can't
        // understand.)

        // Actually it isn't just the locale name part "en_US" of a locale like "en_US.UTF-8" that
        // is problematic, but also the encoding part, "UTF-8". The Microsoft C/C++ library does not
        // support UTF-8 locales. The error message that our own report hook catches says:
        // "f:\dd\vctools\crt\crtw32\stdcpp\xmbtowc.c(89) : Assertion failed: ploc->_Mbcurmax == 1
        // || ploc->_Mbcurmax == 2". Clearly in a UTF-8 locale (perhaps one that boost internally
        // constructs?) the maximum bytes per character will be more than 2.

        // With a debug C++ runtime, we need to avoid the error dialog, and just ignore the error.

        struct CrtSetReportHook
        {
            int mnCrtSetReportHookSucceeded;

            CrtSetReportHook()
            {
                mnCrtSetReportHookSucceeded = _CrtSetReportHookW2(_CRT_RPTHOOK_INSTALL, IgnoringCrtReportHook);
            }

            ~CrtSetReportHook()
            {
                if (mnCrtSetReportHookSucceeded >= 0)
                    _CrtSetReportHookW2(_CRT_RPTHOOK_REMOVE, IgnoringCrtReportHook);
            }
        } aHook;

#endif

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
            assert(!strchr(pId, '\004') && "should be using nget, not get");
        }

        //if it's a key id locale, generate it here
        if (std::use_facet<boost::locale::info>(loc).language() == "qtz")
        {
            OString sKeyId(genKeyId(OString(pContextAndId).replace('\004', '|')));
            return OUString::fromUtf8(sKeyId) + OUStringLiteral1(0x2016) + createFromUtf8(pId, strlen(pId));
        }

        //otherwise translate it
        const std::string ret = boost::locale::pgettext(sContext.getStr(), pId, loc);
        OUString result(ExpandVariables(createFromUtf8(ret.data(), ret.size())));

        if (comphelper::LibreOfficeKit::isActive())
        {
            // If it is de-CH, change sharp s to double s.
            if (std::use_facet<boost::locale::info>(loc).country() == "CH" &&
                std::use_facet<boost::locale::info>(loc).language() == "de")
                result = result.replaceAll(OUString::fromUtf8("\xC3\x9F"), "ss");
        }
        return result;
    }

    OUString nget(const char* pContextAndIds, int n, const std::locale &loc)
    {
        OString sContextIdId(pContextAndIds);
        std::vector<OString> aContextIdId;
        sal_Int32 nIndex = 0;
        do
        {
            aContextIdId.push_back(sContextIdId.getToken(0, '\004', nIndex));
        }
        while (nIndex >= 0);
        assert(aContextIdId.size() == 3 && "should be using get, not nget");

        //if it's a key id locale, generate it here
        if (std::use_facet<boost::locale::info>(loc).language() == "qtz")
        {
            OString sKeyId(genKeyId(aContextIdId[0] + "|" + aContextIdId[1]));
            int nForm = n == 0 ? 1 : 2;
            return OUString::fromUtf8(sKeyId) + OUStringLiteral1(0x2016) + createFromUtf8(aContextIdId[nForm].getStr(), aContextIdId[nForm].getLength());
        }

        //otherwise translate it
        const std::string ret = boost::locale::npgettext(aContextIdId[0].getStr(), aContextIdId[1].getStr(), aContextIdId[2].getStr(), n, loc);
        OUString result(ExpandVariables(createFromUtf8(ret.data(), ret.size())));

        if (comphelper::LibreOfficeKit::isActive())
        {
            if (std::use_facet<boost::locale::info>(loc).country() == "CH" &&
                std::use_facet<boost::locale::info>(loc).language() == "de")
                result = result.replaceAll(OUString::fromUtf8("\xC3\x9F"), "ss");
        }
        return result;
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
