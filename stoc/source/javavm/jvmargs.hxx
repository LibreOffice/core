/*************************************************************************
 *
 *  $RCSfile: jvmargs.hxx,v $
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

#ifndef __JVM
#define __JVM

#if STLPORT_VERSION < 321
#include <tools/presys.h>
#include <vector.h>
#include <tools/postsys.h>
#else
#include <cstdarg>
#include <stl/vector>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#include "jni.h"

using namespace ::rtl;

#ifndef JNI_VERSION_1_2

#define JNI_VERSION_1_1 0x00010001
#define JNI_VERSION_1_2 0x00010002

#define JNI_EDETACHED    (-2)              /* thread detached from the VM */
#define JNI_EVERSION     (-3)              /* JNI version error */
#define JNI_ENOMEM       (-4)              /* not enough memory */
#define JNI_EEXIST       (-5)              /* VM already created */
#define JNI_EINVAL       (-6)              /* invalid arguments */


struct JNIInvokeInterface12_;
struct JavaVM12_;

typedef JavaVM12_ JavaVM12;
#define JAVAVM JavaVM12

struct JNIInvokeInterface12_
{
    void *reserved0;
    void *reserved1;
    void *reserved2;

    jint (JNICALL *DestroyJavaVM)(JavaVM12 *vm);
    jint (JNICALL *AttachCurrentThread)(JavaVM12 *vm, void **penv, void *args);
    jint (JNICALL *DetachCurrentThread)(JavaVM12 *vm);
    jint (JNICALL *GetEnv)(JavaVM12 *vm, void **penv, jint version);
};

struct JavaVM12_
{
    const struct JNIInvokeInterface12_ *functions;

    jint DestroyJavaVM()
    {
        return functions->DestroyJavaVM(this);
    }

    jint AttachCurrentThread(void **penv, void *args)
    {
        return functions->AttachCurrentThread(this, penv, args);
    }

    jint DetachCurrentThread()
    {
        return functions->DetachCurrentThread(this);
    }

    jint GetEnv(void **penv, jint version)
    {
        return functions->GetEnv(this, penv, version);
    }
};

typedef struct JavaVMOption
{
    char *optionString;
    void *extraInfo;
} JavaVMOption;

typedef struct JavaVMInitArgs
{
    jint version;

    jint nOptions;
    JavaVMOption *options;
    jboolean ignoreUnrecognized;
} JavaVMInitArgs;

typedef struct JavaVMAttachArgs
{
    jint version;

    char *name;
    jobject group;
} JavaVMAttachArgs;

#else
#define JAVAVM JavaVM
#endif

typedef    jint (JNICALL *JNIvfprintf)(FILE *fp, const char *format, va_list args);
typedef    void (JNICALL *JNIexit)(jint code);
typedef    void (JNICALL *JNIabort)(void);

extern "C" {

#ifdef OS2
typedef jint JNICALL0 JNI_InitArgs_Type(void *);
typedef jint JNICALL0 JNI_CreateVM_Type(JAVAVM **, JNIEnv **, void *);
#else
typedef jint JNICALL JNI_InitArgs_Type(void *);
typedef jint JNICALL JNI_CreateVM_Type(JAVAVM **, JNIEnv **, void *);
#endif

}

class JVM
{
    ::std::vector<JavaVMOption> p_props;

    JavaVMInitArgs javaVMInitArgs;
    JDK1_1InitArgs  jDK1_1InitArgs;

    ::std::vector<OUString> props;

    sal_Bool debug;
    jint jiDebugPort;
    OUString usCompiler;

protected:
    void pushPProp(OUString uString, void * extraInfo = NULL);

public:
    JVM(JNI_InitArgs_Type * pVMInitArgs) ;
    ~JVM() ;

    void pushProp(const OUString & uString);

    void disableAsyncGC(jboolean jbFlag);
    void enableClassGC(jboolean jbFlag);
    void enableVerboseGC(jboolean jbFlag);
    void verbose(jboolean jbFlag);

    void setCompiler(const OUString & usCompiler);
    void nativeStackSize(jint jiSize);
    void javaStackSize(jint jiSize);
    void verifyMode(OUString uStr);
    void minHeapSize(jint jiSize);
    void maxHeapSize(jint jiSize);
    void setDebug(sal_Bool flag);
    void setDebugPort(jint jiDebugPort);
    void classPath(OString str);
    void vfprintf(JNIvfprintf vfprintf);
    void exit(JNIexit exit);
    void abort(JNIabort abort);

    sal_Bool getDebug();
    OUString getCompiler();

    const JavaVMInitArgs * getJavaVMInitArgs();

    const JDK1_1InitArgs * getJDK1_1InitArgs();
};

#endif
