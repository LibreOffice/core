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
#include <config_features.h>
#include <config_folders.h>

#include "rtl/bootstrap.h"
#include "rtl/bootstrap.hxx"
#include <osl/diagnose.h>
#include <osl/module.h>
#include <osl/process.h>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <osl/profile.hxx>
#include <osl/security.hxx>
#include <rtl/alloc.h>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <rtl/byteseq.hxx>
#include <rtl/instance.hxx>
#include <rtl/malformeduriexception.hxx>
#include <rtl/uri.hxx>

#include <list>
#include <algorithm>
#include <unordered_map>

#ifdef ANDROID
#include <osl/detail/android-bootstrap.h>
#endif

#ifdef IOS
#include <premac.h>
#import <Foundation/Foundation.h>
#include <postmac.h>
#endif

using osl::DirectoryItem;
using osl::FileStatus;

using rtl::OString;
using rtl::OUString;
using rtl::OUStringToOString;

struct Bootstrap_Impl;

namespace {

static char const VND_SUN_STAR_PATHNAME[] = "vnd.sun.star.pathname:";

bool isPathnameUrl(rtl::OUString const & url) {
    return url.matchIgnoreAsciiCase(VND_SUN_STAR_PATHNAME);
}

bool resolvePathnameUrl(rtl::OUString * url) {
    OSL_ASSERT(url !=  nullptr);
    if (!isPathnameUrl(*url) ||
        (osl::FileBase::getFileURLFromSystemPath(
            url->copy(RTL_CONSTASCII_LENGTH(VND_SUN_STAR_PATHNAME)), *url) ==
         osl::FileBase::E_None))
    {
        return true;
    }
    *url = rtl::OUString();
    return false;
}

enum LookupMode {
    LOOKUP_MODE_NORMAL, LOOKUP_MODE_URE_BOOTSTRAP,
    LOOKUP_MODE_URE_BOOTSTRAP_EXPANSION };

struct ExpandRequestLink {
    ExpandRequestLink const * next;
    Bootstrap_Impl const * file;
    rtl::OUString key;
};

rtl::OUString expandMacros(
    Bootstrap_Impl const * file, rtl::OUString const & text, LookupMode mode,
    ExpandRequestLink const * requestStack);

rtl::OUString recursivelyExpandMacros(
    Bootstrap_Impl const * file, rtl::OUString const & text, LookupMode mode,
    Bootstrap_Impl const * requestFile, rtl::OUString const & requestKey,
    ExpandRequestLink const * requestStack)
{
    for (; requestStack != nullptr; requestStack = requestStack->next) {
        if (requestStack->file == requestFile &&
            requestStack->key == requestKey)
        {
            return rtl::OUString("***RECURSION DETECTED***");
        }
    }
    ExpandRequestLink link = { requestStack, requestFile, requestKey };
    return expandMacros(file, text, mode, &link);
}

}

struct rtl_bootstrap_NameValue
{
    OUString sName;
    OUString sValue;

    rtl_bootstrap_NameValue()
        {}
    rtl_bootstrap_NameValue(
        OUString const & name, OUString const & value )
        : sName( name ),
          sValue( value )
        {}
};

typedef std::list<rtl_bootstrap_NameValue> NameValueList;

bool find(
    NameValueList const & list, rtl::OUString const & key,
    rtl::OUString * value)
{
    OSL_ASSERT(value != nullptr);
    for (NameValueList::const_iterator i(list.begin()); i != list.end(); ++i) {
        if (i->sName == key) {
            *value = i->sValue;
            return true;
        }
    }
    return false;
}

namespace {
    struct rtl_bootstrap_set_list :
        public rtl::Static< NameValueList, rtl_bootstrap_set_list > {};
}

static bool getFromCommandLineArgs(
    rtl::OUString const & key, rtl::OUString * value )
{
    OSL_ASSERT(value != nullptr);
    static NameValueList *pNameValueList = nullptr;
    if( ! pNameValueList )
    {
        static NameValueList nameValueList;

        sal_Int32 nArgCount = osl_getCommandArgCount();
        for(sal_Int32 i = 0; i < nArgCount; ++ i)
        {
            rtl_uString *pArg = nullptr;
            osl_getCommandArg( i, &pArg );
            if( (pArg->buffer[0] == '-' || pArg->buffer[0] == '/' ) &&
                pArg->buffer[1] == 'e' &&
                pArg->buffer[2] == 'n' &&
                pArg->buffer[3] == 'v' &&
                pArg->buffer[4] == ':' )
            {
                sal_Int32 nIndex = rtl_ustr_indexOfChar( pArg->buffer, '=' );
                if( nIndex >= 0 )
                {

                    rtl_bootstrap_NameValue nameValue;
                    nameValue.sName = OUString( &(pArg->buffer[5]), nIndex - 5  );
                    nameValue.sValue = OUString( &(pArg->buffer[nIndex+1]) );
                    if( i == nArgCount-1 &&
                        nameValue.sValue.getLength() &&
                        nameValue.sValue[nameValue.sValue.getLength()-1] == 13 )
                    {
                        // avoid the 13 linefeed for the last argument,
                        // when the executable is started from a script,
                        // that was edited on windows
                        nameValue.sValue = nameValue.sValue.copy(0,nameValue.sValue.getLength()-1);
                    }
                    nameValueList.push_back( nameValue );
                }
            }
            rtl_uString_release( pArg );
        }
        pNameValueList = &nameValueList;
    }

    bool found = false;

    for( NameValueList::iterator ii = pNameValueList->begin() ;
         ii != pNameValueList->end() ;
         ++ii )
    {
        if( (*ii).sName.equals(key) )
        {
            *value = (*ii).sValue;
            found = true;
            break;
        }
    }

    return found;
}

static void getExecutableDirectory_Impl (rtl_uString ** ppDirURL)
{
    OUString fileName;
    osl_getExecutableFile(&(fileName.pData));

    sal_Int32 nDirEnd = fileName.lastIndexOf('/');
    OSL_ENSURE(nDirEnd >= 0, "Cannot locate executable directory");

    rtl_uString_newFromStr_WithLength(ppDirURL,fileName.getStr(),nDirEnd);
}

static OUString & getIniFileName_Impl()
{
    osl::MutexGuard guard( osl::Mutex::getGlobalMutex() );
    static OUString *pStaticName = nullptr;
    if( ! pStaticName )
    {
        OUString fileName;

#if defined IOS
        // On iOS hardcode the inifile as "rc" in the .app
        // directory. Apps are self-contained anyway, there is no
        // possibility to have several "applications" in the same
        // installation location with different inifiles.
        const char *inifile = [[@"vnd.sun.star.pathname:" stringByAppendingString: [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent: @"rc"]] UTF8String];
        fileName = rtl::OUString(inifile, strlen(inifile), RTL_TEXTENCODING_UTF8);
        resolvePathnameUrl(&fileName);
#elif defined ANDROID
        // Apps are self-contained on Android, too, can as well hardcode
        // it as "rc" in the "/assets" directory, i.e.  inside the app's
        // .apk (zip) archive as the /assets/rc file.
        fileName = rtl::OUString("vnd.sun.star.pathname:/assets/rc");
        resolvePathnameUrl(&fileName);
#else
        if(getFromCommandLineArgs(
               "INIFILENAME", &fileName))
        {
            resolvePathnameUrl(&fileName);
        }
        else
        {
            osl_getExecutableFile(&(fileName.pData));

            // get rid of a potential executable extension
            OUString progExt = ".bin";
            if(fileName.getLength() > progExt.getLength()
            && fileName.copy(fileName.getLength() - progExt.getLength()).equalsIgnoreAsciiCase(progExt))
                fileName = fileName.copy(0, fileName.getLength() - progExt.getLength());

            progExt = ".exe";
            if(fileName.getLength() > progExt.getLength()
            && fileName.copy(fileName.getLength() - progExt.getLength()).equalsIgnoreAsciiCase(progExt))
                fileName = fileName.copy(0, fileName.getLength() - progExt.getLength());

            // append config file suffix
            fileName += SAL_CONFIGFILE("");

#ifdef MACOSX
            // We keep only executables in the MacOS folder, and all
            // rc files in LIBO_ETC_FOLDER (typically "Resources").
            sal_Int32 off = fileName.lastIndexOf( "/MacOS/" );
            if ( off != -1 )
                fileName = fileName.replaceAt( off + 1, strlen("MacOS"), LIBO_ETC_FOLDER );
#endif
        }
#endif

        static OUString theFileName;
        if(fileName.getLength())
            theFileName = fileName;

        pStaticName = &theFileName;
    }

    return *pStaticName;
}

// ensure the given file url has no final slash

inline void EnsureNoFinalSlash (rtl::OUString & url)
{
    sal_Int32 i = url.getLength();
    if (i > 0 && url[i - 1] == '/') {
        url = url.copy(0, i - 1);
    }
}

struct Bootstrap_Impl
{
    sal_Int32 _nRefCount;
    Bootstrap_Impl * _base_ini;

    NameValueList _nameValueList;
    OUString      _iniName;

    explicit Bootstrap_Impl (OUString const & rIniName);
    ~Bootstrap_Impl();

    static void * operator new (std::size_t n)
        { return rtl_allocateMemory (sal_uInt32(n)); }
    static void operator delete (void * p , std::size_t)
        { rtl_freeMemory (p); }

    bool getValue(
        rtl::OUString const & key, rtl_uString ** value,
        rtl_uString * defaultValue, LookupMode mode, bool override,
        ExpandRequestLink const * requestStack) const;
    bool getDirectValue(
        rtl::OUString const & key, rtl_uString ** value, LookupMode mode,
        ExpandRequestLink const * requestStack) const;
    bool getAmbienceValue(
        rtl::OUString const & key, rtl_uString ** value, LookupMode mode,
        ExpandRequestLink const * requestStack) const;
    void expandValue(
        rtl_uString ** value, rtl::OUString const & text, LookupMode mode,
        Bootstrap_Impl const * requestFile, rtl::OUString const & requestKey,
        ExpandRequestLink const * requestStack) const;
};

Bootstrap_Impl::Bootstrap_Impl( OUString const & rIniName )
    : _nRefCount( 0 ),
      _base_ini( nullptr ),
      _iniName (rIniName)
{
    OUString base_ini( getIniFileName_Impl() );
    // normalize path
    FileStatus status( osl_FileStatus_Mask_FileURL );
    DirectoryItem dirItem;
    if (DirectoryItem::get( base_ini, dirItem ) == DirectoryItem::E_None &&
        dirItem.getFileStatus( status ) == DirectoryItem::E_None)
    {
        base_ini = status.getFileURL();
        if (! rIniName.equals( base_ini ))
        {
            _base_ini = static_cast< Bootstrap_Impl * >(
                rtl_bootstrap_args_open( base_ini.pData ) );
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
            OString line( reinterpret_cast<const char *>(seq.getConstArray()), seq.getLength() );
            sal_Int32 nIndex = line.indexOf('=');
            if (nIndex >= 1)
            {
                struct rtl_bootstrap_NameValue nameValue;
                nameValue.sName = OStringToOUString(
                    line.copy(0,nIndex).trim(), RTL_TEXTENCODING_ASCII_US );
                nameValue.sValue = OStringToOUString(
                    line.copy(nIndex+1).trim(), RTL_TEXTENCODING_UTF8 );

                SAL_INFO("sal.bootstrap", "pushing: name=" << nameValue.sName << " value=" << nameValue.sValue);

                _nameValueList.push_back(nameValue);
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
    if (_base_ini != nullptr)
        rtl_bootstrap_args_close( _base_ini );
}

namespace {

Bootstrap_Impl * get_static_bootstrap_handle()
{
    osl::MutexGuard guard( osl::Mutex::getGlobalMutex() );
    static Bootstrap_Impl * s_handle = nullptr;
    if (s_handle == nullptr)
    {
        OUString iniName (getIniFileName_Impl());
        s_handle = static_cast< Bootstrap_Impl * >(
            rtl_bootstrap_args_open( iniName.pData ) );
        if (s_handle == nullptr)
        {
            Bootstrap_Impl * that = new Bootstrap_Impl( iniName );
            ++that->_nRefCount;
            s_handle = that;
        }
    }
    return s_handle;
}

struct FundamentalIniData {
    rtlBootstrapHandle ini;

    FundamentalIniData() {
        OUString uri;
        ini =
            (get_static_bootstrap_handle()->getValue(
                "URE_BOOTSTRAP", &uri.pData, nullptr, LOOKUP_MODE_NORMAL, false,
                nullptr)
             && resolvePathnameUrl(&uri))
            ? rtl_bootstrap_args_open(uri.pData) : nullptr;
    }

    ~FundamentalIniData() { rtl_bootstrap_args_close(ini); }

    FundamentalIniData(const FundamentalIniData&) = delete;
    FundamentalIniData& operator=(const FundamentalIniData&) = delete;
};

struct FundamentalIni: public rtl::Static< FundamentalIniData, FundamentalIni >
{};

}

bool Bootstrap_Impl::getValue(
    rtl::OUString const & key, rtl_uString ** value, rtl_uString * defaultValue,
    LookupMode mode, bool override, ExpandRequestLink const * requestStack)
    const
{
    if (mode == LOOKUP_MODE_NORMAL && key == "URE_BOOTSTRAP") {
        mode = LOOKUP_MODE_URE_BOOTSTRAP;
    }
    if (override && getDirectValue(key, value, mode, requestStack)) {
        return true;
    }
    if (key == "_OS") {
        rtl_uString_assign(
            value, rtl::OUString(RTL_OS).pData);
        return true;
    }
    if (key == "_ARCH") {
        rtl_uString_assign(
            value, rtl::OUString(RTL_ARCH).pData);
        return true;
    }
    if (key == "_CPPU_ENV") {
        rtl_uString_assign(
            value,
            (rtl::OUString(
                SAL_STRINGIFY(CPPU_ENV)).
             pData));
        return true;
    }
#ifdef ANDROID
    if (key == "APP_DATA_DIR") {
        const char *app_data_dir = lo_get_app_data_dir();
        rtl_uString_assign(
            value, rtl::OUString(app_data_dir, strlen(app_data_dir), RTL_TEXTENCODING_UTF8).pData);
        return true;
    }
#endif
#ifdef IOS
    if (key == "APP_DATA_DIR") {
        const char *app_data_dir = [[[[NSBundle mainBundle] bundlePath] stringByAddingPercentEncodingWithAllowedCharacters: [NSCharacterSet URLPathAllowedCharacterSet]] UTF8String];
        rtl_uString_assign(
            value, rtl::OUString(app_data_dir, strlen(app_data_dir), RTL_TEXTENCODING_UTF8).pData);
        return true;
    }
#endif
    if (key == "ORIGIN") {
        rtl_uString_assign(
            value,
            _iniName.copy(
                0, std::max<sal_Int32>(0, _iniName.lastIndexOf('/'))).pData);
        return true;
    }
    if (getAmbienceValue(key, value, mode, requestStack)) {
        return true;
    }
    if (key == "SYSUSERCONFIG") {
        rtl::OUString v;
        bool b = osl::Security().getConfigDir(v);
        EnsureNoFinalSlash(v);
        rtl_uString_assign(value, v.pData);
        return b;
    }
    if (key == "SYSUSERHOME") {
        rtl::OUString v;
        bool b = osl::Security().getHomeDir(v);
        EnsureNoFinalSlash(v);
        rtl_uString_assign(value, v.pData);
        return b;
    }
    if (key == "SYSBINDIR") {
        getExecutableDirectory_Impl(value);
        return true;
    }
    if (_base_ini != nullptr &&
        _base_ini->getDirectValue(key, value, mode, requestStack))
    {
        return true;
    }
    if (!override && getDirectValue(key, value, mode, requestStack)) {
        return true;
    }
    if (mode == LOOKUP_MODE_NORMAL) {
        FundamentalIniData const & d = FundamentalIni::get();
        Bootstrap_Impl const * b = static_cast<Bootstrap_Impl const *>(d.ini);
        if (b != nullptr && b != this &&
            b->getDirectValue(key, value, mode, requestStack))
        {
            return true;
        }
    }
    if (defaultValue != nullptr) {
        rtl_uString_assign(value, defaultValue);
        return true;
    }
    rtl_uString_new(value);
    return false;
}

bool Bootstrap_Impl::getDirectValue(
    rtl::OUString const & key, rtl_uString ** value, LookupMode mode,
    ExpandRequestLink const * requestStack) const
{
    rtl::OUString v;
    if (find(_nameValueList, key, &v)) {
        expandValue(value, v, mode, this, key, requestStack);
        return true;
    }
    return false;
}

bool Bootstrap_Impl::getAmbienceValue(
    rtl::OUString const & key, rtl_uString ** value, LookupMode mode,
    ExpandRequestLink const * requestStack) const
{
    rtl::OUString v;
    bool f;
    {
        osl::MutexGuard g(osl::Mutex::getGlobalMutex());
        f = find(rtl_bootstrap_set_list::get(), key, &v);
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
    rtl_uString ** value, rtl::OUString const & text, LookupMode mode,
    Bootstrap_Impl const * requestFile, rtl::OUString const & requestKey,
    ExpandRequestLink const * requestStack) const
{
    rtl_uString_assign(
        value,
        (mode == LOOKUP_MODE_URE_BOOTSTRAP && isPathnameUrl(text) ?
         text :
         recursivelyExpandMacros(
             this, text,
             (mode == LOOKUP_MODE_URE_BOOTSTRAP ?
              LOOKUP_MODE_URE_BOOTSTRAP_EXPANSION : mode),
             requestFile, requestKey, requestStack)).pData);
}

namespace {

struct bootstrap_map {
    typedef std::unordered_map<
        rtl::OUString, Bootstrap_Impl *,
        rtl::OUStringHash > t;

    bootstrap_map(const bootstrap_map&) = delete;
    bootstrap_map& operator=(const bootstrap_map&) = delete;

    // get and release must only be called properly synchronized via some mutex
    // (e.g., osl::Mutex::getGlobalMutex()):

    static t * get() {
        if (m_map == nullptr) {
            m_map = new t;
        }
        return m_map;
    }

    static void release() {
        if (m_map != nullptr && m_map->empty()) {
            delete m_map;
            m_map = nullptr;
        }
    }

private:
    static t * m_map;
};

bootstrap_map::t * bootstrap_map::m_map = nullptr;

}

rtlBootstrapHandle SAL_CALL rtl_bootstrap_args_open (
    rtl_uString * pIniName
)
{
    OUString iniName( pIniName );

    // normalize path
    FileStatus status( osl_FileStatus_Mask_FileURL );
    DirectoryItem dirItem;
    if (DirectoryItem::get( iniName, dirItem ) != DirectoryItem::E_None ||
        dirItem.getFileStatus( status ) != DirectoryItem::E_None)
    {
        return nullptr;
    }
    iniName = status.getFileURL();

    Bootstrap_Impl * that;
    osl::ResettableMutexGuard guard( osl::Mutex::getGlobalMutex() );
    bootstrap_map::t* p_bootstrap_map = bootstrap_map::get();
    bootstrap_map::t::const_iterator iFind( p_bootstrap_map->find( iniName ) );
    if (iFind == p_bootstrap_map->end())
    {
        bootstrap_map::release();
        guard.clear();
        that = new Bootstrap_Impl( iniName );
        guard.reset();
        p_bootstrap_map = bootstrap_map::get();
        iFind = p_bootstrap_map->find( iniName );
        if (iFind == p_bootstrap_map->end())
        {
            ++that->_nRefCount;
            ::std::pair< bootstrap_map::t::iterator, bool > insertion(
                p_bootstrap_map->insert(
                    bootstrap_map::t::value_type( iniName, that ) ) );
            OSL_ASSERT( insertion.second );
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

void SAL_CALL rtl_bootstrap_args_close (
    rtlBootstrapHandle handle
) SAL_THROW_EXTERN_C()
{
    if (handle == nullptr)
        return;
    Bootstrap_Impl * that = static_cast< Bootstrap_Impl * >( handle );

    osl::MutexGuard guard( osl::Mutex::getGlobalMutex() );
    bootstrap_map::t* p_bootstrap_map = bootstrap_map::get();
    OSL_ASSERT(
        p_bootstrap_map->find( that->_iniName )->second == that );
    --that->_nRefCount;
    if (that->_nRefCount == 0)
    {
        std::size_t const nLeaking = 8; // only hold up to 8 files statically
        if (p_bootstrap_map->size() > nLeaking)
        {
            ::std::size_t erased = p_bootstrap_map->erase( that->_iniName );
            if (erased != 1) {
                OSL_ASSERT( false );
            }
            delete that;
        }
        bootstrap_map::release();
    }
}

sal_Bool SAL_CALL rtl_bootstrap_get_from_handle(
    rtlBootstrapHandle handle,
    rtl_uString      * pName,
    rtl_uString     ** ppValue,
    rtl_uString      * pDefault
)
{
    osl::MutexGuard guard( osl::Mutex::getGlobalMutex() );

    bool found = false;
    if(ppValue && pName)
    {
        if (handle == nullptr)
            handle = get_static_bootstrap_handle();
        found = static_cast< Bootstrap_Impl * >( handle )->getValue(
            pName, ppValue, pDefault, LOOKUP_MODE_NORMAL, false, nullptr );
    }

    return found;
}

void SAL_CALL rtl_bootstrap_get_iniName_from_handle (
    rtlBootstrapHandle handle,
    rtl_uString     ** ppIniName
)
{
    if(ppIniName)
    {
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
}

void SAL_CALL rtl_bootstrap_setIniFileName (
    rtl_uString * pName
)
{
    osl::MutexGuard guard( osl::Mutex::getGlobalMutex() );
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
    const OUString name( pName );
    const OUString value( pValue );

    osl::MutexGuard guard( osl::Mutex::getGlobalMutex() );

    NameValueList& r_rtl_bootstrap_set_list = rtl_bootstrap_set_list::get();
    NameValueList::iterator iPos( r_rtl_bootstrap_set_list.begin() );
    NameValueList::iterator iEnd( r_rtl_bootstrap_set_list.end() );
    for ( ; iPos != iEnd; ++iPos )
    {
        if (iPos->sName.equals( name ))
        {
            iPos->sValue = value;
            return;
        }
    }

    SAL_INFO("sal.bootstrap", "explicitly getting: name=" << name << " value=" <<value);

    r_rtl_bootstrap_set_list.push_back( rtl_bootstrap_NameValue( name, value ) );
}

void SAL_CALL rtl_bootstrap_expandMacros_from_handle(
    rtlBootstrapHandle handle,
    rtl_uString     ** macro)
{
    if (handle == nullptr) {
        handle = get_static_bootstrap_handle();
    }
    OUString expanded( expandMacros( static_cast< Bootstrap_Impl * >( handle ),
                                     OUString::unacquired( macro ),
                                     LOOKUP_MODE_NORMAL, nullptr ) );
    rtl_uString_assign( macro, expanded.pData );
}

void SAL_CALL rtl_bootstrap_expandMacros(rtl_uString ** macro)
{
    rtl_bootstrap_expandMacros_from_handle(nullptr, macro);
}

void rtl_bootstrap_encode( rtl_uString const * value, rtl_uString ** encoded )
{
    OSL_ASSERT(value != nullptr);
    rtl::OUStringBuffer b;
    for (sal_Int32 i = 0; i < value->length; ++i) {
        sal_Unicode c = value->buffer[i];
        if (c == '$' || c == '\\') {
            b.append('\\');
        }
        b.append(c);
    }
    rtl_uString_assign(encoded, b.makeStringAndClear().pData);
}

namespace {

int hex(sal_Unicode c) {
    return
        c >= '0' && c <= '9' ? c - '0' :
        c >= 'A' && c <= 'F' ? c - 'A' + 10 :
        c >= 'a' && c <= 'f' ? c - 'a' + 10 : -1;
}

sal_Unicode read(rtl::OUString const & text, sal_Int32 * pos, bool * escaped) {
    OSL_ASSERT(
        pos != nullptr && *pos >= 0 && *pos < text.getLength() && escaped != nullptr);
    sal_Unicode c = text[(*pos)++];
    if (c == '\\') {
        int n1, n2, n3, n4;
        if (*pos < text.getLength() - 4 && text[*pos] == 'u' &&
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
        if (*pos < text.getLength()) {
            *escaped = true;
            return text[(*pos)++];
        }
    }
    *escaped = false;
    return c;
}

rtl::OUString lookup(
    Bootstrap_Impl const * file, LookupMode mode, bool override,
    rtl::OUString const & key, ExpandRequestLink const * requestStack)
{
    rtl::OUString v;
    (file == nullptr ? get_static_bootstrap_handle() : file)->getValue(
        key, &v.pData, nullptr, mode, override, requestStack);
    return v;
}

rtl::OUString expandMacros(
    Bootstrap_Impl const * file, rtl::OUString const & text, LookupMode mode,
    ExpandRequestLink const * requestStack)
{
    SAL_INFO("sal.bootstrap", "expandMacros called with: " << text);
    rtl::OUStringBuffer buf;
    for (sal_Int32 i = 0; i < text.getLength();) {
        bool escaped;
        sal_Unicode c = read(text, &i, &escaped);
        if (escaped || c != '$') {
            buf.append(c);
        } else {
            if (i < text.getLength() && text[i] == '{') {
                ++i;
                sal_Int32 p = i;
                sal_Int32 nesting = 0;
                rtl::OUString seg[3];
                int n = 0;
                while (i < text.getLength()) {
                    sal_Int32 j = i;
                    c = read(text, &i, &escaped);
                    if (!escaped) {
                        switch (c) {
                        case '{':
                            ++nesting;
                            break;
                        case '}':
                            if (nesting == 0) {
                                seg[n++] = text.copy(p, j - p);
                                goto done;
                            } else {
                                --nesting;
                            }
                            break;
                        case ':':
                            if (nesting == 0 && n < 2) {
                                seg[n++] = text.copy(p, j - p);
                                p = i;
                            }
                            break;
                        }
                    }
                }
            done:
                for (int j = 0; j < n; ++j) {
                    seg[j] = expandMacros(file, seg[j], mode, requestStack);
                }
                if (n == 1) {
                    buf.append(lookup(file, mode, false, seg[0], requestStack));
                } else if (n == 3 && seg[0] == ".override") {
                    rtl::Bootstrap b(seg[1]);
                    Bootstrap_Impl * f = static_cast< Bootstrap_Impl * >(
                        b.getHandle());
                    buf.append(
                        lookup(f, mode, f != nullptr, seg[2], requestStack));
                } else {
                    if (n == 3 && seg[1].isEmpty()) {
                        // For backward compatibility, treat ${file::key} the
                        // same as just ${file:key}:
                        seg[1] = seg[2];
                        n = 2;
                    }
                    if (n == 2) {
                        buf.append(
                            lookup(
                                static_cast< Bootstrap_Impl * >(
                                    rtl::Bootstrap(seg[0]).getHandle()),
                                mode, false, seg[1], requestStack));
                    } else {
                        // Going through osl::Profile, this code erroneously
                        // does not recursively expand macros in the resulting
                        // replacement text (and if it did, it would fail to
                        // detect cycles that pass through here):
                        buf.append(
                            rtl::OStringToOUString(
                                osl::Profile(seg[0]).readString(
                                    rtl::OUStringToOString(
                                        seg[1], RTL_TEXTENCODING_UTF8),
                                    rtl::OUStringToOString(
                                        seg[2], RTL_TEXTENCODING_UTF8),
                                    rtl::OString()),
                                RTL_TEXTENCODING_UTF8));
                    }
                }
            } else {
                rtl::OUStringBuffer kbuf;
                for (; i < text.getLength();) {
                    sal_Int32 j = i;
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
