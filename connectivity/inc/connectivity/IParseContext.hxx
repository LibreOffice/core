/*************************************************************************
 *
 *  $RCSfile: IParseContext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-25 18:25:44 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
            ERROR_GENERAL_HINT,             // "before \"#\" expression.", uses 1 parameter
            ERROR_VALUE_NO_LIKE,            // "The value # can not be used with LIKE!", uses 1 parameter
            ERROR_FIELD_NO_LIKE,            // "LIKE can not be used with this field!"
            ERROR_INVALID_COMPARE,          // "The entered criterion can not be compared with this field!";
            ERROR_INVALID_INT_COMPARE,      // "The field can not be compared with a number!"
            ERROR_INVALID_STRING_COMPARE,   // "The field can not be compared with a string!"
            ERROR_INVALID_DATE_COMPARE,     // "The field can not be compared with a date!"
            ERROR_INVALID_REAL_COMPARE,     // "The field can not be compared with a floating point number!"
            ERROR_INVALID_TABLE,            // "The table \"#\" is unknown in the database!"
            ERROR_INVALID_COLUMN,           // "The column \"#\" is unknown in the table \"#\"!"
            ERROR_INVALID_TABLE_EXIST       // "The table or view \"#\" is already in the database!"
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

