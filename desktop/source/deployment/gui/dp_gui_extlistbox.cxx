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

#include <dp_shared.hxx>
#include <strings.hrc>
#include "dp_gui.h"
#include "dp_gui_extlistbox.hxx"
#include "dp_gui_theextmgr.hxx"
#include <dp_dependencies.hxx>
#include <bitmaps.hlst>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <com/sun/star/deployment/DependencyException.hpp>
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/deployment/ExtensionRemovedException.hpp>
#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <cppuhelper/weakref.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <vcl/event.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
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
            for ( const auto& i : std::as_const(depExc.UnsatisfiedDependencies) )
            {
                aMissingDep.append("\n");
                aMissingDep.append(dp_misc::Dependencies::getErrorText(i));
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
ExtensionBox_Impl::ExtensionBox_Impl(std::unique_ptr<weld::ScrolledWindow> xScroll)
    : m_bHasScrollBar( false )
    , m_bHasActive( false )
    , m_bNeedsRecalc( true )
    , m_bInCheckMode( false )
    , m_bAdjustActive( false )
    , m_bInDelete( false )
    , m_nActive( 0 )
    , m_nTopIndex( 0 )
    , m_nStdHeight( 0 )
    , m_nActiveHeight( 0 )
    , m_aSharedImage(StockImage::Yes, RID_BMP_SHARED)
    , m_aLockedImage(StockImage::Yes, RID_BMP_LOCKED)
    , m_aWarningImage(StockImage::Yes, RID_BMP_WARNING)
    , m_aDefaultImage(StockImage::Yes, RID_BMP_EXTENSION)
    , m_pManager( nullptr )
    , m_xScrollBar(std::move(xScroll))
{
}

void ExtensionBox_Impl::Init()
{
    m_xScrollBar->connect_vadjustment_changed( LINK( this, ExtensionBox_Impl, ScrollHdl ) );

    auto nIconHeight = 2*TOP_OFFSET + SMALL_ICON_SIZE;
    auto nTitleHeight = 2*TOP_OFFSET + GetTextHeight();
    if ( nIconHeight < nTitleHeight )
        m_nStdHeight = nTitleHeight;
    else
        m_nStdHeight = nIconHeight;
    m_nStdHeight += GetTextHeight() + TOP_OFFSET;

    nIconHeight = ICON_HEIGHT + 2*TOP_OFFSET + 1;
    if ( m_nStdHeight < nIconHeight )
        m_nStdHeight = nIconHeight;

    m_nActiveHeight = m_nStdHeight;

    m_xRemoveListener = new ExtensionRemovedListener( this );

    m_pLocale.reset( new lang::Locale( Application::GetSettings().GetLanguageTag().getLocale() ) );
    m_pCollator.reset( new CollatorWrapper( ::comphelper::getProcessComponentContext() ) );
    m_pCollator->loadDefaultCollator( *m_pLocale, i18n::CollatorOptions::CollatorOptions_IGNORE_CASE );
}

ExtensionBox_Impl::~ExtensionBox_Impl()
{
    if ( ! m_bInDelete )
        DeleteRemoved();

    m_bInDelete = true;

    for (auto const& entry : m_vEntries)
    {
        entry->m_xPackage->removeEventListener( m_xRemoveListener.get() );
    }

    m_vEntries.clear();

    m_xRemoveListener.clear();

    m_pLocale.reset();
    m_pCollator.reset();
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

    aSize.AdjustWidth( -(ICON_OFFSET) );
    aSize.setHeight( 10000 );

    OUString aText( m_vEntries[ nPos ]->m_sErrorText );
    if ( !aText.isEmpty() )
        aText += "\n";
    aText += m_vEntries[ nPos ]->m_sDescription;

    tools::Rectangle aRect = GetDrawingArea()->get_ref_device().GetTextRect(tools::Rectangle( Point(), aSize ), aText,
                                                                            DrawTextFlags::MultiLine | DrawTextFlags::WordBreak);
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

    m_vRemovedEntries.clear();

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
    // expand the point size of the desired font to the equivalent pixel size
    if (vcl::Window* pDefaultDevice = dynamic_cast<vcl::Window*>(Application::GetDefaultDevice()))
        pDefaultDevice->SetPointFont(rRenderContext, GetDrawingArea()->get_font());
    vcl::Font aStdFont(rRenderContext.GetFont());
    vcl::Font aBoldFont(aStdFont);
    aBoldFont.SetWeight(WEIGHT_BOLD);
    rRenderContext.SetFont(aBoldFont);
    auto aTextHeight = rRenderContext.GetTextHeight();

    // Get max title width
    auto nMaxTitleWidth = rRect.GetWidth() - ICON_OFFSET;
    nMaxTitleWidth -= (2 * SMALL_ICON_SIZE) + (4 * SPACE_BETWEEN);
    rRenderContext.SetFont(aStdFont);
    long nLinkWidth = 0;
    if (!rEntry->m_sPublisher.isEmpty())
    {
        nLinkWidth = rRenderContext.GetTextWidth(rEntry->m_sPublisher);
        nMaxTitleWidth -= nLinkWidth + (2 * SPACE_BETWEEN);
    }
    long aVersionWidth = rRenderContext.GetTextWidth(rEntry->m_sVersion);

    aPos = rRect.TopLeft() + Point(ICON_OFFSET, TOP_OFFSET);

    rRenderContext.SetFont(aBoldFont);
    long aTitleWidth = rRenderContext.GetTextWidth(rEntry->m_sTitle) + (aTextHeight / 3);
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
        const long nWidth = rRenderContext.GetTextWidth( sDescription );
        if (nWidth > rRect.GetWidth() - aPos.X())
            sDescription = rRenderContext.GetEllipsisString(sDescription, rRect.GetWidth() - aPos.X());
        rRenderContext.DrawText(aPos, sDescription);
    }

    // Draw publisher link
    if (!rEntry->m_sPublisher.isEmpty())
    {
        aPos = rRect.TopLeft() + Point( ICON_OFFSET + nMaxTitleWidth + (2*SPACE_BETWEEN), TOP_OFFSET );

        rRenderContext.Push(PushFlags::FONT | PushFlags::TEXTCOLOR | PushFlags::TEXTFILLCOLOR);
        rRenderContext.SetTextColor(rStyleSettings.GetLinkColor());
        rRenderContext.SetTextFillColor(rStyleSettings.GetFieldColor());
        vcl::Font aFont = rRenderContext.GetFont();
        // to underline
        aFont.SetUnderline(LINESTYLE_SINGLE);
        rRenderContext.SetFont(aFont);
        rRenderContext.DrawText(aPos, rEntry->m_sPublisher);
        rEntry->m_aLinkRect = tools::Rectangle(aPos, Size(nLinkWidth, aTextHeight));
        rRenderContext.Pop();
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
                m_xScrollBar->vadjustment_set_value( m_nTopIndex );
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
    const auto nTotalHeight = GetTotalHeight();
    const bool bNeedsScrollBar = ( nTotalHeight > aSize.Height() );

    if ( bNeedsScrollBar )
    {
        if ( m_nTopIndex + aSize.Height() > nTotalHeight )
            m_nTopIndex = nTotalHeight - aSize.Height();

        m_xScrollBar->vadjustment_configure(m_nTopIndex, 0, nTotalHeight,
                                            m_nStdHeight, ( aSize.Height() * 4 ) / 5,
                                            aSize.Height());

        if (!m_bHasScrollBar)
            m_xScrollBar->set_vpolicy(VclPolicyType::ALWAYS);
    }
    else if ( m_bHasScrollBar )
    {
        m_xScrollBar->set_vpolicy(VclPolicyType::NEVER);
        m_nTopIndex = 0;
    }

    m_bHasScrollBar = bNeedsScrollBar;
}


void ExtensionBox_Impl::Resize()
{
    RecalcAll();
    Invalidate();
}

void ExtensionBox_Impl::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    Size aSize = pDrawingArea->get_ref_device().LogicToPixel(Size(250, 150), MapMode(MapUnit::MapAppFont));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    SetOutputSizePixel(aSize);

    Init();
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

bool ExtensionBox_Impl::MouseMove( const MouseEvent& rMEvt )
{
    bool bOverHyperlink = false;

    auto nPos = PointToPos( rMEvt.GetPosPixel() );
    if ( ( nPos >= 0 ) && ( nPos < static_cast<long>(m_vEntries.size()) ) )
    {
        const auto& rEntry = m_vEntries[nPos];
        bOverHyperlink = !rEntry->m_sPublisher.isEmpty() && rEntry->m_aLinkRect.IsInside(rMEvt.GetPosPixel());
    }

    if (bOverHyperlink)
        SetPointer(PointerStyle::RefHand);
    else
        SetPointer(PointerStyle::Arrow);

    return false;
}

OUString ExtensionBox_Impl::RequestHelp(tools::Rectangle& rRect)
{
    auto nPos = PointToPos( rRect.TopLeft() );
    if ( ( nPos >= 0 ) && ( nPos < static_cast<long>(m_vEntries.size()) ) )
    {
        const auto& rEntry = m_vEntries[nPos];
        bool bOverHyperlink = !rEntry->m_sPublisher.isEmpty() && rEntry->m_aLinkRect.IsInside(rRect);
        if (bOverHyperlink)
        {
            rRect = rEntry->m_aLinkRect;
            return rEntry->m_sPublisherURL;
        }
    }

    return OUString();
}

bool ExtensionBox_Impl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        if (rMEvt.IsMod1() && m_bHasActive)
            selectEntry(ExtensionBox_Impl::ENTRY_NOTFOUND);   // Selecting a not existing entry will deselect the current one
        else
        {
            auto nPos = PointToPos( rMEvt.GetPosPixel() );

            if ( ( nPos >= 0 ) && ( nPos < static_cast<long>(m_vEntries.size()) ) )
            {
                const auto& rEntry = m_vEntries[nPos];
                if (!rEntry->m_sPublisher.isEmpty() && rEntry->m_aLinkRect.IsInside(rMEvt.GetPosPixel()))
                {
                    try
                    {
                        css::uno::Reference<css::system::XSystemShellExecute> xSystemShellExecute(
                            css::system::SystemShellExecute::create(comphelper::getProcessComponentContext()));
                        //throws css::lang::IllegalArgumentException, css::system::SystemShellExecuteException
                        xSystemShellExecute->execute(rEntry->m_sPublisherURL, OUString(), css::system::SystemShellExecuteFlags::URIS_ONLY);
                    }
                    catch (...)
                    {
                    }
                    return true;
                }
            }

            selectEntry( nPos );
        }
        return true;
    }

    return false;
}

bool ExtensionBox_Impl::KeyInput(const KeyEvent& rKEvt)
{
    if ( !m_bInDelete )
        DeleteRemoved();

    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
    sal_uInt16 nKeyCode = aKeyCode.GetCode();

    bool bHandled = false;
    if (nKeyCode != KEY_TAB && aKeyCode.GetGroup() == KEYGROUP_CURSOR)
        bHandled = HandleCursorKey(nKeyCode);

    return bHandled;
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
    PackageState eState = TheExtensionManager::getPackageState( xPackage );
    bool         bLocked = m_pManager->isReadOnly( xPackage );

    TEntry_Impl pEntry = std::make_shared<Entry_Impl>( xPackage, eState, bLocked );

    // Don't add empty entries
    if ( pEntry->m_sTitle.isEmpty() )
        return;

    {
        osl::MutexGuard guard(m_entriesMutex);
        long nPos = 0;
        if (m_vEntries.empty())
        {
            addEventListenerOnce(xPackage);
            m_vEntries.push_back(pEntry);
        }
        else
        {
            if (!FindEntryPos(pEntry, 0, m_vEntries.size() - 1, nPos))
            {
                addEventListenerOnce(xPackage);
                m_vEntries.insert(m_vEntries.begin() + nPos, pEntry);
            }
            else if (!m_bInCheckMode)
            {
                OSL_FAIL("ExtensionBox_Impl::addEntry(): Will not add duplicate entries");
            }
        }

        pEntry->m_bHasOptions = m_pManager->supportsOptions(xPackage);
        pEntry->m_bUser = (xPackage->getRepositoryName() == USER_PACKAGE_MANAGER);
        pEntry->m_bShared = (xPackage->getRepositoryName() == SHARED_PACKAGE_MANAGER);
        pEntry->m_bNew = m_bInCheckMode;
        pEntry->m_bMissingLic = bLicenseMissing;

        if (bLicenseMissing)
            pEntry->m_sErrorText = DpResId(RID_STR_ERROR_MISSING_LICENSE);

        //access to m_nActive must be guarded
        if (!m_bInCheckMode && m_bHasActive && (m_nActive >= nPos))
            m_nActive += 1;
    }

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
//listeners to be notified and then this function is called. At this moment xPackage
//is in the disposing state and all calls on it may result in a DisposedException.
void ExtensionBox_Impl::removeEntry( const uno::Reference< deployment::XPackage > &xPackage )
{
    if (  m_bInDelete )
        return;

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

    {
        osl::MutexGuard guard(m_entriesMutex);
        auto iIndex = m_vEntries.begin();
        while (iIndex != m_vEntries.end())
        {
            if (!(*iIndex)->m_bChecked)
            {
                (*iIndex)->m_bChecked = true;
                bNeedsUpdate = true;
                nPos = iIndex - m_vEntries.begin();
                if ((*iIndex)->m_bNew)
                { // add entry to list and correct active pos
                    if (nNewPos == -1)
                        nNewPos = nPos;
                    if (nPos <= m_nActive)
                        m_nActive += 1;
                    ++iIndex;
                }
                else
                { // remove entry from list
                    if (nPos < nNewPos)
                    {
                        --nNewPos;
                    }
                    if (nPos < nChangedActivePos)
                    {
                        --nChangedActivePos;
                    }
                    if (nPos < m_nActive)
                        m_nActive -= 1;
                    else if (nPos == m_nActive)
                    {
                        nChangedActivePos = nPos;
                        m_nActive = -1;
                        m_bHasActive = false;
                    }
                    m_vRemovedEntries.push_back(*iIndex);
                    iIndex = m_vEntries.erase(iIndex);
                }
            }
            else
                ++iIndex;
        }
    }

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

IMPL_LINK(ExtensionBox_Impl, ScrollHdl, weld::ScrolledWindow&, rScrBar, void)
{
    m_nTopIndex = rScrBar.vadjustment_get_value();
    Invalidate();
}

} //namespace dp_gui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
