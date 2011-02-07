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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <comphelper/processfactory.hxx>

#include <vcl/dockwin.hxx>
#include <vcl/decoview.hxx>
#include <vcl/image.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/toolbox.hxx>

#include "svtools/valueset.hxx"
#include "svtools/toolbarmenu.hxx"
#include "toolbarmenuimp.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::accessibility;

namespace svtools {

// --------------------------------------------------------------------

static Window* GetTopMostParentSystemWindow( Window* pWindow )
{
    OSL_ASSERT( pWindow );
    if ( pWindow )
    {
        // ->manually search topmost system window
        // required because their might be another system window between this and the top window
        pWindow = pWindow->GetParent();
        SystemWindow* pTopMostSysWin = NULL;
        while ( pWindow )
        {
            if ( pWindow->IsSystemWindow() )
                pTopMostSysWin = (SystemWindow*)pWindow;
            pWindow = pWindow->GetParent();
        }
        pWindow = pTopMostSysWin;
        OSL_ASSERT( pWindow );
        return pWindow;
    }

    return NULL;
}

// --------------------------------------------------------------------

void ToolbarMenuEntry::init( int nEntryId, MenuItemBits nBits )
{
    mnEntryId = nEntryId;
    mnBits = nBits;

    mbHasText = false;
    mbHasImage = false;
    mbChecked = false;
    mbEnabled = true;

    mpControl = NULL;
}

// --------------------------------------------------------------------

ToolbarMenuEntry::ToolbarMenuEntry( ToolbarMenu& rMenu, int nEntryId, const String& rText, MenuItemBits nBits )
: mrMenu( rMenu )
{
    init( nEntryId, nBits );

    maText = rText;
    mbHasText = true;
}

// --------------------------------------------------------------------

ToolbarMenuEntry::ToolbarMenuEntry( ToolbarMenu& rMenu, int nEntryId, const Image& rImage, MenuItemBits nBits )
: mrMenu( rMenu )
{
    init( nEntryId, nBits );

    maImage = rImage;
    mbHasImage = true;
}

// --------------------------------------------------------------------

ToolbarMenuEntry::ToolbarMenuEntry( ToolbarMenu& rMenu, int nEntryId, const Image& rImage, const String& rText, MenuItemBits nBits )
: mrMenu( rMenu )
{
    init( nEntryId, nBits );

    maText = rText;
    mbHasText = true;

    maImage = rImage;
    mbHasImage = true;
}

// --------------------------------------------------------------------

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

// --------------------------------------------------------------------

ToolbarMenuEntry::~ToolbarMenuEntry()
{
    if( mxAccContext.is() )
    {
        Reference< XComponent > xComponent( mxAccContext, UNO_QUERY );
        if( xComponent.is() )
            xComponent->dispose();
        mxAccContext.clear();
    }
    delete mpControl;
}

// --------------------------------------------------------------------

const Reference< XAccessibleContext >& ToolbarMenuEntry::GetAccessible( bool bCreate /* = false */ )
{
    if( !mxAccContext.is() && bCreate )
    {
        if( mpControl )
        {
            mxAccContext = Reference< XAccessibleContext >( mpControl->GetAccessible( TRUE ), UNO_QUERY );
        }
        else
        {
            mxAccContext = Reference< XAccessibleContext >( new ToolbarMenuEntryAcc( this ) );
        }
    }

    return mxAccContext;
}

// --------------------------------------------------------------------

sal_Int32 ToolbarMenuEntry::getAccessibleChildCount() throw (RuntimeException)
{
    if( mpControl )
    {
        const Reference< XAccessibleContext >& xContext = GetAccessible( true );
        if( xContext.is() )
        {
            return xContext->getAccessibleChildCount();
        }
    }
    return 1;
}

// --------------------------------------------------------------------

Reference< XAccessible > ToolbarMenuEntry::getAccessibleChild( sal_Int32 index ) throw (IndexOutOfBoundsException, RuntimeException)
{
    const Reference< XAccessibleContext >& xContext = GetAccessible( true );
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

// --------------------------------------------------------------------

ToolbarMenu_Impl::ToolbarMenu_Impl( ToolbarMenu& rMenu, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame )
: mrMenu( rMenu )
, mxFrame( xFrame )
, mxServiceManager( ::comphelper::getProcessServiceFactory() )
, mnCheckPos(0)
, mnImagePos(0)
, mnTextPos(0)
, mnHighlightedEntry(-1)
, mnSelectedEntry(-1)
, mnLastColumn(0)
{
}

// --------------------------------------------------------------------

ToolbarMenu_Impl::~ToolbarMenu_Impl()
{
    setAccessible( 0 );
}

// --------------------------------------------------------------------

void ToolbarMenu_Impl::setAccessible( ToolbarMenuAcc* pAccessible )
{
    if( mxAccessible.get() != pAccessible )
    {
        if( mxAccessible.is() )
            mxAccessible->dispose();

        mxAccessible.set( pAccessible );
    }
}

// -----------------------------------------------------------------------

void ToolbarMenu_Impl::fireAccessibleEvent( short nEventId, const ::com::sun::star::uno::Any& rOldValue, const ::com::sun::star::uno::Any& rNewValue )
{
    if( mxAccessible.is() )
        mxAccessible->FireAccessibleEvent( nEventId, rOldValue, rNewValue );
}

// -----------------------------------------------------------------------

bool ToolbarMenu_Impl::hasAccessibleListeners()
{
    return( mxAccessible.is() && mxAccessible->HasAccessibleListeners() );
}

// --------------------------------------------------------------------

sal_Int32 ToolbarMenu_Impl::getAccessibleChildCount() throw (RuntimeException)
{
    sal_Int32 nCount = 0;
    const int nEntryCount = maEntryVector.size();
    for( int nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* pEntry = maEntryVector[nEntry];
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

// --------------------------------------------------------------------

Reference< XAccessible > ToolbarMenu_Impl::getAccessibleChild( sal_Int32 index ) throw (IndexOutOfBoundsException, RuntimeException)
{
    const int nEntryCount = maEntryVector.size();
    for( int nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* pEntry = maEntryVector[nEntry];
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

// --------------------------------------------------------------------

Reference< XAccessible > ToolbarMenu_Impl::getAccessibleChild( Control* pControl, sal_Int32 childIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    const int nEntryCount = maEntryVector.size();
    for( int nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* pEntry = maEntryVector[nEntry];
        if( pEntry && (pEntry->mpControl == pControl) )
        {
            return pEntry->getAccessibleChild( childIndex );
        }
    }

    throw IndexOutOfBoundsException();
}

// --------------------------------------------------------------------

void ToolbarMenu_Impl::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    const int nEntryCount = maEntryVector.size();
    for( int nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* pEntry = maEntryVector[nEntry];
        if( pEntry )
        {
            const sal_Int32 nCount = pEntry->getAccessibleChildCount();
            if( nChildIndex < nCount )
            {
                if( pEntry->mpControl )
                {
                    Reference< XAccessibleSelection > xSel( pEntry->GetAccessible(true), UNO_QUERY_THROW );
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

// --------------------------------------------------------------------

sal_Bool ToolbarMenu_Impl::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    const int nEntryCount = maEntryVector.size();
    for( int nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* pEntry = maEntryVector[nEntry];
        if( pEntry )
        {
            const sal_Int32 nCount = pEntry->getAccessibleChildCount();
            if( nChildIndex < nCount )
            {
                if( mnHighlightedEntry == nEntry )
                {
                    if( pEntry->mpControl )
                    {
                        Reference< XAccessibleSelection > xSel( pEntry->GetAccessible(true), UNO_QUERY_THROW );
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

// --------------------------------------------------------------------

void ToolbarMenu_Impl::clearAccessibleSelection()
{
    if( mnHighlightedEntry != -1 )
    {
        mrMenu.implHighlightEntry( mnHighlightedEntry, false );
        mnHighlightedEntry = -1;
    }
}


// --------------------------------------------------------------------

void ToolbarMenu_Impl::notifyHighlightedEntry()
{
    if( hasAccessibleListeners() )
    {
        ToolbarMenuEntry* pEntry = implGetEntry( mnHighlightedEntry );
        if( pEntry && pEntry->mbEnabled && (pEntry->mnEntryId != TITLE_ID) )
        {
            Any aNew;
            Any aOld( mxOldSelection );
            if( pEntry->mpControl )
            {
                sal_Int32 nChildIndex = 0;
                // todo: if other controls than ValueSet are allowed, addapt this code
                ValueSet* pValueSet = dynamic_cast< ValueSet* >( pEntry->mpControl );
                if( pValueSet )
                    nChildIndex = static_cast< sal_Int32 >( pValueSet->GetItemPos( pValueSet->GetSelectItemId() ) );

                if( nChildIndex >= pEntry->getAccessibleChildCount() )
                    return;

                aNew <<= getAccessibleChild( pEntry->mpControl, nChildIndex );
            }
            else
            {
                aNew <<= pEntry->GetAccessible(true);
            }

            fireAccessibleEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOld, aNew );
            fireAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, aOld, aNew );
            fireAccessibleEvent( AccessibleEventId::STATE_CHANGED, Any(), Any( AccessibleStateType::FOCUSED ) );
            aNew >>= mxOldSelection;
        }
    }
}

// --------------------------------------------------------------------

ToolbarMenuEntry* ToolbarMenu_Impl::implGetEntry( int nEntry ) const
{
    if( (nEntry < 0) || (nEntry >= (int)maEntryVector.size() ) )
        return NULL;

    return maEntryVector[nEntry];
}


// --------------------------------------------------------------------

IMPL_LINK( ToolbarMenu, HighlightHdl, Control *, pControl )
{
    (void)pControl;
    mpImpl->notifyHighlightedEntry();
    return 0;
}

// ====================================================================

ToolbarMenu::ToolbarMenu( const Reference< XFrame >& rFrame, Window* pParentWindow, WinBits nBits )
: DockingWindow(pParentWindow, nBits)
{
    implInit(rFrame);
}

// --------------------------------------------------------------------

ToolbarMenu::ToolbarMenu( const Reference< XFrame >& rFrame, Window* pParentWindow, const ResId& rResId )
: DockingWindow(pParentWindow, rResId)
{
    implInit(rFrame);
}

// --------------------------------------------------------------------

void ToolbarMenu::implInit(const Reference< XFrame >& rFrame)
{
    mpImpl = new ToolbarMenu_Impl( *this, rFrame );

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetControlBackground( rStyleSettings.GetMenuColor() );

    initWindow();

    Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        ((SystemWindow *)pWindow)->GetTaskPaneList()->AddWindow( this );
}

// --------------------------------------------------------------------

ToolbarMenu::~ToolbarMenu()
{
    Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        ((SystemWindow *)pWindow)->GetTaskPaneList()->RemoveWindow( this );

    if ( mpImpl->mxStatusListener.is() )
    {
        mpImpl->mxStatusListener->dispose();
        mpImpl->mxStatusListener.clear();
    }

    // delete all menu entries
    const int nEntryCount = mpImpl->maEntryVector.size();
    int nEntry;
    for( nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        delete mpImpl->maEntryVector[nEntry];
    }

    delete mpImpl;
}

// --------------------------------------------------------------------

int ToolbarMenu::getSelectedEntryId() const
{
    ToolbarMenuEntry* pEntry = implGetEntry( mpImpl->mnSelectedEntry );
    return pEntry ? pEntry->mnEntryId : -1;
}

// --------------------------------------------------------------------

int ToolbarMenu::getHighlightedEntryId() const
{
    ToolbarMenuEntry* pEntry = implGetEntry( mpImpl->mnHighlightedEntry );
    return pEntry ? pEntry->mnEntryId : -1;
}

// --------------------------------------------------------------------

void ToolbarMenu::checkEntry( int nEntryId, bool bChecked )
{
    ToolbarMenuEntry* pEntry = implSearchEntry( nEntryId );
    if( pEntry && pEntry->mbChecked != bChecked )
    {
        pEntry->mbChecked = bChecked;
        Invalidate();
    }
}

// --------------------------------------------------------------------

bool ToolbarMenu::isEntryChecked( int nEntryId ) const
{
    ToolbarMenuEntry* pEntry = implSearchEntry( nEntryId );
    return pEntry && pEntry->mbChecked;
}

// --------------------------------------------------------------------

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

// --------------------------------------------------------------------

bool ToolbarMenu::isEntryEnabled( int nEntryId ) const
{
    ToolbarMenuEntry* pEntry = implSearchEntry( nEntryId );
    return pEntry && pEntry->mbEnabled;
}

// --------------------------------------------------------------------

void ToolbarMenu::setEntryText( int nEntryId, const String& rStr )
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

// --------------------------------------------------------------------

const String& ToolbarMenu::getEntryText( int nEntryId ) const
{
    ToolbarMenuEntry* pEntry = implSearchEntry( nEntryId );
    if( pEntry )
        return pEntry->maText;
    else
    {
        static String aEmptyStr;
        return aEmptyStr;
    }
}

// --------------------------------------------------------------------

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

// --------------------------------------------------------------------

const Image& ToolbarMenu::getEntryImage( int nEntryId ) const
{
    ToolbarMenuEntry* pEntry = implSearchEntry( nEntryId );
    if( pEntry )
        return pEntry->maImage;
    else
    {
        static Image aEmptyImage;
        return aEmptyImage;
    }
}

// --------------------------------------------------------------------

void ToolbarMenu::initWindow()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    SetPointFont( rStyleSettings.GetMenuFont() );
    SetBackground( Wallpaper( GetControlBackground() ) );
    SetTextColor( rStyleSettings.GetMenuTextColor() );
    SetTextFillColor();
    SetLineColor();

    mpImpl->maSize = implCalcSize();
}

// --------------------------------------------------------------------

static long ImplGetNativeCheckAndRadioSize( Window* pWin, long& rCheckHeight, long& rRadioHeight, long &rMaxWidth )
{
    rMaxWidth = rCheckHeight = rRadioHeight = 0;

    ImplControlValue aVal;
    Rectangle aNativeBounds;
    Rectangle aNativeContent;
    Point tmp( 0, 0 );
    Rectangle aCtrlRegion( tmp, Size( 100, 15 ) );
    if( pWin->IsNativeControlSupported( CTRL_MENU_POPUP, PART_MENU_ITEM_CHECK_MARK ) )
    {
        if( pWin->GetNativeControlRegion( ControlType(CTRL_MENU_POPUP),
                                          ControlPart(PART_MENU_ITEM_CHECK_MARK),
                                          aCtrlRegion,
                                          ControlState(CTRL_STATE_ENABLED),
                                          aVal,
                                          OUString(),
                                          aNativeBounds,
                                          aNativeContent )
        )
        {
            rCheckHeight = aNativeBounds.GetHeight();
            rMaxWidth = aNativeContent.GetWidth();
        }
    }
    if( pWin->IsNativeControlSupported( CTRL_MENU_POPUP, PART_MENU_ITEM_RADIO_MARK ) )
    {
        if( pWin->GetNativeControlRegion( ControlType(CTRL_MENU_POPUP),
                                          ControlPart(PART_MENU_ITEM_RADIO_MARK),
                                          aCtrlRegion,
                                          ControlState(CTRL_STATE_ENABLED),
                                          aVal,
                                          OUString(),
                                          aNativeBounds,
                                          aNativeContent )
        )
        {
            rRadioHeight = aNativeBounds.GetHeight();
            rMaxWidth = Max (rMaxWidth, aNativeContent.GetWidth());
        }
    }
    return (rCheckHeight > rRadioHeight) ? rCheckHeight : rRadioHeight;
}


Size ToolbarMenu::implCalcSize()
{
    const long nFontHeight = GetTextHeight();
    long nExtra = nFontHeight/4;

    Size aSz;
    Size aMaxImgSz;
    long nMaxTextWidth = 0;
    long nMinMenuItemHeight = nFontHeight+2;
    sal_Bool bCheckable = sal_False;

    const int nEntryCount = mpImpl->maEntryVector.size();
    int nEntry;

    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    const bool bUseImages = rSettings.GetUseImagesInMenus();

    // get maximum image size
    if( bUseImages )
    {
        for( nEntry = 0; nEntry < nEntryCount; nEntry++ )
        {
            ToolbarMenuEntry* pEntry = mpImpl->maEntryVector[nEntry];
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
    if ( bCheckable )
        mpImpl->mnTextPos += 16;

    // set heights, calc maximum width
    for( nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* pEntry = mpImpl->maEntryVector[nEntry];

        if( pEntry )
        {
            if ( ( pEntry->mnBits ) & ( MIB_RADIOCHECK | MIB_CHECKABLE ) )
                bCheckable = sal_True;

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
                if( this->IsNativeControlSupported( CTRL_MENU_POPUP,
                                                     (pEntry->mnBits & MIB_RADIOCHECK)
                                                     ? PART_MENU_ITEM_CHECK_MARK
                                                     : PART_MENU_ITEM_RADIO_MARK ) )
                {
                    long nCheckHeight = 0, nRadioHeight = 0, nMaxCheckWidth = 0;
                    ImplGetNativeCheckAndRadioSize( this, nCheckHeight, nRadioHeight, nMaxCheckWidth );

                    long nCtrlHeight = (pEntry->mnBits & MIB_RADIOCHECK) ? nCheckHeight : nRadioHeight;
                    nMaxTextWidth += nCtrlHeight + 1;
                }
                else if( pEntry->mbChecked )
                {
                    long nSymbolWidth = (nFontHeight*25)/40;
                    if ( pEntry->mnBits & MIB_RADIOCHECK )
                        nSymbolWidth = nFontHeight/2;

                    nMaxTextWidth += nSymbolWidth;
                }
            }
        }
    }

    aSz.Width() = nMaxTextWidth + (BORDER_X<<1);

    // positionate controls
    int nY = BORDER_Y;
    for( nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* pEntry = mpImpl->maEntryVector[nEntry];

        if( pEntry )
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

// --------------------------------------------------------------------

void ToolbarMenu::highlightFirstEntry()
{
    implChangeHighlightEntry( 0 );
}

// --------------------------------------------------------------------

void ToolbarMenu::GetFocus()
{
    if( mpImpl->mnHighlightedEntry == -1 )
        implChangeHighlightEntry( 0 );

    DockingWindow::GetFocus();
}

// --------------------------------------------------------------------

void ToolbarMenu::LoseFocus()
{
    if( mpImpl->mnHighlightedEntry != -1 )
        implChangeHighlightEntry( -1 );

    DockingWindow::LoseFocus();
}

// --------------------------------------------------------------------

void ToolbarMenu::appendEntry( int nEntryId, const String& rStr, MenuItemBits nItemBits )
{
    appendEntry( new ToolbarMenuEntry( *this, nEntryId, rStr, nItemBits ) );
}

// --------------------------------------------------------------------

void ToolbarMenu::appendEntry( int nEntryId, const Image& rImage, MenuItemBits nItemBits )
{
    appendEntry( new ToolbarMenuEntry( *this, nEntryId, rImage, nItemBits ) );
}

// --------------------------------------------------------------------

void ToolbarMenu::appendEntry( int nEntryId, const String& rStr, const Image& rImage, MenuItemBits nItemBits )
{
    appendEntry( new ToolbarMenuEntry( *this, nEntryId, rImage, rStr, nItemBits ) );
}

// --------------------------------------------------------------------

void ToolbarMenu::appendEntry( int nEntryId, Control* pControl, MenuItemBits nItemBits )
{
    appendEntry( new ToolbarMenuEntry( *this, nEntryId, pControl, nItemBits ) );
}

// --------------------------------------------------------------------

void ToolbarMenu::appendEntry( ToolbarMenuEntry* pEntry )
{
    mpImpl->maEntryVector.push_back( pEntry );
    mpImpl->maSize = implCalcSize();
    if( IsVisible() )
        Invalidate();
}

// --------------------------------------------------------------------

void ToolbarMenu::appendSeparator()
{
    appendEntry( 0 );
}

// --------------------------------------------------------------------

/** creates an empty ValueSet that is initialized and can be inserted with appendEntry. */
ValueSet* ToolbarMenu::createEmptyValueSetControl()
{
    ValueSet* pSet = new ValueSet( this, WB_TABSTOP | WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NOBORDER | WB_NO_DIRECTSELECT );
    pSet->EnableFullItemMode( FALSE );
    pSet->SetColor( GetControlBackground() );
    pSet->SetHighlightHdl( LINK( this, ToolbarMenu, HighlightHdl ) );
    return pSet;
}

// --------------------------------------------------------------------

ToolbarMenuEntry* ToolbarMenu::implGetEntry( int nEntry ) const
{
    return mpImpl->implGetEntry( nEntry );
}

// --------------------------------------------------------------------

ToolbarMenuEntry* ToolbarMenu::implSearchEntry( int nEntryId ) const
{
    const int nEntryCount = mpImpl->maEntryVector.size();
    int nEntry;
    for( nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* p = mpImpl->maEntryVector[nEntry];
        if( p && p->mnEntryId == nEntryId )
        {
            return p;
        }
    }

    return NULL;
}

// --------------------------------------------------------------------

void ToolbarMenu::implHighlightEntry( int nHighlightEntry, bool bHighlight )
{
    Size    aSz( GetOutputSizePixel() );
    long    nX = 0, nY = 0;

    const int nEntryCount = mpImpl->maEntryVector.size();
    int nEntry;
    for( nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* pEntry = mpImpl->maEntryVector[nEntry];
        if( pEntry && (nEntry == nHighlightEntry) )
        {
            // no highlights for controls only items
            if( pEntry->mpControl )
            {
                if( !bHighlight )
                {
                    ValueSet* pValueSet = dynamic_cast< ValueSet* >( pEntry->mpControl );
                    if( pValueSet )
                    {
                        pValueSet->SetNoSelection();
                    }
                }
                break;
            }

            bool bRestoreLineColor = false;
            Color oldLineColor;
            bool bDrawItemRect = true;

            Rectangle aItemRect( Point( nX, nY ), Size( aSz.Width(), pEntry->maSize.Height() ) );
            if ( pEntry->mnBits & MIB_POPUPSELECT )
            {
                long nFontHeight = GetTextHeight();
                aItemRect.Right() -= nFontHeight + nFontHeight/4;
            }

            if( IsNativeControlSupported( CTRL_MENU_POPUP, PART_ENTIRE_CONTROL ) )
            {
                Size aPxSize( GetOutputSizePixel() );
                Push( PUSH_CLIPREGION );
                IntersectClipRegion( Rectangle( Point( nX, nY ), Size( aSz.Width(), pEntry->maSize.Height() ) ) );
                Rectangle aCtrlRect( Point( nX, 0 ), Size( aPxSize.Width()-nX, aPxSize.Height() ) );
                DrawNativeControl( CTRL_MENU_POPUP, PART_ENTIRE_CONTROL,
                                   aCtrlRect,
                                   CTRL_STATE_ENABLED,
                                   ImplControlValue(),
                                   OUString() );
                if( bHighlight && IsNativeControlSupported( CTRL_MENU_POPUP, PART_MENU_ITEM ) )
                {
                    bDrawItemRect = false;
                    if( FALSE == DrawNativeControl( CTRL_MENU_POPUP, PART_MENU_ITEM,
                                                    aItemRect,
                                                    CTRL_STATE_SELECTED | ( pEntry->mbEnabled? CTRL_STATE_ENABLED: 0 ),
                                                    ImplControlValue(),
                                                    OUString() ) )
                    {
                        bDrawItemRect = bHighlight;
                    }
                }
                else
                    bDrawItemRect = bHighlight;
                Pop();
            }
            if( bDrawItemRect )
            {
                if ( bHighlight )
                {
                    if( pEntry->mbEnabled )
                        SetFillColor( GetSettings().GetStyleSettings().GetMenuHighlightColor() );
                    else
                    {
                        SetFillColor();
                        oldLineColor = GetLineColor();
                        SetLineColor( GetSettings().GetStyleSettings().GetMenuHighlightColor() );
                        bRestoreLineColor = true;
                    }
                }
                else
                    SetFillColor( GetSettings().GetStyleSettings().GetMenuColor() );

                DrawRect( aItemRect );
            }
            implPaint( pEntry, bHighlight );
            if( bRestoreLineColor )
                SetLineColor( oldLineColor );
            break;
        }

        nY += pEntry ? pEntry->maSize.Height() : SEPARATOR_HEIGHT;
    }
}

// --------------------------------------------------------------------

void ToolbarMenu::implSelectEntry( int nSelectedEntry )
{
    mpImpl->mnSelectedEntry = nSelectedEntry;

    ToolbarMenuEntry* pEntry = NULL;
    if( nSelectedEntry != -1 )
        pEntry = mpImpl->maEntryVector[ nSelectedEntry ];

    if( pEntry )
        mpImpl->maSelectHdl.Call( this );
}

// --------------------------------------------------------------------

void ToolbarMenu::MouseButtonDown( const MouseEvent& rMEvt )
{
    implHighlightEntry( rMEvt, true );

    implSelectEntry( mpImpl->mnHighlightedEntry );
}

// --------------------------------------------------------------------

void ToolbarMenu::MouseButtonUp( const MouseEvent& )
{
}

// --------------------------------------------------------------------

void ToolbarMenu::MouseMove( const MouseEvent& rMEvt )
{
    if ( !IsVisible() )
        return;

    implHighlightEntry( rMEvt, false );
}

// --------------------------------------------------------------------

void ToolbarMenu::implHighlightEntry( const MouseEvent& rMEvt, bool bMBDown )
{
    long nY = 0;
    long nMouseY = rMEvt.GetPosPixel().Y();
    Size aOutSz = GetOutputSizePixel();
    if ( ( nMouseY >= 0 ) && ( nMouseY < aOutSz.Height() ) )
    {
        bool bHighlighted = FALSE;

        const int nEntryCount = mpImpl->maEntryVector.size();
        int nEntry;
        for( nEntry = 0; nEntry < nEntryCount; nEntry++ )
        {
            ToolbarMenuEntry* pEntry = mpImpl->maEntryVector[nEntry];
            if( pEntry )
            {
                long nOldY = nY;
                nY += pEntry->maSize.Height();

                if( pEntry->mnEntryId != TITLE_ID )
                {
                    if ( ( nOldY <= nMouseY ) && ( nY > nMouseY ) )
                    {
                        if( bMBDown )
                        {
                            if( nEntry != mpImpl->mnHighlightedEntry )
                            {
                                implChangeHighlightEntry( nEntry );
                            }
                        }
                        else
                        {
                            if ( nEntry != mpImpl->mnHighlightedEntry )
                            {
                                implChangeHighlightEntry( nEntry );
                            }
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

// --------------------------------------------------------------------

void ToolbarMenu::implChangeHighlightEntry( int nEntry )
{
    if( mpImpl->mnHighlightedEntry != -1 )
    {
        implHighlightEntry( mpImpl->mnHighlightedEntry, false );
    }

    mpImpl->mnHighlightedEntry = nEntry;
    Invalidate();

    if( mpImpl->mnHighlightedEntry != -1 )
    {
        implHighlightEntry( mpImpl->mnHighlightedEntry, true );
    }

    mpImpl->notifyHighlightedEntry();
}

// --------------------------------------------------------------------

static bool implCheckSubControlCursorMove( Control* pControl, bool bUp, int& nLastColumn )
{
    ValueSet* pValueSet = dynamic_cast< ValueSet* >( pControl );
    if( pValueSet )
    {
        USHORT nItemPos = pValueSet->GetItemPos( pValueSet->GetSelectItemId() );
        if( nItemPos != VALUESET_ITEM_NOTFOUND )
        {
            const USHORT nColCount = pValueSet->GetColCount();
            const USHORT nLine = nItemPos / nColCount;

            nLastColumn = nItemPos - (nLine * nColCount);

            if( bUp )
            {
                return nLine > 0;
            }
            else
            {
                const USHORT nLineCount = (pValueSet->GetItemCount() + nColCount - 1) / nColCount;
                return (nLine+1) < nLineCount;
            }
        }
    }

    return false;
}

// --------------------------------------------------------------------

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
            ToolbarMenuEntry* pData = mpImpl->maEntryVector[n];
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

        ToolbarMenuEntry* pData = mpImpl->maEntryVector[n];
        if( pData && (pData->mnEntryId != TITLE_ID) )
        {
            implChangeHighlightEntry( n );
            return pData;
        }
    } while ( n != nLoop );

    return 0;
}

// --------------------------------------------------------------------

void ToolbarMenu_Impl::implHighlightControl( USHORT nCode, Control* pControl )
{
    ValueSet* pValueSet = dynamic_cast< ValueSet* >( pControl );
    if( pValueSet )
    {
        const USHORT nItemCount = pValueSet->GetItemCount();
        USHORT nItemPos = VALUESET_ITEM_NOTFOUND;
        switch( nCode )
        {
        case KEY_UP:
        {
            const USHORT nColCount = pValueSet->GetColCount();
            const USHORT nLastLine = nItemCount / nColCount;
            nItemPos = std::min( ((nLastLine-1) * nColCount) + mnLastColumn, nItemCount-1 );
            break;
        }
        case KEY_DOWN:
            nItemPos = std::min( mnLastColumn, nItemCount-1 );
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

// --------------------------------------------------------------------

void ToolbarMenu::KeyInput( const KeyEvent& rKEvent )
{
    Control* pForwardControl = 0;
    USHORT nCode = rKEvent.GetKeyCode().GetCode();
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

// --------------------------------------------------------------------
static void ImplPaintCheckBackground( Window* i_pWindow, const Rectangle& i_rRect, bool i_bHighlight )
{
    BOOL bNativeOk = FALSE;
    if( i_pWindow->IsNativeControlSupported( CTRL_TOOLBAR, PART_BUTTON ) )
    {
        ImplControlValue    aControlValue;
        ControlState        nState = CTRL_STATE_PRESSED | CTRL_STATE_ENABLED;

        aControlValue.setTristateVal( BUTTONVALUE_ON );

        bNativeOk = i_pWindow->DrawNativeControl( CTRL_TOOLBAR, PART_BUTTON,
                                                  i_rRect, nState, aControlValue,
                                                  rtl::OUString() );
    }

    if( ! bNativeOk )
    {
        const StyleSettings& rSettings = i_pWindow->GetSettings().GetStyleSettings();
        Color aColor( i_bHighlight ? rSettings.GetMenuHighlightTextColor() : rSettings.GetHighlightColor() );
        i_pWindow->DrawSelectionBackground( i_rRect, 0, i_bHighlight, TRUE, FALSE, 2, NULL, &aColor );
    }
}

void ToolbarMenu::implPaint( ToolbarMenuEntry* pThisOnly, bool bHighlighted )
{
    USHORT nBorder = 0; long nStartY = 0; // from Menu implementations, needed when we support native menu background & scrollable menu

    long nFontHeight = GetTextHeight();
//    long nExtra = nFontHeight/4;

    long nCheckHeight = 0, nRadioHeight = 0, nMaxCheckWidth = 0;
    ImplGetNativeCheckAndRadioSize( this, nCheckHeight, nRadioHeight, nMaxCheckWidth );

    DecorationView aDecoView( this );
    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    const bool bUseImages = rSettings.GetUseImagesInMenus();

    int nOuterSpace = 0; // ImplGetSVData()->maNWFData.mnMenuFormatExtraBorder;
    Point aTopLeft( nOuterSpace, nOuterSpace ), aTmpPos;

    Size aOutSz( GetOutputSizePixel() );
    const int nEntryCount = mpImpl->maEntryVector.size();
    int nEntry;
    for( nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* pEntry = mpImpl->maEntryVector[nEntry];

        Point aPos( aTopLeft );
        aPos.Y() += nBorder;
        aPos.Y() += nStartY;


        if( (pEntry == 0) && !pThisOnly )
        {
            // Separator
            aTmpPos.Y() = aPos.Y() + ((SEPARATOR_HEIGHT-2)/2);
            aTmpPos.X() = aPos.X() + 2 + nOuterSpace;
            SetLineColor( rSettings.GetShadowColor() );
            DrawLine( aTmpPos, Point( aOutSz.Width() - 3 - 2*nOuterSpace, aTmpPos.Y() ) );
            aTmpPos.Y()++;
            SetLineColor( rSettings.GetLightColor() );
            DrawLine( aTmpPos, Point( aOutSz.Width() - 3 - 2*nOuterSpace, aTmpPos.Y() ) );
            SetLineColor();
        }
        else if( !pThisOnly || ( pEntry == pThisOnly ) )
        {
            const bool bTitle = pEntry->mnEntryId == TITLE_ID;

            if ( pThisOnly && bHighlighted )
                SetTextColor( rSettings.GetMenuHighlightTextColor() );

            if( aPos.Y() >= 0 )
            {
                long nTextOffsetY = ((pEntry->maSize.Height()-nFontHeight)/2);

                USHORT  nTextStyle   = 0;
                USHORT  nSymbolStyle = 0;
                USHORT  nImageStyle  = 0;

                if( !pEntry->mbEnabled )
                {
                    nTextStyle   |= TEXT_DRAW_DISABLE;
                    nSymbolStyle |= SYMBOL_DRAW_DISABLE;
                    nImageStyle  |= IMAGE_DRAW_DISABLE;
                }

                Rectangle aOuterCheckRect( Point( aPos.X()+mpImpl->mnCheckPos, aPos.Y() ), Size( pEntry->maSize.Height(), pEntry->maSize.Height() ) );
                aOuterCheckRect.Left()      += 1;
                aOuterCheckRect.Right()     -= 1;
                aOuterCheckRect.Top()       += 1;
                aOuterCheckRect.Bottom()    -= 1;

                if( bTitle )
                {
                    // fill the background
                    Rectangle aRect( aTopLeft, Size( aOutSz.Width(), pEntry->maSize.Height() ) );
                    SetFillColor(rSettings.GetDialogColor());
                    SetLineColor();
                    DrawRect(aRect);
                    SetLineColor( rSettings.GetLightColor() );
                    DrawLine( aRect.TopLeft(), aRect.TopRight() );
                    SetLineColor( rSettings.GetShadowColor() );
                    DrawLine( aRect.BottomLeft(), aRect.BottomRight() );
                }

                // CheckMark
                if ( pEntry->HasCheck() )
                {
                    // draw selection transparent marker if checked
                    // onto that either a checkmark or the item image
                    // will be painted
                    // however do not do this if native checks will be painted since
                    // the selection color too often does not fit the theme's check and/or radio

                    if( !pEntry->mbHasImage )
                    {
                        if( this->IsNativeControlSupported( CTRL_MENU_POPUP,
                                                             (pEntry->mnBits & MIB_RADIOCHECK)
                                                             ? PART_MENU_ITEM_CHECK_MARK
                                                             : PART_MENU_ITEM_RADIO_MARK ) )
                        {
                            ControlPart nPart = ((pEntry->mnBits & MIB_RADIOCHECK)
                                                 ? PART_MENU_ITEM_RADIO_MARK
                                                 : PART_MENU_ITEM_CHECK_MARK);

                            ControlState nState = 0;

                            if ( pEntry->mbChecked )
                                nState |= CTRL_STATE_PRESSED;

                            if ( pEntry->mbEnabled )
                                nState |= CTRL_STATE_ENABLED;

                            if ( bHighlighted )
                                nState |= CTRL_STATE_SELECTED;

                            long nCtrlHeight = (pEntry->mnBits & MIB_RADIOCHECK) ? nCheckHeight : nRadioHeight;
                            aTmpPos.X() = aOuterCheckRect.Left() + (aOuterCheckRect.GetWidth() - nCtrlHeight)/2;
                            aTmpPos.Y() = aOuterCheckRect.Top() + (aOuterCheckRect.GetHeight() - nCtrlHeight)/2;

                            Rectangle aCheckRect( aTmpPos, Size( nCtrlHeight, nCtrlHeight ) );
                            DrawNativeControl( CTRL_MENU_POPUP, nPart, aCheckRect, nState, ImplControlValue(), OUString() );
                            aPos.setX( aPos.getX() + nCtrlHeight + 1 );
                        }
                        else if ( pEntry->mbChecked ) // by default do nothing for unchecked items
                        {
                            ImplPaintCheckBackground( this, aOuterCheckRect, pThisOnly && bHighlighted );

                            SymbolType eSymbol;
                            Size aSymbolSize;
                            if ( pEntry->mnBits & MIB_RADIOCHECK )
                            {
                                eSymbol = SYMBOL_RADIOCHECKMARK;
                                aSymbolSize = Size( nFontHeight/2, nFontHeight/2 );
                            }
                            else
                            {
                                eSymbol = SYMBOL_CHECKMARK;
                                aSymbolSize = Size( (nFontHeight*25)/40, nFontHeight/2 );
                            }
                            aTmpPos.X() = aOuterCheckRect.Left() + (aOuterCheckRect.GetWidth() - aSymbolSize.Width())/2;
                            aTmpPos.Y() = aOuterCheckRect.Top() + (aOuterCheckRect.GetHeight() - aSymbolSize.Height())/2;
                            Rectangle aRect( aTmpPos, aSymbolSize );
                            aDecoView.DrawSymbol( aRect, eSymbol, GetTextColor(), nSymbolStyle );
                            aPos.setX( aPos.getX() + aSymbolSize.getWidth( ) + 1 );
                        }
                    }
                }

                // Image:
                if( pEntry->mbHasImage && bUseImages )
                {
                    // Don't render an image for a check thing
                     /* if((nMenuFlags & MENU_FLAG_SHOWCHECKIMAGES) || !pEntry->HasCheck() )*/
                    {
                        if( pEntry->mbChecked )
                            ImplPaintCheckBackground( this, aOuterCheckRect, pThisOnly && bHighlighted );
                        aTmpPos = aOuterCheckRect.TopLeft();
                        aTmpPos.X() += (aOuterCheckRect.GetWidth()-pEntry->maImage.GetSizePixel().Width())/2;
                        aTmpPos.Y() += (aOuterCheckRect.GetHeight()-pEntry->maImage.GetSizePixel().Height())/2;
                        DrawImage( aTmpPos, pEntry->maImage, nImageStyle );
                    }
                }

                // Text:
                if( pEntry->mbHasText )
                {
                    aTmpPos.X() = aPos.X() + (bTitle ? 4 : mpImpl->mnTextPos);
                    aTmpPos.Y() = aPos.Y();
                    aTmpPos.Y() += nTextOffsetY;
                    USHORT nStyle = nTextStyle|TEXT_DRAW_MNEMONIC;

                    DrawCtrlText( aTmpPos, pEntry->maText, 0, pEntry->maText.Len(), nStyle, NULL, NULL ); // pVector, pDisplayText );
                }

/*
                // Accel
                if ( !bLayout && !bIsMenuBar && pData->aAccelKey.GetCode() && !ImplAccelDisabled() )
                {
                    XubString aAccText = pData->aAccelKey.GetName();
                    aTmpPos.X() = aOutSz.Width() - this->GetTextWidth( aAccText );
                    aTmpPos.X() -= 4*nExtra;

                    aTmpPos.X() -= nOuterSpace;
                    aTmpPos.Y() = aPos.Y();
                    aTmpPos.Y() += nTextOffsetY;
                    this->DrawCtrlText( aTmpPos, aAccText, 0, aAccText.Len(), nTextStyle );
                }
*/

/*
                // SubMenu?
                if ( !bLayout && !bIsMenuBar && pData->pSubMenu )
                {
                    aTmpPos.X() = aOutSz.Width() - nFontHeight + nExtra - nOuterSpace;
                    aTmpPos.Y() = aPos.Y();
                    aTmpPos.Y() += nExtra/2;
                    aTmpPos.Y() += ( pEntry->maSize.Height() / 2 ) - ( nFontHeight/4 );
                    if ( pEntry->mnBits & MIB_POPUPSELECT )
                    {
                        this->SetTextColor( rSettings.GetMenuTextColor() );
                        Point aTmpPos2( aPos );
                        aTmpPos2.X() = aOutSz.Width() - nFontHeight - nFontHeight/4;
                        aDecoView.DrawFrame(
                            Rectangle( aTmpPos2, Size( nFontHeight+nFontHeight/4, pEntry->maSize.Height() ) ), FRAME_DRAW_GROUP );
                    }
                    aDecoView.DrawSymbol(
                        Rectangle( aTmpPos, Size( nFontHeight/2, nFontHeight/2 ) ),
                        SYMBOL_SPIN_RIGHT, this->GetTextColor(), nSymbolStyle );
//                  if ( pEntry->mnBits & MIB_POPUPSELECT )
//                  {
//                      aTmpPos.Y() += nFontHeight/2 ;
//                      this->SetLineColor( rSettings.GetShadowColor() );
//                      this->DrawLine( aTmpPos, Point( aTmpPos.X() + nFontHeight/3, aTmpPos.Y() ) );
//                      this->SetLineColor( rSettings.GetLightColor() );
//                      aTmpPos.Y()++;
//                      this->DrawLine( aTmpPos, Point( aTmpPos.X() + nFontHeight/3, aTmpPos.Y() ) );
//                      this->SetLineColor();
//                  }
                }
*/

                if ( pThisOnly && bHighlighted )
                {
                    // This restores the normal menu or menu bar text
                    // color for when it is no longer highlighted.
                    SetTextColor( rSettings.GetMenuTextColor() );
                 }
            }
        }

        aTopLeft.Y() += pEntry ? pEntry->maSize.Height() : SEPARATOR_HEIGHT;
    }
}

// --------------------------------------------------------------------

void ToolbarMenu::Paint( const Rectangle& )
{
    SetFillColor( GetSettings().GetStyleSettings().GetMenuColor() );

    implPaint();

    if( mpImpl->mnHighlightedEntry != -1 )
        implHighlightEntry( mpImpl->mnHighlightedEntry, true );
}

// --------------------------------------------------------------------

void ToolbarMenu::RequestHelp( const HelpEvent& rHEvt )
{
    DockingWindow::RequestHelp( rHEvt );
}

// --------------------------------------------------------------------

void ToolbarMenu::StateChanged( StateChangedType nType )
{
    DockingWindow::StateChanged( nType );

    if ( ( nType == STATE_CHANGE_CONTROLFOREGROUND ) || ( nType == STATE_CHANGE_CONTROLBACKGROUND ) )
    {
        initWindow();
        Invalidate();
    }
}

// --------------------------------------------------------------------

void ToolbarMenu::DataChanged( const DataChangedEvent& rDCEvt )
{
    DockingWindow::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        initWindow();
        Invalidate();
    }
}

// --------------------------------------------------------------------

void ToolbarMenu::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_WHEEL )
    {
        const CommandWheelData* pData = rCEvt.GetWheelData();
        if( !pData->GetModifier() && ( pData->GetMode() == COMMAND_WHEEL_SCROLL ) )
        {
            implCursorUpDown( pData->GetDelta() > 0L, false );
        }
    }
}

// --------------------------------------------------------------------

Reference< ::com::sun::star::accessibility::XAccessible > ToolbarMenu::CreateAccessible()
{
    mpImpl->setAccessible( new ToolbarMenuAcc( *mpImpl ) );
    return Reference< XAccessible >( mpImpl->mxAccessible.get() );
}

// --------------------------------------------------------------------

// todo: move to new base class that will replace SfxPopupWindo
void ToolbarMenu::AddStatusListener( const rtl::OUString& rCommandURL )
{
    initStatusListener();
    mpImpl->mxStatusListener->addStatusListener( rCommandURL );
}

// --------------------------------------------------------------------

void ToolbarMenu::RemoveStatusListener( const rtl::OUString& rCommandURL )
{
    mpImpl->mxStatusListener->removeStatusListener( rCommandURL );
}
// --------------------------------------------------------------------


void ToolbarMenu::UpdateStatus( const rtl::OUString& rCommandURL )
{
    mpImpl->mxStatusListener->updateStatus( rCommandURL );
}

// --------------------------------------------------------------------

// XStatusListener (subclasses must override this one to get the status updates
void SAL_CALL ToolbarMenu::statusChanged( const ::com::sun::star::frame::FeatureStateEvent& /*Event*/ ) throw ( ::com::sun::star::uno::RuntimeException )
{
}

// --------------------------------------------------------------------

class ToolbarMenuStatusListener : public svt::FrameStatusListener
{
public:
    ToolbarMenuStatusListener( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xServiceManager,
                               const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                               ToolbarMenu& rToolbarMenu );

    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

    ToolbarMenu* mpMenu;
};

// --------------------------------------------------------------------

ToolbarMenuStatusListener::ToolbarMenuStatusListener(
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xServiceManager,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
    ToolbarMenu& rToolbarMenu )
: svt::FrameStatusListener( xServiceManager, xFrame )
, mpMenu( &rToolbarMenu )
{
}

// --------------------------------------------------------------------

void SAL_CALL ToolbarMenuStatusListener::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    mpMenu = 0;
    svt::FrameStatusListener::dispose();
}

// --------------------------------------------------------------------

void SAL_CALL ToolbarMenuStatusListener::statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException )
{
    if( mpMenu )
        mpMenu->statusChanged( Event );
}

// --------------------------------------------------------------------

void ToolbarMenu::initStatusListener()
{
    if( !mpImpl->mxStatusListener.is() )
        mpImpl->mxStatusListener.set( new ToolbarMenuStatusListener( mpImpl->mxServiceManager, mpImpl->mxFrame, *this ) );
}

// --------------------------------------------------------------------

bool ToolbarMenu::IsInPopupMode()
{
    return GetDockingManager()->IsInPopupMode(this);
}

// --------------------------------------------------------------------

void ToolbarMenu::EndPopupMode()
{
    GetDockingManager()->EndPopupMode(this);
}

// --------------------------------------------------------------------

const Size& ToolbarMenu::getMenuSize() const
{
    return mpImpl->maSize;
}

// --------------------------------------------------------------------

void ToolbarMenu::SetSelectHdl( const Link& rLink )
{
    mpImpl->maSelectHdl = rLink;
}

// --------------------------------------------------------------------

const Link& ToolbarMenu::GetSelectHdl() const
{
    return mpImpl->maSelectHdl;
}

// --------------------------------------------------------------------

Reference< XFrame > ToolbarMenu::GetFrame() const
{
    return mpImpl->mxFrame;
}

// --------------------------------------------------------------------


// --------------------------------------------------------------------

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
