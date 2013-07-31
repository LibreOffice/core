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

#ifndef _SD_CUSTOMANIMATIONPANE_HXX
#define _SD_CUSTOMANIMATIONPANE_HXX

#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <vcl/dialog.hxx>
#include <vcl/layout.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include "CustomAnimationPreset.hxx"
#include "CustomAnimationList.hxx"
#include "CustomAnimationCreateDialog.hxx"

#include "motionpathtag.hxx"
#include "misc/scopelock.hxx"

#include <vector>

class PushButton;
class FixedLine;
class FixedText;
class ListBox;
class ComboBox;
class CheckBox;

namespace com { namespace sun { namespace star { namespace animations {
    class XAnimationNode;
} } } }

namespace sd { namespace tools {
class EventMultiplexerEvent;
} }

namespace sd {

class PropertyControl;
class STLPropertySet;
class ViewShellBase;

typedef std::vector< rtl::Reference< MotionPathTag > > MotionPathTagVector;

// --------------------------------------------------------------------

class CustomAnimationPane : public PanelLayout, public ICustomAnimationListController
{
    friend class MotionPathTag;
public:
    CustomAnimationPane( ::Window* pParent, ViewShellBase& rBase, const cssu::Reference<css::frame::XFrame>& rxFrame, const Size& rMinSize );
    virtual ~CustomAnimationPane();

    // callbacks
    void onSelectionChanged();
    void onChangeCurrentPage();
    void onChange( bool bCreate );
    void onRemove();
    void onChangeStart();
    void onChangeStart( sal_Int16 nNodeType );
    void onChangeProperty();
    void onChangeSpeed();

    // methods
    void preview( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xAnimationNode );
    void remove( CustomAnimationEffectPtr& pEffect );

    // Control
    virtual void StateChanged( StateChangedType nStateChange );
    virtual void KeyInput( const KeyEvent& rKEvt );

    // ICustomAnimationListController
    virtual void onSelect();
    virtual void onDoubleClick();
    virtual void onContextMenu( sal_uInt16 nSelectedPopupEntry );

    // Window
    virtual void DataChanged (const DataChangedEvent& rEvent);

    void addUndo();

    void updatePathFromMotionPathTag( const rtl::Reference< MotionPathTag >& xTag );

private:
    void addListener();
    void removeListener();
    void updateControls();
    void updateMotionPathTags();
    void markShapesFromSelectedEffects();

    void showOptions( sal_uInt16 nPage = 0 );
    void moveSelection( bool bUp );
    void onPreview( bool bForcePreview );

    void createPath( PathKind eKind, std::vector< ::com::sun::star::uno::Any >& rTargets, double fDuration );

    STLPropertySet* createSelectionSet();
    void changeSelection( STLPropertySet* pResultSet, STLPropertySet* pOldSet );

    ::com::sun::star::uno::Any getProperty1Value( sal_Int32 nType, CustomAnimationEffectPtr pEffect );
    bool setProperty1Value( sal_Int32 nType, CustomAnimationEffectPtr pEffect, const ::com::sun::star::uno::Any& rValue );
    void UpdateLook (void);

    DECL_LINK( implControlHdl, Control* );
    DECL_LINK(implPropertyHdl, void *);
    DECL_LINK(EventMultiplexerListener, tools::EventMultiplexerEvent*);
    DECL_LINK(lateInitCallback, void *);

private:
    ViewShellBase& mrBase;

    const CustomAnimationPresets* mpCustomAnimationPresets;

    PushButton* mpPBAddEffect;
    PushButton* mpPBChangeEffect;
    PushButton* mpPBRemoveEffect;
    FixedText*  mpFTEffect;
    FixedText*  mpFTStart;
    ListBox*    mpLBStart;
    FixedText*  mpFTProperty;
    VclHBox*    mpPlaceholderBox;
    PropertyControl*    mpLBProperty;
    PushButton* mpPBPropertyMore;
    FixedText*  mpFTSpeed;
    ListBox*   mpCBSpeed;
    CustomAnimationList*    mpCustomAnimationList;
    FixedText*  mpFTChangeOrder;
    PushButton* mpPBMoveUp;
    PushButton* mpPBMoveDown;
    PushButton* mpPBPlay;
    PushButton* mpPBSlideShow;
    CheckBox*   mpCBAutoPreview;

    String      maStrModify;
    String      maStrProperty;

    sal_Int32   mnPropertyType;

    Size        maMinSize;

    EffectSequence maListSelection;
    ::com::sun::star::uno::Any maViewSelection;

    MainSequencePtr mpMainSequence;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > mxCurrentPage;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawView > mxView;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > mxModel;

    /** The mpCustomAnimationPresets is initialized either on demand or
        after a short time after the construction of a new object of this
        class.  This timer is responsible for the later.
    */
    Timer maLateInitTimer;

    /** This method initializes the mpCustomAnimationPresets on demand and
        returns a reference to the list.
    */
    const CustomAnimationPresets& getPresets (void);

    MotionPathTagVector maMotionPathTags;

    ScopeLock maSelectionLock;
};

}

#endif // _SD_CUSTOMANIMATIONPANE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
