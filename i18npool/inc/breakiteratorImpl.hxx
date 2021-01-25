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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <cppuhelper/implbase.hxx>

#include <vector>

namespace com::sun::star::uno { class XComponentContext; }

namespace i18npool {



class BreakIteratorImpl : public cppu::WeakImplHelper
<
    css::i18n::XBreakIterator,
    css::lang::XServiceInfo
>
{
public:
    BreakIteratorImpl( const css::uno::Reference < css::uno::XComponentContext >& rxContext );
    BreakIteratorImpl();
    virtual ~BreakIteratorImpl() override;

    virtual sal_Int32 SAL_CALL nextCharacters( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& rLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount,
        sal_Int32& nDone ) override;
    virtual sal_Int32 SAL_CALL previousCharacters( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& nLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount,
        sal_Int32& nDone ) override;

    virtual css::i18n::Boundary SAL_CALL previousWord( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& nLocale, sal_Int16 WordType) override;
    virtual css::i18n::Boundary SAL_CALL nextWord( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& nLocale, sal_Int16 WordType) override;
    virtual css::i18n::Boundary SAL_CALL getWordBoundary( const OUString& Text, sal_Int32 nPos,
        const css::lang::Locale& nLocale, sal_Int16 WordType, sal_Bool bDirection ) override;

    virtual sal_Bool SAL_CALL isBeginWord( const OUString& Text, sal_Int32 nPos,
        const css::lang::Locale& nLocale, sal_Int16 WordType ) override;
    virtual sal_Bool SAL_CALL isEndWord( const OUString& Text, sal_Int32 nPos,
        const css::lang::Locale& nLocale, sal_Int16 WordType ) override;
    virtual sal_Int16 SAL_CALL getWordType( const OUString& Text, sal_Int32 nPos,
        const css::lang::Locale& nLocale ) override;

    virtual sal_Int32 SAL_CALL beginOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& nLocale ) override;
    virtual sal_Int32 SAL_CALL endOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& nLocale ) override;

    virtual css::i18n::LineBreakResults SAL_CALL getLineBreak( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& nLocale, sal_Int32 nMinBreakPos,
        const css::i18n::LineBreakHyphenationOptions& hOptions,
        const css::i18n::LineBreakUserOptions& bOptions ) override;

    virtual sal_Int16 SAL_CALL getScriptType( const OUString& Text, sal_Int32 nPos ) override;
    virtual sal_Int32 SAL_CALL beginOfScript( const OUString& Text, sal_Int32 nStartPos,
        sal_Int16 ScriptType ) override;
    virtual sal_Int32 SAL_CALL endOfScript( const OUString& Text, sal_Int32 nStartPos,
        sal_Int16 ScriptType ) override;
    virtual sal_Int32 SAL_CALL previousScript( const OUString& Text, sal_Int32 nStartPos,
        sal_Int16 ScriptType ) override;
    virtual sal_Int32 SAL_CALL nextScript( const OUString& Text, sal_Int32 nStartPos,
        sal_Int16 ScriptType ) override;

    virtual sal_Int32 SAL_CALL beginOfCharBlock( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& nLocale, sal_Int16 CharType ) override;
    virtual sal_Int32 SAL_CALL endOfCharBlock( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& nLocale, sal_Int16 CharType ) override;
    virtual sal_Int32 SAL_CALL previousCharBlock( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& nLocale, sal_Int16 CharType ) override;
    virtual sal_Int32 SAL_CALL nextCharBlock( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& nLocale, sal_Int16 CharType ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    static sal_Int16 getScriptClass(sal_uInt32 currentChar);
protected:
    css::i18n::Boundary result; // for word break iterator

private:

    struct lookupTableItem {
        lookupTableItem(const css::lang::Locale& _aLocale, css::uno::Reference < XBreakIterator > const & _xBI) : aLocale(_aLocale), xBI(_xBI) {};
        css::lang::Locale aLocale;
        css::uno::Reference < XBreakIterator > xBI;
    };
    std::vector<lookupTableItem>                        lookupTable;
    css::lang::Locale                                   aLocale;
    css::uno::Reference < XBreakIterator >              xBI;
    css::uno::Reference < css::uno::XComponentContext > m_xContext;

    /// @throws css::uno::RuntimeException
    bool createLocaleSpecificBreakIterator( const OUString& aLocaleName );
    /// @throws css::uno::RuntimeException
    const css::uno::Reference < XBreakIterator > & getLocaleSpecificBreakIterator( const css::lang::Locale& rLocale );

};

} // i18npool


#endif // INCLUDED_I18NPOOL_INC_BREAKITERATORIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
