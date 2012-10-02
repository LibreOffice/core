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


#include "querytokenizer.hxx"


namespace filter{
    namespace config{




QueryTokenizer::QueryTokenizer(const ::rtl::OUString& sQuery)
    : m_bValid(sal_True)
{
    sal_Int32 token = 0;
    while(token != -1)
    {
        ::rtl::OUString sToken = sQuery.getToken(0, ':', token);
        if (!sToken.isEmpty())
        {
            sal_Int32 equal = sToken.indexOf('=');

            if (equal == 0)
                m_bValid = sal_False;
            OSL_ENSURE(m_bValid, "QueryTokenizer::QueryTokenizer()\nFound non boolean query parameter ... but its key is empty. Will be ignored!\n");

            ::rtl::OUString sKey;
            ::rtl::OUString sVal;

            sKey = sToken;
            if (equal > 0)
            {
                sKey = sToken.copy(0      , equal                       );
                sVal = sToken.copy(equal+1, sToken.getLength()-(equal+1));
            }

            if (find(sKey) != end())
                m_bValid = sal_False;
            OSL_ENSURE(m_bValid, "QueryTokenizer::QueryTokenizer()\nQuery contains same param more then once. Last one wins :-)\n");

            (*this)[sKey] = sVal;
        }
    }
}



QueryTokenizer::~QueryTokenizer()
{
    /*TODO*/
}



sal_Bool QueryTokenizer::valid() const
{
    return m_bValid;
}

    } // namespace config
} // namespace filter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
