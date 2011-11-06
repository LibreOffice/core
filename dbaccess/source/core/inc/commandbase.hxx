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



#ifndef _DBA_CORE_COMMANDBASE_HXX_
#define _DBA_CORE_COMMANDBASE_HXX_

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

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

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_CORE_COMMANDBASE_HXX_


