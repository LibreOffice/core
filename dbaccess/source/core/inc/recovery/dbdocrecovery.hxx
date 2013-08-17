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

#ifndef DBACCESS_DBDOCRECOVERY_HXX
#define DBACCESS_DBDOCRECOVERY_HXX

#include <dbaccess/dbaccessdllapi.h>

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vector>
#include <memory>

namespace dbaccess
{

    // DatabaseDocumentRecovery
    struct DatabaseDocumentRecovery_Data;
    class DBACCESS_DLLPRIVATE DatabaseDocumentRecovery
    {
    public:
        DatabaseDocumentRecovery(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_rContext
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
