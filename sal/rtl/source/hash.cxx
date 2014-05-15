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
#include "precompiled_sal.hxx"

#include "hash.h"
#include "strimp.h"

#include <hash_set>

namespace {

struct UStringHash
{
    size_t operator()(rtl_uString * const &rString) const
    { return (size_t)rtl_ustr_hashCode_WithLength( rString->buffer, rString->length ); }
};

struct UStringEqual
{
    sal_Bool operator() ( rtl_uString * const &pStringA,
                          rtl_uString * const &pStringB) const
    {
        if (pStringA == pStringB)
            return true;
        if (pStringA->length != pStringB->length)
            return false;
        return !rtl_ustr_compare_WithLength( pStringA->buffer, pStringA->length,
                                             pStringB->buffer, pStringB->length);
    }
};

typedef std::hash_set< rtl_uString *, UStringHash, UStringEqual > StringHashTable;

StringHashTable *
getHashTable ()
{
    static StringHashTable *pInternPool = NULL;
    if (pInternPool == NULL) {
        static StringHashTable aImpl(1024);
        pInternPool = &aImpl;
    }
    return pInternPool;
}

}

extern "C" {

rtl_uString *
rtl_str_hash_intern (rtl_uString       *pString,
                     int                can_return)
{
    StringHashTable *pHash = getHashTable();
    StringHashTable::iterator aIter;
    aIter = pHash->find(pString);
    if (aIter != pHash->end())
    {
        rtl_uString *pHashStr = *aIter;
        rtl_uString_acquire (pHashStr);
        return pHashStr;
    }
    if (!can_return)
    {
        rtl_uString *pCopy = NULL;
        rtl_uString_newFromString( &pCopy, pString );
        pString = pCopy;
        if (!pString)
            return NULL;
    }

    if (!SAL_STRING_IS_STATIC (pString))
        pString->refCount |= SAL_STRING_INTERN_FLAG;
    pHash->insert(pString);

    return pString;
}

void
rtl_str_hash_remove (rtl_uString       *pString)
{
    getHashTable()->erase(pString);
}

}
