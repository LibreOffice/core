/*************************************************************************
 *
 *  $RCSfile: transliterationwrapper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: er $ $Date: 2001-08-06 10:01:54 $
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

#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#define _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _COM_SUN_STAR_I18N_XTRANSLITERATION_HPP_
#include <com/sun/star/i18n/XTransliteration.hpp>
#endif

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
}}}

namespace utl
{

class TransliterationWrapper
{
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory > xSMgr;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::i18n::XTransliteration > xTrans;
    ::com::sun::star::lang::Locale aLocale;
    sal_uInt32 nType;
    sal_uInt16 nLanguage;
    mutable sal_Bool bFirstCall;

                                // not implemented, prevent usage
    TransliterationWrapper( const TransliterationWrapper& );
    TransliterationWrapper& operator=( const TransliterationWrapper& );

    void loadModuleImpl() const;
    void setLanguageLocaleImpl( sal_uInt16 nLang );

public:
    TransliterationWrapper( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory > & xSF,
                    sal_uInt32 nType );

    ~TransliterationWrapper();

    // get current Locale / Language
    const ::com::sun::star::lang::Locale& getLocale() const { return aLocale;}
    sal_uInt16 getLanguage() const { return nLanguage; }

    sal_uInt32 getType() const { return nType; }

    sal_Bool needLanguageForTheMode() const;

    /** set a new language and load the corresponding transliteration module if
        needed for the mode set with nType in the ctor */
    void loadModuleIfNeeded( sal_uInt16 nLang );

    // Wrapper implementations of class Transliteration
    String transliterate( const String& rStr, sal_uInt16 nLanguage,
                        xub_StrLen nStart, xub_StrLen nEnd,
                        ::com::sun::star::uno::Sequence <long>* pOffset );

    /** Does not only return true if the two strings are equal per this
        translation but also if one string matches the start of the other, i.e.
        equals( "a", 0, 1, nMatch1, "aaa", 0, 3, nMatch2 )
        returns true and nMatch:=1 and nMatch2:=3
     */
    sal_Bool equals(
        const String& rStr1, sal_Int32 nPos1, sal_Int32 nCount1, sal_Int32& nMatch1,
        const String& rStr2, sal_Int32 nPos2, sal_Int32 nCount2, sal_Int32& nMatch2 ) const;

    sal_Int32 compareSubstring(
        const String& rStr1, sal_Int32 nOff1, sal_Int32 nLen1,
        const String& rStr2, sal_Int32 nOff2, sal_Int32 nLen2 ) const;

    sal_Int32 compareString( const String& rStr1, const String& rStr2 ) const;


    // helpers

    /** If two strings are really equal as per this translation, and not just
        one string is matching the start of the other. Use this method instead
        of compareString()==0 because it is much faster.
     */
    sal_Bool isEqual( const String& rStr1, const String& rStr2 ) const
        {
            sal_Int32 nMatch1, nMatch2;
            sal_Bool bMatch = equals(
                rStr1, 0, rStr1.Len(), nMatch1,
                rStr2, 0, rStr2.Len(), nMatch2 );
            return bMatch && nMatch1 == nMatch2 && nMatch1 == rStr1.Len() &&
                nMatch2 == rStr2.Len();
        }

    /** If string rStr1 matches the start of string rStr2, i.e. "a" in "aaa"
     */
    sal_Bool isMatch( const String& rStr1, const String& rStr2 ) const
        {
            sal_Int32 nMatch1, nMatch2;
            sal_Bool bMatch = equals(
                rStr1, 0, rStr1.Len(), nMatch1,
                rStr2, 0, rStr2.Len(), nMatch2 );
            return bMatch && nMatch1 <= nMatch2 && nMatch1 == rStr1.Len();
        }
};

// ............................................................................
}       // namespace utl
// ............................................................................

#endif
