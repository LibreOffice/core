/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
 *
 ************************************************************************/

#ifndef DBACCESS_DOCERRORHANDLING_HXX
#define DBACCESS_DOCERRORHANDLING_HXX

/** === begin UNO includes === **/
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
/** === end UNO includes === **/

namespace comphelper {
    class ComponentContext;
}

#include <memory>

//........................................................................
namespace dbmm
{
//........................................................................

    //====================================================================
    //= DocumentErrorHandling
    //====================================================================
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
        InteractionHandler( const ::comphelper::ComponentContext& _rContext, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxDocument );

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
                    const ::rtl::OUString& _rDocumentName,
                          ::rtl::OUString& _io_rPassword
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

//........................................................................
} // namespace dbmm
//........................................................................

#endif // DBACCESS_DOCERRORHANDLING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
