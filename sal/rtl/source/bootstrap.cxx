/*************************************************************************
 *
 *  $RCSfile: bootstrap.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kr $ $Date: 2001-07-25 08:12:51 $
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
#include <list>
#include <stdio.h>

#include <osl/process.h>
#include <osl/file.h>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>

#include <rtl/bootstrap.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/byteseq.hxx>

#include "macro.hxx"

// I need C++ for static variables and for lists (stl) !
// If we don't want C++, we need another solution for static vars !
using namespace ::rtl;
using namespace ::osl;

struct rtl_bootstrap_NameValue
{
    ::rtl::OUString sName;
    ::rtl::OUString sValue;
};

typedef ::std::list< struct rtl_bootstrap_NameValue > NameValueList;

static void getFromCommandLineArgs( rtl_uString **ppValue , rtl_uString *pName )
{
    static NameValueList *pNameValueList = 0;
    if( ! pNameValueList )
    {
        static NameValueList nameValueList;
        sal_Int32 i = 0;
        sal_Int32 nArgCount = osl_getCommandArgCount();
        for(  ; i < nArgCount && ! *ppValue ; i ++ )
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

                    struct rtl_bootstrap_NameValue nameValue;
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

    OUString name( pName );
    for( NameValueList::iterator ii = pNameValueList->begin() ;
         ii != pNameValueList->end() ;
         ++ii )
    {
        if( (*ii).sName.equals(name) )
        {
            rtl_uString_assign( ppValue, (*ii).sValue.pData );
            break;
        }
    }
}

static ::rtl::OUString &getIniFileNameImpl()
{
    static OUString *pStaticName = 0;
    if( ! pStaticName )
    {
        rtl_uString *pFileName = 0;
        ::rtl::OUString sVarName( RTL_CONSTASCII_USTRINGPARAM( "INIFILENAME"));
        getFromCommandLineArgs( &pFileName , sVarName.pData );
        if( !pFileName )
        {
            rtl_uString *pExeName = 0;
            osl_getExecutableFile( &pExeName );

            sal_Int32 n = pExeName->length;

            if( n )
            {
                sal_Int32 nCapacity =
                    rtl_uStringbuffer_newFromStringBuffer(
                        &pFileName, pExeName->length + 4, pExeName );

                const sal_Char *pNewSuffix = SAL_CONFIGFILE();
                OUString sOldSuffix( RTL_CONSTASCII_USTRINGPARAM( ".exe" ) );

                if(pFileName->length > sOldSuffix.getLength()
                && 0 == rtl_ustr_compareIgnoreAsciiCase_WithLength(pFileName->buffer + n - sOldSuffix.getLength(),
                                                                   sOldSuffix.getLength(),
                                                                   sOldSuffix.pData->buffer,
                                                                   sOldSuffix.getLength()))
                    pFileName->length = pFileName->length - sOldSuffix.getLength();

                sOldSuffix = OUString(RTL_CONSTASCII_USTRINGPARAM( ".bin" ) );
                if(pFileName->length > sOldSuffix.getLength()
                && 0 == rtl_ustr_compareIgnoreAsciiCase_WithLength(pFileName->buffer + n - sOldSuffix.getLength(),
                                                                   sOldSuffix.getLength(),
                                                                   sOldSuffix.pData->buffer,
                                                                   sOldSuffix.getLength()))
                    pFileName->length = pFileName->length - sOldSuffix.getLength();


                rtl_uStringbuffer_insert_ascii(
                    &pFileName,
                    &nCapacity,
                    pFileName->length,
                    pNewSuffix,
                    strlen(pNewSuffix) );
            }
            rtl_uString_release( pExeName );
        }

        static OUString theFileName;
        if( pFileName )
            theFileName = pFileName;

        rtl_uString_release( pFileName );
        pStaticName = &theFileName;
    }
    return *pStaticName;
}

static void getFileSize( oslFileHandle handle, sal_uInt64 *pSize )
{
    sal_uInt64 nOldPos=0;
    OSL_VERIFY( osl_File_E_None == osl_getFilePos( handle, &nOldPos ) &&
                osl_File_E_None == osl_setFilePos( handle, osl_Pos_End , 0 ) &&
                osl_File_E_None == osl_getFilePos( handle, pSize ) &&
                osl_File_E_None == osl_setFilePos( handle, osl_Pos_Absolut, nOldPos ) );
}

static void getFromIniFile( rtl_uString **ppValue, rtl_uString *pName )
{
    static NameValueList *pNameValueList = 0;

    if( !pNameValueList )
    {
        static NameValueList nameValueList;
        OUString &file = getIniFileNameImpl();
        oslFileHandle handle;
        if( file.getLength() &&
            osl_File_E_None == osl_openFile( file.pData, &handle , osl_File_OpenFlag_Read ) )
        {
            ByteSequence seq;
            sal_uInt64 nSize = 0;

            getFileSize( handle, &nSize );
            while( sal_True )
            {
                sal_uInt64 nPos;
                if( osl_File_E_None != osl_getFilePos( handle, &nPos ) ||
                    nPos >= nSize )
                {
                    break;
                }
                if(osl_File_E_None != osl_readLine( handle , (sal_Sequence ** ) &seq ))
                {
                    break;
                }
                OString line( (const sal_Char * ) seq.getConstArray() , seq.getLength() );
                sal_Int32 nIndex = line.indexOf( '=' );
                struct rtl_bootstrap_NameValue nameValue;
                if( nIndex >= 1 && nIndex +1 < line.getLength())
                {
                    nameValue.sName =
                        OStringToOUString(
                            line.copy(0,nIndex).trim(), RTL_TEXTENCODING_ASCII_US );
                    nameValue.sValue =
                        OStringToOUString(
                            line.copy(nIndex+1).trim(), RTL_TEXTENCODING_UTF8 );
                    nameValueList.push_back( nameValue );
                }
            }
            osl_closeFile( handle );
        }
        pNameValueList = &nameValueList;
    }

    OUString name( pName );
    for( NameValueList::iterator ii = pNameValueList->begin() ;
         ii != pNameValueList->end() ;
         ++ii )
    {
        if( (*ii).sName.equals(name) )
        {
            rtl_uString_assign( ppValue, (*ii).sValue.pData );
            break;
        }
    }
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


static void getFromDefault( rtl_uString **ppValue, rtl_uString *pName, rtl_uString *pDefault )
{
    static NameValueList *pNameValueList = 0;

    if( !pNameValueList )
    {
        static NameValueList nameValueList;
        pNameValueList = &nameValueList;
    }

    OUString name( pName );
    for( NameValueList::iterator ii = pNameValueList->begin() ;
         ii != pNameValueList->end() ;
         ++ii )
    {
        if( (*ii).sName.equals(name) )
        {
            rtl_uString_assign( ppValue, (*ii).sValue.pData );
            break;
        }
    }
    if( ! *ppValue )
    {
        rtl_uString_assign( ppValue , pDefault );
    }
}

static void getFlatArg(rtl_uString * pName, rtl_uString ** ppValue, rtl_uString * pDefault)
{
    static const OUString sysUserConfig(RTL_CONSTASCII_USTRINGPARAM("SYSUSERCONFIG"));
    static const OUString sysUserHome(RTL_CONSTASCII_USTRINGPARAM("SYSUSERHOME"));

    // we have build ins:
    if(!rtl_ustr_compare_WithLength(pName->buffer, pName->length, sysUserConfig.pData->buffer, sysUserConfig.pData->length))
    {
        oslSecurity security = osl_getCurrentSecurity();
        osl_getConfigDir(security, ppValue);
        osl_freeSecurityHandle(security);
    }
    else if(!rtl_ustr_compare_WithLength(pName->buffer, pName->length, sysUserHome.pData->buffer, sysUserHome.pData->length))
    {
        oslSecurity security = osl_getCurrentSecurity();
        osl_getHomeDir(security, ppValue);
        osl_freeSecurityHandle(security);
    }
    else
    {
        getFromCommandLineArgs( ppValue, pName );
        if( ! *ppValue )
        {
            getFromIniFile( ppValue, pName );
            if( ! *ppValue )
            {
                getFromEnvironment( ppValue, pName );
                if( ! *ppValue && pDefault )
                {
                    rtl_uString_assign( ppValue , pDefault );
                }
            }
        }
    }
}


extern "C"
{
void SAL_CALL rtl_bootstrap_setIniFileName( rtl_uString *pName )
{
    MutexGuard guard( Mutex::getGlobalMutex() );
    OUString & file = getIniFileNameImpl();
    file = pName;
}

sal_Bool SAL_CALL rtl_bootstrap_get( rtl_uString *pName, rtl_uString **ppValue , rtl_uString *pDefault )
{
    MutexGuard guard( Mutex::getGlobalMutex() );
    OSL_ASSERT( ppValue );
    if( *ppValue )
    {
        rtl_uString_release( *ppValue );
        *ppValue = 0;
    }

    getFlatArg(pName, ppValue, pDefault);

    if(*ppValue) {
         OUString result = expandMacros(OUString(*ppValue));

        rtl_uString_assign(ppValue, result.pData );
    }

    if( ! *ppValue )
    {
        rtl_uString_new( ppValue );
        return sal_False;
    }
    return sal_True;
}

}
