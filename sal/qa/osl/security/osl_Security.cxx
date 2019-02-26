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

#include <algorithm>
#ifdef _WIN32
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <sddl.h>
#undef min
#endif
#include "osl_Security_Const.h"
#include <osl/thread.h>
#include <rtl/process.h>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

using namespace osl;
using namespace rtl;

/** print a UNI_CODE String.
*/
static void printUString( const OUString & str )
{
    OString aString;

    //t_print("#printUString_u# " );
    aString = OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    t_print("%s\n", aString.getStr( ) );
}

// test code start here

namespace osl_Security
{

    /** testing the method:
        Security()
    */
    class ctors : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        void ctors_001( )
        {
            ::osl::Security aSec;

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: create a security  its handle should not be NULL.",
                                    aSec.getHandle( ) != nullptr );
        }

        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class ctors

    /** testing the methods:
        inline sal_Bool SAL_CALL logonUser(const OUString& strName,
                                       const OUString& strPasswd);
        inline sal_Bool SAL_CALL logonUser(const OUString & strName,
                                       const OUString & strPasswd,
                                       const OUString & strFileServer);
    */
    class logonUser : public CppUnit::TestFixture
    {
    public:
        bool bRes;

        void logonUser_user_pwd( )
        {
            ::osl::Security aSec;
            bRes = aSec.logonUser( aLogonUser, aLogonPasswd );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: check logon user through forwarded user name, pwd, passed in (UNX), failed in (W32).",
                                    bRes );
        }

        void logonUser_user_pwd_server( )
        {
            ::osl::Security aSec;
            bRes = aSec.logonUser( aLogonUser, aLogonPasswd, aFileServer );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: check logon user through forwarded user name, pwd and server name, failed in (UNX)(W32).",
                                    bRes );
        }

        CPPUNIT_TEST_SUITE( logonUser );
        if  ( !aStringForward.isEmpty() && aStringForward.indexOf( ' ' ) != -1 && ( aStringForward.indexOf( ' ' ) ==  aStringForward.lastIndexOf( ' ' ) ) )
        /// if user name and passwd are forwarded
        {
            CPPUNIT_TEST( logonUser_user_pwd );
        }
        if  ( !aStringForward.isEmpty() && aStringForward.indexOf( ' ' ) != -1 && ( aStringForward.indexOf( ' ' ) !=  aStringForward.lastIndexOf( ' ' ) ) )
        /// if user name and passwd and file server are forwarded
        {
            CPPUNIT_TEST( logonUser_user_pwd_server );
        }
        CPPUNIT_TEST_SUITE_END( );
    }; // class logonUser

    /** testing the method:
        inline sal_Bool Security::getUserIdent( OUString& strIdent) const
    */
    class getUserIdent : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        void getUserIdent_001( )
        {
            ::osl::Security aSec;
            OUString strID;
            bRes = aSec.getUserIdent( strID );

            OStringBuffer aMessage;
            aMessage.append("strUserID: ");
            aMessage.append(OUStringToOString(strUserID, osl_getThreadTextEncoding()));
            aMessage.append(", strID: ");
            aMessage.append(OUStringToOString(strID, osl_getThreadTextEncoding()));
            aMessage.append(", bRes: ");
            aMessage.append(bRes);

            CPPUNIT_ASSERT_EQUAL_MESSAGE( aMessage.getStr(), strUserID, strID );
            CPPUNIT_ASSERT_MESSAGE( aMessage.getStr(), bRes );
        }

        CPPUNIT_TEST_SUITE( getUserIdent );
        CPPUNIT_TEST( getUserIdent_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class getUserIdent

    /** testing the method:
        inline sal_Bool SAL_CALL getUserName( OUString& strName) const;
    */
    class getUserName : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        void getUserName_001( )
        {
            ::osl::Security aSec;
#ifdef _WIN32
            OUString strName( strUserName ), strGetName;
#else
            OUString strName( strUserName ), strGetName;
#endif
            bRes = aSec.getUserName( strGetName );

            sal_Int32 nPos = -1;
            if (!strName.isEmpty())
            {
                nPos = strGetName.indexOf(strName);
            }
            CPPUNIT_ASSERT_MESSAGE( "#test comment#: get UserName and compare it with names got at the beginning of the test.",
                                    ( nPos >= 0 ) );
            CPPUNIT_ASSERT_MESSAGE( "#test comment#: get UserName and compare it with names got at the beginning of the test.",
                                    bRes );
        }

        CPPUNIT_TEST_SUITE( getUserName );
        CPPUNIT_TEST( getUserName_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class getUserName

    /** testing the method:
        inline sal_Bool Security::getConfigDir( OUString& strDirectory ) const
    */
    class getConfigDir : public CppUnit::TestFixture
    {
    public:
        bool bRes, bRes1;

        void getConfigDir_001( )
        {
            ::osl::Security aSec;
            OUString strConfig;
            bRes = aSec.getConfigDir( strConfig );

            CPPUNIT_ASSERT_MESSAGE( "failed to find a ConfigDir!", bRes );
        }

        CPPUNIT_TEST_SUITE( getConfigDir );
        CPPUNIT_TEST( getConfigDir_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class getConfigDir

    /** testing the method:
        inline sal_Bool SAL_CALL isAdministrator() const;
    */
    class isAdministrator : public CppUnit::TestFixture
    {
    public:
        bool bRes;

        void isAdministrator_001( )
        {
            ::osl::Security aSec;
            bRes = aSec.isAdministrator(  );

            CPPUNIT_ASSERT_EQUAL_MESSAGE( "#test comment#: check if the user is administrator at beginning, compare here.",
                                     isAdmin, bRes );
        }

        CPPUNIT_TEST_SUITE( isAdministrator );
        CPPUNIT_TEST( isAdministrator_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class isAdministrator

    /** testing the method:
        inline oslSecurity getHandle() const;
    */
    class getHandle : public CppUnit::TestFixture
    {
    public:
        bool bRes;

        void getHandle_001( )
        {
            ::osl::Security aSec;
            bRes = aSec.isAdministrator( ) == bool(osl_isAdministrator( aSec.getHandle( ) ));

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: use getHandle function to call C API.",
                                     bRes );
        }

        CPPUNIT_TEST_SUITE( getHandle );
        CPPUNIT_TEST( getHandle_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class getHandle

    class UserProfile : public CppUnit::TestFixture
    {
    public:

        void loadUserProfile( )
            {
                ::osl::Security aSec;
                bool bValue = osl_loadUserProfile(aSec.getHandle());

                CPPUNIT_ASSERT_MESSAGE( "empty function.", !bValue );
            }

        void unloadUserProfile( )
            {
                ::osl::Security aSec;
                osl_unloadUserProfile(aSec.getHandle());
                CPPUNIT_ASSERT_MESSAGE( "empty function.", true );
            }

        CPPUNIT_TEST_SUITE( UserProfile );
        CPPUNIT_TEST( loadUserProfile );
        CPPUNIT_TEST( unloadUserProfile );
        CPPUNIT_TEST_SUITE_END( );
    }; // class UserProfile

    class loginUserOnFileServer : public CppUnit::TestFixture
    {
    public:

        void loginUserOnFileServer_001( )
            {
                OUString suUserName;
                OUString suPassword;
                OUString suFileServer;
                ::osl::Security aSec;
                oslSecurity pSec = aSec.getHandle();

                oslSecurityError erg = osl_loginUserOnFileServer(suUserName.pData, suPassword.pData, suFileServer.pData, &pSec);

                CPPUNIT_ASSERT_EQUAL_MESSAGE( "empty function.", osl_Security_E_UserUnknown, erg );
            }

        CPPUNIT_TEST_SUITE( loginUserOnFileServer );
        CPPUNIT_TEST( loginUserOnFileServer_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class loginUserOnFileServer

CPPUNIT_TEST_SUITE_REGISTRATION(osl_Security::ctors);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Security::logonUser);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Security::getUserIdent);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Security::getUserName);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Security::getConfigDir);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Security::isAdministrator);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Security::getHandle);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Security::UserProfile);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Security::loginUserOnFileServer);

} // namespace osl_Security

/* This defines an own TestPlugIn implementation with an own initialize()
    method that will be called after loading the PlugIn
    */
#include <cppunit/plugin/TestPlugInDefaultImpl.h>

class MyTestPlugInImpl: public CPPUNIT_NS::TestPlugInDefaultImpl
{
    public:
    MyTestPlugInImpl() {};
    void initialize( CPPUNIT_NS::TestFactoryRegistry *registry,
                   const CPPUNIT_NS::PlugInParameters &parameters ) override;
};

void MyTestPlugInImpl::initialize( CPPUNIT_NS::TestFactoryRegistry *,
                   const CPPUNIT_NS::PlugInParameters & )
{
    /// start message
    t_print("#Initializing ...\n" );
    t_print("#\n#logonUser function need root/Administrator account to test.\n" );
    t_print("#You can test by login with root/Administrator, and execute:\n" );
    t_print("#testshl2 -forward \"username password\" ../../../wntmsci9/bin/Security.dll\n" );
    t_print("#      where username and password are forwarded account info.\n" );
    t_print("#if no text forwarded, this function will be skipped.\n" );

    /// get system information
#if ( defined UNX )
    /// some initialization work for UNIX OS

    struct passwd* pw;
    CPPUNIT_ASSERT_MESSAGE( "getpwuid: no password entry\n",( pw = getpwuid( getuid() ) ) != nullptr );

    /// get user ID;
    strUserID = OUString::number( getuid( ) );

    /// get user Name;
    strUserName = OUString::createFromAscii( pw->pw_name );

    /// get home directory;
    char *pw_dir = pw->pw_dir;
    if( getenv( "FAKEROOTKEY" ) )
        pw_dir = getenv("HOME");
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "#Convert from system path to URL failed.",
                            ::osl::File::E_None, ::osl::File::getFileURLFromSystemPath( OUString::createFromAscii( pw_dir ), strHomeDirectory ) );

    /// get config directory;
    strConfigDirectory = strHomeDirectory.copy(0);

    /// is administrator;
    if( !getuid( ) )
        isAdmin = true;

#endif
#if defined(_WIN32)
    /// some initialization work for Windows OS

    /// Get the user name, computer name, user home directory.
    LPWSTR lpszSystemInfo;      // pointer to system information string
    DWORD cchBuff = BUFSIZE;    // size of computer or user name
    WCHAR wchBuffer[BUFSIZE];   // buffer for string

    lpszSystemInfo = wchBuffer;
    if( GetUserNameW(lpszSystemInfo, &cchBuff) )
        strUserName = o3tl::toU(lpszSystemInfo);

    cchBuff = BUFSIZE;
    if( GetComputerNameW(lpszSystemInfo, &cchBuff) )
        strComputerName = o3tl::toU(lpszSystemInfo);

    /// Get user home directory.
    HKEY hRegKey;
    if (RegOpenKeyW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", &hRegKey) == ERROR_SUCCESS)
    {
        sal_Unicode PathW[_MAX_PATH];
        LSTATUS lRet;
        DWORD lSize = sizeof(PathW);
        DWORD Type;

        lRet = RegQueryValueExW(hRegKey, L"AppData", nullptr, &Type, reinterpret_cast<unsigned char *>(PathW), &lSize);
        if ( ( lRet == ERROR_SUCCESS ) && ( Type == REG_SZ ) &&  ( _waccess( o3tl::toW(PathW), 0 ) == 0 ) )
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "#Convert from system path to URL failed.",
                                    ::osl::File::E_None, ::osl::File::getFileURLFromSystemPath( PathW, strConfigDirectory ) );
        }

        lSize = sizeof(PathW);
        lRet = RegQueryValueExW(hRegKey, L"Personal", nullptr, &Type, reinterpret_cast<unsigned char *>(PathW), &lSize);
        if ( ( lRet == ERROR_SUCCESS ) && ( Type == REG_SZ ) &&  ( _waccess( o3tl::toW(PathW), 0 ) == 0 ) )
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "#Convert from system path to URL failed.",
                                    ::osl::File::E_None, ::osl::File::getFileURLFromSystemPath( PathW, strHomeDirectory ) );
        }

        RegCloseKey(hRegKey);
    }

    /// Get user Security ID:

    // Create buffers that may be large enough. If a buffer is too small, the count parameter will be set to the size needed.
    const DWORD INITIAL_SIZE = 32;
    DWORD cbSid = 0;
    DWORD dwSidBufferSize = INITIAL_SIZE;
    DWORD cchDomainName = 0;
    DWORD dwDomainBufferSize = INITIAL_SIZE;
    WCHAR * wszDomainName = nullptr;
    SID_NAME_USE eSidType;
    DWORD dwErrorCode = 0;

    LPCWSTR wszAccName = o3tl::toW(strUserName.getStr( ));

    // Create buffers for the SID and the domain name.
    PSID pSid = static_cast<PSID>(new BYTE[dwSidBufferSize]);
    memset( pSid, 0, dwSidBufferSize);

    wszDomainName = new WCHAR[dwDomainBufferSize];
    memset(wszDomainName, 0, dwDomainBufferSize*sizeof(WCHAR));

    // Obtain the SID for the account name passed.
    for ( ; ; )
    {
        // Set the count variables to the buffer sizes and retrieve the SID.
        cbSid = dwSidBufferSize;
        cchDomainName = dwDomainBufferSize;
        if (LookupAccountNameW(
                           nullptr,            // Computer name. NULL for the local computer
                           wszAccName,
                           pSid,          // Pointer to the SID buffer. Use NULL to get the size needed,
                           &cbSid,          // Size of the SID buffer needed.
                           wszDomainName,   // wszDomainName,
                           &cchDomainName,
                           &eSidType
                           ))
        {
            if (IsValidSid( pSid) == FALSE)
                wprintf(L"# The SID for %s is invalid.\n", wszAccName);
            break;
        }
        dwErrorCode = GetLastError();

        // Check if one of the buffers was too small.
        if (dwErrorCode == ERROR_INSUFFICIENT_BUFFER)
        {
            if (cbSid > dwSidBufferSize)
            {
                // Reallocate memory for the SID buffer.
                wprintf(L"# The SID buffer was too small. It will be reallocated.\n");
                delete[] static_cast<BYTE*>(pSid);
                pSid = static_cast<PSID>(new BYTE[cbSid]);
                memset( pSid, 0, cbSid);
                dwSidBufferSize = cbSid;
            }
            if (cchDomainName > dwDomainBufferSize)
            {
                // Reallocate memory for the domain name buffer.
                wprintf(L"# The domain name buffer was too small. It will be reallocated.\n");
                delete [] wszDomainName;
                wszDomainName = new WCHAR[cchDomainName];
                memset(wszDomainName, 0, cchDomainName*sizeof(WCHAR));
                dwDomainBufferSize = cchDomainName;
            }
        }
        else
        {
            wprintf(L"# LookupAccountNameW failed. GetLastError returned: %d\n", dwErrorCode);
            break;
        }
    }

    LPWSTR pSidStr = nullptr;
    if (ConvertSidToStringSidW(pSid, &pSidStr))
    {
        strUserID = o3tl::toU(pSidStr);
        LocalFree(pSidStr);
    }
    else
    {
        wprintf(L"# ConvertSidToStringSidW failed. GetLastError returned: %d\n", GetLastError());
    }

    delete [] static_cast<BYTE*>(pSid);
    delete [] wszDomainName;

    /// check if logged in user is administrator:

    BOOL b;
    SID_IDENTIFIER_AUTHORITY NtAuthority = { SECURITY_NT_AUTHORITY };
    PSID AdministratorsGroup;
    b = AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &AdministratorsGroup);
    if(b)
    {
        if (!CheckTokenMembership( nullptr, AdministratorsGroup, &b))
        {
             b = FALSE;
        }
        FreeSid(AdministratorsGroup);
    }

    isAdmin = b;

#endif

    /// print the information.
    t_print("#\n#Retrieved system information is below:\n");

    t_print("Computer Name:              ");
    if ( strComputerName.isEmpty())
        t_print("Not retrieved\n" );
    else
        printUString( strComputerName );

    t_print("Current User Name:          ");
    if ( strUserName.isEmpty())
        t_print("Not retrieved\n" );
    else
        printUString( strUserName );

    t_print("Current User Home Directory:");
    if ( strHomeDirectory.isEmpty())
        t_print("Not retrieved\n" );
    else
        printUString( strHomeDirectory );

    t_print("Current Config Directory:   ");
    if ( strConfigDirectory.isEmpty())
        t_print("Not retrieved\n" );
    else
        printUString( strConfigDirectory );

    t_print("Current UserID:             ");
    if ( strUserID.isEmpty())
        t_print("Not retrieved\n" );
    else
        printUString( strUserID );

    t_print("Current User is:            ");
    if ( !isAdmin )
        t_print("NOT Administrator.\n" );
    else
        t_print("Administrator.\n" );

    /// get and display forwarded text if available.
    OUString args[ 3 ];
    int argsCount = 0;
    sal_uInt32 n = rtl_getAppCommandArgCount();
    for (sal_uInt32 i = 0; i < n; ++i)
    {
        OUString arg;
        rtl_getAppCommandArg(i, &arg.pData);
        if( arg.startsWith("-") )
            continue;
        if( argsCount >= 3 )
        {
            SAL_WARN( "sal.osl", "Too many test arguments" );
            continue;
        }
        args[ argsCount++ ] = arg;
    }
    /// only forwarded two parameters, username and password.
    if( argsCount == 2 )
    {
        aLogonUser = args[ 0 ];
        t_print("\n#Forwarded username: ");
        printUString( aLogonUser);

        aLogonPasswd = args[ 1 ];
        t_print("#Forwarded password: ");
        for (int i = 0; i < aLogonPasswd.getLength(); ++i)
            t_print("*");
        t_print("\n" );
    }
    else if( argsCount == 3 )
    /// forwarded three parameters, username, password and fileserver.
    {
        aLogonUser = args[ 0 ];
        t_print("#Forwarded username: ");
        printUString( aLogonUser);

        aLogonPasswd = args[ 1 ];
        t_print("#Forwarded password: ");
        for (int i = 0; i < aLogonPasswd.getLength(); ++i)
            t_print("*");
        t_print("\n" );

        aFileServer = args[ 2 ];
        t_print("#Forwarded FileServer: ");
        printUString( aFileServer );
    }
    t_print("#\n#Initialization Done.\n" );

}

/* Instantiate and register the own TestPlugIn and instantiate the default
    main() function.
    (This is done by CPPUNIT_PLUGIN_IMPLEMENT() for TestPlugInDefaultImpl)
    */

CPPUNIT_PLUGIN_EXPORTED_FUNCTION_IMPL( MyTestPlugInImpl );
CPPUNIT_PLUGIN_IMPLEMENT_MAIN();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
