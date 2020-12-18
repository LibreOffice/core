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

#ifndef INCLUDED_SD_SOURCE_UI_ANIMATIONS_CUSTOMANIMATIONLIST_HXX
#define INCLUDED_SD_SOURCE_UI_ANIMATIONS_CUSTOMANIMATIONLIST_HXX

#include <sal/config.h>

#include <memory>

#include <vcl/transfer.hxx>
#include <vcl/weld.hxx>
#include <CustomAnimationEffect.hxx>

namespace com::sun::star::drawing { class XShape; }

struct ImplSVEvent;

namespace sd {

typedef std::shared_ptr< CustomAnimationEffect > CustomAnimationEffectPtr;

class ICustomAnimationListController
{
public:
    virtual void onSelect() = 0;
    virtual void onDoubleClick() = 0;
    virtual void onContextMenu(const OString &rIdent) = 0;
    virtual void onDragNDropComplete( std::vector< CustomAnimationEffectPtr > pEffectsDragged, CustomAnimationEffectPtr pEffectInsertBefore ) = 0;
    virtual ~ICustomAnimationListController() {}
};

class CustomAnimationList;
class CustomAnimationListEntryItem;

class CustomAnimationListDropTarget : public DropTargetHelper
{
private:
    CustomAnimationList& m_rTreeView;

    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

public:
    CustomAnimationListDropTarget(CustomAnimationList& rTreeView);
};

class CustomAnimationList : public ISequenceListener
{
    friend class CustomAnimationListEntryItem;
    friend struct stl_append_effect_func;

public:
    explicit CustomAnimationList(std::unique_ptr<weld::TreeView> xTreeView,
                                 std::unique_ptr<weld::Label> xLabel,
                                 std::unique_ptr<weld::Widget> xScrolledWindow);
    virtual ~CustomAnimationList();

    // methods

    /** selects or deselects the given effect.
        Selections of other effects are not changed */
    void select( const CustomAnimationEffectPtr& pEffect );

    /** populates the list with all effects from the given MainSequence */
    void update( const MainSequencePtr& pMainSequence );

    void update();

    EffectSequence getSelection() const;

    // events
    void onSelectionChanged(const css::uno::Any& rSelection);

    void Select();

    virtual void notify_change() override;

    bool isExpanded( const CustomAnimationEffectPtr& pEffect ) const;
    bool isVisible( const CustomAnimationEffectPtr& pEffect ) const;

    // clears all entries from the listbox
    void clear();

    void setController( ICustomAnimationListController* pController )
    {
        mpController = pController;
    };

    sal_Int8     AcceptDrop(const AcceptDropEvent& rEvt);
    sal_Int8     ExecuteDrop(const ExecuteDropEvent& rEvt);

    void set_sensitive(bool bSensitive) { mxTreeView->set_sensitive(bSensitive); }
    int get_height_rows(int nRows) { return mxTreeView->get_height_rows(nRows); }
    int get_approximate_digit_width() const { return mxTreeView->get_approximate_digit_width(); }
    void set_size_request(int nWidth, int nHeight) { mxTreeView->set_size_request(nWidth, nHeight); }
    void unselect_all() { mxTreeView->unselect_all(); }
    weld::TreeView& get_widget() { return *mxTreeView; }

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(ExpandHdl, const weld::TreeIter&, bool);
    DECL_LINK(PostExpandHdl, void*, void);
    DECL_LINK(CollapseHdl, const weld::TreeIter&, bool);
    DECL_LINK(PostCollapseHdl, void*, void);

private:
    std::unique_ptr<weld::TreeView> mxTreeView;
    CustomAnimationListDropTarget maDropTargetHelper;
    std::unique_ptr<weld::Label> mxEmptyLabel;
    std::unique_ptr<weld::Widget> mxEmptyLabelParent;
    std::vector<std::unique_ptr<CustomAnimationListEntryItem>> mxEntries;
    std::vector<std::unique_ptr<weld::TreeIter>> lastSelectedEntries;

    bool    mbIgnorePaint;

    DECL_LINK(SelectHdl, weld::TreeView&, void);
    DECL_LINK(CommandHdl, const CommandEvent&, bool);
    DECL_LINK(DoubleClickHdl, weld::TreeView&, bool);
    DECL_LINK(DragBeginHdl, bool&, bool);
    DECL_STATIC_LINK(CustomAnimationList, CustomRenderHdl, weld::TreeView::render_args, void);
    DECL_STATIC_LINK(CustomAnimationList, CustomGetSizeHdl, weld::TreeView::get_size_args, Size);

    void ExecuteContextMenuAction(const OString& rSelectedPopupEntry);

    /** appends the given effect to the list*/
    void append( CustomAnimationEffectPtr pEffect );

    ICustomAnimationListController* mpController;

    MainSequencePtr mpMainSequence;

    css::uno::Reference< css::drawing::XShape > mxLastTargetShape;
    sal_Int32 mnLastGroupId;
    ImplSVEvent* mnPostExpandEvent;
    ImplSVEvent* mnPostCollapseEvent;

    std::unique_ptr<weld::TreeIter> mxLastParentEntry;

    // drag & drop
    std::unique_ptr<weld::TreeIter> mxDndEffectDragging;
    std::vector<std::unique_ptr<weld::TreeIter>> mDndEffectsSelected;
};

OUString getPropertyName( sal_Int32 nPropertyType );

OUString getShapeDescription( const css::uno::Reference< css::drawing::XShape >& xShape, bool bWithText );

}

#endif // INCLUDED_SD_SOURCE_UI_ANIMATIONS_CUSTOMANIMATIONLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
