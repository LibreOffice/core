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



#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_MANAGER_DP_ACTIVEPACKAGES_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_MANAGER_DP_ACTIVEPACKAGES_HXX

#include "sal/config.h"

#include <utility>
#include <vector>

#include "dp_persmap.h"

namespace rtl { class OUString; }

namespace dp_manager {

class ActivePackages {
public:
    struct Data {
        Data(): failedPrerequisites(::rtl::OUString::valueOf((sal_Int32)0))
            {}
        /* name of the temporary file (shared, user extension) or the name of
           the folder of the bundled extension.
           It does not contain the trailing '_' of the folder.
           UTF-8 encoded
        */
        ::rtl::OUString temporaryName;
        /* The file name (shared, user) or the folder name (bundled)
           If the key is the file name, then file name is not encoded.
           If the key is the idendifier then the file name is UTF-8 encoded.
         */
        ::rtl::OUString fileName;
        ::rtl::OUString mediaType;
        ::rtl::OUString version;
        /* If this string contains the value according to
           com::sun::star::deployment::Prerequisites or "0". That is, if
           the value is > 0 then
           the call to XPackage::checkPrerequisites failed.
           In this case the extension must not be registered.
         */
        ::rtl::OUString failedPrerequisites;
    };

    typedef ::std::vector< ::std::pair< ::rtl::OUString, Data > > Entries;

    ActivePackages();

    ActivePackages(::rtl::OUString const & url, bool readOnly);

    ~ActivePackages();

    bool has(::rtl::OUString const & id, ::rtl::OUString const & fileName)
        const;

    bool get(
        Data * data, ::rtl::OUString const & id,
        ::rtl::OUString const & fileName) const;

    Entries getEntries() const;

    void put(::rtl::OUString const & id, Data const & value);

    void erase(::rtl::OUString const & id, ::rtl::OUString const & fileName);

private:
    ActivePackages(ActivePackages &); // not defined
    void operator =(ActivePackages &); // not defined

    ::dp_misc::PersistentMap m_map;
};

}

#endif
