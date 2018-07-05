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

#ifndef INCLUDED_CONNECTIVITY_DBCHARSET_HXX
#define INCLUDED_CONNECTIVITY_DBCHARSET_HXX

#include <sal/config.h>

#include <cstddef>
#include <set>

#include <rtl/textenc.h>
#include <rtl/tencinfo.h>
#include <rtl/ustring.hxx>
#include <connectivity/dbtoolsdllapi.hxx>


namespace dbtools
{


    //= OCharsetMap

    /** is a class which translates between different charset representations.

        <p>The set of recognized charsets is very limited: only the ones which are database relevant are
        implemented at the moment</p>

        <p>Possible representations are:
        <ul>
            <li><b>IANA names.</b>
            Have a look at <A href="http://www.iana.org/assignments/character-sets">this document</A> for
            more details</li>
            <li><b>rtl_TextEncoding</b></li>
        </ul>
        </p>
    */
    class OOO_DLLPUBLIC_DBTOOLS OCharsetMap
    {
    protected:
        typedef std::set<rtl_TextEncoding> TextEncBag;

        TextEncBag  m_aEncodings;

    public:
        class CharsetIterator;
        friend class OCharsetMap::CharsetIterator;
        typedef CharsetIterator iterator;
        typedef CharsetIterator const_iterator;

        OCharsetMap();
        virtual ~OCharsetMap();

        /** find the given text encoding in the map.
            @return the <em>end</em> iterator if the encoding could not be found.
        */
        CharsetIterator find(const rtl_TextEncoding _eEncoding) const;
        /** find the given IANA name in the map.
            @return the <em>end</em> iterator if the IANA name could not be found.
        */
        CharsetIterator findIanaName(const OUString& _rIanaName) const;

        /// get access to the first element of the charset collection
        CharsetIterator begin() const;
        /// get access to the (last + 1st) element of the charset collection
        CharsetIterator end() const;

    protected:
        // needed because we want to call a virtual method during construction
                void lateConstruct();
        void ensureConstructed( ) const { if ( m_aEncodings.empty() ) const_cast< OCharsetMap* >( this )->lateConstruct(); }

        virtual bool approveEncoding( const rtl_TextEncoding _eEncoding, const rtl_TextEncodingInfo& _rInfo ) const;
    };


    //- CharsetIteratorDerefHelper

    class OOO_DLLPUBLIC_DBTOOLS CharsetIteratorDerefHelper
    {
        friend class OCharsetMap::CharsetIterator;

        rtl_TextEncoding    m_eEncoding;
        OUString     m_aIanaName;

    public:
        CharsetIteratorDerefHelper(const CharsetIteratorDerefHelper& _rSource);

        rtl_TextEncoding    getEncoding() const { return m_eEncoding; }
        const OUString&     getIanaName() const { return m_aIanaName; }

    protected:
        CharsetIteratorDerefHelper( const rtl_TextEncoding _eEncoding, const OUString& _rIanaName );

    };


    //- OCharsetMap::CharsetIterator

    class OOO_DLLPUBLIC_DBTOOLS OCharsetMap::CharsetIterator
    {
        friend class OCharsetMap;

        friend OOO_DLLPUBLIC_DBTOOLS bool operator==(const CharsetIterator& lhs, const CharsetIterator& rhs);
        friend bool operator!=(const CharsetIterator& lhs, const CharsetIterator& rhs) { return !(lhs == rhs); }

//      friend sal_Int32 operator-(const CharsetIterator& lhs, const CharsetIterator& rhs);

    protected:
        const OCharsetMap*                      m_pContainer;
        OCharsetMap::TextEncBag::const_iterator m_aPos;

    public:
        CharsetIteratorDerefHelper operator*() const;
        // no -> operator
        // this would require us to a) store CharsetIteratorDerefHelper instances ourself so that we
        // can return a pointer or b) introduce a -> operator on the CharsetIteratorDerefHelper, too.

        /// prefix increment
        const CharsetIterator&  operator++();

        /// prefix decrement
        const CharsetIterator&  operator--();

    protected:
        CharsetIterator(const OCharsetMap* _pContainer, OCharsetMap::TextEncBag::const_iterator const & _aPos );
    };


}   // namespace dbtools


#endif // INCLUDED_CONNECTIVITY_DBCHARSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
