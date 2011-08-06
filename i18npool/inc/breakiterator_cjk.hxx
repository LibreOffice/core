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
#ifndef _I18N_BREAKITERATOR_CJK_HXX_
#define _I18N_BREAKITERATOR_CJK_HXX_

#include <breakiterator_unicode.hxx>
#include <xdictionary.hxx>

namespace com { namespace sun { namespace star { namespace i18n {
//  ----------------------------------------------------
//  class BreakIterator_CJK
//  ----------------------------------------------------
class BreakIterator_CJK : public BreakIterator_Unicode
{
public:
    BreakIterator_CJK();

    Boundary SAL_CALL nextWord( const rtl::OUString& Text, sal_Int32 nStartPos,
            const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType)
            throw(com::sun::star::uno::RuntimeException);
    Boundary SAL_CALL previousWord( const rtl::OUString& Text, sal_Int32 nStartPos,
            const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType)
            throw(com::sun::star::uno::RuntimeException);
    Boundary SAL_CALL getWordBoundary( const rtl::OUString& Text, sal_Int32 nPos,
            const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType, sal_Bool bDirection )
            throw(com::sun::star::uno::RuntimeException);
    LineBreakResults SAL_CALL getLineBreak( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int32 nMinBreakPos,
        const LineBreakHyphenationOptions& hOptions, const LineBreakUserOptions& bOptions )
        throw(com::sun::star::uno::RuntimeException);

protected:
    xdictionary *dict;
    rtl::OUString hangingCharacters;
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
