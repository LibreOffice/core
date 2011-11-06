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
