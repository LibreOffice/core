/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: kcondition.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:41:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "kcondition.hxx"

#ifndef _CONNECTIVITY_KAB_FIELDS_HXX_
#include "kfields.hxx"
#endif

#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif

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
