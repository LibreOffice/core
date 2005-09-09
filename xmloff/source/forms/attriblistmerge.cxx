/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: attriblistmerge.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:02:54 $
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

#ifndef _XMLOFF_FORMS_ATTRIBLISTMERGE_HXX_
#include "attriblistmerge.hxx"
#endif

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
}   // namespace xmloff
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.2.404.1  2005/09/05 14:38:53  rt
 *  #i54170# Change license header: remove SISSL
 *
 *  Revision 1.2  2003/12/01 16:22:27  rt
 *  INTEGRATION: CWS ooo20031110 (1.1.204); FILE MERGED
 *  2003/11/11 11:03:31 waratah 1.1.204.1: #i22301# Correct for scoping rules
 *
 *  Revision 1.1.204.1  2003/11/11 11:03:31  waratah
 *  #i22301# Correct for scoping rules
 *
 *  Revision 1.1  2000/12/12 12:02:06  fs
 *  initial checkin - helper class for mergin XAttributeList instances
 *
 *
 *  Revision 1.0 12.12.00 10:32:56  fs
 ************************************************************************/

