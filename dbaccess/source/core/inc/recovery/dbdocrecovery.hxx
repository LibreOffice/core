/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

namespace dbaccess
{

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

} // namespace dbaccess

#endif // DBACCESS_DBDOCRECOVERY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
