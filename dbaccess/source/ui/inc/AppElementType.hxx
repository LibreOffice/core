/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef DBAUI_APPELEMENTTYPE_HXX
#define DBAUI_APPELEMENTTYPE_HXX

#include <com/sun/star/sdb/application/DatabaseObject.hpp>

//........................................................................
namespace dbaui
{
//........................................................................

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

//........................................................................
} // namespace dbaui
//........................................................................
#endif // DBAUI_APPELEMENTTYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
