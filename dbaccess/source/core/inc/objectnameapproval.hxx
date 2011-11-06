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



#ifndef DBACCESS_OBJECTNAMEAPPROVAL_HXX
#define DBACCESS_OBJECTNAMEAPPROVAL_HXX

#ifndef DBACCESS_CONTAINERAPPROVE_HXX
#include "containerapprove.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
/** === end UNO includes === **/

//........................................................................
namespace dbaccess
{
//........................................................................

    //====================================================================
    //= ObjectNameApproval
    //====================================================================
    struct ObjectNameApproval_Impl;
    /** implementation of the IContainerApprove interface which approves
        elements for insertion into a query or tables container.

        The only check done by this instance is whether the query name is
        not already used, taking into account that in some databases, queries
        and tables share the same namespace.

        The class is not thread-safe.
    */
    class ObjectNameApproval : public IContainerApprove
    {
        ::std::auto_ptr< ObjectNameApproval_Impl >   m_pImpl;

    public:
        enum ObjectType
        {
            TypeQuery,
            TypeTable
        };

    public:
        /** constructs the instance

            @param _rxConnection
                the connection relative to which the names should be checked. This connection
                will be held weak. In case it is closed, subsequent calls to this instance's
                methods throw a DisposedException.
            @param _eType
                specifies which type of objects is to be approved with this instance
        */
        ObjectNameApproval(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            ObjectType _eType
        );
        virtual ~ObjectNameApproval();

        // IContainerApprove
        virtual void SAL_CALL approveElement( const ::rtl::OUString& _rName, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement );

    };

//........................................................................
} // namespace dbaccess
//........................................................................

#endif // DBACCESS_OBJECTNAMEAPPROVAL_HXX
