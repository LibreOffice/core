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

#ifndef _CONNECTIVITY_KAB_CONDITION_HXX_
#define _CONNECTIVITY_KAB_CONDITION_HXX_

#include <comphelper/types.hxx>
#include <shell/kde_headers.h>
#include <connectivity/dbexception.hxx>

namespace connectivity
{
    namespace kab
    {

class KabCondition
{
    public:
        virtual ~KabCondition();
        virtual sal_Bool isAlwaysTrue() const = 0;
        virtual sal_Bool isAlwaysFalse() const = 0;
        virtual sal_Bool eval(const ::KABC::Addressee &aAddressee) const = 0;
};

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

class KabConditionColumn : public KabCondition
{
    protected:
        sal_Int32 m_nFieldNumber;

        QString value(const ::KABC::Addressee &aAddressee) const;

    public:
        KabConditionColumn(
            const OUString &sColumnName) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool isAlwaysTrue() const;
        virtual sal_Bool isAlwaysFalse() const;
};

class KabConditionNull : public KabConditionColumn
{
    public:
        KabConditionNull(
            const OUString &sColumnName) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool eval(const ::KABC::Addressee &aAddressee) const;
};

class KabConditionNotNull : public KabConditionColumn
{
    public:
        KabConditionNotNull(
            const OUString &sColumnName) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool eval(const ::KABC::Addressee &aAddressee) const;
};

class KabConditionCompare : public KabConditionColumn
{
    protected:
        const OUString m_sMatchString;

    public:
        KabConditionCompare(
            const OUString &sColumnName,
            const OUString &sMatchString) throw(::com::sun::star::sdbc::SQLException);
};

class KabConditionEqual : public KabConditionCompare
{
    public:
        KabConditionEqual(
            const OUString &sColumnName,
            const OUString &sMatchString) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool eval(const ::KABC::Addressee &aAddressee) const;
};

class KabConditionDifferent : public KabConditionCompare
{
    public:
        KabConditionDifferent(
            const OUString &sColumnName,
            const OUString &sMatchString) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool eval(const ::KABC::Addressee &aAddressee) const;
};

class KabConditionSimilar : public KabConditionCompare
{
    public:
        KabConditionSimilar(
            const OUString &sColumnName,
            const OUString &sMatchString) throw(::com::sun::star::sdbc::SQLException);
        virtual sal_Bool eval(const ::KABC::Addressee &aAddressee) const;
};

class KabConditionBoolean : public KabCondition
{
    protected:
        KabCondition *m_pLeft, *m_pRight;

    public:
        KabConditionBoolean(KabCondition *pLeft, KabCondition *pRight);
        virtual ~KabConditionBoolean();
};

class KabConditionOr : public KabConditionBoolean
{
    public:
        KabConditionOr(KabCondition *pLeft, KabCondition *pRight);
        virtual sal_Bool isAlwaysTrue() const;
        virtual sal_Bool isAlwaysFalse() const;
        virtual sal_Bool eval(const ::KABC::Addressee &aAddressee) const;
};

class KabConditionAnd : public KabConditionBoolean
{
    public:
        KabConditionAnd(KabCondition *pLeft, KabCondition *pRight);
        virtual sal_Bool isAlwaysTrue() const;
        virtual sal_Bool isAlwaysFalse() const;
        virtual sal_Bool eval(const ::KABC::Addressee &addressee) const;
};

    }
}

#endif // _CONNECTIVITY_KAB_CONDITION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
