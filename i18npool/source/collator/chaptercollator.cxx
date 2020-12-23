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

#include <cppuhelper/supportsservice.hxx>
#include <chaptercollator.hxx>
#include <com/sun/star/i18n/KCharacterType.hpp>
#include <com/sun/star/i18n/ParseResult.hpp>
#include <com/sun/star/i18n/CharacterClassification.hpp>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;

namespace i18npool {

ChapterCollator::ChapterCollator( const Reference < XComponentContext >& rxContext ) : CollatorImpl(rxContext)
{
    cclass = CharacterClassification::create( rxContext );
}

ChapterCollator::~ChapterCollator()
{
}

sal_Int32 SAL_CALL
ChapterCollator::compareString( const OUString& s1, const OUString& s2)
{
    return compareSubstring(s1, 0, s1.getLength(),  s2, 0, s2.getLength());
}

#define DIGIT KCharacterType::DIGIT

sal_Int32 SAL_CALL
ChapterCollator::compareSubstring( const OUString& str1, sal_Int32 off1, sal_Int32 len1,
    const OUString& str2, sal_Int32 off2, sal_Int32 len2)
{
    if( len1 <= 1 || len2 <= 1 || ! cclass.is() )
        return CollatorImpl::compareSubstring( str1, off1,  len1, str2, off2, len2 );

    sal_Int32 i1, i2;
    for (i1 = len1; i1 && (cclass->getCharacterType(str1, off1+i1-1, nLocale) & DIGIT); i1--) ;
    for (i2 = len2; i2 && (cclass->getCharacterType(str2, off2+i2-1, nLocale) & DIGIT); i2--) ;

    sal_Int32 ans = CollatorImpl::compareSubstring(str1, off1, i1, str2, off2, i2);
    if( ans != 0 )
        return ans;

    const OUString aAddAllowed("?");
    ParseResult res1, res2;
    // since parseAnyToken does not take length as parameter, we have to copy
    // it to a temp. string.
    OUString s1 = str1.copy(off1+i1, len1-i1), s2 = str2.copy(off2+i2, len2-i2);
    res1 = cclass->parseAnyToken( s1, 0, nLocale, DIGIT, aAddAllowed, DIGIT, aAddAllowed );
    res2 = cclass->parseAnyToken( s2, 0, nLocale, DIGIT, aAddAllowed, DIGIT, aAddAllowed );

    return res1.Value == res2.Value ? 0 : res1.Value > res2.Value ? 1 : -1;
}

const OUStringLiteral cChapCollator = u"com.sun.star.i18n.ChapterCollator";

OUString SAL_CALL
ChapterCollator::getImplementationName()
{
    return cChapCollator;
}

sal_Bool SAL_CALL
ChapterCollator::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL
ChapterCollator::getSupportedServiceNames()
{
    Sequence< OUString > aRet { cChapCollator };
    return aRet;
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
