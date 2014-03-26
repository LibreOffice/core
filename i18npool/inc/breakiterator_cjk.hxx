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
#ifndef _I18N_BREAKITERATOR_CJK_HXX_
#define _I18N_BREAKITERATOR_CJK_HXX_

#include <breakiterator_unicode.hxx>
#include <xdictionary.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

//  class BreakIterator_CJK

class BreakIterator_CJK : public BreakIterator_Unicode
{
public:
    BreakIterator_CJK();

    Boundary SAL_CALL nextWord( const OUString& Text, sal_Int32 nStartPos,
            const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType)
            throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    Boundary SAL_CALL previousWord( const OUString& Text, sal_Int32 nStartPos,
            const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType)
            throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    Boundary SAL_CALL getWordBoundary( const OUString& Text, sal_Int32 nPos,
            const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType, sal_Bool bDirection )
            throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    LineBreakResults SAL_CALL getLineBreak( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int32 nMinBreakPos,
        const LineBreakHyphenationOptions& hOptions, const LineBreakUserOptions& bOptions )
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
    xdictionary *dict;
    OUString hangingCharacters;
};

#define BREAKITERATOR_CJK( lang ) \
class BreakIterator_##lang : public BreakIterator_CJK {\
public:\
    BreakIterator_##lang (); \
    ~BreakIterator_##lang (); \
};

#ifdef BREAKITERATOR_ALL
BREAKITERATOR_CJK( zh )
BREAKITERATOR_CJK( zh_TW )
BREAKITERATOR_CJK( ja )
BREAKITERATOR_CJK( ko )
#endif
#undef BREAKITERATOR__CJK

} } } }

#endif // _I18N_BREAKITERATOR_CJK_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
