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



#ifndef SFX2_DOCSTORAGEMODIFYLISTENER_HXX
#define SFX2_DOCSTORAGEMODIFYLISTENER_HXX

#include "sfx2/dllapi.h"

/** === begin UNO includes === **/
#include <com/sun/star/util/XModifyListener.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>

namespace vos
{
    class IMutex;
}

//........................................................................
namespace sfx2
{
//........................................................................

    //====================================================================
    //= IModifiableDocument
    //====================================================================
    /** callback for the DocumentStorageModifyListener class
    */
    class SAL_NO_VTABLE IModifiableDocument
    {
    public:
        /// indicates the root or a sub storage of the document has been modified
        virtual void storageIsModified() = 0;
    };

    //====================================================================
    //= DocumentStorageModifyListener
    //====================================================================
    typedef ::cppu::WeakImplHelper1 < ::com::sun::star::util::XModifyListener > DocumentStorageModifyListener_Base;

    class SFX2_DLLPUBLIC DocumentStorageModifyListener : public DocumentStorageModifyListener_Base
    {
        IModifiableDocument*    m_pDocument;
        ::vos::IMutex&          m_rMutex;

    public:
        DocumentStorageModifyListener( IModifiableDocument& _rDocument, ::vos::IMutex& _rMutex );

        void dispose();

        // XModifyListener
        virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~DocumentStorageModifyListener();

    private:
        DocumentStorageModifyListener();                                                    // never implemented
        DocumentStorageModifyListener( const DocumentStorageModifyListener& );              // never implemented
        DocumentStorageModifyListener& operator=( const DocumentStorageModifyListener& );   // never implemented
    };

//........................................................................
} // namespace sfx2
//........................................................................

#endif // SFX2_DOCSTORAGEMODIFYLISTENER_HXX
