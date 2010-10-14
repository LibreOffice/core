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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
