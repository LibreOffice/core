/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_FMUNDO_HXX
#define _SVX_FMUNDO_HXX


#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/script/XScriptListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

#include <cppuhelper/implbase3.hxx>




#include "svdouno.hxx"

#include <comphelper/uno3.hxx>
FORWARD_DECLARE_INTERFACE(awt,XControl)
FORWARD_DECLARE_INTERFACE(awt,XControlContainer)
namespace binfilter {

class FmFormModel;
class FmFormObj;
class SdrObject;
class FmXFormView;

//STRIP008 FORWARD_DECLARE_INTERFACE(awt,XControl)
//STRIP008 FORWARD_DECLARE_INTERFACE(awt,XControlContainer)
//FORWARD_DECLARE_INTERFACE(uno,Reference)

//==================================================================
// FmUndoPropertyAction
//==================================================================

//==================================================================
// FmUndoContainerAction
//==================================================================

//==================================================================
// FmUndoModelReplaceAction
//==================================================================

//========================================================================
class FmXUndoEnvironment
    : public ::cppu::WeakImplHelper3<	::com::sun::star::beans::XPropertyChangeListener
                                    ,	::com::sun::star::container::XContainerListener
                                    ,	::com::sun::star::script::XScriptListener
                                    >
    , public SfxListener
                           //	public ::cppu::OWeakObject
{
    friend class FmFormModel;
    friend class FmXFormView;
    FmFormModel& rModel;

    void*		m_pPropertySetCache;
    sal_uInt32	nLocks;
    sal_Bool	bReadOnly;



public:
    FmXUndoEnvironment(FmFormModel& _rModel);
    ~FmXUndoEnvironment();

    // UNO Anbindung
    //	SMART_UNO_DECLARATION(FmXUndoEnvironment, ::cppu::OWeakObject);
    //	virtual sal_Bool queryInterface(UsrUik, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>&);
    //	virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass>>	getIdlClasses(void);

    void Lock() {nLocks++;}
    void UnLock() {nLocks--;}
    sal_Bool IsLocked() const {return nLocks != 0;}

protected:
// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

// ::com::sun::star::beans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XContainerListener
    virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException);

// XScriptListener
    virtual void SAL_CALL firing(const  ::com::sun::star::script::ScriptEvent& evt) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL approveFiring(const  ::com::sun::star::script::ScriptEvent& evt) throw(::com::sun::star::reflection::InvocationTargetException, ::com::sun::star::uno::RuntimeException);

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

// Einfuegen von Objekten
    void AddForms(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& rForms);
    void RemoveForms(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& rForms);

    void SetReadOnly(sal_Bool bRead) {bReadOnly = bRead;}
    sal_Bool IsReadOnly() const {return bReadOnly;}

    void ModeChanged();

private:
    void AddElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element);
    void RemoveElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element);
    void AlterPropertyListening(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element);

public:
    // Methoden zur Zuordnung von Controls zu Forms,
    // werden von der Seite und der UndoUmgebung genutzt
    void Inserted(SdrObject* pObj);
    void Removed(SdrObject* pObj);

    void Inserted(FmFormObj* pObj);
    void Removed(FmFormObj* pObj);
};


}//end of namespace binfilter
#endif	//_SVX_FMUNDO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
