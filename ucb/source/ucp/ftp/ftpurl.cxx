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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include <memory>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <string.h>
#include <rtl/uri.hxx>

#include "ftpstrcont.hxx"
#include "ftpurl.hxx"
#include "ftphandleprovider.hxx"
#include "ftpcfunc.hxx"
#include "ftpcontainer.hxx"

using namespace ftp;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;

namespace {

rtl::OUString encodePathSegment(rtl::OUString const & decoded) {
    return rtl::Uri::encode(
        decoded, rtl_UriCharClassPchar, rtl_UriEncodeIgnoreEscapes,
        RTL_TEXTENCODING_UTF8);
}

rtl::OUString decodePathSegment(rtl::OUString const & encoded) {
    return rtl::Uri::decode(
        encoded, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
}

}

MemoryContainer::MemoryContainer()
    : m_nLen(0),
      m_nWritePos(0),
      m_pBuffer(0)
{
}

MemoryContainer::~MemoryContainer()
{
    rtl_freeMemory(m_pBuffer);
}


int MemoryContainer::append(
    const void* pBuffer,
    size_t size,
    size_t nmemb
) throw()
{
    sal_uInt32 nLen = size*nmemb;
    sal_uInt32 tmp(nLen + m_nWritePos);

    if(m_nLen < tmp) { // enlarge in steps of multiples of 1K
        do {
            m_nLen+=1024;
        } while(m_nLen < tmp);

        m_pBuffer = rtl_reallocateMemory(m_pBuffer,m_nLen);
    }

    memcpy(static_cast<sal_Int8*>(m_pBuffer)+m_nWritePos,
                   pBuffer,nLen);
    m_nWritePos = tmp;
    return nLen;
}


extern "C" {

    int memory_write(void *buffer,size_t size,size_t nmemb,void *stream)
    {
        MemoryContainer *_stream =
            reinterpret_cast<MemoryContainer*>(stream);

        if(!_stream)
            return 0;

        return _stream->append(buffer,size,nmemb);
    }

}


FTPURL::FTPURL(const FTPURL& r)
    : m_mutex(),
      m_pFCP(r.m_pFCP),
      m_aUsername(r.m_aUsername),
      m_bShowPassword(r.m_bShowPassword),
      m_aHost(r.m_aHost),
      m_aPort(r.m_aPort),
      m_aPathSegmentVec(r.m_aPathSegmentVec)

{
}


FTPURL::FTPURL(const rtl::OUString& url,
               FTPHandleProvider* pFCP)
    throw(
        malformed_exception
    )
    : m_pFCP(pFCP),
      m_aUsername("anonymous"),
      m_bShowPassword(false),
      m_aPort("21")
{
    parse(url);  // can reset m_bShowPassword
}


FTPURL::~FTPURL()
{
}


void FTPURL::parse(const rtl::OUString& url)
    throw(
        malformed_exception
    )
{
    rtl::OUString aPassword,aAccount;
    rtl::OString aIdent(url.getStr(),
                        url.getLength(),
                        RTL_TEXTENCODING_UTF8);

    rtl::OString lower = aIdent.toAsciiLowerCase();
    if(lower.getLength() < 6 ||
       strncmp("ftp://",lower.getStr(),6))
        throw malformed_exception();

    char *buffer = new char[1+aIdent.getLength()];
    const char* p2 = aIdent.getStr();
    p2 += 6;

    char ch;
    char *p1 = buffer;      // determine "username:password@host:port"
    while((ch = *p2++) != '/' && ch)
        *p1++ = ch;
    *p1 = 0;

    rtl::OUString aExpr(rtl::OUString(buffer,strlen(buffer),
                                      RTL_TEXTENCODING_UTF8));

    sal_Int32 l = aExpr.indexOf(sal_Unicode('@'));
    m_aHost = aExpr.copy(1+l);

    if(l != -1) {
        // Now username and password.
        aExpr = aExpr.copy(0,l);
        l = aExpr.indexOf(sal_Unicode(':'));
        if(l != -1) {
            aPassword = aExpr.copy(1+l);
            if(!aPassword.isEmpty())
                m_bShowPassword = true;
        }
        if(l > 0)
            // Overwritte only if the username is not empty.
            m_aUsername = aExpr.copy(0,l);
        else if(!aExpr.isEmpty())
            m_aUsername = aExpr;
    }

    l = m_aHost.lastIndexOf(sal_Unicode(':'));
    sal_Int32 ipv6Back = m_aHost.lastIndexOf(sal_Unicode(']'));
    if((ipv6Back == -1 && l != -1) // not ipv6, but a port
       ||
       (ipv6Back != -1 && 1+ipv6Back == l) // ipv6, and a port
    )
    {
        if(1+l<m_aHost.getLength())
            m_aPort = m_aHost.copy(1+l);
        m_aHost = m_aHost.copy(0,l);
    }

    while(ch) {  // now determine the pathsegments ...
        p1 = buffer;
        while((ch = *p2++) != '/' && ch)
            *p1++ = ch;
        *p1 = 0;

        if(buffer[0]) {
            if( strcmp(buffer,"..") == 0 && !m_aPathSegmentVec.empty() && m_aPathSegmentVec.back() != ".." )
                m_aPathSegmentVec.pop_back();
            else if(strcmp(buffer,".") == 0)
                ; // Ignore
            else
                // This is a legal name.
                m_aPathSegmentVec.push_back(
                    rtl::OUString(buffer,
                                  strlen(buffer),
                                  RTL_TEXTENCODING_UTF8));
        }
    }

    delete[] buffer;

    if(m_bShowPassword)
        m_pFCP->setHost(m_aHost,
                        m_aPort,
                        m_aUsername,
                        aPassword,
                        aAccount);

    // now check for something like ";type=i" at end of url
    if(m_aPathSegmentVec.size() &&
       (l = m_aPathSegmentVec.back().indexOf(sal_Unicode(';'))) != -1) {
        m_aType = m_aPathSegmentVec.back().copy(l);
        m_aPathSegmentVec.back() = m_aPathSegmentVec.back().copy(0,l);
    }
}


rtl::OUString FTPURL::ident(bool withslash,bool internal) const
{
    // rebuild the url as one without ellipses,
    // and more important, as one without username and
    // password. ( These are set together with the command. )

    rtl::OUStringBuffer bff;
    bff.appendAscii("ftp://");

    if( m_aUsername != "anonymous" ) {
        bff.append(m_aUsername);

        rtl::OUString aPassword,aAccount;
        m_pFCP->forHost(m_aHost,
                        m_aPort,
                        m_aUsername,
                        aPassword,
                        aAccount);

        if((m_bShowPassword || internal) &&
           !aPassword.isEmpty() )
            bff.append(sal_Unicode(':'))
                .append(aPassword);

        bff.append(sal_Unicode('@'));
    }
    bff.append(m_aHost);

    if( m_aPort != "21" )
        bff.append(sal_Unicode(':'))
            .append(m_aPort)
            .append(sal_Unicode('/'));
    else
        bff.append(sal_Unicode('/'));

    for(unsigned i = 0; i < m_aPathSegmentVec.size(); ++i)
        if(i == 0)
            bff.append(m_aPathSegmentVec[i]);
        else
            bff.append(sal_Unicode('/')).append(m_aPathSegmentVec[i]);
    if(withslash)
        if(bff.getLength() && bff[bff.getLength()-1] != sal_Unicode('/'))
            bff.append(sal_Unicode('/'));

    bff.append(m_aType);
    return bff.makeStringAndClear();
}


rtl::OUString FTPURL::parent(bool internal) const
{
    rtl::OUStringBuffer bff;

    bff.appendAscii("ftp://");

    if( m_aUsername != "anonymous" ) {
        bff.append(m_aUsername);

        rtl::OUString aPassword,aAccount;
        m_pFCP->forHost(m_aHost,
                        m_aPort,
                        m_aUsername,
                        aPassword,
                        aAccount);

        if((internal || m_bShowPassword) && !aPassword.isEmpty())
            bff.append(sal_Unicode(':'))
                .append(aPassword);

        bff.append(sal_Unicode('@'));
    }

    bff.append(m_aHost);

    if( m_aPort != "21" )
        bff.append(sal_Unicode(':'))
            .append(m_aPort)
            .append(sal_Unicode('/'));
    else
        bff.append(sal_Unicode('/'));

    rtl::OUString last;

    for(unsigned int i = 0; i < m_aPathSegmentVec.size(); ++i)
        if(1+i == m_aPathSegmentVec.size())
            last = m_aPathSegmentVec[i];
        else if(i == 0)
            bff.append(m_aPathSegmentVec[i]);
        else
            bff.append(sal_Unicode('/')).append(m_aPathSegmentVec[i]);

    if(last.isEmpty())
        bff.appendAscii("..");
    else if ( last == ".." )
        bff.append(last).appendAscii("/..");

    bff.append(m_aType);
    return bff.makeStringAndClear();
}


void FTPURL::child(const rtl::OUString& title)
{
    m_aPathSegmentVec.push_back(encodePathSegment(title));
}


rtl::OUString FTPURL::child() const
{
    return
        m_aPathSegmentVec.size() ?
        decodePathSegment(m_aPathSegmentVec.back()) : rtl::OUString();
}



/** Listing of a directory.
 */

namespace ftp {

    enum OS {
        FTP_DOS,FTP_UNIX,FTP_VMS,FTP_UNKNOWN
    };

}


#define SET_CONTROL_CONTAINER \
    MemoryContainer control;                                      \
    curl_easy_setopt(curl,                                        \
                     CURLOPT_HEADERFUNCTION,                      \
                     memory_write);                               \
    curl_easy_setopt(curl,                                        \
                     CURLOPT_WRITEHEADER,                         \
                     &control)


#define SET_DATA_CONTAINER                                        \
        curl_easy_setopt(curl,CURLOPT_NOBODY,false);              \
    MemoryContainer data;                                         \
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,memory_write);    \
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,&data)

#define SET_URL(url)                                              \
    rtl::OString urlParAscii(url.getStr(),                        \
                             url.getLength(),                     \
                             RTL_TEXTENCODING_UTF8);              \
    curl_easy_setopt(curl,                                        \
                     CURLOPT_URL,                                 \
                     urlParAscii.getStr());

FILE* FTPURL::open()
    throw(curl_exception)
{
    if(m_aPathSegmentVec.empty())
        throw curl_exception(CURLE_FTP_COULDNT_RETR_FILE);

    CURL *curl = m_pFCP->handle();

    SET_CONTROL_CONTAINER;
    rtl::OUString url(ident(false,true));
    SET_URL(url);
    FILE *res = tmpfile();
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,file_write);
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,res);

    curl_easy_setopt(curl,CURLOPT_POSTQUOTE,0);
    CURLcode err = curl_easy_perform(curl);

    if(err == CURLE_OK)
        rewind(res);
    else {
        fclose(res),res = 0;
        throw curl_exception(err);
    }

    return res;
}


std::vector<FTPDirentry> FTPURL::list(
    sal_Int16 nMode
) const
    throw(
        curl_exception
    )
{
    CURL *curl = m_pFCP->handle();

    SET_CONTROL_CONTAINER;
    SET_DATA_CONTAINER;
    rtl::OUString url(ident(true,true));
    SET_URL(url);
    curl_easy_setopt(curl,CURLOPT_POSTQUOTE,0);

    CURLcode err = curl_easy_perform(curl);
    if(err != CURLE_OK)
        throw curl_exception(err);

    // now evaluate the error messages

    sal_uInt32 len = data.m_nWritePos;
    char* fwd = (char*) data.m_pBuffer;
    rtl::OString str(fwd,len);
    char *p1, *p2;
    p1 = p2 = fwd;

    OS osKind(FTP_UNKNOWN);
    std::vector<FTPDirentry> resvec;
    FTPDirentry aDirEntry;
    // ensure slash at the end
    rtl::OUString viewurl(ident(true,false));

    while(true) {
        while(p2-fwd < int(len) && *p2 != '\n') ++p2;
        if(p2-fwd == int(len)) break;

        *p2 = 0;
        switch(osKind) {
            // While FTP knows the 'system'-command,
            // which returns the operating system type,
            // this is not usable here: There are Windows-server
            // formatting the output like UNIX-ls command.
        case FTP_DOS:
            FTPDirectoryParser::parseDOS(aDirEntry,p1);
            break;
        case FTP_UNIX:
            FTPDirectoryParser::parseUNIX(aDirEntry,p1);
            break;
        case FTP_VMS:
            FTPDirectoryParser::parseVMS(aDirEntry,p1);
            break;
        default:
            if(FTPDirectoryParser::parseUNIX(aDirEntry,p1))
                osKind = FTP_UNIX;
            else if(FTPDirectoryParser::parseDOS(aDirEntry,p1))
                osKind = FTP_DOS;
            else if(FTPDirectoryParser::parseVMS(aDirEntry,p1))
                osKind = FTP_VMS;
        }
        aDirEntry.m_aName = aDirEntry.m_aName.trim();
        if( osKind != int(FTP_UNKNOWN) && aDirEntry.m_aName != ".." && aDirEntry.m_aName != "." ) {
            aDirEntry.m_aURL = viewurl + encodePathSegment(aDirEntry.m_aName);

            sal_Bool isDir =
                sal_Bool(aDirEntry.m_nMode&INETCOREFTP_FILEMODE_ISDIR);
            switch(nMode) {
                case OpenMode::DOCUMENTS:
                    if(!isDir)
                        resvec.push_back(aDirEntry);
                    break;
                case OpenMode::FOLDERS:
                    if(isDir)
                        resvec.push_back(aDirEntry);
                    break;
                default:
                    resvec.push_back(aDirEntry);
            };
        }
        aDirEntry.clear();
        p1 = p2 + 1;
    }

    return resvec;
}


rtl::OUString FTPURL::net_title() const
    throw(curl_exception)
{
    CURL *curl = m_pFCP->handle();

    SET_CONTROL_CONTAINER;
    curl_easy_setopt(curl,CURLOPT_NOBODY,true);       // no data => no transfer
    struct curl_slist *slist = 0;
    // post request
    slist = curl_slist_append(slist,"PWD");
    curl_easy_setopt(curl,CURLOPT_POSTQUOTE,slist);

    bool try_more(true);
    CURLcode err;
    rtl::OUString aNetTitle;

    while(true) {
        rtl::OUString url(ident(false,true));

        if(try_more &&
           1+url.lastIndexOf(sal_Unicode('/')) != url.getLength())
            url += rtl::OUString("/");  // add end-slash
        else if(!try_more &&
                1+url.lastIndexOf(sal_Unicode('/')) == url.getLength())
            url = url.copy(0,url.getLength()-1);         // remove end-slash

        SET_URL(url);
        err = curl_easy_perform(curl);

        if(err == CURLE_OK) {       // get the title from the server
            char* fwd = (char*) control.m_pBuffer;
            sal_uInt32 len = (sal_uInt32) control.m_nWritePos;

            aNetTitle = rtl::OUString(fwd,len,RTL_TEXTENCODING_UTF8);
            // the buffer now contains the name of the file;
            // analyze the output:
            // Format of current working directory:
            // 257 "/bla/bla" is current directory
            sal_Int32 index1 = aNetTitle.lastIndexOf(
                rtl::OUString("257"));
            index1 = 1+aNetTitle.indexOf(sal_Unicode('"'),index1);
            sal_Int32 index2 = aNetTitle.indexOf(sal_Unicode('"'),index1);
            aNetTitle = aNetTitle.copy(index1,index2-index1);
            if( aNetTitle != "/" ) {
                index1 = aNetTitle.lastIndexOf(sal_Unicode('/'));
                aNetTitle = aNetTitle.copy(1+index1);
            }
            try_more = false;
        } else if(err == CURLE_BAD_PASSWORD_ENTERED)
            // the client should retry after getting the correct
            // username + password
            throw curl_exception(err);
#if LIBCURL_VERSION_NUM>=0x070d01 /* 7.13.1 */
        else if(err == CURLE_LOGIN_DENIED)
            // the client should retry after getting the correct
            // username + password
            throw curl_exception(err);
#endif
        else if(try_more && err == CURLE_FTP_ACCESS_DENIED) {
            // We  were  either denied access when trying to login to
            //  an FTP server or when trying to change working directory
            //  to the one given in the URL.
            if(!m_aPathSegmentVec.empty())
                // determine title form url
                aNetTitle = decodePathSegment(m_aPathSegmentVec.back());
            else
                // must be root
                aNetTitle = rtl::OUString("/");
            try_more = false;
        }

        if(try_more)
            try_more = false;
        else
            break;
    }

    curl_slist_free_all(slist);
    return aNetTitle;
}


FTPDirentry FTPURL::direntry() const
    throw(curl_exception)
{
    rtl::OUString nettitle = net_title();
    FTPDirentry aDirentry;

    aDirentry.m_aName = nettitle;                 // init aDirentry
    if( nettitle == "/" || nettitle == ".." )
        aDirentry.m_nMode = INETCOREFTP_FILEMODE_ISDIR;
    else
        aDirentry.m_nMode = INETCOREFTP_FILEMODE_UNKNOWN;

    aDirentry.m_nSize = 0;

    if( nettitle != "/" ) {
        // try to open the parent directory
        FTPURL aURL(parent(),m_pFCP);

        std::vector<FTPDirentry> aList = aURL.list(OpenMode::ALL);

        for(unsigned i = 0; i < aList.size(); ++i) {
            if(aList[i].m_aName == nettitle) { // the relevant file is found
                aDirentry = aList[i];
                break;
            }
        }
    }
    return aDirentry;
}


extern "C" {

    size_t memory_read(void *ptr,size_t size,size_t nmemb,void *stream)
    {
        sal_Int32 nRequested = sal_Int32(size*nmemb);
        CurlInput *curlInput = static_cast<CurlInput*>(stream);
        if(curlInput)
            return size_t(curlInput->read(((sal_Int8*)ptr),nRequested));
        else
            return 0;
    }

}


void FTPURL::insert(bool replaceExisting,void* stream) const
    throw(curl_exception)
{
    if(!replaceExisting) {
//          FTPDirentry aDirentry(direntry());
//          if(aDirentry.m_nMode == INETCOREFTP_FILEMODE_UNKNOWN)
        // throw curl_exception(FILE_EXIST_DURING_INSERT);
        throw curl_exception(FILE_MIGHT_EXIST_DURING_INSERT);
    } // else
    // overwrite is default in libcurl

    CURL *curl = m_pFCP->handle();

    SET_CONTROL_CONTAINER;
    curl_easy_setopt(curl,CURLOPT_NOBODY,false);    // no data => no transfer
    curl_easy_setopt(curl,CURLOPT_POSTQUOTE,0);
    curl_easy_setopt(curl,CURLOPT_QUOTE,0);
    curl_easy_setopt(curl,CURLOPT_READFUNCTION,memory_read);
    curl_easy_setopt(curl,CURLOPT_READDATA,stream);
    curl_easy_setopt(curl, CURLOPT_UPLOAD,1);

    rtl::OUString url(ident(false,true));
    SET_URL(url);

    CURLcode err = curl_easy_perform(curl);
    curl_easy_setopt(curl, CURLOPT_UPLOAD,false);

    if(err != CURLE_OK)
        throw curl_exception(err);
}



void FTPURL::mkdir(bool ReplaceExisting) const
    throw(curl_exception)
{
    rtl::OString title;
    if(!m_aPathSegmentVec.empty()) {
        rtl::OUString titleOU = m_aPathSegmentVec.back();
        titleOU = decodePathSegment(titleOU);
        title = rtl::OString(titleOU.getStr(),
                            titleOU.getLength(),
                            RTL_TEXTENCODING_UTF8);
    }
    else
        // will give an error
        title = rtl::OString("/");

    rtl::OString aDel("del "); aDel += title;
    rtl::OString mkd("mkd "); mkd += title;

    struct curl_slist *slist = 0;

    FTPDirentry aDirentry(direntry());
    if(!ReplaceExisting) {
//          if(aDirentry.m_nMode != INETCOREFTP_FILEMODE_UNKNOWN)
//              throw curl_exception(FOLDER_EXIST_DURING_INSERT);
        throw curl_exception(FOLDER_MIGHT_EXIST_DURING_INSERT);
    } else if(aDirentry.m_nMode != INETCOREFTP_FILEMODE_UNKNOWN)
        slist = curl_slist_append(slist,aDel.getStr());

    slist = curl_slist_append(slist,mkd.getStr());

    CURL *curl = m_pFCP->handle();
    SET_CONTROL_CONTAINER;
    curl_easy_setopt(curl,CURLOPT_NOBODY,true);       // no data => no transfer
    curl_easy_setopt(curl,CURLOPT_QUOTE,0);

    // post request
    curl_easy_setopt(curl,CURLOPT_POSTQUOTE,slist);

    rtl::OUString url(parent(true));
    if(1+url.lastIndexOf(sal_Unicode('/')) != url.getLength())
        url += rtl::OUString("/");
    SET_URL(url);

    CURLcode err = curl_easy_perform(curl);
    curl_slist_free_all(slist);
    if(err != CURLE_OK)
        throw curl_exception(err);
}


rtl::OUString FTPURL::ren(const rtl::OUString& NewTitle)
    throw(curl_exception)
{
    CURL *curl = m_pFCP->handle();

    // post request
    rtl::OString renamefrom("RNFR ");
    rtl::OUString OldTitle = net_title();
    renamefrom +=
        rtl::OString(OldTitle.getStr(),
                     OldTitle.getLength(),
                     RTL_TEXTENCODING_UTF8);

    rtl::OString renameto("RNTO ");
    renameto +=
        rtl::OString(NewTitle.getStr(),
                     NewTitle.getLength(),
                     RTL_TEXTENCODING_UTF8);

    struct curl_slist *slist = 0;
    slist = curl_slist_append(slist,renamefrom.getStr());
    slist = curl_slist_append(slist,renameto.getStr());
    curl_easy_setopt(curl,CURLOPT_POSTQUOTE,slist);

    SET_CONTROL_CONTAINER;
    curl_easy_setopt(curl,CURLOPT_NOBODY,true);       // no data => no transfer
    curl_easy_setopt(curl,CURLOPT_QUOTE,0);

    rtl::OUString url(parent(true));
    if(1+url.lastIndexOf(sal_Unicode('/')) != url.getLength())
        url += rtl::OUString("/");
    SET_URL(url);

    CURLcode err = curl_easy_perform(curl);
    curl_slist_free_all(slist);
    if(err != CURLE_OK)
        throw curl_exception(err);
    else if( m_aPathSegmentVec.size() && m_aPathSegmentVec.back() != ".." )
        m_aPathSegmentVec.back() = encodePathSegment(NewTitle);
    return OldTitle;
}



void FTPURL::del() const
    throw(curl_exception)
{
    FTPDirentry aDirentry(direntry());

    rtl::OString dele(aDirentry.m_aName.getStr(),
                      aDirentry.m_aName.getLength(),
                      RTL_TEXTENCODING_UTF8);

    if(aDirentry.m_nMode & INETCOREFTP_FILEMODE_ISDIR) {
        std::vector<FTPDirentry> vec = list(sal_Int16(OpenMode::ALL));
        for( unsigned int i = 0; i < vec.size(); ++i )
            try {
                FTPURL url(vec[i].m_aURL,m_pFCP);
                url.del();
            } catch(const curl_exception&) {
            }
        dele = rtl::OString("RMD ") + dele;
    }
    else if(aDirentry.m_nMode != INETCOREFTP_FILEMODE_UNKNOWN)
        dele = rtl::OString("DELE ") + dele;
    else
        return;

    // post request
    CURL *curl = m_pFCP->handle();
    struct curl_slist *slist = 0;
    slist = curl_slist_append(slist,dele.getStr());
    curl_easy_setopt(curl,CURLOPT_POSTQUOTE,slist);

    SET_CONTROL_CONTAINER;
    curl_easy_setopt(curl,CURLOPT_NOBODY,true);       // no data => no transfer
    curl_easy_setopt(curl,CURLOPT_QUOTE,0);

    rtl::OUString url(parent(true));
    if(1+url.lastIndexOf(sal_Unicode('/')) != url.getLength())
        url += rtl::OUString("/");
    SET_URL(url);

    CURLcode err = curl_easy_perform(curl);
    curl_slist_free_all(slist);
    if(err != CURLE_OK)
        throw curl_exception(err);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
