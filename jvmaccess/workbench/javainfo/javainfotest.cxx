/*************************************************************************
 *
 *  $RCSfile: javainfotest.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 12:41:31 $
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

#include "jvmaccess/javainfo.hxx"

#include "osl/file.hxx"
#include "osl/thread.h"
#include "rtl/ustring.hxx"

#include <stdio.h>
#include <vector>

using namespace rtl;
using namespace std;
using namespace osl;

using jvmaccess::JavaInfo;

#define OUSTR( x )  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( x ))

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
//  arRet[i++]= test_createAllInfo();
//     arRet[i++]= test_constructor1();
//     arRet[i++]= test_constructor2();

//    arRet[i++]= test_compareVersions();
//  arRet[i++]= test_createBestInfo(false);
    //  arRet[i++]= test_isEqual();
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
    JavaInfo info(OUSTR("file:///d:/java/j2sdk1.4.0"));
//    JavaInfo info(OUSTR("file:///local/jl/java/j2sdk1.4.0"));
//    JavaInfo info(OUSTR("file:///usr/local2/jl/java/j2sdk1.4.0"));

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

//    JavaInfo a(OUSTR("file:///d:/java/j2sdk1.4.0"));
//    -----------------------------------------------------
    JavaInfo a(OUSTR("file:///usr/local2/jl/java/j2sdk1.4.0"));
//------------------------------------------------------------
//    JavaInfo a(OUSTR("file:///local/jl/java/j2sdk1.4.0"));
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
        JavaInfo info(OUString(OUSTR("1.4.0")), JavaInfo::EqualVersion);
        arRet[i++]= info.compareVersions(a) == 0;
    }
    catch( ...) {
        arRet[i++]= false;
    }
    try{
        JavaInfo info(OUString(OUSTR("1.4.0")), 0);
        arRet[i++]= info.compareVersions(a) == 0;
    }
    catch( ...) {
        arRet[i++]= false;
    }

    try{
        JavaInfo info(OUString(OUSTR("1.4.0")), JavaInfo::EqualVersion | JavaInfo::GreaterVersion);
        arRet[i++]= info.compareVersions(a) >= 0;
    }
    catch( ...) {
        arRet[i++]= false;
    }
    try{
        JavaInfo info(OUString(OUSTR("1.4.0")), JavaInfo::EqualVersion | JavaInfo::SmallerVersion);
        arRet[i++]= info.compareVersions(a) <= 0;
    }
    catch( ...) {
        arRet[i++]= false;
    }
    try{
        JavaInfo info(OUString(OUSTR("1.4.0")),  JavaInfo::SmallerVersion);
        arRet[i++]= info.compareVersions(a) < 0;
    }
    catch( ...) {
        arRet[i++]= false;
    }
    try{
        JavaInfo info(OUString(OUSTR("1.4.0")), JavaInfo::GreaterVersion);
        arRet[i++]= info.compareVersions(a) > 0;
    }
    catch( ...) {
        arRet[i++]= false;
    }

    try{
        JavaInfo info(OUString(OUSTR("1.4.0")), JavaInfo::EqualVersion | JavaInfo::SmallerVersion
             | JavaInfo::Accessibility);
        arRet[i++]= info.compareVersions(a) <= 0 && info.supportsAccessibility();
    }
    catch( ...) {
        arRet[i++]= false;
    }
    try{
        JavaInfo info(OUString(OUSTR("1.4.0")), JavaInfo::GreaterVersion | JavaInfo::Accessibility);
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

//     JavaInfo a(OUSTR("file:///d:/java/jdk1.3.1"));
//     JavaInfo b(OUSTR("file:///d:/java/jdk1.3.1_03"));
//     JavaInfo c(OUSTR("file:///d:/java/jdk1.3.1_04"));
//     JavaInfo d(OUSTR("file:///d:/java/j2sdk1.4.0"));
//     JavaInfo e(OUSTR("file:///d:/java/j2sdk1.4.0_01"));
//     JavaInfo f(OUSTR("file:///d:/java/j2sdk1.4.0_02"));
//     JavaInfo g(OUSTR("file:///d:/java/j2sdk1.4.1"));
//     JavaInfo h(OUSTR("file:///d:/java/j2sdk1.4.1_01"));

    JavaInfo a(OUSTR("file:///usr/local2/jl/java/j2re1_3_1_02"));
    JavaInfo b(OUSTR("file:///usr/local2/jl/java/j2sdk1.4.0"));
    JavaInfo c(OUSTR("file:///usr/local2/jl/java/j2sdk1.4.1"));
    JavaInfo d(OUSTR("file:///usr/local2/jl/java/j2re1.4.1_01"));

//     JavaInfo a(OUSTR("file:///local/jl/java/jre1.3.1"));
//     JavaInfo b(OUSTR("file:///local/jl/java/jdk1.3.1_04"));
//     JavaInfo c(OUSTR("file:///local/jl/java/j2sdk1.4.0"));
//     JavaInfo d(OUSTR("file:///local/jl/java/j2sdk1.4.1"));
//     JavaInfo e(OUSTR("file:///local/jl/java/j2re1.4.1_01"));
//     fprintf(stderr,"###ok1");

    if( (a.compareVersions(a) == 0
         && a.compareVersions(b) < 0
         && a.compareVersions(c) < 0
         && a.compareVersions(d) < 0
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
//     JavaInfo a(OUSTR("file:///d:/java/j2sdk1.4.0")); // accessible
//     JavaInfo b(OUSTR("file:///d:/java/j2sdk1.4.0"));

//     JavaInfo c(OUSTR("file:///c:/local/r/j2sdk1.4.0"));// not Accessible
//     JavaInfo d(OUSTR("file:///d:/java/copy_j2sdk1.4.0"));
//-------------------------------------------------------------------
    JavaInfo a(OUSTR("file:///usr/local2/jl/java/j2re1_3_1_02"));
    JavaInfo b(OUSTR("file:///usr/local2/jl/java/j2re1_3_1_02"));

    JavaInfo c(OUSTR("file:///usr/local2/jl/java/j2sdk1.4.1"));
    JavaInfo d(OUSTR("file:///usr/local2/jl/java/copyj2sdk1.4.1"));

//     JavaInfo a(OUSTR("file:///local/jl/java/jre1.3.1"));
//     JavaInfo b(OUSTR("file:///local/jl/java/jre1.3.1"));
//     JavaInfo c(OUSTR("file:///local/jl/java/j2sdk1.4.1"));
//     JavaInfo d(OUSTR("file:///local/jl/java/copyj2sdk1.4.1"));

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
    JavaInfo info(OUString(OUSTR("1.3.1")), JavaInfo::GreaterVersion | JavaInfo::EqualVersion );
    OUString s= info.getJarFilePath(OUSTR("javaplugin.jar"));

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
