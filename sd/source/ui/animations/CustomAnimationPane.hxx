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

#include <vcl/layout.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include "CustomAnimationDialog.hxx"
#include "CustomAnimationList.hxx"
#include "CategoryListBox.hxx"
#include "motionpathtag.hxx"
#include <misc/scopelock.hxx>

#include <vector>

namespace com { namespace sun { namespace star { namespace drawing { class XDrawPage; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XDrawView; } } } }
namespace sd { class CustomAnimationPresets; }

namespace weld { class ComboBox; }

enum class PathKind { NONE, CURVE, POLYGON, FREEFORM };

namespace com { namespace sun { namespace star { namespace animations {
    class XAnimationNode;
} } } }

namespace sd { namespace tools {
class EventMultiplexerEvent;
} }

namespace sd {

class STLPropertySet;
class ViewShellBase;

typedef std::vector< rtl::Reference< MotionPathTag > > MotionPathTagVector;

class CustomAnimationPane : public PanelLayout, public ICustomAnimationListController
{
    friend class MotionPathTag;
public:
    CustomAnimationPane( vcl::Window* pParent, ViewShellBase& rBase, const css::uno::Reference<css::frame::XFrame>& rxFrame );
    CustomAnimationPane( vcl::Window* pParent, ViewShellBase& rBase, const css::uno::Reference<css::frame::XFrame>& rxFrame, bool bHorizontal );
    virtual ~CustomAnimationPane() override;
    virtual void dispose() override;

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

    double getDuration();
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
    bool setProperty1Value( sal_Int32 nType, const CustomAnimationEffectPtr& pEffect, const css::uno::Any& rValue );
    void UpdateLook();
    sal_uInt32 fillAnimationLB( bool bHasText );
    PathKind getCreatePathKind() const;
    void createPath( PathKind eKind, std::vector< ::com::sun::star::uno::Any >& rTargets, double fDuration );

    DECL_LINK( implControlListBoxHdl, ListBox&, void );
    DECL_LINK( implClickHdl, Button*, void );
    DECL_LINK( implPropertyHdl, LinkParamNone*, void );
    DECL_LINK( EventMultiplexerListener, tools::EventMultiplexerEvent&, void );
    DECL_LINK( lateInitCallback, Timer *, void );
    DECL_LINK( DurationModifiedHdl, Edit&, void );
    DECL_LINK( DelayModifiedHdl, Edit&, void );
    DECL_LINK( DelayLoseFocusHdl, Control&, void );
    DECL_LINK( UpdateAnimationLB, ListBox&, void );
    DECL_LINK( AnimationSelectHdl, ListBox&, void );
    void implControlHdl(Control const *);

private:
    ViewShellBase& mrBase;

    const CustomAnimationPresets* mpCustomAnimationPresets;

    // UI Elements
    VclPtr<FixedText>   mpFTAnimation;
    VclPtr<CustomAnimationList> mpCustomAnimationList;
    VclPtr<PushButton>  mpPBAddEffect;
    VclPtr<PushButton>  mpPBRemoveEffect;
    VclPtr<PushButton>  mpPBMoveUp;
    VclPtr<PushButton>  mpPBMoveDown;
    VclPtr<FixedText>   mpFTCategory;
    VclPtr<ListBox>     mpLBCategory;
    VclPtr<FixedText>   mpFTEffect;
    VclPtr<CategoryListBox> mpLBAnimation;
    VclPtr<FixedText>   mpFTStart;
    VclPtr<ListBox>     mpLBStart;
    VclPtr<FixedText>   mpFTProperty;
    VclPtr<PropertyControl> mpLBProperty;
    VclPtr<VclHBox>     mpPlaceholderBox;
    VclPtr<PushButton>  mpPBPropertyMore;
    VclPtr<FixedText>   mpFTDuration;
    VclPtr<MetricBox>   mpCBXDuration;
    VclPtr<FixedText>   mpFTStartDelay;
    VclPtr<MetricField> mpMFStartDelay;
    VclPtr<CheckBox>    mpCBAutoPreview;
    VclPtr<PushButton>  mpPBPlay;

    OUString    maStrModify;
    OUString    maStrProperty;

    sal_Int32   mnPropertyType;
    static sal_Int32 const gnMotionPathPos = 3;
    sal_Int32   mnCurvePathPos;
    sal_Int32   mnPolygonPathPos;
    sal_Int32   mnFreeformPathPos;

    bool const      mbHorizontal;

    EffectSequence  maListSelection;
    css::uno::Any   maViewSelection;

    MainSequencePtr mpMainSequence;

    css::uno::Reference< css::drawing::XDrawPage > mxCurrentPage;
    css::uno::Reference< css::drawing::XDrawView > mxView;

    /** The mpCustomAnimationPresets is initialized either on demand or
        after a short time after the construction of a new object of this
        class.  This timer is responsible for the later.
    */
    Timer maLateInitTimer;

    /** This method initializes the mpCustomAnimationPresets on demand and
        returns a reference to the list.
    */
    const CustomAnimationPresets& getPresets();

    MotionPathTagVector maMotionPathTags;

    ScopeLock maSelectionLock;
};

void fillRepeatComboBox(weld::ComboBox& rBox);

}

#endif // INCLUDED_SD_SOURCE_UI_ANIMATIONS_CUSTOMANIMATIONPANE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
