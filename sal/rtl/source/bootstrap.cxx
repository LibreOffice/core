/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "boost/noncopyable.hpp"
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
#include "rtl/allocator.hxx"

#include <algorithm>
#include <map>
#include <memory>
#include <utility>

#ifdef IOS
#include <premac.h>
#import <Foundation/Foundation.h>
#include <postmac.h>
#endif

#define MY_STRING_(x) # x
#define MY_STRING(x) MY_STRING_(x)

extern "C" oslProcessError SAL_CALL osl_bootstrap_getExecutableFile_Impl(
    rtl_uString ** ppFileURL) SAL_THROW_EXTERN_C();

using osl::DirectoryItem;
using osl::FileStatus;

using rtl::OString;
using rtl::OUString;
using rtl::OUStringToOString;

struct Bootstrap_Impl;

namespace {

static char const VND_SUN_STAR_PATHNAME[] = "vnd.sun.star.pathname:";

bool isPathnameUrl(rtl::OUString const & url) {
    return url.matchIgnoreAsciiCaseAsciiL(
        RTL_CONSTASCII_STRINGPARAM(VND_SUN_STAR_PATHNAME));
}

bool resolvePathnameUrl(rtl::OUString * url) {
    OSL_ASSERT(url !=  NULL);
    if (!isPathnameUrl(*url) ||
        (osl::FileBase::getFileURLFromSystemPath(
            url->copy(RTL_CONSTASCII_LENGTH(VND_SUN_STAR_PATHNAME)), *url) ==
         osl::FileBase::E_None))
    {
        return true;
    } else {
        *url = rtl::OUString();
        return false;
    }
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
    for (; requestStack != NULL; requestStack = requestStack->next) {
        if (requestStack->file == requestFile &&
            requestStack->key == requestKey)
        {
            return rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("***RECURSION DETECTED***"));
        }
    }
    ExpandRequestLink link = { requestStack, requestFile, requestKey };
    return expandMacros(file, text, mode, &link);
}

class ParameterMap: private boost::noncopyable {
public:
    bool get(rtl::OUString const & key, rtl::OUString * value) const;

protected:
    ParameterMap() {}

    ~ParameterMap() {}

    typedef std::map< rtl::OUString, rtl::OUString > Map;

    Map map_;
};

bool ParameterMap::get(rtl::OUString const & key, rtl::OUString * value) const {
    OSL_ASSERT(value != 0);
    Map::const_iterator i(map_.find(key));
    if (i == map_.end()) {
        return false;
    } else {
        *value = i->second;
        return true;
    }
}

class ExplicitParameterMap: public ParameterMap {
public:
    bool get(rtl::OUString const & key, rtl::OUString * value) const;

    void set(rtl::OUString const & key, rtl::OUString const & value);

private:
    mutable osl::Mutex mutex_;
};

bool ExplicitParameterMap::get(rtl::OUString const & key, rtl::OUString * value)
    const
{
    osl::MutexGuard g(mutex_);
    return ParameterMap::get(key, value);
}

void ExplicitParameterMap::set(
    rtl::OUString const & key, rtl::OUString const & value)
{
    osl::MutexGuard g(mutex_);
    map_[key] = value;
}

struct ExplicitParameters:
    public rtl::Static< ExplicitParameterMap, ExplicitParameters >
{};

class CommandLineParameterMap: public ParameterMap {
public:
    CommandLineParameterMap();
};

CommandLineParameterMap::CommandLineParameterMap() {
    sal_uInt32 n = osl_getCommandArgCount();
    for (sal_uInt32 i = 0; i != n; ++i) {
        rtl::OUString s;
        osl_getCommandArg(i, &s.pData);
        static char const PREFIX[] = "-env:";
        if (s.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(PREFIX))) {
            sal_Int32 j = s.indexOf('=', RTL_CONSTASCII_LENGTH(PREFIX));
            if (j < 0) {
                map_.erase(s.copy(RTL_CONSTASCII_LENGTH(PREFIX)));
            } else {
                map_[
                    s.copy(
                        RTL_CONSTASCII_LENGTH(PREFIX),
                        j - RTL_CONSTASCII_LENGTH(PREFIX))] =
                    s.copy(j + 1);
            }
        }
    }
}

struct CommandLineParameters:
    public rtl::Static< CommandLineParameterMap, CommandLineParameters >
{};

}

static void getExecutableDirectory_Impl (rtl_uString ** ppDirURL)
{
    OUString fileName;
    osl_bootstrap_getExecutableFile_Impl (&(fileName.pData));

    sal_Int32 nDirEnd = fileName.lastIndexOf('/');
    OSL_ENSURE(nDirEnd >= 0, "Cannot locate executable directory");

    rtl_uString_newFromStr_WithLength(ppDirURL,fileName.getStr(),nDirEnd);
}

static inline bool path_exists( OUString const & path )
{
    DirectoryItem dirItem;
    return (DirectoryItem::E_None == DirectoryItem::get( path, dirItem ));
}

//----------------------------------------------------------------------------
// #111772#
// ensure the given file url has no final slash

inline void EnsureNoFinalSlash (rtl::OUString & url)
{
    sal_Int32 i = url.getLength();
    if (i > 0 && url[i - 1] == '/') {
        url = url.copy(0, i - 1);
    }
}

struct Bootstrap_Impl: private ParameterMap
{
    OUString      _iniName;

    explicit Bootstrap_Impl (OUString const & rIniName);

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

//----------------------------------------------------------------------------

Bootstrap_Impl::Bootstrap_Impl( OUString const & rIniName )
    : _iniName (rIniName)
{
#if OSL_DEBUG_LEVEL > 1
    OString sFile = OUStringToOString(_iniName, RTL_TEXTENCODING_ASCII_US);
    OSL_TRACE("Bootstrap_Impl(): sFile=%s", sFile.getStr());
#endif /* OSL_DEBUG_LEVEL > 1 */

    oslFileHandle handle;
    if (!_iniName.isEmpty() &&
        osl_File_E_None == osl_openFile(_iniName.pData, &handle, osl_File_OpenFlag_Read))
    {
        rtl::ByteSequence seq;

        while (osl_File_E_None == osl_readLine(handle , (sal_Sequence **)&seq))
        {
            OString line( (const sal_Char *) seq.getConstArray(), seq.getLength() );
            sal_Int32 nIndex = line.indexOf('=');
            if (nIndex >= 1)
            {
                rtl::OUString sName = OStringToOUString(
                    line.copy(0,nIndex).trim(), RTL_TEXTENCODING_ASCII_US );
                rtl::OUString sValue = OStringToOUString(
                    line.copy(nIndex+1).trim(), RTL_TEXTENCODING_UTF8 );

#if OSL_DEBUG_LEVEL > 1
				OString name_tmp = OUStringToOString(sName, RTL_TEXTENCODING_ASCII_US);
				OString value_tmp = OUStringToOString(sValue, RTL_TEXTENCODING_UTF8);
                OSL_TRACE(
                    "pushing: name=%s value=%s",
                    name_tmp.getStr(), value_tmp.getStr() );
#endif /* OSL_DEBUG_LEVEL > 1 */

				map_[sName] = sValue;
            }
        }
        osl_closeFile(handle);
    }
#if OSL_DEBUG_LEVEL > 1
    else
    {
        OString file_tmp = OUStringToOString(_iniName, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE( "couldn't open file: %s", file_tmp.getStr() );
    }
#endif /* OSL_DEBUG_LEVEL > 1 */
}

namespace {

class BootstrapMap: private boost::noncopyable {
public:
    BootstrapMap();

    ~BootstrapMap();

    Bootstrap_Impl * getIni(rtl::OUString const & uri, bool alwaysCreate);

    void setBaseIniUri(rtl::OUString const & uri);

    Bootstrap_Impl * getBaseIni();

    Bootstrap_Impl * getFundamentalIni();

private:
    typedef std::map< rtl::OUString, Bootstrap_Impl * > Map;

    osl::Mutex mutex_;
    Map map_;
    rtl::OUString baseIniUri_;
    Bootstrap_Impl * baseIni_;
    bool hasFundamentalIni_;
    Bootstrap_Impl * fundamentalIni_;
};

BootstrapMap::BootstrapMap():
    baseIni_(0), hasFundamentalIni_(false), fundamentalIni_(0)
{}

BootstrapMap::~BootstrapMap() {
    for (Map::iterator i(map_.begin()); i != map_.end(); ++i) {
        delete i->second;
    }
}

Bootstrap_Impl * BootstrapMap::getIni(
    rtl::OUString const & uri, bool alwaysCreate)
{
    rtl::OUString normUri; // normalize URI if possible
    DirectoryItem dirItem;
    FileStatus status(osl_FileStatus_Mask_FileURL);
    if (DirectoryItem::get(uri, dirItem) == DirectoryItem::E_None &&
        dirItem.getFileStatus(status) == DirectoryItem::E_None)
    {
        normUri = status.getFileURL();
    } else if (alwaysCreate) {
        normUri = uri;
    } else {
        return 0;
    }
    osl::MutexGuard g(mutex_);
    Map::iterator i(map_.find(normUri));
    if (i == map_.end()) {
        std::auto_ptr< Bootstrap_Impl > b(new Bootstrap_Impl(normUri));
        std::pair< Map::iterator, bool > ins(
            map_.insert(Map::value_type(normUri, b.get())));
        b.release();
        OSL_ASSERT(ins.second);
        i = ins.first;
    }
    return i->second;
}

void BootstrapMap::setBaseIniUri(rtl::OUString const & uri) {
    OSL_ASSERT(!uri.isEmpty());
    osl::MutexGuard g(mutex_);
    OSL_ASSERT(baseIniUri_.isEmpty() && baseIni_ == 0);
    baseIniUri_ = uri;
}

Bootstrap_Impl * BootstrapMap::getBaseIni() {
    osl::MutexGuard g(mutex_);
    if (baseIni_ == 0) {
        rtl::OUString uri;
        if (baseIniUri_.isEmpty()) {
#if defined IOS
            // On iOS hardcode the inifile as "rc" in the .app
            // directory. Apps are self-contained anyway, there is no
            // possibility to have several "applications" in the same
            // installation location with different inifiles.
            const char *inifile = [[@"vnd.sun.star.pathname:" stringByAppendingString: [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent: @"rc"]] UTF8String];
            uri = rtl::OUString(inifile, strlen(inifile), RTL_TEXTENCODING_UTF8);
            resolvePathnameUrl(&uri);
#elif defined ANDROID
            // Apps are self-contained on Android, too, can as well hardcode
            // it as "rc" in the "/assets" directory, i.e.  inside the app's
            // .apk (zip) archive as the /assets/rc file.
            uri = rtl::OUString("vnd.sun.star.pathname:/assets/rc");
            resolvePathnameUrl(&uri);
#else
            if (CommandLineParameters::get().get(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("INIFILENAME")),
                    &uri))
            {
                resolvePathnameUrl(&uri);
            } else {
                osl_bootstrap_getExecutableFile_Impl(&uri.pData);
                // Strip potentially two such extensions, to allow for
                // renaming of soffice.bin to soffice.bin.exe so that
                // Visual Studio agrees to start it, if you want to
                // debug it from the start.
                static char const BIN_EXT[] = ".bin";
                static char const EXE_EXT[] = ".exe";
                for (int i = 0; i < 2; i++) {
                    if (uri.endsWithAsciiL(RTL_CONSTASCII_STRINGPARAM(BIN_EXT))) {
                        uri = uri.copy(
                            0, uri.getLength() - RTL_CONSTASCII_LENGTH(BIN_EXT));
                    } else if (uri.endsWithAsciiL(
                                   RTL_CONSTASCII_STRINGPARAM(EXE_EXT))) {
                        uri = uri.copy(
                            0, uri.getLength() - RTL_CONSTASCII_LENGTH(EXE_EXT));
                    }
                }
                uri += rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(SAL_CONFIGFILE("")));
            }
#endif
        } else {
            uri = baseIniUri_;
        }
        baseIni_ = getIni(uri, true);
    }
    return baseIni_;
}

Bootstrap_Impl * BootstrapMap::getFundamentalIni() {
    osl::MutexGuard g(mutex_);
    if (!hasFundamentalIni_) {
        rtl::OUString uri;
        fundamentalIni_ =
            (getBaseIni()->getValue(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URE_BOOTSTRAP")),
                &uri.pData, 0, LOOKUP_MODE_NORMAL, false, 0) &&
             resolvePathnameUrl(&uri))
            ? getIni(uri, false) : 0;
        hasFundamentalIni_ = true;
    }
    return fundamentalIni_;
}

struct BootstrapMapSingleton:
    public rtl::Static< BootstrapMap, BootstrapMapSingleton >
{};

}

bool Bootstrap_Impl::getValue(
    rtl::OUString const & key, rtl_uString ** value, rtl_uString * defaultValue,
    LookupMode mode, bool override, ExpandRequestLink const * requestStack)
    const
{
    if (mode == LOOKUP_MODE_NORMAL &&
        key.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("URE_BOOTSTRAP")))
    {
        mode = LOOKUP_MODE_URE_BOOTSTRAP;
    }
    if (override && getDirectValue(key, value, mode, requestStack)) {
        return true;
    }
    if ( key == "_OS" ) {
        rtl_uString_assign(
            value, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(RTL_OS)).pData);
        return true;
    }
    if ( key == "_ARCH" ) {
        rtl_uString_assign(
            value, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(RTL_ARCH)).pData);
        return true;
    }
    if ( key == "_CPPU_ENV" ) {
        rtl_uString_assign(
            value,
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(MY_STRING(CPPU_ENV))).
             pData));
        return true;
    }
    if ( key == "ORIGIN" ) {
        rtl_uString_assign(
            value,
            _iniName.copy(
                0, std::max<sal_Int32>(0, _iniName.lastIndexOf('/'))).pData);
        return true;
    }
    if (getAmbienceValue(key, value, mode, requestStack)) {
        return true;
    }
    if ( key == "SYSUSERCONFIG" ) {
        rtl::OUString v;
        bool b = osl::Security().getConfigDir(v);
        EnsureNoFinalSlash(v);
        rtl_uString_assign(value, v.pData);
        return b;
    }
    if ( key == "SYSUSERHOME" ) {
        rtl::OUString v;
        bool b = osl::Security().getHomeDir(v);
        EnsureNoFinalSlash(v);
        rtl_uString_assign(value, v.pData);
        return b;
    }
    if ( key == "SYSBINDIR" ) {
        getExecutableDirectory_Impl(value);
        return true;
    }
    Bootstrap_Impl * b = BootstrapMapSingleton::get().getBaseIni();
    if (b != this && b->getDirectValue(key, value, mode, requestStack)) {
        return true;
    }
    if (!override && getDirectValue(key, value, mode, requestStack)) {
        return true;
    }
    if (mode == LOOKUP_MODE_NORMAL) {
        b = BootstrapMapSingleton::get().getFundamentalIni();
        if (b != 0 && b != this &&
            b->getDirectValue(key, value, mode, requestStack))
        {
            return true;
        }
    }
    if (defaultValue != NULL) {
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
    if (get(key, &v)) {
        expandValue(value, v, mode, this, key, requestStack);
        return true;
    } else {
        return false;
    }
}

bool Bootstrap_Impl::getAmbienceValue(
    rtl::OUString const & key, rtl_uString ** value, LookupMode mode,
    ExpandRequestLink const * requestStack) const
{
    rtl::OUString v;
    if (ExplicitParameters::get().get(key, &v) ||
        CommandLineParameters::get().get(key, &v) ||
        osl_getEnvironment(key.pData, &v.pData) == osl_Process_E_None)
    {
        expandValue(value, v, mode, NULL, key, requestStack);
        return true;
    } else {
        return false;
    }
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

rtlBootstrapHandle SAL_CALL rtl_bootstrap_args_open (
    rtl_uString * pIniName
) SAL_THROW_EXTERN_C()
{
    return BootstrapMapSingleton::get().getIni(rtl::OUString(pIniName), false);
}

//----------------------------------------------------------------------------

void SAL_CALL rtl_bootstrap_args_close (
    SAL_UNUSED_PARAMETER rtlBootstrapHandle
) SAL_THROW_EXTERN_C()
{
    // do nothing; the BootstrapMap::map_ just keeps growing for now
}

//----------------------------------------------------------------------------

sal_Bool SAL_CALL rtl_bootstrap_get_from_handle(
    rtlBootstrapHandle handle,
    rtl_uString      * pName,
    rtl_uString     ** ppValue,
    rtl_uString      * pDefault
) SAL_THROW_EXTERN_C()
{
    return
        (handle == 0
         ? BootstrapMapSingleton::get().getBaseIni()
         : static_cast< Bootstrap_Impl * >(handle))->
        getValue(pName, ppValue, pDefault, LOOKUP_MODE_NORMAL, false, 0);
}

//----------------------------------------------------------------------------

void SAL_CALL rtl_bootstrap_get_iniName_from_handle (
    rtlBootstrapHandle handle,
    rtl_uString     ** ppIniName
) SAL_THROW_EXTERN_C()
{
    rtl_uString_assign(
        ppIniName,
        ((handle == 0
          ? BootstrapMapSingleton::get().getBaseIni()
          : static_cast<Bootstrap_Impl*>(handle))->
         _iniName.pData));
}

//----------------------------------------------------------------------------

void SAL_CALL rtl_bootstrap_setIniFileName (
    rtl_uString * pName
) SAL_THROW_EXTERN_C()
{
    BootstrapMapSingleton::get().setBaseIniUri(rtl::OUString(pName));
}

//----------------------------------------------------------------------------

sal_Bool SAL_CALL rtl_bootstrap_get (
    rtl_uString  * pName,
    rtl_uString ** ppValue,
    rtl_uString  * pDefault
) SAL_THROW_EXTERN_C()
{
    return rtl_bootstrap_get_from_handle(0, pName, ppValue, pDefault);
}

//----------------------------------------------------------------------------

void SAL_CALL rtl_bootstrap_set (
    rtl_uString * pName,
    rtl_uString * pValue
) SAL_THROW_EXTERN_C()
{
    ExplicitParameters::get().set(rtl::OUString(pName), rtl::OUString(pValue));
}

//----------------------------------------------------------------------------

void SAL_CALL rtl_bootstrap_expandMacros_from_handle (
    rtlBootstrapHandle handle,
    rtl_uString     ** macro
) SAL_THROW_EXTERN_C()
{
    rtl::OUString expanded(
        expandMacros(
            (handle == 0
             ? BootstrapMapSingleton::get().getBaseIni()
             : static_cast< Bootstrap_Impl * >(handle)),
            *reinterpret_cast< OUString const * >(macro), LOOKUP_MODE_NORMAL,
            0));
    rtl_uString_assign(macro, expanded.pData);
}

//----------------------------------------------------------------------------

void SAL_CALL rtl_bootstrap_expandMacros(
    rtl_uString ** macro )
    SAL_THROW_EXTERN_C()
{
    rtl_bootstrap_expandMacros_from_handle(NULL, macro);
}

void rtl_bootstrap_encode( rtl_uString const * value, rtl_uString ** encoded )
    SAL_THROW_EXTERN_C()
{
    OSL_ASSERT(value != NULL);
    rtl::OUStringBuffer b;
    for (sal_Int32 i = 0; i < value->length; ++i) {
        sal_Unicode c = value->buffer[i];
        if (c == '$' || c == '\\') {
            b.append(sal_Unicode('\\'));
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
        pos != NULL && *pos >= 0 && *pos < text.getLength() && escaped != NULL);
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
        } else if (*pos < text.getLength()) {
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
    (file == 0 ? BootstrapMapSingleton::get().getBaseIni() : file)->getValue(
        key, &v.pData, NULL, mode, override, requestStack);
    return v;
}

rtl::OUString expandMacros(
    Bootstrap_Impl const * file, rtl::OUString const & text, LookupMode mode,
    ExpandRequestLink const * requestStack)
{
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
                } else if (n == 2 && seg[0] == ".link") {
                    osl::File f(seg[1]);
                    rtl::ByteSequence seq;
                    rtl::OUString line;
                    rtl::OUString url;
                    // Silently ignore any errors (is that good?):
                    if ((f.open(osl_File_OpenFlag_Read) ==
                         osl::FileBase::E_None) &&
                        f.readLine(seq) == osl::FileBase::E_None &&
                        rtl_convertStringToUString(
                            &line.pData,
                            reinterpret_cast< char const * >(
                                seq.getConstArray()),
                            seq.getLength(), RTL_TEXTENCODING_UTF8,
                            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR |
                             RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR |
                             RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)) &&
                        (osl::File::getFileURLFromSystemPath(line, url) ==
                         osl::FileBase::E_None))
                    {
                        try {
                            buf.append(
                                rtl::Uri::convertRelToAbs(seg[1], url));
                        } catch (const rtl::MalformedUriException &) {}
                    }
                } else if (n == 3 && seg[0] == ".override") {
                    rtl::Bootstrap b(seg[1]);
                    Bootstrap_Impl * f = static_cast< Bootstrap_Impl * >(
                        b.getHandle());
                    buf.append(
                        lookup(f, mode, f != NULL, seg[2], requestStack));
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
    return buf.makeStringAndClear();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
