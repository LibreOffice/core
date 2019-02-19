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

#include <svtools/controldims.hxx>

#include <dp_shared.hxx>
#include <strings.hrc>
#include "dp_gui.h"
#include "dp_gui_extlistbox.hxx"
#include "dp_gui_theextmgr.hxx"
#include "dp_gui_dialog2.hxx"
#include <dp_dependencies.hxx>
#include <bitmaps.hlst>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <com/sun/star/deployment/DependencyException.hpp>
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/deployment/ExtensionRemovedException.hpp>
#include <cppuhelper/weakref.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/commandevent.hxx>
#include <algorithm>

#define USER_PACKAGE_MANAGER    "user"
#define SHARED_PACKAGE_MANAGER  "shared"

using namespace ::com::sun::star;

namespace dp_gui {

namespace {

struct FindWeakRef
{
    const uno::Reference<deployment::XPackage> m_extension;

    explicit FindWeakRef( uno::Reference<deployment::XPackage> const & ext): m_extension(ext) {}
    bool operator () (uno::WeakReference< deployment::XPackage >  const & ref);
};

bool FindWeakRef::operator () (uno::WeakReference< deployment::XPackage >  const & ref)
{
    const uno::Reference<deployment::XPackage> ext(ref);
    return ext == m_extension;
}

} // end namespace

//                          struct Entry_Impl

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
    m_pPublisher( nullptr ),
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

        if ( eState == AMBIGUOUS )
            m_sErrorText = DpResId( RID_STR_ERROR_UNKNOWN_STATUS );
        else if ( eState == NOT_REGISTERED )
            checkDependencies();
    }
    catch (const deployment::ExtensionRemovedException &) {}
    catch (const uno::RuntimeException &) {}
}


Entry_Impl::~Entry_Impl()
{}


sal_Int32 Entry_Impl::CompareTo( const CollatorWrapper *pCollator, const TEntry_Impl& rEntry ) const
{
    sal_Int32 eCompare = pCollator->compareString( m_sTitle, rEntry->m_sTitle );
    if ( eCompare == 0 )
    {
        eCompare = m_sVersion.compareTo( rEntry->m_sVersion );
        if ( eCompare == 0 )
        {
            sal_Int32 nCompare = m_xPackage->getRepositoryName().compareTo( rEntry->m_xPackage->getRepositoryName() );
            if ( nCompare < 0 )
                eCompare = -1;
            else if ( nCompare > 0 )
                eCompare = 1;
        }
    }
    return eCompare;
}


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
            OUStringBuffer aMissingDep( DpResId( RID_STR_ERROR_MISSING_DEPENDENCIES ) );
            for ( sal_Int32 i = 0; i < depExc.UnsatisfiedDependencies.getLength(); ++i )
            {
                aMissingDep.append("\n");
                aMissingDep.append(dp_misc::Dependencies::getErrorText( depExc.UnsatisfiedDependencies[i]));
            }
            aMissingDep.append("\n");
            m_sErrorText = aMissingDep.makeStringAndClear();
            m_bMissingDeps = true;
        }
    }
}

// ExtensionRemovedListener

void ExtensionRemovedListener::disposing( lang::EventObject const & rEvt )
{
    uno::Reference< deployment::XPackage > xPackage( rEvt.Source, uno::UNO_QUERY );

    if ( xPackage.is() )
    {
        m_pParent->removeEntry( xPackage );
    }
}


ExtensionRemovedListener::~ExtensionRemovedListener()
{
}


// ExtensionBox_Impl
ExtensionBox_Impl::ExtensionBox_Impl(vcl::Window* pParent) :
    IExtensionListBox( pParent ),
    m_bHasScrollBar( false ),
    m_bHasActive( false ),
    m_bNeedsRecalc( true ),
    m_bInCheckMode( false ),
    m_bAdjustActive( false ),
    m_bInDelete( false ),
    m_nActive( 0 ),
    m_nTopIndex( 0 ),
    m_nActiveHeight( 0 ),
    m_aSharedImage(StockImage::Yes, RID_BMP_SHARED),
    m_aLockedImage(StockImage::Yes, RID_BMP_LOCKED),
    m_aWarningImage(StockImage::Yes, RID_BMP_WARNING),
    m_aDefaultImage(StockImage::Yes, RID_BMP_EXTENSION),
    m_pScrollBar( nullptr ),
    m_pManager( nullptr )
{
    Init();
}

void ExtensionBox_Impl::Init()
{
    m_pScrollBar = VclPtr<ScrollBar>::Create( this, WB_VERT );
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

    m_pLocale.reset( new lang::Locale( Application::GetSettings().GetLanguageTag().getLocale() ) );
    m_pCollator.reset( new CollatorWrapper( ::comphelper::getProcessComponentContext() ) );
    m_pCollator->loadDefaultCollator( *m_pLocale, i18n::CollatorOptions::CollatorOptions_IGNORE_CASE );

    Show();
}


ExtensionBox_Impl::~ExtensionBox_Impl()
{
    disposeOnce();
}

void ExtensionBox_Impl::dispose()
{
    if ( ! m_bInDelete )
        DeleteRemoved();

    m_bInDelete = true;

    for (auto const& entry : m_vEntries)
    {
        entry->m_pPublisher.disposeAndClear();
        entry->m_xPackage->removeEventListener( m_xRemoveListener.get() );
    }

    m_vEntries.clear();

    m_pScrollBar.disposeAndClear();

    m_xRemoveListener.clear();

    m_pLocale.reset();
    m_pCollator.reset();
    ::svt::IExtensionListBox::dispose();
}


sal_Int32 ExtensionBox_Impl::getItemCount() const
{
    return static_cast< sal_Int32 >( m_vEntries.size() );
}


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
        aSize.AdjustWidth( -(m_pScrollBar->GetSizePixel().Width()) );

    aSize.AdjustWidth( -(ICON_OFFSET) );
    aSize.setHeight( 10000 );

    OUString aText( m_vEntries[ nPos ]->m_sErrorText );
    if ( !aText.isEmpty() )
        aText += "\n";
    aText += m_vEntries[ nPos ]->m_sDescription;

    tools::Rectangle aRect = GetTextRect( tools::Rectangle( Point(), aSize ), aText,
                                   DrawTextFlags::MultiLine | DrawTextFlags::WordBreak );
    aTextHeight += aRect.GetHeight();

    if ( aTextHeight < m_nStdHeight )
        aTextHeight = m_nStdHeight;

    m_nActiveHeight = aTextHeight;

    if ( m_vEntries[ nPos ]->m_bHasButtons )
        m_nActiveHeight += 2;
}

tools::Rectangle ExtensionBox_Impl::GetEntryRect( const long nPos ) const
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );

    Size aSize( GetOutputSizePixel() );

    if ( m_bHasScrollBar )
        aSize.AdjustWidth( -(m_pScrollBar->GetSizePixel().Width()) );

    if ( m_vEntries[ nPos ]->m_bActive )
        aSize.setHeight( m_nActiveHeight );
    else
        aSize.setHeight( m_nStdHeight );

    Point aPos( 0, -m_nTopIndex + nPos * m_nStdHeight );
    if ( m_bHasActive && ( nPos < m_nActive ) )
        aPos.AdjustY(m_nActiveHeight - m_nStdHeight );

    return tools::Rectangle( aPos, aSize );
}


void ExtensionBox_Impl::DeleteRemoved()
{
    const ::osl::MutexGuard aGuard( m_entriesMutex );

    m_bInDelete = true;

    if ( ! m_vRemovedEntries.empty() )
    {
        for (auto const& removedEntry : m_vRemovedEntries)
        {
            removedEntry->m_pPublisher.disposeAndClear();
        }

        m_vRemovedEntries.clear();
    }

    m_bInDelete = false;
}


//This function may be called with nPos < 0
void ExtensionBox_Impl::selectEntry( const long nPos )
{
    bool invalidate = false;
    {
        //ToDo we should not use the guard at such a big scope here.
        //Currently it is used to guard m_vEntries and m_nActive. m_nActive will be
        //modified in this function.
        //It would be probably best to always use a copy of m_vEntries
        //and some other state variables from ExtensionBox_Impl for
        //the whole painting operation. See issue i86993
        ::osl::MutexGuard guard(m_entriesMutex);

        if ( m_bInCheckMode )
            return;

        if ( m_bHasActive )
        {
            if ( nPos == m_nActive )
                return;

            m_bHasActive = false;
            m_vEntries[ m_nActive ]->m_bActive = false;
        }

        if ( ( nPos >= 0 ) && ( nPos < static_cast<long>(m_vEntries.size()) ) )
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
            invalidate = true;
        }
    }

    if (invalidate)
    {
        SolarMutexGuard g;
        Invalidate();
    }
}


void ExtensionBox_Impl::DrawRow(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect, const TEntry_Impl& rEntry)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    if (rEntry->m_bActive)
        rRenderContext.SetTextColor(rStyleSettings.GetHighlightTextColor());
    else if ((rEntry->m_eState != REGISTERED) && (rEntry->m_eState != NOT_AVAILABLE))
        rRenderContext.SetTextColor(rStyleSettings.GetDisableColor());
    else if (IsControlForeground())
        rRenderContext.SetTextColor(GetControlForeground());
    else
        rRenderContext.SetTextColor(rStyleSettings.GetFieldTextColor());

    if (rEntry->m_bActive)
    {
        rRenderContext.SetLineColor();
        rRenderContext.SetFillColor(rStyleSettings.GetHighlightColor());
        rRenderContext.DrawRect(rRect);
    }
    else
    {
        if (IsControlBackground())
            rRenderContext.SetBackground(GetControlBackground());
        else
            rRenderContext.SetBackground(rStyleSettings.GetFieldColor());

        rRenderContext.SetTextFillColor();
        rRenderContext.Erase(rRect);
    }

    // Draw extension icon
    Point aPos( rRect.TopLeft() );
    aPos += Point(TOP_OFFSET, TOP_OFFSET);
    Image aImage;
    if (!rEntry->m_aIcon)
        aImage = m_aDefaultImage;
    else
        aImage = rEntry->m_aIcon;
    Size aImageSize = aImage.GetSizePixel();
    if ((aImageSize.Width() <= ICON_WIDTH ) && ( aImageSize.Height() <= ICON_HEIGHT ) )
        rRenderContext.DrawImage(Point(aPos.X() + ((ICON_WIDTH - aImageSize.Width()) / 2),
                                       aPos.Y() + ((ICON_HEIGHT - aImageSize.Height()) / 2)),
                                 aImage);
    else
        rRenderContext.DrawImage(aPos, Size(ICON_WIDTH, ICON_HEIGHT), aImage);

    // Setup fonts
    vcl::Font aStdFont(rRenderContext.GetFont());
    vcl::Font aBoldFont(aStdFont);
    aBoldFont.SetWeight(WEIGHT_BOLD);
    rRenderContext.SetFont(aBoldFont);
    long aTextHeight = rRenderContext.GetTextHeight();

    // Init publisher link here
    if (!rEntry->m_pPublisher && !rEntry->m_sPublisher.isEmpty())
    {
        rEntry->m_pPublisher = VclPtr<FixedHyperlink>::Create(this);
        rEntry->m_pPublisher->SetBackground();
        rEntry->m_pPublisher->SetPaintTransparent(true);
        rEntry->m_pPublisher->SetURL(rEntry->m_sPublisherURL);
        rEntry->m_pPublisher->SetText(rEntry->m_sPublisher);
        Size aSize = FixedText::CalcMinimumTextSize(rEntry->m_pPublisher);
        rEntry->m_pPublisher->SetSizePixel(aSize);
    }

    // Get max title width
    long nMaxTitleWidth = rRect.GetWidth() - ICON_OFFSET;
    nMaxTitleWidth -= (2 * SMALL_ICON_SIZE) + (4 * SPACE_BETWEEN);
    if (rEntry->m_pPublisher)
    {
        nMaxTitleWidth -= rEntry->m_pPublisher->GetSizePixel().Width() + (2 * SPACE_BETWEEN);
    }

    long aVersionWidth = rRenderContext.GetTextWidth(rEntry->m_sVersion);
    long aTitleWidth = rRenderContext.GetTextWidth(rEntry->m_sTitle) + (aTextHeight / 3);

    aPos = rRect.TopLeft() + Point(ICON_OFFSET, TOP_OFFSET);

    if (aTitleWidth > nMaxTitleWidth - aVersionWidth)
    {
        aTitleWidth = nMaxTitleWidth - aVersionWidth - (aTextHeight / 3);
        OUString aShortTitle = rRenderContext.GetEllipsisString(rEntry->m_sTitle, aTitleWidth);
        rRenderContext.DrawText(aPos, aShortTitle);
        aTitleWidth += (aTextHeight / 3);
    }
    else
        rRenderContext.DrawText(aPos, rEntry->m_sTitle);

    rRenderContext.SetFont(aStdFont);
    rRenderContext.DrawText(Point(aPos.X() + aTitleWidth, aPos.Y()), rEntry->m_sVersion);

    long nIconHeight = TOP_OFFSET + SMALL_ICON_SIZE;
    long nTitleHeight = TOP_OFFSET + GetTextHeight();
    if ( nIconHeight < nTitleHeight )
        aTextHeight = nTitleHeight;
    else
        aTextHeight = nIconHeight;

    // draw description
    OUString sDescription;
    if (!rEntry->m_sErrorText.isEmpty())
    {
        if (rEntry->m_bActive)
            sDescription = rEntry->m_sErrorText + "\n" + rEntry->m_sDescription;
        else
            sDescription = rEntry->m_sErrorText;
    }
    else
        sDescription = rEntry->m_sDescription;

    aPos.AdjustY(aTextHeight );
    if (rEntry->m_bActive)
    {
        long nExtraHeight = 0;

        if (rEntry->m_bHasButtons)
            nExtraHeight = 2;

        rRenderContext.DrawText(tools::Rectangle(aPos.X(), aPos.Y(), rRect.Right(), rRect.Bottom() - nExtraHeight),
                                sDescription, DrawTextFlags::MultiLine | DrawTextFlags::WordBreak );
    }
    else
    {
        //replace LF to space, so words do not stick together in one line view
        sDescription = sDescription.replace(0x000A, ' ');
        const long nWidth = GetTextWidth( sDescription );
        if (nWidth > rRect.GetWidth() - aPos.X())
            sDescription = rRenderContext.GetEllipsisString(sDescription, rRect.GetWidth() - aPos.X());
        rRenderContext.DrawText(aPos, sDescription);
    }

    // Draw publisher link
    if (rEntry->m_pPublisher)
    {
        rEntry->m_pPublisher->Show();
        aPos = rRect.TopLeft() + Point( ICON_OFFSET + nMaxTitleWidth + (2*SPACE_BETWEEN), TOP_OFFSET );
        rEntry->m_pPublisher->SetPosPixel(aPos);
    }

    // Draw status icons
    if (!rEntry->m_bUser)
    {
        aPos = rRect.TopRight() + Point( -(RIGHT_ICON_OFFSET + SMALL_ICON_SIZE), TOP_OFFSET );
        if (rEntry->m_bLocked)
            rRenderContext.DrawImage(aPos, Size(SMALL_ICON_SIZE, SMALL_ICON_SIZE), m_aLockedImage);
        else
            rRenderContext.DrawImage(aPos, Size(SMALL_ICON_SIZE, SMALL_ICON_SIZE), m_aSharedImage);
    }
    if ((rEntry->m_eState == AMBIGUOUS ) || rEntry->m_bMissingDeps || rEntry->m_bMissingLic)
    {
        aPos = rRect.TopRight() + Point(-(RIGHT_ICON_OFFSET + SPACE_BETWEEN + 2 * SMALL_ICON_SIZE), TOP_OFFSET);
        rRenderContext.DrawImage(aPos, Size(SMALL_ICON_SIZE, SMALL_ICON_SIZE), m_aWarningImage);
    }

    rRenderContext.SetLineColor(COL_LIGHTGRAY);
    rRenderContext.DrawLine(rRect.BottomLeft(), rRect.BottomRight());
}


void ExtensionBox_Impl::RecalcAll()
{
    if ( m_bHasActive )
        CalcActiveHeight( m_nActive );

    SetupScrollBar();

    if ( m_bHasActive )
    {
        tools::Rectangle aEntryRect = GetEntryRect( m_nActive );

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
    if ( nSelect >= static_cast<long>(m_vEntries.size()) )
        nSelect = m_vEntries.size() - 1;

    selectEntry( nSelect );

    return true;
}


void ExtensionBox_Impl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rPaintRect*/)
{
    if ( !m_bInDelete )
        DeleteRemoved();

    if ( m_bNeedsRecalc )
        RecalcAll();

    Point aStart( 0, -m_nTopIndex );
    Size aSize(GetOutputSizePixel());

    if ( m_bHasScrollBar )
        aSize.AdjustWidth( -(m_pScrollBar->GetSizePixel().Width()) );

    const ::osl::MutexGuard aGuard( m_entriesMutex );

    for (auto const& entry : m_vEntries)
    {
        aSize.setHeight( entry->m_bActive ? m_nActiveHeight : m_nStdHeight );
        tools::Rectangle aEntryRect( aStart, aSize );
        DrawRow(rRenderContext, aEntryRect, entry);
        aStart.AdjustY(aSize.Height() );
    }
}


long ExtensionBox_Impl::GetTotalHeight() const
{
    long nHeight = m_vEntries.size() * m_nStdHeight;

    if ( m_bHasActive )
    {
        nHeight += m_nActiveHeight - m_nStdHeight;
    }

    return nHeight;
}


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


void ExtensionBox_Impl::Resize()
{
    RecalcAll();
}

Size ExtensionBox_Impl::GetOptimalSize() const
{
    return LogicToPixel(Size(250, 150), MapMode(MapUnit::MapAppFont));
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeExtensionBox(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap &)
{
    rRet = VclPtr<ExtensionBox_Impl>::Create(pParent);
}

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


bool ExtensionBox_Impl::EventNotify( NotifyEvent& rNEvt )
{
    if ( !m_bInDelete )
        DeleteRemoved();

    bool bHandled = false;

    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        vcl::KeyCode aKeyCode = pKEvt->GetKeyCode();
        sal_uInt16 nKeyCode = aKeyCode.GetCode();

        if ( nKeyCode == KEY_TAB )
             ;
        else if ( aKeyCode.GetGroup() == KEYGROUP_CURSOR )
            bHandled = HandleCursorKey( nKeyCode );
    }

    if ( rNEvt.GetType() == MouseNotifyEvent::COMMAND )
    {
        if ( m_bHasScrollBar &&
             ( rNEvt.GetCommandEvent()->GetCommand() == CommandEventId::Wheel ) )
        {
            const CommandWheelData* pData = rNEvt.GetCommandEvent()->GetWheelData();
            if ( pData->GetMode() == CommandWheelMode::SCROLL )
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
        return Control::EventNotify(rNEvt);
    else
        return true;
}


bool ExtensionBox_Impl::FindEntryPos( const TEntry_Impl& rEntry, const long nStart,
                                      const long nEnd, long &nPos )
{
    nPos = nStart;
    if ( nStart > nEnd )
        return false;

    sal_Int32 eCompare;

    if ( nStart == nEnd )
    {
        eCompare = rEntry->CompareTo( m_pCollator.get(), m_vEntries[ nStart ] );
        if ( eCompare < 0 )
            return false;
        else if ( eCompare == 0 )
        {
            //Workaround. See i86963.
            if (rEntry->m_xPackage != m_vEntries[nStart]->m_xPackage)
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
    eCompare = rEntry->CompareTo( m_pCollator.get(), m_vEntries[ nMid ] );

    if ( eCompare < 0 )
        return FindEntryPos( rEntry, nStart, nMid-1, nPos );
    else if ( eCompare > 0 )
        return FindEntryPos( rEntry, nMid+1, nEnd, nPos );
    else
    {
        //Workaround.See i86963.
        if (rEntry->m_xPackage != m_vEntries[nMid]->m_xPackage)
            return false;

        if ( m_bInCheckMode )
            m_vEntries[ nMid ]->m_bChecked = true;
        nPos = nMid;
        return true;
    }
}

void ExtensionBox_Impl::cleanVecListenerAdded()
{
    m_vListenerAdded.erase(std::remove_if(m_vListenerAdded.begin(), m_vListenerAdded.end(),
        [](const uno::WeakReference<deployment::XPackage>& rxListener) {
            const uno::Reference<deployment::XPackage> hardRef(rxListener);
            return !hardRef.is();
        }),
        m_vListenerAdded.end());
}

void ExtensionBox_Impl::addEventListenerOnce(
    uno::Reference<deployment::XPackage > const & extension)
{
    //make sure to only add the listener once
    cleanVecListenerAdded();
    if ( std::none_of(m_vListenerAdded.begin(), m_vListenerAdded.end(),
                        FindWeakRef(extension)) )
    {
        extension->addEventListener( m_xRemoveListener.get() );
        m_vListenerAdded.emplace_back(extension);
    }
}


void ExtensionBox_Impl::addEntry( const uno::Reference< deployment::XPackage > &xPackage,
                                  bool bLicenseMissing )
{
    long         nPos = 0;
    PackageState eState = TheExtensionManager::getPackageState( xPackage );
    bool         bLocked = m_pManager->isReadOnly( xPackage );

    TEntry_Impl pEntry( new Entry_Impl( xPackage, eState, bLocked ) );

    // Don't add empty entries
    if ( pEntry->m_sTitle.isEmpty() )
        return;

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
        pEntry->m_sErrorText = DpResId( RID_STR_ERROR_MISSING_LICENSE );

    //access to m_nActive must be guarded
    if ( !m_bInCheckMode && m_bHasActive && ( m_nActive >= nPos ) )
        m_nActive += 1;
    guard.clear();

    if ( IsReallyVisible() )
        Invalidate();

    m_bNeedsRecalc = true;
}

void ExtensionBox_Impl::updateEntry( const uno::Reference< deployment::XPackage > &xPackage )
{
    for (auto const& entry : m_vEntries)
    {
        if ( entry->m_xPackage == xPackage )
        {
            PackageState eState = TheExtensionManager::getPackageState( xPackage );
            entry->m_bHasOptions = m_pManager->supportsOptions( xPackage );
            entry->m_eState = eState;
            entry->m_sTitle = xPackage->getDisplayName();
            entry->m_sVersion = xPackage->getVersion();
            entry->m_sDescription = xPackage->getDescription();

            if ( eState == REGISTERED )
                entry->m_bMissingLic = false;

            if ( eState == AMBIGUOUS )
                entry->m_sErrorText = DpResId( RID_STR_ERROR_UNKNOWN_STATUS );
            else if ( ! entry->m_bMissingLic )
                entry->m_sErrorText.clear();

            if ( IsReallyVisible() )
                Invalidate();
            break;
        }
    }
}

//This function is also called as a result of removing an extension.
//see PackageManagerImpl::removePackage
//The gui is a registered as listener on the package. Removing it will cause the
//listeners to be notified an then this function is called. At this moment xPackage
//is in the disposing state and all calls on it may result in a DisposedException.
void ExtensionBox_Impl::removeEntry( const uno::Reference< deployment::XPackage > &xPackage )
{
   if ( ! m_bInDelete )
    {
        bool invalidate = false;
        {
            ::osl::ClearableMutexGuard aGuard( m_entriesMutex );

            auto iIndex = std::find_if(m_vEntries.begin(), m_vEntries.end(),
                [&xPackage](const TEntry_Impl& rxEntry) { return rxEntry->m_xPackage == xPackage; });
            if (iIndex != m_vEntries.end())
            {
                long nPos = iIndex - m_vEntries.begin();

                // Entries mustn't be removed here, because they contain a hyperlink control
                // which can only be deleted when the thread has the solar mutex. Therefore
                // the entry will be moved into the m_vRemovedEntries list which will be
                // cleared on the next paint event
                m_vRemovedEntries.push_back( *iIndex );
                (*iIndex)->m_xPackage->removeEventListener(m_xRemoveListener.get());
                m_vEntries.erase( iIndex );

                m_bNeedsRecalc = true;

                if ( IsReallyVisible() )
                    invalidate = true;

                if ( m_bHasActive )
                {
                    if ( nPos < m_nActive )
                        m_nActive -= 1;
                    else if ( ( nPos == m_nActive ) &&
                              ( nPos == static_cast<long>(m_vEntries.size()) ) )
                        m_nActive -= 1;

                    m_bHasActive = false;
                    //clear before calling out of this method
                    aGuard.clear();
                    selectEntry( m_nActive );
                }
            }
        }

        if (invalidate)
        {
            SolarMutexGuard g;
            Invalidate();
        }
    }
}


void ExtensionBox_Impl::RemoveUnlocked()
{
    bool bAllRemoved = false;

    while ( ! bAllRemoved )
    {
        bAllRemoved = true;

        ::osl::ClearableMutexGuard aGuard( m_entriesMutex );

        for (auto const& entry : m_vEntries)
        {
            if ( !entry->m_bLocked )
            {
                bAllRemoved = false;
                uno::Reference< deployment::XPackage> xPackage = entry->m_xPackage;
                aGuard.clear();
                removeEntry( xPackage );
                break;
            }
        }
    }
}


void ExtensionBox_Impl::prepareChecking()
{
    m_bInCheckMode = true;
    for (auto const& entry : m_vEntries)
    {
        entry->m_bChecked = false;
        entry->m_bNew = false;
    }
}


void ExtensionBox_Impl::checkEntries()
{
    long nNewPos = -1;
    long nChangedActivePos = -1;
    long nPos = 0;
    bool bNeedsUpdate = false;

    ::osl::ClearableMutexGuard guard(m_entriesMutex);
    auto iIndex = m_vEntries.begin();
    while ( iIndex != m_vEntries.end() )
    {
        if ( !(*iIndex)->m_bChecked )
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
                if (nPos < nNewPos) {
                    --nNewPos;
                }
                if (nPos < nChangedActivePos) {
                    --nChangedActivePos;
                }
                if ( nPos < m_nActive )
                    m_nActive -= 1;
                else if ( nPos == m_nActive )
                {
                    nChangedActivePos = nPos;
                    m_nActive = -1;
                    m_bHasActive = false;
                }
                m_vRemovedEntries.push_back( *iIndex );
                iIndex = m_vEntries.erase( iIndex );
            }
        }
        else
            ++iIndex;
    }
    guard.clear();

    m_bInCheckMode = false;

    if ( nNewPos != - 1)
        selectEntry( nNewPos );
    else if (nChangedActivePos != -1) {
        selectEntry(nChangedActivePos);
    }

    if ( bNeedsUpdate )
    {
        m_bNeedsRecalc = true;
        if ( IsReallyVisible() )
            Invalidate();
    }
}


void ExtensionBox_Impl::DoScroll( long nDelta )
{
    m_nTopIndex += nDelta;
    Point aNewSBPt( m_pScrollBar->GetPosPixel() );

    tools::Rectangle aScrRect( Point(), GetOutputSizePixel() );
    aScrRect.AdjustRight( -(m_pScrollBar->GetSizePixel().Width()) );
    Scroll( 0, -nDelta, aScrRect );

    m_pScrollBar->SetPosPixel( aNewSBPt );
}


IMPL_LINK( ExtensionBox_Impl, ScrollHdl, ScrollBar*, pScrBar, void )
{
    DoScroll( pScrBar->GetDelta() );
}

} //namespace dp_gui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
