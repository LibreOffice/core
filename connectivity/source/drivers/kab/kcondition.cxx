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

#include "kcondition.hxx"
#include "kfields.hxx"
#include "connectivity/CommonTools.hxx"

using namespace ::connectivity::kab;
using namespace ::com::sun::star::sdbc;
// -----------------------------------------------------------------------------
KabCondition::~KabCondition()
{
}
// -----------------------------------------------------------------------------
KabConditionConstant::KabConditionConstant(const sal_Bool bValue)
    : KabCondition(),
      m_bValue(bValue)
{
}
// -----------------------------------------------------------------------------
sal_Bool KabConditionConstant::isAlwaysTrue() const
{
    return m_bValue;
}
// -----------------------------------------------------------------------------
sal_Bool KabConditionConstant::isAlwaysFalse() const
{
    return !m_bValue;
}
// -----------------------------------------------------------------------------
sal_Bool KabConditionConstant::eval(const ::KABC::Addressee &) const
{
    return m_bValue;
}
// -----------------------------------------------------------------------------
KabConditionColumn::KabConditionColumn(const ::rtl::OUString &sColumnName) throw(SQLException)
    : KabCondition(),
      m_nFieldNumber(findKabField(sColumnName))
{
}
// -----------------------------------------------------------------------------
sal_Bool KabConditionColumn::isAlwaysTrue() const
{
    // Sometimes true, sometimes false
    return sal_False;
}
// -----------------------------------------------------------------------------
sal_Bool KabConditionColumn::isAlwaysFalse() const
{
    // Sometimes true, sometimes false
    return sal_False;
}
// -----------------------------------------------------------------------------
KabConditionNull::KabConditionNull(const ::rtl::OUString &sColumnName) throw(SQLException)
    : KabConditionColumn(sColumnName)
{
}
// -----------------------------------------------------------------------------
sal_Bool KabConditionNull::eval(const ::KABC::Addressee &aAddressee) const
{
    QString aQtName = valueOfKabField(aAddressee, m_nFieldNumber);

    return aQtName.isNull();
// KDE address book currently does not use NULL values.
// But it might do it someday
}
// -----------------------------------------------------------------------------
KabConditionNotNull::KabConditionNotNull(const ::rtl::OUString &sColumnName) throw(SQLException)
    : KabConditionColumn(sColumnName)
{
}
// -----------------------------------------------------------------------------
sal_Bool KabConditionNotNull::eval(const ::KABC::Addressee &aAddressee) const
{
    QString aQtName = valueOfKabField(aAddressee, m_nFieldNumber);

    return !aQtName.isNull();
// KDE address book currently does not use NULL values.
// But it might do it someday
}
// -----------------------------------------------------------------------------
KabConditionCompare::KabConditionCompare(const ::rtl::OUString &sColumnName, const ::rtl::OUString &sMatchString) throw(SQLException)
    : KabConditionColumn(sColumnName),
      m_sMatchString(sMatchString)
{
}
// -----------------------------------------------------------------------------
KabConditionEqual::KabConditionEqual(const ::rtl::OUString &sColumnName, const ::rtl::OUString &sMatchString) throw(SQLException)
    : KabConditionCompare(sColumnName, sMatchString)
{
}
// -----------------------------------------------------------------------------
sal_Bool KabConditionEqual::eval(const ::KABC::Addressee &aAddressee) const
{
    QString aQtName = valueOfKabField(aAddressee, m_nFieldNumber);
// Timestamps should not be compared according to their string value
// The syntax for such queries should be like
//  {ts '2004-03-29 12:55:00.000000'}
// They should also support operators like '<' or '>='

    if (aQtName.isNull()) return sal_False;

    ::rtl::OUString sValue((const sal_Unicode *) aQtName.ucs2());
    return sValue == m_sMatchString;
}
// -----------------------------------------------------------------------------
KabConditionDifferent::KabConditionDifferent(const ::rtl::OUString &sColumnName, const ::rtl::OUString &sMatchString) throw(SQLException)
    : KabConditionCompare(sColumnName, sMatchString)
{
}
// -----------------------------------------------------------------------------
sal_Bool KabConditionDifferent::eval(const ::KABC::Addressee &aAddressee) const
{
    QString aQtName = valueOfKabField(aAddressee, m_nFieldNumber);

    if (aQtName.isNull()) return sal_False;

    ::rtl::OUString sValue((const sal_Unicode *) aQtName.ucs2());
    return sValue != m_sMatchString;
}
// -----------------------------------------------------------------------------
KabConditionSimilar::KabConditionSimilar(const ::rtl::OUString &sColumnName, const ::rtl::OUString &sMatchString) throw(SQLException)
    : KabConditionCompare(sColumnName, sMatchString)
{
}
// -----------------------------------------------------------------------------
sal_Bool KabConditionSimilar::eval(const ::KABC::Addressee &aAddressee) const
{
    QString aQtName = valueOfKabField(aAddressee, m_nFieldNumber);

    if (aQtName.isNull()) return sal_False;

    ::rtl::OUString sValue((const sal_Unicode *) aQtName.ucs2());
    return match(m_sMatchString, sValue, '\0');
}
// -----------------------------------------------------------------------------
KabConditionBoolean::KabConditionBoolean(KabCondition *pLeft, KabCondition *pRight)
    : KabCondition(),
      m_pLeft(pLeft),
      m_pRight(pRight)
{
}
// -----------------------------------------------------------------------------
KabConditionBoolean::~KabConditionBoolean()
{
    delete m_pLeft;
    delete m_pRight;
}
// -----------------------------------------------------------------------------
KabConditionOr::KabConditionOr(KabCondition *pLeft, KabCondition *pRight)
    : KabConditionBoolean(pLeft, pRight)
{
}
// -----------------------------------------------------------------------------
sal_Bool KabConditionOr::isAlwaysTrue() const
{
    return m_pLeft->isAlwaysTrue() || m_pRight->isAlwaysTrue();
}
// -----------------------------------------------------------------------------
sal_Bool KabConditionOr::isAlwaysFalse() const
{
    return m_pLeft->isAlwaysFalse() && m_pRight->isAlwaysFalse();
}
// -----------------------------------------------------------------------------
sal_Bool KabConditionOr::eval(const ::KABC::Addressee &aAddressee) const
{
    // We avoid evaluating terms as much as we can
    if (m_pLeft->isAlwaysTrue() || m_pRight->isAlwaysTrue()) return sal_True;
    if (m_pLeft->isAlwaysFalse() && m_pRight->isAlwaysFalse()) return sal_False;

    if (m_pLeft->eval(aAddressee)) return sal_True;
    if (m_pRight->eval(aAddressee)) return sal_True;

    return sal_False;
}
// -----------------------------------------------------------------------------
KabConditionAnd::KabConditionAnd(KabCondition *pLeft, KabCondition *pRight)
    : KabConditionBoolean(pLeft, pRight)
{
}
// -----------------------------------------------------------------------------
sal_Bool KabConditionAnd::isAlwaysTrue() const
{
    return m_pLeft->isAlwaysTrue() && m_pRight->isAlwaysTrue();
}
// -----------------------------------------------------------------------------
sal_Bool KabConditionAnd::isAlwaysFalse() const
{
    return m_pLeft->isAlwaysFalse() || m_pRight->isAlwaysFalse();
}
// -----------------------------------------------------------------------------
sal_Bool KabConditionAnd::eval(const ::KABC::Addressee &aAddressee) const
{
    // We avoid evaluating terms as much as we can
    if (m_pLeft->isAlwaysFalse() || m_pRight->isAlwaysFalse()) return sal_False;
    if (m_pLeft->isAlwaysTrue() && m_pRight->isAlwaysTrue()) return sal_True;

    if (!m_pLeft->eval(aAddressee)) return sal_False;
    if (!m_pRight->eval(aAddressee)) return sal_False;

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
