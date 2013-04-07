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
            const OUString &sColumnName) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool isAlwaysTrue() const;
        virtual sal_Bool isAlwaysFalse() const;
};
// -----------------------------------------------------------------------------
class MacabConditionNull : public MacabConditionColumn
{
    public:
        MacabConditionNull(
            const MacabHeader *header,
            const OUString &sColumnName) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool eval(const MacabRecord *aRecord) const;
};
// -----------------------------------------------------------------------------
class MacabConditionNotNull : public MacabConditionColumn
{
    public:
        MacabConditionNotNull(
            const MacabHeader *header,
            const OUString &sColumnName) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool eval(const MacabRecord *aRecord) const;
};
// -----------------------------------------------------------------------------
class MacabConditionCompare : public MacabConditionColumn
{
    protected:
        const OUString m_sMatchString;

    public:
        MacabConditionCompare(
            const MacabHeader *header,
            const OUString &sColumnName,
            const OUString &sMatchString) throw(::com::sun::star::sdbc::SQLException);
};
// -----------------------------------------------------------------------------
class MacabConditionEqual : public MacabConditionCompare
{
    public:
        MacabConditionEqual(
            const MacabHeader *header,
            const OUString &sColumnName,
            const OUString &sMatchString) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool eval(const MacabRecord *aRecord) const;
};
// -----------------------------------------------------------------------------
class MacabConditionDifferent : public MacabConditionCompare
{
    public:
        MacabConditionDifferent(
            const MacabHeader *header,
            const OUString &sColumnName,
            const OUString &sMatchString) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool eval(const MacabRecord *aRecord) const;
};
// -----------------------------------------------------------------------------
class MacabConditionSimilar : public MacabConditionCompare
{
    public:
        MacabConditionSimilar(
            const MacabHeader *header,
            const OUString &sColumnName,
            const OUString &sMatchString) throw(::com::sun::star::sdbc::SQLException);
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
