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

#include "svtools/controldims.hrc"

#include "RemoteDialogClientBox.hxx"
#include "RemoteDialog.hrc"

#include "comphelper/processfactory.hxx"
#include "com/sun/star/i18n/CollatorOptions.hpp"
#include "com/sun/star/deployment/DependencyException.hpp"
#include "com/sun/star/deployment/DeploymentException.hpp"

#define USER_PACKAGE_MANAGER    "user"
#define SHARED_PACKAGE_MANAGER  "shared"
#define BUNDLED_PACKAGE_MANAGER "bundled"

using namespace ::com::sun::star;

namespace sd {

//------------------------------------------------------------------------------
//                          struct ClientBoxEntry
//------------------------------------------------------------------------------
ClientBoxEntry::ClientBoxEntry( ClientInfo* pClientInfo ) :
    m_bActive( false ),
    m_pClientInfo( pClientInfo )
{
}

//------------------------------------------------------------------------------
ClientBoxEntry::~ClientBoxEntry()
{}


//------------------------------------------------------------------------------
// ClientRemovedListener
//------------------------------------------------------------------------------
void ClientRemovedListener::disposing( lang::EventObject const & rEvt )
    throw ( uno::RuntimeException )
{
    (void) rEvt;
}

//------------------------------------------------------------------------------
ClientRemovedListener::~ClientRemovedListener()
{
}

//------------------------------------------------------------------------------
// ClientBox
//------------------------------------------------------------------------------
ClientBox::ClientBox( Dialog* pParent, RemoteServer *pServer,
                      const SdResId& aId ) :
    Control( pParent, aId ),
    m_bHasScrollBar( false ),
    m_bHasActive( false ),
    m_bNeedsRecalc( true ),
    m_bHasNew( false ),
    m_bInCheckMode( false ),
    m_bAdjustActive( false ),
    m_bInDelete( false ),
    m_nActive( 0 ),
    m_nTopIndex( 0 ),
    m_nActiveHeight( 0 ),
    m_nExtraHeight( 2 ),
    m_aPinBox( this, SdResId( INPUT_PIN ) ),
    m_aPinDescription( this, SdResId( TEXT_PIN ) ),
    m_pScrollBar( new ScrollBar( this, WB_VERT ) ),
    m_pServer( pServer )
{
    m_pScrollBar->SetScrollHdl( LINK( this, ClientBox, ScrollHdl ) );
    m_pScrollBar->EnableDrag();

    m_aPinBox.SetUseThousandSep(false);

    SetPaintTransparent( true );
    SetPosPixel( Point( RSC_SP_DLG_INNERBORDER_LEFT, RSC_SP_DLG_INNERBORDER_TOP ) );
    long nIconHeight = 2*TOP_OFFSET + SMALL_ICON_SIZE;
    long nTitleHeight = 2*TOP_OFFSET + GetTextHeight();
    if ( nIconHeight < nTitleHeight )
        m_nStdHeight = nTitleHeight;
    else
        m_nStdHeight = nIconHeight;
    m_nStdHeight += GetTextHeight() + TOP_OFFSET;

//     nIconHeight = ICON_HEIGHT + 2*TOP_OFFSET + 1;
//     if ( m_nStdHeight < nIconHeight )
//         m_nStdHeight = nIconHeight;

    m_nActiveHeight = m_nStdHeight;

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if( IsControlBackground() )
        SetBackground( GetControlBackground() );
    else
        SetBackground( rStyleSettings.GetFieldColor() );

    m_xRemoveListener = new ClientRemovedListener( this );

    Show();
}

//------------------------------------------------------------------------------
ClientBox::~ClientBox()
{
    if ( ! m_bInDelete )
        DeleteRemoved();

    m_bInDelete = true;

    typedef std::vector< TClientBoxEntry >::iterator ITER;

    for ( ITER iIndex = m_vEntries.begin(); iIndex < m_vEntries.end(); ++iIndex )
    {
//         (*iIndex)->m_xPackage->removeEventListener( uno::Reference< lang::XEventListener > ( m_xRemoveListener, uno::UNO_QUERY ) );
    }

    m_vEntries.clear();

    m_xRemoveListener.clear();
}


//------------------------------------------------------------------------------
void ClientBox::checkIndex( sal_Int32 nIndex ) const
{
    if ( nIndex < 0 )
        throw lang::IllegalArgumentException( "The list index starts with 0",0, 0 );
    if ( static_cast< sal_uInt32 >( nIndex ) >= m_vEntries.size())
        throw lang::IllegalArgumentException( "There is no element at the provided position."
        "The position exceeds the number of available list entries",0, 0 );
}

//------------------------------------------------------------------------------
// Title + description
void ClientBox::CalcActiveHeight( const long nPos )
{
    (void) nPos;
    const ::osl::MutexGuard aGuard( m_entriesMutex );

    // get title height
    long aTextHeight;
    long nIconHeight = 2*TOP_OFFSET + SMALL_ICON_SIZE;
    long nTitleHeight = 2*TOP_OFFSET + GetTextHeight();
    if ( nIconHeight < nTitleHeight )
        aTextHeight = nTitleHeight;
    else
        aTextHeight = nIconHeight;

    // Text entry height
    Size aSize = GetOutputSizePixel();
    if ( m_bHasScrollBar )
        aSize.Width() -= m_pScrollBar->GetSizePixel().Width();

    aSize.Width() -= ICON_OFFSET;


    aTextHeight += m_aPinBox.GetOutputHeightPixel();

    if ( aTextHeight < m_nStdHeight )
        aTextHeight = m_nStdHeight;

    m_nActiveHeight = aTextHeight + 2;
}

//------------------------------------------------------------------------------
const Size ClientBox::GetMinOutputSizePixel() const
{
    return Size( 200, 80 );
}

//------------------------------------------------------------------------------
Rectangle ClientBox::GetEntryRect( const long nPos ) const
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );

    Size aSize( GetOutputSizePixel() );

    if ( m_bHasScrollBar )
        aSize.Width() -= m_pScrollBar->GetSizePixel().Width();

    if ( m_vEntries[ nPos ]->m_bActive )
        aSize.Height() = m_nActiveHeight;
    else
        aSize.Height() = m_nStdHeight;

    Point aPos( 0, -m_nTopIndex + nPos * m_nStdHeight );
    if ( m_bHasActive && ( nPos < m_nActive ) )
        aPos.Y() += m_nActiveHeight - m_nStdHeight;

    return Rectangle( aPos, aSize );
}

//------------------------------------------------------------------------------
void ClientBox::DeleteRemoved()
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );

    m_bInDelete = true;

    if ( ! m_vRemovedEntries.empty() )
    {
        typedef std::vector< TClientBoxEntry >::iterator ITER;

        m_vRemovedEntries.clear();
    }

    m_bInDelete = false;
}

long ClientBox::GetActiveEntryIndex()
{
    if ( m_bHasActive )
        return m_nActive;
    else
        return -1;
}

//------------------------------------------------------------------------------
//This function may be called with nPos < 0
void ClientBox::selectEntry( const long nPos )
{
    //ToDo whe should not use the guard at such a big scope here.
    //Currently it is used to gard m_vEntries and m_nActive. m_nActive will be
    //modified in this function.
    //It would be probably best to always use a copy of m_vEntries
    //and some other state variables from ClientBox for
    //the whole painting operation. See issue i86993
    ::osl::ClearableMutexGuard guard(m_entriesMutex);

    if ( m_bInCheckMode )
        return;

    if ( m_bHasActive )
    {
        if ( nPos == m_nActive )
            return;

        m_bHasActive = false;
        m_vEntries[ m_nActive ]->m_bActive = false;
    }

    if ( ( nPos >= 0 ) && ( nPos < (long) m_vEntries.size() ) )
    {
        m_bHasActive = true;
        m_nActive = nPos;
        m_vEntries[ nPos ]->m_bActive = true;

        if ( IsReallyVisible() )
        {
            m_bAdjustActive = true;
        }
    }

    if ( IsReallyVisible() )
    {
        m_bNeedsRecalc = true;
        Invalidate();
    }

    guard.clear();
}

// -----------------------------------------------------------------------
void ClientBox::DrawRow( const Rectangle& rRect, const TClientBoxEntry pEntry )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( pEntry->m_bActive )
        SetTextColor( rStyleSettings.GetHighlightTextColor() );
    else
        SetTextColor( rStyleSettings.GetFieldTextColor() );

    if ( pEntry->m_bActive )
    {
        SetLineColor();
        SetFillColor( rStyleSettings.GetHighlightColor() );
        DrawRect( rRect );
    }
    else
    {
        if( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( rStyleSettings.GetFieldColor() );

        SetTextFillColor();
        Erase( rRect );
    }

    // FIXME: draw bluetooth or wifi icon
     Point aPos( rRect.TopLeft() );
//     aPos += Point( TOP_OFFSET, TOP_OFFSET );
//     Image aImage;
//     if ( ! pEntry->m_aIcon )
//         aImage = m_aDefaultImage;
//     else
//         aImage = pEntry->m_aIcon;
//     Size aImageSize = aImage.GetSizePixel();
//     if ( ( aImageSize.Width() <= ICON_WIDTH ) && ( aImageSize.Height() <= ICON_HEIGHT ) )
//         DrawImage( Point( aPos.X()+((ICON_WIDTH-aImageSize.Width())/2), aPos.Y()+((ICON_HEIGHT-aImageSize.Height())/2) ), aImage );
//     else
//         DrawImage( aPos, Size( ICON_WIDTH, ICON_HEIGHT ), aImage );

    // Setup fonts
    Font aStdFont( GetFont() );
    Font aBoldFont( aStdFont );
    aBoldFont.SetWeight( WEIGHT_BOLD );
    SetFont( aBoldFont );
    long aTextHeight = GetTextHeight();

    // Get max title width
    long nMaxTitleWidth = rRect.GetWidth() - ICON_OFFSET;
    nMaxTitleWidth -= ( 2 * SMALL_ICON_SIZE ) + ( 4 * SPACE_BETWEEN );

    long aTitleWidth = GetTextWidth( String( pEntry->m_pClientInfo->mName ) ) + (aTextHeight / 3);

    aPos = rRect.TopLeft() + Point( ICON_OFFSET, TOP_OFFSET );

    if ( aTitleWidth > nMaxTitleWidth )
    {
        aTitleWidth = nMaxTitleWidth - (aTextHeight / 3);
        String aShortTitle = GetEllipsisString( pEntry->m_pClientInfo->mName,
                                                aTitleWidth );
        DrawText( aPos, aShortTitle );
        aTitleWidth += (aTextHeight / 3);
    }
    else
        DrawText( aPos, pEntry->m_pClientInfo->mName );

    SetFont( aStdFont );

    aPos.Y() += aTextHeight;
    if ( pEntry->m_bActive )
    {
        const Rectangle aRect( GetEntryRect( m_nActive ) );
        Size  aBtnSize( m_aPinBox.GetSizePixel() );
        Point aBtnPos( aRect.Left() + ICON_OFFSET,
                   aRect.Bottom() - TOP_OFFSET - aBtnSize.Height() );

        m_aPinBox.SetPosPixel( aBtnPos );

        Size aSize = LogicToPixel( Size( RSC_CD_PUSHBUTTON_WIDTH, RSC_CD_PUSHBUTTON_HEIGHT ),
                               MapMode( MAP_APPFONT ) );
    m_aPinBox.SetSizePixel( aSize );
//         long nExtraHeight = 0;
//
//         if ( pEntry->m_bHasButtons )
//             nExtraHeight = m_nExtraHeight;
//
//         DrawText( Rectangle( aPos.X(), aPos.Y(), rRect.Right(), rRect.Bottom() - nExtraHeight ),
//                   sDescription, TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
    }
    else
    {
    }

    SetLineColor( Color( COL_LIGHTGRAY ) );
    DrawLine( rRect.BottomLeft(), rRect.BottomRight() );
}

// -----------------------------------------------------------------------
void ClientBox::RecalcAll()
{
    if ( m_bHasActive )
        CalcActiveHeight( m_nActive );

    SetupScrollBar();

    if ( m_bHasActive )
    {
        Rectangle aEntryRect = GetEntryRect( m_nActive );

        if ( m_bAdjustActive )
        {
            m_bAdjustActive = false;

            // If the top of the selected entry isn't visible, make it visible
            if ( aEntryRect.Top() < 0 )
            {
                m_nTopIndex += aEntryRect.Top();
                aEntryRect.Move( 0, -aEntryRect.Top() );
            }

            // If the bottom of the selected entry isn't visible, make it visible even if now the top
            // isn't visible any longer ( the buttons are more important )
            Size aOutputSize = GetOutputSizePixel();
            if ( aEntryRect.Bottom() > aOutputSize.Height() )
            {
                m_nTopIndex += ( aEntryRect.Bottom() - aOutputSize.Height() );
                aEntryRect.Move( 0, -( aEntryRect.Bottom() - aOutputSize.Height() ) );
            }

            // If there is unused space below the last entry but all entries don't fit into the box,
            // move the content down to use the whole space
            const long nTotalHeight = GetTotalHeight();
            if ( m_bHasScrollBar && ( aOutputSize.Height() + m_nTopIndex > nTotalHeight ) )
            {
                long nOffset = m_nTopIndex;
                m_nTopIndex = nTotalHeight - aOutputSize.Height();
                nOffset -= m_nTopIndex;
                aEntryRect.Move( 0, nOffset );
            }

            if ( m_bHasScrollBar )
                m_pScrollBar->SetThumbPos( m_nTopIndex );
        }
    }

    m_bNeedsRecalc = false;
}

// -----------------------------------------------------------------------
bool ClientBox::HandleTabKey( bool )
{
    return false;
}

// -----------------------------------------------------------------------
bool ClientBox::HandleCursorKey( sal_uInt16 nKeyCode )
{
    if ( m_vEntries.empty() )
        return true;

    long nSelect = 0;

    if ( m_bHasActive )
    {
        long nPageSize = GetOutputSizePixel().Height() / m_nStdHeight;
        if ( nPageSize < 2 )
            nPageSize = 2;

        if ( ( nKeyCode == KEY_DOWN ) || ( nKeyCode == KEY_RIGHT ) )
            nSelect = m_nActive + 1;
        else if ( ( nKeyCode == KEY_UP ) || ( nKeyCode == KEY_LEFT ) )
            nSelect = m_nActive - 1;
        else if ( nKeyCode == KEY_HOME )
            nSelect = 0;
        else if ( nKeyCode == KEY_END )
            nSelect = m_vEntries.size() - 1;
        else if ( nKeyCode == KEY_PAGEUP )
            nSelect = m_nActive - nPageSize + 1;
        else if ( nKeyCode == KEY_PAGEDOWN )
            nSelect = m_nActive + nPageSize - 1;
    }
    else // when there is no selected entry, we will select the first or the last.
    {
        if ( ( nKeyCode == KEY_DOWN ) || ( nKeyCode == KEY_PAGEDOWN ) || ( nKeyCode == KEY_HOME ) )
            nSelect = 0;
        else if ( ( nKeyCode == KEY_UP ) || ( nKeyCode == KEY_PAGEUP ) || ( nKeyCode == KEY_END ) )
            nSelect = m_vEntries.size() - 1;
    }

    if ( nSelect < 0 )
        nSelect = 0;
    if ( nSelect >= (long) m_vEntries.size() )
        nSelect = m_vEntries.size() - 1;

    selectEntry( nSelect );

    return true;
}

// -----------------------------------------------------------------------
void ClientBox::Paint( const Rectangle &/*rPaintRect*/ )
{
    if ( !m_bInDelete )
        DeleteRemoved();

    if ( m_bNeedsRecalc )
        RecalcAll();

    Point aStart( 0, -m_nTopIndex );
    Size aSize( GetOutputSizePixel() );

    if ( m_bHasScrollBar )
        aSize.Width() -= m_pScrollBar->GetSizePixel().Width();

    const ::osl::MutexGuard aGuard( m_entriesMutex );

    typedef std::vector< TClientBoxEntry >::iterator ITER;
    for ( ITER iIndex = m_vEntries.begin(); iIndex < m_vEntries.end(); ++iIndex )
    {
        aSize.Height() = (*iIndex)->m_bActive ? m_nActiveHeight : m_nStdHeight;
        Rectangle aEntryRect( aStart, aSize );
        DrawRow( aEntryRect, *iIndex );
        aStart.Y() += aSize.Height();
    }
}

// -----------------------------------------------------------------------
long ClientBox::GetTotalHeight() const
{
    long nHeight = m_vEntries.size() * m_nStdHeight;

    if ( m_bHasActive )
    {
        nHeight += m_nActiveHeight - m_nStdHeight;
    }

    return nHeight;
}

// -----------------------------------------------------------------------
void ClientBox::SetupScrollBar()
{
    const Size aSize = GetOutputSizePixel();
    const long nScrBarSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    const long nTotalHeight = GetTotalHeight();
    const bool bNeedsScrollBar = ( nTotalHeight > aSize.Height() );

    if ( bNeedsScrollBar )
    {
        if ( m_nTopIndex + aSize.Height() > nTotalHeight )
            m_nTopIndex = nTotalHeight - aSize.Height();

        m_pScrollBar->SetPosSizePixel( Point( aSize.Width() - nScrBarSize, 0 ),
                                       Size( nScrBarSize, aSize.Height() ) );
        m_pScrollBar->SetRangeMax( nTotalHeight );
        m_pScrollBar->SetVisibleSize( aSize.Height() );
        m_pScrollBar->SetPageSize( ( aSize.Height() * 4 ) / 5 );
        m_pScrollBar->SetLineSize( m_nStdHeight );
        m_pScrollBar->SetThumbPos( m_nTopIndex );

        if ( !m_bHasScrollBar )
            m_pScrollBar->Show();
    }
    else if ( m_bHasScrollBar )
    {
        m_pScrollBar->Hide();
        m_nTopIndex = 0;
    }

    m_bHasScrollBar = bNeedsScrollBar;
}

// -----------------------------------------------------------------------
void ClientBox::Resize()
{
    RecalcAll();
}

//------------------------------------------------------------------------------
long ClientBox::PointToPos( const Point& rPos )
{
    long nPos = ( rPos.Y() + m_nTopIndex ) / m_nStdHeight;

    if ( m_bHasActive && ( nPos > m_nActive ) )
    {
        if ( rPos.Y() + m_nTopIndex <= m_nActive*m_nStdHeight + m_nActiveHeight )
            nPos = m_nActive;
        else
            nPos = ( rPos.Y() + m_nTopIndex - (m_nActiveHeight - m_nStdHeight) ) / m_nStdHeight;
    }

    return nPos;
}

OUString ClientBox::getPin()
{
    return OUString::valueOf( m_aPinBox.GetValue() );
}

//------------------------------------------------------------------------------
void ClientBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    long nPos = PointToPos( rMEvt.GetPosPixel() );

    if ( rMEvt.IsLeft() )
    {
        if ( rMEvt.IsMod1() && m_bHasActive )
            selectEntry( m_vEntries.size() );   // Selecting an not existing entry will deselect the current one
        else
            selectEntry( nPos );
    }
}

//------------------------------------------------------------------------------
long ClientBox::Notify( NotifyEvent& rNEvt )
{
    if ( !m_bInDelete )
        DeleteRemoved();

    bool bHandled = false;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        KeyCode         aKeyCode = pKEvt->GetKeyCode();
        sal_uInt16          nKeyCode = aKeyCode.GetCode();

        if ( nKeyCode == KEY_TAB )
            bHandled = HandleTabKey( aKeyCode.IsShift() );
        else if ( aKeyCode.GetGroup() == KEYGROUP_CURSOR )
            bHandled = HandleCursorKey( nKeyCode );
    }

    if ( rNEvt.GetType() == EVENT_COMMAND )
    {
        if ( m_bHasScrollBar &&
             ( rNEvt.GetCommandEvent()->GetCommand() == COMMAND_WHEEL ) )
        {
            const CommandWheelData* pData = rNEvt.GetCommandEvent()->GetWheelData();
            if ( pData->GetMode() == COMMAND_WHEEL_SCROLL )
            {
                long nThumbPos = m_pScrollBar->GetThumbPos();
                if ( pData->GetDelta() < 0 )
                    m_pScrollBar->DoScroll( nThumbPos + m_nStdHeight );
                else
                    m_pScrollBar->DoScroll( nThumbPos - m_nStdHeight );
                bHandled = true;
            }
        }
    }

    if ( !bHandled )
        return Control::Notify( rNEvt );
    else
        return true;
}


//------------------------------------------------------------------------------
long ClientBox::addEntry( ClientInfo* pClientInfo )
{
    long         nPos = 0;
//     PackageState eState = m_pManager->getPackageState( xPackage );
//     bool         bLocked = m_pManager->isReadOnly( xPackage );

    TClientBoxEntry pEntry( new ClientBoxEntry( pClientInfo ) );

    bool bNewEntryInserted = false;

    ::osl::ClearableMutexGuard guard(m_entriesMutex);
    if ( m_vEntries.empty() )
    {
        m_vEntries.push_back( pEntry );
        bNewEntryInserted = true;
    }
    else
    {
//         if ( !FindEntryPos( pEntry, 0, m_vEntries.size()-1, nPos ) )
//         {
            m_vEntries.insert( m_vEntries.begin()+nPos, pEntry );
            bNewEntryInserted = true;
//         }
//         else if ( !m_bInCheckMode )
//         {
//             OSL_FAIL( "ClientBox::addEntry(): Will not add duplicate entries"  );
//         }
    }

    //Related: rhbz#702833 Only add a Listener if we're adding a new entry, to
    //keep in sync with removeEventListener logic
    if (bNewEntryInserted)
    {

        //         pEntry->m_xPackage->addEventListener(uno::Reference< lang::XEventListener > ( m_xRemoveListener, uno::UNO_QUERY ) );
    }


//     pEntry->m_bHasOptions = m_pManager->supportsOptions( xPackage );
//     pEntry->m_bUser       = xPackage->getRepositoryName().equals( USER_PACKAGE_MANAGER );
//     pEntry->m_bShared     = xPackage->getRepositoryName().equals( SHARED_PACKAGE_MANAGER );
//     pEntry->m_bNew        = m_bInCheckMode;
//     pEntry->m_bMissingLic = bLicenseMissing;

//     if ( bLicenseMissing )
//         pEntry->m_sErrorText = DialogHelper::getResourceString( RID_STR_ERROR_MISSING_LICENSE );

    //access to m_nActive must be guarded
    if ( !m_bInCheckMode && m_bHasActive && ( m_nActive >= nPos ) )
        m_nActive += 1;

    guard.clear();

    if ( IsReallyVisible() )
        Invalidate();

    m_bNeedsRecalc = true;

    return nPos;
}

//------------------------------------------------------------------------------
void ClientBox::updateEntry( const ClientInfo* pClientInfo )
{
    (void) pClientInfo;
//     typedef std::vector< TClientBoxEntry >::iterator ITER;
//     for ( ITER iIndex = m_vEntries.begin(); iIndex < m_vEntries.end(); ++iIndex )
//     {
//         if ( (*iIndex)->m_xPackage == xPackage )
//         {
//             PackageState eState = m_pManager->getPackageState( xPackage );
//             (*iIndex)->m_bHasOptions = m_pManager->supportsOptions( xPackage );
//             (*iIndex)->m_eState = eState;
//             (*iIndex)->m_sTitle = xPackage->getDisplayName();
//             (*iIndex)->m_sVersion = xPackage->getVersion();
//             (*iIndex)->m_sDescription = xPackage->getDescription();
//
//             if ( eState == REGISTERED )
//                 (*iIndex)->m_bMissingLic = false;
//
//             if ( eState == AMBIGUOUS )
//                 (*iIndex)->m_sErrorText = DialogHelper::getResourceString( RID_STR_ERROR_UNKNOWN_STATUS );
//             else if ( ! (*iIndex)->m_bMissingLic )
//                 (*iIndex)->m_sErrorText = String();
//
//             if ( IsReallyVisible() )
//                 Invalidate();
//             break;
//         }
//     }
}

//------------------------------------------------------------------------------
void ClientBox::removeEntry( const ClientInfo* pClientInfo )
{
    (void) pClientInfo;
//     if ( ! m_bInDelete )
//     {
//         ::osl::ClearableMutexGuard aGuard( m_entriesMutex );
//
//         typedef std::vector< TClientBoxEntry >::iterator ITER;
//
//         for ( ITER iIndex = m_vEntries.begin(); iIndex < m_vEntries.end(); ++iIndex )
//         {
//             if ( (*iIndex)->m_xPackage == xPackage )
//             {
//                 long nPos = iIndex - m_vEntries.begin();
//
//                 // Entries mustn't removed here, because they contain a hyperlink control
//                 // which can only be deleted when the thread has the solar mutex. Therefor
//                 // the entry will be moved into the m_vRemovedEntries list which will be
//                 // cleared on the next paint event
//                 m_vRemovedEntries.push_back( *iIndex );
//                 m_vEntries.erase( iIndex );
//
//                 m_bNeedsRecalc = true;
//
//                 if ( IsReallyVisible() )
//                     Invalidate();
//
//                 if ( m_bHasActive )
//                 {
//                     if ( nPos < m_nActive )
//                         m_nActive -= 1;
//                     else if ( ( nPos == m_nActive ) &&
//                               ( nPos == (long) m_vEntries.size() ) )
//                         m_nActive -= 1;
//
//                     m_bHasActive = false;
//                     //clear before calling out of this method
//                     aGuard.clear();
//                     selectEntry( m_nActive );
//                 }
//                 break;
//             }
//         }
//     }
}

//------------------------------------------------------------------------------
void ClientBox::RemoveUnlocked()
{
//     bool bAllRemoved = false;
//
//     while ( ! bAllRemoved )
//     {
//         bAllRemoved = true;
//
//         ::osl::ClearableMutexGuard aGuard( m_entriesMutex );
//
//         typedef std::vector< TClientBoxEntry >::iterator ITER;
//
//         for ( ITER iIndex = m_vEntries.begin(); iIndex < m_vEntries.end(); ++iIndex )
//         {
//             if ( !(*iIndex)->m_bLocked )
//             {
//                 bAllRemoved = false;
//                 uno::Reference< deployment::XPackage> xPackage = (*iIndex)->m_xPackage;
//                 aGuard.clear();
//                 removeEntry( xPackage );
//                 break;
//             }
//         }
//     }
}

//------------------------------------------------------------------------------
void ClientBox::prepareChecking()
{
    m_bInCheckMode = true;
    typedef std::vector< TClientBoxEntry >::iterator ITER;
    for ( ITER iIndex = m_vEntries.begin(); iIndex < m_vEntries.end(); ++iIndex )
    {
//         (*iIndex)->m_bChecked = false;
//         (*iIndex)->m_bNew = false;
    }
}

//------------------------------------------------------------------------------
void ClientBox::checkEntries()
{
    long nNewPos = -1;
//     long nPos = 0;
    bool bNeedsUpdate = false;

    ::osl::ClearableMutexGuard guard(m_entriesMutex);
    typedef std::vector< TClientBoxEntry >::iterator ITER;
    ITER iIndex = m_vEntries.begin();
    while ( iIndex < m_vEntries.end() )
    {
//         if ( (*iIndex)->m_bChecked == false )
//         {
//             (*iIndex)->m_bChecked = true;
//             bNeedsUpdate = true;
//             nPos = iIndex-m_vEntries.begin();
//             if ( (*iIndex)->m_bNew )
//             { // add entry to list and correct active pos
//                 if ( nNewPos == - 1)
//                     nNewPos = nPos;
//                 if ( nPos <= m_nActive )
//                     m_nActive += 1;
//                 ++iIndex;
//             }
//             else
//             {   // remove entry from list
//                 if ( nPos < m_nActive )
//                     m_nActive -= 1;
//                 else if ( ( nPos == m_nActive ) && ( nPos == (long) m_vEntries.size() - 1 ) )
//                     m_nActive -= 1;
//                 m_vRemovedEntries.push_back( *iIndex );
//                 m_vEntries.erase( iIndex );
//                 iIndex = m_vEntries.begin() + nPos;
//             }
//         }
//         else
            ++iIndex;
    }
    guard.clear();

    m_bInCheckMode = false;

    if ( nNewPos != - 1)
        selectEntry( nNewPos );

    if ( bNeedsUpdate )
    {
        m_bNeedsRecalc = true;
        if ( IsReallyVisible() )
            Invalidate();
    }
}

//------------------------------------------------------------------------------
void ClientBox::SetScrollHdl( const Link& rLink )
{
    m_pScrollBar->SetScrollHdl( rLink );
}

// -----------------------------------------------------------------------
void ClientBox::DoScroll( long nDelta )
{
    m_nTopIndex += nDelta;
    Point aNewSBPt( m_pScrollBar->GetPosPixel() );

    Rectangle aScrRect( Point(), GetOutputSizePixel() );
    aScrRect.Right() -= m_pScrollBar->GetSizePixel().Width();
    Scroll( 0, -nDelta, aScrRect );

    m_pScrollBar->SetPosPixel( aNewSBPt );
}

// -----------------------------------------------------------------------
IMPL_LINK( ClientBox, ScrollHdl, ScrollBar*, pScrBar )
{
    DoScroll( pScrBar->GetDelta() );

    return 1;
}

} //namespace dp_gui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
