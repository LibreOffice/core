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

#ifndef INCLUDED_SVX_SOURCE_INC_FMUNDO_HXX
#define INCLUDED_SVX_SOURCE_INC_FMUNDO_HXX

#include <svx/svdundo.hxx>
#include <svx/svdouno.hxx>
#include "fmscriptingenv.hxx"


#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/ContainerEvent.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <cppuhelper/implbase.hxx>


#include <svl/lstner.hxx>

class FmFormModel;
class FmFormObj;
class SdrObject;

class FmUndoPropertyAction: public SdrUndoAction
{
    css::uno::Reference< css::beans::XPropertySet> xObj;
    OUString               aPropertyName;
    css::uno::Any          aNewValue;
    css::uno::Any          aOldValue;

public:
    FmUndoPropertyAction(FmFormModel& rMod, const css::beans::PropertyChangeEvent& evt);

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;

};

class FmUndoContainerAction final : public SdrUndoAction
{
public:
    enum Action
    {
        Inserted = 1,
        Removed  = 2
    };

    FmUndoContainerAction(FmFormModel& rMod,
                          Action _eAction,
                          const css::uno::Reference< css::container::XIndexContainer >& xCont,
                          const css::uno::Reference< css::uno::XInterface >& xElem,
                          sal_Int32 nIdx);
    virtual ~FmUndoContainerAction() override;

    virtual void Undo() override;
    virtual void Redo() override;

    static void DisposeElement( const css::uno::Reference< css::uno::XInterface >& xElem );

private:
    void    implReInsert( );
    void    implReRemove( );

    css::uno::Reference< css::container::XIndexContainer >
                    m_xContainer;   // container which the action applies to
    css::uno::Reference< css::uno::XInterface >
                    m_xElement;     // object not owned by the action
    css::uno::Reference< css::uno::XInterface >
                    m_xOwnElement;  // object owned by the action
    sal_Int32       m_nIndex;       // index of the object within its container
    css::uno::Sequence< css::script::ScriptEventDescriptor >
                    m_aEvents;      // events of the object
    Action          m_eAction;
};

class FmUndoModelReplaceAction : public SdrUndoAction
{
    css::uno::Reference< css::awt::XControlModel> m_xReplaced;
    SdrUnoObj*          m_pObject;

public:
    FmUndoModelReplaceAction(FmFormModel& rMod, SdrUnoObj* pObject, const css::uno::Reference< css::awt::XControlModel>& xReplaced);
    virtual ~FmUndoModelReplaceAction() override;

    virtual void Undo() override;
    virtual void Redo() override { Undo(); }

    virtual OUString GetComment() const override;

    static void DisposeElement( const css::uno::Reference< css::awt::XControlModel>& xReplaced );
};


class FmXUndoEnvironment final
    : public ::cppu::WeakImplHelper<   css::beans::XPropertyChangeListener
                                    ,   css::container::XContainerListener
                                    ,   css::util::XModifyListener
                                    >
    , public SfxListener
{
public:
    FmXUndoEnvironment(FmFormModel& _rModel);
    virtual ~FmXUndoEnvironment() override;

    // UNO binding
    //  SMART_UNO_DECLARATION(FmXUndoEnvironment, ::cppu::OWeakObject);
    //  virtual sal_Bool queryInterface(UsrUik, css::uno::Reference< css::uno::XInterface>&);
    //  virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlClass>>    getIdlClasses();

    void Lock() { osl_atomic_increment( &m_Locks ); }
    void UnLock() { osl_atomic_decrement( &m_Locks ); }
    bool IsLocked() const { return m_Locks != 0; }

    // access control
    struct Accessor { friend class FmFormModel; private: Accessor() { } };

    // addition and removal of form collections
    void AddForms( const css::uno::Reference< css::container::XNameContainer>& rForms );
    void RemoveForms( const css::uno::Reference< css::container::XNameContainer>& rForms );

    // readonly-ness
    void SetReadOnly( bool bRead, const Accessor& ) { bReadOnly = bRead; }
    bool IsReadOnly() const {return bReadOnly;}

    // Methods for assigning controls to forms,
    // used by the page and the undo environment
    void Inserted(SdrObject* pObj);
    void Removed(SdrObject* pObj);

    static void Inserted(FmFormObj* pObj);
    static void Removed(FmFormObj* pObj);

private:
    // XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& evt) override;

    // XContainerListener
    virtual void SAL_CALL elementInserted(const css::container::ContainerEvent& rEvent) override;
    virtual void SAL_CALL elementReplaced(const css::container::ContainerEvent& rEvent) override;
    virtual void SAL_CALL elementRemoved(const css::container::ContainerEvent& rEvent) override;

    // XModifyListener
    virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) override;

    void ModeChanged();
    void dispose();

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    void AddElement(const css::uno::Reference< css::uno::XInterface>& Element);
    void RemoveElement(const css::uno::Reference< css::uno::XInterface>& Element);
    void TogglePropertyListening(const css::uno::Reference< css::uno::XInterface>& Element);

    void    implSetModified();

    void    switchListening( const css::uno::Reference< css::container::XIndexContainer >& _rxContainer, bool _bStartListening );
    void    switchListening( const css::uno::Reference< css::uno::XInterface >& _rxObject, bool _bStartListening );

    FmFormModel&                            rModel;
    void*                                   m_pPropertySetCache;
    ::rtl::Reference<svxform::FormScriptingEnvironment> m_pScriptingEnv;
    oslInterlockedCount                     m_Locks;
    ::osl::Mutex                            m_aMutex;
    bool                                    bReadOnly;
    bool                                    m_bDisposed;
    css::uno::Reference< css::script::XScriptListener > m_vbaListener;
};


#endif // INCLUDED_SVX_SOURCE_INC_FMUNDO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
