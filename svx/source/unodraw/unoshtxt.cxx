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

#include <vcl/svapp.hxx>

#include <svx/unoshtxt.hxx>
#include <editeng/unoedhlp.hxx>
#include <svl/lstner.hxx>
#include <rtl/ref.hxx>
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
#include <cell.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <unotools/configmgr.hxx>


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

    SdrObject*                      mpObject;           // TTTT could be reference (?)
    SdrText*                        mpText;
    SdrView*                        mpView;
    VclPtr<const vcl::Window>       mpWindow;
    SdrModel*                       mpModel;            // TTTT probably not needed -> use SdrModel from SdrObject (?)
    std::unique_ptr<SdrOutliner>    mpOutliner;
    std::unique_ptr<SvxOutlinerForwarder> mpTextForwarder;
    std::unique_ptr<SvxDrawOutlinerViewForwarder> mpViewForwarder;    // if non-NULL, use GetViewModeTextForwarder text forwarder
    css::uno::Reference< css::linguistic2::XLinguServiceManager2 > m_xLinguServiceManager;
    Point                           maTextOffset;
    bool                            mbDataValid;
    bool                            mbIsLocked;
    bool                            mbNeedsUpdate;
    bool                            mbOldUndoMode;
    bool                            mbForwarderIsEditMode;      // have to reflect that, since ENDEDIT can happen more often
    bool                            mbShapeIsEditMode;          // only true, if SdrHintKind::BeginEdit was received
    bool                            mbNotificationsDisabled;    // prevent EditEngine/Outliner notifications (e.g. when setting up forwarder)
    bool                            mbNotifyEditOutlinerSet;

    SvxUnoTextRangeBaseVec          mvTextRanges;

    SvxTextForwarder*               GetBackgroundTextForwarder();
    SvxTextForwarder*               GetEditModeTextForwarder();
    std::unique_ptr<SvxDrawOutlinerViewForwarder> CreateViewForwarder();

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
    virtual ~SvxTextEditSourceImpl() override;

    void acquire();
    void release();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    SvxTextForwarder*       GetTextForwarder();
    SvxEditViewForwarder*   GetEditViewForwarder( bool );
    void                    UpdateData();

    void addRange( SvxUnoTextRangeBase* pNewRange );
    void removeRange( SvxUnoTextRangeBase* pOldRange );
    const SvxUnoTextRangeBaseVec& getRanges() const { return mvTextRanges;}

    void                    lock();
    void                    unlock();

    bool                    IsValid() const;

    Point                   LogicToPixel( const Point&, const MapMode& rMapMode );
    Point                   PixelToLogic( const Point&, const MapMode& rMapMode );

    DECL_LINK( NotifyHdl, EENotify&, void );

    virtual void ObjectInDestruction(const SdrObject& rObject) override;

    void                    UpdateOutliner();
};


SvxTextEditSourceImpl::SvxTextEditSourceImpl( SdrObject* pObject, SdrText* pText )
  : maRefCount      ( 0 ),
    mpObject        ( pObject ),
    mpText          ( pText ),
    mpView          ( nullptr ),
    mpWindow        ( nullptr ),
    mpModel         ( pObject ? &pObject->getSdrModelFromSdrObject() : nullptr ), // TTTT should be reference
    mbDataValid     ( false ),
    mbIsLocked      ( false ),
    mbNeedsUpdate   ( false ),
    mbOldUndoMode   ( false ),
    mbForwarderIsEditMode ( false ),
    mbShapeIsEditMode     ( false ),
    mbNotificationsDisabled ( false ),
    mbNotifyEditOutlinerSet ( false )
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
    mpModel         ( &rObject.getSdrModelFromSdrObject() ), // TTTT should be reference
    mbDataValid     ( false ),
    mbIsLocked      ( false ),
    mbNeedsUpdate   ( false ),
    mbOldUndoMode   ( false ),
    mbForwarderIsEditMode ( false ),
    mbShapeIsEditMode     ( true ),
    mbNotificationsDisabled ( false ),
    mbNotifyEditOutlinerSet ( false )
{
    if( !mpText )
    {
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( mpObject );
        if( pTextObj )
            mpText = pTextObj->getText( 0 );
    }

    StartListening( *mpModel );
    StartListening( *mpView );
    mpObject->AddObjectUser( *this );

    // Init edit mode state from shape info (IsTextEditActive())
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
        if( std::find( mvTextRanges.begin(), mvTextRanges.end(), pNewRange ) == mvTextRanges.end() )
            mvTextRanges.push_back( pNewRange );
}


void SvxTextEditSourceImpl::removeRange( SvxUnoTextRangeBase* pOldRange )
{
    if( pOldRange )
        mvTextRanges.erase( std::remove(mvTextRanges.begin(), mvTextRanges.end(), pOldRange), mvTextRanges.end() );
}


void SvxTextEditSourceImpl::acquire()
{
    osl_atomic_increment( &maRefCount );
}


void SvxTextEditSourceImpl::release()
{
    if( ! osl_atomic_decrement( &maRefCount ) )
        delete this;
}

void SvxTextEditSourceImpl::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    // #i105988 keep reference to this object
    rtl::Reference< SvxTextEditSourceImpl > xThis( this );

    if (SfxHintId::Dying == rHint.GetId())
    {
        if (&rBC == mpView)
        {
            mpView = nullptr;
            mpViewForwarder.reset();
        }
    }
    else if (const SvxViewChangedHint* pViewHint = dynamic_cast<const SvxViewChangedHint*>(&rHint))
    {
        Broadcast( *pViewHint );
    }
    else if (const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>(&rHint))
    {
        switch( pSdrHint->GetKind() )
        {
            case SdrHintKind::ObjectChange:
            {
                mbDataValid = false;                        // Text has to be get again

                if( HasView() )
                {
                    // Update maTextOffset, object has changed
                    // Cannot call that here, since TakeTextRect() (called from there)
                    // changes outliner content.
                    // UpdateOutliner();

                    // Broadcast object changes, as they might change visible attributes
                    SvxViewChangedHint aHint;
                    Broadcast( aHint );
                }
                break;
            }

            case SdrHintKind::BeginEdit:
                if( mpObject == pSdrHint->GetObject() )
                {
                    // Once SdrHintKind::BeginEdit is broadcast, each EditSource of
                    // AccessibleCell will handle it here and call below:
                    // mpView->GetTextEditOutliner()->SetNotifyHdl(), which
                    // will replace the Notifier for current editable cell. It
                    // is totally wrong. So add check here to avoid the
                    // incorrect replacement of notifier.

                    // Currently it only happens on the editsource of
                    // AccessibleCell
                    if (mpObject && mpText)
                    {
                        sdr::table::SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( mpObject );
                        if(pTableObj)
                        {
                            const sdr::table::CellRef& xCell = pTableObj->getActiveCell();
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
                        mpTextForwarder.reset();
                    }

                    // register as listener - need to broadcast state change messages
                    if( mpView && mpView->GetTextEditOutliner() )
                    {
                        mpView->GetTextEditOutliner()->SetNotifyHdl( LINK(this, SvxTextEditSourceImpl, NotifyHdl) );
                        mbNotifyEditOutlinerSet = true;
                    }

                    // Only now we're really in edit mode
                    mbShapeIsEditMode = true;

                    Broadcast( *pSdrHint );
                }
                break;

            case SdrHintKind::EndEdit:
                if( mpObject == pSdrHint->GetObject() )
                {
                    Broadcast( *pSdrHint );

                    // We're no longer in edit mode
                    mbShapeIsEditMode = false;

                    // remove as listener - outliner might outlive ourselves
                    if( mpView && mpView->GetTextEditOutliner() )
                    {
                        mpView->GetTextEditOutliner()->SetNotifyHdl( Link<EENotify&,void>() );
                        mbNotifyEditOutlinerSet = false;
                    }

                    // destroy view forwarder, OutlinerView no longer
                    // valid (no need for UpdateData(), it's been
                    // synched on SdrEndTextEdit)
                    mpViewForwarder.reset();

                    // Invalidate text forwarder, we might
                    // not be called again before entering edit mode a
                    // second time! Then, the old outliner might be
                    // invalid.
                    if( mbForwarderIsEditMode )
                    {
                        mbForwarderIsEditMode = false;
                        mpTextForwarder.reset();
                    }
                }
                break;

            case SdrHintKind::ModelCleared:
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
    mpObject = nullptr;
    dispose();
    Broadcast( SfxHint( SfxHintId::Dying ) );
}

/* unregister at all objects and set all references to 0 */
void SvxTextEditSourceImpl::dispose()
{
    mpTextForwarder.reset();
    mpViewForwarder.reset();

    if( mpOutliner )
    {
        if( mpModel )
        {
            mpModel->disposeOutliner( std::move(mpOutliner) );
        }
        else
        {
            mpOutliner.reset();
        }
    }

    if( mpModel )
    {
        EndListening( *mpModel );
        mpModel = nullptr;
    }

    if( mpView )
    {
        // remove as listener - outliner might outlive ourselves
        if (mbNotifyEditOutlinerSet && mpView->GetTextEditOutliner())
        {
            mpView->GetTextEditOutliner()->SetNotifyHdl(Link<EENotify&,void>());
            mbNotifyEditOutlinerSet = false;
        }
        EndListening( *mpView );
        mpView = nullptr;
    }

    if( mpObject )
    {
        mpObject->RemoveObjectUser( *this );
        mpObject = nullptr;
    }
    mpWindow = nullptr;
}


void SvxTextEditSourceImpl::SetupOutliner()
{
    // only for UAA edit source: setup outliner equivalently as in
    // SdrTextObj::Paint(), such that formatting equals screen
    // layout
    if( mpObject && mpOutliner )
    {
        SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>( mpObject  );
        tools::Rectangle aPaintRect;
        if( pTextObj )
        {
            tools::Rectangle aBoundRect( pTextObj->GetCurrentBoundRect() );
            pTextObj->SetupOutlinerFormatting( *mpOutliner, aPaintRect );

            // calc text offset from shape anchor
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
        tools::Rectangle aPaintRect;
        if( pTextObj )
        {
            tools::Rectangle aBoundRect( pTextObj->GetCurrentBoundRect() );
            pTextObj->UpdateOutlinerFormatting( *mpOutliner, aPaintRect );

            // calc text offset from shape anchor
            maTextOffset = aPaintRect.TopLeft() - aBoundRect.TopLeft();
        }
    }
}


SvxTextForwarder* SvxTextEditSourceImpl::GetBackgroundTextForwarder()
{
    bool bCreated = false;

    // prevent EE/Outliner notifications during setup
    mbNotificationsDisabled = true;

    if (!mpTextForwarder)
    {
        if( mpOutliner == nullptr )
        {
            SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>( mpObject  );
            OutlinerMode nOutlMode = OutlinerMode::TextObject;
            if( pTextObj && pTextObj->IsTextFrame() && pTextObj->GetTextKind() == OBJ_OUTLINETEXT )
                nOutlMode = OutlinerMode::OutlineObject;

            mpOutliner = mpModel->createOutliner( nOutlMode );

            // Do the setup after outliner creation, would be useless otherwise
            if( HasView() )
            {
                // Setup outliner _before_ filling it
                SetupOutliner();
            }

            mpOutliner->SetTextObjNoInit( pTextObj );
            if( mbIsLocked )
            {
                const_cast<EditEngine*>(&(mpOutliner->GetEditEngine()))->SetUpdateMode( false );
                mbOldUndoMode = const_cast<EditEngine*>(&(mpOutliner->GetEditEngine()))->IsUndoEnabled();
                const_cast<EditEngine*>(&(mpOutliner->GetEditEngine()))->EnableUndo( false );
            }

            if (!utl::ConfigManager::IsFuzzing())
            {
                if ( !m_xLinguServiceManager.is() )
                {
                    css::uno::Reference< css::uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
                    m_xLinguServiceManager.set(css::linguistic2::LinguServiceManager::create(xContext));
                }

                css::uno::Reference< css::linguistic2::XHyphenator > xHyphenator( m_xLinguServiceManager->getHyphenator(), css::uno::UNO_QUERY );
                if( xHyphenator.is() )
                    mpOutliner->SetHyphenator( xHyphenator );
            }
        }


        mpTextForwarder.reset(new SvxOutlinerForwarder( *mpOutliner, (mpObject->GetObjInventor() == SdrInventor::Default) && (mpObject->GetObjIdentifier() == OBJ_OUTLINETEXT) ));
        // delay listener subscription and UAA initialization until Outliner is fully setup
        bCreated = true;

        mbForwarderIsEditMode = false;
        mbDataValid = false;
    }

    if( mpObject && mpText && !mbDataValid && mpObject->IsInserted() && mpObject->getSdrPageFromSdrObject() )
    {
        mpTextForwarder->flushCache();

        OutlinerParaObject* pOutlinerParaObject = nullptr;
        SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>( mpObject  );
        if( pTextObj && pTextObj->getActiveText() == mpText )
            pOutlinerParaObject = pTextObj->GetEditOutlinerParaObject().release(); // Get the OutlinerParaObject if text edit is active
        bool bOwnParaObj(false);

        if( pOutlinerParaObject )
            bOwnParaObj = true; // text edit active
        else
            pOutlinerParaObject = mpText->GetOutlinerParaObject();

        if( pOutlinerParaObject && ( bOwnParaObj || !mpObject->IsEmptyPresObj() || mpObject->getSdrPageFromSdrObject()->IsMasterPage() ) )
        {
            mpOutliner->SetText( *pOutlinerParaObject );

            // put text to object and set EmptyPresObj to FALSE
            if( mpText && bOwnParaObj && mpObject->IsEmptyPresObj() && pTextObj->IsReallyEdited() )
            {
                mpObject->SetEmptyPresObj( false );
                static_cast< SdrTextObj* >( mpObject)->NbcSetOutlinerParaObjectForText( std::unique_ptr<OutlinerParaObject>(pOutlinerParaObject), mpText );

                // #i103982# Here, due to mpObject->NbcSetOutlinerParaObjectForText, we LOSE ownership of the
                // OPO, so do NOT delete it when leaving this method (!)
                bOwnParaObj = false;
            }
        }
        else
        {
            bool bVertical = pOutlinerParaObject && pOutlinerParaObject->IsVertical();

            // set objects style sheet on empty outliner
            SfxStyleSheetPool* pPool = static_cast<SfxStyleSheetPool*>(mpObject->getSdrModelFromSdrObject().GetStyleSheetPool());
            if( pPool )
                mpOutliner->SetStyleSheetPool( pPool );

            SfxStyleSheet* pStyleSheet = mpObject->getSdrPageFromSdrObject()->GetTextStyleSheetForObject( mpObject );
            if( pStyleSheet )
                mpOutliner->SetStyleSheet( 0, pStyleSheet );

            if( bVertical )
                mpOutliner->SetVertical( true, pOutlinerParaObject->IsTopToBottom());
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

    // prevent EE/Outliner notifications during setup
    mbNotificationsDisabled = false;

    return mpTextForwarder.get();
}


SvxTextForwarder* SvxTextEditSourceImpl::GetEditModeTextForwarder()
{
    if( !mpTextForwarder && HasView() )
    {
        SdrOutliner* pEditOutliner = mpView->GetTextEditOutliner();

        if( pEditOutliner )
        {
            mpTextForwarder.reset(new SvxOutlinerForwarder( *pEditOutliner, (mpObject->GetObjInventor() == SdrInventor::Default) && (mpObject->GetObjIdentifier() == OBJ_OUTLINETEXT) ));
            mbForwarderIsEditMode = true;
        }
    }

    return mpTextForwarder.get();
}


SvxTextForwarder* SvxTextEditSourceImpl::GetTextForwarder()
{
    if( mpObject == nullptr )
        return nullptr;

    if( mpModel == nullptr )
        mpModel = &mpObject->getSdrModelFromSdrObject();

    // distinguish the cases
    // a) connected to view, maybe edit mode is active, can work directly on the EditOutliner
    // b) background Outliner, reflect changes into ParaOutlinerObject (this is exactly the old UNO code)
    if( HasView() )
    {
        if( IsEditMode() != mbForwarderIsEditMode )
        {
            // forwarder mismatch - create new
            mpTextForwarder.reset();
        }

        if( IsEditMode() )
            return GetEditModeTextForwarder();
        else
            return GetBackgroundTextForwarder();
    }
    else
    {
        // tdf#123470 if the text edit mode of the shape is active, then we
        // cannot trust a previously cached TextForwarder state as the text may
        // be out of date, so force a refetch in that case.
        if (IsEditMode())
        {
            assert(!mbForwarderIsEditMode); // because without a view there is no other option except !mbForwarderIsEditMode
            bool bTextEditActive = false;
            SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>(mpObject);
            // similar to the GetBackgroundTextForwarder check, see if the text edit is active
            if (pTextObj && pTextObj->getActiveText() == mpText && pTextObj->GetEditOutlinerParaObject())
                bTextEditActive = true; // text edit active
            if (bTextEditActive)
                mbDataValid = false;
        }

        return GetBackgroundTextForwarder();
    }
}

std::unique_ptr<SvxDrawOutlinerViewForwarder> SvxTextEditSourceImpl::CreateViewForwarder()
{
    if( mpView->GetTextEditOutlinerView() && mpObject )
    {
        // register as listener - need to broadcast state change messages
        mpView->GetTextEditOutliner()->SetNotifyHdl( LINK(this, SvxTextEditSourceImpl, NotifyHdl) );
        mbNotifyEditOutlinerSet = true;

        SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>( mpObject  );
        if( pTextObj )
        {
            tools::Rectangle aBoundRect( pTextObj->GetCurrentBoundRect() );
            OutlinerView& rOutlView = *mpView->GetTextEditOutlinerView();

            return std::unique_ptr<SvxDrawOutlinerViewForwarder>(new SvxDrawOutlinerViewForwarder( rOutlView, aBoundRect.TopLeft() ));
        }
    }

    return nullptr;
}

SvxEditViewForwarder* SvxTextEditSourceImpl::GetEditViewForwarder( bool bCreate )
{
    if( mpObject == nullptr )
        return nullptr;

    if( mpModel == nullptr )
        mpModel = &mpObject->getSdrModelFromSdrObject();

    // shall we delete?
    if( mpViewForwarder )
    {
        if( !IsEditMode() )
        {
            // destroy all forwarders (no need for UpdateData(),
            // it's been synched on SdrEndTextEdit)
            mpViewForwarder.reset();
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

            mpTextForwarder.reset();

            // enter edit mode
            mpView->SdrEndTextEdit();

            if(mpView->SdrBeginTextEdit(mpObject))
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

    return mpViewForwarder.get();
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
            if( mpOutliner && mpObject && mpText )
            {
                SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( mpObject );
                if( pTextObj )
                {
                    if( mpOutliner->GetParagraphCount() != 1 || mpOutliner->GetEditEngine().GetTextLen( 0 ) )
                    {
                        if( mpOutliner->GetParagraphCount() > 1 )
                        {
                            if (pTextObj->IsTextFrame() && pTextObj->GetTextKind() == OBJ_TITLETEXT)
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
                        pTextObj->NbcSetOutlinerParaObjectForText( nullptr,mpText );
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

Point SvxTextEditSourceImpl::LogicToPixel( const Point& rPoint, const MapMode& rMapMode )
{
    // The responsibilities of ViewForwarder happen to be
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
        aPoint1.AdjustX(maTextOffset.X() );
        aPoint1.AdjustY(maTextOffset.Y() );

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
    // The responsibilities of ViewForwarder happen to be
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
        aPoint2.AdjustX( -(maTextOffset.X()) );
        aPoint2.AdjustY( -(maTextOffset.Y()) );

        return aPoint2;
    }

    return Point();
}

IMPL_LINK(SvxTextEditSourceImpl, NotifyHdl, EENotify&, rNotify, void)
{
    if( !mbNotificationsDisabled )
    {
        std::unique_ptr< SfxHint > aHint( SvxEditSourceHelper::EENotification2Hint( &rNotify) );

        if (aHint)
            Broadcast(*aHint);
    }
}

SvxTextEditSource::SvxTextEditSource( SdrObject* pObject, SdrText* pText )
{
    mpImpl = new SvxTextEditSourceImpl( pObject, pText );
}


SvxTextEditSource::SvxTextEditSource( SdrObject& rObj, SdrText* pText, SdrView& rView, const vcl::Window& rWindow )
{
    mpImpl = new SvxTextEditSourceImpl( rObj, pText, rView, rWindow );
}


SvxTextEditSource::SvxTextEditSource( SvxTextEditSourceImpl* pImpl )
{
    mpImpl = pImpl;
}


SvxTextEditSource::~SvxTextEditSource()
{
    ::SolarMutexGuard aGuard;
    mpImpl.clear();
}


std::unique_ptr<SvxEditSource> SvxTextEditSource::Clone() const
{
    return std::unique_ptr<SvxEditSource>(new SvxTextEditSource( mpImpl.get() ));
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

const SvxUnoTextRangeBaseVec& SvxTextEditSource::getRanges() const
{
    return mpImpl->getRanges();
}

void SvxTextEditSource::UpdateOutliner()
{
    mpImpl->UpdateOutliner();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
