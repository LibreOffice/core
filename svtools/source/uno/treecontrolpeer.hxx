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

#ifndef INCLUDED_SVTOOLS_SOURCE_UNO_TREECONTROLPEER_HXX
#define INCLUDED_SVTOOLS_SOURCE_UNO_TREECONTROLPEER_HXX

#include <com/sun/star/awt/tree/XTreeControl.hpp>
#include <com/sun/star/awt/tree/XTreeDataModel.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxwindows.hxx>

#include <vcl/image.hxx>

#include <cppuhelper/implbase.hxx>

#include <tools/contnr.hxx>

#include <map>

namespace com { namespace sun { namespace star { namespace awt { namespace tree {

class XTreeNode;

}}}}}

class UnoTreeListEntry;
class TreeControlPeer;
class UnoTreeListBoxImpl;

class TreeControlPeer : public ::cppu::ImplInheritanceHelper< VCLXWindow, css::awt::tree::XTreeControl, css::awt::tree::XTreeDataModelListener >
{
    typedef std::map<css::uno::Reference<css::awt::tree::XTreeNode>, UnoTreeListEntry*> TreeNodeMap;

    friend class UnoTreeListBoxImpl;
    friend class UnoTreeListEntry;
public:
    TreeControlPeer();
    virtual ~TreeControlPeer();

    vcl::Window* createVclControl( vcl::Window* pParent, sal_Int64 nWinStyle );

    // css::view::XSelectionSupplier
    virtual sal_Bool SAL_CALL select( const css::uno::Any& xSelection ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getSelection(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

    // css::view::XMultiSelectionSupplier
    virtual sal_Bool SAL_CALL addSelection( const css::uno::Any& Selection ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeSelection( const css::uno::Any& Selection ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL clearSelection(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getSelectionCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createSelectionEnumeration(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createReverseSelectionEnumeration(  ) throw (css::uno::RuntimeException, std::exception) override;

    // css::awt::XTreeControl
    virtual OUString SAL_CALL getDefaultExpandedGraphicURL() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDefaultExpandedGraphicURL( const OUString& _defaultexpandedgraphicurl ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getDefaultCollapsedGraphicURL() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDefaultCollapsedGraphicURL( const OUString& _defaultcollapsedgraphicurl ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isNodeExpanded( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isNodeCollapsed( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL makeNodeVisible( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) throw (css::lang::IllegalArgumentException, css::awt::tree::ExpandVetoException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isNodeVisible( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL expandNode( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) throw (css::lang::IllegalArgumentException, css::awt::tree::ExpandVetoException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL collapseNode( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) throw (css::lang::IllegalArgumentException, css::awt::tree::ExpandVetoException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addTreeExpansionListener( const css::uno::Reference< css::awt::tree::XTreeExpansionListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTreeExpansionListener( const css::uno::Reference< css::awt::tree::XTreeExpansionListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::awt::tree::XTreeNode > SAL_CALL getNodeForLocation( ::sal_Int32 x, ::sal_Int32 y ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::awt::tree::XTreeNode > SAL_CALL getClosestNodeForLocation( ::sal_Int32 x, ::sal_Int32 y ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Rectangle SAL_CALL getNodeRect( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isEditing(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL stopEditing(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL cancelEditing(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL startEditingAtNode( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addTreeEditListener( const css::uno::Reference< css::awt::tree::XTreeEditListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTreeEditListener( const css::uno::Reference< css::awt::tree::XTreeEditListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;

    // css::awt::tree::TreeDataModelListener
    virtual void SAL_CALL treeNodesChanged( const css::awt::tree::TreeDataModelEvent& aEvent ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL treeNodesInserted( const css::awt::tree::TreeDataModelEvent& aEvent ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL treeNodesRemoved( const css::awt::tree::TreeDataModelEvent& aEvent ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL treeStructureChanged( const css::awt::tree::TreeDataModelEvent& aEvent ) throw (css::uno::RuntimeException, std::exception) override;

    // XEventListener
    void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XLayoutConstrains
    css::awt::Size SAL_CALL getMinimumSize() throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL getPreferredSize() throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

private:
    UnoTreeListEntry* getEntry( const css::uno::Reference< css::awt::tree::XTreeNode >& xNode, bool bThrow = true ) throw (css::lang::IllegalArgumentException );

    void disposeControl();

    bool onEditingEntry( UnoTreeListEntry* pEntry );
    bool onEditedEntry( UnoTreeListEntry* pEntry, const OUString& rNewText );

    void fillTree( UnoTreeListBoxImpl& rTree, const css::uno::Reference< css::awt::tree::XTreeDataModel >& xDataModel );
    void addNode( UnoTreeListBoxImpl& rTree, const css::uno::Reference< css::awt::tree::XTreeNode >& xNode, UnoTreeListEntry* pParentEntry );

    UnoTreeListEntry* createEntry( const css::uno::Reference< css::awt::tree::XTreeNode >& xNode, UnoTreeListEntry* pParent, sal_uLong nPos );
    void updateEntry( UnoTreeListEntry* pEntry );

    void updateTree( const css::awt::tree::TreeDataModelEvent& rEvent );
    void updateNode( UnoTreeListBoxImpl& rTree, const css::uno::Reference< css::awt::tree::XTreeNode >& xNode );
    void updateChildNodes( UnoTreeListBoxImpl& rTree, const css::uno::Reference< css::awt::tree::XTreeNode >& xParentNode, UnoTreeListEntry* pParentEntry );

    static OUString getEntryString( const css::uno::Any& rValue );

    UnoTreeListBoxImpl& getTreeListBoxOrThrow() const throw (css::uno::RuntimeException );
    void ChangeNodesSelection( const css::uno::Any& rSelection, bool bSelect, bool bSetSelection ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

    void onChangeDataModel( UnoTreeListBoxImpl& rTree, const css::uno::Reference< css::awt::tree::XTreeDataModel >& xDataModel );

    void onSelectionChanged();
    void onRequestChildNodes( const css::uno::Reference< css::awt::tree::XTreeNode >& xNode );
    bool onExpanding( const css::uno::Reference< css::awt::tree::XTreeNode >& xNode, bool bExpanding );
    void onExpanded( const css::uno::Reference< css::awt::tree::XTreeNode >& xNode, bool bExpanding );

    void onChangeRootDisplayed( bool bIsRootDisplayed );

    void addEntry( UnoTreeListEntry* pEntry );
    void removeEntry( UnoTreeListEntry* pEntry );

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
    TreeNodeMap* mpTreeNodeMap;
    css::uno::Reference< css::graphic::XGraphicProvider > mxGraphicProvider;
};

#endif // INCLUDED_SVTOOLS_SOURCE_UNO_TREECONTROLPEER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
