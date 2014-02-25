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

#ifndef INCLUDED_SVL_ADRPARSE_HXX
#define INCLUDED_SVL_ADRPARSE_HXX

#include <svl/svldllapi.h>
#include <vector>


struct SvAddressEntry_Impl
{
    OUString m_aAddrSpec;
    OUString m_aRealName;

    SvAddressEntry_Impl()
    {
    }

    SvAddressEntry_Impl(const OUString& rTheAddrSpec,
                        const OUString& rTheRealName)
        : m_aAddrSpec(rTheAddrSpec)
        , m_aRealName(rTheRealName)
    {
    }
};


typedef ::std::vector< SvAddressEntry_Impl* > SvAddressList_Impl;


class SVL_DLLPUBLIC SvAddressParser
{
    friend class SvAddressParser_Impl;

    SvAddressEntry_Impl m_aFirst;
    SvAddressList_Impl m_aRest;
    bool m_bHasFirst;

public:
    SvAddressParser(const OUString& rInput);

    ~SvAddressParser();

    sal_Int32 Count() const { return m_bHasFirst ? m_aRest.size() + 1 : 0; }

    const OUString& GetEmailAddress(sal_Int32 nIndex) const
    {
        return nIndex == 0 ? m_aFirst.m_aAddrSpec :
                             m_aRest[ nIndex - 1 ]->m_aAddrSpec;
    }

    const OUString& GetRealName(sal_Int32 nIndex) const
    {
        return nIndex == 0 ? m_aFirst.m_aRealName :
                             m_aRest[ nIndex - 1 ]->m_aRealName;
    }
};

#endif // INCLUDED_SVL_ADRPARSE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
