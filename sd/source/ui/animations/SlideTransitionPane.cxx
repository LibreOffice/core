/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlideTransitionPane.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2005-10-11 08:16:38 $
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

#ifndef _COM_SUN_STAR_ANIMATIONS_XANIMATIONNODE_HPP_
#include <com/sun/star/animations/XAnimationNode.hpp>
#endif

#include "SlideTransitionPane.hxx"
#include "SlideTransitionPane.hrc"
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
#include "DrawController.hxx"

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
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef _SD_SLIDESHOW_HXX
#include "slideshow.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#ifndef _SD_SDUNDOGR_HXX
#include "sdundogr.hxx"
#endif
#ifndef _SD_UNDO_ANIM_HXX
#include "undoanim.hxx"
#endif
#ifndef _SD_OPTSITEM_HXX
#include "optsitem.hxx"
#endif
#ifndef _SDDLL_HXX
#include "sddll.hxx"
#endif

#include "DialogListBox.hxx"

#include <algorithm>
#include <memory>


using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::com::sun::star::uno::RuntimeException;

// ____________________________
//
// ::sd::impl::TransitionEffect
// ____________________________

namespace sd
{
namespace impl
{
struct TransitionEffect
{
    TransitionEffect() :
            mnType( 0 ),
            mnSubType( 0 ),
            mbDirection( sal_True ),
            mnFadeColor( 0 )
    {
        init();
    }
    explicit TransitionEffect( const ::sd::TransitionPreset & rPreset ) :
            mnType( rPreset.getTransition()),
            mnSubType( rPreset.getSubtype()),
            mbDirection( rPreset.getDirection()),
            mnFadeColor( rPreset.getFadeColor())
    {
        init();
    }
    explicit TransitionEffect( sal_Int16 nType, sal_Int16 nSubType,
                               sal_Bool  bDirection, sal_Int32 nFadeColor ) :
            mnType( nType),
            mnSubType( nSubType ),
            mbDirection( bDirection ),
            mnFadeColor( nFadeColor )
    {
        init();
    }
    explicit TransitionEffect( const SdPage & rPage ) :
            mnType( rPage.getTransitionType() ),
            mnSubType( rPage.getTransitionSubtype() ),
            mbDirection( rPage.getTransitionDirection() ),
            mnFadeColor( rPage.getTransitionFadeColor() )
    {
        init();

        mfDuration = rPage.getTransitionDuration();
        mnTime = rPage.GetTime();
        mePresChange = rPage.GetPresChange();
        mbSoundOn = rPage.IsSoundOn();
        maSound = rPage.GetSoundFile();
    }

    void init()
    {
        mfDuration = 2.0;
        mnTime = 0;
        mePresChange = PRESCHANGE_MANUAL;
        mbSoundOn = FALSE;

        mbEffectAmbiguous = false;
        mbDurationAmbiguous = false;
        mbTimeAmbiguous = false;
        mbPresChangeAmbiguous = false;
        mbSoundOnAmbiguous = false;
        mbSoundAmbiguous = false;
    }

    void setAllAmbiguous()
    {
        mbEffectAmbiguous = true;
        mbDurationAmbiguous = true;
        mbTimeAmbiguous = true;
        mbPresChangeAmbiguous = true;
        mbSoundOnAmbiguous = true;
        mbSoundAmbiguous = true;
    }

    bool operator == ( const ::sd::TransitionPreset & rPreset ) const
    {
        return
            (mnType == rPreset.getTransition()) &&
            (mnSubType == rPreset.getSubtype()) &&
            (mbDirection == rPreset.getDirection()) &&
            (mnFadeColor ==  rPreset.getFadeColor());
    }

    void applyTo( SdPage & rOutPage ) const
    {
        if( ! mbEffectAmbiguous )
        {
            rOutPage.setTransitionType( mnType );
            rOutPage.setTransitionSubtype( mnSubType );
            rOutPage.setTransitionDirection( mbDirection );
            rOutPage.setTransitionFadeColor( mnFadeColor );
        }

        if( ! mbDurationAmbiguous )
            rOutPage.setTransitionDuration( mfDuration );
        if( ! mbTimeAmbiguous )
            rOutPage.SetTime( mnTime );
        if( ! mbPresChangeAmbiguous )
            rOutPage.SetPresChange( mePresChange );
        if( ! mbSoundOnAmbiguous )
            rOutPage.SetSound( mbSoundOn );
        if( ! mbSoundAmbiguous )
            rOutPage.SetSoundFile( maSound );
    }

    void compareWith( const SdPage & rPage )
    {
        TransitionEffect aOtherEffect( rPage );
        mbEffectAmbiguous = mbEffectAmbiguous || aOtherEffect.mbEffectAmbiguous;
        mbDurationAmbiguous = mbDurationAmbiguous || aOtherEffect.mbDurationAmbiguous;
        mbTimeAmbiguous = mbTimeAmbiguous || aOtherEffect.mbTimeAmbiguous;
        mbPresChangeAmbiguous = mbPresChangeAmbiguous || aOtherEffect.mbPresChangeAmbiguous;
        mbSoundOnAmbiguous = mbSoundOnAmbiguous || aOtherEffect.mbSoundOnAmbiguous;
        mbSoundAmbiguous = mbSoundAmbiguous || aOtherEffect.mbSoundAmbiguous;
    }

    // effect
    sal_Int16 mnType;
    sal_Int16 mnSubType;
    sal_Bool  mbDirection;
    sal_Int32 mnFadeColor;

    // other settings
    double      mfDuration;
    ULONG       mnTime;
    PresChange  mePresChange;
    BOOL        mbSoundOn;
    String      maSound;

    bool mbEffectAmbiguous;
    bool mbDurationAmbiguous;
    bool mbTimeAmbiguous;
    bool mbPresChangeAmbiguous;
    bool mbSoundOnAmbiguous;
    bool mbSoundAmbiguous;
};

} // namespace impl
} // namespace sd

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

void lcl_ApplyToPages( ::std::vector< SdPage * > aPages, const ::sd::impl::TransitionEffect & rEffect )
{
    ::std::vector< SdPage * >::const_iterator aIt( aPages.begin());
    const ::std::vector< SdPage * >::const_iterator aEndIt( aPages.end());
    for( ; aIt != aEndIt; ++aIt )
    {
        rEffect.applyTo( *(*aIt) );
    }
}

void lcl_CreateUndoForPages(  ::std::vector< SdPage * > aPages, ::sd::ViewShellBase& rBase )
{
    ::sd::DrawDocShell* pDocSh  = rBase.GetDocShell();
    SfxUndoManager* pManager    = pDocSh->GetUndoManager();
    SdDrawDocument* pDoc        = pDocSh->GetDoc();
    if( pManager && pDocSh && pDoc )
    {
        String aComment( SdResId(STR_UNDO_SLIDE_PARAMS) );
        pManager->EnterListAction(aComment, aComment);
        SdUndoGroup* pUndoGroup = new SdUndoGroup( pDoc );
        pUndoGroup->SetComment( aComment );

        ::std::vector< SdPage * >::const_iterator aIt( aPages.begin());
        const ::std::vector< SdPage * >::const_iterator aEndIt( aPages.end());
        for( ; aIt != aEndIt; ++aIt )
        {
            pUndoGroup->AddAction( new sd::UndoTransition( pDoc, (*aIt) ) );
        }

        pManager->AddUndoAction( pUndoGroup );
        pManager->LeaveListAction();
    }
}

sal_uInt16 lcl_getTransitionEffectIndex(
    SdDrawDocument * pDoc,
    const ::sd::impl::TransitionEffect & rTransition )
{
    // first entry: "<none>"
    sal_uInt16 nResultIndex = LISTBOX_ENTRY_NOTFOUND;

    if( pDoc )
    {
        sal_uInt16 nCurrentIndex = 0;
        const ::sd::TransitionPresetList & rPresetList = ::sd::TransitionPreset::getTransitionPresetList();
        ::sd::TransitionPresetList::const_iterator aIt( rPresetList.begin());
        const ::sd::TransitionPresetList::const_iterator aEndIt( rPresetList.end());
        for( ; aIt != aEndIt; ++aIt, ++nCurrentIndex )
        {
            if( rTransition.operator==( *(*aIt) ))
            {
                nResultIndex = nCurrentIndex;
                break;
            }
        }
    }

    return nResultIndex;
}

::sd::TransitionPresetPtr lcl_getTransitionPresetByUIName(
    SdDrawDocument * pDoc,
    const OUString & rUIName )
{
    ::sd::TransitionPresetPtr pResult;
    if( pDoc )
    {
        const ::sd::TransitionPresetList& rPresetList = ::sd::TransitionPreset::getTransitionPresetList();
        ::sd::TransitionPresetList::const_iterator aIter( rPresetList.begin() );
        const ::sd::TransitionPresetList::const_iterator aEnd( rPresetList.end() );
        for( ; aIter != aEnd; ++aIter )
        {
            if( (*aIter)->getUIName().equals( rUIName ))
            {
                pResult = *aIter;
                break;
            }
        }
    }

    return pResult;
}

struct lcl_EqualsSoundFileName : public ::std::unary_function< String, bool >
{
    explicit lcl_EqualsSoundFileName( const String & rStr ) :
            maStr( rStr )
    {}

    bool operator() ( const String & rStr ) const
    {
        // note: formerly this was a case insensitive search for all
        // platforms. It seems more sensible to do this platform-dependent
#if defined( WIN ) || defined( WNT )
        return maStr.EqualsIgnoreCaseAscii( rStr );
#else
        return maStr.Equals( rStr );
#endif
    }

private:
    String maStr;
};

// returns -1 if no object was found
bool lcl_findSoundInList( const ::std::vector< String > & rSoundList,
                          const String & rFileName,
                          ::std::vector< String >::size_type & rOutPosition )
{
    ::std::vector< String >::const_iterator aIt =
          ::std::find_if( rSoundList.begin(), rSoundList.end(),
                          lcl_EqualsSoundFileName( rFileName ));
    if( aIt != rSoundList.end())
    {
        rOutPosition = ::std::distance( rSoundList.begin(), aIt );
        return true;
    }

    return false;
}

String lcl_getSoundFileURL(
    const ::std::vector< String > & rSoundList,
    const ListBox & rListBox )
{
    String aResult;

    if( rListBox.GetSelectEntryCount() > 0 )
    {
        sal_uInt16 nPos = rListBox.GetSelectEntryPos();
        // the first three entries are no actual sounds
        if( nPos >= 3 )
        {
            DBG_ASSERT( rListBox.GetEntryCount() - 3 == rSoundList.size(),
                        "Sound list-box is not synchronized to sound list" );
            nPos -= 3;
            if( rSoundList.size() > nPos )
                aResult = rSoundList[ nPos ];
        }
    }

    return aResult;
}

struct lcl_AppendSoundToListBox : public ::std::unary_function< String, void >
{
    lcl_AppendSoundToListBox( ListBox & rListBox ) :
            mrListBox( rListBox )
    {}

    void operator() ( const String & rString ) const
    {
        INetURLObject aURL( rString );
        mrListBox.InsertEntry( aURL.GetBase(), LISTBOX_APPEND );
    }

private:
    ListBox & mrListBox;
};

void lcl_FillSoundListBox(
    const ::std::vector< String > & rSoundList,
    ListBox & rOutListBox )
{
    sal_uInt16 nCount = rOutListBox.GetEntryCount();

    // keep first three entries
    for( sal_uInt16 i=nCount - 1; i>=3; --i )
        rOutListBox.RemoveEntry( i );

    ::std::for_each( rSoundList.begin(), rSoundList.end(),
                     lcl_AppendSoundToListBox( rOutListBox ));
}

} // anonymous namespace

namespace sd
{

// ___________________
//
// SlideTransitionPane
// ___________________

SlideTransitionPane::SlideTransitionPane(
    ::Window * pParent,
    ViewShellBase & rBase,
    const Size& rMinSize,
    SdDrawDocument* pDoc ) :
        Control( pParent, SdResId( DLG_SLIDE_TRANSITION_PANE ) ),

        mrBase( rBase ),
        mpDrawDoc( pDoc ),
        maMinSize( rMinSize ),
        maFL_APPLY_TRANSITION( this, SdResId( FL_APPLY_TRANSITION ) ),
        maLB_SLIDE_TRANSITIONS( this, SdResId( LB_SLIDE_TRANSITIONS ) ),
        maFL_MODIFY_TRANSITION( this, SdResId( FL_MODIFY_TRANSITION ) ),
        maFT_SPEED( this, SdResId( FT_SPEED ) ),
        maLB_SPEED( this, SdResId( LB_SPEED ) ),
        maFT_SOUND( this, SdResId( FT_SOUND ) ),
        maLB_SOUND( this, SdResId( LB_SOUND ) ),
        maCB_LOOP_SOUND( this, SdResId( CB_LOOP_SOUND ) ),
        maFL_ADVANCE_SLIDE( this, SdResId( FL_ADVANCE_SLIDE ) ),
        maRB_ADVANCE_ON_MOUSE( this, SdResId( RB_ADVANCE_ON_MOUSE ) ),
        maRB_ADVANCE_AUTO( this, SdResId( RB_ADVANCE_AUTO ) ),
        maMF_ADVANCE_AUTO_AFTER( this, SdResId( MF_ADVANCE_AUTO_AFTER ) ),
        maFL_EMPTY1( this, SdResId( FL_EMPTY1 ) ),
        maPB_APPLY_TO_ALL( this, SdResId( PB_APPLY_TO_ALL ) ),
        maPB_PLAY( this, SdResId( PB_PLAY ) ),
        maPB_SLIDE_SHOW( this, SdResId( PB_SLIDE_SHOW ) ),
        maFL_EMPTY2( this, SdResId( FL_EMPTY2 ) ),
        maCB_AUTO_PREVIEW( this, SdResId( CB_AUTO_PREVIEW ) ),

        maSTR_NO_TRANSITION( SdResId( STR_NO_TRANSITION ) ),
        mbHasSelection( false ),
        mbUpdatingControls( false )
{
    // use no resource ids from here on
    FreeResource();

    // use bold font for group headings (same font for all fixed lines):
    Font font( maFL_APPLY_TRANSITION.GetFont() );
    font.SetWeight( WEIGHT_BOLD );
    maFL_APPLY_TRANSITION.SetFont( font );
    maFL_MODIFY_TRANSITION.SetFont( font );
    maFL_ADVANCE_SLIDE.SetFont( font );

    if( pDoc )
        mxModel.set( pDoc->getUnoModel(), uno::UNO_QUERY );
    // TODO: get correct view
    if( mxModel.is())
        mxView.set( mxModel->getCurrentController(), uno::UNO_QUERY );

    // fill list box of slide transitions
    maLB_SLIDE_TRANSITIONS.InsertEntry( maSTR_NO_TRANSITION );

    // test
    const TransitionPresetList& rPresetList = TransitionPreset::getTransitionPresetList();
    TransitionPresetList::const_iterator aIter( rPresetList.begin() );
    const TransitionPresetList::const_iterator aEnd( rPresetList.end() );
    ::std::size_t nIndex = 0;
    ::std::size_t nUIIndex = 0;
    while( aIter != aEnd )
    {
        TransitionPresetPtr pPreset = (*aIter++);
        const OUString aUIName( pPreset->getUIName() );
         if( aUIName.getLength() )
        {
            maLB_SLIDE_TRANSITIONS.InsertEntry( aUIName );
            m_aPresetIndexes[ nIndex ] = nUIIndex;
            ++nUIIndex;
        }
        ++nIndex;
    }

    // set defaults
    maCB_AUTO_PREVIEW.Check();      // automatic preview on

    // update control states before adding handlers
    updateLayout();
    updateSoundList();
    updateControls();

    // set handlers
    maPB_APPLY_TO_ALL.SetClickHdl( LINK( this, SlideTransitionPane, ApplyToAllButtonClicked ));
    maPB_PLAY.SetClickHdl( LINK( this, SlideTransitionPane, PlayButtonClicked ));
    maPB_SLIDE_SHOW.SetClickHdl( LINK( this, SlideTransitionPane, SlideShowButtonClicked ));

    maLB_SLIDE_TRANSITIONS.SetSelectHdl( LINK( this, SlideTransitionPane, TransitionSelected ));

    maLB_SPEED.SetSelectHdl( LINK( this, SlideTransitionPane, SpeedListBoxSelected ));
    maLB_SOUND.SetSelectHdl( LINK( this, SlideTransitionPane, SoundListBoxSelected ));
    maCB_LOOP_SOUND.SetClickHdl( LINK( this, SlideTransitionPane, LoopSoundBoxChecked ));

    maRB_ADVANCE_ON_MOUSE.SetToggleHdl( LINK( this, SlideTransitionPane, AdvanceSlideRadioButtonToggled ));
    maRB_ADVANCE_AUTO.SetToggleHdl( LINK( this, SlideTransitionPane, AdvanceSlideRadioButtonToggled ));
    maMF_ADVANCE_AUTO_AFTER.SetModifyHdl( LINK( this, SlideTransitionPane, AdvanceTimeModified ));
    maCB_AUTO_PREVIEW.SetClickHdl( LINK( this, SlideTransitionPane, AutoPreviewClicked ));
    addListener();
}

SlideTransitionPane::~SlideTransitionPane()
{
    removeListener();
}

void SlideTransitionPane::Resize()
{
    updateLayout();
}

void SlideTransitionPane::onSelectionChanged()
{
    updateControls();
}

void SlideTransitionPane::onChangeCurrentPage()
{
    updateControls();
}

::std::vector< SdPage * > SlideTransitionPane::getSelectedPages()
{
    ::sd::slidesorter::SlideSorterViewShell * pSlideSorterViewShell =
          lcl_getSlideSorterViewShell( mrBase );
//    DBG_ASSERT( pSlideSorterViewShell, "No Slide-Sorter available" );
    ::std::vector< SdPage * > aSelectedPages;

    if( pSlideSorterViewShell )
    {
        pSlideSorterViewShell->GetSelectedPages( aSelectedPages );
    }
    else if( mxView.is() )
    {
        SdPage* pPage = SdPage::getImplementation( mxView->getCurrentPage() );
        if( pPage )
            aSelectedPages.push_back( pPage );
    }

    return aSelectedPages;
}

void SlideTransitionPane::updateLayout()
{
    ::Size aPaneSize( GetSizePixel() );
    if( aPaneSize.Width() < maMinSize.Width() )
        aPaneSize.Width() = maMinSize.Width();

    if( aPaneSize.Height() < maMinSize.Height() )
        aPaneSize.Height() = maMinSize.Height();

    // start layouting elements from bottom to top.  The remaining space is used
    // for the topmost list box
    ::Point aOffset( LogicToPixel( Point( 3, 3 ), MAP_APPFONT ) );
    long nOffsetX = aOffset.getX();
    long nOffsetY = aOffset.getY();
    long nOffsetBtnX = LogicToPixel( Point( 6, 1 ), MAP_APPFONT ).getX();

    const long nMinCtrlWidth = LogicToPixel( ::Point( 32, 1 ), MAP_APPFONT ).getX();
    const long nTextIndent = LogicToPixel( ::Point( RSC_SP_CHK_TEXTINDENT, 1 ), MAP_APPFONT ).getX();

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

    // advance automatically after ... seconds
    long nItemWidth = maRB_ADVANCE_AUTO.CalcMinimumSize().getWidth();

    if( (nItemWidth + nMinCtrlWidth + nOffsetX) <= nMaxWidth )
    {
        long nBase = aUpperLeft.getY();

        // place controls side by side
        aCtrlSize = maRB_ADVANCE_AUTO.GetSizePixel();
        aUpperLeft.setY( nBase - aCtrlSize.getHeight());
        aCtrlSize.setWidth( nItemWidth );
        maRB_ADVANCE_AUTO.SetPosSizePixel( aUpperLeft, aCtrlSize );

        aCtrlSize = maMF_ADVANCE_AUTO_AFTER.GetSizePixel();
        aUpperLeft.setY( nBase - aCtrlSize.getHeight() );
        aUpperLeft.setX( aUpperLeft.getX() + nItemWidth + nOffsetX );
        aCtrlSize.setWidth( nMinCtrlWidth );
        maMF_ADVANCE_AUTO_AFTER.SetPosSizePixel( aUpperLeft, aCtrlSize );
        aUpperLeft.setX( nOffsetX );
    }
    else
    {
        // place controls on top of each other
        aCtrlSize = maMF_ADVANCE_AUTO_AFTER.GetSizePixel();
        aUpperLeft.setX( nOffsetX + nTextIndent );
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
        aCtrlSize.setWidth( nMinCtrlWidth );
        maMF_ADVANCE_AUTO_AFTER.SetPosSizePixel( aUpperLeft, aCtrlSize );

        aCtrlSize = maRB_ADVANCE_AUTO.GetSizePixel();
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
        aUpperLeft.setX( nOffsetX );
        aCtrlSize.setWidth( nItemWidth );
        maRB_ADVANCE_AUTO.SetPosSizePixel( aUpperLeft, aCtrlSize );
        aUpperLeft.setX( nOffsetX );
    }

    // check box "On mouse click"
    aCtrlSize = maRB_ADVANCE_ON_MOUSE.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    maRB_ADVANCE_ON_MOUSE.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // fixed line "Advance slide"
    aCtrlSize = maFL_ADVANCE_SLIDE.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    maFL_ADVANCE_SLIDE.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // check box "Loop until next sound"
    long nFTSpeedWidth = maFT_SPEED.CalcMinimumSize().getWidth() + 2 * nOffsetX;
    long nFTSoundWidth = maFT_SOUND.CalcMinimumSize().getWidth() + 2 * nOffsetX;
    long nIndent = ::std::max( nFTSoundWidth, nFTSpeedWidth );

    bool bStack = ( (nIndent + nMinCtrlWidth + nOffsetX) > nMaxWidth );

    if( bStack )
        nIndent = nTextIndent;

    aCtrlSize = maCB_LOOP_SOUND.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth - nIndent );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    aUpperLeft.setX( nIndent );
    maCB_LOOP_SOUND.SetPosSizePixel( aUpperLeft, aCtrlSize );

    aCtrlSize = maLB_SOUND.GetSizePixel();
    aCtrlSize.setWidth( ::std::max( nMaxWidth - nIndent, nMinCtrlWidth ) );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    maLB_SOUND.SetPosSizePixel( aUpperLeft, aCtrlSize );
    maLB_SOUND.SetDropDownLineCount( 8 );
    aUpperLeft.setX( nOffsetX );

    aCtrlSize = maFT_SOUND.GetSizePixel();
    if( bStack )
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
    aCtrlSize.setWidth( nFTSoundWidth );
    maFT_SOUND.SetPosSizePixel( aUpperLeft, aCtrlSize );

    aUpperLeft.setX( nIndent );
    aCtrlSize = maLB_SPEED.GetSizePixel();
    aCtrlSize.setWidth( ::std::max( nMaxWidth - nIndent, nMinCtrlWidth ) );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    maLB_SPEED.SetPosSizePixel( aUpperLeft, aCtrlSize );
    maLB_SPEED.SetDropDownLineCount( 3 );
    aUpperLeft.setX( nOffsetX );

    aCtrlSize = maFT_SPEED.GetSizePixel();
    if( bStack )
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
    aCtrlSize.setWidth( nFTSpeedWidth );
    maFT_SPEED.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // fixed line "Modify Transition"
    aCtrlSize = maFL_MODIFY_TRANSITION.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    maFL_MODIFY_TRANSITION.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // fixed line "Apply to selected slides"
    aCtrlSize = maFL_APPLY_TRANSITION.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth );
    ::Point aUpperLeftCorner( nOffsetX, nOffsetY );
    maFL_APPLY_TRANSITION.SetPosSizePixel( aUpperLeftCorner, aCtrlSize );
    aUpperLeftCorner.setY( aUpperLeftCorner.getY() + aCtrlSize.getHeight() + nOffsetY );
    aUpperLeft.setY( aUpperLeft.getY() - nOffsetY );

    // list box slide transitions
    aCtrlSize.setWidth( nMaxWidth );
    aCtrlSize.setHeight( aUpperLeft.getY() - aUpperLeftCorner.getY() );
    maLB_SLIDE_TRANSITIONS.SetPosSizePixel( aUpperLeftCorner, aCtrlSize );
}

void SlideTransitionPane::updateControls()
{
    ::std::vector< SdPage * > aSelectedPages( getSelectedPages());
    if( aSelectedPages.empty())
    {
        mbHasSelection = false;
        return;
    }
    mbHasSelection = true;

    DBG_ASSERT( ! mbUpdatingControls, "Multiple Control Updates" );
    mbUpdatingControls = true;

    // get model data for first page
    SdPage * pFirstPage = aSelectedPages.front();
    DBG_ASSERT( pFirstPage, "Invalid Page" );

    impl::TransitionEffect aEffect( *pFirstPage );

    // merge with other pages
    ::std::vector< SdPage * >::const_iterator aIt( aSelectedPages.begin());
    const ::std::vector< SdPage * >::const_iterator aEndIt( aSelectedPages.end());

    // start with second page (note aIt != aEndIt, because ! aSelectedPages.empty())
    for( ++aIt ;aIt != aEndIt; ++aIt )
    {
        if( *aIt )
            aEffect.compareWith( *(*aIt) );
    }

    // detect current slide effect
    if( aEffect.mbEffectAmbiguous )
        maLB_SLIDE_TRANSITIONS.SetNoSelection();
    else
    {
        // ToDo: That 0 is "no transition" is documented nowhere except in the
        // CTOR of sdpage
        if( aEffect.mnType == 0 )
            maLB_SLIDE_TRANSITIONS.SelectEntryPos( 0 );
        else
        {
            sal_uInt16 nEntry = lcl_getTransitionEffectIndex( mpDrawDoc, aEffect );
            if( nEntry == LISTBOX_ENTRY_NOTFOUND )
                maLB_SLIDE_TRANSITIONS.SetNoSelection();
            else
            {
                // first entry in list is "none", so add 1 after translation
                if( m_aPresetIndexes.find( nEntry ) != m_aPresetIndexes.end())
                    maLB_SLIDE_TRANSITIONS.SelectEntryPos( m_aPresetIndexes[ nEntry ] + 1 );
                else
                    maLB_SLIDE_TRANSITIONS.SetNoSelection();
            }
        }
    }

    if( aEffect.mbDurationAmbiguous )
        maLB_SPEED.SetNoSelection();
    else
        maLB_SPEED.SelectEntryPos(
            (aEffect.mfDuration > 2.0 )
            ? 0 : (aEffect.mfDuration < 2.0)
            ? 1 : 2 );       // else FADE_SPEED_FAST

    if( aEffect.mbSoundOnAmbiguous ||
        aEffect.mbSoundAmbiguous )
    {
        maLB_SOUND.SetNoSelection();
        maCurrentSoundFile.Erase();
    }
    else
    {
        if( aEffect.mbSoundOn &&
            aEffect.maSound.Len() > 0 )
        {
            tSoundListType::size_type nPos = 0;
            if( lcl_findSoundInList( maSoundList, aEffect.maSound, nPos ))
            {
                // skip first three entries
                maLB_SOUND.SelectEntryPos( nPos + 3 );
                maCurrentSoundFile = aEffect.maSound;
            }
            else
            {
                maCurrentSoundFile.Erase();
            }
        }
        else
        {
            maLB_SOUND.SelectEntryPos( 0 );
            maCurrentSoundFile.Erase();
        }
    }

    if( aEffect.mbPresChangeAmbiguous )
    {
        maRB_ADVANCE_ON_MOUSE.Check( FALSE );
        maRB_ADVANCE_AUTO.Check( FALSE );
    }
    else
    {
        maRB_ADVANCE_ON_MOUSE.Check( aEffect.mePresChange == PRESCHANGE_MANUAL );
        maRB_ADVANCE_AUTO.Check( aEffect.mePresChange == PRESCHANGE_AUTO );
        maMF_ADVANCE_AUTO_AFTER.SetValue( aEffect.mnTime );
    }

    SdOptions* pOptions = SD_MOD()->GetSdOptions(DOCUMENT_TYPE_IMPRESS);
    maCB_AUTO_PREVIEW.Check( pOptions->IsPreviewTransitions() == sal_True );

    mbUpdatingControls = false;

    updateControlState();
}

void SlideTransitionPane::updateControlState()
{
    maLB_SLIDE_TRANSITIONS.Enable( mbHasSelection );
    maLB_SPEED.Enable( mbHasSelection );
    maLB_SOUND.Enable( mbHasSelection );
    maCB_LOOP_SOUND.Enable( mbHasSelection && (maLB_SOUND.GetSelectEntryPos() > 2));
    maRB_ADVANCE_ON_MOUSE.Enable( mbHasSelection );
    maRB_ADVANCE_AUTO.Enable( mbHasSelection );
    maMF_ADVANCE_AUTO_AFTER.Enable( mbHasSelection && maRB_ADVANCE_AUTO.IsChecked());

    maPB_APPLY_TO_ALL.Enable( mbHasSelection );
    maPB_PLAY.Enable( mbHasSelection );
//     maPB_SLIDE_SHOW.Enable( TRUE );
    maCB_AUTO_PREVIEW.Enable( mbHasSelection );
}

void SlideTransitionPane::updateSoundList()
{
    List aSoundList;

    GalleryExplorer::FillObjList( GALLERY_THEME_SOUNDS, aSoundList );
    GalleryExplorer::FillObjList( GALLERY_THEME_USERSOUNDS, aSoundList );

    sal_uInt32 nCount = aSoundList.Count();
    maSoundList.clear();
    maSoundList.reserve( nCount );
    for( sal_uInt32 i=0; i<nCount; ++i )
    {
        String * pEntry = reinterpret_cast< String * >( aSoundList.GetObject( i ));
        if( pEntry )
        {
            // store copy of string in member list
            maSoundList.push_back( *pEntry );
            // delete pointer in temporary List
            delete pEntry;
        }
    }

    lcl_FillSoundListBox( maSoundList, maLB_SOUND );
}

void SlideTransitionPane::openSoundFileDialog()
{
    if( ! maLB_SOUND.IsEnabled())
        return;

    SdOpenSoundFileDialog aFileDialog;

    String aFile;
    DBG_ASSERT( maLB_SOUND.GetSelectEntryPos() == 2,
                "Dialog should only open when \"Other sound\" is selected" );
    aFile = SvtPathOptions().GetGraphicPath();

    aFileDialog.SetPath( aFile );

    bool bValidSoundFile( false );
    bool bQuitLoop( false );

    while( ! bQuitLoop &&
           aFileDialog.Execute() == ERRCODE_NONE )
    {
        aFile = aFileDialog.GetPath();
        tSoundListType::size_type nPos = 0;
        bValidSoundFile = lcl_findSoundInList( maSoundList, aFile, nPos );

        if( bValidSoundFile )
        {
            bQuitLoop = true;
        }
        else // not in sound list
        {
            // try to insert into gallery
            if( GalleryExplorer::InsertURL( GALLERY_THEME_USERSOUNDS, aFile, SGA_FORMAT_SOUND ) )
            {
                updateSoundList();
                bValidSoundFile = lcl_findSoundInList( maSoundList, aFile, nPos );
                DBG_ASSERT( bValidSoundFile, "Adding sound to gallery failed" );

                bQuitLoop = true;
            }
            else
            {
                String aStrWarning(SdResId(STR_WARNING_NOSOUNDFILE));
                String aStr( sal_Unicode( '%' ));
                aStrWarning.SearchAndReplace( aStr , aFile );
                WarningBox aWarningBox( NULL, WB_3DLOOK | WB_RETRY_CANCEL, aStrWarning );
                aWarningBox.SetModalInputMode (TRUE);
                bQuitLoop = (aWarningBox.Execute() != RET_RETRY);

                bValidSoundFile = false;
            }
        }

        if( bValidSoundFile )
            // skip first three entries in list
            maLB_SOUND.SelectEntryPos( nPos + 3 );
    }

    if( ! bValidSoundFile )
    {
        if( maCurrentSoundFile.Len() > 0 )
        {
            tSoundListType::size_type nPos = 0;
            if( lcl_findSoundInList( maSoundList, maCurrentSoundFile, nPos ))
                maLB_SOUND.SelectEntryPos( nPos + 3 );
            else
                maLB_SOUND.SelectEntryPos( 0 );  // NONE
        }
        else
            maLB_SOUND.SelectEntryPos( 0 );  // NONE
    }
}

impl::TransitionEffect SlideTransitionPane::getTransitionEffectFromControls() const
{
    impl::TransitionEffect aResult;
    aResult.setAllAmbiguous();

    // check first (aResult might be overwritten)
    if( maLB_SLIDE_TRANSITIONS.IsEnabled() &&
        maLB_SLIDE_TRANSITIONS.GetSelectEntryCount() > 0 )
    {
        TransitionPresetPtr pPreset = lcl_getTransitionPresetByUIName(
            mpDrawDoc, OUString( maLB_SLIDE_TRANSITIONS.GetSelectEntry()));

        if( pPreset.get())
        {
            aResult = impl::TransitionEffect( *pPreset );
            aResult.setAllAmbiguous();
        }
        else
        {
            aResult.mnType = 0;
        }
        aResult.mbEffectAmbiguous = false;
    }

    // speed
    if( maLB_SPEED.IsEnabled() &&
        maLB_SPEED.GetSelectEntryCount() > 0 )
    {
        sal_uInt16 nPos = maLB_SPEED.GetSelectEntryPos();
        aResult.mfDuration = (nPos == 0)
            ? 3.0
            : (nPos == 1)
            ? 2.0
            : 1.0;   // nPos == 2
        DBG_ASSERT( aResult.mfDuration != 1.0 || nPos == 2, "Invalid Listbox Entry" );

        aResult.mbDurationAmbiguous = false;
    }

    // slide-advance mode
    if( maRB_ADVANCE_ON_MOUSE.IsEnabled() && maRB_ADVANCE_AUTO.IsEnabled() &&
        (maRB_ADVANCE_ON_MOUSE.IsChecked() || maRB_ADVANCE_AUTO.IsChecked()))
    {
        if( maRB_ADVANCE_ON_MOUSE.IsChecked())
            aResult.mePresChange = PRESCHANGE_MANUAL;
        else
        {
            aResult.mePresChange = PRESCHANGE_AUTO;
            if( maMF_ADVANCE_AUTO_AFTER.IsEnabled())
            {
//                 sal_uInt16 nDigits = maMF_ADVANCE_AUTO_AFTER.GetDecimalDigits();
                aResult.mnTime = maMF_ADVANCE_AUTO_AFTER.GetValue();
                // / static_cast< sal_uInt16 >( pow( 10.0, static_cast< double >( nDigits )));
                aResult.mbTimeAmbiguous = false;
            }
        }

        aResult.mbPresChangeAmbiguous = false;
    }

    // sound
    if( maLB_SOUND.IsEnabled())
    {
        if( maLB_SOUND.GetSelectEntryCount() > 0 )
        {
            sal_uInt16 nPos = maLB_SOUND.GetSelectEntryPos();
            aResult.mbSoundOn = (nPos != 0);
            aResult.maSound = lcl_getSoundFileURL( maSoundList, maLB_SOUND );
            aResult.mbSoundOnAmbiguous = false;
            aResult.mbSoundAmbiguous = false;
            maCurrentSoundFile = aResult.maSound;
        }
        else
        {
            maCurrentSoundFile.Erase();
        }
    }

    return aResult;
}

void SlideTransitionPane::applyToSelectedPages()
{
    if( ! mbUpdatingControls )
    {
        ::std::vector< SdPage * > aSelectedPages( getSelectedPages());
        if( ! aSelectedPages.empty())
        {
            lcl_CreateUndoForPages( aSelectedPages, mrBase );
            lcl_ApplyToPages( aSelectedPages, getTransitionEffectFromControls() );
            mrBase.GetDocShell()->SetModified();
        }
        if( maCB_AUTO_PREVIEW.IsEnabled() &&
            maCB_AUTO_PREVIEW.IsChecked())
        {
            playCurrentEffect();
        }
    }
}

void SlideTransitionPane::playCurrentEffect()
{
    if( mxView.is() )
    {
        DrawViewShell* pViewShell = dynamic_cast< DrawViewShell* >( mrBase.GetPaneManager().GetViewShell() );
        if( pViewShell == 0 )
            return;

        DrawView* pView = pViewShell->GetDrawView();

        pViewShell->SetSlideShow( 0 );
        std::auto_ptr<Slideshow> pSlideshow(
            new Slideshow( pViewShell, pView, pViewShell->GetDoc() ) );
        Reference< ::com::sun::star::animations::XAnimationNode > xNode;
        if (pSlideshow->startPreview( mxView->getCurrentPage(), xNode ))
            pViewShell->SetSlideShow( pSlideshow.release() );
    }
}

void SlideTransitionPane::addListener()
{
    Link aLink( LINK(this,SlideTransitionPane,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer().AddEventListener (
        aLink,
        tools::EventMultiplexer::ET_CURRENT_PAGE
        | tools::EventMultiplexer::ET_EDIT_VIEW_SELECTION
        | tools::EventMultiplexer::ET_MAIN_VIEW);
}

void SlideTransitionPane::removeListener()
{
    Link aLink( LINK(this,SlideTransitionPane,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer().RemoveEventListener( aLink );
}

IMPL_LINK(SlideTransitionPane,EventMultiplexerListener,
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
            // At this moment the controller may not yet been set at model
            // or ViewShellBase.  Take it from the view shell passed with
            // the event.
            if (mrBase.GetMainViewShell() != NULL)
            {
                mxView = Reference<drawing::XDrawView>::query(
                    static_cast<drawing::XDrawView*>(mrBase.GetMainViewShell()->GetController()));
                onSelectionChanged();
                onChangeCurrentPage();
            }
            break;

        default:
            break;
    }
    return 0;
}

IMPL_LINK( SlideTransitionPane, ApplyToAllButtonClicked, void *, EMPTYARG )
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

    if( ! aPages.empty())
    {
        lcl_CreateUndoForPages( aPages, mrBase );
        lcl_ApplyToPages( aPages, getTransitionEffectFromControls() );
    }

    return 0;
}


IMPL_LINK( SlideTransitionPane, PlayButtonClicked, void *, EMPTYARG )
{
    playCurrentEffect();
    return 0;
}

IMPL_LINK( SlideTransitionPane, SlideShowButtonClicked, void *, EMPTYARG )
{
    mrBase.StartPresentation();
    return 0;
}

IMPL_LINK( SlideTransitionPane, TransitionSelected, void *, EMPTYARG )
{
    applyToSelectedPages();
    return 0;
}

IMPL_LINK( SlideTransitionPane, AdvanceSlideRadioButtonToggled, void *, EMPTYARG )
{
    updateControlState();
    applyToSelectedPages();
    return 0;
}

IMPL_LINK( SlideTransitionPane, AdvanceTimeModified, void *, EMPTYARG )
{
    applyToSelectedPages();
    return 0;
}

IMPL_LINK( SlideTransitionPane, SpeedListBoxSelected, void *, EMPTYARG )
{
    applyToSelectedPages();
    return 0;
}

IMPL_LINK( SlideTransitionPane, SoundListBoxSelected, void *, EMPTYARG )
{
    if( maLB_SOUND.GetSelectEntryCount() )
    {
        sal_uInt16 nPos = maLB_SOUND.GetSelectEntryPos();
        if( nPos == 2 )
        {
            // other sound ...
            openSoundFileDialog();
        }
    }
    updateControlState();
    applyToSelectedPages();
    return 0;
}

IMPL_LINK( SlideTransitionPane, LoopSoundBoxChecked, void *, EMPTYARG )
{
    applyToSelectedPages();
    return 0;
}

IMPL_LINK( SlideTransitionPane, AutoPreviewClicked, void *, EMPTYARG )
{
    SdOptions* pOptions = SD_MOD()->GetSdOptions(DOCUMENT_TYPE_IMPRESS);
    pOptions->SetPreviewTransitions( maCB_AUTO_PREVIEW.IsChecked() ? sal_True : sal_False );
    return 0;
}

::Window * createSlideTransitionPanel( ::Window* pParent, ViewShellBase& rBase )
{
    DialogListBox* pWindow = 0;

    DrawDocShell* pDocSh = rBase.GetDocShell();
    if( pDocSh )
    {
        pWindow = new DialogListBox( pParent, WB_CLIPCHILDREN|WB_TABSTOP|WB_AUTOHSCROLL );

        Size aMinSize( pWindow->LogicToPixel( Size( 72, 216 ), MAP_APPFONT ) );
        ::Window* pPaneWindow = new SlideTransitionPane( pWindow, rBase, aMinSize, pDocSh->GetDoc() );
        pWindow->SetChildWindow( pPaneWindow, aMinSize );
        pWindow->SetText( pPaneWindow->GetText() );
    }

    return pWindow;
}


} //  namespace sd
