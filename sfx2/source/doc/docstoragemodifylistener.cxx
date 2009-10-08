/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: docstoragemodifylistener.cxx,v $
 * $Revision: 1.4.28.1 $
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
