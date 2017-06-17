/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DESKTOP_SOURCE_LIB_LOKUNDOMANAGERLISTENER_HXX
#define INCLUDED_DESKTOP_SOURCE_LIB_LOKUNDOMANAGERLISTENER_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/document/XUndoManager.hpp>

namespace desktop {
    struct LibLibreOffice_Impl;
}


class LOKUndoManagerListener : public ::cppu::WeakImplHelper <::css::document::XUndoManagerListener>
{
    private:
        ::css::uno::Reference<::css::document::XUndoManager> m_xUndoManager;
        desktop::LibLibreOffice_Impl * m_pLOKit;
        bool m_bDocRepairState;

        void broadcastViews();

    public:
        LOKUndoManagerListener(
            const ::css::uno::Reference<::css::document::XUndoManager>&,
            desktop::LibLibreOffice_Impl*);

        virtual ~LOKUndoManagerListener() override;

        virtual void SAL_CALL undoActionAdded( const ::css::document::UndoManagerEvent& iEvent ) override;

        virtual void SAL_CALL actionUndone( const ::css::document::UndoManagerEvent& iEvent ) override;

        virtual void SAL_CALL actionRedone( const ::css::document::UndoManagerEvent& iEvent ) override;

        virtual void SAL_CALL allActionsCleared( const ::css::lang::EventObject& iEvent ) override;

        virtual void SAL_CALL redoActionsCleared( const ::css::lang::EventObject& iEvent ) override;

        virtual void SAL_CALL resetAll( const ::css::lang::EventObject& iEvent ) override;

        virtual void SAL_CALL enteredContext( const ::css::document::UndoManagerEvent& iEvent ) override;

        virtual void SAL_CALL enteredHiddenContext( const ::css::document::UndoManagerEvent& iEvent ) override;

        virtual void SAL_CALL leftContext( const ::css::document::UndoManagerEvent& iEvent ) override;

        virtual void SAL_CALL leftHiddenContext( const ::css::document::UndoManagerEvent& iEvent ) override;

        virtual void SAL_CALL cancelledContext( const ::css::document::UndoManagerEvent& iEvent ) override;

        // XEventListener
        virtual void SAL_CALL disposing(const ::css::lang::EventObject& rSource) override;
};

#endif // INCLUDED_DESKTOP_SOURCE_LIB_LOKUNDOMANAGERLISTENER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
