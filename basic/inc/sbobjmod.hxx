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
    SbObjModule( const OUString& rName, const css::script::ModuleInfo& mInfo, bool bIsVbaCompatible );
    virtual SbxVariable* Find( const OUString& rName, SbxClassType t ) override;

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    using SbxValue::GetObject;
    SbxVariable* GetObject();
    void SetUnoObject( const css::uno::Any& aObj )throw ( css::uno::RuntimeException, std::exception ) ;
};

class FormObjEventListenerImpl;

class BASIC_DLLPUBLIC SbUserFormModule : public SbObjModule
{
    css::script::ModuleInfo m_mInfo;
    ::rtl::Reference< FormObjEventListenerImpl > m_DialogListener;
    css::uno::Reference<css::awt::XDialog> m_xDialog;
    css::uno::Reference<css::frame::XModel> m_xModel;
    bool mbInit;

//protected:
    void InitObject();
public:
    SbUserFormModule( const OUString& rName, const css::script::ModuleInfo& mInfo, bool bIsVBACompat );
    virtual ~SbUserFormModule();
    virtual SbxVariable* Find( const OUString& rName, SbxClassType t ) override;
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

    bool getInitState()
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
        const css::script::ModuleInfo& mInfo, bool bIsVBACompat );

    virtual bool IsClass( const OUString& ) const override;
    virtual SbxVariable* Find( const OUString& rName, SbxClassType t ) override;
};

#endif // INCLUDED_BASIC_INC_SBOBJMOD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
