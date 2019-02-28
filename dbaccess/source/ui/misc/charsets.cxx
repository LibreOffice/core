/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <charsets.hxx>
#include <core_resource.hxx>
#include <osl/diagnose.h>
#include <strings.hrc>
#include <rtl/tencinfo.h>

namespace dbaui
{
    using namespace ::dbtools;

    // OCharsetDisplay
    OCharsetDisplay::OCharsetDisplay()
        : OCharsetMap()
        , m_aSystemDisplayName(DBA_RES( STR_RSC_CHARSETS ))
    {
    }

    bool OCharsetDisplay::approveEncoding( const rtl_TextEncoding _eEncoding, const rtl_TextEncodingInfo& _rInfo ) const
    {
        if ( !OCharsetMap::approveEncoding( _eEncoding, _rInfo ) )
            return false;

        if ( RTL_TEXTENCODING_DONTKNOW == _eEncoding )
            return true;

        return !SvxTextEncodingTable::GetTextString(_eEncoding).isEmpty();
    }

    OCharsetDisplay::const_iterator OCharsetDisplay::begin() const
    {
        return const_iterator( this, OCharsetMap::begin() );
    }

    OCharsetDisplay::const_iterator OCharsetDisplay::end() const
    {
        return const_iterator( this, OCharsetMap::end() );
    }

    OCharsetDisplay::const_iterator OCharsetDisplay::findEncoding(const rtl_TextEncoding _eEncoding) const
    {
        OCharsetMap::const_iterator aBaseIter = OCharsetMap::find(_eEncoding);
        return const_iterator( this, aBaseIter );
    }

    OCharsetDisplay::const_iterator OCharsetDisplay::findIanaName(const OUString& _rIanaName) const
    {
        OCharsetMap::const_iterator aBaseIter = OCharsetMap::findIanaName(_rIanaName);
        return const_iterator( this, aBaseIter );
    }

    OCharsetDisplay::const_iterator OCharsetDisplay::findDisplayName(const OUString& _rDisplayName) const
    {
        rtl_TextEncoding eEncoding = RTL_TEXTENCODING_DONTKNOW;
        if ( _rDisplayName != m_aSystemDisplayName )
        {
            eEncoding = SvxTextEncodingTable::GetTextEncoding(_rDisplayName);
            OSL_ENSURE( RTL_TEXTENCODING_DONTKNOW != eEncoding,
                "OCharsetDisplay::find: non-empty display name, but DONTKNOW!" );
        }
        return const_iterator( this, OCharsetMap::find( eEncoding ) );
    }

    // CharsetDisplayDerefHelper
    CharsetDisplayDerefHelper::CharsetDisplayDerefHelper(const CharsetDisplayDerefHelper& _rSource)
        :CharsetDisplayDerefHelper_Base(_rSource)
        ,m_sDisplayName(_rSource.m_sDisplayName)
    {
    }

    CharsetDisplayDerefHelper::CharsetDisplayDerefHelper(const ::dbtools::CharsetIteratorDerefHelper& _rBase, const OUString& _rDisplayName)
        :CharsetDisplayDerefHelper_Base(_rBase)
        ,m_sDisplayName(_rDisplayName)
    {
        OSL_ENSURE( !m_sDisplayName.isEmpty(), "CharsetDisplayDerefHelper::CharsetDisplayDerefHelper: invalid display name!" );
    }

    // OCharsetDisplay::ExtendedCharsetIterator
    OCharsetDisplay::ExtendedCharsetIterator::ExtendedCharsetIterator( const OCharsetDisplay* _pContainer, const base_iterator& _rPosition )
        :m_pContainer(_pContainer)
        ,m_aPosition(_rPosition)
    {
        OSL_ENSURE(m_pContainer, "OCharsetDisplay::ExtendedCharsetIterator::ExtendedCharsetIterator : invalid container!");
    }

    CharsetDisplayDerefHelper OCharsetDisplay::ExtendedCharsetIterator::operator*() const
    {
        OSL_ENSURE( m_aPosition != m_pContainer->OCharsetDisplay_Base::end(), "OCharsetDisplay::ExtendedCharsetIterator::operator* : invalid position!");

        rtl_TextEncoding eEncoding = (*m_aPosition).getEncoding();
        return CharsetDisplayDerefHelper(
            *m_aPosition,
            RTL_TEXTENCODING_DONTKNOW == eEncoding ? m_pContainer->m_aSystemDisplayName : SvxTextEncodingTable::GetTextString(eEncoding)
        );
    }

    const OCharsetDisplay::ExtendedCharsetIterator& OCharsetDisplay::ExtendedCharsetIterator::operator++()
    {
        OSL_ENSURE( m_aPosition != m_pContainer->OCharsetDisplay_Base::end(), "OCharsetDisplay::ExtendedCharsetIterator::operator++ : invalid position!");
        if ( m_aPosition != m_pContainer->OCharsetDisplay_Base::end() )
            ++m_aPosition;
        return *this;
    }

    bool operator==(const OCharsetDisplay::ExtendedCharsetIterator& lhs, const OCharsetDisplay::ExtendedCharsetIterator& rhs)
    {
        return (lhs.m_pContainer == rhs.m_pContainer) && (lhs.m_aPosition == rhs.m_aPosition);
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
