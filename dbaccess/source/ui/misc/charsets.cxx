/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "charsets.hxx"
#include <osl/diagnose.h>
#include "dbu_misc.hrc"
#include <rtl/tencinfo.h>
#include <tools/rcid.h>
#include "localresaccess.hxx"

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
            LocalResourceAccess aCharsetStrings( RSC_CHARSETS, RSC_RESOURCE );
            m_aSystemDisplayName = String( ModuleRes( 1 ) );
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
    OCharsetDisplay::const_iterator OCharsetDisplay::findEncoding(const rtl_TextEncoding _eEncoding) const
    {
        OCharsetMap::const_iterator aBaseIter = OCharsetMap::find(_eEncoding);
        return const_iterator( this, aBaseIter );
    }

    //-------------------------------------------------------------------------
    OCharsetDisplay::const_iterator OCharsetDisplay::findIanaName(const ::rtl::OUString& _rIanaName) const
    {
        OCharsetMap::const_iterator aBaseIter = OCharsetMap::find(_rIanaName, OCharsetMap::IANA());
        return const_iterator( this, aBaseIter );
    }

    //-------------------------------------------------------------------------
    OCharsetDisplay::const_iterator OCharsetDisplay::findDisplayName(const ::rtl::OUString& _rDisplayName) const
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
        OSL_ENSURE( m_sDisplayName.getLength(), "CharsetDisplayDerefHelper::CharsetDisplayDerefHelper: invalid display name!" );
    }

    //=========================================================================
    //= OCharsetDisplay::ExtendedCharsetIterator
    //=========================================================================
    //-------------------------------------------------------------------------
    OCharsetDisplay::ExtendedCharsetIterator::ExtendedCharsetIterator( const OCharsetDisplay* _pContainer, const base_iterator& _rPosition )
        :m_pContainer(_pContainer)
        ,m_aPosition(_rPosition)
    {
        OSL_ENSURE(m_pContainer, "OCharsetDisplay::ExtendedCharsetIterator::ExtendedCharsetIterator : invalid container!");
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
        OSL_ENSURE( m_aPosition != m_pContainer->OCharsetDisplay_Base::end(), "OCharsetDisplay::ExtendedCharsetIterator::operator* : invalid position!");

        rtl_TextEncoding eEncoding = (*m_aPosition).getEncoding();
        return CharsetDisplayDerefHelper(
            *m_aPosition,
            RTL_TEXTENCODING_DONTKNOW == eEncoding ? m_pContainer->m_aSystemDisplayName : (::rtl::OUString)m_pContainer->GetTextString( eEncoding )
        );
    }

    //-------------------------------------------------------------------------
    const OCharsetDisplay::ExtendedCharsetIterator& OCharsetDisplay::ExtendedCharsetIterator::operator++()
    {
        OSL_ENSURE( m_aPosition != m_pContainer->OCharsetDisplay_Base::end(), "OCharsetDisplay::ExtendedCharsetIterator::operator++ : invalid position!");
        if ( m_aPosition != m_pContainer->OCharsetDisplay_Base::end() )
            ++m_aPosition;
        return *this;
    }

    //-------------------------------------------------------------------------
    const OCharsetDisplay::ExtendedCharsetIterator& OCharsetDisplay::ExtendedCharsetIterator::operator--()
    {
        OSL_ENSURE( m_aPosition != m_pContainer->OCharsetDisplay_Base::begin(), "OCharsetDisplay::ExtendedCharsetIterator::operator-- : invalid position!");
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
