/*************************************************************************
 *
 *  $RCSfile: charsets.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:52:52 $
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
#ifndef _DBAUI_LOCALRESACCESS_HXX_
#include "localresaccess.hxx"
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
        ,SvxTextEncodingTable()
    {
        {
            OLocalResourceAccess aCharsetStrings( RSC_CHARSETS, RSC_RESOURCE );
            m_aSystemDisplayName = String( ResId( 1 ) );
        }
    }

    //-------------------------------------------------------------------------
    sal_Bool OCharsetDisplay::approveEncoding( const rtl_TextEncoding _eEncoding, const rtl_TextEncodingInfo& _rInfo ) const
    {
        if ( !OCharsetMap::approveEncoding( _eEncoding, _rInfo ) )
            return sal_False;

        if ( RTL_TEXTENCODING_DONTKNOW == _eEncoding )
            return sal_True;

        return 0 != GetTextString( _eEncoding ).Len();
    }

    //-------------------------------------------------------------------------
    OCharsetDisplay::const_iterator OCharsetDisplay::begin() const
    {
        return const_iterator( this, OCharsetMap::begin() );
    }

    //-------------------------------------------------------------------------
    OCharsetDisplay::const_iterator OCharsetDisplay::end() const
    {
        return const_iterator( this, OCharsetMap::end() );
    }

    //-------------------------------------------------------------------------
    OCharsetDisplay::const_iterator OCharsetDisplay::find(const rtl_TextEncoding _eEncoding) const
    {
        OCharsetMap::const_iterator aBaseIter = OCharsetMap::find(_eEncoding);
        return const_iterator( this, aBaseIter );
    }

    //-------------------------------------------------------------------------
    OCharsetDisplay::const_iterator OCharsetDisplay::find(const ::rtl::OUString& _rIanaName, const IANA&) const
    {
        OCharsetMap::const_iterator aBaseIter = OCharsetMap::find(_rIanaName, OCharsetMap::IANA());
        return const_iterator( this, aBaseIter );
    }

    //-------------------------------------------------------------------------
    OCharsetDisplay::const_iterator OCharsetDisplay::find(const ::rtl::OUString& _rDisplayName, const Display&) const
    {
        rtl_TextEncoding eEncoding = RTL_TEXTENCODING_DONTKNOW;
        if ( _rDisplayName != m_aSystemDisplayName )
        {
            eEncoding = GetTextEncoding( _rDisplayName );
            OSL_ENSURE( RTL_TEXTENCODING_DONTKNOW != eEncoding,
                "OCharsetDisplay::find: non-empty display name, but DONTKNOW!" );
        }
        return const_iterator( this, OCharsetMap::find( eEncoding ) );
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
        DBG_ASSERT( m_sDisplayName.getLength(), "CharsetDisplayDerefHelper::CharsetDisplayDerefHelper: invalid display name!" );
    }

    //-------------------------------------------------------------------------
    CharsetDisplayDerefHelper::CharsetDisplayDerefHelper()
    {
    }

    //=========================================================================
    //= OCharsetDisplay::ExtendedCharsetIterator
    //=========================================================================
    //-------------------------------------------------------------------------
    OCharsetDisplay::ExtendedCharsetIterator::ExtendedCharsetIterator( const OCharsetDisplay* _pContainer, const base_iterator& _rPosition )
        :m_pContainer(_pContainer)
        ,m_aPosition(_rPosition)
    {
        DBG_ASSERT(m_pContainer, "OCharsetDisplay::ExtendedCharsetIterator::ExtendedCharsetIterator : invalid container!");
    }

    //-------------------------------------------------------------------------
    OCharsetDisplay::ExtendedCharsetIterator::ExtendedCharsetIterator(const ExtendedCharsetIterator& _rSource)
        :m_pContainer( _rSource.m_pContainer )
        ,m_aPosition( _rSource.m_aPosition )
    {
    }

    //-------------------------------------------------------------------------
    CharsetDisplayDerefHelper OCharsetDisplay::ExtendedCharsetIterator::operator*() const
    {
        DBG_ASSERT( m_aPosition != m_pContainer->OCharsetDisplay_Base::end(), "OCharsetDisplay::ExtendedCharsetIterator::operator* : invalid position!");

        rtl_TextEncoding eEncoding = (*m_aPosition).getEncoding();
        return CharsetDisplayDerefHelper(
            *m_aPosition,
            RTL_TEXTENCODING_DONTKNOW == eEncoding ? m_pContainer->m_aSystemDisplayName : (::rtl::OUString)m_pContainer->GetTextString( eEncoding )
        );
    }

    //-------------------------------------------------------------------------
    const OCharsetDisplay::ExtendedCharsetIterator& OCharsetDisplay::ExtendedCharsetIterator::operator++()
    {
        DBG_ASSERT( m_aPosition != m_pContainer->OCharsetDisplay_Base::end(), "OCharsetDisplay::ExtendedCharsetIterator::operator++ : invalid position!");
        if ( m_aPosition != m_pContainer->OCharsetDisplay_Base::end() )
            ++m_aPosition;
        return *this;
    }

    //-------------------------------------------------------------------------
    const OCharsetDisplay::ExtendedCharsetIterator& OCharsetDisplay::ExtendedCharsetIterator::operator--()
    {
        DBG_ASSERT( m_aPosition != m_pContainer->OCharsetDisplay_Base::begin(), "OCharsetDisplay::ExtendedCharsetIterator::operator-- : invalid position!");
        if ( m_aPosition != m_pContainer->OCharsetDisplay_Base::begin() )
            --m_aPosition;
        return *this;
    }

    //-------------------------------------------------------------------------
    bool operator==(const OCharsetDisplay::ExtendedCharsetIterator& lhs, const OCharsetDisplay::ExtendedCharsetIterator& rhs)
    {
        return (lhs.m_pContainer == rhs.m_pContainer) && (lhs.m_aPosition == rhs.m_aPosition);
    }

//.........................................................................
}   // namespace dbaui
//.........................................................................

