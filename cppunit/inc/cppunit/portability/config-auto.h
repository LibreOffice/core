#ifndef _INCLUDE_CPPUNIT_CONFIG_AUTO_H
#define _INCLUDE_CPPUNIT_CONFIG_AUTO_H 1

/* include/cppunit/config-auto.h. Generated automatically at end of configure. */
/* config/config.h.  Generated automatically by configure.  */
/* config/config.h.in.  Generated automatically from configure.in by autoheader.  */

/* define if library uses std::string::compare(string,pos,n) */
#ifndef CPPUNIT_FUNC_STRING_COMPARE_STRING_FIRST
#define CPPUNIT_FUNC_STRING_COMPARE_STRING_FIRST  1
#endif

/* define if the library defines strstream */
#ifndef CPPUNIT_HAVE_CLASS_STRSTREAM
#define CPPUNIT_HAVE_CLASS_STRSTREAM  1
#endif

/* Define if you have the <cmath> header file. */
#ifndef CPPUNIT_HAVE_CMATH
#define CPPUNIT_HAVE_CMATH  1
#endif

/* Define if you have the <dlfcn.h> header file. */
#ifndef CPPUNIT_HAVE_DLFCN_H
#define CPPUNIT_HAVE_DLFCN_H  1
#endif

/* define to 1 if the compiler implements namespaces */
#ifndef CPPUNIT_HAVE_NAMESPACES
#define CPPUNIT_HAVE_NAMESPACES  1
#endif

/* define if the compiler supports Run-Time Type Identification */
#ifndef CPPUNIT_HAVE_RTTI
#define CPPUNIT_HAVE_RTTI  0
#endif

/* define if the compiler has stringstream */
/* #undef CPPUNIT_HAVE_SSTREAM */

/* Define if you have the <strstream> header file. */
#ifndef CPPUNIT_HAVE_STRSTREAM
#define CPPUNIT_HAVE_STRSTREAM  1
#endif

/* Name of package */
#ifndef CPPUNIT_PACKAGE
#define CPPUNIT_PACKAGE  "cppunit"
#endif

/* Define to 1 to use type_info::name() for class names */
#ifndef CPPUNIT_USE_TYPEINFO_NAME
#define CPPUNIT_USE_TYPEINFO_NAME  CPPUNIT_HAVE_RTTI
#endif

/* Version number of package */
#ifndef CPPUNIT_VERSION
#define CPPUNIT_VERSION  "1.8.0"
#endif

/* _INCLUDE_CPPUNIT_CONFIG_AUTO_H */
#endif
