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
#ifndef DBAUI_TABLEWINDOW_HXX
#define DBAUI_TABLEWINDOW_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "TableWindowTitle.hxx"
#include <tools/rtti.hxx>
#include <rtl/ref.hxx>
#include "TableWindowData.hxx"
#include <vector>
#include <vcl/window.hxx>

#include <comphelper/containermultiplexer.hxx>
#include "cppuhelper/basemutex.hxx"

class SvLBoxEntry;
namespace dbaui
{
    //////////////////////////////////////////////////////////////////////////
    // Flags fuer die Groessenanpassung der SbaJoinTabWins
    const sal_uInt16 SIZING_NONE    = 0x0000;
    const sal_uInt16 SIZING_TOP     = 0x0001;
    const sal_uInt16 SIZING_BOTTOM  = 0x0002;
    const sal_uInt16 SIZING_LEFT    = 0x0004;
    const sal_uInt16 SIZING_RIGHT   = 0x0008;

    class OTableWindowListBox;
    class OJoinDesignView;
    class OJoinTableView;
    class OTableWindowAccess;

    class OTableWindow : public ::cppu::BaseMutex
                        ,public ::comphelper::OContainerListener
                        ,public Window
    {
        friend class OTableWindowTitle;
        friend class OTableWindowListBox;
    protected:
        // und die Tabelle selber (brauche ich, da ich sie locken will, solange das Fenster lebt)
        FixedImage              m_aTypeImage;
        OTableWindowTitle       m_aTitle;
        OTableWindowListBox*    m_pListBox;
        OTableWindowAccess*     m_pAccessible;

    private:
        TTableWindowData::value_type
                                m_pData;
        ::rtl::Reference< comphelper::OContainerListenerAdapter>
                                m_pContainerListener;
        sal_Int32               m_nMoveCount;           // how often the arrow keys was pressed
        sal_Int32               m_nMoveIncrement;       // how many pixel we should move
        sal_uInt16                  m_nSizingFlags;
        sal_Bool                    m_bActive;

        void Draw3DBorder( const Rectangle& rRect );
        // OContainerListener
        virtual void _elementInserted( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
        virtual void _elementRemoved( const  ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
        virtual void _elementReplaced( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);

    protected:
        virtual void    Resize();
        virtual void    Paint( const Rectangle& rRect );
        virtual void    MouseMove( const MouseEvent& rEvt );
        virtual void    MouseButtonDown( const MouseEvent& rEvt );
        virtual void    DataChanged( const DataChangedEvent& rDCEvt );

        virtual OTableWindowListBox*    CreateListBox();
            // wird im ERSTEN Init aufgerufen
        sal_Bool FillListBox();
            // wird in JEDEM Init aufgerufen

        virtual void OnEntryDoubleClicked(SvLBoxEntry* /*pEntry*/) { }
            // wird aus dem DoubleClickHdl der ListBox heraus aufgerufen

        /** HandleKeyInput triues to handle the KeyEvent. Movement or deletion
            @param  rEvt
                The KEyEvent
            @return
                <TRUE/> when the table could handle the keyevent.
        */
        sal_Bool            HandleKeyInput( const KeyEvent& rEvt );

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
        virtual void* createUserData(const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::beans::XPropertySet>& _xColumn,
                                    bool _bPrimaryKey);

        /** updates m_aTypeImage
        */
        void    impl_updateImage();

        OTableWindow( Window* pParent, const TTableWindowData::value_type& pTabWinData );

    public:
        virtual ~OTableWindow();

        // spaeter Constructor, siehe auch CreateListbox und FillListbox
        virtual sal_Bool Init();

        OJoinTableView*             getTableView();
        const OJoinTableView*       getTableView() const;
        OJoinDesignView*            getDesignView();
        void                        SetPosPixel( const Point& rNewPos );
        void                        SetSizePixel( const Size& rNewSize );
        void                        SetPosSizePixel( const Point& rNewPos, const Size& rNewSize );

        String                      getTitle() const;
        void                        SetBoldTitle( sal_Bool bBold );
        void                        setActive(sal_Bool _bActive = sal_True);

        void                        Remove();
        sal_Bool                        IsActiveWindow(){ return m_bActive; }

        ::rtl::OUString             GetTableName() const { return m_pData->GetTableName(); }
        ::rtl::OUString             GetWinName() const { return m_pData->GetWinName(); }
        ::rtl::OUString             GetComposedName() const { return m_pData->GetComposedName(); }
        OTableWindowListBox*        GetListBox() const { return m_pListBox; }
        TTableWindowData::value_type GetData() const { return m_pData; }
        OTableWindowTitle*          GetTitleCtrl() { return &m_aTitle; }

        /** returns the name which should be used when displaying join or relations
            @return
                The composed name or the window name.
        */
        virtual ::rtl::OUString     GetName() const = 0;

        inline ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > GetOriginalColumns() const { return m_pData->getColumns(); }
        inline ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >    GetTable() const { return m_pData->getTable(); }

        sal_uInt16                      GetSizingFlags() const { return m_nSizingFlags; }
        /** set the sizing flag to the direction
            @param  _rPos
                The EndPosition after resizing.
        */
        void                        setSizingFlag(const Point& _rPos);
        /** set the rsizing flag to NONE.
        */
        void                        resetSizingFlag() { m_nSizingFlags = SIZING_NONE; }

        /** returns the new sizing
        */
        Rectangle getSizingRect(const Point& _rPos,const Size& _rOutputSize) const;

        // window override
        virtual void                StateChanged( StateChangedType nStateChange );
        virtual void                GetFocus();
        virtual long                PreNotify( NotifyEvent& rNEvt );
        virtual void                Command(const CommandEvent& rEvt);

        // Accessibility
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

        // habe ich Connections nach aussen ?
        sal_Bool ExistsAConn() const;

        void EnumValidFields(::std::vector< ::rtl::OUString>& arrstrFields);

        /** clears the listbox inside. Must be called be the dtor is called.
        */
        void clearListBox();
    };
}
#endif //DBAUI_TABLEWINDOW_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
