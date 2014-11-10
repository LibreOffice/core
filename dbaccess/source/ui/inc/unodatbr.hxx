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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_UNODATBR_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_UNODATBR_HXX

#include "brwctrlr.hxx"
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/i18n/XCollator.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <com/sun/star/ui/XContextMenuInterception.hpp>
#include <com/sun/star/sdb/application/DatabaseObject.hpp>
#include <com/sun/star/sdb/application/DatabaseObjectContainer.hpp>
#include <com/sun/star/sdb/XDatabaseRegistrationsListener.hpp>
#include <cppuhelper/implbase5.hxx>
#include "callbacks.hxx"
#include <vcl/timer.hxx>
#include <svtools/transfer.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <sot/storage.hxx>
#include "TableCopyHelper.hxx"
#include "commontypes.hxx"

class SvTreeListEntry;
class Splitter;
struct SvSortData;

namespace com { namespace sun{ namespace star { namespace container { class XNameContainer; } } } }

class SvTreeList;
namespace dbaui
{

    class DBTreeView;
    struct DBTreeEditedEntry;
    class ImageProvider;

    typedef ::cppu::ImplHelper5 <   ::com::sun::star::frame::XStatusListener
                                ,   ::com::sun::star::view::XSelectionSupplier
                                ,   ::com::sun::star::document::XScriptInvocationContext
                                ,   ::com::sun::star::ui::XContextMenuInterception
                                ,   ::com::sun::star::sdb::XDatabaseRegistrationsListener
                                >   SbaTableQueryBrowser_Base;
    class SbaTableQueryBrowser
                :public SbaXDataBrowserController
                ,public SbaTableQueryBrowser_Base
                ,public IControlActionListener
                ,public IContextMenuProvider
    {
    protected:

        ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCollator >   m_xCollator;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >     m_xCurrentFrameParent;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >      m_xMainToolbar;

        struct ExternalFeature
        {
            ::com::sun::star::util::URL     aURL;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >
                                            xDispatcher;
            bool                        bEnabled;

            ExternalFeature() : bEnabled( false ) { }
            ExternalFeature( const ::com::sun::star::util::URL& _rURL ) : aURL( _rURL ), bEnabled( false ) { }
        };

        typedef ::std::map< sal_uInt16, ExternalFeature, ::std::less< sal_uInt16 > >  ExternalFeaturesMap;
        ExternalFeaturesMap     m_aExternalFeatures;

        ::svx::ODataAccessDescriptor    m_aDocumentDataSource;
            // if we're part of a document, this is the state of the DocumentDataSource slot

        ::cppu::OInterfaceContainerHelper   m_aSelectionListeners;
        ::cppu::OInterfaceContainerHelper   m_aContextMenuInterceptors;

        OTableCopyHelper::DropDescriptor    m_aAsyncDrop;
        OTableCopyHelper                    m_aTableCopyHelper;

        OUString         m_sQueryCommand;    // the command of the query currently loaded (if any)
        //OUString         m_sToBeLoaded;      // contains the element name which should be loaded if any

        DBTreeView*             m_pTreeView;
        Splitter*               m_pSplitter;
        SvTreeList*         m_pTreeModel;           // contains the datasources of the registry
        SvTreeListEntry*            m_pCurrentlyDisplayed;
        ImplSVEvent *               m_nAsyncDrop;

        sal_Int16               m_nBorder;              // sal_True when border should be shown

        bool                m_bQueryEscapeProcessing : 1;   // the escape processing flag of the query currently loaded (if any)
        bool                m_bShowMenu;            // if sal_True the menu should be visible otherwise not
        bool                m_bInSuspend;
        bool                m_bEnableBrowser;
        ::boost::optional< bool >
                                m_aDocScriptSupport;    // relevant if and only if we are associated with exactly one DBDoc

        virtual OUString getPrivateTitle( ) const SAL_OVERRIDE;
    // attribute access
    public:
        SbaTableQueryBrowser(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rM);
        virtual ~SbaTableQueryBrowser();

        enum EntryType
        {
            // don't change the above definitions! There are places (in particular SbaTableQueryBrowser::getCurrentSelection)
            // which rely on the fact that the EntryType values really equal the DatabaseObject(Container) values!
            etDatasource     = ::com::sun::star::sdb::application::DatabaseObjectContainer::DATA_SOURCE,
            etQueryContainer = ::com::sun::star::sdb::application::DatabaseObjectContainer::QUERIES,
            etTableContainer = ::com::sun::star::sdb::application::DatabaseObjectContainer::TABLES,
            etQuery          = ::com::sun::star::sdb::application::DatabaseObject::QUERY,
            etTableOrView    = ::com::sun::star::sdb::application::DatabaseObject::TABLE,
            etUnknown        = -1
        };

        /** returns a DatabaseObject value corresponding to the given EntryType
            @param _eType
                the entry type. Must not be etUnknown.
        */
        static sal_Int32    getDatabaseObjectType( EntryType _eType );

        // need by registration
        static OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

        DECLARE_UNO3_DEFAULTS(SbaTableQueryBrowser,SbaXDataBrowserController)
        // late construction
        virtual bool Construct(vcl::Window* pParent) SAL_OVERRIDE;
        // XInterface
        virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ::com::sun::star::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ::com::sun::star::frame::XController
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual void SAL_CALL attachFrame(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & xFrame) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // ::com::sun::star::lang::XComponent
        virtual void        SAL_CALL disposing() SAL_OVERRIDE;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XSelectionSupplier
        virtual sal_Bool SAL_CALL select( const ::com::sun::star::uno::Any& aSelection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Any SAL_CALL getSelection(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL addSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        // ::com::sun::star::frame::XFrameActionListener
        virtual void SAL_CALL frameAction(const ::com::sun::star::frame::FrameActionEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        //IController
        virtual void notifyHiContrastChanged() SAL_OVERRIDE;

        // XScriptInvocationContext
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedScripts > SAL_CALL getScriptContainer() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XContextMenuInterception
        virtual void SAL_CALL registerContextMenuInterceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XContextMenuInterceptor >& Interceptor ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL releaseContextMenuInterceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XContextMenuInterceptor >& Interceptor ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XDatabaseRegistrationsListener
        virtual void SAL_CALL registeredDatabaseLocation( const ::com::sun::star::sdb::DatabaseRegistrationEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL revokedDatabaseLocation( const ::com::sun::star::sdb::DatabaseRegistrationEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL changedDatabaseLocation( const ::com::sun::star::sdb::DatabaseRegistrationEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    protected:
        // SbaXDataBrowserController overridables
        virtual bool     InitializeForm( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& i_formProperties ) SAL_OVERRIDE;
        virtual bool     InitializeGridModel(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > & xGrid) SAL_OVERRIDE;

        virtual bool     preReloadForm() SAL_OVERRIDE;
        virtual void     postReloadForm() SAL_OVERRIDE;

        virtual void addModelListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel) SAL_OVERRIDE;
        virtual void removeModelListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel) SAL_OVERRIDE;

        virtual void AddColumnListener(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xCol) SAL_OVERRIDE;
        virtual void RemoveColumnListener(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xCol) SAL_OVERRIDE;

        virtual void LoadFinished(bool _bWasSynch) SAL_OVERRIDE;

        virtual void criticalFail() SAL_OVERRIDE;

        virtual void            describeSupportedFeatures() SAL_OVERRIDE;
        virtual FeatureState    GetState(sal_uInt16 nId) const SAL_OVERRIDE;
        virtual void            Execute(sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) SAL_OVERRIDE;

        // IControlActionListener overridables
        virtual bool        requestQuickHelp( const SvTreeListEntry* _pEntry, OUString& _rText ) const SAL_OVERRIDE;
        virtual bool        requestDrag( sal_Int8 _nAction, const Point& _rPosPixel ) SAL_OVERRIDE;
        virtual sal_Int8    queryDrop( const AcceptDropEvent& _rEvt, const DataFlavorExVector& _rFlavors ) SAL_OVERRIDE;
        virtual sal_Int8    executeDrop( const ExecuteDropEvent& _rEvt ) SAL_OVERRIDE;

        // IContextMenuProvider
        virtual PopupMenu*      getContextMenu( Control& _rControl ) const SAL_OVERRIDE;
        virtual IController&    getCommandController() SAL_OVERRIDE;
        virtual ::cppu::OInterfaceContainerHelper*
                                getContextMenuInterceptors() SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Any
                                getCurrentSelection( Control& _rControl ) const SAL_OVERRIDE;

        virtual void impl_initialize() SAL_OVERRIDE;

        // SbaGridListener overridables
        virtual void RowChanged() SAL_OVERRIDE;
        virtual void ColumnChanged() SAL_OVERRIDE;
        virtual void SelectionChanged() SAL_OVERRIDE;

        // methods for showing/hiding the explorer part
        bool        haveExplorer() const;
        void        hideExplorer();
        void        showExplorer();
        void        toggleExplorer() { if (haveExplorer()) hideExplorer(); else showExplorer(); }

        // methods for handling the 'selection' (paintin them bold) of SvLBoxEntries
        // returns <TRUE/> if the entry is selected (which means it's part of the selected path)
        bool        isSelected(SvTreeListEntry* _pEntry) const;
        // select the entry (and only the entry, not the whole path)
        void        select(SvTreeListEntry* _pEntry, bool _bSelect = true);
        // select the path of the entry (which must be an entry without children)
        void        selectPath(SvTreeListEntry* _pEntry, bool _bSelect = true);

        virtual void loadMenu(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _xFrame) SAL_OVERRIDE;

    private:
        // check the state of the external slot given, update any UI elements if necessary
        void implCheckExternalSlot( sal_uInt16 _nId );

        // connect to the external dispatchers (if any)
        void connectExternalDispatches();

        /** get the state of an external slot
            <p>The slot is available if an external dispatcher is responsible for it, _and_ if this dispatcher
            told us the slot is available.</p>
        */
        bool getExternalSlotState( sal_uInt16 _nId ) const;

        /** add an entry (including the subentries for queries/tables) to the list model

            <p>The given names and images may be empty, in this case they're filled with the correct
            values. This way they may be reused for the next call, which saves some resource manager calls.</p>
        */
        void implAddDatasource(const OUString& _rDbName, Image& _rDbImage,
                OUString& _rQueryName, Image& _rQueryImage,
                OUString& _rTableName, Image& _rTableImage,
                const SharedConnection& _rxConnection
            );

        void    implAddDatasource( const OUString& _rDataSourceName, const SharedConnection& _rxConnection );

        /// removes (and cleans up) the entry for the given data source
        void        impl_cleanupDataSourceEntry( const OUString& _rDataSourceName );

        /// clears the tree list box
        void clearTreeModel();

        /** unloads the form, empties the grid model, cleans up anything related to the currently displayed object
            @param _bDisposeConnection
                <TRUE/> if the connection should be disposed
            @param _bFlushData
                <TRUE/> if the currently displayed object (if any) should be flushed
        */
        void unloadAndCleanup( bool _bDisposeConnection = true );

        // disposes the connection associated with the given entry (which must represent a data source)
        void        disposeConnection( SvTreeListEntry* _pDSEntry );

        /// flushs and disposes the given connection, and de-registers as listener
        void        impl_releaseConnection( SharedConnection& _rxConnection );

        /** close the connection (and collapse the list entries) of the given list entries
        */
        void        closeConnection(SvTreeListEntry* _pEntry, bool _bDisposeConnection = true);

        void        populateTree(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xNameAccess, SvTreeListEntry* _pParent, EntryType _eEntryType);
        void        initializeTreeModel();

        /** search in the tree for query- or tablecontainer equal to this interface and return
            this container entry
        */
        SvTreeListEntry* getEntryFromContainer(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _rxNameAccess);
        // return true when there is connection available
        bool ensureConnection(SvTreeListEntry* _pDSEntry, void * pDSData, SharedConnection& _rConnection );
        bool ensureConnection(SvTreeListEntry* _pAnyEntry, SharedConnection& _rConnection );

        bool getExistentConnectionFor( SvTreeListEntry* _pDSEntry, SharedConnection& _rConnection );
        /** returns an image provider which works with the connection belonging to the given entry
        */
        ::std::unique_ptr< ImageProvider >
                getImageProviderFor( SvTreeListEntry* _pAnyEntry );

        void    implAdministrate( SvTreeListEntry* _pApplyTo );

        TransferableHelper*
                implCopyObject( SvTreeListEntry* _pApplyTo, sal_Int32 _nCommandType, bool _bAllowConnection = true );

        EntryType getEntryType( const SvTreeListEntry* _pEntry ) const;
        EntryType   getChildType( SvTreeListEntry* _pEntry ) const;
        bool    isObject( EntryType _eType ) const { return ( etTableOrView== _eType ) || ( etQuery == _eType ); }
        bool    isContainer( EntryType _eType ) const { return (etTableContainer == _eType) || (etQueryContainer == _eType); }
        bool isContainer( const SvTreeListEntry* _pEntry ) const { return isContainer( getEntryType( _pEntry ) ); }

        // ensure that the xObject for the given entry is set on the user data
        bool          ensureEntryObject( SvTreeListEntry* _pEntry );

        // get the display text of the entry given
        OUString      GetEntryText( SvTreeListEntry* _pEntry ) const;

        // is called when a table or a query was selected
        DECL_LINK( OnSelectionChange, void* );
        DECL_LINK( OnExpandEntry, SvTreeListEntry* );

        DECL_LINK( OnCopyEntry, void* );

        DECL_LINK( OnTreeEntryCompare, const SvSortData* );

        DECL_LINK( OnAsyncDrop, void* );

        void implRemoveStatusListeners();

        bool implSelect(const ::svx::ODataAccessDescriptor& _rDescriptor, bool _bSelectDirect = false);
        bool implSelect( SvTreeListEntry* _pEntry );

        /// selects the entry given and loads the grid control with the object's data
        bool implSelect(
            const OUString& _rDataSourceName,
            const OUString& _rCommand,
            const sal_Int32 _nCommandType,
            const bool _bEscapeProcessing,
            const SharedConnection& _rxConnection,
            bool _bSelectDirect = false
        );

        SvTreeListEntry* implGetConnectionEntry(SvTreeListEntry* _pEntry) const;
        /// inserts an entry into the tree
        SvTreeListEntry* implAppendEntry(
            SvTreeListEntry* _pParent,
            const OUString& _rName,
            void* _pUserData,
            EntryType _eEntryType
        );

        /// loads the grid control with the data object specified (which may be a table, a query or a command)
        bool implLoadAnything(const OUString& _rDataSourceName, const OUString& _rCommand,
            const sal_Int32 _nCommandType, const bool _bEscapeProcessing, const SharedConnection& _rxConnection = SharedConnection() );

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
        SvTreeListEntry* getObjectEntry(const ::svx::ODataAccessDescriptor& _rDescriptor,
            SvTreeListEntry** _ppDataSourceEntry = NULL, SvTreeListEntry** _ppContainerEntry = NULL,
            bool _bExpandAncestors = true
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
        SvTreeListEntry* getObjectEntry(
            const OUString& _rDataSource, const OUString& _rCommand, sal_Int32 _nCommandType,
            SvTreeListEntry** _ppDataSourceEntry = NULL, SvTreeListEntry** _ppContainerEntry = NULL,
            bool _bExpandAncestors = true,
            const SharedConnection& _rxConnection = SharedConnection()
        );

        /// checks if m_aDocumentDataSource describes a known object
        void checkDocumentDataSource();

        void extractDescriptorProps(const ::svx::ODataAccessDescriptor& _rDescriptor,
            OUString& _rDataSource, OUString& _rCommand, sal_Int32& _rCommandType, bool& _rEscapeProcessing);

        void transferChangedControlProperty(const OUString& _rProperty, const ::com::sun::star::uno::Any& _rNewValue);

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > connectWithStatus(
            const OUString& _rDataSourceName,
            void* _pTreeListUserData    // in reality a DBTreeListUserData*, but we do not know this class here...
        );

#if OSL_DEBUG_LEVEL > 0
        // checks whether the given tree entry denotes a data source
        bool impl_isDataSourceEntry( SvTreeListEntry* _pEntry ) const;
#endif

        /// retrieves the data source URL/name for the given entry representing a data source
        OUString  getDataSourceAcessor( SvTreeListEntry* _pDataSourceEntry ) const;

        /** get the signature (command/escape processing) of the query the form is based on
            <p>If the for is not based on a query or not even loaded, nothing happens and <FALSE/> is returned.</p>
        */
        bool implGetQuerySignature( OUString& _rCommand, bool& _bEscapeProcessing );

        bool isEntryCopyAllowed(SvTreeListEntry* _pEntry) const;

        void copyEntry(SvTreeListEntry* _pEntry);

        // remove all grid columns and dispose them
        void clearGridColumns(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _xColContainer);

        /** checks if the currently displayed entry changed
            @param  _sName
                    Name of the changed entry
            @param  _pContainer
                    The container of the displayed entry
            @return
                    <TRUE/> if it is the currently displayed otherwise <FALSE/>
        */
        bool isCurrentlyDisplayedChanged(const OUString& _sName, SvTreeListEntry* _pContainer);

        /** called whenever the content of the browser is used for preview, as the very last action
            of the load process
        */
        void initializePreviewMode();

        /** checks whether the Order/Filter clauses set at our row set are valid, removes them if not so
        */
        void    impl_sanitizeRowSetClauses_nothrow();
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_UNODATBR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
