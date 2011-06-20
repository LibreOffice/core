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

#ifndef DBAUI_SINGLEDOCCONTROLLER_HXX
#define DBAUI_SINGLEDOCCONTROLLER_HXX

#include "dbsubcomponentcontroller.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/document/XUndoManagerSupplier.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>

class SfxUndoAction;
class SfxUndoManager;

//......................................................................................................................
namespace dbaui
{
//......................................................................................................................

    //==================================================================================================================
    //= OSingleDocumentController
    //==================================================================================================================
    struct OSingleDocumentController_Data;
    typedef ::cppu::ImplInheritanceHelper1  <   DBSubComponentController
                                            ,   ::com::sun::star::document::XUndoManagerSupplier
                                            >   OSingleDocumentController_Base;
    class OSingleDocumentController : public OSingleDocumentController_Base
    {
    protected:
        OSingleDocumentController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxORB );
        virtual ~OSingleDocumentController();

        // OComponentHelper
        virtual void SAL_CALL disposing();

    public:
        /// need for undo's and redo's
        SfxUndoManager& GetUndoManager() const;

        /// complete clears the Undo/Redo stacks
        void ClearUndoManager();

        /** addUndoActionAndInvalidate adds an undo action to the undoManager,
            additionally invalidates the UNDO and REDO slot
            @param  pAction the undo action to add
        */
        void addUndoActionAndInvalidate( SfxUndoAction* pAction );

        // OGenericUnoController
        virtual FeatureState    GetState( sal_uInt16 nId ) const;
        virtual void            Execute( sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs );

        // XUndoManagerSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager > SAL_CALL getUndoManager(  ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

    private:
        ::std::auto_ptr< OSingleDocumentController_Data >   m_pData;
    };

//......................................................................................................................
} // namespace dbaui
//......................................................................................................................

#endif // DBAUI_SINGLEDOCCONTROLLER_HXX
