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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_APP_APPDETAILVIEW_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_APP_APPDETAILVIEW_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/sdb/application/NamedDatabaseObject.hpp>
#include <vcl/split.hxx>
#include <vcl/fixed.hxx>
#include <vcl/mnemonic.hxx>
#include <IClipBoardTest.hxx>
#include "AppTitleWindow.hxx"
#include <AppElementType.hxx>
#include <vcl/treelistbox.hxx>
#include <VertSplitView.hxx>

#include <vector>

class SvTreeListEntry;

namespace dbaui
{
    class OAppBorderWindow;
    class OApplicationDetailView;
    class OAppDetailPageHelper;
    class OTasksWindow;

    class OCreationList : public SvTreeListBox
    {
        OTasksWindow&   m_rTaskWindow;

        // members related to drawing the currently hovered/selected entry
        SvTreeListEntry*        m_pMouseDownEntry;
        SvTreeListEntry*        m_pLastActiveEntry;
        Color                   m_aOriginalBackgroundColor;
        vcl::Font               m_aOriginalFont;

    public:
        explicit OCreationList( OTasksWindow& _rParent );
        // Window overrides
        virtual void MouseMove( const MouseEvent& rMEvt ) override;
        virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
        virtual void MouseButtonUp( const MouseEvent& rMEvt ) override;
        virtual void KeyInput( const KeyEvent& rKEvt ) override;
        virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
        virtual void StartDrag( sal_Int8 _nAction, const Point& _rPosPixel ) override;
        virtual void GetFocus() override;
        virtual void LoseFocus() override;

        void resetLastActive() { m_pLastActiveEntry = nullptr;}

        void    updateHelpText();

    protected:
        virtual void        PreparePaint(vcl::RenderContext& rRenderContext, SvTreeListEntry& rEntry) override;
        virtual tools::Rectangle   GetFocusRect(const SvTreeListEntry* _pEntry, long _nLine) override;
        virtual void        ModelHasCleared() override;

    private:
        void    onSelected( SvTreeListEntry const * _pEntry ) const;
        /** sets a new current entry, and invalidates the old and the new one, if necessary
            @return <TRUE/> if and only if the "current entry" changed
        */
        bool    setCurrentEntryInvalidate( SvTreeListEntry* _pEntry );
    };

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

    class OTasksWindow : public vcl::Window
    {
        VclPtr<OCreationList>               m_aCreation;
        VclPtr<FixedText>                   m_aDescription;
        VclPtr<FixedText>                   m_aHelpText;
        VclPtr<FixedLine>                   m_aFL;
        VclPtr<OApplicationDetailView>      m_pDetailView;

        DECL_LINK( OnEntrySelectHdl, SvTreeListBox*, void );
        void ImplInitSettings();
    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt) override;
    public:
        OTasksWindow(vcl::Window* _pParent,OApplicationDetailView* _pDetailView);
        virtual ~OTasksWindow() override;
        virtual void dispose() override;

        // Window overrides
        virtual void Resize() override;

        OApplicationDetailView* getDetailView() const { return m_pDetailView; }

        /// fills the Creation listbox with the necessary strings and images
        void fillTaskEntryList( const TaskEntryList& _rList );

        void Clear();
        void setHelpText(const char* pId);
    };
    class OApplicationDetailView : public OSplitterView
                                 , public IClipboardTest
    {
        VclPtr<Splitter>                    m_aHorzSplitter;
        VclPtr<OTitleWindow>                m_aTasks;
        VclPtr<OTitleWindow>                m_aContainer;
        OAppBorderWindow&                   m_rBorderWin;       // my parent
        VclPtr<OAppDetailPageHelper>        m_pControlHelper;
        std::vector< TaskPaneData >       m_aTaskPaneData;
        MnemonicGenerator                   m_aExternalMnemonics;

        void ImplInitSettings();

    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt) override;

    public:
        OApplicationDetailView(OAppBorderWindow& _rParent,PreviewMode _ePreviewMode);
        virtual ~OApplicationDetailView() override;
        // Window overrides
        virtual void dispose() override;

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
        OTasksWindow& getTasksWindow() const { return *static_cast< OTasksWindow* >( m_aTasks->getChildWindow() ); }

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
        OUString getQualifiedName( SvTreeListEntry* _pEntry ) const;

        /** returns if an entry is a leaf
            @param _pEntry
                The entry to check
            @return
                <TRUE/> if the entry is a leaf, otherwise <FALSE/>
        */
        static bool isLeaf(SvTreeListEntry const * _pEntry);

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
                    const Control& _rControl,
                    css::uno::Sequence< css::sdb::application::NamedDatabaseObject >& _out_rSelectedObjects
                );

        /** describes the current selection for the given ElementType
        */
        void    describeCurrentSelectionForType(
                    const ElementType _eType,
                    css::uno::Sequence< css::sdb::application::NamedDatabaseObject >& _out_rSelectedObjects
                );

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
        SvTreeListEntry* elementAdded(ElementType eType
                        ,const OUString& _rName
                        ,const css::uno::Any& _rObject );

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

        SvTreeListEntry* getEntry( const Point& _aPoint ) const;

        vcl::Window* getTreeWindow() const;
    private:
        void                impl_createPage(
                                ElementType _eType,
                                const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
                                const css::uno::Reference< css::container::XNameAccess >& _rxNonTableElements
                            );

        const TaskPaneData& impl_getTaskPaneData( ElementType _eType );
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_APP_APPDETAILVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
