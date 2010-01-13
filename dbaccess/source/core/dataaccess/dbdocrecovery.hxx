/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
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
************************************************************************/

#ifndef DBACCESS_DBDOCRECOVERY_HXX
#define DBACCESS_DBDOCRECOVERY_HXX

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
    class DatabaseDocumentRecovery
    {
    public:
        DatabaseDocumentRecovery(
            const ::comphelper::ComponentContext& i_rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rDocumentSalvageStorage
        );
        ~DatabaseDocumentRecovery();

        /** saves the modified sub components of the given controller(s) to the "recovery" sub storage of the document
            storage.
        */
        void saveModifiedSubComponents(
                const ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > >& i_rControllers
            );

    private:
        const ::std::auto_ptr< DatabaseDocumentRecovery_Data >  m_pData;
    };

//........................................................................
} // namespace dbaccess
//........................................................................

#endif // DBACCESS_DBDOCRECOVERY_HXX
