/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: classpath.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-12 10:42:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_JVMACCESS_CLASSPATH_HXX
#define INCLUDED_JVMACCESS_CLASSPATH_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif

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

    // Functions that replace jobjectArray and jclass with void *, so that their
    // mangled C++ names do not depend on the JDK version used at compile time:

    static void * doTranslateToUrls(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext > const & context,
        ::JNIEnv * environment, ::rtl::OUString const & classPath);

    static void * doLoadClass(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext > const & context,
        ::JNIEnv * environment, ::rtl::OUString const & classPath,
        ::rtl::OUString const & name);
};

}

#endif
