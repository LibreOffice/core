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



#ifndef _DBAUI_INDEXES_HXX_
#define _DBAUI_INDEXES_HXX_

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

//......................................................................
namespace dbaui
{
//......................................................................
    //==================================================================
    //= OIndexField
    //==================================================================
    struct OIndexField
    {
        String              sFieldName;
        sal_Bool            bSortAscending;

        OIndexField() : bSortAscending(sal_True) { }
    };

    DECLARE_STL_VECTOR( OIndexField, IndexFields );

    //==================================================================
    //= OIndex
    //==================================================================
    struct GrantIndexAccess
    {
        friend class OIndexCollection;
    private:
        GrantIndexAccess() { }
    };

    //..................................................................
    struct OIndex
    {
    protected:
        ::rtl::OUString     sOriginalName;
        sal_Bool            bModified;

    public:
        ::rtl::OUString     sName;
        ::rtl::OUString     sDescription;
        sal_Bool            bPrimaryKey;
        sal_Bool            bUnique;
        IndexFields         aFields;

    public:
        OIndex(const ::rtl::OUString& _rOriginalName)
            : sOriginalName(_rOriginalName), bModified(sal_False), sName(_rOriginalName), bPrimaryKey(sal_False), bUnique(sal_False)
        {
        }

        const ::rtl::OUString& getOriginalName() const { return sOriginalName; }

        sal_Bool    isModified() const { return bModified; }
        void        setModified(sal_Bool _bModified) { bModified = _bModified; }
        void        clearModified() { setModified(sal_False); }

        sal_Bool    isNew() const { return 0 == getOriginalName().getLength(); }
        void        flagAsNew(const GrantIndexAccess&) { sOriginalName = ::rtl::OUString(); }
        void        flagAsCommitted(const GrantIndexAccess&) { sOriginalName = sName; }


    private:
        OIndex();   // not implemented
    };

    DECLARE_STL_VECTOR( OIndex, Indexes );

//......................................................................
}   // namespace dbaui
//......................................................................

#endif // _DBAUI_INDEXES_HXX_

