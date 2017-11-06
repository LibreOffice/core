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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_SBAGRID_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_SBAGRID_HXX

#include <svx/fmgridcl.hxx>

#include <svx/fmgridif.hxx>

#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/util/URL.hpp>
#include <comphelper/uno3.hxx>
#include "sbamultiplex.hxx"
#include <svx/dataaccessdescriptor.hxx>
#include <rtl/ref.hxx>
#include <map>
#include <queue>

class SvNumberFormatter;

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
}}}

namespace dbaui
{
    struct SbaURLCompare
    {
        bool operator() (const css::util::URL& x, const css::util::URL& y) const { return x.Complete == y.Complete; }
    };

    class SbaXStatusMultiplexer;
    class SbaXGridControl
                :public FmXGridControl
                ,public css::frame::XDispatch
    {
        typedef std::map<css::util::URL, rtl::Reference<SbaXStatusMultiplexer>, SbaURLCompare> StatusMultiplexerArray;
        StatusMultiplexerArray      m_aStatusMultiplexer;

    public:
        SbaXGridControl(const css::uno::Reference< css::uno::XComponentContext >&);
        virtual ~SbaXGridControl() override;

        // UNO
        DECLARE_UNO3_DEFAULTS(SbaXGridControl, FmXGridControl)
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

        // XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

        // css::lang::XServiceInfo
        OUString SAL_CALL getImplementationName() override;
        virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
        // need by registration
        /// @throws css::uno::RuntimeException
        static OUString getImplementationName_Static();
        /// @throws css::uno::RuntimeException
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static();
        static css::uno::Reference< css::uno::XInterface >
                SAL_CALL Create(const css::uno::Reference< css::lang::XMultiServiceFactory >&);

        // css::frame::XDispatch
        virtual void SAL_CALL dispatch(const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& aArgs) override;
        virtual void SAL_CALL addStatusListener(const css::uno::Reference< css::frame::XStatusListener > & xControl, const css::util::URL& aURL) override;
        virtual void SAL_CALL removeStatusListener(const css::uno::Reference< css::frame::XStatusListener > & xControl, const css::util::URL& aURL) override;

        // css::lang::XComponent
        virtual void SAL_CALL dispose() override;

        virtual void SAL_CALL createPeer(const css::uno::Reference< css::awt::XToolkit > & rToolkit, const css::uno::Reference< css::awt::XWindowPeer > & rParentPeer) override;

    protected:
        virtual FmXGridPeer*    imp_CreatePeer(vcl::Window* pParent) override;
    };

    // SbaXGridPeer

    class SbaXGridPeer final
                :public FmXGridPeer
                ,public css::frame::XDispatch
    {
        cppu::OMultiTypeInterfaceContainerHelperVar<
            css::util::URL, void, SbaURLCompare>  m_aStatusListeners;

    public:
        SbaXGridPeer(const css::uno::Reference< css::uno::XComponentContext >&);
        virtual ~SbaXGridPeer() override;

        // UNO
        virtual void SAL_CALL  acquire() throw() override { FmXGridPeer::acquire(); }
        virtual void SAL_CALL release() throw() override { FmXGridPeer::release(); }
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

        static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId();
        sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 > & rId ) override;
        static SbaXGridPeer* getImplementation(const css::uno::Reference< css::uno::XInterface>& _rxIFace);

        // css::frame::XDispatch
        virtual void SAL_CALL dispatch(const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& aArgs) override;
        virtual void SAL_CALL addStatusListener(const css::uno::Reference< css::frame::XStatusListener > & xControl, const css::util::URL& aURL) override;
        virtual void SAL_CALL removeStatusListener(const css::uno::Reference< css::frame::XStatusListener > & xControl, const css::util::URL& aURL) override;

        // css::frame::XDispatchProvider
        virtual css::uno::Reference< css::frame::XDispatch >  SAL_CALL queryDispatch(const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) override;

        // css::lang::XComponent
        virtual void SAL_CALL dispose() override;

    private:
        virtual VclPtr<FmGridControl>  imp_CreateControl(vcl::Window* pParent, WinBits nStyle) override;
#ifdef _MSC_VER
        typedef css::frame::XStatusListener xstlist_type;
        typedef css::uno::Reference< xstlist_type > xlistener_type;
        void NotifyStatusChanged(const css::util::URL& aUrl, const xlistener_type & xControl = xlistener_type() );
#else
        void NotifyStatusChanged(const css::util::URL& aUrl, const css::uno::Reference< css::frame::XStatusListener > & xControl);
#endif // # _MSC_VER

        // for any execution of XDispatch::dispatch
        struct DispatchArgs
        {
            css::util::URL                                                 aURL;
            css::uno::Sequence< css::beans::PropertyValue >   aArgs;
        };
        std::queue< DispatchArgs >    m_aDispatchArgs;
        DECL_LINK( OnDispatchEvent, void*, void );

        // for dynamic states of our 4 dispatchable URLs
        enum DispatchType
        {
            dtBrowserAttribs,
            dtRowHeight,
            dtColumnAttribs,
            dtColumnWidth,

            dtUnknown
        };
        static DispatchType classifyDispatchURL( const css::util::URL& _rURL );

        typedef std::map<DispatchType, sal_Bool> MapDispatchToBool;
        MapDispatchToBool   m_aDispatchStates;
    };

    // SbaGridHeader

    class SbaGridHeader
                :public FmGridHeader
                ,public DragSourceHelper
    {
    public:
        SbaGridHeader(BrowseBox* pParent);
        virtual void dispose() override;
        virtual ~SbaGridHeader() override;
    protected:

        // FmGridHeader overridables
        virtual void    PreExecuteColumnContextMenu(sal_uInt16 nColId, PopupMenu& rMenu) override;
        virtual void    PostExecuteColumnContextMenu(sal_uInt16 nColId, const PopupMenu& rMenu, sal_uInt16 nExecutionResult) override;

    private:
        // DragSourceHelper overridables
        virtual void StartDrag( sal_Int8 _nAction, const Point& _rPosPixel ) override;

        // Window overridables
        virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;

        void ImplStartColumnDrag(sal_Int8 _nAction, const Point& _rMousePos);
    };

    // interfaces for communication between the vcl grid control and a controller
    class SbaGridListener
    {
    public:
        virtual void RowChanged()       = 0;
        virtual void ColumnChanged()    = 0;
        virtual void SelectionChanged() = 0;
        virtual void CellActivated()    = 0;
        virtual void CellDeactivated()  = 0;
        virtual void BeforeDrop()       = 0;
        virtual void AfterDrop()        = 0;

    protected:
        ~SbaGridListener() {}
    };

    // SbaGridControl
    class SbaGridControl final : public FmGridControl
    {
        friend class SbaGridHeader;
        friend class SbaXGridPeer;

    // Attributes
        svx::ODataAccessDescriptor    m_aDataDescriptor;
        SbaGridListener*              m_pMasterListener;

        ImplSVEvent *                 m_nAsyncDropEvent;

        bool                          m_bActivatingForDrop;

    public:
        SbaGridControl(css::uno::Reference< css::uno::XComponentContext > const & _rM, Window* pParent, FmXGridPeer* _pPeer, WinBits nBits = WB_TABSTOP);
        virtual ~SbaGridControl() override;
        virtual void dispose() override;

        virtual void Select() override;

        void SetMasterListener(SbaGridListener* pListener)  { m_pMasterListener = pListener; }

        virtual void ActivateCell(long nRow, sal_uInt16 nCol, bool bSetCellFocus = true) override;
        virtual void DeactivateCell(bool bUpdate = true) override;
        using FmGridControl::ActivateCell;

        bool IsAllSelected() const { return (GetSelectRowCount() == GetRowCount()) && (GetRowCount() > 0); }

        HeaderBar* GetHeaderBar() const { return FmGridControl::GetHeaderBar(); }

        /** return the description of the specified object.
            @param  eObjType
                The type to ask for
            @param  _nPosition
                The position of a tablecell (index position), header bar  column/row cell
            @return
                The description of the specified object.
        */
        virtual OUString GetAccessibleObjectDescription( ::svt::AccessibleBrowseBoxObjType eObjType,sal_Int32 _nPosition = -1) const override;

        using FmGridControl::DeleteSelectedRows;
        /** copies the currently selected rows to the clipboard
            @precond
                at least one row is selected
        */
        void CopySelectedRowsToClipboard();

    private:
        // DragSourceHelper overridables
        virtual void StartDrag( sal_Int8 _nAction, const Point& _rPosPixel ) override;

        // BrowseBox overridables
        virtual sal_Int8 AcceptDrop( const BrowserAcceptDropEvent& rEvt ) override;
        virtual sal_Int8 ExecuteDrop( const BrowserExecuteDropEvent& rEvt ) override;
        virtual void    MouseButtonDown( const BrowserMouseEvent& rMEvt) override;

        // EditBrowseBox overridables
        virtual VclPtr<BrowserHeader> imp_CreateHeaderBar(BrowseBox* pParent) override;
        virtual ::svt::CellController* GetController(long nRow, sal_uInt16 nCol) override;

        // DbGridControl overridables
        virtual void PreExecuteRowContextMenu(sal_uInt16 nRow, PopupMenu& rMenu) override;
        virtual void PostExecuteRowContextMenu(sal_uInt16 nRow, const PopupMenu& rMenu, sal_uInt16 nExecutionResult) override;

        // DbGridControl overridables
        virtual void onRowChange() override;
        virtual void onColumnChange() override;

        // get a fields property set from a model pos
        css::uno::Reference< css::beans::XPropertySet >  getField(sal_uInt16 nModelPos);

        // get my data source
        css::uno::Reference< css::beans::XPropertySet >  getDataSource() const;

        // drag events
        void DoColumnDrag(sal_uInt16 nColumnPos);
        void DoFieldDrag(sal_uInt16 nColumnPos, sal_Int16 nRowPos);

        void SetBrowserAttrs();
        void SetColWidth(sal_uInt16 nColId);
        void SetRowHeight();
        void SetColAttrs(sal_uInt16 nColId);

        SvNumberFormatter* GetDatasourceFormatter();

        DECL_LINK(AsynchDropEvent, void*, void);

        bool IsReadOnlyDB() const;
        void implTransferSelectedRows( sal_Int16 nRowPos, bool _bTrueIfClipboardFalseIfDrag );

        using FmGridControl::AcceptDrop;
        using FmGridControl::ExecuteDrop;
        using FmGridControl::MouseButtonDown;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_SBAGRID_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
