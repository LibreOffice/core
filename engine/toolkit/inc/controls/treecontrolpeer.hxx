/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <com/sun/star/awt/tree/XTreeControl.hpp>
#include <com/sun/star/awt/tree/XTreeDataModel.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>

#include <vcl/image.hxx>

#include <cppuhelper/implbase.hxx>

#include <map>

namespace com::sun::star::awt::tree { class XTreeNode; }

class UnoTreeListEntry;
class UnoTreeListBoxImpl;

class TreeControlPeer final : public ::cppu::ImplInheritanceHelper< VCLXWindow, css::awt::tree::XTreeControl, css::awt::tree::XTreeDataModelListener >
{
    typedef std::map<css::uno::Reference<css::awt::tree::XTreeNode>, UnoTreeListEntry*> TreeNodeMap;

    friend class UnoTreeListBoxImpl;
    friend class UnoTreeListEntry;
public:
    TreeControlPeer();
    virtual ~TreeControlPeer() override;

    vcl::Window* createVclControl( vcl::Window* pParent, sal_Int64 nWinStyle );

    // css::view::XSelectionSupplier
    virtual bool select( const cpo::uno::Any& xSelection ) override;
    virtual cpo::uno::Any getSelection(  ) override;
    virtual void addSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;
    virtual void removeSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;

    // css::view::XMultiSelectionSupplier
    virtual bool addSelection( const cpo::uno::Any& Selection ) override;
    virtual void removeSelection( const cpo::uno::Any& Selection ) override;
    virtual void clearSelection(  ) override;
    virtual ::sal_Int32 getSelectionCount(  ) override;
    virtual css::uno::Reference< css::container::XEnumeration > createSelectionEnumeration(  ) override;
    virtual css::uno::Reference< css::container::XEnumeration > createReverseSelectionEnumeration(  ) override;

    // css::awt::XTreeControl
    virtual OUString getDefaultExpandedGraphicURL() override;
    virtual void setDefaultExpandedGraphicURL( const OUString& _defaultexpandedgraphicurl ) override;
    virtual OUString getDefaultCollapsedGraphicURL() override;
    virtual void setDefaultCollapsedGraphicURL( const OUString& _defaultcollapsedgraphicurl ) override;
    virtual bool isNodeExpanded( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual bool isNodeCollapsed( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual void makeNodeVisible( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual bool isNodeVisible( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual void expandNode( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual void collapseNode( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual void addTreeExpansionListener( const css::uno::Reference< css::awt::tree::XTreeExpansionListener >& Listener ) override;
    virtual void removeTreeExpansionListener( const css::uno::Reference< css::awt::tree::XTreeExpansionListener >& Listener ) override;
    virtual css::uno::Reference< css::awt::tree::XTreeNode > getNodeForLocation( ::sal_Int32 x, ::sal_Int32 y ) override;
    virtual css::uno::Reference< css::awt::tree::XTreeNode > getClosestNodeForLocation( ::sal_Int32 x, ::sal_Int32 y ) override;
    virtual css::awt::Rectangle getNodeRect( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual bool isEditing(  ) override;
    virtual bool stopEditing(  ) override;
    virtual void cancelEditing(  ) override;
    virtual void startEditingAtNode( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual void addTreeEditListener( const css::uno::Reference< css::awt::tree::XTreeEditListener >& Listener ) override;
    virtual void removeTreeEditListener( const css::uno::Reference< css::awt::tree::XTreeEditListener >& Listener ) override;

    // css::awt::tree::TreeDataModelListener
    virtual void treeNodesChanged( const css::awt::tree::TreeDataModelEvent& aEvent ) override;
    virtual void treeNodesInserted( const css::awt::tree::TreeDataModelEvent& aEvent ) override;
    virtual void treeNodesRemoved( const css::awt::tree::TreeDataModelEvent& aEvent ) override;
    virtual void treeStructureChanged( const css::awt::tree::TreeDataModelEvent& aEvent ) override;

    // XEventListener
    void disposing( const css::lang::EventObject& Source ) override;

    // css::awt::XLayoutConstrains
    css::awt::Size getMinimumSize() override;
    css::awt::Size getPreferredSize() override;
    css::awt::Size calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    // css::awt::XVclWindowPeer
    void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    cpo::uno::Any getProperty( const OUString& PropertyName ) override;

private:
    /// @throws css::lang::IllegalArgumentException
    UnoTreeListEntry* getEntry( const css::uno::Reference< css::awt::tree::XTreeNode >& xNode, bool bThrow = true );

    void disposeControl();

    bool onEditingEntry( UnoTreeListEntry const * pEntry );
    bool onEditedEntry( UnoTreeListEntry const * pEntry, const OUString& rNewText );

    void fillTree( UnoTreeListBoxImpl& rTree, const css::uno::Reference< css::awt::tree::XTreeDataModel >& xDataModel );
    void addNode( UnoTreeListBoxImpl& rTree, const css::uno::Reference< css::awt::tree::XTreeNode >& xNode, UnoTreeListEntry* pParentEntry );

    UnoTreeListEntry* createEntry( const css::uno::Reference< css::awt::tree::XTreeNode >& xNode, UnoTreeListEntry* pParent, sal_uInt32 nPos );
    void updateEntry( UnoTreeListEntry* pEntry );

    void updateTree( const css::awt::tree::TreeDataModelEvent& rEvent );
    void updateNode( UnoTreeListBoxImpl const & rTree, const css::uno::Reference< css::awt::tree::XTreeNode >& xNode );
    void updateChildNodes( UnoTreeListBoxImpl const & rTree, const css::uno::Reference< css::awt::tree::XTreeNode >& xParentNode, UnoTreeListEntry* pParentEntry );

    static OUString getEntryString( const cpo::uno::Any& rValue );

    /// @throws cpo::uno::RuntimeException
    UnoTreeListBoxImpl& getTreeListBoxOrThrow() const;
    /// @throws cpo::uno::RuntimeException
    /// @throws css::lang::IllegalArgumentException
    void ChangeNodesSelection( const cpo::uno::Any& rSelection, bool bSelect, bool bSetSelection );

    void onChangeDataModel( UnoTreeListBoxImpl& rTree, const css::uno::Reference< css::awt::tree::XTreeDataModel >& xDataModel );

    void onSelectionChanged();
    void onRequestChildNodes( const css::uno::Reference< css::awt::tree::XTreeNode >& xNode );
    bool onExpanding( const css::uno::Reference< css::awt::tree::XTreeNode >& xNode, bool bExpanding );
    void onExpanded( const css::uno::Reference< css::awt::tree::XTreeNode >& xNode, bool bExpanding );

    void onChangeRootDisplayed( bool bIsRootDisplayed );

    void addEntry( UnoTreeListEntry* pEntry );
    void removeEntry( UnoTreeListEntry const * pEntry );

    bool loadImage( const OUString& rURL, Image& rImage );

private:
    css::uno::Reference< css::awt::tree::XTreeDataModel >mxDataModel;
    TreeSelectionListenerMultiplexer maSelectionListeners;
    TreeExpansionListenerMultiplexer maTreeExpansionListeners;
    TreeEditListenerMultiplexer maTreeEditListeners;
    bool mbIsRootDisplayed;
    VclPtr<UnoTreeListBoxImpl> mpTreeImpl;
    sal_Int32 mnEditLock;
    OUString msDefaultCollapsedGraphicURL;
    OUString msDefaultExpandedGraphicURL;
    Image maDefaultExpandedImage;
    Image maDefaultCollapsedImage;
    std::unique_ptr<TreeNodeMap> mpTreeNodeMap;
    css::uno::Reference< css::graphic::XGraphicProvider > mxGraphicProvider;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
