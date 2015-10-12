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
#ifndef INCLUDED_I18NPOOL_INC_BREAKITERATOR_CTL_HXX
#define INCLUDED_I18NPOOL_INC_BREAKITERATOR_CTL_HXX

#include <breakiterator_unicode.hxx>
#include <xdictionary.hxx>

namespace com { namespace sun { namespace star { namespace i18n {


//  class BreakIterator_CTL

class BreakIterator_CTL : public BreakIterator_Unicode
{
public:
    BreakIterator_CTL();
    virtual ~BreakIterator_CTL();
    virtual sal_Int32 SAL_CALL previousCharacters(const OUString& text, sal_Int32 start,
        const lang::Locale& nLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 count,
        sal_Int32& nDone) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL nextCharacters(const OUString& text, sal_Int32 start,
        const lang::Locale& rLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 count,
        sal_Int32& nDone) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual LineBreakResults SAL_CALL getLineBreak( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int32 nMinBreakPos,
        const LineBreakHyphenationOptions& hOptions, const LineBreakUserOptions& bOptions )
        throw(com::sun::star::uno::RuntimeException, std::exception) override;
protected:
    OUString cachedText; // for cell index
    sal_Int32* nextCellIndex;
    sal_Int32* previousCellIndex;
    sal_Int32 cellIndexSize;

    virtual void SAL_CALL makeIndex(const OUString& text, sal_Int32 pos) throw(com::sun::star::uno::RuntimeException);
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
