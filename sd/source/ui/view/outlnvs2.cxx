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

#include <OutlineViewShell.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <app.hrc>
#include <svx/hlnkitem.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <svl/eitem.hxx>
#include <sfx2/zoomitem.hxx>
#include <svx/svxids.hrc>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/editstat.hxx>
#include <optsitem.hxx>
#include <unotools/useroptions.hxx>

#include <sfx2/viewfrm.hxx>
#include <Outliner.hxx>
#include <Window.hxx>
#include <fubullet.hxx>
#include <fuolbull.hxx>
#include <FrameView.hxx>
#include <fuzoom.hxx>
#include <fuscale.hxx>
#include <fuchar.hxx>
#include <fuinsfil.hxx>
#include <fuprobjs.hxx>
#include <futhes.hxx>
#include <futempl.hxx>
#include <fusldlg.hxx>
#include <zoomlist.hxx>
#include <fuexpand.hxx>
#include <fusumry.hxx>
#include <fucushow.hxx>
#include <drawdoc.hxx>
#include <sdattr.hxx>
#include <ViewShellBase.hxx>
#include <sdabstdlg.hxx>
#include <framework/FrameworkHelper.hxx>
#include <DrawDocShell.hxx>
#include <DrawViewShell.hxx>
#include <OutlineView.hxx>
#include <slideshow.hxx>
#include <memory>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

namespace sd {

/************************************************************************/

/**
 * SfxRequests for temporary functions
 */

void OutlineViewShell::FuTemporary(SfxRequest &rReq)
{
    DeactivateCurrentFunction();

    OutlinerView* pOutlinerView = pOlView->GetViewByWindow( GetActiveWindow() );
    sal_uInt16 nSId = rReq.GetSlot();

    switch( nSId )
    {
        case SID_ATTR_ZOOM:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            if ( pArgs )
            {
                SvxZoomType eZT = pArgs->Get( SID_ATTR_ZOOM ).GetType();
                switch( eZT )
                {
                    case SvxZoomType::PERCENT:
                        SetZoom( static_cast<long>( pArgs->Get( SID_ATTR_ZOOM ).GetValue()) );
                        Invalidate( SID_ATTR_ZOOM );
                        Invalidate( SID_ATTR_ZOOMSLIDER );
                        break;
                    default:
                        break;
                }
                rReq.Done();
            }
            else
            {
                // open the zoom dialog here
                SetCurrentFunction( FuScale::Create( this, GetActiveWindow(), pOlView.get(), GetDoc(), rReq ) );
            }
            Cancel();
        }
        break;

        case SID_ATTR_ZOOMSLIDER:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            const SfxUInt16Item* pScale = (pArgs && pArgs->Count () == 1) ?
                rReq.GetArg<SfxUInt16Item>(SID_ATTR_ZOOMSLIDER) : nullptr;
            if (pScale && CHECK_RANGE (5, pScale->GetValue (), 3000))
            {
                SetZoom (pScale->GetValue ());

                SfxBindings& rBindings = GetViewFrame()->GetBindings();
                rBindings.Invalidate( SID_ATTR_ZOOM );
                rBindings.Invalidate( SID_ZOOM_IN );
                rBindings.Invalidate( SID_ZOOM_OUT );
                rBindings.Invalidate( SID_ATTR_ZOOMSLIDER );

            }

            Cancel();
            rReq.Done ();
            break;
        }

        case SID_ZOOM_OUT:
        {
            SetZoom( std::min<long>( GetActiveWindow()->GetZoom() * 2, GetActiveWindow()->GetMaxZoom() ) );
            ::tools::Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( ::tools::Rectangle( Point(0,0),
                                             GetActiveWindow()->GetOutputSizePixel()) );
            mpZoomList->InsertZoomRect(aVisAreaWin);
            Invalidate( SID_ATTR_ZOOM );
            Invalidate( SID_ZOOM_IN );
            Invalidate( SID_ATTR_ZOOMSLIDER );
            Cancel();
            rReq.Done();
        }
        break;

        case SID_SIZE_REAL:
        {
            SetZoom( 100 );
            ::tools::Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( ::tools::Rectangle( Point(0,0),
                                             GetActiveWindow()->GetOutputSizePixel()) );
            mpZoomList->InsertZoomRect(aVisAreaWin);
            Invalidate( SID_ATTR_ZOOM );
            Invalidate( SID_ATTR_ZOOMSLIDER );
            Cancel();
            rReq.Done();
        }
        break;

        case SID_ZOOM_IN:
        {
            SetZoom( std::max<long>( GetActiveWindow()->GetZoom() / 2, GetActiveWindow()->GetMinZoom() ) );
            ::tools::Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( ::tools::Rectangle( Point(0,0),
                                             GetActiveWindow()->GetOutputSizePixel()) );
            mpZoomList->InsertZoomRect(aVisAreaWin);
            Invalidate( SID_ATTR_ZOOM );
            Invalidate( SID_ZOOM_OUT);
            Invalidate( SID_ZOOM_IN );
            Invalidate( SID_ATTR_ZOOMSLIDER );
            Cancel();
            rReq.Done();
        }
        break;

        case SID_OUTLINE_COLLAPSE_ALL:
        {
            pOutlinerView->CollapseAll();
            Cancel();
            rReq.Done();
        }
        break;

        case SID_OUTLINE_COLLAPSE:
        {
            pOutlinerView->Collapse();
            Cancel();
            rReq.Done();
        }
        break;

        case SID_OUTLINE_EXPAND_ALL:
        {
            pOutlinerView->ExpandAll();
            Cancel();
            rReq.Done();
        }
        break;

        case SID_OUTLINE_EXPAND:
        {
            pOutlinerView->Expand();
            Cancel();
            rReq.Done();
        }
        break;

        case SID_OUTLINE_FORMAT:
        {
            ::Outliner* pOutl = pOutlinerView->GetOutliner();
            pOutl->SetFlatMode( !pOutl->IsFlatMode() );
            Invalidate( SID_COLORVIEW );
            Cancel();
            rReq.Done();
        }
        break;

        case SID_SELECTALL:
        {
            ::Outliner& rOutl = pOlView->GetOutliner();
            sal_Int32 nParaCount = rOutl.GetParagraphCount();
            if (nParaCount > 0)
            {
                pOutlinerView->SelectRange( 0, nParaCount );
            }
            Cancel();
        }
        break;

        case SID_PRESENTATION:
        case SID_PRESENTATION_CURRENT_SLIDE:
        case SID_REHEARSE_TIMINGS:
        {
            pOlView->PrepareClose();
            slideshowhelp::ShowSlideShow(rReq, *GetDoc());
            Cancel();
            rReq.Done();
        }
        break;

        case SID_COLORVIEW:
        {
            ::Outliner* pOutl = pOutlinerView->GetOutliner();
            EEControlBits nCntrl = pOutl->GetControlWord();

            if ( !(nCntrl & EEControlBits::NOCOLORS) )
            {
                // color view is enabled: disable
                pOutl->SetControlWord(nCntrl | EEControlBits::NOCOLORS);
            }
            else
            {
                // color view is disabled: enable
                pOutl->SetControlWord(nCntrl & ~EEControlBits::NOCOLORS);
            }

            InvalidateWindows();
            Invalidate( SID_COLORVIEW );
            Cancel();
            rReq.Done();
        }
        break;

        case SID_STYLE_EDIT:
        case SID_STYLE_UPDATE_BY_EXAMPLE:
        {
            if( rReq.GetArgs() )
            {
                SetCurrentFunction( FuTemplate::Create( this, GetActiveWindow(), pOlView.get(), GetDoc(), rReq ) );
                Cancel();
            }

            rReq.Ignore ();
        }
        break;

        case SID_PRESENTATION_DLG:
        {
            SetCurrentFunction( FuSlideShowDlg::Create( this, GetActiveWindow(), pOlView.get(), GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_REMOTE_DLG:
        {
#ifdef ENABLE_SDREMOTE
             SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
             ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateRemoteDialog(GetActiveWindow()));
             pDlg->Execute();
#endif
        }
        break;

        case SID_CUSTOMSHOW_DLG:
        {
            SetCurrentFunction( FuCustomShowDlg::Create( this, GetActiveWindow(), pOlView.get(), GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_PHOTOALBUM:
        {
            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
            vcl::Window* pWin = GetActiveWindow();
            ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateSdPhotoAlbumDialog(
                pWin ? pWin->GetFrameWeld() : nullptr,
                GetDoc()));

            pDlg->Execute();

            Cancel();
            rReq.Ignore ();
        }
        break;
    }

    if(HasCurrentFunction())
        GetCurrentFunction()->Activate();

    Invalidate( SID_OUTLINE_COLLAPSE_ALL );
    Invalidate( SID_OUTLINE_COLLAPSE );
    Invalidate( SID_OUTLINE_EXPAND_ALL );
    Invalidate( SID_OUTLINE_EXPAND );

    SfxBindings& rBindings = GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_OUTLINE_LEFT );
    rBindings.Invalidate( SID_OUTLINE_RIGHT );
    rBindings.Invalidate( SID_OUTLINE_UP );
    rBindings.Invalidate( SID_OUTLINE_DOWN );

    Invalidate( SID_OUTLINE_FORMAT );
    Invalidate( SID_COLORVIEW );
    Invalidate(SID_CUT);
    Invalidate(SID_COPY);
    Invalidate(SID_PASTE);
    Invalidate(SID_PASTE_UNFORMATTED);
}

void OutlineViewShell::FuTemporaryModify(SfxRequest &rReq)
{
    sal_uInt16 nSId = rReq.GetSlot();
    std::unique_ptr<OutlineViewModelChangeGuard, o3tl::default_delete<OutlineViewModelChangeGuard>> aGuard;
    if (nSId != SID_OUTLINE_BULLET && nSId != FN_SVX_SET_BULLET && nSId != FN_SVX_SET_NUMBER)
    {
        aGuard.reset( new OutlineViewModelChangeGuard(*pOlView) );
    }
    DeactivateCurrentFunction();

    OutlinerView* pOutlinerView = pOlView->GetViewByWindow( GetActiveWindow() );
    //sal_uInt16 nSId = rReq.GetSlot();

    switch( nSId )
    {
        case SID_HYPERLINK_SETLINK:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if (pReqArgs)
            {
                const SvxHyperlinkItem* pHLItem =
                    &pReqArgs->Get(SID_HYPERLINK_SETLINK);

                SvxFieldItem aURLItem(SvxURLField(pHLItem->GetURL(),
                                                  pHLItem->GetName(),
                                                  SvxURLFormat::Repr), EE_FEATURE_FIELD);
                ESelection aSel( pOutlinerView->GetSelection() );
                pOutlinerView->InsertField(aURLItem);
                if ( aSel.nStartPos <= aSel.nEndPos )
                    aSel.nEndPos = aSel.nStartPos + 1;
                else
                    aSel.nStartPos = aSel.nEndPos + 1;
                pOutlinerView->SetSelection( aSel );
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case FN_INSERT_SOFT_HYPHEN:
        case FN_INSERT_HARDHYPHEN:
        case FN_INSERT_HARD_SPACE:
        case FN_INSERT_NNBSP:
        case SID_INSERT_RLM :
        case SID_INSERT_LRM :
        case SID_INSERT_ZWNBSP :
        case SID_INSERT_ZWSP:
        case SID_CHARMAP:
        {
            SetCurrentFunction( FuBullet::Create( this, GetActiveWindow(), pOlView.get(), GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_OUTLINE_BULLET:
        case FN_SVX_SET_BULLET:
        case FN_SVX_SET_NUMBER:
        {
            SetCurrentFunction( FuOutlineBullet::Create( this, GetActiveWindow(), pOlView.get(), GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_THESAURUS:
        {
            SetCurrentFunction( FuThesaurus::Create( this, GetActiveWindow(), pOlView.get(), GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_CHAR_DLG_EFFECT:
        case SID_CHAR_DLG:
        {
            SetCurrentFunction( FuChar::Create( this, GetActiveWindow(), pOlView.get(), GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_INSERTFILE:
        {
            SetCurrentFunction( FuInsertFile::Create(this, GetActiveWindow(), pOlView.get(), GetDoc(), rReq) );
            Cancel();
        }
        break;

        case SID_PRESENTATIONOBJECT:
        {
            SetCurrentFunction( FuPresentationObjects::Create(this, GetActiveWindow(), pOlView.get(), GetDoc(), rReq) );
            Cancel();
        }
        break;

        case SID_SET_DEFAULT:
        {
            pOutlinerView->RemoveAttribs(true); // sal_True = also paragraph attributes
            Cancel();
            rReq.Done();
        }
        break;

        case SID_SUMMARY_PAGE:
        {
            pOlView->SetSelectedPages();
            SetCurrentFunction( FuSummaryPage::Create( this, GetActiveWindow(), pOlView.get(), GetDoc(), rReq ) );
            pOlView->GetOutliner().Clear();
            pOlView->FillOutliner();
            pOlView->GetActualPage();
            Cancel();
        }
        break;

        case SID_EXPAND_PAGE:
        {
            pOlView->SetSelectedPages();
            SetCurrentFunction( FuExpandPage::Create( this, GetActiveWindow(), pOlView.get(), GetDoc(), rReq ) );
            pOlView->GetOutliner().Clear();
            pOlView->FillOutliner();
            pOlView->GetActualPage();
            Cancel();
        }
        break;

        case SID_INSERT_FLD_DATE_FIX:
        case SID_INSERT_FLD_DATE_VAR:
        case SID_INSERT_FLD_TIME_FIX:
        case SID_INSERT_FLD_TIME_VAR:
        case SID_INSERT_FLD_AUTHOR:
        case SID_INSERT_FLD_PAGE:
        case SID_INSERT_FLD_PAGE_TITLE:
        case SID_INSERT_FLD_PAGES:
        case SID_INSERT_FLD_FILE:
        {
            std::unique_ptr<SvxFieldItem> pFieldItem;

            switch( nSId )
            {
                case SID_INSERT_FLD_DATE_FIX:
                    pFieldItem.reset(new SvxFieldItem(
                        SvxDateField( Date( Date::SYSTEM ), SvxDateType::Fix ), EE_FEATURE_FIELD ));
                break;

                case SID_INSERT_FLD_DATE_VAR:
                    pFieldItem.reset(new SvxFieldItem( SvxDateField(), EE_FEATURE_FIELD ));
                break;

                case SID_INSERT_FLD_TIME_FIX:
                    pFieldItem.reset(new SvxFieldItem(
                        SvxExtTimeField( ::tools::Time( ::tools::Time::SYSTEM ), SvxTimeType::Fix ), EE_FEATURE_FIELD ));
                break;

                case SID_INSERT_FLD_TIME_VAR:
                    pFieldItem.reset(new SvxFieldItem( SvxExtTimeField(), EE_FEATURE_FIELD ));
                break;

                case SID_INSERT_FLD_AUTHOR:
                {
                    SvtUserOptions aUserOptions;
                    pFieldItem.reset(new SvxFieldItem(
                            SvxAuthorField(
                                aUserOptions.GetFirstName(), aUserOptions.GetLastName(), aUserOptions.GetID() )
                                , EE_FEATURE_FIELD ));
                }
                break;

                case SID_INSERT_FLD_PAGE:
                    pFieldItem.reset(new SvxFieldItem( SvxPageField(), EE_FEATURE_FIELD ));
                break;

                case SID_INSERT_FLD_PAGE_TITLE:
                    pFieldItem.reset(new SvxFieldItem( SvxPageTitleField(), EE_FEATURE_FIELD));
                break;

                case SID_INSERT_FLD_PAGES:
                    pFieldItem.reset(new SvxFieldItem( SvxPagesField(), EE_FEATURE_FIELD ));
                break;

                case SID_INSERT_FLD_FILE:
                {
                    OUString aName;
                    if( GetDocSh()->HasName() )
                        aName = GetDocSh()->GetMedium()->GetName();
                    //else
                    //  aName = GetDocSh()->GetName();
                    pFieldItem.reset(new SvxFieldItem( SvxExtFileField( aName ), EE_FEATURE_FIELD ));
                }
                break;
            }

            const SvxFieldItem* pOldFldItem = pOutlinerView->GetFieldAtSelection();

            if( pOldFldItem && ( nullptr != dynamic_cast< const SvxURLField *>( pOldFldItem->GetField() ) ||
                                nullptr != dynamic_cast< const SvxDateField *>( pOldFldItem->GetField() ) ||
                                nullptr != dynamic_cast< const SvxTimeField *>( pOldFldItem->GetField() ) ||
                                nullptr != dynamic_cast< const SvxExtTimeField *>( pOldFldItem->GetField() ) ||
                                nullptr != dynamic_cast< const SvxExtFileField *>( pOldFldItem->GetField() ) ||
                                nullptr != dynamic_cast< const SvxAuthorField *>( pOldFldItem->GetField() ) ||
                                nullptr != dynamic_cast< const SvxPageField *>( pOldFldItem->GetField() ) ||
                                nullptr != dynamic_cast< const SvxPagesField *>( pOldFldItem->GetField() )) )
            {
                // select field, so it gets deleted on Insert
                ESelection aSel = pOutlinerView->GetSelection();
                if( aSel.nStartPos == aSel.nEndPos )
                    aSel.nEndPos++;
                pOutlinerView->SetSelection( aSel );
            }

            if( pFieldItem )
                pOutlinerView->InsertField( *pFieldItem );

            pFieldItem.reset();

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_MODIFY_FIELD:
        {
            const SvxFieldItem* pFldItem = pOutlinerView->GetFieldAtSelection();

            if( pFldItem && (nullptr != dynamic_cast< const SvxDateField *>( pFldItem->GetField() ) ||
                                nullptr != dynamic_cast< const SvxAuthorField *>( pFldItem->GetField() ) ||
                                nullptr != dynamic_cast< const SvxExtFileField *>( pFldItem->GetField() ) ||
                                nullptr != dynamic_cast< const SvxExtTimeField *>( pFldItem->GetField() ) ) )
            {
                // Dialog...
                SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
                vcl::Window* pWin = GetActiveWindow();
                ScopedVclPtr<AbstractSdModifyFieldDlg> pDlg(pFact->CreateSdModifyFieldDlg(pWin ? pWin->GetFrameWeld() : nullptr, pFldItem->GetField(), pOutlinerView->GetAttribs() ));
                if( pDlg->Execute() == RET_OK )
                {
                    std::unique_ptr<SvxFieldData> pField(pDlg->GetField());
                    if( pField )
                    {
                        SvxFieldItem aFieldItem( *pField, EE_FEATURE_FIELD );
                        //pOLV->DeleteSelected(); <-- unfortunately missing!
                        // select field, so it gets deleted on Insert
                        ESelection aSel = pOutlinerView->GetSelection();
                        bool bSel = true;
                        if( aSel.nStartPos == aSel.nEndPos )
                        {
                            bSel = false;
                            aSel.nEndPos++;
                        }
                        pOutlinerView->SetSelection( aSel );

                        pOutlinerView->InsertField( aFieldItem );

                        // reset selection to original state
                        if( !bSel )
                            aSel.nEndPos--;
                        pOutlinerView->SetSelection( aSel );

                        pField.reset();
                    }

                    SfxItemSet aSet( pDlg->GetItemSet() );
                    if( aSet.Count() )
                    {
                        pOutlinerView->SetAttribs( aSet );

                        ::Outliner* pOutliner = pOutlinerView->GetOutliner();
                        if( pOutliner )
                            pOutliner->UpdateFields();
                    }
                }
            }

            Cancel();
            rReq.Ignore ();
        }
        break;
    }

    if(HasCurrentFunction())
        GetCurrentFunction()->Activate();

    Invalidate( SID_OUTLINE_COLLAPSE_ALL );
    Invalidate( SID_OUTLINE_COLLAPSE );
    Invalidate( SID_OUTLINE_EXPAND_ALL );
    Invalidate( SID_OUTLINE_EXPAND );

    SfxBindings& rBindings = GetViewFrame()->GetBindings();
    rBindings.Invalidate( SID_OUTLINE_LEFT );
    rBindings.Invalidate( SID_OUTLINE_RIGHT );
    rBindings.Invalidate( SID_OUTLINE_UP );
    rBindings.Invalidate( SID_OUTLINE_DOWN );

    Invalidate( SID_OUTLINE_FORMAT );
    Invalidate( SID_COLORVIEW );
    Invalidate(SID_CUT);
    Invalidate(SID_COPY);
    Invalidate(SID_PASTE);
    Invalidate(SID_PASTE_UNFORMATTED);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
