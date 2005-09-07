/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vendorlist.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:33:34 $
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
#if !defined INCLUDED_JFW_PLUGIN_VENDORLIST_HXX
#define INCLUDED_JFW_PLUGIN_VENDORLIST_HXX

#include "rtl/ref.hxx"
#include "vendorbase.hxx"
#include "com/sun/star/uno/Sequence.hxx"

namespace jfw_plugin
{

//extern VendorSupportMapEntry gVendorMap[];

typedef char  const * const * (* getJavaExePaths_func)(int*);
typedef rtl::Reference<VendorBase> (* createInstance_func) ();

// struct Blas
// {
//         char const * sVendorName;
//     getJavaExePaths_func getJavaFunc;
//     createInstance_func  createFunc;
// };

struct VendorSupportMapEntry
{
    char const * sVendorName;
    getJavaExePaths_func getJavaFunc;
    createInstance_func  createFunc;
};

#define BEGIN_VENDOR_MAP() \
VendorSupportMapEntry gVendorMap[] ={

#define VENDOR_MAP_ENTRY(x,y) \
    {x, & y::getJavaExePaths, & y::createInstance},

#define END_VENDOR_MAP() \
    {NULL, NULL, NULL} };


com::sun::star::uno::Sequence<rtl::OUString> getVendorNames();

/* Examines if the vendor supplied in parameter sVendor is part of the
   list of supported vendors. That is the arry of VendorSupportMapEntry
   is search for an respective entry.
*/
bool isVendorSupported(const rtl::OUString & sVendor);
}

#endif
