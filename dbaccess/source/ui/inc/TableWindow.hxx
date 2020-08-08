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
#include <cppuhelper/basemutex.hxx>
#include <o3tl/typed_flags_set.hxx>

class SvTreeListEntry;

// Flags for the size adjustment of SbaJoinTabWins
enum class SizingFlags {
    NONE    = 0x0000,
    Top     = 0x0001,
    Bottom  = 0x0002,
    Left    = 0x0004,
    Right   = 0x0008,
};
namespace o3tl {
    template<> struct typed_flags<SizingFlags> : is_typed_flags<SizingFlags, 0x0f> {};
}


namespace dbaui
{
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

    private:
        TTableWindowData::value_type
                                m_pData;
        ::rtl::Reference< comphelper::OContainerListenerAdapter>
                                m_pContainerListener;
        sal_Int32               m_nMoveCount;           // how often the arrow keys was pressed
        sal_Int32               m_nMoveIncrement;       // how many pixel we should move
        SizingFlags             m_nSizingFlags;

        // OContainerListener
        virtual void _elementInserted( const css::container::ContainerEvent& _rEvent ) override;
        virtual void _elementRemoved( const  css::container::ContainerEvent& _rEvent ) override;
        virtual void _elementReplaced( const css::container::ContainerEvent& _rEvent ) override;

    protected:
        virtual void    Resize() override;
        virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
        virtual void    MouseMove( const MouseEvent& rEvt ) override;
        virtual void    MouseButtonDown( const MouseEvent& rEvt ) override;
        virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

            // called at FIRST Init
        void FillListBox();
            // called at EACH Init

        virtual void OnEntryDoubleClicked(weld::TreeIter& /*rEntry*/) { }
            // called from the DoubleClickHdl of the ListBox

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
        virtual ~OTableWindow() override;
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

        OUString const &    GetTableName() const { return m_pData->GetTableName(); }
        OUString const &    GetWinName() const { return m_pData->GetWinName(); }
        OUString const &    GetComposedName() const { return m_pData->GetComposedName(); }
        const VclPtr<OTableWindowListBox>&  GetListBox() const { return m_xListBox; }
        const TTableWindowData::value_type& GetData() const { return m_pData; }
        const VclPtr<OTableWindowTitle>&    GetTitleCtrl() const { return m_xTitle; }

        /** returns the name which should be used when displaying join or relations
            @return
                The composed name or the window name.
        */
        virtual OUString     GetName() const = 0;

        css::uno::Reference< css::container::XNameAccess > GetOriginalColumns() const { return m_pData->getColumns(); }
        css::uno::Reference< css::beans::XPropertySet >    GetTable() const { return m_pData->getTable(); }

        /** set the sizing flag to the direction
            @param  _rPos
                The EndPosition after resizing.
        */
        void                        setSizingFlag(const Point& _rPos);

        /** returns the new sizing
        */
        tools::Rectangle getSizingRect(const Point& _rPos,const Size& _rOutputSize) const;

        // window override
        virtual void                StateChanged( StateChangedType nStateChange ) override;
        virtual void                GetFocus() override;
        virtual bool                PreNotify( NotifyEvent& rNEvt ) override;
        virtual void                Command(const CommandEvent& rEvt) override;

        // Accessibility
        virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

        // do I have connections to the outside?
        bool ExistsAConn() const;

        void EnumValidFields(std::vector< OUString>& arrstrFields);

        /** clears the listbox inside. Must be called be the dtor is called.
        */
        void clearListBox();
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
