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

#include <dbaccess/dataview.hxx>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/sdb/application/NamedDatabaseObject.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <unotools/eventlisteneradapter.hxx>
#include <vcl/InterimItemWindow.hxx>
#include <vcl/weld.hxx>
#include <IClipBoardTest.hxx>
#include <AppElementType.hxx>

namespace com::sun::star::beans    { class XPropertySet; }

class Control;
class MnemonicGenerator;

namespace dbaui
{
    class OApplicationView;
    class OApplicationDetailView;
    class OApplicationSwapWindow;
    class OTitleWindow;
    class OApplicationController;

    class OAppBorderWindow final : public InterimItemWindow
    {
        std::unique_ptr<weld::Container> m_xPanelParent;
        std::unique_ptr<weld::Container> m_xDetailViewParent;
        std::unique_ptr<OTitleWindow> m_xPanel;
        std::unique_ptr<OApplicationDetailView> m_xDetailView;
        VclPtr<OApplicationView>            m_xView;

    public:
        OAppBorderWindow(OApplicationView* pParent, PreviewMode ePreviewMode);
        virtual ~OAppBorderWindow() override;
        virtual void dispose() override;

        // Window overrides
        virtual void GetFocus() override;

        OApplicationView*       getView() const { return m_xView.get(); }
        OApplicationSwapWindow* getPanel() const;
        OApplicationDetailView* getDetailView() const { return m_xDetailView.get(); }
        weld::Container& getTopLevel() { return *m_xContainer; }
    };

    class OApplicationView : public ODataView
                            ,public IClipboardTest
                            ,public ::utl::OEventListenerAdapter
    {
        enum ChildFocusState
        {
            PANELSWAP,
            DETAIL,
            NONE
        };
    private:
        css::uno::Reference< css::lang::XComponent >
                                            m_xObject;
        VclPtr<OAppBorderWindow>            m_pWin;
        OApplicationController&             m_rAppController;

        ChildFocusState getChildFocus() const;
        IClipboardTest* getActiveChild() const;

        void ImplInitSettings();
    protected:

        // return the Rectangle where I can paint myself
        virtual void resizeDocumentView(tools::Rectangle& rRect) override;

        // OEventListenerAdapter
        virtual void _disposing( const css::lang::EventObject& _rSource ) override;

        // Window
        virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;
    public:
        OApplicationView(   vcl::Window* pParent
                            ,const css::uno::Reference< css::uno::XComponentContext >&
                            ,OApplicationController&            _rAppController
                            ,PreviewMode _ePreviewMode
                            );
        virtual ~OApplicationView() override;
        virtual void dispose() override;

        /// automatically creates mnemonics for the icon/texts in our left hand side panel
        void    createIconAutoMnemonics( MnemonicGenerator& _rMnemonics );

        /// automatically creates mnemonics for the texts in our task pane
        void    setTaskExternalMnemonics( MnemonicGenerator const & _rMnemonics );

        // Window overrides
        virtual bool PreNotify( NotifyEvent& rNEvt ) override;
        virtual void GetFocus() override;

        OApplicationController&                  getAppController() const { return m_rAppController; }

        // IClipboardTest
        virtual bool isCutAllowed() override;
        virtual bool isCopyAllowed() override;
        virtual bool isPasteAllowed() override;
        virtual void copy() override;
        virtual void cut() override;
        virtual void paste() override;

        /// get the left panel
        OApplicationSwapWindow*  getPanel()      const { return m_pWin->getPanel(); }
        /// get the detail page
        OApplicationDetailView*  getDetailView() const { return m_pWin->getDetailView(); }

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
                The TreeView rEntry belongs to
            @param rEntry
                The entry to check
            @return
                <TRUE/> if the entry is a leaf, otherwise <FALSE/>
        */
        bool isLeaf(const weld::TreeView& rTreeView, const weld::TreeIter& rEntry) const;

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

        /// returns the count of entries
        sal_Int32 getElementCount() const;

        /// returns the count of selected entries
        sal_Int32 getSelectionCount() const;

        /** clears the detail page and the selection on the left side.
            The task window will also be cleared.
        */
        void clearPages();

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
        vcl::Window* getMenuParent() const;

        /** adjust rPos relative to rControl to instead relative to getMenuParent */
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

        /** changes the container which should be displayed. The select handler will also be called.
            @param  _eType
                Which container to show.
        */
        void selectContainer(ElementType _eType);

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
            @param  _xConnection
                the connection which will be shared
            @param  _sName
                the name of table or query
            @param  _bTable
                <TRUE/> if it is a table, otherwise <FALSE/>
            @return void
        */
        void showPreview(   const OUString& _sDataSourceName,
                            const css::uno::Reference< css::sdbc::XConnection>& _xConnection,
                            const OUString& _sName,
                            bool _bTable);

        std::unique_ptr<weld::TreeIter> getEntry(const Point& rPosPixel) const;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
