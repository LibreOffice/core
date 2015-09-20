/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

//------------------------------------------------------------------------
// header file
//------------------------------------------------------------------------
#include <osl_Security_Const.h>

using namespace osl;
using namespace rtl;


//------------------------------------------------------------------------
// helper functions and classes
//------------------------------------------------------------------------

/** print Boolean value.
*/
inline void printBool( sal_Bool bOk )
{
    //printf("#printBool# " );
    ( sal_True == bOk ) ? printf("TRUE!\n" ): printf("FALSE!\n" );
}

/** print a UNI_CODE String.
*/
inline void printUString( const ::rtl::OUString & str )
{
    rtl::OString aString;

    //printf("#printUString_u# " );
    aString = ::rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US );
    printf("%s\n", aString.getStr( ) );
}


//------------------------------------------------------------------------
// test code start here
//------------------------------------------------------------------------

namespace osl_Security
{

    /** testing the method:
        Security()
    */
    class ctors : public ::testing::Test
    {
    public:
        sal_Bool bRes, bRes1;

    }; // class ctors

    TEST_F(ctors, ctors_001)
    {
        ::osl::Security aSec;

        ASSERT_TRUE(aSec.getHandle( ) != NULL) << "#test comment#: create a security  its handle should not be NULL.";
    }

    /** testing the methods:
        inline sal_Bool SAL_CALL logonUser(const ::rtl::OUString& strName,
                                       const ::rtl::OUString& strPasswd);
        inline sal_Bool SAL_CALL logonUser(const ::rtl::OUString & strName,
                                       const ::rtl::OUString & strPasswd,
                                       const ::rtl::OUString & strFileServer);
    */
    class logonUser : public ::testing::Test
    {
    public:
        sal_Bool bRes;

        void logonUser_user_pwd( )
        {
            ::osl::Security aSec;
            bRes = aSec.logonUser( aLogonUser, aLogonPasswd );

            ASSERT_TRUE(( sal_True == bRes )) << "#test comment#: check logon user through forwarded user name, pwd, passed in (UNX), failed in (W32).";
        }

        void logonUser_user_pwd_server( )
        {
            ::osl::Security aSec;
            bRes = aSec.logonUser( aLogonUser, aLogonPasswd, aFileServer );

            ASSERT_TRUE(( sal_True == bRes )) << "#test comment#: check logon user through forwarded user name, pwd and server name, failed in (UNX)(W32).";
        }
    }; // class logonUser

    TEST_F(logonUser, logonUser_001)
    {
        if  ( !aStringForward.equals( aNullURL )  && aStringForward.indexOf( (sal_Unicode)' ' ) != -1 && ( aStringForward.indexOf( ( sal_Unicode ) ' ' ) ==  aStringForward.lastIndexOf( ( sal_Unicode ) ' ' ) ) )
        /// if user name and passwd are forwarded
        {
            logonUser_user_pwd();
        }
    }

    TEST_F(logonUser, logonUser_002)
    {
        if  ( !aStringForward.equals( aNullURL )  && aStringForward.indexOf( (sal_Unicode)' ' ) != -1 && ( aStringForward.indexOf( ( sal_Unicode ) ' ' ) !=  aStringForward.lastIndexOf( ( sal_Unicode ) ' ' ) ) )
        /// if user name and passwd and file server are forwarded
        {
            logonUser_user_pwd_server();
        }
    }

    /** testing the method:
        inline sal_Bool Security::getUserIdent( rtl::OUString& strIdent) const
    */
    class getUserIdent : public ::testing::Test
    {
    public:
        sal_Bool bRes, bRes1;
    }; // class getUserIdent

    TEST_F(getUserIdent, getUserIdent_001)
    {
        ::osl::Security aSec;
        ::rtl::OUString strID;
        bRes = aSec.getUserIdent( strID );

        ASSERT_TRUE(( sal_True == strUserID.equals( strID ) ) && ( sal_True == bRes )) << "#test comment#: get UserID and compare it with names got at the beginning of the test.";
    }


    /** testing the method:
        inline sal_Bool SAL_CALL getUserName( ::rtl::OUString& strName) const;
    */
    class getUserName : public ::testing::Test
    {
    public:
        sal_Bool bRes, bRes1;
    }; // class getUserName

    TEST_F(getUserName, getUserName_001)
    {
        ::osl::Security aSec;
#ifdef WNT
        ::rtl::OUString strName( strUserName ), strGetName;
#else
        ::rtl::OUString strName( strUserName ), strGetName;
#endif
        bRes = aSec.getUserName( strGetName );

        sal_Int32 nPos = -1;
        if (strName.getLength() > 0)
        {
            nPos = strGetName.indexOf(strName);
        }
        ASSERT_TRUE(( nPos >= 0 ) && ( sal_True == bRes )) << "#test comment#: get UserName and compare it with names got at the beginning of the test.";
    }

    /** testing the method:
        inline sal_Bool SAL_CALL getHomeDir( ::rtl::OUString& strDirectory) const;
    */
    class getHomeDir : public ::testing::Test
    {
    public:
        sal_Bool bRes, bRes1;
    }; // class getHomeDir

    TEST_F(getHomeDir, getHomeDir_001)
    {
        ::osl::Security aSec;
        ::rtl::OUString strHome;
        bRes = aSec.getHomeDir( strHome );

        ASSERT_TRUE(( sal_True == strHomeDirectory.equals( strHome ) ) && ( sal_True == bRes )) << "#test comment#: getHomeDir and compare it with the info we get at the beginning.";
    }

    /** testing the method:
        inline sal_Bool Security::getConfigDir( rtl::OUString& strDirectory ) const
    */
    class getConfigDir : public ::testing::Test
    {
    public:
        sal_Bool bRes, bRes1;
    }; // class getConfigDir

    TEST_F(getConfigDir, getConfigDir_001)
    {
        ::osl::Security aSec;
        ::rtl::OUString strConfig;
        bRes = aSec.getConfigDir( strConfig );

        ASSERT_TRUE(( sal_True == strConfigDirectory.equals( strConfig ) ) && ( sal_True == bRes )) << "#test comment#: getHomeDir and compare it with the info we get at the beginning.";
    }

    /** testing the method:
        inline sal_Bool SAL_CALL isAdministrator() const;
    */
    class isAdministrator : public ::testing::Test
    {
    public:
        sal_Bool bRes;
    }; // class isAdministrator

    TEST_F(isAdministrator, isAdministrator_001)
    {
        ::osl::Security aSec;
        bRes = aSec.isAdministrator(  );

        ASSERT_TRUE(bRes == isAdmin) << "#test comment#: check if the user is administrator at beginning, compare here.";
    }

    /** testing the method:
        inline oslSecurity getHandle() const;
    */
    class getHandle : public ::testing::Test
    {
    public:
        sal_Bool bRes;
    }; // class getHandle

    TEST_F(getHandle, getHandle_001)
    {
        ::osl::Security aSec;
        bRes = aSec.isAdministrator( ) == osl_isAdministrator( aSec.getHandle( ) );

        ASSERT_TRUE(bRes == sal_True) << "#test comment#: use getHandle function to call C API.";
    }

    class UserProfile : public ::testing::Test
    {
    public:
    }; // class UserProfile

    TEST_F(UserProfile, loadUserProfile)
    {
        ::osl::Security aSec;
        sal_Bool bValue = osl_loadUserProfile(aSec.getHandle());

        ASSERT_TRUE(bValue == sal_False) << "empty function.";
    }

    TEST_F(UserProfile, unloadUserProfile)
    {
        ::osl::Security aSec;
        osl_unloadUserProfile(aSec.getHandle());
        ASSERT_TRUE(sal_True) << "empty function.";
    }

    class loginUserOnFileServer : public ::testing::Test
    {
    public:
    }; // class loginUserOnFileServer

    TEST_F(loginUserOnFileServer, loginUserOnFileServer_001)
    {
        rtl::OUString suUserName;
        rtl::OUString suPassword;
        rtl::OUString suFileServer;
        ::osl::Security aSec;
        oslSecurity pSec = aSec.getHandle();

        oslSecurityError erg = osl_loginUserOnFileServer(suUserName.pData, suPassword.pData, suFileServer.pData, &pSec);

        ASSERT_TRUE(erg == osl_Security_E_UserUnknown) << "empty function.";
    }

} // namespace osl_Security


// -----------------------------------------------------------------------------

/** to do some initialized work, we replace the NOADDITIONAL macro with the initialize work which
      get current user name, .
*/

int main(int argc, char **argv)
{
    /// start message
    printf("#Initializing ...\n" );
    printf("#\n#logonUser function need root/Administrator account to test.\n" );
    printf("#You can test by login with root/Administrator, and excute:\n" );
    printf("#testshl2 -forward \"username password\" ../../../wntmsci9/bin/Security.dll\n" );
    printf("#      where username and password are forwarded account info.\n" );
    printf("#if no text forwarded, this function will be skipped.\n" );

    /// get system information
#if ( defined UNX ) || ( defined OS2 )
    /// some initialization work for UNIX OS


    struct passwd* pw;
    EXPECT_TRUE(( pw = getpwuid( getuid() ) ) != NULL) << "getpwuid: no password entry\n";

    /// get user ID;
    strUserID = ::rtl::OUString::valueOf( ( sal_Int32 )getuid( ) );

    /// get user Name;
    strUserName = ::rtl::OUString::createFromAscii( pw->pw_name );

    /// get home directory;
    EXPECT_TRUE(::osl::File::E_None == ::osl::File::getFileURLFromSystemPath( ::rtl::OUString::createFromAscii( pw->pw_dir ), strHomeDirectory )) << "#Convert from system path to URL failed.";

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
            EXPECT_TRUE(::osl::File::E_None == ::osl::File::getFileURLFromSystemPath( ::rtl::OUString::createFromAscii( PathA ), strConfigDirectory )) << "#Convert from system path to URL failed.";
        }

        lRet = RegQueryValueEx(hRegKey, "Personal", NULL, &Type, ( unsigned char * )PathA, ( unsigned long * )&lSize);
        if ( ( lRet == ERROR_SUCCESS ) && ( Type == REG_SZ ) &&  ( _access( PathA, 0 ) == 0 ) )
        {
            EXPECT_TRUE(::osl::File::E_None == ::osl::File::getFileURLFromSystemPath( ::rtl::OUString::createFromAscii( PathA ), strHomeDirectory )) << "#Convert from system path to URL failed.";
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
    PSID pSid = (PSID) new WIN_BYTE[dwSidBufferSize];
    EXPECT_TRUE(pSid!= NULL) << "# creating SID buffer failed.\n";
    memset( pSid, 0, dwSidBufferSize);

    wszDomainName = new WCHAR[dwDomainBufferSize];
    EXPECT_TRUE(wszDomainName != NULL) << "# creating Domain name buffer failed.\n";
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
                pSid = (PSID) new WIN_BYTE[cbSid];
                EXPECT_TRUE(pSid!= NULL) << "# re-creating SID buffer failed.\n";
                memset( pSid, 0, cbSid);
                dwSidBufferSize = cbSid;
            }
            if (cchDomainName > dwDomainBufferSize)
            {
                // Reallocate memory for the domain name buffer.
                wprintf(L"# The domain name buffer was too small. It will be reallocated.\n");
                delete [] wszDomainName;
                wszDomainName = new WCHAR[cchDomainName];
                EXPECT_TRUE(wszDomainName!= NULL) << "# re-creating domain name buffer failed.\n";
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
    dwSubAuthorities=*GetSidSubAuthorityCount(pSid)<=5?*GetSidSubAuthorityCount(pSid):5;

    /* buffer length: S-SID_REVISION- + identifierauthority- + subauthorities- + NULL */
    Ident=(sal_Char * )malloc(88*sizeof(sal_Char));

    /* prepare S-SID_REVISION- */
    dwSidSize=wsprintf(Ident, TEXT("S-%lu-"), dwSidRev);

    /* prepare SidIdentifierAuthority */
    if ((psia->Value[0] != 0) || (psia->Value[1] != 0))
    {
        dwSidSize+=wsprintf(Ident + strlen(Ident),
                    TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                    (USHORT)psia->Value[0],
                    (USHORT)psia->Value[1],
                    (USHORT)psia->Value[2],
                    (USHORT)psia->Value[3],
                    (USHORT)psia->Value[4],
                    (USHORT)psia->Value[5]);
    }
    else
    {
        dwSidSize+=wsprintf(Ident + strlen(Ident),
                    TEXT("%lu"),
                    (ULONG)(psia->Value[5]      )   +
                    (ULONG)(psia->Value[4] <<  8)   +
                    (ULONG)(psia->Value[3] << 16)   +
                    (ULONG)(psia->Value[2] << 24)   );
    }

    /* loop through SidSubAuthorities */
    for (dwCounter=0; dwCounter < dwSubAuthorities; dwCounter++)
    {
        dwSidSize+=wsprintf(Ident + dwSidSize, TEXT("-%lu"),
                    *GetSidSubAuthority(pSid, dwCounter) );
    }

    strUserID = ::rtl::OUString::createFromAscii( Ident );

    free(Ident);
     delete pSid;
    delete [] wszDomainName;


    /// check if logged in user is administrator:

    WIN_BOOL b;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
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
    printf("#\n#Retrived system information is below:\n");

    printf("Computer Name:              ");
    if ( strComputerName == aNullURL )
        printf(" Not retrived\n" );
    else
        printUString( strComputerName );

    printf("Current User Name:          ");
    if ( strUserName == aNullURL )
        printf(" Not retrived\n" );
    else
        printUString( strUserName );

    printf("Current User Home Directory:");
    if ( strHomeDirectory == aNullURL )
        printf(" Not retrived\n" );
    else
        printUString( strHomeDirectory );

    printf("Current Config Directory:   ");
    if ( strConfigDirectory == aNullURL )
        printf(" Not retrived\n" );
    else
        printUString( strConfigDirectory );

    printf("Current UserID:             ");
    if ( strUserID == aNullURL )
        printf(" Not retrived\n" );
    else
        printUString( strUserID );

    printf("Current User is");
    if ( isAdmin == sal_False )
        printf(" NOT Administrator.\n" );
    else
        printf(" Administrator.\n" );


    /// get and display forwarded text if available.
    aStringForward = ::rtl::OUString::createFromAscii( getForwardString() );
    if ( !aStringForward.equals( aNullURL ) && aStringForward.indexOf( (sal_Unicode)' ' ) != -1 )
    {
        sal_Int32 nFirstSpacePoint = aStringForward.indexOf( (sal_Unicode)' ' );;
        sal_Int32 nLastSpacePoint = aStringForward.lastIndexOf( (sal_Unicode)' ' );;
        if ( nFirstSpacePoint == nLastSpacePoint )
        /// only forwarded two parameters, username and password.
        {
            aLogonUser = aStringForward.copy( 0, nFirstSpacePoint );
            printf("\n#Forwarded username: ");
            printUString( aLogonUser);

            aLogonPasswd = aStringForward.copy( nFirstSpacePoint +1, aStringForward.getLength( ) - 1 );
            printf("#Forwarded password: ");
            for ( int i = nFirstSpacePoint +1; i <= aStringForward.getLength( ) - 1; i++, printf("*") );
            printf("\n" );
        }
        else
        /// forwarded three parameters, username, password and fileserver.
        {
            aLogonUser = aStringForward.copy( 0, nFirstSpacePoint );
            printf("#Forwarded username: ");
            printUString( aLogonUser);

            aLogonPasswd = aStringForward.copy( nFirstSpacePoint +1, nLastSpacePoint );
            printf("#Forwarded password: ");
            for ( int i = nFirstSpacePoint +1; i <= nLastSpacePoint; i++, printf("*") );
            printf("\n" );

            aFileServer = aStringForward.copy( nLastSpacePoint +1, aStringForward.getLength( ) - 1 );
            printf("#Forwarded FileServer: ");
            printUString( aFileServer );

        }
    }

    printf("#\n#Initialization Done.\n" );

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
