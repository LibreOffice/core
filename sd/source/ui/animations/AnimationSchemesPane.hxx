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
#ifndef SD_ANIMATIONSCHEMESPANE_HXX
#define SD_ANIMATIONSCHEMESPANE_HXX

#include <vcl/ctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/field.hxx>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/frame/XModel.hpp>

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
