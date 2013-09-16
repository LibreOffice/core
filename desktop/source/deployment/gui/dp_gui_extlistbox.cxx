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

#include "svtools/controldims.hrc"

#include "dp_gui.h"
#include "dp_gui_extlistbox.hxx"
#include "dp_gui_theextmgr.hxx"
#include "dp_gui_dialog2.hxx"
#include "dp_dependencies.hxx"

#include "comphelper/processfactory.hxx"
#include "com/sun/star/i18n/CollatorOptions.hpp"
#include "com/sun/star/deployment/DependencyException.hpp"
#include "com/sun/star/deployment/DeploymentException.hpp"
#include "cppuhelper/weakref.hxx"

#define USER_PACKAGE_MANAGER    "user"
#define SHARED_PACKAGE_MANAGER  "shared"

using namespace ::com::sun::star;

namespace dp_gui {

namespace {

struct FindWeakRef
{
    const uno::Reference<deployment::XPackage> m_extension;

    FindWeakRef( uno::Reference<deployment::XPackage> const & ext): m_extension(ext) {}
    bool operator () (uno::WeakReference< deployment::XPackage >  const & ref);
};

bool FindWeakRef::operator () (uno::WeakReference< deployment::XPackage >  const & ref)
{
    const uno::Reference<deployment::XPackage> ext(ref);
    if (ext == m_extension)
        return true;
    return false;
}

} // end namespace
//------------------------------------------------------------------------------
//                          struct Entry_Impl
//------------------------------------------------------------------------------
Entry_Impl::Entry_Impl( const uno::Reference< deployment::XPackage > &xPackage,
                        const PackageState eState, const bool bReadOnly ) :
    m_bActive( false ),
    m_bLocked( bReadOnly ),
    m_bHasOptions( false ),
    m_bUser( false ),
    m_bShared( false ),
    m_bNew( false ),
    m_bChecked( false ),
    m_bMissingDeps( false ),
    m_bHasButtons( false ),
    m_bMissingLic( false ),
    m_eState( eState ),
    m_pPublisher( NULL ),
    m_xPackage( xPackage )
{
    try
    {
        m_sTitle = xPackage->getDisplayName();
        m_sVersion = xPackage->getVersion();
        m_sDescription = xPackage->getDescription();
    m_sLicenseText = xPackage->getLicenseText();

        beans::StringPair aInfo( m_xPackage->getPublisherInfo() );
        m_sPublisher = aInfo.First;
        m_sPublisherURL = aInfo.Second;

        // get the icons for the package if there are any
        uno::Reference< graphic::XGraphic > xGraphic = xPackage->getIcon( false );
        if ( xGraphic.is() )
            m_aIcon = Image( xGraphic );

        xGraphic = xPackage->getIcon( true );
        if ( xGraphic.is() )
            m_aIconHC = Image( xGraphic );
        else
            m_aIconHC = m_aIcon;

        if ( eState == AMBIGUOUS )
            m_sErrorText = DialogHelper::getResourceString( RID_STR_ERROR_UNKNOWN_STATUS );
        else if ( eState == NOT_REGISTERED )
            checkDependencies();
    }
    catch (const deployment::ExtensionRemovedException &) {}
    catch (const uno::RuntimeException &) {}
}

//------------------------------------------------------------------------------
Entry_Impl::~Entry_Impl()
{}

//------------------------------------------------------------------------------
sal_Int32 Entry_Impl::CompareTo( const CollatorWrapper *pCollator, const TEntry_Impl pEntry ) const
{
    sal_Int32 eCompare = pCollator->compareString( m_sTitle, pEntry->m_sTitle );
    if ( eCompare == 0 )
    {
        eCompare = m_sVersion.compareTo( pEntry->m_sVersion );
        if ( eCompare == 0 )
        {
            sal_Int32 nCompare = m_xPackage->getRepositoryName().compareTo( pEntry->m_xPackage->getRepositoryName() );
            if ( nCompare < 0 )
                eCompare = -1;
            else if ( nCompare > 0 )
                eCompare = 1;
        }
    }
    return eCompare;
}

//------------------------------------------------------------------------------
void Entry_Impl::checkDependencies()
{
    try {
        m_xPackage->checkDependencies( uno::Reference< ucb::XCommandEnvironment >() );
    }
    catch ( const deployment::DeploymentException &e )
    {
        deployment::DependencyException depExc;
        if ( e.Cause >>= depExc )
        {
            OUString aMissingDep( DialogHelper::getResourceString( RID_STR_ERROR_MISSING_DEPENDENCIES ) );
            for ( sal_Int32 i = 0; i < depExc.UnsatisfiedDependencies.getLength(); ++i )
            {
                aMissingDep += "\n";
                aMissingDep += dp_misc::Dependencies::getErrorText( depExc.UnsatisfiedDependencies[i]);
            }
            aMissingDep += "\n";
            m_sErrorText = aMissingDep;
            m_bMissingDeps = true;
        }
    }
}
//------------------------------------------------------------------------------
// ExtensionRemovedListener
//------------------------------------------------------------------------------
void ExtensionRemovedListener::disposing( lang::EventObject const & rEvt )
    throw ( uno::RuntimeException )
{
    uno::Reference< deployment::XPackage > xPackage( rEvt.Source, uno::UNO_QUERY );

    if ( xPackage.is() )
    {
        m_pParent->removeEntry( xPackage );
    }
}

//------------------------------------------------------------------------------
ExtensionRemovedListener::~ExtensionRemovedListener()
{
}

//------------------------------------------------------------------------------
// ExtensionBox_Impl
//------------------------------------------------------------------------------
ExtensionBox_Impl::ExtensionBox_Impl( Window* pParent, TheExtensionManager *pManager ) :
    IExtensionListBox( pParent, WB_BORDER | WB_TABSTOP | WB_CHILDDLGCTRL ),
    m_bHasScrollBar( false ),
    m_bHasActive( false ),
    m_bNeedsRecalc( true ),
    m_bInCheckMode( false ),
    m_bAdjustActive( false ),
    m_bInDelete( false ),
    m_nActive( 0 ),
    m_nTopIndex( 0 ),
    m_nActiveHeight( 0 ),
    m_nExtraHeight( 2 ),
    m_aSharedImage( DialogHelper::getResId( RID_IMG_SHARED ) ),
    m_aLockedImage( DialogHelper::getResId( RID_IMG_LOCKED ) ),
    m_aWarningImage( DialogHelper::getResId( RID_IMG_WARNING ) ),
    m_aDefaultImage( DialogHelper::getResId( RID_IMG_EXTENSION ) ),
    m_pScrollBar( NULL ),
    m_pManager( pManager )
{
    Init();
}

ExtensionBox_Impl::ExtensionBox_Impl(Window* pParent) :
    IExtensionListBox( pParent, WB_BORDER | WB_TABSTOP | WB_CHILDDLGCTRL ),
    m_bHasScrollBar( false ),
    m_bHasActive( false ),
    m_bNeedsRecalc( true ),
    m_bInCheckMode( false ),
    m_bAdjustActive( false ),
    m_bInDelete( false ),
    m_nActive( 0 ),
    m_nTopIndex( 0 ),
    m_nActiveHeight( 0 ),
    m_nExtraHeight( 2 ),
    m_aSharedImage( DialogHelper::getResId( RID_IMG_SHARED ) ),
    m_aLockedImage( DialogHelper::getResId( RID_IMG_LOCKED ) ),
    m_aWarningImage( DialogHelper::getResId( RID_IMG_WARNING ) ),
    m_aDefaultImage( DialogHelper::getResId( RID_IMG_EXTENSION ) ),
    m_pScrollBar( NULL ),
    m_pManager( NULL )
{
    Init();
}

void ExtensionBox_Impl::Init()
{
    SetHelpId( HID_EXTENSION_MANAGER_LISTBOX );

    m_pScrollBar = new ScrollBar( this, WB_VERT );
    m_pScrollBar->SetScrollHdl( LINK( this, ExtensionBox_Impl, ScrollHdl ) );
    m_pScrollBar->EnableDrag();

    SetPaintTransparent( true );
    SetPosPixel( Point( RSC_SP_DLG_INNERBORDER_LEFT, RSC_SP_DLG_INNERBORDER_TOP ) );
    long nIconHeight = 2*TOP_OFFSET + SMALL_ICON_SIZE;
    long nTitleHeight = 2*TOP_OFFSET + GetTextHeight();
    if ( nIconHeight < nTitleHeight )
        m_nStdHeight = nTitleHeight;
    else
        m_nStdHeight = nIconHeight;
    m_nStdHeight += GetTextHeight() + TOP_OFFSET;

    nIconHeight = ICON_HEIGHT + 2*TOP_OFFSET + 1;
    if ( m_nStdHeight < nIconHeight )
        m_nStdHeight = nIconHeight;

    m_nActiveHeight = m_nStdHeight;

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if( IsControlBackground() )
        SetBackground( GetControlBackground() );
    else
        SetBackground( rStyleSettings.GetFieldColor() );

    m_xRemoveListener = new ExtensionRemovedListener( this );

    m_pLocale = new lang::Locale( Application::GetSettings().GetLanguageTag().getLocale() );
    m_pCollator = new CollatorWrapper( ::comphelper::getProcessComponentContext() );
    m_pCollator->loadDefaultCollator( *m_pLocale, i18n::CollatorOptions::CollatorOptions_IGNORE_CASE );

    Show();
}

//------------------------------------------------------------------------------
ExtensionBox_Impl::~ExtensionBox_Impl()
{
    if ( ! m_bInDelete )
        DeleteRemoved();

    m_bInDelete = true;

    typedef std::vector< TEntry_Impl >::iterator ITER;

    for ( ITER iIndex = m_vEntries.begin(); iIndex < m_vEntries.end(); ++iIndex )
    {
        if ( (*iIndex)->m_pPublisher )
        {
            delete (*iIndex)->m_pPublisher;
            (*iIndex)->m_pPublisher = NULL;
        }
        (*iIndex)->m_xPackage->removeEventListener( uno::Reference< lang::XEventListener > ( m_xRemoveListener, uno::UNO_QUERY ) );
    }

    m_vEntries.clear();

    delete m_pScrollBar;

    m_xRemoveListener.clear();

    delete m_pLocale;
    delete m_pCollator;
}

//------------------------------------------------------------------------------
sal_Int32 ExtensionBox_Impl::getItemCount() const
{
    return static_cast< sal_Int32 >( m_vEntries.size() );
}

//------------------------------------------------------------------------------
sal_Int32 ExtensionBox_Impl::getSelIndex() const
{
    if ( m_bHasActive )
    {
        OSL_ASSERT( m_nActive >= -1);
        return static_cast< sal_Int32 >( m_nActive );
    }
    else
        return ENTRY_NOTFOUND;
}

//------------------------------------------------------------------------------
void ExtensionBox_Impl::checkIndex( sal_Int32 nIndex ) const
{
    if ( nIndex < 0 )
        throw lang::IllegalArgumentException( "The list index starts with 0",0, 0 );
    if ( static_cast< sal_uInt32 >( nIndex ) >= m_vEntries.size())
        throw lang::IllegalArgumentException( "There is no element at the provided position. The position exceeds the number of available list entries",0, 0 );
}

//------------------------------------------------------------------------------
OUString ExtensionBox_Impl::getItemName( sal_Int32 nIndex ) const
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );
    checkIndex( nIndex );
    return m_vEntries[ nIndex ]->m_sTitle;
}

//------------------------------------------------------------------------------
OUString ExtensionBox_Impl::getItemVersion( sal_Int32 nIndex ) const
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );
    checkIndex( nIndex );
    return m_vEntries[ nIndex ]->m_sVersion;
}

//------------------------------------------------------------------------------
OUString ExtensionBox_Impl::getItemDescription( sal_Int32 nIndex ) const
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );
    checkIndex( nIndex );
    return m_vEntries[ nIndex ]->m_sDescription;
}

//------------------------------------------------------------------------------
OUString ExtensionBox_Impl::getItemPublisher( sal_Int32 nIndex ) const
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );
    checkIndex( nIndex );
    return m_vEntries[ nIndex ]->m_sPublisher;
}

//------------------------------------------------------------------------------
OUString ExtensionBox_Impl::getItemPublisherLink( sal_Int32 nIndex ) const
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );
    checkIndex( nIndex );
    return m_vEntries[ nIndex ]->m_sPublisherURL;
}

//------------------------------------------------------------------------------
void ExtensionBox_Impl::select( sal_Int32 nIndex )
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );
    checkIndex( nIndex );
    selectEntry( nIndex );
}

//------------------------------------------------------------------------------
void ExtensionBox_Impl::select( const OUString & sName )
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );
    typedef ::std::vector< TEntry_Impl >::const_iterator It;

    for ( It iIter = m_vEntries.begin(); iIter != m_vEntries.end(); ++iIter )
    {
        if ( sName.equals( (*iIter)->m_sTitle ))
        {
            long nPos = iIter - m_vEntries.begin();
            selectEntry( nPos );
            break;
        }
    }
}

//------------------------------------------------------------------------------
// Title + description
void ExtensionBox_Impl::CalcActiveHeight( const long nPos )
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );

    // get title height
    long aTextHeight;
    long nIconHeight = 2*TOP_OFFSET + SMALL_ICON_SIZE;
    long nTitleHeight = 2*TOP_OFFSET + GetTextHeight();
    if ( nIconHeight < nTitleHeight )
        aTextHeight = nTitleHeight;
    else
        aTextHeight = nIconHeight;

    // calc description height
    Size aSize = GetOutputSizePixel();
    if ( m_bHasScrollBar )
        aSize.Width() -= m_pScrollBar->GetSizePixel().Width();

    aSize.Width() -= ICON_OFFSET;
    aSize.Height() = 10000;

    OUString aText( m_vEntries[ nPos ]->m_sErrorText );
    if ( !aText.isEmpty() )
        aText += "\n";
    aText += m_vEntries[ nPos ]->m_sDescription;

    Rectangle aRect = GetTextRect( Rectangle( Point(), aSize ), aText,
                                   TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
    aTextHeight += aRect.GetHeight();

    if ( aTextHeight < m_nStdHeight )
        aTextHeight = m_nStdHeight;

    if ( m_vEntries[ nPos ]->m_bHasButtons )
        m_nActiveHeight = aTextHeight + m_nExtraHeight;
    else
        m_nActiveHeight = aTextHeight + 2;
}

//------------------------------------------------------------------------------
const Size ExtensionBox_Impl::GetMinOutputSizePixel() const
{
    return Size( 200, 80 );
}

//------------------------------------------------------------------------------
Rectangle ExtensionBox_Impl::GetEntryRect( const long nPos ) const
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
void ExtensionBox_Impl::DeleteRemoved()
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );

    m_bInDelete = true;

    if ( ! m_vRemovedEntries.empty() )
    {
        typedef std::vector< TEntry_Impl >::iterator ITER;

        for ( ITER iIndex = m_vRemovedEntries.begin(); iIndex < m_vRemovedEntries.end(); ++iIndex )
        {
            if ( (*iIndex)->m_pPublisher )
            {
                delete (*iIndex)->m_pPublisher;
                (*iIndex)->m_pPublisher = NULL;
            }
        }

        m_vRemovedEntries.clear();
    }

    m_bInDelete = false;
}

//------------------------------------------------------------------------------
//This function may be called with nPos < 0
void ExtensionBox_Impl::selectEntry( const long nPos )
{
    //ToDo whe should not use the guard at such a big scope here.
    //Currently it is used to gard m_vEntries and m_nActive. m_nActive will be
    //modified in this function.
    //It would be probably best to always use a copy of m_vEntries
    //and some other state variables from ExtensionBox_Impl for
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
void ExtensionBox_Impl::DrawRow( const Rectangle& rRect, const TEntry_Impl pEntry )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( pEntry->m_bActive )
        SetTextColor( rStyleSettings.GetHighlightTextColor() );
    else if ( ( pEntry->m_eState != REGISTERED ) && ( pEntry->m_eState != NOT_AVAILABLE ) )
        SetTextColor( rStyleSettings.GetDisableColor() );
    else if ( IsControlForeground() )
        SetTextColor( GetControlForeground() );
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

    // Draw extension icon
    Point aPos( rRect.TopLeft() );
    aPos += Point( TOP_OFFSET, TOP_OFFSET );
    Image aImage;
    if ( ! pEntry->m_aIcon )
        aImage = m_aDefaultImage;
    else
        aImage = pEntry->m_aIcon;
    Size aImageSize = aImage.GetSizePixel();
    if ( ( aImageSize.Width() <= ICON_WIDTH ) && ( aImageSize.Height() <= ICON_HEIGHT ) )
        DrawImage( Point( aPos.X()+((ICON_WIDTH-aImageSize.Width())/2), aPos.Y()+((ICON_HEIGHT-aImageSize.Height())/2) ), aImage );
    else
        DrawImage( aPos, Size( ICON_WIDTH, ICON_HEIGHT ), aImage );

    // Setup fonts
    Font aStdFont( GetFont() );
    Font aBoldFont( aStdFont );
    aBoldFont.SetWeight( WEIGHT_BOLD );
    SetFont( aBoldFont );
    long aTextHeight = GetTextHeight();

    // Init publisher link here
    if ( !pEntry->m_pPublisher && !pEntry->m_sPublisher.isEmpty() )
    {
        pEntry->m_pPublisher = new FixedHyperlink( this );
        pEntry->m_pPublisher->SetBackground();
        pEntry->m_pPublisher->SetPaintTransparent( true );
        pEntry->m_pPublisher->SetURL( pEntry->m_sPublisherURL );
        pEntry->m_pPublisher->SetText( pEntry->m_sPublisher );
        Size aSize = FixedText::CalcMinimumTextSize( pEntry->m_pPublisher );
        pEntry->m_pPublisher->SetSizePixel( aSize );

        if ( m_aClickHdl.IsSet() )
            pEntry->m_pPublisher->SetClickHdl( m_aClickHdl );
    }

    // Get max title width
    long nMaxTitleWidth = rRect.GetWidth() - ICON_OFFSET;
    nMaxTitleWidth -= ( 2 * SMALL_ICON_SIZE ) + ( 4 * SPACE_BETWEEN );
    if ( pEntry->m_pPublisher )
    {
        nMaxTitleWidth -= pEntry->m_pPublisher->GetSizePixel().Width() + (2*SPACE_BETWEEN);
    }

    long aVersionWidth = GetTextWidth( pEntry->m_sVersion );
    long aTitleWidth = GetTextWidth( pEntry->m_sTitle ) + (aTextHeight / 3);

    aPos = rRect.TopLeft() + Point( ICON_OFFSET, TOP_OFFSET );

    if ( aTitleWidth > nMaxTitleWidth - aVersionWidth )
    {
        aTitleWidth = nMaxTitleWidth - aVersionWidth - (aTextHeight / 3);
        OUString aShortTitle = GetEllipsisString( pEntry->m_sTitle, aTitleWidth );
        DrawText( aPos, aShortTitle );
        aTitleWidth += (aTextHeight / 3);
    }
    else
        DrawText( aPos, pEntry->m_sTitle );

    SetFont( aStdFont );
    DrawText( Point( aPos.X() + aTitleWidth, aPos.Y() ), pEntry->m_sVersion );

    long nIconHeight = TOP_OFFSET + SMALL_ICON_SIZE;
    long nTitleHeight = TOP_OFFSET + GetTextHeight();
    if ( nIconHeight < nTitleHeight )
        aTextHeight = nTitleHeight;
    else
        aTextHeight = nIconHeight;

    // draw description
    OUString sDescription;
    if ( !pEntry->m_sErrorText.isEmpty() )
    {
        if ( pEntry->m_bActive )
            sDescription = pEntry->m_sErrorText + OUString("\n") + pEntry->m_sDescription;
        else
            sDescription = pEntry->m_sErrorText;
    }
    else
        sDescription = pEntry->m_sDescription;

    aPos.Y() += aTextHeight;
    if ( pEntry->m_bActive )
    {
        long nExtraHeight = 0;

        if ( pEntry->m_bHasButtons )
            nExtraHeight = m_nExtraHeight;

        DrawText( Rectangle( aPos.X(), aPos.Y(), rRect.Right(), rRect.Bottom() - nExtraHeight ),
                  sDescription, TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
    }
    else
    {
        //replace LF to space, so words do not stick together in one line view
        sDescription = sDescription.replace(0x000A, ' ');
        const long nWidth = GetTextWidth( sDescription );
        if ( nWidth > rRect.GetWidth() - aPos.X() )
            sDescription = GetEllipsisString( sDescription, rRect.GetWidth() - aPos.X() );
        DrawText( aPos, sDescription );
    }

    // Draw publisher link
    if ( pEntry->m_pPublisher )
    {
        pEntry->m_pPublisher->Show();
        aPos = rRect.TopLeft() + Point( ICON_OFFSET + nMaxTitleWidth + (2*SPACE_BETWEEN), TOP_OFFSET );
        pEntry->m_pPublisher->SetPosPixel( aPos );
    }

    // Draw status icons
    if ( !pEntry->m_bUser )
    {
        aPos = rRect.TopRight() + Point( -(RIGHT_ICON_OFFSET + SMALL_ICON_SIZE), TOP_OFFSET );
        if ( pEntry->m_bLocked )
            DrawImage( aPos, Size( SMALL_ICON_SIZE, SMALL_ICON_SIZE ), m_aLockedImage );
        else
            DrawImage( aPos, Size( SMALL_ICON_SIZE, SMALL_ICON_SIZE ), m_aSharedImage );
    }
    if ( ( pEntry->m_eState == AMBIGUOUS ) || pEntry->m_bMissingDeps || pEntry->m_bMissingLic )
    {
        aPos = rRect.TopRight() + Point( -(RIGHT_ICON_OFFSET + SPACE_BETWEEN + 2*SMALL_ICON_SIZE), TOP_OFFSET );
        DrawImage( aPos, Size( SMALL_ICON_SIZE, SMALL_ICON_SIZE ), m_aWarningImage );
    }

    SetLineColor( Color( COL_LIGHTGRAY ) );
    DrawLine( rRect.BottomLeft(), rRect.BottomRight() );
}

// -----------------------------------------------------------------------
void ExtensionBox_Impl::RecalcAll()
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
bool ExtensionBox_Impl::HandleTabKey( bool )
{
    return false;
}

// -----------------------------------------------------------------------
bool ExtensionBox_Impl::HandleCursorKey( sal_uInt16 nKeyCode )
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
void ExtensionBox_Impl::Paint( const Rectangle &/*rPaintRect*/ )
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

    typedef std::vector< TEntry_Impl >::iterator ITER;
    for ( ITER iIndex = m_vEntries.begin(); iIndex < m_vEntries.end(); ++iIndex )
    {
        aSize.Height() = (*iIndex)->m_bActive ? m_nActiveHeight : m_nStdHeight;
        Rectangle aEntryRect( aStart, aSize );
        DrawRow( aEntryRect, *iIndex );
        aStart.Y() += aSize.Height();
    }
}

// -----------------------------------------------------------------------
long ExtensionBox_Impl::GetTotalHeight() const
{
    long nHeight = m_vEntries.size() * m_nStdHeight;

    if ( m_bHasActive )
    {
        nHeight += m_nActiveHeight - m_nStdHeight;
    }

    return nHeight;
}

// -----------------------------------------------------------------------
void ExtensionBox_Impl::SetupScrollBar()
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
void ExtensionBox_Impl::Resize()
{
    RecalcAll();
}

//------------------------------------------------------------------------------
long ExtensionBox_Impl::PointToPos( const Point& rPos )
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

//------------------------------------------------------------------------------
void ExtensionBox_Impl::MouseButtonDown( const MouseEvent& rMEvt )
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
long ExtensionBox_Impl::Notify( NotifyEvent& rNEvt )
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
bool ExtensionBox_Impl::FindEntryPos( const TEntry_Impl pEntry, const long nStart,
                                      const long nEnd, long &nPos )
{
    nPos = nStart;
    if ( nStart > nEnd )
        return false;

    sal_Int32 eCompare;

    if ( nStart == nEnd )
    {
        eCompare = pEntry->CompareTo( m_pCollator, m_vEntries[ nStart ] );
        if ( eCompare < 0 )
            return false;
        else if ( eCompare == 0 )
        {
            //Workaround. See i86963.
            if (pEntry->m_xPackage != m_vEntries[nStart]->m_xPackage)
                return false;

            if ( m_bInCheckMode )
                m_vEntries[ nStart ]->m_bChecked = true;
            return true;
        }
        else
        {
            nPos = nStart + 1;
            return false;
        }
    }

    const long nMid = nStart + ( ( nEnd - nStart ) / 2 );
    eCompare = pEntry->CompareTo( m_pCollator, m_vEntries[ nMid ] );

    if ( eCompare < 0 )
        return FindEntryPos( pEntry, nStart, nMid-1, nPos );
    else if ( eCompare > 0 )
        return FindEntryPos( pEntry, nMid+1, nEnd, nPos );
    else
    {
        //Workaround.See i86963.
        if (pEntry->m_xPackage != m_vEntries[nMid]->m_xPackage)
            return false;

        if ( m_bInCheckMode )
            m_vEntries[ nMid ]->m_bChecked = true;
        nPos = nMid;
        return true;
    }
}

void ExtensionBox_Impl::cleanVecListenerAdded()
{
    typedef ::std::vector<uno::WeakReference<deployment::XPackage> >::iterator IT;
    IT i = m_vListenerAdded.begin();
    while( i != m_vListenerAdded.end())
    {
        const uno::Reference<deployment::XPackage> hardRef(*i);
        if (!hardRef.is())
            i = m_vListenerAdded.erase(i);
        else
            ++i;
    }
}

void ExtensionBox_Impl::addEventListenerOnce(
    uno::Reference<deployment::XPackage > const & extension)
{
    //make sure to only add the listener once
    cleanVecListenerAdded();
    if ( ::std::find_if(m_vListenerAdded.begin(), m_vListenerAdded.end(),
                        FindWeakRef(extension))
         == m_vListenerAdded.end())
    {
        extension->addEventListener( uno::Reference< lang::XEventListener > (
                                         m_xRemoveListener, uno::UNO_QUERY ) );
        m_vListenerAdded.push_back(extension);
    }
}

//------------------------------------------------------------------------------
long ExtensionBox_Impl::addEntry( const uno::Reference< deployment::XPackage > &xPackage,
                                  bool bLicenseMissing )
{
    long         nPos = 0;
    PackageState eState = m_pManager->getPackageState( xPackage );
    bool         bLocked = m_pManager->isReadOnly( xPackage );

    TEntry_Impl pEntry( new Entry_Impl( xPackage, eState, bLocked ) );

    // Don't add empty entries
    if ( pEntry->m_sTitle.isEmpty() )
        return 0;

    ::osl::ClearableMutexGuard guard(m_entriesMutex);
    if ( m_vEntries.empty() )
    {
        addEventListenerOnce(xPackage);
        m_vEntries.push_back( pEntry );
    }
    else
    {
        if ( !FindEntryPos( pEntry, 0, m_vEntries.size()-1, nPos ) )
        {
            addEventListenerOnce(xPackage);
            m_vEntries.insert( m_vEntries.begin()+nPos, pEntry );
        }
        else if ( !m_bInCheckMode )
        {
            OSL_FAIL( "ExtensionBox_Impl::addEntry(): Will not add duplicate entries"  );
        }
    }

    pEntry->m_bHasOptions = m_pManager->supportsOptions( xPackage );
    pEntry->m_bUser       = (xPackage->getRepositoryName() == USER_PACKAGE_MANAGER);
    pEntry->m_bShared     = (xPackage->getRepositoryName() == SHARED_PACKAGE_MANAGER);
    pEntry->m_bNew        = m_bInCheckMode;
    pEntry->m_bMissingLic = bLicenseMissing;

    if ( bLicenseMissing )
        pEntry->m_sErrorText = DialogHelper::getResourceString( RID_STR_ERROR_MISSING_LICENSE );

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
void ExtensionBox_Impl::updateEntry( const uno::Reference< deployment::XPackage > &xPackage )
{
    typedef std::vector< TEntry_Impl >::iterator ITER;
    for ( ITER iIndex = m_vEntries.begin(); iIndex < m_vEntries.end(); ++iIndex )
    {
        if ( (*iIndex)->m_xPackage == xPackage )
        {
            PackageState eState = m_pManager->getPackageState( xPackage );
            (*iIndex)->m_bHasOptions = m_pManager->supportsOptions( xPackage );
            (*iIndex)->m_eState = eState;
            (*iIndex)->m_sTitle = xPackage->getDisplayName();
            (*iIndex)->m_sVersion = xPackage->getVersion();
            (*iIndex)->m_sDescription = xPackage->getDescription();

            if ( eState == REGISTERED )
                (*iIndex)->m_bMissingLic = false;

            if ( eState == AMBIGUOUS )
                (*iIndex)->m_sErrorText = DialogHelper::getResourceString( RID_STR_ERROR_UNKNOWN_STATUS );
            else if ( ! (*iIndex)->m_bMissingLic )
                (*iIndex)->m_sErrorText = "";

            if ( IsReallyVisible() )
                Invalidate();
            break;
        }
    }
}

//------------------------------------------------------------------------------
//This function is also called as a result of removing an extension.
//see PackageManagerImpl::removePackage
//The gui is a registered as listener on the package. Removing it will cause the
//listeners to be notified an then this function is called. At this moment xPackage
//is in the disposing state and all calls on it may result in a DisposedException.
void ExtensionBox_Impl::removeEntry( const uno::Reference< deployment::XPackage > &xPackage )
{
   if ( ! m_bInDelete )
    {
        ::osl::ClearableMutexGuard aGuard( m_entriesMutex );

        typedef std::vector< TEntry_Impl >::iterator ITER;

        for ( ITER iIndex = m_vEntries.begin(); iIndex < m_vEntries.end(); ++iIndex )
        {
            if ( (*iIndex)->m_xPackage == xPackage )
            {
                long nPos = iIndex - m_vEntries.begin();

                // Entries mustn't removed here, because they contain a hyperlink control
                // which can only be deleted when the thread has the solar mutex. Therefor
                // the entry will be moved into the m_vRemovedEntries list which will be
                // cleared on the next paint event
                m_vRemovedEntries.push_back( *iIndex );
                (*iIndex)->m_xPackage->removeEventListener(
                    uno::Reference<lang::XEventListener>(m_xRemoveListener, uno::UNO_QUERY));
                m_vEntries.erase( iIndex );

                m_bNeedsRecalc = true;

                if ( IsReallyVisible() )
                    Invalidate();

                if ( m_bHasActive )
                {
                    if ( nPos < m_nActive )
                        m_nActive -= 1;
                    else if ( ( nPos == m_nActive ) &&
                              ( nPos == (long) m_vEntries.size() ) )
                        m_nActive -= 1;

                    m_bHasActive = false;
                    //clear before calling out of this method
                    aGuard.clear();
                    selectEntry( m_nActive );
                }
                break;
            }
        }
    }
}

//------------------------------------------------------------------------------
void ExtensionBox_Impl::RemoveUnlocked()
{
    bool bAllRemoved = false;

    while ( ! bAllRemoved )
    {
        bAllRemoved = true;

        ::osl::ClearableMutexGuard aGuard( m_entriesMutex );

        typedef std::vector< TEntry_Impl >::iterator ITER;

        for ( ITER iIndex = m_vEntries.begin(); iIndex < m_vEntries.end(); ++iIndex )
        {
            if ( !(*iIndex)->m_bLocked )
            {
                bAllRemoved = false;
                uno::Reference< deployment::XPackage> xPackage = (*iIndex)->m_xPackage;
                aGuard.clear();
                removeEntry( xPackage );
                break;
            }
        }
    }
}

//------------------------------------------------------------------------------
void ExtensionBox_Impl::prepareChecking()
{
    m_bInCheckMode = true;
    typedef std::vector< TEntry_Impl >::iterator ITER;
    for ( ITER iIndex = m_vEntries.begin(); iIndex < m_vEntries.end(); ++iIndex )
    {
        (*iIndex)->m_bChecked = false;
        (*iIndex)->m_bNew = false;
    }
}

//------------------------------------------------------------------------------
void ExtensionBox_Impl::checkEntries()
{
    long nNewPos = -1;
    long nPos = 0;
    bool bNeedsUpdate = false;

    ::osl::ClearableMutexGuard guard(m_entriesMutex);
    typedef std::vector< TEntry_Impl >::iterator ITER;
    ITER iIndex = m_vEntries.begin();
    while ( iIndex < m_vEntries.end() )
    {
        if ( (*iIndex)->m_bChecked == false )
        {
            (*iIndex)->m_bChecked = true;
            bNeedsUpdate = true;
            nPos = iIndex-m_vEntries.begin();
            if ( (*iIndex)->m_bNew )
            { // add entry to list and correct active pos
                if ( nNewPos == - 1)
                    nNewPos = nPos;
                if ( nPos <= m_nActive )
                    m_nActive += 1;
                ++iIndex;
            }
            else
            {   // remove entry from list
                if ( nPos < m_nActive )
                    m_nActive -= 1;
                else if ( ( nPos == m_nActive ) && ( nPos == (long) m_vEntries.size() - 1 ) )
                    m_nActive -= 1;
                m_vRemovedEntries.push_back( *iIndex );
                m_vEntries.erase( iIndex );
                iIndex = m_vEntries.begin() + nPos;
            }
        }
        else
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
void ExtensionBox_Impl::SetScrollHdl( const Link& rLink )
{
    if ( m_pScrollBar )
        m_pScrollBar->SetScrollHdl( rLink );
}

// -----------------------------------------------------------------------
void ExtensionBox_Impl::DoScroll( long nDelta )
{
    m_nTopIndex += nDelta;
    Point aNewSBPt( m_pScrollBar->GetPosPixel() );

    Rectangle aScrRect( Point(), GetOutputSizePixel() );
    aScrRect.Right() -= m_pScrollBar->GetSizePixel().Width();
    Scroll( 0, -nDelta, aScrRect );

    m_pScrollBar->SetPosPixel( aNewSBPt );
}

// -----------------------------------------------------------------------
IMPL_LINK( ExtensionBox_Impl, ScrollHdl, ScrollBar*, pScrBar )
{
    DoScroll( pScrBar->GetDelta() );

    return 1;
}

} //namespace dp_gui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
