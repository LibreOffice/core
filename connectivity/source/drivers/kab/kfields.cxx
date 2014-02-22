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


#include "kfields.hxx"
#include "resource/common_res.hrc"
#include "resource/sharedresources.hxx"

using namespace ::connectivity::kab;
using namespace ::com::sun::star::sdbc;

namespace connectivity
{
    namespace kab
    {

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

// search the KDE address book field number of a given column name
sal_uInt32 findKabField(const OUString& columnName) throw(SQLException)
{
    QString aQtName;
    OUString aName;

    aQtName = KABC::Addressee::revisionLabel();
    aName = OUString((const sal_Unicode *) aQtName.ucs2());
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
        aName = OUString((const sal_Unicode *) aQtName.ucs2());

        if (columnName == aName)
            return nResult;
    }

    ::connectivity::SharedResources aResources;
    const OUString sError( aResources.getResourceStringWithSubstitution(
            STR_INVALID_COLUMNNAME,
            "$columnname$",columnName
         ) );
    ::dbtools::throwGenericSQLException(sError,NULL);
    // Unreachable:
    OSL_ASSERT(false);
    return 0;
}

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
