/*************************************************************************
 *
 *  $RCSfile: main.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-25 11:27:38 $
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
#include <interface.hxx>
#include <cstdio>
#include <sys/utsname.h>
#include <_version.h>
#include <errno.h>
#include <string>
#include <string.h>
#include <assert.h>

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pwd.h>
#include <pthread.h>

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

static bool g_bDebugMode = false;
static int  g_signal = 0;

static string g_strReportServer;
static unsigned short g_uReportPort = 80;
static string g_buildid;
static string g_strDefaultLanguage;
static string g_strXMLFileName;
static string g_strPStackFileName;
static string g_strProgramDir;

static char g_szStackFile[L_tmpnam] = "";
static char g_szDescriptionFile[2048] = "";
static char g_szReportFile[2048] = "";

#define SO_CRASHREPORT_MAIL "so-report@sun.com"
#define PSTACK_CMD          "pstack %d"

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

    while ( temp.length() && temp[0] == ' ' || temp[0] == '\t' )
        temp.erase( 0, 1 );

    string::size_type   len = temp.length();

    while ( len && temp[len-1] == ' ' || temp[len-1] == '\t' )
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

    // First replace all occurences of '&' because it may occur in further
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
bool write_stack( long pid )
{
    bool bSuccess = false;

    if ( pid )
    {
        char *stmp= tmpnam( g_szStackFile );
        FILE *fout, *fin;
        char buf[1024];

        /* open mailfile */
        fout = fopen(stmp, "w");

        if ( fout )
        {
            char cmdbuf[1024];

            fputs( "<***stacks***>\n", fout );

            if ( !g_strPStackFileName.length() )
            {

                snprintf(cmdbuf, 1024, PSTACK_CMD, pid);
                fin = popen(cmdbuf, "r");

                if ( fin )
                {
                    while (fgets(buf, 1024, fin) != NULL)
                    {
                        bSuccess = true;
                        fputs(buf, fout);
                    }
                    pclose( fin );

                }

            }
            else
            {
                fin = fopen( g_strPStackFileName.c_str(), "r" );
                if ( fin )
                {
                    bSuccess = true;
                    fcopy( fout, fin );
                    fclose( fin );
                }
            }

            fputs( "</***stacks***>\n", fout );

            if ( bSuccess )
            {
                fputs( "<***maps***>\n", fout );
                bSuccess = false;

                snprintf(cmdbuf, 1024, PMAP_CMD, pid);
                fin = popen(cmdbuf, "r");

                if ( fin )
                {
                    while (fgets(buf, 1024, fin) != NULL)
                    {
                        bSuccess = true;
                        fputs(buf, fout);
                    }
                    pclose( fin );

                }
                fputs( "</***maps***>\n", fout );
            }

            fclose ( fout );

        }
    }


    return bSuccess;
}


bool write_report( const hash_map< string, string >& rSettings )
{
    FILE    *fp = fopen( tmpnam( g_szReportFile ), "w" );
    const char *pszUserType = getenv( "STAROFFICE_USERTYPE" );
    const char *pszProductName = getenv( "PRODUCTNAME" );

    fprintf( fp,
       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
       "<!DOCTYPE errormail:errormail PUBLIC \"-//OpenOffice.org//DTD ErrorMail 1.0//EN\" \"errormail.dtd\">\n"
       "<errormail:errormail xmlns:errormail=\"http://openoffice.org/2002/errormail\" usertype=\"%s\">\n"
       "<reportmail:mail xmlns:reportmail=\"http://openoffice.org/2002/reportmail\" version=\"1.0\" feedback=\"%s\" email=\"%s\">\n"
       "<reportmail:title>%s</reportmail:title>\n"
       "<reportmail:attachment name=\"description.txt\" media-type=\"text/plain\" class=\"UserComment\"/>\n"
       "<reportmail:attachment name=\"stack.txt\" media-type=\"text/plain\" class=\"pstack output\"/>\n"
       "</reportmail:mail>\n"
       "<officeinfo:officeinfo xmlns:officeinfo=\"http://openoffice.org/2002/officeinfo\" build=\"%s\" platform=\"%s\" language=\"%s\" exceptiontype=\"%d\" product=\"%s\" procpath=\"%s\"/>\n"
       ,
       pszUserType ? pszUserType : "",
       xml_encode(rSettings.find( "CONTACT" )->second).c_str(),
       xml_encode(rSettings.find( "EMAIL" )->second).c_str(),
       xml_encode(rSettings.find( "TITLE" )->second).c_str(),
       g_buildid.length() ? g_buildid.c_str() : "unknown",
       _INPATH,
       g_strDefaultLanguage.c_str(),
       g_signal,
       pszProductName ? pszProductName : "unknown",
       getprogramdir().c_str()
       );

    struct utsname  info;

    memset( &info, 0, sizeof(info) );
    uname( &info );

    fprintf( fp,
       "<systeminfo:systeminfo xmlns:systeminfo=\"http://openoffice.org/2002/systeminfo\">\n"
       "<systeminfo:System name=\"%s\" version=\"%s\" build=\"%s\" locale=\"%s\"/>\n"
       ,
       info.sysname,
       info.version,
       info.release,
       getlocale()
       );
    fprintf( fp, "<systeminfo:CPU type=\"%s\"/>\n", info.machine );
    fprintf( fp, "</systeminfo:systeminfo>\n" );

    FILE *fpxml = fopen( g_strXMLFileName.c_str(), "r" );
    if ( fpxml )
    {
        fcopy( fp, fpxml );
        fclose( fpxml );
    }
    fprintf( fp, "</errormail:errormail>\n" );

    fclose( fp );

    return true;
}


bool write_description( const hash_map< string, string >& rSettings )
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


static void printSettings( const hash_map<string,string>& rSettings )
{
    printf( "Settings:\n" );
    for( hash_map<string,string>::const_iterator it = rSettings.begin(); it != rSettings.end(); ++it )
    {
        printf( "%s=\"%s\"\n", it->first.c_str(), it->second.c_str() );
    }
}

bool save_crash_report( const string& rFileName, const hash_map< string, string >& rSettings )
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
                        length
                        );
                else
                    sprintf( buffer,
                        "POST /soap/servlet/rpcrouter HTTP/1.0\r\n"
                        "Content-Type: text/xml; charset=\"utf-8\"\r\n"
                        "Content-Length: %d\r\n"
                        "SOAPAction: \"\"\r\n\r\n",
                        length
                        );

                if ( g_bDebugMode )
                {
                    printf( "*** Sending HTTP request ***\n\n" );
                    printf( buffer );
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
                                printf( buffer );
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
    bool    success;
    FILE    *fpin;
    const char *pServer;
    unsigned short uPort;
    const char *pProxyServer;
    unsigned short uProxyPort;
    WizardDialog *pDialog;
};

extern "C" void *http_send_thread( void *arg )
{
    RequestParams *request = (RequestParams *)arg;

    int oldtype;

    /* Killing me hardly ;-) */
    pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype );

    request->success = SendHTTPRequest(
        request->fpin,
        request->pServer,
        request->uPort,
        request->pProxyServer,
        request->uProxyPort
        );

    request->pDialog->hide_sendingstatus();

    return 0;
}

bool SendAsyncHTTPRequest(
                WizardDialog &rDialog,
                FILE *fp,
                const char *pszServer,
                unsigned short uPort = 80,
                const char *pszProxyServer = NULL,
                unsigned short uProxyPort = 8080 )
{
    RequestParams   request;

    request.success = false;
    request.pDialog = &rDialog;
    request.fpin = fp;
    request.pServer = pszServer;
    request.uPort = uPort;
    request.pProxyServer = pszProxyServer;
    request.uProxyPort = uProxyPort;

    pthread_t thread = 0;

    if (
        0 == pthread_create (
            &thread,
            NULL,
            http_send_thread,
            (void*)&request )
            )
    {
        gint response = rDialog.show_sendingstatus( TRUE );

        pthread_cancel( thread );

        void *thread_result = NULL;

        pthread_join( thread, &thread_result );
    }

    return request.success;
}

bool send_crash_report( WizardDialog &rDialog, const hash_map< string, string >& rSettings )
{
    if ( 0 == strcasecmp( rSettings.find( "CONTACT" )->second.c_str(), "true" ) &&
         !trim_string(rSettings.find( "EMAIL" )->second).length() )
    {
        rDialog.show_messagebox( StringResource::get( "%ERROR_MSG_NOEMAILADDRESS%" ) );
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

        /*
        bSuccess = SendHTTPRequest(
            fptemp,
            REPORT_SERVER, REPORT_PORT,
            bUseProxy ? pProxyServer : NULL,
            uProxyPort ? uProxyPort : 8080
            );
        */

        bSuccess = SendAsyncHTTPRequest(
            rDialog,
            fptemp,
            REPORT_SERVER, REPORT_PORT,
            bUseProxy ? pProxyServer : NULL,
            uProxyPort ? uProxyPort : 8080
            );

        fclose( fptemp );

        if ( bSuccess )
            rDialog.show_sendingstatus( FALSE );
        else
            rDialog.show_messagebox( StringResource::get( "%ERROR_MSG_PROXY%" ) );
    }
    else
        rDialog.show_messagebox( "%ERROR_MSG_DISK_FULL%" );

    unlink( g_szDescriptionFile );
    unlink( g_szReportFile );

    return bSuccess;
}


static bool append_file( const char *filename, string& rString )
{
    char    buf[1024];
    bool    bSuccess = false;

    FILE *fp = fopen( filename, "r" );
    if ( fp )
    {
        bSuccess = true;
        while (fgets(buf, sizeof(buf), fp) != NULL)
        {
            rString.append( buf );
        }
        fclose( fp );
    }

    return true;
}

string crash_get_details( const hash_map< string, string >& rSettings )
{
    string aRet;

    FILE    *fp;
    char    buf[1024];

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
    string aDir = progname;
    size_t pos = aDir.rfind( '/' );
    // FIXME: search PATH if necessary
    assert( pos != string::npos );
    g_strProgramDir = aDir.substr( 0, pos + 1 );
    aDir.erase( pos );
    chdir( aDir.c_str() );
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
        else if ( argv[n] && strlen(argv[n]) )
        {
            printf(
                "\n%s crash_report %s\n\n" \
                "/?, -h[elp]          %s\n\n" \
                "%-20s %s\n\n",
                StringResource::get( "%MSG_CMDLINE_USAGE%" ),
                StringResource::get( "%MSG_PARAM_PROCESSID%" ),
                StringResource::get( "%MSG_PARAM_HELP_DESCRIPTION%" ),
                StringResource::get( "%MSG_PARAM_PROCESSID%" ),
                StringResource::get( "%MSG_PARAM_PROCESSID_DESCRIPTION%" )
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
        string section;

        while ( read_line( fp, line ) )
        {
            line = trim_string( line );


            string::size_type iEqualSign = line.find( '=', 0 );

            if ( iEqualSign != string::npos )
            {
                string  keyname = line.substr( 0, iEqualSign );
                keyname = trim_string( keyname );

                string  value = line.substr( iEqualSign + 1, -1 );
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

                    string  value = line.substr( iEqualSign + 1, -1 );
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

#define RCFILE ".crash_reportrc"

static bool write_settings( const hash_map< string, string >& rSettings )
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

static void read_settings( hash_map< string, string >& rSettings )
{
    string  sRCFile = get_home_dir();

    sRCFile += "/";
    sRCFile += string(RCFILE);

    rSettings[ "EMAIL" ] = get_profile_string( sRCFile.c_str(), "Options", "ReturnAddress" );
    rSettings[ "SERVER" ] = get_profile_string( sRCFile.c_str(), "Options", "ProxyServer" );
    rSettings[ "PORT" ] = get_profile_string( sRCFile.c_str(), "Options", "ProxyPort" );
    rSettings[ "USEPROXY" ] = get_profile_string( sRCFile.c_str(), "Options", "UseProxy" );
    rSettings[ "CONTACT" ] = get_profile_string( sRCFile.c_str(), "Options", "AllowContact" );
}

static bool setup_version()
{
    if ( !getenv( "PRODUCTNAME" ) )
    {
        string productkey = get_profile_string( "bootstraprc", "Bootstrap", "ProductKey" );

        if ( productkey.length() )
        {
            static string productname;
            static string productversion;
            string::size_type   iSpace = productkey.find( ' ', 0 );

            if ( string::npos != iSpace )
            {
                productname = productkey.substr( 0, iSpace );
                productversion = productkey.substr( iSpace + 1, -1 );
            }
            else
                productname = productkey;

            productname.insert( 0, "PRODUCTNAME=" );
            putenv( (char *)productname.c_str() );

            productversion.insert( 0, "PRODUCTVERSION=" );
            putenv( (char *)productversion.c_str() );
        }
    }

    g_buildid = get_profile_string( "bootstraprc", "Bootstrap", "BuildId" );
    g_strDefaultLanguage = get_script_string( "instdb.ins", "DefaultLanguage"  );

    g_strReportServer = get_profile_string( "bootstraprc", "ErrorReport", "ErrorReportServer" );

    string strReportPort = get_profile_string( "bootstraprc", "ErrorReport", "ErrorReportPort", "80" );
    char *endptr = NULL;
    unsigned short uReportPort = (unsigned short)strtoul( strReportPort.c_str(), &endptr, 10 );
    g_uReportPort = uReportPort ? uReportPort : 80;

    return 0 != g_strReportServer.length();
}

// Use gconftool-2 to determine if gnome accessiblity is enabled

static bool get_accessibility_state()
{
    bool bAccessible = false;
    FILE *fin = popen( "gconftool-2 -g /desktop/gnome/interface/accessibility", "r");

    if ( fin )
    {
        char buffer[sizeof("true")];

        bAccessible = fgets( buffer, sizeof(buffer), fin ) && 0 == strcmp( buffer, "true" );

        pclose( fin );
    }

    return bAccessible;
}

int main( int argc, char** argv )
{
    freopen( "/dev/null", "w", stderr );

    setup_program_dir( argv[0] );

    // Don't start if accessiblity is enabled or report server is not given

    if ( !get_accessibility_state() && setup_version() )
    {
           gtk_set_locale ();
           gtk_init (&argc, &argv);

           StringResource::init( argc, argv );

        long pid = setup_commandline_arguments( argc, argv, &g_signal );

        if ( write_stack( pid ) )
        {
            WizardDialog aDialog;

            hash_map< string, string >& rDialogSettings = aDialog.getSettings();

            read_settings( rDialogSettings );

            aDialog.insertPage( new WelcomePage( &aDialog ) );
            aDialog.insertPage( new MainPage( &aDialog ) );

            aDialog.show();

            gtk_main();

            write_settings( rDialogSettings );

            unlink( g_szStackFile );

            return 0;
        }
    }

    return -1;
}
