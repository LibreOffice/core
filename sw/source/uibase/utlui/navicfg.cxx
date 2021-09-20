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

#include <swtypes.hxx>
#include <navicfg.hxx>
#include <swcont.hxx>
#include <o3tl/any.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <com/sun/star/uno/Sequence.hxx>

using namespace ::utl;
using namespace ::com::sun::star::uno;

Sequence<OUString> SwNavigationConfig::GetPropertyNames()
{
    return css::uno::Sequence<OUString>{
        OUString("RootType"),
        OUString("SelectedPosition"),
        OUString("OutlineLevel"),
        OUString("InsertMode"),
        OUString("ActiveBlock"),
        OUString("ShowListBox"),
        OUString("GlobalDocMode")};
}

SwNavigationConfig::SwNavigationConfig() :
    utl::ConfigItem("Office.Writer/Navigator"),
    m_nRootType(ContentTypeId::UNKNOWN),
    m_nSelectedPos(0),
    m_nOutlineLevel(MAXLEVEL),
    m_nRegionMode(RegionMode::NONE),
    m_nActiveBlock(0),
    m_bIsSmall(false),
    m_bIsGlobalActive(true)
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() != aNames.getLength())
        return;

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        if(pValues[nProp].hasValue())
        {
            switch(nProp)
            {
                case 0:
                {
                    sal_Int32 nTmp = {}; // spurious -Werror=maybe-uninitialized
                    if (pValues[nProp] >>= nTmp)
                    {
                        if (nTmp < sal_Int32(ContentTypeId::UNKNOWN)
                            || nTmp > sal_Int32(ContentTypeId::LAST))
                        {
                            SAL_WARN(
                                "sw",
                                "out-of-bounds ContentTypeId " << nTmp);
                            nTmp = sal_Int32(ContentTypeId::UNKNOWN);
                        }
                        m_nRootType = static_cast<ContentTypeId>(nTmp);
                    }
                    break;
                }
                case 1: pValues[nProp] >>= m_nSelectedPos;   break;
                case 2: pValues[nProp] >>= m_nOutlineLevel;  break;
                case 3:
                {
                        sal_uInt16 nTmp;
                        if (pValues[nProp] >>= nTmp)
                            m_nRegionMode = static_cast<RegionMode>(nTmp);
                        break;
                }
                case 4: pValues[nProp] >>= m_nActiveBlock;    break;
                case 5: m_bIsSmall        = *o3tl::doAccess<bool>(pValues[nProp]);  break;
                case 6: m_bIsGlobalActive = *o3tl::doAccess<bool>(pValues[nProp]);  break;
            }
        }
    }
}

SwNavigationConfig::~SwNavigationConfig()
{
}

void SwNavigationConfig::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case 0: pValues[nProp] <<= static_cast<sal_Int32>(m_nRootType);     break;
            case 1: pValues[nProp] <<= m_nSelectedPos;  break;
            case 2: pValues[nProp] <<= m_nOutlineLevel; break;
            case 3: pValues[nProp] <<= static_cast<sal_uInt16>(m_nRegionMode); break;
            case 4: pValues[nProp] <<= m_nActiveBlock;    break;
            case 5: pValues[nProp] <<= m_bIsSmall; break;
            case 6: pValues[nProp] <<= m_bIsGlobalActive; break;
        }
    }
    PutProperties(aNames, aValues);
}

void SwNavigationConfig::Notify( const css::uno::Sequence< OUString >& ) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
