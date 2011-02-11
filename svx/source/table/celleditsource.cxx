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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <com/sun/star/linguistic2/XLinguServiceManager.hpp>

#include <rtl/ref.hxx>
#include <osl/mutex.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/processfactory.hxx>
#include <svl/lstner.hxx>
#include <svl/hint.hxx>
#include <svl/style.hxx>

#include "celleditsource.hxx"
#include "cell.hxx"
#include "svx/svdmodel.hxx"
#include "svx/svdoutl.hxx"
#include "svx/svdobj.hxx"
#include "editeng/unoedhlp.hxx"
#include "svx/svdview.hxx"
#include "svx/svdetc.hxx"
#include "editeng/outliner.hxx"
#include "editeng/unoforou.hxx"
#include "editeng/unoviwou.hxx"
#include "editeng/outlobj.hxx"
#include "svx/svdotext.hxx"
#include "svx/svdpage.hxx"
#include "editeng/editeng.hxx"
#include "editeng/unotext.hxx"
#include "svx/sdrpaintwindow.hxx"

//------------------------------------------------------------------------

using ::rtl::OUString;
using namespace ::osl;
using namespace ::vos;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::lang;

//------------------------------------------------------------------------

namespace sdr { namespace table {

//------------------------------------------------------------------------
// CellEditSourceImpl
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
class CellEditSourceImpl : public SfxListener, public SfxBroadcaster
{
private:
    oslInterlockedCount             maRefCount;

    SdrView*                        mpView;
    const Window*                   mpWindow;
    SdrModel*                       mpModel;
    SdrOutliner*                    mpOutliner;
    SvxOutlinerForwarder*           mpTextForwarder;
    SvxDrawOutlinerViewForwarder*   mpViewForwarder;
    Reference< ::com::sun::star::linguistic2::XLinguServiceManager > mxLinguServiceManager;
    Point                           maTextOffset;
    bool                            mbDataValid;
    bool                            mbDisposed;
    bool                            mbIsLocked;
    bool                            mbNeedsUpdate;
    bool                            mbOldUndoMode;
    bool                            mbForwarderIsEditMode;      // have to reflect that, since ENDEDIT can happen more often
    bool                            mbShapeIsEditMode;          // #104157# only true, if HINT_BEGEDIT was received
    bool                            mbNotificationsDisabled;    // prevent EditEngine/Outliner notifications (e.g. when setting up forwarder)

    CellRef                         mxCell;
    SvxUnoTextRangeBaseList         maTextRanges;

    SvxTextForwarder*               GetBackgroundTextForwarder();
    SvxTextForwarder*               GetEditModeTextForwarder();
    SvxDrawOutlinerViewForwarder*   CreateViewForwarder();

    void                            SetupOutliner();
    void                            UpdateOutliner();

    bool                            HasView() const { return mpView != 0; }
    bool                            IsEditMode() const { return mxCell->IsTextEditActive(); };
    void                            dispose();

public:
    CellEditSourceImpl( const CellRef& xCell );
    CellEditSourceImpl( const CellRef& xCell, SdrView& rView, const Window& rWindow );
    ~CellEditSourceImpl();

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

    void                    lock();
    void                    unlock();

    sal_Bool                    IsValid() const;

    Rectangle               GetVisArea();
    Point                   LogicToPixel( const Point&, const MapMode& rMapMode );
    Point                   PixelToLogic( const Point&, const MapMode& rMapMode );

    DECL_LINK( NotifyHdl, EENotify* );

    void ChangeModel( SdrModel* pNewModel );
};

//------------------------------------------------------------------------

CellEditSourceImpl::CellEditSourceImpl( const CellRef& xCell )
  : maRefCount      ( 0 ),
    mpView          ( NULL ),
    mpWindow        ( NULL ),
    mpModel         ( NULL ),
    mpOutliner      ( NULL ),
    mpTextForwarder ( NULL ),
    mpViewForwarder ( NULL ),
    mbDataValid     ( false ),
    mbDisposed      ( false ),
    mbIsLocked      ( false ),
    mbNeedsUpdate   ( false ),
    mbOldUndoMode   ( false ),
    mbForwarderIsEditMode ( false ),
    mbShapeIsEditMode     ( false ),
    mbNotificationsDisabled ( false ),
    mxCell( xCell )
{
}

//------------------------------------------------------------------------

CellEditSourceImpl::CellEditSourceImpl( const CellRef& xCell, SdrView& rView, const Window& rWindow )
  : maRefCount      ( 0 ),
    mpView          ( &rView ),
    mpWindow        ( &rWindow ),
    mpModel         ( NULL ),
    mpOutliner      ( NULL ),
    mpTextForwarder ( NULL ),
    mpViewForwarder ( NULL ),
    mbDataValid     ( false ),
    mbDisposed      ( false ),
    mbIsLocked      ( false ),
    mbNeedsUpdate   ( false ),
    mbOldUndoMode   ( false ),
    mbForwarderIsEditMode ( false ),
    mbShapeIsEditMode     ( true ),
    mbNotificationsDisabled ( false ),
    mxCell( xCell )
{
    if( mpView )
        StartListening( *mpView );

    // #104157# Init edit mode state from shape info (IsTextEditActive())
    mbShapeIsEditMode = IsEditMode();
}

//------------------------------------------------------------------------

CellEditSourceImpl::~CellEditSourceImpl()
{
    DBG_ASSERT( mbIsLocked == sal_False, "CellEditSourceImpl::~CellEditSourceImpl(), was not unlocked before dispose!" );
    dispose();
}

//------------------------------------------------------------------------

void CellEditSourceImpl::addRange( SvxUnoTextRangeBase* pNewRange )
{
    if( pNewRange )
        if( std::find( maTextRanges.begin(), maTextRanges.end(), pNewRange ) == maTextRanges.end() )
            maTextRanges.push_back( pNewRange );
}

//------------------------------------------------------------------------

void CellEditSourceImpl::removeRange( SvxUnoTextRangeBase* pOldRange )
{
    if( pOldRange )
        maTextRanges.remove( pOldRange );
}

//------------------------------------------------------------------------

const SvxUnoTextRangeBaseList& CellEditSourceImpl::getRanges() const
{
    return maTextRanges;
}

//------------------------------------------------------------------------

void SAL_CALL CellEditSourceImpl::acquire()
{
    osl_incrementInterlockedCount( &maRefCount );
}

//------------------------------------------------------------------------

void SAL_CALL CellEditSourceImpl::release()
{
    if( ! osl_decrementInterlockedCount( &maRefCount ) )
        delete this;
}

void CellEditSourceImpl::ChangeModel( SdrModel* pNewModel )
{
    if( mpModel != pNewModel )
    {
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
        mxLinguServiceManager.clear();

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
    }
}

//------------------------------------------------------------------------

void CellEditSourceImpl::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
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
/* todo
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
                        mpView->GetTextEditOutliner()->SetNotifyHdl( LINK(this, CellEditSourceImpl, NotifyHdl) );

                    // #104157# Only now we're really in edit mode
                    mbShapeIsEditMode = sal_True;

                    Broadcast( *pSdrHint );
                }
*/
                break;

            case HINT_ENDEDIT:
/* todo
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
*/
                break;

            case HINT_MODELCLEARED:
                dispose();
                break;
            default:
                break;
        }
    }
}

/* unregister at all objects and set all references to 0 */
void CellEditSourceImpl::dispose()
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

    if( mpView )
    {
        EndListening( *mpView );
        mpView = 0;
    }

    mpModel = 0;
    mpWindow = 0;
}

//------------------------------------------------------------------------

void CellEditSourceImpl::SetupOutliner()
{
    // #101029#
    // only for UAA edit source: setup outliner equivalently as in
    // SdrTextObj::Paint(), such that formatting equals screen
    // layout
/* todo
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
*/
}

//------------------------------------------------------------------------

void CellEditSourceImpl::UpdateOutliner()
{
    // #104157#
    // only for UAA edit source: update outliner equivalently as in
    // SdrTextObj::Paint(), such that formatting equals screen
    // layout
/* todo
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
*/
}

//------------------------------------------------------------------------


SvxTextForwarder* CellEditSourceImpl::GetBackgroundTextForwarder()
{
    sal_Bool bCreated = sal_False;

    // #99840#: prevent EE/Outliner notifications during setup
    mbNotificationsDisabled = true;

    if (!mpTextForwarder)
    {
        if( mpOutliner == NULL )
        {
            mpOutliner = mpModel->createOutliner( OUTLINERMODE_TEXTOBJECT );

            // #109151# Do the setup after outliner creation, would be useless otherwise
            if( HasView() )
            {
                // #101029#, #104157# Setup outliner _before_ filling it
                SetupOutliner();
            }

// todo?            mpOutliner->SetTextObjNoInit( pTextObj );

            if( mbIsLocked )
            {
                ((EditEngine*)&(mpOutliner->GetEditEngine()))->SetUpdateMode( sal_False );
                mbOldUndoMode = ((EditEngine*)&(mpOutliner->GetEditEngine()))->IsUndoEnabled();
                ((EditEngine*)&(mpOutliner->GetEditEngine()))->EnableUndo( sal_False );
            }

            if ( !mxLinguServiceManager.is() )
            {
                Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
                mxLinguServiceManager = Reference< XLinguServiceManager >(
                    xMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.linguistic2.LinguServiceManager" ))), UNO_QUERY );
            }

            if ( mxLinguServiceManager.is() )
            {
                Reference< XHyphenator > xHyphenator( mxLinguServiceManager->getHyphenator(), UNO_QUERY );
                if( xHyphenator.is() )
                    mpOutliner->SetHyphenator( xHyphenator );
            }
        }

        mpTextForwarder = new SvxOutlinerForwarder( *mpOutliner );

        // delay listener subscription and UAA initialization until Outliner is fully setup
        bCreated = true;
        mbForwarderIsEditMode = false;
    }

    if( !mbDataValid )
    {
        mpTextForwarder->flushCache();

        OutlinerParaObject* pOutlinerParaObject = NULL;
        bool bTextEditActive = false;

        pOutlinerParaObject = mxCell->GetEditOutlinerParaObject(); // Get the OutlinerParaObject if text edit is active

        if( pOutlinerParaObject )
            bTextEditActive = true; // text edit active
        else
            pOutlinerParaObject = mxCell->GetOutlinerParaObject();

        if( pOutlinerParaObject )
        {
            mpOutliner->SetText( *pOutlinerParaObject );
        }
        else
        {
            bool bVertical = false; // todo?

            // set objects style sheet on empty outliner
            SfxStyleSheetPool* pPool = mxCell->GetStyleSheetPool();
            if( pPool )
                mpOutliner->SetStyleSheetPool( pPool );

            SfxStyleSheet* pStyleSheet = mxCell->GetStyleSheet();
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

                if(mxCell->GetStyleSheet())
                    mpOutliner->SetStyleSheet( 0, mxCell->GetStyleSheet());
            }
        }

        if( bTextEditActive )
            delete pOutlinerParaObject;

        mbDataValid = true;
    }

    if( bCreated && mpOutliner && HasView() )
    {
        // register as listener - need to broadcast state change messages
        // registration delayed until outliner is completely set up
        mpOutliner->SetNotifyHdl( LINK(this, CellEditSourceImpl, NotifyHdl) );
    }

    // #99840#: prevent EE/Outliner notifications during setup
    mbNotificationsDisabled = false;

    return mpTextForwarder;
}

//------------------------------------------------------------------------

SvxTextForwarder* CellEditSourceImpl::GetEditModeTextForwarder()
{
    if( !mpTextForwarder && HasView() )
    {
        SdrOutliner* pEditOutliner = mpView->GetTextEditOutliner();

        if( pEditOutliner )
        {
            mpTextForwarder = new SvxOutlinerForwarder( *pEditOutliner );
            mbForwarderIsEditMode = true;
        }
    }

    return mpTextForwarder;
}

//------------------------------------------------------------------------

SvxTextForwarder* CellEditSourceImpl::GetTextForwarder()
{
    if( mbDisposed )
        return NULL;

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

SvxDrawOutlinerViewForwarder* CellEditSourceImpl::CreateViewForwarder()
{
    if( mpView->GetTextEditOutlinerView() )
    {
        // register as listener - need to broadcast state change messages
        mpView->GetTextEditOutliner()->SetNotifyHdl( LINK(this, CellEditSourceImpl, NotifyHdl) );

        Rectangle aBoundRect( mxCell->GetCurrentBoundRect() );
        OutlinerView& rOutlView = *mpView->GetTextEditOutlinerView();

        return new SvxDrawOutlinerViewForwarder( rOutlView, aBoundRect.TopLeft() );
    }

    return NULL;
}

SvxEditViewForwarder* CellEditSourceImpl::GetEditViewForwarder( sal_Bool bCreate )
{
    if( mbDisposed )
        return NULL;

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

/* todo
            if(mpView->SdrBeginTextEdit(mpObject, 0L, 0L, sal_False, (SdrOutliner*)0L, 0L, sal_False, sal_False))
            {
                if( mxCell->IsTextEditActive() )
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
*/
        }
    }

    return mpViewForwarder;
}

//------------------------------------------------------------------------

void CellEditSourceImpl::UpdateData()
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
            if( mpOutliner && !mbDisposed )
            {
                if( mpOutliner->GetParagraphCount() != 1 || mpOutliner->GetEditEngine().GetTextLen( 0 ) )
                {
                    mxCell->SetOutlinerParaObject( mpOutliner->CreateParaObject() );
                }
                else
                {
                    mxCell->SetOutlinerParaObject( NULL );
                }
            }
        }
    }
}

void CellEditSourceImpl::lock()
{
    mbIsLocked = true;
    if( mpOutliner )
    {
        ((EditEngine*)&(mpOutliner->GetEditEngine()))->SetUpdateMode( sal_False );
        mbOldUndoMode = ((EditEngine*)&(mpOutliner->GetEditEngine()))->IsUndoEnabled();
        ((EditEngine*)&(mpOutliner->GetEditEngine()))->EnableUndo( sal_False );
    }
}

void CellEditSourceImpl::unlock()
{
    mbIsLocked = false;

    if( mbNeedsUpdate )
    {
        UpdateData();
        mbNeedsUpdate = false;
    }

    if( mpOutliner )
    {
        ((EditEngine*)&(mpOutliner->GetEditEngine()))->SetUpdateMode( sal_True );
        ((EditEngine*)&(mpOutliner->GetEditEngine()))->EnableUndo( mbOldUndoMode );
    }
}

sal_Bool CellEditSourceImpl::IsValid() const
{
    return mpView && mpWindow ? sal_True : sal_False;
}

Rectangle CellEditSourceImpl::GetVisArea()
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
        Rectangle aAnchorRect;
        mxCell->TakeTextAnchorRect( aAnchorRect );
        aVisArea.Move( -aAnchorRect.Left(), -aAnchorRect.Top() );

        MapMode aMapMode(mpWindow->GetMapMode());
        aMapMode.SetOrigin(Point());
        return mpWindow->LogicToPixel( aVisArea, aMapMode );
    }

    return Rectangle();
}

Point CellEditSourceImpl::LogicToPixel( const Point& rPoint, const MapMode& rMapMode )
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

Point CellEditSourceImpl::PixelToLogic( const Point& rPoint, const MapMode& rMapMode )
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

IMPL_LINK(CellEditSourceImpl, NotifyHdl, EENotify*, aNotify)
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
// CellEditSource
// --------------------------------------------------------------------

CellEditSource::CellEditSource( const CellRef& xCell )
{
    mpImpl = new CellEditSourceImpl( xCell );
    mpImpl->acquire();
}

// --------------------------------------------------------------------
CellEditSource::CellEditSource( const CellRef& xCell, SdrView& rView, const Window& rWindow )
{
    mpImpl = new CellEditSourceImpl( xCell, rView, rWindow );
    mpImpl->acquire();
}

// --------------------------------------------------------------------

CellEditSource::CellEditSource( CellEditSourceImpl* pImpl )
{
    mpImpl = pImpl;
    mpImpl->acquire();
}

//------------------------------------------------------------------------
CellEditSource::~CellEditSource()
{
    OGuard aGuard( Application::GetSolarMutex() );
    mpImpl->release();
}

//------------------------------------------------------------------------
SvxEditSource* CellEditSource::Clone() const
{
    return new CellEditSource( mpImpl );
}

//------------------------------------------------------------------------
SvxTextForwarder* CellEditSource::GetTextForwarder()
{
    return mpImpl->GetTextForwarder();
}

//------------------------------------------------------------------------
SvxEditViewForwarder* CellEditSource::GetEditViewForwarder( sal_Bool bCreate )
{
    return mpImpl->GetEditViewForwarder( bCreate );
}

//------------------------------------------------------------------------

SvxViewForwarder* CellEditSource::GetViewForwarder()
{
    return this;
}

//------------------------------------------------------------------------

void CellEditSource::UpdateData()
{
    mpImpl->UpdateData();
}

//------------------------------------------------------------------------

SfxBroadcaster& CellEditSource::GetBroadcaster() const
{
    return *mpImpl;
}

//------------------------------------------------------------------------

void CellEditSource::lock()
{
    mpImpl->lock();
}

//------------------------------------------------------------------------

void CellEditSource::unlock()
{
    mpImpl->unlock();
}

//------------------------------------------------------------------------

sal_Bool CellEditSource::IsValid() const
{
    return mpImpl->IsValid();
}

//------------------------------------------------------------------------

Rectangle CellEditSource::GetVisArea() const
{
    return mpImpl->GetVisArea();
}

//------------------------------------------------------------------------

Point CellEditSource::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
{
    return mpImpl->LogicToPixel( rPoint, rMapMode );
}

//------------------------------------------------------------------------

Point CellEditSource::PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const
{
    return mpImpl->PixelToLogic( rPoint, rMapMode );
}

//------------------------------------------------------------------------

void CellEditSource::addRange( SvxUnoTextRangeBase* pNewRange )
{
    mpImpl->addRange( pNewRange );
}

//------------------------------------------------------------------------

void CellEditSource::removeRange( SvxUnoTextRangeBase* pOldRange )
{
    mpImpl->removeRange( pOldRange );
}

//------------------------------------------------------------------------

const SvxUnoTextRangeBaseList& CellEditSource::getRanges() const
{
    return mpImpl->getRanges();
}

//------------------------------------------------------------------------

void CellEditSource::ChangeModel( SdrModel* pNewModel )
{
    mpImpl->ChangeModel( pNewModel );
}

//------------------------------------------------------------------------

} }
