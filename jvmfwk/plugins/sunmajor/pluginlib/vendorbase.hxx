/*************************************************************************
 *
 *  $RCSfile: vendorbase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 11:53:12 $
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

#if !defined INCLUDED_JFW_PLUGIN_VENDORBASE_HXX
#define INCLUDED_JFW_PLUGIN_VENDORBASE_HXX

#include "rtl/ustring.hxx"
#include "rtl/ref.hxx"
#include "salhelper/simplereferenceobject.hxx"
#include <vector>

namespace jfw_plugin
{


//Used by subclasses of VendorBase to build paths to Java runtime
#if defined UNX
#if defined SPARC
#define JFW_PLUGIN_ARCH "sparc"
#elif defined INTEL
#define JFW_PLUGIN_ARCH "i386"
#elif defined POWERPC
#define JFW_PLUGIN_ARCH "ppc"
#elif defined MIPS
#define JFW_PLUGIN_ARCH "mips"
#elif defined S390
#define JFW_PLUGIN_ARCH "s390"
#else // SPARC, INTEL, POWERPC, MIPS
#error unknown plattform
#endif // SPARC, INTEL, POWERPC, MIPS
#endif


class MalformedVersionException
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
    /* returns relativ paths to the java executable as
       file URLs.

       For example "bin/java.exe". You need
       to implement this function in a derived class, if
       the paths differ.  this implmentation provides for
       Windows "bin/java.exe" and for Unix "bin/java".
       The paths are relative file URLs. That is, they always
       contain '/' even on windows. The paths are relative
       to the installation directory of a JRE.


       The signature of this function must correspond to
       getJavaExePaths_func.
     */
    static char const* const *  getJavaExePaths(int* size);

    /* creates an instance of this class. MUST be overridden
       in a derived class.
       ####################################################
       OVERRIDE in derived class
       ###################################################
       @param
       Key - value pairs of the system properties of the JRE.
     */
    static rtl::Reference<VendorBase> createInstance();

    /* called automatically on the instance created by createInstance.

       @return
       true - the object could completely initialize.
       false - the object could not completly initialize. In this case
       it will be discarded by the caller.
    */
    virtual bool initialize(
        std::vector<std::pair<rtl::OUString, rtl::OUString> > props);

    /* returns relative file URLs to the runtime library.
       For example         "/bin/client/jvm.dll"
    */
    virtual char const* const* getRuntimePaths(int* size);

    virtual char const* const* getLibraryPaths(int* size);

    virtual const rtl::OUString & getVendor() const;
    virtual const rtl::OUString & getVersion() const;
    virtual const rtl::OUString & getHome() const;
    virtual const rtl::OUString & getRuntimeLibrary() const;
    virtual const rtl::OUString & getLibraryPaths() const;
    virtual bool supportsAccessibility() const;
    /* determines if prior to running java something has to be done,
       like setting the LD_LIBRARY_PATH. This implementation checks
       if an LD_LIBRARY_PATH (getLD_LIBRARY_PATH) needs to be set and
       if so, needsRestart returns true.
     */
    virtual bool needsRestart() const;

    /*
      @return
      0 this.version == sSecond
      1 this.version > sSecond
      -1 this.version < sSEcond

      @throw
      MalformedVersionException if the version string was not recognized.
     */
    virtual int compareVersions(const rtl::OUString& sSecond) const;

protected:

    rtl::OUString m_sVendor;
    rtl::OUString m_sVersion;
    rtl::OUString m_sHome;
    rtl::OUString m_sRuntimeLibrary;
    rtl::OUString m_sLD_LIBRARY_PATH;
    bool m_bAccessibility;


    typedef rtl::Reference<VendorBase> (* createInstance_func) ();
    friend rtl::Reference<VendorBase> createInstance(
        createInstance_func pFunc,
        std::vector<std::pair<rtl::OUString, rtl::OUString> > properties);
};

}

#endif
