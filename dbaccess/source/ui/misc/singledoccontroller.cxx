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

#include <dbaccess/dbaundomanager.hxx>
#include <dbaccess/dataview.hxx>
#include "core_resource.hxx"
#include "singledoccontroller.hxx"
#include "browserids.hxx"
#include "strings.hrc"
#include "stringconstants.hxx"

#include <svl/undo.hxx>

namespace dbaui
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::document::XUndoManager;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::lang::EventObject;

    // OSingleDocumentController_Data
    struct OSingleDocumentController_Data
    {
        rtl::Reference< UndoManager >  m_xUndoManager;

        OSingleDocumentController_Data( ::cppu::OWeakObject& i_parent, ::osl::Mutex& i_mutex )
            :m_xUndoManager( new UndoManager( i_parent, i_mutex ) )
        {
        }
    };

    // OSingleDocumentController
    OSingleDocumentController::OSingleDocumentController( const Reference< XComponentContext >& _rxORB )
        :OSingleDocumentController_Base( _rxORB )
        ,m_pData( new OSingleDocumentController_Data( *this, getMutex() ) )
    {
    }

    OSingleDocumentController::~OSingleDocumentController()
    {
    }

    void SAL_CALL OSingleDocumentController::disposing()
    {
        OSingleDocumentController_Base::disposing();
        ClearUndoManager();
        m_pData->m_xUndoManager->disposing();
    }

    void OSingleDocumentController::ClearUndoManager()
    {
        GetUndoManager().Clear();
    }

    SfxUndoManager& OSingleDocumentController::GetUndoManager() const
    {
        return m_pData->m_xUndoManager->GetSfxUndoManager();
    }

    void OSingleDocumentController::addUndoActionAndInvalidate(SfxUndoAction *_pAction)
    {
        // add undo action
        GetUndoManager().AddUndoAction( _pAction );

        // when we add an undo action the controller was modified
        setModified( true );

        // now inform me that or states changed
        InvalidateFeature( ID_BROWSER_UNDO );
        InvalidateFeature( ID_BROWSER_REDO );
    }

    Reference< XUndoManager > SAL_CALL OSingleDocumentController::getUndoManager(  )
    {
        return m_pData->m_xUndoManager.get();
    }

    FeatureState OSingleDocumentController::GetState(sal_uInt16 _nId) const
    {
        FeatureState aReturn;
        switch ( _nId )
        {
            case ID_BROWSER_UNDO:
                aReturn.bEnabled = isEditable() && GetUndoManager().GetUndoActionCount() != 0;
                if ( aReturn.bEnabled )
                {
                    OUString sUndo(DBA_RES(STR_UNDO_COLON));
                    sUndo += " ";
                    sUndo += GetUndoManager().GetUndoActionComment();
                    aReturn.sTitle = sUndo;
                }
                break;

            case ID_BROWSER_REDO:
                aReturn.bEnabled = isEditable() && GetUndoManager().GetRedoActionCount() != 0;
                if ( aReturn.bEnabled )
                {
                    OUString sRedo(DBA_RES(STR_REDO_COLON));
                    sRedo += " ";
                    sRedo += GetUndoManager().GetRedoActionComment();
                    aReturn.sTitle = sRedo;
                }
                break;

            default:
                aReturn = OSingleDocumentController_Base::GetState(_nId);
        }
        return aReturn;
    }
    void OSingleDocumentController::Execute( sal_uInt16 _nId, const Sequence< PropertyValue >& _rArgs )
    {
        switch ( _nId )
        {
            case ID_BROWSER_UNDO:
                GetUndoManager().Undo();
                InvalidateFeature( ID_BROWSER_UNDO );
                InvalidateFeature( ID_BROWSER_REDO );
                break;

            case ID_BROWSER_REDO:
                GetUndoManager().Redo();
                InvalidateFeature( ID_BROWSER_UNDO );
                InvalidateFeature( ID_BROWSER_REDO );
                break;

            default:
                OSingleDocumentController_Base::Execute( _nId, _rArgs );
                break;
        }
        InvalidateFeature(_nId);
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
