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
#include "singledoccontroller.hxx"
#include "browserids.hxx"
#include "dbu_misc.hrc"
#include "dbustrings.hrc"
#include "moduledbu.hxx"

#include <svl/undo.hxx>
#include <osl/diagnose.h>

#include <boost/scoped_ptr.hpp>

namespace dbaui
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::document::XUndoManager;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::lang::EventObject;

    // OSingleDocumentController_Data
    struct OSingleDocumentController_Data
    {
        ::boost::scoped_ptr< UndoManager >  m_pUndoManager;

        OSingleDocumentController_Data( ::cppu::OWeakObject& i_parent, ::osl::Mutex& i_mutex )
            :m_pUndoManager( new UndoManager( i_parent, i_mutex ) )
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
        m_pData->m_pUndoManager->disposing();
    }

    void SAL_CALL OSingleDocumentController::disposing( const EventObject& i_event ) throw( RuntimeException )
    {
        // simply disambiguate
        OSingleDocumentController_Base::disposing( i_event );
    }

    void OSingleDocumentController::ClearUndoManager()
    {
        GetUndoManager().Clear();
    }

    SfxUndoManager& OSingleDocumentController::GetUndoManager() const
    {
        return m_pData->m_pUndoManager->GetSfxUndoManager();
    }

    void OSingleDocumentController::addUndoActionAndInvalidate(SfxUndoAction *_pAction)
    {
        // add undo action
        GetUndoManager().AddUndoAction( _pAction );

        // when we add an undo action the controller was modified
        setModified( sal_True );

        // now inform me that or states changed
        InvalidateFeature( ID_BROWSER_UNDO );
        InvalidateFeature( ID_BROWSER_REDO );
    }

    Reference< XUndoManager > SAL_CALL OSingleDocumentController::getUndoManager(  ) throw (RuntimeException)
    {
        return m_pData->m_pUndoManager.get();
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
                    String sUndo(ModuleRes(STR_UNDO_COLON));
                    sUndo += String(RTL_CONSTASCII_USTRINGPARAM(" "));
                    sUndo += GetUndoManager().GetUndoActionComment();
                    aReturn.sTitle = sUndo;
                }
                break;

            case ID_BROWSER_REDO:
                aReturn.bEnabled = isEditable() && GetUndoManager().GetRedoActionCount() != 0;
                if ( aReturn.bEnabled )
                {
                    String sRedo(ModuleRes(STR_REDO_COLON));
                    sRedo += String(RTL_CONSTASCII_USTRINGPARAM(" "));
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
