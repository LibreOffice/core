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

#ifndef _CONNECTIVITY_MACAB_CONDITION_HXX_
#define _CONNECTIVITY_MACAB_CONDITION_HXX_

#include "MacabHeader.hxx"
#include "MacabRecord.hxx"

#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>

namespace connectivity
{
    namespace macab
    {
// -----------------------------------------------------------------------------
class MacabCondition
{
    public:
        virtual ~MacabCondition();
        virtual sal_Bool isAlwaysTrue() const = 0;
        virtual sal_Bool isAlwaysFalse() const = 0;
        virtual sal_Bool eval(const MacabRecord *aRecord) const = 0;
};
// -----------------------------------------------------------------------------
class MacabConditionConstant : public MacabCondition
{
    protected:
        sal_Bool m_bValue;

    public:
        MacabConditionConstant(const sal_Bool bValue);
        virtual sal_Bool isAlwaysTrue() const;
        virtual sal_Bool isAlwaysFalse() const;
        virtual sal_Bool eval(const MacabRecord *aRecord) const;
};
// -----------------------------------------------------------------------------
class MacabConditionColumn : public MacabCondition
{
    protected:
        sal_Int32 m_nFieldNumber;

    public:
        MacabConditionColumn(
            const MacabHeader *header,
            const ::rtl::OUString &sColumnName) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool isAlwaysTrue() const;
        virtual sal_Bool isAlwaysFalse() const;
};
// -----------------------------------------------------------------------------
class MacabConditionNull : public MacabConditionColumn
{
    public:
        MacabConditionNull(
            const MacabHeader *header,
            const ::rtl::OUString &sColumnName) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool eval(const MacabRecord *aRecord) const;
};
// -----------------------------------------------------------------------------
class MacabConditionNotNull : public MacabConditionColumn
{
    public:
        MacabConditionNotNull(
            const MacabHeader *header,
            const ::rtl::OUString &sColumnName) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool eval(const MacabRecord *aRecord) const;
};
// -----------------------------------------------------------------------------
class MacabConditionCompare : public MacabConditionColumn
{
    protected:
        const ::rtl::OUString m_sMatchString;

    public:
        MacabConditionCompare(
            const MacabHeader *header,
            const ::rtl::OUString &sColumnName,
            const ::rtl::OUString &sMatchString) throw(::com::sun::star::sdbc::SQLException);
};
// -----------------------------------------------------------------------------
class MacabConditionEqual : public MacabConditionCompare
{
    public:
        MacabConditionEqual(
            const MacabHeader *header,
            const ::rtl::OUString &sColumnName,
            const ::rtl::OUString &sMatchString) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool eval(const MacabRecord *aRecord) const;
};
// -----------------------------------------------------------------------------
class MacabConditionDifferent : public MacabConditionCompare
{
    public:
        MacabConditionDifferent(
            const MacabHeader *header,
            const ::rtl::OUString &sColumnName,
            const ::rtl::OUString &sMatchString) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool eval(const MacabRecord *aRecord) const;
};
// -----------------------------------------------------------------------------
class MacabConditionSimilar : public MacabConditionCompare
{
    public:
        MacabConditionSimilar(
            const MacabHeader *header,
            const ::rtl::OUString &sColumnName,
            const ::rtl::OUString &sMatchString) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool eval(const MacabRecord *aRecord) const;
};
// -----------------------------------------------------------------------------
class MacabConditionBoolean : public MacabCondition
{
    protected:
        MacabCondition *m_pLeft, *m_pRight;

    public:
        MacabConditionBoolean(MacabCondition *pLeft, MacabCondition *pRight);
        virtual ~MacabConditionBoolean();
};
// -----------------------------------------------------------------------------
class MacabConditionOr : public MacabConditionBoolean
{
    public:
        MacabConditionOr(MacabCondition *pLeft, MacabCondition *pRight);
        virtual sal_Bool isAlwaysTrue() const;
        virtual sal_Bool isAlwaysFalse() const;
        virtual sal_Bool eval(const MacabRecord *aRecord) const;
};
// -----------------------------------------------------------------------------
class MacabConditionAnd : public MacabConditionBoolean
{
    public:
        MacabConditionAnd(MacabCondition *pLeft, MacabCondition *pRight);
        virtual sal_Bool isAlwaysTrue() const;
        virtual sal_Bool isAlwaysFalse() const;
        virtual sal_Bool eval(const MacabRecord *aRecord) const;
};
// -----------------------------------------------------------------------------
    }
}

#endif // _CONNECTIVITY_MACAB_CONDITION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
