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


#include <SidebarWin.hxx>
#include <SidebarWinAcc.hxx>
#include <PostItMgr.hxx>

#include <SidebarTxtControl.hxx>
#include <AnchorOverlayObject.hxx>
#include <ShadowOverlayObject.hxx>

#include <annotation.hrc>
#include <popup.hrc>
#include <docvw.hrc>
#include <app.hrc>
#include <access.hrc>

#include <viewopt.hxx>
#include <cmdid.h>

#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editeng.hxx>
#include <editeng/eeitem.hxx>

#include <svl/zforlist.hxx>
#include <svl/undo.hxx>
#include <svl/stritem.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

#include <vcl/scrbar.hxx>
#include <vcl/svapp.hxx>
#include <vcl/menubtn.hxx>

#include <edtwin.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <doc.hxx>
#include <swmodule.hxx>
#include <langhelper.hxx>

#include <sw_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>

namespace sw { namespace sidebarwindows {

#define METABUTTON_WIDTH        16
#define METABUTTON_HEIGHT       18
#define METABUTTON_AREA_WIDTH   30
#define POSTIT_META_HEIGHT  (sal_Int32)     30
#define POSTIT_MINIMUMSIZE_WITHOUT_META     50

SwSidebarWin::SwSidebarWin( SwEditWin& rEditWin,
                            WinBits nBits,
                            SwPostItMgr& aMgr,
                            SwPostItBits aBits,
                            SwSidebarItem& rSidebarItem )
    : Window(&rEditWin, nBits)
    , mrMgr(aMgr)
    , mrView( rEditWin.GetView() )
    , nFlags(aBits)
    , mnEventId(0)
    , mpOutlinerView(0)
    , mpOutliner(0)
    , mpSidebarTxtControl(0)
    , mpVScrollbar(0)
    , mpMetadataAuthor(0)
    , mpMetadataDate(0)
    , mpMenuButton(0)
    , mpAnchor(0)
    , mpShadow(0)
    , mColorAnchor()
    , mColorDark()
    , mColorLight()
    , mChangeColor()
    , meSidebarPosition( sw::sidebarwindows::SIDEBAR_NONE )
    , mPosSize()
    , mAnchorRect()
    , mPageBorder( 0 )
    , mbMouseOver( false )
    , mLayoutStatus( SwPostItHelper::INVISIBLE )
    , mbReadonly( false )
    , mbIsFollow( false )
    , mrSidebarItem( rSidebarItem )
    , mpAnchorFrm( rSidebarItem.maLayoutInfo.mpAnchorFrm )
{
    mpShadow = ShadowOverlayObject::CreateShadowOverlayObject( mrView );
    if ( mpShadow )
    {
        mpShadow->setVisible(false);
    }

    mrMgr.ConnectSidebarWinToFrm( *(mrSidebarItem.maLayoutInfo.mpAnchorFrm),
                                  *(mrSidebarItem.GetFmtFld()),
                                  *this );
}

SwSidebarWin::~SwSidebarWin()
{
    mrMgr.DisconnectSidebarWinFromFrm( *(mrSidebarItem.maLayoutInfo.mpAnchorFrm),
                                       *this );

    Disable();

    if ( mpSidebarTxtControl )
    {
        if ( mpOutlinerView )
        {
            mpOutlinerView->SetWindow( 0 );
        }
        delete mpSidebarTxtControl;
        mpSidebarTxtControl = 0;
    }

    if ( mpOutlinerView )
    {
        delete mpOutlinerView;
        mpOutlinerView = 0;
    }

    if (mpOutliner)
    {
        delete mpOutliner;
        mpOutliner = 0;
    }

    if (mpMetadataAuthor)
    {
        mpMetadataAuthor->RemoveEventListener( LINK( this, SwSidebarWin, WindowEventListener ) );
        delete mpMetadataAuthor;
        mpMetadataAuthor = 0;
    }

    if (mpMetadataDate)
    {
        mpMetadataDate->RemoveEventListener( LINK( this, SwSidebarWin, WindowEventListener ) );
        delete mpMetadataDate;
        mpMetadataDate = 0;
    }

    if (mpVScrollbar)
    {
        mpVScrollbar->RemoveEventListener( LINK( this, SwSidebarWin, WindowEventListener ) );
        delete mpVScrollbar;
        mpVScrollbar = 0;
    }

    AnchorOverlayObject::DestroyAnchorOverlayObject( mpAnchor );
    mpAnchor = 0;

    ShadowOverlayObject::DestroyShadowOverlayObject( mpShadow );
    mpShadow = 0;

    delete mpMenuButton;
    mpMenuButton = 0;

    if (mnEventId)
        Application::RemoveUserEvent( mnEventId );
}

void SwSidebarWin::Paint( const Rectangle& rRect)
{
    Window::Paint(rRect);

    if (mpMetadataAuthor->IsVisible() )
    {
        //draw left over space
        if ( Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        {
            SetFillColor(COL_BLACK);
        }
        else
        {
            SetFillColor(mColorDark);
        }
        SetLineColor();
        DrawRect( PixelToLogic(
                Rectangle( Point( mpMetadataAuthor->GetPosPixel().X() +
                                    mpMetadataAuthor->GetSizePixel().Width(),
                                  mpMetadataAuthor->GetPosPixel().Y() ),
                           Size( GetMetaButtonAreaWidth(),
                                 mpMetadataAuthor->GetSizePixel().Height() +
                                    mpMetadataDate->GetSizePixel().Height() ) ) ) );
    }
}

void SwSidebarWin::SetPosSizePixelRect( long nX,
                                        long nY,
                                        long nWidth,
                                        long nHeight,
                                        const SwRect &aRect,
                                        const long aPageBorder)
{
    mPosSize = Rectangle(Point(nX,nY),Size(nWidth,nHeight));
    mAnchorRect = aRect;
    mPageBorder = aPageBorder;
}

void SwSidebarWin::SetSize( const Size& rNewSize )
{
    mPosSize.SetSize(rNewSize);
}

void SwSidebarWin::SetVirtualPosSize( const Point& aPoint, const Size& aSize)
{
    mPosSize = Rectangle(aPoint,aSize);
}

void SwSidebarWin::TranslateTopPosition(const long aAmount)
{
    mPosSize.Move(0,aAmount);
}

void SwSidebarWin::ShowAnchorOnly(const Point &aPoint)
{
    HideNote();
    SetPosAndSize();
    if (mpAnchor)
    {
        mpAnchor->SetSixthPosition(basegfx::B2DPoint(aPoint.X(),aPoint.Y()));
        mpAnchor->SetSeventhPosition(basegfx::B2DPoint(aPoint.X(),aPoint.Y()));
        mpAnchor->SetAnchorState(AS_ALL);
        mpAnchor->setVisible(true);
    }
    if (mpShadow)
        mpShadow->setVisible(false);
}

SfxItemSet SwSidebarWin::DefaultItem()
{
    SfxItemSet aItem( mrView.GetDocShell()->GetPool() );
    aItem.Put(SvxFontHeightItem(200,100,EE_CHAR_FONTHEIGHT));
    aItem.Put(SvxFontItem(FAMILY_SWISS,
                          GetSettings().GetStyleSettings().GetFieldFont().GetName(),
                          OUString(),
                          PITCH_DONTKNOW,
                          RTL_TEXTENCODING_DONTKNOW,
                          EE_CHAR_FONTINFO));
    return aItem;
}

void SwSidebarWin::InitControls()
{
    AddEventListener( LINK( this, SwSidebarWin, WindowEventListener ) );

    // actual window which holds the user text
    mpSidebarTxtControl = new SidebarTxtControl( *this,
                                                 WB_NODIALOGCONTROL,
                                                 mrView, mrMgr );
    mpSidebarTxtControl->SetPointer(Pointer(POINTER_TEXT));

    // window controls for author and date
    mpMetadataAuthor = new Edit( this, 0 );
    mpMetadataAuthor->SetAccessibleName( SW_RES( STR_ACCESS_ANNOTATION_AUTHOR_NAME ) );
    mpMetadataAuthor->EnableRTL(Application::GetSettings().GetLayoutRTL());
    mpMetadataAuthor->SetReadOnly();
    mpMetadataAuthor->AlwaysDisableInput(true);
    mpMetadataAuthor->SetCallHandlersOnInputDisabled(true);
    mpMetadataAuthor->AddEventListener( LINK( this, SwSidebarWin, WindowEventListener ) );
    // we should leave this setting alone, but for this we need a better layout algo
    // with variable meta size height
    {
        AllSettings aSettings = mpMetadataAuthor->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();
        Font aFont = aStyleSettings.GetFieldFont();
        aFont.SetHeight(8);
        aStyleSettings.SetFieldFont(aFont);
        aSettings.SetStyleSettings(aStyleSettings);
        mpMetadataAuthor->SetSettings(aSettings);
    }

    mpMetadataDate = new Edit( this, 0 );
    mpMetadataDate->SetAccessibleName( SW_RES( STR_ACCESS_ANNOTATION_DATE_NAME ) );
    mpMetadataDate->EnableRTL(Application::GetSettings().GetLayoutRTL());
    mpMetadataDate->SetReadOnly();
    mpMetadataDate->AlwaysDisableInput(true);
    mpMetadataDate->SetCallHandlersOnInputDisabled(true);
    mpMetadataDate->AddEventListener( LINK( this, SwSidebarWin, WindowEventListener ) );
    // we should leave this setting alone, but for this we need a better layout algo
    // with variable meta size height
    {
        AllSettings aSettings = mpMetadataDate->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();
        Font aFont = aStyleSettings.GetFieldFont();
        aFont.SetHeight(8);
        aStyleSettings.SetFieldFont(aFont);
        aSettings.SetStyleSettings(aStyleSettings);
        mpMetadataDate->SetSettings(aSettings);
    }


    SwDocShell* aShell = mrView.GetDocShell();
    mpOutliner = new Outliner(&aShell->GetPool(),OUTLINERMODE_TEXTOBJECT);
    aShell->GetDoc()->SetCalcFieldValueHdl( mpOutliner );
    mpOutliner->SetUpdateMode( sal_True );
    Rescale();

    mpSidebarTxtControl->EnableRTL( sal_False );
    mpOutlinerView = new OutlinerView ( mpOutliner, mpSidebarTxtControl );
    mpOutlinerView->SetBackgroundColor(COL_TRANSPARENT);
    mpOutliner->InsertView(mpOutlinerView );
    mpOutlinerView->SetOutputArea( PixelToLogic( Rectangle(0,0,1,1) ) );

    mpOutlinerView->SetAttribs(DefaultItem());

    //create Scrollbars
    mpVScrollbar = new ScrollBar(this, WB_3DLOOK |WB_VSCROLL|WB_DRAG);
    mpVScrollbar->EnableNativeWidget(false);
    mpVScrollbar->EnableRTL( false );
    mpVScrollbar->SetScrollHdl(LINK(this, SwSidebarWin, ScrollHdl));
    mpVScrollbar->EnableDrag();
    mpVScrollbar->AddEventListener( LINK( this, SwSidebarWin, WindowEventListener ) );

    const SwViewOption* pVOpt = mrView.GetWrtShellPtr()->GetViewOptions();
    sal_uLong nCntrl = mpOutliner->GetControlWord();
    // TODO: crash when AUTOCOMPLETE enabled
    nCntrl |= EE_CNTRL_MARKFIELDS | EE_CNTRL_PASTESPECIAL | EE_CNTRL_AUTOCORRECT  | EV_CNTRL_AUTOSCROLL | EE_CNTRL_URLSFXEXECUTE; // | EE_CNTRL_AUTOCOMPLETE;
    if (pVOpt->IsFieldShadings())
        nCntrl |= EE_CNTRL_MARKFIELDS;
    else
        nCntrl &= ~EE_CNTRL_MARKFIELDS;
    if (pVOpt->IsOnlineSpell())
        nCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;
    mpOutliner->SetControlWord(nCntrl);

    sal_uInt16 aIndex = SW_MOD()->InsertRedlineAuthor(GetAuthor());
    SetColor( mrMgr.GetColorDark(aIndex),
              mrMgr.GetColorLight(aIndex),
              mrMgr.GetColorAnchor(aIndex));

    CheckMetaText();

    mpMenuButton = CreateMenuButton();

    SetLanguage(GetLanguage());
    GetOutlinerView()->StartSpeller();
    SetPostItText();
    Engine()->CompleteOnlineSpelling();

    mpSidebarTxtControl->Show();
    mpMetadataAuthor->Show();
    mpMetadataDate->Show();
    mpVScrollbar->Show();
}

void SwSidebarWin::CheckMetaText()
{
    const SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocalData = aSysLocale.GetLocaleData();
    OUString sMeta = GetAuthor();
    if (sMeta.isEmpty())
    {
        sMeta = SW_RESSTR(STR_NOAUTHOR);
    }
    else if (sMeta.getLength() > 23)
    {
        sMeta = sMeta.copy(0, 20) + "...";
    }
    if ( mpMetadataAuthor->GetText() != sMeta )
    {
        mpMetadataAuthor->SetText(sMeta);
    }

    Date aSysDate( Date::SYSTEM );
    Date aDate = GetDate();
    if (aDate==aSysDate)
    {
        sMeta = SW_RESSTR(STR_POSTIT_TODAY);
    }
    else if (aDate == Date(aSysDate-1))
    {
        sMeta = SW_RESSTR(STR_POSTIT_YESTERDAY);
    }
    else if (aDate.IsValidAndGregorian() )
    {
        sMeta = rLocalData.getDate(aDate);
    }
    else
    {
        sMeta = SW_RESSTR(STR_NODATE);
    }
    if (GetTime()!=0)
    {
        sMeta += " " + rLocalData.getTime( GetTime(),false );
    }
    if ( mpMetadataDate->GetText() != sMeta )
    {
        mpMetadataDate->SetText(sMeta);
    }
}

void SwSidebarWin::Rescale()
{
    MapMode aMode = GetParent()->GetMapMode();
    aMode.SetOrigin( Point() );
    mpOutliner->SetRefMapMode( aMode );
    SetMapMode( aMode );
    mpSidebarTxtControl->SetMapMode( aMode );
    if ( mpMetadataAuthor )
    {
        Font aFont( mpMetadataAuthor->GetSettings().GetStyleSettings().GetFieldFont() );
        sal_Int32 nHeight = aFont.GetHeight();
        nHeight = nHeight * aMode.GetScaleY().GetNumerator() / aMode.GetScaleY().GetDenominator();
        aFont.SetHeight( nHeight );
        mpMetadataAuthor->SetControlFont( aFont );
    }
    if ( mpMetadataDate )
    {
        Font aFont( mpMetadataDate->GetSettings().GetStyleSettings().GetFieldFont() );
        sal_Int32 nHeight = aFont.GetHeight();
        nHeight = nHeight * aMode.GetScaleY().GetNumerator() / aMode.GetScaleY().GetDenominator();
        aFont.SetHeight( nHeight );
        mpMetadataDate->SetControlFont( aFont );
    }
}

void SwSidebarWin::SetPosAndSize()
{
    bool bChange = false;

    if (GetSizePixel() != mPosSize.GetSize())
    {
        bChange = true;
        SetSizePixel(mPosSize.GetSize());
        DoResize();
    }

    if (GetPosPixel().X() != mPosSize.TopLeft().X() || (abs(GetPosPixel().Y() - mPosSize.TopLeft().Y()) > 5) )
    {
        bChange = true;
        SetPosPixel(mPosSize.TopLeft());

        Point aLineStart;
        Point aLineEnd ;
        switch ( meSidebarPosition )
        {
            case sw::sidebarwindows::SIDEBAR_LEFT:
            {
                aLineStart = EditWin()->PixelToLogic( Point(GetPosPixel().X()+GetSizePixel().Width(),GetPosPixel().Y()-1) );
                aLineEnd = EditWin()->PixelToLogic( Point(GetPosPixel().X(),GetPosPixel().Y()-1) );
            }
            break;
            case sw::sidebarwindows::SIDEBAR_RIGHT:
            {
                aLineStart = EditWin()->PixelToLogic( Point(GetPosPixel().X(),GetPosPixel().Y()-1) );
                aLineEnd = EditWin()->PixelToLogic( Point(GetPosPixel().X()+GetSizePixel().Width(),GetPosPixel().Y()-1) );
            }
            break;
            default:
                OSL_FAIL( "<SwSidebarWin::SetPosAndSize()> - unexpected position of sidebar" );
            break;
        }

        if (!IsPreview())
        {
            if (mpAnchor)
            {
                mpAnchor->SetAllPosition( basegfx::B2DPoint( mAnchorRect.Left() , mAnchorRect.Bottom() - 5* 15),
                                          basegfx::B2DPoint( mAnchorRect.Left()-5*15 , mAnchorRect.Bottom()+5*15),
                                          basegfx::B2DPoint( mAnchorRect.Left()+5*15 , mAnchorRect.Bottom()+5*15),
                                          basegfx::B2DPoint( mAnchorRect.Left(), mAnchorRect.Bottom()+2*15),
                                          basegfx::B2DPoint( mPageBorder ,mAnchorRect.Bottom()+2*15),
                                          basegfx::B2DPoint( aLineStart.X(),aLineStart.Y()),
                                          basegfx::B2DPoint( aLineEnd.X(),aLineEnd.Y()));
                mpAnchor->SetHeight(mAnchorRect.Height());
            }
            else
            {
                mpAnchor = AnchorOverlayObject::CreateAnchorOverlayObject( mrView,
                                                                           mAnchorRect,
                                                                           mPageBorder,
                                                                           aLineStart,
                                                                           aLineEnd,
                                                                           mColorAnchor );
                if ( mpAnchor )
                {
                    mpAnchor->SetHeight(mAnchorRect.Height());
                    mpAnchor->setVisible(true);
                    mpAnchor->SetAnchorState(AS_TRI);
                    if (HasChildPathFocus())
                    {
                        mpAnchor->setLineSolid(true);
                    }
                }
            }
        }
    }
    else
    {
        if ( mpAnchor &&
             ( mpAnchor->getBasePosition() != basegfx::B2DPoint( mAnchorRect.Left() , mAnchorRect.Bottom()-5*15) ) )
        {
            mpAnchor->SetTriPosition( basegfx::B2DPoint( mAnchorRect.Left() , mAnchorRect.Bottom() - 5* 15),
                                      basegfx::B2DPoint( mAnchorRect.Left()-5*15 , mAnchorRect.Bottom()+5*15),
                                      basegfx::B2DPoint( mAnchorRect.Left()+5*15 , mAnchorRect.Bottom()+5*15),
                                      basegfx::B2DPoint( mAnchorRect.Left(), mAnchorRect.Bottom()+2*15),
                                      basegfx::B2DPoint( mPageBorder , mAnchorRect.Bottom()+2*15));
        }
    }

    if (bChange)
    {
        Point aStart = EditWin()->PixelToLogic(GetPosPixel()+Point(0,GetSizePixel().Height()));
        Point aEnd = EditWin()->PixelToLogic(GetPosPixel()+Point(GetSizePixel().Width()-1,GetSizePixel().Height()));
        mpShadow->SetPosition(basegfx::B2DPoint(aStart.X(),aStart.Y()), basegfx::B2DPoint(aEnd.X(),aEnd.Y()));
    }

    if (mrMgr.ShowNotes())
    {
        if (IsFollow() && !HasChildPathFocus())
        {
            // #i111964#
            if ( mpAnchor )
            {
                mpAnchor->SetAnchorState(AS_END);
            }
        }
        else
        {
            // #i111964#
            if ( mpAnchor )
            {
                mpAnchor->SetAnchorState(AS_ALL);
            }
            SwSidebarWin* pWin = GetTopReplyNote();
            // #i111964#
            if ( pWin && pWin->Anchor() )
            {
                pWin->Anchor()->SetAnchorState(AS_END);
            }
        }
    }
}

void SwSidebarWin::DoResize()
{
    long aTextHeight    =  LogicToPixel( mpOutliner->CalcTextSize()).Height();
    long aHeight        =  GetSizePixel().Height();
    unsigned long aWidth    =  GetSizePixel().Width();

    aHeight -= GetMetaHeight();
    mpMetadataAuthor->Show();
    mpMetadataDate->Show();
    mpSidebarTxtControl->SetQuickHelpText(OUString());

    if ((aTextHeight > aHeight) && !IsPreview())
    {   // we need vertical scrollbars and have to reduce the width
        aWidth -= GetScrollbarWidth();
        mpVScrollbar->Show();
    }
    else
    {
        mpVScrollbar->Hide();
    }

    {
        const Size aSizeOfMetadataControls( GetSizePixel().Width() - GetMetaButtonAreaWidth(),
                                            GetMetaHeight()/2 );
        mpMetadataAuthor->setPosSizePixel( 0,
                                           aHeight,
                                           aSizeOfMetadataControls.Width(),
                                           aSizeOfMetadataControls.Height() );
        mpMetadataDate->setPosSizePixel( 0,
                                         aHeight + aSizeOfMetadataControls.Height(),
                                         aSizeOfMetadataControls.Width(),
                                         aSizeOfMetadataControls.Height() );
    }

    mpOutliner->SetPaperSize( PixelToLogic( Size(aWidth,aHeight) ) ) ;
    if (!mpVScrollbar->IsVisible())
    {   // if we do not have a scrollbar anymore, we want to see the complete text
        mpOutlinerView->SetVisArea( PixelToLogic( Rectangle(0,0,aWidth,aHeight) ) );
    }
    mpOutlinerView->SetOutputArea( PixelToLogic( Rectangle(0,0,aWidth,aHeight) ) );

    if (!Application::GetSettings().GetLayoutRTL())
    {
        mpSidebarTxtControl->setPosSizePixel(0, 0, aWidth, aHeight);
        mpVScrollbar->setPosSizePixel( aWidth, 0, GetScrollbarWidth(), aHeight);
    }
    else
    {
        mpSidebarTxtControl->setPosSizePixel( ( (aTextHeight > aHeight) && !IsPreview()
                                      ? GetScrollbarWidth() : 0 ) , 0,
                                      aWidth, aHeight);
        mpVScrollbar->setPosSizePixel( 0, 0, GetScrollbarWidth(), aHeight);
    }

    mpVScrollbar->SetVisibleSize( PixelToLogic(Size(0,aHeight)).Height() );
    mpVScrollbar->SetPageSize( PixelToLogic(Size(0,aHeight)).Height() * 8 / 10 );
    mpVScrollbar->SetLineSize( mpOutliner->GetTextHeight() / 10 );
    SetScrollbar();
    mpVScrollbar->SetRange( Range(0, mpOutliner->GetTextHeight()));

    //calculate rects for meta- button
    const Fraction& fx( GetMapMode().GetScaleX() );
    const Fraction& fy( GetMapMode().GetScaleY() );

    const Point aPos( mpMetadataAuthor->GetPosPixel());
    Rectangle aRectMetaButton;
    if (IsPreview())
    {
        aRectMetaButton = PixelToLogic(
            Rectangle( Point( aPos.X()+GetSizePixel().Width()-(METABUTTON_WIDTH*4+10)*fx.GetNumerator()/fx.GetDenominator(),
                              aPos.Y()+5*fy.GetNumerator()/fy.GetDenominator() ),
                       Size( METABUTTON_WIDTH*4*fx.GetNumerator()/fx.GetDenominator(),
                             METABUTTON_HEIGHT*fy.GetNumerator()/fy.GetDenominator() ) ) );
    }
    else
    {
        aRectMetaButton = PixelToLogic(
            Rectangle( Point( aPos.X()+GetSizePixel().Width()-(METABUTTON_WIDTH+10)*fx.GetNumerator()/fx.GetDenominator(),
                              aPos.Y()+5*fy.GetNumerator()/fy.GetDenominator() ),
                       Size( METABUTTON_WIDTH*fx.GetNumerator()/fx.GetDenominator(),
                             METABUTTON_HEIGHT*fy.GetNumerator()/fy.GetDenominator() ) ) );
    }

    {
        const Rectangle aRectMetaButtonPixel( LogicToPixel( aRectMetaButton ) );
        mpMenuButton->setPosSizePixel( aRectMetaButtonPixel.Left(),
                                       aRectMetaButtonPixel.Top(),
                                       aRectMetaButtonPixel.GetWidth(),
                                       aRectMetaButtonPixel.GetHeight() );
    }
}

void SwSidebarWin::SetSizePixel( const Size& rNewSize )
{
    Window::SetSizePixel(rNewSize);

    if (mpShadow)
    {
        Point aStart = EditWin()->PixelToLogic(GetPosPixel()+Point(0,GetSizePixel().Height()));
        Point aEnd = EditWin()->PixelToLogic(GetPosPixel()+Point(GetSizePixel().Width()-1,GetSizePixel().Height()));
        mpShadow->SetPosition(basegfx::B2DPoint(aStart.X(),aStart.Y()), basegfx::B2DPoint(aEnd.X(),aEnd.Y()));
    }
}

void SwSidebarWin::SetScrollbar()
{
    mpVScrollbar->SetThumbPos( mpOutlinerView->GetVisArea().Top()+ mpOutlinerView->GetEditView().GetCursor()->GetOffsetY());
}

void SwSidebarWin::ResizeIfNeccessary(long aOldHeight, long aNewHeight)
{
    if (aOldHeight != aNewHeight)
    {
        //check for lower border or next note
        long aBorder = mrMgr.GetNextBorder();
        if (aBorder != -1)
        {
            if (aNewHeight > GetMinimumSizeWithoutMeta())
            {
                long aNewLowerValue = GetPosPixel().Y() + aNewHeight + GetMetaHeight();
                if (aNewLowerValue < aBorder)
                    SetSizePixel(Size(GetSizePixel().Width(),aNewHeight+GetMetaHeight()));
                else
                    SetSizePixel(Size(GetSizePixel().Width(),aBorder - GetPosPixel().Y()));
                DoResize();
                Invalidate();
            }
            else
            {
                if (GetSizePixel().Height() != GetMinimumSizeWithoutMeta() + GetMetaHeight())
                    SetSizePixel(Size(GetSizePixel().Width(),GetMinimumSizeWithoutMeta() + GetMetaHeight()));
                DoResize();
                Invalidate();
            }
        }
        else
        {
            DoResize();
            Invalidate();
        }
    }
    else
    {
        SetScrollbar();
    }
}

void SwSidebarWin::SetColor(Color aColorDark,Color aColorLight, Color aColorAnchor)
{
    mColorDark =  aColorDark;
    mColorLight = aColorLight;
    mColorAnchor = aColorAnchor;

    if ( !Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        {
            mpMetadataAuthor->SetControlBackground(mColorDark);
            AllSettings aSettings = mpMetadataAuthor->GetSettings();
            StyleSettings aStyleSettings = aSettings.GetStyleSettings();
            aStyleSettings.SetFieldTextColor(aColorAnchor);
            aSettings.SetStyleSettings(aStyleSettings);
            mpMetadataAuthor->SetSettings(aSettings);
        }

        {
            mpMetadataDate->SetControlBackground(mColorDark);
            AllSettings aSettings = mpMetadataDate->GetSettings();
            StyleSettings aStyleSettings = aSettings.GetStyleSettings();
            aStyleSettings.SetFieldTextColor(aColorAnchor);
            aSettings.SetStyleSettings(aStyleSettings);
            mpMetadataDate->SetSettings(aSettings);
        }

        AllSettings aSettings2 = mpVScrollbar->GetSettings();
        StyleSettings aStyleSettings2 = aSettings2.GetStyleSettings();
        aStyleSettings2.SetButtonTextColor(Color(0,0,0));
        aStyleSettings2.SetCheckedColor(mColorLight); // backgound
        aStyleSettings2.SetShadowColor(mColorAnchor);
        aStyleSettings2.SetFaceColor(mColorDark);
        aSettings2.SetStyleSettings(aStyleSettings2);
        mpVScrollbar->SetSettings(aSettings2);
    }
}

void SwSidebarWin::SetSidebarPosition(sw::sidebarwindows::SidebarPosition eSidebarPosition)
{
    meSidebarPosition = eSidebarPosition;
}

void SwSidebarWin::SetReadonly(sal_Bool bSet)
{
    mbReadonly = bSet;
    GetOutlinerView()->SetReadOnly(bSet);
}

void SwSidebarWin::SetLanguage(const SvxLanguageItem aNewItem)
{
    Link pLink = Engine()->GetModifyHdl();
    Engine()->SetModifyHdl( Link() );
    ESelection aOld = GetOutlinerView()->GetSelection();

    ESelection aNewSelection( 0, 0, Engine()->GetParagraphCount()-1, EE_TEXTPOS_ALL );
    GetOutlinerView()->SetSelection( aNewSelection );
    SfxItemSet aEditAttr(GetOutlinerView()->GetAttribs());
    aEditAttr.Put(aNewItem);
    GetOutlinerView()->SetAttribs( aEditAttr );

    GetOutlinerView()->SetSelection(aOld);
    Engine()->SetModifyHdl( pLink );

    const SwViewOption* pVOpt = mrView.GetWrtShellPtr()->GetViewOptions();
    sal_uLong nCntrl = Engine()->GetControlWord();
    // turn off
    nCntrl &= ~EE_CNTRL_ONLINESPELLING;
    Engine()->SetControlWord(nCntrl);

    //turn back on
    if (pVOpt->IsOnlineSpell())
        nCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;
    Engine()->SetControlWord(nCntrl);

    Engine()->CompleteOnlineSpelling();
    Invalidate();
}

void SwSidebarWin::DataChanged( const DataChangedEvent& aEvent)
{
    Window::DataChanged( aEvent );
}

void SwSidebarWin::GetFocus()
{
    if (mpSidebarTxtControl)
        mpSidebarTxtControl->GrabFocus();
}

void SwSidebarWin::LoseFocus()
{
}

void SwSidebarWin::ShowNote()
{
    SetPosAndSize();
    if (!IsVisible())
        Window::Show();
    if (mpShadow && !mpShadow->isVisible())
        mpShadow->setVisible(true);
    if (mpAnchor && !mpAnchor->isVisible())
        mpAnchor->setVisible(true);
}

void SwSidebarWin::HideNote()
{
    if (IsVisible())
        Window::Hide();
    if (mpAnchor)
    {
        if (mrMgr.IsShowAnchor())
            mpAnchor->SetAnchorState(AS_TRI);
        else
            mpAnchor->setVisible(false);
    }
    if (mpShadow && mpShadow->isVisible())
        mpShadow->setVisible(false);
}

void SwSidebarWin::ActivatePostIt()
{
    mrMgr.AssureStdModeAtShell();

    mpOutliner->ClearModifyFlag();
    mpOutliner->GetUndoManager().Clear();

    CheckMetaText();
    SetViewState(VS_EDIT);
    GetOutlinerView()->ShowCursor();

    mpOutlinerView->GetEditView().SetInsertMode(mrView.GetWrtShellPtr()->IsInsMode());

    if ( !Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        GetOutlinerView()->SetBackgroundColor(mColorDark);
}

void SwSidebarWin::DeactivatePostIt()
{
    // remove selection, #i87073#
    if (GetOutlinerView()->GetEditView().HasSelection())
    {
        ESelection aSelection = GetOutlinerView()->GetEditView().GetSelection();
        aSelection.nEndPara = aSelection.nStartPara;
        aSelection.nEndPos = aSelection.nStartPos;
        GetOutlinerView()->GetEditView().SetSelection(aSelection);
    }

    mpOutliner->CompleteOnlineSpelling();

    SetViewState(VS_NORMAL);
    // write the visible text back into the SwField
    UpdateData();

    if ( !Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        GetOutlinerView()->SetBackgroundColor(COL_TRANSPARENT);


    if ( !IsProtected() && Engine()->GetEditEngine().GetText().isEmpty() )
    {
        mnEventId = Application::PostUserEvent( LINK( this, SwSidebarWin, DeleteHdl), 0 );
    }
}

void SwSidebarWin::ToggleInsMode()
{
    if (!mrView.GetWrtShell().IsRedlineOn())
    {
        //change outliner
        mpOutlinerView->GetEditView().SetInsertMode(!mpOutlinerView->GetEditView().IsInsertMode());
        //change document
        mrView.GetWrtShell().ToggleInsMode();
        //update statusbar
        SfxBindings &rBnd = mrView.GetViewFrame()->GetBindings();
        rBnd.Invalidate(SID_ATTR_INSERT);
        rBnd.Update(SID_ATTR_INSERT);
    }
}

void SwSidebarWin::ExecuteCommand(sal_uInt16 nSlot)
{
    mrMgr.AssureStdModeAtShell();

    switch (nSlot)
    {
        case FN_POSTIT:
        case FN_REPLY:
        {
            // if this note is empty, it will be deleted once losing the focus, so no reply, but only a new note
            // will be created
            if (!Engine()->GetEditEngine().GetText().isEmpty())
            {
                OutlinerParaObject* pPara = new OutlinerParaObject(*GetOutlinerView()->GetEditView().CreateTextObject());
                mrMgr.RegisterAnswer(pPara);
            }
            if (mrMgr.HasActiveSidebarWin())
                mrMgr.SetActiveSidebarWin(0);
            SwitchToFieldPos();
            mrView.GetViewFrame()->GetDispatcher()->Execute(FN_POSTIT);
            break;
        }
        case FN_DELETE_COMMENT:

                //Delete(); // do not kill the parent of our open popup menu
                mnEventId = Application::PostUserEvent( LINK( this, SwSidebarWin, DeleteHdl), 0 );
            break;
        case FN_DELETE_ALL_NOTES:
        case FN_HIDE_ALL_NOTES:
            // not possible as slot as this would require that "this" is the active postit
            mrView.GetViewFrame()->GetBindings().Execute( nSlot, 0, 0, SFX_CALLMODE_ASYNCHRON );
            break;
        case FN_DELETE_NOTE_AUTHOR:
        case FN_HIDE_NOTE_AUTHOR:
        {
            // not possible as slot as this would require that "this" is the active postit
            SfxStringItem aItem( nSlot, GetAuthor() );
            const SfxPoolItem* aItems[2];
            aItems[0] = &aItem;
            aItems[1] = 0;
            mrView.GetViewFrame()->GetBindings().Execute( nSlot, aItems, 0, SFX_CALLMODE_ASYNCHRON );
        }
            break;
        default:
            mrView.GetViewFrame()->GetBindings().Execute( nSlot );
            break;
    }
}

SwEditWin*  SwSidebarWin::EditWin()
{
    return &mrView.GetEditWin();
}

long SwSidebarWin::GetPostItTextHeight()
{
    return mpOutliner ? LogicToPixel(mpOutliner->CalcTextSize()).Height() : 0;
}

void SwSidebarWin::SwitchToPostIt(sal_uInt16 aDirection)
{
    SwSidebarWin* pPostIt = mrMgr.GetNextPostIt(aDirection, this);
    if (pPostIt)
        pPostIt->GrabFocus();
}

IMPL_LINK( SwSidebarWin, WindowEventListener, VclSimpleEvent*, pEvent )
{
    VclWindowEvent* pWinEvent = dynamic_cast<VclWindowEvent*>(pEvent);
    if ( pWinEvent )
    {
        if ( pWinEvent->GetId() == VCLEVENT_WINDOW_MOUSEMOVE )
        {
            MouseEvent* pMouseEvt = (MouseEvent*)pWinEvent->GetData();
            if ( pMouseEvt->IsEnterWindow() )
            {
                mbMouseOver = true;
                if ( !HasFocus() )
                {
                    SetViewState(VS_VIEW);
                    Invalidate();
                }
            }
            else if ( pMouseEvt->IsLeaveWindow())
            {
                if (!IsPreview())
                {
                    mbMouseOver = false;
                    if ( !HasFocus() )
                    {
                        SetViewState(VS_NORMAL);
                        Invalidate();
                    }
                }
            }
        }
        else if ( pWinEvent->GetId() == VCLEVENT_WINDOW_ACTIVATE &&
                  pWinEvent->GetWindow() == mpSidebarTxtControl )
        {
            const sal_Bool bLockView = mrView.GetWrtShell().IsViewLocked();
            mrView.GetWrtShell().LockView( sal_True );

            if ( !IsPreview() )
            {
                mrMgr.SetActiveSidebarWin( this );
            }

            mrView.GetWrtShell().LockView( bLockView );
            mrMgr.MakeVisible( this );
        }
    }
    return sal_True;
}

void SwSidebarWin::Delete()
{
    if ( mrMgr.GetActiveSidebarWin() == this)
    {
        mrMgr.SetActiveSidebarWin(0);
        // if the note is empty, the previous line will send a delete event, but we are already there
        if (mnEventId)
        {
            Application::RemoveUserEvent( mnEventId );
            mnEventId = 0;
        }
    }
}

IMPL_LINK(SwSidebarWin, ScrollHdl, ScrollBar*, pScroll)
{
    long nDiff = GetOutlinerView()->GetEditView().GetVisArea().Top() - pScroll->GetThumbPos();
    GetOutlinerView()->Scroll( 0, nDiff );
    return 0;
}

IMPL_LINK_NOARG(SwSidebarWin, ModifyHdl)
{
    mrView.GetDocShell()->SetModified(sal_True);
    return 0;
}

IMPL_LINK_NOARG(SwSidebarWin, DeleteHdl)
{
    mnEventId = 0;
    Delete();
    return 0;
}


void SwSidebarWin::ResetAttributes()
{
    mpOutlinerView->RemoveAttribsKeepLanguages(sal_True);
    mpOutliner->RemoveFields(sal_True);
    mpOutlinerView->SetAttribs(DefaultItem());
}

sal_Int32 SwSidebarWin::GetScrollbarWidth()
{
    return mrView.GetWrtShell().GetViewOptions()->GetZoom() / 10;
}

sal_Int32 SwSidebarWin::GetMetaButtonAreaWidth()
{
    const Fraction& f( GetMapMode().GetScaleX() );
    if (IsPreview())
        return 3 * METABUTTON_AREA_WIDTH * f.GetNumerator() / f.GetDenominator();
    else
        return METABUTTON_AREA_WIDTH * f.GetNumerator() / f.GetDenominator();
}

sal_Int32 SwSidebarWin::GetMetaHeight()
{
    const Fraction& f( GetMapMode().GetScaleY() );
    return POSTIT_META_HEIGHT * f.GetNumerator() / f.GetDenominator();
}

sal_Int32 SwSidebarWin::GetMinimumSizeWithMeta()
{
    return mrMgr.GetMinimumSizeWithMeta();
}

sal_Int32 SwSidebarWin::GetMinimumSizeWithoutMeta()
{
    const Fraction& f( GetMapMode().GetScaleY() );
    return POSTIT_MINIMUMSIZE_WITHOUT_META * f.GetNumerator() / f.GetDenominator();
}

void SwSidebarWin::SetSpellChecking()
{
    const SwViewOption* pVOpt = mrView.GetWrtShellPtr()->GetViewOptions();
    sal_uLong nCntrl = mpOutliner->GetControlWord();
    if (pVOpt->IsOnlineSpell())
        nCntrl |= EE_CNTRL_ONLINESPELLING;
    else
        nCntrl &= ~EE_CNTRL_ONLINESPELLING;
    mpOutliner->SetControlWord(nCntrl);

    mpOutliner->CompleteOnlineSpelling();
    Invalidate();
}

void SwSidebarWin::SetViewState(ViewState bViewState)
{
    switch (bViewState)
    {
        case VS_EDIT:
        {
            if (mpAnchor)
            {
                mpAnchor->SetAnchorState(AS_ALL);
                SwSidebarWin* pWin = GetTopReplyNote();
                // #i111964#
                if ( pWin && pWin->Anchor() )
                {
                    pWin->Anchor()->SetAnchorState(AS_END);
                }
                mpAnchor->setLineSolid(true);
            }
            if (mpShadow)
                mpShadow->SetShadowState(SS_EDIT);
            break;
        }
        case VS_VIEW:
        {
            if (mpAnchor)
                mpAnchor->setLineSolid(true);
            if (mpShadow)
                mpShadow->SetShadowState(SS_VIEW);
            break;
        }
        case VS_NORMAL:
        {
            if (mpAnchor)
            {
                if (IsFollow())
                {
                    // if there is no visible parent note, we want to see the complete anchor ??
                    //if (IsAnyStackParentVisible())
                    mpAnchor->SetAnchorState(AS_END);
                    SwSidebarWin* pTopWinSelf = GetTopReplyNote();
                    SwSidebarWin* pTopWinActive = mrMgr.HasActiveSidebarWin()
                                                  ? mrMgr.GetActiveSidebarWin()->GetTopReplyNote()
                                                  : 0;
                    // #i111964#
                    if ( pTopWinSelf && ( pTopWinSelf != pTopWinActive ) &&
                         pTopWinSelf->Anchor() )
                    {
                        if ( pTopWinSelf != mrMgr.GetActiveSidebarWin() )
                        {
                            pTopWinSelf->Anchor()->setLineSolid(false);
                        }
                        pTopWinSelf->Anchor()->SetAnchorState(AS_ALL);
                    }
                }
                mpAnchor->setLineSolid(false);
            }
            if ( mpShadow )
            {
                mpShadow->SetShadowState(SS_NORMAL);
            }
            break;
        }
    }
}

SwSidebarWin* SwSidebarWin::GetTopReplyNote()
{
    SwSidebarWin* pTopNote = 0;
    SwSidebarWin* pSidebarWin = IsFollow() ? mrMgr.GetNextPostIt(KEY_PAGEUP, this) : 0;
    while (pSidebarWin)
    {
        pTopNote = pSidebarWin;
        pSidebarWin = pSidebarWin->IsFollow() ? mrMgr.GetNextPostIt(KEY_PAGEUP, pSidebarWin) : 0;
    }
    return pTopNote;
}

void SwSidebarWin::SwitchToFieldPos()
{
    if ( mrMgr.GetActiveSidebarWin() == this )
            mrMgr.SetActiveSidebarWin(0);
    GotoPos();
    sal_uInt32 aCount = MoveCaret();
    if (aCount)
        mrView.GetDocShell()->GetWrtShell()->SwCrsrShell::Right(aCount, 0, sal_False);
    GrabFocusToDocument();
}

SvxLanguageItem SwSidebarWin::GetLanguage(void)
{
    return SvxLanguageItem(SwLangHelper::GetLanguage(mrView.GetWrtShell(),RES_CHRATR_LANGUAGE),RES_CHRATR_LANGUAGE);
}

void SwSidebarWin::SetChangeTracking( const SwPostItHelper::SwLayoutStatus aLayoutStatus,
                                      const Color& aChangeColor )
{
    if ( (mLayoutStatus != aLayoutStatus) ||
         (mChangeColor != aChangeColor) )
    {
        mLayoutStatus = aLayoutStatus;
        mChangeColor = aChangeColor;
        Invalidate();
    }
}

bool SwSidebarWin::HasScrollbar() const
{
    return mpVScrollbar != 0;
}

bool SwSidebarWin::IsScrollbarVisible() const
{
    return HasScrollbar() && mpVScrollbar->IsVisible();
}

void SwSidebarWin::ChangeSidebarItem( SwSidebarItem& rSidebarItem )
{
    const bool bAnchorChanged = mpAnchorFrm != rSidebarItem.maLayoutInfo.mpAnchorFrm;
    if ( bAnchorChanged )
    {
        mrMgr.DisconnectSidebarWinFromFrm( *(mpAnchorFrm), *this );
    }

    mrSidebarItem = rSidebarItem;
    mpAnchorFrm = mrSidebarItem.maLayoutInfo.mpAnchorFrm;

    if ( GetWindowPeer() )
    {
        SidebarWinAccessible* pAcc =
                        static_cast<SidebarWinAccessible*>( GetWindowPeer() );
        OSL_ENSURE( dynamic_cast<SidebarWinAccessible*>( GetWindowPeer() ),
                "<SwSidebarWin::ChangeSidebarItem(..)> - unexpected type of window peer -> crash possible!" );
        pAcc->ChangeSidebarItem( mrSidebarItem );
    }

    if ( bAnchorChanged )
    {
        mrMgr.ConnectSidebarWinToFrm( *(mrSidebarItem.maLayoutInfo.mpAnchorFrm),
                                      *(mrSidebarItem.GetFmtFld()),
                                      *this );
    }
}

css::uno::Reference< css::accessibility::XAccessible > SwSidebarWin::CreateAccessible()
{
    SidebarWinAccessible* pAcc( new SidebarWinAccessible( *this,
                                                          mrView.GetWrtShell(),
                                                          mrSidebarItem ) );
    css::uno::Reference< css::awt::XWindowPeer > xWinPeer( pAcc );
    SetWindowPeer( xWinPeer, pAcc );

    css::uno::Reference< css::accessibility::XAccessible > xAcc( xWinPeer, css::uno::UNO_QUERY );
    return xAcc;
}

} } // eof of namespace sw::sidebarwindows

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
