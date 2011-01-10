/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbobjmod.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author:  $ $Date: 2007/08/27 16:31:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
    TYPEINFO();
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
    TYPEINFO();
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

