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
#include "SectionColumnHandler.hxx"
#include "ConversionHelper.hxx"
#include <ooxml/resourceids.hxx>
#include <osl/diagnose.h>

namespace writerfilter::dmapper
{
using namespace ::com::sun::star;

SectionColumnHandler::SectionColumnHandler()
    : LoggedProperties("SectionColumnHandler")
    , m_bEqualWidth(false)
    , m_nSpace(1270) // 720 twips
    , m_nNum(0)
    , m_bSep(false)
{
    m_aTempColumn.nWidth = m_aTempColumn.nSpace = 0;
}

SectionColumnHandler::~SectionColumnHandler() {}

void SectionColumnHandler::lcl_attribute(Id rName, Value& rVal)
{
    sal_Int32 nIntValue = rVal.getInt();
    switch (rName)
    {
        case NS_ooxml::LN_CT_Columns_equalWidth:
            m_bEqualWidth = (nIntValue != 0);
            break;
        case NS_ooxml::LN_CT_Columns_space:
            m_nSpace = ConversionHelper::convertTwipToMM100(nIntValue);
            break;
        case NS_ooxml::LN_CT_Columns_num:
            m_nNum = nIntValue;
            break;
        case NS_ooxml::LN_CT_Columns_sep:
            m_bSep = (nIntValue != 0);
            break;

        case NS_ooxml::LN_CT_Column_w:
            m_aTempColumn.nWidth = ConversionHelper::convertTwipToMM100(nIntValue);
            break;
        case NS_ooxml::LN_CT_Column_space:
            m_aTempColumn.nSpace = ConversionHelper::convertTwipToMM100(nIntValue);
            break;
        default:
            OSL_FAIL("SectionColumnHandler: unknown attribute");
    }
}

void SectionColumnHandler::lcl_sprm(Sprm& rSprm)
{
    switch (rSprm.getId())
    {
        case NS_ooxml::LN_CT_Columns_col:
        {
            m_aTempColumn.nWidth = m_aTempColumn.nSpace = 0;
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if (pProperties)
            {
                pProperties->resolve(*this);
                m_aCols.push_back(m_aTempColumn);
            }
        }
        break;
        default:
            OSL_FAIL("SectionColumnHandler: unknown sprm");
    }
}
} //namespace writerfilter::dmapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
