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

#include <tools/debug.hxx>
#include <undo/undoobjects.hxx>
#include <sdpage.hxx>
#include <CustomAnimationEffect.hxx>
#include <drawdoc.hxx>
#include <undoanim.hxx>
#include <ViewShell.hxx>
#include <ViewShellBase.hxx>
#include <DrawDocShell.hxx>

using namespace sd;

SdUndoAction::SdUndoAction(SdDrawDocument* pSdDrawDocument)
    : mpDoc(pSdDrawDocument),
      mnViewShellId(-1)
{
    sd::DrawDocShell* pDocShell = pSdDrawDocument ? pSdDrawDocument->GetDocSh() : nullptr;
    sd::ViewShell* pViewShell = pDocShell ? pDocShell->GetViewShell() : nullptr;
    if (pViewShell)
        mnViewShellId = pViewShell->GetViewShellBase().GetViewShellId();
}

ViewShellId SdUndoAction::GetViewShellId() const
{
    return mnViewShellId;
}

UndoRemovePresObjectImpl::UndoRemovePresObjectImpl( SdrObject& rObject )
{
    SdPage* pPage = dynamic_cast< SdPage* >( rObject.getSdrPageFromSdrObject() );
    if( !pPage )
        return;

    if( pPage->IsPresObj(&rObject) )
        mpUndoPresObj.reset( new UndoObjectPresentationKind( rObject ) );
    if( rObject.GetUserCall() )
        mpUndoUsercall.reset( new UndoObjectUserCall(rObject) );

    if( pPage->hasAnimationNode() )
    {
        css::uno::Reference< css::drawing::XShape > xShape( rObject.getUnoShape(), css::uno::UNO_QUERY );
        if( pPage->getMainSequence()->hasEffect( xShape ) )
        {
            mpUndoAnimation.reset(
                new UndoAnimation( // TTTT may use ref? Or just *SdrPage?
                    static_cast< SdDrawDocument* >(&pPage->getSdrModelFromSdrPage()),
                    pPage));
        }
    }
}

UndoRemovePresObjectImpl::~UndoRemovePresObjectImpl()
{
}

void UndoRemovePresObjectImpl::Undo()
{
    if( mpUndoUsercall )
        mpUndoUsercall->Undo();
    if( mpUndoPresObj )
        mpUndoPresObj->Undo();
    if( mpUndoAnimation )
        mpUndoAnimation->Undo();
}

void UndoRemovePresObjectImpl::Redo()
{
    if( mpUndoAnimation )
        mpUndoAnimation->Redo();
    if( mpUndoPresObj )
        mpUndoPresObj->Redo();
    if( mpUndoUsercall )
        mpUndoUsercall->Redo();
}

UndoRemoveObject::UndoRemoveObject( SdrObject& rObject )
: SdrUndoRemoveObj( rObject ), UndoRemovePresObjectImpl( rObject )
, mxSdrObject(&rObject)
{
}

void UndoRemoveObject::Undo()
{
    DBG_ASSERT( mxSdrObject.get().is(), "sd::UndoRemoveObject::Undo(), object already dead!" );
    if( mxSdrObject.get().is() )
    {
        SdrUndoRemoveObj::Undo();
        UndoRemovePresObjectImpl::Undo();
    }
}

void UndoRemoveObject::Redo()
{
    DBG_ASSERT( mxSdrObject.get().is(), "sd::UndoRemoveObject::Redo(), object already dead!" );
    if( mxSdrObject.get().is() )
    {
        UndoRemovePresObjectImpl::Redo();
        SdrUndoRemoveObj::Redo();
    }
}

UndoDeleteObject::UndoDeleteObject( SdrObject& rObject, bool bOrdNumDirect )
: SdrUndoDelObj( rObject, bOrdNumDirect )
, UndoRemovePresObjectImpl( rObject )
, mxSdrObject(&rObject)
{
}

void UndoDeleteObject::Undo()
{
    DBG_ASSERT( mxSdrObject.get().is(), "sd::UndoDeleteObject::Undo(), object already dead!" );
    if( mxSdrObject.get().is() )
    {
        SdrUndoDelObj::Undo();
        UndoRemovePresObjectImpl::Undo();
    }
}

void UndoDeleteObject::Redo()
{
    DBG_ASSERT( mxSdrObject.get().is(), "sd::UndoDeleteObject::Redo(), object already dead!" );
    if( mxSdrObject.get().is() )
    {
        UndoRemovePresObjectImpl::Redo();
        SdrUndoDelObj::Redo();
    }
}

UndoReplaceObject::UndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject )
: SdrUndoReplaceObj( rOldObject, rNewObject )
, UndoRemovePresObjectImpl( rOldObject )
, mxSdrObject( &rOldObject )
{
}

void UndoReplaceObject::Undo()
{
    DBG_ASSERT( mxSdrObject.get().is(), "sd::UndoReplaceObject::Undo(), object already dead!" );
    if( mxSdrObject.get().is() )
    {
        SdrUndoReplaceObj::Undo();
        UndoRemovePresObjectImpl::Undo();
    }
}

void UndoReplaceObject::Redo()
{
    DBG_ASSERT( mxSdrObject.get().is(), "sd::UndoReplaceObject::Redo(), object already dead!" );
    if( mxSdrObject.get().is() )
    {
        UndoRemovePresObjectImpl::Redo();
        SdrUndoReplaceObj::Redo();
    }
}

UndoObjectSetText::UndoObjectSetText( SdrObject& rObject, sal_Int32 nText )
: SdrUndoObjSetText( rObject, nText )
, mbNewEmptyPresObj(false)
, mxSdrObject( &rObject )
{
    SdPage* pPage = dynamic_cast< SdPage* >( rObject.getSdrPageFromSdrObject() );
    if( pPage && pPage->hasAnimationNode() )
    {
        css::uno::Reference< css::drawing::XShape > xShape( rObject.getUnoShape(), css::uno::UNO_QUERY );
        if( pPage->getMainSequence()->hasEffect( xShape ) )
        {
            mpUndoAnimation.reset(
                new UndoAnimation(
                    static_cast< SdDrawDocument* >(&pPage->getSdrModelFromSdrPage()),
                    pPage));
        }
    }
}

UndoObjectSetText::~UndoObjectSetText()
{
}

void UndoObjectSetText::Undo()
{
    DBG_ASSERT( mxSdrObject.get().is(), "sd::UndoObjectSetText::Undo(), object already dead!" );
    if( auto pSdrObject = mxSdrObject.get() )
    {
        mbNewEmptyPresObj = pSdrObject->IsEmptyPresObj();
        SdrUndoObjSetText::Undo();
        if( mpUndoAnimation )
            mpUndoAnimation->Undo();
    }
}

void UndoObjectSetText::Redo()
{
    DBG_ASSERT( mxSdrObject.get().is(), "sd::UndoObjectSetText::Redo(), object already dead!" );
    if( auto pSdrObject = mxSdrObject.get() )
    {
        if( mpUndoAnimation )
            mpUndoAnimation->Redo();
        SdrUndoObjSetText::Redo();
        pSdrObject->SetEmptyPresObj(mbNewEmptyPresObj);
    }
}

// Undo for SdrObject::SetUserCall()

UndoObjectUserCall::UndoObjectUserCall(SdrObject& rObject)
:   SdrUndoObj(rObject)
,   mpOldUserCall(static_cast<SdPage*>(rObject.GetUserCall()))
,   mpNewUserCall(nullptr)
,   mxSdrObject( &rObject )
{
}

void UndoObjectUserCall::Undo()
{
    DBG_ASSERT( mxSdrObject.get().is(), "sd::UndoObjectUserCall::Undo(), object already dead!" );
    if( auto pSdrObject = mxSdrObject.get() )
    {
        mpNewUserCall = pSdrObject->GetUserCall();
        pSdrObject->SetUserCall(mpOldUserCall);
    }
}

void UndoObjectUserCall::Redo()
{
    DBG_ASSERT( mxSdrObject.get().is(), "sd::UndoObjectUserCall::Redo(), object already dead!" );
    if( auto pSdrObject = mxSdrObject.get() )
    {
        pSdrObject->SetUserCall(mpNewUserCall);
    }
}

// Undo for SdPage::InsertPresObj() and SdPage::RemovePresObj()

UndoObjectPresentationKind::UndoObjectPresentationKind(SdrObject& rObject)
:   SdrUndoObj(rObject)
,   meOldKind(PresObjKind::NONE)
,   meNewKind(PresObjKind::NONE)
,   mxPage( static_cast<SdPage*>(rObject.getSdrPageFromSdrObject()) )
,   mxSdrObject( &rObject )
{
    DBG_ASSERT( mxPage.get(), "sd::UndoObjectPresentationKind::UndoObjectPresentationKind(), does not work for shapes without a slide!" );

    if( auto pPage = mxPage.get() )
        meOldKind = pPage->GetPresObjKind( &rObject );
}

void UndoObjectPresentationKind::Undo()
{
    if( auto pSdrObject = mxSdrObject.get() )
    {
        if( rtl::Reference<SdPage> pPage = mxPage.get() )
        {
            meNewKind =  pPage->GetPresObjKind( pSdrObject.get() );
            if( meNewKind != PresObjKind::NONE )
                pPage->RemovePresObj( pSdrObject.get() );
            if( meOldKind != PresObjKind::NONE )
                pPage->InsertPresObj( pSdrObject.get(), meOldKind );
        }
    }
}

void UndoObjectPresentationKind::Redo()
{
    if( auto pSdrObject = mxSdrObject.get() )
    {
        if( rtl::Reference<SdPage> pPage = mxPage.get() )
        {
            if( meOldKind != PresObjKind::NONE )
                pPage->RemovePresObj( pSdrObject.get() );
            if( meNewKind != PresObjKind::NONE )
                pPage->InsertPresObj( pSdrObject.get(), meNewKind );
        }
    }
}

UndoAutoLayoutPosAndSize::UndoAutoLayoutPosAndSize( SdPage& rPage )
: mxPage( &rPage )
{
}

void UndoAutoLayoutPosAndSize::Undo()
{
    // do nothing
}

void UndoAutoLayoutPosAndSize::Redo()
{
    rtl::Reference<SdPage> pPage = mxPage.get();
    if( pPage )
        pPage->SetAutoLayout( pPage->GetAutoLayout() );
}

UndoGeoObject::UndoGeoObject( SdrObject& rNewObj )
: SdrUndoGeoObj( rNewObj )
, mxPage( static_cast<SdPage*>(rNewObj.getSdrPageFromSdrObject()) )
, mxSdrObject( &rNewObj )
{
}

void UndoGeoObject::Undo()
{
    DBG_ASSERT( mxSdrObject.get().is(), "sd::UndoGeoObject::Undo(), object already dead!" );
    if( auto pSdrObject = mxSdrObject.get() )
    {
        if( auto pPage = mxPage.get() )
        {
            ScopeLockGuard aGuard( pPage->maLockAutoLayoutArrangement );
            SdrUndoGeoObj::Undo();
        }
        else
        {
            SdrUndoGeoObj::Undo();
        }
    }
}

void UndoGeoObject::Redo()
{
    DBG_ASSERT( mxSdrObject.get().is(), "sd::UndoGeoObject::Redo(), object already dead!" );
    if( auto pSdrObject = mxSdrObject.get() )
    {
        if( auto pPage = mxPage.get() )
        {
            ScopeLockGuard aGuard( pPage->maLockAutoLayoutArrangement );
            SdrUndoGeoObj::Redo();
        }
        else
        {
            SdrUndoGeoObj::Redo();
        }
    }
}

UndoAttrObject::UndoAttrObject( SdrObject& rObject, bool bStyleSheet1, bool bSaveText )
: SdrUndoAttrObj( rObject, bStyleSheet1, bSaveText )
, mxPage( static_cast<SdPage*>(rObject.getSdrPageFromSdrObject()) )
, mxSdrObject( &rObject )
{
}

void UndoAttrObject::Undo()
{
    DBG_ASSERT( mxSdrObject.get().is(), "sd::UndoAttrObject::Undo(), object already dead!" );
    if( auto pSdrObject = mxSdrObject.get() )
    {
        if( auto pPage = mxPage.get() )
        {
            ScopeLockGuard aGuard( pPage->maLockAutoLayoutArrangement );
            SdrUndoAttrObj::Undo();
        }
        else
        {
            SdrUndoAttrObj::Undo();
        }
    }
}

void UndoAttrObject::Redo()
{
    DBG_ASSERT( mxSdrObject.get().is(), "sd::UndoAttrObject::Redo(), object already dead!" );
    if( auto pSdrObject = mxSdrObject.get() )
    {
        if( auto pPage = mxPage.get() )
        {
            ScopeLockGuard aGuard( pPage->maLockAutoLayoutArrangement );
            SdrUndoAttrObj::Redo();
        }
        else
        {
            SdrUndoAttrObj::Redo();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
