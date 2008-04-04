/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unodatbr.hxx,v $
 *
 *  $Revision: 1.71 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 15:00:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SBA_UNODATBR_HXX_
#define _SBA_UNODATBR_HXX_

#ifndef _SBA_BWRCTRLR_HXX
#include "brwctrlr.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
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
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XSCRIPTINVOCATIONCONTEXT_HPP_
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
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
#ifndef DBUI_TABLECOPYHELPER_HXX
#include "TableCopyHelper.hxx"
#endif
#ifndef _DBAUI_COMMON_TYPES_HXX_
#include "commontypes.hxx"
#endif

// =========================================================================
class SvLBoxEntry;
class Splitter;
struct SvSortData;

#define CONTAINER_QUERIES       sal_Int32( etQuery - etQuery )
#define CONTAINER_TABLES        sal_Int32( etTableOrView - etQuery )

namespace com { namespace sun{ namespace star { namespace container { class XNameContainer; } } } }
// .........................................................................
namespace dbaui
{
// .........................................................................

    class DBTreeView;
    class DBTreeListModel;
    struct DBTreeEditedEntry;
    class ImageProvider;

    // =====================================================================
    typedef ::cppu::ImplHelper3 <   ::com::sun::star::frame::XStatusListener
                                ,   ::com::sun::star::view::XSelectionSupplier
                                ,   ::com::sun::star::document::XScriptInvocationContext
                                >   SbaTableQueryBrowser_Base;
    class SbaTableQueryBrowser
                :public SbaXDataBrowserController
                ,public SbaTableQueryBrowser_Base
                ,public IControlActionListener
    {
    protected:

        // ---------------------------
        ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCollator >   m_xCollator;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >     m_xCurrentFrameParent;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >      m_xMainToolbar;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >     m_xCurrentDatabaseDocument;

        ::osl::Mutex            m_aEntryMutex;

        // ---------------------------
        struct ExternalFeature
        {
            ::com::sun::star::util::URL     aURL;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >
                                            xDispatcher;
            sal_Bool                        bEnabled;

            ExternalFeature() : bEnabled( sal_False ) { }
            ExternalFeature( const ::com::sun::star::util::URL& _rURL ) : aURL( _rURL ), bEnabled( sal_False ) { }
        };

        typedef ::std::map< sal_uInt16, ExternalFeature, ::std::less< sal_uInt16 > >  ExternalFeaturesMap;
        ExternalFeaturesMap     m_aExternalFeatures;

        ::svx::ODataAccessDescriptor    m_aDocumentDataSource;
            // if we're part of a document, this is the state of the DocumentDataSource slot

        ::cppu::OInterfaceContainerHelper   m_aSelectionListeners;

        OTableCopyHelper::DropDescriptor    m_aAsyncDrop;
        OTableCopyHelper                    m_aTableCopyHelper;

        ::rtl::OUString         m_sQueryCommand;    // the command of the query currently loaded (if any)

        DBTreeView*             m_pTreeView;
        Splitter*               m_pSplitter;
        DBTreeListModel*        m_pTreeModel;           // contains the datasources of the registry
        SvLBoxEntry*            m_pCurrentlyDisplayed;
        sal_Int32               m_nAsyncDrop;

        sal_Int16               m_nBorder;              // TRUE when border should be shown

        sal_Bool                m_bQueryEscapeProcessing : 1;   // the escape processing flag of the query currently loaded (if any)
        sal_Bool                m_bShowMenu;            // if TRUE the menu should be visible otherwise not
        sal_Bool                m_bInSuspend;
        sal_Bool                m_bEnableBrowser;
        ::boost::optional< bool >
                                m_aDocScriptSupport;    // relevant if and only if we are associated with exactly one DBDoc


        virtual ::rtl::OUString getPrivateTitle( ) const;
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
            etTableOrView,
            etUnknown
        };
        /** returns a DatabaseObject value corresponding to the given EntryType
            @param _eType
                the entry type. Must not be etUnknown.
        */
        static sal_Int32    getDatabaseObjectType( EntryType _eType );

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

        // XScriptInvocationContext
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedScripts > SAL_CALL getScriptContainer() throw (::com::sun::star::uno::RuntimeException);

    protected:
        // SbaXDataBrowserController overridables
        virtual sal_Bool InitializeForm(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > & xForm);
        virtual sal_Bool InitializeGridModel(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > & xGrid);

        virtual sal_Bool preReloadForm();
        virtual void     postReloadForm();

        virtual void addModelListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel);
        virtual void removeModelListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel);

        virtual void AddColumnListener(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xCol);
        virtual void RemoveColumnListener(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xCol);

        virtual void LoadFinished(sal_Bool _bWasSynch);

        virtual void criticalFail();

        virtual void            describeSupportedFeatures();
        virtual FeatureState    GetState(sal_uInt16 nId) const;
        virtual void            Execute(sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);

        // IControlActionListener overridables
        virtual sal_Bool    requestContextMenu( const CommandEvent& _rEvent );
        virtual sal_Bool    requestQuickHelp( const SvLBoxEntry* _pEntry, String& _rText ) const;
        virtual sal_Bool    requestDrag( sal_Int8 _nAction, const Point& _rPosPixel );
        virtual sal_Int8    queryDrop( const AcceptDropEvent& _rEvt, const DataFlavorExVector& _rFlavors );
        virtual sal_Int8    executeDrop( const ExecuteDropEvent& _rEvt );

        virtual void impl_initialize();

        // SbaGridListener overridables
        virtual void RowChanged();
        virtual void ColumnChanged();
        virtual void SelectionChanged();

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
        void implCheckExternalSlot( sal_uInt16 _nId );

        // connect to the external dispatchers (if any)
        void connectExternalDispatches();

        /** get the state of an external slot
            <p>The slot is available if an external dispatcher is responsible for it, _and_ if this dispatcher
            told us the slot is available.</p>
        */
        sal_Bool    getExternalSlotState( sal_uInt16 _nId ) const;

        /** add an entry (including the subentries for queries/tables) to the list model

            <p>The given names and images may be empty, in this case they're filled with the correct
            values. This way they may be reused for the next call, which saves some resource manager calls.</p>
        */
        void implAddDatasource(const String& _rDbName, Image& _rDbImage,
                String& _rQueryName, Image& _rQueryImage,
                String& _rTableName, Image& _rTableImage,
                const SharedConnection& _rxConnection
            );

        /// clears the tree list box
        void clearTreeModel();

        /** unloads the form, empties the grid model, cleans up anything related to the currently displayed object
            @param _bDisposeConnection
                <TRUE/> if the connection should be disposed
            @param _bFlushData
                <TRUE/> if the currently displayed object (if any) should be flushed
        */
        void unloadAndCleanup( sal_Bool _bDisposeConnection = sal_True );

        // disposes the connection associated with the given entry (which must represent a data source)
        void        disposeConnection( SvLBoxEntry* _pDSEntry );

        /// flushs and disposes the given connection, and de-registers as listener
        void        impl_releaseConnection( SharedConnection& _rxConnection );

        /** close the connection (and collapse the list entries) of the given list entries
        */
        void        closeConnection(SvLBoxEntry* _pEntry,sal_Bool _bDisposeConnection = sal_True);

        sal_Bool    populateTree(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xNameAccess, SvLBoxEntry* _pParent, EntryType _eEntryType);
        void        initializeTreeModel();

        /** search in the tree for query- or tablecontainer equal to this interface and return
            this container entry
        */
        SvLBoxEntry* getEntryFromContainer(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _rxNameAccess);
        // return true when there is connection available
        sal_Bool ensureConnection(SvLBoxEntry* _pDSEntry, void * pDSData, SharedConnection& _rConnection );
        sal_Bool ensureConnection(SvLBoxEntry* _pAnyEntry, SharedConnection& _rConnection );

        sal_Bool getExistentConnectionFor( SvLBoxEntry* _pDSEntry, SharedConnection& _rConnection );
        /** returns an image provider which works with the connection belonging to the given entry
        */
        ::std::auto_ptr< ImageProvider >
                getImageProviderFor( SvLBoxEntry* _pAnyEntry );

        void    implAdministrate( SvLBoxEntry* _pApplyTo );

        TransferableHelper*
                implCopyObject( SvLBoxEntry* _pApplyTo, sal_Int32 _nCommandType, sal_Bool _bAllowConnection = sal_True );

        EntryType   getEntryType( SvLBoxEntry* _pEntry ) const;
        EntryType   getChildType( SvLBoxEntry* _pEntry ) const;
        sal_Bool    isObject( EntryType _eType ) const { return ( etTableOrView== _eType ) || ( etQuery == _eType ); }
        sal_Bool    isContainer( EntryType _eType ) const { return (etTableContainer == _eType) || (etQueryContainer == _eType); }
        sal_Bool    isContainer( SvLBoxEntry* _pEntry ) const { return isContainer( getEntryType( _pEntry ) ); }

        // ensure that the xObject for the given entry is set on the user data
        sal_Bool    ensureEntryObject( SvLBoxEntry* _pEntry );

        // get the display text of the entry given
        String      GetEntryText( SvLBoxEntry* _pEntry ) const;

        // is called when a table or a query was selected
        DECL_LINK( OnSelectEntry, SvLBoxEntry* );
        DECL_LINK( OnExpandEntry, SvLBoxEntry* );

        DECL_LINK( OnCopyEntry, SvLBoxEntry* );

        DECL_LINK( OnTreeEntryCompare, const SvSortData* );

        DECL_LINK( OnAsyncDrop, void* );

        void implRemoveStatusListeners();

        sal_Bool implSelect(const ::svx::ODataAccessDescriptor& _rDescriptor,sal_Bool _bSelectDirect = sal_False);

        /// selects the entry given and loads the grid control with the object's data
        sal_Bool implSelect(
            const ::rtl::OUString& _rDataSourceName,
            const ::rtl::OUString& _rCommand,
            const sal_Int32 _nCommandType,
            const sal_Bool _bEscapeProcessing,
            const SharedConnection& _rxConnection,
            sal_Bool _bSelectDirect = sal_False
        );

        /// inserts an entry into the tree
        void implAppendEntry(
            SvLBoxEntry* _pParent,
            const String& _rName,
            void* _pUserData,
            EntryType _eEntryType
        );

        /// loads the grid control with the data object specified (which may be a table, a query or a command)
        sal_Bool implLoadAnything(const ::rtl::OUString& _rDataSourceName, const ::rtl::OUString& _rCommand,
            const sal_Int32 _nCommandType, const sal_Bool _bEscapeProcessing, const SharedConnection& _rxConnection = SharedConnection() );

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
            sal_Bool _bExpandAncestors = sal_True,
            const SharedConnection& _rxConnection = SharedConnection()
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

#ifdef DBG_UTIL
        // checks whether the given tree entry denotes a data source
        bool impl_isDataSourceEntry( SvLBoxEntry* _pEntry ) const;
#endif

        /// retrieves the data source URL/name for the given entry representing a data source
        String  getDataSourceAcessor( SvLBoxEntry* _pDataSourceEntry ) const;

        /** get the signature (command/escape processing) of the query the form is based on
            <p>If the for is not based on a query or not even loaded, nothing happens and <FALSE/> is returned.</p>
        */
        sal_Bool implGetQuerySignature( ::rtl::OUString& _rCommand, sal_Bool& _bEscapeProcessing );

        sal_Bool isEntryCopyAllowed(SvLBoxEntry* _pEntry) const;

        void copyEntry(SvLBoxEntry* _pEntry);

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

        /** called whenever the content of the browser is used for preview, as the very last action
            of the load process
        */
        void initializePreviewMode();
    };

// .........................................................................
}   // namespace dbaui
// .........................................................................

#endif // _SBA_UNODATBR_HXX_

