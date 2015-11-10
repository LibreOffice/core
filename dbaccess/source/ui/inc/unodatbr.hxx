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

    typedef ::cppu::ImplHelper5 <   css::frame::XStatusListener
                                ,   css::view::XSelectionSupplier
                                ,   css::document::XScriptInvocationContext
                                ,   css::ui::XContextMenuInterception
                                ,   css::sdb::XDatabaseRegistrationsListener
                                >   SbaTableQueryBrowser_Base;
    class SbaTableQueryBrowser
                :public SbaXDataBrowserController
                ,public SbaTableQueryBrowser_Base
                ,public IControlActionListener
                ,public IContextMenuProvider
    {
    protected:

        css::uno::Reference< css::i18n::XCollator >   m_xCollator;
        css::uno::Reference< css::frame::XFrame >     m_xCurrentFrameParent;
        css::uno::Reference< css::awt::XWindow >      m_xMainToolbar;

        struct ExternalFeature
        {
            css::util::URL              aURL;
            css::uno::Reference< css::frame::XDispatch >
                                        xDispatcher;
            bool                        bEnabled;

            ExternalFeature() : bEnabled( false ) { }
            ExternalFeature( const css::util::URL& _rURL ) : aURL( _rURL ), bEnabled( false ) { }
        };

        typedef ::std::map< sal_uInt16, ExternalFeature, ::std::less< sal_uInt16 > >  ExternalFeaturesMap;
        ExternalFeaturesMap     m_aExternalFeatures;

        svx::ODataAccessDescriptor    m_aDocumentDataSource;
            // if we're part of a document, this is the state of the DocumentDataSource slot

        ::cppu::OInterfaceContainerHelper   m_aSelectionListeners;
        ::cppu::OInterfaceContainerHelper   m_aContextMenuInterceptors;

        OTableCopyHelper::DropDescriptor    m_aAsyncDrop;
        OTableCopyHelper                    m_aTableCopyHelper;

        OUString         m_sQueryCommand;    // the command of the query currently loaded (if any)
        //OUString         m_sToBeLoaded;      // contains the element name which should be loaded if any

        VclPtr<DBTreeView>      m_pTreeView;
        VclPtr<Splitter>        m_pSplitter;
        SvTreeList*             m_pTreeModel;           // contains the datasources of the registry
        SvTreeListEntry*        m_pCurrentlyDisplayed;
        ImplSVEvent *           m_nAsyncDrop;

        sal_Int16               m_nBorder;              // sal_True when border should be shown

        bool                m_bQueryEscapeProcessing : 1;   // the escape processing flag of the query currently loaded (if any)
        bool                m_bShowMenu;            // if sal_True the menu should be visible otherwise not
        bool                m_bInSuspend;
        bool                m_bEnableBrowser;
        ::boost::optional< bool >
                                m_aDocScriptSupport;    // relevant if and only if we are associated with exactly one DBDoc

        virtual OUString getPrivateTitle( ) const override;
    // attribute access
    public:
        SbaTableQueryBrowser(const css::uno::Reference< css::uno::XComponentContext >& _rM);
        virtual ~SbaTableQueryBrowser();

        enum EntryType
        {
            // don't change the above definitions! There are places (in particular SbaTableQueryBrowser::getCurrentSelection)
            // which rely on the fact that the EntryType values really equal the DatabaseObject(Container) values!
            etDatasource     = css::sdb::application::DatabaseObjectContainer::DATA_SOURCE,
            etQueryContainer = css::sdb::application::DatabaseObjectContainer::QUERIES,
            etTableContainer = css::sdb::application::DatabaseObjectContainer::TABLES,
            etQuery          = css::sdb::application::DatabaseObject::QUERY,
            etTableOrView    = css::sdb::application::DatabaseObject::TABLE,
            etUnknown        = -1
        };

        /** returns a DatabaseObject value corresponding to the given EntryType
            @param _eType
                the entry type. Must not be etUnknown.
        */
        static sal_Int32    getDatabaseObjectType( EntryType _eType );

        // need by registration
        static OUString getImplementationName_Static() throw( css::uno::RuntimeException );
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( css::uno::RuntimeException );
        static css::uno::Reference< css::uno::XInterface >
                SAL_CALL Create(const css::uno::Reference< css::lang::XMultiServiceFactory >&);

        DECLARE_UNO3_DEFAULTS(SbaTableQueryBrowser,SbaXDataBrowserController)
        // late construction
        virtual bool Construct(vcl::Window* pParent) override;
        // XInterface
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) throw (css::uno::RuntimeException, std::exception) override;

        // XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (css::uno::RuntimeException, std::exception) override;

        // css::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& evt) throw(css::uno::RuntimeException, std::exception) override;

        // css::frame::XController
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL attachFrame(const css::uno::Reference< css::frame::XFrame > & xFrame) throw( css::uno::RuntimeException, std::exception ) override;

        // css::lang::XComponent
        virtual void        SAL_CALL disposing() override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw(css::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override;

        // XSelectionSupplier
        virtual sal_Bool SAL_CALL select( const css::uno::Any& aSelection ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getSelection(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

        // XContainerListener
        virtual void SAL_CALL elementInserted( const css::container::ContainerEvent& Event ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL elementRemoved( const css::container::ContainerEvent& Event ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL elementReplaced( const css::container::ContainerEvent& Event ) throw(css::uno::RuntimeException, std::exception) override;
        // css::frame::XFrameActionListener
        virtual void SAL_CALL frameAction(const css::frame::FrameActionEvent& aEvent) throw( css::uno::RuntimeException, std::exception ) override;

        //IController
        virtual void notifyHiContrastChanged() override;

        // XScriptInvocationContext
        virtual css::uno::Reference< css::document::XEmbeddedScripts > SAL_CALL getScriptContainer() throw (css::uno::RuntimeException, std::exception) override;

        // XContextMenuInterception
        virtual void SAL_CALL registerContextMenuInterceptor( const css::uno::Reference< css::ui::XContextMenuInterceptor >& Interceptor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL releaseContextMenuInterceptor( const css::uno::Reference< css::ui::XContextMenuInterceptor >& Interceptor ) throw (css::uno::RuntimeException, std::exception) override;

        // XDatabaseRegistrationsListener
        virtual void SAL_CALL registeredDatabaseLocation( const css::sdb::DatabaseRegistrationEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL revokedDatabaseLocation( const css::sdb::DatabaseRegistrationEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL changedDatabaseLocation( const css::sdb::DatabaseRegistrationEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;

    protected:
        // SbaXDataBrowserController overridables
        virtual bool     InitializeForm( const css::uno::Reference< css::beans::XPropertySet >& i_formProperties ) override;
        virtual bool     InitializeGridModel(const css::uno::Reference< css::form::XFormComponent > & xGrid) override;

        virtual bool     preReloadForm() override;
        virtual void     postReloadForm() override;

        virtual void addModelListeners(const css::uno::Reference< css::awt::XControlModel > & _xGridControlModel) override;
        virtual void removeModelListeners(const css::uno::Reference< css::awt::XControlModel > & _xGridControlModel) override;

        virtual void AddColumnListener(const css::uno::Reference< css::beans::XPropertySet > & xCol) override;
        virtual void RemoveColumnListener(const css::uno::Reference< css::beans::XPropertySet > & xCol) override;

        virtual void LoadFinished(bool _bWasSynch) override;

        virtual void criticalFail() override;

        virtual void            describeSupportedFeatures() override;
        virtual FeatureState    GetState(sal_uInt16 nId) const override;
        virtual void            Execute(sal_uInt16 nId, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) override;

        // IControlActionListener overridables
        virtual bool        requestQuickHelp( const SvTreeListEntry* _pEntry, OUString& _rText ) const override;
        virtual bool        requestDrag( sal_Int8 _nAction, const Point& _rPosPixel ) override;
        virtual sal_Int8    queryDrop( const AcceptDropEvent& _rEvt, const DataFlavorExVector& _rFlavors ) override;
        virtual sal_Int8    executeDrop( const ExecuteDropEvent& _rEvt ) override;

        // IContextMenuProvider
        virtual PopupMenu*      getContextMenu( Control& _rControl ) const override;
        virtual IController&    getCommandController() override;
        virtual ::cppu::OInterfaceContainerHelper*
                                getContextMenuInterceptors() override;
        virtual css::uno::Any
                                getCurrentSelection( Control& _rControl ) const override;

        virtual void impl_initialize() override;

        // SbaGridListener overridables
        virtual void RowChanged() override;
        virtual void ColumnChanged() override;
        virtual void SelectionChanged() override;

        // methods for showing/hiding the explorer part
        bool        haveExplorer() const;
        void        hideExplorer();
        void        showExplorer();
        void        toggleExplorer() { if (haveExplorer()) hideExplorer(); else showExplorer(); }

        // methods for handling the 'selection' (painting them bold) of SvLBoxEntries
        // returns <TRUE/> if the entry is selected (which means it's part of the selected path)
        static bool isSelected(SvTreeListEntry* _pEntry);
        // select the entry (and only the entry, not the whole path)
        void        select(SvTreeListEntry* _pEntry, bool _bSelect = true);
        // select the path of the entry (which must be an entry without children)
        void        selectPath(SvTreeListEntry* _pEntry, bool _bSelect = true);

        virtual void loadMenu(const css::uno::Reference< css::frame::XFrame >& _xFrame) override;

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

        /// flushes and disposes the given connection, and de-registers as listener
        void        impl_releaseConnection( SharedConnection& _rxConnection );

        /** close the connection (and collapse the list entries) of the given list entries
        */
        void        closeConnection(SvTreeListEntry* _pEntry, bool _bDisposeConnection = true);

        void        populateTree(const css::uno::Reference< css::container::XNameAccess>& _xNameAccess, SvTreeListEntry* _pParent, EntryType _eEntryType);
        void        initializeTreeModel();

        /** search in the tree for query- or tablecontainer equal to this interface and return
            this container entry
        */
        SvTreeListEntry* getEntryFromContainer(const css::uno::Reference< css::container::XNameAccess>& _rxNameAccess);
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
        static bool    isObject( EntryType _eType ) { return ( etTableOrView== _eType ) || ( etQuery == _eType ); }
        static bool    isContainer( EntryType _eType ) { return (etTableContainer == _eType) || (etQueryContainer == _eType); }
        bool isContainer( const SvTreeListEntry* _pEntry ) const { return isContainer( getEntryType( _pEntry ) ); }

        // ensure that the xObject for the given entry is set on the user data
        bool          ensureEntryObject( SvTreeListEntry* _pEntry );

        // get the display text of the entry given
        OUString      GetEntryText( SvTreeListEntry* _pEntry ) const;

        // is called when a table or a query was selected
        DECL_LINK_TYPED( OnSelectionChange, LinkParamNone*, void );
        DECL_LINK_TYPED( OnExpandEntry, SvTreeListEntry*, bool );

        DECL_LINK_TYPED( OnCopyEntry, LinkParamNone*, void );

        DECL_LINK_TYPED( OnTreeEntryCompare, const SvSortData&, sal_Int32 );

        DECL_LINK_TYPED( OnAsyncDrop, void*, void );

        void implRemoveStatusListeners();

        bool implSelect(const svx::ODataAccessDescriptor& _rDescriptor, bool _bSelectDirect = false);
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
        SvTreeListEntry* getObjectEntry(const svx::ODataAccessDescriptor& _rDescriptor,
            SvTreeListEntry** _ppDataSourceEntry = nullptr, SvTreeListEntry** _ppContainerEntry = nullptr,
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
            SvTreeListEntry** _ppDataSourceEntry = nullptr, SvTreeListEntry** _ppContainerEntry = nullptr,
            bool _bExpandAncestors = true,
            const SharedConnection& _rxConnection = SharedConnection()
        );

        /// checks if m_aDocumentDataSource describes a known object
        void checkDocumentDataSource();

        static void extractDescriptorProps(const svx::ODataAccessDescriptor& _rDescriptor,
            OUString& _rDataSource, OUString& _rCommand, sal_Int32& _rCommandType, bool& _rEscapeProcessing);

        void transferChangedControlProperty(const OUString& _rProperty, const css::uno::Any& _rNewValue);

        // checks whether the given tree entry denotes a data source
        bool impl_isDataSourceEntry( SvTreeListEntry* _pEntry ) const;

        /// retrieves the data source URL/name for the given entry representing a data source
        OUString  getDataSourceAcessor( SvTreeListEntry* _pDataSourceEntry ) const;

        /** get the signature (command/escape processing) of the query the form is based on
            <p>If the for is not based on a query or not even loaded, nothing happens and <FALSE/> is returned.</p>
        */
        bool implGetQuerySignature( OUString& _rCommand, bool& _bEscapeProcessing );

        bool isEntryCopyAllowed(SvTreeListEntry* _pEntry) const;

        void copyEntry(SvTreeListEntry* _pEntry);

        // remove all grid columns and dispose them
        static void clearGridColumns(const css::uno::Reference< css::container::XNameContainer >& _xColContainer);

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
