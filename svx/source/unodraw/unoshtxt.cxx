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

#include <sal/config.h>

#include <memory>

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
#include <svx/svdotable.hxx>
#include "../table/cell.hxx"
#include <svx/sdrpaintwindow.hxx>

using namespace ::osl;

using ::com::sun::star::uno::XInterface;


// SvxTextEditSourceImpl


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
    VclPtr<const vcl::Window>       mpWindow;
    SdrModel*                       mpModel;
    SdrOutliner*                    mpOutliner;
    SvxOutlinerForwarder*           mpTextForwarder;
    SvxDrawOutlinerViewForwarder*   mpViewForwarder;    // if non-NULL, use GetViewModeTextForwarder text forwarder
    css::uno::Reference< css::linguistic2::XLinguServiceManager2 > m_xLinguServiceManager;
    Point                           maTextOffset;
    bool                            mbDataValid;
    bool                            mbDestroyed;
    bool                            mbIsLocked;
    bool                            mbNeedsUpdate;
    bool                            mbOldUndoMode;
    bool                            mbForwarderIsEditMode;      // have to reflect that, since ENDEDIT can happen more often
    bool                            mbShapeIsEditMode;          // #104157# only true, if HINT_BEGEDIT was received
    bool                            mbNotificationsDisabled;    // prevent EditEngine/Outliner notifications (e.g. when setting up forwarder)

    SvxUnoTextRangeBaseList         maTextRanges;

    SvxTextForwarder*               GetBackgroundTextForwarder();
    SvxTextForwarder*               GetEditModeTextForwarder();
    SvxDrawOutlinerViewForwarder*   CreateViewForwarder();

    void                            SetupOutliner();

    bool                            HasView() const { return mpView != nullptr; }
    bool                            IsEditMode() const
                                    {
                                        SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>( mpObject  );
                                        return mbShapeIsEditMode && pTextObj && pTextObj->IsTextEditActive();
                                    }

    void                            dispose();

public:
    SvxTextEditSourceImpl( SdrObject* pObject, SdrText* pText );
    SvxTextEditSourceImpl( SdrObject& rObject, SdrText* pText, SdrView& rView, const vcl::Window& rWindow );
    virtual ~SvxTextEditSourceImpl();

    void SAL_CALL acquire();
    void SAL_CALL release();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    SvxTextForwarder*       GetTextForwarder();
    SvxEditViewForwarder*   GetEditViewForwarder( bool );
    void                    UpdateData();

    void addRange( SvxUnoTextRangeBase* pNewRange );
    void removeRange( SvxUnoTextRangeBase* pOldRange );
    const SvxUnoTextRangeBaseList& getRanges() const { return maTextRanges;}

    void                    lock();
    void                    unlock();

    bool                    IsValid() const;

    Rectangle               GetVisArea();
    Point                   LogicToPixel( const Point&, const MapMode& rMapMode );
    Point                   PixelToLogic( const Point&, const MapMode& rMapMode );

    DECL_LINK_TYPED( NotifyHdl, EENotify&, void );

    virtual void ObjectInDestruction(const SdrObject& rObject) override;

    void ChangeModel( SdrModel* pNewModel );

    void                    UpdateOutliner();
};



SvxTextEditSourceImpl::SvxTextEditSourceImpl( SdrObject* pObject, SdrText* pText )
  : maRefCount      ( 0 ),
    mpObject        ( pObject ),
    mpText          ( pText ),
    mpView          ( NULL ),
    mpWindow        ( NULL ),
    mpModel         ( pObject ? pObject->GetModel() : NULL ),
    mpOutliner      ( NULL ),
    mpTextForwarder ( NULL ),
    mpViewForwarder ( NULL ),
    mbDataValid     ( false ),
    mbDestroyed     ( false ),
    mbIsLocked      ( false ),
    mbNeedsUpdate   ( false ),
    mbOldUndoMode   ( false ),
    mbForwarderIsEditMode ( false ),
    mbShapeIsEditMode     ( false ),
    mbNotificationsDisabled ( false )
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



SvxTextEditSourceImpl::SvxTextEditSourceImpl( SdrObject& rObject, SdrText* pText, SdrView& rView, const vcl::Window& rWindow )
  : maRefCount      ( 0 ),
    mpObject        ( &rObject ),
    mpText          ( pText ),
    mpView          ( &rView ),
    mpWindow        ( &rWindow ),
    mpModel         ( rObject.GetModel() ),
    mpOutliner      ( NULL ),
    mpTextForwarder ( NULL ),
    mpViewForwarder ( NULL ),
    mbDataValid     ( false ),
    mbDestroyed     ( false ),
    mbIsLocked      ( false ),
    mbNeedsUpdate   ( false ),
    mbOldUndoMode   ( false ),
    mbForwarderIsEditMode ( false ),
    mbShapeIsEditMode     ( true ),
    mbNotificationsDisabled ( false )
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



SvxTextEditSourceImpl::~SvxTextEditSourceImpl()
{
    DBG_ASSERT( !mbIsLocked, "text edit source was not unlocked before dispose!" );
    if( mpObject )
        mpObject->RemoveObjectUser( *this );

    dispose();
}



void SvxTextEditSourceImpl::addRange( SvxUnoTextRangeBase* pNewRange )
{
    if( pNewRange )
        if( std::find( maTextRanges.begin(), maTextRanges.end(), pNewRange ) == maTextRanges.end() )
            maTextRanges.push_back( pNewRange );
}



void SvxTextEditSourceImpl::removeRange( SvxUnoTextRangeBase* pOldRange )
{
    if( pOldRange )
        maTextRanges.remove( pOldRange );
}






void SAL_CALL SvxTextEditSourceImpl::acquire()
{
    osl_atomic_increment( &maRefCount );
}



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



void SvxTextEditSourceImpl::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    // #i105988 keep reference to this object
    rtl::Reference< SvxTextEditSourceImpl > xThis( this );

    const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>(&rHint);
    const SvxViewHint* pViewHint = dynamic_cast<const SvxViewHint*>(&rHint);
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);

    if (pSimpleHint)
    {
        if (SFX_HINT_DYING == pSimpleHint->GetId())
        {
            if (&rBC == mpView)
            {
                mpView = 0;
                if (mpViewForwarder)
                {
                    delete mpViewForwarder;
                    mpViewForwarder = 0;
                }
            }
        }
    }
    else if( pViewHint )
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
                mbDataValid = false;                        // Text muss neu geholt werden

                if( HasView() )
                {
                    // #104157# Update maTextOffset, object has changed
                    // #105196#, #105203#: Cannot call that // here,
                    // since TakeTextRect() (called from there)
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
                    // Once HINT_BEGEDIT is broadcast, each EditSource of
                    // AccessibleCell will handle it here and call below:
                    // mpView->GetTextEditOutliner()->SetNotifyHdl(), which
                    // will replace the Notifer for current editable cell. It
                    // is totally wrong. So add check here to avoid the
                    // incorrect replacement of notifer.

                    // Currently it only happens on the editsource of
                    // AccessibleCell
                    if (mpObject && mpText)
                    {
                        sdr::table::SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( mpObject );
                        if(pTableObj)
                        {
                            sdr::table::CellRef xCell = pTableObj->getActiveCell();
                            if (xCell.is())
                            {
                                sdr::table::Cell* pCellObj = dynamic_cast< sdr::table::Cell* >( mpText );
                                if (pCellObj && xCell.get() != pCellObj)
                                    break;
                            }
                        }
                    }
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
                    mbShapeIsEditMode = true;

                    Broadcast( *pSdrHint );
                }
                break;

            case HINT_ENDEDIT:
                if( mpObject == pSdrHint->GetObject() )
                {
                    Broadcast( *pSdrHint );

                    // #104157# We're no longer in edit mode
                    mbShapeIsEditMode = false;

                    // remove as listener - outliner might outlive ourselves
                    if( mpView && mpView->GetTextEditOutliner() )
                        mpView->GetTextEditOutliner()->SetNotifyHdl( Link<EENotify&,void>() );

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
                        mbForwarderIsEditMode = false;
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



void SvxTextEditSourceImpl::SetupOutliner()
{
    // only for UAA edit source: setup outliner equivalently as in
    // SdrTextObj::Paint(), such that formatting equals screen
    // layout
    if( mpObject && mpOutliner )
    {
        SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>( mpObject  );
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



void SvxTextEditSourceImpl::UpdateOutliner()
{
    // only for UAA edit source: update outliner equivalently as in
    // SdrTextObj::Paint(), such that formatting equals screen
    // layout
    if( mpObject && mpOutliner )
    {
        SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>( mpObject  );
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





SvxTextForwarder* SvxTextEditSourceImpl::GetBackgroundTextForwarder()
{
    bool bCreated = false;

    // #99840#: prevent EE/Outliner notifications during setup
    mbNotificationsDisabled = true;

    if (!mpTextForwarder)
    {
        if( mpOutliner == NULL )
        {
            SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>( mpObject  );
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
                const_cast<EditEngine*>(&(mpOutliner->GetEditEngine()))->SetUpdateMode( false );
                mbOldUndoMode = const_cast<EditEngine*>(&(mpOutliner->GetEditEngine()))->IsUndoEnabled();
                const_cast<EditEngine*>(&(mpOutliner->GetEditEngine()))->EnableUndo( false );
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
        bCreated = true;

        mbForwarderIsEditMode = false;
    }

    if( mpObject && mpText && !mbDataValid && mpObject->IsInserted() && mpObject->GetPage() )
    {
        mpTextForwarder->flushCache();

        OutlinerParaObject* pOutlinerParaObject = NULL;
        SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>( mpObject  );
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
            if( mpText && bOwnParaObj && mpObject->IsEmptyPresObj() && pTextObj->IsReallyEdited() )
            {
                mpObject->SetEmptyPresObj( false );
                static_cast< SdrTextObj* >( mpObject)->NbcSetOutlinerParaObjectForText( pOutlinerParaObject, mpText );

                // #i103982# Here, due to mpObject->NbcSetOutlinerParaObjectForText, we LOSE ownership of the
                // OPO, so do NOT delete it when leaving this method (!)
                bOwnParaObj = false;
            }
        }
        else
        {
            bool bVertical = pOutlinerParaObject && pOutlinerParaObject->IsVertical();

            // set objects style sheet on empty outliner
            SfxStyleSheetPool* pPool = static_cast<SfxStyleSheetPool*>(mpObject->GetModel()->GetStyleSheetPool());
            if( pPool )
                mpOutliner->SetStyleSheetPool( pPool );

            SfxStyleSheet* pStyleSheet = mpObject->GetPage()->GetTextStyleSheetForObject( mpObject );
            if( pStyleSheet )
                mpOutliner->SetStyleSheet( 0, pStyleSheet );

            if( bVertical )
                mpOutliner->SetVertical( true );
        }

        // maybe we have to set the border attributes
        if (mpOutliner->GetParagraphCount()==1)
        {
            // if we only have one paragraph we check if it is empty
            OUString aStr(mpOutliner->GetText(mpOutliner->GetParagraph(0)));

            if (aStr.isEmpty())
            {
                // its empty, so we have to force the outliner to initialise itself
                mpOutliner->SetText( "", mpOutliner->GetParagraph( 0 ) );

                if(mpObject->GetStyleSheet())
                    mpOutliner->SetStyleSheet( 0, mpObject->GetStyleSheet());
            }
        }

        mbDataValid = true;

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
    mbNotificationsDisabled = false;

    return mpTextForwarder;
}



SvxTextForwarder* SvxTextEditSourceImpl::GetEditModeTextForwarder()
{
    if( !mpTextForwarder && HasView() )
    {
        SdrOutliner* pEditOutliner = mpView->GetTextEditOutliner();

        if( pEditOutliner )
        {
            mpTextForwarder = new SvxOutlinerForwarder( *pEditOutliner, (mpObject->GetObjInventor() == SdrInventor) && (mpObject->GetObjIdentifier() == OBJ_OUTLINETEXT) );
            mbForwarderIsEditMode = true;
        }
    }

    return mpTextForwarder;
}



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



SvxDrawOutlinerViewForwarder* SvxTextEditSourceImpl::CreateViewForwarder()
{
    if( mpView->GetTextEditOutlinerView() && mpObject )
    {
        // register as listener - need to broadcast state change messages
        mpView->GetTextEditOutliner()->SetNotifyHdl( LINK(this, SvxTextEditSourceImpl, NotifyHdl) );

        SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>( mpObject  );
        if( pTextObj )
        {
            Rectangle aBoundRect( pTextObj->GetCurrentBoundRect() );
            OutlinerView& rOutlView = *mpView->GetTextEditOutlinerView();

            return new SvxDrawOutlinerViewForwarder( rOutlView, aBoundRect.TopLeft() );
        }
    }

    return NULL;
}

SvxEditViewForwarder* SvxTextEditSourceImpl::GetEditViewForwarder( bool bCreate )
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

            if(mpView->SdrBeginTextEdit(mpObject, 0L, 0L, false, nullptr, 0L, false, false))
            {
                SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>( mpObject  );
                if (pTextObj && pTextObj->IsTextEditActive())
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
            mbNeedsUpdate = true;
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
                    mpObject->SetEmptyPresObj(false);
            }
        }
    }
}

void SvxTextEditSourceImpl::lock()
{
    mbIsLocked = true;
    if( mpOutliner )
    {
        const_cast<EditEngine*>(&(mpOutliner->GetEditEngine()))->SetUpdateMode( false );
        mbOldUndoMode = const_cast<EditEngine*>(&(mpOutliner->GetEditEngine()))->IsUndoEnabled();
        const_cast<EditEngine*>(&(mpOutliner->GetEditEngine()))->EnableUndo( false );
    }
}

void SvxTextEditSourceImpl::unlock()
{
    mbIsLocked = false;

    if( mbNeedsUpdate )
    {
        UpdateData();
        mbNeedsUpdate = false;
    }

    if( mpOutliner )
    {
        const_cast<EditEngine*>(&(mpOutliner->GetEditEngine()))->SetUpdateMode( true );
        const_cast<EditEngine*>(&(mpOutliner->GetEditEngine()))->EnableUndo( mbOldUndoMode );
    }
}

bool SvxTextEditSourceImpl::IsValid() const
{
    return mpView && mpWindow;
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
        SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>( mpObject  );
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
        SvxEditViewForwarder* pForwarder = GetEditViewForwarder(false);

        if( pForwarder )
            return pForwarder->LogicToPixel( rPoint, rMapMode );
    }
    else if( IsValid() && mpModel )
    {
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
        SvxEditViewForwarder* pForwarder = GetEditViewForwarder(false);

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
        aPoint2.X() -= maTextOffset.X();
        aPoint2.Y() -= maTextOffset.Y();

        return aPoint2;
    }

    return Point();
}

IMPL_LINK_TYPED(SvxTextEditSourceImpl, NotifyHdl, EENotify&, rNotify, void)
{
    if( !mbNotificationsDisabled )
    {
        std::unique_ptr< SfxHint > aHint( SvxEditSourceHelper::EENotification2Hint( &rNotify) );

        if( aHint.get() )
            Broadcast( *aHint.get() );
    }
}

SvxTextEditSource::SvxTextEditSource( SdrObject* pObject, SdrText* pText )
{
    mpImpl = new SvxTextEditSourceImpl( pObject, pText );
    mpImpl->acquire();
}


SvxTextEditSource::SvxTextEditSource( SdrObject& rObj, SdrText* pText, SdrView& rView, const vcl::Window& rWindow )
{
    mpImpl = new SvxTextEditSourceImpl( rObj, pText, rView, rWindow );
    mpImpl->acquire();
}



SvxTextEditSource::SvxTextEditSource( SvxTextEditSourceImpl* pImpl )
{
    mpImpl = pImpl;
    mpImpl->acquire();
}


SvxTextEditSource::~SvxTextEditSource()
{
    ::SolarMutexGuard aGuard;

    mpImpl->release();
}


SvxEditSource* SvxTextEditSource::Clone() const
{
    return new SvxTextEditSource( mpImpl );
}


SvxTextForwarder* SvxTextEditSource::GetTextForwarder()
{
    return mpImpl->GetTextForwarder();
}


SvxEditViewForwarder* SvxTextEditSource::GetEditViewForwarder( bool bCreate )
{
    return mpImpl->GetEditViewForwarder( bCreate );
}


SvxViewForwarder* SvxTextEditSource::GetViewForwarder()
{
    return this;
}


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

bool SvxTextEditSource::IsValid() const
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
