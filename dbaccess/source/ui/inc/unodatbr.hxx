/*************************************************************************
 *
 *  $RCSfile: unodatbr.hxx,v $
 *
 *  $Revision: 1.53 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:04:02 $
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
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _DBACCESS_UI_CALLBACKS_HXX_
#include "callbacks.hxx"
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif
#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif

// =========================================================================
class SvLBoxEntry;
class Splitter;
struct SvSortData;

#define CONTAINER_QUERIES       sal_Int32(etQuery - etQuery)
#define CONTAINER_TABLES        sal_Int32(etTable - etQuery)

namespace com { namespace sun{ namespace star { namespace container { class XNameContainer; } } } }
// .........................................................................
namespace dbaui
{
// .........................................................................

    class DBTreeView;
    class DBTreeListModel;
    struct DBTreeEditedEntry;
    // =====================================================================
    typedef ::cppu::ImplHelper2 <   ::com::sun::star::frame::XStatusListener
                                ,   ::com::sun::star::view::XSelectionSupplier
                                >   SbaTableQueryBrowser_Base;
    class SbaTableQueryBrowser
                :public SbaXDataBrowserController
                ,public SbaTableQueryBrowser_Base
                ,public IControlActionListener
    {
    protected:
        // ---------------------------
        DECLARE_STL_STDKEY_MAP( sal_Int32, ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >, SpecialSlotDispatchers);
        DECLARE_STL_STDKEY_MAP( sal_Int32, sal_Bool, SpecialSlotStates);

        struct DropDescriptor
        {
            ::svx::ODataAccessDescriptor    aDroppedData;
            String                          aUrl;
            SotStorageStreamRef             aHtmlRtfStorage;
            SvLBoxEntry*                    pDroppedAt;
            sal_Bool                        bTable;
            sal_Bool                        bHtml;
            sal_Bool                        bError;

            DropDescriptor() : pDroppedAt(NULL), bTable(sal_True) { }
        };

        // ---------------------------
        ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCollator >   m_xCollator;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >     m_xCurrentFrameParent;

        ::osl::Mutex            m_aEntryMutex;

        SpecialSlotDispatchers  m_aDispatchers;         // external dispatchers for slots we do not execute ourself
        SpecialSlotStates       m_aDispatchStates;      // states of the slots handled by external dispatchers

        ::svx::ODataAccessDescriptor    m_aDocumentDataSource;
            // if we're part of a document, this is the state of the DocumentDataSource slot

        ::cppu::OInterfaceContainerHelper   m_aSelectionListeners;

        DropDescriptor          m_aAsyncDrop;
        Timer                   m_aRefreshMenu;     // the timer for the menu "refresh"

        ::rtl::OUString         m_sQueryCommand;    // the command of the query currently loaded (if any)

        DBTreeView*             m_pTreeView;
        Splitter*               m_pSplitter;
        DBTreeListModel*        m_pTreeModel;           // contains the datasources of the registry
        SvLBoxEntry*            m_pCurrentlyDisplayed;

        sal_Int16               m_nBorder;              // TRUE when border should be shown

        sal_Bool                m_bQueryEscapeProcessing : 1;   // the escape processing flag of the query currently loaded (if any)
        sal_Bool                m_bHiContrast;          // in which mode we are
        sal_Bool                m_bShowMenu;            // if TRUE the menu should be visible otherwise not
        sal_Bool                m_bShowToolbox;         // if TRUE the toolbox should be visible otherwise not
        sal_Bool                m_bPreview;             // if TRUE the grid will hide some features


    // attribute access
    public:
        SbaTableQueryBrowser(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM);
        ~SbaTableQueryBrowser();

        enum EntryType
        {
            etDatasource,
            etQueryContainer,
            etTableContainer,
            etQuery,
            etTable,
            etView,
            etUnknown
        };

        // need by registration
        static ::rtl::OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

        DECLARE_UNO3_DEFAULTS(SbaTableQueryBrowser,SbaXDataBrowserController);
        // late construction
        virtual sal_Bool Construct(Window* pParent);
        // XInterface
        virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException);

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::frame::XController
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL attachFrame(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & xFrame) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::lang::XComponent
        virtual void        SAL_CALL disposing();

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw(::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

        // XSelectionSupplier
        virtual sal_Bool SAL_CALL select( const ::com::sun::star::uno::Any& aSelection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getSelection(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

        // XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        // ::com::sun::star::frame::XFrameActionListener
        virtual void SAL_CALL frameAction(const ::com::sun::star::frame::FrameActionEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException );

        //IController
        virtual void notifyHiContrastChanged();

    protected:
        // SbaXDataBrowserController overridables
        virtual sal_Bool InitializeForm(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > & xForm);
        virtual sal_Bool InitializeGridModel(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > & xGrid);

        virtual ToolBox* CreateToolBox(Window* pParent);
#if defined(_MSC_VER) && (_MSC_VER >= 1310 )
        typedef ::com::sun::star::frame::XStatusListener xstlist_type;
        typedef ::com::sun::star::uno::Reference< xstlist_type > xlister_type;
        virtual void InvalidateFeature(sal_uInt16 nId, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xListener = xlistener_type() );
#else
        virtual void InvalidateFeature(sal_uInt16 nId, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xListener = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > ());
#endif //_MSC_VER >= 1310

        virtual void addModelListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel);
        virtual void removeModelListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel);

        virtual void AddColumnListener(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xCol);
        virtual void RemoveColumnListener(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xCol);

        virtual void LoadFinished(sal_Bool _bWasSynch);

        virtual void criticalFail();

        virtual void            AddSupportedFeatures();
        virtual FeatureState    GetState(sal_uInt16 nId) const;
        virtual void            Execute(sal_uInt16 nId);

        virtual void onToolBoxSelected( sal_uInt16 _nSelectedItem );
        virtual void onToolBoxClicked( sal_uInt16 _nClickedItem );

        // IControlActionListener overridables
        virtual sal_Bool    requestContextMenu( const CommandEvent& _rEvent );
        virtual sal_Bool    requestDrag( sal_Int8 _nAction, const Point& _rPosPixel );
        virtual sal_Int8    queryDrop( const AcceptDropEvent& _rEvt, const DataFlavorExVector& _rFlavors );
        virtual sal_Int8    executeDrop( const ExecuteDropEvent& _rEvt );

        virtual void impl_initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments );

        // SbaGridListener overridables
        virtual void RowChanged();
        virtual void ColumnChanged();

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

        virtual void loadMenu(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _xFrame);

    private:
        // check the state of the external slot given, update any UI elements if necessary
        void implCheckExternalSlot(sal_Int32 _nId);

        // connect to the external dispatchers (if any)
        void connectExternalDispatches();

        /** get the state of an external slot
            <p>The slot is available if an external dispatcher is responsible for it, _and_ if this dispatcher
            told us the slot is available.</p>
        */
        sal_Bool    getExternalSlotState( sal_Int32 _nId ) const;

        /** add an entry (including the subentries for queries/tables) to the list model

            <p>The given names and images may be empty, in this case they're filled with the correct
            values. This way they may be reused for the next call, which saves some resource manager calls.</p>
        */
        void implAddDatasource(const String& _rDbName, Image& _rDbImage,
                String& _rQueryName, Image& _rQueryImage,
                String& _rTableName, Image& _rTableImage);

        /** unloads the form, empties the grid model, cleans up anything related to the currently displayed object
            @param _bDisposeConnection
                <TRUE/> if the connection should be disposed
            @param _bFlushData
                <TRUE/> if the currently displayed object (if any) should be flushed
        */
        void unloadAndCleanup(sal_Bool _bDisposeConnection = sal_True, sal_Bool _bFlushData = sal_True);

        // disposes the connection associated with the given entry (which must represent a data source)
        void        disposeConnection( SvLBoxEntry* _pDSEntry );

        /** close the connection (and collapse the list entries) of the given list entries
        */
        void        closeConnection(SvLBoxEntry* _pEntry,sal_Bool _bDisposeConnection = sal_True);

        sal_Bool    populateTree(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xNameAccess, SvLBoxEntry* _pParent, const EntryType& _rEntryType);
        void        initializeTreeModel();

        /** search in the tree for query- or tablecontainer equal to this interface and return
            this container entry
        */
        SvLBoxEntry* getEntryFromContainer(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _rxNameAccess);
        // return true when there is connection available
        sal_Bool ensureConnection(SvLBoxEntry* _pDSEntry,void * pDSData,::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);
        sal_Bool ensureConnection(SvLBoxEntry* _pAnyEntry, ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);

        void    implAdministrate( SvLBoxEntry* _pApplyTo );
        void    implDirectSQL( SvLBoxEntry* _pApplyTo );

        TransferableHelper*
                implCopyObject( SvLBoxEntry* _pApplyTo, sal_Int32 _nCommandType, sal_Bool _bAllowConnection = sal_True );

        EntryType   getEntryType( SvLBoxEntry* _pEntry ) const;
        EntryType   getChildType( SvLBoxEntry* _pEntry ) const;
        sal_Bool    isObject( EntryType _eType ) const { return (etTable == _eType) || (etView == _eType) || (etQuery == _eType);}
        sal_Bool    isContainer( EntryType _eType ) const { return (etTableContainer == _eType) || (etQueryContainer == _eType); }
        sal_Bool    isContainer( SvLBoxEntry* _pEntry ) const { return isContainer( getEntryType( _pEntry ) ); }

        // ensure that the xObject for the given entry is set on the user data
        sal_Bool    ensureEntryObject( SvLBoxEntry* _pEntry );

        // get the display text of the entry given
        String      GetEntryText( SvLBoxEntry* _pEntry );

        // is called when a table or a query was selected
        DECL_LINK( OnEntryDoubleClicked, SvLBoxEntry* );
        DECL_LINK( OnSelectEntry, SvLBoxEntry* );
        DECL_LINK( OnExpandEntry, SvLBoxEntry* );

        DECL_LINK( OnCopyEntry, SvLBoxEntry* );

        DECL_LINK( OnTreeEntryCompare, const SvSortData* );

        DECL_LINK( OnShowRefreshDropDown, void* );

        void implRemoveStatusListeners();

        sal_Bool implSelect(const ::svx::ODataAccessDescriptor& _rDescriptor,sal_Bool _bSelectDirect = sal_False);

        /// selects the entry given and loads the grid control with the object's data
        sal_Bool implSelect(const ::rtl::OUString& _rDataSourceName, const ::rtl::OUString& _rCommand,
            const sal_Int32 _nCommandType, const sal_Bool _bEscapeProcessing,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConnection=NULL
            ,sal_Bool _bSelectDirect = sal_False);

        /// loads the grid control with the data object specified (which may be a table, a query or a command)
        sal_Bool implLoadAnything(const ::rtl::OUString& _rDataSourceName, const ::rtl::OUString& _rCommand,
            const sal_Int32 _nCommandType, const sal_Bool _bEscapeProcessing, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _rxConnection = NULL);

        /** retrieves the tree entry for the object described by <arg>_rDescriptor</arg>
            @param _rDescriptor
                the object descriptor
            @param _ppDataSourceEntry
                If not <NULL/>, the data source tree entry will be returned here
            @param _ppContainerEntry
                If not <NULL/>, the object container tree entry will be returned here
            @param _bExpandAncestors
                If <TRUE/>, all ancestor on the way to the entry will be expanded
        */
        SvLBoxEntry* getObjectEntry(const ::svx::ODataAccessDescriptor& _rDescriptor,
            SvLBoxEntry** _ppDataSourceEntry = NULL, SvLBoxEntry** _ppContainerEntry = NULL,
            sal_Bool _bExpandAncestors = sal_True
        );
        /** retrieves the tree entry for the object described by data source name, command and command type
            @param _rDataSource
                the data source name
            @param _rCommand
                the command
            @param _nCommandType
                the command type
            @param _rDescriptor
                the object descriptor
            @param _ppDataSourceEntry
                If not <NULL/>, the data source tree entry will be returned here
            @param _ppContainerEntry
                If not <NULL/>, the object container tree entry will be returned here
            @param _bExpandAncestors
                If <TRUE/>, all ancestor on the way to the entry will be expanded
        */
        SvLBoxEntry* getObjectEntry(
            const ::rtl::OUString& _rDataSource, const ::rtl::OUString& _rCommand, sal_Int32 _nCommandType,
            SvLBoxEntry** _ppDataSourceEntry = NULL, SvLBoxEntry** _ppContainerEntry = NULL,
            sal_Bool _bExpandAncestors = sal_True
        );

        /// checks if m_aDocumentDataSource describes a known object
        void checkDocumentDataSource();

        void extractDescriptorProps(const ::svx::ODataAccessDescriptor& _rDescriptor,
            ::rtl::OUString& _rDataSource, ::rtl::OUString& _rCommand, sal_Int32& _rCommandType, sal_Bool& _rEscapeProcessing);

        void transferChangedControlProperty(const ::rtl::OUString& _rProperty, const ::com::sun::star::uno::Any& _rNewValue);

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > connectWithStatus(
            const ::rtl::OUString& _rDataSourceName,
            void* _pTreeListUserData    // in rela a DBTreeListModel::DBTreeListUserData*, but we do not know this class here ....
        );

        // set _rsName as title at the frame
        void setTitle(const ::rtl::OUString& _rsDataSourceName,const ::rtl::OUString& _rsName) const;
        void setDefaultTitle() const;

        /** get the signature (command/escape processing) of the query the form is based on
            <p>If the for is not based on a query or not even loaded, nothing happens and <FALSE/> is returned.</p>
        */
        sal_Bool implGetQuerySignature( ::rtl::OUString& _rCommand, sal_Bool& _bEscapeProcessing );

        sal_Bool isEntryCutAllowed(SvLBoxEntry* _pEntry) const;
        sal_Bool isEntryCopyAllowed(SvLBoxEntry* _pEntry) const;
        sal_Bool isEntryPasteAllowed(SvLBoxEntry* _pEntry) const;

        void copyEntry(SvLBoxEntry* _pEntry);

        void ensureObjectExists(SvLBoxEntry* _pApplyTo);
        // remove all grid columns and dispose them
        void clearGridColumns(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _xColContainer);

        sal_Bool isHiContrast() const;


        /** checks if the currently displayed entry changed
            @param  _sName
                    Name of the changed entry
            @param  _pContainer
                    The container of the displayed entry
            @return
                    <TRUE/> if it is the currently displayed otherwise <FALSE/>
        */
        sal_Bool isCurrentlyDisplayedChanged(const String& _sName,SvLBoxEntry* _pContainer);
    };

// .........................................................................
}   // namespace dbaui
// .........................................................................

#endif // _SBA_UNODATBR_HXX_

