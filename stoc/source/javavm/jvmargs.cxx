/*************************************************************************
 *
 *  $RCSfile: jvmargs.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kr $ $Date: 2000-09-28 17:35:21 $
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

#ifdef UNIX
#define CLASSPATH_DELIMETER ":"

#else
#define CLASSPATH_DELIMETER ";"

#endif


#include "jvmargs.hxx"

#include <osl/diagnose.h>

#include <rtl/ustring.hxx>

using namespace rtl;

namespace stoc_javavm {

    JVM::JVM() throw()
        : _is_debugPort(sal_False),
          _is_disableAsyncGC(sal_False),
          _is_enableClassGC(sal_False),
          _is_enableVerboseGC(sal_False),
          _is_checkSource(sal_False),
          _is_nativeStackSize(sal_False),
          _is_javaStackSize(sal_False),
          _is_minHeapSize(sal_False),
          _is_maxHeapSize(sal_False),
          _enabled(sal_False),
          _is_verifyMode(sal_False),
          _is_print(sal_False),
          _is_exit(sal_False),
          _is_abort(sal_False)
    {
    }

    void JVM::pushProp(const OUString & property) throw() {
        sal_Int32 index = property.indexOf((sal_Unicode)'=');
        OUString left = property.copy(0, index).trim();
        OUString right(property.copy(index + 1).trim());

//  #ifdef DEBUG
//      OString left_tmp = OUStringToOString(left, RTL_TEXTENCODING_ASCII_US);
//      OSL_TRACE("javavm - left %s", left_tmp.getStr());
//      OString right_tmp = OUStringToOString(right, RTL_TEXTENCODING_ASCII_US);
//      OSL_TRACE("javam - right %s", right_tmp.getStr());
//  #endif

        if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("Debug"))))
            setDebugPort(1199);

        else if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("Java"))))
            setEnabled(right.toInt32());

        else if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("Version"))))
            ;

        else if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("Home"))))
            ;

        else if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("SystemClasspath"))))
            setSystemClasspath(right);

        else if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("UserClasspath"))))
            setUserClasspath(right);

        else if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("DisableAsyncGC"))))
            setDisableAsyncGC(right.toInt32());

        else if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("EnableClassGC"))))
            setEnableClassGC(right.toInt32());

        else if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("EnableVerboseGC"))))
            setEnableVerboseGC(right.toInt32());

        else if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("NativeStackSize"))))
            setNativeStackSize(right.toInt32());

        else if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("JavaStackSize"))))
            setJavaStackSize(right.toInt32());

        else if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("VerifyMode"))))
            setVerifyMode(right);

        else if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("MinHeapSize"))))
            setMinHeapSize(right.toInt32());

        else if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("MaxHeapSize"))))
            setMaxHeapSize(right.toInt32());

        else if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("DebugPort"))))
            setDebugPort(right.toInt32());

        else if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("RuntimeLib"))))
            setRuntimeLib(right);

        else if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("CheckSource"))))
            setCheckSource(right.toInt32());

        else
            _props.push_back(property);
    }

    void JVM::setEnabled(sal_Bool sbFlag) throw() {
        _enabled = sbFlag;
    }

    void JVM::setDisableAsyncGC(jint jiValue) throw() {
        _is_disableAsyncGC = sal_True;
        _disableAsyncGC = jiValue;
    }

    void JVM::setEnableClassGC(jint jiValue) throw() {
        _is_enableClassGC = sal_True;
        _enableClassGC = jiValue;
    }

    void JVM::setEnableVerboseGC(jint jiValue) throw() {
        _is_enableVerboseGC = sal_True;
        _enableVerboseGC = jiValue;
    }

    void JVM::setCheckSource(jint jiValue) throw() {
        OSL_TRACE("JVM::setCheckSource: %i", jiValue);

        _is_checkSource = sal_True;
        _checkSource = jiValue;
    }

    void JVM::setNativeStackSize(jint jiValue) throw() {
        _is_nativeStackSize = sal_True;
        _nativeStackSize = jiValue;
    }

    void JVM::setJavaStackSize(jint jiValue) throw() {
        _is_javaStackSize = sal_True;
        _javaStackSize = jiValue;
    }

    void JVM::setVerifyMode(const OUString & mode) throw() {
        _is_verifyMode = sal_True;

        if(mode.compareToAscii("none") == 0)
            _verifyMode = 0;

        else if(mode.compareToAscii("remote") == 0)
            _verifyMode = 1;

        else if(mode.compareToAscii("all") == 0 )
            _verifyMode = 2;
    }

    void JVM::setMinHeapSize(jint jiValue) throw() {
        _is_minHeapSize = sal_True;
        _minHeapSize = jiValue;
    }

    void JVM::setMaxHeapSize(jint jiValue) throw() {
        _is_maxHeapSize = sal_True;
        _maxHeapSize = jiValue;
    }

    void JVM::setDebugPort(jint jiValue) throw() {
        _is_debugPort = sal_True;
        _debugPort = jiValue;
    }

    void JVM::setSystemClasspath(const OUString & classpath) throw() {
        OString tmp = OUStringToOString(classpath, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("JVM::setSystemClasspath: %s", tmp.getStr());

        _systemClasspath = classpath;
    }

    void JVM::setUserClasspath(const OUString & classpath) throw() {
        OString tmp = OUStringToOString(classpath, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("JVM::setUserClasspath: %s", tmp.getStr());

        _userClasspath = classpath;
    }

    void JVM::setPrint(JNIvfprintf vfprintf) throw() {
        _is_print = sal_True;
        _print = vfprintf;
    }

    void JVM::setExit(JNIexit exit) throw() {
        _is_exit = sal_True;
        _exit = exit;
    }

    void JVM::setAbort(JNIabort abort) throw() {
        _is_abort = sal_True;
        _abort = abort;
    }

    void JVM::setArgs(JDK1_1InitArgs * pargs) const throw() {
        OUString classpath = _systemClasspath;
        classpath += OUString(RTL_CONSTASCII_USTRINGPARAM(CLASSPATH_DELIMETER));
        classpath += _userClasspath;

        pargs->classpath = strdup(OUStringToOString(classpath, RTL_TEXTENCODING_ASCII_US));

        if(_is_debugPort) {
            pargs->debugging = JNI_TRUE;
            pargs->debugPort = _debugPort;
        }

        if(_is_disableAsyncGC)
            pargs->disableAsyncGC = _disableAsyncGC;

        if(_is_enableClassGC)
            pargs->enableClassGC = _enableClassGC;

        if(_is_enableVerboseGC)
            pargs->enableVerboseGC = _enableVerboseGC;

        if(_is_checkSource)
            pargs->checkSource = _checkSource;

        if(_is_nativeStackSize)
            pargs->nativeStackSize = _nativeStackSize;

        if(_is_javaStackSize)
            pargs->javaStackSize = _javaStackSize;

        if(_is_minHeapSize) {
            // Workaround ! Der neue Wert wird nur uebernommen, wenn dieser ueber der
            // DefaultSize ( 1 MB ) liegt. Ein zu kleiner initialer HeapSize fuehrt unter Solaris Sparc zum Deadlock.
            if(_minHeapSize > (sal_uInt32) pargs->minHeapSize)
                pargs->minHeapSize = _minHeapSize;
        }

        if(_is_maxHeapSize)
            pargs->maxHeapSize = _maxHeapSize;

        if(_is_verifyMode)
            pargs->verifyMode = _verifyMode;

        if(_is_print)
            pargs->vfprintf = _print;

        if(_is_exit)
            pargs->exit = _exit;

        if(_is_abort)
            pargs->abort = _abort;

        size_t size = _props.size();
        pargs->properties = (char **)calloc(sizeof(const char *), size + 1);

        for(size_t i = 0; i < size; ++ i) {
            const OUString & str = _props[i];

            pargs->properties[i] = strdup(OUStringToOString(str, RTL_TEXTENCODING_ASCII_US));
        }

        pargs->properties[size] = NULL;
    }

    void JVM::setRuntimeLib(const OUString & libName) throw() {
        _runtimeLib = libName;
    }

    const OUString & JVM::getRuntimeLib() const throw() {
        return _runtimeLib;
    }

    sal_Bool JVM::isEnabled() const throw() {
        return _enabled && _systemClasspath.getLength() && _runtimeLib.getLength();
    }
}
