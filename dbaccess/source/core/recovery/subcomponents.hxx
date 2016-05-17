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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_RECOVERY_SUBCOMPONENTS_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_RECOVERY_SUBCOMPONENTS_HXX

#include <dbaccess/dbaccessdllapi.h>

#include <com/sun/star/sdb/application/DatabaseObject.hpp>

#include <rtl/ustring.hxx>

#include <map>
#include <unordered_map>

namespace dbaccess
{

    enum SubComponentType
    {
        TABLE = css::sdb::application::DatabaseObject::TABLE,
        QUERY = css::sdb::application::DatabaseObject::QUERY,
        FORM = css::sdb::application::DatabaseObject::FORM,
        REPORT = css::sdb::application::DatabaseObject::REPORT,

        RELATION_DESIGN = 1000,

        UNKNOWN         = 10001
    };

    struct DBACCESS_DLLPRIVATE SubComponentDescriptor
    {
        OUString     sName;
        bool                bForEditing;

        SubComponentDescriptor()
            :sName()
            ,bForEditing( false )
        {
        }
    };

    typedef std::unordered_map< OUString, SubComponentDescriptor, OUStringHash > MapStringToCompDesc;
    typedef std::map< SubComponentType, MapStringToCompDesc > MapCompTypeToCompDescs;

} // namespace dbaccess

#endif // INCLUDED_DBACCESS_SOURCE_CORE_RECOVERY_SUBCOMPONENTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
