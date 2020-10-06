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
#pragma once

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/sdb/application/NamedDatabaseObject.hpp>
#include <vcl/mnemonic.hxx>
#include <IClipBoardTest.hxx>
#include "AppTitleWindow.hxx"
#include <AppElementType.hxx>

#include <vector>

namespace dbaui
{
    class OAppBorderWindow;
    class OApplicationDetailView;
    class OAppDetailPageHelper;
    class OTasksWindow;
    class TreeListBox;

    struct TaskEntry
    {
        OUString        sUNOCommand;
        const char*     pHelpID;
        OUString        sTitle;
        bool            bHideWhenDisabled;
            // TODO: we should be consistent in the task pane and the menus/toolbars:
            // If an entry is disabled in the latter, it should also be disabled in the former.
            // If an entry is *hidden* in the former, it should also be hidden in the latter.

        TaskEntry( const char* _pAsciiUNOCommand, const char* pHelpID, const char* pTitleResourceID, bool _bHideWhenDisabled = false );
    };
    typedef std::vector< TaskEntry >  TaskEntryList;

    struct TaskPaneData
    {
        /// the tasks available in the pane
        TaskEntryList   aTasks;
        /// the resource ID for the title of the pane
        const char*     pTitleId;
    };

    class OTasksWindow final : public OChildWindow
    {
        std::unique_ptr<weld::TreeView> m_xTreeView;
        std::unique_ptr<weld::Label> m_xDescription;
        std::unique_ptr<weld::TextView> m_xHelpText;
        OApplicationDetailView* m_pDetailView;

        int m_nCursorIndex;

        DECL_LINK(onSelected, weld::TreeView&, bool);
        DECL_LINK(OnEntrySelectHdl, weld::TreeView&, void);
        DECL_LINK(FocusInHdl, weld::Widget&, void);
        DECL_LINK(FocusOutHdl, weld::Widget&, void);

        void updateHelpText();

    public:
        OTasksWindow(weld::Container* pParent, OApplicationDetailView* pDetailView);
        ~OTasksWindow();

        virtual void GrabFocus() override;

        virtual bool HasChildPathFocus() const override;

        OApplicationDetailView* getDetailView() const { return m_pDetailView; }

        /// fills the Creation listbox with the necessary strings and images
        void fillTaskEntryList( const TaskEntryList& _rList );

        void Clear();
        void setHelpText(const char* pId);
    };

    class OApplicationDetailView final : public IClipboardTest
    {
        std::unique_ptr<weld::Builder>      m_xBuilder;
        std::unique_ptr<weld::Container>    m_xContainer;
        std::unique_ptr<weld::Paned>        m_xHorzSplitter;
        std::unique_ptr<weld::Container>    m_xTasksParent;
        std::unique_ptr<weld::Container>    m_xContainerParent;
        std::unique_ptr<OTitleWindow>       m_xTasks;
        std::unique_ptr<OTitleWindow>       m_xTitleContainer;
        OAppBorderWindow&                   m_rBorderWin;       // my parent
        std::shared_ptr<OChildWindow>       m_xControlHelper;
        std::vector< TaskPaneData >         m_aTaskPaneData;
        MnemonicGenerator                   m_aExternalMnemonics;

        const OAppDetailPageHelper* GetControlHelper() const;
        OAppDetailPageHelper* GetControlHelper();

    public:
        OApplicationDetailView(weld::Container* pParent, OAppBorderWindow& rBorder, PreviewMode ePreviewMode);
        ~OApplicationDetailView();

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

        void setTaskExternalMnemonics( MnemonicGenerator const & _rMnemonics );

        OAppBorderWindow& getBorderWin() const { return m_rBorderWin; }
        OTasksWindow& getTasksWindow() const { return *static_cast< OTasksWindow* >( m_xTasks->getChildWindow() ); }

        bool isCutAllowed() override ;
        bool isCopyAllowed() override    ;
        bool isPasteAllowed() override;
        void copy() override;
        void cut() override;
        void paste() override;

        /** return the qualified name.
            @param  _pEntry
                The entry of a table, or query, form, report to get the qualified name.
                If the entry is <NULL/>, the first selected is chosen.
            @return
                the qualified name
        */
        OUString getQualifiedName(const weld::TreeIter* _pEntry) const;

        /** returns if an entry is a leaf
            @param rTreeView
                The TreeView pEntry belongs to
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

        /** select all entries in the detail page
        */
        void selectAll();

        /// returns <TRUE/> if it sorts ascending
        bool isSortUp() const;

        /// sort the entries in the detail page down
        void sortDown();

        /// sort the entries in the detail page up
        void sortUp();

        /// returns <TRUE/> when a detail page was filled
        bool isFilled() const;

        /// return the element of currently select entry
        ElementType getElementType() const;

        /** clears the detail pages.
            @param  _bTaskAlso
                If <TRUE/> the task window will also be cleared.
        */
        void clearPages(bool _bTaskAlso = true);

        /// returns the count of entries
        sal_Int32 getElementCount() const;

        /// returns the count of selected entries
        sal_Int32 getSelectionCount();

        /** returns the element names which are selected
            @param  _rNames
                The list will be filled.
        */
        void getSelectionElementNames(std::vector< OUString>& _rNames ) const;

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
        vcl::Window* getMenuParent() const;
        void adjustMenuPosition(const weld::TreeView& rControl, ::Point& rPos) const;

        /** select all names on the currently selected container. Non existence names where ignored.
        *
        * \param _aNames the element names
        */
        void selectElements(const css::uno::Sequence< OUString>& _aNames);

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
            @param  _xObject
                The object which was replaced
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
        PreviewMode getPreviewMode() const;

        /// <TRUE/> if the preview is enabled
        bool isPreviewEnabled() const;

        /** switches to the given preview mode
            @param  _eMode
                the mode to set for the preview
        */
        void switchPreview(PreviewMode _eMode);

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

        std::unique_ptr<weld::TreeIter> getEntry(const Point& rPosPixel) const;

        TreeListBox* getTreeWindow() const;

        bool HasChildPathFocus() const;
    private:
        void                impl_createPage(
                                ElementType _eType,
                                const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
                                const css::uno::Reference< css::container::XNameAccess >& _rxNonTableElements
                            );

        const TaskPaneData& impl_getTaskPaneData( ElementType _eType );
        void                impl_fillTaskPaneData( ElementType _eType, TaskPaneData& _rData ) const;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
