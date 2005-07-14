/*************************************************************************
 *
 *  $RCSfile: CustomAnimationPane.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:44:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _COM_SUN_STAR_ANIMATIONS_XANIMATIONNODESUPPLIER_HPP_
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWVIEW_HPP_
#include <com/sun/star/drawing/XDrawView.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_EFFECTNODETYPE_HPP_
#include <com/sun/star/presentation/EffectNodeType.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_EFFECTCOMMANDS_HPP_
#include <com/sun/star/presentation/EffectCommands.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_ANIMATIONTRANSFORMTYPE_HPP_
#include <com/sun/star/animations/AnimationTransformType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGECOMPARE_HPP_
#include <com/sun/star/text/XTextRangeCompare.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_PARAGRAPHTARGET_HPP_
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif

#ifndef _SD_STLPROPERTYSET_HXX
#include "STLPropertySet.hxx"
#endif

#ifndef _SD_CUSTOMANIMATIONPANE_HXX
#include "CustomAnimationPane.hxx"
#endif
#ifndef _SD_CUSTOMANIMATIONDIALOG_HXX
#include "CustomAnimationDialog.hxx"
#endif
#ifndef _SD_CUSTOMANIMATIONCREATEDIALOG_HXX
#include "CustomAnimationCreateDialog.hxx"
#endif
#ifndef _SD_CUSTOMANIMATIONPANE_HRC
#include "CustomAnimationPane.hrc"
#endif
#ifndef _SD_CUSTOMANIMATION_HRC
#include "CustomAnimation.hrc"
#endif
#ifndef _SD_CUSTOMANIMATIONLIST_HXX
#include "CustomAnimationList.hxx"
#endif

#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif

#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif

#ifndef SD_DRAW_DOC_SHELL_HXX
#include <DrawDocShell.hxx>
#endif

#ifndef SD_VIEW_SHELL_BASE_HXX
#include <ViewShellBase.hxx>
#endif

#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif

#ifndef SD_DRAW_CONTROLLER_HXX
#include "DrawController.hxx"
#endif

#ifndef SD_RESID_HXX
#include "sdresid.hxx"
#endif

#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif

#ifndef _SD_SLIDESHOW_HXX
#include "slideshow.hxx"
#endif

#ifndef SD_PANE_MANAGER_HXX
#include "PaneManager.hxx"
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

#include "EventMultiplexer.hxx"
#include "DialogListBox.hxx"

#include "glob.hrc"
#include "sdpage.hxx"
#include "drawdoc.hxx"

#include <memory>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::text;

using ::rtl::OUString;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::view::XSelectionSupplier;
using ::com::sun::star::view::XSelectionChangeListener;
using ::com::sun::star::frame::XController;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertyChangeListener;
using ::com::sun::star::drawing::XDrawView;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::drawing::XShapes;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::text::XText;

namespace sd {

// --------------------------------------------------------------------

void fillDurationComboBox( ComboBox* pBox )
{
    static const double gdVerySlow = 5.0;
    static const double gdSlow = 3.0;
    static const double gdNormal = 2.0;
    static const double gdFast = 1.0;
    static const double gdVeryFast = 0.5;

    String aVerySlow( SdResId( STR_CUSTOMANIMATION_DURATION_VERY_SLOW ) );
    pBox->SetEntryData( pBox->InsertEntry( aVerySlow ), (void*)&gdVerySlow );

    String aSlow( SdResId( STR_CUSTOMANIMATION_DURATION_SLOW ) );
    pBox->SetEntryData( pBox->InsertEntry( aSlow ), (void*)&gdSlow );

    String aNormal( SdResId( STR_CUSTOMANIMATION_DURATION_NORMAL ) );
    pBox->SetEntryData( pBox->InsertEntry( aNormal ), (void*)&gdNormal );

    String aFast( SdResId( STR_CUSTOMANIMATION_DURATION_FAST ) );
    pBox->SetEntryData( pBox->InsertEntry( aFast ), (void*)&gdFast );

    String aVeryFast( SdResId( STR_CUSTOMANIMATION_DURATION_VERY_FAST ) );
    pBox->SetEntryData( pBox->InsertEntry( aVeryFast ), (void*)&gdVeryFast );
}

void fillRepeatComboBox( ComboBox* pBox )
{
    String aNone( SdResId( STR_CUSTOMANIMATION_REPEAT_NONE ) );
    pBox->SetEntryData( pBox->InsertEntry( aNone ), (void*)((sal_Int32)0) );

    pBox->SetEntryData( pBox->InsertEntry( String::CreateFromInt32( 2 ) ), (void*)((sal_Int32)1) );
    pBox->SetEntryData( pBox->InsertEntry( String::CreateFromInt32( 3 ) ), (void*)((sal_Int32)3) );
    pBox->SetEntryData( pBox->InsertEntry( String::CreateFromInt32( 4 ) ), (void*)((sal_Int32)4) );
    pBox->SetEntryData( pBox->InsertEntry( String::CreateFromInt32( 5 ) ), (void*)((sal_Int32)5) );
    pBox->SetEntryData( pBox->InsertEntry( String::CreateFromInt32( 10 ) ), (void*)((sal_Int32)10) );

    String aUntilClick( SdResId( STR_CUSTOMANIMATION_REPEAT_UNTIL_NEXT_CLICK ) );
    pBox->SetEntryData( pBox->InsertEntry( aUntilClick ), (void*)((sal_Int32)-1) );

    String aEndOfSlide( SdResId( STR_CUSTOMANIMATION_REPEAT_UNTIL_END_OF_SLIDE ) );
    pBox->SetEntryData( pBox->InsertEntry( aEndOfSlide ), (void*)((sal_Int32)-2) );
}

// --------------------------------------------------------------------

CustomAnimationPane::CustomAnimationPane( ::Window* pParent, ViewShellBase& rBase, const Size& rMinSize )
:   Control( pParent, SdResId(DLG_CUSTOMANIMATIONPANE) ),
    mrBase( rBase ),
    mxModel( rBase.GetDocShell()->GetDoc()->getUnoModel(), UNO_QUERY ),
    mrPresets( CustomAnimationPresets::getCustomAnimationPresets() ),
    mnPropertyType( nPropertyTypeNone ),
    maMinSize( rMinSize )
{
    // load resources
    mpFLEffect = new FixedLine( this, SdResId( FL_EFFECT ) );

    mpPBAddEffect = new PushButton( this, SdResId( PB_ADD_EFFECT ) );
    mpPBChangeEffect = new PushButton( this, SdResId( PB_CHANGE_EFFECT ) );
    mpPBRemoveEffect = new PushButton( this, SdResId( PB_REMOVE_EFFECT ) );

    mpFLModify = new FixedLine( this, SdResId( FL_MODIFY ) );

    mpFTStart = new FixedText( this, SdResId( FT_START ) );
    mpLBStart = new ListBox( this, SdResId( LB_START ) );
    mpFTProperty = new FixedText( this, SdResId( FT_PROPERTY ) );
    mpLBProperty = new PropertyControl( this, SdResId( LB_PROPERTY ) );
    mpPBPropertyMore = new PushButton( this, SdResId( PB_PROPERTY_MORE ) );

    mpFTSpeed = new FixedText( this, SdResId( FT_SPEED ) );
    mpCBSpeed = new ComboBox( this, SdResId( CB_SPEED ) );

    mpCustomAnimationList = new CustomAnimationList( this, SdResId( CT_CUSTOM_ANIMATION_LIST ), this );

    mpPBMoveUp = new PushButton( this, SdResId( PB_MOVE_UP ) );
    mpPBMoveDown = new PushButton( this, SdResId( PB_MOVE_DOWN ) );
    mpFTChangeOrder = new FixedText( this, SdResId( FT_CHANGE_ORDER ) );
    mpFLSeperator1 = new FixedLine( this, SdResId( FL_SEPERATOR1 ) );
    mpPBPlay = new PushButton( this, SdResId( PB_PLAY ) );
    mpPBSlideShow = new PushButton( this, SdResId( PB_SLIDE_SHOW ) );
    mpFLSeperator2 = new FixedLine( this, SdResId( FL_SEPERATOR2 ) );
    mpCBAutoPreview = new CheckBox( this, SdResId( CB_AUTOPREVIEW ) );

    maStrProperty = mpFTProperty->GetText();

    FreeResource();

    // use bold font for group headings (same font for all fixed lines):
    Font font( mpFLEffect->GetFont() );
    font.SetWeight( WEIGHT_BOLD );
    mpFLEffect->SetFont( font );
    mpFLModify->SetFont( font );

    fillDurationComboBox( mpCBSpeed );
    mpPBMoveUp->SetSymbol( SYMBOL_ARROW_UP );
    mpPBMoveDown->SetSymbol( SYMBOL_ARROW_DOWN );

    mpPBAddEffect->SetClickHdl( LINK( this, CustomAnimationPane, implControlHdl ) );
    mpPBChangeEffect->SetClickHdl( LINK( this, CustomAnimationPane, implControlHdl ) );
    mpPBRemoveEffect->SetClickHdl( LINK( this, CustomAnimationPane, implControlHdl ) );
    mpLBStart->SetSelectHdl( LINK( this, CustomAnimationPane, implControlHdl ) );
    mpCBSpeed->SetSelectHdl( LINK( this, CustomAnimationPane, implControlHdl ) );
    mpPBPropertyMore->SetClickHdl( LINK( this, CustomAnimationPane, implControlHdl ) );
    mpPBMoveUp->SetClickHdl( LINK( this, CustomAnimationPane, implControlHdl ) );
    mpPBMoveDown->SetClickHdl( LINK( this, CustomAnimationPane, implControlHdl ) );
    mpPBPlay->SetClickHdl( LINK( this, CustomAnimationPane, implControlHdl ) );
    mpPBSlideShow->SetClickHdl( LINK( this, CustomAnimationPane, implControlHdl ) );
    mpCBAutoPreview->SetClickHdl( LINK( this, CustomAnimationPane, implControlHdl ) );

    maStrModify = mpFLEffect->GetText();

    // resize controls according to current size
    updateLayout();

    // get current controller and initialize listeners
    try
    {
        mxView = Reference< XDrawView >::query( static_cast<drawing::XDrawView*>(mrBase.GetMainViewShell()->GetController()));
/*      mxView = Reference< XDrawView >::query( mxModel->getCurrentController() );*/
        addListener();
    }
    catch( Exception& e )
    {
        (void)e;
        DBG_ERROR( "sd::CustomAnimationPane::CustomAnimationPane(), Exception cought!" );
    }

    // get current page and update custom animation list
    onChangeCurrentPage();

    // update selection and control states
    onSelectionChanged();
}

CustomAnimationPane::~CustomAnimationPane()
{
    removeListener();

    delete mpFLModify;
    delete mpPBAddEffect;
    delete mpPBChangeEffect;
    delete mpPBRemoveEffect;
    delete mpFLEffect;
    delete mpFTStart;
    delete mpLBStart;
    delete mpFTProperty;
    delete mpLBProperty;
    delete mpPBPropertyMore;
    delete mpFTSpeed;
    delete mpCBSpeed;
    delete mpCustomAnimationList;
    delete mpFTChangeOrder;
    delete mpPBMoveUp;
    delete mpPBMoveDown;
    delete mpFLSeperator1;
    delete mpPBPlay;
    delete mpPBSlideShow;
    delete mpFLSeperator2;
    delete mpCBAutoPreview;
}

void CustomAnimationPane::addUndo()
{
    SfxUndoManager* pManager = mrBase.GetDocShell()->GetUndoManager();
    if( pManager )
    {
        SdPage* pPage = SdPage::getImplementation( mxCurrentPage );
        if( pPage )
            pManager->AddUndoAction( new UndoAnimation( mrBase.GetDocShell()->GetDoc(), pPage ) );
    }
}

void CustomAnimationPane::Resize()
{
    updateLayout();
}

void CustomAnimationPane::addListener()
{
    Link aLink( LINK(this,CustomAnimationPane,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer().AddEventListener (
        aLink,
        tools::EventMultiplexer::ET_DISPOSING
        | tools::EventMultiplexer::ET_CURRENT_PAGE
        | tools::EventMultiplexer::ET_EDIT_VIEW_SELECTION
        | tools::EventMultiplexer::ET_MAIN_VIEW
        | tools::EventMultiplexer::ET_TEXT_EDIT);
}

void CustomAnimationPane::removeListener()
{
    Link aLink( LINK(this,CustomAnimationPane,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer().RemoveEventListener( aLink );
}

IMPL_LINK(CustomAnimationPane,EventMultiplexerListener,
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
            mxView = Reference<XDrawView>();
            break;

        case tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED:
            // At this moment the controller may not yet been set at model
            // or ViewShellBase.  Take it from the view shell passed with
            // the event.
            if (mrBase.GetMainViewShell() != NULL)
            {
                mxView = Reference< XDrawView >::query(
                    static_cast<drawing::XDrawView*>(mrBase.GetMainViewShell()->GetController()));
                onSelectionChanged();
                onChangeCurrentPage();
            }
            break;

        case tools::EventMultiplexerEvent::EID_DISPOSING:
            mxView = Reference<XDrawView>();
            onSelectionChanged();
            onChangeCurrentPage();
            break;
        case tools::EventMultiplexerEvent::EID_END_TEXT_EDIT:
            if( mpMainSequence.get() && pEvent->mpUserData )
                mpCustomAnimationList->update( mpMainSequence );
            break;
    }
    return 0;
}


void CustomAnimationPane::updateLayout()
{
    Size aPaneSize( GetSizePixel() );
    if( aPaneSize.Width() < maMinSize.Width() )
        aPaneSize.Width() = maMinSize.Width();

    if( aPaneSize.Height() < maMinSize.Height() )
        aPaneSize.Height() = maMinSize.Height();

    Point aOffset( LogicToPixel( Point(3,3), MAP_APPFONT ) );
    Point aCursor( aOffset );

    // place the modify fixed line

    // place the "modify effect" fixed line
    Size aSize( mpFLModify->GetSizePixel() );
    aSize.Width() = aPaneSize.Width() - 2 * aOffset.X();

    mpFLModify->SetPosSizePixel( aCursor, aSize );

    aCursor.Y() += aSize.Height() + aOffset.Y();

    const int nButtonExtraWidth = 4 * aOffset.X();

    // the "add effect" button is placed top-left
    Size aCtrlSize( mpPBAddEffect->GetSizePixel() );
    aCtrlSize.setWidth( mpPBAddEffect->CalcMinimumSize( aSize.Width() ).getWidth() + nButtonExtraWidth );
    mpPBAddEffect->SetPosSizePixel( aCursor, aCtrlSize );

    aCursor.X() += aOffset.X() + aCtrlSize.Width();

    // place the "change effect" button

    // if the "change" button does not fit right of the "add effect", put it on the next line
    aCtrlSize = mpPBChangeEffect->GetSizePixel();
    aCtrlSize.setWidth( mpPBChangeEffect->CalcMinimumSize( aSize.Width() ).getWidth() + nButtonExtraWidth );
    if( ( aCursor.X() + aCtrlSize.Width() + aOffset.X() ) > aPaneSize.Width() )
    {
        aCursor.X() = aOffset.X();
        aCursor.Y() += aCtrlSize.Height() + aOffset.Y();
    }
    mpPBChangeEffect->SetPosSizePixel( aCursor, aCtrlSize );

    aCursor.X() += aOffset.X() + aCtrlSize.Width();

    // place the "remove effect" button

    // if the "remove" button does not fit right of the "add effect", put it on the next line
    aCtrlSize = mpPBRemoveEffect->GetSizePixel();
    aCtrlSize.setWidth( mpPBRemoveEffect->CalcMinimumSize( aSize.Width() ).getWidth() + nButtonExtraWidth );
    if( ( aCursor.X() + aCtrlSize.Width() + aOffset.X() ) > aPaneSize.Width() )
    {
        aCursor.X() = aOffset.X();
        aCursor.Y() += aCtrlSize.Height() + aOffset.Y();
    }

    mpPBRemoveEffect->SetPosSizePixel( aCursor, aCtrlSize );

    aCursor.X() = aOffset.X();
    aCursor.Y() += aCtrlSize.Height() + 2 * aOffset.Y();

    // place the "modify effect" fixed line
    aSize = mpFLEffect->GetSizePixel();
    aSize.Width() = aPaneSize.Width() - 2 * aOffset.X();

    mpFLEffect->SetPosSizePixel( aCursor, aSize );

    aCursor.Y() += aSize.Height() + aOffset.Y();

    // ---------------------------------------------------------------------------
    // place the properties controls

    // calc minimum width for fixedtext

    Size aFixedTextSize( mpFTStart->CalcMinimumSize() );
    long nWidth = aFixedTextSize.Width();
    aFixedTextSize = mpFTProperty->CalcMinimumSize();
    nWidth = std::max( nWidth, aFixedTextSize.Width() );
    aFixedTextSize = mpFTSpeed->CalcMinimumSize();
    aFixedTextSize.Width() = std::max( nWidth, aFixedTextSize.Width() ) + aOffset.X();
    mpFTStart->SetSizePixel(aFixedTextSize);
    mpFTProperty->SetSizePixel(aFixedTextSize);
    mpFTSpeed->SetSizePixel(aFixedTextSize);

    aSize = mpPBPropertyMore->GetSizePixel();

    // place the "start" fixed text

    Point aFTPos( aCursor );
    Point aLBPos( aCursor );
    Size aListBoxSize( LogicToPixel( Size( 60, 12 ), MAP_APPFONT ) );
    long nDeltaY = aListBoxSize.Height() + aOffset.Y();

    // linebreak?
    if( (aFixedTextSize.Width() + aListBoxSize.Width() + aSize.Width() + 4 * aOffset.X()) > aPaneSize.Width() )
    {
        // y position for list box is below fixed text
        aLBPos.Y() += aFixedTextSize.Height() + aOffset.Y();

        // height of fixed text + list box + something = 2 * list box
        nDeltaY = aListBoxSize.Height() +  aFixedTextSize.Height() + 2*aOffset.Y();
    }
    else
    {
        // x position for list box is right of fixed text
        aLBPos.X() += aFixedTextSize.Width() + aOffset.X();

        if( aListBoxSize.Height() > aFixedTextSize.Height() )
            aFTPos.Y() = aLBPos.Y() + ((aListBoxSize.Height() - aFixedTextSize.Height()) >> 1);
        else
            aLBPos.Y() = aFTPos.Y() + ((aFixedTextSize.Height() - aListBoxSize.Height()) >> 1);
    }

    // width of the listbox is from its left side until end of pane
    aListBoxSize.Width() = aPaneSize.Width() - aLBPos.X() - aSize.Width() - 2 * aOffset.X();

    mpFTStart->SetPosPixel( aFTPos );
    mpLBStart->SetPosSizePixel( aLBPos, aListBoxSize );

    aFTPos.Y() += nDeltaY; aLBPos.Y() += nDeltaY;

    mpFTProperty->SetPosPixel( aFTPos );
    mpLBProperty->SetPosSizePixel( aLBPos, aListBoxSize );
    mpLBProperty->Resize();

    Point aMorePos( aLBPos );
    aMorePos.X() += aListBoxSize.Width() + aOffset.X();
    mpPBPropertyMore->SetPosPixel( aMorePos );

    aFTPos.Y() += nDeltaY; aLBPos.Y() += nDeltaY;

    mpFTSpeed->SetPosPixel( aFTPos );
    mpCBSpeed->SetPosSizePixel( aLBPos, aListBoxSize );

    aFTPos.Y() += nDeltaY + aOffset.Y();

    Point aListPos( aFTPos );

    // positionate the buttons on the bottom

    // place the auto preview checkbox
    aCursor = Point( aOffset.X(), aPaneSize.Height() - mpCBAutoPreview->GetSizePixel().Height() - aOffset.Y() );
    mpCBAutoPreview->SetPosPixel( aCursor );

    // place the seperator 2 fixed line
    aCursor.Y() -= /* aOffset.Y() + */ mpFLSeperator2->GetSizePixel().Height();
    aSize = mpFLSeperator2->GetSizePixel();
    aSize.Width() = aPaneSize.Width() - 2 * aOffset.X();
    mpFLSeperator2->SetPosSizePixel( aCursor, aSize );

    // next, layout and place the play and slide show buttons
    aCtrlSize = mpPBSlideShow->GetSizePixel();
    aCtrlSize.setWidth( mpPBSlideShow->CalcMinimumSize( aSize.Width() ).getWidth() + nButtonExtraWidth );

    Size aPlaySize( mpPBPlay->GetSizePixel() );
    aPlaySize.setWidth( mpPBPlay->CalcMinimumSize( aSize.Width() ).getWidth() + nButtonExtraWidth );

    aCursor.Y() -= aCtrlSize.Height() /* + aOffset.Y() */;

    // do we need two lines for the buttons?
    int aTestWidth = aCursor.X() + mpPBPlay->GetSizePixel().Width() + 2 * aOffset.X() + mpPBSlideShow->GetSizePixel().Width();
    if( aTestWidth > aPaneSize.Width() )
    {
        mpPBSlideShow->SetPosSizePixel( aCursor, aCtrlSize );
        aCursor.Y() -= aCtrlSize.Height() + aOffset.Y();
        mpPBPlay->SetPosSizePixel( aCursor, aPlaySize );
    }
    else
    {
        mpPBPlay->SetPosSizePixel( aCursor, aPlaySize );
        aCursor.X() += aPlaySize.Width() + aOffset.X();
        mpPBSlideShow->SetPosSizePixel( aCursor, aCtrlSize );
    }

    // place the seperator 1 fixed line
    aCursor.X() = aOffset.X();
    aCursor.Y() -= /* aOffset.Y() + */ mpFLSeperator1->GetSizePixel().Height();
    aSize = mpFLSeperator1->GetSizePixel();
    aSize.Width() = aPaneSize.Width() - 2 * aOffset.X();
    mpFLSeperator1->SetPosSizePixel( aCursor, aSize );

    // place the move down button
    aSize = mpPBMoveDown->GetSizePixel();

    aCursor.X() = aPaneSize.Width() - aOffset.X() - aSize.Width();
    aCursor.Y() -= aOffset.Y() + aSize.Height();
    mpPBMoveDown->SetPosPixel( aCursor );

    aCursor.X() -= aOffset.X() + aSize.Width();
    mpPBMoveUp->SetPosPixel( aCursor );

    // Place the change order label.
    // Its width has to be calculated dynamically so that is can be
    // displayed flush right without having too much space to the buttons
    // with some languages or truncated text with others.
    {
        Size aSize (mpFTChangeOrder->CalcMinimumSize());
        mpFTChangeOrder->SetSizePixel(aSize);
    }
    aCursor.X() -= aOffset.X() + mpFTChangeOrder->GetSizePixel().Width();
    aCursor.Y() += (aSize.Height() - mpFTChangeOrder->GetSizePixel().Height()) >> 1;
    mpFTChangeOrder->SetPosPixel( aCursor );

    // positionate the custom animation list control
    Size aCustomAnimationListSize( aPaneSize.Width() - aListPos.X() - aOffset.X(), aCursor.Y() - aListPos.Y() - 2 * aOffset.Y() );
    mpCustomAnimationList->SetPosSizePixel( aListPos, aCustomAnimationListSize );
}

static sal_Int32 getPropertyType( const OUString& rProperty )
{
    if( rProperty.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Direction") ) )
        return nPropertyTypeDirection;

    if( rProperty.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Spokes") ) )
        return nPropertyTypeSpokes;

    if( rProperty.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Zoom") ) )
        return nPropertyTypeZoom;

    if( rProperty.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Accelerate") ) )
        return nPropertyTypeAccelerate;

    if( rProperty.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Decelerate") ) )
        return nPropertyTypeDecelerate;

    if( rProperty.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Color1") ) )
        return nPropertyTypeFirstColor;

    if( rProperty.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Color2") ) )
        return nPropertyTypeSecondColor;

    if( rProperty.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("FillColor") ) )
        return nPropertyTypeFillColor;

    if( rProperty.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("ColorStyle") ) )
        return nPropertyTypeColorStyle;

    if( rProperty.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("AutoReverse") ) )
        return nPropertyTypeAutoReverse;

    if( rProperty.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("FontStyle") ) )
        return nPropertyTypeFont;

    if( rProperty.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("CharColor") ) )
        return nPropertyTypeCharColor;

    if( rProperty.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("CharHeight") ) )
        return nPropertyTypeCharHeight;

    if( rProperty.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("CharDecoration") ) )
        return nPropertyTypeCharDecoration;

    if( rProperty.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("LineColor") ) )
        return nPropertyTypeLineColor;

    if( rProperty.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Rotate") ) )
        return nPropertyTypeRotate;

    if( rProperty.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Transparency") ) )
        return nPropertyTypeTransparency;

    if( rProperty.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Color") ) )
        return nPropertyTypeColor;

    if( rProperty.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Scale") ) )
        return nPropertyTypeScale;

    return nPropertyTypeNone;
}

OUString getPropertyName( sal_Int32 nPropertyType )
{
    switch( nPropertyType )
    {
    case nPropertyTypeDirection:
        return OUString( String( SdResId( STR_CUSTOMANIMATION_DIRECTION_PROPERTY ) ) );

    case nPropertyTypeSpokes:
        return OUString( String( SdResId( STR_CUSTOMANIMATION_SPOKES_PROPERTY ) ) );

    case nPropertyTypeFirstColor:
        return OUString( String( SdResId( STR_CUSTOMANIMATION_FIRST_COLOR_PROPERTY ) ) );

    case nPropertyTypeSecondColor:
        return OUString( String( SdResId( STR_CUSTOMANIMATION_SECOND_COLOR_PROPERTY ) ) );

    case nPropertyTypeZoom:
        return OUString( String( SdResId( STR_CUSTOMANIMATION_ZOOM_PROPERTY ) ) );

    case nPropertyTypeFillColor:
        return OUString( String( SdResId( STR_CUSTOMANIMATION_FILL_COLOR_PROPERTY ) ) );

    case nPropertyTypeColorStyle:
        return OUString( String( SdResId( STR_CUSTOMANIMATION_STYLE_PROPERTY ) ) );

    case nPropertyTypeFont:
        return OUString( String( SdResId( STR_CUSTOMANIMATION_FONT_PROPERTY ) ) );

    case nPropertyTypeCharHeight:
        return OUString( String( SdResId( STR_CUSTOMANIMATION_SIZE_PROPERTY ) ) );

    case nPropertyTypeCharColor:
        return OUString( String( SdResId( STR_CUSTOMANIMATION_FONT_COLOR_PROPERTY ) ) );

    case nPropertyTypeCharHeightStyle:
        return OUString( String( SdResId( STR_CUSTOMANIMATION_FONT_SIZE_STYLE_PROPERTY ) ) );

    case nPropertyTypeCharDecoration:
        return OUString( String( SdResId( STR_CUSTOMANIMATION_FONT_STYLE_PROPERTY ) ) );

    case nPropertyTypeLineColor:
        return OUString( String( SdResId( STR_CUSTOMANIMATION_LINE_COLOR_PROPERTY ) ) );

    case nPropertyTypeRotate:
        return OUString( String( SdResId( STR_CUSTOMANIMATION_AMOUNT_PROPERTY ) ) );

    case nPropertyTypeColor:
        return OUString( String( SdResId( STR_CUSTOMANIMATION_COLOR_PROPERTY ) ) );

    case nPropertyTypeTransparency:
        return OUString( String( SdResId( STR_CUSTOMANIMATION_AMOUNT_PROPERTY ) ) );

    case nPropertyTypeScale:
        return OUString( String( SdResId( STR_CUSTOMANIMATION_SCALE_PROPERTY ) ) );
    }

    OUString aStr;
    return aStr;
}

void CustomAnimationPane::updateControls()
{
    const int nSelectionCount = maListSelection.size();

    mpPBAddEffect->Enable( maViewSelection.hasValue() );
    mpPBChangeEffect->Enable( nSelectionCount);
    mpPBRemoveEffect->Enable(nSelectionCount);

    mpFLEffect->Enable(nSelectionCount > 0);
    mpFTStart->Enable(nSelectionCount > 0);
    mpLBStart->Enable(nSelectionCount > 0);
    mpPBPropertyMore->Enable(nSelectionCount > 0);

//  mpPBPlay->Enable(nSelectionCount > 0);

    mpFTProperty->SetText( maStrProperty );

    mnPropertyType = nPropertyTypeNone;

    if( nSelectionCount == 1 )
    {
        CustomAnimationEffectPtr pEffect = maListSelection.front();

        OUString aUIName( mrPresets.getUINameForPresetId( pEffect->getPresetId() ) );

        OUString aTemp( maStrModify );

        if( aUIName.getLength() )
        {
            aTemp += OUString( (sal_Unicode)' ' );
            aTemp += aUIName;
        }
        mpFLEffect->SetText( aTemp );

        CustomAnimationPresetPtr pDescriptor = mrPresets.getEffectDescriptor( pEffect->getPresetId() );
        if( pDescriptor.get() )
        {
            PropertySubControl* pSubControl = NULL;

            UStringList aProperties( pDescriptor->getProperties() );
            if( aProperties.size() >= 1 )
            {
                OUString aProperty( aProperties.front() );

                mnPropertyType = getPropertyType( aProperties.front() );

                mpFTProperty->SetText( getPropertyName( mnPropertyType )  );

                Any aValue( getProperty1Value( mnPropertyType, pEffect ) );
                if( aValue.hasValue() )
                {
                    pSubControl = PropertySubControl::create( mnPropertyType, this, aValue, pEffect->getPresetId(), LINK( this, CustomAnimationPane, implPropertyHdl ) );
                }
            }

            mpLBProperty->setSubControl( pSubControl );

            bool bEnable = (pSubControl != 0) && (pSubControl->getControl()->IsEnabled());
            mpLBProperty->Enable( bEnable );
            mpFTProperty->Enable( bEnable );

            //
            // ---
            //
            USHORT nPos;

            sal_Int16 nNodeType = pEffect->getNodeType();
            switch( nNodeType )
            {
            case EffectNodeType::ON_CLICK:          nPos = 0; break;
            case EffectNodeType::WITH_PREVIOUS:     nPos = 1; break;
            case EffectNodeType::AFTER_PREVIOUS:    nPos = 2; break;
            default:                                nPos = 0xffff; break;
            }

            mpLBStart->SelectEntryPos( nPos );

            double fDuration = pEffect->getDuration();
            const bool bHasSpeed = fDuration > 0.001;

            mpFTSpeed->Enable(bHasSpeed);
            mpCBSpeed->Enable(bHasSpeed);

            if( bHasSpeed )
            {
                if( fDuration == 5.0 )
                    nPos = 0;
                else if( fDuration == 3.0 )
                    nPos = 1;
                else if( fDuration == 2.0 )
                    nPos = 2;
                else if( fDuration == 1.0 )
                    nPos = 3;
                else if( fDuration == 0.5 )
                    nPos = 4;
                else
                    nPos = 0xffff;

                mpCBSpeed->SelectEntryPos( nPos );
            }

            mpPBPropertyMore->Enable( TRUE );

            mpFTChangeOrder->Enable( TRUE );
        }
        else
        {
            mpLBProperty->setSubControl( 0 );
            mpFTProperty->Enable( FALSE );
            mpLBProperty->Enable( FALSE );
            mpPBPropertyMore->Enable( FALSE );
            mpFTSpeed->Enable(FALSE);
            mpCBSpeed->Enable(FALSE);
            mpFTChangeOrder->Enable( FALSE );
        }
    }
    else
    {
        mpLBProperty->setSubControl( 0 );
        mpFTProperty->Enable( FALSE );
        mpLBProperty->Enable( FALSE );
        mpPBPropertyMore->Enable( FALSE );
        mpFTSpeed->Enable(FALSE);
        mpCBSpeed->Enable(FALSE);
        mpFTChangeOrder->Enable( FALSE );
    }

    bool bEnableUp = true;
    bool bEnableDown = true;
    if( nSelectionCount == 0 )
    {
        bEnableUp = false;
        bEnableDown = false;
    }
    else
    {
        if( mpMainSequence->find( maListSelection.front() ) == mpMainSequence->getBegin() )
            bEnableUp = false;

        EffectSequence::iterator aIter( mpMainSequence->find( maListSelection.back() ) );
        if( aIter == mpMainSequence->getEnd() )
        {
            bEnableDown = false;
        }
        else
        {
            do
            {
                aIter++;
            }
            while( (aIter != mpMainSequence->getEnd()) && !(mpCustomAnimationList->isExpanded((*aIter)) ) );

            if( aIter == mpMainSequence->getEnd() )
                bEnableDown = false;
        }

        if( bEnableUp || bEnableDown )
        {
            MainSequenceRebuildGuard aGuard( mpMainSequence );

            EffectSequenceHelper* pSequence = 0;
            EffectSequence::iterator aIter( maListSelection.begin() );
            const EffectSequence::iterator aEnd( maListSelection.end() );
            while( aIter != aEnd )
            {
                CustomAnimationEffectPtr pEffect = (*aIter++);

                if( pEffect.get() )
                {
                    if( pSequence == 0 )
                    {
                        pSequence = pEffect->getEffectSequence();
                    }
                    else
                    {
                        if( pSequence != pEffect->getEffectSequence() )
                        {
                            bEnableUp = false;
                            bEnableDown = false;
                            break;
                        }
                    }
                }
            }
        }
    }

    mpPBMoveUp->Enable(bEnableUp);
    mpPBMoveDown->Enable(bEnableDown);

    SdOptions* pOptions = SD_MOD()->GetSdOptions(DOCUMENT_TYPE_IMPRESS);
    mpCBAutoPreview->Check( pOptions->IsPreviewChangedEffects() == sal_True );
}

void CustomAnimationPane::onSelectionChanged()
{
    if( mxView.is() ) try
    {
        Reference< XSelectionSupplier >  xSel( mxView, UNO_QUERY_THROW );
        if (xSel.is())
        {
            maViewSelection = xSel->getSelection();
            mpCustomAnimationList->onSelectionChanged( maViewSelection );
            updateControls();
        }
    }
    catch( Exception& )
    {
        DBG_ERROR( "sd::CustomAnimationPane::onSelectionChanged(), Exception catched!" );
    }
}

void CustomAnimationPane::onDoubleClick()
{
    showOptions();
}

void CustomAnimationPane::onContextMenu( USHORT nSelectedPopupEntry )
{
    switch( nSelectedPopupEntry )
    {
    case CM_WITH_CLICK:     onChangeStart( EffectNodeType::ON_CLICK ); break;
    case CM_WITH_PREVIOUS:  onChangeStart( EffectNodeType::WITH_PREVIOUS  ); break;
    case CM_AFTER_PREVIOUS: onChangeStart( EffectNodeType::AFTER_PREVIOUS ); break;
    case CM_OPTIONS:        showOptions(); break;
    case CM_DURATION:       showOptions(RID_TP_CUSTOMANIMATION_DURATION); break;
    case CM_REMOVE:         onRemove(); break;
    case CM_CREATE:         if( maViewSelection.hasValue() ) onChange( true ); break;
    }

    updateControls();
}

void addValue( STLPropertySet* pSet, sal_Int32 nHandle, const Any& rValue )
{
    switch( pSet->getPropertyState( nHandle ) )
    {
    case STLPropertyState_AMBIGUOUS:
        // value is already ambiguous, do nothing
        break;
    case STLPropertyState_DIRECT:
        // set to ambiguous if existing value is different
        if( rValue != pSet->getPropertyValue( nHandle ) )
            pSet->setPropertyState( nHandle, STLPropertyState_AMBIGUOUS );
        break;
    case STLPropertyState_DEFAULT:
        // just set new value
        pSet->setPropertyValue( nHandle, rValue );
        break;
    }
}

static sal_Int32 calcMaxParaDepth( Reference< XShape > xTargetShape )
{
    sal_Int32 nMaxParaDepth = -1;

    if( xTargetShape.is() )
    {
        Reference< XEnumerationAccess > xText( xTargetShape, UNO_QUERY );
        if( xText.is() )
        {
            Reference< XPropertySet > xParaSet;
            const OUString strNumberingLevel( RTL_CONSTASCII_USTRINGPARAM("NumberingLevel") );

            Reference< XEnumeration > xEnumeration( xText->createEnumeration(), UNO_QUERY_THROW );
            while( xEnumeration->hasMoreElements() )
            {
                xEnumeration->nextElement() >>= xParaSet;
                if( xParaSet.is() )
                {
                    sal_Int32 nParaDepth;
                    xParaSet->getPropertyValue( strNumberingLevel ) >>= nParaDepth;

                    if( nParaDepth > nMaxParaDepth )
                        nMaxParaDepth = nParaDepth;
                }
            }
        }
    }

    return nMaxParaDepth + 1;
}

Any CustomAnimationPane::getProperty1Value( sal_Int32 nType, CustomAnimationEffectPtr pEffect )
{
    switch( nType )
    {
    case nPropertyTypeDirection:
    case nPropertyTypeSpokes:
    case nPropertyTypeZoom:
        return makeAny( pEffect->getPresetSubType() );

    case nPropertyTypeColor:
    case nPropertyTypeFillColor:
    case nPropertyTypeFirstColor:
    case nPropertyTypeSecondColor:
    case nPropertyTypeCharColor:
    case nPropertyTypeLineColor:
        {
            const sal_Int32 nIndex = (nPropertyTypeFirstColor == nType) ? 0 : 1;
            return pEffect->getColor( nIndex );
        }

    case nPropertyTypeFont:
        return pEffect->getProperty( AnimationNodeType::SET, OUString( RTL_CONSTASCII_USTRINGPARAM("CharFontName") ), VALUE_TO );

    case nPropertyTypeCharHeight:
        {
            const OUString aAttributeName( RTL_CONSTASCII_USTRINGPARAM( "CharHeight" ) );
            Any aValue( pEffect->getProperty( AnimationNodeType::SET, aAttributeName, VALUE_TO ) );
            if( !aValue.hasValue() )
                aValue = pEffect->getProperty( AnimationNodeType::ANIMATE, aAttributeName, VALUE_TO );
            return aValue;
        }

    case nPropertyTypeRotate:
        return pEffect->getTransformationProperty( AnimationTransformType::ROTATE, VALUE_BY);

    case nPropertyTypeTransparency:
        return pEffect->getProperty( AnimationNodeType::SET, OUString(RTL_CONSTASCII_USTRINGPARAM("Opacity")), VALUE_TO );

    case nPropertyTypeScale:
        return pEffect->getTransformationProperty( AnimationTransformType::SCALE, VALUE_BY );

    case nPropertyTypeCharDecoration:
        {
            Sequence< Any > aValues(3);
            aValues[0] = pEffect->getProperty( AnimationNodeType::SET, OUString(RTL_CONSTASCII_USTRINGPARAM("CharWeight")), VALUE_TO );
            aValues[1] = pEffect->getProperty( AnimationNodeType::SET, OUString(RTL_CONSTASCII_USTRINGPARAM("CharPosture")), VALUE_TO );
            aValues[2] = pEffect->getProperty( AnimationNodeType::SET, OUString(RTL_CONSTASCII_USTRINGPARAM("CharUnderline")), VALUE_TO );
            return makeAny( aValues );
        }
    }

    Any aAny;
    return aAny;
}

bool CustomAnimationPane::setProperty1Value( sal_Int32 nType, CustomAnimationEffectPtr pEffect, const Any& rValue )
{
    bool bEffectChanged = false;
    switch( nType )
    {
    case nPropertyTypeDirection:
    case nPropertyTypeSpokes:
    case nPropertyTypeZoom:
        {
            OUString aPresetSubType;
            rValue >>= aPresetSubType;
            if( aPresetSubType != pEffect->getPresetSubType() )
            {
                mrPresets.changePresetSubType( pEffect, aPresetSubType );
                bEffectChanged = true;
            }
        }
        break;

    case nPropertyTypeFillColor:
    case nPropertyTypeColor:
    case nPropertyTypeFirstColor:
    case nPropertyTypeSecondColor:
    case nPropertyTypeCharColor:
    case nPropertyTypeLineColor:
        {
            const sal_Int32 nIndex = (nPropertyTypeFirstColor == nType) ? 0 : 1;
            Any aOldColor( pEffect->getColor( nIndex ) );
            if( aOldColor != rValue )
            {
                pEffect->setColor( nIndex, rValue );
                bEffectChanged = true;
            }
        }
        break;

    case nPropertyTypeFont:
        bEffectChanged = pEffect->setProperty( AnimationNodeType::SET, OUString( RTL_CONSTASCII_USTRINGPARAM( "CharFontName" ) ), VALUE_TO, rValue );
        break;

    case nPropertyTypeCharHeight:
        {
            const OUString aAttributeName( RTL_CONSTASCII_USTRINGPARAM( "CharHeight" ) );
            bEffectChanged = pEffect->setProperty( AnimationNodeType::SET, aAttributeName, VALUE_TO, rValue );
            if( !bEffectChanged )
                bEffectChanged = pEffect->setProperty( AnimationNodeType::ANIMATE, aAttributeName, VALUE_TO, rValue );
        }
        break;
    case nPropertyTypeRotate:
        bEffectChanged = pEffect->setTransformationProperty( AnimationTransformType::ROTATE, VALUE_BY , rValue );
        break;

    case nPropertyTypeTransparency:
        bEffectChanged = pEffect->setProperty( AnimationNodeType::SET, OUString( RTL_CONSTASCII_USTRINGPARAM("Opacity") ), VALUE_TO, rValue );
        break;

    case nPropertyTypeScale:
        bEffectChanged = pEffect->setTransformationProperty( AnimationTransformType::SCALE, VALUE_BY, rValue );
        break;

    case nPropertyTypeCharDecoration:
        {
            Sequence< Any > aValues(3);
            rValue >>= aValues;
            bEffectChanged = pEffect->setProperty( AnimationNodeType::SET, OUString(RTL_CONSTASCII_USTRINGPARAM("CharWeight")), VALUE_TO, aValues[0] );
            bEffectChanged |= pEffect->setProperty( AnimationNodeType::SET, OUString(RTL_CONSTASCII_USTRINGPARAM("CharPosture")), VALUE_TO, aValues[1] );
            bEffectChanged |= pEffect->setProperty( AnimationNodeType::SET, OUString(RTL_CONSTASCII_USTRINGPARAM("CharUnderline")), VALUE_TO, aValues[2] );
        }
        break;

    }

    return bEffectChanged;
}


STLPropertySet* CustomAnimationPane::createSelectionSet()
{
    STLPropertySet* pSet = CustomAnimationDialog::createDefaultSet();

    pSet->setPropertyValue( nHandleCurrentPage, makeAny( mxCurrentPage ) );

    sal_Int32 nMaxParaDepth = 0;

    // get options from selected effects
    EffectSequence::iterator aIter( maListSelection.begin() );
    const EffectSequence::iterator aEnd( maListSelection.end() );
    while( aIter != aEnd )
    {
        CustomAnimationEffectPtr pEffect = (*aIter++);

        EffectSequenceHelper* pEffectSequence = pEffect->getEffectSequence();
        if( !pEffectSequence )
            pEffectSequence = mpMainSequence.get();

        if( pEffect->hasText() )
        {
            sal_Int32 n = calcMaxParaDepth(pEffect->getTargetShape());
            if( n > nMaxParaDepth )
                nMaxParaDepth = n;
        }

        addValue( pSet, nHandleHasAfterEffect, makeAny( pEffect->hasAfterEffect() ) );
        addValue( pSet, nHandleMasterRel, makeAny( pEffect->getMasterRel() ) );
        addValue( pSet, nHandleDimColor, pEffect->getDimColor() );
        addValue( pSet, nHandleIterateType, makeAny( pEffect->getIterateType() ) );

        // convert absolute time to percentage value
        // This calculation is done in float to avoid some rounding artifacts.
        float fIterateInterval = (float)pEffect->getIterateInterval();
        if( pEffect->getDuration() )
            fIterateInterval = (float)(fIterateInterval / pEffect->getDuration() );
        fIterateInterval *= 100.0;
        addValue( pSet, nHandleIterateInterval, makeAny( (double)fIterateInterval ) );

        addValue( pSet, nHandleBegin, makeAny( pEffect->getBegin() ) );
        addValue( pSet, nHandleDuration, makeAny( pEffect->getDuration() ) );
        addValue( pSet, nHandleStart, makeAny( pEffect->getNodeType() ) );
        addValue( pSet, nHandleRepeat, makeAny( pEffect->getRepeatCount() ) );
        addValue( pSet, nHandleEnd, pEffect->getEnd() );
        addValue( pSet, nHandleRewind, makeAny( pEffect->getFill() ) );

        addValue( pSet, nHandlePresetId, makeAny( pEffect->getPresetId() ) );

        addValue( pSet, nHandleHasText, makeAny( (sal_Bool)pEffect->hasText() ) );

        Any aSoundSource;
        if( pEffect->getAudio().is() )
        {
            aSoundSource = pEffect->getAudio()->getSource();
            addValue( pSet, nHandleSoundVolumne, makeAny( pEffect->getAudio()->getVolume() ) );
// todo     addValue( pSet, nHandleSoundEndAfterSlide, makeAny( pEffect->getAudio()->getEndAfterSlide() ) );
// this is now stored at the XCommand parameter sequence
        }
        else if( pEffect->getCommand() == EffectCommands::STOPAUDIO )
        {
            aSoundSource = makeAny( (sal_Bool)sal_True );
        }
        addValue( pSet, nHandleSoundURL, aSoundSource );

        sal_Int32 nGroupId = pEffect->getGroupId();
        CustomAnimationTextGroupPtr pTextGroup;
        if( nGroupId != -1 )
            pTextGroup = pEffectSequence->findGroup( nGroupId );

        addValue( pSet, nHandleTextGrouping, makeAny( pTextGroup.get() ? pTextGroup->getTextGrouping() : (sal_Int32)-1 ) );
        addValue( pSet, nHandleAnimateForm, makeAny( pTextGroup.get() ? (sal_Bool)pTextGroup->getAnimateForm() : sal_True ) );
        addValue( pSet, nHandleTextGroupingAuto, makeAny( pTextGroup.get() ? pTextGroup->getTextGroupingAuto() : (double)-1.0 ) );
        addValue( pSet, nHandleTextReverse, makeAny( pTextGroup.get() ? (sal_Bool)pTextGroup->getTextReverse() : sal_False ) );

        if( pEffectSequence->getSequenceType() == EffectNodeType::INTERACTIVE_SEQUENCE  )
        {
            InteractiveSequence* pIS = static_cast< InteractiveSequence* >( pEffectSequence );
            addValue( pSet, nHandleTrigger, makeAny( pIS->getTriggerShape() ) );
        }

        //

        CustomAnimationPresetPtr pDescriptor = mrPresets.getEffectDescriptor( pEffect->getPresetId() );
        if( pDescriptor.get() )
        {
            sal_Int32 nType = nPropertyTypeNone;

            UStringList aProperties( pDescriptor->getProperties() );
            if( aProperties.size() >= 1 )
                nType = getPropertyType( aProperties.front() );

            if( nType != nPropertyTypeNone )
            {
                addValue( pSet, nHandleProperty1Type, makeAny( nType ) );
                addValue( pSet, nHandleProperty1Value, getProperty1Value( nType, pEffect ) );
            }

            if( pDescriptor->hasProperty( OUString( RTL_CONSTASCII_USTRINGPARAM( "Accelerate" ) ) ) )
            {
                addValue( pSet, nHandleAccelerate, makeAny( pEffect->getAcceleration() ) );
            }

            if( pDescriptor->hasProperty( OUString( RTL_CONSTASCII_USTRINGPARAM( "Decelerate" ) ) ) )
            {
                addValue( pSet, nHandleDecelerate, makeAny( pEffect->getDecelerate() ) );
            }

            if( pDescriptor->hasProperty( OUString( RTL_CONSTASCII_USTRINGPARAM( "AutoReverse" ) ) ) )
            {
                addValue( pSet, nHandleAutoReverse, makeAny( pEffect->getAutoReverse() ) );
            }
        }
    }

    addValue( pSet, nHandleMaxParaDepth, makeAny( nMaxParaDepth ) );

    return pSet;
}

void CustomAnimationPane::changeSelection( STLPropertySet* pResultSet, STLPropertySet* pOldSet )
{
    // change selected effect
    bool bChanged = false;

    MainSequenceRebuildGuard aGuard( mpMainSequence );

    EffectSequence::iterator aIter( maListSelection.begin() );
    const EffectSequence::iterator aEnd( maListSelection.end() );
    while( aIter != aEnd )
    {
        CustomAnimationEffectPtr pEffect = (*aIter++);

        double fDuration; // we might need this for iterate-interval
        if( pResultSet->getPropertyState( nHandleDuration ) == STLPropertyState_DIRECT )
        {
            pResultSet->getPropertyValue( nHandleDuration ) >>= fDuration;
        }
        else
        {
            fDuration = pEffect->getDuration();
        }

        if( pResultSet->getPropertyState( nHandleIterateType ) == STLPropertyState_DIRECT )
        {
            sal_Int16 nIterateType;
            pResultSet->getPropertyValue( nHandleIterateType ) >>= nIterateType;
            if( pEffect->getIterateType() != nIterateType )
            {
                pEffect->setIterateType( nIterateType );
                bChanged = true;
            }
        }

        if( pEffect->getIterateType() )
        {
            if( pResultSet->getPropertyState( nHandleIterateInterval ) == STLPropertyState_DIRECT )
            {
                double fIterateInterval;
                pResultSet->getPropertyValue( nHandleIterateInterval ) >>= fIterateInterval;
                if( pEffect->getIterateInterval() != fIterateInterval )
                {
                    const double f = fIterateInterval * pEffect->getDuration() / 100;
                    pEffect->setIterateInterval( f );
                    bChanged = true;
                }
            }
        }

        if( pResultSet->getPropertyState( nHandleBegin ) == STLPropertyState_DIRECT )
        {
            double fBegin;
            pResultSet->getPropertyValue( nHandleBegin ) >>= fBegin;
            if( pEffect->getBegin() != fBegin )
            {
                pEffect->setBegin( fBegin );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleDuration ) == STLPropertyState_DIRECT )
        {
            if( pEffect->getDuration() != fDuration )
            {
                pEffect->setDuration( fDuration );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleStart ) == STLPropertyState_DIRECT )
        {
            sal_Int16 nNodeType;
            pResultSet->getPropertyValue( nHandleStart ) >>= nNodeType;
            if( pEffect->getNodeType() != nNodeType )
            {
                pEffect->setNodeType( nNodeType );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleRepeat ) == STLPropertyState_DIRECT )
        {
            Any aRepeatCount( pResultSet->getPropertyValue( nHandleRepeat ) );
            if( aRepeatCount != pEffect->getRepeatCount() )
            {
                pEffect->setRepeatCount( aRepeatCount );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleEnd ) == STLPropertyState_DIRECT )
        {
            Any aEnd( pResultSet->getPropertyValue( nHandleEnd ) );
            if( pEffect->getEnd() != aEnd )
            {
                pEffect->setEnd( aEnd );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleRewind ) == STLPropertyState_DIRECT )
        {
            sal_Int16 nFill;
            pResultSet->getPropertyValue( nHandleRewind ) >>= nFill;
            if( pEffect->getFill() != nFill )
            {
                pEffect->setFill( nFill );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleHasAfterEffect ) == STLPropertyState_DIRECT )
        {
            sal_Bool bHasAfterEffect;
            if( pResultSet->getPropertyValue( nHandleHasAfterEffect )  >>= bHasAfterEffect )
            {
                if( pEffect->hasAfterEffect() != bHasAfterEffect )
                {
                    pEffect->setHasAfterEffect( bHasAfterEffect );
                    bChanged = true;
                }
            }
        }

        if( pResultSet->getPropertyState( nHandleMasterRel ) == STLPropertyState_DIRECT )
        {
            sal_Int32 nMasterRel;
            if( (pResultSet->getPropertyValue( nHandleMasterRel ) >>= nMasterRel) && (pEffect->getMasterRel() != nMasterRel) )
            {
                pEffect->setMasterRel( nMasterRel );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleDimColor ) == STLPropertyState_DIRECT )
        {
            Any aDimColor( pResultSet->getPropertyValue( nHandleDimColor ) );
            if( pEffect->getDimColor() != aDimColor )
            {
                pEffect->setDimColor( aDimColor );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleAccelerate ) == STLPropertyState_DIRECT )
        {
            double fAccelerate;
            pResultSet->getPropertyValue( nHandleAccelerate ) >>= fAccelerate;
            if( pEffect->getAcceleration() != fAccelerate )
            {
                pEffect->setAcceleration( fAccelerate );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleDecelerate ) == STLPropertyState_DIRECT )
        {
            double fDecelerate;
            pResultSet->getPropertyValue( nHandleDecelerate ) >>= fDecelerate;
            if( pEffect->getDecelerate() != fDecelerate )
            {
                pEffect->setDecelerate( fDecelerate );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleAutoReverse ) == STLPropertyState_DIRECT )
        {
            sal_Bool bAutoReverse;
            pResultSet->getPropertyValue( nHandleAutoReverse ) >>= bAutoReverse;
            if( pEffect->getAutoReverse() != bAutoReverse )
            {
                pEffect->setAutoReverse( bAutoReverse );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleProperty1Value ) == STLPropertyState_DIRECT )
        {
            sal_Int32 nType;
            pOldSet->getPropertyValue( nHandleProperty1Type ) >>= nType;

            bChanged |= setProperty1Value( nType, pEffect, pResultSet->getPropertyValue( nHandleProperty1Value ) );
        }

        if( pResultSet->getPropertyState( nHandleSoundURL ) == STLPropertyState_DIRECT )
        {
            const Any aSoundSource( pResultSet->getPropertyValue( nHandleSoundURL ) );

            if( aSoundSource.getValueType() == ::getCppuType((const sal_Bool*)0) )
            {
                pEffect->setStopAudio();
                bChanged = true;
            }
            else
            {
                OUString aSoundURL;
                aSoundSource >>= aSoundURL;

                if( aSoundURL.getLength() )
                {
                    if( !pEffect->getAudio().is() )
                    {
                        pEffect->createAudio( aSoundSource );
                        bChanged = true;
                    }
                    else
                    {
                        if( pEffect->getAudio()->getSource() != aSoundSource )
                        {
                            pEffect->getAudio()->setSource( aSoundSource );
                            bChanged = true;
                        }
                    }
                }
                else
                {
                    if( pEffect->getAudio().is() || pEffect->getStopAudio() )
                    {
                        pEffect->removeAudio();
                        bChanged = true;
                    }
                }
            }
        }

        if( pResultSet->getPropertyState( nHandleTrigger ) == STLPropertyState_DIRECT )
        {
            Reference< XShape > xTriggerShape;
            pResultSet->getPropertyValue( nHandleTrigger ) >>= xTriggerShape;
            bChanged |= mpMainSequence->setTrigger( pEffect, xTriggerShape );
        }
    }

    const bool bHasTextGrouping = pResultSet->getPropertyState( nHandleTextGrouping ) == STLPropertyState_DIRECT;
    const bool bHasAnimateForm = pResultSet->getPropertyState( nHandleAnimateForm ) == STLPropertyState_DIRECT;
    const bool bHasTextGroupingAuto = pResultSet->getPropertyState( nHandleTextGroupingAuto ) == STLPropertyState_DIRECT;
    const bool bHasTextReverse = pResultSet->getPropertyState( nHandleTextReverse ) == STLPropertyState_DIRECT;

    if( bHasTextGrouping || bHasAnimateForm || bHasTextGroupingAuto || bHasTextReverse )
    {
        // we need to do a second pass for text grouping options
        // since changing them can cause effects to be removed
        // or replaced, we do this after we aplied all other options
        // above

        sal_Int32 nTextGrouping = 0;
        sal_Bool bAnimateForm = sal_True, bTextReverse = sal_False;
        double fTextGroupingAuto = -1.0;

        if( bHasTextGrouping )
            pResultSet->getPropertyValue(nHandleTextGrouping) >>= nTextGrouping;

        if( bHasAnimateForm )
            pResultSet->getPropertyValue(nHandleAnimateForm) >>= bAnimateForm;

        if( bHasTextGroupingAuto )
            pResultSet->getPropertyValue(nHandleTextGroupingAuto) >>= fTextGroupingAuto;

        if( bHasTextReverse )
            pResultSet->getPropertyValue(nHandleTextReverse) >>= bTextReverse;

        aIter = maListSelection.begin();
        while( aIter != aEnd )
        {
            CustomAnimationEffectPtr pEffect = (*aIter++);

            EffectSequenceHelper* pEffectSequence = pEffect->getEffectSequence();
            if( !pEffectSequence )
                pEffectSequence = mpMainSequence.get();

            sal_Int32 nGroupId = pEffect->getGroupId();
            CustomAnimationTextGroupPtr pTextGroup;
            if( (nGroupId != -1) )
            {
                // use existing group
                pTextGroup = pEffectSequence->findGroup( nGroupId );
            }
            else
            {
                // somethings changed so we need a group now
                pTextGroup = pEffectSequence->createTextGroup( pEffect, nTextGrouping, fTextGroupingAuto, bAnimateForm, bTextReverse );
                bChanged = true;
            }

            if( bHasTextGrouping )
            {
                if( (pTextGroup->getTextGrouping() != nTextGrouping) )
                {
                    pEffectSequence->setTextGrouping( pTextGroup, nTextGrouping );
                    bChanged = true;
                }
            }

            if( bHasAnimateForm )
            {
                if( pTextGroup->getAnimateForm() != bAnimateForm )
                {
                    pEffectSequence->setAnimateForm( pTextGroup, bAnimateForm );
                    bChanged = true;
                }
            }

            if( bHasTextGroupingAuto )
            {
                if( pTextGroup->getTextGroupingAuto() != fTextGroupingAuto )
                {
                    pEffectSequence->setTextGroupingAuto( pTextGroup, fTextGroupingAuto );
                    bChanged = true;
                }
            }

            if( bHasTextReverse )
            {
                if( pTextGroup->getTextReverse() != bTextReverse )
                {
                    pEffectSequence->setTextReverse( pTextGroup, bTextReverse );
                    bChanged = true;
                }
            }
        }
    }

    if( bChanged )
    {
        mpMainSequence->rebuild();
        updateControls();
        mrBase.GetDocShell()->SetModified();
    }
}

void CustomAnimationPane::showOptions( USHORT nPage /* = 0 */ )
{
    STLPropertySet* pSet = createSelectionSet();

    CustomAnimationDialog* pDlg = new CustomAnimationDialog( this, pSet, nPage );
    if( pDlg->Execute() )
    {
        addUndo();
        changeSelection( pDlg->getResultSet(), pSet );
        updateControls();
    }

    delete pDlg;
}

void CustomAnimationPane::onChangeCurrentPage()
{
    if( mxView.is() ) try
    {
        mxCurrentPage = mxView->getCurrentPage();
        SdPage* pPage = SdPage::getImplementation( mxCurrentPage );
        if( pPage )
        {
            mpMainSequence = pPage->getMainSequence();
            mpCustomAnimationList->update( mpMainSequence );
        }
        updateControls();
    }
    catch( Exception& )
    {
        DBG_ERROR( "sd::CustomAnimationPane::onChangeCurrentPage(), exception catched!" );
    }
}

bool getTextSelection( const Any& rSelection, Reference< XShape >& xShape, std::list< sal_Int16 >& rParaList )
{
    Reference< XTextRange > xSelectedText;
    rSelection >>= xSelectedText;
    if( xSelectedText.is() ) try
    {
        xShape.set( xSelectedText->getText(), UNO_QUERY_THROW );

        Reference< XTextRangeCompare > xTextRangeCompare( xShape, UNO_QUERY_THROW );
        Reference< XEnumerationAccess > xParaEnumAccess( xShape, UNO_QUERY_THROW );
        Reference< XEnumeration > xParaEnum( xParaEnumAccess->createEnumeration(), UNO_QUERY_THROW );
        Reference< XTextRange > xRange;
        Reference< XTextRange > xStart( xSelectedText->getStart() );
        Reference< XTextRange > xEnd( xSelectedText->getEnd() );

        sal_Int16 nPara = 0;
        while( xParaEnum->hasMoreElements() )
        {
            xParaEnum->nextElement() >>= xRange;

            // break if start of selection is prior to end of current paragraph
            if( xRange.is() && (xTextRangeCompare->compareRegionEnds( xStart, xRange ) >= 0 ) )
                break;

            nPara++;
        }

        while( xRange.is() )
        {
            if( xRange.is() && xRange->getString().getLength() )
                rParaList.push_back( nPara );

            // break if end of selection is before or at end of current paragraph
            if( xRange.is() && xTextRangeCompare->compareRegionEnds( xEnd, xRange ) >= 0 )
                break;

            nPara++;

            if( xParaEnum->hasMoreElements() )
                xParaEnum->nextElement() >>= xRange;
            else
                xRange.clear();
        }

        return true;
    }
    catch( Exception& e )
    {
        (void)e;
        DBG_ERROR( "sd::CustomAnimationPane::getTextSelection(), exception cought!" );
    }

    return false;
}

void CustomAnimationPane::onChange( bool bCreate )
{
    bool bHasText = true;

    // first create vector of targets for dialog preview
    std::vector< Any > aTargets;
    if( bCreate )
    {
        // gather shapes from the selection
        Reference< XSelectionSupplier >  xSel( mxView, UNO_QUERY_THROW );
        maViewSelection = xSel->getSelection();

        if( maViewSelection.getValueType() == ::getCppuType((const Reference< XShapes >*)0) )
        {
            Reference< XIndexAccess > xShapes;
            maViewSelection >>= xShapes;

            sal_Int32 nCount = xShapes->getCount();
            sal_Int32 nIndex;
            for( nIndex = 0; nIndex < nCount; nIndex++ )
            {
                Any aTarget( xShapes->getByIndex( nIndex ) );
                aTargets.push_back( aTarget );
                if( bHasText )
                {
                    Reference< XText > xText;
                    aTarget >>= xText;
                    if( !xText.is() || xText->getString().getLength() == 0 )
                        bHasText = false;
                }
            }
        }
        else if ( maViewSelection.getValueType() == ::getCppuType((const Reference< XShape >*)0) )
        {
            aTargets.push_back( maViewSelection );
            Reference< XText > xText;
            maViewSelection >>= xText;
            if( !xText.is() || xText->getString().getLength() == 0 )
                bHasText = false;
        }
        else if ( maViewSelection.getValueType() == ::getCppuType((const Reference< XTextCursor >*)0) )
        {
            Reference< XShape > xShape;
            sal_Int16 nFirstPara, nLastPara;
            std::list< sal_Int16 > aParaList;
            if( getTextSelection( maViewSelection, xShape, aParaList ) )
            {
                ParagraphTarget aParaTarget;
                aParaTarget.Shape = xShape;

                std::list< sal_Int16 >::iterator aIter( aParaList.begin() );
                for( ; aIter != aParaList.end(); aIter++ )
                {
                    aParaTarget.Paragraph = (*aIter);
                    aTargets.push_back( makeAny( aParaTarget ) );
                   }
            }
        }
        else
        {
            DBG_ERROR("sd::CustomAnimationPane::onChange(), unknown view selection!" );
            return;
        }
    }
    else
    {
        // get selected effect
        EffectSequence::iterator aIter( maListSelection.begin() );
        const EffectSequence::iterator aEnd( maListSelection.end() );
        while( aIter != aEnd )
        {
            if( !bHasText || !(*aIter)->hasText() )
                bHasText = false;
            aTargets.push_back( (*aIter++)->getTarget() );
        }
    }

    CustomAnimationCreateDialog* pDlg = new CustomAnimationCreateDialog( this, this, aTargets, bHasText );
    if( pDlg->Execute() )
    {
        addUndo();
        CustomAnimationPresetPtr pDescriptor = pDlg->getSelectedPreset();
        if( pDescriptor.get() )
        {
            double fDuration = pDlg->getSelectedDuration();

            if( bCreate )
            {
                mpCustomAnimationList->SelectAll( FALSE );

                // gather shapes from the selection
                std::vector< Any >::iterator aIter( aTargets.begin() );
                const std::vector< Any >::iterator aEnd( aTargets.end() );
                bool bFirst = true;
                for( ; aIter != aEnd; aIter++ )
                {
                    CustomAnimationEffectPtr pCreated = mpMainSequence->append( pDescriptor, (*aIter), fDuration );

                    if( bFirst )
                        bFirst = false;
                    else
                        pCreated->setNodeType( EffectNodeType::WITH_PREVIOUS );

                    if( pCreated.get() )
                    {
                        mpCustomAnimationList->select( pCreated );
                    }
                }
            }
            else
            {
                MainSequenceRebuildGuard aGuard( mpMainSequence );

                // get selected effect
                EffectSequence::iterator aIter( maListSelection.begin() );
                const EffectSequence::iterator aEnd( maListSelection.end() );
                while( aIter != aEnd )
                {
                    CustomAnimationEffectPtr pEffect = (*aIter++);

                    EffectSequenceHelper* pEffectSequence = pEffect->getEffectSequence();
                    if( !pEffectSequence )
                        pEffectSequence = mpMainSequence.get();

                    pEffectSequence->replace( pEffect, pDescriptor, fDuration );
                }
            }
        }
        mrBase.GetDocShell()->SetModified();
    }

    delete pDlg;

    updateControls();

    // stop running preview from dialog
    DrawViewShell* pViewShell = dynamic_cast< DrawViewShell* >( mrBase.GetPaneManager().GetViewShell() );
    if( pViewShell )
        pViewShell->SetSlideShow( 0 );
}

void CustomAnimationPane::onRemove()
{
    if( maListSelection.size() )
    {
        addUndo();

        MainSequenceRebuildGuard aGuard( mpMainSequence );

        EffectSequence aList( maListSelection );

        EffectSequence::iterator aIter( aList.begin() );
        const EffectSequence::iterator aEnd( aList.end() );
        while( aIter != aEnd )
        {
            CustomAnimationEffectPtr pEffect = (*aIter++);
            if( pEffect->getEffectSequence() )
                pEffect->getEffectSequence()->remove( pEffect );
        }

        maListSelection.clear();
        mrBase.GetDocShell()->SetModified();
    }
}

void CustomAnimationPane::onChangeStart()
{
    if( mpLBStart->GetSelectEntryCount() == 1 )
    {
        sal_Int16 nNodeType;
        USHORT nPos= mpLBStart->GetSelectEntryPos();
        switch( nPos )
        {
        case 0: nNodeType = EffectNodeType::ON_CLICK; break;
        case 1: nNodeType = EffectNodeType::WITH_PREVIOUS; break;
        case 2: nNodeType = EffectNodeType::AFTER_PREVIOUS; break;
        default:
            return;
        }

        onChangeStart( nNodeType );
    }
}

void CustomAnimationPane::onChangeStart( sal_Int16 nNodeType )
{
    addUndo();

    MainSequenceRebuildGuard aGuard( mpMainSequence );

    bool bNeedRebuild = false;

    EffectSequence::iterator aIter( maListSelection.begin() );
    const EffectSequence::iterator aEnd( maListSelection.end() );
    while( aIter != aEnd )
    {
        CustomAnimationEffectPtr pEffect = (*aIter++);
        if( pEffect->getNodeType() != nNodeType )
        {
            pEffect->setNodeType( nNodeType );
            bNeedRebuild = true;
        }
    }

    if( bNeedRebuild )
    {
        mpMainSequence->rebuild();
        updateControls();
        mrBase.GetDocShell()->SetModified();
    }
}

void CustomAnimationPane::onChangeProperty()
{
    if( mpLBProperty->getSubControl() )
    {
        addUndo();

        MainSequenceRebuildGuard aGuard( mpMainSequence );

        const Any aValue( mpLBProperty->getSubControl()->getValue() );

        bool bNeedUpdate = false;

        // change selected effect
        EffectSequence::iterator aIter( maListSelection.begin() );
        const EffectSequence::iterator aEnd( maListSelection.end() );
        while( aIter != aEnd )
        {
            CustomAnimationEffectPtr pEffect = (*aIter++);

            if( setProperty1Value( mnPropertyType, pEffect, aValue ) )
                bNeedUpdate = true;
        }

        if( bNeedUpdate )
        {
            mpMainSequence->rebuild();
            updateControls();
            mrBase.GetDocShell()->SetModified();
        }

        onPreview( false );
    }
}

void CustomAnimationPane::onChangeSpeed()
{
    if( mpCBSpeed->GetSelectEntryCount() == 1 )
    {
        addUndo();

        MainSequenceRebuildGuard aGuard( mpMainSequence );

        double fDuration;

        USHORT nPos= mpCBSpeed->GetSelectEntryPos();

        switch( nPos )
        {
        case 0: fDuration = 5.0; break;
        case 1: fDuration = 3.0; break;
        case 2: fDuration = 2.0; break;
        case 3: fDuration = 1.0; break;
        case 4: fDuration = 0.5; break;
        default:
            return;
        }

        // change selected effect
        EffectSequence::iterator aIter( maListSelection.begin() );
        const EffectSequence::iterator aEnd( maListSelection.end() );
        while( aIter != aEnd )
        {
            CustomAnimationEffectPtr pEffect = (*aIter++);
            pEffect->setDuration( fDuration );
        }

        mpMainSequence->rebuild();
        updateControls();
        mrBase.GetDocShell()->SetModified();

        onPreview( false );
    }
}

/// this link is called when the property box is modified by the user
IMPL_LINK( CustomAnimationPane, implPropertyHdl, Control*, pControl )
{
    onChangeProperty();
    return 0;
}

/// this link is called when one of the controls is modified
IMPL_LINK( CustomAnimationPane, implControlHdl, Control*, pControl )
{
    if( pControl == mpPBAddEffect )
        onChange(true);
    else if( pControl == mpPBChangeEffect )
        onChange(false);
    else if( pControl == mpPBRemoveEffect )
        onRemove();
    else if( pControl == mpLBStart )
        onChangeStart();
    else if( pControl == mpCBSpeed )
        onChangeSpeed();
    else if( pControl == mpPBPropertyMore )
        showOptions();
    else if( pControl == mpPBMoveUp )
        moveSelection( true );
    else if( pControl == mpPBMoveDown )
        moveSelection( false );
    else if( pControl == mpPBPlay )
        onPreview( true );
    else if( pControl == mpPBSlideShow )
    {
        mrBase.StartPresentation();
    }
    else if( pControl == mpCBAutoPreview )
    {
        SdOptions* pOptions = SD_MOD()->GetSdOptions(DOCUMENT_TYPE_IMPRESS);
        pOptions->SetPreviewChangedEffects( mpCBAutoPreview->IsChecked() ? sal_True : sal_False );
    }

    updateControls();

    return 0;
}

void CustomAnimationPane::moveSelection( bool bUp )
{
    if( maListSelection.empty() )
        return;

    EffectSequenceHelper* pSequence = maListSelection.front()->getEffectSequence();
    if( pSequence == 0 )
        return;

    addUndo();

    bool bChanged = false;

    MainSequenceRebuildGuard aGuard( mpMainSequence );
    EffectSequence& rEffectSequence = pSequence->getSequence();

    if( bUp )
    {
        EffectSequence::iterator aIter( maListSelection.begin() );
        const EffectSequence::iterator aEnd( maListSelection.end() );

        while( aIter != aEnd )
        {
            CustomAnimationEffectPtr pEffect = (*aIter++);

            EffectSequence::iterator aEffectPos( pSequence->find( pEffect ) );
            if( aEffectPos != rEffectSequence.end() )
            {
                EffectSequence::iterator aInsertPos( rEffectSequence.erase( aEffectPos ) );

                if( aInsertPos != rEffectSequence.begin() )
                {
                    aInsertPos--;
                    while( (aInsertPos != rEffectSequence.begin()) && !mpCustomAnimationList->isExpanded(*aInsertPos))
                        aInsertPos--;

                    rEffectSequence.insert( aInsertPos, pEffect );
                }
                else
                {
                    rEffectSequence.push_front( pEffect );
                }
                bChanged = true;
            }
        }
    }
    else
    {
        EffectSequence::reverse_iterator aIter( maListSelection.rbegin() );
        const EffectSequence::reverse_iterator aEnd( maListSelection.rend() );

        while( aIter != aEnd )
        {
            CustomAnimationEffectPtr pEffect = (*aIter++);

            EffectSequence::iterator aEffectPos( pSequence->find( pEffect ) );
            if( aEffectPos != rEffectSequence.end() )
            {
                EffectSequence::iterator aInsertPos( rEffectSequence.erase( aEffectPos ) );

                if( aInsertPos != rEffectSequence.end() )
                {
                    aInsertPos++;
                    while( (aInsertPos != rEffectSequence.end()) && !mpCustomAnimationList->isExpanded(*aInsertPos))
                        aInsertPos++;

                    rEffectSequence.insert( aInsertPos, pEffect );
                }
                else
                {
                    rEffectSequence.push_back( pEffect );
                }
                bChanged = true;
            }
        }
    }

    if( bChanged )
    {
        mpMainSequence->rebuild();
        updateControls();
        mrBase.GetDocShell()->SetModified();
    }
}

void CustomAnimationPane::onPreview( bool bForcePreview )
{
    if( !bForcePreview && !mpCBAutoPreview->IsChecked() )
        return;

    if( maListSelection.empty() )
    {
        Reference< XAnimationNodeSupplier > xNodeSupplier( mxCurrentPage, UNO_QUERY );
        if( !xNodeSupplier.is() )
            return;

        preview( xNodeSupplier->getAnimationNode() );
    }
    else
    {
        MainSequencePtr pSequence( new MainSequence() );

        EffectSequence::iterator aIter( maListSelection.begin() );
        const EffectSequence::iterator aEnd( maListSelection.end() );

        while( aIter != aEnd )
        {
            CustomAnimationEffectPtr pEffect = (*aIter++);
            pSequence->append( pEffect->clone() );
        }

        preview( pSequence->getRootNode() );
    }
}

void CustomAnimationPane::preview( const Reference< XAnimationNode >& xAnimationNode )
{
    DrawViewShell* pViewShell = dynamic_cast< DrawViewShell* >( mrBase.GetPaneManager().GetViewShell() );
    if( pViewShell == 0 )
        return;

    DrawView* pView = pViewShell->GetDrawView();


    Reference< XTimeContainer > xRoot(::comphelper::getProcessServiceFactory()->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.animations.ParallelTimeContainer"))), UNO_QUERY);
    if( xRoot.is() )
    {
        Sequence< ::com::sun::star::beans::NamedValue > aUserData( 1 );
        aUserData[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "node-type" ) );
        aUserData[0].Value <<= ::com::sun::star::presentation::EffectNodeType::TIMING_ROOT;
        xRoot->setUserData( aUserData );
        xRoot->appendChild( xAnimationNode );

        pViewShell->SetSlideShow( 0 );
        std::auto_ptr<Slideshow> pSlideshow(
            new Slideshow( pViewShell, pView, pViewShell->GetDoc() ) );
        Reference< XAnimationNode > xNode( xRoot, UNO_QUERY );
        if (pSlideshow->startPreview( mxCurrentPage, xNode ))
            pViewShell->SetSlideShow( pSlideshow.release() );
    }
}


// ICustomAnimationListController
void CustomAnimationPane::onSelect()
{
    maListSelection = mpCustomAnimationList->getSelection();
    updateControls();
}

// ====================================================================

::Window * createCustomAnimationPanel( ::Window* pParent, ViewShellBase& rBase )
{
    DialogListBox* pWindow = 0;

    DrawDocShell* pDocSh = rBase.GetDocShell();
    if( pDocSh )
    {
        pWindow = new DialogListBox( pParent, WB_CLIPCHILDREN|WB_TABSTOP|WB_AUTOHSCROLL );

        Size aMinSize( pWindow->LogicToPixel( Size( 80, 256 ), MAP_APPFONT ) );
        ::Window* pPaneWindow = new CustomAnimationPane( pWindow, rBase, aMinSize );
        pWindow->SetChildWindow( pPaneWindow, aMinSize );
        pWindow->SetText( pPaneWindow->GetText() );
    }

    return pWindow;
}



}
