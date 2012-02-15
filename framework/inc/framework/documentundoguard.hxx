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
