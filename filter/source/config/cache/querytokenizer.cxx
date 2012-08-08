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
