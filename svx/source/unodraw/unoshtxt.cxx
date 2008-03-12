/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoshtxt.cxx,v $
 *
 *  $Revision: 1.59 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:13:29 $
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
#include "precompiled_svx.hxx"

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include <svx/unoshtxt.hxx>
#include <unoedhlp.hxx>

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _SFXHINT_HXX //autogen
#include <svtools/hint.hxx>
#endif
#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDOUTL_HXX //autogen
#include <svx/svdoutl.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _SVDVIEW_HXX
#include <svx/svdview.hxx>
#endif
#ifndef _SVDETC_HXX
#include <svx/svdetc.hxx>
#endif
#ifndef _OUTLINER_HXX
#include <svx/outliner.hxx>
#endif
#ifndef _SVX_UNOFOROU_HXX
#include <svx/unoforou.hxx>
#endif
#ifndef _SVX_UNOVIWOU_HXX
#include <svx/unoviwou.hxx>
#endif
#ifndef _OUTLOBJ_HXX
#include <svx/outlobj.hxx>
#endif
#include <svx/svdotext.hxx>
#include <svx/svdpage.hxx>
#include <svx/editeng.hxx>
#ifndef _EDITOBJ_HXX
#include <svx/editobj.hxx>
#endif

#include <svx/unotext.hxx>
#ifndef _COM_SUN_STAR_LINGUISTIC2_XLINGUSERVICEMANAGER_HPP_
#include <com/sun/star/linguistic2/XLinguServiceManager.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SDRPAINTWINDOW_HXX
#include <sdrpaintwindow.hxx>
#endif

using namespace ::osl;
using namespace ::vos;
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
    css::uno::Reference< css::linguistic2::XLinguServiceManager > m_xLinguServiceManager;
    Point                           maTextOffset;
    BOOL                            mbDataValid;
    BOOL                            mbDestroyed;
    BOOL                            mbIsLocked;
    BOOL                            mbNeedsUpdate;
    BOOL                            mbOldUndoMode;
    BOOL                            mbForwarderIsEditMode;      // have to reflect that, since ENDEDIT can happen more often
    BOOL                            mbShapeIsEditMode;          // #104157# only true, if HINT_BEGEDIT was received
    BOOL                            mbNotificationsDisabled;    // prevent EditEngine/Outliner notifications (e.g. when setting up forwarder)

    XInterface*                     mpOwner;
    SvxUnoTextRangeBaseList         maTextRanges;

    SvxTextForwarder*               GetBackgroundTextForwarder();
    SvxTextForwarder*               GetEditModeTextForwarder();
    SvxDrawOutlinerViewForwarder*   CreateViewForwarder();

    void                            SetupOutliner();
    void                            UpdateOutliner();

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

    BOOL                    IsValid() const;

    Rectangle               GetVisArea();
    Point                   LogicToPixel( const Point&, const MapMode& rMapMode );
    Point                   PixelToLogic( const Point&, const MapMode& rMapMode );

    DECL_LINK( NotifyHdl, EENotify* );

    virtual void ObjectInDestruction(const SdrObject& rObject);

    void ChangeModel( SdrModel* pNewModel );
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
    mbDataValid     ( FALSE ),
    mbDestroyed     ( FALSE ),
    mbIsLocked      ( FALSE ),
    mbNeedsUpdate   ( FALSE ),
    mbOldUndoMode   ( FALSE ),
    mbForwarderIsEditMode ( FALSE ),
    mbShapeIsEditMode     ( FALSE ),
    mbNotificationsDisabled ( FALSE ),
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
    mbDataValid     ( FALSE ),
    mbDestroyed     ( FALSE ),
    mbIsLocked      ( FALSE ),
    mbNeedsUpdate   ( FALSE ),
    mbOldUndoMode   ( FALSE ),
    mbForwarderIsEditMode ( FALSE ),
    mbShapeIsEditMode     ( TRUE ),
    mbNotificationsDisabled ( FALSE ),
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
    osl_incrementInterlockedCount( &maRefCount );
}

//------------------------------------------------------------------------

void SAL_CALL SvxTextEditSourceImpl::release()
{
    if( ! osl_decrementInterlockedCount( &maRefCount ) )
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
                mbDataValid = FALSE;                        // Text muss neu geholt werden

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

// -
            if ( !m_xLinguServiceManager.is() )
            {
                css::uno::Reference< css::lang::XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
                m_xLinguServiceManager = css::uno::Reference< css::linguistic2::XLinguServiceManager >(
                    xMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.linguistic2.LinguServiceManager" ))), css::uno::UNO_QUERY );
            }

            if ( m_xLinguServiceManager.is() )
            {
                css::uno::Reference< css::linguistic2::XHyphenator > xHyphenator( m_xLinguServiceManager->getHyphenator(), css::uno::UNO_QUERY );
                if( xHyphenator.is() )
                    mpOutliner->SetHyphenator( xHyphenator );
            }
// -
        }

        mpTextForwarder = new SvxOutlinerForwarder( *mpOutliner, mpObject );

        // delay listener subscription and UAA initialization until Outliner is fully setup
        bCreated = sal_True;

        mbForwarderIsEditMode = sal_False;
    }

    if( mpObject && mpText && !mbDataValid && mpObject->IsInserted() && mpObject->GetPage() )
    {
        mpTextForwarder->flushCache();

        OutlinerParaObject* mpOutlinerParaObject = NULL;
        BOOL bTextEditActive = FALSE;
        SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, mpObject );
        if( pTextObj && pTextObj->getActiveText() == mpText )
            mpOutlinerParaObject = pTextObj->GetEditOutlinerParaObject(); // Get the OutlinerParaObject if text edit is active

        if( mpOutlinerParaObject )
            bTextEditActive = TRUE; // text edit active
        else
            mpOutlinerParaObject = mpText->GetOutlinerParaObject();

        if( mpOutlinerParaObject && ( bTextEditActive || !mpObject->IsEmptyPresObj() || mpObject->GetPage()->IsMasterPage() ) )
        {
            mpOutliner->SetText( *mpOutlinerParaObject );

            // #91254# put text to object and set EmptyPresObj to FALSE
            if( mpText && bTextEditActive && mpOutlinerParaObject && mpObject->IsEmptyPresObj() && pTextObj->IsRealyEdited() )
            {
                mpObject->SetEmptyPresObj( FALSE );
                mpText->SetOutlinerParaObject( mpOutlinerParaObject );
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

                    mpText->SetOutlinerParaObject( mpOutliner->CreateParaObject() );
                }
                else
                    mpText->SetOutlinerParaObject( NULL );

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
    OGuard aGuard( Application::GetSolarMutex() );

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

SdrObject* SvxTextEditSource::GetSdrObject() const
{
    return mpImpl->GetSdrObject();
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
