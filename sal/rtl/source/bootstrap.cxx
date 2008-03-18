/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bootstrap.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 13:17:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include "rtl/bootstrap.h"
#include "rtl/bootstrap.hxx"

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#include <osl/profile.hxx>

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#include <rtl/ustrbuf.hxx>
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_BYTESEQ_HXX_
#include <rtl/byteseq.hxx>
#endif
#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif
#include <rtl/malformeduriexception.hxx>
#include <rtl/uri.hxx>

#ifndef INCLUDED_RTL_ALLOCATOR_HXX
#include "rtl/allocator.hxx"
#endif

#include "macro.hxx"

#include <hash_map>
#include <list>

//----------------------------------------------------------------------------

using osl::DirectoryItem;
using osl::FileStatus;

using rtl::OString;
using rtl::OUString;
using rtl::OUStringToOString;

struct Bootstrap_Impl;

namespace {

static char const URE_BOOTSTRAP[] = "URE_BOOTSTRAP";

struct FundamentalIniData {
    rtlBootstrapHandle ini;

    FundamentalIniData() {
        OUString uri;
        ini =
            rtl::Bootstrap::get(
                OUString(RTL_CONSTASCII_USTRINGPARAM(URE_BOOTSTRAP)), uri)
            ? rtl_bootstrap_args_open(uri.pData) : NULL;
    }

    ~FundamentalIniData() { rtl_bootstrap_args_close(ini); }

private:
    FundamentalIniData(FundamentalIniData &); // not defined
    void operator =(FundamentalIniData &); // not defined
};

struct FundamentalIni: public rtl::Static< FundamentalIniData, FundamentalIni >
{};

struct ExpandRequestLink {
    ExpandRequestLink const * next;
    Bootstrap_Impl const * file;
    rtl::OUString key;
};

rtl::OUString expandMacros(
    Bootstrap_Impl const * file, rtl::OUString const & text,
    ExpandRequestLink const * requestStack);

}

//----------------------------------------------------------------------------

struct rtl_bootstrap_NameValue
{
    OUString sName;
    OUString sValue;

    inline rtl_bootstrap_NameValue() SAL_THROW( () )
        {}
    inline rtl_bootstrap_NameValue(
        OUString const & name, OUString const & value ) SAL_THROW( () )
        : sName( name ),
          sValue( value )
        {}
};

typedef std::list<
    rtl_bootstrap_NameValue,
    rtl::Allocator< rtl_bootstrap_NameValue >
> NameValueList;

namespace {
    struct rtl_bootstrap_set_list :
        public rtl::Static< NameValueList, rtl_bootstrap_set_list > {};
}

//----------------------------------------------------------------------------

static sal_Bool getFromCommandLineArgs(
    rtl_uString **ppValue , rtl_uString *pName )
{
    static NameValueList *pNameValueList = 0;
    if( ! pNameValueList )
    {
        static NameValueList nameValueList;

        sal_Int32 nArgCount = osl_getCommandArgCount();
        for(sal_Int32 i = 0; i < nArgCount; ++ i)
        {
            rtl_uString *pArg = 0;
            osl_getCommandArg( i, &pArg );
            if( ('-' == pArg->buffer[0] || '/' == pArg->buffer[0] ) &&
                'e' == pArg->buffer[1] &&
                'n' == pArg->buffer[2] &&
                'v' == pArg->buffer[3] &&
                ':' == pArg->buffer[4] )
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

    sal_Bool found = sal_False;

    OUString name( pName );
    for( NameValueList::iterator ii = pNameValueList->begin() ;
         ii != pNameValueList->end() ;
         ++ii )
    {
        if( (*ii).sName.equals(name) )
        {
            rtl_uString_assign( ppValue, (*ii).sValue.pData );
            found = sal_True;
            break;
        }
    }

    return found;
}

//----------------------------------------------------------------------------

extern "C" oslProcessError SAL_CALL osl_bootstrap_getExecutableFile_Impl (
    rtl_uString ** ppFileURL) SAL_THROW_EXTERN_C();

inline void getExecutableFile_Impl (rtl_uString ** ppFileURL)
{
    osl_bootstrap_getExecutableFile_Impl (ppFileURL);
}

//----------------------------------------------------------------------------

static void getExecutableDirectory_Impl (rtl_uString ** ppDirURL)
{
    OUString fileName;
    getExecutableFile_Impl (&(fileName.pData));

    sal_Int32 nDirEnd = fileName.lastIndexOf('/');
    OSL_ENSURE(nDirEnd >= 0, "Cannot locate executable directory");

    rtl_uString_newFromStr_WithLength(ppDirURL,fileName.getStr(),nDirEnd);
}

//----------------------------------------------------------------------------

static OUString & getIniFileName_Impl()
{
    static OUString *pStaticName = 0;
    if( ! pStaticName )
    {
        OUString fileName;

        if(getFromCommandLineArgs(
               &fileName.pData,
               OUString(RTL_CONSTASCII_USTRINGPARAM("INIFILENAME")).pData))
        {
            // do nothing
        }
        else if(getFromCommandLineArgs(
                    &fileName.pData,
                    OUString(RTL_CONSTASCII_USTRINGPARAM("INIFILEPATH")).pData))
        {
            OUString url;
            if (osl::FileBase::getFileURLFromSystemPath(fileName, url) ==
                osl::FileBase::E_None)
            {
                fileName = url;
            }
            else
            {
                fileName = OUString();
            }
        }
        else
        {
            getExecutableFile_Impl (&(fileName.pData));

            // get rid of a potential executable extension
            OUString progExt (RTL_CONSTASCII_USTRINGPARAM(".bin"));
            if(fileName.getLength() > progExt.getLength()
            && fileName.copy(fileName.getLength() - progExt.getLength()).equalsIgnoreAsciiCase(progExt))
                fileName = fileName.copy(0, fileName.getLength() - progExt.getLength());

            progExt = OUString::createFromAscii(".exe");
            if(fileName.getLength() > progExt.getLength()
            && fileName.copy(fileName.getLength() - progExt.getLength()).equalsIgnoreAsciiCase(progExt))
                fileName = fileName.copy(0, fileName.getLength() - progExt.getLength());

            // append config file suffix
            fileName += OUString(RTL_CONSTASCII_USTRINGPARAM(SAL_CONFIGFILE("")));
        }

        OUString workDir;
        osl_getProcessWorkingDir(&workDir.pData);
        osl::FileBase::getAbsoluteFileURL(workDir, fileName, fileName);

        static OUString theFileName;
        if(fileName.getLength())
            theFileName = fileName;

        pStaticName = &theFileName;
    }

    return *pStaticName;
}

//----------------------------------------------------------------------------

static void getFromEnvironment( rtl_uString **ppValue, rtl_uString *pName )
{
    if( osl_Process_E_None != osl_getEnvironment( pName , ppValue ) )
    {
        // osl behaves different on win or unx.
        if( *ppValue )
        {
            rtl_uString_release( *ppValue );
            *ppValue = 0;
        }
    }

}

//----------------------------------------------------------------------------

static void getFromList(
    NameValueList const  * pNameValueList,
    rtl_uString         ** ppValue,
    rtl_uString          * pName )
{
    OUString const & name = * reinterpret_cast< OUString const * >( &pName );

    NameValueList::const_iterator iEnd( pNameValueList->end() );
    for( NameValueList::const_iterator ii = pNameValueList->begin(); ii != iEnd; ++ii )
    {
        if( (*ii).sName.equals(name) )
        {
            rtl_uString_assign( ppValue, (*ii).sValue.pData );
            break;
        }
    }
}

//----------------------------------------------------------------------------

static inline bool path_exists( OUString const & path )
{
    DirectoryItem dirItem;
    return (DirectoryItem::E_None == DirectoryItem::get( path, dirItem ));
}

//----------------------------------------------------------------------------
// #111772#
// ensure the given file url has no final slash

inline void EnsureNoFinalSlash (rtl_uString** file_url)
{
    sal_Int32 l = rtl_uString_getLength(*file_url);
    if (rtl_ustr_lastIndexOfChar((*file_url)->buffer, '/') == (l-1))
    {
        (*file_url)->buffer[l-1] = 0;
        (*file_url)->length--;
    }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

struct Bootstrap_Impl
{
    sal_Int32 _nRefCount;
    Bootstrap_Impl * _base_ini;

    NameValueList _nameValueList;
    OUString      _iniName;

    explicit Bootstrap_Impl (OUString const & rIniName);
    ~Bootstrap_Impl();

    static void * operator new (std::size_t n) SAL_THROW(())
        { return rtl_allocateMemory (sal_uInt32(n)); }
    static void operator delete (void * p , std::size_t) SAL_THROW(())
        { rtl_freeMemory (p); }

    sal_Bool getValue( rtl_uString * pName, rtl_uString ** ppValue, rtl_uString * pDefault, bool recursive, ExpandRequestLink const * requestStack ) const;
};

//----------------------------------------------------------------------------

Bootstrap_Impl::Bootstrap_Impl( OUString const & rIniName )
    : _nRefCount( 0 ),
      _base_ini( 0 ),
      _iniName (rIniName)
{
    OUString base_ini( getIniFileName_Impl() );
    // normalize path
    FileStatus status( FileStatusMask_FileURL );
    DirectoryItem dirItem;
    if (DirectoryItem::E_None == DirectoryItem::get( base_ini, dirItem ) &&
        DirectoryItem::E_None == dirItem.getFileStatus( status ))
    {
        base_ini = status.getFileURL();
        if (! rIniName.equals( base_ini ))
        {
            _base_ini = static_cast< Bootstrap_Impl * >(
                rtl_bootstrap_args_open( base_ini.pData ) );
        }
    }

#if OSL_DEBUG_LEVEL > 1
    OString sFile = OUStringToOString(_iniName, RTL_TEXTENCODING_ASCII_US);
    OSL_TRACE(__FILE__" -- Bootstrap_Impl() - %s\n", sFile.getStr());
#endif /* OSL_DEBUG_LEVEL > 1 */

    oslFileHandle handle;
    if (_iniName.getLength() &&
        osl_File_E_None == osl_openFile(_iniName.pData, &handle, osl_File_OpenFlag_Read))
    {
        rtl::ByteSequence seq;

        while (osl_File_E_None == osl_readLine(handle , (sal_Sequence **)&seq))
        {
            OString line( (const sal_Char *) seq.getConstArray(), seq.getLength() );
            sal_Int32 nIndex = line.indexOf('=');
            if (nIndex >= 1)
            {
                struct rtl_bootstrap_NameValue nameValue;
                nameValue.sName = OStringToOUString(
                    line.copy(0,nIndex).trim(), RTL_TEXTENCODING_ASCII_US );
                nameValue.sValue = OStringToOUString(
                    line.copy(nIndex+1).trim(), RTL_TEXTENCODING_UTF8 );

#if OSL_DEBUG_LEVEL > 1
                OString name_tmp = OUStringToOString(nameValue.sName, RTL_TEXTENCODING_ASCII_US);
                OString value_tmp = OUStringToOString(nameValue.sValue, RTL_TEXTENCODING_UTF8);
                OSL_TRACE(
                    __FILE__" -- pushing: name=%s value=%s\n",
                    name_tmp.getStr(), value_tmp.getStr() );
#endif /* OSL_DEBUG_LEVEL > 1 */

                _nameValueList.push_back(nameValue);
            }
        }
        osl_closeFile(handle);
    }
#if OSL_DEBUG_LEVEL > 1
    else
    {
        OString file_tmp = OUStringToOString(_iniName, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE( __FILE__" -- couldn't open file: %s", file_tmp.getStr() );
    }
#endif /* OSL_DEBUG_LEVEL > 1 */
}

//----------------------------------------------------------------------------

Bootstrap_Impl::~Bootstrap_Impl()
{
    if (_base_ini != 0)
        rtl_bootstrap_args_close( _base_ini );
}

//----------------------------------------------------------------------------

sal_Bool Bootstrap_Impl::getValue(
    rtl_uString * pName, rtl_uString ** ppValue, rtl_uString * pDefault,
    bool recursive, ExpandRequestLink const * requestStack ) const
{
    // lookup this ini
    sal_Bool result = sal_True;
    bool further_macro_expansion = true;

    OUString const & name = *reinterpret_cast< OUString const * >( &pName );
    ExpandRequestLink link = { requestStack, this, name };
    if (name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("_CPPU_ENV") ))
    {
#define MSTR_(x) # x
#define MSTR(x) MSTR_(x)
#define STR_CPPU_ENV() MSTR(CPPU_ENV)
        OUString val( RTL_CONSTASCII_USTRINGPARAM(STR_CPPU_ENV()) );
        rtl_uString_assign( ppValue, val.pData );
        further_macro_expansion = false;
    }
    else if (name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("ORIGIN") ))
    {
        OUString iniPath;
        sal_Int32 last_slash = _iniName.lastIndexOf( '/' );
        if (last_slash >= 0)
            iniPath = _iniName.copy( 0, last_slash );
        rtl_uString_assign( ppValue, iniPath.pData );
        further_macro_expansion = false;
    }
    else if (name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("_OS") ))
    {
        OUString val( RTL_CONSTASCII_USTRINGPARAM(THIS_OS) );
        rtl_uString_assign( ppValue, val.pData );
        further_macro_expansion = false;
    }
    else if (name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("_ARCH") ))
    {
        OUString val( RTL_CONSTASCII_USTRINGPARAM(THIS_ARCH) );
        rtl_uString_assign( ppValue, val.pData );
        further_macro_expansion = false;
    }
    else
    {
        if (0 != *ppValue)
        {
            rtl_uString_release( *ppValue );
            *ppValue = 0;
        }
        getFromList( &rtl_bootstrap_set_list::get(), ppValue, pName );
        if (! *ppValue)
        {
            getFromCommandLineArgs( ppValue, pName );
            if(!*ppValue)
            {
                getFromEnvironment( ppValue, pName );
                if( ! *ppValue )
                {
                    if (name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("SYSUSERCONFIG") ))
                    {
                        oslSecurity security = osl_getCurrentSecurity();
                        osl_getConfigDir(security, ppValue);
                        EnsureNoFinalSlash(ppValue);
                        osl_freeSecurityHandle(security);
                        further_macro_expansion = false;
                    }
                    else if (name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("SYSUSERHOME") ))
                    {
                        oslSecurity security = osl_getCurrentSecurity();
                        osl_getHomeDir(security, ppValue);
                        EnsureNoFinalSlash(ppValue);
                        osl_freeSecurityHandle(security);
                        further_macro_expansion = false;
                    }
                    else if (name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("SYSBINDIR") ))
                    {
                        getExecutableDirectory_Impl(ppValue);
                        EnsureNoFinalSlash(ppValue);
                        further_macro_expansion = false;
                    }
                    else if (!recursive && _base_ini && (_base_ini->getValue(pName, ppValue, NULL, true, &link)))
                    {
                        further_macro_expansion = false;
                    }
                    else
                    {
                        // note:  _base_ini->getValue assigns ppValue in any case
                        if (0 != *ppValue)
                        {
                            rtl_uString_release( *ppValue );
                            *ppValue = 0;
                        }

                        getFromList( &_nameValueList, ppValue, pName );
                        if (*ppValue == NULL && !recursive &&
                            !name.equalsAsciiL(
                                RTL_CONSTASCII_STRINGPARAM(URE_BOOTSTRAP)))
                        {
                            FundamentalIniData & d = FundamentalIni::get();
                            if (d.ini != NULL && d.ini != this) {
                                if (static_cast< Bootstrap_Impl * >(d.ini)->
                                    getValue(pName, ppValue, NULL, true, &link))
                                {
                                    further_macro_expansion = false;
                                } else {
                                    rtl_uString_release(*ppValue);
                                    *ppValue = NULL;
                                }
                            }
                        }
                        if( ! *ppValue )
                        {
                            if(pDefault) {
                                rtl_uString_assign( ppValue, pDefault );
                                further_macro_expansion = false;
                            }
                            else
                                result = sal_False;
                        }
                    }
                }
            }
        }
    }

    if (0 == *ppValue)
    {
        rtl_uString_new(ppValue);
    }
    else
    {
        if (further_macro_expansion)
        {
            OUString val = expandMacros(
                this, OUString::unacquired(ppValue), &link );
            rtl_uString_assign( ppValue, val.pData );
        }
    }

#if OSL_DEBUG_LEVEL > 1
    OString sName = OUStringToOString(OUString(pName), RTL_TEXTENCODING_ASCII_US);
    OString sValue = OUStringToOString(OUString(*ppValue), RTL_TEXTENCODING_ASCII_US);
    OSL_TRACE(
    __FILE__ " -- Bootstrap_Impl::getValue() - name:%s value:%s result:%i\n",
    sName.getStr(), sValue.getStr(), result );
#endif /* OSL_DEBUG_LEVEL > 1 */

    return result;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

namespace {

struct bootstrap_map {
    // map<> may be preferred here, but hash_map<> is implemented fully inline,
    // thus there is no need to link against the stlport:
    typedef std::hash_map<
        rtl::OUString, Bootstrap_Impl *,
        rtl::OUStringHash, std::equal_to< rtl::OUString >,
        rtl::Allocator< OUString > > t;

    // get and release must only be called properly synchronized via some mutex
    // (e.g., osl::Mutex::getGlobalMutex()):

    static t * get() {
        if (m_map == NULL) {
            m_map = new t;
        }
        return m_map;
    }

    static void release() {
        if (m_map != NULL && m_map->empty()) {
            delete m_map;
            m_map = NULL;
        }
    }

private:
    bootstrap_map(); // not defined

    static t * m_map;
};

bootstrap_map::t * bootstrap_map::m_map = NULL;

}

//----------------------------------------------------------------------------

rtlBootstrapHandle SAL_CALL rtl_bootstrap_args_open (
    rtl_uString * pIniName
) SAL_THROW_EXTERN_C()
{
    OUString workDir;
    OUString iniName( pIniName );

    osl_getProcessWorkingDir( &workDir.pData );
    osl::FileBase::getAbsoluteFileURL( workDir, iniName, iniName );

    // normalize path
    FileStatus status( FileStatusMask_FileURL );
    DirectoryItem dirItem;
    if (DirectoryItem::E_None != DirectoryItem::get( iniName, dirItem ) ||
        DirectoryItem::E_None != dirItem.getFileStatus( status ))
    {
        return 0;
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

//----------------------------------------------------------------------------

void SAL_CALL rtl_bootstrap_args_close (
    rtlBootstrapHandle handle
) SAL_THROW_EXTERN_C()
{
    if (handle == 0)
        return;
    Bootstrap_Impl * that = static_cast< Bootstrap_Impl * >( handle );

    osl::MutexGuard guard( osl::Mutex::getGlobalMutex() );
    bootstrap_map::t* p_bootstrap_map = bootstrap_map::get();
    OSL_ASSERT(
        p_bootstrap_map->find( that->_iniName )->second == that );
    --that->_nRefCount;
    if (that->_nRefCount == 0)
    {
        ::std::size_t nLeaking = 8; // only hold up to 8 files statically

#if OSL_DEBUG_LEVEL == 1 // nonpro
        nLeaking = 0;
#elif OSL_DEBUG_LEVEL > 1 // debug
        nLeaking = 1;
#endif /* OSL_DEBUG_LEVEL */

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

//----------------------------------------------------------------------------

static rtlBootstrapHandle get_static_bootstrap_handle() SAL_THROW(())
{
    osl::MutexGuard guard( osl::Mutex::getGlobalMutex() );
    static rtlBootstrapHandle s_handle = 0;
    if (s_handle == 0)
    {
        OUString iniName (getIniFileName_Impl());
        s_handle = rtl_bootstrap_args_open( iniName.pData );
        if (s_handle == 0)
        {
            Bootstrap_Impl * that = new Bootstrap_Impl( iniName );
            ++that->_nRefCount;
            s_handle = static_cast<rtlBootstrapHandle>(that);
        }
    }
    return s_handle;
}

//----------------------------------------------------------------------------

sal_Bool SAL_CALL rtl_bootstrap_get_from_handle (
    rtlBootstrapHandle handle,
    rtl_uString      * pName,
    rtl_uString     ** ppValue,
    rtl_uString      * pDefault
) SAL_THROW_EXTERN_C()
{
    osl::MutexGuard guard( osl::Mutex::getGlobalMutex() );

    sal_Bool found = sal_False;
    if(ppValue && pName)
    {
        if (handle == 0)
            handle = get_static_bootstrap_handle();
        found = static_cast< Bootstrap_Impl * >( handle )->getValue(
            pName, ppValue, pDefault, false, NULL );
    }

    return found;
}

//----------------------------------------------------------------------------

void SAL_CALL rtl_bootstrap_get_iniName_from_handle (
    rtlBootstrapHandle handle,
    rtl_uString     ** ppIniName
) SAL_THROW_EXTERN_C()
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

//----------------------------------------------------------------------------

void SAL_CALL rtl_bootstrap_setIniFileName (
    rtl_uString * pName
) SAL_THROW_EXTERN_C()
{
    osl::MutexGuard guard( osl::Mutex::getGlobalMutex() );
    OUString & file = getIniFileName_Impl();
    file = pName;
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
    OUString const & name = *reinterpret_cast< OUString const * >( &pName );
    OUString const & value = *reinterpret_cast< OUString const * >( &pValue );

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

#if OSL_DEBUG_LEVEL > 1
    OString cstr_name( OUStringToOString( name, RTL_TEXTENCODING_ASCII_US ) );
    OString cstr_value( OUStringToOString( value, RTL_TEXTENCODING_ASCII_US ) );
    OSL_TRACE(
        "bootstrap.cxx: explicitly setting: name=%s value=%s\n",
        cstr_name.getStr(), cstr_value.getStr() );
#endif /* OSL_DEBUG_LEVEL > 1 */

    r_rtl_bootstrap_set_list.push_back( rtl_bootstrap_NameValue( name, value ) );
}

//----------------------------------------------------------------------------

void SAL_CALL rtl_bootstrap_expandMacros_from_handle (
    rtlBootstrapHandle handle,
    rtl_uString     ** macro
) SAL_THROW_EXTERN_C()
{
    if (handle == NULL) {
        handle = get_static_bootstrap_handle();
    }
    OUString expanded( expandMacros( static_cast< Bootstrap_Impl * >( handle ),
                                     * reinterpret_cast< OUString const * >( macro ),
                                     NULL ) );
    rtl_uString_assign( macro, expanded.pData );
}

//----------------------------------------------------------------------------

void SAL_CALL rtl_bootstrap_expandMacros(
    rtl_uString ** macro )
    SAL_THROW_EXTERN_C()
{
    rtl_bootstrap_expandMacros_from_handle(NULL, macro);
}

//----------------------------------------------------------------------------

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
    Bootstrap_Impl const * file, rtl::OUString const & key,
    ExpandRequestLink const * requestStack)
{
    OSL_ASSERT(file != NULL);
    for (; requestStack != NULL; requestStack = requestStack->next) {
        if (requestStack->file == file && requestStack->key == key) {
            return rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("***RECURSION DETECTED***"));
        }
    }
    rtl::OUString v;
    file->getValue(key.pData, &v.pData, NULL, false, requestStack);
    return v;
}

rtl::OUString expandMacros(
    Bootstrap_Impl const * file, rtl::OUString const & text,
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
                    seg[j] = expandMacros(file, seg[j], requestStack);
                }
                if (n == 3 && seg[1].getLength() == 0) {
                    // For backward compatibility, treat ${file::key} the same
                    // as just ${file:key}:
                    seg[1] = seg[2];
                    n = 2;
                }
                if (n == 1) {
                    buf.append(lookup(file, seg[0], requestStack));
                } else if (n == 2) {
                    if (seg[0].equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM(".link")))
                    {
                        osl::File f(seg[1]);
                        rtl::ByteSequence seq;
                        rtl::OUString line;
                        rtl::OUString url;
                        // Silently ignore any errors (is that good?):
                        if (f.open(OpenFlag_Read) == osl::FileBase::E_None &&
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
                            } catch (rtl::MalformedUriException &) {}
                        }
                    } else {
                        rtl::Bootstrap b(seg[0]);
                        Bootstrap_Impl * f = static_cast< Bootstrap_Impl * >(
                            b.getHandle());
                        // Silently ignore bootstrap files that cannot be opened
                        // (is that good?):
                        if (f != NULL) {
                            buf.append(lookup(f, seg[1], requestStack));
                        }
                    }
                } else {
                    // Going through osl::Profile, this code erroneously does
                    // not recursively expand macros in the resulting
                    // replacement text (and if it did, it would fail to detect
                    // cycles that pass through here):
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
                    lookup(file, kbuf.makeStringAndClear(), requestStack));
            }
        }
    }
    return buf.makeStringAndClear();
}

}
