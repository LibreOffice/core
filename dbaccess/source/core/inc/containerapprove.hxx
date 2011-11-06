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



#ifndef DBACCESS_CONTAINERAPPROVE_HXX
#define DBACCESS_CONTAINERAPPROVE_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
/** === end UNO includes === **/

#include <boost/shared_ptr.hpp>

//........................................................................
namespace dbaccess
{
//........................................................................

    //====================================================================
    //= IContainerApprove
    //====================================================================
    /** interface for approving elements to be inserted into a container

        On the long run, one could imagine that this interface completely encapsulates
        container/element approvals in all our various container classes herein (document
        containers, definition containers, table containers, query containers,
        command definition containers, bookmark containers). This would decrease coupling
        of the respective classes.
     */
    class SAL_NO_VTABLE IContainerApprove
    {
    public:
        /** approves a given element for insertion into the container
            @param  _rName
                specifies the name under which the element is going to be inserted
            @param  _rxElement
                specifies the element which is going to be inserted
            @throws Exception
                if the name or the object are invalid, or not eligible for insertion
                into the container
        */
        virtual void SAL_CALL   approveElement(
            const ::rtl::OUString& _rName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement
        ) = 0;
    };

    typedef ::boost::shared_ptr< IContainerApprove >    PContainerApprove;

//........................................................................
} // namespace dbaccess
//........................................................................

#endif // DBACCESS_CONTAINERAPPROVE_HXX

