/*************************************************************************
 *
 *  $RCSfile: jvmargs.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:34 $
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

#include "jvmargs.hxx"


JVM::JVM(JNI_InitArgs_Type * pVMInitArgs)
    : debug(sal_False)
{
    /*
    ** The following is used to specify that we require at least
    ** JNI version 1.1. Currently, this field is not checked but
    ** will be starting with JDK/JRE 1.2. The value returned after
    ** calling JNI_GetDefaultJavaVMInitArgs() is the actual JNI version
    ** supported, and is always higher that the requested version.
    */


    jDK1_1InitArgs.version = JNI_VERSION_1_1;
    pVMInitArgs(&jDK1_1InitArgs);

    javaVMInitArgs.nOptions = 0;
    javaVMInitArgs.options = NULL;
    jiDebugPort = 1199;
}

JVM::~JVM()
{
    if(javaVMInitArgs.options)
    {
        for(size_t i = 0; i < javaVMInitArgs.nOptions; i++)
        {
            free(javaVMInitArgs.options[i].optionString);
        }

        free(javaVMInitArgs.options);
        javaVMInitArgs.nOptions = 0;
        javaVMInitArgs.options = NULL;
    }

    size_t i = 0;
    if(jDK1_1InitArgs.properties)
    {
        while(jDK1_1InitArgs.properties[i])
            free(jDK1_1InitArgs.properties[i ++]);

        free(jDK1_1InitArgs.properties);
    }
}

void JVM::pushPProp(OUString uString, void * extraInfo)
{
    JavaVMOption javaVMOption;

    javaVMOption.optionString = strdup(OUStringToOString(uString, RTL_TEXTENCODING_ASCII_US));
    javaVMOption.extraInfo = extraInfo;

    p_props.push_back(javaVMOption);
}

void JVM::pushProp(const OUString & uString)
{
    props.push_back(uString);
}

void JVM::disableAsyncGC(jboolean jbFlag)
{
    jDK1_1InitArgs.disableAsyncGC = jbFlag;
}

void JVM::enableClassGC(jboolean jbFlag)
{
    jDK1_1InitArgs.enableClassGC = jbFlag;
    if(!jbFlag)
    {
        OUString tmpStr( RTL_CONSTASCII_USTRINGPARAM("-Xnoclassgc") );
        pushPProp(tmpStr);
    }
}

void JVM::enableVerboseGC(jboolean jbFlag)
{
    jDK1_1InitArgs.enableVerboseGC = jbFlag;
    pushPProp( OUString(RTL_CONSTASCII_USTRINGPARAM("-verbose:gc")));
}

void JVM::verbose(jboolean jbFlag)
{
    jDK1_1InitArgs.verbose = jbFlag;
    pushPProp(OUString(RTL_CONSTASCII_USTRINGPARAM("-verbose:class")));
}

void JVM::nativeStackSize(jint jiSize)
{
    jDK1_1InitArgs.nativeStackSize = jiSize;
}

void JVM::javaStackSize(jint jiSize)
{
    jDK1_1InitArgs.javaStackSize = jiSize;
}

void JVM::verifyMode(OUString uStr)
{
    pushPProp( OUString(RTL_CONSTASCII_USTRINGPARAM("-Xverify:")) + uStr );

    if( uStr.compareToAscii("none") == 0 )
        jDK1_1InitArgs.verifyMode = 0;
    else if( uStr.compareToAscii("remote") == 0)
        jDK1_1InitArgs.verifyMode = 1;
    else if( uStr.compareToAscii("all") == 0 )
        jDK1_1InitArgs.verifyMode = 2;
}

void JVM::minHeapSize(jint jiSize)
{
    // Workaround ! Der neue Wert wird nur uebernommen, wenn dieser ueber dem
    // DefaultSize ( 1 MB ) liegt. Ein zu kleiner initialer HeapSize f’hrt unter Solaris Sparc zum Deadlock.
    if(jiSize > (sal_uInt32) jDK1_1InitArgs.minHeapSize)
        jDK1_1InitArgs.minHeapSize = jiSize;

    pushPProp( OUString(RTL_CONSTASCII_USTRINGPARAM("-Xms")) + OUString::valueOf((sal_Int32) jiSize) );
}

void JVM::maxHeapSize(jint jiSize)
{
    jDK1_1InitArgs.maxHeapSize = jiSize;
    pushPProp( OUString(RTL_CONSTASCII_USTRINGPARAM("-Xmx")) + OUString::valueOf((sal_Int32)jiSize) );
}

void JVM::setDebug(sal_Bool flag)
{
    fprintf(stderr, "#### JVM::setDebug: %d\n", flag);
    debug = flag;
}

sal_Bool JVM::getDebug()
{
    return debug;
}

void JVM::setDebugPort(jint jiPort)
{
    jiDebugPort = jiPort;
}

void JVM::setCompiler(const OUString & usCompiler)
{
    this->usCompiler = usCompiler;
}

OUString JVM::getCompiler()
{
    return usCompiler;
}

void JVM::classPath( OString str )
{
//    jDK1_1InitArgs.classpath = strdup(str.GetStr());
      jDK1_1InitArgs.classpath = strdup(str);
      pushPProp( OUString(RTL_CONSTASCII_USTRINGPARAM("java.class.path=")) + OStringToOUString(str, RTL_TEXTENCODING_ASCII_US ) );
}

void JVM::vfprintf(JNIvfprintf vfprintf)
{
    jDK1_1InitArgs.vfprintf = vfprintf;
    pushPProp( OUString(RTL_CONSTASCII_USTRINGPARAM("vfprintf")), (void *)vfprintf );
}

void JVM::exit(JNIexit exit)
{
    jDK1_1InitArgs.exit = exit;
    pushPProp( OUString(RTL_CONSTASCII_USTRINGPARAM("exit")), (void *)exit );
}

void JVM::abort(JNIabort abort)
{
    jDK1_1InitArgs.abort = abort;
    pushPProp( OUString(RTL_CONSTASCII_USTRINGPARAM("abort=")), (void *)abort );
}


const JDK1_1InitArgs * JVM::getJDK1_1InitArgs()
{
    if( usCompiler.getLength() )
        pushProp( OUString(RTL_CONSTASCII_USTRINGPARAM("java.compiler=")) + usCompiler);

    if(debug)
    {
        jDK1_1InitArgs.debugging = JNI_TRUE;
        jDK1_1InitArgs.debugPort = jiDebugPort;
        pushPProp( OUString(RTL_CONSTASCII_USTRINGPARAM("-Xdebug")), &jiDebugPort);
    }

    if(!jDK1_1InitArgs.properties)
    {
        size_t size = props.size() + 1;
        jDK1_1InitArgs.properties = (char **) calloc(sizeof(const char *), props.size() + 1);

        for(size_t i = 0; i < props.size(); i++)
        {
            OUString str = props[i];

            jDK1_1InitArgs.properties[i] = strdup( OUStringToOString(str, RTL_TEXTENCODING_ASCII_US) );
        }

        jDK1_1InitArgs.properties[props.size()] = NULL;
    }

    return &jDK1_1InitArgs;
}

const JavaVMInitArgs * JVM::getJavaVMInitArgs()
{
    if(usCompiler.getLength())
        pushProp( OUString(RTL_CONSTASCII_USTRINGPARAM("java.compiler=")) + usCompiler);

    javaVMInitArgs.version              = JNI_VERSION_1_2;
    javaVMInitArgs.nOptions             = props.size() + p_props.size();
    javaVMInitArgs.ignoreUnrecognized   = JNI_TRUE;

    javaVMInitArgs.options = (JavaVMOption *)calloc(sizeof(JavaVMOption), javaVMInitArgs.nOptions);

    size_t i;

    for(i = 0; i < p_props.size(); i++)
    {
        javaVMInitArgs.options[i] = p_props[i];
    }


    for(;i < javaVMInitArgs.nOptions; i++)
    {
        OUString str = props[i - p_props.size()];

        if( str.copy(0, 2).compareToAscii("-D") != 0  && str.copy(0, 2).compareToAscii("-X") != 0 )
            str = OUString(RTL_CONSTASCII_USTRINGPARAM("-D")) + str;

        javaVMInitArgs.options[i].optionString = strdup( OUStringToOString(str, RTL_TEXTENCODING_ASCII_US) );
        javaVMInitArgs.options[i].extraInfo = NULL;
    }

    return &javaVMInitArgs;
}

