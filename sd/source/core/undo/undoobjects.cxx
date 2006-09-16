/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: undoobjects.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 18:18:29 $
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
: mpUndoUsercall(0), mpUndoPresObj(0), mpUndoAnimation(0)
{
    SdPage* pPage = dynamic_cast< SdPage* >( rObject.GetPage() );
    if( pPage )
    {
        if( pPage->IsPresObj(&rObject) )
            mpUndoPresObj = new UndoObjectPresentationKind( rObject );
        if( rObject.GetUserCall() )
            mpUndoUsercall = new UndoObjectUserCall(rObject);

        if( pPage->hasAnimationNode() )
        {
            com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xShape( rObject.getUnoShape(), com::sun::star::uno::UNO_QUERY );
            if( pPage->getMainSequence()->hasEffect( xShape ) )
            {
                mpUndoAnimation = new UndoAnimation( static_cast< SdDrawDocument* >( pPage->GetModel() ), pPage );
            }
        }
    }
}

//---------------------------------------------------------------------

UndoRemovePresObjectImpl::~UndoRemovePresObjectImpl()
{
    delete mpUndoAnimation;
    delete mpUndoPresObj;
    delete mpUndoUsercall;
}

//---------------------------------------------------------------------

void UndoRemovePresObjectImpl::Undo()
{
    if( mpUndoUsercall )
        mpUndoUsercall->Undo();
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
    if( mpUndoUsercall )
        mpUndoUsercall->Redo();
}

///////////////////////////////////////////////////////////////////////


UndoRemoveObject::UndoRemoveObject( SdrObject& rObject, bool bOrdNumDirect )
: SdrUndoRemoveObj( rObject, bOrdNumDirect ), UndoRemovePresObjectImpl( rObject ), UndoShapeWatcher(rObject)
{
}

//---------------------------------------------------------------------

void UndoRemoveObject::Undo()
{
    DBG_ASSERT( mpObjectSafe, "sd::UndoRemoveObject::Undo(), object already dead!" );
    if( mpObjectSafe )
    {
        SdrUndoRemoveObj::Undo();
        UndoRemovePresObjectImpl::Undo();
    }
}

//---------------------------------------------------------------------

void UndoRemoveObject::Redo()
{
    DBG_ASSERT( mpObjectSafe, "sd::UndoRemoveObject::Redo(), object already dead!" );
    if( mpObjectSafe )
    {
        UndoRemovePresObjectImpl::Redo();
        SdrUndoRemoveObj::Redo();
    }
}

///////////////////////////////////////////////////////////////////////

UndoDeleteObject::UndoDeleteObject( SdrObject& rObject, bool bOrdNumDirect )
: SdrUndoDelObj( rObject, bOrdNumDirect ), UndoRemovePresObjectImpl( rObject ), UndoShapeWatcher(rObject)
{
}

//---------------------------------------------------------------------

void UndoDeleteObject::Undo()
{
    DBG_ASSERT( mpObjectSafe, "sd::UndoDeleteObject::Undo(), object already dead!" );
    if( mpObjectSafe )
    {
        SdrUndoDelObj::Undo();
        UndoRemovePresObjectImpl::Undo();
    }
}

//---------------------------------------------------------------------

void UndoDeleteObject::Redo()
{
    DBG_ASSERT( mpObjectSafe, "sd::UndoDeleteObject::Redo(), object already dead!" );
    if( mpObjectSafe )
    {
        UndoRemovePresObjectImpl::Redo();
        SdrUndoDelObj::Redo();
    }
}

///////////////////////////////////////////////////////////////////////

UndoReplaceObject::UndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject, bool bOrdNumDirect )
: SdrUndoReplaceObj( rOldObject, rNewObject, bOrdNumDirect ), UndoRemovePresObjectImpl( rOldObject ), UndoShapeWatcher( rOldObject )
{
}

//---------------------------------------------------------------------

void UndoReplaceObject::Undo()
{
    DBG_ASSERT( mpObjectSafe, "sd::UndoReplaceObject::Undo(), object already dead!" );
    if( mpObjectSafe )
    {
        SdrUndoReplaceObj::Undo();
        UndoRemovePresObjectImpl::Undo();
    }
}

//---------------------------------------------------------------------

void UndoReplaceObject::Redo()
{
    DBG_ASSERT( mpObjectSafe, "sd::UndoReplaceObject::Redo(), object already dead!" );
    if( mpObjectSafe )
    {
        UndoRemovePresObjectImpl::Redo();
        SdrUndoReplaceObj::Redo();
    }
}

///////////////////////////////////////////////////////////////////////

UndoObjectSetText::UndoObjectSetText( SdrObject& rObject )
: SdrUndoObjSetText( rObject ), mpUndoAnimation(0), mbNewEmptyPresObj(false), UndoShapeWatcher(rObject)
{
    SdPage* pPage = dynamic_cast< SdPage* >( rObject.GetPage() );
    if( pPage && pPage->hasAnimationNode() )
    {
        com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xShape( rObject.getUnoShape(), com::sun::star::uno::UNO_QUERY );
        if( pPage->getMainSequence()->hasEffect( xShape ) )
        {
            mpUndoAnimation = new UndoAnimation( static_cast< SdDrawDocument* >( pPage->GetModel() ), pPage );
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
    DBG_ASSERT( mpObjectSafe, "sd::UndoObjectSetText::Undo(), object already dead!" );
    if( mpObjectSafe )
    {
        mbNewEmptyPresObj = mpObjectSafe->IsEmptyPresObj() ? true : false;
        SdrUndoObjSetText::Undo();
        if( mpUndoAnimation )
            mpUndoAnimation->Undo();
    }
}

//---------------------------------------------------------------------

void UndoObjectSetText::Redo()
{
    DBG_ASSERT( mpObjectSafe, "sd::UndoObjectSetText::Redo(), object already dead!" );
    if( mpObjectSafe )
    {
        if( mpUndoAnimation )
            mpUndoAnimation->Redo();
        SdrUndoObjSetText::Redo();
        mpObjectSafe->SetEmptyPresObj(mbNewEmptyPresObj ? TRUE : FALSE );
    }
}

//////////////////////////////////////////////////////////////////////////////
// Undo for SdrObject::SetUserCall()

UndoObjectUserCall::UndoObjectUserCall(SdrObject& rObject)
:   SdrUndoObj(rObject),
    UndoShapeWatcher(rObject),
    mpOldUserCall((SdPage*)rObject.GetUserCall()),
    mpNewUserCall(0)
{
}

//---------------------------------------------------------------------

void UndoObjectUserCall::Undo()
{
    DBG_ASSERT( mpObjectSafe, "sd::UndoObjectUserCall::Undo(), object already dead!" );
    if( mpObjectSafe )
    {
        mpNewUserCall = mpObjectSafe->GetUserCall();
        mpObjectSafe->SetUserCall(mpOldUserCall);
    }
}

//---------------------------------------------------------------------

void UndoObjectUserCall::Redo()
{
    DBG_ASSERT( mpObjectSafe, "sd::UndoObjectUserCall::Redo(), object already dead!" );
    if( mpObjectSafe )
    {
        mpObjectSafe->SetUserCall(mpNewUserCall);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Undo for SdPage::InsertPresObj() and SdPage::RemovePresObj()

UndoObjectPresentationKind::UndoObjectPresentationKind(SdrObject& rObject)
:   SdrUndoObj(rObject),
    UndoShapeWatcher(rObject),
    meOldKind(PRESOBJ_NONE),
    meNewKind(PRESOBJ_NONE),
    mpPage( dynamic_cast< SdPage* >( rObject.GetPage() ) )
{
    DBG_ASSERT( mpPage, "sd::UndoObjectPresentationKind::UndoObjectPresentationKind(), does not work for shapes without a slide!" );

    if( mpPage)
        meOldKind = mpPage->GetPresObjKind( &rObject );
}

//---------------------------------------------------------------------

void UndoObjectPresentationKind::Undo()
{
    if( mpPage && mpObjectSafe )
    {
        meNewKind = mpPage->GetPresObjKind( mpObjectSafe );
        if( meNewKind != PRESOBJ_NONE )
            mpPage->RemovePresObj( mpObjectSafe );
        if( meOldKind != PRESOBJ_NONE )
            mpPage->InsertPresObj( mpObjectSafe, meOldKind );
    }
}

//---------------------------------------------------------------------

void UndoObjectPresentationKind::Redo()
{
    if( mpPage && mpObjectSafe )
    {
        if( meOldKind != PRESOBJ_NONE )
            mpPage->RemovePresObj( mpObjectSafe );
        if( meNewKind != PRESOBJ_NONE )
            mpPage->InsertPresObj( mpObjectSafe, meNewKind );
    }
}

//////////////////////////////////////////////////////////////////////////////

UndoAutoLayoutPosAndSize::UndoAutoLayoutPosAndSize( SdPage& rPage )
: mrPage( rPage )
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
    mrPage.SetAutoLayout( mrPage.GetAutoLayout(), FALSE, FALSE );
}

//////////////////////////////////////////////////////////////////////////////

UndoGeoObject::UndoGeoObject( SdrObject& rNewObj )
: SdrUndoGeoObj( rNewObj )
, UndoShapeWatcher( rNewObj )
, mpPage( dynamic_cast< SdPage* >( rNewObj.GetPage() ) )
{
}

//---------------------------------------------------------------------

void UndoGeoObject::Undo()
{
    DBG_ASSERT( mpObjectSafe, "sd::UndoGeoObject::Undo(), object already dead!" );
    if( mpObjectSafe )
    {
        if( mpPage )
        {
            ScopeLockGuard aGuard( mpPage->maLockAutoLayoutArrangement );
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
    DBG_ASSERT( mpObjectSafe, "sd::UndoGeoObject::Redo(), object already dead!" );
    if( mpObjectSafe )
    {
        if( mpPage )
        {
            ScopeLockGuard aGuard( mpPage->maLockAutoLayoutArrangement );
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
: SdrUndoAttrObj( rObject, bStyleSheet1 ? TRUE : FALSE, bSaveText ? TRUE : FALSE )
, UndoShapeWatcher( rObject )
, mpPage( dynamic_cast< SdPage* >( rObject.GetPage() ) )
{
}

//---------------------------------------------------------------------

void UndoAttrObject::Undo()
{
    DBG_ASSERT( mpObjectSafe, "sd::UndoAttrObject::Undo(), object already dead!" );
    if( mpObjectSafe )
    {
        if( mpPage )
        {
            ScopeLockGuard aGuard( mpPage->maLockAutoLayoutArrangement );
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
    DBG_ASSERT( mpObjectSafe, "sd::UndoAttrObject::Redo(), object already dead!" );
    if( mpObjectSafe )
    {
        if( mpPage )
        {
            ScopeLockGuard aGuard( mpPage->maLockAutoLayoutArrangement );
            SdrUndoAttrObj::Redo();
        }
        else
        {
            SdrUndoAttrObj::Redo();
        }
    }
}

//---------------------------------------------------------------------

UndoShapeWatcher::UndoShapeWatcher( SdrObject& rObject )
: mpObjectSafe( &rObject )
{
    rObject.AddObjectUser( *this );
}

UndoShapeWatcher::~UndoShapeWatcher()
{
    if( mpObjectSafe )
    {
        mpObjectSafe->RemoveObjectUser( *this );
        mpObjectSafe = 0;
    }
}

void UndoShapeWatcher::ObjectInDestruction(const SdrObject& rObject)
{
//  DBG_ERROR( "sd::UndoShapeWatcher::ObjectInDestruction(), shape destructed while still in undo! undo stack now is corrupted!" );
    mpObjectSafe = 0;
}
