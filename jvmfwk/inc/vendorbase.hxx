/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_JVMFWK_PLUGINS_SUNMAJOR_PLUGINLIB_VENDORBASE_HXX
#define INCLUDED_JVMFWK_PLUGINS_SUNMAJOR_PLUGINLIB_VENDORBASE_HXX

#include "config_java.h"

#include "rtl/ustring.hxx"
#include "rtl/ref.hxx"
#include "osl/endian.h"
#include "salhelper/simplereferenceobject.hxx"
#include <vector>

namespace jfw_plugin
{


//Used by subclasses of VendorBase to build paths to Java runtime
#if defined(JAVA_ARCH)
#define JFW_PLUGIN_ARCH JAVA_ARCH
#elif defined(__sparcv9)
#define JFW_PLUGIN_ARCH "sparcv9"
#elif defined SPARC
#define JFW_PLUGIN_ARCH "sparc"
#elif defined X86_64
#define JFW_PLUGIN_ARCH "amd64"
#elif defined INTEL
#define JFW_PLUGIN_ARCH "i386"
#elif defined POWERPC64
#define JFW_PLUGIN_ARCH "ppc64"
#elif defined POWERPC
#define JFW_PLUGIN_ARCH "ppc"
#elif defined MIPS
#ifdef OSL_BIGENDIAN
#  define JFW_PLUGIN_ARCH "mips"
#else
/* FIXME: do JDKs have some JDK-specific define? This is for
OpenJDK at least, but probably not true for Lemotes JDK */
#  define JFW_PLUGIN_ARCH "mipsel"
#endif
#elif defined MIPS64
#ifdef OSL_BIGENDIAN
#  define JFW_PLUGIN_ARCH "mips64"
#else
#  define JFW_PLUGIN_ARCH "mips64el"
#endif
#elif defined S390X
#define JFW_PLUGIN_ARCH "s390x"
#elif defined S390
#define JFW_PLUGIN_ARCH "s390"
#elif defined ARM
#define JFW_PLUGIN_ARCH "arm"
#elif defined IA64
#define JFW_PLUGIN_ARCH "ia64"
#elif defined M68K
#define JFW_PLUGIN_ARCH "m68k"
#elif defined HPPA
#define JFW_PLUGIN_ARCH "parisc"
#elif defined AXP
#define JFW_PLUGIN_ARCH "alpha"
#elif defined AARCH64
#define JFW_PLUGIN_ARCH "aarch64"
#else // SPARC, INTEL, POWERPC, MIPS, MIPS64, ARM, IA64, M68K, HPPA, ALPHA
#error unknown platform
#endif // SPARC, INTEL, POWERPC, MIPS, MIPS64, ARM, IA64, M68K, HPPA, ALPHA


class MalformedVersionException : public std::exception
{
public:
    MalformedVersionException();

    MalformedVersionException(const MalformedVersionException &);

    virtual ~MalformedVersionException();

    MalformedVersionException & operator =(const MalformedVersionException &);
};

class VendorBase: public salhelper::SimpleReferenceObject
{
public:
    VendorBase();
    /* static char const* const *  getJavaExePaths(int* size);

       returns relative paths to the java executable as
       file URLs.

       For example "bin/java.exe". You need
       to implement this function in a derived class, if
       the paths differ.  this implementation provides for
       Windows "bin/java.exe" and for Unix "bin/java".
       The paths are relative file URLs. That is, they always
       contain '/' even on windows. The paths are relative
       to the installation directory of a JRE.


       The signature of this function must correspond to
       getJavaExePaths_func.
     */

    /* static rtl::Reference<VendorBase> createInstance();

       creates an instance of this class. MUST be overridden
       in a derived class.
       ####################################################
       OVERRIDE in derived class
       ###################################################
       @param
       Key - value pairs of the system properties of the JRE.
     */

    const OUString & getVendor() const;
    const OUString & getVersion() const;
    const OUString & getHome() const;
    const OUString & getRuntimeLibrary() const;
    const OUString & getLibraryPath() const;
    bool supportsAccessibility() const;
     /* determines if prior to running java something has to be done,
        like setting the LD_LIBRARY_PATH. This implementation checks
        if an LD_LIBRARY_PATH (getLD_LIBRARY_PATH) needs to be set and
        if so, needsRestart returns true.
      */
     bool needsRestart() const;

    /* compares versions of this vendor. MUST be overridden
       in a derived class.
       ####################################################
       OVERRIDE in derived class
       ###################################################
      @return
      0 this.version == sSecond
      1 this.version > sSecond
      -1 this.version < sSEcond

      @throw
      MalformedVersionException if the version string was not recognized.
     */
    virtual int compareVersions(const OUString& sSecond) const = 0;

protected:
    /* called automatically on the instance created by createInstance.

       @return
       true - the object could completely initialize.
       false - the object could not completely initialize. In this case
       it will be discarded by the caller.
    */
    virtual bool initialize(
        std::vector<std::pair<OUString, OUString> > props);

    /* returns relative file URLs to the runtime library.
       For example         "/bin/client/jvm.dll"
    */
    virtual char const* const* getRuntimePaths(int* size) = 0;

    virtual char const* const* getLibraryPaths(int* size) = 0;

    OUString m_sVendor;
    OUString m_sVersion;
    OUString m_sHome;
    OUString m_sRuntimeLibrary;
    OUString m_sLD_LIBRARY_PATH;
    bool m_bAccessibility;


    typedef rtl::Reference<VendorBase> (* createInstance_func) ();
    friend rtl::Reference<VendorBase> createInstance(
        createInstance_func pFunc,
        const std::vector<std::pair<OUString, OUString> >& properties);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
