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

class TreeControlPeer : public ::cppu::ImplInheritanceHelper< VCLXWindow, ::com::sun::star::awt::tree::XTreeControl, ::com::sun::star::awt::tree::XTreeDataModelListener >
{
    typedef std::map<com::sun::star::uno::Reference<com::sun::star::awt::tree::XTreeNode>, UnoTreeListEntry*> TreeNodeMap;

    friend class UnoTreeListBoxImpl;
    friend class UnoTreeListEntry;
public:
    TreeControlPeer();
    virtual ~TreeControlPeer();

    vcl::Window* createVclControl( vcl::Window* pParent, sal_Int64 nWinStyle );

    // VCLXWindow
    virtual void SetWindow( const VclPtr< vcl::Window > &pWindow ) override;

    // ::com::sun::star::view::XSelectionSupplier
    virtual sal_Bool SAL_CALL select( const ::com::sun::star::uno::Any& xSelection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getSelection(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::view::XMultiSelectionSupplier
    virtual sal_Bool SAL_CALL addSelection( const ::com::sun::star::uno::Any& Selection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeSelection( const ::com::sun::star::uno::Any& Selection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL clearSelection(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getSelectionCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createSelectionEnumeration(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createReverseSelectionEnumeration(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XTreeControl
    virtual OUString SAL_CALL getDefaultExpandedGraphicURL() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDefaultExpandedGraphicURL( const OUString& _defaultexpandedgraphicurl ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getDefaultCollapsedGraphicURL() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDefaultCollapsedGraphicURL( const OUString& _defaultcollapsedgraphicurl ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isNodeExpanded( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isNodeCollapsed( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL makeNodeVisible( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::awt::tree::ExpandVetoException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isNodeVisible( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL expandNode( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::awt::tree::ExpandVetoException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL collapseNode( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::awt::tree::ExpandVetoException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addTreeExpansionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeExpansionListener >& Listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTreeExpansionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeExpansionListener >& Listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode > SAL_CALL getNodeForLocation( ::sal_Int32 x, ::sal_Int32 y ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode > SAL_CALL getClosestNodeForLocation( ::sal_Int32 x, ::sal_Int32 y ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getNodeRect( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isEditing(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL stopEditing(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL cancelEditing(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL startEditingAtNode( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addTreeEditListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeEditListener >& Listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTreeEditListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeEditListener >& Listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::tree::TreeDataModelListener
    virtual void SAL_CALL treeNodesChanged( const ::com::sun::star::awt::tree::TreeDataModelEvent& aEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL treeNodesInserted( const ::com::sun::star::awt::tree::TreeDataModelEvent& aEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL treeNodesRemoved( const ::com::sun::star::awt::tree::TreeDataModelEvent& aEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL treeStructureChanged( const ::com::sun::star::awt::tree::TreeDataModelEvent& aEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XEventListener
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    UnoTreeListEntry* getEntry( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& xNode, bool bThrow = true ) throw (::com::sun::star::lang::IllegalArgumentException );

    void disposeControl();

    bool onEditingEntry( UnoTreeListEntry* pEntry );
    bool onEditedEntry( UnoTreeListEntry* pEntry, const OUString& rNewText );

    void fillTree( UnoTreeListBoxImpl& rTree, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeDataModel >& xDataModel );
    void addNode( UnoTreeListBoxImpl& rTree, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& xNode, UnoTreeListEntry* pParentEntry );

    UnoTreeListEntry* createEntry( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& xNode, UnoTreeListEntry* pParent, sal_uLong nPos = TREELIST_APPEND );
    bool updateEntry( UnoTreeListEntry* pEntry );

    void updateTree( const ::com::sun::star::awt::tree::TreeDataModelEvent& rEvent, bool bRecursive );
    void updateNode( UnoTreeListBoxImpl& rTree, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& xNode, bool bRecursive );
    void updateChildNodes( UnoTreeListBoxImpl& rTree, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& xParentNode, UnoTreeListEntry* pParentEntry );

    static OUString getEntryString( const ::com::sun::star::uno::Any& rValue );

    UnoTreeListBoxImpl& getTreeListBoxOrThrow() const throw (::com::sun::star::uno::RuntimeException );
    void ChangeNodesSelection( const ::com::sun::star::uno::Any& rSelection, bool bSelect, bool bSetSelection ) throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

    void onChangeDataModel( UnoTreeListBoxImpl& rTree, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeDataModel >& xDataModel );

    void onSelectionChanged();
    void onRequestChildNodes( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& xNode );
    bool onExpanding( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& xNode, bool bExpanding );
    void onExpanded( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& xNode, bool bExpanding );

    void onChangeRootDisplayed( bool bIsRootDisplayed );

    void addEntry( UnoTreeListEntry* pEntry );
    void removeEntry( UnoTreeListEntry* pEntry );

    bool loadImage( const OUString& rURL, Image& rImage );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeDataModel >mxDataModel;
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
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphicProvider > mxGraphicProvider;
};

#endif // INCLUDED_SVTOOLS_SOURCE_UNO_TREECONTROLPEER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
