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

#ifndef INCLUDED_SFX2_SOURCE_INC_DOCUNDOMANAGER_HXX
#define INCLUDED_SFX2_SOURCE_INC_DOCUNDOMANAGER_HXX

#include <sfx2/sfxbasemodel.hxx>

#include <com/sun/star/document/XUndoManager.hpp>

#include <cppuhelper/implbase.hxx>
#include <vcl/svapp.hxx>

#include <memory>

/** base class for sub components of an SfxBaseModel, which share their ref count and lifetime with the SfxBaseModel
*/
class SfxModelSubComponent
{
public:
    /** checks whether the instance is alive, i.e. properly initialized, and not yet disposed
    */
    void    MethodEntryCheck()
    {
        m_rModel.MethodEntryCheck( true );
    }

protected:
    SfxModelSubComponent( SfxBaseModel& i_model )
        :m_rModel( i_model )
    {
    }
    virtual ~SfxModelSubComponent();

    void acquireModel()  { m_rModel.acquire(); }
    void releaseModel()  { m_rModel.release(); }

protected:
    const SfxBaseModel& getBaseModel() const { return m_rModel; }
          SfxBaseModel& getBaseModel()       { return m_rModel; }

private:
    SfxBaseModel&   m_rModel;
};

class SfxModelGuard
{
public:
    enum AllowedModelState
    {
        // not yet initialized
        E_INITIALIZING,
        // fully alive, i.e. initialized, and not yet disposed
        E_FULLY_ALIVE
    };

    SfxModelGuard( SfxBaseModel const & i_rModel, const AllowedModelState i_eState = E_FULLY_ALIVE )
        : m_aGuard()
    {
        i_rModel.MethodEntryCheck( i_eState != E_INITIALIZING );
    }
    SfxModelGuard( SfxModelSubComponent& i_rSubComponent )
        :m_aGuard()
    {
        i_rSubComponent.MethodEntryCheck();
    }

    void clear()
    {
        m_aGuard.clear();
    }

private:
    SolarMutexClearableGuard  m_aGuard;
};

namespace sfx2
{
    //= DocumentUndoManager

    struct DocumentUndoManager_Impl;
    class DocumentUndoManager   :public ::cppu::WeakImplHelper<css::document::XUndoManager>
                                ,public SfxModelSubComponent
    {
        friend struct DocumentUndoManager_Impl;

    public:
        DocumentUndoManager( SfxBaseModel& i_document );
        virtual ~DocumentUndoManager() override;
        DocumentUndoManager(const DocumentUndoManager&) = delete;
        DocumentUndoManager& operator=(const DocumentUndoManager&) = delete;

        void    disposing();

        // non-UNO API for our owner
        /** determines whether we have an open Undo context. No mutex locking within this method, no disposal check - this
            is the responsibility of the owner.
        */
        bool    isInContext() const;

        // XInterface
        virtual void SAL_CALL acquire(  ) noexcept override;
        virtual void SAL_CALL release(  ) noexcept override;

        // XUndoManager
        virtual void SAL_CALL enterUndoContext( const OUString& i_title ) override;
        virtual void SAL_CALL enterHiddenUndoContext(  ) override;
        virtual void SAL_CALL leaveUndoContext(  ) override;
        virtual void SAL_CALL addUndoAction( const css::uno::Reference< css::document::XUndoAction >& i_action ) override;
        virtual void SAL_CALL undo(  ) override;
        virtual void SAL_CALL redo(  ) override;
        virtual sal_Bool SAL_CALL isUndoPossible(  ) override;
        virtual sal_Bool SAL_CALL isRedoPossible(  ) override;
        virtual OUString SAL_CALL getCurrentUndoActionTitle(  ) override;
        virtual OUString SAL_CALL getCurrentRedoActionTitle(  ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getAllUndoActionTitles(  ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getAllRedoActionTitles(  ) override;
        virtual void SAL_CALL clear(  ) override;
        virtual void SAL_CALL clearRedo(  ) override;
        virtual void SAL_CALL reset(  ) override;
        virtual void SAL_CALL addUndoManagerListener( const css::uno::Reference< css::document::XUndoManagerListener >& i_listener ) override;
        virtual void SAL_CALL removeUndoManagerListener( const css::uno::Reference< css::document::XUndoManagerListener >& i_listener ) override;

        // XLockable, base of XUndoManager
        virtual void SAL_CALL lock(  ) override;
        virtual void SAL_CALL unlock(  ) override;
        virtual sal_Bool SAL_CALL isLocked(  ) override;

        // XChild, base of XUndoManager
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) override;
        virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;

    private:
        std::unique_ptr< DocumentUndoManager_Impl > m_pImpl;
    };


} // namespace sfx2


#endif // INCLUDED_SFX2_SOURCE_INC_DOCUNDOMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
