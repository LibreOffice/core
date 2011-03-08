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

#ifndef _DBHELPER_DBCHARSET_HXX_
#define _DBHELPER_DBCHARSET_HXX_

#include <comphelper/stl_types.hxx>
#include <rtl/textenc.h>
#include <rtl/tencinfo.h>
#include <rtl/ustring.hxx>
#include "connectivity/dbtoolsdllapi.hxx"

//.........................................................................
namespace dbtools
{
//.........................................................................

    //=========================================================================
    //= OCharsetMap
    //=========================================================================
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
        DECLARE_STL_STDKEY_SET( rtl_TextEncoding, TextEncBag );

        TextEncBag  m_aEncodings;

    public:
        class CharsetIterator;
        friend class OCharsetMap::CharsetIterator;
        typedef CharsetIterator iterator;
        typedef CharsetIterator const_iterator;

        OCharsetMap();
        virtual ~OCharsetMap();

        struct IANA { };

        /** find the given text encoding in the map.
            @return the <em>end</em> iterator if the encoding could not be found.
        */
        CharsetIterator find(const rtl_TextEncoding _eEncoding) const;
        /** find the given IANA name in the map.
            @return the <em>end</em> iterator if the IANA name could not be found.
        */
        CharsetIterator find(const ::rtl::OUString& _rIanaName, const IANA&) const;

        sal_Int32   size() const { ensureConstructed( ); return m_aEncodings.size(); }

        /// get access to the first element of the charset collection
        CharsetIterator begin() const;
        /// get access to the (last + 1st) element of the charset collection
        CharsetIterator end() const;

    protected:
        // needed because we want to call a virtual method during construction
                void lateConstruct();
        inline  void ensureConstructed( ) const { if ( m_aEncodings.empty() ) const_cast< OCharsetMap* >( this )->lateConstruct(); }

        virtual sal_Bool approveEncoding( const rtl_TextEncoding _eEncoding, const rtl_TextEncodingInfo& _rInfo ) const;
    };

    //-------------------------------------------------------------------------
    //- CharsetIteratorDerefHelper
    //-------------------------------------------------------------------------
    class OOO_DLLPUBLIC_DBTOOLS CharsetIteratorDerefHelper
    {
        friend class OCharsetMap::CharsetIterator;

        rtl_TextEncoding    m_eEncoding;
        ::rtl::OUString     m_aIanaName;

    public:
        CharsetIteratorDerefHelper(const CharsetIteratorDerefHelper& _rSource);

        rtl_TextEncoding    getEncoding() const { return m_eEncoding; }
        ::rtl::OUString     getIanaName() const { return m_aIanaName; }

    protected:
        CharsetIteratorDerefHelper();
        CharsetIteratorDerefHelper( const rtl_TextEncoding _eEncoding, const ::rtl::OUString& _rIanaName );

    };


    //-------------------------------------------------------------------------
    //- OCharsetMap::CharsetIterator
    //-------------------------------------------------------------------------
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
        CharsetIterator(const CharsetIterator& _rSource);
        ~CharsetIterator();

        CharsetIteratorDerefHelper operator*() const;
        // no -> operator
        // this would require us to a) store CharsetIteratorDerefHelper instances ourself so that we
        // can return a pointer or b) introduce a -> operator on the CharsetIteratorDerefHelper, too.

        /// prefix increment
        const CharsetIterator&  operator++();
        /// postfix increment
        const CharsetIterator   operator++(int) { CharsetIterator hold(*this); ++*this; return hold; }

        /// prefix decrement
        const CharsetIterator&  operator--();
        /// postfix decrement
        const CharsetIterator   operator--(int) { CharsetIterator hold(*this); --*this; return hold; }

    protected:
        CharsetIterator(const OCharsetMap* _pContainer, OCharsetMap::TextEncBag::const_iterator _aPos );
    };

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // _DBHELPER_DBCHARSET_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
