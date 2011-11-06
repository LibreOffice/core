/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef INCLUDED_JVMACCESS_CLASSPATH_HXX
#define INCLUDED_JVMACCESS_CLASSPATH_HXX

#include "sal/config.h"
#include "com/sun/star/uno/Reference.hxx"

#if defined SOLAR_JAVA
#include "jni.h"
#else
struct JNIEnv;
typedef void * jclass;
typedef void * jobjectArray;
#endif

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }
namespace rtl { class OUString; }

namespace jvmaccess {

/**
   Helper functions for class path handling.
*/
class ClassPath {
public:
    /**
       translates a class path into a java.net.URL[] instance.

       @param context
       a component context; must not be null.

       @param environment
       a JNI environment; must not be null.

       @param classPath
       a list of zero or more internal (see the
       com.sun.star.uri.ExternalUriReferenceTranslator service) URI references,
       where any space characters (U+0020) are ignored (and, in particular,
       separate adjacent URI references).  Any vnd.sun.star.expand URL
       references in the list are expanded using the
       com.sun.star.util.theMacroExpander singleton of the given context.

       @returns
       a local reference to a java.net.URL[] instance containing the external
       (see the com.sun.star.uri.ExternalUriReferenceTranslator service)
       equivalents of all the URI references in the given classPath.  If null, a
       (still pending) JNI exception occurred.

       @throws com::sun::star::uno::RuntimeException
    */
    static inline ::jobjectArray
    translateToUrls(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext > const & context,
        ::JNIEnv * environment, ::rtl::OUString const & classPath)
    {
        return
            static_cast< ::jobjectArray >(
                doTranslateToUrls(context, environment, classPath));
    }

    /**
       loads a class via a java.net.URLClassLoader.

       @param context
       a component context; must not be null.

       @param environment
       a JNI environment; must not be null.

       @param classPath
       a list of zero or more internal (see the
       com.sun.star.uri.ExternalUriReferenceTranslator service) URI references,
       where any space characters (U+0020) are ignored (and, in particular,
       separate adjacent URI references).  Any vnd.sun.star.expand URL
       references in the list are expanded using the
       com.sun.star.util.theMacroExpander singleton of the given context.

       @param name
       the Java binary name of the class to load.

       @returns
       a local reference to a java.lang.Class instance.  If null, a (still
       pending) JNI exception occurred.

       @throws com::sun::star::uno::RuntimeException
    */
    static inline ::jclass loadClass(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext > const & context,
        ::JNIEnv * environment, ::rtl::OUString const & classPath,
        ::rtl::OUString const & name)
    {
        return
            static_cast< ::jclass >(
                doLoadClass(context, environment, classPath, name));
    }

private:
    ClassPath(); // not defined
    ClassPath(ClassPath &); // not defined
    ~ClassPath(); // not defined
    void operator =(ClassPath &); // not defined

    // Functions that replace JNIEnv, jobjectArray, and jclass with void *, so
    // that their mangled C++ names do not depend on the JDK version used at
    // compile time:

    static void * doTranslateToUrls(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext > const & context,
        void * environment, ::rtl::OUString const & classPath);

    static void * doLoadClass(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext > const & context,
        void * environment, ::rtl::OUString const & classPath,
        ::rtl::OUString const & name);
};

}

#endif
