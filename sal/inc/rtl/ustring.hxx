/*************************************************************************
 *
 *  $RCSfile: ustring.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: th $ $Date: 2001-03-16 15:17:03 $
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

#ifndef _RTL_USTRING_HXX_
#define _RTL_USTRING_HXX_

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#ifndef _RTL_LOCALE_HXX_
#include <rtl/locale.hxx>
#endif

namespace rtl
{

/**
 * Converts all of the characters in this <code>OUString</code> to lower
 * case using the rules of the given locale.
 * @param locale use the case transformation rules for this locale
 * @return the OUString, converted to lowercase.
 * @see     java.lang.Character#toLowerCase(char)
 * @see     java.lang.OUString#toUpperCase()
 * @since   JDK1.1
 */
inline OUString OUString::toLowerCase( const OLocale & locale ) const
{
    OUString newStr;
    rtl_uString_newToLowerCase( &newStr.pData, pData, *(rtl_Locale **)&locale );
    return newStr;
}


/**
 * Converts all of the characters in this <code>OUString</code> to upper
 * case using the rules of the given locale.
 * @param locale use the case transformation rules for this locale
 * @return the OUString, converted to uppercase.
 * @see     java.lang.Character#toUpperCase(char)
 * @see     java.lang.OUString#toLowerCase(char)
 * @since   JDK1.1
 */
inline OUString OUString::toUpperCase( const OLocale & locale) const
{
    OUString newStr;
    rtl_uString_newToUpperCase( &newStr.pData, pData, *(rtl_Locale **)&locale );
    return newStr;
}

// Helper functions
inline OUString OStringToOUString( const OString & rStr,
                                   rtl_TextEncoding encoding,
                                   sal_uInt32 convertFlags = OSTRING_TO_OUSTRING_CVTFLAGS )
{
    OUString newStr( rStr.getStr(), rStr.getLength(), encoding, convertFlags );
    return newStr;
}

inline OString OUStringToOString( const OUString & rUnicode,
                                  rtl_TextEncoding encoding,
                                  sal_uInt32 convertFlags = OUSTRING_TO_OSTRING_CVTFLAGS )
{
    OString newStr( rUnicode.getStr(), rUnicode.getLength(), encoding, convertFlags );
    return newStr;
}

}

#endif /* _RTL_USTRING_HXX_ */
