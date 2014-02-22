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


#include "macabcondition.hxx"
#include "MacabHeader.hxx"
#include "MacabRecord.hxx"
#include "connectivity/CommonTools.hxx"

using namespace ::connectivity::macab;
using namespace ::com::sun::star::sdbc;

MacabCondition::~MacabCondition()
{
}

MacabConditionConstant::MacabConditionConstant(const sal_Bool bValue)
    : MacabCondition(),
      m_bValue(bValue)
{
}

sal_Bool MacabConditionConstant::isAlwaysTrue() const
{
    return m_bValue;
}

sal_Bool MacabConditionConstant::isAlwaysFalse() const
{
    return !m_bValue;
}

sal_Bool MacabConditionConstant::eval(const MacabRecord *) const
{
    return m_bValue;
}

MacabConditionColumn::MacabConditionColumn(const MacabHeader *header, const OUString &sColumnName) throw(SQLException)
    : MacabCondition(),
      m_nFieldNumber(header->getColumnNumber(sColumnName))
{
}

sal_Bool MacabConditionColumn::isAlwaysTrue() const
{
    // Sometimes true, sometimes false
    return sal_False;
}

sal_Bool MacabConditionColumn::isAlwaysFalse() const
{
    // Sometimes true, sometimes false
    return sal_False;
}

MacabConditionNull::MacabConditionNull(const MacabHeader *header, const OUString &sColumnName) throw(SQLException)
    : MacabConditionColumn(header, sColumnName)
{
}

sal_Bool MacabConditionNull::eval(const MacabRecord *aRecord) const
{
    macabfield *aValue = aRecord->get(m_nFieldNumber);

    if(aValue == NULL)
        return sal_True;
    else if(aValue->value == NULL)
        return sal_True;
    else
        return sal_False;
}

MacabConditionNotNull::MacabConditionNotNull(const MacabHeader *header, const OUString &sColumnName) throw(SQLException)
    : MacabConditionColumn(header, sColumnName)
{
}

sal_Bool MacabConditionNotNull::eval(const MacabRecord *aRecord) const
{
    macabfield *aValue = aRecord->get(m_nFieldNumber);

    if(aValue == NULL)
        return sal_False;
    else if(aValue->value == NULL)
        return sal_False;
    else
        return sal_True;
}

MacabConditionCompare::MacabConditionCompare(const MacabHeader *header, const OUString &sColumnName, const OUString &sMatchString) throw(SQLException)
    : MacabConditionColumn(header, sColumnName),
      m_sMatchString(sMatchString)
{
}

MacabConditionEqual::MacabConditionEqual(const MacabHeader *header, const OUString &sColumnName, const OUString &sMatchString) throw(SQLException)
    : MacabConditionCompare(header, sColumnName, sMatchString)
{
}

sal_Bool MacabConditionEqual::eval(const MacabRecord *aRecord) const
{
    macabfield *aValue = aRecord->get(m_nFieldNumber);

    if(aValue == NULL)
        return sal_False;

    macabfield *aValue2 = MacabRecord::createMacabField(m_sMatchString,aValue->type);

    if(aValue2 == NULL)
        return sal_False;

    sal_Int32 nReturn = MacabRecord::compareFields(aValue, aValue2);

    delete aValue2;
    return nReturn == 0;
}

MacabConditionDifferent::MacabConditionDifferent(const MacabHeader *header, const OUString &sColumnName, const OUString &sMatchString) throw(SQLException)
    : MacabConditionCompare(header, sColumnName, sMatchString)
{
}

sal_Bool MacabConditionDifferent::eval(const MacabRecord *aRecord) const
{
    macabfield *aValue = aRecord->get(m_nFieldNumber);

    if(aValue == NULL)
        return sal_False;

    macabfield *aValue2 = MacabRecord::createMacabField(m_sMatchString,aValue->type);

    if(aValue2 == NULL)
        return sal_False;

    sal_Int32 nReturn = MacabRecord::compareFields(aValue, aValue2);

    delete aValue2;
    return nReturn != 0;
}

MacabConditionSimilar::MacabConditionSimilar(const MacabHeader *header, const OUString &sColumnName, const OUString &sMatchString) throw(SQLException)
    : MacabConditionCompare(header, sColumnName, sMatchString)
{
}

sal_Bool MacabConditionSimilar::eval(const MacabRecord *aRecord) const
{
    macabfield *aValue = aRecord->get(m_nFieldNumber);

    if(aValue == NULL)
        return sal_False;

    OUString sName = MacabRecord::fieldToString(aValue);

    return match(m_sMatchString, sName, '\0');
}

MacabConditionBoolean::MacabConditionBoolean(MacabCondition *pLeft, MacabCondition *pRight)
    : MacabCondition(),
      m_pLeft(pLeft),
      m_pRight(pRight)
{
}

MacabConditionBoolean::~MacabConditionBoolean()
{
    delete m_pLeft;
    delete m_pRight;
}

MacabConditionOr::MacabConditionOr(MacabCondition *pLeft, MacabCondition *pRight)
    : MacabConditionBoolean(pLeft, pRight)
{
}

sal_Bool MacabConditionOr::isAlwaysTrue() const
{
    return m_pLeft->isAlwaysTrue() || m_pRight->isAlwaysTrue();
}

sal_Bool MacabConditionOr::isAlwaysFalse() const
{
    return m_pLeft->isAlwaysFalse() && m_pRight->isAlwaysFalse();
}

sal_Bool MacabConditionOr::eval(const MacabRecord *aRecord) const
{
    // We avoid evaluating terms as much as we can
    if (m_pLeft->isAlwaysTrue() || m_pRight->isAlwaysTrue()) return sal_True;
    if (m_pLeft->isAlwaysFalse() && m_pRight->isAlwaysFalse()) return sal_False;

    if (m_pLeft->eval(aRecord)) return sal_True;
    if (m_pRight->eval(aRecord)) return sal_True;

    return sal_False;
}

MacabConditionAnd::MacabConditionAnd(MacabCondition *pLeft, MacabCondition *pRight)
    : MacabConditionBoolean(pLeft, pRight)
{
}

sal_Bool MacabConditionAnd::isAlwaysTrue() const
{
    return m_pLeft->isAlwaysTrue() && m_pRight->isAlwaysTrue();
}

sal_Bool MacabConditionAnd::isAlwaysFalse() const
{
    return m_pLeft->isAlwaysFalse() || m_pRight->isAlwaysFalse();
}

sal_Bool MacabConditionAnd::eval(const MacabRecord *aRecord) const
{
    // We avoid evaluating terms as much as we can
    if (m_pLeft->isAlwaysFalse() || m_pRight->isAlwaysFalse()) return sal_False;
    if (m_pLeft->isAlwaysTrue() && m_pRight->isAlwaysTrue()) return sal_True;

    if (!m_pLeft->eval(aRecord)) return sal_False;
    if (!m_pRight->eval(aRecord)) return sal_False;

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
