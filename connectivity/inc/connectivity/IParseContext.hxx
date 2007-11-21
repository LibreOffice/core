/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IParseContext.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 14:56:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CONNECTIVITY_IPARSECONTEXT_HXX
#define CONNECTIVITY_IPARSECONTEXT_HXX

namespace connectivity { class OSQLParseNode; }

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

namespace connectivity
{
    //==========================================================================
    //= IParseContext
    //==========================================================================
    class IParseContext
    {
    public:
        enum    ErrorCode
        {
            ERROR_NONE      = 0,
            ERROR_GENERAL,                  // "Syntax error in SQL expression"
            ERROR_VALUE_NO_LIKE,            // "The value #1 can not be used with LIKE."
            ERROR_FIELD_NO_LIKE,            // "LIKE can not be used with this field."
            ERROR_INVALID_COMPARE,          // "The entered criterion can not be compared with this field."
            ERROR_INVALID_INT_COMPARE,      // "The field can not be compared with a number."
            ERROR_INVALID_DATE_COMPARE,     // "The field can not be compared with a date."
            ERROR_INVALID_REAL_COMPARE,     // "The field can not be compared with a floating point number."
            ERROR_INVALID_TABLE,            // "The database does not contain a table named \"#\"."
            ERROR_INVALID_TABLE_OR_QUERY,   // "The database does contain neither a table nor a query named \"#\"."
            ERROR_INVALID_COLUMN,           // "The column \"#1\" is unknown in the table \"#2\"."
            ERROR_INVALID_TABLE_EXIST,      // "The database already contains a table or view with name \"#\"."
            ERROR_INVALID_QUERY_EXIST       // "The database already contains a query with name \"#\".";
        };

        enum    InternationalKeyCode
        {
            KEY_NONE = 0,
            KEY_LIKE,
            KEY_NOT,
            KEY_NULL,
            KEY_TRUE,
            KEY_FALSE,
            KEY_IS,
            KEY_BETWEEN,
            KEY_OR,
            KEY_AND,
            KEY_AVG,
            KEY_COUNT,
            KEY_MAX,
            KEY_MIN,
            KEY_SUM
        };

    public:
        // retrieves language specific error messages
        virtual ::rtl::OUString getErrorMessage(ErrorCode _eCodes) const = 0;

        // retrieves language specific keyword strings (only ASCII allowed)
        virtual ::rtl::OString getIntlKeywordAscii(InternationalKeyCode _eKey) const = 0;

        // finds out, if we have an international keyword (only ASCII allowed)
        virtual InternationalKeyCode getIntlKeyCode(const ::rtl::OString& rToken) const = 0;

        /** get's a locale instance which should be used when parsing in the context specified by this instance
            <p>if this is not overridden by derived classes, it returns the static default locale.</p>
        */
        virtual ::com::sun::star::lang::Locale getPreferredLocale( ) const = 0;
    };
}

#endif // CONNECTIVITY_IPARSECONTEXT_HXX

