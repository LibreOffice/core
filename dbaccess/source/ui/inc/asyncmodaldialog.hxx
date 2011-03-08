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

#ifndef DBACCESS_ASYNCMODALDIALOG_HXX
#define DBACCESS_ASYNCMODALDIALOG_HXX

/** === begin UNO includes === **/
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
/** === end UNO includes === **/

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= AsyncDialogExecutor
    //====================================================================
    /** helper class for executing (UNO) dialogs modal, but asynchronously
    */
    class AsyncDialogExecutor
    {
    public:
        /** executes the given dialog asynchronously, but still modal

            @raises IllegalArgumentException
                if the given dialog is <NULL/>
            @todo
                allow for a callback for the result
        */
        static void executeModalDialogAsync(
            const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XExecutableDialog >& _rxDialog
        );
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_ASYNCMODALDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
