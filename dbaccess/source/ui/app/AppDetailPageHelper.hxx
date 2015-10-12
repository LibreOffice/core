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
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdb/application/NamedDatabaseObject.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include "AppElementType.hxx"
#include <svtools/treelistbox.hxx>
#include <svtools/DocumentInfoPreview.hxx>
#include <vcl/fixed.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/graph.hxx>
#include <svtools/grfmgr.hxx>
#include "callbacks.hxx"
#include <memory>

namespace com{ namespace sun { namespace star { namespace awt   { class XWindow; } } } }
namespace com{ namespace sun { namespace star { namespace frame { class XFrame2; } } } }
namespace com{ namespace sun { namespace star { namespace io    { class XPersist; } } } }

#define ELEMENT_COUNT   size_t(E_ELEMENT_TYPE_COUNT)

namespace dbaui
{
    class OAppBorderWindow;
    class DBTreeListBox;

    class OPreviewWindow : public vcl::Window
    {
        GraphicObject       m_aGraphicObj;
        Rectangle           m_aPreviewRect;

        /** gets the graphic center rect
            @param  rGraphic
                the graphic
            @param  rResultRect
                the resulting rectangle

            @return
                <TRUE/> when successful
        */
        bool ImplGetGraphicCenterRect( const Graphic& rGraphic, Rectangle& rResultRect ) const;
        void ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
    protected:
        virtual void DataChanged(const DataChangedEvent& rDCEvt) override;
    public:
        explicit OPreviewWindow(vcl::Window* _pParent);

        // Window overrides
        virtual void Paint(vcl::RenderContext& /*rRenderContext*/, const Rectangle& rRect) override;

        void setGraphic(const Graphic& _rGraphic ) { m_aGraphicObj.SetGraphic(_rGraphic); }
    };
    // A helper class for the controls in the detail page.
    // Combines general functionality.
    class OAppDetailPageHelper : public vcl::Window
    {
        VclPtr<DBTreeListBox>     m_pLists[ELEMENT_COUNT];
        OAppBorderWindow&         m_rBorderWin;
        VclPtr<FixedLine>         m_aFL;
        VclPtr<ToolBox>           m_aTBPreview;
        VclPtr<Window>            m_aBorder;
        VclPtr<OPreviewWindow>    m_aPreview;
        VclPtr< ::svtools::ODocumentInfoPreview>
                                  m_aDocumentInfo;
        VclPtr<vcl::Window>       m_pTablePreview;
        ::std::unique_ptr<PopupMenu> m_aMenu;
        PreviewMode               m_ePreviewMode;
        css::uno::Reference < css::frame::XFrame2 >
                                  m_xFrame;
        css::uno::Reference < css::io::XPersist >
                                  m_xDocInfo;
        css::uno::Reference< css::awt::XWindow >
                                  m_xWindow;

        /// returns the index of the visible control
        int getVisibleControlIndex() const;

        /** sorts the entries in the tree list box.
            @param  _nPos
                Which list should be sorted.
            @param  _eSortMode
                How should be sorted.
        */
        void sort(int _nPos,SvSortMode _eSortMode );

        /** retrieves the resource ids of the images representing elements of the given type
        */
        static void getElementIcons( ElementType _eType, sal_uInt16& _rImageId);

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
                        const sal_uInt16 _nImageId,
                        SvTreeListEntry* _pParent );

        /** sets the detail page
            @param  _pWindow
                The control which should be visible.
        */
        void setDetailPage(vcl::Window* _pWindow);

        /** sets all HandleCallbacks
            @param  _pTreeView
                The newly created DBTreeListBox
            @param  _rImage
                the resource id of the default icon
            @return
                The new tree.
        */
        DBTreeListBox* createTree( DBTreeListBox* _pTreeView, const Image& _rImage );

        /** creates the tree and sets all HandleCallbacks
            @param  _nHelpId
                The help id of the control
            @param  _nCollapsedBitmap
                The image to use in high contrast mode.
            @return
                The new tree.
        */
        DBTreeListBox* createSimpleTree( const OString& _sHelpId, const Image& _rImage);

        DECL_LINK_TYPED( OnEntryDoubleClick,    SvTreeListBox*, bool );
        DECL_LINK_TYPED( OnEntryEnterKey,       DBTreeListBox*, void );
        DECL_LINK_TYPED( OnEntrySelChange,      LinkParamNone*, void );

        DECL_LINK_TYPED( OnCopyEntry,           LinkParamNone*, void );
        DECL_LINK_TYPED( OnPasteEntry,          LinkParamNone*, void );
        DECL_LINK_TYPED( OnDeleteEntry,         LinkParamNone*, void );

        // click a TB slot
        DECL_LINK_TYPED(OnDropdownClickHdl, ToolBox*, void);

        inline OAppBorderWindow& getBorderWin() const { return m_rBorderWin; }
        void ImplInitSettings();

    public:
        OAppDetailPageHelper(vcl::Window* _pParent,OAppBorderWindow& _rBorderWin,PreviewMode _ePreviewMode);
        virtual ~OAppDetailPageHelper();
        virtual void dispose() override;

        // Window overrides
        virtual void Resize() override;
        virtual void KeyInput( const KeyEvent& rKEvt ) override;

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
        inline DBTreeListBox* getCurrentView() const
        {
            ElementType eType = getElementType();
            return (eType != E_NONE ) ? m_pLists[static_cast<sal_Int32>(eType)].get() : NULL;
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
        void getSelectionElementNames( ::std::vector< OUString>& _rNames ) const;

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

        /** return the qualified name.
            @param  _pEntry
                The entry of a table, or query, form, report to get the qualified name.
                If the entry is <NULL/>, the first selected is chosen.
            @return
                the qualified name
        */
        OUString getQualifiedName( SvTreeListEntry* _pEntry ) const;

        /// return the element of currently select entry
        ElementType getElementType() const;

        /// returns the count of selected entries
        sal_Int32 getSelectionCount();

        /// returns the count of entries
        sal_Int32 getElementCount();

        /** returns if an entry is a leaf
            @param _pEntry
                The entry to check
            @return
                <TRUE/> if the entry is a leaf, otherwise <FALSE/>
        */
        static bool isLeaf(SvTreeListEntry* _pEntry);

        /** returns if one of the selected entries is a leaf
            @return
                <TRUE/> if the entry is a leaf, otherwise <FALSE/>
        */
        bool isALeafSelected() const;

        SvTreeListEntry* getEntry( const Point& _aPosPixel ) const;

        /// clears the detail pages
        void clearPages();

        /// returns <TRUE/> when a detail page was filled
        bool isFilled() const;

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
        SvTreeListEntry*  elementAdded(ElementType eType
                        ,const OUString& _rName
                        ,const css::uno::Any& _rObject );

        /** replaces a objects name with a new one
            @param  _eType
                The type where the entry shold be appended.
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
                The type where the entry shold be appended.
            @param  _rName
                The name of the element to be removed.
            @param  _rxConn
                If we remove a table, the connection must be set.
        */
        void elementRemoved(ElementType _eType
                            ,const OUString& _rName );

        /// returns the preview mode
        PreviewMode getPreviewMode() { return m_ePreviewMode;}

        /// <TRUE/> if the preview is enabled
        bool isPreviewEnabled();

        /** switches to the given preview mode
            @param  _eMode
                the mode to set for the preview
            @param  _bForce
                Force the preview to be resetted
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

    protected:
        void DataChanged( const DataChangedEvent& rDCEvt ) override;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_APP_APPDETAILPAGEHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
