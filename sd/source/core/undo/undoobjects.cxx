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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "undo/undoobjects.hxx"
#include "sdpage.hxx"
#include "CustomAnimationEffect.hxx"
#include "drawdoc.hxx"
#include "undoanim.hxx"

using namespace sd;

///////////////////////////////////////////////////////////////////////

UndoRemovePresObjectImpl::UndoRemovePresObjectImpl( SdrObject& rObject )
: mpUndoSdPage(0)
, mpUndoAnimation(0)
, mpUndoPresObj(0)
{
    SdPage* pPage = dynamic_cast< SdPage* >( rObject.getSdrPageFromSdrObject() );
    if( pPage )
    {
        if( pPage->IsPresObj(&rObject) )
            mpUndoPresObj = new UndoObjectPresentationKind( rObject );
        if( findConnectionToSdrObject(&rObject) )
            mpUndoSdPage = new UndoConnectionToSdrObject(rObject);

        if( pPage->hasAnimationNode() )
        {
            com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xShape( rObject.getUnoShape(), com::sun::star::uno::UNO_QUERY );
            if( pPage->getMainSequence()->hasEffect( xShape ) )
            {
                mpUndoAnimation = new UndoAnimation( static_cast< SdDrawDocument* >( &pPage->getSdrModelFromSdrPage() ), pPage );
            }
        }
    }
}

//---------------------------------------------------------------------

UndoRemovePresObjectImpl::~UndoRemovePresObjectImpl()
{
    delete mpUndoAnimation;
    delete mpUndoPresObj;
    delete mpUndoSdPage;
}

//---------------------------------------------------------------------

void UndoRemovePresObjectImpl::Undo()
{
    if( mpUndoSdPage )
        mpUndoSdPage->Undo();
    if( mpUndoPresObj )
        mpUndoPresObj->Undo();
    if( mpUndoAnimation )
        mpUndoAnimation->Undo();
}

//---------------------------------------------------------------------

void UndoRemovePresObjectImpl::Redo()
{
    if( mpUndoAnimation )
        mpUndoAnimation->Redo();
    if( mpUndoPresObj )
        mpUndoPresObj->Redo();
    if( mpUndoSdPage )
        mpUndoSdPage->Redo();
}

///////////////////////////////////////////////////////////////////////


UndoRemoveObject::UndoRemoveObject( SdrObject& rObject )
: SdrUndoRemoveObj( rObject ), UndoRemovePresObjectImpl( rObject )
, mxSdrObject(&rObject)
{
}

//---------------------------------------------------------------------

void UndoRemoveObject::Undo()
{
    DBG_ASSERT( mxSdrObject.is(), "sd::UndoRemoveObject::Undo(), object already dead!" );
    if( mxSdrObject.is() )
    {
        SdrUndoRemoveObj::Undo();
        UndoRemovePresObjectImpl::Undo();
    }
}

//---------------------------------------------------------------------

void UndoRemoveObject::Redo()
{
    DBG_ASSERT( mxSdrObject.is(), "sd::UndoRemoveObject::Redo(), object already dead!" );
    if( mxSdrObject.is() )
    {
        UndoRemovePresObjectImpl::Redo();
        SdrUndoRemoveObj::Redo();
    }
}

///////////////////////////////////////////////////////////////////////

UndoDeleteObject::UndoDeleteObject( SdrObject& rObject )
: SdrUndoDelObj( rObject )
, UndoRemovePresObjectImpl( rObject )
, mxSdrObject(&rObject)
{
}

//---------------------------------------------------------------------

void UndoDeleteObject::Undo()
{
    DBG_ASSERT( mxSdrObject.is(), "sd::UndoDeleteObject::Undo(), object already dead!" );
    if( mxSdrObject.is() )
    {
        SdrUndoDelObj::Undo();
        UndoRemovePresObjectImpl::Undo();
    }
}

//---------------------------------------------------------------------

void UndoDeleteObject::Redo()
{
    DBG_ASSERT( mxSdrObject.is(), "sd::UndoDeleteObject::Redo(), object already dead!" );
    if( mxSdrObject.is() )
    {
        UndoRemovePresObjectImpl::Redo();
        SdrUndoDelObj::Redo();
    }
}

///////////////////////////////////////////////////////////////////////

UndoReplaceObject::UndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject )
: SdrUndoReplaceObj( rOldObject, rNewObject )
, UndoRemovePresObjectImpl( rOldObject )
, mxSdrObject( &rOldObject )
{
}

//---------------------------------------------------------------------

void UndoReplaceObject::Undo()
{
    DBG_ASSERT( mxSdrObject.is(), "sd::UndoReplaceObject::Undo(), object already dead!" );
    if( mxSdrObject.is() )
    {
        SdrUndoReplaceObj::Undo();
        UndoRemovePresObjectImpl::Undo();
    }
}

//---------------------------------------------------------------------

void UndoReplaceObject::Redo()
{
    DBG_ASSERT( mxSdrObject.is(), "sd::UndoReplaceObject::Redo(), object already dead!" );
    if( mxSdrObject.is() )
    {
        UndoRemovePresObjectImpl::Redo();
        SdrUndoReplaceObj::Redo();
    }
}

///////////////////////////////////////////////////////////////////////

UndoObjectSetText::UndoObjectSetText( SdrObject& rObject, sal_Int32 nText )
: SdrUndoObjSetText( rObject, nText )
, mpUndoAnimation(0)
, mbNewEmptyPresObj(false)
, mxSdrObject( &rObject )
{
    SdPage* pPage = dynamic_cast< SdPage* >( rObject.getSdrPageFromSdrObject() );
    if( pPage && pPage->hasAnimationNode() )
    {
        com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xShape( rObject.getUnoShape(), com::sun::star::uno::UNO_QUERY );
        if( pPage->getMainSequence()->hasEffect( xShape ) )
        {
            mpUndoAnimation = new UndoAnimation( static_cast< SdDrawDocument* >( &pPage->getSdrModelFromSdrPage() ), pPage );
        }
    }
}

//---------------------------------------------------------------------

UndoObjectSetText::~UndoObjectSetText()
{
    delete mpUndoAnimation;
}

//---------------------------------------------------------------------

void UndoObjectSetText::Undo()
{
    DBG_ASSERT( mxSdrObject.is(), "sd::UndoObjectSetText::Undo(), object already dead!" );
    if( mxSdrObject.is() )
    {
        mbNewEmptyPresObj = mxSdrObject->IsEmptyPresObj() ? true : false;
        SdrUndoObjSetText::Undo();
        if( mpUndoAnimation )
            mpUndoAnimation->Undo();
    }
}

//---------------------------------------------------------------------

void UndoObjectSetText::Redo()
{
    DBG_ASSERT( mxSdrObject.is(), "sd::UndoObjectSetText::Redo(), object already dead!" );
    if( mxSdrObject.is() )
    {
        if( mpUndoAnimation )
            mpUndoAnimation->Redo();
        SdrUndoObjSetText::Redo();
        mxSdrObject->SetEmptyPresObj(mbNewEmptyPresObj ? true : false );
    }
}

//////////////////////////////////////////////////////////////////////////////
// Undo for connecting SdrObject to SdPage

UndoConnectionToSdrObject::UndoConnectionToSdrObject(SdrObject& rObject)
:   SdrUndoObj(rObject)
,   mpOldSdPage(findConnectionToSdrObject(&rObject))
,   mpNewSdPage(0)
,   mxSdrObject( &rObject )
{
}

//---------------------------------------------------------------------

void UndoConnectionToSdrObject::Undo()
{
    DBG_ASSERT( mxSdrObject.is(), "sd::UndoConnectionToSdrObject::Undo(), object already dead!" );
    if( mxSdrObject.is() )
    {
        mpNewSdPage = findConnectionToSdrObject(mxSdrObject.get());
        establishConnectionToSdrObject(mxSdrObject.get(), mpOldSdPage);
    }
}

//---------------------------------------------------------------------

void UndoConnectionToSdrObject::Redo()
{
    DBG_ASSERT( mxSdrObject.is(), "sd::UndoConnectionToSdrObject::Redo(), object already dead!" );
    if( mxSdrObject.is() )
    {
        establishConnectionToSdrObject(mxSdrObject.get(), mpNewSdPage);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Undo for SdPage::InsertPresObj() and SdPage::RemovePresObj()

UndoObjectPresentationKind::UndoObjectPresentationKind(SdrObject& rObject)
:   SdrUndoObj(rObject)
,   meOldKind(PRESOBJ_NONE)
,   meNewKind(PRESOBJ_NONE)
,   mxPage( rObject.getSdrPageFromSdrObject() )
,   mxSdrObject( &rObject )
{
    DBG_ASSERT( mxPage.is(), "sd::UndoObjectPresentationKind::UndoObjectPresentationKind(), does not work for shapes without a slide!" );

    if( mxPage.is() )
        meOldKind = static_cast< SdPage* >( mxPage.get() )->GetPresObjKind( &rObject );
}

//---------------------------------------------------------------------

void UndoObjectPresentationKind::Undo()
{
    if( mxPage.is() && mxSdrObject.is() )
    {
        SdPage* pPage = static_cast< SdPage* >( mxPage.get() );
        meNewKind =  pPage->GetPresObjKind( mxSdrObject.get() );
        if( meNewKind != PRESOBJ_NONE )
            pPage->RemovePresObj( mxSdrObject.get() );
        if( meOldKind != PRESOBJ_NONE )
            pPage->InsertPresObj( mxSdrObject.get(), meOldKind );
    }
}

//---------------------------------------------------------------------

void UndoObjectPresentationKind::Redo()
{
    if( mxPage.is() && mxSdrObject.is() )
    {
        SdPage* pPage = static_cast< SdPage* >( mxPage.get() );
        if( meOldKind != PRESOBJ_NONE )
            pPage->RemovePresObj( mxSdrObject.get() );
        if( meNewKind != PRESOBJ_NONE )
            pPage->InsertPresObj( mxSdrObject.get(), meNewKind );
    }
}

//////////////////////////////////////////////////////////////////////////////

UndoAutoLayoutPosAndSize::UndoAutoLayoutPosAndSize( SdPage& rPage )
: mxPage( &rPage )
{
}

//---------------------------------------------------------------------

void UndoAutoLayoutPosAndSize::Undo()
{
    // do nothing
}

//---------------------------------------------------------------------

void UndoAutoLayoutPosAndSize::Redo()
{
    SdPage* pPage = static_cast< SdPage* >( mxPage.get() );
    if( pPage )
        pPage->SetAutoLayout( pPage->GetAutoLayout(), false, false );
}

//////////////////////////////////////////////////////////////////////////////

UndoGeoObject::UndoGeoObject( SdrObject& rNewObj )
: SdrUndoGeoObj( rNewObj )
, mxPage( rNewObj.getSdrPageFromSdrObject() )
, mxSdrObject( &rNewObj )
{
}

//---------------------------------------------------------------------

void UndoGeoObject::Undo()
{
    DBG_ASSERT( mxSdrObject.is(), "sd::UndoGeoObject::Undo(), object already dead!" );
    if( mxSdrObject.is() )
    {
        if( mxPage.is() )
        {
            ScopeLockGuard aGuard( static_cast< SdPage* >( mxPage.get() )->maLockAutoLayoutArrangement );
            SdrUndoGeoObj::Undo();
        }
        else
        {
            SdrUndoGeoObj::Undo();
        }
    }
}

//---------------------------------------------------------------------

void UndoGeoObject::Redo()
{
    DBG_ASSERT( mxSdrObject.is(), "sd::UndoGeoObject::Redo(), object already dead!" );
    if( mxSdrObject.is() )
    {
        if( mxPage.is() )
        {
            ScopeLockGuard aGuard( static_cast< SdPage* >(mxPage.get())->maLockAutoLayoutArrangement );
            SdrUndoGeoObj::Redo();
        }
        else
        {
            SdrUndoGeoObj::Redo();
        }
    }
}

//---------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////

UndoAttrObject::UndoAttrObject( SdrObject& rObject, bool bStyleSheet1, bool bSaveText )
: SdrUndoAttrObj( rObject, bStyleSheet1 ? true : false, bSaveText ? true : false )
, mxPage( rObject.getSdrPageFromSdrObject() )
, mxSdrObject( &rObject )
{
}

//---------------------------------------------------------------------

void UndoAttrObject::Undo()
{
    DBG_ASSERT( mxSdrObject.is(), "sd::UndoAttrObject::Undo(), object already dead!" );
    if( mxSdrObject.is() )
    {
        if( mxPage.is() )
        {
            ScopeLockGuard aGuard( static_cast< SdPage* >( mxPage.get() )->maLockAutoLayoutArrangement );
            SdrUndoAttrObj::Undo();
        }
        else
        {
            SdrUndoAttrObj::Undo();
        }
    }
}

//---------------------------------------------------------------------

void UndoAttrObject::Redo()
{
    DBG_ASSERT( mxSdrObject.is(), "sd::UndoAttrObject::Redo(), object already dead!" );
    if( mxSdrObject.is() )
    {
        if( mxPage.is() )
        {
            ScopeLockGuard aGuard( static_cast< SdPage* >( mxPage.get() )->maLockAutoLayoutArrangement );
            SdrUndoAttrObj::Redo();
        }
        else
        {
            SdrUndoAttrObj::Redo();
        }
    }
}
