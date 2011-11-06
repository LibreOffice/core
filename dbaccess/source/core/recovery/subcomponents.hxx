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



#ifndef SUBCOMPONENTS_HXX
#define SUBCOMPONENTS_HXX

#include "dbaccessdllapi.h"

/** === begin UNO includes === **/
#include <com/sun/star/sdb/application/DatabaseObject.hpp>
/** === end UNO includes === **/

#include <rtl/ustring.hxx>

#include <hash_map>
#include <map>

//........................................................................
namespace dbaccess
{
//........................................................................

    namespace DatabaseObject = ::com::sun::star::sdb::application::DatabaseObject;

    // -------------------------------------------------------------------
    enum SubComponentType
    {
        TABLE = DatabaseObject::TABLE,
        QUERY = DatabaseObject::QUERY,
        FORM = DatabaseObject::FORM,
        REPORT = DatabaseObject::REPORT,

        RELATION_DESIGN = 1000,

        UNKNOWN         = 10001
    };

    // -------------------------------------------------------------------
    struct DBACCESS_DLLPRIVATE SubComponentDescriptor
    {
        ::rtl::OUString     sName;
        bool                bForEditing;

        SubComponentDescriptor()
            :sName()
            ,bForEditing( false )
        {
        }

        SubComponentDescriptor( const ::rtl::OUString& i_rName, const bool i_bForEditing )
            :sName( i_rName )
            ,bForEditing( i_bForEditing )
        {
        }
    };

    // -------------------------------------------------------------------
    typedef ::std::hash_map< ::rtl::OUString, SubComponentDescriptor, ::rtl::OUStringHash > MapStringToCompDesc;
    typedef ::std::map< SubComponentType, MapStringToCompDesc > MapCompTypeToCompDescs;


//........................................................................
} // namespace dbaccess
//........................................................................

#endif // SUBCOMPONENTS_HXX
