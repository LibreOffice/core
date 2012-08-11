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
#include <cstdio>
#include <stdlib.h>
#include <sys/utsname.h>
#include <_version.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pwd.h>
#include <pthread.h>
#include <limits.h>

#include <boost/unordered_map.hpp>
#include <vector>
#include <string>

#if defined (LINUX) || (FREEBSD)
#include <netinet/in.h>
#endif

typedef int SOCKET;

#define closesocket     close
#define SOCKET_ERROR    -1

#ifdef SOLARIS
const char *basename( const char *filename )
{
    const char *pSlash = strrchr( filename, '/' );

    return pSlash ? pSlash + 1 : pSlash;
}
#endif

using namespace std;

static bool g_bNoUI = false;
static bool g_bSendReport = false;
static bool g_bLoadReport = false;

static bool g_bDebugMode = false;
static int  g_signal = 0;

static string g_strProductKey;
static string g_strReportServer;
static unsigned short g_uReportPort = 80;
static string g_buildid;
static string g_strDefaultLanguage;
static string g_strXMLFileName;
static string g_strPStackFileName;
static string g_strChecksumFileName;
static string g_strProgramDir;

static char g_szStackFile[L_tmpnam] = "";
static char g_szDescriptionFile[2048] = "";
static char g_szReportFile[2048] = "";

#ifdef LINUX
#define PMAP_CMD            "cat /proc/%d/maps"
#else
#define PMAP_CMD            "pmap %d"
#endif

#define REPORT_SERVER   (g_strReportServer.c_str())
#define REPORT_PORT     g_uReportPort

static string getprogramdir()
{
    return g_strProgramDir;
}

static const char *getlocale()
{
    const char * locale = getenv( "LC_ALL" );

    if( NULL == locale )
        locale = getenv( "LC_CTYPE" );

    if( NULL == locale )
        locale = getenv( "LANG" );

    if( NULL == locale )
        locale = "C";

    return locale;
}

static const char *get_home_dir()
{
    struct passwd *ppwd = getpwuid( getuid() );

    return ppwd ? (ppwd->pw_dir ? ppwd->pw_dir : "/") : "/";
}

static string trim_string( const string& rString )
{
    string temp = rString;

    while ( temp.length() && (temp[0] == ' ' || temp[0] == '\t') )
        temp.erase( 0, 1 );

    string::size_type   len = temp.length();

    while ( len && (temp[len-1] == ' ' || temp[len-1] == '\t') )
    {
        temp.erase( len - 1, 1 );
        len = temp.length();
    }

    return temp;
}

static string xml_encode( const string &rString )
{
    string temp = rString;
    string::size_type pos = 0;

    // First replace all occurrences of '&' because it may occur in further
    // encoded chardters too

    for( pos = 0; (pos = temp.find( '&', pos )) != string::npos; pos += 4 )
        temp.replace( pos, 1, "&amp;" );

    for( pos = 0; (pos = temp.find( '<', pos )) != string::npos; pos += 4 )
        temp.replace( pos, 1, "&lt;" );

    for( pos = 0; (pos = temp.find( '>', pos )) != string::npos; pos += 4 )
        temp.replace( pos, 1, "&gt;" );

    return temp;
}

static size_t fcopy( FILE *fpout, FILE *fpin )
{
    char buffer[1024];
    size_t nBytes;
    size_t nBytesWritten = 0;

    while ( 0 != (nBytes = fread( buffer, 1, sizeof(buffer), fpin )) )
    {
        nBytesWritten += fwrite( buffer, 1, nBytes, fpout );
    }

    return nBytesWritten;
}

/*
   writes the report to a temp-file
   from which it can be reviewed and sent
*/

bool write_report( const boost::unordered_map< string, string >& rSettings )
{
    FILE    *fp = fopen( tmpnam( g_szReportFile ), "w" );
    const char *pszUserType = getenv( "STAROFFICE_USERTYPE" );

    fprintf( fp,
       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
       "<!DOCTYPE errormail:errormail PUBLIC \"-//OpenOffice.org//DTD ErrorMail 1.0//EN\" \"errormail.dtd\">\n"
       "<errormail:errormail xmlns:errormail=\"http://openoffice.org/2002/errormail\" usertype=\"%s\">\n"
       "<reportmail:mail xmlns:reportmail=\"http://openoffice.org/2002/reportmail\" version=\"1.1\" feedback=\"%s\" email=\"%s\">\n"
       "<reportmail:title>%s</reportmail:title>\n"
       "<reportmail:attachment name=\"description.txt\" media-type=\"text/plain\" class=\"UserComment\"/>\n"
       "<reportmail:attachment name=\"stack.txt\" media-type=\"text/plain\" class=\"pstack output\"/>\n"
       "</reportmail:mail>\n"
       "<officeinfo:officeinfo xmlns:officeinfo=\"http://openoffice.org/2002/officeinfo\" build=\"%s\" platform=\"%s\" language=\"%s\" exceptiontype=\"%d\" product=\"%s\" procpath=\"%s\"/>\n"
       ,
       pszUserType ? xml_encode( pszUserType ).c_str() : "",
       xml_encode(rSettings.find( "CONTACT" )->second).c_str(),
       xml_encode(rSettings.find( "EMAIL" )->second).c_str(),
       xml_encode(rSettings.find( "TITLE" )->second).c_str(),
       g_buildid.length() ? xml_encode( g_buildid ).c_str() : "unknown",
       _INPATH,
       g_strDefaultLanguage.c_str(),
       g_signal,
       g_strProductKey.length() ? xml_encode(g_strProductKey).c_str() : "unknown",
       xml_encode(getprogramdir()).c_str()
       );

    struct utsname  info;

    memset( &info, 0, sizeof(info) );
    uname( &info );

    fprintf( fp,
       "<systeminfo:systeminfo xmlns:systeminfo=\"http://openoffice.org/2002/systeminfo\">\n"
       "<systeminfo:System name=\"%s\" version=\"%s\" build=\"%s\" locale=\"%s\"/>\n"
       ,
       xml_encode( info.sysname ).c_str(),
       xml_encode( info.version ).c_str(),
       xml_encode( info.release ).c_str(),
       xml_encode( getlocale() ).c_str()
       );
    fprintf( fp, "<systeminfo:CPU type=\"%s\"/>\n", xml_encode( info.machine ).c_str() );
    fprintf( fp, "</systeminfo:systeminfo>\n" );

    FILE *fpxml = fopen( g_strXMLFileName.c_str(), "r" );
    if ( fpxml )
    {
        fcopy( fp, fpxml );
        fclose( fpxml );
    }

    FILE *fpchk = fopen( g_strChecksumFileName.c_str(), "r" );
    if ( fpchk )
    {
        fcopy( fp, fpchk );
        fclose( fpchk );
    }

    fprintf( fp, "</errormail:errormail>\n" );

    fclose( fp );

    return true;
}


bool write_description( const boost::unordered_map< string, string >& rSettings )
{
    bool    bSuccess = false;
    FILE    *fp = fopen( tmpnam( g_szDescriptionFile ), "w" );

    if ( fp )
    {
        bSuccess = true;
        fprintf( fp, "\xEF\xBB\xBF" );
        fprintf( fp, "%s\n", rSettings.find( "DESCRIPTION" )->second.c_str() );
        fclose( fp );
    }

    return bSuccess;
}

#if 0
// unused
static void printSettings( const boost::unordered_map<string,string>& rSettings )
{
    printf( "Settings:\n" );
    for( boost::unordered_map<string,string>::const_iterator it = rSettings.begin(); it != rSettings.end(); ++it )
    {
        printf( "%s=\"%s\"\n", it->first.c_str(), it->second.c_str() );
    }
}
#endif

bool save_crash_report( const string& rFileName, const boost::unordered_map< string, string >& /*rSettings*/ )
{
    bool bSuccess = false;
    FILE    *fpout = fopen( rFileName.c_str(), "w" );

    if ( fpout )
    {
        FILE *fpin = fopen( g_szStackFile, "r" );

        if ( fpin )
        {
            char    buf[1024];

            while (fgets(buf, sizeof(buf), fpin) != NULL)
            {
                fputs(buf, fpout);
            }

            bSuccess = true;

            fclose ( fpin );
        }

        fclose( fpout );
    }

    return bSuccess;
}

bool SendHTTPRequest(
                FILE *fp,
                const char *pszServer,
                unsigned short uPort = 80,
                const char *pszProxyServer = NULL,
                unsigned short uProxyPort = 8080 )
{
    bool success = false;

    struct hostent *hp;

    if ( pszProxyServer )
        hp = gethostbyname( pszProxyServer );
    else
        hp = gethostbyname( pszServer );

    if ( hp )
    {
        SOCKET  s = socket( AF_INET, SOCK_STREAM, 0 );

        if ( s )
        {
            struct sockaddr_in address;

            memcpy(&(address.sin_addr.s_addr), *(hp->h_addr_list),sizeof(struct in_addr));
            address.sin_family = AF_INET;

            if ( pszProxyServer )
                address.sin_port = ntohs( uProxyPort );
            else
                address.sin_port = ntohs( uPort );

            if ( 0 == connect( s, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) )
            {
                fseek( fp, 0, SEEK_END );
                size_t length = ftell( fp );
                fseek( fp, 0, SEEK_SET );

                char buffer[2048];

                if ( pszProxyServer )
                    sprintf( buffer,
                    "POST http://%s:%d/soap/servlet/rpcrouter HTTP/1.0\r\n"
                        "Content-Type: text/xml; charset=\"utf-8\"\r\n"
                        "Content-Length: %d\r\n"
                        "SOAPAction: \"\"\r\n\r\n",
                        pszServer,
                        uPort,
                        static_cast<int>(length)
                        );
                else
                    sprintf( buffer,
                        "POST /soap/servlet/rpcrouter HTTP/1.0\r\n"
                        "Content-Type: text/xml; charset=\"utf-8\"\r\n"
                        "Content-Length: %d\r\n"
                        "SOAPAction: \"\"\r\n\r\n",
                        static_cast<int>(length)
                        );

                if ( g_bDebugMode )
                {
                    printf( "*** Sending HTTP request ***\n\n" );
                    printf( "%s", buffer );
                }

                if ( SOCKET_ERROR != send( s, buffer, strlen(buffer), 0 ) )
                {
                    size_t nBytes;

                    do
                    {
                        nBytes = fread( buffer, 1, sizeof(buffer), fp );

                        if ( nBytes )
                        {
                            if ( g_bDebugMode )
                                fwrite( buffer, 1, nBytes, stdout );
                            success = SOCKET_ERROR != send( s, buffer, nBytes, 0 );
                        }
                    } while( nBytes && success );

                    if ( success )
                    {
                        if ( g_bDebugMode )
                            printf( "*** Receiving HTTP response ***\n\n" );

                        memset( buffer, 0, sizeof(buffer) );
                        success = SOCKET_ERROR != recv( s, buffer, sizeof(buffer), 0 );
                        if ( success )
                        {
                            char szHTTPSignature[sizeof(buffer)] = "";
                            unsigned uHTTPReturnCode = 0;

                            sscanf( buffer, "%s %d ", szHTTPSignature, &uHTTPReturnCode );
                            success = uHTTPReturnCode == 200;
                        }
                        if ( g_bDebugMode )
                            do
                            {
                                printf( "%s", buffer );
                                memset( buffer, 0, sizeof(buffer) );
                            } while ( 0 < recv( s, buffer, sizeof(buffer), 0 ) );
                    }
                }

            }

            closesocket( s );
        }
    }

    return success;
}

static void WriteSOAPRequest( FILE *fp )
{
    fprintf( fp,
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\"\n"
        "xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\"\n"
        "xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance\"\n"
        "xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\"\n"
        "xmlns:rds=\"urn:ReportDataService\"\n"
        "xmlns:apache=\"http://xml.apache.org/xml-soap\"\n"
        "SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\n"
        "<SOAP-ENV:Body>\n"
        );

    fprintf( fp, "<rds:submitReport>\n"  );
    fprintf( fp, "<body xsi:type=\"xsd:string\">This is an autogenerated crash report mail.</body>\n" );
    fprintf( fp, "<hash xsi:type=\"apache:Map\">\n" );

    FILE    *fpin = fopen( g_szReportFile, "r" );
    if ( fpin )
    {
        fprintf( fp,
            "<item>\n"
            "<key xsi:type=\"xsd:string\">reportmail.xml</key>\n"
            "<value xsi:type=\"xsd:string\"><![CDATA[" );
        fcopy( fp, fpin );
        fprintf( fp, "]]></value></item>\n" );
        fclose( fpin );
    }

    fpin = fopen( g_szDescriptionFile, "r" );
    if ( fpin )
    {
        fprintf( fp,
            "<item>\n"
            "<key xsi:type=\"xsd:string\">description.txt</key>\n"
            "<value xsi:type=\"xsd:string\"><![CDATA[" );
        fcopy( fp, fpin );
        fprintf( fp, "]]></value></item>\n" );
        fclose( fpin );
    };

    fpin = fopen( g_szStackFile, "r" );
    if ( fpin )
    {
        fprintf( fp,
            "<item>\n"
            "<key xsi:type=\"xsd:string\">stack.txt</key>\n"
            "<value xsi:type=\"xsd:string\"><![CDATA[" );
        fcopy( fp, fpin );
        fprintf( fp, "]]></value></item>\n" );
        fclose( fpin );
    };

    fprintf( fp,
        "</hash>\n"
        "</rds:submitReport>\n"
        "</SOAP-ENV:Body>\n"
        "</SOAP-ENV:Envelope>\n"
        );
}

struct RequestParams
{
    FILE *fpin;
    const char *pServer;
    const char *pProxyServer;
};


bool send_crash_report( const boost::unordered_map< string, string >& rSettings )
{
    if ( 0 == strcasecmp( rSettings.find( "CONTACT" )->second.c_str(), "true" ) &&
         !trim_string(rSettings.find( "EMAIL" )->second).length() )
    {
        return false;
    }

    char *endptr = NULL;

    const char *pProxyServer = rSettings.find( "SERVER" )->second.c_str();
    unsigned short uProxyPort = (unsigned short)strtoul( rSettings.find( "PORT" )->second.c_str(), &endptr, 10 );

    bool bUseProxy = !strcasecmp( "true", rSettings.find( "USEPROXY" )->second.c_str() );


    write_description( rSettings );
    write_report( rSettings );

    bool bSuccess = false;

    FILE    *fptemp = tmpfile();
    if ( fptemp )
    {
        WriteSOAPRequest( fptemp );
        fseek( fptemp, 0, SEEK_SET );

        bSuccess = SendHTTPRequest(
            fptemp,
            REPORT_SERVER, REPORT_PORT,
            bUseProxy ? pProxyServer : NULL,
            uProxyPort ? uProxyPort : 8080
            );

        fclose( fptemp );

    }

    unlink( g_szDescriptionFile );
    unlink( g_szReportFile );

    return bSuccess;
}


static bool append_file( const char *filename, string& rString )
{
    char buf[1024];
    bool bSuccess = false;

    FILE *fp = fopen( filename, "r" );
    if ( fp )
    {
        while (fgets(buf, sizeof(buf), fp) != NULL)
        {
            rString.append( buf );
        }
        fclose( fp );
        bSuccess = true;
    }

    return bSuccess;
}

string crash_get_details( const boost::unordered_map< string, string >& rSettings )
{
    string aRet;

    write_description( rSettings );
    write_report( rSettings );

    aRet.append( rSettings.find( "TITLE" )->second.c_str() );
    aRet.append( "\n\n" );
    append_file( g_szDescriptionFile, aRet );
    aRet.append( "\n\n-------\n\n" );
    append_file( g_szReportFile, aRet );
    aRet.append( "\n\n-------\n\n" );
    append_file( g_szStackFile, aRet );

    unlink( g_szDescriptionFile );
    unlink( g_szReportFile );

    return aRet;
}


// ensure validity of program relative paths
static void setup_program_dir( const char* progname )
{
    char    szCanonicProgPath[PATH_MAX];


    if ( realpath( progname, szCanonicProgPath ) )
    {
        string aDir = szCanonicProgPath;

        size_t pos = aDir.rfind( '/' );
        // FIXME: search PATH if necessary
        assert( pos != string::npos );

        g_strProgramDir = aDir.substr( 0, pos + 1 );
        aDir.erase( pos );
        int ret = chdir( aDir.c_str() );
        if (!ret)
        {
            return;
        }
    }
}

//*************************************************************************

static long setup_commandline_arguments( int argc, char** argv, int *pSignal )
{
    long    pid = 0;
    int     signal = 0;

    for ( int n = 1; n < argc; n++ )
    {
        if ( 0 == strcmp( argv[n], "-p" ) )
        {
            if ( ++n < argc )
                pid = strtol( argv[n], NULL, 0 );
        }
        else if ( 0 == strcmp( argv[n], "-s" ) )
        {
            if ( ++n < argc )
                signal = strtol( argv[n], NULL, 0 );
        }
        else if ( 0 == strcmp( argv[n], "-debug" ) )
        {
            g_bDebugMode = true;
        }
        else if ( 0 == strcmp( argv[n], "-xml" ) )
        {
            if ( ++n < argc )
                g_strXMLFileName = argv[n];
        }
        else if ( 0 == strcmp( argv[n], "-stack" ) )
        {
            if ( ++n < argc )
                g_strPStackFileName = argv[n];
        }
        else if ( 0 == strcmp( argv[n], "-chksum" ) )
        {
            if ( ++n < argc )
                g_strChecksumFileName = argv[n];
        }
        else if ( 0 == strcmp( argv[n], "-noui" ) )
        {
            g_bNoUI = true;
        }
        else if ( 0 == strcmp( argv[n], "-send" ) )
        {
            g_bSendReport = true;
        }
        else if ( 0 == strcmp( argv[n], "-load" ) )
        {
            g_bLoadReport = true;
        }
        else if ( argv[n] && strlen(argv[n]) )
        {
            printf(
                "\n%s crash_report %s\n\n" \
                "/?, -h[elp]          %s\n\n" \
                "%-20s %s\n\n",
                "%MSG_CMDLINE_USAGE%",
                "%MSG_PARAM_PROCESSID%",
                "%MSG_PARAM_HELP_DESCRIPTION%",
                "%MSG_PARAM_PROCESSID%",
                "%MSG_PARAM_PROCESSID_DESCRIPTION%"
                );
            break;
        }
    }

    *pSignal = signal;

    return pid;
}

//*************************************************************************

static bool read_line( FILE *fp, string& rLine )
{
    char szBuffer[1024];
    bool bSuccess = false;
    bool bEOL = false;
    string  line;


    while ( !bEOL && fgets( szBuffer, sizeof(szBuffer), fp ) )
    {
        int len = strlen(szBuffer);

        bSuccess = true;

        while ( len && szBuffer[len - 1] == '\n' )
        {
            szBuffer[--len] = 0;
            bEOL = true;
        }

        line.append( szBuffer );
    }

    rLine = line;
    return bSuccess;
}

static string get_script_string( const char *pFileName, const char *pKeyName )
{
    FILE    *fp = fopen( pFileName, "r" );
    string  retValue;

    if ( fp )
    {
        string line;

        while ( read_line( fp, line ) )
        {
            line = trim_string( line );


            string::size_type iEqualSign = line.find( '=', 0 );

            if ( iEqualSign != string::npos )
            {
                string  keyname = line.substr( 0, iEqualSign );
                keyname = trim_string( keyname );

                string  value = line.substr( iEqualSign + 1, string::npos );
                value = trim_string( value );

                if ( value.length() && '\"' == value[0] )
                {
                    value.erase( 0, 1 );

                    string::size_type iQuotes = value.find( '"', 0 );

                    if ( iQuotes != string::npos )
                        value.erase( iQuotes );
                }

                if ( 0 == strcasecmp( keyname.c_str(), pKeyName ) )
                {
                    retValue = value;
                    break;
                }
            }
        }

        fclose( fp );
    }

    return retValue;
}

static string get_profile_string( const char *pFileName, const char *pSectionName, const char *pKeyName, const char *pDefault = NULL )
{
    FILE    *fp = fopen( pFileName, "r" );
    string  retValue = pDefault ? pDefault : "";

    if ( fp )
    {
        string line;
        string section;

        while ( read_line( fp, line ) )
        {
            line = trim_string( line );

            if ( line.length() && line[0] == '[' )
            {
                line.erase( 0, 1 );
                string::size_type end = line.find( ']', 0 );

                if ( string::npos != end )
                    section = trim_string( line.substr( 0, end ) );
            }
            else
            {

                string::size_type iEqualSign = line.find( '=', 0 );

                if ( iEqualSign != string::npos )
                {
                    string  keyname = line.substr( 0, iEqualSign );
                    keyname = trim_string( keyname );

                    string  value = line.substr( iEqualSign + 1, string::npos );
                    value = trim_string( value );

                    if (
                        0 == strcasecmp( section.c_str(), pSectionName ) &&
                        0 == strcasecmp( keyname.c_str(), pKeyName )
                         )
                    {
                        retValue = value;
                        break;
                    }
                }
            }
        }

        fclose( fp );
    }

    return retValue;
}

static string get_environment_string( const char *pEnvName )
{
    const char *pEnvValue = getenv( pEnvName );

    if ( pEnvValue )
        return pEnvValue;
    else
        return "";
}

static string read_from_file( const string& rFileName )
{
    string  content;
    FILE *fp = fopen( rFileName.c_str(), "r" );

    if ( fp )
    {
        char    buffer[256 + 1];
        size_t  nBytesRead;

        while( 0 != ( nBytesRead = fread( buffer, 1, sizeof(buffer) - 1,  fp ) ) )
        {
            buffer[nBytesRead] = 0;
            content += buffer;
        }

        fclose( fp );
    }

    return content;
}

#define RCFILE ".crash_reportrc"
#define XMLFILE ".crash_report_frames"
#define CHKFILE ".crash_report_checksum"
#define LCKFILE ".crash_report_unsent"
#define PRVFILE ".crash_report_preview"

static void load_crash_data()
{
    g_strXMLFileName = get_home_dir();
    g_strXMLFileName += "/";
    g_strXMLFileName += string(XMLFILE);

    g_strChecksumFileName = get_home_dir();
    g_strChecksumFileName += "/";
    g_strChecksumFileName += string(CHKFILE);
}

static bool write_crash_data()
{
    bool success = true;
    string  sFile = get_home_dir();

    sFile += "/";
    sFile += string(XMLFILE);

    FILE *fp = fopen( sFile.c_str(), "w" );

    if ( fp )
    {
        FILE    *fpin = fopen( g_strXMLFileName.c_str(), "r" );

        if ( fpin )
        {
            fcopy( fp, fpin );
            fclose( fpin );
        }

        fclose( fp );
    }

    sFile = get_home_dir();

    sFile += "/";
    sFile += string(CHKFILE);

    fp = fopen( sFile.c_str(), "w" );

    if ( fp )
    {
        FILE    *fpin = fopen( g_strChecksumFileName.c_str(), "r" );

        if ( fpin )
        {
            fcopy( fp, fpin );
            fclose( fpin );
        }

        fclose( fp );
    }

    sFile = get_home_dir();

    sFile += "/";
    sFile += string(LCKFILE);

    fp = fopen( sFile.c_str(), "w" );

    if ( fp )
    {
        fprintf( fp, "Unsent\n" );
        fclose( fp );
    }

    return success;
}

#if 0
// unused
static bool write_settings( const boost::unordered_map< string, string >& rSettings )
{
    bool success = false;
    string  sRCFile = get_home_dir();

    sRCFile += "/";
    sRCFile += string(RCFILE);

    FILE *fp = fopen( sRCFile.c_str(), "w" );

    if ( fp )
    {
        fprintf( fp, "[Options]\n" );
        fprintf( fp, "UseProxy=%s\n", rSettings.find( "USEPROXY" )->second.c_str() );
        fprintf( fp, "ProxyServer=%s\n", rSettings.find( "SERVER" )->second.c_str() );
        fprintf( fp, "ProxyPort=%s\n", rSettings.find( "PORT" )->second.c_str() );
        fprintf( fp, "ReturnAddress=%s\n", rSettings.find( "EMAIL" )->second.c_str() );
        fprintf( fp, "AllowContact=%s\n", rSettings.find( "CONTACT" )->second.c_str() );
        fclose( fp );
    }

    return success;
}
#endif

static void read_settings( boost::unordered_map< string, string >& rSettings )
{
    string  sRCFile = get_home_dir();

    sRCFile += "/";
    sRCFile += string(RCFILE);

    rSettings[ "EMAIL" ] = get_profile_string( sRCFile.c_str(), "Options", "ReturnAddress" );
    rSettings[ "SERVER" ] = get_profile_string( sRCFile.c_str(), "Options", "ProxyServer" );
    rSettings[ "PORT" ] = get_profile_string( sRCFile.c_str(), "Options", "ProxyPort" );
    rSettings[ "USEPROXY" ] = get_profile_string( sRCFile.c_str(), "Options", "UseProxy" );
    rSettings[ "CONTACT" ] = get_profile_string( sRCFile.c_str(), "Options", "AllowContact" );
    rSettings[ "DESCRIPTION" ] = "";
    rSettings[ "TITLE" ] = "";
}

static void read_settings_from_environment( boost::unordered_map< string, string >& rSettings )
{
    string  strEnv;

    strEnv = get_environment_string( "ERRORREPORT_RETURNADDRESS" );
    if ( strEnv.length() )
    {
        rSettings[ "EMAIL" ] = strEnv;
        if ( !(rSettings.find( "CONTACT" )->second).length() )
            rSettings[ "CONTACT" ] = "true";
    }
    else if ( !(rSettings.find( "CONTACT" )->second).length() )
        rSettings[ "CONTACT" ] = "false";


    strEnv = get_environment_string( "ERRORREPORT_HTTPPROXYSERVER" );
    if ( strEnv.length() )
        rSettings[ "SERVER" ] = strEnv;

    strEnv = get_environment_string( "ERRORREPORT_HTTPPROXYPORT" );
    if ( strEnv.length() )
        rSettings[ "PORT" ] = strEnv;

    strEnv = get_environment_string( "ERRORREPORT_HTTPCONNECTIONTYPE" );
    if ( strEnv.length() )
        rSettings[ "USEPROXY" ] = 0 == strcasecmp( strEnv.c_str(), "MANUALPROXY" ) ? "true" : "false";

    strEnv = get_environment_string( "ERRORREPORT_BODYFILE" );
    if ( strEnv.length() )
        rSettings[ "DESCRIPTION" ] = read_from_file( strEnv );

    strEnv = get_environment_string( "ERRORREPORT_SUBJECT" );
    if ( strEnv.length() )
        rSettings[ "TITLE" ] = strEnv;
}

static bool setup_version()
{
    if ( !getenv( "PRODUCTNAME" ) )
    {
        string productkey = get_profile_string( "bootstraprc", "Bootstrap", "ProductKey" );

        g_strProductKey = productkey;

        if ( productkey.length() )
        {
            static string productname;
            static string productversion;
            string::size_type   iSpace = productkey.find( ' ', 0 );

            if ( string::npos != iSpace )
            {
                productname = productkey.substr( 0, iSpace );
                productversion = productkey.substr( iSpace + 1, string::npos );
            }
            else
                productname = productkey;

            productname.insert( 0, "PRODUCTNAME=" );
            putenv( (char *)productname.c_str() );

            productversion.insert( 0, "PRODUCTVERSION=" );
            putenv( (char *)productversion.c_str() );
        }
    }

    g_buildid = get_profile_string( "versionrc", "Version", "BuildId" );
    g_strDefaultLanguage = get_script_string( "instdb.ins", "DefaultLanguage"  );

    g_strReportServer = get_profile_string( "bootstraprc", "ErrorReport", "ErrorReportServer" );

    string strReportPort = get_profile_string( "bootstraprc", "ErrorReport", "ErrorReportPort", "80" );
    char *endptr = NULL;
    unsigned short uReportPort = (unsigned short)strtoul( strReportPort.c_str(), &endptr, 10 );
    g_uReportPort = uReportPort ? uReportPort : 80;

    return 0 != g_strReportServer.length();
}

int main( int argc, char** argv )
{
    FILE *fin = freopen( "/dev/null", "w", stderr );
    if (!fin)
    {
        return -1;
    }

    setup_program_dir( argv[0] );

    // Don't start if report server is not given

    if ( setup_version() )
    {
        /*long pid =*/ setup_commandline_arguments( argc, argv, &g_signal );

        if ( g_bLoadReport )
        {
            load_crash_data();
        }

        if ( g_bSendReport )
        {
            boost::unordered_map< string, string > aDialogSettings;

            read_settings( aDialogSettings );
            read_settings_from_environment( aDialogSettings );

            send_crash_report( aDialogSettings );
        }
        else
        {
            boost::unordered_map< string, string > aDialogSettings;

            read_settings( aDialogSettings );
            read_settings_from_environment( aDialogSettings );

            write_crash_data();
            write_report( aDialogSettings );

            string  sPreviewFile = get_home_dir();
            sPreviewFile += "/";
            sPreviewFile += string(PRVFILE);

            FILE *fpout = fopen( sPreviewFile.c_str(), "w+" );
            if ( fpout )
            {
                FILE *fpin = fopen( g_szReportFile, "r" );
                if ( fpin )
                {
                    fcopy( fpout, fpin );
                    fclose( fpin );
                }
                fclose( fpout );
            }

            unlink( g_szReportFile );
        }

        if ( g_bLoadReport )
        {
            unlink( g_strXMLFileName.c_str() );
            unlink( g_strChecksumFileName.c_str() );
        }

        unlink( g_szStackFile );

        return 0;
    }

    return -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
