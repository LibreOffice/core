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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_APP_APPCONTROLLER_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_APP_APPCONTROLLER_HXX

#include "AppElementType.hxx"
#include "callbacks.hxx"
#include "commontypes.hxx"
#include "dsntypes.hxx"
#include <dbaccess/genericcontroller.hxx>
#include "linkeddocuments.hxx"
#include "moduledbu.hxx"
#include "TableCopyHelper.hxx"

#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/ui/XContextMenuInterception.hpp>

#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase5.hxx>
#include <sot/storage.hxx>
#include <svtools/transfer.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <vcl/timer.hxx>

#include <memory>

class SvTreeListEntry;
class SvTreeListBox;
class TransferableHelper;
class TransferableClipboardListener;

namespace com { namespace sun { namespace star {
    namespace container {
        class XNameContainer;
        class XContainer;
    }
    namespace ucb {
        class XContent;
    }
}}}

namespace dbaui
{
    class SubComponentManager;
    class OApplicationController;
    class OApplicationView;
    class OLinkedDocumentsAccess;
    class SelectionNotifier;

    typedef ::cppu::ImplHelper5 <   css::container::XContainerListener
                                ,   css::beans::XPropertyChangeListener
                                ,   css::sdb::application::XDatabaseDocumentUI
                                ,   css::ui::XContextMenuInterception
                                ,   css::view::XSelectionSupplier
                                >   OApplicationController_Base;


    class OApplicationController
            :public OGenericUnoController
            ,public OApplicationController_Base
            ,public IControlActionListener
            ,public IContextMenuProvider
    {
    public:
        typedef ::std::vector< css::uno::Reference< css::container::XContainer > >  TContainerVector;

    private:

        OTableCopyHelper::DropDescriptor            m_aAsyncDrop;

        SharedConnection        m_xDataSourceConnection;
        css::uno::Reference< css::sdbc::XDatabaseMetaData >
                                m_xMetaData;

        OModuleClient           m_aModuleClient;
        TransferableDataHelper  m_aSystemClipboard;     // content of the clipboard
        css::uno::Reference< css::beans::XPropertySet >
                                m_xDataSource;
        css::uno::Reference< css::frame::XModel >
                                m_xModel;
        ::cppu::OInterfaceContainerHelper
                                m_aContextMenuInterceptors;

        TContainerVector        m_aCurrentContainers;       // the containers where we are listener on
        ::rtl::Reference< SubComponentManager >
                                m_pSubComponentManager;
        ::dbaccess::ODsnTypeCollection
                                m_aTypeCollection;
        OTableCopyHelper        m_aTableCopyHelper;
        TransferableClipboardListener*
                                m_pClipbordNotifier;        // notifier for changes in the clipboard
        ImplSVEvent *           m_nAsyncDrop;
        OAsynchronousLink       m_aSelectContainerEvent;
        PreviewMode             m_ePreviewMode;             // the mode of the preview
        ElementType             m_eCurrentType;
        bool                    m_bNeedToReconnect;         // true when the settings of the data source were modified and the connection is no longer up to date
        bool                    m_bSuspended;               // is true when the controller was already suspended

        ::std::unique_ptr< SelectionNotifier >
                                m_pSelectionNotifier;
        typedef ::std::map< ElementType, ::std::vector< OUString > > SelectionByElementType;
        SelectionByElementType  m_aPendingSelection;

    private:

        OApplicationView*       getContainer() const;

        /** returns the database name
            @return
                the database name
        */
        OUString getDatabaseName() const;

        /** returns the stripped database name.
            @return
                The stripped database name either the registered name or if it is a file url the last segment.
        */
        OUString getStrippedDatabaseName() const;

        /** return the element type for given container
            @param  _xContainer The container where the element type has to be found
            @return the element type corresponding to the given container
        */
        static ElementType getElementType(const css::uno::Reference< css::container::XContainer >& _xContainer);

        /** opens a new frame with either the table or the query or report or form or view
            @param  _sName
                The name of the object to open
            @param  _eType
                Defines the type to open
            @param  _eOpenMode
                denotes the mode in which to open the object
            @param _nInstigatorCommand
                denotes the command which instigated the action. Might be 0.
            @return the form or report model will only be returned, otherwise <NULL/>
        */
        css::uno::Reference< css::lang::XComponent > openElement(
            const OUString& _sName,
            ElementType _eType,
            ElementOpenMode _eOpenMode,
            sal_uInt16 _nInstigatorCommand = 0
        );

        /** opens a new sub frame with a table/query/form/report/view, passing additional arguments
        */
        css::uno::Reference< css::lang::XComponent > openElementWithArguments(
            const OUString& _sName,
            ElementType _eType,
            ElementOpenMode _eOpenMode,
            sal_uInt16 _nInstigatorCommand,
            const ::comphelper::NamedValueCollection& _rAdditionalArguments
        );

        /** opens a new frame for creation or auto pilot
            @param  _eType
                Defines the type to open
            @param  i_rAdditionalArguments
                Additional arguments to pass when creating the component
        */
        css::uno::Reference< css::lang::XComponent >
            newElement(
                ElementType _eType,
                const ::comphelper::NamedValueCollection& i_rAdditionalArguments,
                css::uno::Reference< css::lang::XComponent >& o_rDocumentDefinition
            );

        /** creates a new database object, using an auto pilot
            @param _eType
                Defines the type of the object to create
            @precond
                Our mutex must not be locked.
            @since #i39203#
        */
        void newElementWithPilot( ElementType _eType );

        /** converts the query to a view
            @param  _sName
                The name of the query.
        */
        void convertToView(const OUString& _sName);

        /** checks if the connection for the selected data source is read only. If the connection doesn't exist, <TRUE/> will be returned.
            @return
                <TRUE/> if read only or doesn't exist, otherwise <FALSE/>
        */
        bool isConnectionReadOnly() const;

        /// fills the list with the selected entries.
        void getSelectionElementNames( ::std::vector< OUString>& _rNames ) const;

        /// deletes the entries selected.
        void deleteEntries();

        /// renames the selected entry in the detail page
        void renameEntry();

        /** deletes queries, forms, or reports
            @param  _eType
                the type of the objects
            @param  _rList
                The names of the elements to delete
            @param  _bConfirm
                determines whether the user must confirm the deletion
        */
        void deleteObjects( ElementType _eType,
                            const ::std::vector< OUString>& _rList,
                            bool _bConfirm );

        /** deletes tables.
            @param  _rList
                The list of tables.
        */
        void deleteTables(const ::std::vector< OUString>& _rList);

        /// copies the current object into clipboard
        TransferableHelper* copyObject();

        /// returns the nameaccess
        css::uno::Reference< css::container::XNameAccess > getElements(ElementType _eType);

        /** returns the document access for the specific type
            @param  _eType
                the type
            @return ::std::unique_ptr<OLinkedDocumentsAccess>
        */
        ::std::unique_ptr<OLinkedDocumentsAccess> getDocumentsAccess(ElementType _eType);

        /// returns the query definitions of the active data source.
        css::uno::Reference< css::container::XNameContainer> getQueryDefinitions() const;

        /** pastes a special format from the system clipboard to the currently selected object types
            @param  _nFormatId
                The format to be copied.
        */
        void pasteFormat(SotClipboardFormatId _nFormatId);

        /** pastes a query, form or report into the data source
            @param  _eType
                The type of the object to paste.
            @param  _rPasteData
                The data descriptor.
            @param  _sParentFolder
                The name of the parent folder if it exists.
            @param  _bMove
                if <TRUE/> the name of the content must be inserted without any change, otherwise not.
            @return
                <TRUE/> if the paste opertions was successful, otherwise <FALSE/>.
        */
        bool paste( ElementType _eType, const svx::ODataAccessDescriptor& _rPasteData, const OUString& _sParentFolder = OUString(), bool _bMove = false);

        /// returns the system clipboard.
        const TransferableDataHelper& getViewClipboard() const { return m_aSystemClipboard; }

        /// returns <TRUE/> if the clipboard supports a table format, otherwise <FALSE/>.
        bool isTableFormat() const;

        /** fills the vector with all supported formats
            @param  _eType
                The type for which we need the formats
            @param  _rFormatIds
                The vector to be filled up.
        */
        static void getSupportedFormats(ElementType _eType,::std::vector<SotClipboardFormatId>& _rFormatIds);

        /** adds a listener to the current name access.
            @param  _xCollection
                The collection where we want to listen on.
        */
        void addContainerListener(const css::uno::Reference< css::container::XNameAccess>& _xCollection);

        /** opens a uno dialog withthe currently selected data source as initialize argument
            @param  _sServiceName
                The serivce name of the dialog to be executed.
        */
        void openDialog(const OUString& _sServiceName);

        /** opens the administration dialog for the selected data source
        */
        void openDataSourceAdminDialog();

        /** opens the table filter dialog for the selected data source
        */
        void openTableFilterDialog();

        /** opens the DirectSQLDialog to execute hand made sql statements.
        */
        void openDirectSQLDialog();

        /** when the settings of the data source changed,
            it opens a dialog which ask to close all depending documents, then recreate the connection.
            The SolarMutex has to be locked before calling this.
        */
        void askToReconnect();

        /** remember a newly opened sub document for later access
        */
        void onDocumentOpened(
            const OUString&  _rName,
            const sal_Int32         _nType,
            const ElementOpenMode   _eMode,
            const css::uno::Reference< css::lang::XComponent >& _xDocument,
            const css::uno::Reference< css::lang::XComponent >& _xDefinition
        );

        /** Inserts a new object into the hierarchy given be the type.
            @param  _eType
                Where to insert the new item.
            @param  _sParentFolder
                The name of the parent folder if it exists.
            @param  _xContent
                The content to insert.
            @param  _bMove
                if <TRUE/> the name of the content must be inserted without any change, otherwise not.
            @return
                <TRUE/> if the insert opertions was successful, otherwise <FALSE/>.
        */
        bool insertHierachyElement(  ElementType _eType
                                    ,const OUString& _sParentFolder
                                    ,bool _bCollection = true
                                    ,const css::uno::Reference< css::ucb::XContent>& _xContent = css::uno::Reference< css::ucb::XContent>()
                                    ,bool _bMove = false);
        /** checks if delete command or rename comamnd is allowed
            @param  _eType
                The element type.
            @param  _bDelete
                If <TRUE> then the delete command should be checked.
            @return
                <TRUE> if the command is allowed
        */
        bool isRenameDeleteAllowed(ElementType _eType, bool _bDelete) const;
        /** all selected entries will be opened, or edited, or converted to a view
            @param  _nId
                The slot which should be executed.
            @param  _eOpenMode
                Defines the mode of opening. @see ElementOpenMode
        */
        void doAction(sal_uInt16 _nId, ElementOpenMode _eOpenMode);

        /** returns the currently selected table or query name.
        *
        * \return the name of the currently table or query. If the tables or query container is selected otherwise an empty string will be returned.
        */
        OUString getCurrentlySelectedName(sal_Int32& _rnCommandType) const;

        /** shows the preview for the given entry
        */
        void showPreviewFor( const ElementType _eType,const OUString& _sName );

        /** called we were attached to a frame

            In particular, this is called *after* the controller has been announced to the model
            (XModel::connectController)
        */
        void onAttachedFrame();

        /// determines whether the given table name denotes a view which can be altered
        bool    impl_isAlterableView_nothrow( const OUString& _rTableOrViewName ) const;

        /** does the macro/script migration, where macros/scripts in forms/reports are moved
            to the database document itself.
        */
        void    impl_migrateScripts_nothrow();

        /** verifies the object type denotes a valid DatabaseObject, and the object name denotes an existing
            object of this type. Throws if not.
        */
        void    impl_validateObjectTypeAndName_throw( const sal_Int32 _nObjectType, const ::boost::optional< OUString >& i_rObjectName );

    protected:
        // initalizing members

        // state of a feature. 'feature' may be the handle of a css::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const override;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) override;

        // OGenericUnoController
        virtual void            onLoadedMenu( const css::uno::Reference< css::frame::XLayoutManager >& _xLayoutManager ) override;

        virtual css::uno::Reference< css::frame::XModel > getPrivateModel() const override
        {
            return m_xModel;
        }

        virtual ~OApplicationController();

    public:
        explicit OApplicationController(const css::uno::Reference< css::uno::XComponentContext >& _rxORB);

        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;
        // need by registration
        static OUString getImplementationName_Static() throw( css::uno::RuntimeException );
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( css::uno::RuntimeException );
        static css::uno::Reference< css::uno::XInterface >
                SAL_CALL Create(const css::uno::Reference< css::lang::XMultiServiceFactory >&);

        // css::frame::XController
        virtual void SAL_CALL attachFrame(const css::uno::Reference< css::frame::XFrame > & xFrame) throw( css::uno::RuntimeException, std::exception ) override;
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( css::uno::RuntimeException, std::exception ) override;
        virtual sal_Bool SAL_CALL attachModel(const css::uno::Reference< css::frame::XModel > & xModel) throw( css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Reference< css::frame::XModel >  SAL_CALL getModel() throw( css::uno::RuntimeException, std::exception ) override;

        // css::container::XContainerListener
        virtual void SAL_CALL elementInserted(const css::container::ContainerEvent& Event) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL elementRemoved(const css::container::ContainerEvent& Event) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL elementReplaced(const css::container::ContainerEvent& Event) throw( css::uno::RuntimeException, std::exception ) override;

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& evt ) throw (css::uno::RuntimeException, std::exception) override;

        // XDatabaseDocumentUI
        virtual css::uno::Reference< css::sdbc::XDataSource > SAL_CALL getDataSource() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL getApplicationMainWindow() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getActiveConnection() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::uno::Reference< css::lang::XComponent > > SAL_CALL getSubComponents() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isConnected(  ) throw (css::uno::RuntimeException, std::exception) override;
        // DO NOT CALL with getMutex() held!!
        virtual void SAL_CALL connect(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::beans::Pair< ::sal_Int32, OUString > SAL_CALL identifySubComponent( const css::uno::Reference< css::lang::XComponent >& SubComponent ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL closeSubComponents(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::lang::XComponent > SAL_CALL loadComponent( ::sal_Int32 ObjectType, const OUString& ObjectName, sal_Bool ForEditing ) throw (css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::lang::XComponent > SAL_CALL loadComponentWithArguments( ::sal_Int32 ObjectType, const OUString& ObjectName, sal_Bool ForEditing, const css::uno::Sequence< css::beans::PropertyValue >& Arguments ) throw (css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::lang::XComponent > SAL_CALL createComponent( ::sal_Int32 ObjectType, css::uno::Reference< css::lang::XComponent >& o_DocumentDefinition ) throw (css::lang::IllegalArgumentException, css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::lang::XComponent > SAL_CALL createComponentWithArguments( ::sal_Int32 ObjectType, const css::uno::Sequence< css::beans::PropertyValue >& Arguments, css::uno::Reference< css::lang::XComponent >& o_DocumentDefinition ) throw (css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

        // XContextMenuInterception
        virtual void SAL_CALL registerContextMenuInterceptor( const css::uno::Reference< css::ui::XContextMenuInterceptor >& Interceptor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL releaseContextMenuInterceptor( const css::uno::Reference< css::ui::XContextMenuInterceptor >& Interceptor ) throw (css::uno::RuntimeException, std::exception) override;

        // XSelectionSupplier
        virtual sal_Bool SAL_CALL select( const css::uno::Any& xSelection ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getSelection(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

        /** retrieves the current connection, creates it if necessary

            If an error occurs, then this is either stored in the location pointed to by <arg>_pErrorInfo</arg>,
            or, if <code>_pErrorInfo</code> is <NULL/>, then the error is displayed to the user.

            DO NOT CALL with getMutex() held!!
        */
        const SharedConnection& ensureConnection( ::dbtools::SQLExceptionInfo* _pErrorInfo = nullptr );

        /** retrieves the current connection
        */
        const SharedConnection& getConnection() const { return m_xDataSourceConnection; }

        /// determines whether we're currently connected to the database
        bool isConnected() const { return m_xDataSourceConnection.is(); }

        /** refreshes the tables
        */
        void refreshTables();

        /** called when an entry in a tree list box has been double-clicked
            @param  _rTree
                The tree list box.
            @return
                <TRUE/> if the double click event has been handled by the called, and should not
                be processed further.
        */
        bool onEntryDoubleClick(SvTreeListBox& _rTree);
        /** called when a container (category) in the application view has been selected
            @param  _pTree
                The tree list box.
            @return
                <TRUE/> if the cotainer could be changed otherwise <FALSE/>
        */
        bool onContainerSelect(ElementType _eType);
        /** called when an entry in a tree view has been selected
            @param  _pEntry
                the selected entry
        */
        void onSelectionChanged();
        /** called when a "Copy" command is executed in a tree view
        */
        void onCopyEntry();
        /** called when a "Paste" command is executed in a tree view
        */
        void onPasteEntry();
        /** called when a "Delete" command is executed in a tree view
        */
        void onDeleteEntry();
        /// called when the preview mode was changed
        void previewChanged( sal_Int32 _nMode);
        /// called when an object container of any kind was found during enumerating tree view elements
        void containerFound( const css::uno::Reference< css::container::XContainer >& _xContainer);

        // IController
        virtual void        executeUnChecked(const css::util::URL& _rCommand, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) override;
        virtual void        executeChecked(const css::util::URL& _rCommand, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) override;
        virtual void        executeUnChecked(sal_uInt16 _nCommandId, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) override;
        virtual void        executeChecked(sal_uInt16 _nCommandId, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) override;
        virtual bool        isCommandEnabled(sal_uInt16 _nCommandId) const override;
        virtual bool        isCommandEnabled( const OUString& _rCompleteCommandURL ) const override;
        virtual sal_uInt16  registerCommandURL( const OUString& _rCompleteCommandURL ) override;
        virtual void        notifyHiContrastChanged() override;
        virtual bool        isDataSourceReadOnly() const override;
        virtual css::uno::Reference< css::frame::XController >
                            getXController() throw( css::uno::RuntimeException ) override;
        virtual bool        interceptUserInput( const NotifyEvent& _rEvent ) override;

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

        void OnInvalidateClipboard();
        DECL_LINK_TYPED( OnClipboardChanged, TransferableDataHelper*, void );
        DECL_LINK_TYPED( OnAsyncDrop, void*, void );
        DECL_LINK_TYPED( OnCreateWithPilot, void*, void );
        DECL_LINK_TYPED( OnSelectContainer, void*, void );
        void OnFirstControllerConnected();

    protected:
        using OGenericUnoController::connect;

        /** disconnects from our XConnection, and cleans up this connection
        */
        void        disconnect();

        // late construction
        virtual bool        Construct(vcl::Window* pParent) override;
        virtual void        describeSupportedFeatures() override;

    protected:
        // XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw( css::uno::RuntimeException, std::exception ) override;

        // OComponentHelper
        virtual void SAL_CALL disposing() override;
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_APP_APPCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
