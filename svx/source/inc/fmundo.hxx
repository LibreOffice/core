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
#include <com/sun/star/script/ScriptEvent.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/ContainerEvent.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <cppuhelper/implbase.hxx>


#include <svl/lstner.hxx>
#include <comphelper/uno3.hxx>

class FmFormModel;
class FmFormObj;
class SdrObject;

class FmUndoPropertyAction: public SdrUndoAction
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xObj;
    OUString         aPropertyName;
    ::com::sun::star::uno::Any          aNewValue;
    ::com::sun::star::uno::Any          aOldValue;

public:
    FmUndoPropertyAction(FmFormModel& rMod, const ::com::sun::star::beans::PropertyChangeEvent& evt);

    virtual void Undo() override;
    virtual void Redo() override;

    virtual OUString GetComment() const override;

};

class FmUndoContainerAction: public SdrUndoAction
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >
                    m_xContainer;   // container which the action applies to
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                    m_xElement;     // object not owned by the action
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                    m_xOwnElement;  // object owned by the action
    sal_Int32       m_nIndex;       // index of the object within it's container
    ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >
                    m_aEvents;      // events of the object

public:
    enum Action
    {
        Inserted = 1,
        Removed  = 2
    };

private:
    Action              m_eAction;

public:
    FmUndoContainerAction(FmFormModel& rMod,
                          Action _eAction,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& xCont,
                          const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xElem,
                          sal_Int32 nIdx = -1);
    virtual ~FmUndoContainerAction();

    virtual void Undo() override;
    virtual void Redo() override;

    static void DisposeElement( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xElem );

protected:
    void    implReInsert( );
    void    implReRemove( );
};

class FmUndoModelReplaceAction : public SdrUndoAction
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> m_xReplaced;
    SdrUnoObj*          m_pObject;

public:
    FmUndoModelReplaceAction(FmFormModel& rMod, SdrUnoObj* pObject, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>& xReplaced);
    virtual ~FmUndoModelReplaceAction();

    virtual void Undo() override;
    virtual void Redo() override { Undo(); }

    virtual OUString GetComment() const override;

    static void DisposeElement( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>& xReplaced );
};


class SVX_DLLPRIVATE FmXUndoEnvironment
    : public ::cppu::WeakImplHelper<   ::com::sun::star::beans::XPropertyChangeListener
                                    ,   ::com::sun::star::container::XContainerListener
                                    ,   ::com::sun::star::util::XModifyListener
                                    >
    , public SfxListener
                           //   public ::cppu::OWeakObject
{
    FmFormModel& rModel;

    void*                                   m_pPropertySetCache;
    ::svxform::PFormScriptingEnvironment    m_pScriptingEnv;
    oslInterlockedCount                     m_Locks;
    ::osl::Mutex                            m_aMutex;
    bool                                    bReadOnly;
    bool                                    m_bDisposed;

public:
    FmXUndoEnvironment(FmFormModel& _rModel);
    virtual ~FmXUndoEnvironment();

    // UNO Anbindung
    //  SMART_UNO_DECLARATION(FmXUndoEnvironment, ::cppu::OWeakObject);
    //  virtual sal_Bool queryInterface(UsrUik, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>&);
    //  virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass>>    getIdlClasses();

    void Lock() { osl_atomic_increment( &m_Locks ); }
    void UnLock() { osl_atomic_decrement( &m_Locks ); }
    bool IsLocked() const { return m_Locks != 0; }

    // access control
    struct Accessor { friend class FmFormModel; private: Accessor() { } };

    // addition and removal of form collections
    void AddForms( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& rForms );
    void RemoveForms( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& rForms );

    // readonly-ness
    void SetReadOnly( bool bRead, const Accessor& ) { bReadOnly = bRead; }
    bool IsReadOnly() const {return bReadOnly;}

protected:
    // XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XContainerListener
    virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    void ModeChanged();
    void dispose();

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

private:
    void AddElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element);
    void RemoveElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element);
    void TogglePropertyListening(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element);

    void    implSetModified();

    void    switchListening( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& _rxContainer, bool _bStartListening );
    void    switchListening( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxObject, bool _bStartListening );
    ::com::sun::star::uno::Reference< com::sun::star::script::XScriptListener > m_vbaListener;
public:
    // Methoden zur Zuordnung von Controls zu Forms,
    // werden von der Seite und der UndoUmgebung genutzt
    void Inserted(SdrObject* pObj);
    void Removed(SdrObject* pObj);

    static void Inserted(FmFormObj* pObj);
    static void Removed(FmFormObj* pObj);
};


#endif // INCLUDED_SVX_SOURCE_INC_FMUNDO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
