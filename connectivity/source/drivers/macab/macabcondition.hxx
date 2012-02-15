/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _CONNECTIVITY_MACAB_CONDITION_HXX_
#define _CONNECTIVITY_MACAB_CONDITION_HXX_

#include "MacabHeader.hxx"
#include "MacabRecord.hxx"

#ifndef _COMPHELPER_TYPES_H_
#include <comphelper/types.hxx>
#endif
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
