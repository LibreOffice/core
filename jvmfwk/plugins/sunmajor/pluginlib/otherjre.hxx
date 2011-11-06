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



#if !defined INCLUDED_JFW_PLUGIN_OTHERJRE_HXX
#define  INCLUDED_JFW_PLUGIN_OTHERJRE_HXX

#include "vendorbase.hxx"
#include "vendorlist.hxx"

namespace jfw_plugin
{
/* Do not forget to put this class in the vendor map in vendorlist.cxx
 */
class OtherInfo: public VendorBase
{
public:
    static char const* const* getJavaExePaths(int * size);

    static rtl::Reference<VendorBase> createInstance();

    using VendorBase::getLibraryPaths;
    virtual char const* const* getRuntimePaths(int * size);
    virtual char const* const* getLibraryPaths(int* size);
    virtual int compareVersions(const rtl::OUString& sSecond) const;

};

}
#endif
