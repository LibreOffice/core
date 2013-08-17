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
#ifndef DBAUI_APPELEMENTTYPE_HXX
#define DBAUI_APPELEMENTTYPE_HXX

#include <com/sun/star/sdb/application/DatabaseObject.hpp>

namespace dbaui
{

    enum ElementType
    {
        E_TABLE     = ::com::sun::star::sdb::application::DatabaseObject::TABLE,
        E_QUERY     = ::com::sun::star::sdb::application::DatabaseObject::QUERY,
        E_FORM      = ::com::sun::star::sdb::application::DatabaseObject::FORM,
        E_REPORT    = ::com::sun::star::sdb::application::DatabaseObject::REPORT,

        E_NONE      = 4,
        E_ELEMENT_TYPE_COUNT = E_NONE
    };

    enum PreviewMode
    {
        E_PREVIEWNONE   = 0,
        E_DOCUMENT      = 1,
        E_DOCUMENTINFO  = 2
    };

    enum ElementOpenMode
    {
        E_OPEN_NORMAL,
        E_OPEN_DESIGN,
        E_OPEN_FOR_MAIL
    };

} // namespace dbaui
#endif // DBAUI_APPELEMENTTYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
