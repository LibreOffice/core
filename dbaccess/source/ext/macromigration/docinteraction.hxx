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

#ifndef DBACCESS_DOCERRORHANDLING_HXX
#define DBACCESS_DOCERRORHANDLING_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <memory>

namespace dbmm
{

    // DocumentErrorHandling
    struct InteractionHandler_Data;
    /** wraps common operations with an interaction handler.
    */
    class InteractionHandler
    {
    public:
        /** creates an interaction handler by instantiating a css.task.InteractionHandler
            component at the given component context, or using the given document's interaction handler,
            if one is specified in the document's media descriptor.
        */
        InteractionHandler(
                 const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rContext,
                 const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxDocument );

        /** destructor
        */
        ~InteractionHandler();

        /** requests a document password
            @param _rDocumentName
                the document name
            @param _io_rPassword
                the initial password on method entry, the password as entered by the user on method leave
            @return
                <TRUE/> if and only if the user entered a password, and confirmed with OK, <FALSE/>
                if the user aborted the request.
        */
        bool    requestDocumentPassword(
                    const OUString& _rDocumentName,
                          OUString& _io_rPassword
                );

        /** reports the given error (usually an exception caught on the caller's side)
            to the user
        */
        void    reportError(
                    const ::com::sun::star::uno::Any& _rError
                );

    private:
        ::std::auto_ptr< InteractionHandler_Data >  m_pData;
    };

} // namespace dbmm

#endif // DBACCESS_DOCERRORHANDLING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
