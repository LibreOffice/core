/*************************************************************************
 *
 *  $RCSfile: sbagrid.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: fs $ $Date: 2001-02-19 10:48:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SBA_GRID_HXX
#define _SBA_GRID_HXX

//  #include <sdb/tools.hxx>

#ifndef _SVX_FMGRIDCL_HXX
#include <svx/fmgridcl.hxx>
#endif

#ifndef _SVX_FMGRIDIF_HXX
#include <svx/fmgridif.hxx>
#endif

#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _SBA_MULTIPLEX_HXX
#include "sbamultiplex.hxx"
#endif

class SvNumberFormatter;

namespace dbaui
{
    //------------------------------------------------------------------
    struct SbaURLCompare : public binary_function< ::com::sun::star::util::URL, ::com::sun::star::util::URL, bool>
    {
        bool operator() (const ::com::sun::star::util::URL& x, const ::com::sun::star::util::URL& y) const {return x.Complete < y.Complete;}
    };

    struct SbaURLHash
    {
        sal_Int32 operator() (const ::com::sun::star::util::URL& x) const {return x.Complete.hashCode();}
    };
    //==================================================================
    // SbaXGridControl
    //==================================================================

    //  class ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > ;
    FORWARD_DECLARE_INTERFACE(lang,XMultiServiceFactory)
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SbaXGridControl_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > &) throw( ::com::sun::star::uno::Exception );

    class SbaXStatusMultiplexer;
    class SbaXGridControl
                :public FmXGridControl
                ,public ::com::sun::star::frame::XDispatch
    {
        DECLARE_STL_MAP(::com::sun::star::util::URL, SbaXStatusMultiplexer*, SbaURLCompare,StatusMultiplexerArray);
        StatusMultiplexerArray      m_aStatusMultiplexer;
        //  ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_MultiServiceFacatory;

    public:
        SbaXGridControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);
        virtual ~SbaXGridControl();

        // UNO
        DECLARE_UNO3_DEFAULTS(SbaXGridControl, FmXGridControl);
        virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XServiceInfo
        ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
        // need by registration
        static ::rtl::OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

        // ::com::sun::star::frame::XDispatch
        virtual void SAL_CALL dispatch(const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL addStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL removeStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::lang::XComponent
        virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL createPeer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > & rToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > & rParentPeer) throw( ::com::sun::star::uno::RuntimeException );

    protected:
        virtual FmXGridPeer*    imp_CreatePeer(Window* pParent);
    };

    //==================================================================
    // SbaXGridPeer
    //==================================================================

    //  Reflection* ::getCppuType((const SbaXGridPeer*)0);

    class SbaXGridPeer
                :public FmXGridPeer
                ,public ::com::sun::star::frame::XDispatch
    {
        //  friend Reflection* ::getCppuType((const SbaXGridPeer*)0);

        ::cppu::OMultiTypeInterfaceContainerHelperVar< ::com::sun::star::util::URL,SbaURLHash , SbaURLCompare>  m_aStatusListeners;

    public:
        SbaXGridPeer(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);
        virtual ~SbaXGridPeer();

        // UNO
        virtual void SAL_CALL  acquire() { FmXGridPeer::acquire(); }
        virtual void SAL_CALL release() { FmXGridPeer::release(); }
        virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);

        static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();
        sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 > & rId );
        static SbaXGridPeer* getImplementation(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rxIFace);

        // ::com::sun::star::frame::XDispatch
        virtual void SAL_CALL dispatch(const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL addStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL removeStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::frame::XDispatchProvider
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  SAL_CALL queryDispatch(const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::view::XSelectionChangeListener
        virtual void SAL_CALL selectionChanged(const ::com::sun::star::lang::EventObject& aEvent);

        // ::com::sun::star::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt);

        // FmXGridPeer overridables
        virtual void addColumnListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xCol);
        virtual void removeColumnListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xCol);

        // ::com::sun::star::lang::XComponent
        virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException );

    protected:
        virtual FmGridControl*  imp_CreateControl(Window* pParent, WinBits nStyle);

        void NotifyStatusChanged(const ::com::sun::star::util::URL& aUrl, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > ());
    };

    //==================================================================
    // SbaGridHeader
    //==================================================================

    class SbaGridHeader : public FmGridHeader
    {
    public:
        SbaGridHeader(BrowseBox* pParent, WinBits nWinBits = WB_STDHEADERBAR | WB_DRAG);

    protected:
        // Window overridables
        virtual void Command( const CommandEvent& rCEvt );
        virtual void Select();

        // FmGridHeader overridables
        virtual void    PreExecuteColumnContextMenu(sal_uInt16 nColId, PopupMenu& rMenu);
        virtual void    PostExecuteColumnContextMenu(sal_uInt16 nColId, const PopupMenu& rMenu, sal_uInt16 nExecutionResult);

    private:
        // Window overridables
        void MouseButtonDown( const MouseEvent& rMEvt );

        void ImplSelect(sal_uInt16 nId);

        sal_Bool ImplStartColumnDrag(const Point& _rMousePos);
    };

    // =========================================================================
    // interfaces for communication between the vcl grid control and a controller
    // -------------------------------------------------------------------------
    class SbaGridListener
    {
    public:
        virtual void RowChanged()       = 0;
        virtual void ColumnChanged()    = 0;
        virtual void SelectionChanged() = 0;
        virtual void CellActivated()    = 0;
        virtual void CellDeactivated()  = 0;
    };

    //==================================================================
    // SbaGridControl
    //==================================================================
    //------------------------------------------------------------------
    class UnoDataBrowserController;
    class SbaGridControl : public FmGridControl
    {
        friend class SbaGridHeader;
        friend class SbaXGridPeer;

    // Attributes
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer >        m_xComposer;    // for DnD we need a composed query ...
        long        m_nCurrentSelectedColumn;   // this is the column model (not the view) posisition ...
        SbaGridListener*    m_pMasterListener;
        sal_uInt16  m_nLastColId;
        sal_uInt16  m_nLastRowId;

        sal_uInt16  m_nCurrentActionColId;
            // ui actions (e.g. a context menu) may be performed on columns which aren't the current one
            // and aren't selected, so we have to track this column id

        sal_Bool    m_bSelecting;

        sal_Bool    m_bActivatingForDrop;
    // Attribute Access
    public:
        long    GetSelectedColumn() const       { return m_nCurrentSelectedColumn; }
        sal_uInt16  GetCurrentActionColumn() const  { return m_nCurrentActionColId; }

    public:
        SbaGridControl(::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >,Window* pParent, FmXGridPeer* _pPeer, WinBits nBits = WB_TABSTOP);
        virtual ~SbaGridControl();

        void SetMasterListener(SbaGridListener* pListener)  { m_pMasterListener = pListener; }

        virtual void ActivateCell(long nRow, sal_uInt16 nCol, sal_Bool bSetCellFocus = sal_True);
        virtual void DeactivateCell(sal_Bool bUpdate = sal_True);
        void ActivateCell() { FmGridControl::ActivateCell(); }

        sal_Bool IsAllSelected() const { return (GetSelectRowCount() == GetRowCount()) && (GetRowCount() > 0); }

        virtual void setDataSource(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > & rCursor,
            sal_uInt16 nOpts = OPT_INSERT | OPT_UPDATE | OPT_DELETE);
            // m_xComposer has to be initialized depending on the data source

        HeaderBar* GetHeaderBar() const { return FmGridControl::GetHeaderBar(); }

    protected:
        // Window overridables
        virtual void Command(const CommandEvent& rEvt);

        // BrowseBox overridables
        virtual void    Select();
        virtual void    CursorMoved();
        virtual sal_Bool    QueryDrop(const BrowserDropEvent& rEvt);
        virtual sal_Bool    Drop(const BrowserDropEvent& rEvt);
        virtual void    MouseButtonDown( const BrowserMouseEvent& rMEvt);

        // DbBrowseBox overridables
        virtual BrowserHeader* imp_CreateHeaderBar(BrowseBox* pParent);
        virtual DbCellController* GetController(long nRow, sal_uInt16 nCol);

        // DbGridControl overridables
        virtual void PreExecuteRowContextMenu(sal_uInt16 nRow, PopupMenu& rMenu);
        virtual void PostExecuteRowContextMenu(sal_uInt16 nRow, const PopupMenu& rMenu, sal_uInt16 nExecutionResult);

        // my own overridables
        virtual void RowChanged();  // the default implementation calls the according link (if set)
        virtual void ColChanged();

        // get a fields property set from a model pos
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  getField(sal_uInt16 nModelPos);

        // get my data source
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  getDataSource() const;

        // drag events
        virtual void DoColumnDrag(sal_uInt16 nColumnPos);
        virtual void DoRowDrag(sal_uInt16 nRowPos);
        virtual void DoFieldDrag(sal_uInt16 nColumnPos, sal_uInt16 nRowPos);

        void refresh();

        void SetBrowserAttrs();
        void SetColWidth(sal_uInt16 nColId);
        void SetRowHeight();
        void SetColAttrs(sal_uInt16 nColId);

        SvNumberFormatter* GetDatasourceFormatter();

    private:
        sal_Bool    IsReadOnlyDB() const;
    };
}
#endif // _SBA_GRID_HXX

