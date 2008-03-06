/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docstoragemodifylistener.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:54:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include "sfx2/docstoragemodifylistener.hxx"

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
    DocumentStorageModifyListener::DocumentStorageModifyListener( ::osl::Mutex& _rMutex, IModifiableDocument& _rDocument )
        :m_rMutex( _rMutex )
        ,m_pDocument( &_rDocument )
    {
    }

    //--------------------------------------------------------------------
    DocumentStorageModifyListener::~DocumentStorageModifyListener()
    {
    }

    //--------------------------------------------------------------------
    void DocumentStorageModifyListener::dispose()
    {
        ::osl::MutexGuard aGuard( m_rMutex );
        m_pDocument = NULL;
    }

    //--------------------------------------------------------------------
    void SAL_CALL DocumentStorageModifyListener::modified( const EventObject& /*aEvent*/ ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_rMutex );
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
