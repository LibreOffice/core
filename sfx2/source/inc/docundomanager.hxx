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

#include <memory>
#include <boost/noncopyable.hpp>

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

    // called when the SfxBaseModel which the component is superordinate of is being disposed
    virtual void disposing();

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

    SfxModelGuard( SfxBaseModel& i_rModel, const AllowedModelState i_eState = E_FULLY_ALIVE )
        : m_aGuard()
    {
        i_rModel.MethodEntryCheck( i_eState != E_INITIALIZING );
    }
    SfxModelGuard( SfxModelSubComponent& i_rSubComponent )
        :m_aGuard()
    {
        i_rSubComponent.MethodEntryCheck();
    }
    ~SfxModelGuard()
    {
    }

    void clear()
    {
        m_aGuard.clear();
    }

private:
    SolarMutexResettableGuard  m_aGuard;
};

namespace sfx2
{
    //= DocumentUndoManager

    typedef ::cppu::WeakImplHelper <css::document::XUndoManager> DocumentUndoManager_Base;
    struct DocumentUndoManager_Impl;
    class DocumentUndoManager   :public DocumentUndoManager_Base
                                ,public SfxModelSubComponent
                                ,public ::boost::noncopyable
    {
        friend struct DocumentUndoManager_Impl;

    public:
        DocumentUndoManager( SfxBaseModel& i_document );
        virtual ~DocumentUndoManager();

        // SfxModelSubComponent overridables
        virtual void    disposing() override;

        // non-UNO API for our owner
        /** determines whether we have an open Undo context. No mutex locking within this method, no disposal check - this
            is the responsibility of the owner.
        */
        bool    isInContext() const;

        // XInterface
        virtual void SAL_CALL acquire(  ) throw () override;
        virtual void SAL_CALL release(  ) throw () override;

        // XUndoManager
        virtual void SAL_CALL enterUndoContext( const OUString& i_title ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL enterHiddenUndoContext(  ) throw (css::document::EmptyUndoStackException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL leaveUndoContext(  ) throw (css::util::InvalidStateException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addUndoAction( const css::uno::Reference< css::document::XUndoAction >& i_action ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL undo(  ) throw (css::document::EmptyUndoStackException, css::document::UndoContextNotClosedException, css::document::UndoFailedException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL redo(  ) throw (css::document::EmptyUndoStackException, css::document::UndoContextNotClosedException, css::document::UndoFailedException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isUndoPossible(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isRedoPossible(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getCurrentUndoActionTitle(  ) throw (css::document::EmptyUndoStackException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getCurrentRedoActionTitle(  ) throw (css::document::EmptyUndoStackException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getAllUndoActionTitles(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getAllRedoActionTitles(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL clear(  ) throw (css::document::UndoContextNotClosedException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL clearRedo(  ) throw (css::document::UndoContextNotClosedException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL reset(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addUndoManagerListener( const css::uno::Reference< css::document::XUndoManagerListener >& i_listener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeUndoManagerListener( const css::uno::Reference< css::document::XUndoManagerListener >& i_listener ) throw (css::uno::RuntimeException, std::exception) override;

        // XLockable, base of XUndoManager
        virtual void SAL_CALL lock(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL unlock(  ) throw (css::util::NotLockedException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isLocked(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XChild, base of XUndoManager
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

    private:
        std::unique_ptr< DocumentUndoManager_Impl > m_pImpl;
    };


} // namespace sfx2


#endif // INCLUDED_SFX2_SOURCE_INC_DOCUNDOMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
