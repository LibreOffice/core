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

#include <transliteration_Ignore.hxx>
#include <transliteration_OneToOne.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace com { namespace sun { namespace star { namespace i18n {

OUString SAL_CALL
ignoreSize_ja_JP::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException, std::exception)
{
    Reference< smallToLarge_ja_JP > t1(new smallToLarge_ja_JP);
    return t1->transliterate(inStr, startPos, nCount, offset);
}


Sequence< OUString > SAL_CALL
ignoreSize_ja_JP::transliterateRange( const OUString& str1, const OUString& str2 )
  throw(RuntimeException, std::exception)
{
    Reference< smallToLarge_ja_JP > t1(new smallToLarge_ja_JP);
    Reference< largeToSmall_ja_JP > t2(new largeToSmall_ja_JP);

    return transliteration_Ignore::transliterateRange(str1, str2, *t1.get(), *t2.get());
}

sal_Unicode SAL_CALL
ignoreSize_ja_JP::transliterateChar2Char( sal_Unicode inChar) throw(RuntimeException, MultipleCharsOutputException, std::exception)
{
    Reference< smallToLarge_ja_JP > t1(new smallToLarge_ja_JP);
    return t1->transliterateChar2Char(inChar);
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
