/*************************************************************************
 *
 *  $RCSfile: javaldx.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 11:49:55 $
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "osl/thread.h"
#include "sal/types.h"
#include "rtl/ustring.hxx"
#include "rtl/byteseq.hxx"
#include "jvmfwk/framework.h"



using namespace rtl;

#define OUSTR(x) OUString(RTL_CONSTASCII_USTRINGPARAM( x ))

static sal_Bool hasOption(char* szOption, int argc, char** argv);
static rtl::OString getLD_LIBRARY_PATH(const rtl::ByteSequence & vendorData);
//static sal_Bool printPaths(const OUString& sPathFile);

#define HELP_TEXT    \
"\njavaldx is necessary to make Java work on some UNIX platforms." \
"It prints a string to std out that consists of directories which " \
"have to be included into the LD_LIBRARY_PATH variable.The setting of " \
"the variable usually occurs in a shell script that runs javaldx.\n" \
"The directories are from the chosen java installation. \n" \
"Options are: \n"\
"--help or -h\n"


int main(int argc, char **argv)
{
    if( hasOption("--help",argc, argv) || hasOption("-h", argc, argv))
    {
        fprintf(stdout, HELP_TEXT);// default
        return 0;
    }
    sal_Bool bEnabled = sal_False;
    if (jfw_getEnabled( & bEnabled) != JFW_E_NONE)
    {
        fprintf(stderr,"javaldx failed! \n");
        return -1;
    }

    if (bEnabled == sal_False)
    {
        //Do not do any preparation because that may only slow startup time.
        return 0;
    }

    JavaInfo * pInfo = NULL;
    javaFrameworkError errcode = JFW_E_NONE;
    errcode = jfw_getSelectedJRE( & pInfo);

    if (errcode == JFW_E_INVALID_SETTINGS)
    {
        fprintf(stderr,"javaldx failed. User must select a JRE from options dialog!");
        return -1;
    }
    else if (errcode != JFW_E_NONE)
    {
        fprintf(stderr,"javaldx failed! \n");
        return -1;
    }

    if (pInfo == NULL)
    {
        errcode = jfw_findAndSelectJRE( & pInfo);
        if (errcode == JFW_E_NO_JAVA_FOUND)
        {
            fprintf(stderr,"javaldx: Could not find a Java Runtime Environment! \n");
            return 0;
        }
        else if (errcode != JFW_E_NONE)
        {
            fprintf(stderr,"javaldx failed!\n");
            return -1;
        }
    }

    //Only do something if the sunjavaplugin created this JavaInfo
    rtl::OUString sVendor1(RTL_CONSTASCII_USTRINGPARAM("Sun Microsystems Inc."));
    rtl::OUString sVendor2(RTL_CONSTASCII_USTRINGPARAM("IBM Corporation"));
    rtl::OUString sVendor3(RTL_CONSTASCII_USTRINGPARAM("Blackdown Java-Linux Team"));
    rtl::OUString sVendor4(RTL_CONSTASCII_USTRINGPARAM("Apple Computer, Inc."));
    if ( ! (sVendor1.equals(pInfo->sVendor) == sal_True
            || sVendor2.equals(pInfo->sVendor) == sal_True
            || sVendor3.equals(pInfo->sVendor) == sal_True
            || sVendor4.equals(pInfo->sVendor) == sal_True))
        return 0;

    rtl::OString sPaths = getLD_LIBRARY_PATH(pInfo->arVendorData);
    fprintf(stdout, "%s\n", sPaths.getStr());
    jfw_freeJavaInfo(pInfo);

    return 0;
}

rtl::OString getLD_LIBRARY_PATH(const rtl::ByteSequence & vendorData)
{
    const sal_Unicode* chars = (sal_Unicode*) vendorData.getConstArray();
    sal_Int32 len = vendorData.getLength();
    rtl::OUString sData(chars, len / 2);
    //the runtime lib is on the first line
    sal_Int32 index = 0;
    rtl::OUString aToken = sData.getToken( 1, '\n', index);

    rtl::OString paths =
        rtl::OUStringToOString(aToken, osl_getThreadTextEncoding());
    return paths;
}

static sal_Bool hasOption(char* szOption, int argc, char** argv)
{
    sal_Bool retVal= sal_False;
    for(sal_Int16 i= 1; i < argc; i++)
    {
        if( ! strcmp(argv[i], szOption))
        {
            retVal= sal_True;
            break;
        }
    }
    return retVal;
}





