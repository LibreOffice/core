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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include "macabcondition.hxx"
#include "MacabHeader.hxx"
#include "MacabRecord.hxx"
#include "connectivity/CommonTools.hxx"

using namespace ::connectivity::macab;
using namespace ::com::sun::star::sdbc;
// -----------------------------------------------------------------------------
MacabCondition::~MacabCondition()
{
}
// -----------------------------------------------------------------------------
MacabConditionConstant::MacabConditionConstant(const sal_Bool bValue)
    : MacabCondition(),
      m_bValue(bValue)
{
}
// -----------------------------------------------------------------------------
sal_Bool MacabConditionConstant::isAlwaysTrue() const
{
    return m_bValue;
}
// -----------------------------------------------------------------------------
sal_Bool MacabConditionConstant::isAlwaysFalse() const
{
    return !m_bValue;
}
// -----------------------------------------------------------------------------
sal_Bool MacabConditionConstant::eval(const MacabRecord *) const
{
    return m_bValue;
}
// -----------------------------------------------------------------------------
MacabConditionColumn::MacabConditionColumn(const MacabHeader *header, const ::rtl::OUString &sColumnName) throw(SQLException)
    : MacabCondition(),
      m_nFieldNumber(header->getColumnNumber(sColumnName))
{
}
// -----------------------------------------------------------------------------
sal_Bool MacabConditionColumn::isAlwaysTrue() const
{
    // Sometimes true, sometimes false
    return sal_False;
}
// -----------------------------------------------------------------------------
sal_Bool MacabConditionColumn::isAlwaysFalse() const
{
    // Sometimes true, sometimes false
    return sal_False;
}
// -----------------------------------------------------------------------------
MacabConditionNull::MacabConditionNull(const MacabHeader *header, const ::rtl::OUString &sColumnName) throw(SQLException)
    : MacabConditionColumn(header, sColumnName)
{
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
MacabConditionNotNull::MacabConditionNotNull(const MacabHeader *header, const ::rtl::OUString &sColumnName) throw(SQLException)
    : MacabConditionColumn(header, sColumnName)
{
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
MacabConditionCompare::MacabConditionCompare(const MacabHeader *header, const ::rtl::OUString &sColumnName, const ::rtl::OUString &sMatchString) throw(SQLException)
    : MacabConditionColumn(header, sColumnName),
      m_sMatchString(sMatchString)
{
}
// -----------------------------------------------------------------------------
MacabConditionEqual::MacabConditionEqual(const MacabHeader *header, const ::rtl::OUString &sColumnName, const ::rtl::OUString &sMatchString) throw(SQLException)
    : MacabConditionCompare(header, sColumnName, sMatchString)
{
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
MacabConditionDifferent::MacabConditionDifferent(const MacabHeader *header, const ::rtl::OUString &sColumnName, const ::rtl::OUString &sMatchString) throw(SQLException)
    : MacabConditionCompare(header, sColumnName, sMatchString)
{
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
MacabConditionSimilar::MacabConditionSimilar(const MacabHeader *header, const ::rtl::OUString &sColumnName, const ::rtl::OUString &sMatchString) throw(SQLException)
    : MacabConditionCompare(header, sColumnName, sMatchString)
{
}
// -----------------------------------------------------------------------------
sal_Bool MacabConditionSimilar::eval(const MacabRecord *aRecord) const
{
    macabfield *aValue = aRecord->get(m_nFieldNumber);

    if(aValue == NULL)
        return sal_False;

    ::rtl::OUString sName = MacabRecord::fieldToString(aValue);

    return match(m_sMatchString, sName, '\0');
}
// -----------------------------------------------------------------------------
MacabConditionBoolean::MacabConditionBoolean(MacabCondition *pLeft, MacabCondition *pRight)
    : MacabCondition(),
      m_pLeft(pLeft),
      m_pRight(pRight)
{
}
// -----------------------------------------------------------------------------
MacabConditionBoolean::~MacabConditionBoolean()
{
    delete m_pLeft;
    delete m_pRight;
}
// -----------------------------------------------------------------------------
MacabConditionOr::MacabConditionOr(MacabCondition *pLeft, MacabCondition *pRight)
    : MacabConditionBoolean(pLeft, pRight)
{
}
// -----------------------------------------------------------------------------
sal_Bool MacabConditionOr::isAlwaysTrue() const
{
    return m_pLeft->isAlwaysTrue() || m_pRight->isAlwaysTrue();
}
// -----------------------------------------------------------------------------
sal_Bool MacabConditionOr::isAlwaysFalse() const
{
    return m_pLeft->isAlwaysFalse() && m_pRight->isAlwaysFalse();
}
// -----------------------------------------------------------------------------
sal_Bool MacabConditionOr::eval(const MacabRecord *aRecord) const
{
    // We avoid evaluating terms as much as we can
    if (m_pLeft->isAlwaysTrue() || m_pRight->isAlwaysTrue()) return sal_True;
    if (m_pLeft->isAlwaysFalse() && m_pRight->isAlwaysFalse()) return sal_False;

    if (m_pLeft->eval(aRecord)) return sal_True;
    if (m_pRight->eval(aRecord)) return sal_True;

    return sal_False;
}
// -----------------------------------------------------------------------------
MacabConditionAnd::MacabConditionAnd(MacabCondition *pLeft, MacabCondition *pRight)
    : MacabConditionBoolean(pLeft, pRight)
{
}
// -----------------------------------------------------------------------------
sal_Bool MacabConditionAnd::isAlwaysTrue() const
{
    return m_pLeft->isAlwaysTrue() && m_pRight->isAlwaysTrue();
}
// -----------------------------------------------------------------------------
sal_Bool MacabConditionAnd::isAlwaysFalse() const
{
    return m_pLeft->isAlwaysFalse() || m_pRight->isAlwaysFalse();
}
// -----------------------------------------------------------------------------
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
