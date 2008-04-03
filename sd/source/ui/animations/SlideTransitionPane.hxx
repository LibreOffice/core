/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlideTransitionPane.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:25:47 $
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
#ifndef SD_SLIDETRANSITIONPANE_HXX
#define SD_SLIDETRANSITIONPANE_HXX

#ifndef SD_TOOLS_EVENT_MULTIPLEXER_HXX
#include "EventMultiplexer.hxx"
#endif

#include "SlideSorterViewShell.hxx"

#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
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
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XDRAWVIEW_HPP_
#include <com/sun/star/drawing/XDrawView.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#include <vector>
#include <map>

class SdDrawDocument;
class SdPage;

namespace sd
{

class ViewShellBase;

namespace impl
{
    struct TransitionEffect;
}

class SlideTransitionPane : public Control
{
public:
    explicit SlideTransitionPane(
        ::Window * pParent,
        ViewShellBase & rBase,
        const Size& rMinSize,
        SdDrawDocument* pDoc );
    virtual ~SlideTransitionPane();

    virtual void Resize();

    void onSelectionChanged();
    void onChangeCurrentPage();

private:
    void updateLayout();
    void updateControls();
    void updateControlState();

    void updateSoundList();
    void openSoundFileDialog();

    impl::TransitionEffect getTransitionEffectFromControls() const;

    void applyToSelectedPages();
    void playCurrentEffect();

    void addListener();
    void removeListener();

    ::sd::slidesorter::SharedPageSelection getSelectedPages (void) const;

    DECL_LINK( ApplyToAllButtonClicked, void * );
    DECL_LINK( PlayButtonClicked, void * );
    DECL_LINK( SlideShowButtonClicked, void * );
    DECL_LINK( AutoPreviewClicked, void * );

    DECL_LINK( TransitionSelected, void * );
    DECL_LINK( AdvanceSlideRadioButtonToggled, void * );
    DECL_LINK( AdvanceTimeModified, void * );
    DECL_LINK( SpeedListBoxSelected, void * );
    DECL_LINK( SoundListBoxSelected, void * );
    DECL_LINK( LoopSoundBoxChecked, void * );
    DECL_LINK(EventMultiplexerListener, tools::EventMultiplexerEvent*);
    DECL_LINK(LateInitCallback, Timer*);

private:
    ViewShellBase &   mrBase;
    SdDrawDocument *  mpDrawDoc;
    Size              maMinSize;

    FixedLine    maFL_APPLY_TRANSITION;
    ListBox      maLB_SLIDE_TRANSITIONS;
    FixedLine    maFL_MODIFY_TRANSITION;
    FixedText    maFT_SPEED;
    ListBox      maLB_SPEED;
    FixedText    maFT_SOUND;
    ListBox      maLB_SOUND;
    CheckBox     maCB_LOOP_SOUND;
    FixedLine    maFL_ADVANCE_SLIDE;
    RadioButton  maRB_ADVANCE_ON_MOUSE;
    RadioButton  maRB_ADVANCE_AUTO;
    MetricField  maMF_ADVANCE_AUTO_AFTER;
    FixedLine    maFL_EMPTY1;
    PushButton   maPB_APPLY_TO_ALL;
    PushButton   maPB_PLAY;
    PushButton   maPB_SLIDE_SHOW;
    FixedLine    maFL_EMPTY2;
    CheckBox     maCB_AUTO_PREVIEW;

    String       maSTR_NO_TRANSITION;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawView >             mxView;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >                  mxModel;

    bool         mbHasSelection;
    bool         mbUpdatingControls;
    bool         mbIsMainViewChangePending;

    typedef ::std::vector< String > tSoundListType;
    tSoundListType  maSoundList;
    mutable String  maCurrentSoundFile;

    typedef ::std::map< sal_uInt16, sal_uInt16 > tPresetIndexesType;
    tPresetIndexesType m_aPresetIndexes;

    Timer maLateInitTimer;
};

} //  namespace sd

// SD_SLIDETRANSITIONPANE_HXX
#endif
