/*************************************************************************
 *
 *  $RCSfile: locale.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: th $ $Date: 2001-03-16 15:13:17 $
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

#ifndef _RTL_LOCALE_H_
#define _RTL_LOCALE_H_

#ifndef _RTL_USTRING_H
#include <rtl/ustring.h>
#endif /* _RTL_USTRING_H */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SAL_W32
#pragma pack(push, 8)
#elif defined(SAL_OS2)
#pragma pack(1)
#endif

/**
 * The implementation structur of a locale. Do not create this structure
 * direct. Only use the functions rtl_locale_register and
 * rtl_locale_setDefault. The strings Language, Country and Variant
 * are constants, so it is not necessary to acquire and release them.
 */
typedef struct _rtl_Locale
{
    /**
     * Lowercase two-letter ISO-639 code.
     */
    rtl_uString *   Language;
    /**
     * uppercase two-letter ISO-3166 code.
     */
    rtl_uString *   Country;
    /**
     * Lowercase vendor and browser specific code.
     */
    rtl_uString *   Variant;
    /**
     * The merged hash value of the Language, Country and Variant strings.
     */
    sal_Int32       HashCode;
} rtl_Locale;

#ifdef SAL_W32
#pragma pack(pop)
#elif defined(SAL_OS2)
#pragma pack()
#endif

/**
 * Register a locale from language, country and variant.
 * @param language lowercase two-letter ISO-639 code.
 * @param country uppercase two-letter ISO-3166 code. May be null.
 * @param variant vendor and browser specific code. May be null.
 */
rtl_Locale * SAL_CALL rtl_locale_register( const sal_Unicode * language, const sal_Unicode * country, const sal_Unicode * variant );

/**
 * Common method of getting the current default Locale.
 * Used for the presentation: menus, dialogs, etc.
 * Generally set once when your applet or application is initialized,
 * then never reset. (If you do reset the default locale, you
 * probably want to reload your GUI, so that the change is reflected
 * in your interface.)
 * <p>More advanced programs will allow users to use different locales
 * for different fields, e.g. in a spreadsheet.
 * <BR>Note that the initial setting will match the host system.
 */
rtl_Locale * SAL_CALL rtl_locale_getDefault();

/**
 * Sets the default.
 * Normally set once at the beginning of applet or application,
 * then never reset. <code>setDefault</code> does not reset the host locale.
 * @param language lowercase two-letter ISO-639 code.
 * @param country uppercase two-letter ISO-3166 code.
 * @param variant vendor and browser specific code. See class description.
 */
void SAL_CALL rtl_locale_setDefault( const sal_Unicode * language, const sal_Unicode * country, const sal_Unicode * variant );

/**
 * Getter for programmatic name of field,
 * an lowercased two-letter ISO-639 code.
 * @see #getDisplayLanguage
 */
rtl_uString * SAL_CALL rtl_locale_getLanguage( rtl_Locale * This );

/**
 * Getter for programmatic name of field,
 * an uppercased two-letter ISO-3166 code.
 * @see #getDisplayCountry
 */
rtl_uString * SAL_CALL rtl_locale_getCountry( rtl_Locale * This );

/**
 * Getter for programmatic name of field.
 * @see #getDisplayVariant
 */
rtl_uString * SAL_CALL rtl_locale_getVariant( rtl_Locale * This );

/**
 * Returns the hash code of the locale This.
 */
sal_Int32 SAL_CALL rtl_locale_hashCode( rtl_Locale * This );

/**
 * Returns true if the locals are equal, otherwis false.
 */
sal_Int32 SAL_CALL rtl_locale_equals( rtl_Locale * This, rtl_Locale * obj  );

#ifdef __cplusplus
}
#endif

#endif /* _RTL_LOCALE_H_ */


