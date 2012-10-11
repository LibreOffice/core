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
#ifndef DBAUI_APPDETAILVIEW_HXX
#define DBAUI_APPDETAILVIEW_HXX

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/sdb/application/NamedDatabaseObject.hpp>
#include <vcl/split.hxx>
#include <vcl/fixed.hxx>
#include <vcl/mnemonic.hxx>
#include "IClipBoardTest.hxx"
#include "AppTitleWindow.hxx"
#include "AppElementType.hxx"
#include <svtools/treelistbox.hxx>
#include "VertSplitView.hxx"

#include <vector>

class SvLBoxEntry;

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
        SvLBoxEntry*        m_pMouseDownEntry;
        SvLBoxEntry*        m_pLastActiveEntry;
        Color               m_aOriginalBackgroundColor;
        Font                m_aOriginalFont;

    public:
        OCreationList( OTasksWindow& _rParent );
        // window overloads
        virtual void MouseMove( const MouseEvent& rMEvt );
        virtual void MouseButtonDown( const MouseEvent& rMEvt );
        virtual void MouseButtonUp( const MouseEvent& rMEvt );
        virtual void KeyInput( const KeyEvent& rKEvt );
        virtual void Paint( const Rectangle& rRect );
        virtual void StartDrag( sal_Int8 _nAction, const Point& _rPosPixel );
        virtual void GetFocus();
        virtual void LoseFocus();

        inline void resetLastActive() { m_pLastActiveEntry = NULL;}

        void    updateHelpText();

    protected:
        virtual void        PreparePaint( SvLBoxEntry* _pEntry );
        virtual Rectangle   GetFocusRect( SvLBoxEntry* _pEntry, long _nLine );
        virtual void        ModelHasCleared();

        // IMnemonicEntryList
        virtual void        SelectSearchEntry( const void* _pEntry );
        virtual void        ExecuteSearchEntry( const void* _pEntry ) const;

    private:
        void    onSelected( SvLBoxEntry* _pEntry ) const;
        /** sets a new current entry, and invalidates the old and the new one, if necessary
            @return <TRUE/> if and only if the "current entry" changed
        */
        bool    setCurrentEntryInvalidate( SvLBoxEntry* _pEntry );
    };

    struct TaskEntry
    {
        ::rtl::OUString sUNOCommand;
        sal_uInt16          nHelpID;
        String          sTitle;
        bool            bHideWhenDisabled;
            // TODO: we should be consistent in the task pane and the menus/toolbars:
            // If an entry is disabled in the latter, it should also be disabled in the former.
            // If an entry is *hidden* in the former, it should also be hidden in the latter.

        TaskEntry( const sal_Char* _pAsciiUNOCommand, sal_uInt16 _nHelpID, sal_uInt16 _nTitleResourceID, bool _bHideWhenDisabled = false );
    };
    typedef ::std::vector< TaskEntry >  TaskEntryList;

    struct TaskPaneData
    {
        /// the tasks available in the pane
        TaskEntryList   aTasks;
        /// the resource ID for the title of the pane
        sal_uInt16          nTitleId;
    };

    class OTasksWindow : public Window
    {
        OCreationList                       m_aCreation;
        FixedText                           m_aDescription;
        FixedText                           m_aHelpText;
        FixedLine                           m_aFL;
        OApplicationDetailView*             m_pDetailView;

        DECL_LINK( OnEntrySelectHdl,        SvTreeListBox* );
        void ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt);
    public:
        OTasksWindow(Window* _pParent,OApplicationDetailView* _pDetailView);
        virtual ~OTasksWindow();

        // window overloads
        virtual void Resize();

        OApplicationDetailView* getDetailView() const { return m_pDetailView; }

        /// fills the Creation listbox with the necessary strings and images
        void fillTaskEntryList( const TaskEntryList& _rList );

        inline bool HandleKeyInput( const KeyEvent& _rKEvt )
        {
            return m_aCreation.HandleKeyInput( _rKEvt );
        }

        void Clear();
        void setHelpText(sal_uInt16 _nId);
    };
    //==================================================================
    class OApplicationDetailView : public OSplitterView
                                 , public IClipboardTest
    {
        Splitter                            m_aHorzSplitter;
        OTitleWindow                        m_aTasks;
        OTitleWindow                        m_aContainer;
        OAppBorderWindow&                   m_rBorderWin;       // my parent
        OAppDetailPageHelper*               m_pControlHelper;
        ::std::vector< TaskPaneData >       m_aTaskPaneData;
        MnemonicGenerator                   m_aExternalMnemonics;

        void ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );

    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt);

    public:
        OApplicationDetailView(OAppBorderWindow& _rParent,PreviewMode _ePreviewMode);
        virtual ~OApplicationDetailView();
        // window overloads
        virtual void GetFocus();

        /** creates the tables page
            @param  _xConnection
                The connection to get the table names
        */
        void createTablesPage(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);

        /** creates the page for the specific type.
            @param  _eType
                The type which should be created. E_TABLE isn't allowed.
            @param  _xContainer
                The container of the elements to be inserted.
        */
        void createPage(ElementType _eType,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xContainer);

        void setTaskExternalMnemonics( MnemonicGenerator& _rMnemonics );

        /** called to give the window the chance to intercept key events, while it has not
            the focus

            @return <TRUE/> if and only if the event has been handled, and should not
                not be further processed
        */
        bool    interceptKeyInput( const KeyEvent& _rEvent );

        inline OAppBorderWindow& getBorderWin() const { return m_rBorderWin; }
        inline OTasksWindow& getTasksWindow() const { return *static_cast< OTasksWindow* >( m_aTasks.getChildWindow() ); }

        sal_Bool isCutAllowed() ;
        sal_Bool isCopyAllowed()    ;
        sal_Bool isPasteAllowed();
        virtual sal_Bool hasChildPathFocus() { return HasChildPathFocus(); }
        void copy();
        void cut();
        void paste();

        /** return the qualified name.
            @param  _pEntry
                The entry of a table, or query, form, report to get the qualified name.
                If the entry is <NULL/>, the first selected is chosen.
            @return
                the qualified name
        */
        ::rtl::OUString getQualifiedName( SvLBoxEntry* _pEntry ) const;

        /** returns if an entry is a leaf
            @param _pEntry
                The entry to check
            @return
                <TRUE/> if the entry is a leaf, otherwise <FALSE/>
        */
        sal_Bool isLeaf(SvLBoxEntry* _pEntry) const;

        /** returns if one of the selected entries is a leaf
            @return
                <TRUE/> if the entry is a leaf, otherwise <FALSE/>
        */
        sal_Bool isALeafSelected() const;

        /** select all entries in the detail page
        */
        void selectAll();

        /// returns <TRUE/> if it sorts ascending
        sal_Bool isSortUp() const;

        /// sort the entries in the detail page down
        void sortDown();

        /// sort the entries in the detail page up
        void sortUp();

        /// returns <TRUE/> when a detail page was filled
        sal_Bool isFilled() const;

        /// return the element of currently select entry
        ElementType getElementType() const;

        /** clears the detail pages.
            @param  _bTaskAlso
                If <TRUE/> the task window will also be cleared.
        */
        void clearPages(sal_Bool _bTaskAlso = sal_True);

        /// returns the count of entries
        sal_Int32 getElementCount();

        /// returns the count of selected entries
        sal_Int32 getSelectionCount();

        /** returns the element names which are selected
            @param  _rNames
                The list will be filled.
        */
        void getSelectionElementNames(::std::vector< ::rtl::OUString>& _rNames ) const;

        /** describes the current selection for the given control
        */
        void    describeCurrentSelectionForControl(
                    const Control& _rControl,
                    ::com::sun::star::uno::Sequence< ::com::sun::star::sdb::application::NamedDatabaseObject >& _out_rSelectedObjects
                );

        /** describes the current selection for the given ElementType
        */
        void    describeCurrentSelectionForType(
                    const ElementType _eType,
                    ::com::sun::star::uno::Sequence< ::com::sun::star::sdb::application::NamedDatabaseObject >& _out_rSelectedObjects
                );

        /** select all names on the currently selected container. Non existence names where ignored.
        *
        * \param _aNames the element names
        */
        void selectElements(const ::com::sun::star::uno::Sequence< ::rtl::OUString>& _aNames);

        /** adds a new object to the detail page.
            @param  _eType
                The type where the entry shold be appended.
            @param  _rName
                The name of the object to be inserted
            @param  _rObject
                The object to add.
            @param  _rxConn
                If we insert a table, the connection must be set.
        */
        SvLBoxEntry* elementAdded(ElementType eType
                        ,const ::rtl::OUString& _rName
                        ,const ::com::sun::star::uno::Any& _rObject );

        /** replaces a objects name with a new one
            @param  _eType
                The type where the entry shold be appended.
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
                        ,const ::rtl::OUString& _rOldName
                        ,const ::rtl::OUString& _rNewName );

        /** removes an element from the detail page.
            @param  _eType
                The type where the entry shold be appended.
            @param  _rName
                The name of the element to be removed.
            @param  _rxConn
                If we remove a table, the connection must be set.
        */
        void elementRemoved(ElementType _eType
                            ,const ::rtl::OUString& _rName );

        /// returns the preview mode
        PreviewMode getPreviewMode();

        /// <TRUE/> if the preview is enabled
        sal_Bool isPreviewEnabled();


        /** switches to the given preview mode
            @param  _eMode
                the mode to set for the preview
        */
        void switchPreview(PreviewMode _eMode);

        /** shows the Preview of the content when it is enabled.
            @param  _xContent
                The content which must support the "preview" command.
        */
        void showPreview(const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent >& _xContent);

        /** shows the Preview of a table or query
            @param  _sDataSourceName
                the name of the data source
            @param  _sName
                the name of table or query
            @param  _bTable
                <TRUE/> if it is a table, otherwise <FALSE/>
            @return void
        */
        void showPreview(   const ::rtl::OUString& _sDataSourceName,
                            const ::rtl::OUString& _sName,
                            sal_Bool _bTable);

        SvLBoxEntry* getEntry( const Point& _aPoint ) const;

        Window* getTreeWindow() const;
    private:
        void                impl_createPage(
                                ElementType _eType,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxNonTableElements
                            );

        const TaskPaneData& impl_getTaskPaneData( ElementType _eType );
        void                impl_fillTaskPaneData( ElementType _eType, TaskPaneData& _rData ) const;
    };
}
#endif // DBAUI_APPDETAILVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
