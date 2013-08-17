/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef DBAUI_SINGLEDOCCONTROLLER_HXX
#define DBAUI_SINGLEDOCCONTROLLER_HXX

#include <dbaccess/dbsubcomponentcontroller.hxx>

#include <com/sun/star/document/XUndoManagerSupplier.hpp>

#include <cppuhelper/implbase1.hxx>

class SfxUndoAction;
class SfxUndoManager;

namespace dbaui
{

    // OSingleDocumentController
    struct OSingleDocumentController_Data;
    typedef ::cppu::ImplInheritanceHelper1  <   DBSubComponentController
                                            ,   ::com::sun::star::document::XUndoManagerSupplier
                                            >   OSingleDocumentController_Base;
    class OSingleDocumentController : public OSingleDocumentController_Base
    {
    protected:
        OSingleDocumentController( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxORB );
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

} // namespace dbaui

#endif // DBAUI_SINGLEDOCCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
