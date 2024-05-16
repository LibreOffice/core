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

#include <boost/version.hpp>
#if BOOST_VERSION < 106700
// Needed when #include <boost/locale.hpp> below includes Boost 1.65.1
// workdir/UnpackedTarball/boost/boost/locale/format.hpp using "std::auto_ptr<data> d;", but must
// come very early here in case <memory> is already (indirectly) included earlier:
#include <config_libcxx.h>
#if HAVE_LIBCPP
#define _LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR
#elif defined _MSC_VER
#define _HAS_AUTO_PTR_ETC 1
#endif
#endif

#include <sal/config.h>

#include <cassert>

#include <string.h>
#include <stdio.h>
#if defined UNX && !defined MACOSX && !defined IOS && !defined ANDROID && !defined EMSCRIPTEN
#   include <libintl.h>
#endif

#include <comphelper/lok.hxx>
#include <unotools/resmgr.hxx>
#include <osl/thread.h>
#include <osl/file.hxx>
#include <rtl/crc.h>
#include <rtl/bootstrap.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <boost/locale.hpp>
#include <boost/locale/gnu_gettext.hpp>

#include <unordered_map>

#ifdef ANDROID
#include <osl/detail/android-bootstrap.h>
#endif

#ifdef EMSCRIPTEN
#include <osl/detail/emscripten-bootstrap.h>
#endif

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
        return sKeyId;
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
    std::locale Create(std::string_view aPrefixName, const LanguageTag& rLocale)
    {
        static std::unordered_map<OString, std::locale> aCache;
        OString sIdentifier = rLocale.getGlibcLocaleString(u".UTF-8").toUtf8();
        OString sUnique = sIdentifier + aPrefixName;
        auto aFind = aCache.find(sUnique);
        if (aFind != aCache.end())
            return aFind->second;
        boost::locale::generator gen;
#if BOOST_VERSION < 108100
        gen.characters(boost::locale::char_facet);
        gen.categories(boost::locale::message_facet | boost::locale::information_facet);
#else
        gen.characters(boost::locale::char_facet_t::char_f);
        gen.categories(boost::locale::category_t::message | boost::locale::category_t::information);
#endif
#if defined(ANDROID) || defined(EMSCRIPTEN)
        OString sPath(OString(lo_get_app_data_dir()) + "/program/resource");
#else
        OUString uri(u"$BRAND_BASE_DIR/$BRAND_SHARE_RESOURCE_SUBDIR/"_ustr);
        rtl::Bootstrap::expandMacros(uri);
        OUString path;
        osl::File::getSystemPathFromFileURL(uri, path);
#if defined _WIN32
        // add_messages_path is documented to treat path string in the *created* locale's encoding
        // on Windows; creating an UTF-8 encoding, we're lucky to have Unicode path support here.
        constexpr rtl_TextEncoding eEncoding = RTL_TEXTENCODING_UTF8;
#else
        const rtl_TextEncoding eEncoding = osl_getThreadTextEncoding();
#endif
        OString sPath(OUStringToOString(path, eEncoding));
#endif
        gen.add_messages_path(std::string(sPath));
#if defined UNX && !defined MACOSX && !defined IOS && !defined ANDROID && !defined EMSCRIPTEN
        // allow gettext to find these .mo files e.g. so gtk dialogs can use them
        bindtextdomain(aPrefixName.data(), sPath.getStr());
        // tdf#131069 gtk, and anything sane, always wants utf-8 strings as output
        bind_textdomain_codeset(aPrefixName.data(), "UTF-8");
#endif
        gen.add_messages_domain(aPrefixName.data());

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

        std::locale aRet(gen(std::string(sIdentifier)));

        aCache[sUnique] = aRet;
        return aRet;
    }

    OUString get(TranslateId sContextAndId, const std::locale &loc)
    {
        assert(!strchr(sContextAndId.getId(), '\004') && "should be using nget, not get");

        //if it's a key id locale, generate it here
        if (std::use_facet<boost::locale::info>(loc).language() == "qtz")
        {
            OString sKeyId(genKeyId(OString::Concat(sContextAndId.mpContext) + "|" + std::string_view(sContextAndId.getId())));
            return OUString::fromUtf8(sKeyId) + u"\u2016" + createFromUtf8(sContextAndId.getId(), strlen(sContextAndId.getId()));
        }

        //otherwise translate it
        const std::string ret = boost::locale::pgettext(sContextAndId.mpContext, sContextAndId.getId(), loc);
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

    OUString nget(TranslateNId aContextSingularPlural, int n, const std::locale &loc)
    {
        //if it's a key id locale, generate it here
        if (std::use_facet<boost::locale::info>(loc).language() == "qtz")
        {
            OString sKeyId(genKeyId(OString::Concat(aContextSingularPlural.mpContext) + "|" + aContextSingularPlural.mpSingular));
            const char* pForm = n == 0 ? aContextSingularPlural.mpSingular : aContextSingularPlural.mpPlural;
            return OUString::fromUtf8(sKeyId) + u"\u2016" + createFromUtf8(pForm, strlen(pForm));
        }

        //otherwise translate it
        const std::string ret = boost::locale::npgettext(aContextSingularPlural.mpContext, aContextSingularPlural.mpSingular, aContextSingularPlural.mpPlural, n, loc);
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

bool TranslateId::operator==(const TranslateId& other) const
{
    if (mpContext == nullptr || other.mpContext == nullptr)
    {
        if (mpContext != other.mpContext)
            return false;
    }
    else if (strcmp(mpContext, other.mpContext) != 0)
        return false;

    if (mpId == nullptr || other.mpId == nullptr)
    {
        return mpId == other.mpId;
    }
    return strcmp(getId(),other.getId()) == 0;
}

bool TranslateNId::operator==(const TranslateNId& other) const
{
    if (mpContext == nullptr || other.mpContext == nullptr)
    {
        if (mpContext != other.mpContext)
            return false;
    }
    else if (strcmp(mpContext, other.mpContext) != 0)
        return false;

    if (mpSingular == nullptr || other.mpSingular == nullptr)
    {
        if (mpSingular != other.mpSingular)
            return false;
    }
    else if (strcmp(mpSingular, other.mpSingular) != 0)
        return false;

    if (mpPlural == nullptr || other.mpPlural == nullptr)
    {
        return mpPlural == other.mpPlural;
    }
    return strcmp(mpPlural,other.mpPlural) == 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
