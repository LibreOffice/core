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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_KAB_KCONDITION_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_KAB_KCONDITION_HXX

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
        virtual bool isAlwaysTrue() const = 0;
        virtual bool isAlwaysFalse() const = 0;
        virtual bool eval(const ::KABC::Addressee &aAddressee) const = 0;
};

class KabConditionConstant : public KabCondition
{
    protected:
        bool m_bValue;

    public:
        explicit KabConditionConstant(const bool bValue);
        virtual bool isAlwaysTrue() const override;
        virtual bool isAlwaysFalse() const override;
        virtual bool eval(const ::KABC::Addressee &aAddressee) const override;
};

class KabConditionColumn : public KabCondition
{
    protected:
        sal_Int32 m_nFieldNumber;

        QString value(const ::KABC::Addressee &aAddressee) const;

    public:
        explicit KabConditionColumn(
            const OUString &sColumnName) throw(::com::sun::star::sdbc::SQLException);
        virtual bool isAlwaysTrue() const override;
        virtual bool isAlwaysFalse() const override;
};

class KabConditionNull : public KabConditionColumn
{
    public:
        explicit KabConditionNull(
            const OUString &sColumnName) throw(::com::sun::star::sdbc::SQLException);
        virtual bool eval(const ::KABC::Addressee &aAddressee) const override;
};

class KabConditionNotNull : public KabConditionColumn
{
    public:
        explicit KabConditionNotNull(
            const OUString &sColumnName) throw(::com::sun::star::sdbc::SQLException);
        virtual bool eval(const ::KABC::Addressee &aAddressee) const override;
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
        virtual bool eval(const ::KABC::Addressee &aAddressee) const override;
};

class KabConditionDifferent : public KabConditionCompare
{
    public:
        KabConditionDifferent(
            const OUString &sColumnName,
            const OUString &sMatchString) throw(::com::sun::star::sdbc::SQLException);
        virtual bool eval(const ::KABC::Addressee &aAddressee) const override;
};

class KabConditionSimilar : public KabConditionCompare
{
    public:
        KabConditionSimilar(
            const OUString &sColumnName,
            const OUString &sMatchString) throw(::com::sun::star::sdbc::SQLException);
        virtual bool eval(const ::KABC::Addressee &aAddressee) const override;
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
        virtual bool isAlwaysTrue() const override;
        virtual bool isAlwaysFalse() const override;
        virtual bool eval(const ::KABC::Addressee &aAddressee) const override;
};

class KabConditionAnd : public KabConditionBoolean
{
    public:
        KabConditionAnd(KabCondition *pLeft, KabCondition *pRight);
        virtual bool isAlwaysTrue() const override;
        virtual bool isAlwaysFalse() const override;
        virtual bool eval(const ::KABC::Addressee &addressee) const override;
};

    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_KAB_KCONDITION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
