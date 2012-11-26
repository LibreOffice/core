/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SB_OBJMOD_HXX
#define _SB_OBJMOD_HXX

#include <rtl/ref.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbstar.hxx>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/frame/XModel.hpp>

namespace css = ::com::sun::star;

// Basic-Module for excel object.

class SbObjModule : public SbModule
{
    SbObjModule( const SbObjModule& );
    SbObjModule();

protected:
    virtual ~SbObjModule();

public:
    SbObjModule( const String& rName, const com::sun::star::script::ModuleInfo& mInfo, bool bIsVbaCompatible );
    virtual SbxVariable* Find( const XubString& rName, SbxClassType t );

    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );

    using SbxValue::GetObject;
    SbxVariable* GetObject();
    void SetUnoObject( const com::sun::star::uno::Any& aObj )throw ( com::sun::star::uno::RuntimeException ) ;
};

class FormObjEventListenerImpl;

class SbUserFormModule : public SbObjModule
{
    com::sun::star::script::ModuleInfo m_mInfo;
    ::rtl::Reference< FormObjEventListenerImpl > m_DialogListener;
    css::uno::Reference<css::awt::XDialog> m_xDialog;
    css::uno::Reference<css::frame::XModel> m_xModel;
    String sFormName;
    bool mbInit;
    SbUserFormModule( const SbUserFormModule& );
    SbUserFormModule();

//protected:
    virtual void InitObject();
public:
    SbUserFormModule( const String& rName, const com::sun::star::script::ModuleInfo& mInfo, bool bIsVBACompat );
    virtual ~SbUserFormModule();
    virtual SbxVariable* Find( const XubString& rName, SbxClassType t );
    void ResetApiObj( bool bTriggerTerminateEvent = true );
    void Unload();
    void Load();
    void triggerMethod( const String& );
    void triggerMethod( const String&, css::uno::Sequence< css::uno::Any >&  );
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

class SbUserFormModuleInstance : public SbUserFormModule
{
    SbUserFormModule* m_pParentModule;

public:
    SbUserFormModuleInstance( SbUserFormModule* pParentModule, const String& rName,
        const com::sun::star::script::ModuleInfo& mInfo, bool bIsVBACompat );

    virtual sal_Bool IsClass( const String& ) const;
    virtual SbxVariable* Find( const XubString& rName, SbxClassType t );
};


#ifndef __SB_SBOBJMODULEREF_HXX
#define __SB_SBOBJMODULEREF_HXX

SV_DECL_IMPL_REF(SbObjModule);

#endif
#endif

