/*************************************************************************
 *
 *  $RCSfile: cclass_unicode.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 11:02:43 $
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
#ifndef _I18N_CCLASS_UNICODE_HXX_
#define _I18N_CCLASS_UNICODE_HXX_

#include <com/sun/star/i18n/XNativeNumberSupplier.hpp>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <com/sun/star/i18n/XLocaleData.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <com/sun/star/lang/XServiceInfo.hpp>

#define TRANSLITERATION_casemapping
#include <transliteration_body.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

typedef sal_uInt32 UPT_FLAG_TYPE;

class cclass_Unicode : public cppu::WeakImplHelper1 < XCharacterClassification >
{
public:
    cclass_Unicode(com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > xSMgr );
    ~cclass_Unicode();

    virtual rtl::OUString SAL_CALL toUpper( const rtl::OUString& Text, sal_Int32 nPos, sal_Int32 nCount,
        const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL toLower( const rtl::OUString& Text, sal_Int32 nPos, sal_Int32 nCount,
        const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL toTitle( const rtl::OUString& Text, sal_Int32 nPos, sal_Int32 nCount,
        const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getType( const rtl::OUString& Text, sal_Int32 nPos )  throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getCharacterDirection( const rtl::OUString& Text, sal_Int32 nPos )
        throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getScript( const rtl::OUString& Text, sal_Int32 nPos ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getCharacterType( const rtl::OUString& text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getStringType( const rtl::OUString& text, sal_Int32 nPos, sal_Int32 nCount,
        const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual ParseResult SAL_CALL parseAnyToken( const rtl::OUString& Text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& rLocale, sal_Int32 nStartCharFlags, const rtl::OUString& userDefinedCharactersStart,
        sal_Int32 nContCharFlags, const rtl::OUString& userDefinedCharactersCont ) throw(com::sun::star::uno::RuntimeException);
    virtual ParseResult SAL_CALL parsePredefinedToken( sal_Int32 nTokenType, const rtl::OUString& Text,
        sal_Int32 nPos, const com::sun::star::lang::Locale& rLocale, sal_Int32 nStartCharFlags,
        const rtl::OUString& userDefinedCharactersStart, sal_Int32 nContCharFlags,
        const rtl::OUString& userDefinedCharactersCont ) throw(com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw( com::sun::star::uno::RuntimeException );

protected:
    sal_Char *cClass;

private:
    Transliteration_casemapping *trans;

// --- parser specific (implemented in cclass_unicode_parser.cxx) ---

    enum ScanState
    {
        ssGetChar,
        ssGetValue,
        ssGetWord,
        ssGetWordFirstChar,
        ssGetString,
        ssGetBool,
        ssRewindFromValue,
        ssStopBack,
        ssBounce,
        ssStop
    };

    static const sal_uInt8      nDefCnt;
    static const UPT_FLAG_TYPE  pDefaultParserTable[];
    static const sal_Int32      pParseTokensType[];

    /// Flag values of table.
    static const UPT_FLAG_TYPE  TOKEN_ILLEGAL;
    static const UPT_FLAG_TYPE  TOKEN_CHAR;
    static const UPT_FLAG_TYPE  TOKEN_CHAR_BOOL;
    static const UPT_FLAG_TYPE  TOKEN_CHAR_WORD;
    static const UPT_FLAG_TYPE  TOKEN_CHAR_VALUE;
    static const UPT_FLAG_TYPE  TOKEN_CHAR_STRING;
    static const UPT_FLAG_TYPE  TOKEN_CHAR_DONTCARE;
    static const UPT_FLAG_TYPE  TOKEN_BOOL;
    static const UPT_FLAG_TYPE  TOKEN_WORD;
    static const UPT_FLAG_TYPE  TOKEN_WORD_SEP;
    static const UPT_FLAG_TYPE  TOKEN_VALUE;
    static const UPT_FLAG_TYPE  TOKEN_VALUE_SEP;
    static const UPT_FLAG_TYPE  TOKEN_VALUE_EXP;
    static const UPT_FLAG_TYPE  TOKEN_VALUE_SIGN;
    static const UPT_FLAG_TYPE  TOKEN_VALUE_EXP_VALUE;
    static const UPT_FLAG_TYPE  TOKEN_VALUE_DIGIT;
    static const UPT_FLAG_TYPE  TOKEN_NAME_SEP;
    static const UPT_FLAG_TYPE  TOKEN_STRING_SEP;
    static const UPT_FLAG_TYPE  TOKEN_EXCLUDED;

    /// If and where c occurs in pStr
    static  const sal_Unicode*  StrChr( const sal_Unicode* pStr, sal_Unicode c );


    com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > xMSF;

    /// used for parser only
    com::sun::star::lang::Locale    aParserLocale;
    com::sun::star::uno::Reference < XLocaleData > xLocaleData;
    com::sun::star::uno::Reference < com::sun::star::i18n::XNativeNumberSupplier > xNatNumSup;
    rtl::OUString             aStartChars;
    rtl::OUString             aContChars;
    UPT_FLAG_TYPE*              pTable;
    UPT_FLAG_TYPE*              pStart;
    UPT_FLAG_TYPE*              pCont;
    sal_Int32                   nStartTypes;
    sal_Int32                   nContTypes;
    ScanState                   eState;
    sal_Unicode                 cGroupSep;
    sal_Unicode                 cDecimalSep;

    /// Get corresponding KParseTokens flag for a character
    sal_Int32 getParseTokensType( const sal_Unicode* aStr, sal_Int32 nPos );

    /// Access parser table flags.
    UPT_FLAG_TYPE getFlags( const sal_Unicode* aStr, sal_Int32 nPos );

    /// Access parser flags via International and special definitions.
    UPT_FLAG_TYPE getFlagsExtended( const sal_Unicode* aStr, sal_Int32 nPos );

    /// Access parser table flags for user defined start characters.
    UPT_FLAG_TYPE getStartCharsFlags( sal_Unicode c );

    /// Access parser table flags for user defined continuation characters.
    UPT_FLAG_TYPE getContCharsFlags( sal_Unicode c );

    /// Setup parser table. Calls initParserTable() only if needed.
    void setupParserTable( const com::sun::star::lang::Locale& rLocale, sal_Int32 startCharTokenType,
        const rtl::OUString& userDefinedCharactersStart, sal_Int32 contCharTokenType,
        const rtl::OUString& userDefinedCharactersCont );

    /// Init parser table.
    void initParserTable( const com::sun::star::lang::Locale& rLocale, sal_Int32 startCharTokenType,
        const rtl::OUString& userDefinedCharactersStart, sal_Int32 contCharTokenType,
        const rtl::OUString& userDefinedCharactersCont );

    /// Destroy parser table.
    void destroyParserTable();

    /// Parse a text.
    void parseText( ParseResult& r, const rtl::OUString& rText, sal_Int32 nPos,
        sal_Int32 nTokenType = 0xffffffff );

    /// Setup International class, new'ed only if different from existing.
    sal_Bool setupInternational( const com::sun::star::lang::Locale& rLocale );

    /// Implementation of getCharacterType() for one single character
    sal_Int32 getCharType( sal_Unicode c );

};

} } } }

#endif
