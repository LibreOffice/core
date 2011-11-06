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
#include "precompiled_xmloff.hxx"
#include "attriblistmerge.hxx"

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml;

    //=====================================================================
    //= OAttribListMerger
    //=====================================================================
    //---------------------------------------------------------------------
    void OAttribListMerger::addList(const Reference< sax::XAttributeList >& _rxList)
    {
        OSL_ENSURE(_rxList.is(), "OAttribListMerger::addList: invalid list!");
        if (_rxList.is())
            m_aLists.push_back(_rxList);
    }

    //---------------------------------------------------------------------
    sal_Bool OAttribListMerger::seekToIndex(sal_Int16 _nGlobalIndex, Reference< sax::XAttributeList >& _rSubList, sal_Int16& _rLocalIndex)
    {
        sal_Int16 nLeftOver = _nGlobalIndex;
        ConstAttributeListArrayIterator aLookupSublist = m_aLists.begin();

        for ( ; (aLookupSublist != m_aLists.end()) && (nLeftOver >= (*aLookupSublist)->getLength());
                ++aLookupSublist
            )
            nLeftOver = nLeftOver - (*aLookupSublist)->getLength();

        if (aLookupSublist == m_aLists.end())
        {
            OSL_ENSURE(sal_False, "OAttribListMerger::seekToIndex: invalid index!");
            return sal_False;
        }
        _rSubList = *aLookupSublist;
        _rLocalIndex = nLeftOver;
        return sal_True;
    }

    //---------------------------------------------------------------------
    sal_Bool OAttribListMerger::seekToName(const ::rtl::OUString& _rName, Reference< sax::XAttributeList >& _rSubList, sal_Int16& _rLocalIndex)
    {
        for (   ConstAttributeListArrayIterator aLookupSublist = m_aLists.begin();
                aLookupSublist != m_aLists.end();
                ++aLookupSublist
            )
            for (sal_Int16 i=0; i<(*aLookupSublist)->getLength(); ++i)
                if ((*aLookupSublist)->getNameByIndex(i) == _rName)
                {
                    _rSubList = *aLookupSublist;
                    _rLocalIndex = i;
                    return sal_True;
                }

        OSL_ENSURE(sal_False, "OAttribListMerger::seekToName: did not find the name!");
        return sal_False;
    }

    //---------------------------------------------------------------------
    sal_Int16 SAL_CALL OAttribListMerger::getLength(  ) throw(RuntimeException)
    {
        sal_Int16 nCount = 0;
        for (   ConstAttributeListArrayIterator aAccumulate = m_aLists.begin();
                aAccumulate != m_aLists.end();
                ++aAccumulate
            )
            nCount = nCount + (*aAccumulate)->getLength();
        return nCount;
    }

    //---------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OAttribListMerger::getNameByIndex( sal_Int16 i ) throw(RuntimeException)
    {
        Reference< sax::XAttributeList > xSubList;
        sal_Int16 nLocalIndex;

        if (!seekToIndex(i, xSubList, nLocalIndex))
            return ::rtl::OUString();

        return xSubList->getNameByIndex(nLocalIndex);
    }

    //---------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OAttribListMerger::getTypeByIndex( sal_Int16 i ) throw(RuntimeException)
    {
        Reference< sax::XAttributeList > xSubList;
        sal_Int16 nLocalIndex;

        if (!seekToIndex(i, xSubList, nLocalIndex))
            return ::rtl::OUString();

        return xSubList->getTypeByIndex(nLocalIndex);
    }

    //---------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OAttribListMerger::getTypeByName( const ::rtl::OUString& _rName ) throw(RuntimeException)
    {
        Reference< sax::XAttributeList > xSubList;
        sal_Int16 nLocalIndex;

        if (!seekToName(_rName, xSubList, nLocalIndex))
            return ::rtl::OUString();

        // though we're in getTypeByName here, we reroute this to the getTypeByIndex of the sub list,
        // assuming that this is faster
        return xSubList->getTypeByIndex(nLocalIndex);
    }

    //---------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OAttribListMerger::getValueByIndex( sal_Int16 i ) throw(RuntimeException)
    {
        Reference< sax::XAttributeList > xSubList;
        sal_Int16 nLocalIndex;

        if (!seekToIndex(i, xSubList, nLocalIndex))
            return ::rtl::OUString();

        return xSubList->getValueByIndex(nLocalIndex);
    }

    //---------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OAttribListMerger::getValueByName( const ::rtl::OUString& _rName ) throw(RuntimeException)
    {
        Reference< sax::XAttributeList > xSubList;
        sal_Int16 nLocalIndex;

        if (!seekToName(_rName, xSubList, nLocalIndex))
            return ::rtl::OUString();

        // though we're in getValueByName here, we reroute this to the getValueByIndex of the sub list,
        // assuming that this is faster
        return xSubList->getValueByIndex(nLocalIndex);
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................

