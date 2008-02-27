#ifndef _INCLUDE_CPPUNIT_CONFIG_MSVC6_H
#define _INCLUDE_CPPUNIT_CONFIG_MSVC6_H 1

#define HAVE_CMATH 1

/* include/cppunit/config-msvc6.h. Manually adapted from
   include/cppunit/config-auto.h */

/* define if library uses std::string::compare(string,pos,n) */
#ifdef CPPUNIT_FUNC_STRING_COMPARE_STRING_FIRST
#undef CPPUNIT_FUNC_STRING_COMPARE_STRING_FIRST
#endif

/* Define if you have the <dlfcn.h> header file. */
#ifdef CPPUNIT_HAVE_DLFCN_H
#undef CPPUNIT_HAVE_DLFCN_H
#endif

/* define to 1 if the compiler implements namespaces */
#ifndef CPPUNIT_HAVE_NAMESPACES
#define CPPUNIT_HAVE_NAMESPACES  1
#endif

#ifndef CPPUNIT_HAVE_RTTI
#define CPPUNIT_HAVE_RTTI  0
#endif

#ifndef CPPUNIT_USE_TYPEINFO_NAME
#define CPPUNIT_USE_TYPEINFO_NAME  CPPUNIT_HAVE_RTTI
#endif

#define CPPUNIT_HAVE_SSTREAM 1

/* Name of package */
#ifndef CPPUNIT_PACKAGE
#define CPPUNIT_PACKAGE  "cppunit"
#endif

#undef CPPUNIT_API

// define CPPUNIT_DLL_BUILD when building CppUnit dll.
#ifdef CPPUNIT_BUILD_DLL
#define CPPUNIT_API __declspec(dllexport)
#endif

// define CPPUNIT_DLL when linking to CppUnit dll.
#ifdef CPPUNIT_DLL
#define CPPUNIT_API __declspec(dllimport)
#endif

#ifdef CPPUNIT_API
#undef CPPUNIT_NEED_DLL_DECL
#define CPPUNIT_NEED_DLL_DECL 1
#endif

#if _MSC_VER > 1000     // VC++
#pragma warning( disable : 4786 )   // disable warning debug symbol > 255...
#endif // _MSC_VER > 1000


/* _INCLUDE_CPPUNIT_CONFIG_MSVC6_H */
#endif
