/*************************************************************************
 *
 *  $RCSfile: defaultnumberingprovider.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: khong $ $Date: 2002-07-11 00:01:31 $
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

#ifndef _I18N_DEFAULT_NUMBERING_PROVIDER_HXX_
#include <defaultnumberingprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_STYLE_NUMBERINGTYPE_HPP_
#include <com/sun/star/style/NumberingType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HPP_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#include <localedata.hxx>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <rtl/ustring.hxx>
#include <tools/string.hxx>
#include <com/sun/star/i18n/XTransliteration.hpp>
#include <com/sun/star/i18n/TransliterationType.hpp>
#include <com/sun/star/i18n/TransliterationModulesNew.hpp>
#include <com/sun/star/i18n/XLocaleData.hpp>
#include <rtl/ustring.hxx>
#include <tools/string.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

DefaultNumberingProvider::DefaultNumberingProvider( const Reference < XMultiServiceFactory >& xMSF ) : xSMgr(xMSF)
{
    translit = new TransliterationImpl(xMSF);
}

DefaultNumberingProvider::~DefaultNumberingProvider()
{
    delete translit;
}

Sequence< Reference<container::XIndexAccess> >
DefaultNumberingProvider::getDefaultOutlineNumberings(const Locale& rLocale ) throw(RuntimeException)
{
     return LocaleData().getOutlineNumberingLevels( rLocale );
}

Sequence< Sequence<beans::PropertyValue> >
DefaultNumberingProvider::getDefaultContinuousNumberingLevels( const Locale& rLocale ) throw(RuntimeException)
{
     return LocaleData().getContinuousNumberingLevels( rLocale );
}


OUString toRoman( sal_Int32 n )
{

//      i, ii, iii, iv, v, vi, vii, vii, viii, ix
//                          (Dummy),1000,500,100,50,10,5,1
    static const sal_Char coRomanArr[] = "MDCLXVI--";   // +2 Dummy entries !!
    const sal_Char* cRomanStr = coRomanArr;
    USHORT nMask = 1000;
    xub_StrLen nOver1000 = n / nMask;
    n -= ( nOver1000 * nMask );

    String sTmp;
    sTmp.Fill( nOver1000, *coRomanArr );

    while( nMask )
    {
        BYTE nZahl = BYTE( n / nMask );
        BYTE nDiff = 1;
        n %= nMask;

        if( 5 < nZahl )
        {
            if( nZahl < 9 )
                sTmp += *(cRomanStr-1);
            ++nDiff;
            nZahl -= 5;
        }
        switch( nZahl )
        {
        case 3: sTmp += *cRomanStr;     //no break!
        case 2: sTmp += *cRomanStr;     //no break!
        case 1: sTmp += *cRomanStr;     break;
        case 4: ( sTmp += *cRomanStr ) += *(cRomanStr-nDiff); break;
        case 5: sTmp += *(cRomanStr-nDiff); break;
        }

        nMask /= 10;            // to the next decade
        cRomanStr += 2;
    }
    return OUString( sTmp );
}

static
const char* expected_name( int i, int last )
{
     if(0);
     else if( i==0    ) return "Prefix";
     else if( i==1    ) return "NumberingType";
     else if( i==2    ) return "Suffix";
     else if( i==last ) return "Value";
     else { assert(0); return ""; }
}
static
const char* expected_type( int i, int last )
{
     if(0);
     else if( i==0    ) return "OUString";
     else if( i==1    ) return "sal_Int16";
     else if( i==2    ) return "OUString";
     else if( i==last ) return "sal_Int32";
     else { assert(0); return ""; }
}
static
void failedToConvert( int i, int last )
{
     throw IllegalArgumentException();
}

static
void lcl_formatChars( char A, int n, OUString& s )
{
     // string representation of n is appended to s.
     // if A=='A' then 0=>A, 1=>B, ..., 25=>Z, 26=>AA, 27=>AB, ...
     // if A=='a' then 0=>a, 1=>b, ..., 25=>z, 26=>aa, 27=>ab, ...

     if( n>=26 ) lcl_formatChars( A, (n-26)/26, s );

     s += OUString::valueOf( (sal_Unicode) (( n%26 ) + A) );
}

static
should_ignore( OUString s )
{
    // return true if blank or null
    return s.compareToAscii(" ")==0 || (s.getLength()>0 && s[0]==0);
}

static
Any getPropertyByName( const Sequence<beans::PropertyValue>& aProperties,
                                                const char* name, sal_Bool bRequired )
{
    for( int i=0; i<aProperties.getLength(); i++ )
    {
        if( aProperties[i].Name.equalsAscii(name) )
        {
            return aProperties[i].Value;
        }
    }
    if(bRequired)
        throw IllegalArgumentException();
    return Any();
}

//XNumberingFormatter
OUString
DefaultNumberingProvider::makeNumberingString( const Sequence<beans::PropertyValue>& aProperties,
                                               const Locale& aLocale )
     throw( IllegalArgumentException, RuntimeException )
{
     // the Sequence of PropertyValues is expected to have at least 4 elements:
     // elt Name              Type             purpose
     // -----------------------------------------------------------------
     //
     // 0.  "Prefix"          OUString
     // 1.  "NumberingType"   sal_Int16        type of formatting from style::NumberingType (roman, arabic, etc)
     // 2.  "Suffix"          OUString
     // ... ...               ...
     // n.  "Value"           sal_Int32        the number to be formatted
     // example:
     // given the Sequence { '(', NumberingType::ROMAN_UPPER, ')', ..., 7 }
     // makeNumberingString() returns the string "(VII)".

     // Q: why is the type of numType sal_Int16 instead of style::NumberingType?
     // A: an Any can't hold a style::NumberingType for some reason.
    //  add.: style::NumberingType holds constants of type sal_Int16, it's not an enum type

     OUString  prefix;
     sal_Int16        numType; // type of formatting from style::NumberingType (roman, arabic, etc)
     OUString  suffix;
     sal_Int32        number = -12345; // the number that needs to be formatted.

     int nProperties = aProperties.getLength();
     int last        = nProperties-1;

     try
     {
        getPropertyByName(aProperties, "Prefix", sal_False)      >>=prefix;
     }
     catch (Exception&)
     {
        //prefix _must_ be empty here!
     }
     try
     {
        getPropertyByName(aProperties, "Suffix", sal_False)      >>=suffix;
     }
     catch (Exception&)
     {
        //suffix _must_ be empty here!
     }
     try
     {
        getPropertyByName(aProperties, "NumberingType", sal_True)   >>=numType;
     }
     catch (Exception& )
     {
        numType = -1;
     }
     try
     {
        getPropertyByName(aProperties, "Value", sal_True)       >>=number;
     }
     catch (Exception& )
     {
        number = -1;
     }

     if( number <= 0 )
     {
          throw IllegalArgumentException();
     }

     // start empty
     OUString result;

     // this should be locale dependent.
     // for now, assume en_US

     // en_US
     {
          // append prefix
          if( !should_ignore(prefix) ) result += prefix;

          // append formatted number
          using namespace style::NumberingType;
          switch( numType )
          {
          case CHARS_UPPER_LETTER:
               lcl_formatChars( 'A', number-1, result ); // 1=>A, 2=>B, ..., 26=>Z, 27=>AA, 28=>AB, ...
               break;
          case CHARS_LOWER_LETTER:
               lcl_formatChars( 'a', number-1, result );
               break;
          case ROMAN_UPPER:
               result += toRoman( number );
               break;
          case ROMAN_LOWER:
               result += toRoman( number ).toAsciiLowerCase();
               break;
          case ARABIC:
               result += OUString::valueOf( number );
               break;
          case NUMBER_NONE:
               return OUString::createFromAscii(""); // ignore prefix and suffix
               break;
          case CHAR_SPECIAL:
               // apparently, we're supposed to return an empty string in this case...
               return OUString::createFromAscii(""); // ignore prefix and suffix
               break;
          case PAGE_DESCRIPTOR:
          case BITMAP:
               assert(0);
               throw IllegalArgumentException();
               break;
          case CHARS_UPPER_LETTER_N:
               {
                    sal_Unicode c    = ((--number)%26) + 'A';
                    int repeat_count = number / 26 + 1;
                    for( int i=0; i<repeat_count; i++ )
                        result += OUString::valueOf( c );
               }
               break;
          case CHARS_LOWER_LETTER_N:
               {
                    sal_Unicode c    = ((--number)%26) + 'a';
                    int repeat_count = number / 26 + 1;
                    for( int i=0; i<repeat_count; i++ )
                        result += OUString::valueOf( c );
               }
               break;
#ifdef DEBUG
        case 20:
        {
            if(number >= 0 && number <= 5)
            {
                    static const sal_Char* aTestNumbers[] =
                    {
                        "None","First","Second","Third","Fourth","Fifth"
                    };
                    result = OUString::createFromAscii(aTestNumbers[number]);
            }
            else
                result = OUString::createFromAscii("too big!");
        }
        break;

#endif

          case TRANSLITERATION:
           try {
            OUString &tmp = OUString::valueOf( number );
            OUString transliteration;
            getPropertyByName(aProperties, "Transliteration", sal_True) >>= transliteration;
            translit->loadModuleByImplName(transliteration, aLocale);
            Sequence< long > offset;
            result += translit->transliterate(tmp, 0, tmp.getLength(), offset);
           } catch (Exception& ) {
                    assert(0);
                    throw IllegalArgumentException();
           }
               break;

          default:
               assert(0);
               throw IllegalArgumentException();
               break;
          }

          // append suffix
      if( !should_ignore(suffix) ) result += suffix;
     }
     // en_US


     return result;
}
/* -----------------------------21.02.01 15:57--------------------------------

 ---------------------------------------------------------------------------*/
struct Supported_NumberingType
{
    sal_Int16       nType;
    const sal_Char* cSymbol;
};
static const Supported_NumberingType aSupportedTypes[] =
{
    {style::NumberingType::CHARS_UPPER_LETTER,  "A"},
    {style::NumberingType::CHARS_LOWER_LETTER,  "a"},
    {style::NumberingType::ROMAN_UPPER,             "I"},
    {style::NumberingType::ROMAN_LOWER,             "i"},
    {style::NumberingType::ARABIC,              "1"},
    {style::NumberingType::NUMBER_NONE,             "''"},
    {style::NumberingType::CHAR_SPECIAL,            "Bullet"},
    {style::NumberingType::PAGE_DESCRIPTOR,         "Page"},
    {style::NumberingType::BITMAP,              "Bitmap"},
    {style::NumberingType::CHARS_UPPER_LETTER_N,    "AAA"},
    {style::NumberingType::CHARS_LOWER_LETTER_N,    "aaa"},
#ifdef DEBUG
    {20,    "First"},
#endif
    {style::NumberingType::CHARS_LOWER_LETTER_N,    "Transliteration"}
};
    static const sal_Int32 nSupported_NumberingTypes = sizeof(aSupportedTypes) / sizeof(Supported_NumberingType);
/* -----------------------------21.02.01 15:57--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< sal_Int16 > DefaultNumberingProvider::getSupportedNumberingTypes(  )
                throw(RuntimeException)
{
    Sequence< sal_Int16 > aRet(nSupported_NumberingTypes );
    sal_Int16* pArray = aRet.getArray();
    for(sal_Int16 i = 0; i < nSupported_NumberingTypes; i++)
        pArray[i] = aSupportedTypes[i].nType;
    return aRet;
}
/* -----------------------------21.02.01 15:57--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int16 DefaultNumberingProvider::getNumberingType( const OUString& rNumberingIdentifier )
                throw(RuntimeException)
{
    for(sal_Int16 i = 0; i < nSupported_NumberingTypes; i++)
        if(!rNumberingIdentifier.compareToAscii(aSupportedTypes[i].cSymbol))
            return aSupportedTypes[i].nType;
    throw RuntimeException();
    return -1;
}
/* -----------------------------21.02.01 15:57--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool DefaultNumberingProvider::hasNumberingType( const OUString& rNumberingIdentifier )
                throw(RuntimeException)
{
    for(sal_Int16 i = 0; i < nSupported_NumberingTypes; i++)
        if(!rNumberingIdentifier.compareToAscii(aSupportedTypes[i].cSymbol))
            return sal_True;
    return sal_False;
}
/* -----------------------------21.02.01 15:57--------------------------------

 ---------------------------------------------------------------------------*/
OUString DefaultNumberingProvider::getNumberingIdentifier( sal_Int16 nNumberingType )
                throw(RuntimeException)
{
    for(sal_Int16 i = 0; i < nSupported_NumberingTypes; i++)
        if(nNumberingType == aSupportedTypes[i].nType)
            return OUString::createFromAscii(aSupportedTypes[i].cSymbol);
    return OUString();
}
/* -----------------------------05.07.01 13:34--------------------------------

 ---------------------------------------------------------------------------*/
const sal_Char cDefaultNumberingProvider[] = "com.sun.star.text.DefaultNumberingProvider";
OUString DefaultNumberingProvider::getImplementationName(void)
                throw( RuntimeException )
{
    return OUString::createFromAscii(cDefaultNumberingProvider);
}
/* -----------------------------05.07.01 13:34--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool DefaultNumberingProvider::supportsService(const rtl::OUString& rServiceName)
                throw( RuntimeException )
{
    return rServiceName.equalsAscii(cDefaultNumberingProvider);
}
/* -----------------------------05.07.01 13:34--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > DefaultNumberingProvider::getSupportedServiceNames(void)
                throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cDefaultNumberingProvider);
    return aRet;
}

} } } }
