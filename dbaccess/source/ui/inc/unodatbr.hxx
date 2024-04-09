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

#pragma once

#include <memory>
#include "brwctrlr.hxx"
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/i18n/XCollator.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <com/sun/star/ui/XContextMenuInterception.hpp>
#include <com/sun/star/sdb/application/DatabaseObject.hpp>
#include <com/sun/star/sdb/application/DatabaseObjectContainer.hpp>
#include <com/sun/star/sdb/XDatabaseRegistrationsListener.hpp>
#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/implbase5.hxx>
#include "callbacks.hxx"
#include <utility>
#include <vcl/transfer.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include "TableCopyHelper.hxx"
#include "commontypes.hxx"

class Splitter;
class ODataClipboard;

namespace com::sun::star::container { class XNameContainer; }

namespace dbaui
{
    struct DBTreeEditedEntry;
    struct DBTreeListUserData;
    class ImageProvider;

    typedef ::cppu::ImplHelper5 <   css::frame::XStatusListener
                                ,   css::view::XSelectionSupplier
                                ,   css::document::XScriptInvocationContext
                                ,   css::ui::XContextMenuInterception
                                ,   css::sdb::XDatabaseRegistrationsListener
                                >   SbaTableQueryBrowser_Base;
    class SbaTableQueryBrowser final
                :public SbaXDataBrowserController
                ,public SbaTableQueryBrowser_Base
                ,public IControlActionListener
                ,public IContextMenuProvider
    {
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
            ExternalFeature( css::util::URL _aURL ) : aURL(std::move( _aURL )), bEnabled( false ) { }
        };

        typedef std::map< sal_uInt16, ExternalFeature >  ExternalFeaturesMap;
        ExternalFeaturesMap     m_aExternalFeatures;

        svx::ODataAccessDescriptor    m_aDocumentDataSource;
            // if we're part of a document, this is the state of the DocumentDataSource slot

        ::comphelper::OInterfaceContainerHelper2   m_aSelectionListeners;
        ::comphelper::OInterfaceContainerHelper2   m_aContextMenuInterceptors;

        OTableCopyHelper::DropDescriptor    m_aAsyncDrop;
        OTableCopyHelper                    m_aTableCopyHelper;

        OUString         m_sQueryCommand;    // the command of the query currently loaded (if any)
        //OUString         m_sToBeLoaded;      // contains the element name which should be loaded if any

        VclPtr<InterimDBTreeListBox> m_pTreeView; // contains the datasources of the registry
        VclPtr<Splitter>        m_pSplitter;
        std::unique_ptr<weld::TreeIter> m_xCurrentlyDisplayed;
        ImplSVEvent *           m_nAsyncDrop;

        bool                m_bQueryEscapeProcessing : 1;   // the escape processing flag of the query currently loaded (if any)
        bool                m_bShowMenu;            // if sal_True the menu should be visible otherwise not
        bool                m_bInSuspend;
        bool                m_bEnableBrowser;
        ::std::optional< bool >
                                m_aDocScriptSupport;    // relevant if and only if we are associated with exactly one DBDoc

        virtual OUString getPrivateTitle( ) const override;
    // attribute access
    public:
        SbaTableQueryBrowser(const css::uno::Reference< css::uno::XComponentContext >& _rM);
        virtual ~SbaTableQueryBrowser() override;

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

        DECLARE_UNO3_DEFAULTS(SbaTableQueryBrowser,SbaXDataBrowserController)
        // late construction
        virtual bool Construct(vcl::Window* pParent) override;
        // XInterface
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

        // XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

        // css::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& evt) override;

        // css::frame::XController
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) override;
        virtual void SAL_CALL attachFrame(const css::uno::Reference< css::frame::XFrame > & xFrame) override;

        // css::lang::XComponent
        virtual void        SAL_CALL disposing() override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // XSelectionSupplier
        virtual sal_Bool SAL_CALL select( const css::uno::Any& aSelection ) override;
        virtual css::uno::Any SAL_CALL getSelection(  ) override;
        virtual void SAL_CALL addSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;
        virtual void SAL_CALL removeSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

        // XContainerListener
        virtual void SAL_CALL elementInserted( const css::container::ContainerEvent& Event ) override;
        virtual void SAL_CALL elementRemoved( const css::container::ContainerEvent& Event ) override;
        virtual void SAL_CALL elementReplaced( const css::container::ContainerEvent& Event ) override;
        // css::frame::XFrameActionListener
        virtual void SAL_CALL frameAction(const css::frame::FrameActionEvent& aEvent) override;

        // XScriptInvocationContext
        virtual css::uno::Reference< css::document::XEmbeddedScripts > SAL_CALL getScriptContainer() override;

        // XContextMenuInterception
        virtual void SAL_CALL registerContextMenuInterceptor( const css::uno::Reference< css::ui::XContextMenuInterceptor >& Interceptor ) override;
        virtual void SAL_CALL releaseContextMenuInterceptor( const css::uno::Reference< css::ui::XContextMenuInterceptor >& Interceptor ) override;

        // XDatabaseRegistrationsListener
        virtual void SAL_CALL registeredDatabaseLocation( const css::sdb::DatabaseRegistrationEvent& Event ) override;
        virtual void SAL_CALL revokedDatabaseLocation( const css::sdb::DatabaseRegistrationEvent& Event ) override;
        virtual void SAL_CALL changedDatabaseLocation( const css::sdb::DatabaseRegistrationEvent& Event ) override;

    private:
        // SbaXDataBrowserController overridable
        virtual bool     InitializeForm( const css::uno::Reference< css::beans::XPropertySet >& i_formProperties ) override;

        void             InitializeGridModel(const css::uno::Reference< css::form::XFormComponent > & xGrid);

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
        virtual bool        requestQuickHelp(const void* pUserData, OUString& rText) const override;
        virtual bool        requestDrag(const weld::TreeIter& rEntry) override;
        virtual sal_Int8    queryDrop( const AcceptDropEvent& _rEvt, const DataFlavorExVector& _rFlavors ) override;
        virtual sal_Int8    executeDrop( const ExecuteDropEvent& _rEvt ) override;

        // IContextMenuProvider
        virtual OUString          getContextMenuResourceName() const override;
        virtual IController&      getCommandController() override;
        virtual ::comphelper::OInterfaceContainerHelper2*
                                  getContextMenuInterceptors() override;
        virtual css::uno::Any     getCurrentSelection(weld::TreeView& rControl) const override;
        virtual vcl::Window* getMenuParent() const override;
        virtual void adjustMenuPosition(const weld::TreeView& rControl, ::Point& rPos) const override;

        virtual void impl_initialize(const ::comphelper::NamedValueCollection& rArguments) override;

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
        bool isSelected(const weld::TreeIter& rEntry) const;
        // select the entry (and only the entry, not the whole path)
        void        select(const weld::TreeIter* pEntry, bool bSelect);
        // select the path of the entry (which must be an entry without children)
        void        selectPath(const weld::TreeIter* pEntry, bool bSelect = true);

        virtual void loadMenu(const css::uno::Reference< css::frame::XFrame >& _xFrame) override;

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
        void implAddDatasource(const OUString& _rDbName, OUString& _rDbImage,
                OUString& _rQueryName, OUString& _rQueryImage,
                OUString& _rTableName, OUString& _rTableImage,
                const SharedConnection& _rxConnection
            );

        void    implAddDatasource( const OUString& _rDataSourceName, const SharedConnection& _rxConnection );

        /// removes (and cleans up) the entry for the given data source
        void        impl_cleanupDataSourceEntry( std::u16string_view _rDataSourceName );

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
        void        disposeConnection(const weld::TreeIter* xpDSEntry);

        /// flushes and disposes the given connection, and de-registers as listener
        void        impl_releaseConnection( SharedConnection& _rxConnection );

        /** close the connection (and collapse the list entries) of the given list entries
        */
        void        closeConnection(const weld::TreeIter& rEntry, bool bDisposeConnection = true);

        void        populateTree(const css::uno::Reference< css::container::XNameAccess>& xNameAccess, const weld::TreeIter& rParent, EntryType eEntryType);
        void        initializeTreeModel();

        /** search in the tree for query- or tablecontainer equal to this interface and return
            this container entry
        */
        std::unique_ptr<weld::TreeIter> getEntryFromContainer(const css::uno::Reference<css::container::XNameAccess>& rxNameAccess);

        // return true when there is connection available
        bool ensureConnection(const weld::TreeIter* pDSEntry, void * pDSData, SharedConnection& rConnection);
        bool ensureConnection(const weld::TreeIter* pAnyEntry, SharedConnection& rConnection);

        bool getExistentConnectionFor(const weld::TreeIter* pDSEntry, SharedConnection& rConnection);
        /** returns an image provider which works with the connection belonging to the given entry
        */
        std::unique_ptr<ImageProvider> getImageProviderFor(const weld::TreeIter* pAnyEntry);

        void    implAdministrate(const weld::TreeIter& rApplyTo);

        bool implCopyObject(ODataClipboard& rExchange, const weld::TreeIter& rApplyTo, sal_Int32 nCommandType);

        EntryType getEntryType(const weld::TreeIter& rEntry) const;
        EntryType getChildType(const weld::TreeIter& rEntry) const;
        static bool    isObject( EntryType _eType ) { return ( etTableOrView== _eType ) || ( etQuery == _eType ); }
        static bool    isContainer( EntryType _eType ) { return (etTableContainer == _eType) || (etQueryContainer == _eType); }
        bool isContainer(const weld::TreeIter& rEntry) const { return isContainer(getEntryType(rEntry)); }

        // ensure that the xObject for the given entry is set on the user data
        bool          ensureEntryObject(const weld::TreeIter& rEntry);

        // get the display text of the entry given
        OUString      GetEntryText(const weld::TreeIter& rEntry) const;

        // is called when a table or a query was selected
        DECL_LINK( OnSelectionChange, LinkParamNone*, void );
        DECL_LINK( OnExpandEntry, const weld::TreeIter&, bool );

        DECL_LINK( OnCopyEntry, LinkParamNone*, void );

        int OnTreeEntryCompare(const weld::TreeIter& rLHS, const weld::TreeIter& rRHS);

        DECL_LINK( OnAsyncDrop, void*, void );

        void implRemoveStatusListeners();

        bool implSelect(const svx::ODataAccessDescriptor& _rDescriptor, bool _bSelectDirect = false);
        bool implSelect(const weld::TreeIter* pEntry);

        /// selects the entry given and loads the grid control with the object's data
        bool implSelect(
            const OUString& _rDataSourceName,
            const OUString& _rCommand,
            const sal_Int32 _nCommandType,
            const bool _bEscapeProcessing,
            const SharedConnection& _rxConnection,
            bool _bSelectDirect
        );

        std::unique_ptr<weld::TreeIter> implGetConnectionEntry(const weld::TreeIter& rEntry) const;
        /// inserts an entry into the tree
        std::unique_ptr<weld::TreeIter> implAppendEntry(
            const weld::TreeIter* pParent,
            const OUString& rName,
            const DBTreeListUserData* pUserData);

        /// loads the grid control with the data object specified (which may be a table, a query or a command)
        bool implLoadAnything(const OUString& _rDataSourceName, const OUString& _rCommand,
            const sal_Int32 _nCommandType, const bool _bEscapeProcessing, const SharedConnection& _rxConnection );

        /** retrieves the tree entry for the object described by <arg>_rDescriptor</arg>
            @param rDescriptor
                the object descriptor
            @param ppDataSourceEntry
                If not <NULL/>, the data source tree entry will be returned here
            @param ppContainerEntry
                If not <NULL/>, the object container tree entry will be returned here
        */
        std::unique_ptr<weld::TreeIter> getObjectEntry(const svx::ODataAccessDescriptor& rDescriptor,
            std::unique_ptr<weld::TreeIter>* ppDataSourceEntry, std::unique_ptr<weld::TreeIter>* ppContainerEntry
        );
        /** retrieves the tree entry for the object described by data source name, command and command type
            @param rDataSource
                the data source name
            @param rCommand
                the command
            @param nCommandType
                the command type
            @param rDescriptor
                the object descriptor
            @param ppDataSourceEntry
                If not <NULL/>, the data source tree entry will be returned here
            @param ppContainerEntry
                If not <NULL/>, the object container tree entry will be returned here
            @param bExpandAncestors
                If <TRUE/>, all ancestor on the way to the entry will be expanded
        */
        std::unique_ptr<weld::TreeIter> getObjectEntry(
            const OUString& rDataSource, const OUString& rCommand, sal_Int32 nCommandType,
            std::unique_ptr<weld::TreeIter>* ppDataSourceEntry, std::unique_ptr<weld::TreeIter>* ppContainerEntry,
            bool _bExpandAncestors = true,
            const SharedConnection& rxConnection = SharedConnection()
        );

        /// checks if m_aDocumentDataSource describes a known object
        void checkDocumentDataSource();

        static void extractDescriptorProps(const svx::ODataAccessDescriptor& _rDescriptor,
            OUString& _rDataSource, OUString& _rCommand, sal_Int32& _rCommandType, bool& _rEscapeProcessing);

        void transferChangedControlProperty(const OUString& _rProperty, const css::uno::Any& _rNewValue);

        // checks whether the given tree entry denotes a data source
        bool impl_isDataSourceEntry(const weld::TreeIter* pEntry) const;

        /// retrieves the data source URL/name for the given entry representing a data source
        OUString  getDataSourceAccessor(const weld::TreeIter& rDataSourceEntry) const;

        /** get the signature (command/escape processing) of the query the form is based on
            <p>If the for is not based on a query or not even loaded, nothing happens and <FALSE/> is returned.</p>
        */
        bool implGetQuerySignature( OUString& _rCommand, bool& _bEscapeProcessing );

        bool isEntryCopyAllowed(const weld::TreeIter& rEntry) const;

        void copyEntry(const weld::TreeIter& rEntry);

        // remove all grid columns and dispose them
        static void clearGridColumns(const css::uno::Reference< css::container::XNameContainer >& _xColContainer);

        /** checks if the currently displayed entry changed
            @param  rName
                    Name of the changed entry
            @param  rContainer
                    The container of the displayed entry
            @return
                    <TRUE/> if it is the currently displayed otherwise <FALSE/>
        */
        bool isCurrentlyDisplayedChanged(std::u16string_view rName, const weld::TreeIter& rContainer);

        /** called whenever the content of the browser is used for preview, as the very last action
            of the load process
        */
        void initializePreviewMode();

        /** checks whether the Order/Filter clauses set at our row set are valid, removes them if not so
        */
        void    impl_sanitizeRowSetClauses_nothrow();
    };

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
