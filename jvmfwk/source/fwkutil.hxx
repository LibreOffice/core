/*************************************************************************
 *
 *  $RCSfile: fwkutil.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2004-04-26 11:20:34 $
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
#if !defined INCLUDED_JVMFWK_FWKUTIL_HXX
#define INCLUDED_JVMFWK_FWKUTIL_HXX

#include "osl/mutex.hxx"
#include "osl/module.hxx"
#include "rtl/byteseq.hxx"
#include "libxml/parser.h"
#include "libxml/xpath.h"

namespace jfw
{
osl::Mutex * getFwkMutex();

rtl::ByteSequence encodeBase16(const rtl::ByteSequence& rawData);
rtl::ByteSequence decodeBase16(const rtl::ByteSequence& data);

/** Get the file URL to the javasettings.xml
 */
rtl::OUString getUserSettingsURL();
rtl::OString getUserSettingsPath();
rtl::OUString getSharedSettingsURL();
rtl::OString getSharedSettingsPath();

rtl::OUString getBaseInstallation();
rtl::OUString getVendorSettingsURL();

rtl::OString getVendorSettingsPath();

struct PluginLibrary;
class VersionInfo;
class CJavaInfo;

javaFrameworkError getVendorPluginURLs(
    const xmlDocPtr doc,
    const xmlXPathContextPtr  context,
    std::vector<PluginLibrary> * vecPlugins);
bool isAccessibilitySupportDesired();

javaFrameworkError getVersionInformation(
    const xmlDocPtr doc,
    const xmlXPathContextPtr context,
    const rtl::OString & sVendor,
    VersionInfo *pVersionInfo);


/** Gets the file URL to the plubin library for the currently selected Java.
 */
javaFrameworkError getPluginLibrary(rtl::OUString & sLibPathe);

/** Called from writeJavaInfoData. It sets the process identifier. When
java is to be started, then the current id is compared to the one set by
this function. If they are identical then the Java was selected in the
same process. If that Java needs a prepared environment, such as a
LD_LIBRARY_PATH, then it must not be started in this process.
*/
void setJavaSelected();

/** Determines if the currently selected Java was set in this process.

    @see setProcessId()
 */
bool wasJavaSelectedInSameProcess();
/**
    @param pDoc
       must not be freed within the function.
    @param pJavaInfoNode
       must not be freed within the function.
 */
javaFrameworkError writeElementJavaInfo(xmlDoc* pDoc,
                                        xmlNode* pJavaInfoNode,
                                        const jfw::CJavaInfo & aInfo);


javaFrameworkError buildClassPathFromDirectory(const rtl::OUString & relPath,
                                               rtl::OUString & sClassPath);

rtl::OUString retrieveClassPath( ::rtl::OUString const & macro );

// class CProcessId
// {
//     sal_uInt8 m_arId[16];
//     bool m_bValid;
// public:
//     CProcessId();
//     /**
//        If the argument is NULL or the object is invalid then
//        false is returned.
//      */
//     bool operator == (const sal_uInt8 * arId) const;
//     void set();
//     bool isValid() const;
// };

}
#endif
