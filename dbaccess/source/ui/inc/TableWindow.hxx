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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEWINDOW_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEWINDOW_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "TableWindowTitle.hxx"
#include <rtl/ref.hxx>
#include "TableWindowData.hxx"
#include "TableWindowListBox.hxx"
#include <vector>
#include <vcl/window.hxx>

#include <comphelper/containermultiplexer.hxx>
#include "cppuhelper/basemutex.hxx"

class SvTreeListEntry;
namespace dbaui
{
    // Flags for the size adjustment of SbaJoinTabWins
    const sal_uInt16 SIZING_NONE    = 0x0000;
    const sal_uInt16 SIZING_TOP     = 0x0001;
    const sal_uInt16 SIZING_BOTTOM  = 0x0002;
    const sal_uInt16 SIZING_LEFT    = 0x0004;
    const sal_uInt16 SIZING_RIGHT   = 0x0008;

    class OJoinDesignView;
    class OJoinTableView;
    class OTableWindowAccess;

    class OTableWindow : public ::cppu::BaseMutex
                        ,public ::comphelper::OContainerListener
                        ,public vcl::Window
    {
        friend class OTableWindowTitle;
        friend class OTableWindowListBox;
    protected:
        // and the table itself (needed for me as I want to lock it as long as the window is alive)
        VclPtr<FixedImage>          m_aTypeImage;
        VclPtr<OTableWindowTitle>   m_xTitle;
        VclPtr<OTableWindowListBox> m_xListBox;
        OTableWindowAccess*     m_pAccessible;

    private:
        TTableWindowData::value_type
                                m_pData;
        ::rtl::Reference< comphelper::OContainerListenerAdapter>
                                m_pContainerListener;
        sal_Int32               m_nMoveCount;           // how often the arrow keys was pressed
        sal_Int32               m_nMoveIncrement;       // how many pixel we should move
        sal_uInt16              m_nSizingFlags;
        bool                m_bActive;

        // OContainerListener
        virtual void _elementInserted( const css::container::ContainerEvent& _rEvent ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void _elementRemoved( const  css::container::ContainerEvent& _rEvent ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void _elementReplaced( const css::container::ContainerEvent& _rEvent ) throw(css::uno::RuntimeException, std::exception) override;

    protected:
        virtual void    Resize() override;
        virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
        virtual void    MouseMove( const MouseEvent& rEvt ) override;
        virtual void    MouseButtonDown( const MouseEvent& rEvt ) override;
        virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

        VclPtr<OTableWindowListBox> CreateListBox();
            // called at FIRST Init
        bool FillListBox();
            // called at EACH Init

        virtual void OnEntryDoubleClicked(SvTreeListEntry* /*pEntry*/) { }
            // called from the DoubleClickHdl of the ListBox

        /** HandleKeyInput tries to handle the KeyEvent. Movement or deletion
            @param  rEvt
                The KEyEvent
            @return
                <TRUE/> when the table could handle the keyevent.
        */
        bool            HandleKeyInput( const KeyEvent& rEvt );

        /** delete the user data with the equal type as created within createUserData
            @param  _pUserData
                The user data store in the listbox entries. Created with a call to createUserData.
                _pUserData may be <NULL/>. _pUserData will be set to <NULL/> after call.
        */
        virtual void deleteUserData(void*& _pUserData);

        /** creates user information that will be append at the ListBoxentry
            @param  _xColumn
                The corresponding column, can be <NULL/>.
            @param  _bPrimaryKey
                <TRUE/> when the column belongs to the primary key
            @return
                the user data which will be append at the listbox entry, may be <NULL/>
        */
        virtual void* createUserData(const css::uno::Reference<
                                    css::beans::XPropertySet>& _xColumn,
                                    bool _bPrimaryKey);

        /** updates m_aTypeImage
        */
        void    impl_updateImage();

        OTableWindow( vcl::Window* pParent, const TTableWindowData::value_type& pTabWinData );

    public:
        virtual ~OTableWindow();
        virtual void dispose() override;

        // late Constructor, see also CreateListbox and FillListbox
        virtual bool Init();

        OJoinTableView*             getTableView();
        const OJoinTableView*       getTableView() const;
        OJoinDesignView*            getDesignView();
        void                        SetPosPixel( const Point& rNewPos ) override;
        void                        SetSizePixel( const Size& rNewSize ) override;
        void                        SetPosSizePixel( const Point& rNewPos, const Size& rNewSize ) override;

        OUString                    getTitle() const;
        void                        SetBoldTitle( bool bBold );
        void                        setActive(bool _bActive = true);

        void                        Remove();

        OUString             GetTableName() const { return m_pData->GetTableName(); }
        OUString             GetWinName() const { return m_pData->GetWinName(); }
        OUString             GetComposedName() const { return m_pData->GetComposedName(); }
        VclPtr<OTableWindowListBox>  GetListBox() const { return m_xListBox; }
        TTableWindowData::value_type GetData() const { return m_pData; }
        VclPtr<OTableWindowTitle>    GetTitleCtrl() { return m_xTitle; }

        /** returns the name which should be used when displaying join or relations
            @return
                The composed name or the window name.
        */
        virtual OUString     GetName() const = 0;

        inline css::uno::Reference< css::container::XNameAccess > GetOriginalColumns() const { return m_pData->getColumns(); }
        inline css::uno::Reference< css::beans::XPropertySet >    GetTable() const { return m_pData->getTable(); }

        sal_uInt16                      GetSizingFlags() const { return m_nSizingFlags; }
        /** set the sizing flag to the direction
            @param  _rPos
                The EndPosition after resizing.
        */
        void                        setSizingFlag(const Point& _rPos);
        /** set the resizing flag to NONE.
        */
        void                        resetSizingFlag() { m_nSizingFlags = SIZING_NONE; }

        /** returns the new sizing
        */
        Rectangle getSizingRect(const Point& _rPos,const Size& _rOutputSize) const;

        // window override
        virtual void                StateChanged( StateChangedType nStateChange ) override;
        virtual void                GetFocus() override;
        virtual bool                PreNotify( NotifyEvent& rNEvt ) override;
        virtual void                Command(const CommandEvent& rEvt) override;

        // Accessibility
        virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

        // do I have connections to the outside?
        bool ExistsAConn() const;

        void EnumValidFields(::std::vector< OUString>& arrstrFields);

        /** clears the listbox inside. Must be called be the dtor is called.
        */
        void clearListBox();
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
