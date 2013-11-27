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

#ifndef _ACCRESOURCE_HXX
#define _ACCRESOURCE_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

class SimpleResMgr;

#define ACC_RES_STRING(id) ResourceManager::loadString(id)

//==================================================================
//= ResourceManager
//= handling ressources within the FormLayer library
//==================================================================
class ResourceManager
{
    static SimpleResMgr*    m_pImpl;

private:
    // no instantiation allowed
    ResourceManager()
    { }
    ~ResourceManager()
    { }

    // we'll instantiate one static member of the following class, which, in it's dtor,
    // ensures that m_pImpl will be deleted
    class EnsureDelete
    {
    public:
        EnsureDelete()
        { }
        ~EnsureDelete();
    };
    friend class EnsureDelete;

protected:
    static void ensureImplExists();

public:
    /** loads the string with the specified resource id from the FormLayer resource file
    */
    static ::rtl::OUString loadString(sal_uInt16 _nResId);
};


#endif
