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

#include <vcl/treelistbox.hxx>
#include <CustomAnimationEffect.hxx>

namespace com { namespace sun { namespace star { namespace drawing { class XShape; } } } }

class VclBuilder;

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

class CustomAnimationList : public SvTreeListBox, public ISequenceListener
{
    friend class CustomAnimationListEntryItem;
    friend struct stl_append_effect_func;

public:
    explicit CustomAnimationList( vcl::Window* pParent );
    virtual ~CustomAnimationList() override;
    virtual void dispose() override;

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

    // overrides
    virtual void    SelectHdl() override;
    virtual void    DeselectHdl() override;
    virtual bool    DoubleClickHdl() override;

    virtual void    Paint( vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect ) override;

    virtual VclPtr<PopupMenu> CreateContextMenu() override;
    virtual void    ExecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry ) override;

    virtual void KeyInput( const KeyEvent& rKEvt ) override;

    virtual void notify_change() override;

    virtual bool Expand( SvTreeListEntry* pParent ) override;
    virtual bool Collapse( SvTreeListEntry* pParent ) override;
    bool isExpanded( const CustomAnimationEffectPtr& pEffect ) const;
    bool isVisible( const CustomAnimationEffectPtr& pEffect ) const;

    // clears all entries from the listbox
    void clear();

    void setController( ICustomAnimationListController* pController )
    {
        mpController = pController;
    };


protected:
    // drag & drop
    virtual void         StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;
    virtual DragDropMode NotifyStartDrag( TransferDataContainer& rData, SvTreeListEntry* pEntry ) override;
    virtual sal_Int8     AcceptDrop( const AcceptDropEvent& rEvt ) override;
            void         ReparentChildrenDuringDrag();
            void         ReorderEffectsInUiDuringDragOver( SvTreeListEntry* pOverEntry);
    virtual sal_Int8     ExecuteDrop( const ExecuteDropEvent& rEvt ) override;
    virtual void         DragFinished( sal_Int8 nDropAction ) override;

private:
    std::unique_ptr<VclBuilder> mxBuilder;
    VclPtr<PopupMenu> mxMenu;

    bool    mbIgnorePaint;

    /** appends the given effect to the list*/
    void append( CustomAnimationEffectPtr pEffect );

    ICustomAnimationListController* mpController;

    MainSequencePtr mpMainSequence;

    css::uno::Reference< css::drawing::XShape > mxLastTargetShape;
    sal_Int32 mnLastGroupId;
    SvTreeListEntry* mpLastParentEntry;

    // drag & drop
    SvTreeListEntry* mpDndEffectDragging;
    SvTreeListEntry* mpDndEffectInsertBefore;
    std::vector< SvTreeListEntry* > mDndEffectsSelected;
};

OUString getPropertyName( sal_Int32 nPropertyType );

OUString getShapeDescription( const css::uno::Reference< css::drawing::XShape >& xShape, bool bWithText );

}

#endif // INCLUDED_SD_SOURCE_UI_ANIMATIONS_CUSTOMANIMATIONLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
