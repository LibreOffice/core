/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_I18NPOOL_INC_BREAKITERATORIMPL_HXX
#define INCLUDED_I18NPOOL_INC_BREAKITERATORIMPL_HXX

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
#include <cppuhelper/implbase.hxx>

#include <vector>

namespace com { namespace sun { namespace star { namespace i18n {


//  class BreakIterator

class BreakIteratorImpl : public cppu::WeakImplHelper
<
    XBreakIterator,
    com::sun::star::lang::XServiceInfo
>
{
public:
    BreakIteratorImpl( const com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >& rxContext );
    BreakIteratorImpl();
    virtual ~BreakIteratorImpl();

    virtual sal_Int32 SAL_CALL nextCharacters( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& rLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount,
        sal_Int32& nDone ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL previousCharacters( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount,
        sal_Int32& nDone ) throw(com::sun::star::uno::RuntimeException, std::exception) override;

    virtual Boundary SAL_CALL previousWord( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual Boundary SAL_CALL nextWord( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual Boundary SAL_CALL getWordBoundary( const OUString& Text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType, sal_Bool bDirection )
        throw(com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL isBeginWord( const OUString& Text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isEndWord( const OUString& Text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getWordType( const OUString& Text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& nLocale ) throw(com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL beginOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL endOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale ) throw(com::sun::star::uno::RuntimeException, std::exception) override;

    virtual LineBreakResults SAL_CALL getLineBreak( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int32 nMinBreakPos,
        const LineBreakHyphenationOptions& hOptions, const LineBreakUserOptions& bOptions )
        throw(com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Int16 SAL_CALL getScriptType( const OUString& Text, sal_Int32 nPos )
        throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL beginOfScript( const OUString& Text, sal_Int32 nStartPos,
        sal_Int16 ScriptType ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL endOfScript( const OUString& Text, sal_Int32 nStartPos,
        sal_Int16 ScriptType ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL previousScript( const OUString& Text, sal_Int32 nStartPos,
        sal_Int16 ScriptType ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL nextScript( const OUString& Text, sal_Int32 nStartPos,
        sal_Int16 ScriptType ) throw(com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL beginOfCharBlock( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 CharType ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL endOfCharBlock( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 CharType ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL previousCharBlock( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 CharType ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL nextCharBlock( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 CharType ) throw(com::sun::star::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;

    static sal_Int16 SAL_CALL getScriptClass(sal_uInt32 currentChar);
protected:
    Boundary result; // for word break iterator

private:

    struct lookupTableItem {
        lookupTableItem(const com::sun::star::lang::Locale& _aLocale, com::sun::star::uno::Reference < XBreakIterator >& _xBI) : aLocale(_aLocale), xBI(_xBI) {};
        com::sun::star::lang::Locale aLocale;
        com::sun::star::uno::Reference < XBreakIterator > xBI;
    };
    std::vector<lookupTableItem*> lookupTable;
    com::sun::star::lang::Locale aLocale;
    com::sun::star::uno::Reference < XBreakIterator > xBI;
    com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext > m_xContext;

    bool SAL_CALL createLocaleSpecificBreakIterator( const OUString& aLocaleName )
        throw( com::sun::star::uno::RuntimeException );
    com::sun::star::uno::Reference < XBreakIterator > SAL_CALL getLocaleSpecificBreakIterator( const com::sun::star::lang::Locale& rLocale )
        throw( com::sun::star::uno::RuntimeException );

};

} // i18n
} // star
} // sun
} // com


#endif // INCLUDED_I18NPOOL_INC_BREAKITERATORIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
