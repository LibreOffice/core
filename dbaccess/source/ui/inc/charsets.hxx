/*************************************************************************
 *
 *  $RCSfile: charsets.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:52:38 $
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
#define _DBAUI_CHARSETS_HXX_

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_RC_HXX
#include <tools/rc.hxx>
#endif
#ifndef _DBHELPER_DBCHARSET_HXX_
#include <connectivity/dbcharset.hxx>
#endif
#ifndef _SVX_TXENCTAB_HXX
#include <svx/txenctab.hxx>
#endif

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

        struct IANA     { };
        struct Display  { };

        // various find operations
        const_iterator find(const rtl_TextEncoding _eEncoding) const;
        const_iterator find(const ::rtl::OUString& _rIanaName, const IANA&) const;
        const_iterator find(const ::rtl::OUString& _rDisplayName, const Display&) const;

        /// get access to the first element of the charset collection
        const_iterator  begin() const;
        /// get access to the (last + 1st) element of the charset collection
        const_iterator  end() const;
        // size of the map
        sal_Int32   size() const { return OCharsetDisplay_Base::size(); }

    protected:
        virtual sal_Bool approveEncoding( const rtl_TextEncoding _eEncoding, const rtl_TextEncodingInfo& _rInfo ) const;
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
        CharsetDisplayDerefHelper();
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

