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

#ifndef _DBA_CORE_COMMANDBASE_HXX_
#define _DBA_CORE_COMMANDBASE_HXX_

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

namespace dbaccess
{

// OCommandBase - a base class (in fact just a container for some members)
//                 for classes implementing the sdb.CommandDefinition service
class OCommandBase
{
public: // need public access
// <properties>
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>
                                                m_aLayoutInformation;
    OUString                             m_sCommand;
    sal_Bool                                    m_bEscapeProcessing;        // no BitField ! so it can be used with a OPropertyStateContainer
    OUString                             m_sUpdateTableName;
    OUString                             m_sUpdateSchemaName;
    OUString                             m_sUpdateCatalogName;
// </properties>

protected:
    OCommandBase() : m_bEscapeProcessing(sal_True) { }

};

}   // namespace dbaccess

#endif // _DBA_CORE_COMMANDBASE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
