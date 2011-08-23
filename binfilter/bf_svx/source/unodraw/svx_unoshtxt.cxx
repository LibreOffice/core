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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <unoshtxt.hxx>
#include <unoedhlp.hxx>

#include <bf_svtools/style.hxx>
#include <svdoutl.hxx>
#include <svdview.hxx>
#include <unoviwou.hxx>
#include "outlobj.hxx"
#include "svdpage.hxx"
#include "editeng.hxx"
#include "editobj.hxx"

#include "unotext.hxx"
#include <com/sun/star/linguistic2/XLinguServiceManager.hpp>
#include <legacysmgr/legacy_binfilters_smgr.hxx>

namespace binfilter {


using namespace ::osl;
using namespace ::rtl;

namespace css = ::com::sun::star;


//------------------------------------------------------------------------
// SvxTextEditSourceImpl
//------------------------------------------------------------------------

/** @descr 
    <p>This class essentially provides the text and view forwarders. If
    no SdrView is given, this class handles the UNO objects, which are
    currently not concerned with view issues. In this case,
    GetViewForwarder() always returns NULL and the underlying
    EditEngine of the SvxTextForwarder is a background one (i.e. not
    the official DrawOutliner, but one created exclusively for this
    object, with no relation to a view).
    </p>

    <p>If a SdrView is given at construction time, the caller is
    responsible for destroying this object when the view becomes
    invalid (the views cannot notify). If GetViewForwarder(sal_True)
    is called, the underlying shape is put into edit mode, the view
    forwarder returned encapsulates the OutlinerView and the next call
    to GetTextForwarder() yields a forwarder encapsulating the actual
    DrawOutliner. Thus, changes on that Outliner are immediately
    reflected on the screen. If the object leaves edit mode, the old
    behaviour is restored.</p>
 */
class SvxTextEditSourceImpl : public SfxListener, public SfxBroadcaster
{
private:
    oslInterlockedCount	maRefCount;

    SdrObject*						mpObject;
    SdrView*						mpView;
    const Window*					mpWindow;
    SdrModel*						mpModel;
    SdrOutliner*					mpOutliner;
    SvxOutlinerForwarder*			mpTextForwarder;
    SvxDrawOutlinerViewForwarder*	mpViewForwarder;	// if non-NULL, use GetViewModeTextForwarder text forwarder
    css::uno::Reference< css::linguistic2::XLinguServiceManager > m_xLinguServiceManager;
    Point							maTextOffset;
    BOOL							mbDataValid;
    BOOL							mbDestroyed;
    BOOL							mbIsLocked;
    BOOL							mbNeedsUpdate;
    BOOL							mbOldUndoMode;
    BOOL							mbForwarderIsEditMode;		// have to reflect that, since ENDEDIT can happen more often
    BOOL							mbShapeIsEditMode;			// #104157# only true, if HINT_BEGEDIT was received
    BOOL							mbNotificationsDisabled;	// prevent EditEngine/Outliner notifications (e.g. when setting up forwarder)

    SvxTextForwarder*				GetBackgroundTextForwarder();
    SvxTextForwarder*				GetEditModeTextForwarder();
    SvxDrawOutlinerViewForwarder*	CreateViewForwarder();

    void 							SetupOutliner();

    sal_Bool						HasView() const { return mpView ? sal_True : sal_False; }
    sal_Bool						IsEditMode() const 
                                    { 
                                        SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, mpObject );
                                        return mbShapeIsEditMode && pTextObj && pTextObj->IsTextEditActive() ? sal_True : sal_False;
                                    }

public:
    SvxTextEditSourceImpl( SdrObject* pObject );
    ~SvxTextEditSourceImpl();

    void SAL_CALL acquire();
    void SAL_CALL release();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    SvxEditSource*			Clone() const;
    SvxTextForwarder*		GetTextForwarder();
    SvxEditViewForwarder* 	GetEditViewForwarder( sal_Bool );
    void					UpdateData();

    SdrObject* 				GetSdrObject() const { return mpObject; }

    void 					lock();
    void 					unlock();

    BOOL					IsValid() const;

    Rectangle				GetVisArea();
    Point					LogicToPixel( const Point&, const MapMode& rMapMode );
    Point 					PixelToLogic( const Point&, const MapMode& rMapMode );

    DECL_LINK( NotifyHdl, EENotify* );

};

//------------------------------------------------------------------------

SvxTextEditSourceImpl::SvxTextEditSourceImpl( SdrObject* pObject )
  :	maRefCount		( 0 ),
    mpObject		( pObject ),
    mpView			( NULL ),
    mpWindow		( NULL ),
    mpModel			( pObject ? pObject->GetModel() : NULL ),
    mpOutliner		( NULL ),
    mpTextForwarder	( NULL ),
    mpViewForwarder	( NULL ),
    mbDataValid		( FALSE ),
    mbDestroyed		( FALSE ),
    mbIsLocked		( FALSE ),
    mbNeedsUpdate	( FALSE ),
    mbOldUndoMode	( FALSE ),
    mbForwarderIsEditMode ( FALSE ),
    mbShapeIsEditMode	  ( FALSE ),
    mbNotificationsDisabled ( FALSE )
{
    DBG_ASSERT( mpObject, "invalid pObject!" );

    if( mpModel )
        StartListening( *mpModel );
}

//------------------------------------------------------------------------

SvxTextEditSourceImpl::~SvxTextEditSourceImpl()
{
    DBG_ASSERT( mbIsLocked == sal_False, "text edit source was not unlocked before dispose!" );

    if( mpModel )
        EndListening( *mpModel );
    if( mpView )
        EndListening( *mpView );

    delete mpTextForwarder;
    delete mpViewForwarder;
    if( mpOutliner )
    {
        // #101088# Deregister on outliner, might be reused from outliner cache
        mpOutliner->SetNotifyHdl( Link() );

        if( mpModel )
        {
            mpModel->disposeOutliner( mpOutliner );
        }
        else
        {
            delete mpOutliner;
        }
    }
}

//------------------------------------------------------------------------

void SAL_CALL SvxTextEditSourceImpl::acquire()
{
    osl_incrementInterlockedCount( &maRefCount );
}	

//------------------------------------------------------------------------

void SAL_CALL SvxTextEditSourceImpl::release()
{
    if( ! osl_decrementInterlockedCount( &maRefCount ) )
        delete this;
}	

//------------------------------------------------------------------------

void SvxTextEditSourceImpl::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );
    const SvxViewHint* pViewHint = PTR_CAST( SvxViewHint, &rHint );

    if( pViewHint )
    {
        switch( pViewHint->GetId() )
        {
            case SVX_HINT_VIEWCHANGED:
                Broadcast( *pViewHint );
                break;
        }
    }
    else if( pSdrHint )
    {
        switch( pSdrHint->GetKind() )
        {
            case HINT_OBJCHG:
            {
                mbDataValid = FALSE;						// Text muss neu geholt werden

                if( HasView() )
                {
                    // #104157# Update maTextOffset, object has changed 
                    // #105196#, #105203#: Cannot call that // here,
                    // since TakeTextRect() (called from there) //
                    // changes outliner content.  

                    // #101029# Broadcast object changes, as they might change visible attributes
                    SvxViewHint aHint(SVX_HINT_VIEWCHANGED);
                    Broadcast( aHint );
                }
                break;
            }

            case HINT_OBJREMOVED:
                if( mpObject == pSdrHint->GetObject() )
                {
                    mbDestroyed = TRUE;
                }
                break;

            case HINT_MODELCLEARED:
                mbDestroyed = TRUE;
                break;

            case HINT_OBJLISTCLEAR:
            {
                SdrObjList* pObjList = mpObject ? mpObject->GetObjList() : NULL;
                while( pObjList )
                {
                    if( pSdrHint->GetObjList() == pObjList )
                    {
                        mbDestroyed = sal_True;
                        break;
                    }
                    
                    pObjList = pObjList->GetUpList();
                }
                break;
            }

            case HINT_BEGEDIT:
                if( mpObject == pSdrHint->GetObject() )
                {
                    // invalidate old forwarder
                    if( !mbForwarderIsEditMode )
                    {
                        delete mpTextForwarder;
                        mpTextForwarder = NULL;
                    }

                    // register as listener - need to broadcast state change messages
                    if( mpView && mpView->GetTextEditOutliner() )
                        mpView->GetTextEditOutliner()->SetNotifyHdl( LINK(this, SvxTextEditSourceImpl, NotifyHdl) );

                    // #104157# Only now we're really in edit mode
                    mbShapeIsEditMode = TRUE;

                    Broadcast( *pSdrHint );
                }
                break;

            case HINT_ENDEDIT:
                if( mpObject == pSdrHint->GetObject() )
                {
                    Broadcast( *pSdrHint );

                    // #104157# We're no longer in edit mode
                    mbShapeIsEditMode = FALSE;

                    // remove as listener - outliner might outlive ourselves
                    if( mpView && mpView->GetTextEditOutliner() )
                        mpView->GetTextEditOutliner()->SetNotifyHdl( Link() );

                    // destroy view forwarder, OutlinerView no longer
                    // valid (no need for UpdateData(), it's been
                    // synched on EndTextEdit)
                    delete mpViewForwarder;
                    mpViewForwarder = NULL;

                    // #100424# Invalidate text forwarder, we might
                    // not be called again before entering edit mode a
                    // second time! Then, the old outliner might be
                    // invalid.
                    if( mbForwarderIsEditMode )
                    {
                        mbForwarderIsEditMode = sal_False;
                        delete mpTextForwarder;
                        mpTextForwarder = NULL;
                    }
                }
                break;
        }
    }

    if( mbDestroyed )
    {
        delete mpTextForwarder;
        mpTextForwarder = NULL;

        delete mpViewForwarder;
        mpViewForwarder = NULL;

        if( mpOutliner )
        {
            // #101088# Deregister on outliner, might be reused from outliner cache
            mpOutliner->SetNotifyHdl( Link() );

            if( mpModel )
            {
                mpModel->disposeOutliner( mpOutliner );
            }
            else
            {
                delete mpOutliner;
            }
            mpOutliner = NULL;
        }

        if( mpModel )
        {
            EndListening( *mpModel );
            mpModel = NULL;
        }
        if( mpView )
        {
            EndListening( *mpView );
            mpView = NULL;
        }

        mpObject = NULL;
        mpWindow = NULL;

        Broadcast( SfxSimpleHint( SFX_HINT_DYING ) );
    }
}

//------------------------------------------------------------------------

void SvxTextEditSourceImpl::SetupOutliner()
{
    // #101029#
    // only for UAA edit source: setup outliner equivalently as in
    // SdrTextObj::Paint(), such that formatting equals screen
    // layout
    if( mpObject && mpOutliner )
    {
        SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, mpObject );
        Rectangle aPaintRect;
        if( pTextObj )
        {
            Rectangle aBoundRect( pTextObj->GetBoundRect() );
            pTextObj->SetupOutlinerFormatting( *mpOutliner, aPaintRect );

            // #101029# calc text offset from shape anchor
            maTextOffset = aPaintRect.TopLeft() - aBoundRect.TopLeft();
        }
    }
}

//------------------------------------------------------------------------

SvxTextForwarder* SvxTextEditSourceImpl::GetBackgroundTextForwarder()
{
    sal_Bool bCreated = sal_False;

    // #99840#: prevent EE/Outliner notifications during setup
    mbNotificationsDisabled = sal_True;

    if (!mpTextForwarder)
    {
        if( mpOutliner == NULL )
        {
            SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, mpObject );
            USHORT nOutlMode = OUTLINERMODE_TEXTOBJECT;
            if( pTextObj && pTextObj->IsTextFrame() && pTextObj->GetTextKind() == OBJ_OUTLINETEXT )
                nOutlMode = OUTLINERMODE_OUTLINEOBJECT;

            mpOutliner = mpModel->createOutliner( nOutlMode );

            // #109151# Do the setup after outliner creation, would be useless otherwise
            if( HasView() )
            {
                // #101029#, #104157# Setup outliner _before_ filling it
                SetupOutliner();
            }

            mpOutliner->SetTextObjNoInit( pTextObj );
/*
            mpOutliner = SdrMakeOutliner( nOutlMode, pModel );
            Outliner& aDrawOutliner = pModel->GetDrawOutliner();
            mpOutliner->SetCalcFieldValueHdl( aDrawOutliner.GetCalcFieldValueHdl() );
*/
            if( mbIsLocked )
            {
                ((EditEngine*)&(mpOutliner->GetEditEngine()))->SetUpdateMode( sal_False );
                mbOldUndoMode = ((EditEngine*)&(mpOutliner->GetEditEngine()))->IsUndoEnabled();
                ((EditEngine*)&(mpOutliner->GetEditEngine()))->EnableUndo( sal_False );
            }
        }

        mpTextForwarder = new SvxOutlinerForwarder( *mpOutliner, mpObject );

        // delay listener subscription and UAA initialization until Outliner is fully setup
        bCreated = sal_True;

        mbForwarderIsEditMode = sal_False;
    }

    if( mpObject && !mbDataValid && mpObject->IsInserted() && mpObject->GetPage() )
    {
        mpTextForwarder->flushCache();

        OutlinerParaObject* mpOutlinerParaObject = NULL;
        BOOL bTextEditActive = FALSE;
        SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, mpObject );
        if( pTextObj )
            mpOutlinerParaObject = pTextObj->GetEditOutlinerParaObject(); // Get the OutlinerParaObject if text edit is active

        if( mpOutlinerParaObject )
            bTextEditActive = TRUE;	// text edit active
        else
            mpOutlinerParaObject = mpObject->GetOutlinerParaObject();
        
        if( mpOutlinerParaObject && ( bTextEditActive || !mpObject->IsEmptyPresObj() || mpObject->GetPage()->IsMasterPage() ) )
        {
            mpOutliner->SetText( *mpOutlinerParaObject );

            // #91254# put text to object and set EmptyPresObj to FALSE
            if( pTextObj && bTextEditActive && mpOutlinerParaObject && mpObject->IsEmptyPresObj() && pTextObj->IsRealyEdited() )
            {
                mpObject->SetEmptyPresObj( FALSE );
                pTextObj->SetOutlinerParaObject( mpOutlinerParaObject );
            }
        }
        else
        {
            sal_Bool bVertical = mpOutlinerParaObject ? mpOutlinerParaObject->IsVertical() : sal_False;

            // set objects style sheet on empty outliner
            SfxStyleSheetPool* pPool = (SfxStyleSheetPool*)mpObject->GetModel()->GetStyleSheetPool();
            if( pPool )
                mpOutliner->SetStyleSheetPool( pPool );

            SfxStyleSheet* pStyleSheet = mpObject->GetPage()->GetTextStyleSheetForObject( mpObject );
            if( pStyleSheet )
                mpOutliner->SetStyleSheet( 0, pStyleSheet );

            if( bVertical )
                mpOutliner->SetVertical( sal_True );
        }

        // evtually we have to set the border attributes
        if (mpOutliner->GetParagraphCount()==1)
        {
            // if we only have one paragraph we check if it is empty
            XubString aStr( mpOutliner->GetText( mpOutliner->GetParagraph( 0 ) ) );

            if(!aStr.Len())
            {
                // its empty, so we have to force the outliner to initialise itself
                mpOutliner->SetText( String(), mpOutliner->GetParagraph( 0 ) );

                if(mpObject->GetStyleSheet())
                    mpOutliner->SetStyleSheet( 0, mpObject->GetStyleSheet());
            }
        }

        mbDataValid = TRUE;
    }

    if( bCreated && mpOutliner && HasView() )
    {
        // register as listener - need to broadcast state change messages
        // registration delayed until outliner is completely set up
        mpOutliner->SetNotifyHdl( LINK(this, SvxTextEditSourceImpl, NotifyHdl) );
    }

    // #99840#: prevent EE/Outliner notifications during setup
    mbNotificationsDisabled = sal_False;

    return mpTextForwarder;
}

//------------------------------------------------------------------------

SvxTextForwarder* SvxTextEditSourceImpl::GetEditModeTextForwarder()
{
    if( !mpTextForwarder && HasView() )
    {
        SdrOutliner* pEditOutliner = mpView->GetTextEditOutliner();

        if( pEditOutliner )
        {
            mpTextForwarder = new SvxOutlinerForwarder( *pEditOutliner, mpObject );
                
            mbForwarderIsEditMode = sal_True;
        }
    }

    return mpTextForwarder;
}

//------------------------------------------------------------------------

SvxTextForwarder* SvxTextEditSourceImpl::GetTextForwarder()
{
    if( mbDestroyed || mpObject == NULL )
        return NULL;

    if( mpModel == NULL )
        mpModel = mpObject->GetModel();

    if( mpModel == NULL )
        return NULL;

    // distinguish the cases 
    // a) connected to view, maybe edit mode is active, can work directly on the EditOutliner
    // b) background Outliner, reflect changes into ParaOutlinerObject (this is exactly the old UNO code)
    if( HasView() )
    {
        if( IsEditMode() != mbForwarderIsEditMode )
        {
            // forwarder mismatch - create new
            delete mpTextForwarder;
            mpTextForwarder = NULL;
        }

        if( IsEditMode() )
            return GetEditModeTextForwarder();
        else
            return GetBackgroundTextForwarder();
    }
    else
        return GetBackgroundTextForwarder();
}

//------------------------------------------------------------------------

SvxDrawOutlinerViewForwarder* SvxTextEditSourceImpl::CreateViewForwarder()
{
    if( mpView->GetTextEditOutlinerView() && mpObject )
    {
        // register as listener - need to broadcast state change messages
        mpView->GetTextEditOutliner()->SetNotifyHdl( LINK(this, SvxTextEditSourceImpl, NotifyHdl) );

        SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, mpObject );
        if( pTextObj )
        {
            Rectangle aBoundRect( pTextObj->GetBoundRect() );
            OutlinerView& rOutlView = *mpView->GetTextEditOutlinerView();

            return new SvxDrawOutlinerViewForwarder( rOutlView, aBoundRect.TopLeft() );
        }
    }

    return NULL;
}

SvxEditViewForwarder* SvxTextEditSourceImpl::GetEditViewForwarder( sal_Bool bCreate )
{
    if( mbDestroyed || mpObject == NULL )
        return NULL;

    if( mpModel == NULL )
        mpModel = mpObject->GetModel();

    if( mpModel == NULL )
        return NULL;

    // shall we delete?
    if( mpViewForwarder )
    {
        if( !IsEditMode() )
        {
            // destroy all forwarders (no need for UpdateData(),
            // it's been synched on EndTextEdit)
            delete mpViewForwarder;
            mpViewForwarder = NULL;
        }
    }
    // which to create? Directly in edit mode, create new, or none?
    else if( mpView )
    {
        if( IsEditMode() )
        {
            // create new view forwarder
            mpViewForwarder = CreateViewForwarder();
        }
        else if( bCreate )
        {
            // dispose old text forwarder
            UpdateData();
        
            delete mpTextForwarder;
            mpTextForwarder = NULL;

            // enter edit mode
            mpView->EndTextEdit();
            if( mpView->BegTextEdit( mpObject, NULL, NULL, (SdrOutliner*)NULL, NULL, FALSE, FALSE ) )
            {
                SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, mpObject );
                if( pTextObj->IsTextEditActive() )
                {                
                    // create new view forwarder
                    mpViewForwarder = CreateViewForwarder();
                }
                else
                {
                    // failure. Somehow, BegTextEdit did not set
                    // our SdrTextObj into edit mode
                    mpView->EndTextEdit();
                }
            }
        }
    }

    return mpViewForwarder;
}

//------------------------------------------------------------------------

void SvxTextEditSourceImpl::UpdateData()
{
    // if we have a view and in edit mode, we're working with the
    // DrawOutliner. Thus, all changes made on the text forwarder are
    // reflected on the view and committed to the model on
    // EndTextEdit(). Thus, no need for explicit updates here.
    if( !HasView() || !IsEditMode() )
    {
        if( mbIsLocked  )
        {
            mbNeedsUpdate = sal_True;
        }
        else
        {
            if( mpOutliner && mpObject && !mbDestroyed )
            {
                if( mpOutliner->GetParagraphCount() != 1 || mpOutliner->GetEditEngine().GetTextLen( 0 ) )
                {
                    if( mpOutliner->GetParagraphCount() > 1 )
                    {
                        SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, mpObject );
                        if( pTextObj && pTextObj->IsTextFrame() && pTextObj->GetTextKind() == OBJ_TITLETEXT )
                        {
                            while( mpOutliner->GetParagraphCount() > 1 )
                            {
                                ESelection aSel( 0,mpOutliner->GetEditEngine().GetTextLen( 0 ), 1,0 );
                                mpOutliner->QuickInsertLineBreak( aSel );
                            }
                        }
                    }
                        
                    mpObject->SetOutlinerParaObject( mpOutliner->CreateParaObject() );
                }
                else
                    mpObject->SetOutlinerParaObject( NULL );
                
                if( mpObject->IsEmptyPresObj() )
                    mpObject->SetEmptyPresObj(sal_False);
            }
        }
    }
}

void SvxTextEditSourceImpl::lock()
{
    mbIsLocked = sal_True;
    if( mpOutliner )
    {
        ((EditEngine*)&(mpOutliner->GetEditEngine()))->SetUpdateMode( sal_False );
        mbOldUndoMode = ((EditEngine*)&(mpOutliner->GetEditEngine()))->IsUndoEnabled();
        ((EditEngine*)&(mpOutliner->GetEditEngine()))->EnableUndo( sal_False );
    }
}

void SvxTextEditSourceImpl::unlock()
{
    mbIsLocked = sal_False;

    if( mbNeedsUpdate )
    {
        UpdateData();
        mbNeedsUpdate = sal_False;
    }

    if( mpOutliner )
    {
        ((EditEngine*)&(mpOutliner->GetEditEngine()))->SetUpdateMode( sal_True );
        ((EditEngine*)&(mpOutliner->GetEditEngine()))->EnableUndo( mbOldUndoMode );
    }
}

BOOL SvxTextEditSourceImpl::IsValid() const
{
    return mpView && mpWindow ? TRUE : FALSE;
}

Rectangle SvxTextEditSourceImpl::GetVisArea()
{
    if( IsValid() )
    {
        Rectangle aVisArea = mpView->GetVisibleArea( mpView->FindWin( const_cast< Window* > (mpWindow) ) );

        // offset vis area by edit engine left-top position
        SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, mpObject );
        if( pTextObj )
        {
            Rectangle aAnchorRect;
            pTextObj->TakeTextAnchorRect( aAnchorRect );
            aVisArea.Move( -aAnchorRect.Left(), -aAnchorRect.Top() );

            MapMode aMapMode(mpWindow->GetMapMode());
            aMapMode.SetOrigin(Point());
            return mpWindow->LogicToPixel( aVisArea, aMapMode );
        }        
    }

    return Rectangle();
}

Point SvxTextEditSourceImpl::LogicToPixel( const Point& rPoint, const MapMode& rMapMode )
{
    // #101029#: The responsibilities of ViewForwarder happen to be
    // somewhat mixed in this case. On the one hand, we need the
    // different interface queries on the SvxEditSource interface,
    // since we need both VisAreas. On the other hand, if an
    // EditViewForwarder exists, maTextOffset does not remain static,
    // but may change with every key press.
    if( IsEditMode() )
    {
        SvxEditViewForwarder* pForwarder = GetEditViewForwarder(sal_False);

        if( pForwarder )
            return pForwarder->LogicToPixel( rPoint, rMapMode );
    }    
    else if( IsValid() && mpModel )
    {
        // #101029#
        Point aPoint1( rPoint );
        aPoint1.X() += maTextOffset.X();
        aPoint1.Y() += maTextOffset.Y();

        Point aPoint2( OutputDevice::LogicToLogic( aPoint1, rMapMode, 
                                                   MapMode(mpModel->GetScaleUnit()) ) );
        MapMode aMapMode(mpWindow->GetMapMode());
        aMapMode.SetOrigin(Point());
        return mpWindow->LogicToPixel( aPoint2, aMapMode );
    }
    
    return Point();
}

Point SvxTextEditSourceImpl::PixelToLogic( const Point& rPoint, const MapMode& rMapMode )
{
    // #101029#: The responsibilities of ViewForwarder happen to be
    // somewhat mixed in this case. On the one hand, we need the
    // different interface queries on the SvxEditSource interface,
    // since we need both VisAreas. On the other hand, if an
    // EditViewForwarder exists, maTextOffset does not remain static,
    // but may change with every key press.
    if( IsEditMode() )
    {
        SvxEditViewForwarder* pForwarder = GetEditViewForwarder(sal_False);

        if( pForwarder )
            return pForwarder->PixelToLogic( rPoint, rMapMode );
    }    
    else if( IsValid() && mpModel )
    {
        MapMode aMapMode(mpWindow->GetMapMode());
        aMapMode.SetOrigin(Point());
        Point aPoint1( mpWindow->PixelToLogic( rPoint, aMapMode ) );
        Point aPoint2( OutputDevice::LogicToLogic( aPoint1, 
                                                   MapMode(mpModel->GetScaleUnit()), 
                                                   rMapMode ) );
        // #101029#
        aPoint2.X() -= maTextOffset.X();
        aPoint2.Y() -= maTextOffset.Y();

        return aPoint2;
    }
    
    return Point();
}

IMPL_LINK(SvxTextEditSourceImpl, NotifyHdl, EENotify*, aNotify)
{
    return 0;
}

//------------------------------------------------------------------------

// --------------------------------------------------------------------
// SvxTextEditSource
// --------------------------------------------------------------------

SvxTextEditSource::SvxTextEditSource( SdrObject* pObject )
{
    mpImpl = new SvxTextEditSourceImpl( pObject );
    mpImpl->acquire();
}

// --------------------------------------------------------------------

SvxTextEditSource::SvxTextEditSource( SvxTextEditSourceImpl* pImpl )
{
    mpImpl = pImpl;
    mpImpl->acquire();
}

//------------------------------------------------------------------------
SvxTextEditSource::~SvxTextEditSource()
{
    ::SolarMutexGuard aGuard;

    mpImpl->release();
}

//------------------------------------------------------------------------
SvxEditSource* SvxTextEditSource::Clone() const
{
    return new SvxTextEditSource( mpImpl );
}

//------------------------------------------------------------------------
SvxTextForwarder* SvxTextEditSource::GetTextForwarder()
{
    return mpImpl->GetTextForwarder();
}

//------------------------------------------------------------------------
SvxEditViewForwarder* SvxTextEditSource::GetEditViewForwarder( sal_Bool bCreate )
{
    return mpImpl->GetEditViewForwarder( bCreate );
}

//------------------------------------------------------------------------
SvxViewForwarder* SvxTextEditSource::GetViewForwarder()
{
    return this;
}

//------------------------------------------------------------------------
void SvxTextEditSource::UpdateData()
{
    mpImpl->UpdateData();
}

SfxBroadcaster& SvxTextEditSource::GetBroadcaster() const
{
    return *mpImpl;
}

void SvxTextEditSource::lock()
{
    mpImpl->lock();
}

void SvxTextEditSource::unlock()
{
    mpImpl->unlock();
}

BOOL SvxTextEditSource::IsValid() const
{
    return mpImpl->IsValid();
}

Rectangle SvxTextEditSource::GetVisArea() const
{
    return mpImpl->GetVisArea();
}

Point SvxTextEditSource::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
{
    return mpImpl->LogicToPixel( rPoint, rMapMode );
}

Point SvxTextEditSource::PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const
{
    return mpImpl->PixelToLogic( rPoint, rMapMode );
}

/** this method returns true if the outliner para object of the given shape has
    a paragraph with a level > 0 or if there is a paragraph with the EE_PARA_BULLETSTATE
    set to true. This is needed for xml export to decide if we need to export the
    level information.
*/
sal_Bool SvxTextEditSource::hasLevels( const SdrObject* pObject )
{
    OutlinerParaObject* pOutlinerParaObject = pObject->GetOutlinerParaObject();
    if( NULL == pOutlinerParaObject )
        return sal_False;

    USHORT nParaCount = (USHORT)pOutlinerParaObject->Count();
    USHORT nPara;
    for( nPara = 0; nPara < nParaCount; nPara++ )
    {
        if( pOutlinerParaObject->GetDepth( nPara ) > 0 )
            return sal_True;
    }

    sal_Bool bHadBulletStateOnEachPara = sal_True;

    const EditTextObject& rEditTextObject = pOutlinerParaObject->GetTextObject();
    const SfxPoolItem* pItem;

    for( nPara = 0; nPara < nParaCount; nPara++ )
    {
        SfxItemSet aSet = rEditTextObject.GetParaAttribs( nPara );
        if( aSet.GetItemState(EE_PARA_BULLETSTATE, sal_False, &pItem) == SFX_ITEM_SET )
        {
            if( ((const SfxUInt16Item*) pItem)->GetValue() )
                return sal_True;
        }
        else
        {
            bHadBulletStateOnEachPara = sal_False;
        }
    }

    // if there was at least one paragraph without a bullet state item we
    // also need to check the stylesheet for a bullet state item
    if( !bHadBulletStateOnEachPara && pObject->GetStyleSheet() )
    {
        const SfxItemSet& rSet = pObject->GetStyleSheet()->GetItemSet();
        if( rSet.GetItemState(EE_PARA_BULLETSTATE, sal_False, &pItem) == SFX_ITEM_SET )
        {
            if( ((const SfxUInt16Item*)pItem)->GetValue() )
                return sal_True;
        }
    }

    return sal_False;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
