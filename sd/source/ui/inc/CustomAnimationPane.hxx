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

#ifndef INCLUDED_SD_SOURCE_UI_ANIMATIONS_CUSTOMANIMATIONPANE_HXX
#define INCLUDED_SD_SOURCE_UI_ANIMATIONS_CUSTOMANIMATIONPANE_HXX

#include <sfx2/sidebar/ILayoutableWindow.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include "CustomAnimationList.hxx"
#include <misc/scopelock.hxx>

#include <vector>

namespace com::sun::star::drawing { class XDrawPage; }
namespace com::sun::star::drawing { class XDrawView; }
namespace weld { class ComboBox; }
namespace com::sun::star::animations { class XAnimationNode; }
namespace sd::tools { class EventMultiplexerEvent; }

enum class PathKind { NONE, CURVE, POLYGON, FREEFORM };

namespace sd {

class MotionPathTag;
class SdPropertySubControl;
class STLPropertySet;
class ViewShellBase;

typedef std::vector< rtl::Reference< MotionPathTag > > MotionPathTagVector;

class CustomAnimationPane : public PanelLayout
                          , public sfx2::sidebar::ILayoutableWindow
                          , public ICustomAnimationListController
{
    friend class MotionPathTag;
public:
    CustomAnimationPane( vcl::Window* pParent, ViewShellBase& rBase, const css::uno::Reference<css::frame::XFrame>& rxFrame );
    virtual ~CustomAnimationPane() override;
    virtual void dispose() override;

    // ILayoutableWindow
    virtual css::ui::LayoutSize GetHeightForWidth (const sal_Int32 nWidth) override;

    // callbacks
    void onSelectionChanged();
    void onChangeCurrentPage();
    void onAdd();
    void onRemove();
    void onChangeStart();
    void onChangeStart( sal_Int16 nNodeType );
    void onChangeSpeed();

    // methods
    void preview( const css::uno::Reference< css::animations::XAnimationNode >& xAnimationNode );
    void remove( CustomAnimationEffectPtr const & pEffect );

    // Control
    virtual void StateChanged( StateChangedType nStateChange ) override;
    virtual void KeyInput( const KeyEvent& rKEvt ) override;

    // ICustomAnimationListController
    virtual void onSelect() override;
    virtual void onDoubleClick() override;
    virtual void onContextMenu(const OString& rIdent) override;
    virtual void onDragNDropComplete( std::vector< CustomAnimationEffectPtr > pEffectsDragged, CustomAnimationEffectPtr pEffectInsertBefore ) override;

    // Window
    virtual void DataChanged (const DataChangedEvent& rEvent) override;

    void addUndo();

    double getDuration() const;
    void updatePathFromMotionPathTag( const rtl::Reference< MotionPathTag >& xTag );

private:
    void initialize();
    void addListener();
    void removeListener();
    void updateControls();
    void updateMotionPathTags();

    void showOptions(const OString& sPage = OString());
    void moveSelection( bool bUp );
    void onPreview( bool bForcePreview );

    std::unique_ptr<STLPropertySet> createSelectionSet();
    void changeSelection( STLPropertySet const * pResultSet, STLPropertySet const * pOldSet );

    static css::uno::Any getProperty1Value( sal_Int32 nType, const CustomAnimationEffectPtr& pEffect );
    static bool setProperty1Value( sal_Int32 nType, const CustomAnimationEffectPtr& pEffect, const css::uno::Any& rValue );
    void UpdateLook();
    sal_Int32 fillAnimationLB( bool bHasText );
    PathKind getCreatePathKind() const;
    void createPath( PathKind eKind, std::vector< ::com::sun::star::uno::Any >& rTargets, double fDuration );

    DECL_LINK( implControlListBoxHdl, weld::ComboBox&, void );
    DECL_LINK( implClickHdl, weld::Button&, void );
    DECL_LINK( implPropertyHdl, LinkParamNone*, void );
    DECL_LINK( EventMultiplexerListener, tools::EventMultiplexerEvent&, void );
    DECL_LINK( lateInitCallback, Timer *, void );
    DECL_LINK( DurationModifiedHdl, weld::MetricSpinButton&, void );
    DECL_LINK( DelayModifiedHdl, weld::MetricSpinButton&, void );
    DECL_LINK( DelayLoseFocusHdl, weld::Widget&, void );
    DECL_LINK( UpdateAnimationLB, weld::ComboBox&, void );
    DECL_LINK( AnimationSelectHdl, weld::TreeView&, void );
    DECL_LINK( SelectionHandler, Timer*, void );
    void implControlHdl(const weld::Widget* pControl);

private:
    ViewShellBase& mrBase;

    // UI Elements
    std::unique_ptr<weld::Label> mxFTAnimation;
    std::unique_ptr<CustomAnimationList> mxCustomAnimationList;
    std::unique_ptr<weld::Button> mxPBAddEffect;
    std::unique_ptr<weld::Button> mxPBRemoveEffect;
    std::unique_ptr<weld::Button> mxPBMoveUp;
    std::unique_ptr<weld::Button> mxPBMoveDown;
    std::unique_ptr<weld::Label> mxFTCategory;
    std::unique_ptr<weld::ComboBox> mxLBCategory;
    std::unique_ptr<weld::Label> mxFTEffect;
    std::unique_ptr<weld::TreeView> mxLBAnimation;
    std::unique_ptr<weld::Label> mxFTStart;
    std::unique_ptr<weld::ComboBox> mxLBStart;
    std::unique_ptr<weld::Label> mxFTProperty;
    std::unique_ptr<SdPropertySubControl> mxLBSubControl;
    std::unique_ptr<weld::Container> mxPlaceholderBox;
    std::unique_ptr<weld::Button> mxPBPropertyMore;
    std::unique_ptr<weld::Label> mxFTDuration;
    std::unique_ptr<weld::MetricSpinButton> mxCBXDuration;
    std::unique_ptr<weld::Label> mxFTStartDelay;
    std::unique_ptr<weld::MetricSpinButton> mxMFStartDelay;
    std::unique_ptr<weld::CheckButton> mxCBAutoPreview;
    std::unique_ptr<weld::Button> mxPBPlay;

    Idle maIdle;

    OUString    maStrModify;
    OUString    maStrProperty;

    sal_Int32   mnLastSelectedAnimation;
    sal_Int32   mnPropertyType;
    static sal_Int32 const gnMotionPathPos = 3;
    sal_Int32   mnCurvePathPos;
    sal_Int32   mnPolygonPathPos;
    sal_Int32   mnFreeformPathPos;

    EffectSequence  maListSelection;
    css::uno::Any   maViewSelection;

    MainSequencePtr mpMainSequence;

    css::uno::Reference< css::drawing::XDrawPage > mxCurrentPage;
    css::uno::Reference< css::drawing::XDrawView > mxView;

    /** The CustomAnimationPresets is initialized either on demand or
        after a short time after the construction of a new object of this
        class.  This timer is responsible for the later.
    */
    Timer maLateInitTimer;

    MotionPathTagVector maMotionPathTags;

    ScopeLock maSelectionLock;
};

void fillRepeatComboBox(weld::ComboBox& rBox);

}

#endif // INCLUDED_SD_SOURCE_UI_ANIMATIONS_CUSTOMANIMATIONPANE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
