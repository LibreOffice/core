/*************************************************************************
 *
 *  $RCSfile: SlideTransitionPane.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:56:48 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef SD_SLIDETRANSITIONPANE_HXX
#define SD_SLIDETRANSITIONPANE_HXX

#ifndef SD_TOOLS_EVENT_MULTIPLEXER_HXX
#include "EventMultiplexer.hxx"
#endif

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

    ::std::vector< SdPage * > getSelectedPages();

    DECL_LINK( ApplyToAllButtonClicked, void * );
    DECL_LINK( PlayButtonClicked, void * );
    DECL_LINK( SlideShowButtonClicked, void * );

    DECL_LINK( TransitionSelected, void * );
    DECL_LINK( AdvanceSlideRadioButtonToggled, void * );
    DECL_LINK( AdvanceTimeModified, void * );
    DECL_LINK( SpeedListBoxSelected, void * );
    DECL_LINK( SoundListBoxSelected, void * );
    DECL_LINK( LoopSoundBoxChecked, void * );
    DECL_LINK(EventMultiplexerListener, tools::EventMultiplexerEvent*);

private:
    ViewShellBase &   mrBase;
    SdDrawDocument *  mpDrawDoc;

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

    typedef ::std::vector< String > tSoundListType;
    tSoundListType  maSoundList;
    mutable String  maCurrentSoundFile;
};

} //  namespace sd

// SD_SLIDETRANSITIONPANE_HXX
#endif
