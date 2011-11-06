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



#ifndef DBACCESS_DBDOCRECOVERY_HXX
#define DBACCESS_DBDOCRECOVERY_HXX

#include "dbaccessdllapi.h"

/** === begin UNO includes === **/
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/XController.hpp>
/** === end UNO includes === **/

#include <vector>
#include <memory>

namespace comphelper
{
    class ComponentContext;
}

//........................................................................
namespace dbaccess
{
//........................................................................

    //====================================================================
    //= DatabaseDocumentRecovery
    //====================================================================
    struct DatabaseDocumentRecovery_Data;
    class DBACCESS_DLLPRIVATE DatabaseDocumentRecovery
    {
    public:
        DatabaseDocumentRecovery(
            const ::comphelper::ComponentContext& i_rContext
        );
        ~DatabaseDocumentRecovery();

        /** saves the modified sub components of the given controller(s) to the "recovery" sub storage of the document
            storage.

            @throws ::com::sun::star::uno::Exception
                in case of an error.
        */
        void saveModifiedSubComponents(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rTargetStorage,
                const ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > >& i_rControllers
            );

        /** recovery sub components from the given document storage, if applicable

            If the given document storage does not contain a recovery folder, the method silently returns.

            @throws ::com::sun::star::uno::Exception
                in case of an error.
        */
        void recoverSubDocuments(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rDocumentStorage,
                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& i_rTargetController
            );

    private:
        const ::std::auto_ptr< DatabaseDocumentRecovery_Data >  m_pData;
    };

//........................................................................
} // namespace dbaccess
//........................................................................

#endif // DBACCESS_DBDOCRECOVERY_HXX
