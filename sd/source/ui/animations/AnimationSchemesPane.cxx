/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AnimationSchemesPane.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:50:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include "AnimationSchemesPane.hxx"
#include "AnimationSchemesPane.hrc"
#include "CustomAnimation.hrc"

#include "TransitionPreset.hxx"
#include "sdresid.hxx"
#include "ViewShellBase.hxx"
#include "DrawDocShell.hxx"
#include "SlideSorterViewShell.hxx"
#include "PaneManager.hxx"
#include "drawdoc.hxx"
#include "filedlg.hxx"
#include "strings.hrc"
#include "EventMultiplexer.hxx"

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _SVT_CONTROLDIMS_HRC_
#include <svtools/controldims.hrc>
#endif
#ifndef _GALLERY_HXX_
#include <svx/gallery.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#include <algorithm>


using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::com::sun::star::uno::RuntimeException;

// ______________________
//
// Local Helper Functions
// ______________________

namespace
{

::sd::slidesorter::SlideSorterViewShell * lcl_getSlideSorterViewShell( ::sd::ViewShellBase& rBase )
{
    // Find a slide sorter to get the selection from.  When one is
    // displayed in the center pane use that.  Otherwise use the one in
    // the left pane.
    ::sd::ViewShell * pSlideSorter =
        rBase.GetPaneManager().GetViewShell( ::sd::PaneManager::PT_CENTER );

    if( pSlideSorter &&
        pSlideSorter->GetShellType() != ::sd::ViewShell::ST_SLIDE_SORTER )
    {
        pSlideSorter =
            rBase.GetPaneManager().GetViewShell( ::sd::PaneManager::PT_LEFT );
    }

    if( pSlideSorter &&
        pSlideSorter->GetShellType() != ::sd::ViewShell::ST_SLIDE_SORTER )
        pSlideSorter = 0;

    ::sd::slidesorter::SlideSorterViewShell * pSlideSorterViewShell =
          dynamic_cast< ::sd::slidesorter::SlideSorterViewShell * >( pSlideSorter );

    return pSlideSorterViewShell;
}

::std::vector< SdPage * > lcl_getSelectedPages( ::sd::ViewShellBase& rBase )
{
    ::sd::slidesorter::SlideSorterViewShell * pSlideSorterViewShell =
          lcl_getSlideSorterViewShell( rBase );
    DBG_ASSERT( pSlideSorterViewShell, "No Slide-Sorter available" );
    ::std::vector< SdPage * > aSelectedPages;

    if( pSlideSorterViewShell )
    {
        pSlideSorterViewShell->GetSelectedPages( aSelectedPages );
    }

    return aSelectedPages;
}

// void lcl_ApplyToPages( ::std::vector< SdPage * > aPages, const ::sd::impl::TransitionEffect & rEffect )
// {
//     ::std::vector< SdPage * >::const_iterator aIt( aPages.begin());
//     const ::std::vector< SdPage * >::const_iterator aEndIt( aPages.end());
//     for( ; aIt != aEndIt; ++aIt )
//     {
//         rEffect.applyTo( *(*aIt) );
//     }
// }

} // anonymous namespace

namespace sd
{

// ____________________
//
// AnimationSchemesPane
// ____________________

AnimationSchemesPane::AnimationSchemesPane(
    ::Window * pParent,
    ViewShellBase & rBase,
    SdDrawDocument* pDoc ) :
        Control( pParent, SdResId( DLG_ANIMATION_SCHEMES_PANE ) ),

        mrBase( rBase ),
        mpDrawDoc( pDoc ),
        maFL_APPLY_SCHEME( this, SdResId( FL_APPLY_SCHEME ) ),
        maLB_ANIMATION_SCHEMES( this, SdResId( LB_ANIMATION_SCHEMES ) ),
        maFL_EMPTY1( this, SdResId( FL_EMPTY1 ) ),
        maPB_APPLY_TO_ALL( this, SdResId( PB_APPLY_TO_ALL ) ),
        maPB_PLAY( this, SdResId( PB_PLAY ) ),
        maPB_SLIDE_SHOW( this, SdResId( PB_SLIDE_SHOW ) ),
        maFL_EMPTY2( this, SdResId( FL_EMPTY2 ) ),
        maCB_AUTO_PREVIEW( this, SdResId( CB_AUTO_PREVIEW ) ),

        maSTR_NO_SCHEME( SdResId( STR_NO_SCHEME ) ),
        mbHasSelection( false ),
        mbUpdatingControls( false )
{
    // use no resource ids from here on
    FreeResource();

    if( pDoc )
        mxModel.set( pDoc->getUnoModel(), uno::UNO_QUERY );
    // TODO: get correct view
    if( mxModel.is())
        mxView.set( mxModel->getCurrentController(), uno::UNO_QUERY );

    // fill list box of slide transitions
    maLB_ANIMATION_SCHEMES.InsertEntry( maSTR_NO_SCHEME );

    // set defaults
    maCB_AUTO_PREVIEW.Check();      // automatic preview on

    // update control states before adding handlers
    updateLayout();
    updateControls();

    // set handlers
    maPB_APPLY_TO_ALL.SetClickHdl( LINK( this, AnimationSchemesPane, ApplyToAllButtonClicked ));
    maPB_PLAY.SetClickHdl( LINK( this, AnimationSchemesPane, PlayButtonClicked ));
    maPB_SLIDE_SHOW.SetClickHdl( LINK( this, AnimationSchemesPane, SlideShowButtonClicked ));

    maLB_ANIMATION_SCHEMES.SetSelectHdl( LINK( this, AnimationSchemesPane, SchemeSelected ));

    addListener();
}

AnimationSchemesPane::~AnimationSchemesPane()
{
    removeListener();
}

void AnimationSchemesPane::Resize()
{
    updateLayout();
}

void AnimationSchemesPane::onSelectionChanged()
{
    updateControls();
}

void AnimationSchemesPane::onChangeCurrentPage()
{
    updateControls();
}

void AnimationSchemesPane::updateLayout()
{
    ::Size aPaneSize( GetSizePixel() );

    // start layouting elements from bottom to top.  The remaining space is used
    // for the topmost list box
    ::Point aOffset( LogicToPixel( Point( 3, 3 ), MAP_APPFONT ) );
    long nOffsetX = aOffset.getX();
    long nOffsetY = aOffset.getY();
    long nOffsetBtnX = LogicToPixel( Point( 6, 1 ), MAP_APPFONT ).getX();

    ::Point aUpperLeft( nOffsetX, aPaneSize.getHeight() - nOffsetY );
    long nMaxWidth = aPaneSize.getWidth() - 2 * nOffsetX;

    // auto preview check-box
    ::Size aCtrlSize = maCB_AUTO_PREVIEW.GetSizePixel();
    aCtrlSize.setWidth( maCB_AUTO_PREVIEW.CalcMinimumSize( nMaxWidth ).getWidth());
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
    maCB_AUTO_PREVIEW.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // fixed line above check-box
    aCtrlSize = maFL_EMPTY2.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
    maFL_EMPTY2.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // buttons "Play" and "Slide Show"
    long nPlayButtonWidth = maPB_PLAY.CalcMinimumSize().getWidth() + 2 * nOffsetBtnX;
    long nSlideShowButtonWidth = maPB_SLIDE_SHOW.CalcMinimumSize().getWidth() + 2 * nOffsetBtnX;

    if( (nPlayButtonWidth + nSlideShowButtonWidth + nOffsetX) <= nMaxWidth )
    {
        // place buttons side by side
        aCtrlSize = maPB_PLAY.GetSizePixel();
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
        aCtrlSize.setWidth( nPlayButtonWidth );
        maPB_PLAY.SetPosSizePixel( aUpperLeft, aCtrlSize );

        aUpperLeft.setX( aUpperLeft.getX() + nPlayButtonWidth + nOffsetX );
        aCtrlSize.setWidth( nSlideShowButtonWidth );
        maPB_SLIDE_SHOW.SetPosSizePixel( aUpperLeft, aCtrlSize );
        aUpperLeft.setX( nOffsetX );
    }
    else
    {
        // place buttons on top of each other
        aCtrlSize = maPB_SLIDE_SHOW.GetSizePixel();
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
        aCtrlSize.setWidth( nSlideShowButtonWidth );
        maPB_SLIDE_SHOW.SetPosSizePixel( aUpperLeft, aCtrlSize );

        aCtrlSize = maPB_PLAY.GetSizePixel();
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
        aCtrlSize.setWidth( nPlayButtonWidth );
        maPB_PLAY.SetPosSizePixel( aUpperLeft, aCtrlSize );
    }

    // "Apply to All Slides" button
    aCtrlSize = maPB_APPLY_TO_ALL.GetSizePixel();
    aCtrlSize.setWidth( maPB_APPLY_TO_ALL.CalcMinimumSize( nMaxWidth ).getWidth() + 2 * nOffsetBtnX );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    maPB_APPLY_TO_ALL.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // fixed line above "Apply to All Slides" button
    aCtrlSize = maFL_EMPTY1.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
    maFL_EMPTY1.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // fixed line "Apply to selected slides"
    aCtrlSize = maFL_APPLY_SCHEME.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth );
    ::Point aUpperLeftCorner( nOffsetX, nOffsetY );
    maFL_APPLY_SCHEME.SetPosSizePixel( aUpperLeftCorner, aCtrlSize );
    aUpperLeftCorner.setY( aUpperLeftCorner.getY() + aCtrlSize.getHeight() + nOffsetY );
    aUpperLeft.setY( aUpperLeft.getY() - nOffsetY );

    // list box animation scheme
    aCtrlSize.setWidth( nMaxWidth );
    aCtrlSize.setHeight( aUpperLeft.getY() - aUpperLeftCorner.getY() );
    maLB_ANIMATION_SCHEMES.SetPosSizePixel( aUpperLeftCorner, aCtrlSize );
}

void AnimationSchemesPane::updateControls()
{
    ::std::vector< SdPage * > aSelectedPages( lcl_getSelectedPages( mrBase ));
    if( aSelectedPages.empty())
    {
        mbHasSelection = false;
        return;
    }
    mbHasSelection = true;

    DBG_ASSERT( ! mbUpdatingControls, "Multiple Control Updates" );
    mbUpdatingControls = true;

    // get model data for first page
#ifdef DBG_UTIL
    SdPage * pFirstPage = aSelectedPages.front();
    DBG_ASSERT( pFirstPage, "Invalid Page" );
#endif

//     // merge with other pages
//     ::std::vector< SdPage * >::const_iterator aIt( aSelectedPages.begin());
//     const ::std::vector< SdPage * >::const_iterator aEndIt( aSelectedPages.end());

//     // start with second page (note aIt != aEndIt, because ! aSelectedPages.empty())
//     for( ++aIt ;aIt != aEndIt; ++aIt )
//     {
//         if( *aIt )
//             // merge
//     }

    mbUpdatingControls = false;

    updateControlState();
}

void AnimationSchemesPane::updateControlState()
{
    maLB_ANIMATION_SCHEMES.Enable( mbHasSelection );

    maPB_APPLY_TO_ALL.Enable( mbHasSelection );
    maPB_PLAY.Enable( mbHasSelection );
//     maPB_SLIDE_SHOW.Enable( TRUE );
    maCB_AUTO_PREVIEW.Enable( mbHasSelection );
}

void AnimationSchemesPane::getAnimationSchemeFromControls() const
{
    // TODO: Implement
}

void AnimationSchemesPane::applyToSelectedPages()
{
    if( ! mbUpdatingControls )
    {
        ::std::vector< SdPage * > aSelectedPages( lcl_getSelectedPages( mrBase ));
        if( ! aSelectedPages.empty())
        {
//             lcl_ApplyToPages( aSelectedPages, getAnimationSchemeFromControls() );
        }
        if( maCB_AUTO_PREVIEW.IsEnabled() &&
            maCB_AUTO_PREVIEW.IsChecked())
        {
            playCurrentScheme();
        }
    }
}

void AnimationSchemesPane::playCurrentScheme()
{
    // TODO: Implement a preview in the center view
}

void AnimationSchemesPane::addListener()
{
    Link aLink( LINK(this,AnimationSchemesPane,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer().AddEventListener(
        aLink,
        tools::EventMultiplexerEvent::EID_EDIT_VIEW_SELECTION
        | tools::EventMultiplexerEvent::EID_CURRENT_PAGE
        | tools::EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED
        | tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED);
}

void AnimationSchemesPane::removeListener()
{
    Link aLink( LINK(this,AnimationSchemesPane,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer().RemoveEventListener( aLink );
}

IMPL_LINK(AnimationSchemesPane,EventMultiplexerListener,
    tools::EventMultiplexerEvent*,pEvent)
{
    switch (pEvent->meEventId)
    {
        case tools::EventMultiplexerEvent::EID_EDIT_VIEW_SELECTION:
            onSelectionChanged();
            break;

        case tools::EventMultiplexerEvent::EID_CURRENT_PAGE:
            onChangeCurrentPage();
            break;

        case tools::EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED:
            mxView = Reference<drawing::XDrawView>();
            onSelectionChanged();
            onChangeCurrentPage();
            break;

        case tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED:
            mxView = Reference<drawing::XDrawView>::query( mxModel->getCurrentController() );
            onSelectionChanged();
            onChangeCurrentPage();
            break;
    }
    return 0;
}

IMPL_LINK( AnimationSchemesPane, ApplyToAllButtonClicked, void *, EMPTYARG )
{
    DBG_ASSERT( mpDrawDoc, "Invalid Draw Document!" );
    if( !mpDrawDoc )
        return 0;

    ::std::vector< SdPage * > aPages;

    sal_uInt16 nPageCount = mpDrawDoc->GetSdPageCount( PK_STANDARD );
    aPages.reserve( nPageCount );
    for( sal_uInt16 i=0; i<nPageCount; ++i )
    {
        SdPage * pPage = mpDrawDoc->GetSdPage( i, PK_STANDARD );
        if( pPage )
            aPages.push_back( pPage );
    }

//     if( ! aPages.empty())
//         lcl_ApplyToPages( aPages, getAnimationSchemeFromControls() );

    return 0;
}


IMPL_LINK( AnimationSchemesPane, PlayButtonClicked, void *, EMPTYARG )
{
    playCurrentScheme();
    return 0;
}

IMPL_LINK( AnimationSchemesPane, SlideShowButtonClicked, void *, EMPTYARG )
{
    mrBase.StartPresentation();
    return 0;
}

IMPL_LINK( AnimationSchemesPane, SchemeSelected, void *, EMPTYARG )
{
    applyToSelectedPages();
    return 0;
}

::Window * createAnimationSchemesPanel( ::Window* pParent, ViewShellBase& rBase )
{
    ::Window* pWindow = 0;

    DrawDocShell* pDocSh = rBase.GetDocShell();
    if( pDocSh )
        pWindow = new AnimationSchemesPane( pParent, rBase, pDocSh->GetDoc() );

    return pWindow;
}


} //  namespace sd
