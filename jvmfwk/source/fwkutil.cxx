/*************************************************************************
 *
 *  $RCSfile: fwkutil.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: jl $ $Date: 2004-05-14 14:44:09 $
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

#ifdef WNT
#include <windows.h>
#endif

#include "libxmlutil.hxx"
#include "osl/mutex.hxx"
#include "osl/module.hxx"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/bootstrap.hxx"
#include "osl/file.hxx"
#include "osl/thread.hxx"
#include "osl/process.h"
#include "rtl/instance.hxx"
#include "rtl/uri.hxx"
#include "rtl/process.h"
#include "osl/getglobalmutex.hxx"
#include "libxml/xpathInternals.h"

#include "framework.hxx"
#include "elements.hxx"
#include "fwkutil.hxx"


#define JAVASETTINGS "javasettings"
#define VENDORSETTINGS "javavendors.xml"
/** The vector contains on return file urls to the plugins.
 */
namespace jfw
{

bool  g_bJavaSet = false;

struct Init
{
    osl::Mutex * operator()()
        {
            static osl::Mutex aInstance;
            return &aInstance;
        }
};
osl::Mutex * getFwkMutex()
{
    return rtl_Instance< osl::Mutex, Init, ::osl::MutexGuard,
        ::osl::GetGlobalMutex >::create(
            Init(), ::osl::GetGlobalMutex());
}

const rtl::Bootstrap & getBootstrapHandle()
{
    static rtl::Bootstrap *pBootstrap = 0;
    if( !pBootstrap )
    {
        rtl::OUString exe;
        osl_getExecutableFile( &(exe.pData) );

        sal_Int32 nIndex = exe.lastIndexOf( '/' );
        rtl::OUString ret;
        if( exe.getLength() && nIndex != -1 )
        {
            rtl::OUStringBuffer buf( exe.getLength() + 10 );
            buf.append( exe.getStr() , nIndex +1 ).appendAscii( SAL_CONFIGFILE("uno") );
            ret = buf.makeStringAndClear();
        }
#if OSL_DEBUG_LEVEL > 1
        rtl::OString o = rtl::OUStringToOString( ret , RTL_TEXTENCODING_ASCII_US );
        printf( "JavaVM: Used ininame %s\n" , o.getStr() );
#endif
        static rtl::Bootstrap  bootstrap( ret );
        pBootstrap = &bootstrap;
    }
    return *pBootstrap;
}
rtl::OUString retrieveClassPath( ::rtl::OUString const & macro )
{
    ::rtl::OUString classpath( macro );
    getBootstrapHandle().expandMacrosFrom( classpath );
    ::rtl::OUStringBuffer buf;
    sal_Int32 index = 0;
    char szClassPathSep[] = {SAL_PATHSEPARATOR,0};
    do
    {
        ::rtl::OUString token( classpath.getToken( 0, ' ', index ).trim() );
        if (token.getLength())
        {
            if (token.matchIgnoreAsciiCaseAsciiL(
                    RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.expand:") ))
            {
                token = ::rtl::Uri::decode(
                    token.copy( sizeof ("vnd.sun.star.expand:") -1 ),
                    rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
                getBootstrapHandle().expandMacrosFrom( token );
            }

            ::rtl::OUString systemPathElement;
            oslFileError rc = osl_getSystemPathFromFileURL(
                token.pData, &systemPathElement.pData );
            OSL_ASSERT( rc == osl_File_E_None );
            if (rc == osl_File_E_None && systemPathElement.getLength() > 0)
            {
                if (buf.getLength() > 0)
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                                         szClassPathSep) );
                buf.append( systemPathElement );
            }
        }
    }
    while (index >= 0);
    return buf.makeStringAndClear();
}

//ToDo we should use a SAL function to determine which platform and OS we are working on.
rtl::OUString getPlatform()
{
    char * szArchitecture =
#if defined SPARC
    "sparc";
#elif defined INTEL
    "x86";
#elif defined POWERPC
    "ppc";
#elif defined MIPS
    "mips";
#elif defined S390
    "s390";
#else
#error unknown plattform
#endif

    char * szOS =

#if defined WNT
        "wnt";
#elif defined  SOLARIS
    "solaris";
#elif defined LINUX
    "linux";
#elif defined MACOSX
    "macosx";
#elif defined FREEBSD
    "freebsd";
#elif defined NETBSD
    "netbsd";
#elif defined AIX
    "aix";
#else
#error unknown operating system
#endif
    rtl::OUStringBuffer buf(256);
    buf.appendAscii("_");
    rtl::OUString sOS = rtl::OStringToOUString(
        rtl::OString(szOS), RTL_TEXTENCODING_UTF8);
    buf.append(sOS);
    buf.appendAscii("_");
    rtl::OUString sArch = rtl::OStringToOUString(
        rtl::OString(szArchitecture), RTL_TEXTENCODING_UTF8);
    buf.append(sArch);
    return buf.makeStringAndClear();
}

javaFrameworkError getPluginLibrary(rtl::OUString & sLibUrl)
{
    javaFrameworkError errcode = JFW_E_NONE;
    CNodeJava javaNode;
    if ((errcode = javaNode.loadFromSettings()) != JFW_E_NONE)
        return errcode;

    CJavaInfo aInfo(javaNode.getJavaInfo());
    if (aInfo == NULL)
        return JFW_E_NO_SELECT;

    //With the vendor name we can get the associated plugin library
    //from the javavendors.xml
    rtl::OString sVendorsPath = getVendorSettingsPath();
    CXmlDocPtr docVendor;
    CXPathContextPtr contextVendor;
    docVendor = xmlParseFile(sVendorsPath.getStr());
    if (docVendor == NULL)
        return JFW_E_CONFIG_READWRITE;

    contextVendor = xmlXPathNewContext(docVendor);
    if (xmlXPathRegisterNs(contextVendor, (xmlChar*) "jf",
        (xmlChar*) NS_JAVA_FRAMEWORK) == -1)
        return JFW_E_CONFIG_READWRITE;

    rtl::OUStringBuffer usBuffer(256);
    usBuffer.appendAscii("/jf:javaSelection/jf:plugins/jf:library[@vendor=\"");
    usBuffer.append(aInfo.getVendor());
    usBuffer.appendAscii("\"]/text()");
    rtl::OUString ouExpr = usBuffer.makeStringAndClear();
    rtl::OString sExpression =
        rtl::OUStringToOString(ouExpr, osl_getThreadTextEncoding());
    CXPathObjectPtr pathObjVendor;
    pathObjVendor = xmlXPathEvalExpression(
        (xmlChar*) sExpression.getStr(), contextVendor);
    if (xmlXPathNodeSetIsEmpty(pathObjVendor->nodesetval))
        return JFW_E_FORMAT_STORE;
    CXmlCharPtr xmlCharPlugin;
    xmlCharPlugin =
        xmlNodeListGetString(
            docVendor,pathObjVendor->nodesetval->nodeTab[0], 1);

    //make an absolute file url from the relativ plugin URL
    rtl::OUString sLibPath = getBaseInstallation()  +
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) + xmlCharPlugin;

    sLibUrl = sLibPath;
    return errcode;
}


javaFrameworkError getVendorPluginURLs(
    const xmlDocPtr doc,
    const xmlXPathContextPtr  context,
    std::vector<PluginLibrary> * vecPlugins)
{
    OSL_ASSERT(vecPlugins && doc && context);

    //get the nodeset for the library elements
    jfw::CXPathObjectPtr result;
    result = xmlXPathEvalExpression(
        (xmlChar*)"/jf:javaSelection/jf:plugins/jf:library", context);
    if (xmlXPathNodeSetIsEmpty(result->nodesetval))
    {
        return JFW_E_ERROR;
    }
    vecPlugins->clear();

    //get the values of the library elements + vendor attribute
    xmlNode* cur = result->nodesetval->nodeTab[0];
    while (cur != NULL)
    {
        //between library elements are also text elements
        if (cur->type == XML_ELEMENT_NODE)
        {
            jfw::CXmlCharPtr sAttrVendor(xmlGetProp(cur, (xmlChar*) "vendor"));
            jfw::CXmlCharPtr sTextLibrary(
                xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
            PluginLibrary plugin;
            plugin.sVendor = rtl::OString((sal_Char*)(xmlChar*) sAttrVendor);
            //create the file URL to the library

            rtl::OUString sBase = getBaseInstallation();
            plugin.sPath = sBase +
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) + sTextLibrary;

            vecPlugins->push_back(plugin);
        }
        cur = cur->next;
    }
    return JFW_E_NONE;
}

/** Get the file URL to the javasettings.xml
 */
rtl::OUString getUserSettingsURL()
{
    //get the system path to the javasettings.xml file
    rtl::OUString sUserDir;
    rtl::Bootstrap::get(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UserInstallation")),
        sUserDir,
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("${$SYSBINDIR/"
             SAL_CONFIGFILE("bootstrap") ":UserInstallation}")));

    if (sUserDir.getLength() == 0)
        return rtl::OUString();
    rtl::OUStringBuffer sSettingsBuffer(512);
    sSettingsBuffer.append(sUserDir);
    sSettingsBuffer.appendAscii("/user/config/");
    sSettingsBuffer.appendAscii(JAVASETTINGS);
    sSettingsBuffer.append(getPlatform());
    sSettingsBuffer.appendAscii(".xml");
    return sSettingsBuffer.makeStringAndClear();
}

rtl::OString getUserSettingsPath()
{
    rtl::OUString sURL = getUserSettingsURL();
    rtl::OUString sSystemPathSettings;
    if (osl_getSystemPathFromFileURL(sURL.pData,
        & sSystemPathSettings.pData) != osl_File_E_None)
        return rtl::OString();

    rtl::OString osSystemPathSettings =
        rtl::OUStringToOString(sSystemPathSettings,osl_getThreadTextEncoding());
    return osSystemPathSettings;
}
rtl::OUString getSharedSettingsURL()
{
    rtl::OUString sBase = getBaseInstallation();
    if (sBase.getLength() == 0)
        return sBase;
    rtl::OUStringBuffer sBufSettings(256);
    sBufSettings.append(sBase);
    sBufSettings.appendAscii("/share/config/");
    sBufSettings.appendAscii(JAVASETTINGS);
    sBufSettings.append(getPlatform());
    sBufSettings.appendAscii(".xml");
    return sBufSettings.makeStringAndClear();
}

rtl::OUString getSharedSettingsURLNoPlatformSuffix()
{
    rtl::OUString sBase = getBaseInstallation();
    if (sBase.getLength() == 0)
        return sBase;
    rtl::OUStringBuffer sBufSettings(256);
    sBufSettings.append(sBase);
    sBufSettings.appendAscii("/share/config/");
    sBufSettings.appendAscii(JAVASETTINGS);
    sBufSettings.appendAscii(".xml");
    return sBufSettings.makeStringAndClear();
}

rtl::OString getSharedSettingsPath()
{
    rtl::OUString sURL = getSharedSettingsURL();
    rtl::OUString sSystemPathSettings;
    if (osl_getSystemPathFromFileURL(sURL.pData,
        & sSystemPathSettings.pData) != osl_File_E_None)
        return rtl::OString();

    rtl::OString osSystemPathSettings =
        rtl::OUStringToOString(sSystemPathSettings,osl_getThreadTextEncoding());
    return osSystemPathSettings;
}
rtl::OString getSharedSettingsPathNoPlatformSuffix()
{
    rtl::OUString sURL = getSharedSettingsURLNoPlatformSuffix();
    rtl::OUString sSystemPathSettings;
    if (osl_getSystemPathFromFileURL(sURL.pData,
        & sSystemPathSettings.pData) != osl_File_E_None)
        return rtl::OString();

    rtl::OString osSystemPathSettings =
        rtl::OUStringToOString(sSystemPathSettings,osl_getThreadTextEncoding());
    return osSystemPathSettings;
}

rtl::OUString getBaseInstallation()
{
    rtl::OUString sBaseDir;
    rtl::Bootstrap::get(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BaseInstallation")),
        sBaseDir,
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "${$SYSBINDIR/"SAL_CONFIGFILE("bootstrap") ":BaseInstallation}")));
    return sBaseDir;

}
rtl::OUString getVendorSettingsURL()
{
    //get the system path to the javavendors.xml file
    rtl::OUString sBaseDir = getBaseInstallation();
    if (sBaseDir.getLength() != 0)
    {
        //We are run within office installation
        rtl::OUStringBuffer sSettings(256);
        sSettings.append(sBaseDir);
        sSettings.appendAscii("/share/config/");
        sSettings.appendAscii(VENDORSETTINGS);
        return sSettings.makeStringAndClear();
    }
    else
    {
        //We are not in an office, try to find the javavendors.xml next to this
        //library
        rtl::OUString sLib;
        if (osl_getModuleURLFromAddress((void *) & getVendorSettingsURL,
                                        & sLib.pData) == sal_True)
        {
            rtl::OUStringBuffer sSettings(256);
            sSettings.append(sLib);
            sSettings.appendAscii("/");
            sLib = getDirFromFile(sLib);
            sSettings.appendAscii(VENDORSETTINGS);
            return sSettings.makeStringAndClear();
        }
        else
            return rtl::OUString();

    }
}

rtl::OString getVendorSettingsPath()
{
    rtl::OUString sURL = getVendorSettingsURL();
    rtl::OUString sSystemPathSettings;
    if (osl_getSystemPathFromFileURL(sURL.pData,
        & sSystemPathSettings.pData) != osl_File_E_None)
        return rtl::OString();

    rtl::OString osSystemPathSettings =
        rtl::OUStringToOString(sSystemPathSettings,osl_getThreadTextEncoding());
    return osSystemPathSettings;
}

bool isAccessibilitySupportDesired()
{
    bool retVal = false;
#ifdef WNT
    HKEY    hKey = 0;
    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                     "Software\\OpenOffice.org\\Accessibility\\AtToolSupport",
                     0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD   dwType = 0;
        DWORD   dwLen = 16;
        unsigned char arData[16];
        if( RegQueryValueEx(hKey, "SupportAssistiveTechnology", NULL, &dwType, arData,
                            & dwLen)== ERROR_SUCCESS)
        {
            if (dwType == REG_SZ)
            {
                if (strcmp((char*) arData, "true") == 0
                    || strcmp((char*) arData, "1") == 0)
                    retVal = true;
                else if (strcmp((char*) arData, "false") == 0
                         || strcmp((char*) arData, "0") == 0)
                    retVal = false;
#if OSL_DEBUG_LEVER > 1
                else
                    OSL_ASSERT(0);
#endif
            }
            else if (dwType == REG_DWORD)
            {
                if (arData[0] == 1)
                    retVal = true;
                else if (arData[0] == 0)
                    retVal = false;
#if OSL_DEBUG_LEVER > 1
                else
                    OSL_ASSERT(0);
#endif
            }
        }
    }
    RegCloseKey(hKey);

#elif UNX
    char buf[16];
    // use 2 shells to suppress the eventual "gcontool-2 not found" message
    // of the shell trying to execute the command
    FILE* fp = popen( "/bin/sh 2>/dev/null -c \"gconftool-2 -g /desktop/gnome/interface/accessibility\"", "r" );
    if( fp )
    {
        if( fgets( buf, sizeof(buf), fp ) )
        {
            int nCompare = strncasecmp( buf, "true", 4 );
            retVal = (nCompare == 0 ? true : false);
        }
        pclose( fp );
    }
#endif
    return retVal;
}


javaFrameworkError getVersionInformation(
    const xmlDocPtr doc,
    const xmlXPathContextPtr context,
    const rtl::OString & sVendor,
    VersionInfo *pVersionInfo)
{
    OSL_ASSERT(doc && context && sVendor.getLength() && pVersionInfo);

    //Get minVersion
    rtl::OString sExpresion= rtl::OString(
        "/jf:javaSelection/jf:vendorInfos/jf:vendor[@name=\"") +
        sVendor + rtl::OString("\"]/jf:minVersion");

    jfw::CXPathObjectPtr xPathObjectMin;
    xPathObjectMin =
        xmlXPathEvalExpression((xmlChar*) sExpresion.getStr(), context);
    if (xmlXPathNodeSetIsEmpty(xPathObjectMin->nodesetval))
    {
        pVersionInfo->sMinVersion = rtl::OUString();
    }
    else
    {
        jfw::CXmlCharPtr sVersion;
        sVersion = xmlNodeListGetString(
            doc, xPathObjectMin->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        rtl::OString osVersion((sal_Char*)(xmlChar*) sVersion);
        pVersionInfo->sMinVersion = rtl::OStringToOUString(
            osVersion, RTL_TEXTENCODING_UTF8);
    }

    //Get maxVersion
    sExpresion = rtl::OString("/jf:javaSelection/jf:vendorInfos/jf:vendor[@name=\"") +
        sVendor + rtl::OString("\"]/jf:maxVersion");
    jfw::CXPathObjectPtr xPathObjectMax;
    xPathObjectMax = xmlXPathEvalExpression(
        (xmlChar*) sExpresion.getStr(), context);
    if (xmlXPathNodeSetIsEmpty(xPathObjectMax->nodesetval))
    {
        pVersionInfo->sMaxVersion = rtl::OUString();
    }
    else
    {
        jfw::CXmlCharPtr sVersion;
        sVersion = xmlNodeListGetString(
            doc,xPathObjectMax->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        rtl::OString osVersion((sal_Char*) (xmlChar*) sVersion);
        pVersionInfo->sMaxVersion = rtl::OStringToOUString(
            osVersion, RTL_TEXTENCODING_UTF8);
    }

    //Get excludeVersions
    sExpresion = rtl::OString("/jf:javaSelection/jf:vendorInfos/jf:vendor[@name=\"") +
        sVendor + rtl::OString("\"]/jf:excludeVersions/jf:version");
    jfw::CXPathObjectPtr xPathObjectVersions;
    xPathObjectVersions =
        xmlXPathEvalExpression((xmlChar*) sExpresion.getStr(), context);
    if (xPathObjectVersions->nodesetval)
    {
        xmlNode* cur = xPathObjectVersions->nodesetval->nodeTab[0];
        while (cur != NULL)
        {
            if (cur->type == XML_ELEMENT_NODE )
            {
                if (xmlStrcmp(cur->name, (xmlChar*) "version") == 0)
                {
                    jfw::CXmlCharPtr sVersion;
                    sVersion = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
                    rtl::OString osVersion((sal_Char*)(xmlChar*) sVersion);
                    rtl::OUString usVersion = rtl::OStringToOUString(
                        osVersion, RTL_TEXTENCODING_UTF8);
                    pVersionInfo->addExcludeVersion(usVersion);
                }
            }
            cur = cur->next;
        }
    }
    return JFW_E_NONE;
}

rtl::ByteSequence encodeBase16(const rtl::ByteSequence& rawData)
{
    static char EncodingTable[] =
        {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    sal_Int32 lenRaw = rawData.getLength();
    char* pBuf = new char[lenRaw * 2];
    const sal_Int8* arRaw = rawData.getConstArray();

    char* pCurBuf = pBuf;
    for (int i = 0; i < lenRaw; i++)
    {
        char curChar = arRaw[i];
        curChar >>= 4;

        *pCurBuf = EncodingTable[curChar];
        pCurBuf++;

        curChar = arRaw[i];
        curChar &= 0x0F;

        *pCurBuf = EncodingTable[curChar];
        pCurBuf++;
    }

    rtl::ByteSequence ret((sal_Int8*) pBuf, lenRaw * 2);
    delete [] pBuf;
    return ret;
}

rtl::ByteSequence decodeBase16(const rtl::ByteSequence& data)
{
    static char decodingTable[] =
        {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    sal_Int32 lenData = data.getLength();
    sal_Int32 lenBuf = lenData / 2; //always divisable by two
    char* pBuf = new char[lenBuf];
    const sal_Int8* arData = data.getConstArray();

    char* pCurBuf = pBuf;
    const sal_Int8* pData = arData;
    for (int i = 0; i < lenBuf; i++)
    {
        sal_Int8 curChar = *pData;
        //find the index of the first 4bits
        char nibble;
        for (int ii = 0; ii < 16; ii++)
        {
            if (curChar == decodingTable[ii])
            {
                nibble = ii;
                break;
            }
        }
        nibble <<= 4;
        pData++;
        curChar = *pData;
        //find the index for the next 4bits
        for (int j = 0; j < 16; j++)
        {
            if (curChar == decodingTable[j])
            {
                nibble |= j;
                break;
            }
        }
        *pCurBuf = nibble;
        pData++;
        pCurBuf++;
    }
    rtl::ByteSequence ret((sal_Int8*) pBuf, lenBuf );
    delete [] pBuf;
    return ret;
}



javaFrameworkError buildClassPathFromDirectory(const rtl::OUString & relPath,
                             rtl::OUString & sClassPath)
{
    rtl::OUStringBuffer sBufRel(512);
    sBufRel.append(getBaseInstallation());
    sBufRel.appendAscii("/");
    sBufRel.append(relPath);
    rtl::OUString sClassesDir = sBufRel.makeStringAndClear();

    osl::Directory dir(sClassesDir);
    osl::FileBase::RC fileErrorCode;
    if ((fileErrorCode = dir.open()) != osl::FileBase::E_None)
    {
        return JFW_E_ERROR;
    }

    osl::DirectoryItem dirItem;
    rtl::OUStringBuffer sBuffer(2048);
    char szSep[] = {SAL_PATHSEPARATOR,0};

    //insert the path to the directory, so that .class files can be found
    rtl::OUString sDirPath;
    if ((fileErrorCode = osl::FileBase::getSystemPathFromFileURL(
             sClassesDir, sDirPath))
        != osl::FileBase::E_None)
    {
        return JFW_E_ERROR;
    }
    sBuffer.append(sDirPath);

    rtl::OUString sJarExtension(RTL_CONSTASCII_USTRINGPARAM(".jar"));
    sal_Int32 nJarExtensionLength = sJarExtension.getLength();
    for(;;)
    {
        fileErrorCode = dir.getNextItem(dirItem);
        if (fileErrorCode == osl::FileBase::E_None)
        {
            osl::FileStatus stat(FileStatusMask_All);
            if ((fileErrorCode = dirItem.getFileStatus(stat)) !=
                osl::FileBase::E_None)
            {
                return JFW_E_ERROR;
            }
            // check if the item is a file.
            switch (stat.getFileType())
            {
            case osl::FileStatus::Regular:
                break;
            case osl::FileStatus::Link:
            {
                rtl::OUString sLinkURL = stat.getLinkTargetURL();
                osl::DirectoryItem itemLink;
                if (osl::DirectoryItem::get(sLinkURL, itemLink)
                    != osl::FileBase::E_None)
                {
                   return JFW_E_ERROR;
                }
                osl::FileStatus statLink(FileStatusMask_All);
                if (statLink.getFileType() != osl::FileStatus::Regular)
                    continue;
                break;
            }
            default:
                continue;
            }

            //check if the file is a .jar, class files are ignored
            rtl::OUString sFileName = stat.getFileName();
            sal_Int32 len = sFileName.getLength();
            sal_Int32 nIndex = sFileName.lastIndexOf(sJarExtension);
            if ((nIndex == -1)
                || (nIndex + nJarExtensionLength != len))
                continue;

            rtl::OUString sFileURL = stat.getFileURL();
            rtl::OUString sFilePath;
            if ((fileErrorCode = osl::FileBase::getSystemPathFromFileURL(
                     sFileURL, sFilePath))
                != osl::FileBase::E_None)
            {
                return JFW_E_ERROR;
            }
            sBuffer.appendAscii(szSep);
            sBuffer.append(sFilePath);
        }
        else if (fileErrorCode == osl::FileBase::E_NOENT)
        {
            break;
        }
        else
        {
            return JFW_E_ERROR;
        }
    }
    sClassPath = sBuffer.makeStringAndClear();
    return JFW_E_NONE;
}

void setJavaSelected()
{
    g_bJavaSet = true;
}

bool wasJavaSelectedInSameProcess()
{
    //g_setJavaProcId not set means no Java selected
    if (g_bJavaSet == true)
        return true;
    return false;
}

rtl::OUString getDirFromFile(const rtl::OUString& usFilePath)
{
    sal_Int32 index= usFilePath.lastIndexOf('/');
    return rtl::OUString(usFilePath.getStr(), index);
}

}
