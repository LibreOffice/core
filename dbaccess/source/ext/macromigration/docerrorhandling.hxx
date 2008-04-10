/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: docerrorhandling.hxx,v $
 * $Revision: 1.3 $
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
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
/** === end UNO includes === **/

namespace comphelper {
    class ComponentContext;
}

//........................................................................
namespace dbmm
{
//........................................................................

    //====================================================================
    //= DocumentErrorHandling
    //====================================================================
    class DocumentErrorHandling
    {
    public:
        /** reports the given error (usually an exception caught on the caller's side)
            to the user, using the document's interaction handler, if present. If the document
            does not have an own interaction handler, the given component context is used
            to create a new one.
        */
        static void reportError(
                const ::comphelper::ComponentContext& _rContext,
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XOfficeDatabaseDocument >& _rxDocument,
                const ::com::sun::star::uno::Any& _rError
               );
    };

//........................................................................
} // namespace dbmm
//........................................................................

#endif // DBACCESS_DOCERRORHANDLING_HXX
