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

#include <rtl/bootstrap.h>
#include <rtl/bootstrap.hxx>
#include <osl/diagnose.h>
#include <osl/process.h>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <osl/profile.hxx>
#include <osl/security.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <rtl/byteseq.hxx>
#include <sal/log.hxx>
#include <o3tl/lru_map.hxx>
#include <o3tl/string_view.hxx>

#include <utility>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <string_view>
#include <unordered_map>

#ifdef ANDROID
#include <osl/detail/android-bootstrap.h>
#endif

#ifdef EMSCRIPTEN
#include <osl/detail/emscripten-bootstrap.h>
#endif

#ifdef IOS
#include <premac.h>
#import <Foundation/Foundation.h>
#include <postmac.h>
#endif

using osl::DirectoryItem;
using osl::FileStatus;

namespace
{

struct Bootstrap_Impl;

constexpr std::u16string_view VND_SUN_STAR_PATHNAME = u"vnd.sun.star.pathname:";

bool isPathnameUrl(std::u16string_view url)
{
    return o3tl::matchIgnoreAsciiCase(url, VND_SUN_STAR_PATHNAME);
}

bool resolvePathnameUrl(OUString * url)
{
    OSL_ASSERT(url);
    if (!isPathnameUrl(*url) ||
        (osl::FileBase::getFileURLFromSystemPath(
            url->copy(VND_SUN_STAR_PATHNAME.size()), *url) ==
         osl::FileBase::E_None))
    {
        return true;
    }
    *url = OUString();
    return false;
}

enum class LookupMode {
    NORMAL, URE_BOOTSTRAP,
    URE_BOOTSTRAP_EXPANSION };

struct ExpandRequestLink {
    ExpandRequestLink const * next;
    Bootstrap_Impl const * file;
    OUString key;
};

OUString expandMacros(
    Bootstrap_Impl const * file, std::u16string_view text, LookupMode mode,
    ExpandRequestLink const * requestStack);

OUString recursivelyExpandMacros(
    Bootstrap_Impl const * file, std::u16string_view text, LookupMode mode,
    Bootstrap_Impl const * requestFile, OUString const & requestKey,
    ExpandRequestLink const * requestStack)
{
    for (; requestStack; requestStack = requestStack->next) {
        if (requestStack->file == requestFile &&
            requestStack->key == requestKey)
        {
            return u"***RECURSION DETECTED***"_ustr;
        }
    }
    ExpandRequestLink link = { requestStack, requestFile, requestKey };
    return expandMacros(file, text, mode, &link);
}

struct rtl_bootstrap_NameValue
{
    OUString sName;
    OUString sValue;

    rtl_bootstrap_NameValue()
        {}
    rtl_bootstrap_NameValue(OUString name, OUString value )
        : sName(std::move( name )),
          sValue(std::move( value ))
        {}
};

} // end namespace

typedef std::vector<rtl_bootstrap_NameValue> NameValueVector;

static bool find(
    NameValueVector const & vector, OUString const & key,
    OUString * value)
{
    OSL_ASSERT(value);
    auto i = std::find_if(vector.begin(), vector.end(),
        [&key](const rtl_bootstrap_NameValue& rNameValue) { return rNameValue.sName == key; });
    if (i != vector.end())
    {
        *value = i->sValue;
        return true;
    }
    return false;
}

namespace
{
    NameValueVector rtl_bootstrap_set_vector;
}

static bool getFromCommandLineArgs(
    OUString const & key, OUString * value )
{
    OSL_ASSERT(value);

    static NameValueVector nameValueVector = []()
    {
        NameValueVector tmp;

        sal_Int32 nArgCount = osl_getCommandArgCount();
        for(sal_Int32 i = 0; i < nArgCount; ++ i)
        {
            OUString pArg;
            osl_getCommandArg( i, &pArg.pData );
            if( (pArg.startsWith("-") || pArg.startsWith("/") ) &&
                pArg.match("env:", 1) )
            {
                sal_Int32 nIndex = pArg.indexOf( '=' );

                if( nIndex >= 0 )
                {
                    rtl_bootstrap_NameValue nameValue;
                    nameValue.sName = pArg.copy( 5, nIndex - 5  );
                    nameValue.sValue = pArg.copy( nIndex+1 );

                    if( i == nArgCount-1 &&
                        nameValue.sValue.getLength() &&
                        nameValue.sValue[nameValue.sValue.getLength()-1] == 13 )
                    {
                        // avoid the 13 linefeed for the last argument,
                        // when the executable is started from a script,
                        // that was edited on windows
                        nameValue.sValue = nameValue.sValue.copy(0,nameValue.sValue.getLength()-1);
                    }

                    tmp.push_back( nameValue );
                }
            }
        };
        return tmp;
    }();

    return find(nameValueVector, key, value);
}

static void getExecutableDirectory_Impl(rtl_uString ** ppDirURL)
{
    OUString fileName;
    osl_getExecutableFile(&(fileName.pData));

    sal_Int32 nDirEnd = fileName.lastIndexOf('/');
    OSL_ENSURE(nDirEnd >= 0, "Cannot locate executable directory");

    rtl_uString_newFromStr_WithLength(ppDirURL,fileName.getStr(),nDirEnd);
}

static OUString getIniFileName(bool overriding) {
    OUString fileName;

#if defined IOS
    // On iOS hardcode the inifile as "rc" in the .app
    // directory. Apps are self-contained anyway, there is no
    // possibility to have several "applications" in the same
    // installation location with different inifiles.
    const char *inifile = [[@"vnd.sun.star.pathname:" stringByAppendingString: [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent: (overriding ? @"fundamental.override.ini" : @"rc")]] UTF8String];
    fileName = OUString(inifile, strlen(inifile), RTL_TEXTENCODING_UTF8);
    resolvePathnameUrl(&fileName);
#elif defined ANDROID
    // Apps are self-contained on Android, too, can as well hardcode
    // it as "rc" in the "/assets" directory, i.e.  inside the app's
    // .apk (zip) archive as the /assets/rc file.
    fileName = overriding
        ? OUString("vnd.sun.star.pathname:/assets/fundamental.override.ini")
        : OUString("vnd.sun.star.pathname:/assets/rc");
    resolvePathnameUrl(&fileName);
#elif defined(EMSCRIPTEN)
    fileName = overriding
        ? OUString("vnd.sun.star.pathname:/instdir/program/fundamental.override.ini")
        : OUString("vnd.sun.star.pathname:/instdir/program/sofficerc");
    resolvePathnameUrl(&fileName);
#else
    if (!overriding && getFromCommandLineArgs(u"INIFILENAME"_ustr, &fileName))
    {
        resolvePathnameUrl(&fileName);
    }
    else
    {
        osl_getExecutableFile(&(fileName.pData));

        if (overriding) {
            auto const i = fileName.lastIndexOf('/') + 1;
            fileName = fileName.replaceAt(i, fileName.getLength() - i, u"fundamental.override.ini");
        } else {
            // get rid of a potential executable extension
            OUString progExt = u".bin"_ustr;
            if (fileName.getLength() > progExt.getLength()
                && o3tl::equalsIgnoreAsciiCase(fileName.subView(fileName.getLength() - progExt.getLength()), progExt))
            {
                fileName = fileName.copy(0, fileName.getLength() - progExt.getLength());
            }

            progExt = ".exe";
            if (fileName.getLength() > progExt.getLength()
                && o3tl::equalsIgnoreAsciiCase(fileName.subView(fileName.getLength() - progExt.getLength()), progExt))
            {
                fileName = fileName.copy(0, fileName.getLength() - progExt.getLength());
            }

            // append config file suffix
            fileName += SAL_CONFIGFILE("");
        }

#ifdef MACOSX
        // We keep only executables in the MacOS folder, and all
        // rc files in LIBO_ETC_FOLDER (typically "Resources").
        sal_Int32 off = fileName.lastIndexOf( "/MacOS/" );
        if (off != -1)
            fileName = fileName.replaceAt(off + 1, strlen("MacOS"), u"" LIBO_ETC_FOLDER);
#endif
    }
#endif

    return fileName;
}

static OUString const & getOverrideIniFileName_Impl()
{
    static OUString aStaticName = getIniFileName(true);

    return aStaticName;
}

static OUString & getIniFileName_Impl()
{
    static OUString aStaticName = getIniFileName(false);

    return aStaticName;
}

// ensure the given file url has no final slash

static void EnsureNoFinalSlash (OUString & url)
{
    sal_Int32 i = url.getLength();

    if (i > 0 && url[i - 1] == '/')
        url = url.copy(0, i - 1);
}

namespace {

struct Bootstrap_Impl
{
    sal_Int32 _nRefCount;
    Bootstrap_Impl * _override_base_ini;
    Bootstrap_Impl * _base_ini;

    NameValueVector _nameValueVector;
    OUString      _iniName;

    explicit Bootstrap_Impl (OUString const & rIniName);
    ~Bootstrap_Impl();

    static void * operator new (std::size_t n)
        { return malloc (sal_uInt32(n)); }
    static void operator delete (void * p , std::size_t)
        { free (p); }

    bool getValue(
        OUString const & key, rtl_uString ** value,
        rtl_uString * defaultValue, LookupMode mode, bool override,
        ExpandRequestLink const * requestStack) const;
    bool getDirectValue(
        OUString const & key, rtl_uString ** value, LookupMode mode,
        ExpandRequestLink const * requestStack) const;
    bool getAmbienceValue(
        OUString const & key, rtl_uString ** value, LookupMode mode,
        ExpandRequestLink const * requestStack) const;
    void expandValue(
        rtl_uString ** value, OUString const & text, LookupMode mode,
        Bootstrap_Impl const * requestFile, OUString const & requestKey,
        ExpandRequestLink const * requestStack) const;
};

}

Bootstrap_Impl::Bootstrap_Impl( OUString const & rIniName )
    : _nRefCount( 0 ),
      _override_base_ini( nullptr ),
      _base_ini( nullptr ),
      _iniName (rIniName)
{
    OUString override_base_ini(getOverrideIniFileName_Impl());
    // normalize path
    FileStatus override_status( osl_FileStatus_Mask_FileURL );
    DirectoryItem override_dirItem;
    bool skip_base_ini = false;
    if (DirectoryItem::get(override_base_ini, override_dirItem) == DirectoryItem::E_None &&
        override_dirItem.getFileStatus(override_status) == DirectoryItem::E_None)
    {
        override_base_ini = override_status.getFileURL();
        if (rIniName != override_base_ini)
        {
            _override_base_ini = static_cast< Bootstrap_Impl * >(
                rtl_bootstrap_args_open(override_base_ini.pData));
        }
        else
        {
            skip_base_ini = true;
        }
    }

    if (!skip_base_ini) {
        OUString base_ini(getIniFileName_Impl());
        // normalize path
        FileStatus status( osl_FileStatus_Mask_FileURL );
        DirectoryItem dirItem;
        if (DirectoryItem::get(base_ini, dirItem) == DirectoryItem::E_None &&
            dirItem.getFileStatus(status) == DirectoryItem::E_None)
        {
            base_ini = status.getFileURL();
            if (rIniName != base_ini)
            {
                _base_ini = static_cast< Bootstrap_Impl * >(
                    rtl_bootstrap_args_open(base_ini.pData));
            }
        }
    }

    SAL_INFO("sal.bootstrap", "Bootstrap_Impl(): sFile=" << _iniName);
    oslFileHandle handle;
    if (!_iniName.isEmpty() &&
        osl_openFile(_iniName.pData, &handle, osl_File_OpenFlag_Read) == osl_File_E_None)
    {
        rtl::ByteSequence seq;

        while (osl_readLine(handle , reinterpret_cast<sal_Sequence **>(&seq)) == osl_File_E_None)
        {
            OString line(reinterpret_cast<const char *>(seq.getConstArray()), seq.getLength());
            sal_Int32 nIndex = line.indexOf('=');
            if (nIndex >= 1)
            {
                struct rtl_bootstrap_NameValue nameValue;
                nameValue.sName = OStringToOUString(o3tl::trim(line.subView(0,nIndex)), RTL_TEXTENCODING_ASCII_US);
                nameValue.sValue = OStringToOUString(o3tl::trim(line.subView(nIndex+1)), RTL_TEXTENCODING_UTF8);

                SAL_INFO("sal.bootstrap", "pushing: name=" << nameValue.sName << " value=" << nameValue.sValue);

                _nameValueVector.push_back(nameValue);
            }
        }
        osl_closeFile(handle);
    }
    else
    {
        SAL_INFO( "sal.bootstrap", "couldn't open file: " <<  _iniName );
    }
}

Bootstrap_Impl::~Bootstrap_Impl()
{
    if (_base_ini)
        rtl_bootstrap_args_close( _base_ini );
    if (_override_base_ini)
        rtl_bootstrap_args_close( _override_base_ini );
}

namespace {

Bootstrap_Impl * get_static_bootstrap_handle()
{
    static Bootstrap_Impl* s_handle = []() {
        OUString iniName(getIniFileName_Impl());
        Bootstrap_Impl* that = static_cast<Bootstrap_Impl*>(rtl_bootstrap_args_open(iniName.pData));
        if (!that)
        {
            that = new Bootstrap_Impl(iniName);
            ++that->_nRefCount;
        }
        return that;
    }();

    return s_handle;
}

struct FundamentalIniData
{
    rtlBootstrapHandle ini;

    FundamentalIniData()
    {
        OUString uri;
        ini =
            (get_static_bootstrap_handle()->getValue(
                u"URE_BOOTSTRAP"_ustr, &uri.pData, nullptr, LookupMode::NORMAL, false,
                nullptr)
             && resolvePathnameUrl(&uri))
            ? rtl_bootstrap_args_open(uri.pData) : nullptr;
    }

    ~FundamentalIniData() { rtl_bootstrap_args_close(ini); }

    FundamentalIniData(const FundamentalIniData&) = delete;
    FundamentalIniData& operator=(const FundamentalIniData&) = delete;
};

FundamentalIniData& FundamentalIni()
{
    static FundamentalIniData SINGLETON;
    return SINGLETON;
}

}

bool Bootstrap_Impl::getValue(
    OUString const & key, rtl_uString ** value, rtl_uString * defaultValue,
    LookupMode mode, bool override, ExpandRequestLink const * requestStack)
    const
{
    if (mode == LookupMode::NORMAL && key == "URE_BOOTSTRAP")
        mode =  LookupMode::URE_BOOTSTRAP;

    if (override && getDirectValue(key, value, mode, requestStack))
        return true;

    if (_override_base_ini != nullptr
        && _override_base_ini->getDirectValue(key, value, mode, requestStack))
    {
        SAL_INFO("sal.bootstrap", "getValue(" << key << ") from fundamental.override.ini");
        return true;
    }

    if (key == "_OS")
    {
        rtl_uString_assign(
            value, (u"" RTL_OS ""_ustr).pData);
        return true;
    }

    if (key == "_ARCH")
    {
        rtl_uString_assign(
            value, (u"" RTL_ARCH ""_ustr).pData);
        return true;
    }

    if (key == "_CPPU_ENV")
    {
        rtl_uString_assign(
            value,
            (u"" SAL_STRINGIFY(CPPU_ENV) ""_ustr).pData);
        return true;
    }

#if defined ANDROID || defined EMSCRIPTEN
    if (key == "APP_DATA_DIR")
    {
        const char *app_data_dir = lo_get_app_data_dir();
        rtl_uString_assign(
            value, OUString(app_data_dir, strlen(app_data_dir), RTL_TEXTENCODING_UTF8).pData);
        return true;
    }
#endif

#ifdef IOS
    if (key == "APP_DATA_DIR")
    {
        const char *app_data_dir = [[[[NSBundle mainBundle] bundlePath] stringByAddingPercentEncodingWithAllowedCharacters: [NSCharacterSet URLPathAllowedCharacterSet]] UTF8String];
        rtl_uString_assign(
            value, OUString(app_data_dir, strlen(app_data_dir), RTL_TEXTENCODING_UTF8).pData);
        return true;
    }
#endif

    if (key == "ORIGIN")
    {
        rtl_uString_assign(
            value,
            _iniName.copy(
                0, std::max<sal_Int32>(0, _iniName.lastIndexOf('/'))).pData);
        return true;
    }

    if (getAmbienceValue(key, value, mode, requestStack))
        return true;

    if (key == "SYSUSERCONFIG")
    {
        OUString v;
        bool b = osl::Security().getConfigDir(v);
        EnsureNoFinalSlash(v);
        rtl_uString_assign(value, v.pData);
        return b;
    }

    if (key == "SYSUSERHOME")
    {
        OUString v;
        bool b = osl::Security().getHomeDir(v);
        EnsureNoFinalSlash(v);
        rtl_uString_assign(value, v.pData);
        return b;
    }

    if (key == "SYSBINDIR")
    {
        getExecutableDirectory_Impl(value);
        return true;
    }

    if (_base_ini != nullptr && _base_ini->getDirectValue(key, value, mode, requestStack))
        return true;

    if (!override && getDirectValue(key, value, mode, requestStack))
        return true;

    if (mode == LookupMode::NORMAL)
    {
        FundamentalIniData const & d = FundamentalIni();
        Bootstrap_Impl const * b = static_cast<Bootstrap_Impl const *>(d.ini);
        if (b != nullptr && b != this && b->getDirectValue(key, value, mode, requestStack))
            return true;
    }

    if (defaultValue != nullptr)
    {
        rtl_uString_assign(value, defaultValue);
        return true;
    }

    rtl_uString_new(value);
    return false;
}

bool Bootstrap_Impl::getDirectValue(
    OUString const & key, rtl_uString ** value, LookupMode mode,
    ExpandRequestLink const * requestStack) const
{
    OUString v;
    if (find(_nameValueVector, key, &v))
    {
        expandValue(value, v, mode, this, key, requestStack);
        return true;
    }

    return false;
}

bool Bootstrap_Impl::getAmbienceValue(
    OUString const & key, rtl_uString ** value, LookupMode mode,
    ExpandRequestLink const * requestStack) const
{
    OUString v;
    bool f;

    {
        osl::MutexGuard g(osl::Mutex::getGlobalMutex());
        f = find(rtl_bootstrap_set_vector, key, &v);
    }

    if (f || getFromCommandLineArgs(key, &v) ||
        osl_getEnvironment(key.pData, &v.pData) == osl_Process_E_None)
    {
        expandValue(value, v, mode, nullptr, key, requestStack);
        return true;
    }

    return false;
}

void Bootstrap_Impl::expandValue(
    rtl_uString ** value, OUString const & text, LookupMode mode,
    Bootstrap_Impl const * requestFile, OUString const & requestKey,
    ExpandRequestLink const * requestStack) const
{
    rtl_uString_assign(
        value,
        (mode ==  LookupMode::URE_BOOTSTRAP && isPathnameUrl(text) ?
         text :
         recursivelyExpandMacros(
             this, text,
             (mode ==  LookupMode::URE_BOOTSTRAP ?
               LookupMode::URE_BOOTSTRAP_EXPANSION : mode),
             requestFile, requestKey, requestStack)).pData);
}

namespace {

typedef std::unordered_map< OUString, Bootstrap_Impl * > bootstrap_map_t;
bootstrap_map_t bootstrap_map;

}

rtlBootstrapHandle SAL_CALL rtl_bootstrap_args_open(rtl_uString * pIniName)
{
    static o3tl::lru_map<OUString,OUString> fileUrlLookupCache(16);

    OUString originalIniName( pIniName );
    OUString iniName;

    osl::ResettableMutexGuard guard(osl::Mutex::getGlobalMutex());
    auto cacheIt = fileUrlLookupCache.find(originalIniName);
    bool foundInCache = cacheIt != fileUrlLookupCache.end();
    if (foundInCache)
        iniName = cacheIt->second;
    guard.clear();

    // normalize path
    if (!foundInCache)
    {
        FileStatus status(osl_FileStatus_Mask_FileURL);
        DirectoryItem dirItem;
        if (DirectoryItem::get(originalIniName, dirItem) != DirectoryItem::E_None ||
            dirItem.getFileStatus(status) != DirectoryItem::E_None)
        {
            return nullptr;
        }
        iniName = status.getFileURL();
    }

    guard.reset();
    if (!foundInCache)
        fileUrlLookupCache.insert({originalIniName, iniName});
    Bootstrap_Impl * that;
    auto iFind(bootstrap_map.find(iniName));
    if (iFind == bootstrap_map.end())
    {
        guard.clear();
        that = new Bootstrap_Impl(iniName);
        guard.reset();
        iFind = bootstrap_map.find(iniName);
        if (iFind == bootstrap_map.end())
        {
            ++that->_nRefCount;
            ::std::pair< bootstrap_map_t::iterator, bool > insertion(
                bootstrap_map.emplace(iniName, that));
            OSL_ASSERT(insertion.second);
        }
        else
        {
            Bootstrap_Impl * obsolete = that;
            that = iFind->second;
            ++that->_nRefCount;
            guard.clear();
            delete obsolete;
        }
    }
    else
    {
        that = iFind->second;
        ++that->_nRefCount;
    }
    return static_cast< rtlBootstrapHandle >( that );
}

void SAL_CALL rtl_bootstrap_args_close(rtlBootstrapHandle handle) SAL_THROW_EXTERN_C()
{
    if (!handle)
        return;

    Bootstrap_Impl * that = static_cast< Bootstrap_Impl * >( handle );

    osl::MutexGuard guard(osl::Mutex::getGlobalMutex());
    OSL_ASSERT(bootstrap_map.find(that->_iniName)->second == that);
    --that->_nRefCount;

    if (that->_nRefCount != 0)
        return;

    std::size_t const nLeaking = 8; // only hold up to 8 files statically
    if (bootstrap_map.size() > nLeaking)
    {
        ::std::size_t erased = bootstrap_map.erase( that->_iniName );
        if (erased != 1) {
            OSL_ASSERT( false );
        }
        delete that;
    }
}

sal_Bool SAL_CALL rtl_bootstrap_get_from_handle(
    rtlBootstrapHandle handle,
    rtl_uString      * pName,
    rtl_uString     ** ppValue,
    rtl_uString      * pDefault
)
{
    osl::MutexGuard guard(osl::Mutex::getGlobalMutex());

    bool found = false;
    if(ppValue && pName)
    {
        if (!handle)
            handle = get_static_bootstrap_handle();

        found = static_cast< Bootstrap_Impl * >(handle)->getValue(
            pName, ppValue, pDefault,  LookupMode::NORMAL, false, nullptr );
    }

    return found;
}

void SAL_CALL rtl_bootstrap_get_iniName_from_handle (
    rtlBootstrapHandle handle,
    rtl_uString     ** ppIniName
)
{
    if(!ppIniName)
        return;

    if(handle)
    {
        Bootstrap_Impl * pImpl = static_cast<Bootstrap_Impl*>(handle);
        rtl_uString_assign(ppIniName, pImpl->_iniName.pData);
    }
    else
    {
        const OUString & iniName = getIniFileName_Impl();
        rtl_uString_assign(ppIniName, iniName.pData);
    }
}

void SAL_CALL rtl_bootstrap_setIniFileName (
    rtl_uString * pName
)
{
    osl::MutexGuard guard(osl::Mutex::getGlobalMutex());
    OUString & file = getIniFileName_Impl();
    file = pName;
}

sal_Bool SAL_CALL rtl_bootstrap_get (
    rtl_uString  * pName,
    rtl_uString ** ppValue,
    rtl_uString  * pDefault
)
{
    return rtl_bootstrap_get_from_handle(nullptr, pName, ppValue, pDefault);
}

void SAL_CALL rtl_bootstrap_set (
    rtl_uString * pName,
    rtl_uString * pValue
)
{
    const OUString name(pName);
    const OUString value(pValue);

    osl::MutexGuard guard(osl::Mutex::getGlobalMutex());

    for (auto & item : rtl_bootstrap_set_vector)
    {
        if (item.sName == name)
        {
            item.sValue = value;
            return;
        }
    }

    SAL_INFO("sal.bootstrap", "explicitly setting: name=" << name << " value=" <<value);

    rtl_bootstrap_set_vector.emplace_back(name, value);
}

void SAL_CALL rtl_bootstrap_expandMacros_from_handle(
    rtlBootstrapHandle handle,
    rtl_uString     ** macro)
{
    if (!handle)
        handle = get_static_bootstrap_handle();

    OUString expanded(expandMacros(static_cast< Bootstrap_Impl * >(handle),
                                   OUString::unacquired(macro),
                                    LookupMode::NORMAL, nullptr));
    rtl_uString_assign(macro, expanded.pData);
}

void SAL_CALL rtl_bootstrap_expandMacros(rtl_uString ** macro)
{
    rtl_bootstrap_expandMacros_from_handle(nullptr, macro);
}

void rtl_bootstrap_encode(rtl_uString const * value, rtl_uString ** encoded)
{
    OSL_ASSERT(value);
    OUStringBuffer b(value->length+5);
    for (sal_Int32 i = 0; i < value->length; ++i)
    {
        sal_Unicode c = value->buffer[i];
        if (c == '$' || c == '\\')
            b.append('\\');

        b.append(c);
    }

    rtl_uString_assign(encoded, b.makeStringAndClear().pData);
}

namespace {

int hex(sal_Unicode c)
{
    return
        c >= '0' && c <= '9' ? c - '0' :
        c >= 'A' && c <= 'F' ? c - 'A' + 10 :
        c >= 'a' && c <= 'f' ? c - 'a' + 10 : -1;
}

sal_Unicode read(std::u16string_view text, std::size_t * pos, bool * escaped)
{
    OSL_ASSERT(pos && *pos < text.length() && escaped);
    sal_Unicode c = text[(*pos)++];
    if (c == '\\')
    {
        int n1, n2, n3, n4;
        if (*pos < text.length() - 4 && text[*pos] == 'u' &&
            ((n1 = hex(text[*pos + 1])) >= 0) &&
            ((n2 = hex(text[*pos + 2])) >= 0) &&
            ((n3 = hex(text[*pos + 3])) >= 0) &&
            ((n4 = hex(text[*pos + 4])) >= 0))
        {
            *pos += 5;
            *escaped = true;
            return static_cast< sal_Unicode >(
                (n1 << 12) | (n2 << 8) | (n3 << 4) | n4);
        }

        if (*pos < text.length())
        {
            *escaped = true;
            return text[(*pos)++];
        }
    }

    *escaped = false;
    return c;
}

OUString lookup(
    Bootstrap_Impl const * file, LookupMode mode, bool override,
    OUString const & key, ExpandRequestLink const * requestStack)
{
    OUString v;
    (file == nullptr ? get_static_bootstrap_handle() : file)->getValue(
        key, &v.pData, nullptr, mode, override, requestStack);
    return v;
}

OUString expandMacros(
    Bootstrap_Impl const * file, std::u16string_view text, LookupMode mode,
    ExpandRequestLink const * requestStack)
{
    SAL_INFO("sal.bootstrap", "expandMacros called with: " << OUString(text));
    OUStringBuffer buf(2048);

    for (std::size_t i = 0; i < text.length();)
    {
        bool escaped;
        sal_Unicode c = read(text, &i, &escaped);
        if (escaped || c != '$')
        {
            buf.append(c);
        }
        else
        {
            if (i < text.length() && text[i] == '{')
            {
                ++i;
                std::size_t p = i;
                sal_Int32 nesting = 0;
                OUString seg[3];
                int n = 0;

                while (i < text.length())
                {
                    std::size_t j = i;
                    c = read(text, &i, &escaped);

                    if (!escaped)
                    {
                        switch (c)
                        {
                            case '{':
                                ++nesting;
                                break;
                            case '}':
                                if (nesting == 0)
                                {
                                    seg[n++] = text.substr(p, j - p);
                                    goto done;
                                }
                                else
                                {
                                    --nesting;
                                }
                                break;
                            case ':':
                                if (nesting == 0 && n < 2)
                                {
                                    seg[n++] = text.substr(p, j - p);
                                    p = i;
                                }
                                break;
                        }
                    }
                }
            done:
                for (int j = 0; j < n; ++j)
                {
                    seg[j] = expandMacros(file, seg[j], mode, requestStack);
                }

                if (n == 3 && seg[0] != ".override" && seg[1].isEmpty())
                {
                    // For backward compatibility, treat ${file::key} the
                    // same as just ${file:key}:
                    seg[1] = seg[2];
                    n = 2;
                }

                if (n == 1)
                {
                    buf.append(lookup(file, mode, false, seg[0], requestStack));
                }
                else if (n == 2)
                {
                    rtl::Bootstrap b(seg[0]);
                    Bootstrap_Impl * f = static_cast< Bootstrap_Impl * >(b.getHandle());
                    buf.append(lookup(f, mode, false, seg[1], requestStack));
                }
                else if (n == 3 && seg[0] == ".override")
                {
                    rtl::Bootstrap b(seg[1]);
                    Bootstrap_Impl * f = static_cast< Bootstrap_Impl * >(b.getHandle());
                    buf.append(lookup(f, mode, f != nullptr, seg[2], requestStack));
                }
                else
                {
                    // Going through osl::Profile, this code erroneously
                    // does not recursively expand macros in the resulting
                    // replacement text (and if it did, it would fail to
                    // detect cycles that pass through here):
                    buf.append(
                        OStringToOUString(
                            osl::Profile(seg[0]).readString(
                                OUStringToOString(
                                    seg[1], RTL_TEXTENCODING_UTF8),
                                OUStringToOString(
                                    seg[2], RTL_TEXTENCODING_UTF8),
                                OString()),
                            RTL_TEXTENCODING_UTF8));
                }
            }
            else
            {
                OUStringBuffer kbuf(sal_Int32(text.length()));
                for (; i < text.length();)
                {
                    std::size_t j = i;
                    c = read(text, &j, &escaped);
                    if (!escaped &&
                        (c == ' ' || c == '$' || c == '-' || c == '/' ||
                         c == ';' || c == '\\'))
                    {
                        break;
                    }

                    kbuf.append(c);
                    i = j;
                }

                buf.append(
                    lookup(
                        file, mode, false, kbuf.makeStringAndClear(),
                        requestStack));
            }
        }
    }

    OUString result(buf.makeStringAndClear());
    SAL_INFO("sal.bootstrap", "expandMacros result: " << result);

    return result;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
