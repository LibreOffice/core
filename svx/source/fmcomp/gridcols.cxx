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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include "gridcols.hxx"
#include <tools/debug.hxx>
#include <comphelper/types.hxx>
#include "fmservs.hxx"
#include "svx/fmtools.hxx"
using namespace ::com::sun::star::uno;

//------------------------------------------------------------------------------
const ::comphelper::StringSequence& getColumnTypes()
{
    static ::comphelper::StringSequence aColumnTypes(10);
    if (!aColumnTypes.getConstArray()[0].getLength())
    {
        ::rtl::OUString* pNames = aColumnTypes.getArray();
        pNames[TYPE_CHECKBOX] = FM_COL_CHECKBOX;
        pNames[TYPE_COMBOBOX] = FM_COL_COMBOBOX;
        pNames[TYPE_CURRENCYFIELD] = FM_COL_CURRENCYFIELD;
        pNames[TYPE_DATEFIELD] = FM_COL_DATEFIELD;
        pNames[TYPE_FORMATTEDFIELD] = FM_COL_FORMATTEDFIELD;
        pNames[TYPE_LISTBOX] = FM_COL_LISTBOX;
        pNames[TYPE_NUMERICFIELD] = FM_COL_NUMERICFIELD;
        pNames[TYPE_PATTERNFIELD] = FM_COL_PATTERNFIELD;
        pNames[TYPE_TEXTFIELD] = FM_COL_TEXTFIELD;
        pNames[TYPE_TIMEFIELD] = FM_COL_TIMEFIELD;
    }
    return aColumnTypes;
}

//------------------------------------------------------------------
// Vergleichen von PropertyInfo
extern "C" int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC ) && defined( OS2 )
_Optlink
#endif
    NameCompare(const void* pFirst, const void* pSecond)
{
    return ((::rtl::OUString*)pFirst)->compareTo(*(::rtl::OUString*)pSecond);
}

namespace
{
    //------------------------------------------------------------------------------
    sal_Int32 lcl_findPos(const ::rtl::OUString& aStr, const Sequence< ::rtl::OUString>& rList)
    {
        const ::rtl::OUString* pStrList = rList.getConstArray();
        ::rtl::OUString* pResult = (::rtl::OUString*) bsearch(&aStr, (void*)pStrList, rList.getLength(), sizeof(::rtl::OUString),
            &NameCompare);

        if (pResult)
            return (pResult - pStrList);
        else
            return -1;
    }
}

//------------------------------------------------------------------------------
sal_Int32 getColumnTypeByModelName(const ::rtl::OUString& aModelName)
{
    const ::rtl::OUString aModelPrefix = ::rtl::OUString::createFromAscii("com.sun.star.form.component.");
    const ::rtl::OUString aCompatibleModelPrefix = ::rtl::OUString::createFromAscii("stardiv.one.form.component.");

    sal_Int32 nTypeId = -1;
    if (aModelName == FM_COMPONENT_EDIT)
        nTypeId = TYPE_TEXTFIELD;
    else
    {
        sal_Int32 nPrefixPos = aModelName.indexOf(aModelPrefix);
#ifdef DBG_UTIL
        sal_Int32 nCompatiblePrefixPos = aModelName.indexOf(aCompatibleModelPrefix);
        DBG_ASSERT( (nPrefixPos != -1) ||   (nCompatiblePrefixPos != -1), "::getColumnTypeByModelName() : wrong servivce !");
#endif

        ::rtl::OUString aColumnType = (nPrefixPos != -1)
            ? aModelName.copy(aModelPrefix.getLength())
            : aModelName.copy(aCompatibleModelPrefix.getLength());

        const ::comphelper::StringSequence& rColumnTypes = getColumnTypes();
        nTypeId = lcl_findPos(aColumnType, rColumnTypes);
    }
    return nTypeId;
}

