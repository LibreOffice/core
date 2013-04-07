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

#include "jvmaccess/javainfo.hxx"

#include "osl/file.hxx"
#include "osl/thread.h"
#include "rtl/ustring.hxx"

#include <stdio.h>
#include <vector>

using namespace std;
using namespace osl;

using jvmaccess::JavaInfo;


#define JAVA_VERSION "1.4.1_01"

bool test_constructor1();
bool test_constructor2();
bool test_createAllInfo();
bool test_compareVersions();
bool test_createAllInfo();
bool test_createBestInfo();
bool test_isEqual();
bool test_getJarFilePath();

void printInfo(const JavaInfo& info);

int main( int argc, char * argv[], char * envp[])
{
    const int arSize= 20;
    bool arRet[arSize];
    int i =0;
    arRet[i++]= test_createAllInfo();
    arRet[i++]= test_constructor1();
    arRet[i++]= test_constructor2();

    arRet[i++]= test_compareVersions();
    arRet[i++]= test_createBestInfo();
    arRet[i++]= test_isEqual();
    arRet[i++]= test_getJarFilePath();
    bool error= true;
    for(int j= 0; j < i; j++)
        error &= arRet[j];

    if( error == false)
        printf("Errors occurred\n");
    return 0;
}

bool test_constructor1()
{
    printf("\ntest JavaInfo::JavaInfo(const OUString& usJavaHome\n" \
           "!Check output for correctness\n\n");
    try{
    JavaInfo info("file:///d:/java/j2sdk1.4.1_01");
//    JavaInfo info("file:///local/jl/java/j2sdk1.4.0");
//    JavaInfo info("file:///usr/local2/jl/java/j2sdk1.4.0");

    printInfo(info);
    }catch( JavaInfo::InitException& )
    {
        return false;
    }

    return true;
}

bool test_constructor2()
{
    printf("\ntest JavaInfo::JavaInfo(const OUString& usVersion,  int requirements)\n" \
           "!Check output for correctness\n\n");
    bool arRet[20];
    int i= 0;

    JavaInfo a("file:///d:/java/j2sdk1.4.1_01");
//    -----------------------------------------------------
//    JavaInfo a("file:///usr/local2/jl/java/j2sdk1.4.0");
//------------------------------------------------------------
//    JavaInfo a("file:///local/jl/java/j2sdk1.4.0");
    try{
        JavaInfo info= JavaInfo(OUString(), 0);
        arRet[i++]= true;
    }
    catch( ...) {
        arRet[i++]= false;
    }
    try{
        // make sure it supports accessibility
        JavaInfo info= JavaInfo(OUString(), JavaInfo::Accessibility);
        arRet[i++]= info.supportsAccessibility();
    }
    catch( ...) {
        arRet[i++]= false;
    }

    try{
        JavaInfo info(OUString(JAVA_VERSION), JavaInfo::EqualVersion);
        arRet[i++]= info.compareVersions(a) == 0;
    }
    catch( ...) {
        arRet[i++]= false;
    }
    try{
        JavaInfo info(OUString(JAVA_VERSION), 0);
        arRet[i++]= info.compareVersions(a) == 0;
    }
    catch( ...) {
        arRet[i++]= false;
    }

    try{
        JavaInfo info(OUString(JAVA_VERSION), JavaInfo::EqualVersion | JavaInfo::GreaterVersion);
        arRet[i++]= info.compareVersions(a) >= 0;
    }
    catch( ...) {
        arRet[i++]= false;
    }
    try{
        JavaInfo info(OUString(JAVA_VERSION), JavaInfo::EqualVersion | JavaInfo::SmallerVersion);
        arRet[i++]= info.compareVersions(a) <= 0;
    }
    catch( ...) {
        arRet[i++]= false;
    }
    try{
        JavaInfo info(OUString(JAVA_VERSION),  JavaInfo::SmallerVersion);
        arRet[i++]= info.compareVersions(a) < 0;
    }
    catch( ...) {
        arRet[i++]= false;
    }
    try{
        JavaInfo info(OUString(JAVA_VERSION), JavaInfo::GreaterVersion);
        arRet[i++]= info.compareVersions(a) > 0;
    }
    catch( ...) {
        arRet[i++]= false;
    }

    try{
        JavaInfo info(OUString(JAVA_VERSION), JavaInfo::EqualVersion | JavaInfo::SmallerVersion
             | JavaInfo::Accessibility);
        arRet[i++]= info.compareVersions(a) <= 0 && info.supportsAccessibility();
    }
    catch( ...) {
        arRet[i++]= false;
    }
    try{
        JavaInfo info(OUString(JAVA_VERSION), JavaInfo::GreaterVersion | JavaInfo::Accessibility);
        arRet[i++]= info.compareVersions(a) > 0 && info.supportsAccessibility();
    }
    catch( ...) {
        arRet[i++]= false;
    }


    bool err= true;
    for(int j= 0; j < i; j++)
        err &= arRet[j];
    if( err)
        printf("ok\n");
    else
        printf("failed\n");
    return err;

}

bool test_createAllInfo()
{
    printf("\ntest JavaInfo::createAllInfo\n" \
           "! Compare output with values in registry, PATH, JAVA_HOME, LD_LIBRARY_PATH !\n\n");
    vector<JavaInfo> vec;
    JavaInfo::createAllInfo(&vec);
    for(vector<JavaInfo>::size_type i= 0; i < vec.size(); i++)
        printInfo(vec[i]);
    return true;
}

bool test_compareVersions()
{
    bool ret= false;
    printf("\ntest JavaInfo::compareVersions \n" \
           "! Check output for correctness\n\n");


    JavaInfo a("file:///C:/Program%20Files/JavaSoft/JRE/1.3.1");
    JavaInfo b("file:///C:/Program%20Files/JavaSoft/JRE/1.3.1_04");
    JavaInfo c("file:///C:/Program%20Files/Java/j2re1.4.0_03");
    JavaInfo d("file:///C:/Program%20Files/Java/j2re1.4.2_04");
//     JavaInfo e("file:///d:/java/j2sdk1.4.0_01");
//     JavaInfo f("file:///d:/java/j2sdk1.4.0_02");
//     JavaInfo g("file:///d:/java/j2sdk1.4.1");
//     JavaInfo h("file:///d:/java/j2sdk1.4.1_01");

//     JavaInfo a("file:///usr/local2/jl/java/j2re1_3_1_02");
//     JavaInfo b("file:///usr/local2/jl/java/j2sdk1.4.0");
//     JavaInfo c("file:///usr/local2/jl/java/j2sdk1.4.1");
//     JavaInfo d("file:///usr/local2/jl/java/j2re1.4.1_01");

//     JavaInfo a("file:///local/jl/java/jre1.3.1");
//     JavaInfo b("file:///local/jl/java/jdk1.3.1_04");
//     JavaInfo c("file:///local/jl/java/j2sdk1.4.0");
//     JavaInfo d("file:///local/jl/java/j2sdk1.4.1");
//     JavaInfo e("file:///local/jl/java/j2re1.4.1_01");
//     fprintf(stderr,"###ok1");

    if( (a.compareVersions(a) == 0
         && a.compareVersions(b) < 0
         && a.compareVersions(c) < 0
//         && a.compareVersions(d) < 0
//         && a.compareVersions(e) < 0
//         && a.compareVersions(h) < 0
            )
        &&
        (a.compareVersions(a)== 0))

        ret= true;

    if(ret &&
     (b.compareVersions(a) > 0
         && b.compareVersions(b) == 0
         && b.compareVersions(c) < 0
         && b.compareVersions(d) < 0
//      && b.compareVersions(e) < 0
//         && b.compareVersions(h) < 0))
         ))
        ret= true;

    if(ret &&
       (   d.compareVersions(a) > 0
           && d.compareVersions(b) > 0
           && d.compareVersions(c) > 0
           && d.compareVersions(d) == 0
//           && d.compareVersions(e) < 0
//        && d.compareVersions(f) < 0
//        && d.compareVersions(g) < 0
//        && d.compareVersions(h) < 0))
           ))
        ret= true;

//     if(ret
//        && e.compareVersions(a) > 0
//        && e.compareVersions(b) > 0
//        && e.compareVersions(c) > 0
//        && e.compareVersions(d) > 0
//        && e.compareVersions(e) == 0
//         )


//     if(ret &&
//        (f.compareVersions(a) > 0
//         && f.compareVersions(c) > 0
//         && f.compareVersions(d) > 0
//         && f.compareVersions(g) < 0
//         && f.compareVersions(h) < 0))
//         ret= true;

    if( ! ret)
        printf("failed\n");
    else
        printf("ok\n");

    return ret;
}

bool test_createBestInfo()
{
    printf("\ntest JavaInfo::createBestInfo\n" \
           "! Check output for correctness\n\n");
    JavaInfo info= JavaInfo::createBestInfo(false);
    printInfo(info);
    return true;
}

bool test_isEqual()
{
    printf("\ntest JavaInfo::isEqual\n");
    JavaInfo a(JAVA_VERSION); // accessible
    JavaInfo b(JAVA_VERSION);

    JavaInfo c("file:///c:/local/r/j2sdk1.4.0");// not Accessible
    JavaInfo d("file:///d:/java/copy_j2sdk1.4.0");
//-------------------------------------------------------------------
//     JavaInfo a("file:///usr/local2/jl/java/j2re1_3_1_02");
//     JavaInfo b("file:///usr/local2/jl/java/j2re1_3_1_02");

//     JavaInfo c("file:///usr/local2/jl/java/j2sdk1.4.1");
//     JavaInfo d("file:///usr/local2/jl/java/copyj2sdk1.4.1");

//     JavaInfo a("file:///local/jl/java/jre1.3.1");
//     JavaInfo b("file:///local/jl/java/jre1.3.1");
//     JavaInfo c("file:///local/jl/java/j2sdk1.4.1");
//     JavaInfo d("file:///local/jl/java/copyj2sdk1.4.1");

    bool arRet[20];
    int i= 0;
    arRet[i++]= a.isEqual(b);
    arRet[i++]= ! a.isEqual(c); //must fail
    arRet[i++]= a.isEqual(d);

    bool err= true;
    for(int j= 0; j < i; j++)
        err &= arRet[j];
    if( arRet == false)
        printf("failed\n");
    else
        printf("ok\n");
    return err;
}

bool test_getJarFilePath()
{
    printf("\ntest JavaInfo::getJarFilePath\n");
    JavaInfo info(OUString("1.3.1"), JavaInfo::GreaterVersion | JavaInfo::EqualVersion );
    OUString s= info.getJarFilePath("javaplugin.jar");

    printf("check if this URL is correct:\n");
    OString oPath= OUStringToOString(s, osl_getThreadTextEncoding());
    printf("%s\n", oPath.getStr());
    return true;
}

void printInfo(const JavaInfo& info)
{
    OUString usVersion= info.getVersion();
    OString sVersion= OUStringToOString( usVersion, osl_getThreadTextEncoding());
    OUString usHome= info.getInstallationLocation();
    OString sHome= OUStringToOString( usHome, osl_getThreadTextEncoding());
    OUString usType= info.getType();
    OString sType= OUStringToOString(usType, osl_getThreadTextEncoding());
    OUString usLib= info.getRuntimeLibLocation();
    OString sLib= OUStringToOString(usLib, osl_getThreadTextEncoding());
    OUString usLibLocation= info.getLibLocations();
    OString sLibLocation= OUStringToOString(usLibLocation, osl_getThreadTextEncoding());
    sal_Bool baccess= info.supportsAccessibility();

    printf("%s %s\n",sType.getStr(), sVersion.getStr());
    printf("\t%s \n",sHome.getStr());
    printf("\t%s \n",sLib.getStr());
    printf("\tLibDir: %s \n", sLibLocation.getStr());
    printf("\t%s\n", baccess ? "accessible" : "not accessible");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
