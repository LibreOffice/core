/*************************************************************************
 *
 *  $RCSfile: dbcharset.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: oj $ $Date: 2002-03-04 11:05:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBHELPER_DBCHARSET_HXX_
#include <connectivity/dbcharset.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//.........................................................................
namespace dbtools
{
//.........................................................................

    //-------------------------------------------------------------------------
    template <class T>
    sal_Int32 getSequenceIndex( const ::std::vector< T >& _rUnsortedContainer, const T& _rLookupElement )
    {
        for (   ::std::vector< T >::const_iterator aSearch = _rUnsortedContainer.begin();
                aSearch != _rUnsortedContainer.end();
                ++aSearch
            )
            if (*aSearch == _rLookupElement)
                return aSearch - _rUnsortedContainer.begin();

        return _rUnsortedContainer.size();
    }

    //=========================================================================
    //= OCharsetMap
    //=========================================================================
    //-------------------------------------------------------------------------
    OCharsetMap::OCharsetMap()
    #ifdef _DEBUG
        :m_nLivingIterators(0)
    #endif
    {
        m_aEncodings.resize(12);
        m_aEncodings[ 0] = RTL_TEXTENCODING_MS_1252;        // ANSI
        m_aEncodings[ 1] = RTL_TEXTENCODING_APPLE_ROMAN;    // MAC
        m_aEncodings[ 2] = RTL_TEXTENCODING_IBM_437;        // IBMPC_437
        m_aEncodings[ 3] = RTL_TEXTENCODING_IBM_850;        // IBMPC_850
        m_aEncodings[ 4] = RTL_TEXTENCODING_IBM_860;        // IBMPC_860
        m_aEncodings[ 5] = RTL_TEXTENCODING_IBM_861;        // IBMPC_861
        m_aEncodings[ 6] = RTL_TEXTENCODING_IBM_863;        // IBMPC_863
        m_aEncodings[ 7] = RTL_TEXTENCODING_IBM_865;        // IBMPC_865
        m_aEncodings[ 8] = RTL_TEXTENCODING_IBM_866;        // IBMPC_866 (cyrillic)
        m_aEncodings[ 9] = RTL_TEXTENCODING_DONTKNOW;       // SYSTEM
        m_aEncodings[10] = RTL_TEXTENCODING_UTF8;           // UTF-8
        m_aEncodings[11] = RTL_TEXTENCODING_BIG5_HKSCS;     // Big5-HKSCS

        m_aNames.resize(12);
        m_aNames[ 0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ANSI"));
        m_aNames[ 1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MAC"));
        m_aNames[ 2] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBMPC_437"));
        m_aNames[ 3] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBMPC_850"));
        m_aNames[ 4] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBMPC_860"));
        m_aNames[ 5] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBMPC_861"));
        m_aNames[ 6] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBMPC_863"));
        m_aNames[ 7] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBMPC_865"));
        m_aNames[ 8] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBMPC_866"));
        m_aNames[ 9] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SYSTEM"));
        m_aNames[10] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UTF-8"));
        m_aNames[10] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Big5-HKSCS"));

        OSL_ENSURE(m_aEncodings.size() == m_aNames.size(),
            "OCharsetMap::OCharsetMap: inconsistentce(1)!");

        // the IANA representations of the character sets which we know
        m_aIanaNames.resize(12);
        m_aIanaNames[ 0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("windows-1252"));
        m_aIanaNames[ 1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("macintosh"));
        m_aIanaNames[ 2] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBM437"));
        m_aIanaNames[ 3] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBM850"));
        m_aIanaNames[ 4] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBM860"));
        m_aIanaNames[ 5] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBM861"));
        m_aIanaNames[ 6] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBM863"));
        m_aIanaNames[ 7] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBM865"));
        m_aIanaNames[ 8] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBM866"));
        m_aIanaNames[ 9] = ::rtl::OUString();
        m_aIanaNames[10] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UTF-8"));
        m_aIanaNames[11] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Big5-HKSCS"));

        OSL_ENSURE(m_aNames.size() == m_aIanaNames.size(),
            "OCharsetMap::OCharsetMap: inconsistentce(2)!");
    }

    //-------------------------------------------------------------------------
    OCharsetMap::~OCharsetMap()
    {
        OSL_ENSURE(0 == m_nLivingIterators, "OCharsetMap::~OCharsetMap : there are still living iterator objects!");
    }

    //-------------------------------------------------------------------------
    OCharsetMap::CharsetIterator OCharsetMap::begin() const
    {
        return CharsetIterator(this, 0);
    }

    //-------------------------------------------------------------------------
    OCharsetMap::CharsetIterator    OCharsetMap::find(const rtl_TextEncoding _eEncoding) const
    {
        sal_Int32 nSize = m_aEncodings.size();
        TextEncVector::const_iterator aFind = ::std::find(m_aEncodings.begin(),m_aEncodings.end(),(sal_Int32)_eEncoding);
        if(aFind != m_aEncodings.end())
            nSize = aFind - m_aEncodings.begin();

        return CharsetIterator(this, nSize);
    }

    //-------------------------------------------------------------------------
    OCharsetMap::CharsetIterator    OCharsetMap::find(const ::rtl::OUString& _rIanaName, const IANA&) const
    {
        return CharsetIterator(this, getSequenceIndex(m_aIanaNames, _rIanaName));
    }

    //-------------------------------------------------------------------------
    OCharsetMap::CharsetIterator    OCharsetMap::find(const ::rtl::OUString& _rLogicalName, const Logical&) const
    {
        return CharsetIterator(this, getSequenceIndex(m_aNames, _rLogicalName));
    }

    //-------------------------------------------------------------------------
    OCharsetMap::CharsetIterator OCharsetMap::end() const
    {
        return CharsetIterator(this, m_aNames.size());
    }

    //=========================================================================
    //= CharsetIteratorDerefHelper
    //=========================================================================
    //-------------------------------------------------------------------------
    CharsetIteratorDerefHelper::CharsetIteratorDerefHelper(const CharsetIteratorDerefHelper& _rSource)
        :m_eEncoding(_rSource.m_eEncoding)
        ,m_aIanaName(_rSource.m_aIanaName)
        ,m_aName(_rSource.m_aName)
    {
    }

    //-------------------------------------------------------------------------
    CharsetIteratorDerefHelper::CharsetIteratorDerefHelper(const rtl_TextEncoding _eEncoding, const ::rtl::OUString& _rIanaName, const ::rtl::OUString& _rName)
        :m_eEncoding(_eEncoding)
        ,m_aIanaName(_rIanaName)
        ,m_aName(_rName)
    {
    }

    //-------------------------------------------------------------------------
    CharsetIteratorDerefHelper::CharsetIteratorDerefHelper()
        :m_eEncoding(RTL_TEXTENCODING_DONTKNOW)
    {
    }

    //=========================================================================
    //= OCharsetMap::CharsetIterator
    //=========================================================================
    //-------------------------------------------------------------------------
    OCharsetMap::CharsetIterator::CharsetIterator(const OCharsetMap* _pContainer, sal_Int32 _nInitialPos)
        :m_pContainer(_pContainer)
        ,m_nPosition(_nInitialPos)
    {
        OSL_ENSURE(m_pContainer, "OCharsetMap::CharsetIterator::CharsetIterator : invalid container!");
    #ifdef _DEBUG
        ++const_cast<OCharsetMap*>(m_pContainer)->m_nLivingIterators;
    #endif
    }

    //-------------------------------------------------------------------------
    OCharsetMap::CharsetIterator::CharsetIterator(const CharsetIterator& _rSource)
        :m_pContainer(_rSource.m_pContainer)
        ,m_nPosition(_rSource.m_nPosition)
    {
    #ifdef _DEBUG
        ++const_cast<OCharsetMap*>(m_pContainer)->m_nLivingIterators;
    #endif
    }

    //-------------------------------------------------------------------------
    OCharsetMap::CharsetIterator::~CharsetIterator()
    {
    #ifdef _DEBUG
        --const_cast<OCharsetMap*>(m_pContainer)->m_nLivingIterators;
    #endif
    }

    //-------------------------------------------------------------------------
    CharsetIteratorDerefHelper OCharsetMap::CharsetIterator::operator*() const
    {
        OSL_ENSURE((sal_uInt32)m_nPosition < m_pContainer->m_aEncodings.size(), "OCharsetMap::CharsetIterator::operator*: invalid position!");
        OSL_ENSURE((sal_uInt32)m_nPosition < m_pContainer->m_aIanaNames.size(), "OCharsetMap::CharsetIterator::operator*: invalid position!");
        OSL_ENSURE((sal_uInt32)m_nPosition < m_pContainer->m_aNames.size(), "OCharsetMap::CharsetIterator::operator*: invalid position!");
        return CharsetIteratorDerefHelper(
            m_pContainer->m_aEncodings[m_nPosition],
            m_pContainer->m_aIanaNames[m_nPosition],
            m_pContainer->m_aNames[m_nPosition]);
    }

    //-------------------------------------------------------------------------
    const OCharsetMap::CharsetIterator& OCharsetMap::CharsetIterator::operator++()
    {
        OSL_ENSURE((sal_uInt32)m_nPosition < m_pContainer->m_aNames.size(), "OCharsetMap::CharsetIterator::operator++ : invalid position!");
        if ((sal_uInt32)m_nPosition < m_pContainer->m_aNames.size())
            ++m_nPosition;
        return *this;
    }

    //-------------------------------------------------------------------------
    const OCharsetMap::CharsetIterator& OCharsetMap::CharsetIterator::operator--()
    {
        OSL_ENSURE(m_nPosition >= 0, "OCharsetMap::CharsetIterator::operator-- : invalid position!");
        if (m_nPosition >= 0)
            --m_nPosition;
        return *this;
    }

    //-------------------------------------------------------------------------
    sal_Int32 operator-(const OCharsetMap::CharsetIterator& lhs, const OCharsetMap::CharsetIterator& rhs)
    {
        return lhs.m_nPosition - rhs.m_nPosition;
    }

    //-------------------------------------------------------------------------
    bool operator==(const OCharsetMap::CharsetIterator& lhs, const OCharsetMap::CharsetIterator& rhs)
    {
        return (lhs.m_pContainer == rhs.m_pContainer) && (lhs.m_nPosition == rhs.m_nPosition);
    }

//.........................................................................
}   // namespace dbtools
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.6  2001/10/15 13:30:12  fs
 *  #93204# new supported charset IBM866 (cyrillic)
 *
 *  Revision 1.5  2001/05/18 08:51:34  oj
 *  #86528# size changes
 *
 *  Revision 1.4  2001/04/27 08:04:07  fs
 *  #86370# add UTF-8 to the list of supported charsets
 *
 *  Revision 1.3  2001/04/09 06:09:58  fs
 *  m_nLivingIterators for _DEBUG, not DBG_UTIL
 *
 *  Revision 1.2  2001/02/13 09:47:39  fs
 *  #83632# merge IBMPC, IBMPC(850), DOS
 *
 *  Revision 1.1  2000/11/29 22:21:42  fs
 *  initial checkin - helper class for translating charset representations
 *
 *
 *  Revision 1.0 29.11.00 18:42:55  fs
 ************************************************************************/

