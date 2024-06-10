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

#include <dbconfig.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <swdbdata.hxx>

using namespace utl;
using namespace com::sun::star::uno;

const Sequence<OUString>& SwDBConfig::GetPropertyNames()
{
    static Sequence<OUString> aNames {
        u"AddressBook/DataSourceName"_ustr,        //  0
        u"AddressBook/Command"_ustr,               //  1
        u"AddressBook/CommandType"_ustr,           //  2
        u"Bibliography/CurrentDataSource/DataSourceName"_ustr,    //  4
        u"Bibliography/CurrentDataSource/Command"_ustr,           //  5
        u"Bibliography/CurrentDataSource/CommandType"_ustr        //  6
    };
    return aNames;
}

SwDBConfig::SwDBConfig() :
    ConfigItem(u"Office.DataAccess"_ustr, ConfigItemMode::ReleaseTree)
{
};

SwDBConfig::~SwDBConfig()
{
    m_pAdrImpl.reset();
    m_pBibImpl.reset();
}

void SwDBConfig::Load()
{
    const Sequence<OUString>& rNames = GetPropertyNames();
    if (!m_pAdrImpl)
        m_pAdrImpl.reset(new SwDBData);
    if (!m_pBibImpl)
        m_pBibImpl.reset(new SwDBData);
    Sequence<Any> aValues = GetProperties(rNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == rNames.getLength(), "GetProperties failed");
    if(aValues.getLength() != rNames.getLength())
        return;

    for(int nProp = 0; nProp < rNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0: pValues[nProp] >>= m_pAdrImpl->sDataSource;  break;
            case  1: pValues[nProp] >>= m_pAdrImpl->sCommand;     break;
            case  2: pValues[nProp] >>= m_pAdrImpl->nCommandType; break;
            case  3: pValues[nProp] >>= m_pBibImpl->sDataSource;  break;
            case  4: pValues[nProp] >>= m_pBibImpl->sCommand;     break;
            case  5: pValues[nProp] >>= m_pBibImpl->nCommandType; break;
        }
    }
}

const SwDBData& SwDBConfig::GetAddressSource()
{
    if(!m_pAdrImpl)
        Load();
    return *m_pAdrImpl;
}

const SwDBData& SwDBConfig::GetBibliographySource()
{
    if(!m_pBibImpl)
        Load();
    return *m_pBibImpl;
}

void SwDBConfig::ImplCommit() {}
void SwDBConfig::Notify( const css::uno::Sequence< OUString >& ) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
