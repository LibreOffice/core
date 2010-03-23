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
#include <comphelper/processfactory.hxx>

#include <vcl/dockwin.hxx>
#include <vcl/decoview.hxx>
#include <vcl/image.hxx>
#include <vcl/taskpanelist.hxx>

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
    mbHasControl = false;
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
        if( pEntry );
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

IMPL_LINK( ToolbarMenu, HighlightHdl, Control *, pControl )
{
    (void)pControl;
    if( mpImpl->hasAccessibleListeners() )
        mpImpl->fireAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );
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

    SetControlBackground( GetSettings().GetStyleSettings().GetFaceGradientColor() );

    SetPointFont( rStyleSettings.GetMenuFont() );
    SetBackground( Wallpaper( GetControlBackground() ) );
    SetTextColor( rStyleSettings.GetMenuTextColor() );
    SetTextFillColor();
    SetLineColor();

    mpImpl->maSize = implCalcSize();
}

// --------------------------------------------------------------------

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

void ToolbarMenu::GetFocus()
{
    if( mpImpl->mnHighlightedEntry == -1 )
        implChangeHighlightEntry( 0 );

    DockingWindow::GetFocus();

    // Tell the accessible object that we got the focus.
    if( mpImpl->mxAccessible.is() )
        mpImpl->mxAccessible->GetFocus();
}

// --------------------------------------------------------------------

void ToolbarMenu::LoseFocus()
{
    if( mpImpl->mnHighlightedEntry != -1 )
        implChangeHighlightEntry( -1 );

    DockingWindow::LoseFocus();

    // Tell the accessible object that we lost the focus.
    if( mpImpl->mxAccessible.is() )
        mpImpl->mxAccessible->LoseFocus();
}

// --------------------------------------------------------------------

void ToolbarMenu::appendEntry( int nEntryId, const String& rStr, MenuItemBits nItemBits )
{
    appendEntry( new ToolbarMenuEntry( *this, nEntryId, rStr, nItemBits ) );
}


#if 0
todo acc selectentry?
        if( ImplHasAccessibleListeners() )
        {
            // focus event (deselect)
            if( nOldItem )
            {
                const USHORT nPos = GetItemPos( nItemId );

                if( nPos != VALUESET_ITEM_NOTFOUND )
                {
                    ValueItemAcc* pItemAcc = ValueItemAcc::getImplementation(
                        mpImpl->mpItemList->GetObject( nPos )->GetAccessible( mpImpl->mbIsTransientChildrenDisabled ) );

                    if( pItemAcc )
                    {
                        ::com::sun::star::uno::Any aOldAny, aNewAny;
                        if( !mpImpl->mbIsTransientChildrenDisabled)
                        {
                            aOldAny <<= ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(
                                static_cast< ::cppu::OWeakObject* >( pItemAcc ));
                            ImplFireAccessibleEvent (::com::sun::star::accessibility::AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldAny, aNewAny );
                        }
                        else
                        {
                            aOldAny <<= ::com::sun::star::accessibility::AccessibleStateType::FOCUSED;
                            pItemAcc->FireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny );
                        }
                    }
                }
            }

            // focus event (select)
            const USHORT nPos = GetItemPos( mnSelItemId );

            ValueSetItem* pItem;
            if( nPos != VALUESET_ITEM_NOTFOUND )
                pItem = mpImpl->mpItemList->GetObject(nPos);
            else
                pItem = mpNoneItem;

            ValueItemAcc* pItemAcc = NULL;
            if (pItem != NULL)
                pItemAcc = ValueItemAcc::getImplementation(pItem->GetAccessible( mpImpl->mbIsTransientChildrenDisabled ) );

            if( pItemAcc )
            {
                ::com::sun::star::uno::Any aOldAny, aNewAny;
                if( !mpImpl->mbIsTransientChildrenDisabled)
                {
                    aNewAny <<= ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(
                        static_cast< ::cppu::OWeakObject* >( pItemAcc ));
                    ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldAny, aNewAny );
                }
                else
                {
                    aNewAny <<= ::com::sun::star::accessibility::AccessibleStateType::FOCUSED;
                    pItemAcc->FireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::STATE_CHANGED, aOldAny, aNewAny );
                }
            }

            // selection event
            ::com::sun::star::uno::Any aOldAny, aNewAny;
            ImplFireAccessibleEvent( ::com::sun::star::accessibility::AccessibleEventId::SELECTION_CHANGED, aOldAny, aNewAny );
        }
    }
}

#endif

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
    if( (nEntry < 0) || (nEntry >= (int)mpImpl->maEntryVector.size() ) )
        return NULL;

    return mpImpl->maEntryVector[nEntry];
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

static void implDeselectControl( Control* pControl )
{
    ValueSet* pValueSet = dynamic_cast< ValueSet* >( pControl );
    if( pValueSet )
    {
        pValueSet->SetNoSelection();
        pValueSet->Invalidate();
    }
}

// --------------------------------------------------------------------

void ToolbarMenu::implHighlightEntry( int nHighlightEntry, bool bHighlight )
{
    Size    aSz = GetOutputSizePixel();
    long    nY = BORDER_Y;
    long    nX = BORDER_X;

    const int nEntryCount = mpImpl->maEntryVector.size();
    int nEntry;
    for( nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* p = mpImpl->maEntryVector[nEntry];
        if( p )
        {
            if(nEntry == nHighlightEntry)
            {
                if( p->mpControl == NULL )
                {
                    Rectangle aRect( Point( nX, nY ), Size( aSz.Width()-(BORDER_X<<1), p->maSize.Height() ) );
                    SetFillColor( GetSettings().GetStyleSettings().GetMenuColor() );
                    SetLineColor();
                    DrawRect( aRect );

                    if( bHighlight )
                    {
                        aRect.nLeft += 1;
                        aRect.nTop += 1;
                        aRect.nBottom -= 1;
                        aRect.nRight -= 1;
                        DrawSelectionBackground( aRect, true, false, TRUE, TRUE );
                    }
                }
                else
                {
                    if( !bHighlight )
                        implDeselectControl( p->mpControl );
                }

                implPaint( p, bHighlight );
                break;
            }

            nY += p->maSize.Height();
        }
        else
        {
            nY += SEPARATOR_HEIGHT;
        }
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

    if( mpImpl->hasAccessibleListeners() )
        mpImpl->fireAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );
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

static void implHighlightControl( USHORT nCode, Control* pControl, int nLastColumn )
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
            nItemPos = std::min( ((nLastLine-1) * nColCount) + nLastColumn, nItemCount-1 );
            break;
        }
        case KEY_DOWN:
            nItemPos = std::min( nLastColumn, nItemCount-1 );
            break;
        case KEY_END:
            nItemPos = nItemCount -1;
            break;
        case KEY_HOME:
            nItemPos = 0;
            break;
        }
        pValueSet->SelectItem( pValueSet->GetItemId( nItemPos ) );
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
                    implHighlightControl( nCode, p->mpControl, mpImpl->mnLastColumn );
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
                implHighlightControl( nCode, p->mpControl, mpImpl->mnLastColumn );
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

void ToolbarMenu::implPaint( ToolbarMenuEntry* pThisOnly, bool bHighlighted )
{
    const long nFontHeight = GetTextHeight();
    const long nExtra = nFontHeight/4;

    DecorationView aDecoView( this );
    const StyleSettings& rSettings = GetSettings().GetStyleSettings();

    const Size aOutSz( GetOutputSizePixel() );

    Point aTopLeft( BORDER_X, BORDER_Y ), aTmpPos;

    const int nEntryCount = mpImpl->maEntryVector.size();
    int nEntry;
    for( nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* pEntry = mpImpl->maEntryVector[nEntry];
        Point aPos( aTopLeft );

        USHORT  nTextStyle   = 0;
        USHORT  nSymbolStyle = 0;
        USHORT  nImageStyle  = 0;
        if( pEntry && !pEntry->mbEnabled )
        {
            nTextStyle   |= TEXT_DRAW_DISABLE;
            nSymbolStyle |= SYMBOL_DRAW_DISABLE;
            nImageStyle  |= IMAGE_DRAW_DISABLE;
        }

        // Separator
        if( pEntry == NULL )
        {
            if( pThisOnly == NULL  )
            {
                aTmpPos.Y() = aPos.Y() + ((SEPARATOR_HEIGHT-2)/2);

                SetLineColor( rSettings.GetShadowColor() );
                DrawLine( aTmpPos, Point( aOutSz.Width() - 3, aTmpPos.Y() ) );
                aTmpPos.Y()++;
                SetLineColor( rSettings.GetLightColor() );
                DrawLine( aTmpPos, Point( aOutSz.Width() - 3, aTmpPos.Y() ) );
                SetLineColor();
            }

            aTopLeft.Y() += SEPARATOR_HEIGHT;
        }
        else
        {
            if( !pThisOnly || ( pEntry == pThisOnly ) )
            {
                const bool bTitle = pEntry->mnEntryId == TITLE_ID;

                if( pThisOnly && bHighlighted )
                    SetTextColor( rSettings.GetMenuHighlightTextColor() );
                else
                    SetTextColor( rSettings.GetMenuTextColor() );

                Rectangle aRect( aTopLeft, Size( aOutSz.Width(), pEntry->maSize.Height() ) );
                if( bTitle )
                {
                    // fill the background
                    SetFillColor(rSettings.GetDialogColor());
                    SetLineColor();
                    DrawRect(aRect);
                    SetLineColor( rSettings.GetLightColor() );
                    DrawLine( aRect.TopLeft(), aRect.TopRight() );
                    SetLineColor( rSettings.GetShadowColor() );
                    DrawLine( aRect.BottomLeft(), aRect.BottomRight() );
                }
                else if( pEntry->mpControl )
                {
                    SetLineColor( rSettings.GetShadowColor() );
                    DrawLine( aRect.BottomLeft(), aRect.BottomRight() );
                }

                long nTextOffsetY = ((pEntry->maSize.Height()-nFontHeight)/2);

                // Image
                if( pEntry->mbHasImage )
                {
                    aTmpPos.X() = aPos.X() + mpImpl->mnImagePos;
                    aTmpPos.Y() = aPos.Y();
                    aTmpPos.Y() += (pEntry->maSize.Height()-pEntry->maImage.GetSizePixel().Height())/2;
                    DrawImage( aTmpPos, pEntry->maImage, nImageStyle );
                }
                // Text:
                if( pEntry->mbHasText )
                {
                    aTmpPos.X() = aPos.X() + (bTitle ? 4 : mpImpl->mnTextPos);
                    aTmpPos.Y() = aPos.Y();
                    aTmpPos.Y() += nTextOffsetY;
                    USHORT nStyle = nTextStyle|TEXT_DRAW_MNEMONIC;

                    DrawCtrlText( aTmpPos, pEntry->maText, 0, pEntry->maText.Len(), nStyle );
                }

                // CheckMark
                if( pEntry->mbChecked )
                {
                    if( pEntry->mbHasImage )
                    {
                        aTmpPos.X() = aPos.X() + mpImpl->mnImagePos;
                        aTmpPos.Y() = aPos.Y();
                        aTmpPos.Y() += (pEntry->maSize.Height()-pEntry->maImage.GetSizePixel().Height())/2;

                        Rectangle aSelRect( aTmpPos, pEntry->maImage.GetSizePixel() );
                        aSelRect.nLeft -= 1;
                        aSelRect.nTop -= 1;
                        aSelRect.nRight += 1;
                        aSelRect.nBottom += 1;
                        DrawSelectionBackground( aSelRect, false, true, TRUE, TRUE );
                    }
                    else
                    {
                        Rectangle aSelRect;
                        SymbolType eSymbol;
                        aTmpPos.Y() = aPos.Y();
                        aTmpPos.Y() += nExtra/2;
                        aTmpPos.Y() += pEntry->maSize.Height() / 2;
                        if ( pEntry->mnBits & MIB_RADIOCHECK )
                        {
                            aTmpPos.X() = aPos.X() + mpImpl->mnCheckPos;
                            eSymbol = SYMBOL_RADIOCHECKMARK;
                            aTmpPos.Y() -= nFontHeight/4;
                            aSelRect = Rectangle( aTmpPos, Size( nFontHeight/2, nFontHeight/2 ) );
                        }
                        else
                        {
                            aTmpPos.X() = aPos.X() + mpImpl->mnCheckPos;
                            eSymbol = SYMBOL_CHECKMARK;
                            aTmpPos.Y() -= nFontHeight/4;
                            aSelRect = Rectangle( aTmpPos, Size( (nFontHeight*25)/40, nFontHeight/2 ) );
                        }
                        aDecoView.DrawSymbol( aSelRect, eSymbol, GetTextColor(), nSymbolStyle );
                    }
                }
            }

            aTopLeft.Y() += pEntry->maSize.Height();
        }
    }
}

// --------------------------------------------------------------------

void ToolbarMenu::Paint( const Rectangle& )
{
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


