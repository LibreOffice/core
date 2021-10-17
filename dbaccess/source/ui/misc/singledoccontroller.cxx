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
#include <core_resource.hxx>
#include <singledoccontroller.hxx>
#include <browserids.hxx>
#include <strings.hrc>

#include <svl/undo.hxx>

namespace dbaui
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::document::XUndoManager;
    using ::com::sun::star::beans::PropertyValue;

    // OSingleDocumentController_Data
    struct OSingleDocumentController_Data
    {
        // no Reference! see UndoManager::acquire
        std::unique_ptr<UndoManager> m_pUndoManager;

        OSingleDocumentController_Data( ::cppu::OWeakObject& i_parent, ::osl::Mutex& i_mutex )
            : m_pUndoManager(new UndoManager(i_parent, i_mutex))
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

    void OSingleDocumentController::ClearUndoManager()
    {
        GetUndoManager().Clear();
    }

    SfxUndoManager& OSingleDocumentController::GetUndoManager() const
    {
        return m_pData->m_pUndoManager->GetSfxUndoManager();
    }

    void OSingleDocumentController::addUndoActionAndInvalidate(std::unique_ptr<SfxUndoAction> _pAction)
    {
        // add undo action
        GetUndoManager().AddUndoAction( std::move(_pAction) );

        // when we add an undo action the controller was modified
        setModified( true );

        // now inform me that or states changed
        InvalidateFeature( ID_BROWSER_UNDO );
        InvalidateFeature( ID_BROWSER_REDO );
    }

    Reference< XUndoManager > SAL_CALL OSingleDocumentController::getUndoManager(  )
    {
        // see UndoManager::acquire
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
                    OUString sUndo = DBA_RES(STR_UNDO_COLON) + " " +
                        GetUndoManager().GetUndoActionComment();
                    aReturn.sTitle = sUndo;
                }
                break;

            case ID_BROWSER_REDO:
                aReturn.bEnabled = isEditable() && GetUndoManager().GetRedoActionCount() != 0;
                if ( aReturn.bEnabled )
                {
                    OUString sRedo = DBA_RES(STR_REDO_COLON) + " " +
                        GetUndoManager().GetRedoActionComment();
                    aReturn.sTitle = sRedo;
                }
                break;

            case SID_GETUNDOSTRINGS:
            {
                size_t nCount(GetUndoManager().GetUndoActionCount());
                Sequence<OUString> aSeq(nCount);
                auto aSeqRange = asNonConstRange(aSeq);
                for (size_t n = 0; n < nCount; ++n)
                    aSeqRange[n] = GetUndoManager().GetUndoActionComment(n);
                aReturn.aValue <<= aSeq;
                aReturn.bEnabled = true;
                break;
            }

            case SID_GETREDOSTRINGS:
            {
                size_t nCount(GetUndoManager().GetRedoActionCount());
                Sequence<OUString> aSeq(nCount);
                auto aSeqRange = asNonConstRange(aSeq);
                for (size_t n = 0; n < nCount; ++n)
                    aSeqRange[n] = GetUndoManager().GetRedoActionComment(n);
                aReturn.aValue <<= aSeq;
                aReturn.bEnabled = true;
                break;
            }

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
            case ID_BROWSER_REDO:
            {
                sal_Int16 nCount(1);
                if (_rArgs.hasElements() && _rArgs[0].Name != "KeyModifier")
                    _rArgs[0].Value >>= nCount;

                while (nCount--)
                {
                    if (_nId == ID_BROWSER_UNDO)
                        GetUndoManager().Undo();
                    else
                        GetUndoManager().Redo();
                }

                InvalidateFeature( ID_BROWSER_UNDO );
                InvalidateFeature( ID_BROWSER_REDO );
                break;
            }

            default:
                OSingleDocumentController_Base::Execute( _nId, _rArgs );
                break;
        }
        InvalidateFeature(_nId);
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
