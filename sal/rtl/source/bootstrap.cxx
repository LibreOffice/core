/*************************************************************************
 *
 *  $RCSfile: bootstrap.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: hr $ $Date: 2003-07-16 17:22:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <memory>

#include "macro.hxx"

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
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

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif
#ifndef _RTL_BOOTSTRAP_H_
#include <rtl/bootstrap.h>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_BYTESEQ_HXX_
#include <rtl/byteseq.hxx>
#endif

#ifndef INCLUDED_LIST
#include <list>
#define INCLUDED_LIST
#endif

using rtl::OString;
using rtl::OUString;

using rtl::OStringToOUString;
using rtl::OUStringToOString;

//----------------------------------------------------------------------------

template<typename T>
struct MyAllocator
{
    typedef std::size_t    size_type;
    typedef std::ptrdiff_t difference_type;

    typedef T * pointer;
    typedef const T * const_pointer;

    typedef T & reference;
    typedef const T & const_reference;

    typedef T value_type;

    template<typename U>
    struct rebind
    {
        typedef MyAllocator<U> other;
    };

    pointer address (reference value) const
    {
        return &value;
    }
    const_pointer address (const_reference value) const
    {
        return &value;
    }

    MyAllocator (void)
    {}

    template<typename U>
    MyAllocator (const MyAllocator<U> &)
    {}

    MyAllocator (const MyAllocator &)
    {}

    ~MyAllocator (void)
    {}

    size_type max_size() const
    {
        return size_type(-1)/sizeof(T);
    }

    pointer allocate (size_type n, void const * = 0)
    {
        n *= sizeof(T);
        return (pointer)rtl_allocateMemory(sal_uInt32(n));
    }
    void deallocate (pointer p, size_type n)
    {
        n *= sizeof(T);
        rtl_freeMemory(p);
    }

    void construct (pointer p, const_reference value)
    {
        new ((void*)p) T(value);
    }
    void destroy (pointer p)
    {
        p->~T();
    }
};

//----------------------------------------------------------------------------

template<typename T, typename U>
inline bool operator== (const MyAllocator<T> &, const MyAllocator<U> &)
{
    return true;
}

template<typename T, typename U>
inline bool operator!= (const MyAllocator<T> &, const MyAllocator<U> &)
{
    return false;
}

//----------------------------------------------------------------------------
// see stlport '_alloc.h' comments why old compilers require the hack below.
//----------------------------------------------------------------------------

#ifndef __STL_MEMBER_TEMPLATE_CLASSES
namespace _STL
{
    template<typename T, typename U>
    inline MyAllocator<U> & __stl_alloc_rebind (MyAllocator<T> & a, U const *)
    {
        return (MyAllocator<U>&)(a);
    }
}
#endif /* __STL_MEMBER_TEMPLATE_CLASSES */

//----------------------------------------------------------------------------


struct rtl_bootstrap_NameValue
{
    rtl::OUString sName;
    rtl::OUString sValue;

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
    MyAllocator< rtl_bootstrap_NameValue >
> NameValueList;

static sal_Bool getFromCommandLineArgs( rtl_uString **ppValue , rtl_uString *pName )
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

static ::rtl::OUString &getIniFileNameImpl()
{
    static OUString *pStaticName = 0;
    if( ! pStaticName )
    {
        OUString fileName;

        OUString sVarName(RTL_CONSTASCII_USTRINGPARAM("INIFILENAME"));
        if(!getFromCommandLineArgs(&fileName.pData, sVarName.pData))
        {
            osl_getExecutableFile(&fileName.pData);

            // get rid of a potential executable extension
            OUString progExt = OUString::createFromAscii(".bin");
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

        static OUString theFileName;
        if(fileName.getLength())
            theFileName = fileName;

        pStaticName = &theFileName;
    }

    return *pStaticName;
}

static void getExecutableDirectory(rtl_uString **ppDirURL)
{
    OUString fileName;
    osl_getExecutableFile(&fileName.pData);

    sal_Int32 nDirEnd = fileName.lastIndexOf('/');

    OSL_ENSURE(nDirEnd >= 0, "Cannot locate executable directory");

    rtl_uString_newFromStr_WithLength(ppDirURL,fileName.getStr(),nDirEnd);
}

static void getFileSize( oslFileHandle handle, sal_uInt64 *pSize )
{
    sal_uInt64 nOldPos=0;
    OSL_VERIFY( osl_File_E_None == osl_getFilePos( handle, &nOldPos ) &&
                osl_File_E_None == osl_setFilePos( handle, osl_Pos_End , 0 ) &&
                osl_File_E_None == osl_getFilePos( handle, pSize ) &&
                osl_File_E_None == osl_setFilePos( handle, osl_Pos_Absolut, nOldPos ) );
}

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

static void getFromList(
    NameValueList const * pNameValueList, rtl_uString **ppValue, rtl_uString *pName )
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

static NameValueList s_rtl_bootstrap_set_list;

struct Bootstrap_Impl
{
    ::std::auto_ptr< Bootstrap_Impl > _base_ini;

    NameValueList _nameValueList;
    OUString      _iniName;

    explicit Bootstrap_Impl (OUString const & rIniName);
    ~Bootstrap_Impl();

    static void * operator new (std::size_t n) SAL_THROW(())
        { return rtl_allocateMemory (sal_uInt32(n)); }
    static void operator delete (void * p , std::size_t) SAL_THROW(())
        { rtl_freeMemory (p); }

    sal_Bool getValue( rtl_uString * pName, rtl_uString ** ppValue, rtl_uString * pDefault ) const;
};

static inline bool path_exists( ::rtl::OUString const & path )
{
    ::osl::DirectoryItem dirItem;
    return (::osl::DirectoryItem::E_None ==
            ::osl::DirectoryItem::get( path, dirItem ));
}

Bootstrap_Impl::Bootstrap_Impl( OUString const & rIniName )
    : _iniName (rIniName)
{
    OUString const & base_ini = getIniFileNameImpl();
    if (!rIniName.equals( base_ini ) && path_exists( base_ini ))
    {
        _base_ini.reset( new Bootstrap_Impl( base_ini ) );
    }

#if OSL_DEBUG_LEVEL > 1
    OString sFile = OUStringToOString(_iniName, RTL_TEXTENCODING_ASCII_US);
    OSL_TRACE(__FILE__" -- Bootstrap_Impl() - %s\n", sFile.getStr());
#endif
    oslFileHandle handle;
    if (_iniName.getLength() &&
        osl_File_E_None == osl_openFile(_iniName.pData, &handle, osl_File_OpenFlag_Read))
    {
        rtl::ByteSequence seq;
        sal_uInt64 nSize = 0;

        getFileSize(handle, &nSize);
        while (true)
        {
            sal_uInt64 nPos;
            if (osl_File_E_None != osl_getFilePos(handle, &nPos) || nPos >= nSize)
                break;
            if (osl_File_E_None != osl_readLine(handle , (sal_Sequence **) &seq))
                break;
            OString line( (const sal_Char *) seq.getConstArray(), seq.getLength() );
            sal_Int32 nIndex = line.indexOf('=');
            struct rtl_bootstrap_NameValue nameValue;
            if (nIndex >= 1 && nIndex +1 < line.getLength())
            {
                nameValue.sName = OStringToOUString(
                    line.copy(0,nIndex).trim(), RTL_TEXTENCODING_ASCII_US );
                nameValue.sValue = OStringToOUString(
                    line.copy(nIndex+1).trim(), RTL_TEXTENCODING_UTF8 );
                OString name_tmp = OUStringToOString(nameValue.sName, RTL_TEXTENCODING_ASCII_US);
                OString value_tmp = OUStringToOString(nameValue.sValue, RTL_TEXTENCODING_UTF8);
#if OSL_DEBUG_LEVEL > 1
                OSL_TRACE(
                    __FILE__" -- pushing: name=%s value=%s\n",
                    name_tmp.getStr(), value_tmp.getStr() );
#endif
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
#endif
}

Bootstrap_Impl::~Bootstrap_Impl()
{
}


sal_Bool Bootstrap_Impl::getValue(
    rtl_uString * pName, rtl_uString ** ppValue, rtl_uString * pDefault ) const
{
    // lookup this ini
    sal_Bool result = sal_True;
    bool further_macro_expansion = true;

    OUString const & name = *reinterpret_cast< OUString const * >( &pName );
    if (name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("ORIGIN") ))
    {
        OUString iniPath;
        sal_Int32 last_slash = _iniName.lastIndexOf( '/' );
        if (last_slash >= 0)
            iniPath = _iniName.copy( 0, last_slash );
        rtl_uString_assign( ppValue, iniPath.pData );
        further_macro_expansion = false;
    }
    else
    {
        if (0 != _base_ini.get())
        {
            if (_base_ini->getValue( pName, ppValue, pDefault ))
                return sal_True;
        }

        if (0 != *ppValue)
        {
            rtl_uString_release( *ppValue );
            *ppValue = 0;
        }
        getFromList( &s_rtl_bootstrap_set_list, ppValue, pName );
        if (! *ppValue)
        {
            getFromCommandLineArgs( ppValue, pName );
            if(!*ppValue)
            {
                getFromList( &_nameValueList, ppValue, pName );
                if( ! *ppValue )
                {
                    // fallbacks
                    if (name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("SYSUSERCONFIG") ))
                    {
                        oslSecurity security = osl_getCurrentSecurity();
                        osl_getConfigDir(security, ppValue);
                        osl_freeSecurityHandle(security);
                        further_macro_expansion = false;
                    }
                    else if (name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("SYSUSERHOME") ))
                    {
                        oslSecurity security = osl_getCurrentSecurity();
                        osl_getHomeDir(security, ppValue);
                        osl_freeSecurityHandle(security);
                        further_macro_expansion = false;
                    }
                    else if (name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("SYSBINDIR") ))
                    {
                        getExecutableDirectory(ppValue);
                        further_macro_expansion = false;
                    }
                    else
                    {
                        getFromEnvironment( ppValue, pName );
                        if( ! *ppValue )
                        {
                            result = sal_False;

                            if(pDefault)
                                rtl_uString_assign( ppValue, pDefault );
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
            OUString result = expandMacros(
                static_cast< rtlBootstrapHandle >( const_cast< Bootstrap_Impl * >( this ) ),
                * reinterpret_cast< OUString const * >( ppValue ) );
            rtl_uString_assign( ppValue, result.pData );
        }
    }

#ifdef DEBUG
    OString sName = OUStringToOString(OUString(pName), RTL_TEXTENCODING_ASCII_US);
    OString sValue = OUStringToOString(OUString(*ppValue), RTL_TEXTENCODING_ASCII_US);
    OSL_TRACE(
        __FILE__" -- Bootstrap_Impl::getValue() - name:%s value:%s\n",
        sName.getStr(), sValue.getStr() );
#endif

    return result;
}

extern "C"
{

rtlBootstrapHandle SAL_CALL rtl_bootstrap_args_open(rtl_uString * pIniName)
{
    OUString workDir;
    OUString iniName (pIniName);

    osl_getProcessWorkingDir(&workDir.pData);
    osl::FileBase::getAbsoluteFileURL(workDir, iniName, iniName);

    Bootstrap_Impl * pImpl = new Bootstrap_Impl (iniName);
    return static_cast<rtlBootstrapHandle>(pImpl);
}


void SAL_CALL rtl_bootstrap_args_close(rtlBootstrapHandle handle)
{
    delete static_cast<Bootstrap_Impl*>(handle);
}

sal_Bool SAL_CALL rtl_bootstrap_get_from_handle(
    rtlBootstrapHandle handle, rtl_uString *pName, rtl_uString **ppValue, rtl_uString *pDefault )
{
    osl::MutexGuard guard(osl::Mutex::getGlobalMutex());

    sal_Bool found = sal_False;
    if(ppValue && pName)
    {
        if(handle)
        {
            found = static_cast< Bootstrap_Impl * >( handle )->getValue(
                pName, ppValue, pDefault );
        }
        else
        {
            found = rtl_bootstrap_get(pName, ppValue, pDefault);
        }
    }

    return found;
}

void SAL_CALL rtl_bootstrap_get_iniName_from_handle(
    rtlBootstrapHandle handle, rtl_uString ** ppIniName )
{
    if(ppIniName)
    {
        if(handle)
        {
            rtl_uString_assign(ppIniName, ((Bootstrap_Impl *) handle)->_iniName.pData);
        }
        else
        {
            const OUString & iniName = getIniFileNameImpl();
            rtl_uString_assign(ppIniName, iniName.pData);
        }
    }
}

void SAL_CALL rtl_bootstrap_setIniFileName( rtl_uString *pName )
{
    osl::MutexGuard guard( osl::Mutex::getGlobalMutex() );
    OUString & file = getIniFileNameImpl();
    file = pName;
}

static rtlBootstrapHandle get_static_bootstrap_handle() SAL_THROW( () )
{
    osl::MutexGuard guard( osl::Mutex::getGlobalMutex() );

    static Bootstrap_Impl * pBootstrap_Impl = 0;

    if(!pBootstrap_Impl)
    {
        static Bootstrap_Impl g_bootstrap_Impl (getIniFileNameImpl());
        pBootstrap_Impl = &g_bootstrap_Impl;
    }

    return pBootstrap_Impl;
}

sal_Bool SAL_CALL rtl_bootstrap_get(
    rtl_uString *pName, rtl_uString **ppValue , rtl_uString *pDefault )
{
    return rtl_bootstrap_get_from_handle(
        get_static_bootstrap_handle(), pName, ppValue, pDefault);
}

void SAL_CALL rtl_bootstrap_set( rtl_uString * pName, rtl_uString * pValue )
    SAL_THROW_EXTERN_C()
{
    OUString const & name = *reinterpret_cast< OUString const * >( &pName );
    OUString const & value = *reinterpret_cast< OUString const * >( &pValue );

    ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );

    NameValueList::iterator iPos( s_rtl_bootstrap_set_list.begin() );
    NameValueList::iterator iEnd( s_rtl_bootstrap_set_list.end() );
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
#endif

    s_rtl_bootstrap_set_list.push_back( rtl_bootstrap_NameValue( name, value ) );
}

void SAL_CALL rtl_bootstrap_expandMacros_from_handle(
    rtlBootstrapHandle handle, rtl_uString ** macro )
    SAL_THROW_EXTERN_C()
{
    OUString expanded( expandMacros( handle, * reinterpret_cast< OUString const * >( macro ) ) );
    rtl_uString_assign( macro, expanded.pData );
}

void SAL_CALL rtl_bootstrap_expandMacros(
    rtl_uString ** macro )
    SAL_THROW_EXTERN_C()
{
    OUString expanded( expandMacros( get_static_bootstrap_handle(),
                                     * reinterpret_cast< OUString const * >( macro ) ) );
    rtl_uString_assign( macro, expanded.pData );
}

}
