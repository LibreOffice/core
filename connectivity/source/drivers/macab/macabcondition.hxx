/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macabcondition.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-13 17:56:28 $
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

#ifndef _CONNECTIVITY_MACAB_CONDITION_HXX_
#define _CONNECTIVITY_MACAB_CONDITION_HXX_

#ifndef _CONNECTIVITY_MACAB_HEADER_HXX_
#include "MacabHeader.hxx"
#endif

#ifndef _CONNECTIVITY_MACAB_RECORD_HXX_
#include "MacabRecord.hxx"
#endif

#ifndef _COMPHELPER_TYPES_H_
#include <comphelper/types.hxx>
#endif

#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

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
