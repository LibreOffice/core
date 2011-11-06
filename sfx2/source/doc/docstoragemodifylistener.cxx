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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include "sfx2/docstoragemodifylistener.hxx"
#include <vos/mutex.hxx>

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace sfx2
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::lang::EventObject;
    /** === end UNO using === **/

    //====================================================================
    //=
    //====================================================================
    //--------------------------------------------------------------------
    DocumentStorageModifyListener::DocumentStorageModifyListener( IModifiableDocument& _rDocument, ::vos::IMutex& _rMutex )
        :m_pDocument( &_rDocument )
        ,m_rMutex( _rMutex )
    {
    }

    //--------------------------------------------------------------------
    DocumentStorageModifyListener::~DocumentStorageModifyListener()
    {
    }

    //--------------------------------------------------------------------
    void DocumentStorageModifyListener::dispose()
    {
        ::vos::OGuard aGuard( m_rMutex );
        m_pDocument = NULL;
    }

    //--------------------------------------------------------------------
    void SAL_CALL DocumentStorageModifyListener::modified( const EventObject& /*aEvent*/ ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( m_rMutex );
        // storageIsModified must not contain any locking!
        if ( m_pDocument )
            m_pDocument->storageIsModified();
    }

    //--------------------------------------------------------------------
    void SAL_CALL DocumentStorageModifyListener::disposing( const EventObject& /*Source*/ ) throw (RuntimeException)
    {
        // not interested in. In particular, we do *not* dispose ourself when a storage we're
        // listening at is disposed. The reason here is that this listener instance is *reused*
        // in case the document is re-based to another storage.
    }

//........................................................................
} // namespace sfx2
//........................................................................
