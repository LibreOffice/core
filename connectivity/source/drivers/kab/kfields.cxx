/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: kfields.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:41:14 $
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

#include "kfields.hxx"

using namespace ::connectivity::kab;
using namespace ::com::sun::star::sdbc;

namespace connectivity
{
    namespace kab
    {
// -----------------------------------------------------------------------------
// return the value of a KDE address book field, given an addressee and a field number
QString valueOfKabField(const ::KABC::Addressee &aAddressee, sal_Int32 nFieldNumber)
{
    switch (nFieldNumber)
    {
        case KAB_FIELD_REVISION:
            return aAddressee.revision().toString("yyyy-MM-dd hh:mm:ss");
        default:
            ::KABC::Field::List aFields = ::KABC::Field::allFields();
            return aFields[nFieldNumber - KAB_DATA_FIELDS]->value(aAddressee);
    }
}
// ------------------------------------------------------------------------------
// search the KDE address book field number of a given column name
sal_uInt32 findKabField(const ::rtl::OUString& columnName) throw(SQLException)
{
    QString aQtName;
    ::rtl::OUString aName;

    aQtName = KABC::Addressee::revisionLabel();
    aName = ::rtl::OUString((const sal_Unicode *) aQtName.ucs2());
    if (columnName == aName)
        return KAB_FIELD_REVISION;

    ::KABC::Field::List aFields = ::KABC::Field::allFields();
    ::KABC::Field::List::iterator aField;
    sal_uInt32 nResult;

    for (   aField = aFields.begin(), nResult = KAB_DATA_FIELDS;
            aField != aFields.end();
            ++aField, ++nResult)
    {
        aQtName = (*aField)->label();
        aName = ::rtl::OUString((const sal_Unicode *) aQtName.ucs2());

        if (columnName == aName)
            return nResult;
    }

    ::dbtools::throwGenericSQLException(
        ::rtl::OUString::createFromAscii("Invalid column name: ") + columnName,
        NULL);
    // Unreachable:
    OSL_ASSERT(false);
    return 0;
}
// ------------------------------------------------------------------------------
    }
}
