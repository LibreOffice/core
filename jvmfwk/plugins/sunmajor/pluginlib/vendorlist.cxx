/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vendorlist.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-12 15:31:32 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_jvmfwk.hxx"

#include "vendorlist.hxx"
#include "gnujre.hxx"
#include "sunjre.hxx"
#include "otherjre.hxx"
#include "osl/thread.h"
#include <stdio.h>

using namespace com::sun::star::uno;
using namespace rtl;

namespace jfw_plugin
{

/* Note: The vendor strings must be UTF-8. For example, if
   the string contains an a umlaut then it must be expressed
   by "\xXX\xXX"
 */
BEGIN_VENDOR_MAP()
    VENDOR_MAP_ENTRY("Sun Microsystems Inc.", SunInfo)
    VENDOR_MAP_ENTRY("IBM Corporation", OtherInfo)
    VENDOR_MAP_ENTRY("Blackdown Java-Linux Team", OtherInfo)
    VENDOR_MAP_ENTRY("Apple Inc.", OtherInfo)
    VENDOR_MAP_ENTRY("Apple Computer, Inc.", OtherInfo)
    VENDOR_MAP_ENTRY("BEA Systems, Inc.", OtherInfo)
    VENDOR_MAP_ENTRY("Free Software Foundation, Inc.", GnuInfo)
END_VENDOR_MAP()


Sequence<OUString> getVendorNames()
{
    const size_t count = sizeof(gVendorMap) / sizeof (VendorSupportMapEntry) - 1;
    OUString arNames[count];
    for ( size_t pos = 0; pos < count; ++pos )
    {
        OString sVendor(gVendorMap[pos].sVendorName);
        arNames[pos] = OStringToOUString(sVendor, RTL_TEXTENCODING_UTF8);
    }
    return Sequence<OUString>(arNames, count);
}

bool isVendorSupported(const rtl::OUString& sVendor)
{
    Sequence<OUString> seqNames = getVendorNames();
    const OUString * arNames = seqNames.getConstArray();
    sal_Int32 count = seqNames.getLength();

    for (int i = 0; i < count; i++)
    {
        if (sVendor.equals(arNames[i]))
            return true;
    }
#if OSL_DEBUG_LEVEL >= 2
    OString sVendorName = OUStringToOString(sVendor, osl_getThreadTextEncoding());
    fprintf(stderr, "[Java frameworksunjavaplugin.so]sunjavaplugin does not support vendor: %s.\n",
            sVendorName.getStr());
#endif
    return false;
}

}
