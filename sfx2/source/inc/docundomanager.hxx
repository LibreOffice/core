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

#ifndef DOCUMENT_UNDO_MANAGER_HXX
#define DOCUMENT_UNDO_MANAGER_HXX

#include "sfx2/sfxbasemodel.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/document/XUndoManager.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>

#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>


//......................................................................................................................
namespace sfx2
{
//......................................................................................................................

    //==================================================================================================================
    //= DocumentUndoManager
    //==================================================================================================================
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::document::XUndoManager
                                >   DocumentUndoManager_Base;
    struct DocumentUndoManager_Data;
    class DocumentUndoManager   :public DocumentUndoManager_Base
                                ,public SfxModelSubComponent
                                ,public ::boost::noncopyable
    {
    public:
        DocumentUndoManager( SfxBaseModel& i_document );
        virtual ~DocumentUndoManager();

        /** renders the instance non-functional. To be called when the document it belongs to is disposed.
        */
        void    dispose();

        // XInterface
        virtual void SAL_CALL acquire(  ) throw ();
        virtual void SAL_CALL release(  ) throw ();

        // XUndoManager
        virtual void SAL_CALL enterUndoContext( const ::rtl::OUString& i_title ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL enterHiddenUndoContext(  ) throw (::com::sun::star::util::InvalidStateException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL leaveUndoContext(  ) throw (::com::sun::star::util::InvalidStateException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addUndoAction( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoAction >& i_action ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL undo(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL redo(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL clear(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL clearRedo(  ) throw (::com::sun::star::uno::RuntimeException);


    private:
        ::boost::scoped_ptr< DocumentUndoManager_Data > m_pData;
    };

//......................................................................................................................
} // namespace sfx2
//......................................................................................................................

#endif // DOCUMENT_UNDO_MANAGER_HXX
