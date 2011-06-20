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

#ifndef _DBAUI_CHARSETS_HXX_
#define _DBAUI_CHARSETS_HXX_

#include <tools/string.hxx>
#include <tools/rc.hxx>
#include <connectivity/dbcharset.hxx>
#include <svx/txenctab.hxx>

//.........................................................................
namespace dbaui
{
//.........................................................................

    //=========================================================================
    //= OCharsetDisplay
    //=========================================================================
    typedef ::dbtools::OCharsetMap OCharsetDisplay_Base;
    class OCharsetDisplay
            :protected OCharsetDisplay_Base
            ,protected SvxTextEncodingTable
    {
    protected:
        ::rtl::OUString m_aSystemDisplayName;

    public:
        class ExtendedCharsetIterator;
        friend class OCharsetDisplay::ExtendedCharsetIterator;

        typedef ExtendedCharsetIterator iterator;
        typedef ExtendedCharsetIterator const_iterator;

        OCharsetDisplay();

        // various find operations
        const_iterator findEncoding(const rtl_TextEncoding _eEncoding) const;
        const_iterator findIanaName(const ::rtl::OUString& _rIanaName) const;
        const_iterator findDisplayName(const ::rtl::OUString& _rDisplayName) const;

        /// get access to the first element of the charset collection
        const_iterator  begin() const;
        /// get access to the (last + 1st) element of the charset collection
        const_iterator  end() const;
        // size of the map
        sal_Int32   size() const { return OCharsetDisplay_Base::size(); }

    protected:
        virtual sal_Bool approveEncoding( const rtl_TextEncoding _eEncoding, const rtl_TextEncodingInfo& _rInfo ) const;

    private:
        using OCharsetDisplay_Base::find;
    };

    //-------------------------------------------------------------------------
    //- CharsetDisplayDerefHelper
    //-------------------------------------------------------------------------
    typedef ::dbtools::CharsetIteratorDerefHelper CharsetDisplayDerefHelper_Base;
    class CharsetDisplayDerefHelper : protected CharsetDisplayDerefHelper_Base
    {
        friend class OCharsetDisplay::ExtendedCharsetIterator;

        ::rtl::OUString                         m_sDisplayName;

    public:
        CharsetDisplayDerefHelper(const CharsetDisplayDerefHelper& _rSource);

        rtl_TextEncoding    getEncoding() const         { return CharsetDisplayDerefHelper_Base::getEncoding(); }
        ::rtl::OUString     getIanaName() const         { return CharsetDisplayDerefHelper_Base::getIanaName(); }
        ::rtl::OUString     getDisplayName() const      { return m_sDisplayName; }

    protected:
        CharsetDisplayDerefHelper(const ::dbtools::CharsetIteratorDerefHelper& _rBase, const ::rtl::OUString& _rDisplayName);
    };

    //-------------------------------------------------------------------------
    //- OCharsetDisplay::ExtendedCharsetIterator
    //-------------------------------------------------------------------------
    class OCharsetDisplay::ExtendedCharsetIterator
    {
        friend class OCharsetDisplay;

        friend bool operator==(const ExtendedCharsetIterator& lhs, const ExtendedCharsetIterator& rhs);
        friend bool operator!=(const ExtendedCharsetIterator& lhs, const ExtendedCharsetIterator& rhs) { return !(lhs == rhs); }

        typedef ::dbtools::OCharsetMap      container;
        typedef container::CharsetIterator  base_iterator;

    protected:
        const OCharsetDisplay*      m_pContainer;
        base_iterator               m_aPosition;

    public:
        ExtendedCharsetIterator(const ExtendedCharsetIterator& _rSource);

        CharsetDisplayDerefHelper operator*() const;

        /// prefix increment
        const ExtendedCharsetIterator&  operator++();
        /// postfix increment
        const ExtendedCharsetIterator   operator++(int) { ExtendedCharsetIterator hold(*this); ++*this; return hold; }

        /// prefix decrement
        const ExtendedCharsetIterator&  operator--();
        /// postfix decrement
        const ExtendedCharsetIterator   operator--(int) { ExtendedCharsetIterator hold(*this); --*this; return hold; }

    protected:
        ExtendedCharsetIterator( const OCharsetDisplay* _pContainer, const base_iterator& _rPosition );
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_CHARSETS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
