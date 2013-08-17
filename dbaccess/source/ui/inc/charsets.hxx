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

#ifndef _DBAUI_CHARSETS_HXX_
#define _DBAUI_CHARSETS_HXX_

#include <connectivity/dbcharset.hxx>
#include <rtl/ustring.hxx>
#include <svx/txenctab.hxx>

namespace dbaui
{

    // OCharsetDisplay
    typedef ::dbtools::OCharsetMap OCharsetDisplay_Base;
    class OCharsetDisplay
            :protected OCharsetDisplay_Base
            ,protected SvxTextEncodingTable
    {
    protected:
        OUString m_aSystemDisplayName;

    public:
        class ExtendedCharsetIterator;
        friend class OCharsetDisplay::ExtendedCharsetIterator;

        typedef ExtendedCharsetIterator iterator;
        typedef ExtendedCharsetIterator const_iterator;

        OCharsetDisplay();

        // various find operations
        const_iterator findEncoding(const rtl_TextEncoding _eEncoding) const;
        const_iterator findIanaName(const OUString& _rIanaName) const;
        const_iterator findDisplayName(const OUString& _rDisplayName) const;

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

    //- CharsetDisplayDerefHelper
    typedef ::dbtools::CharsetIteratorDerefHelper CharsetDisplayDerefHelper_Base;
    class CharsetDisplayDerefHelper : protected CharsetDisplayDerefHelper_Base
    {
        friend class OCharsetDisplay::ExtendedCharsetIterator;

        OUString                         m_sDisplayName;

    public:
        CharsetDisplayDerefHelper(const CharsetDisplayDerefHelper& _rSource);

        rtl_TextEncoding    getEncoding() const         { return CharsetDisplayDerefHelper_Base::getEncoding(); }
        OUString     getIanaName() const         { return CharsetDisplayDerefHelper_Base::getIanaName(); }
        OUString     getDisplayName() const      { return m_sDisplayName; }

    protected:
        CharsetDisplayDerefHelper(const ::dbtools::CharsetIteratorDerefHelper& _rBase, const OUString& _rDisplayName);
    };

    //- OCharsetDisplay::ExtendedCharsetIterator
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

}   // namespace dbaui

#endif // _DBAUI_CHARSETS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
