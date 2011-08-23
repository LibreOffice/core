/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLOFF_FORMS_ATTRIBLISTMERGE_HXX_
#include "attriblistmerge.hxx"
#endif
namespace binfilter {

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
        ConstAttributeListArrayIterator aLookupSublist;

        for (	aLookupSublist = m_aLists.begin();
                (aLookupSublist != m_aLists.end()) && (nLeftOver >= (*aLookupSublist)->getLength());
                ++aLookupSublist
            )
            nLeftOver -= (*aLookupSublist)->getLength();

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
        for (	ConstAttributeListArrayIterator aLookupSublist = m_aLists.begin();
                aLookupSublist != m_aLists.end();
                ++aLookupSublist
            )
            for	(sal_Int16 i=0; i<(*aLookupSublist)->getLength(); ++i)
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
        for (	ConstAttributeListArrayIterator aAccumulate = m_aLists.begin();
                aAccumulate != m_aLists.end();
                ++aAccumulate
            )
            nCount += (*aAccumulate)->getLength();
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
}	// namespace xmloff
//.........................................................................

}//end of namespace binfilter
