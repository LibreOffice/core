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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_SINGLEDOCCONTROLLER_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_SINGLEDOCCONTROLLER_HXX

#include <dbaccess/dbsubcomponentcontroller.hxx>

#include <com/sun/star/document/XUndoManagerSupplier.hpp>

#include <cppuhelper/implbase.hxx>

class SfxUndoAction;
class SfxUndoManager;

namespace dbaui
{

    // OSingleDocumentController
    struct OSingleDocumentController_Data;
    typedef ::cppu::ImplInheritanceHelper<   DBSubComponentController
                                         ,   css::document::XUndoManagerSupplier
                                         >   OSingleDocumentController_Base;
    class OSingleDocumentController : public OSingleDocumentController_Base
    {
    protected:
        OSingleDocumentController( const css::uno::Reference< css::uno::XComponentContext>& _rxORB );
        virtual ~OSingleDocumentController();

        // OComponentHelper
        virtual void SAL_CALL disposing() override;

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
        virtual FeatureState    GetState( sal_uInt16 nId ) const override;
        virtual void            Execute( sal_uInt16 nId, const css::uno::Sequence< css::beans::PropertyValue>& aArgs ) override;

        // XUndoManagerSupplier
        virtual css::uno::Reference< css::document::XUndoManager > SAL_CALL getUndoManager(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw( css::uno::RuntimeException, std::exception ) override;

    private:
        ::std::unique_ptr< OSingleDocumentController_Data >   m_pData;
    };

} // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_SINGLEDOCCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
