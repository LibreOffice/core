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
#ifndef __BREAKITERATOR_CTL_HXX__
#define __BREAKITERATOR_CTL_HXX__

#include <breakiterator_unicode.hxx>
#include <xdictionary.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class BreakIterator_CTL
//  ----------------------------------------------------
class BreakIterator_CTL : public BreakIterator_Unicode
{
public:
    BreakIterator_CTL();
    ~BreakIterator_CTL();
    virtual sal_Int32 SAL_CALL previousCharacters(const rtl::OUString& text, sal_Int32 start,
        const lang::Locale& nLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 count,
        sal_Int32& nDone) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL nextCharacters(const rtl::OUString& text, sal_Int32 start,
        const lang::Locale& rLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 count,
        sal_Int32& nDone) throw(com::sun::star::uno::RuntimeException);
    virtual LineBreakResults SAL_CALL getLineBreak( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int32 nMinBreakPos,
        const LineBreakHyphenationOptions& hOptions, const LineBreakUserOptions& bOptions )
        throw(com::sun::star::uno::RuntimeException);
protected:
    rtl::OUString cachedText; // for cell index
    sal_Int32* nextCellIndex;
    sal_Int32* previousCellIndex;
    sal_Int32 cellIndexSize;

    virtual void SAL_CALL makeIndex(const rtl::OUString& text, sal_Int32 pos) throw(com::sun::star::uno::RuntimeException);
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
