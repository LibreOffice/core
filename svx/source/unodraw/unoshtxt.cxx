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

#include <com/sun/star/uno/XInterface.hpp>
#include <vcl/svapp.hxx>

#include <svx/unoshtxt.hxx>
#include <editeng/unoedhlp.hxx>
#include <svl/lstner.hxx>
#include <rtl/ref.hxx>
#include <osl/mutex.hxx>
#include <svl/hint.hxx>
#include <svl/style.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdview.hxx>
#include <svx/svdetc.hxx>
#include <editeng/outliner.hxx>
#include <editeng/unoforou.hxx>
#include <editeng/unoviwou.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdpage.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>

#include <editeng/unotext.hxx>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
#include <comphelper/processfactory.hxx>
#include <svx/sdrpaintwindow.hxx>

using namespace ::osl;
using namespace ::rtl;

using ::com::sun::star::uno::XInterface;

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
class SvxTextEditSourceImpl : public SfxListener, public SfxBroadcaster, public sdr::ObjectUser
{
private:
    oslInterlockedCount maRefCount;

    SdrObject*                      mpObject;
    SdrText*                        mpText;
    SdrView*                        mpView;
    const Window*                   mpWindow;
    SdrModel*                       mpModel;
    SdrOutliner*                    mpOutliner;
    SvxOutlinerForwarder*           mpTextForwarder;
    SvxDrawOutlinerViewForwarder*   mpViewForwarder;    // if non-NULL, use GetViewModeTextForwarder text forwarder
    css::uno::Reference< css::linguistic2::XLinguServiceManager2 > m_xLinguServiceManager;
    Point                           maTextOffset;
    sal_Bool                            mbDataValid;
    sal_Bool                            mbDestroyed;
    sal_Bool                            mbIsLocked;
    sal_Bool                            mbNeedsUpdate;
    sal_Bool                            mbOldUndoMode;
    sal_Bool                            mbForwarderIsEditMode;      // have to reflect that, since ENDEDIT can happen more often
    sal_Bool                            mbShapeIsEditMode;          // #104157# only true, if HINT_BEGEDIT was received
    sal_Bool                            mbNotificationsDisabled;    // prevent EditEngine/Outliner notifications (e.g. when setting up forwarder)

    XInterface*                     mpOwner;
    SvxUnoTextRangeBaseList         maTextRanges;

    SvxTextForwarder*               GetBackgroundTextForwarder();
    SvxTextForwarder*               GetEditModeTextForwarder();
    SvxDrawOutlinerViewForwarder*   CreateViewForwarder();

    void                            SetupOutliner();

    sal_Bool                        HasView() const { return mpView ? sal_True : sal_False; }
    sal_Bool                        IsEditMode() const
                                    {
                                        SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, mpObject );
                                        return mbShapeIsEditMode && pTextObj && pTextObj->IsTextEditActive() ? sal_True : sal_False;
                                    }

    void                            dispose();

public:
    SvxTextEditSourceImpl( SdrObject* pObject, SdrText* pText, XInterface* pOwner );
    SvxTextEditSourceImpl( SdrObject& rObject, SdrText* pText, SdrView& rView, const Window& rWindow );
    ~SvxTextEditSourceImpl();

    void SAL_CALL acquire();
    void SAL_CALL release();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    SvxEditSource*          Clone() const;
    SvxTextForwarder*       GetTextForwarder();
    SvxEditViewForwarder*   GetEditViewForwarder( sal_Bool );
    void                    UpdateData();

    void addRange( SvxUnoTextRangeBase* pNewRange );
    void removeRange( SvxUnoTextRangeBase* pOldRange );
    const SvxUnoTextRangeBaseList& getRanges() const;

    SdrObject*              GetSdrObject() const { return mpObject; }

    void                    lock();
    void                    unlock();

    sal_Bool                    IsValid() const;

    Rectangle               GetVisArea();
    Point                   LogicToPixel( const Point&, const MapMode& rMapMode );
    Point                   PixelToLogic( const Point&, const MapMode& rMapMode );

    DECL_LINK( NotifyHdl, EENotify* );

    virtual void ObjectInDestruction(const SdrObject& rObject);

    void ChangeModel( SdrModel* pNewModel );

    void                    UpdateOutliner();
};

//------------------------------------------------------------------------

SvxTextEditSourceImpl::SvxTextEditSourceImpl( SdrObject* pObject, SdrText* pText, XInterface* pOwner )
  : maRefCount      ( 0 ),
    mpObject        ( pObject ),
    mpText          ( pText ),
    mpView          ( NULL ),
    mpWindow        ( NULL ),
    mpModel         ( pObject ? pObject->GetModel() : NULL ),
    mpOutliner      ( NULL ),
    mpTextForwarder ( NULL ),
    mpViewForwarder ( NULL ),
    mbDataValid     ( sal_False ),
    mbDestroyed     ( sal_False ),
    mbIsLocked      ( sal_False ),
    mbNeedsUpdate   ( sal_False ),
    mbOldUndoMode   ( sal_False ),
    mbForwarderIsEditMode ( sal_False ),
    mbShapeIsEditMode     ( sal_False ),
    mbNotificationsDisabled ( sal_False ),
    mpOwner( pOwner )
{
    DBG_ASSERT( mpObject, "invalid pObject!" );

    if( !mpText )
    {
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( mpObject );
        if( pTextObj )
            mpText = pTextObj->getText( 0 );
    }

    if( mpModel )
        StartListening( *mpModel );

    if( mpObject )
        mpObject->AddObjectUser( *this );
}

//------------------------------------------------------------------------

SvxTextEditSourceImpl::SvxTextEditSourceImpl( SdrObject& rObject, SdrText* pText, SdrView& rView, const Window& rWindow )
  : maRefCount      ( 0 ),
    mpObject        ( &rObject ),
    mpText          ( pText ),
    mpView          ( &rView ),
    mpWindow        ( &rWindow ),
    mpModel         ( rObject.GetModel() ),
    mpOutliner      ( NULL ),
    mpTextForwarder ( NULL ),
    mpViewForwarder ( NULL ),
    mbDataValid     ( sal_False ),
    mbDestroyed     ( sal_False ),
    mbIsLocked      ( sal_False ),
    mbNeedsUpdate   ( sal_False ),
    mbOldUndoMode   ( sal_False ),
    mbForwarderIsEditMode ( sal_False ),
    mbShapeIsEditMode     ( sal_True ),
    mbNotificationsDisabled ( sal_False ),
    mpOwner(0)
{
    if( !mpText )
    {
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( mpObject );
        if( pTextObj )
            mpText = pTextObj->getText( 0 );
    }

    if( mpModel )
        StartListening( *mpModel );
    if( mpView )
        StartListening( *mpView );
    if( mpObject )
        mpObject->AddObjectUser( *this );

    // #104157# Init edit mode state from shape info (IsTextEditActive())
    mbShapeIsEditMode = IsEditMode();
}

//------------------------------------------------------------------------

SvxTextEditSourceImpl::~SvxTextEditSourceImpl()
{
    DBG_ASSERT( mbIsLocked == sal_False, "text edit source was not unlocked before dispose!" );
    if( mpObject )
        mpObject->RemoveObjectUser( *this );

    dispose();
}

//------------------------------------------------------------------------

void SvxTextEditSourceImpl::addRange( SvxUnoTextRangeBase* pNewRange )
{
    if( pNewRange )
        if( std::find( maTextRanges.begin(), maTextRanges.end(), pNewRange ) == maTextRanges.end() )
            maTextRanges.push_back( pNewRange );
}

//------------------------------------------------------------------------

void SvxTextEditSourceImpl::removeRange( SvxUnoTextRangeBase* pOldRange )
{
    if( pOldRange )
        maTextRanges.remove( pOldRange );
}

//------------------------------------------------------------------------

const SvxUnoTextRangeBaseList& SvxTextEditSourceImpl::getRanges() const
{
    return maTextRanges;
}

//------------------------------------------------------------------------

void SAL_CALL SvxTextEditSourceImpl::acquire()
{
    osl_atomic_increment( &maRefCount );
}

//------------------------------------------------------------------------

void SAL_CALL SvxTextEditSourceImpl::release()
{
    if( ! osl_atomic_decrement( &maRefCount ) )
        delete this;
}

void SvxTextEditSourceImpl::ChangeModel( SdrModel* pNewModel )
{
    if( mpModel != pNewModel )
    {
        if( mpModel )
            EndListening( *mpModel );

        if( mpOutliner )
        {
            if( mpModel )
                mpModel->disposeOutliner( mpOutliner );
            else
                delete mpOutliner;
            mpOutliner = 0;
        }

        if( mpView )
        {
            EndListening( *mpView );
            mpView = 0;
        }

        mpWindow = 0;
        m_xLinguServiceManager.clear();
        mpOwner = 0;

        mpModel = pNewModel;

        if( mpTextForwarder )
        {
            delete mpTextForwarder;
            mpTextForwarder = 0;
        }

        if( mpViewForwarder )
        {
            delete mpViewForwarder;
            mpViewForwarder = 0;
        }

        if( mpModel )
            StartListening( *mpModel );
    }
}

//------------------------------------------------------------------------

void SvxTextEditSourceImpl::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    // #i105988 keep reference to this object
    rtl::Reference< SvxTextEditSourceImpl > xThis( this );

    const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );
    const SvxViewHint* pViewHint = PTR_CAST( SvxViewHint, &rHint );

    if( pViewHint )
    {
        switch( pViewHint->GetHintType() )
        {
            case SvxViewHint::SVX_HINT_VIEWCHANGED:
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
                mbDataValid = sal_False;                        // Text muss neu geholt werden

                if( HasView() )
                {
                    // #104157# Update maTextOffset, object has changed
                    // #105196#, #105203#: Cannot call that // here,
                    // since TakeTextRect() (called from there) //
                    // changes outliner content.
                    // UpdateOutliner();

                    // #101029# Broadcast object changes, as they might change visible attributes
                    SvxViewHint aHint(SvxViewHint::SVX_HINT_VIEWCHANGED);
                    Broadcast( aHint );
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
                    mbShapeIsEditMode = sal_True;

                    Broadcast( *pSdrHint );
                }
                break;

            case HINT_ENDEDIT:
                if( mpObject == pSdrHint->GetObject() )
                {
                    Broadcast( *pSdrHint );

                    // #104157# We're no longer in edit mode
                    mbShapeIsEditMode = sal_False;

                    // remove as listener - outliner might outlive ourselves
                    if( mpView && mpView->GetTextEditOutliner() )
                        mpView->GetTextEditOutliner()->SetNotifyHdl( Link() );

                    // destroy view forwarder, OutlinerView no longer
                    // valid (no need for UpdateData(), it's been
                    // synched on SdrEndTextEdit)
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

            case HINT_MODELCLEARED:
                dispose();
                break;
            default:
                break;
        }
    }
}

/* this is a callback from the attached SdrObject when it is actually deleted */
void SvxTextEditSourceImpl::ObjectInDestruction(const SdrObject&)
{
    mpObject = 0;
    dispose();
    Broadcast( SfxSimpleHint( SFX_HINT_DYING ) );
}

/* unregister at all objects and set all references to 0 */
void SvxTextEditSourceImpl::dispose()
{
    if( mpTextForwarder )
    {
        delete mpTextForwarder;
        mpTextForwarder = 0;
    }

    if( mpViewForwarder )
    {
        delete mpViewForwarder;
        mpViewForwarder = 0;
    }

    if( mpOutliner )
    {
        if( mpModel )
        {
            mpModel->disposeOutliner( mpOutliner );
        }
        else
        {
            delete mpOutliner;
        }
        mpOutliner = 0;
    }

    if( mpModel )
    {
        EndListening( *mpModel );
        mpModel = 0;
    }

    if( mpView )
    {
        EndListening( *mpView );
        mpView = 0;
    }

    if( mpObject )
    {
        mpObject->RemoveObjectUser( *this );
        mpObject = 0;
    }
    mpWindow = 0;
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
            Rectangle aBoundRect( pTextObj->GetCurrentBoundRect() );
            pTextObj->SetupOutlinerFormatting( *mpOutliner, aPaintRect );

            // #101029# calc text offset from shape anchor
            maTextOffset = aPaintRect.TopLeft() - aBoundRect.TopLeft();
        }
    }
}

//------------------------------------------------------------------------

void SvxTextEditSourceImpl::UpdateOutliner()
{
    // #104157#
    // only for UAA edit source: update outliner equivalently as in
    // SdrTextObj::Paint(), such that formatting equals screen
    // layout
    if( mpObject && mpOutliner )
    {
        SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, mpObject );
        Rectangle aPaintRect;
        if( pTextObj )
        {
            Rectangle aBoundRect( pTextObj->GetCurrentBoundRect() );
            pTextObj->UpdateOutlinerFormatting( *mpOutliner, aPaintRect );

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
            sal_uInt16 nOutlMode = OUTLINERMODE_TEXTOBJECT;
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
            if( mbIsLocked )
            {
                ((EditEngine*)&(mpOutliner->GetEditEngine()))->SetUpdateMode( sal_False );
                mbOldUndoMode = ((EditEngine*)&(mpOutliner->GetEditEngine()))->IsUndoEnabled();
                ((EditEngine*)&(mpOutliner->GetEditEngine()))->EnableUndo( sal_False );
            }

            if ( !m_xLinguServiceManager.is() )
            {
                css::uno::Reference< css::uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
                m_xLinguServiceManager.set(css::linguistic2::LinguServiceManager::create(xContext));
            }

            css::uno::Reference< css::linguistic2::XHyphenator > xHyphenator( m_xLinguServiceManager->getHyphenator(), css::uno::UNO_QUERY );
            if( xHyphenator.is() )
                mpOutliner->SetHyphenator( xHyphenator );
        }


        mpTextForwarder = new SvxOutlinerForwarder( *mpOutliner, (mpObject->GetObjInventor() == SdrInventor) && (mpObject->GetObjIdentifier() == OBJ_OUTLINETEXT) );
        // delay listener subscription and UAA initialization until Outliner is fully setup
        bCreated = sal_True;

        mbForwarderIsEditMode = sal_False;
    }

    if( mpObject && mpText && !mbDataValid && mpObject->IsInserted() && mpObject->GetPage() )
    {
        mpTextForwarder->flushCache();

        OutlinerParaObject* pOutlinerParaObject = NULL;
        SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, mpObject );
        if( pTextObj && pTextObj->getActiveText() == mpText )
            pOutlinerParaObject = pTextObj->GetEditOutlinerParaObject(); // Get the OutlinerParaObject if text edit is active
        bool bOwnParaObj(false);

        if( pOutlinerParaObject )
            bOwnParaObj = true; // text edit active
        else
            pOutlinerParaObject = mpText->GetOutlinerParaObject();

        if( pOutlinerParaObject && ( bOwnParaObj || !mpObject->IsEmptyPresObj() || mpObject->GetPage()->IsMasterPage() ) )
        {
            mpOutliner->SetText( *pOutlinerParaObject );

            // #91254# put text to object and set EmptyPresObj to FALSE
            if( mpText && bOwnParaObj && pOutlinerParaObject && mpObject->IsEmptyPresObj() && pTextObj->IsRealyEdited() )
            {
                mpObject->SetEmptyPresObj( sal_False );
                static_cast< SdrTextObj* >( mpObject)->NbcSetOutlinerParaObjectForText( pOutlinerParaObject, mpText );

                // #i103982# Here, due to mpObject->NbcSetOutlinerParaObjectForText, we LOSE ownership of the
                // OPO, so do NOT delete it when leaving this method (!)
                bOwnParaObj = false;
            }
        }
        else
        {
            sal_Bool bVertical = pOutlinerParaObject ? pOutlinerParaObject->IsVertical() : sal_False;

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
            rtl::OUString aStr(mpOutliner->GetText(mpOutliner->GetParagraph(0)));

            if (aStr.isEmpty())
            {
                // its empty, so we have to force the outliner to initialise itself
                mpOutliner->SetText( String(), mpOutliner->GetParagraph( 0 ) );

                if(mpObject->GetStyleSheet())
                    mpOutliner->SetStyleSheet( 0, mpObject->GetStyleSheet());
            }
        }

        mbDataValid = sal_True;

        if( bOwnParaObj )
            delete pOutlinerParaObject;
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
            mpTextForwarder = new SvxOutlinerForwarder( *pEditOutliner, (mpObject->GetObjInventor() == SdrInventor) && (mpObject->GetObjIdentifier() == OBJ_OUTLINETEXT) );
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
            Rectangle aBoundRect( pTextObj->GetCurrentBoundRect() );
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
            // it's been synched on SdrEndTextEdit)
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
            mpView->SdrEndTextEdit();

            if(mpView->SdrBeginTextEdit(mpObject, 0L, 0L, sal_False, (SdrOutliner*)0L, 0L, sal_False, sal_False))
            {
                SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, mpObject );
                if( pTextObj->IsTextEditActive() )
                {
                    // create new view forwarder
                    mpViewForwarder = CreateViewForwarder();
                }
                else
                {
                    // failure. Somehow, SdrBeginTextEdit did not set
                    // our SdrTextObj into edit mode
                    mpView->SdrEndTextEdit();
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
    // SdrEndTextEdit(). Thus, no need for explicit updates here.
    if( !HasView() || !IsEditMode() )
    {
        if( mbIsLocked  )
        {
            mbNeedsUpdate = sal_True;
        }
        else
        {
            if( mpOutliner && mpObject && mpText && !mbDestroyed )
            {
                SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( mpObject );
                if( pTextObj )
                {
                    if( mpOutliner->GetParagraphCount() != 1 || mpOutliner->GetEditEngine().GetTextLen( 0 ) )
                    {
                        if( mpOutliner->GetParagraphCount() > 1 )
                        {
                            if( pTextObj && pTextObj->IsTextFrame() && pTextObj->GetTextKind() == OBJ_TITLETEXT )
                            {
                                while( mpOutliner->GetParagraphCount() > 1 )
                                {
                                    ESelection aSel( 0,mpOutliner->GetEditEngine().GetTextLen( 0 ), 1,0 );
                                    mpOutliner->QuickInsertLineBreak( aSel );
                                }
                            }
                        }

                        pTextObj->NbcSetOutlinerParaObjectForText( mpOutliner->CreateParaObject(), mpText );
                    }
                    else
                    {
                        pTextObj->NbcSetOutlinerParaObjectForText( NULL,mpText );
                    }
                }

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

sal_Bool SvxTextEditSourceImpl::IsValid() const
{
    return mpView && mpWindow ? sal_True : sal_False;
}

Rectangle SvxTextEditSourceImpl::GetVisArea()
{
    if( IsValid() )
    {
        SdrPaintWindow* pPaintWindow = mpView->FindPaintWindow(*mpWindow);
        Rectangle aVisArea;

        if(pPaintWindow)
        {
            aVisArea = pPaintWindow->GetVisibleArea();
        }

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
    if( aNotify && !mbNotificationsDisabled )
    {
        ::std::auto_ptr< SfxHint > aHint( SvxEditSourceHelper::EENotification2Hint( aNotify) );

        if( aHint.get() )
            Broadcast( *aHint.get() );
    }

    return 0;
}

//------------------------------------------------------------------------

// --------------------------------------------------------------------
// SvxTextEditSource
// --------------------------------------------------------------------

SvxTextEditSource::SvxTextEditSource( SdrObject* pObject, SdrText* pText, XInterface* pOwner )
{
    mpImpl = new SvxTextEditSourceImpl( pObject, pText, pOwner );
    mpImpl->acquire();
}

// --------------------------------------------------------------------
SvxTextEditSource::SvxTextEditSource( SdrObject& rObj, SdrText* pText, SdrView& rView, const Window& rWindow )
{
    mpImpl = new SvxTextEditSourceImpl( rObj, pText, rView, rWindow );
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

sal_Bool SvxTextEditSource::IsValid() const
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

void SvxTextEditSource::addRange( SvxUnoTextRangeBase* pNewRange )
{
    mpImpl->addRange( pNewRange );
}

void SvxTextEditSource::removeRange( SvxUnoTextRangeBase* pOldRange )
{
    mpImpl->removeRange( pOldRange );
}

const SvxUnoTextRangeBaseList& SvxTextEditSource::getRanges() const
{
    return mpImpl->getRanges();
}

void SvxTextEditSource::ChangeModel( SdrModel* pNewModel )
{
    mpImpl->ChangeModel( pNewModel );
}

void SvxTextEditSource::UpdateOutliner()
{
    mpImpl->UpdateOutliner();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
