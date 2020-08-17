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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_APP_APPDETAILPAGEHELPER_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_APP_APPDETAILPAGEHELPER_HXX

#include <vector>

#include <rtl/ustring.hxx>
#include <com/sun/star/sdb/application/NamedDatabaseObject.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <AppElementType.hxx>
#include <sfx2/weldutils.hxx>
#include <vcl/InterimItemWindow.hxx>
#include <vcl/fixed.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/graph.hxx>
#include <vcl/GraphicObject.hxx>
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>
#include "DocumentInfoPreview.hxx"

namespace com::sun::star::awt   { class XWindow; }
namespace com::sun::star::frame { class XFrame2; }
namespace com::sun::star::io    { class XPersist; }

#define ELEMENT_COUNT   size_t(E_ELEMENT_TYPE_COUNT)

namespace dbaui
{
    class OAppBorderWindow;
    class ODocumentInfoPreview;
    class DBTreeViewBase;
    class TreeListBox;

    class OPreviewWindow final : public weld::CustomWidgetController
    {
        GraphicObject       m_aGraphicObj;
        tools::Rectangle    m_aPreviewRect;

        /** gets the graphic center rect
            @param  rRenderContext
                the context to which we are drawing
            @param  rGraphic
                the graphic
            @param  rResultRect
                the resulting rectangle

            @return
                <TRUE/> when successful
        */
        bool ImplGetGraphicCenterRect(const vcl::RenderContext& rRenderContext, const Graphic& rGraphic, tools::Rectangle& rResultRect) const;

    public:
        OPreviewWindow();

        virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

        void setGraphic(const Graphic& _rGraphic ) { m_aGraphicObj.SetGraphic(_rGraphic); }
    };

    // A helper class for the controls in the detail page.
    // Combines general functionality.
    class OAppDetailPageHelper final : public InterimItemWindow
    {
        std::unique_ptr<DBTreeViewBase> m_aLists[ELEMENT_COUNT];
        OAppBorderWindow&         m_rBorderWin;
        std::unique_ptr<weld::Container> m_xBox;
        std::unique_ptr<weld::Widget> m_xFL;
        std::unique_ptr<weld::MenuButton> m_xMBPreview;

        std::unique_ptr<OPreviewWindow> m_xPreview;
        std::unique_ptr<weld::CustomWeld> m_xPreviewWin;

        std::unique_ptr<ODocumentInfoPreview> m_xDocumentInfo;
        std::unique_ptr<weld::CustomWeld> m_xDocumentInfoWin;

        std::unique_ptr<weld::Container> m_xTablePreview;

        PreviewMode               m_ePreviewMode;
        css::uno::Reference < css::frame::XFrame2 >
                                  m_xFrame;
        css::uno::Reference< css::awt::XWindow >
                                  m_xWindow;

        /// returns the index of the visible control
        int getVisibleControlIndex() const;

        /** sorts the entries in the tree list box.
            @param  nPos
                Which list should be sorted.
            @param  bAscending
                If sort should be Ascending of Descending
        */
        void sort(int nPos, bool bAscending);

        /** retrieves the resource ids of the images representing elements of the given type
        */
        static OUString getElementIcons(ElementType _eType);

        /** fills the names in the listbox
            @param  _xContainer
                This can either be the queries, forms or report names.
            @param  _eType
                the type of elements which are being filled
            @param _nImageId
                the resource id of the image to use for non-container entries
            @param  _pParent
                The parent of the entries to be inserted.
        */
        void fillNames( const css::uno::Reference< css::container::XNameAccess >& _xContainer,
                        const ElementType _eType,
                        const OUString& rImageId,
                        weld::TreeIter* _pParent );

        /** sets the detail page
            @param  rTreeView
                The control which should be visible.
        */
        void setDetailPage(DBTreeViewBase& rTreeView);

        /** sets all HandleCallbacks
            @param  rTreeView
                The newly created DBTreeViewBase
        */
        void setupTree(DBTreeViewBase& rTreeView);

        /** creates the tree and sets all HandleCallbacks
            @param  nHelpId
                The help id of the control
            @param  eType
                The element type of the control
            @return
                The new tree.
        */
        std::unique_ptr<DBTreeViewBase> createSimpleTree(const OString& rHelpId, ElementType eType);

        DECL_LINK( OnEntryDoubleClick,    weld::TreeView&, bool );
        DECL_LINK( OnEntrySelChange,      LinkParamNone*, void );

        DECL_LINK( OnCopyEntry,           LinkParamNone*, void );
        DECL_LINK( OnPasteEntry,          LinkParamNone*, void );
        DECL_LINK( OnDeleteEntry,         LinkParamNone*, void );

        DECL_LINK(OnDropdownClickHdl, weld::ToggleButton&, void);
        DECL_LINK(MenuSelectHdl, const OString&, void);

        OAppBorderWindow& getBorderWin() const { return m_rBorderWin; }

    public:
        OAppDetailPageHelper(vcl::Window* _pParent,OAppBorderWindow& _rBorderWin,PreviewMode _ePreviewMode);
        virtual ~OAppDetailPageHelper() override;
        virtual void dispose() override;

        // Window overrides
        virtual void KeyInput( const KeyEvent& rKEvt ) override;

        virtual void GetFocus() override;

        /** creates the tables page
            @param  _xConnection
                The connection to get the table names
        */
        void createTablesPage(const css::uno::Reference< css::sdbc::XConnection>& _xConnection);

        /** creates the page for the specific type.
            @param  _eType
                The type which should be created. E_TABLE isn't allowed.
            @param  _xContainer
                The container of the elements to be inserted.
        */
        void createPage(ElementType _eType,const css::uno::Reference< css::container::XNameAccess >& _xContainer);

        /** returns the current visible tree list box
        */
        DBTreeViewBase* getCurrentView() const
        {
            ElementType eType = getElementType();
            return (eType != E_NONE ) ? m_aLists[static_cast<sal_Int32>(eType)].get() : nullptr;
        }

        /// select all entries in the visible control
        void selectAll();

        /// returns <TRUE/> if it sorts ascending
        bool isSortUp() const;

        /// sorts all entries ascending
        void sortDown();

        /// sorts all entries descending
        void sortUp();

        /** returns the element names which are selected
            @param  _rNames
                The list will be filled.
        */
        void getSelectionElementNames( std::vector< OUString>& _rNames ) const;

        /** describes the current selection for the given control
        */
        void    describeCurrentSelectionForControl(
                    const weld::TreeView& rControl,
                    css::uno::Sequence< css::sdb::application::NamedDatabaseObject >& _out_rSelectedObjects
                );

        /** describes the current selection for the given ElementType
        */
        void    describeCurrentSelectionForType(
                    const ElementType _eType,
                    css::uno::Sequence< css::sdb::application::NamedDatabaseObject >& _out_rSelectedObjects
                );

        /** get the menu parent window for the given control
        */
        vcl::Window* getMenuParent(weld::TreeView& rControl) const;

        /** select all names on the currently selected container. Non existence names where ignored.
        *
        * \param _aNames the element names
        */
        void selectElements(const css::uno::Sequence< OUString>& _aNames);

        /** return the qualified name.
            @param  _pEntry
                The entry of a table, or query, form, report to get the qualified name.
                If the entry is <NULL/>, the first selected is chosen.
            @return
                the qualified name
        */
        OUString getQualifiedName( weld::TreeIter* _pEntry ) const;

        /// return the element of currently select entry
        ElementType getElementType() const;

        /// returns the count of selected entries
        sal_Int32 getSelectionCount();

        /// returns the count of entries
        sal_Int32 getElementCount() const;

        /** returns if an entry is a leaf
            @param rTreeView
                The TreeView rEntry belongs to
            @param rEntry
                The entry to check
            @return
                <TRUE/> if the entry is a leaf, otherwise <FALSE/>
        */
        static bool isLeaf(const weld::TreeView& rTreeView, const weld::TreeIter& rEntry);

        /** returns if one of the selected entries is a leaf
            @return
                <TRUE/> if the entry is a leaf, otherwise <FALSE/>
        */
        bool isALeafSelected() const;

        std::unique_ptr<weld::TreeIter> getEntry(const Point& rPosPixel) const;

        /// clears the detail pages
        void clearPages();

        /// returns <TRUE/> when a detail page was filled
        bool isFilled() const;

        /** adds a new object to the detail page.
            @param  _eType
                The type where the entry should be appended.
            @param  _rName
                The name of the object to be inserted
            @param  _rObject
                The object to add.
            @param  _rxConn
                If we insert a table, the connection must be set.
        */
        std::unique_ptr<weld::TreeIter> elementAdded(ElementType eType,
                                                     const OUString& rName,
                                                     const css::uno::Any& rObject);

        /** replaces an objects name with a new one
            @param  _eType
                The type where the entry should be appended.
            @param  _rOldName
                The old name of the object to be replaced
            @param  _rNewName
                The new name of the object to be replaced
            @param  _rxConn
                If we insert a table, the connection must be set.
        */
        void elementReplaced(ElementType eType
                        ,const OUString& _rOldName
                        ,const OUString& _rNewName );

        /** removes an element from the detail page.
            @param  _eType
                The type where the entry should be appended.
            @param  _rName
                The name of the element to be removed.
            @param  _rxConn
                If we remove a table, the connection must be set.
        */
        void elementRemoved(ElementType _eType
                            ,const OUString& _rName );

        /// returns the preview mode
        PreviewMode getPreviewMode() const { return m_ePreviewMode;}

        /// <TRUE/> if the preview is enabled
        bool isPreviewEnabled() const;

        /** switches to the given preview mode
            @param  _eMode
                the mode to set for the preview
            @param  _bForce
                Force the preview to be reset
        */
        void switchPreview(PreviewMode _eMode,bool _bForce = false);

        /** shows the Preview of the content when it is enabled.
            @param  _xContent
                The content which must support the "preview" command.
        */
        void showPreview(const css::uno::Reference< css::ucb::XContent >& _xContent);

        /** shows the Preview of a table or query
            @param  _sDataSourceName
                the name of the data source
            @param  _sName
                the name of table or query
            @param  _bTable
                <TRUE/> if it is a table, otherwise <FALSE/>
            @return void
        */
        void showPreview(   const OUString& _sDataSourceName,
                            const OUString& _sName,
                            bool _bTable);
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_APP_APPDETAILPAGEHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
