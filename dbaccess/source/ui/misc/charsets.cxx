/*************************************************************************
 *
 *  $RCSfile: charsets.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: oj $ $Date: 2002-08-19 07:51:10 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_CHARSETS_HXX_
#include "charsets.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBU_MISCRES_HRC_
#include "dbumiscres.hrc"
#endif
#ifndef _DBU_MISC_HRC_
#include "dbu_misc.hrc"
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _TOOLS_RCID_H
#include <tools/rcid.h>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................
    using namespace ::dbtools;

    //=========================================================================
    //= OCharsetDisplay
    //=========================================================================
    //-------------------------------------------------------------------------
    OCharsetDisplay::OCharsetDisplay()
        :OCharsetMap()
        ,Resource(RSC_CHARSETS)
    {
#ifdef DBG_UTIL
        sal_Bool bAlreadyAsserted = sal_False;
#endif

        sal_Int32 nSize = size();
        m_aDisplayNames.reserve( nSize );
        for (sal_Int32 i=1; i<=nSize; ++i)
        {
            ResId aLocalId( (USHORT)i );

            if ( IsAvailableRes( aLocalId.SetRT( RSC_STRING ) ) )
                m_aDisplayNames.push_back( String( aLocalId ) );
            else
            {
                DBG_ASSERT( bAlreadyAsserted, "OCharsetDisplay::OCharsetDisplay: invalid resources!" );
#ifdef DBG_UTIL
                bAlreadyAsserted = sal_True;
#endif
                m_aDisplayNames.push_back( ::rtl::OUString::createFromAscii( "<unknown>" ) );
            }
        }

        FreeResource();
    }

    //-------------------------------------------------------------------------
    OCharsetDisplay::const_iterator OCharsetDisplay::begin() const
    {
        return const_iterator(this, OCharsetMap::begin(), 0);
    }

    //-------------------------------------------------------------------------
    OCharsetDisplay::const_iterator OCharsetDisplay::end() const
    {
        return const_iterator(this, OCharsetMap::end(), size());
    }

    //-------------------------------------------------------------------------
    OCharsetDisplay::const_iterator OCharsetDisplay::find(const rtl_TextEncoding _eEncoding) const
    {
        OCharsetMap::const_iterator aBaseIter = OCharsetMap::find(_eEncoding);
        return const_iterator(this, aBaseIter, aBaseIter - OCharsetMap::begin());
    }

    //-------------------------------------------------------------------------
    OCharsetDisplay::const_iterator OCharsetDisplay::find(const ::rtl::OUString& _rIanaName, const IANA&) const
    {
        OCharsetMap::const_iterator aBaseIter = OCharsetMap::find(_rIanaName, OCharsetMap::IANA());
        return const_iterator(this, aBaseIter, aBaseIter - OCharsetMap::begin());
    }

    //-------------------------------------------------------------------------
    OCharsetDisplay::const_iterator OCharsetDisplay::find(const ::rtl::OUString& _rLogicalName, const Logical&) const
    {
        OCharsetMap::const_iterator aBaseIter = OCharsetMap::find(_rLogicalName, OCharsetMap::Logical());
        return const_iterator(this, aBaseIter, aBaseIter - OCharsetMap::begin());
    }

    //-------------------------------------------------------------------------
    OCharsetDisplay::const_iterator OCharsetDisplay::find(const ::rtl::OUString& _rDisplayName, const Display&) const
    {
        OCharsetMap::CharsetIterator aBasePos = OCharsetMap::begin();
        sal_Int32 nIndex = 0;
        for (   ConstStringVectorIterator aSearch = m_aDisplayNames.begin();
                aSearch != m_aDisplayNames.end();
                ++aSearch, ++aBasePos, ++nIndex
            )
            if (*aSearch == _rDisplayName)
                break;

        return const_iterator(this, aBasePos, nIndex);
    }

    //=========================================================================
    //= CharsetDisplayDerefHelper
    //=========================================================================
    //-------------------------------------------------------------------------
    CharsetDisplayDerefHelper::CharsetDisplayDerefHelper(const CharsetDisplayDerefHelper& _rSource)
        :CharsetDisplayDerefHelper_Base(_rSource)
        ,m_sDisplayName(m_sDisplayName)
    {
    }

    //-------------------------------------------------------------------------
    CharsetDisplayDerefHelper::CharsetDisplayDerefHelper(const CharsetDisplayDerefHelper_Base& _rBase, const ::rtl::OUString& _rDisplayName)
        :CharsetDisplayDerefHelper_Base(_rBase)
        ,m_sDisplayName(_rDisplayName)
    {
    }

    //-------------------------------------------------------------------------
    CharsetDisplayDerefHelper::CharsetDisplayDerefHelper()
    {
    }

    //=========================================================================
    //= OCharsetDisplay::ExtendedCharsetIterator
    //=========================================================================
    //-------------------------------------------------------------------------
    OCharsetDisplay::ExtendedCharsetIterator::ExtendedCharsetIterator(const OCharsetDisplay* _pContainer, const base_iterator& _rPosition, const sal_Int32 _nPosition)
        :m_pContainer(_pContainer)
        ,m_aPosition(_rPosition)
        ,m_nPosition(_nPosition)
    {
        DBG_ASSERT(m_pContainer, "OCharsetDisplay::ExtendedCharsetIterator::ExtendedCharsetIterator : invalid container!");
    }

    //-------------------------------------------------------------------------
    OCharsetDisplay::ExtendedCharsetIterator::ExtendedCharsetIterator(const ExtendedCharsetIterator& _rSource)
        :m_pContainer(_rSource.m_pContainer)
        ,m_aPosition(_rSource.m_aPosition)
        ,m_nPosition(_rSource.m_nPosition)
    {
    }

    //-------------------------------------------------------------------------
    CharsetDisplayDerefHelper OCharsetDisplay::ExtendedCharsetIterator::operator*() const
    {
        DBG_ASSERT(m_nPosition < (sal_Int32)m_pContainer->m_aDisplayNames.size(), "OCharsetDisplay::ExtendedCharsetIterator::operator* : invalid position!");
        return CharsetDisplayDerefHelper(*m_aPosition, m_pContainer->m_aDisplayNames[m_nPosition]);
    }

    //-------------------------------------------------------------------------
    const OCharsetDisplay::ExtendedCharsetIterator& OCharsetDisplay::ExtendedCharsetIterator::operator++()
    {
        DBG_ASSERT(m_nPosition < (sal_Int32)m_pContainer->m_aDisplayNames.size(), "OCharsetDisplay::ExtendedCharsetIterator::operator++ : invalid position!");
        if (m_nPosition < (sal_Int32)m_pContainer->m_aDisplayNames.size())
        {
            ++m_nPosition;
            ++m_aPosition;
        }
        return *this;
    }

    //-------------------------------------------------------------------------
    const OCharsetDisplay::ExtendedCharsetIterator& OCharsetDisplay::ExtendedCharsetIterator::operator--()
    {
        DBG_ASSERT(m_nPosition >= 0, "OCharsetDisplay::ExtendedCharsetIterator::operator-- : invalid position!");
        if (m_nPosition >= 0)
        {
            --m_nPosition;
            --m_aPosition;
        }
        return *this;
    }

    //-------------------------------------------------------------------------
    bool operator==(const OCharsetDisplay::ExtendedCharsetIterator& lhs, const OCharsetDisplay::ExtendedCharsetIterator& rhs)
    {
        return (lhs.m_pContainer == rhs.m_pContainer) && (lhs.m_nPosition == rhs.m_nPosition);
    }

//.........................................................................
}   // namespace dbaui
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.5  2002/03/14 08:54:05  fs
 *  #97788# assertion for invalid character set maps corrected
 *
 *  Revision 1.4  2001/10/15 13:38:09  fs
 *  #93204# new supported charset IBM866 (cyrillic)
 *
 *  Revision 1.3  2001/05/10 12:02:10  fs
 *  #86849# be a little more tolerant when display names are missing
 *
 *  Revision 1.2  2000/11/29 22:26:32  fs
 *  #80003# re-implemented, now base on dbtools::OCharsetMap
 *
 *  Revision 1.1  2000/10/05 10:08:39  fs
 *  initial checkin
 *
 *
 *  Revision 1.0 26.09.00 12:18:36  fs
 ************************************************************************/

