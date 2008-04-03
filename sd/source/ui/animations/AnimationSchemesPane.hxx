/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AnimationSchemesPane.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:25:05 $
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
#ifndef SD_ANIMATIONSCHEMESPANE_HXX
#define SD_ANIMATIONSCHEMESPANE_HXX

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

namespace sd { namespace tools {
class EventMultiplexerEvent;
} }

namespace sd
{

class ViewShellBase;

class AnimationSchemesPane : public Control
{
public:
    explicit AnimationSchemesPane(
        ::Window * pParent,
        ViewShellBase & rBase,
        SdDrawDocument* pDoc );
    virtual ~AnimationSchemesPane();

    virtual void Resize();

    void onSelectionChanged();
    void onChangeCurrentPage();

private:
    void updateLayout();
    void updateControls();
    void updateControlState();

    void applyToSelectedPages();
    void playCurrentScheme();

    void addListener();
    void removeListener();

    void getAnimationSchemeFromControls() const;

    DECL_LINK( ApplyToAllButtonClicked, void * );
    DECL_LINK( PlayButtonClicked, void * );
    DECL_LINK( SlideShowButtonClicked, void * );

    DECL_LINK( SchemeSelected, void * );
    DECL_LINK(EventMultiplexerListener, tools::EventMultiplexerEvent*);

private:
    ViewShellBase &   mrBase;
    SdDrawDocument *  mpDrawDoc;

    FixedLine    maFL_APPLY_SCHEME;
    ListBox      maLB_ANIMATION_SCHEMES;
    FixedLine    maFL_EMPTY1;
    PushButton   maPB_APPLY_TO_ALL;
    PushButton   maPB_PLAY;
    PushButton   maPB_SLIDE_SHOW;
    FixedLine    maFL_EMPTY2;
    CheckBox     maCB_AUTO_PREVIEW;

    String       maSTR_NO_SCHEME;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawView >             mxView;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >                  mxModel;

    bool         mbHasSelection;
    bool         mbUpdatingControls;
    bool         mbIsMainViewChangePending;
};

} //  namespace sd

// SD_ANIMATIONSCHEMESPANE_HXX
#endif
