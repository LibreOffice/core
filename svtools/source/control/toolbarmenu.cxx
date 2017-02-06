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

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <comphelper/processfactory.hxx>

#include <o3tl/make_unique.hxx>

#include <vcl/dockwin.hxx>
#include <vcl/decoview.hxx>
#include <vcl/image.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/settings.hxx>

#include <svtools/framestatuslistener.hxx>
#include <svtools/valueset.hxx>
#include <svtools/toolbarmenu.hxx>
#include "toolbarmenuimp.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::accessibility;

namespace svtools {

static vcl::Window* GetTopMostParentSystemWindow( vcl::Window* pWindow )
{
    OSL_ASSERT( pWindow );
    if ( pWindow )
    {
        // ->manually search topmost system window
        // required because their might be another system window between this and the top window
        pWindow = pWindow->GetParent();
        SystemWindow* pTopMostSysWin = nullptr;
        while ( pWindow )
        {
            if ( pWindow->IsSystemWindow() )
                pTopMostSysWin = static_cast<SystemWindow*>(pWindow);
            pWindow = pWindow->GetParent();
        }
        pWindow = pTopMostSysWin;
        OSL_ASSERT( pWindow );
        return pWindow;
    }

    return nullptr;
}


void ToolbarMenuEntry::init( int nEntryId, MenuItemBits nBits )
{
    mnEntryId = nEntryId;
    mnBits = nBits;

    mbHasText = false;
    mbHasImage = false;
    mbChecked = false;
    mbEnabled = true;

    mpControl = nullptr;
}


ToolbarMenuEntry::ToolbarMenuEntry( ToolbarMenu& rMenu, int nEntryId, const OUString& rText, MenuItemBits nBits )
: mrMenu( rMenu )
{
    init( nEntryId, nBits );

    maText = rText;
    mbHasText = true;
}


ToolbarMenuEntry::ToolbarMenuEntry( ToolbarMenu& rMenu, int nEntryId, const Image& rImage, const OUString& rText, MenuItemBits nBits )
: mrMenu( rMenu )
{
    init( nEntryId, nBits );

    maText = rText;
    mbHasText = true;

    maImage = rImage;
    mbHasImage = true;
}


ToolbarMenuEntry::ToolbarMenuEntry( ToolbarMenu& rMenu, int nEntryId, Control* pControl, MenuItemBits nBits )
: mrMenu( rMenu )
{
    init( nEntryId, nBits );

    if( pControl )
    {
        mpControl = pControl;
        mpControl->Show();
    }
}


ToolbarMenuEntry::~ToolbarMenuEntry()
{
    if( mxAccContext.is() )
    {
        Reference< XComponent > xComponent( mxAccContext, UNO_QUERY );
        if( xComponent.is() )
            xComponent->dispose();
        mxAccContext.clear();
    }
    mpControl.disposeAndClear();
}


const Reference< XAccessibleContext >& ToolbarMenuEntry::GetAccessible()
{
    if( !mxAccContext.is() )
    {
        if( mpControl )
        {
            mxAccContext.set( mpControl->GetAccessible(), UNO_QUERY );
        }
        else
        {
            mxAccContext.set( new ToolbarMenuEntryAcc( this ) );
        }
    }

    return mxAccContext;
}


sal_Int32 ToolbarMenuEntry::getAccessibleChildCount()
{
    if( mpControl )
    {
        const Reference< XAccessibleContext >& xContext = GetAccessible();
        if( xContext.is() )
        {
            return xContext->getAccessibleChildCount();
        }
    }
    return 1;
}


Reference< XAccessible > ToolbarMenuEntry::getAccessibleChild( sal_Int32 index )
{
    const Reference< XAccessibleContext >& xContext = GetAccessible();
    if( mpControl )
    {
        if( xContext.is() )
        {
            return xContext->getAccessibleChild(index);
        }
    }
    else if( index == 0 )
    {
        Reference< XAccessible > xRet( xContext, UNO_QUERY );
        if( xRet.is() )
            return xRet;
    }

    throw IndexOutOfBoundsException();
}


ToolbarMenu_Impl::ToolbarMenu_Impl( ToolbarMenu& rMenu )
: mrMenu( rMenu )
, mnCheckPos(0)
, mnImagePos(0)
, mnTextPos(0)
, mnHighlightedEntry(-1)
, mnSelectedEntry(-1)
, mnLastColumn(0)
{
}


ToolbarMenu_Impl::~ToolbarMenu_Impl()
{
    setAccessible( nullptr );
}


void ToolbarMenu_Impl::setAccessible( ToolbarMenuAcc* pAccessible )
{
    if( mxAccessible.get() != pAccessible )
    {
        if( mxAccessible.is() )
            mxAccessible->dispose();

        mxAccessible.set( pAccessible );
    }
}


void ToolbarMenu_Impl::fireAccessibleEvent( short nEventId, const css::uno::Any& rOldValue, const css::uno::Any& rNewValue )
{
    if( mxAccessible.is() )
        mxAccessible->FireAccessibleEvent( nEventId, rOldValue, rNewValue );
}


sal_Int32 ToolbarMenu_Impl::getAccessibleChildCount()
{
    sal_Int32 nCount = 0;
    for( const auto& pEntry : maEntryVector )
    {
        if( pEntry )
        {
            if( pEntry->mpControl )
            {
                nCount += pEntry->getAccessibleChildCount();
            }
            else
            {
                nCount += 1;
            }
        }
    }

    return nCount;
}


Reference< XAccessible > ToolbarMenu_Impl::getAccessibleChild( sal_Int32 index )
{
    for( const auto& pEntry : maEntryVector )
    {
        if( pEntry )
        {
            const sal_Int32 nCount = pEntry->getAccessibleChildCount();
            if( index < nCount )
            {
                return pEntry->getAccessibleChild( index );
            }
            index -= nCount;
        }
    }

    throw IndexOutOfBoundsException();
}


Reference< XAccessible > ToolbarMenu_Impl::getAccessibleChild( Control* pControl, sal_Int32 childIndex )
{
    for( const auto& pEntry : maEntryVector )
    {
        if( pEntry && (pEntry->mpControl.get() == pControl) )
        {
            return pEntry->getAccessibleChild( childIndex );
        }
    }

    throw IndexOutOfBoundsException();
}


void ToolbarMenu_Impl::selectAccessibleChild( sal_Int32 nChildIndex )
{
    const int nEntryCount = maEntryVector.size();
    for( int nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* const pEntry = maEntryVector[nEntry].get();
        if( pEntry )
        {
            const sal_Int32 nCount = pEntry->getAccessibleChildCount();
            if( nChildIndex < nCount )
            {
                if( pEntry->mpControl )
                {
                    Reference< XAccessibleSelection > xSel( pEntry->GetAccessible(), UNO_QUERY_THROW );
                    xSel->selectAccessibleChild(nChildIndex);
                }
                else if( pEntry->mnEntryId != TITLE_ID )
                {
                    mrMenu.implSelectEntry( nEntry );
                }
                return;
            }
            nChildIndex -= nCount;
        }
    }

    throw IndexOutOfBoundsException();
}


bool ToolbarMenu_Impl::isAccessibleChildSelected( sal_Int32 nChildIndex )
{
    const int nEntryCount = maEntryVector.size();
    for( int nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* const pEntry = maEntryVector[nEntry].get();
        if( pEntry )
        {
            const sal_Int32 nCount = pEntry->getAccessibleChildCount();
            if( nChildIndex < nCount )
            {
                if( mnHighlightedEntry == nEntry )
                {
                    if( pEntry->mpControl )
                    {
                        Reference< XAccessibleSelection > xSel( pEntry->GetAccessible(), UNO_QUERY_THROW );
                        xSel->isAccessibleChildSelected(nChildIndex);
                    }
                    return true;
                }
                else
                {
                    return false;
                }
            }
            nChildIndex -= nCount;
        }
    }

    throw IndexOutOfBoundsException();
}


void ToolbarMenu_Impl::clearAccessibleSelection()
{
    if( mnHighlightedEntry != -1 )
    {
        mrMenu.Invalidate();
        mnHighlightedEntry = -1;
    }
}


void ToolbarMenu_Impl::notifyHighlightedEntry()
{
    if( mxAccessible.is() && mxAccessible->HasAccessibleListeners() )
    {
        ToolbarMenuEntry* pEntry = implGetEntry( mnHighlightedEntry );
        if( pEntry && pEntry->mbEnabled && (pEntry->mnEntryId != TITLE_ID) )
        {
            Any aNew;
            Any aOld( mxOldSelection );
            if( pEntry->mpControl )
            {
                sal_Int32 nChildIndex = 0;
                // todo: if other controls than ValueSet are allowed, adapt this code
                ValueSet* pValueSet = dynamic_cast< ValueSet* >( pEntry->mpControl.get() );
                if( pValueSet )
                    nChildIndex = static_cast< sal_Int32 >( pValueSet->GetItemPos( pValueSet->GetSelectItemId() ) );

                if( (nChildIndex >= pEntry->getAccessibleChildCount()) || (nChildIndex < 0) )
                    return;

                aNew <<= getAccessibleChild( pEntry->mpControl, nChildIndex );
            }
            else
            {
                aNew <<= pEntry->GetAccessible();
            }

            fireAccessibleEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOld, aNew );
            fireAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, aOld, aNew );
            fireAccessibleEvent( AccessibleEventId::STATE_CHANGED, Any(), Any( AccessibleStateType::FOCUSED ) );
            aNew >>= mxOldSelection;
        }
    }
}


ToolbarMenuEntry* ToolbarMenu_Impl::implGetEntry( int nEntry ) const
{
    if( (nEntry < 0) || (nEntry >= (int)maEntryVector.size() ) )
        return nullptr;

    return maEntryVector[nEntry].get();
}


IMPL_LINK_NOARG( ToolbarMenu, HighlightHdl, ValueSet*, void )
{
    mpImpl->notifyHighlightedEntry();
}

ToolbarMenu::ToolbarMenu( const css::uno::Reference<css::frame::XFrame>& rFrame, vcl::Window* pParentWindow, WinBits nBits )
    : ToolbarPopup(rFrame, pParentWindow, nBits)
{
    mpImpl.reset( new ToolbarMenu_Impl( *this ) );

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetControlBackground( rStyleSettings.GetMenuColor() );

    initWindow();
}


ToolbarMenu::~ToolbarMenu()
{
    disposeOnce();
}

void ToolbarMenu::dispose()
{
    mpImpl->mxAccessible.clear();

    std::unique_ptr<ToolbarMenu_Impl> pImpl = std::move(mpImpl);
    mpImpl.reset();

    pImpl->maEntryVector.clear();

    ToolbarPopup::dispose();
}


int ToolbarMenu::getSelectedEntryId() const
{
    ToolbarMenuEntry* pEntry = implGetEntry( mpImpl->mnSelectedEntry );
    return pEntry ? pEntry->mnEntryId : -1;
}


int ToolbarMenu::getHighlightedEntryId() const
{
    ToolbarMenuEntry* pEntry = implGetEntry( mpImpl->mnHighlightedEntry );
    return pEntry ? pEntry->mnEntryId : -1;
}


void ToolbarMenu::checkEntry( int nEntryId, bool bChecked )
{
    ToolbarMenuEntry* pEntry = implSearchEntry( nEntryId );
    if( pEntry && pEntry->mbChecked != bChecked )
    {
        pEntry->mbChecked = bChecked;
        Invalidate();
    }
}


void ToolbarMenu::enableEntry( int nEntryId, bool bEnable )
{
    ToolbarMenuEntry* pEntry = implSearchEntry( nEntryId );
    if( pEntry && pEntry->mbEnabled != bEnable )
    {
        pEntry->mbEnabled = bEnable;
        if( pEntry->mpControl )
        {
            pEntry->mpControl->Enable( bEnable );

            // hack for the valueset to make it paint itself anew
            pEntry->mpControl->Resize();
        }
        Invalidate();
    }
}


void ToolbarMenu::setEntryText( int nEntryId, const OUString& rStr )
{
    ToolbarMenuEntry* pEntry = implSearchEntry( nEntryId );
    if( pEntry && pEntry->maText != rStr )
    {
        pEntry->maText = rStr;
        mpImpl->maSize = implCalcSize();
        if( IsVisible() )
            Invalidate();
    }
}


void ToolbarMenu::setEntryImage( int nEntryId, const Image& rImage )
{
    ToolbarMenuEntry* pEntry = implSearchEntry( nEntryId );
    if( pEntry && pEntry->maImage != rImage )
    {
        pEntry->maImage = rImage;
        mpImpl->maSize = implCalcSize();
        if( IsVisible() )
            Invalidate();
    }
}


void ToolbarMenu::initWindow()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    // FIXME RenderContext
    SetPointFont(*this, rStyleSettings.GetMenuFont());
    SetBackground(Wallpaper(GetControlBackground()));
    SetTextColor(rStyleSettings.GetMenuTextColor());
    SetTextFillColor();
    SetLineColor();

    mpImpl->maSize = implCalcSize();
}


static long ImplGetNativeCheckAndRadioSize(vcl::RenderContext& rRenderContext, long& rCheckHeight, long& rRadioHeight, long &rMaxWidth )
{
    rMaxWidth = rCheckHeight = rRadioHeight = 0;

    ImplControlValue aVal;
    Rectangle aNativeBounds;
    Rectangle aNativeContent;
    Point tmp( 0, 0 );
    Rectangle aCtrlRegion( tmp, Size( 100, 15 ) );
    if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup, ControlPart::MenuItemCheckMark))
    {
        if (rRenderContext.GetNativeControlRegion(ControlType::MenuPopup, ControlPart::MenuItemCheckMark,
                                                  aCtrlRegion, ControlState::ENABLED, aVal, OUString(),
                                                  aNativeBounds, aNativeContent)
        )
        {
            rCheckHeight = aNativeBounds.GetHeight();
            rMaxWidth = aNativeContent.GetWidth();
        }
    }
    if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup, ControlPart::MenuItemRadioMark))
    {
        if (rRenderContext.GetNativeControlRegion(ControlType::MenuPopup, ControlPart::MenuItemRadioMark,
                                          aCtrlRegion, ControlState::ENABLED, aVal, OUString(),
                                          aNativeBounds, aNativeContent)
        )
        {
            rRadioHeight = aNativeBounds.GetHeight();
            rMaxWidth = std::max (rMaxWidth, aNativeContent.GetWidth());
        }
    }
    return (rCheckHeight > rRadioHeight) ? rCheckHeight : rRadioHeight;
}

#define gfxExtra 7

Size ToolbarMenu::implCalcSize()
{
    const long nFontHeight = GetTextHeight();
    long nExtra = nFontHeight/4;

    Size aSz;
    Size aMaxImgSz;
    long nMaxTextWidth = 0;
    long nMinMenuItemHeight = nFontHeight+2;

    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    const bool bUseImages = rSettings.GetUseImagesInMenus();

    // get maximum image size
    if( bUseImages )
    {
        for( const auto& pEntry : mpImpl->maEntryVector )
        {
            if( pEntry && pEntry->mbHasImage )
            {
                Size aImgSz( pEntry->maImage.GetSizePixel() );
                nMinMenuItemHeight = std::max( nMinMenuItemHeight, aImgSz.Height() + 6 );
                aMaxImgSz.Width() = std::max( aMaxImgSz.Width(), aImgSz.Width() );
            }
        }
    }

    mpImpl->mnCheckPos = nExtra;
    mpImpl->mnImagePos = nExtra;
    mpImpl->mnTextPos = mpImpl->mnImagePos + aMaxImgSz.Width();

    if ( aMaxImgSz.Width() )
        mpImpl->mnTextPos += std::max( nExtra, 7L );

    // set heights, calc maximum width
    for( const auto& pEntry : mpImpl->maEntryVector )
    {
        if( pEntry )
        {
            // Text:
            if( pEntry->mbHasText || pEntry->mbHasImage )
            {
                pEntry->maSize.Height() = nMinMenuItemHeight;

                if( pEntry->mbHasText )
                {
                    long nTextWidth = GetCtrlTextWidth( pEntry->maText ) + mpImpl->mnTextPos + nExtra;
                    nMaxTextWidth = std::max( nTextWidth, nMaxTextWidth );
                }
            }
            // Control:
            else if( pEntry->mpControl )
            {
                Size aControlSize( pEntry->mpControl->GetOutputSizePixel() );

                nMaxTextWidth = std::max( aControlSize.Width(), nMaxTextWidth );
                pEntry->maSize.Height() = aControlSize.Height() + 1;
            }

            if( pEntry->HasCheck() && !pEntry->mbHasImage )
            {
                if (IsNativeControlSupported(ControlType::MenuPopup, (pEntry->mnBits & MenuItemBits::RADIOCHECK)
                                                     ? ControlPart::MenuItemCheckMark
                                                     : ControlPart::MenuItemRadioMark ) )
                {
                    long nCheckHeight = 0, nRadioHeight = 0, nMaxCheckWidth = 0;
                    ImplGetNativeCheckAndRadioSize(*this, nCheckHeight, nRadioHeight, nMaxCheckWidth);

                    long nCtrlHeight = (pEntry->mnBits & MenuItemBits::RADIOCHECK) ? nCheckHeight : nRadioHeight;
                    nMaxTextWidth += nCtrlHeight + gfxExtra;
                }
                else if( pEntry->mbChecked )
                {
                    long nSymbolWidth = (nFontHeight*25)/40;
                    if ( pEntry->mnBits & MenuItemBits::RADIOCHECK )
                        nSymbolWidth = nFontHeight/2;

                    nMaxTextWidth += nSymbolWidth;
                }
            }
        }
    }

    aSz.Width() = nMaxTextWidth + (BORDER_X<<1);

    // positionate controls
    int nY = BORDER_Y;
    for( const auto& pEntry : mpImpl->maEntryVector )
    {
        if (pEntry)
        {
            pEntry->maSize.Width() = nMaxTextWidth;

            if( pEntry->mpControl )
            {
                Size aControlSize( pEntry->mpControl->GetOutputSizePixel() );
                Point aControlPos( (aSz.Width() - aControlSize.Width())>>1, nY);

                pEntry->mpControl->SetPosPixel( aControlPos );

                pEntry->maRect = Rectangle( aControlPos, aControlSize );
            }
            else
            {
                pEntry->maRect = Rectangle( Point( 0, nY ), pEntry->maSize );
            }

            nY += pEntry->maSize.Height();
        }
        else
        {
            nY += SEPARATOR_HEIGHT;
        }
    }

    aSz.Height() += nY + BORDER_Y;

    return aSz;
}


void ToolbarMenu::GetFocus()
{
    if( mpImpl && mpImpl->mnHighlightedEntry == -1 )
        implChangeHighlightEntry( 0 );

    ToolbarPopup::GetFocus();
}


void ToolbarMenu::LoseFocus()
{
    if( mpImpl && mpImpl->mnHighlightedEntry != -1 )
        implChangeHighlightEntry( -1 );

    ToolbarPopup::LoseFocus();
}


void ToolbarMenu::appendEntry( int nEntryId, const OUString& rStr, MenuItemBits nItemBits )
{
    appendEntry( o3tl::make_unique<ToolbarMenuEntry>( *this, nEntryId, rStr, nItemBits ) );
}


void ToolbarMenu::appendEntry( int nEntryId, const OUString& rStr, const Image& rImage )
{
    appendEntry( o3tl::make_unique<ToolbarMenuEntry>( *this, nEntryId, rImage, rStr, MenuItemBits::NONE ) );
}


void ToolbarMenu::appendEntry( int nEntryId, Control* pControl )
{
    appendEntry( o3tl::make_unique<ToolbarMenuEntry>( *this, nEntryId, pControl, MenuItemBits::NONE ) );
}


void ToolbarMenu::appendEntry( std::unique_ptr<ToolbarMenuEntry> pEntry )
{
    mpImpl->maEntryVector.push_back(std::move(pEntry));
    mpImpl->maSize = implCalcSize();
    if (IsVisible())
        Invalidate();
}


void ToolbarMenu::appendSeparator()
{
    appendEntry( nullptr );
}


/** creates an empty ValueSet that is initialized and can be inserted with appendEntry. */
VclPtr<ValueSet> ToolbarMenu::createEmptyValueSetControl()
{
    VclPtr<ValueSet> pSet = VclPtr<ValueSet>::Create( this, WB_TABSTOP | WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NOBORDER | WB_NO_DIRECTSELECT );
    pSet->EnableFullItemMode( false );
    pSet->SetColor( GetControlBackground() );
    pSet->SetHighlightHdl( LINK( this, ToolbarMenu, HighlightHdl ) );
    return pSet;
}


ToolbarMenuEntry* ToolbarMenu::implGetEntry( int nEntry ) const
{
    return mpImpl->implGetEntry( nEntry );
}


ToolbarMenuEntry* ToolbarMenu::implSearchEntry( int nEntryId ) const
{
    for( const auto& p : mpImpl->maEntryVector )
    {
        if( p && p->mnEntryId == nEntryId )
        {
            return p.get();
        }
    }

    return nullptr;
}


void ToolbarMenu::implHighlightEntry(vcl::RenderContext& rRenderContext, int nHighlightEntry)
{
    Size aSz(GetOutputSizePixel());
    long nX = 0;
    long nY = 0;

    const int nEntryCount = mpImpl->maEntryVector.size();

    for (int nEntry = 0; nEntry < nEntryCount; nEntry++)
    {
        ToolbarMenuEntry* const pEntry = mpImpl->maEntryVector[nEntry].get();
        if (pEntry && (nEntry == nHighlightEntry))
        {
            // no highlights for controls only items
            if (pEntry->mpControl)
            {
                break;
            }

            bool bRestoreLineColor = false;
            Color oldLineColor;
            bool bDrawItemRect = true;

            Rectangle aItemRect(Point(nX, nY), Size(aSz.Width(), pEntry->maSize.Height()));
            if (pEntry->mnBits & MenuItemBits::POPUPSELECT)
            {
                long nFontHeight = GetTextHeight();
                aItemRect.Right() -= nFontHeight + nFontHeight / 4;
            }

            if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup, ControlPart::Entire))
            {
                Size aPxSize(GetOutputSizePixel());
                rRenderContext.Push(PushFlags::CLIPREGION);
                rRenderContext.IntersectClipRegion(Rectangle(Point(nX, nY), Size(aSz.Width(), pEntry->maSize.Height())));
                Rectangle aCtrlRect(Point(nX, 0), Size(aPxSize.Width() - nX, aPxSize.Height()));
                rRenderContext.DrawNativeControl(ControlType::MenuPopup, ControlPart::Entire, aCtrlRect,
                                                 ControlState::ENABLED, ImplControlValue(), OUString());
                if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup, ControlPart::MenuItem))
                {
                    bDrawItemRect = false;
                    ControlState eState = ControlState::SELECTED | (pEntry->mbEnabled ? ControlState::ENABLED : ControlState::NONE);
                    if (!rRenderContext.DrawNativeControl(ControlType::MenuPopup, ControlPart::MenuItem, aItemRect,
                                                          eState, ImplControlValue(), OUString()))
                    {
                        bDrawItemRect = true;
                    }
                }
                else
                {
                    bDrawItemRect = true;
                }
                rRenderContext.Pop();
            }
            if (bDrawItemRect)
            {
                if (pEntry->mbEnabled)
                {
                    rRenderContext.SetFillColor(rRenderContext.GetSettings().GetStyleSettings().GetMenuHighlightColor());
                }
                else
                {
                    rRenderContext.SetFillColor();
                    oldLineColor = rRenderContext.GetLineColor();
                    rRenderContext.SetLineColor(rRenderContext.GetSettings().GetStyleSettings().GetMenuHighlightColor());
                    bRestoreLineColor = true;
                }
                rRenderContext.DrawRect(aItemRect);
            }
            implPaint(rRenderContext, pEntry, true/*bHighlight*/);
            if (bRestoreLineColor)
                rRenderContext.SetLineColor(oldLineColor);
            break;
        }

        nY += pEntry ? pEntry->maSize.Height() : SEPARATOR_HEIGHT;
    }
}


void ToolbarMenu::implSelectEntry( int nSelectedEntry )
{
    mpImpl->mnSelectedEntry = nSelectedEntry;

    ToolbarMenuEntry* pEntry = nullptr;
    if( nSelectedEntry != -1 )
        pEntry = mpImpl->maEntryVector[ nSelectedEntry ].get();

    if( pEntry )
        mpImpl->maSelectHdl.Call( this );
}


void ToolbarMenu::MouseButtonDown( const MouseEvent& rMEvt )
{
    implHighlightAtPosition(rMEvt, true);
    implSelectEntry(mpImpl->mnHighlightedEntry);
}


void ToolbarMenu::MouseButtonUp( const MouseEvent& )
{
}


void ToolbarMenu::MouseMove( const MouseEvent& rMEvt )
{
    if (!IsVisible())
        return;

    implHighlightAtPosition(rMEvt, false);
}


void ToolbarMenu::implHighlightAtPosition(const MouseEvent& rMEvt, bool /*bMBDown*/)
{
    long nMouseY = rMEvt.GetPosPixel().Y();
    Size aOutSz = GetOutputSizePixel();
    if ( ( nMouseY >= 0 ) && ( nMouseY < aOutSz.Height() ) )
    {
        long nY = 0;
        bool bHighlighted = false;

        const int nEntryCount = mpImpl->maEntryVector.size();
        for( int nEntry = 0; nEntry < nEntryCount; nEntry++ )
        {
            ToolbarMenuEntry* pEntry = mpImpl->maEntryVector[nEntry].get();
            if( pEntry )
            {
                long nOldY = nY;
                nY += pEntry->maSize.Height();

                if( pEntry->mnEntryId != TITLE_ID )
                {
                    if ( ( nOldY <= nMouseY ) && ( nY > nMouseY ) )
                    {
                        if( nEntry != mpImpl->mnHighlightedEntry )
                        {
                            implChangeHighlightEntry( nEntry );
                        }
                        bHighlighted = true;
                    }
                }
            }
            else
            {
                nY += SEPARATOR_HEIGHT;
            }
        }
        if ( !bHighlighted )
            implChangeHighlightEntry( -1 );
    }
    else
    {
        implChangeHighlightEntry( -1 );
    }
}


void ToolbarMenu::implChangeHighlightEntry(int nEntry)
{
    mpImpl->mnHighlightedEntry = nEntry;
    Invalidate();

    mpImpl->notifyHighlightedEntry();
}


static bool implCheckSubControlCursorMove( Control* pControl, bool bUp, int& nLastColumn )
{
    ValueSet* pValueSet = dynamic_cast< ValueSet* >( pControl );
    if( pValueSet )
    {
        size_t nItemPos = pValueSet->GetItemPos( pValueSet->GetSelectItemId() );
        if( nItemPos != VALUESET_ITEM_NOTFOUND )
        {
            const sal_uInt16 nColCount = pValueSet->GetColCount();
            const size_t nLine = nItemPos / nColCount;

            nLastColumn = nItemPos - (nLine * nColCount);

            if( bUp )
            {
                return nLine > 0;
            }
            else
            {
                const size_t nLineCount = (pValueSet->GetItemCount() + nColCount - 1) / nColCount;
                return (nLine+1) < nLineCount;
            }
        }
    }

    return false;
}


ToolbarMenuEntry* ToolbarMenu::implCursorUpDown( bool bUp, bool bHomeEnd )
{
    int n = 0, nLoop = 0;
    if( !bHomeEnd )
    {
        n = mpImpl->mnHighlightedEntry;
        if( n == -1 )
        {
            if( bUp )
                n = 0;
            else
                n = mpImpl->maEntryVector.size()-1;
        }
        else
        {
            // if we have a currently selected entry and
            // cursor keys are used than check if this entry
            // has a control that can use those cursor keys
            ToolbarMenuEntry* pData = mpImpl->maEntryVector[n].get();
            if( pData && pData->mpControl && !pData->mbHasText )
            {
                if( implCheckSubControlCursorMove( pData->mpControl, bUp, mpImpl->mnLastColumn ) )
                    return pData;
            }
        }
        nLoop = n;
    }
    else
    {
        // absolute positioning
        if( bUp )
        {
            n = mpImpl->maEntryVector.size();
            nLoop = n-1;
        }
        else
        {
            n = -1;
            nLoop = mpImpl->maEntryVector.size()-1;
        }
    }

    do
    {
        if( bUp )
        {
            if ( n )
                n--;
            else
                if( mpImpl->mnHighlightedEntry == -1 )
                    n = mpImpl->maEntryVector.size()-1;
                else
                    break;
        }
        else
        {
            if( n < ((int)mpImpl->maEntryVector.size()-1) )
                n++;
            else
                if( mpImpl->mnHighlightedEntry == -1 )
                    n = 0;
                else
                    break;
        }

        ToolbarMenuEntry* pData = mpImpl->maEntryVector[n].get();
        if( pData && (pData->mnEntryId != TITLE_ID) )
        {
            implChangeHighlightEntry( n );
            return pData;
        }
    } while ( n != nLoop );

    return nullptr;
}


void ToolbarMenu_Impl::implHighlightControl( sal_uInt16 nCode, Control* pControl )
{
    ValueSet* pValueSet = dynamic_cast< ValueSet* >( pControl );
    if( pValueSet )
    {
        const size_t nItemCount = pValueSet->GetItemCount();
        size_t nItemPos = VALUESET_ITEM_NOTFOUND;
        switch( nCode )
        {
        case KEY_UP:
        {
            const sal_uInt16 nColCount = pValueSet->GetColCount();
            const sal_uInt16 nLastLine = nItemCount / nColCount;
            nItemPos = std::min( static_cast<size_t>(((nLastLine-1) * nColCount) + mnLastColumn), nItemCount-1 );
            break;
        }
        case KEY_DOWN:
            nItemPos = std::min( static_cast<size_t>(mnLastColumn), nItemCount-1 );
            break;
        case KEY_END:
            nItemPos = nItemCount -1;
            break;
        case KEY_HOME:
            nItemPos = 0;
            break;
        }
        pValueSet->SelectItem( pValueSet->GetItemId( nItemPos ) );
        notifyHighlightedEntry();
    }
}


void ToolbarMenu::KeyInput( const KeyEvent& rKEvent )
{
    Control* pForwardControl = nullptr;
    sal_uInt16 nCode = rKEvent.GetKeyCode().GetCode();
    switch ( nCode )
    {
        case KEY_UP:
        case KEY_DOWN:
        {
            int nOldEntry = mpImpl->mnHighlightedEntry;
            ToolbarMenuEntry*p = implCursorUpDown( nCode == KEY_UP, false );
            if( p && p->mpControl )
            {
                if( nOldEntry != mpImpl->mnHighlightedEntry )
                {
                    mpImpl->implHighlightControl( nCode, p->mpControl );
                }
                else
                {
                    // in case we are in a system floating window, GrabFocus does not work :-/
                    pForwardControl = p->mpControl;
                }
            }
        }
        break;
        case KEY_END:
        case KEY_HOME:
        {
            ToolbarMenuEntry* p = implCursorUpDown( nCode == KEY_END, true );
            if( p && p->mpControl )
            {
                mpImpl->implHighlightControl( nCode, p->mpControl );
            }
        }
        break;
        case KEY_F6:
        case KEY_ESCAPE:
        {
            // Ctrl-F6 acts like ESC here, the menu bar however will then put the focus in the document
            if( nCode == KEY_F6 && !rKEvent.GetKeyCode().IsMod1() )
                break;

            implSelectEntry( -1 );
        }
        break;

        case KEY_RETURN:
        {
            ToolbarMenuEntry* pEntry = implGetEntry( mpImpl->mnHighlightedEntry );
            if ( pEntry && pEntry->mbEnabled && (pEntry->mnEntryId != TITLE_ID) )
            {
                if( pEntry->mpControl )
                {
                    pForwardControl = pEntry->mpControl;
                }
                else
                {
                    implSelectEntry( mpImpl->mnHighlightedEntry );
                }
            }
        }
        break;
        default:
        {
            ToolbarMenuEntry* pEntry = implGetEntry( mpImpl->mnHighlightedEntry );
            if ( pEntry && pEntry->mbEnabled && pEntry->mpControl && !pEntry->mbHasText )
            {
                pForwardControl = pEntry->mpControl;
            }
        }

    }
    if( pForwardControl )
        pForwardControl->KeyInput( rKEvent );

}


static void ImplPaintCheckBackground(vcl::RenderContext& rRenderContext, vcl::Window& rWindow, const Rectangle& i_rRect, bool i_bHighlight )
{
    bool bNativeOk = false;
    if (rRenderContext.IsNativeControlSupported(ControlType::Toolbar, ControlPart::Button))
    {
        ImplControlValue aControlValue;
        ControlState nState = ControlState::PRESSED | ControlState::ENABLED;

        aControlValue.setTristateVal(ButtonValue::On);

        bNativeOk = rRenderContext.DrawNativeControl(ControlType::Toolbar, ControlPart::Button,
                                                     i_rRect, nState, aControlValue, OUString());
    }

    if (!bNativeOk)
    {
        const StyleSettings& rSettings = rRenderContext.GetSettings().GetStyleSettings();
        Color aColor(i_bHighlight ? rSettings.GetMenuHighlightTextColor() : rSettings.GetHighlightColor());
        vcl::RenderTools::DrawSelectionBackground(rRenderContext, rWindow, i_rRect, 0, i_bHighlight, true, false, nullptr, 2, &aColor);
    }
}

void ToolbarMenu::implPaint(vcl::RenderContext& rRenderContext, ToolbarMenuEntry* pThisOnly, bool bHighlighted)
{
    sal_uInt16 nBorder = 0; long nStartY = 0; // from Menu implementations, needed when we support native menu background & scrollable menu

    long nFontHeight = GetTextHeight();

    long nCheckHeight = 0, nRadioHeight = 0, nMaxCheckWidth = 0;
    ImplGetNativeCheckAndRadioSize(rRenderContext, nCheckHeight, nRadioHeight, nMaxCheckWidth);

    DecorationView aDecoView(&rRenderContext);
    const StyleSettings& rSettings = rRenderContext.GetSettings().GetStyleSettings();
    const bool bUseImages = rSettings.GetUseImagesInMenus();

    int nOuterSpace = 0; // ImplGetSVData()->maNWFData.mnMenuFormatExtraBorder;
    Point aTopLeft(nOuterSpace, nOuterSpace), aTmpPos;

    Size aOutSz(GetOutputSizePixel());
    for (const auto& pEntry : mpImpl->maEntryVector)
    {
        Point aPos(aTopLeft);
        aPos.Y() += nBorder;
        aPos.Y() += nStartY;

        if ((pEntry == nullptr) && !pThisOnly)
        {
            // Separator
            aTmpPos.Y() = aPos.Y() + ((SEPARATOR_HEIGHT - 2) / 2);
            aTmpPos.X() = aPos.X() + 2 + nOuterSpace;
            rRenderContext.SetLineColor(rSettings.GetShadowColor());
            rRenderContext.DrawLine(aTmpPos, Point(aOutSz.Width() - 3 - 2 * nOuterSpace, aTmpPos.Y()));
            aTmpPos.Y()++;
            rRenderContext.SetLineColor(rSettings.GetLightColor());
            rRenderContext.DrawLine(aTmpPos, Point( aOutSz.Width() - 3 - 2 * nOuterSpace, aTmpPos.Y()));
            rRenderContext.SetLineColor();
        }
        else if (!pThisOnly || (pEntry.get() == pThisOnly))
        {
            const bool bTitle = pEntry->mnEntryId == TITLE_ID;

            if (pThisOnly && bHighlighted)
                rRenderContext.SetTextColor(rSettings.GetMenuHighlightTextColor());

            if( aPos.Y() >= 0 )
            {
                long nTextOffsetY = ((pEntry->maSize.Height() - nFontHeight) / 2);

                DrawTextFlags   nTextStyle   = DrawTextFlags::NONE;
                DrawSymbolFlags nSymbolStyle = DrawSymbolFlags::NONE;
                DrawImageFlags  nImageStyle  = DrawImageFlags::NONE;

                if (!pEntry->mbEnabled)
                {
                    nTextStyle   |= DrawTextFlags::Disable;
                    nSymbolStyle |= DrawSymbolFlags::Disable;
                    nImageStyle  |= DrawImageFlags::Disable;
                }

                Rectangle aOuterCheckRect(Point(aPos.X() + mpImpl->mnCheckPos, aPos.Y()),
                                          Size(pEntry->maSize.Height(), pEntry->maSize.Height()));
                aOuterCheckRect.Left()   += 1;
                aOuterCheckRect.Right()  -= 1;
                aOuterCheckRect.Top()    += 1;
                aOuterCheckRect.Bottom() -= 1;

                if (bTitle)
                {
                    // fill the background
                    Rectangle aRect(aTopLeft, Size(aOutSz.Width(), pEntry->maSize.Height()));
                    rRenderContext.SetFillColor(rSettings.GetDialogColor());
                    rRenderContext.SetLineColor();
                    rRenderContext.DrawRect(aRect);
                    rRenderContext.SetLineColor(rSettings.GetLightColor());
                    rRenderContext.DrawLine(aRect.TopLeft(), aRect.TopRight());
                    rRenderContext.SetLineColor(rSettings.GetShadowColor());
                    rRenderContext.DrawLine(aRect.BottomLeft(), aRect.BottomRight());
                }

                // CheckMark
                if (pEntry->HasCheck())
                {
                    // draw selection transparent marker if checked
                    // onto that either a checkmark or the item image
                    // will be painted
                    // however do not do this if native checks will be painted since
                    // the selection color too often does not fit the theme's check and/or radio

                    if (!pEntry->mbHasImage)
                    {
                        if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup,
                                                             (pEntry->mnBits & MenuItemBits::RADIOCHECK)
                                                             ? ControlPart::MenuItemCheckMark
                                                             : ControlPart::MenuItemRadioMark))
                        {
                            ControlPart nPart = ((pEntry->mnBits & MenuItemBits::RADIOCHECK)
                                                 ? ControlPart::MenuItemRadioMark
                                                 : ControlPart::MenuItemCheckMark);

                            ControlState nState = ControlState::NONE;

                            if (pEntry->mbChecked)
                                nState |= ControlState::PRESSED;

                            if (pEntry->mbEnabled)
                                nState |= ControlState::ENABLED;

                            if ( bHighlighted )
                                nState |= ControlState::SELECTED;

                            long nCtrlHeight = (pEntry->mnBits & MenuItemBits::RADIOCHECK) ? nCheckHeight : nRadioHeight;
                            aTmpPos.X() = aOuterCheckRect.Left() + (aOuterCheckRect.GetWidth() - nCtrlHeight) / 2;
                            aTmpPos.Y() = aOuterCheckRect.Top() + (aOuterCheckRect.GetHeight() - nCtrlHeight) / 2;

                            Rectangle aCheckRect(aTmpPos, Size(nCtrlHeight, nCtrlHeight));
                            rRenderContext.DrawNativeControl(ControlType::MenuPopup, nPart, aCheckRect,
                                                             nState, ImplControlValue(), OUString());
                            aPos.setX(aPos.getX() + nCtrlHeight + gfxExtra);
                        }
                        else if (pEntry->mbChecked) // by default do nothing for unchecked items
                        {
                            ImplPaintCheckBackground(rRenderContext, *this, aOuterCheckRect, pThisOnly && bHighlighted);

                            SymbolType eSymbol;
                            Size aSymbolSize;
                            if (pEntry->mnBits & MenuItemBits::RADIOCHECK)
                            {
                                eSymbol = SymbolType::RADIOCHECKMARK;
                                aSymbolSize = Size(nFontHeight / 2, nFontHeight / 2);
                            }
                            else
                            {
                                eSymbol = SymbolType::CHECKMARK;
                                aSymbolSize = Size((nFontHeight * 25) / 40, nFontHeight / 2);
                            }
                            aTmpPos.X() = aOuterCheckRect.Left() + (aOuterCheckRect.GetWidth() - aSymbolSize.Width())/2;
                            aTmpPos.Y() = aOuterCheckRect.Top() + (aOuterCheckRect.GetHeight() - aSymbolSize.Height())/2;
                            Rectangle aRect( aTmpPos, aSymbolSize );
                            aDecoView.DrawSymbol(aRect, eSymbol, GetTextColor(), nSymbolStyle);
                            aPos.setX(aPos.getX() + aSymbolSize.getWidth( ) + gfxExtra);
                        }
                    }
                }

                // Image:
                if (pEntry->mbHasImage && bUseImages)
                {
                    if (pEntry->mbChecked)
                        ImplPaintCheckBackground(rRenderContext, *this, aOuterCheckRect, pThisOnly && bHighlighted);
                    aTmpPos = aOuterCheckRect.TopLeft();
                    aTmpPos.X() += (aOuterCheckRect.GetWidth()-pEntry->maImage.GetSizePixel().Width())/2;
                    aTmpPos.Y() += (aOuterCheckRect.GetHeight()-pEntry->maImage.GetSizePixel().Height())/2;
                    rRenderContext.DrawImage( aTmpPos, pEntry->maImage, nImageStyle );
                }

                // Text:
                if (pEntry->mbHasText)
                {
                    aTmpPos.X() = aPos.X() + (bTitle ? 4 : mpImpl->mnTextPos);
                    aTmpPos.Y() = aPos.Y();
                    aTmpPos.Y() += nTextOffsetY;
                    DrawTextFlags nStyle = nTextStyle|DrawTextFlags::Mnemonic;

                    rRenderContext.DrawCtrlText(aTmpPos, pEntry->maText, 0, pEntry->maText.getLength(), nStyle);
                }

                if (pThisOnly && bHighlighted)
                {
                    // This restores the normal menu or menu bar text
                    // color for when it is no longer highlighted.
                    rRenderContext.SetTextColor(rSettings.GetMenuTextColor());
                 }
            }
        }

        aTopLeft.Y() += pEntry ? pEntry->maSize.Height() : SEPARATOR_HEIGHT;
    }
}

void ToolbarMenu::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    rRenderContext.SetFillColor(rRenderContext.GetSettings().GetStyleSettings().GetMenuColor());

    implPaint(rRenderContext);

    if (mpImpl->mnHighlightedEntry != -1)
        implHighlightEntry(rRenderContext, mpImpl->mnHighlightedEntry);
}


void ToolbarMenu::StateChanged( StateChangedType nType )
{
    ToolbarPopup::StateChanged( nType );

    if ( ( nType == StateChangedType::ControlForeground ) || ( nType == StateChangedType::ControlBackground ) )
    {
        initWindow();
        Invalidate();
    }
}


void ToolbarMenu::DataChanged( const DataChangedEvent& rDCEvt )
{
    ToolbarPopup::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        initWindow();
        Invalidate();
    }
}


void ToolbarMenu::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == CommandEventId::Wheel )
    {
        const CommandWheelData* pData = rCEvt.GetWheelData();
        if( !pData->GetModifier() && ( pData->GetMode() == CommandWheelMode::SCROLL ) )
        {
            implCursorUpDown( pData->GetDelta() > 0L, false );
        }
    }
}


Reference< css::accessibility::XAccessible > ToolbarMenu::CreateAccessible()
{
    mpImpl->setAccessible( new ToolbarMenuAcc( *mpImpl ) );
    return Reference< XAccessible >( mpImpl->mxAccessible.get() );
}


class ToolbarPopupStatusListener : public svt::FrameStatusListener
{
public:
    ToolbarPopupStatusListener( const css::uno::Reference< css::frame::XFrame >& xFrame,
                                ToolbarPopup& rToolbarPopup );

    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

    VclPtr<ToolbarPopup> mpPopup;
};


ToolbarPopupStatusListener::ToolbarPopupStatusListener(
    const css::uno::Reference< css::frame::XFrame >& xFrame,
    ToolbarPopup& rToolbarPopup )
: svt::FrameStatusListener( ::comphelper::getProcessComponentContext(), xFrame )
, mpPopup( &rToolbarPopup )
{
}


void SAL_CALL ToolbarPopupStatusListener::dispose()
{
    mpPopup.clear();
    svt::FrameStatusListener::dispose();
}


void SAL_CALL ToolbarPopupStatusListener::statusChanged( const css::frame::FeatureStateEvent& Event )
{
    if( mpPopup )
        mpPopup->statusChanged( Event );
}

ToolbarPopup::ToolbarPopup( const css::uno::Reference<css::frame::XFrame>& rFrame, vcl::Window* pParentWindow, WinBits nBits )
    : DockingWindow(pParentWindow, nBits)
    , mxFrame( rFrame )
{
    vcl::Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        static_cast<SystemWindow*>(pWindow)->GetTaskPaneList()->AddWindow( this );
}

ToolbarPopup::~ToolbarPopup()
{
    disposeOnce();
}

void ToolbarPopup::dispose()
{
    vcl::Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        static_cast<SystemWindow*>(pWindow)->GetTaskPaneList()->RemoveWindow( this );

    if ( mxStatusListener.is() )
    {
        mxStatusListener->dispose();
        mxStatusListener.clear();
    }

    mxFrame.clear();
    DockingWindow::dispose();
}

void ToolbarPopup::AddStatusListener( const OUString& rCommandURL )
{
    if( !mxStatusListener.is() )
        mxStatusListener.set( new ToolbarPopupStatusListener( mxFrame, *this ) );

    mxStatusListener->addStatusListener( rCommandURL );
}

void ToolbarPopup::statusChanged( const css::frame::FeatureStateEvent& /*Event*/ )
{
}

bool ToolbarPopup::IsInPopupMode()
{
    return GetDockingManager()->IsInPopupMode(this);
}

void ToolbarPopup::EndPopupMode()
{
    GetDockingManager()->EndPopupMode(this);
}


const Size& ToolbarMenu::getMenuSize() const
{
    return mpImpl->maSize;
}


void ToolbarMenu::SetSelectHdl( const Link<ToolbarMenu*,void>& rLink )
{
    mpImpl->maSelectHdl = rLink;
}


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
