/*************************************************************************
 *
 *  $RCSfile: defaultnumberingprovider.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2001-08-30 23:33:16 $
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

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::rtl;

inline
rtl::OUString C2U( const char* s )
{
  return OUString::createFromAscii(s);
}

static
void printf_String( const char* fmt, ::rtl::OUString str )
{
     const int max = 1024;
     char buf[ max+1 ];

     for(int i=0; i<str.getLength() && i<max; i++) buf[i] = str[i];
     buf[i]='\0';
     printf( fmt, buf );
}

DefaultNumberingProvider::DefaultNumberingProvider(
    const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >& xMSF )
    : xSMgr(xMSF)
{
}

DefaultNumberingProvider::~DefaultNumberingProvider()
{
}

Sequence< Reference<XIndexAccess> >
DefaultNumberingProvider::getDefaultOutlineNumberings(const Locale& rLocale ) throw(RuntimeException)
{
     return LocaleData().getOutlineNumberingLevels( rLocale );
}

Sequence< Sequence<PropertyValue> >
DefaultNumberingProvider::getDefaultContinuousNumberingLevels( const Locale& rLocale ) throw(RuntimeException)
{
     return LocaleData().getContinuousNumberingLevels( rLocale );
}


struct Level
{
     Level( ::rtl::OUString i0, ::rtl::OUString v0, ::rtl::OUString x0 ) : i( i0 ), v( v0 ), x( x0 ) {}
     ::rtl::OUString i, v, x;
};
struct Level level[] = {
     Level(::rtl::OUString::createFromAscii("I"), ::rtl::OUString::createFromAscii("V"), ::rtl::OUString::createFromAscii("X")),
     Level(::rtl::OUString::createFromAscii("X"), ::rtl::OUString::createFromAscii("L"), ::rtl::OUString::createFromAscii("C")),
     Level(::rtl::OUString::createFromAscii("C"), ::rtl::OUString::createFromAscii("D"), ::rtl::OUString::createFromAscii("M")),
};

::rtl::OUString calcDigit( int d, int l )
{
     ::rtl::OUString str;
     if (l > 2)
     {
          for (int m=1; m <= d*pow(10, l-3); m++)
               str += ::rtl::OUString::createFromAscii("M");
          return str;
     }
     else
     {
          switch( d )
          {
          case 0: return rtl::OUString::createFromAscii("");
          case 1: return level[l].i;
          case 2: return level[l].i + level[l].i;
          case 3: return level[l].i + level[l].i + level[l].i;
          case 4: return level[l].i + level[l].v;
          case 5: return level[l].v;
          case 6: return level[l].v + level[l].i;
          case 7: return level[l].v + level[l].i + level[l].i;
          case 8: return level[l].v + level[l].i + level[l].i + level[l].i;
          case 9: return level[l].i + level[l].x;
          default:
               assert(0);
           return ::rtl::OUString::createFromAscii("");
          }
     }
}

::rtl::OUString toRoman( sal_Int32 n )
{
     ::rtl::OUString result;

     OUString tmp = OUString::valueOf( n );
     ::rtl::OUString buf = tmp;
     int length = buf.getLength();

     for( int i=0; i<length; i++ )
     {
          result += calcDigit( buf[i]-'0', length-i-1 );
     }

     return result;
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
     else if( i==0    ) return "::rtl::OUString";
     else if( i==1    ) return "sal_Int16";
     else if( i==2    ) return "::rtl::OUString";
     else if( i==last ) return "sal_Int32";
     else { assert(0); return ""; }
}
static
void failedToConvert( int i, int last )
{
     throw lang::IllegalArgumentException();
}

static
void lcl_formatChars( char A, int n, ::rtl::OUString& s )
{
     // string representation of n is appended to s.
     // if A=='A' then 0=>A, 1=>B, ..., 25=>Z, 26=>AA, 27=>AB, ...
     // if A=='a' then 0=>a, 1=>b, ..., 25=>z, 26=>aa, 27=>ab, ...

     if( n>=26 ) lcl_formatChars( A, (n-26)/26, s );

     s += ::rtl::OUString::valueOf( (sal_Unicode) (( n%26 ) + A) );
}

static
should_ignore( ::rtl::OUString s )
{
    // return true if blank or null
    return s.compareToAscii(" ")==0 || (s.getLength()>0 && s[0]==0);
}

static
uno::Any getPropertyByName( const uno::Sequence<beans::PropertyValue>& aProperties,
                                                const char* name, sal_Bool bRequired )
{
    for( int i=0; i<aProperties.getLength(); i++ )
    {
        if( aProperties[i].Name == C2U(name) )
        {
            return aProperties[i].Value;
        }
    }
    if(bRequired)
        throw lang::IllegalArgumentException();
    return uno::Any();
}

void
DefaultNumberingProvider::getTransliteration()
{
    using namespace com::sun::star::uno;
    using namespace com::sun::star::lang;
    using namespace com::sun::star::i18n;

    Reference < XInterface > xI = xSMgr->createInstance(::rtl::OUString::createFromAscii( "com.sun.star.i18n.Transliteration") );

    if ( xI.is() ) {
        Any x = xI->queryInterface(
            ::getCppuType( (const Reference< XTransliteration >*)0) );
        x >>= translit;
    }
}

//XNumberingFormatter
::rtl::OUString
DefaultNumberingProvider::makeNumberingString( const uno::Sequence<beans::PropertyValue>& aProperties,
                                               const lang::Locale& aLocale )
     throw( lang::IllegalArgumentException, uno::RuntimeException )
{
     // the Sequence of PropertyValues is expected to have at least 4 elements:
     // elt Name              Type             purpose
     // -----------------------------------------------------------------
     //
     // 0.  "Prefix"          ::rtl::OUString
     // 1.  "NumberingType"   sal_Int16        type of formatting from style::NumberingType (roman, arabic, etc)
     // 2.  "Suffix"          ::rtl::OUString
     // ... ...               ...
     // n.  "Value"           sal_Int32        the number to be formatted
     // example:
     // given the Sequence { '(', NumberingType::ROMAN_UPPER, ')', ..., 7 }
     // makeNumberingString() returns the string "(VII)".

     // Q: why is the type of numType sal_Int16 instead of style::NumberingType?
     // A: an Any can't hold a style::NumberingType for some reason.
    //  add.: style::NumberingType holds constants of type sal_Int16, it's not an enum type

     ::rtl::OUString  prefix;
     sal_Int16        numType; // type of formatting from style::NumberingType (roman, arabic, etc)
     ::rtl::OUString  suffix;
     sal_Int32        number = -12345; // the number that needs to be formatted.

     int nProperties = aProperties.getLength();
     int last        = nProperties-1;

     try
     {
        ::getPropertyByName(aProperties, "Prefix", sal_False)      >>=prefix;
     }
     catch (Exception&)
     {
        //prefix _must_ be empty here!
     }
     try
     {
        ::getPropertyByName(aProperties, "Suffix", sal_False)      >>=suffix;
     }
     catch (Exception&)
     {
        //suffix _must_ be empty here!
     }
     try
     {
        ::getPropertyByName(aProperties, "NumberingType", sal_True)   >>=numType;
     }
     catch (Exception& )
     {
        numType = -1;
     }
     try
     {
        ::getPropertyByName(aProperties, "Value", sal_True)       >>=number;
     }
     catch (Exception& )
     {
        number = -1;
     }

     if( number <= 0 )
     {
          throw lang::IllegalArgumentException();
     }

     // start empty
     ::rtl::OUString result;

     // this should be locale dependent.
     // for now, assume en_US

     // en_US
     {
          // append prefix
          if( !should_ignore(prefix) ) result += prefix;

          // append formatted number
          using namespace ::com::sun::star::style::NumberingType;
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
               result += ::rtl::OUString::valueOf( number );
               break;
          case NUMBER_NONE:
               return ::rtl::OUString::createFromAscii(""); // ignore prefix and suffix
               break;
          case CHAR_SPECIAL:
               // apparently, we're supposed to return an empty string in this case...
               return ::rtl::OUString::createFromAscii(""); // ignore prefix and suffix
               break;
          case PAGE_DESCRIPTOR:
          case BITMAP:
               assert(0);
               throw lang::IllegalArgumentException();
               break;
          case CHARS_UPPER_LETTER_N:
               {
                    sal_Unicode c    = ((--number)%26) + 'A';
                    int repeat_count = number / 26 + 1;
                    for( int i=0; i<repeat_count; i++ )
                        result += ::rtl::OUString::valueOf( c );
               }
               break;
          case CHARS_LOWER_LETTER_N:
               {
                    sal_Unicode c    = ((--number)%26) + 'a';
                    int repeat_count = number / 26 + 1;
                    for( int i=0; i<repeat_count; i++ )
                        result += ::rtl::OUString::valueOf( c );
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
               {
               ::rtl::OUString &tmp = ::rtl::OUString::valueOf( number );
               ::rtl::OUString transliteration;
           try {
        ::getPropertyByName(aProperties, "Transliteration", sal_True) >>= transliteration;
           } catch (Exception& ) {
        transliteration = ::rtl::OUString::createFromAscii("");
           }
               getTransliteration();
        if ( ! translit.is() ) {
                    throw lang::IllegalArgumentException();
        }

        Sequence < i18n::TransliterationModulesNew > module(1);
               if( !transliteration.compareToAscii("NumToTextLower_zh_CN") ) {
                    module[0]=i18n::TransliterationModulesNew_NumToTextLower_zh_CN;
               } else if( !transliteration.compareToAscii("NumToTextUpper_zh_CN") ) {
                    module[0]=i18n::TransliterationModulesNew_NumToTextUpper_zh_CN;
               } else if( !transliteration.compareToAscii("NumToTextLower_zh_TW") ) {
                    module[0]=i18n::TransliterationModulesNew_NumToTextLower_zh_TW;
               } else if( !transliteration.compareToAscii("NumToTextUpper_zh_TW") ) {
                    module[0]=i18n::TransliterationModulesNew_NumToTextUpper_zh_TW;
               } else if( !transliteration.compareToAscii("NumToTextFormalHangul_ko") ) {
                    module[0]=i18n::TransliterationModulesNew_NumToTextFormalHangul_ko;
               } else if( !transliteration.compareToAscii("NumToTextFormalLower_ko") ) {
                    module[0]=i18n::TransliterationModulesNew_NumToTextFormalLower_ko;
               } else if( !transliteration.compareToAscii("NumToTextFormalUpper_ko") ) {
                    module[0]=i18n::TransliterationModulesNew_NumToTextFormalUpper_ko;
               } else
               {
                    assert(0);
                    throw lang::IllegalArgumentException();
               }
               translit->loadModuleNew( module, aLocale);
               uno::Sequence< long > offset( tmp.getLength()*2 );
               ::rtl::OUString& res = translit->transliterate(tmp, 0, tmp.getLength(), offset);
               result += res;
               result += C2U("~");
               }
               break;

          default:
               assert(0);
               throw lang::IllegalArgumentException();
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
    {NumberingType::CHARS_UPPER_LETTER,     "A"},
    {NumberingType::CHARS_LOWER_LETTER,     "a"},
    {NumberingType::ROMAN_UPPER,            "I"},
    {NumberingType::ROMAN_LOWER,            "i"},
    {NumberingType::ARABIC,                 "1"},
    {NumberingType::NUMBER_NONE,            "''"},
    {NumberingType::CHAR_SPECIAL,           "Bullet"},
    {NumberingType::PAGE_DESCRIPTOR,        "Page"},
    {NumberingType::BITMAP,                 "Bitmap"},
    {NumberingType::CHARS_UPPER_LETTER_N,   "AAA"},
    {NumberingType::CHARS_LOWER_LETTER_N,   "aaa"}
#ifdef DEBUG
    ,{20,   "First"}
#endif
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
            return C2U(aSupportedTypes[i].cSymbol);
    return OUString();
}
/* -----------------------------05.07.01 13:34--------------------------------

 ---------------------------------------------------------------------------*/
OUString DefaultNumberingProvider::getImplementationName(void)
                throw( RuntimeException )
{
    return C2U("i18n::DefaultNumberingProvider");
}
/* -----------------------------05.07.01 13:34--------------------------------

 ---------------------------------------------------------------------------*/
const sal_Char cDefaultNumberingProvider[] = "com.sun.star.text.DefaultNumberingProvider";
sal_Bool DefaultNumberingProvider::supportsService(const rtl::OUString& rServiceName)
                throw( RuntimeException )
{
    return !rServiceName.compareToAscii(cDefaultNumberingProvider);
}
/* -----------------------------05.07.01 13:34--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > DefaultNumberingProvider::getSupportedServiceNames(void)
                throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = C2U(cDefaultNumberingProvider);
    return aRet;
}


