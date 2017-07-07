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

#include <comphelper/sequence.hxx>

namespace comphelper
{
css::uno::Sequence<sal_Int16> findValue(const css::uno::Sequence< OUString >& _rList, const OUString& _rValue, bool _bOnlyFirst)
{
    sal_Int32 nLength = _rList.getLength();

    if( _bOnlyFirst )
    {
        // at which position do I find the value?
        sal_Int32 nPos = -1;
        const OUString* pTArray = _rList.getConstArray();
        for (sal_Int32 i = 0; i < nLength; ++i, ++pTArray)
        {
            if( *pTArray == _rValue )
            {
                nPos = i;
                break;
            }
        }

        // fill sequence
        if( nPos>-1 )
        {
            css::uno::Sequence<sal_Int16> aRetSeq( 1 );
            aRetSeq.getArray()[0] = (sal_Int16)nPos;

            return aRetSeq;
        }

        return css::uno::Sequence<sal_Int16>();

    }
    else
    {
        css::uno::Sequence<sal_Int16> aRetSeq( nLength );
        sal_Int16* pReturn = aRetSeq.getArray();

        // how often does the value occur?
        const OUString* pTArray = _rList.getConstArray();
        for (sal_Int32 i = 0; i < nLength; ++i, ++pTArray)
        {
            if( *pTArray == _rValue )
            {
                *pReturn = (sal_Int16)i;
                ++pReturn;
            }
        }

        aRetSeq.realloc(pReturn - aRetSeq.getArray());

        return aRetSeq;
    }
}
}   // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
