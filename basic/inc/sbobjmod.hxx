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

#ifndef INCLUDED_BASIC_INC_SBOBJMOD_HXX
#define INCLUDED_BASIC_INC_SBOBJMOD_HXX

#include <rtl/ref.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbstar.hxx>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <basic/basicdllapi.h>

// Basic-Module for excel object.

class BASIC_DLLPUBLIC SbObjModule : public SbModule
{
protected:
    virtual ~SbObjModule();

public:
    TYPEINFO_OVERRIDE();
    SbObjModule( const OUString& rName, const com::sun::star::script::ModuleInfo& mInfo, bool bIsVbaCompatible );
    virtual SbxVariable* Find( const OUString& rName, SbxClassType t ) SAL_OVERRIDE;

    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType ) SAL_OVERRIDE;

    using SbxValue::GetObject;
    SbxVariable* GetObject();
    void SetUnoObject( const com::sun::star::uno::Any& aObj )throw ( com::sun::star::uno::RuntimeException ) ;
};

class FormObjEventListenerImpl;

class BASIC_DLLPUBLIC SbUserFormModule : public SbObjModule
{
    com::sun::star::script::ModuleInfo m_mInfo;
    ::rtl::Reference< FormObjEventListenerImpl > m_DialogListener;
    css::uno::Reference<css::awt::XDialog> m_xDialog;
    css::uno::Reference<css::frame::XModel> m_xModel;
    OUString sFormName;
    bool mbInit;

//protected:
    virtual void InitObject();
public:
    TYPEINFO_OVERRIDE();
    SbUserFormModule( const OUString& rName, const com::sun::star::script::ModuleInfo& mInfo, bool bIsVBACompat );
    virtual ~SbUserFormModule();
    virtual SbxVariable* Find( const OUString& rName, SbxClassType t ) SAL_OVERRIDE;
    void ResetApiObj( bool bTriggerTerminateEvent = true );
    void Unload();
    void Load();
    void triggerMethod( const OUString& );
    void triggerMethod( const OUString&, css::uno::Sequence< css::uno::Any >&  );
    void triggerActivateEvent();
    void triggerDeactivateEvent();
    void triggerInitializeEvent();
    void triggerTerminateEvent();
    void triggerLayoutEvent();
    void triggerResizeEvent();

    bool getInitState( void )
        { return mbInit; }
    void setInitState( bool bInit )
        { mbInit = bInit; }

    class SbUserFormModuleInstance* CreateInstance();
};

class BASIC_DLLPUBLIC SbUserFormModuleInstance : public SbUserFormModule
{
    SbUserFormModule* m_pParentModule;

public:
    SbUserFormModuleInstance( SbUserFormModule* pParentModule, const OUString& rName,
        const com::sun::star::script::ModuleInfo& mInfo, bool bIsVBACompat );

    virtual sal_Bool IsClass( const OUString& ) const SAL_OVERRIDE;
    virtual SbxVariable* Find( const OUString& rName, SbxClassType t ) SAL_OVERRIDE;
};

#endif // INCLUDED_BASIC_INC_SBOBJMOD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
