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

#ifndef _DBA_CORE_COMMANDBASE_HXX_
#define _DBA_CORE_COMMANDBASE_HXX_

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

namespace dbaccess
{

//==========================================================================
//= OCommandBase - a base class (in fact just a container for some members)
//=                 for classes implementing the sdb.CommandDefinition service
//==========================================================================
class OCommandBase
{
public: // need public access
// <properties>
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>
                                                m_aLayoutInformation;
    ::rtl::OUString                             m_sCommand;
    sal_Bool                                    m_bEscapeProcessing;        // no BitField ! so it can be used with a OPropertyStateContainer
    ::rtl::OUString                             m_sUpdateTableName;
    ::rtl::OUString                             m_sUpdateSchemaName;
    ::rtl::OUString                             m_sUpdateCatalogName;
// </properties>

protected:
    OCommandBase() : m_bEscapeProcessing(sal_True) { }

};

}   // namespace dbaccess

#endif // _DBA_CORE_COMMANDBASE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
