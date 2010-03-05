/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: toolbarmenu.cxx,v $
 * $Revision: 1.6 $
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

#include <comphelper/processfactory.hxx>

#include <vcl/dockwin.hxx>
#include <vcl/menu.hxx>
#include <vcl/decoview.hxx>
#include <vcl/image.hxx>
#include <vcl/taskpanelist.hxx>

#include <svtools/valueset.hxx>

#include "svx/toolbarmenu.hxx"

const int EXTRAITEMHEIGHT = 0; // 4;
const int SEPARATOR_HEIGHT = 4;
const int TITLE_ID = -1;
const int BORDER_X = 0;
const int BORDER_Y = 0;

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

class ToolbarMenuEntry
{
public:
    int mnEntryId;
    MenuItemBits mnBits;
    Size maSize;

    bool mbHasText;
    bool mbHasImage;
    bool mbHasControl;
    bool mbChecked;
    bool mbEnabled;

    String maText;
    Image maImage;
    Control* mpControl;

public:
    ToolbarMenuEntry( int nEntryId, const String& rText, MenuItemBits nBits );
    ToolbarMenuEntry( int nEntryId, const Image& rImage, MenuItemBits nBits );
    ToolbarMenuEntry( int nEntryId, const Image& rImage, const String& rText, MenuItemBits nBits );
    ToolbarMenuEntry( int nEntryId, Control* pControl, MenuItemBits nBits );
    ToolbarMenuEntry( int nEntryId, const String& rText, Control* pControl, MenuItemBits nBits );
    ~ToolbarMenuEntry();

    void init( int nEntryId, MenuItemBits nBits );
};

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

ToolbarMenuEntry::ToolbarMenuEntry( int nEntryId, const String& rText, MenuItemBits nBits )
{
    init( nEntryId, nBits );

    maText = rText;
    mbHasText = true;
}

ToolbarMenuEntry::ToolbarMenuEntry( int nEntryId, const Image& rImage, MenuItemBits nBits )
{
    init( nEntryId, nBits );

    maImage = rImage;
    mbHasImage = true;
}

ToolbarMenuEntry::ToolbarMenuEntry( int nEntryId, const Image& rImage, const String& rText, MenuItemBits nBits )
{
    init( nEntryId, nBits );

    maText = rText;
    mbHasText = true;

    maImage = rImage;
    mbHasImage = true;
}

ToolbarMenuEntry::ToolbarMenuEntry( int nEntryId, Control* pControl, MenuItemBits nBits )
{
    init( nEntryId, nBits );

    if( pControl )
    {
        mpControl = pControl;
        mpControl->Show();
    }
}

ToolbarMenuEntry::ToolbarMenuEntry( int nEntryId, const String& rText, Control* pControl, MenuItemBits nBits )
{
    init( nEntryId, nBits );

    maText = rText;
    mbHasText = true;

    if( pControl )
    {
        mpControl = pControl;
        mpControl->Show();
    }
}

ToolbarMenuEntry::~ToolbarMenuEntry()
{
    delete mpControl;
}

ToolbarMenu::ToolbarMenu( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                          Window* pParentWindow,
                          WinBits nBits )
: DockingWindow(pParentWindow, nBits)
, mxFrame( rFrame )
{
    implInit();
}

ToolbarMenu::ToolbarMenu( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                          Window* pParentWindow,
                          const ResId& rResId )
: DockingWindow(pParentWindow, rResId)
, mxFrame( rFrame )
{
    implInit();
}

void ToolbarMenu::implInit()
{
//    SetStyle( GetStyle() | WB_CLIPCHILDREN | WB_NOBORDER | WB_SYSTEMWINDOW );

//    EnableChildTransparentMode();

    Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        ((SystemWindow *)pWindow)->GetTaskPaneList()->AddWindow( this );

    mxServiceManager = ::comphelper::getProcessServiceFactory();

    mnCheckPos = 0;
    mnImagePos = 0;
    mnTextPos = 0;

    mnHighlightedEntry = -1;
    mnSelectedEntry = -1;
    initWindow();
}

ToolbarMenu::~ToolbarMenu()
{
    Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        ((SystemWindow *)pWindow)->GetTaskPaneList()->RemoveWindow( this );

    if ( mxStatusListener.is() )
    {
        mxStatusListener->dispose();
        mxStatusListener.clear();
    }

    // delete all menu entries
    const int nEntryCount = maEntryVector.size();
    int nEntry;
    for( nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        delete maEntryVector[nEntry];
    }
}

int ToolbarMenu::getSelectedEntryId() const
{
    ToolbarMenuEntry* pEntry = implGetEntry( mnSelectedEntry );
    return pEntry ? pEntry->mnEntryId : -1;
}

int ToolbarMenu::getHighlightedEntryId() const
{
    ToolbarMenuEntry* pEntry = implGetEntry( mnHighlightedEntry );
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

bool ToolbarMenu::isEntryChecked( int nEntryId ) const
{
    ToolbarMenuEntry* pEntry = implSearchEntry( nEntryId );
    return pEntry && pEntry->mbChecked;
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

bool ToolbarMenu::isEntryEnabled( int nEntryId ) const
{
    ToolbarMenuEntry* pEntry = implSearchEntry( nEntryId );
    return pEntry && pEntry->mbEnabled;
}

void ToolbarMenu::setEntryText( int nEntryId, const String& rStr )
{
    ToolbarMenuEntry* pEntry = implSearchEntry( nEntryId );
    if( pEntry && pEntry->maText != rStr )
    {
        pEntry->maText = rStr;
        maSize = implCalcSize();
        if( IsVisible() )
            Invalidate();
    }
}

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

void ToolbarMenu::setEntryImage( int nEntryId, const Image& rImage )
{
    ToolbarMenuEntry* pEntry = implSearchEntry( nEntryId );
    if( pEntry && pEntry->maImage != rImage )
    {
        pEntry->maImage = rImage;
        maSize = implCalcSize();
        if( IsVisible() )
            Invalidate();
    }
}

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

void ToolbarMenu::initWindow()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    SetControlBackground( GetSettings().GetStyleSettings().GetFaceGradientColor() );

    SetPointFont( rStyleSettings.GetMenuFont() );
    SetBackground( Wallpaper( GetControlBackground() ) );
    SetTextColor( rStyleSettings.GetMenuTextColor() );
    SetTextFillColor();
    SetLineColor();

    maSize = implCalcSize();
}

Size ToolbarMenu::implCalcSize()
{
    const long nFontHeight = GetTextHeight();
    long nExtra = nFontHeight/4;

    Size aSz;
    Size aMaxImgSz;
    long nMaxTextWidth = 0;
    long nMinMenuItemHeight = nFontHeight;
    sal_Bool bCheckable = sal_False;

    const int nEntryCount = maEntryVector.size();
    int nEntry;

    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    if ( rSettings.GetUseImagesInMenus() )
    {
        nMinMenuItemHeight = 16;

        for( nEntry = 0; nEntry < nEntryCount; nEntry++ )
        {
            ToolbarMenuEntry* pEntry = maEntryVector[nEntry];
            if( pEntry && pEntry->mbHasImage )
            {
                Size aImgSz = pEntry->maImage.GetSizePixel();
                if ( aImgSz.Height() > aMaxImgSz.Height() )
                    aMaxImgSz.Height() = aImgSz.Height();
                if ( aImgSz.Height() > nMinMenuItemHeight )
                    nMinMenuItemHeight = aImgSz.Height();
                break;
            }
        }
    }

    for( nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* pEntry = maEntryVector[nEntry];

        if( pEntry )
        {
            pEntry->maSize.Height() = 0;
            pEntry->maSize.Width() = 0;


            if ( ( pEntry->mnBits ) & ( MIB_RADIOCHECK | MIB_CHECKABLE ) )
                bCheckable = sal_True;

            // Image:
            if( pEntry->mbHasImage )
            {
                Size aImgSz = pEntry->maImage.GetSizePixel();
                if ( (aImgSz.Width() + 4) > aMaxImgSz.Width() )
                    aMaxImgSz.Width() = aImgSz.Width() + 4;
                if ( (aImgSz.Height() + 4) > aMaxImgSz.Height() )
                    aMaxImgSz.Height() = aImgSz.Height() + 4;
                if ( (aImgSz.Height() + 4) > pEntry->maSize.Height() )
                    pEntry->maSize.Height() = aImgSz.Height() + 4;
            }
        }
    }

    int gfxExtra = Max( nExtra, 7L );

    if ( aMaxImgSz.Width() )
        mnTextPos += gfxExtra;
    if ( bCheckable )
        mnTextPos += 16;

    mnCheckPos = nExtra;
    mnImagePos = nExtra;
    mnTextPos = mnImagePos + aMaxImgSz.Width();

    for( nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* pEntry = maEntryVector[nEntry];

        if( pEntry )
        {
            // Text:
            if( pEntry->mbHasText )
            {
                long nTextWidth = GetCtrlTextWidth( pEntry->maText ) + mnTextPos;
                nMaxTextWidth = Max( nTextWidth, nMaxTextWidth );

                pEntry->maSize.Height() = Max( Max( GetTextHeight(), pEntry->maSize.Height() ), nMinMenuItemHeight );
            }
            // Control:
            else if( pEntry->mpControl )
            {
                Size aControlSize( pEntry->mpControl->GetOutputSizePixel() );

                nMaxTextWidth = Max( aControlSize.Width(), nMaxTextWidth );
                pEntry->maSize.Height() = Max( aControlSize.Height(), pEntry->maSize.Height() ) + 1;
            }
            aSz.Height() += pEntry->maSize.Height();
        }
        else
        {
            aSz.Height() += SEPARATOR_HEIGHT;
        }
    }

    aSz.Width() = nMaxTextWidth + (BORDER_X<<1);
    aSz.Height() += BORDER_Y<<1;

    // positionate controls
    int nY = BORDER_Y;
    for( nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* pEntry = maEntryVector[nEntry];

        if( pEntry )
        {
            if( pEntry->mpControl )
            {
                Size aControlSize( pEntry->mpControl->GetOutputSizePixel() );
                Point aControlPos( (aSz.Width() - aControlSize.Width())>>1, nY);
                if( pEntry->mbHasText )
                    aControlPos.X() += GetCtrlTextWidth( pEntry->maText ) + 4*gfxExtra;

                pEntry->mpControl->SetPosPixel( aControlPos );
            }

            nY += pEntry->maSize.Height();
        }
        else
        {
            nY += SEPARATOR_HEIGHT;
        }
    }

    return aSz;
}

void ToolbarMenu::GetFocus()
{
    OSL_TRACE("ToolbarMenu::GetFocus()");

    if( mnHighlightedEntry == -1 )
    {
        implChangeHighlightEntry( 0 );
    }
    DockingWindow::GetFocus();
}

void ToolbarMenu::LoseFocus()
{
    OSL_TRACE("ToolbarMenu::LoseFocus()");
    if( mnHighlightedEntry != -1 )
    {
        implChangeHighlightEntry( -1 );
    }
    DockingWindow::LoseFocus();
}

void ToolbarMenu::appendEntry( int nEntryId, const String& rStr, MenuItemBits nItemBits )
{
    appendEntry( new ToolbarMenuEntry( nEntryId, rStr, nItemBits ) );
}

void ToolbarMenu::appendEntry( int nEntryId, const Image& rImage, MenuItemBits nItemBits )
{
    appendEntry( new ToolbarMenuEntry( nEntryId, rImage, nItemBits ) );
}

void ToolbarMenu::appendEntry( int nEntryId, const String& rStr, const Image& rImage, MenuItemBits nItemBits )
{
    appendEntry( new ToolbarMenuEntry( nEntryId, rImage, rStr, nItemBits ) );
}

void ToolbarMenu::appendEntry( int nEntryId, Control* pControl, MenuItemBits nItemBits )
{
    appendEntry( new ToolbarMenuEntry( nEntryId, pControl, nItemBits ) );
}
/*
void ToolbarMenu::appendEntry( int nEntryId, const String& rStr, Control* pControl, MenuItemBits nItemBits )
{
    appendEntry( new ToolbarMenuEntry( nEntryId, rStr, pControl, nItemBits ) );
}
*/
void ToolbarMenu::appendEntry( ToolbarMenuEntry* pEntry )
{
    maEntryVector.push_back( pEntry );
    maSize = implCalcSize();
    if( IsVisible() )
        Invalidate();
}

void ToolbarMenu::appendSeparator()
{
    appendEntry( 0 );
}

void ToolbarMenu::Resize()
{
    DockingWindow::Resize();
}

ToolbarMenuEntry* ToolbarMenu::implGetEntry( int nEntry ) const
{
    if( (nEntry < 0) || (nEntry >= (int)maEntryVector.size() ) )
        return NULL;

    return maEntryVector[nEntry];
}

ToolbarMenuEntry* ToolbarMenu::implSearchEntry( int nEntryId ) const
{
    const int nEntryCount = maEntryVector.size();
    int nEntry;
    for( nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* p = maEntryVector[nEntry];
        if( p && p->mnEntryId == nEntryId )
        {
            return p;
        }
    }

    return NULL;
}

static void implDeselectControl( Control* pControl )
{
    ValueSet* pValueSet = dynamic_cast< ValueSet* >( pControl );
    if( pValueSet )
    {
        pValueSet->SetNoSelection();
        pValueSet->Invalidate();
    }
}

void ToolbarMenu::implHighlightEntry( int nHighlightEntry, bool bHighlight )
{
    Size    aSz = GetOutputSizePixel();
    long    nY = BORDER_Y;
    long    nX = BORDER_X;

    const int nEntryCount = maEntryVector.size();
    int nEntry;
    for( nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* p = maEntryVector[nEntry];
        if( p )
        {
            if(nEntry == nHighlightEntry)
            {
                if( (p->mpControl == NULL) || (p->mbHasText) )
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
                else if( p->mpControl != NULL )
                {
                    implDeselectControl( p->mpControl );
                }

                implPaint( p, bHighlight );

                maHighlightHdl.Call( this );
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

void ToolbarMenu::implSelectEntry( int nSelectedEntry )
{
    mnSelectedEntry = nSelectedEntry;

    ToolbarMenuEntry* pEntry = NULL;
    if( nSelectedEntry != -1 )
        pEntry = maEntryVector[ nSelectedEntry ];

    if( pEntry )
        maSelectHdl.Call( this );
}

void ToolbarMenu::MouseButtonDown( const MouseEvent& rMEvt )
{
    implHighlightEntry( rMEvt, true );

    implSelectEntry( mnHighlightedEntry );
}

void ToolbarMenu::MouseButtonUp( const MouseEvent& )
{
}

void ToolbarMenu::MouseMove( const MouseEvent& rMEvt )
{
    if ( !IsVisible() )
        return;

    implHighlightEntry( rMEvt, false );
}

void ToolbarMenu::implHighlightEntry( const MouseEvent& rMEvt, bool bMBDown )
{
    long nY = 0;
    long nMouseY = rMEvt.GetPosPixel().Y();
    Size aOutSz = GetOutputSizePixel();
    if ( ( nMouseY >= 0 ) && ( nMouseY < aOutSz.Height() ) )
    {
        bool bHighlighted = FALSE;

        const int nEntryCount = maEntryVector.size();
        int nEntry;
        for( nEntry = 0; nEntry < nEntryCount; nEntry++ )
        {
            ToolbarMenuEntry* pEntry = maEntryVector[nEntry];
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
                            if( nEntry != mnHighlightedEntry )
                            {
                                implChangeHighlightEntry( nEntry );
                            }
                        }
                        else
                        {
                            if ( nEntry != mnHighlightedEntry )
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

void ToolbarMenu::implChangeHighlightEntry( int nEntry )
{
    if( mnHighlightedEntry != -1 )
    {
        implHighlightEntry( mnHighlightedEntry, false );
    }

    mnHighlightedEntry = nEntry;
//    Invalidate();

    if( mnHighlightedEntry != -1 )
    {
        implHighlightEntry( mnHighlightedEntry, true );
    }
}

static bool implCheckSubControlCursorMove( Control* pControl, bool bUp )
{
    ValueSet* pValueSet = dynamic_cast< ValueSet* >( pControl );
    if( pValueSet )
    {
        USHORT nItemPos = pValueSet->GetItemPos( pValueSet->GetSelectItemId() );
        if( nItemPos != VALUESET_ITEM_NOTFOUND )
        {
            const USHORT nColCount = pValueSet->GetColCount();
            const USHORT nLine = nItemPos / nColCount;

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

ToolbarMenuEntry* ToolbarMenu::implCursorUpDown( bool bUp, bool bHomeEnd )
{
    int n = mnHighlightedEntry;
    if( n == -1 )
    {
        if( bUp )
            n = 0;
        else
            n = maEntryVector.size()-1;
    }
    else if( !bHomeEnd )
    {
        // if we have a currently selected entry and
        // cursor keys are used than check if this entry
        // has a control that can use those cursor keys
        ToolbarMenuEntry* pData = maEntryVector[n];
        if( pData && pData->mpControl && !pData->mbHasText )
        {
            if( implCheckSubControlCursorMove( pData->mpControl, bUp ) )
                return pData;
        }
    }

    int nLoop = n;

    if( bHomeEnd )
    {
        // absolute positioning
        if( bUp )
        {
            n = maEntryVector.size();
            nLoop = n-1;
        }
        else
        {
            n = -1;
            nLoop = n+1;
        }
    }

    do
    {
        if( bUp )
        {
            if ( n )
                n--;
            else
                if( mnHighlightedEntry == -1 )
                    n = maEntryVector.size()-1;
                else
                    break;
        }
        else
        {
            if( n < ((int)maEntryVector.size()-1) )
                n++;
            else
                if( mnHighlightedEntry == -1 )
                    n = 0;
                else
                    break;
        }

        ToolbarMenuEntry* pData = maEntryVector[n];
        if( pData && (pData->mnEntryId != TITLE_ID) )
        {
            implChangeHighlightEntry( n );
            return pData;
        }
    } while ( n != nLoop );

    return 0;
}

void ToolbarMenu::KeyInput( const KeyEvent& rKEvent )
{
    OSL_TRACE("ToolbarMenu::KeyInput()");
    Control* pForwardControl = 0;
    USHORT nCode = rKEvent.GetKeyCode().GetCode();
    switch ( nCode )
    {
        case KEY_UP:
        case KEY_DOWN:
        {
            int nOldEntry = mnHighlightedEntry;
            ToolbarMenuEntry*p = implCursorUpDown( nCode == KEY_UP, false );
            if( p && p->mpControl && !p->mbHasText )
            {
                OSL_TRACE("ToolbarMenu::KeyInput() - grab focus to control");
//              p->mpControl->GrabFocus();
                if( nOldEntry != mnHighlightedEntry )
                {
                    KeyCode aKeyCode( (nCode == KEY_UP) ? KEY_END : KEY_HOME );
                    KeyEvent aKeyEvent( 0, aKeyCode );
                    p->mpControl->KeyInput( aKeyEvent );
                    p->mpControl->Invalidate();
                }
                else if( !p->mpControl->HasFocus() )
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
            if( p && p->mpControl && !p->mbHasText )
            {
//              p->mpControl->GrabFocus();
                KeyCode aKeyCode( KEY_HOME );
                KeyEvent aKeyEvent( 0, aKeyCode );
                p->mpControl->KeyInput( aKeyEvent );
                p->mpControl->Invalidate();
            }
        }
        break;
        case KEY_F6:
        case KEY_ESCAPE:
        {
            // Ctrl-F6 acts like ESC here, the menu bar however will then put the focus in the document
//            if( nCode == KEY_F6 && !rKEvent.GetKeyCode().IsMod1() )
//                break;

            implSelectEntry( -1 );
        }
        break;

        case KEY_RETURN:
        {
            ToolbarMenuEntry* pEntry = implGetEntry( mnHighlightedEntry );
            if ( pEntry && pEntry->mbEnabled && (pEntry->mnEntryId != TITLE_ID) )
            {
                if( pEntry->mpControl )
                {
                    pForwardControl = pEntry->mpControl;
                }
                else
                {
                    implSelectEntry( mnHighlightedEntry );
                }
            }
        }
        break;
        default:
        {
            ToolbarMenuEntry* pEntry = implGetEntry( mnHighlightedEntry );
            if ( pEntry && pEntry->mbEnabled && pEntry->mpControl && !pEntry->mbHasText )
            {
//              pEntry->mpControl->GrabFocus();
                pForwardControl = pEntry->mpControl;
            }
        }

    }
    if( pForwardControl )
        pForwardControl->KeyInput( rKEvent );

}

void ToolbarMenu::implPaint( ToolbarMenuEntry* pThisOnly, bool bHighlighted )
{
    const long nFontHeight = GetTextHeight();
    const long nExtra = nFontHeight/4;

    DecorationView aDecoView( this );
    const StyleSettings& rSettings = GetSettings().GetStyleSettings();

    const Size aOutSz( GetOutputSizePixel() );

    Point aTopLeft( BORDER_X, BORDER_Y ), aTmpPos;

    const int nEntryCount = maEntryVector.size();
    int nEntry;
    for( nEntry = 0; nEntry < nEntryCount; nEntry++ )
    {
        ToolbarMenuEntry* pEntry = maEntryVector[nEntry];
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
                    aTmpPos.X() = aPos.X() + mnImagePos;
                    aTmpPos.Y() = aPos.Y();
                    aTmpPos.Y() += (pEntry->maSize.Height()-pEntry->maImage.GetSizePixel().Height())/2;
                    DrawImage( aTmpPos, pEntry->maImage, nImageStyle );
                }
                // Text:
                if( pEntry->mbHasText )
                {
                    aTmpPos.X() = aPos.X() + (bTitle ? 4 : mnTextPos);
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
                        aTmpPos.X() = aPos.X() + mnImagePos;
                        aTmpPos.Y() = aPos.Y();
                        aTmpPos.Y() += (pEntry->maSize.Height()-pEntry->maImage.GetSizePixel().Height())/2;

                        Rectangle aSelRect( aTmpPos, pEntry->maImage.GetSizePixel() );
                        aSelRect.nLeft -= 2;
                        aSelRect.nTop -= 2;
                        aSelRect.nRight += 2;
                        aSelRect.nBottom += 2;
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
                            aTmpPos.X() = aPos.X() + mnCheckPos;
                            eSymbol = SYMBOL_RADIOCHECKMARK;
                            aTmpPos.Y() -= nFontHeight/4;
                            aSelRect = Rectangle( aTmpPos, Size( nFontHeight/2, nFontHeight/2 ) );
                        }
                        else
                        {
                            aTmpPos.X() = aPos.X() + mnCheckPos;
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

void ToolbarMenu::Paint( const Rectangle& )
{
    implPaint();

    if( mnHighlightedEntry != -1 )
        implHighlightEntry( mnHighlightedEntry, true );
}

void ToolbarMenu::RequestHelp( const HelpEvent& rHEvt )
{
    DockingWindow::RequestHelp( rHEvt );
}

void ToolbarMenu::StateChanged( StateChangedType nType )
{
    DockingWindow::StateChanged( nType );

    if ( ( nType == STATE_CHANGE_CONTROLFOREGROUND ) || ( nType == STATE_CHANGE_CONTROLBACKGROUND ) )
    {
        initWindow();
        Invalidate();
    }
}

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

// todo: move to new base class that will replace SfxPopupWindo
void ToolbarMenu::AddStatusListener( const rtl::OUString& rCommandURL )
{
    initStatusListener();
    mxStatusListener->addStatusListener( rCommandURL );
}

// --------------------------------------------------------------------

void ToolbarMenu::RemoveStatusListener( const rtl::OUString& rCommandURL )
{
    mxStatusListener->removeStatusListener( rCommandURL );
}
// --------------------------------------------------------------------


void ToolbarMenu::UpdateStatus( const rtl::OUString& rCommandURL )
{
    mxStatusListener->updateStatus( rCommandURL );
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

ToolbarMenuStatusListener::ToolbarMenuStatusListener(
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xServiceManager,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
    ToolbarMenu& rToolbarMenu )
: svt::FrameStatusListener( xServiceManager, xFrame )
, mpMenu( &rToolbarMenu )
{
}

void SAL_CALL ToolbarMenuStatusListener::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    mpMenu = 0;
    svt::FrameStatusListener::dispose();
}

void SAL_CALL ToolbarMenuStatusListener::statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException )
{
    if( mpMenu )
        mpMenu->statusChanged( Event );
}

void ToolbarMenu::initStatusListener()
{
    if( !mxStatusListener.is() )
        mxStatusListener.set( new ToolbarMenuStatusListener( mxServiceManager, mxFrame, *this ) );
}

bool ToolbarMenu::IsInPopupMode()
{
    return GetDockingManager()->IsInPopupMode(this);
}

void ToolbarMenu::EndPopupMode()
{
    GetDockingManager()->EndPopupMode(this);
}


