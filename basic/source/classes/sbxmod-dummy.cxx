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


#include <list>

#include <vcl/svapp.hxx>
#include <tools/stream.hxx>
#include <svl/brdcst.hxx>
#include <tools/shl.hxx>
#include <basic/sbx.hxx>
#include "sbdiagnose.hxx"
#include "sb.hxx"
#include <sbjsmeth.hxx>
#include "sbjsmod.hxx"
#include "sbintern.hxx"
#include "image.hxx"
#include "opcodes.hxx"
#include "runtime.hxx"
#include "token.hxx"
#include "sbunoobj.hxx"

#include <svtools/syntaxhighlight.hxx>

#include <basic/basrdll.hxx>
#include <osl/mutex.hxx>
#include <basic/sbobjmod.hxx>
#include <basic/vbahelper.hxx>
#include <cppuhelper/implbase3.hxx>
#include <unotools/eventcfg.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/document/XVbaMethodParameter.hpp>
#include <com/sun/star/script/vba/VBAScriptEventId.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/document/XEventListener.hpp>

using namespace com::sun::star;

#include <stdio.h>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <map>
#include <com/sun/star/reflection/XProxyFactory.hpp>
#include <cppuhelper/implbase1.hxx>
#include <basic/sbobjmod.hxx>
#include <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/script/XInvocation.hpp>

using namespace ::com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::reflection;
using namespace com::sun::star::beans;
using namespace com::sun::star::script;


#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/awt/XDialogProvider.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/anytostring.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/VbQueryClose.hpp>

typedef ::cppu::WeakImplHelper1< XInvocation > DocObjectWrapper_BASE;
typedef ::std::map< sal_Int16, Any, ::std::less< sal_Int16 > > OutParamMap;
::com::sun::star::uno::Any sbxToUnoValue( SbxVariable* pVar );
void unoToSbxValue( SbxVariable* pVar, const ::com::sun::star::uno::Any& aValue );

// TYPEINIT1(SbModule,SbxObject)
// TYPEINIT1(SbMethod,SbxMethod)

SbModule::SbModule( const String& /* rName */,  sal_Bool /* bVBACompat */ )
         : SbxObject( String( ) )
{
}

SbModule::~SbModule()
{
}

uno::Reference< script::XInvocation >
SbModule::GetUnoModule()
{
    static uno::Reference< script::XInvocation > dummy;

    return dummy;
}

sal_Bool SbModule::IsCompiled() const
{
    return sal_False;
}

const SbxObject* SbModule::FindType( String /* aTypeName */ ) const
{
    return 0;
}

void SbModule::Clear()
{
}

SbxVariable* SbModule::Find( const XubString& /* rName */, SbxClassType /* t */ )
{
    return 0;
}

const ::rtl::OUString& SbModule::GetSource32() const
{
    static ::rtl::OUString dummy;

    return dummy;
}

const String& SbModule::GetSource() const
{
    static String dummy;

    return dummy;
}

void SbModule::SetParent( SbxObject* /* p */ )
{
}

void SbModule::SFX_NOTIFY( SfxBroadcaster& /* rBC */, const TypeId& /* rBCType */,
                           const SfxHint& /* rHint */, const TypeId& /* rHintType */ )
{
}

void SbModule::SetSource( const String& /* r */ )
{
}

SbMethod* SbModule::GetFunctionForLine( sal_uInt16 /* nLine */ )
{
    return 0;
}

sal_Bool SbModule::IsVBACompat() const
{
    return sal_False;
}

void SbModule::SetVBACompat( sal_Bool /* bCompat */ )
{
}

sal_Bool SbModule::IsBreakable( sal_uInt16 /* nLine */ ) const
{
    return sal_False;
}

size_t SbModule::GetBPCount() const
{
    return 0;
}

sal_uInt16 SbModule::GetBP( size_t /* n */ ) const
{
    return 0;
}

sal_Bool SbModule::IsBP( sal_uInt16 /* nLine */ ) const
{
    return sal_False;
}

sal_Bool SbModule::SetBP( sal_uInt16 /* nLine */ )
{
    return sal_False;
}

sal_Bool SbModule::ClearBP( sal_uInt16 /* nLine */ )
{
    return sal_False;
}

void SbModule::ClearAllBP()
{
}

sal_Bool SbModule::LoadData( SvStream& /* rStrm */, sal_uInt16 /* nVer */ )
{
    return sal_False;
}

sal_Bool SbModule::StoreData( SvStream& /* rStrm */ ) const
{
    return sal_False;
}

sal_Bool SbModule::LoadCompleted()
{
    return sal_False;
}

sal_Bool SbModule::Compile()
{
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
