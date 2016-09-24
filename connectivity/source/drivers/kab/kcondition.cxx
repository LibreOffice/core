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


#include "kcondition.hxx"
#include "kfields.hxx"
#include <connectivity/CommonTools.hxx>

using namespace ::connectivity::kab;
using namespace ::com::sun::star::sdbc;

KabCondition::~KabCondition()
{
}

KabConditionConstant::KabConditionConstant(const bool bValue)
    : KabCondition(),
      m_bValue(bValue)
{
}

bool KabConditionConstant::isAlwaysTrue() const
{
    return m_bValue;
}

bool KabConditionConstant::isAlwaysFalse() const
{
    return !m_bValue;
}

bool KabConditionConstant::eval(const ::KABC::Addressee &) const
{
    return m_bValue;
}

KabConditionColumn::KabConditionColumn(const OUString &sColumnName) throw(SQLException)
    : KabCondition(),
      m_nFieldNumber(findKabField(sColumnName))
{
}

bool KabConditionColumn::isAlwaysTrue() const
{
    // Sometimes true, sometimes false
    return false;
}

bool KabConditionColumn::isAlwaysFalse() const
{
    // Sometimes true, sometimes false
    return false;
}

KabConditionNull::KabConditionNull(const OUString &sColumnName) throw(SQLException)
    : KabConditionColumn(sColumnName)
{
}

bool KabConditionNull::eval(const ::KABC::Addressee &aAddressee) const
{
    QString aQtName = valueOfKabField(aAddressee, m_nFieldNumber);

    return aQtName.isNull();
// KDE address book currently does not use NULL values.
// But it might do it someday
}

KabConditionNotNull::KabConditionNotNull(const OUString &sColumnName) throw(SQLException)
    : KabConditionColumn(sColumnName)
{
}

bool KabConditionNotNull::eval(const ::KABC::Addressee &aAddressee) const
{
    QString aQtName = valueOfKabField(aAddressee, m_nFieldNumber);

    return !aQtName.isNull();
// KDE address book currently does not use NULL values.
// But it might do it someday
}

KabConditionCompare::KabConditionCompare(const OUString &sColumnName, const OUString &sMatchString) throw(SQLException)
    : KabConditionColumn(sColumnName),
      m_sMatchString(sMatchString)
{
}

KabConditionEqual::KabConditionEqual(const OUString &sColumnName, const OUString &sMatchString) throw(SQLException)
    : KabConditionCompare(sColumnName, sMatchString)
{
}

bool KabConditionEqual::eval(const ::KABC::Addressee &aAddressee) const
{
    QString aQtName = valueOfKabField(aAddressee, m_nFieldNumber);
// Timestamps should not be compared according to their string value
// The syntax for such queries should be like
//  {ts '2004-03-29 12:55:00.000000'}
// They should also support operators like '<' or '>='

    if (aQtName.isNull()) return false;

    OUString sValue(reinterpret_cast<const sal_Unicode *>(aQtName.ucs2()));
    return sValue == m_sMatchString;
}

KabConditionDifferent::KabConditionDifferent(const OUString &sColumnName, const OUString &sMatchString) throw(SQLException)
    : KabConditionCompare(sColumnName, sMatchString)
{
}

bool KabConditionDifferent::eval(const ::KABC::Addressee &aAddressee) const
{
    QString aQtName = valueOfKabField(aAddressee, m_nFieldNumber);

    if (aQtName.isNull()) return false;

    OUString sValue(reinterpret_cast<const sal_Unicode *>(aQtName.ucs2()));
    return sValue != m_sMatchString;
}

KabConditionSimilar::KabConditionSimilar(const OUString &sColumnName, const OUString &sMatchString) throw(SQLException)
    : KabConditionCompare(sColumnName, sMatchString)
{
}

bool KabConditionSimilar::eval(const ::KABC::Addressee &aAddressee) const
{
    QString aQtName = valueOfKabField(aAddressee, m_nFieldNumber);

    if (aQtName.isNull()) return false;

    OUString sValue(reinterpret_cast<const sal_Unicode *>(aQtName.ucs2()));
    return match(m_sMatchString, sValue, '\0');
}

KabConditionBoolean::KabConditionBoolean(KabCondition *pLeft, KabCondition *pRight)
    : KabCondition(),
      m_pLeft(pLeft),
      m_pRight(pRight)
{
}

KabConditionBoolean::~KabConditionBoolean()
{
    delete m_pLeft;
    delete m_pRight;
}

KabConditionOr::KabConditionOr(KabCondition *pLeft, KabCondition *pRight)
    : KabConditionBoolean(pLeft, pRight)
{
}

bool KabConditionOr::isAlwaysTrue() const
{
    return m_pLeft->isAlwaysTrue() || m_pRight->isAlwaysTrue();
}

bool KabConditionOr::isAlwaysFalse() const
{
    return m_pLeft->isAlwaysFalse() && m_pRight->isAlwaysFalse();
}

bool KabConditionOr::eval(const ::KABC::Addressee &aAddressee) const
{
    // We avoid evaluating terms as much as we can
    if (m_pLeft->isAlwaysTrue() || m_pRight->isAlwaysTrue()) return true;
    if (m_pLeft->isAlwaysFalse() && m_pRight->isAlwaysFalse()) return false;

    if (m_pLeft->eval(aAddressee)) return true;
    if (m_pRight->eval(aAddressee)) return true;

    return false;
}

KabConditionAnd::KabConditionAnd(KabCondition *pLeft, KabCondition *pRight)
    : KabConditionBoolean(pLeft, pRight)
{
}

bool KabConditionAnd::isAlwaysTrue() const
{
    return m_pLeft->isAlwaysTrue() && m_pRight->isAlwaysTrue();
}

bool KabConditionAnd::isAlwaysFalse() const
{
    return m_pLeft->isAlwaysFalse() || m_pRight->isAlwaysFalse();
}

bool KabConditionAnd::eval(const ::KABC::Addressee &aAddressee) const
{
    // We avoid evaluating terms as much as we can
    if (m_pLeft->isAlwaysFalse() || m_pRight->isAlwaysFalse()) return false;
    if (m_pLeft->isAlwaysTrue() && m_pRight->isAlwaysTrue()) return true;

    if (!m_pLeft->eval(aAddressee)) return false;
    if (!m_pRight->eval(aAddressee)) return false;

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
