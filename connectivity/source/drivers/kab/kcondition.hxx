/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: kcondition.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-06 14:23:11 $
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

#ifndef _CONNECTIVITY_KAB_CONDITION_HXX_
#define _CONNECTIVITY_KAB_CONDITION_HXX_

#ifndef _COMPHELPER_TYPES_H_
#include <comphelper/types.hxx>
#endif

#define KDE_HEADERS_WANT_KABC_ADDRESSEE
#include "kde_headers.hxx"

#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

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
