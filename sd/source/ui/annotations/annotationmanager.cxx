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


#include "sddll.hxx"

#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <comphelper/string.hxx>
#include <svx/svxids.hrc>
#include <vcl/menu.hxx>
#include <vcl/msgbox.hxx>

#include <sal/macros.h>
#include <svl/style.hxx>
#include <svl/itempool.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/saveopt.hxx>

#include <tools/datetime.hxx>

#include <sfx2/imagemgr.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objface.hxx>

#include <editeng/editeng.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>

#include <svx/svdetc.hxx>

#include "annotationmanager.hxx"
#include "annotationmanagerimpl.hxx"
#include "annotationwindow.hxx"
#include "annotations.hrc"

#include "ToolBarManager.hxx"
#include "DrawDocShell.hxx"
#include "DrawViewShell.hxx"
#include "DrawController.hxx"
#include "glob.hrc"
#include "sdresid.hxx"
#include "EventMultiplexer.hxx"
#include "ViewShellManager.hxx"
#include "helpids.h"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "textapi.hxx"
#include "optsitem.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::geometry;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::office;

namespace sd {

// --------------------------------------------------------------------

extern TextApiObject* getTextApiObject( const Reference< XAnnotation >& xAnnotation );

// --------------------------------------------------------------------

SfxItemPool* GetAnnotationPool()
{
    static SfxItemPool* mpAnnotationPool = 0;
    if( mpAnnotationPool == 0 )
    {
        mpAnnotationPool = EditEngine::CreatePool( sal_False );
        mpAnnotationPool->SetPoolDefaultItem(SvxFontHeightItem(423,100,EE_CHAR_FONTHEIGHT));

        Font aAppFont( Application::GetSettings().GetStyleSettings().GetAppFont() );
        mpAnnotationPool->SetPoolDefaultItem(SvxFontItem(aAppFont.GetFamily(),aAppFont.GetName(),"",PITCH_DONTKNOW,RTL_TEXTENCODING_DONTKNOW,EE_CHAR_FONTINFO));
    }

    return mpAnnotationPool;
}

// --------------------------------------------------------------------

static SfxBindings* getBindings( ViewShellBase& rBase )
{
    if( rBase.GetMainViewShell().get() && rBase.GetMainViewShell()->GetViewFrame() )
        return &rBase.GetMainViewShell()->GetViewFrame()->GetBindings();

    return 0;
}

// --------------------------------------------------------------------

static SfxDispatcher* getDispatcher( ViewShellBase& rBase )
{
    if( rBase.GetMainViewShell().get() && rBase.GetMainViewShell()->GetViewFrame() )
        return rBase.GetMainViewShell()->GetViewFrame()->GetDispatcher();

    return 0;
}

com::sun::star::util::DateTime getCurrentDateTime()
{
    DateTime aCurrentDate( DateTime::SYSTEM );
    return com::sun::star::util::DateTime( 0, aCurrentDate.GetSec(),
            aCurrentDate.GetMin(), aCurrentDate.GetHour(),
            aCurrentDate.GetDay(), aCurrentDate.GetMonth(),
            aCurrentDate.GetYear(), false );
}

OUString getAnnotationDateTimeString( const Reference< XAnnotation >& xAnnotation )
{
    OUString sRet;
    if( xAnnotation.is() )
    {
        const SvtSysLocale aSysLocale;
        const LocaleDataWrapper& rLocalData = aSysLocale.GetLocaleData();

        com::sun::star::util::DateTime aDateTime( xAnnotation->getDateTime() );

        Date aSysDate( Date::SYSTEM );
        Date aDate = Date( aDateTime.Day, aDateTime.Month, aDateTime.Year );
        if (aDate==aSysDate)
            sRet = SdResId(STR_ANNOTATION_TODAY);
        else if (aDate == Date(aSysDate-1))
            sRet = SdResId(STR_ANNOTATION_YESTERDAY);
        else if (aDate.IsValidAndGregorian() )
            sRet = rLocalData.getDate(aDate);

        Time aTime( aDateTime.Hours, aDateTime.Minutes, aDateTime.Seconds, aDateTime.NanoSeconds );
        if(aTime.GetTime() != 0)
            sRet = sRet + " "  + rLocalData.getTime( aTime,false );
    }
    return sRet;
}

// --------------------------------------------------------------------

AnnotationManagerImpl::AnnotationManagerImpl( ViewShellBase& rViewShellBase )
: AnnotationManagerImplBase( m_aMutex )
, mrBase( rViewShellBase )
, mpDoc( rViewShellBase.GetDocument() )
, mbShowAnnotations( true )
, mnUpdateTagsEvent( 0 )
{
    SdOptions* pOptions = SD_MOD()->GetSdOptions(mpDoc->GetDocumentType());
    if( pOptions )
        mbShowAnnotations = pOptions->IsShowComments() == sal_True;
}

// --------------------------------------------------------------------

void AnnotationManagerImpl::init()
{
    // get current controller and initialize listeners
    try
    {
        addListener();
        mxView = Reference< XDrawView >::query(mrBase.GetController());
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::AnnotationManagerImpl::AnnotationManagerImpl(), Exception caught!" );
    }

    try
    {
        Reference<XEventBroadcaster> xModel (mrBase.GetDocShell()->GetModel(), UNO_QUERY_THROW );
        Reference<XEventListener> xListener( this );
        xModel->addEventListener( xListener );
    }
    catch( Exception& )
    {
    }
}

// --------------------------------------------------------------------

// WeakComponentImplHelper1
void SAL_CALL AnnotationManagerImpl::disposing ()
{
    try
    {
        Reference<XEventBroadcaster> xModel (mrBase.GetDocShell()->GetModel(), UNO_QUERY_THROW );
        Reference<XEventListener> xListener( this );
        xModel->removeEventListener( xListener );
    }
    catch( Exception& )
    {
    }

    removeListener();
    DisposeTags();

    if( mnUpdateTagsEvent )
    {
        Application::RemoveUserEvent( mnUpdateTagsEvent );
        mnUpdateTagsEvent = 0;
    }

    mxView.clear();
    mxCurrentPage.clear();
}

// --------------------------------------------------------------------

// XEventListener
void SAL_CALL AnnotationManagerImpl::notifyEvent( const ::com::sun::star::document::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException)
{
    if( aEvent.EventName == "OnAnnotationInserted" || aEvent.EventName == "OnAnnotationRemoved" || aEvent.EventName == "OnAnnotationChanged" )
    {
        UpdateTags();
    }
}

void SAL_CALL AnnotationManagerImpl::disposing( const ::com::sun::star::lang::EventObject& /*Source*/ ) throw (::com::sun::star::uno::RuntimeException)
{
}

void AnnotationManagerImpl::ShowAnnotations( bool bShow )
{
    // enforce show annotations if a new annotation is inserted
    if( mbShowAnnotations != bShow )
    {
        mbShowAnnotations = bShow;

        SdOptions* pOptions = SD_MOD()->GetSdOptions(mpDoc->GetDocumentType());
        if( pOptions )
            pOptions->SetShowComments( mbShowAnnotations ? sal_True : sal_False );

        UpdateTags();
    }
}

// --------------------------------------------------------------------

void AnnotationManagerImpl::ExecuteAnnotation(SfxRequest& rReq )
{
    switch( rReq.GetSlot() )
    {
    case SID_INSERT_POSTIT:
        ExecuteInsertAnnotation( rReq );
        break;
    case SID_DELETE_POSTIT:
    case SID_DELETEALL_POSTIT:
    case SID_DELETEALLBYAUTHOR_POSTIT:
        ExecuteDeleteAnnotation( rReq );
        break;
    case SID_PREVIOUS_POSTIT:
    case SID_NEXT_POSTIT:
        SelectNextAnnotation( rReq.GetSlot() == SID_NEXT_POSTIT );
        break;
    case SID_REPLYTO_POSTIT:
        ExecuteReplyToAnnotation( rReq );
        break;
    case SID_SHOW_POSTIT:
        ShowAnnotations( !mbShowAnnotations );
        break;
    }
}

// --------------------------------------------------------------------

void AnnotationManagerImpl::ExecuteInsertAnnotation(SfxRequest& /*rReq*/)
{
    ShowAnnotations(true);
    InsertAnnotation();
}

// --------------------------------------------------------------------

void AnnotationManagerImpl::ExecuteDeleteAnnotation(SfxRequest& rReq)
{
    ShowAnnotations( true );

    const SfxItemSet* pArgs = rReq.GetArgs();

    switch( rReq.GetSlot() )
    {
    case SID_DELETEALL_POSTIT:
        DeleteAllAnnotations();
        break;
    case SID_DELETEALLBYAUTHOR_POSTIT:
        if( pArgs )
        {
            const SfxPoolItem*  pPoolItem = NULL;
            if( SFX_ITEM_SET == pArgs->GetItemState( SID_DELETEALLBYAUTHOR_POSTIT, sal_True, &pPoolItem ) )
            {
                OUString sAuthor( (( const SfxStringItem* ) pPoolItem )->GetValue() );
                DeleteAnnotationsByAuthor( sAuthor );
            }
        }
        break;
    case SID_DELETE_POSTIT:
        {
            Reference< XAnnotation > xAnnotation;

            if( rReq.GetSlot() == SID_DELETE_POSTIT )
            {
                if( pArgs )
                {
                    const SfxPoolItem*  pPoolItem = NULL;
                    if( SFX_ITEM_SET == pArgs->GetItemState( SID_DELETE_POSTIT, sal_True, &pPoolItem ) )
                        ( ( const SfxUnoAnyItem* ) pPoolItem )->GetValue() >>= xAnnotation;
                }
            }

            if( !xAnnotation.is() )
                GetSelectedAnnotation( xAnnotation );

            DeleteAnnotation( xAnnotation );
        }
        break;
    }

    UpdateTags();
}

// --------------------------------------------------------------------

void AnnotationManagerImpl::InsertAnnotation()
{
    SdPage* pPage = GetCurrentPage();
    if( pPage )
    {
        if( mpDoc->IsUndoEnabled() )
            mpDoc->BegUndo( SD_RESSTR( STR_ANNOTATION_UNDO_INSERT ) );

        // find free space for new annotation
        int y = 0, x = 0;

        AnnotationVector aAnnotations( pPage->getAnnotations() );
        if( !aAnnotations.empty() )
        {
            const int page_width = pPage->GetSize().Width();
            const int width = 1000;
            const int height = 800;
            Rectangle aTagRect;

            while( true )
            {
                Rectangle aNewRect( x, y, x + width - 1, y + height - 1 );
                bool bFree = true;

                for( AnnotationVector::iterator iter = aAnnotations.begin(); iter != aAnnotations.end(); ++iter )
                {
                    RealPoint2D aPoint( (*iter)->getPosition() );
                    aTagRect.Left()   = sal::static_int_cast< long >( aPoint.X * 100.0 );
                    aTagRect.Top()    = sal::static_int_cast< long >( aPoint.Y * 100.0 );
                    aTagRect.Right()  = aTagRect.Left() + width - 1;
                    aTagRect.Bottom() = aTagRect.Top() + height - 1;

                    if( aNewRect.IsOver( aTagRect ) )
                    {
                        bFree = false;
                        break;
                    }
                }

                if( bFree == false)
                {
                    x += width;
                    if( x > page_width )
                    {
                        x = 0;
                        y += height;
                    }
                }
                else
                {
                    break;
                }
            }
        }

        Reference< XAnnotation > xAnnotation;
        pPage->createAnnotation( xAnnotation );

        // set current author to new annotation
        SvtUserOptions aUserOptions;
        xAnnotation->setAuthor( aUserOptions.GetFullName() );

        // set current time to new annotation
        xAnnotation->setDateTime( getCurrentDateTime() );

        // set position
        RealPoint2D aPos( ((double)x) / 100.0, ((double)y) / 100.0 );
        xAnnotation->setPosition( aPos );

        if( mpDoc->IsUndoEnabled() )
            mpDoc->EndUndo();

        UpdateTags(true);
        SelectAnnotation( xAnnotation, true );
    }
}

// --------------------------------------------------------------------

void AnnotationManagerImpl::ExecuteReplyToAnnotation( SfxRequest& rReq )
{
    Reference< XAnnotation > xAnnotation;
    const SfxItemSet* pArgs = rReq.GetArgs();
    if( pArgs )
    {
        const SfxPoolItem*  pPoolItem = NULL;
        if( SFX_ITEM_SET == pArgs->GetItemState( rReq.GetSlot(), sal_True, &pPoolItem ) )
            ( ( const SfxUnoAnyItem* ) pPoolItem )->GetValue() >>= xAnnotation;
    }


    TextApiObject* pTextApi = getTextApiObject( xAnnotation );
    if( pTextApi )
    {
        std::auto_ptr< ::Outliner > pOutliner( new ::Outliner(GetAnnotationPool(),OUTLINERMODE_TEXTOBJECT) );

        mpDoc->SetCalcFieldValueHdl( pOutliner.get() );
        pOutliner->SetUpdateMode( sal_True );

        OUString aStr(SD_RESSTR(STR_ANNOTATION_REPLY));
        OUString sAuthor( xAnnotation->getAuthor() );
        if( sAuthor.isEmpty() )
            sAuthor = SD_RESSTR( STR_ANNOTATION_NOAUTHOR );

        aStr = aStr.replaceFirst("%1", sAuthor);

        aStr += " (" + getAnnotationDateTimeString( xAnnotation ) + "): \"";

        OUString sQuote( pTextApi->GetText() );

        if( sQuote.isEmpty() )
            sQuote = "...";
        aStr += sQuote + "\"\n";

        sal_Int32 nParaCount = comphelper::string::getTokenCount(aStr, '\n');
        for( sal_Int32 nPara = 0; nPara < nParaCount; nPara++ )
            pOutliner->Insert( aStr.getToken( nPara, '\n' ), EE_PARA_APPEND, -1 );

        if( pOutliner->GetParagraphCount() > 1 )
        {
            SfxItemSet aAnswerSet( pOutliner->GetEmptyItemSet() );
            aAnswerSet.Put(SvxPostureItem(ITALIC_NORMAL,EE_CHAR_ITALIC));

            ESelection aSel;
            aSel.nEndPara = pOutliner->GetParagraphCount()-2;
            aSel.nEndPos = pOutliner->GetText( pOutliner->GetParagraph( aSel.nEndPara ) ).getLength();

            pOutliner->QuickSetAttribs( aAnswerSet, aSel );
        }

        std::auto_ptr< OutlinerParaObject > pOPO( pOutliner->CreateParaObject() );
        pTextApi->SetText( *pOPO.get() );

        SvtUserOptions aUserOptions;
        xAnnotation->setAuthor( aUserOptions.GetFullName() );

        // set current time to reply
        xAnnotation->setDateTime( getCurrentDateTime() );

        UpdateTags(true);
        SelectAnnotation( xAnnotation, true );
    }
}

// --------------------------------------------------------------------

void AnnotationManagerImpl::DeleteAnnotation( Reference< XAnnotation > xAnnotation )
{
    SdPage* pPage = GetCurrentPage();

    if( xAnnotation.is() && pPage )
    {
        if( mpDoc->IsUndoEnabled() )
            mpDoc->BegUndo( SD_RESSTR( STR_ANNOTATION_UNDO_DELETE ) );

        pPage->removeAnnotation( xAnnotation );

        if( mpDoc->IsUndoEnabled() )
            mpDoc->EndUndo();

        UpdateTags();
    }
}

void AnnotationManagerImpl::DeleteAnnotationsByAuthor( const OUString& sAuthor )
{
    if( mpDoc->IsUndoEnabled() )
        mpDoc->BegUndo( SD_RESSTR( STR_ANNOTATION_UNDO_DELETE ) );

    SdPage* pPage = 0;
    do
    {
        pPage = GetNextPage( pPage, true );

        if( pPage && !pPage->getAnnotations().empty() )
        {
            AnnotationVector aAnnotations( pPage->getAnnotations() );
            for( AnnotationVector::iterator iter = aAnnotations.begin(); iter != aAnnotations.end(); ++iter )
            {
                Reference< XAnnotation > xAnnotation( *iter );
                if( xAnnotation->getAuthor() == sAuthor )
                {
                    if( mxSelectedAnnotation == xAnnotation )
                        mxSelectedAnnotation.clear();
                    pPage->removeAnnotation( xAnnotation );
                }
            }
        }
    } while( pPage );

    if( mpDoc->IsUndoEnabled() )
        mpDoc->EndUndo();
}

void AnnotationManagerImpl::DeleteAllAnnotations()
{
    if( mpDoc->IsUndoEnabled() )
        mpDoc->BegUndo( SD_RESSTR( STR_ANNOTATION_UNDO_DELETE ) );

    SdPage* pPage = 0;
    do
    {
        pPage = GetNextPage( pPage, true );

        if( pPage && !pPage->getAnnotations().empty() )
        {

            AnnotationVector aAnnotations( pPage->getAnnotations() );
            for( AnnotationVector::iterator iter = aAnnotations.begin(); iter != aAnnotations.end(); ++iter )
            {
                pPage->removeAnnotation( (*iter) );
            }
        }
    }
    while( pPage );

    mxSelectedAnnotation.clear();

    if( mpDoc->IsUndoEnabled() )
        mpDoc->EndUndo();
}

// --------------------------------------------------------------------

void AnnotationManagerImpl::GetAnnotationState(SfxItemSet& rSet)
{
    SdPage* pCurrentPage = GetCurrentPage();

    const bool bReadOnly = mrBase.GetDocShell()->IsReadOnly();
    const bool bWrongPageKind = (pCurrentPage == 0) || (pCurrentPage->GetPageKind() != PK_STANDARD);

    const SvtSaveOptions::ODFDefaultVersion nCurrentODFVersion( SvtSaveOptions().GetODFDefaultVersion() );

    if( bReadOnly || bWrongPageKind || (nCurrentODFVersion <= SvtSaveOptions::ODFVER_012) )
        rSet.DisableItem( SID_INSERT_POSTIT );

    rSet.Put(SfxBoolItem(SID_SHOW_POSTIT, mbShowAnnotations));

    Reference< XAnnotation > xAnnotation;
    GetSelectedAnnotation( xAnnotation );

    if( !xAnnotation.is() || bReadOnly )
        rSet.DisableItem( SID_DELETE_POSTIT );

    SdPage* pPage = 0;

    bool bHasAnnotations = false;
    do
    {
        pPage = GetNextPage( pPage, true );

        if( pPage && !pPage->getAnnotations().empty() )
            bHasAnnotations = true;
    }
    while( pPage && !bHasAnnotations );

    if( !bHasAnnotations || bReadOnly )
    {
        rSet.DisableItem( SID_DELETEALL_POSTIT );
    }

    if( bWrongPageKind || !bHasAnnotations )
    {
        rSet.DisableItem( SID_PREVIOUS_POSTIT );
        rSet.DisableItem( SID_NEXT_POSTIT );
    }
}

// --------------------------------------------------------------------

void AnnotationManagerImpl::SelectNextAnnotation(bool bForeward)
{
    ShowAnnotations( true );

    Reference< XAnnotation > xCurrent;
    GetSelectedAnnotation( xCurrent );
    SdPage* pPage = GetCurrentPage();
    if( !pPage )
        return;

    AnnotationVector aAnnotations( pPage->getAnnotations() );

    if( bForeward )
    {
        if( xCurrent.is() )
        {
            for( AnnotationVector::iterator iter = aAnnotations.begin(); iter != aAnnotations.end(); ++iter )
            {
                if( (*iter) == xCurrent )
                {
                    ++iter;
                    if( iter != aAnnotations.end() )
                    {
                        SelectAnnotation( (*iter) );
                        return;
                    }
                    break;
                }
            }
        }
        else if( !aAnnotations.empty() )
        {
            SelectAnnotation( *(aAnnotations.begin()) );
            return;
        }
    }
    else
    {
        if( xCurrent.is() )
        {
            for( AnnotationVector::iterator iter = aAnnotations.begin(); iter != aAnnotations.end(); ++iter )
            {
                if( (*iter) == xCurrent )
                {
                    if( iter != aAnnotations.begin() )
                    {
                        --iter;
                        SelectAnnotation( (*iter) );
                        return;
                    }
                    break;
                }
            }
        }
        else if( !aAnnotations.empty() )
        {
            AnnotationVector::iterator iter( aAnnotations.end() );
            SelectAnnotation( *(--iter) );
            return;
        }
    }

    mxSelectedAnnotation.clear();
    do
    {
        do
        {
            pPage = GetNextPage( pPage, bForeward );

            if( pPage && !pPage->getAnnotations().empty() )
            {
                // switch to next/previous slide with annotations
                ::boost::shared_ptr<DrawViewShell> pDrawViewShell(::boost::dynamic_pointer_cast<DrawViewShell>(mrBase.GetMainViewShell()));
                if (pDrawViewShell.get() != NULL)
                {
                    pDrawViewShell->ChangeEditMode(pPage->IsMasterPage() ? EM_MASTERPAGE : EM_PAGE, sal_False);
                    pDrawViewShell->SwitchPage((pPage->GetPageNum() - 1) >> 1);

                    SfxDispatcher* pDispatcher = getDispatcher( mrBase );
                    if( pDispatcher )
                        pDispatcher->Execute( bForeward ? SID_NEXT_POSTIT : SID_PREVIOUS_POSTIT );

                    return;
                }
            }
        }
        while( pPage );

        // The question text depends on the search direction.
        bool bImpress = mpDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS;
        sal_uInt16 nStringId;
        if(bForeward)
            nStringId = bImpress ? STR_ANNOTATION_WRAP_FORWARD : STR_ANNOTATION_WRAP_FORWARD_DRAW;
        else
            nStringId = bImpress ? STR_ANNOTATION_WRAP_BACKWARD : STR_ANNOTATION_WRAP_BACKWARD_DRAW;

        // Pop up question box that asks the user whether to wrap arround.
        // The dialog is made modal with respect to the whole application.
        QueryBox aQuestionBox ( NULL, (WB_YES_NO | WB_DEF_YES), SD_RESSTR(nStringId));
        aQuestionBox.SetImage (QueryBox::GetStandardImage());
        short nBoxResult = aQuestionBox.Execute();
        if (nBoxResult != RET_YES)
            break;
    }
    while( true );
}

// --------------------------------------------------------------------

void AnnotationManagerImpl::onTagSelected( AnnotationTag& rTag )
{
    mxSelectedAnnotation = rTag.GetAnnotation();
    invalidateSlots();
}

// --------------------------------------------------------------------

void AnnotationManagerImpl::onTagDeselected( AnnotationTag& rTag )
{
    if( rTag.GetAnnotation() == mxSelectedAnnotation )
    {
        mxSelectedAnnotation.clear();
        invalidateSlots();
    }
}

// --------------------------------------------------------------------

void AnnotationManagerImpl::SelectAnnotation( ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation > xAnnotation, bool bEdit /* = sal_False */ )
{
    mxSelectedAnnotation = xAnnotation;

    const AnnotationTagVector::const_iterator aEnd( maTagVector.end() );
    for( AnnotationTagVector::const_iterator iter( maTagVector.begin() );
        iter != aEnd; ++iter)
    {
        if( (*iter)->GetAnnotation() == xAnnotation )
        {
            SmartTagReference xTag( (*iter).get() );
            mrBase.GetMainViewShell()->GetView()->getSmartTags().select( xTag );
            (*iter)->OpenPopup( bEdit );
            break;
        }
    }
}

// --------------------------------------------------------------------

void AnnotationManagerImpl::GetSelectedAnnotation( ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation >& xAnnotation )
{
    xAnnotation = mxSelectedAnnotation;
}

void AnnotationManagerImpl::invalidateSlots()
{
    SfxBindings* pBindings = getBindings( mrBase );
    if( pBindings )
    {
        pBindings->Invalidate( SID_INSERT_POSTIT );
        pBindings->Invalidate( SID_DELETE_POSTIT );
        pBindings->Invalidate( SID_DELETEALL_POSTIT );
        pBindings->Invalidate( SID_PREVIOUS_POSTIT );
        pBindings->Invalidate( SID_NEXT_POSTIT );
        pBindings->Invalidate( SID_UNDO );
        pBindings->Invalidate( SID_REDO );
    }
}

// --------------------------------------------------------------------

void AnnotationManagerImpl::onSelectionChanged()
{
    if( mxView.is() && mrBase.GetDrawView() ) try
    {
        Reference< XAnnotationAccess > xPage( mxView->getCurrentPage(), UNO_QUERY );

        if( xPage != mxCurrentPage )
        {
            mxCurrentPage = xPage;

            UpdateTags(true);
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::AnnotationManagerImpl::onSelectionChanged(), exception caught!" );
    }
}

void AnnotationManagerImpl::UpdateTags( bool bSynchron )
{
    if( bSynchron )
    {
        if( mnUpdateTagsEvent )
            Application::RemoveUserEvent( mnUpdateTagsEvent );

        UpdateTagsHdl(0);
    }
    else
    {
        if( !mnUpdateTagsEvent && mxView.is() )
            mnUpdateTagsEvent = Application::PostUserEvent( LINK( this, AnnotationManagerImpl, UpdateTagsHdl ) );
    }
}

IMPL_LINK_NOARG(AnnotationManagerImpl, UpdateTagsHdl)
{
    mnUpdateTagsEvent  = 0;
    DisposeTags();

    if( mbShowAnnotations )
        CreateTags();

    if( mrBase.GetDrawView() )
        static_cast< ::sd::View* >( mrBase.GetDrawView() )->updateHandles();

    invalidateSlots();

    return 0;
}

void AnnotationManagerImpl::CreateTags()
{

    if( mxCurrentPage.is() && mpDoc ) try
    {
        int nIndex = 1;
        maFont = Application::GetSettings().GetStyleSettings().GetAppFont();

        rtl::Reference< AnnotationTag > xSelectedTag;

        Reference< XAnnotationEnumeration > xEnum( mxCurrentPage->createAnnotationEnumeration() );
        while( xEnum->hasMoreElements() )
        {
            Reference< XAnnotation > xAnnotation( xEnum->nextElement() );
            Color aColor( GetColorLight( mpDoc->GetAnnotationAuthorIndex( xAnnotation->getAuthor() ) ) );
            rtl::Reference< AnnotationTag > xTag( new AnnotationTag( *this, *mrBase.GetMainViewShell()->GetView(), xAnnotation, aColor, nIndex++, maFont ) );
            maTagVector.push_back(xTag);

            if( xAnnotation == mxSelectedAnnotation )
            {
                xSelectedTag = xTag;
            }
        }

        if( xSelectedTag.is() )
        {
            SmartTagReference xTag( xSelectedTag.get() );
            mrBase.GetMainViewShell()->GetView()->getSmartTags().select( xTag );
        }
        else
        {
            // no tag, no selection!
            mxSelectedAnnotation.clear();
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::AnnotationManagerImpl::onSelectionChanged(), exception caught!" );
    }
}

// --------------------------------------------------------------------

void AnnotationManagerImpl::DisposeTags()
{
    if( !maTagVector.empty() )
    {
        AnnotationTagVector::iterator iter = maTagVector.begin();
        do
        {
            (*iter++)->Dispose();
        }
        while( iter != maTagVector.end() );

        maTagVector.clear();
    }
}

// --------------------------------------------------------------------

void AnnotationManagerImpl::addListener()
{
    Link aLink( LINK(this,AnnotationManagerImpl,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->AddEventListener (
        aLink,
        tools::EventMultiplexerEvent::EID_EDIT_VIEW_SELECTION
        | tools::EventMultiplexerEvent::EID_CURRENT_PAGE
        | tools::EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED
        | tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED);
}

// --------------------------------------------------------------------

void AnnotationManagerImpl::removeListener()
{
    Link aLink( LINK(this,AnnotationManagerImpl,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->RemoveEventListener( aLink );
}

// --------------------------------------------------------------------

IMPL_LINK(AnnotationManagerImpl,EventMultiplexerListener,
    tools::EventMultiplexerEvent*,pEvent)
{
    switch (pEvent->meEventId)
    {
        case tools::EventMultiplexerEvent::EID_CURRENT_PAGE:
        case tools::EventMultiplexerEvent::EID_EDIT_VIEW_SELECTION:
            onSelectionChanged();
            break;

        case tools::EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED:
            mxView.clear();
            onSelectionChanged();
            break;

        case tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED:
            mxView = Reference<XDrawView>::query( mrBase.GetController() );
            onSelectionChanged();
            break;
    }
    return 0;
}

void AnnotationManagerImpl::ExecuteAnnotationContextMenu( Reference< XAnnotation > xAnnotation, ::Window* pParent, const Rectangle& rContextRect, bool bButtonMenu /* = false */ )
{
    SfxDispatcher* pDispatcher( getDispatcher( mrBase ) );
    if( !pDispatcher )
        return;

    const bool bReadOnly = mrBase.GetDocShell()->IsReadOnly();

    AnnotationWindow* pAnnotationWindow = bButtonMenu ? 0 : dynamic_cast< AnnotationWindow* >( pParent );

    if( bReadOnly && !pAnnotationWindow )
        return;

    std::auto_ptr< PopupMenu > pMenu( new PopupMenu( SdResId( pAnnotationWindow ? RID_ANNOTATION_CONTEXTMENU : RID_ANNOTATION_TAG_CONTEXTMENU ) ) );

    SvtUserOptions aUserOptions;
    OUString sCurrentAuthor( aUserOptions.GetFullName() );
    OUString sAuthor( xAnnotation->getAuthor() );

    OUString aStr( pMenu->GetItemText( SID_DELETEALLBYAUTHOR_POSTIT ) );
    OUString aReplace( sAuthor );
    if( aReplace.isEmpty() )
        aReplace = SD_RESSTR( STR_ANNOTATION_NOAUTHOR );
    aStr = aStr.replaceFirst("%1", aReplace);
    pMenu->SetItemText( SID_DELETEALLBYAUTHOR_POSTIT, aStr );
    pMenu->EnableItem( SID_REPLYTO_POSTIT, (sAuthor != sCurrentAuthor) && !bReadOnly );
    pMenu->EnableItem( SID_DELETE_POSTIT, (xAnnotation.is() && !bReadOnly) ? sal_True : sal_False );
    pMenu->EnableItem( SID_DELETEALLBYAUTHOR_POSTIT, !bReadOnly );
    pMenu->EnableItem( SID_DELETEALL_POSTIT, !bReadOnly );

    if( pAnnotationWindow )
    {
        if( pAnnotationWindow->IsProtected() || bReadOnly )
        {
            pMenu->EnableItem( SID_ATTR_CHAR_WEIGHT, sal_False );
            pMenu->EnableItem( SID_ATTR_CHAR_POSTURE, sal_False );
            pMenu->EnableItem( SID_ATTR_CHAR_UNDERLINE, sal_False );
            pMenu->EnableItem( SID_ATTR_CHAR_STRIKEOUT, sal_False );
            pMenu->EnableItem( SID_PASTE, sal_False );
        }
        else
        {
            SfxItemSet aSet(pAnnotationWindow->getView()->GetAttribs());

            if ( aSet.GetItemState( EE_CHAR_WEIGHT ) == SFX_ITEM_ON )
            {
                if( ((const SvxWeightItem&)aSet.Get( EE_CHAR_WEIGHT )).GetWeight() == WEIGHT_BOLD )
                    pMenu->CheckItem( SID_ATTR_CHAR_WEIGHT );
            }

            if ( aSet.GetItemState( EE_CHAR_ITALIC ) == SFX_ITEM_ON )
            {
                if( ((const SvxPostureItem&)aSet.Get( EE_CHAR_ITALIC )).GetPosture() != ITALIC_NONE )
                    pMenu->CheckItem( SID_ATTR_CHAR_POSTURE );

            }
            if ( aSet.GetItemState( EE_CHAR_UNDERLINE ) == SFX_ITEM_ON )
            {
                if( ((const SvxUnderlineItem&)aSet.Get( EE_CHAR_UNDERLINE )).GetLineStyle() != UNDERLINE_NONE )
                    pMenu->CheckItem( SID_ATTR_CHAR_UNDERLINE );
            }

            if ( aSet.GetItemState( EE_CHAR_STRIKEOUT ) == SFX_ITEM_ON )
            {
                if( ((const SvxCrossedOutItem&)aSet.Get( EE_CHAR_STRIKEOUT )).GetStrikeout() != STRIKEOUT_NONE )
                    pMenu->CheckItem( SID_ATTR_CHAR_STRIKEOUT );
            }
            TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pAnnotationWindow ) );
            pMenu->EnableItem( SID_PASTE, aDataHelper.GetFormatCount() != 0 );
        }

        pMenu->EnableItem( SID_COPY, pAnnotationWindow->getView()->HasSelection() );
    }

    sal_uInt16 nId = 0;

    // set slot images
    Reference< ::com::sun::star::frame::XFrame > xFrame( mrBase.GetMainViewShell()->GetViewFrame()->GetFrame().GetFrameInterface() );
    if( xFrame.is() )
    {
        for( sal_uInt16 nPos = 0; nPos < pMenu->GetItemCount(); nPos++ )
        {
            nId = pMenu->GetItemId( nPos );
            if( pMenu->IsItemEnabled( nId ) )
            {
                OUString sSlotURL( "slot:" );
                sSlotURL += OUString::valueOf( sal_Int32( nId ));

                Image aImage( GetImage( xFrame, sSlotURL, false ) );
                if( !!aImage )
                    pMenu->SetItemImage( nId, aImage );
            }
        }
    }

    nId = pMenu->Execute( pParent, rContextRect, POPUPMENU_EXECUTE_DOWN|POPUPMENU_NOMOUSEUPCLOSE );
    switch( nId )
    {
    case SID_REPLYTO_POSTIT:
    {
        const SfxUnoAnyItem aItem( SID_REPLYTO_POSTIT, Any( xAnnotation ) );
        pDispatcher->Execute( SID_REPLYTO_POSTIT, SFX_CALLMODE_ASYNCHRON, &aItem, 0 );
        break;
    }
    case SID_DELETE_POSTIT:
    {
        const SfxUnoAnyItem aItem( SID_DELETE_POSTIT, Any( xAnnotation ) );
        pDispatcher->Execute( SID_DELETE_POSTIT, SFX_CALLMODE_ASYNCHRON, &aItem, 0 );
        break;
    }
    case SID_DELETEALLBYAUTHOR_POSTIT:
    {
        const SfxStringItem aItem( SID_DELETEALLBYAUTHOR_POSTIT, sAuthor );
        pDispatcher->Execute( SID_DELETEALLBYAUTHOR_POSTIT, SFX_CALLMODE_ASYNCHRON, &aItem, 0 );
        break;
    }
    case SID_DELETEALL_POSTIT:
        pDispatcher->Execute( SID_DELETEALL_POSTIT );
        break;
    case SID_COPY:
    case SID_PASTE:
    case SID_ATTR_CHAR_WEIGHT:
    case SID_ATTR_CHAR_POSTURE:
    case SID_ATTR_CHAR_UNDERLINE:
    case SID_ATTR_CHAR_STRIKEOUT:
        if( pAnnotationWindow )
            pAnnotationWindow->ExecuteSlot( nId );
        break;
    }
}

// ====================================================================

Color AnnotationManagerImpl::GetColor(sal_uInt16 aAuthorIndex)
{
    if (!Application::GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        static const Color aArrayNormal[] = {
            COL_AUTHOR1_NORMAL,     COL_AUTHOR2_NORMAL,     COL_AUTHOR3_NORMAL,
            COL_AUTHOR4_NORMAL,     COL_AUTHOR5_NORMAL,     COL_AUTHOR6_NORMAL,
            COL_AUTHOR7_NORMAL,     COL_AUTHOR8_NORMAL,     COL_AUTHOR9_NORMAL };

        return Color( aArrayNormal[ aAuthorIndex % (sizeof( aArrayNormal )/ sizeof( aArrayNormal[0] ))]);
    }

    return Color(COL_WHITE);
}

Color AnnotationManagerImpl::GetColorLight(sal_uInt16 aAuthorIndex)
{
    if (!Application::GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        static const Color aArrayLight[] = {
            COL_AUTHOR1_LIGHT,      COL_AUTHOR2_LIGHT,      COL_AUTHOR3_LIGHT,
            COL_AUTHOR4_LIGHT,      COL_AUTHOR5_LIGHT,      COL_AUTHOR6_LIGHT,
            COL_AUTHOR7_LIGHT,      COL_AUTHOR8_LIGHT,      COL_AUTHOR9_LIGHT };

        return Color( aArrayLight[ aAuthorIndex % (sizeof( aArrayLight )/ sizeof( aArrayLight[0] ))]);
    }

    return Color(COL_WHITE);
}

Color AnnotationManagerImpl::GetColorDark(sal_uInt16 aAuthorIndex)
{
    if (!Application::GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        static const Color aArrayAnkor[] = {
            COL_AUTHOR1_DARK,       COL_AUTHOR2_DARK,       COL_AUTHOR3_DARK,
            COL_AUTHOR4_DARK,       COL_AUTHOR5_DARK,       COL_AUTHOR6_DARK,
            COL_AUTHOR7_DARK,       COL_AUTHOR8_DARK,       COL_AUTHOR9_DARK };

        return Color( aArrayAnkor[  aAuthorIndex % (sizeof( aArrayAnkor )   / sizeof( aArrayAnkor[0] ))]);
    }

    return Color(COL_WHITE);
}

SdPage* AnnotationManagerImpl::GetNextPage( SdPage* pPage, bool bForeward )
{
    if( pPage == 0 )
        return bForeward ? GetFirstPage() : GetLastPage();

    sal_uInt16 nPageNum = (pPage->GetPageNum() - 1) >> 1;

    // first all non master pages
    if( !pPage->IsMasterPage() )
    {
        if( bForeward )
        {
            if( nPageNum >= mpDoc->GetSdPageCount(PK_STANDARD)-1 )
            {
                // we reached end of draw pages, start with master pages (skip handout master for draw)
                return mpDoc->GetMasterSdPage( (mpDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS) ? 0 : 1, PK_STANDARD );
            }
            nPageNum++;
        }
        else
        {
            if( nPageNum == 0 )
                return 0; // we are already on the first draw page, finished

            nPageNum--;
        }
        return mpDoc->GetSdPage(nPageNum, PK_STANDARD);
    }
    else
    {
        if( bForeward )
        {
            if( nPageNum >= mpDoc->GetMasterSdPageCount(PK_STANDARD)-1 )
            {
                return 0;   // we reached the end, there is nothing more to see here
            }
            nPageNum++;
        }
        else
        {
            if( nPageNum == (mpDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS) ? 0 : 1 )
            {
                // we reached beginning of master pages, start with end if pages
                return mpDoc->GetSdPage( mpDoc->GetSdPageCount(PK_STANDARD)-1, PK_STANDARD );
            }

            nPageNum--;
        }
        return mpDoc->GetMasterSdPage(nPageNum,PK_STANDARD);
    }
}

SdPage* AnnotationManagerImpl::GetFirstPage()
{
    // return first drawing page
    return mpDoc->GetSdPage(0, PK_STANDARD );
}

SdPage* AnnotationManagerImpl::GetLastPage()
{
    return mpDoc->GetMasterSdPage( mpDoc->GetMasterSdPageCount(PK_STANDARD) - 1, PK_STANDARD );
}

SdPage* AnnotationManagerImpl::GetCurrentPage()
{
    return mrBase.GetMainViewShell()->getCurrentPage();
}

// ====================================================================

AnnotationManager::AnnotationManager( ViewShellBase& rViewShellBase )
: mxImpl( new AnnotationManagerImpl( rViewShellBase ) )
{
    mxImpl->init();
}

AnnotationManager::~AnnotationManager()
{
    mxImpl->dispose();
}

void AnnotationManager::ExecuteAnnotation(SfxRequest& rRequest)
{
    mxImpl->ExecuteAnnotation( rRequest );
}

void AnnotationManager::GetAnnotationState(SfxItemSet& rItemSet)
{
    mxImpl->GetAnnotationState(rItemSet);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
