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

#ifndef _ADRPARSE_HXX
#define _ADRPARSE_HXX

#include <tools/list.hxx>
#include <tools/string.hxx>

namespace binfilter {

//============================================================================
struct SvAddressEntry_Impl
{
    UniString m_aAddrSpec;
    UniString m_aRealName;

    SvAddressEntry_Impl() {};
    SvAddressEntry_Impl(UniString const & rTheAddrSpec,
                        UniString const & rTheRealName):
        m_aAddrSpec(rTheAddrSpec), m_aRealName(rTheRealName) {}
};

//============================================================================
DECLARE_LIST(SvAddressList_Impl, SvAddressEntry_Impl *)

//============================================================================
class  SvAddressParser
{
    friend class SvAddressParser_Impl;

    SvAddressEntry_Impl m_aFirst;
    SvAddressList_Impl m_aRest;
    bool m_bHasFirst;

public:
    SvAddressParser(UniString const & rInput);

    ~SvAddressParser();

    sal_Int32 Count() const { return m_bHasFirst ? m_aRest.Count() + 1 : 0; }

    inline UniString const & GetEmailAddress(sal_Int32 nIndex) const;

    inline UniString const &GetRealName(sal_Int32 nIndex) const;
};

inline UniString const & SvAddressParser::GetEmailAddress(sal_Int32 nIndex)
    const
{
    return nIndex == 0 ? m_aFirst.m_aAddrSpec :
                         m_aRest.GetObject(nIndex - 1)->m_aAddrSpec;
}

inline UniString const & SvAddressParser::GetRealName(sal_Int32 nIndex) const
{
    return nIndex == 0 ? m_aFirst.m_aRealName :
                         m_aRest.GetObject(nIndex - 1)->m_aRealName;
}

}

#endif // _ADRPARSE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
