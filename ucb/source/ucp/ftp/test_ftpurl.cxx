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


#include <com/sun/star/ucb/OpenMode.hpp>
#include "ftpurl.hxx"
#include "ftploaderthread.hxx"
#include "ftphandleprovider.hxx"
#include "debughelper.hxx"
#include <vector>

#define TESTEVAL \
    if(number_of_errors)   \
        fprintf(stderr,"errors in %s: %d\n",name,number_of_errors);  \
    return number_of_errors


struct ServerInfo {
    rtl::OUString host;
    rtl::OUString port;
    rtl::OUString username;
    rtl::OUString password;
    rtl::OUString account;
};


class FTPHandleProviderI
    : public ftp::FTPHandleProvider {

public:

    FTPHandleProviderI()
        : p(new ftp::FTPLoaderThread) {
    }

    ~FTPHandleProviderI() {
        delete p;
    }

    virtual CURL* handle() {
        return p->handle();
    }

    bool forHost(const rtl::OUString& host,
                 const rtl::OUString& port,
                 const rtl::OUString& username,
                 rtl::OUString& password,
                 rtl::OUString& account)
    {
        for(unsigned int i = 0; i < m_ServerInfo.size(); ++i)
            if(host == m_ServerInfo[i].host &&
               port == m_ServerInfo[i].port &&
               username == m_ServerInfo[i].username ) {
                password = m_ServerInfo[i].password;
                account = m_ServerInfo[i].account;
                return true;
            }

        return false;
    }

    virtual bool setHost(const rtl::OUString& host,
                         const rtl::OUString& port,
                         const rtl::OUString& username,
                         const rtl::OUString& password,
                         const rtl::OUString& account)
    {
        ServerInfo inf;
        inf.host = host;
        inf.port = port;
        inf.username = username;
        inf.password = password;
        inf.account = account;

        bool present(false);
        for(unsigned int i = 0; i < m_ServerInfo.size(); ++i)
            if(host == m_ServerInfo[i].host &&
               port == m_ServerInfo[i].port) {
                m_ServerInfo[i] = inf;
                present = true;
            }

        if(!present)
            m_ServerInfo.push_back(inf);

        return !present;

    }


private:

    std::vector<ServerInfo> m_ServerInfo;
    ftp::FTPLoaderThread *p;
};



// Here are some test for the parsing of an url.

#define TESTURL \
{  \
ftp::FTPURL url(rtl::OUString::createFromAscii(ascii),&prov); \
if(!url.username().equalsAscii(n)) {\
++number_of_errors;   \
err_msg("wrong username: ",url.username());  \
}}

int test_ftpurl(void) {
    const char* name = "test_ftpurl";
    int number_of_errors = 0;

    FTPHandleProviderI prov;
    char* ascii,*n,*p;

    ascii = "ftp://abi:psswd@host/eins/../drei", n = "abi", p = "psswd";
    TESTURL;

    ascii = "ftp://:psswd@host:22/eins/../drei", n = "anonymous", p = "psswd";
    TESTURL;

    ascii = "ftp://host/bla/../../test/", n = "anonymous", p = "";
    TESTURL;

    TESTEVAL;
}


int test_ftplist(void) {
    int number_of_errors = 0;
    const char* name = "test_ftplist";

    FTPHandleProviderI provider;

    ftp::FTPURL url(
        rtl::OUString( "ftp://abi:psswd@abi-1/dir"),
        &provider);

    std::vector<ftp::FTPDirentry> vec =
        url.list(com::sun::star::ucb::OpenMode::ALL);

    if(vec.size() != 3)
        ++number_of_errors;

    if(!(vec[0].m_aName == "dir1" && vec[1].m_aName == "dir2" && vec[2].m_aName == "file1" ))
       ++number_of_errors;

    TESTEVAL;
}


#define TESTPARENT   \
    {   \
        ftp::FTPURL url(rtl::OUString::createFromAscii(ascii),&prov);  \
        urlStr = url.parent();          \
        if(!urlStr.equalsAscii(expect)) \
            ++number_of_errors;  \
    }


int test_ftpparent(void) {
    int number_of_errors = 0;
    const char* name = "test_ftpparent";
    FTPHandleProviderI prov;

    rtl::OUString urlStr;
    char *ascii,*expect;

    ascii = "ftp://abi:psswd@abi-1/file";
    expect = "ftp://abi:psswd@abi-1/";
    TESTPARENT;

    ascii = "ftp://abi:psswd@abi-1/dir/../file";
    expect = "ftp://abi:psswd@abi-1/";
    TESTPARENT;

    ascii = "ftp://abi:psswd@abi-1/..";
    expect = "ftp://abi:psswd@abi-1/../..";
    TESTPARENT;

    ascii = "ftp://abi:psswd@abi-1/../../dir";
    expect = "ftp://abi:psswd@abi-1/../..";
    TESTPARENT;

    ascii = "ftp://abi:psswd@abi-1/";
    expect = "ftp://abi:psswd@abi-1/..";
    TESTPARENT;

    TESTEVAL;
}


int test_ftpproperties(void) {
    int number_of_errors = 0;
    const char* name = "test_ftpproperties";
    FTPHandleProviderI provider;

    ftp::FTPURL url(
        rtl::OUString( "ftp://abi:psswd@abi-1/file"),
        &provider);

    ftp::FTPDirentry ade(url.direntry());

    if(!(ade.m_aName == "file" && ade.isFile()))
        ++number_of_errors;

    TESTEVAL;
}


int test_ftpopen(void)
{
    int number_of_errors = 0;
    const char* name = "test_ftpopen";

    FTPHandleProviderI provider;
    ftp::FTPURL url(
        rtl::OUString( "ftp://abi:psswd@abi-1/file"),
        &provider);

    FILE* file = url.open();
    if(file) {
        int nbuf,ndest;
        const int bffsz = 256;
        char buff[bffsz];
        char *dest = (char*) malloc(sizeof(char));
        dest[0] = 0;
        do {
            memset((void*)buff, 0, bffsz);
            fread(buff,bffsz-1,1,file);
            nbuf = strlen(buff);
            ndest = strlen(dest);
            dest = (char*)realloc(dest,ndest + nbuf + 1);
            strncat(dest,buff,nbuf);
        } while(nbuf == bffsz-1);
        fclose(file);

        const char* expected = "You are now looking at the filecontent.\n";
        if(strcmp(expected,dest))
            ++number_of_errors;
        free(dest);
    } else
        ++number_of_errors;

    TESTEVAL;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
