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
#ifndef _I18N_BREAKITERATOR_HXX_
#define _I18N_BREAKITERATOR_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/BreakType.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/CharType.hpp>
#include <com/sun/star/i18n/XLocaleData.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations

#include <vector>

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class BreakIterator
//  ----------------------------------------------------
class BreakIteratorImpl : public cppu::WeakImplHelper2
<
    XBreakIterator,
    com::sun::star::lang::XServiceInfo
>
{
public:
    BreakIteratorImpl( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    BreakIteratorImpl();
    ~BreakIteratorImpl();

    virtual sal_Int32 SAL_CALL nextCharacters( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& rLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount,
        sal_Int32& nDone ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL previousCharacters( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount,
        sal_Int32& nDone ) throw(com::sun::star::uno::RuntimeException);

    virtual Boundary SAL_CALL previousWord( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType) throw(com::sun::star::uno::RuntimeException);
    virtual Boundary SAL_CALL nextWord( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType) throw(com::sun::star::uno::RuntimeException);
    virtual Boundary SAL_CALL getWordBoundary( const rtl::OUString& Text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType, sal_Bool bDirection )
        throw(com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isBeginWord( const rtl::OUString& Text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isEndWord( const rtl::OUString& Text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getWordType( const rtl::OUString& Text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& nLocale ) throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL beginOfSentence( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL endOfSentence( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale ) throw(com::sun::star::uno::RuntimeException);

    virtual LineBreakResults SAL_CALL getLineBreak( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int32 nMinBreakPos,
        const LineBreakHyphenationOptions& hOptions, const LineBreakUserOptions& bOptions )
        throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int16 SAL_CALL getScriptType( const rtl::OUString& Text, sal_Int32 nPos )
        throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL beginOfScript( const rtl::OUString& Text, sal_Int32 nStartPos,
        sal_Int16 ScriptType ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL endOfScript( const rtl::OUString& Text, sal_Int32 nStartPos,
        sal_Int16 ScriptType ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL previousScript( const rtl::OUString& Text, sal_Int32 nStartPos,
        sal_Int16 ScriptType ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL nextScript( const rtl::OUString& Text, sal_Int32 nStartPos,
        sal_Int16 ScriptType ) throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL beginOfCharBlock( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 CharType ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL endOfCharBlock( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 CharType ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL previousCharBlock( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 CharType ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL nextCharBlock( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 CharType ) throw(com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw( com::sun::star::uno::RuntimeException );

    static sal_Int16 SAL_CALL getScriptClass(sal_uInt32 currentChar);
protected:
    Boundary result; // for word break iterator

private :

    struct lookupTableItem {
        lookupTableItem(const com::sun::star::lang::Locale& _aLocale, com::sun::star::uno::Reference < XBreakIterator >& _xBI) : aLocale(_aLocale), xBI(_xBI) {};
        com::sun::star::lang::Locale aLocale;
        com::sun::star::uno::Reference < XBreakIterator > xBI;
    };
    std::vector<lookupTableItem*> lookupTable;
    com::sun::star::lang::Locale aLocale;
    com::sun::star::uno::Reference < XBreakIterator > xBI;

    com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > xMSF;
    sal_Bool SAL_CALL createLocaleSpecificBreakIterator( const rtl::OUString& aLocaleName )
        throw( com::sun::star::uno::RuntimeException );
    com::sun::star::uno::Reference < XBreakIterator > SAL_CALL getLocaleSpecificBreakIterator( const com::sun::star::lang::Locale& rLocale )
        throw( com::sun::star::uno::RuntimeException );
    const com::sun::star::lang::Locale& SAL_CALL getLocaleByScriptType(const com::sun::star::lang::Locale& rLocale, const rtl::OUString& Text,
        sal_Int32 nStartPos, sal_Bool forward, sal_Bool skipWhiteSpace)
        throw(com::sun::star::uno::RuntimeException);

};

} // i18n
} // star
} // sun
} // com


#endif // _I18N_BREAKITERATOR_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
