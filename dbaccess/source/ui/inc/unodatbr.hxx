/*************************************************************************
 *
 *  $RCSfile: unodatbr.hxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-23 10:53:50 $
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

#ifndef _SBA_UNODATBR_HXX_
#define _SBA_UNODATBR_HXX_

#ifndef _SBA_BWRCTRLR_HXX
#include "brwctrlr.hxx"
#endif

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_XCOLLATOR_HPP_
#include <com/sun/star/i18n/XCollator.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _DBACCESS_UI_CALLBACKS_HXX_
#include "callbacks.hxx"
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

// =========================================================================
class SvLBoxEntry;
class Splitter;
struct SvSortData;

#define CONTAINER_QUERIES       0
#define CONTAINER_TABLES        1

// .........................................................................
namespace dbaui
{
// .........................................................................

    class DBTreeView;
    class DBTreeListModel;
    // =====================================================================
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::frame::XStatusListener
                                >   SbaTableQueryBrowser_Base;
    class SbaTableQueryBrowser
                :public SbaXDataBrowserController
                ,public SbaTableQueryBrowser_Base
                ,public IControlActionListener
    {
    protected:
        ::osl::Mutex            m_aEntryMutex;
        DBTreeView*             m_pTreeView;
        Splitter*               m_pSplitter;
        DBTreeListModel*        m_pTreeModel;           // contains the datasources of the registry
        SvLBoxEntry*            m_pCurrentlyDisplayed;
        sal_Int32               m_nAsyncDrop;

        DECLARE_STL_STDKEY_MAP( sal_Int32, ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >, SpecialSlotDispatchers);
        DECLARE_STL_STDKEY_MAP( sal_Int32, sal_Bool, SpecialSlotStates);
        SpecialSlotDispatchers  m_aDispatchers;         // external dispatchers for slots we do not execute ourself
        SpecialSlotStates       m_aDispatchStates;      // states of the slots handled by external dispatchers

        ::rtl::OUString         m_sDefaultDataSourceName;
        ::rtl::OUString         m_sDefaultCommand;
        sal_Int32               m_nDefaultCommandType;
        sal_Bool                m_bHideTreeView;

        ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCollator >
                                m_xCollator;

        // ---------------------------
        struct DropDescriptor
        {
            TransferableDataHelper  aDroppedData;
            SvLBoxEntry*            pDroppedAt;
            sal_Bool                bTable;

            DropDescriptor() : pDroppedAt(NULL), bTable(sal_True) { }
        };
        DropDescriptor              m_aAsyncDrop;

    // attribute access
    public:
        SbaTableQueryBrowser(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM);
        ~SbaTableQueryBrowser();

        // need by registration
        static ::rtl::OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

        DECLARE_UNO3_DEFAULTS(SbaTableQueryBrowser,OGenericUnoController);
        // late construction
        virtual sal_Bool Construct(Window* pParent);

        // ::com::sun::star::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt);

        // ::com::sun::star::frame::XController
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL attachFrame(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & xFrame) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::lang::XComponent
        virtual void        SAL_CALL disposing();

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw(::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
        // lang::XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);

    protected:
        // SbaXDataBrowserController overridables
        virtual sal_Bool InitializeForm(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > & xForm);
        virtual sal_Bool InitializeGridModel(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > & xGrid);

        virtual ToolBox* CreateToolBox(Window* pParent);

        virtual void InvalidateFeature(sal_uInt16 nId, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xListener = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > ());

        virtual void addModelListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel);
        virtual void removeModelListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel);

        virtual void AddColumnListener(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xCol);
        virtual void RemoveColumnListener(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xCol);

        virtual void            AddSupportedFeatures();
        virtual FeatureState    GetState(sal_uInt16 nId);
        virtual void            Execute(sal_uInt16 nId);

        // IControlActionListener overridables
        virtual sal_Bool    requestContextMenu( const CommandEvent& _rEvent );
        virtual sal_Bool    requestDrag( sal_Int8 _nAction, const Point& _rPosPixel );
        virtual sal_Int8    queryDrop( const AcceptDropEvent& _rEvt, const DataFlavorExVector& _rFlavors );
        virtual sal_Int8    executeDrop( const ExecuteDropEvent& _rEvt );

        String getURL() const;

        // methods for showing/hiding the explorer part
        sal_Bool    haveExplorer() const;
        void        hideExplorer();
        void        showExplorer();
        void        toggleExplorer() { if (haveExplorer()) hideExplorer(); else showExplorer(); }

        // methods for handling the 'selection' (paintin them bold) of SvLBoxEntries
        // returns <TRUE/> if the entry is selected (which means it's part of the selected path)
        sal_Bool    isSelected(SvLBoxEntry* _pEntry) const;
        // select the entry (and only the entry, not the whole path)
        void        select(SvLBoxEntry* _pEntry, sal_Bool _bSelect = sal_True);
        // select the path of the entry (which must be an entry without children)
        void        selectPath(SvLBoxEntry* _pEntry, sal_Bool _bSelect = sal_True);

    private:
        // check the state of the external slot given, update any UI elements if necessary
        void implCheckExternalSlot(sal_Int32 _nId);

        /** add an entry (including the subentries for queries/tables) to the list model

            <p>The given names and images may be empty, in this case they're filled with the correct
            values. This way they may be reused for the next call, which saves some resource manager calls.</p>
        */
        void implAddDatasource(const String& _rDbName, Image& _rDbImage,
                String& _rQueryName, Image& _rQueryImage,
                String& _rTableName, Image& _rTableImage);

        /** unloads the form, empties the grid model
            @param _bDisposeConnection
                <TRUE/> if the connection should be disposed
            @param _bFlushData
                <TRUE/> if the currently displayed object (if any) should be flushed
        */
        void unloadForm(sal_Bool _bDisposeConnection = sal_True, sal_Bool _bFlushData = sal_True);

        /** close the connection (and collapse the list entries) of the given list entries
        */
        void closeConnection(SvLBoxEntry* _pEntry,sal_Bool _bDisposeConnection = sal_True);

        sal_Bool    populateTree(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xNameAccess, SvLBoxEntry* _pParent, const Image& _rImage);
        void        initializeTreeModel();

        /** search in the tree for query- or tablecontainer equal to this interface and return
            this container entry
        */
        SvLBoxEntry* getNameAccessFromEntry(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _rxNameAccess);
        // return true when there is connection available
        sal_Bool ensureConnection(SvLBoxEntry* _pDSEntry,void * pDSData,::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);
        sal_Bool ensureConnection(SvLBoxEntry* _pAnyEntry, ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);

        void    implAdministrate( SvLBoxEntry* _pApplyTo );
        void    implCreateObject( SvLBoxEntry* _pApplyTo, sal_uInt16 _nAction );
        void    implRemoveQuery( SvLBoxEntry* _pApplyTo );
        void    implDropTable( SvLBoxEntry* _pApplyTo );
        void    implPasteTable( SvLBoxEntry* _pApplyTo, const TransferableDataHelper& _rPasteData );
        void    implPasteQuery( SvLBoxEntry* _pApplyTo, const TransferableDataHelper& _rPasteData );

        TransferableHelper*
                implCopyObject( SvLBoxEntry* _pApplyTo, sal_Int32 _nCommandType, sal_Bool _bAllowConnection = sal_True );

        enum EntryType
        {
            ET_DATASOURCE,
            ET_TABLE_CONTAINER,
            ET_QUERY_CONTAINER,
            ET_TABLE,
            ET_QUERY,
            ET_UNKNOWN
        };
        EntryType   getEntryType( SvLBoxEntry* _pEntry );
        sal_Bool    isObject( EntryType _eType ) { return (ET_TABLE == _eType) || (ET_QUERY == _eType); }
        sal_Bool    isContainer( EntryType _eType ) { return (ET_TABLE_CONTAINER == _eType) || (ET_QUERY_CONTAINER == _eType); }

        // ensure that the xObject for the given entry is set on the user data
        sal_Bool    ensureEntryObject( SvLBoxEntry* _pEntry );

        // get the display text of the entry given
        String      getEntryText( SvLBoxEntry* _pEntry );

        // is called when a table or a query was selected
        DECL_LINK( OnSelectEntry, SvLBoxEntry* );
        DECL_LINK( OnExpandEntry, SvLBoxEntry* );
        DECL_LINK( OnTreeEntryCompare, const SvSortData* );
        DECL_LINK( OnAsyncDrop, void* );

        void implRemoveStatusListeners();
    };

// .........................................................................
}   // namespace dbaui
// .........................................................................

#endif // _SBA_UNODATBR_HXX_

