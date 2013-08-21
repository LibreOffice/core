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

//------------------------------------------------------------------------
// header file
//------------------------------------------------------------------------
#ifdef WNT
#include <windows.h>
#undef min
#endif
#include <osl_Security_Const.h>
#include <osl/thread.h>
#include <rtl/process.h>
#include <rtl/strbuf.hxx>

using namespace osl;
using namespace rtl;


//------------------------------------------------------------------------
// helper functions and classes
//------------------------------------------------------------------------

/** print Boolean value.
*/
inline void printBool( sal_Bool bOk )
{
    //t_print("#printBool# " );
    ( sal_True == bOk ) ? t_print("TRUE!\n" ): t_print("FALSE!\n" );
}

/** print a UNI_CODE String.
*/
inline void printUString( const ::rtl::OUString & str )
{
    rtl::OString aString;

    //t_print("#printUString_u# " );
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    t_print("%s\n", aString.getStr( ) );
}


//------------------------------------------------------------------------
// test code start here
//------------------------------------------------------------------------

namespace osl_Security
{

    /** testing the method:
        Security()
    */
    class ctors : public CppUnit::TestFixture
    {
    public:
        sal_Bool bRes, bRes1;

        void ctors_001( )
        {
            ::osl::Security aSec;

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: create a security  its handle should not be NULL.",
                                    aSec.getHandle( ) != NULL );
        }

        CPPUNIT_TEST_SUITE( ctors );
        CPPUNIT_TEST( ctors_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class ctors


    /** testing the methods:
        inline sal_Bool SAL_CALL logonUser(const ::rtl::OUString& strName,
                                       const ::rtl::OUString& strPasswd);
        inline sal_Bool SAL_CALL logonUser(const ::rtl::OUString & strName,
                                       const ::rtl::OUString & strPasswd,
                                       const ::rtl::OUString & strFileServer);
    */
    class logonUser : public CppUnit::TestFixture
    {
    public:
        sal_Bool bRes;

        void logonUser_user_pwd( )
        {
            ::osl::Security aSec;
            bRes = aSec.logonUser( aLogonUser, aLogonPasswd );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: check logon user through forwarded user name, pwd, passed in (UNX), failed in (W32).",
                                    ( sal_True == bRes )  );
        }

        void logonUser_user_pwd_server( )
        {
            ::osl::Security aSec;
            bRes = aSec.logonUser( aLogonUser, aLogonPasswd, aFileServer );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: check logon user through forwarded user name, pwd and server name, failed in (UNX)(W32).",
                                    ( sal_True == bRes )  );
        }


        CPPUNIT_TEST_SUITE( logonUser );
        if  ( !aStringForward.isEmpty() && aStringForward.indexOf( (sal_Unicode)' ' ) != -1 && ( aStringForward.indexOf( ( sal_Unicode ) ' ' ) ==  aStringForward.lastIndexOf( ( sal_Unicode ) ' ' ) ) )
        /// if user name and passwd are forwarded
        {
            CPPUNIT_TEST( logonUser_user_pwd );
        }
        if  ( !aStringForward.isEmpty() && aStringForward.indexOf( (sal_Unicode)' ' ) != -1 && ( aStringForward.indexOf( ( sal_Unicode ) ' ' ) !=  aStringForward.lastIndexOf( ( sal_Unicode ) ' ' ) ) )
        /// if user name and passwd and file server are forwarded
        {
            CPPUNIT_TEST( logonUser_user_pwd_server );
        }
        CPPUNIT_TEST_SUITE_END( );
    }; // class logonUser


    /** testing the method:
        inline sal_Bool Security::getUserIdent( rtl::OUString& strIdent) const
    */
    class getUserIdent : public CppUnit::TestFixture
    {
    public:
        sal_Bool bRes, bRes1;

        void getUserIdent_001( )
        {
            ::osl::Security aSec;
            ::rtl::OUString strID;
            bRes = aSec.getUserIdent( strID );

            rtl::OStringBuffer aMessage;
            aMessage.append("strUserID: ");
            aMessage.append(rtl::OUStringToOString(strUserID, osl_getThreadTextEncoding()));
            aMessage.append(", strID: ");
            aMessage.append(rtl::OUStringToOString(strID, osl_getThreadTextEncoding()));
            aMessage.append(", bRes: ");
            aMessage.append(bRes);

            CPPUNIT_ASSERT_MESSAGE( aMessage.getStr(), strUserID.equals(strID) && (bRes == sal_True));
        }

        CPPUNIT_TEST_SUITE( getUserIdent );
        CPPUNIT_TEST( getUserIdent_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class getUserIdent


    /** testing the method:
        inline sal_Bool SAL_CALL getUserName( ::rtl::OUString& strName) const;
    */
    class getUserName : public CppUnit::TestFixture
    {
    public:
        sal_Bool bRes, bRes1;

        void getUserName_001( )
        {
            ::osl::Security aSec;
#ifdef WNT
            ::rtl::OUString strName( strUserName ), strGetName;
#else
            ::rtl::OUString strName( strUserName ), strGetName;
#endif
            bRes = aSec.getUserName( strGetName );

            sal_Int32 nPos = -1;
            if (!strName.isEmpty())
            {
                nPos = strGetName.indexOf(strName);
            }
            CPPUNIT_ASSERT_MESSAGE( "#test comment#: get UserName and compare it with names got at the beginning of the test.",
                                    ( nPos >= 0 ) && ( sal_True == bRes ) );
        }

        CPPUNIT_TEST_SUITE( getUserName );
        CPPUNIT_TEST( getUserName_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class getUserName


    /** testing the method:
        inline sal_Bool Security::getConfigDir( rtl::OUString& strDirectory ) const
    */
    class getConfigDir : public CppUnit::TestFixture
    {
    public:
        sal_Bool bRes, bRes1;

        void getConfigDir_001( )
        {
            ::osl::Security aSec;
            ::rtl::OUString strConfig;
            bRes = aSec.getConfigDir( strConfig );

            CPPUNIT_ASSERT_MESSAGE( "failed to find a ConfigDir!",
                                     ( sal_True == bRes ));
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
        sal_Bool bRes;

        void isAdministrator_001( )
        {
            ::osl::Security aSec;
            bRes = aSec.isAdministrator(  );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: check if the user is administrator at beginning, compare here.",
                                     bRes == isAdmin );
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
        sal_Bool bRes;

        void getHandle_001( )
        {
            ::osl::Security aSec;
            bRes = aSec.isAdministrator( ) == osl_isAdministrator( aSec.getHandle( ) );

            CPPUNIT_ASSERT_MESSAGE( "#test comment#: use getHandle function to call C API.",
                                     bRes == sal_True );
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
                sal_Bool bValue = osl_loadUserProfile(aSec.getHandle());

                CPPUNIT_ASSERT_MESSAGE( "empty function.", bValue == sal_False );
            }

        void unloadUserProfile( )
            {
                ::osl::Security aSec;
                osl_unloadUserProfile(aSec.getHandle());
                CPPUNIT_ASSERT_MESSAGE( "empty function.", sal_True );
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
                rtl::OUString suUserName;
                rtl::OUString suPassword;
                rtl::OUString suFileServer;
                ::osl::Security aSec;
                oslSecurity pSec = aSec.getHandle();

                oslSecurityError erg = osl_loginUserOnFileServer(suUserName.pData, suPassword.pData, suFileServer.pData, &pSec);

                CPPUNIT_ASSERT_MESSAGE( "empty function.", erg == osl_Security_E_UserUnknown );
            }

        CPPUNIT_TEST_SUITE( loginUserOnFileServer );
        CPPUNIT_TEST( loginUserOnFileServer_001 );
        CPPUNIT_TEST_SUITE_END( );
    }; // class loginUserOnFileServer

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Security::ctors);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Security::logonUser);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Security::getUserIdent);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Security::getUserName);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Security::getConfigDir);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Security::isAdministrator);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Security::getHandle);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Security::UserProfile);
CPPUNIT_TEST_SUITE_REGISTRATION(osl_Security::loginUserOnFileServer);

// -----------------------------------------------------------------------------

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
                   const CPPUNIT_NS::PlugInParameters &parameters );
};


void MyTestPlugInImpl::initialize( CPPUNIT_NS::TestFactoryRegistry *,
                   const CPPUNIT_NS::PlugInParameters & )
{
    /// start message
    t_print("#Initializing ...\n" );
    t_print("#\n#logonUser function need root/Administrator account to test.\n" );
    t_print("#You can test by login with root/Administrator, and excute:\n" );
    t_print("#testshl2 -forward \"username password\" ../../../wntmsci9/bin/Security.dll\n" );
    t_print("#      where username and password are forwarded account info.\n" );
    t_print("#if no text forwarded, this function will be skipped.\n" );

    /// get system information
#if ( defined UNX )
    /// some initialization work for UNIX OS


    struct passwd* pw;
    CPPUNIT_ASSERT_MESSAGE( "getpwuid: no password entry\n",( pw = getpwuid( getuid() ) ) != NULL );

    /// get user ID;
    strUserID = OUString::number( getuid( ) );

    /// get user Name;
    strUserName = ::rtl::OUString::createFromAscii( pw->pw_name );

    /// get home directory;
    char *pw_dir = pw->pw_dir;
    if( getenv( "FAKEROOTKEY" ) )
        pw_dir = getenv("HOME");
    CPPUNIT_ASSERT_MESSAGE( "#Convert from system path to URL failed.",
                            ::osl::File::E_None == ::osl::File::getFileURLFromSystemPath( ::rtl::OUString::createFromAscii( pw_dir ), strHomeDirectory ) );

    /// get config directory;
    strConfigDirectory = strHomeDirectory.copy(0);

    /// is administrator;
    if( !getuid( ) )
        isAdmin = sal_True;

#endif
#if defined ( WNT )
    /// some initialization work for Windows OS


    /// Get the user name, computer name, user home directory.
    LPTSTR lpszSystemInfo;      // pointer to system information string
    DWORD cchBuff = BUFSIZE;    // size of computer or user name
    TCHAR tchBuffer[BUFSIZE];   // buffer for string

    lpszSystemInfo = tchBuffer;
    cchBuff = BUFSIZE;
    if( GetUserNameA(lpszSystemInfo, &cchBuff) )
        strUserName = ::rtl::OUString::createFromAscii( lpszSystemInfo );

    if( GetComputerName(lpszSystemInfo, &cchBuff) )
        strComputerName = ::rtl::OUString::createFromAscii( lpszSystemInfo );

    /// Get user home directory.
    HKEY hRegKey;
    sal_Char PathA[_MAX_PATH];
    ::rtl::OUString strHome;
    if (RegOpenKey(HKEY_CURRENT_USER,  "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",  &hRegKey) == ERROR_SUCCESS)
    {
        LONG lRet, lSize = sizeof(PathA);
        DWORD Type;

        lRet = RegQueryValueEx(hRegKey, "AppData", NULL, &Type, ( unsigned char * )PathA, ( unsigned long * )&lSize);
        if ( ( lRet == ERROR_SUCCESS ) && ( Type == REG_SZ ) &&  ( _access( PathA, 0 ) == 0 ) )
        {
            CPPUNIT_ASSERT_MESSAGE( "#Convert from system path to URL failed.",
                                    ::osl::File::E_None == ::osl::File::getFileURLFromSystemPath( ::rtl::OUString::createFromAscii( PathA ), strConfigDirectory ) );
        }

        lRet = RegQueryValueEx(hRegKey, "Personal", NULL, &Type, ( unsigned char * )PathA, ( unsigned long * )&lSize);
        if ( ( lRet == ERROR_SUCCESS ) && ( Type == REG_SZ ) &&  ( _access( PathA, 0 ) == 0 ) )
        {
            CPPUNIT_ASSERT_MESSAGE( "#Convert from system path to URL failed.",
                                    ::osl::File::E_None == ::osl::File::getFileURLFromSystemPath( ::rtl::OUString::createFromAscii( PathA ), strHomeDirectory ) );
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
    WCHAR * wszDomainName = NULL;
    SID_NAME_USE eSidType;
    DWORD dwErrorCode = 0;

    LPCWSTR wszAccName = ( LPWSTR ) strUserName.getStr( );

    // Create buffers for the SID and the domain name.
    PSID pSid = (PSID) new BYTE[dwSidBufferSize];
    CPPUNIT_ASSERT_MESSAGE("# creating SID buffer failed.\n", pSid!= NULL );
    memset( pSid, 0, dwSidBufferSize);

    wszDomainName = new WCHAR[dwDomainBufferSize];
    CPPUNIT_ASSERT_MESSAGE("# creating Domain name buffer failed.\n", wszDomainName != NULL );
    memset(wszDomainName, 0, dwDomainBufferSize*sizeof(WCHAR));

    // Obtain the SID for the account name passed.
    for ( ; ; )
    {
        // Set the count variables to the buffer sizes and retrieve the SID.
        cbSid = dwSidBufferSize;
        cchDomainName = dwDomainBufferSize;
        if (LookupAccountNameW(
                           NULL,            // Computer name. NULL for the local computer
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
                FreeSid( pSid);
                pSid = (PSID) new BYTE[cbSid];
                CPPUNIT_ASSERT_MESSAGE("# re-creating SID buffer failed.\n",  pSid!= NULL );
                memset( pSid, 0, cbSid);
                dwSidBufferSize = cbSid;
            }
            if (cchDomainName > dwDomainBufferSize)
            {
                // Reallocate memory for the domain name buffer.
                wprintf(L"# The domain name buffer was too small. It will be reallocated.\n");
                delete [] wszDomainName;
                wszDomainName = new WCHAR[cchDomainName];
                CPPUNIT_ASSERT_MESSAGE("# re-creating domain name buffer failed.\n", wszDomainName!= NULL );
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

    // now got SID successfully, only need to compare SID, so I copied the rest lines from source to convert SID to OUString.
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwSidRev=SID_REVISION;
    DWORD dwCounter;
    DWORD dwSidSize;
    sal_Char    *Ident;

    /* obtain SidIdentifierAuthority */
    psia=GetSidIdentifierAuthority(pSid);

    /* obtain sidsubauthority count */
    dwSubAuthorities=std::min((int) *GetSidSubAuthorityCount(pSid), 5);

    /* buffer length: S-SID_REVISION- + identifierauthority- + subauthorities- + NULL */
    Ident=(sal_Char * )malloc(88*sizeof(sal_Char));

    /* prepare S-SID_REVISION- */
    dwSidSize=wsprintf(Ident, TEXT("S-%lu-"), dwSidRev);

    /* prepare SidIdentifierAuthority */
    if ((psia->Value[0] != 0) || (psia->Value[1] != 0))
    {
        dwSidSize+=wsprintf(Ident + strlen(Ident),
                    TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                    (sal_uInt16)psia->Value[0],
                    (sal_uInt16)psia->Value[1],
                    (sal_uInt16)psia->Value[2],
                    (sal_uInt16)psia->Value[3],
                    (sal_uInt16)psia->Value[4],
                    (sal_uInt16)psia->Value[5]);
    }
    else
    {
        dwSidSize+=wsprintf(Ident + strlen(Ident),
                    TEXT("%lu"),
                    (sal_uInt32)(psia->Value[5]      )   +
                    (sal_uInt32)(psia->Value[4] <<  8)   +
                    (sal_uInt32)(psia->Value[3] << 16)   +
                    (sal_uInt32)(psia->Value[2] << 24)   );
    }

    /* loop through SidSubAuthorities */
    for (dwCounter=0; dwCounter < dwSubAuthorities; dwCounter++)
    {
        dwSidSize+=wsprintf(Ident + dwSidSize, TEXT("-%lu"),
                    *GetSidSubAuthority(pSid, dwCounter) );
    }

    strUserID = ::rtl::OUString::createFromAscii( Ident );

    free(Ident);
    delete [] reinterpret_cast<BYTE*>(pSid);
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
        if (!CheckTokenMembership( NULL, AdministratorsGroup, &b))
        {
             b = FALSE;
        }
        FreeSid(AdministratorsGroup);
    }

    isAdmin = ( sal_Bool )b;

#endif

    /// print the information.
    t_print("#\n#Retrived system information is below:\n");

    t_print("Computer Name:              ");
    if ( strComputerName.isEmpty())
        t_print("Not retrived\n" );
    else
        printUString( strComputerName );

    t_print("Current User Name:          ");
    if ( strUserName.isEmpty())
        t_print("Not retrived\n" );
    else
        printUString( strUserName );

    t_print("Current User Home Directory:");
    if ( strHomeDirectory.isEmpty())
        t_print("Not retrived\n" );
    else
        printUString( strHomeDirectory );

    t_print("Current Config Directory:   ");
    if ( strConfigDirectory.isEmpty())
        t_print("Not retrived\n" );
    else
        printUString( strConfigDirectory );

    t_print("Current UserID:             ");
    if ( strUserID.isEmpty())
        t_print("Not retrived\n" );
    else
        printUString( strUserID );

    t_print("Current User is:            ");
    if ( isAdmin == sal_False )
        t_print("NOT Administrator.\n" );
    else
        t_print("Administrator.\n" );

    /// get and display forwarded text if available.
    rtl::OUString args[ 3 ];
    int argsCount = 0;
    sal_uInt32 n = rtl_getAppCommandArgCount();
    for (sal_uInt32 i = 0; i < n; ++i)
    {
        rtl::OUString arg;
        rtl_getAppCommandArg(i, &arg.pData);
        if( !arg.isEmpty() && arg[ 0 ] == '-' )
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
