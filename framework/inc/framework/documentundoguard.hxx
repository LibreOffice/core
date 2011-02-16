/*************************************************************************
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

#ifndef FRAMEWORK_DOCUMENTUNDOGUARD_HXX
#define FRAMEWORK_DOCUMENTUNDOGUARD_HXX

#include "framework/fwedllapi.h"

/** === begin UNO includes === **/
#include <com/sun/star/uno/XInterface.hpp>
/** === end UNO includes === **/

#include <boost/scoped_ptr.hpp>

//......................................................................................................................
namespace framework
{
//......................................................................................................................

    //==================================================================================================================
    //= DocumentUndoGuard
    //==================================================================================================================
    struct DocumentUndoGuard_Data;
    /** a helper class guarding the Undo manager of a document

        This class guards, within a given scope, the Undo Manager of a document (or another component supporting
        the XUndoManagerSupplier interface). When entering the scope (i.e. when the <code>DocumentUndoGuard</code>
        instances is constructed), the current state of the undo contexts of the undo manager is examined.
        Upon leaving the scope (i.e. when the <code>DocumentUndoGuard</code> is destructed), the guard will execute
        as many calls to <member scope="com::sun::star::document">XUndoManager::leaveUndoContext</member> as are
        necessary to restore the manager's initial state.
    */
    class FWE_DLLPUBLIC DocumentUndoGuard
    {
    public:
        DocumentUndoGuard( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& i_undoSupplierComponent );
        ~DocumentUndoGuard();

    private:
        ::boost::scoped_ptr< DocumentUndoGuard_Data >   m_pData;
    };

//......................................................................................................................
} // namespace framework
//......................................................................................................................

#endif // FRAMEWORK_DOCUMENTUNDOGUARD_HXX
