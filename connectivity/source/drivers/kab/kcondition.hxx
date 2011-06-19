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

#ifndef _CONNECTIVITY_KAB_CONDITION_HXX_
#define _CONNECTIVITY_KAB_CONDITION_HXX_

#include <comphelper/types.hxx>
#include <shell/kde_headers.h>
#include <connectivity/dbexception.hxx>

namespace connectivity
{
    namespace kab
    {
// -----------------------------------------------------------------------------
class KabCondition
{
    public:
        virtual ~KabCondition();
        virtual sal_Bool isAlwaysTrue() const = 0;
        virtual sal_Bool isAlwaysFalse() const = 0;
        virtual sal_Bool eval(const ::KABC::Addressee &aAddressee) const = 0;
};
// -----------------------------------------------------------------------------
class KabConditionConstant : public KabCondition
{
    protected:
        sal_Bool m_bValue;

    public:
        KabConditionConstant(const sal_Bool bValue);
        virtual sal_Bool isAlwaysTrue() const;
        virtual sal_Bool isAlwaysFalse() const;
        virtual sal_Bool eval(const ::KABC::Addressee &aAddressee) const;
};
// -----------------------------------------------------------------------------
class KabConditionColumn : public KabCondition
{
    protected:
        sal_Int32 m_nFieldNumber;

        QString value(const ::KABC::Addressee &aAddressee) const;

    public:
        KabConditionColumn(
            const ::rtl::OUString &sColumnName) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool isAlwaysTrue() const;
        virtual sal_Bool isAlwaysFalse() const;
};
// -----------------------------------------------------------------------------
class KabConditionNull : public KabConditionColumn
{
    public:
        KabConditionNull(
            const ::rtl::OUString &sColumnName) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool eval(const ::KABC::Addressee &aAddressee) const;
};
// -----------------------------------------------------------------------------
class KabConditionNotNull : public KabConditionColumn
{
    public:
        KabConditionNotNull(
            const ::rtl::OUString &sColumnName) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool eval(const ::KABC::Addressee &aAddressee) const;
};
// -----------------------------------------------------------------------------
class KabConditionCompare : public KabConditionColumn
{
    protected:
        const ::rtl::OUString m_sMatchString;

    public:
        KabConditionCompare(
            const ::rtl::OUString &sColumnName,
            const ::rtl::OUString &sMatchString) throw(::com::sun::star::sdbc::SQLException);
};
// -----------------------------------------------------------------------------
class KabConditionEqual : public KabConditionCompare
{
    public:
        KabConditionEqual(
            const ::rtl::OUString &sColumnName,
            const ::rtl::OUString &sMatchString) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool eval(const ::KABC::Addressee &aAddressee) const;
};
// -----------------------------------------------------------------------------
class KabConditionDifferent : public KabConditionCompare
{
    public:
        KabConditionDifferent(
            const ::rtl::OUString &sColumnName,
            const ::rtl::OUString &sMatchString) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool eval(const ::KABC::Addressee &aAddressee) const;
};
// -----------------------------------------------------------------------------
class KabConditionSimilar : public KabConditionCompare
{
    public:
        KabConditionSimilar(
            const ::rtl::OUString &sColumnName,
            const ::rtl::OUString &sMatchString) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool eval(const ::KABC::Addressee &aAddressee) const;
};
// -----------------------------------------------------------------------------
class KabConditionBoolean : public KabCondition
{
    protected:
        KabCondition *m_pLeft, *m_pRight;

    public:
        KabConditionBoolean(KabCondition *pLeft, KabCondition *pRight);
        virtual ~KabConditionBoolean();
};
// -----------------------------------------------------------------------------
class KabConditionOr : public KabConditionBoolean
{
    public:
        KabConditionOr(KabCondition *pLeft, KabCondition *pRight);
        virtual sal_Bool isAlwaysTrue() const;
        virtual sal_Bool isAlwaysFalse() const;
        virtual sal_Bool eval(const ::KABC::Addressee &aAddressee) const;
};
// -----------------------------------------------------------------------------
class KabConditionAnd : public KabConditionBoolean
{
    public:
        KabConditionAnd(KabCondition *pLeft, KabCondition *pRight);
        virtual sal_Bool isAlwaysTrue() const;
        virtual sal_Bool isAlwaysFalse() const;
        virtual sal_Bool eval(const ::KABC::Addressee &addressee) const;
};
// -----------------------------------------------------------------------------
    }
}

#endif // _CONNECTIVITY_KAB_CONDITION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
