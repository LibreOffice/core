/*************************************************************************
 *
 *  $RCSfile: dbcharset.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2001-04-09 06:09:58 $
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
    template <class TYPE>
    sal_Int32 getSequenceIndex( const ::std::vector< TYPE >& _rUnsortedContainer, const TYPE& _rLookupElement )
    {
        for (   ::std::vector< TYPE >::const_iterator aSearch = _rUnsortedContainer.begin();
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
        m_aEncodings.resize(9);
        m_aEncodings[0] = RTL_TEXTENCODING_MS_1252;     // ANSI
        m_aEncodings[1] = RTL_TEXTENCODING_APPLE_ROMAN; // MAC
        m_aEncodings[2] = RTL_TEXTENCODING_IBM_437;     // IBMPC_437
        m_aEncodings[3] = RTL_TEXTENCODING_IBM_850;     // IBMPC_850
        m_aEncodings[4] = RTL_TEXTENCODING_IBM_860;     // IBMPC_860
        m_aEncodings[5] = RTL_TEXTENCODING_IBM_861;     // IBMPC_861
        m_aEncodings[6] = RTL_TEXTENCODING_IBM_863;     // IBMPC_863
        m_aEncodings[7] = RTL_TEXTENCODING_IBM_865;     // IBMPC_865
        m_aEncodings[8] = RTL_TEXTENCODING_DONTKNOW;    // SYSTEM

        m_aNames.resize(9);
        m_aNames[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ANSI"));
        m_aNames[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MAC"));
        m_aNames[2] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBMPC_437"));
        m_aNames[3] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBMPC_850"));
        m_aNames[4] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBMPC_860"));
        m_aNames[5] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBMPC_861"));
        m_aNames[6] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBMPC_863"));
        m_aNames[7] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBMPC_865"));
        m_aNames[8] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SYSTEM"));

        OSL_ENSURE(m_aEncodings.size() == m_aNames.size(),
            "OCharsetMap::OCharsetMap: inconsistentce(1)!");

        // the IANA representations of the character sets which we know
        m_aIanaNames.resize(9);
        m_aIanaNames[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("windows-1252"));
        m_aIanaNames[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("macintosh"));
        m_aIanaNames[2] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBM437"));
        m_aIanaNames[3] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBM850"));
        m_aIanaNames[4] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBM860"));
        m_aIanaNames[5] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBM861"));
        m_aIanaNames[6] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBM863"));
        m_aIanaNames[7] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IBM865"));
        m_aIanaNames[8] = ::rtl::OUString();

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
        return CharsetIterator(this, getSequenceIndex(m_aEncodings, _eEncoding));
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
        OSL_ENSURE(m_nPosition < m_pContainer->m_aEncodings.size(), "OCharsetMap::CharsetIterator::operator*: invalid position!");
        OSL_ENSURE(m_nPosition < m_pContainer->m_aIanaNames.size(), "OCharsetMap::CharsetIterator::operator*: invalid position!");
        OSL_ENSURE(m_nPosition < m_pContainer->m_aNames.size(), "OCharsetMap::CharsetIterator::operator*: invalid position!");
        return CharsetIteratorDerefHelper(
            m_pContainer->m_aEncodings[m_nPosition],
            m_pContainer->m_aIanaNames[m_nPosition],
            m_pContainer->m_aNames[m_nPosition]);
    }

    //-------------------------------------------------------------------------
    const OCharsetMap::CharsetIterator& OCharsetMap::CharsetIterator::operator++()
    {
        OSL_ENSURE(m_nPosition < m_pContainer->m_aNames.size(), "OCharsetMap::CharsetIterator::operator++ : invalid position!");
        if (m_nPosition < m_pContainer->m_aNames.size())
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
 *  Revision 1.2  2001/02/13 09:47:39  fs
 *  #83632# merge IBMPC, IBMPC(850), DOS
 *
 *  Revision 1.1  2000/11/29 22:21:42  fs
 *  initial checkin - helper class for translating charset representations
 *
 *
 *  Revision 1.0 29.11.00 18:42:55  fs
 ************************************************************************/

