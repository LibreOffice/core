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

#ifndef DBAUI_APPCONTROLLER_HXX
#define DBAUI_APPCONTROLLER_HXX

#include "IApplicationController.hxx"
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

#include <comphelper/stl_types.hxx>
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

    // OApplicationController
    class OApplicationController;
    class OApplicationView;
    class OLinkedDocumentsAccess;
    typedef OGenericUnoController   OApplicationController_CBASE;
    typedef ::cppu::ImplHelper5 <   ::com::sun::star::container::XContainerListener
                                ,   ::com::sun::star::beans::XPropertyChangeListener
                                ,   ::com::sun::star::sdb::application::XDatabaseDocumentUI
                                ,   ::com::sun::star::ui::XContextMenuInterception
                                ,   ::com::sun::star::view::XSelectionSupplier
                                >   OApplicationController_Base;

    class SelectionNotifier;

    class OApplicationController
            :public OApplicationController_CBASE
            ,public OApplicationController_Base
            ,public IApplicationController
    {
    public:
        typedef ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer > TContainer;
        typedef ::std::vector< TContainer >                                                 TContainerVector;

    private:

        OTableCopyHelper::DropDescriptor            m_aAsyncDrop;

        SharedConnection        m_xDataSourceConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >
                                m_xMetaData;

        OModuleClient           m_aModuleClient;
        TransferableDataHelper  m_aSystemClipboard;     // content of the clipboard
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                m_xDataSource;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
                                m_xModel;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifiable >
                                m_xDocumentModify;
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
        sal_uLong               m_nAsyncDrop;
        OAsyncronousLink        m_aSelectContainerEvent;
        PreviewMode             m_ePreviewMode;             // the mode of the preview
        ElementType             m_eCurrentType;
        sal_Bool                m_bNeedToReconnect;         // true when the settings of the data source were modified and the connection is no longer up to date
        sal_Bool                m_bSuspended;               // is true when the controller was already suspended

        ::std::auto_ptr< SelectionNotifier >
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
                The stripped database name either the registered naem or if it is a file url the last segment.
        */
        OUString getStrippedDatabaseName() const;

        /** return the element type for given container
            @param  _xContainer The container where the element type has to be found
            @return the element type coressponding to the given container
        */
        ElementType getElementType(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >& _xContainer) const;

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
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > openElement(
            const OUString& _sName,
            ElementType _eType,
            ElementOpenMode _eOpenMode,
            sal_uInt16 _nInstigatorCommand = 0
        );

        /** opens a new sub frame with a table/query/form/report/view, passing additional arguments
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > openElementWithArguments(
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
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
            newElement(
                ElementType _eType,
                const ::comphelper::NamedValueCollection& i_rAdditionalArguments,
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& o_rDocumentDefinition
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
        sal_Bool isConnectionReadOnly() const;

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
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > getElements(ElementType _eType);

        /** returns the document access for the specific type
            @param  _eType
                the type
            @return ::std::auto_ptr<OLinkedDocumentsAccess>
        */
        ::std::auto_ptr<OLinkedDocumentsAccess> getDocumentsAccess(ElementType _eType);

        /// returns the query definitions of the active data source.
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer> getQueryDefintions() const;

        /** pastes a special format from the system clipboard to the currently selected object types
            @param  _nFormatId
                The format to be copied.
        */
        void pasteFormat(sal_uInt32 _nFormatId);

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
        sal_Bool paste( ElementType _eType,const ::svx::ODataAccessDescriptor& _rPasteData ,const OUString& _sParentFolder = String(),sal_Bool _bMove = sal_False);

        /// returns the system clipboard.
        const TransferableDataHelper& getViewClipboard() const { return m_aSystemClipboard; }

        /// returns <TRUE/> if the clipboard supports a table format, otherwise <FALSE/>.
        sal_Bool isTableFormat() const;

        /** fills the vector with all supported formats
            @param  _eType
                The type for which we need the formats
            @param  _rFormatIds
                The vector to be filled up.
        */
        void getSupportedFormats(ElementType _eType,::std::vector<SotFormatStringId>& _rFormatIds) const;

        /** adds a listener to the current name access.
            @param  _xCollection
                The collection where we want to listen on.
        */
        void addContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xCollection);

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
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _xDocument,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _xDefinition
        );

        /** Inserts a new object into the hierachy given be the type.
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
        sal_Bool insertHierachyElement(  ElementType _eType
                                    ,const OUString& _sParentFolder
                                    ,sal_Bool _bCollection = sal_True
                                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent>& _xContent = ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent>()
                                    ,sal_Bool _bMove = sal_False);
        /** checks if delete command or rename comamnd is allowed
            @param  _eType
                The element type.
            @param  _bDelete
                If <TRUE> then the delete command should be checked.
            @return
                <TRUE> if the command is allowed
        */
        sal_Bool isRenameDeleteAllowed(ElementType _eType,sal_Bool _bDelete) const;
        /** all selected entries will be opened, or edited, or converted to a view
            @param  _nId
                The slot which should be executed.
            @param  _eOpenMode
                Defines the mode of opening. @see ElementOpenMode
        */
        void doAction(sal_uInt16 _nId ,ElementOpenMode _eOpenMode);

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
        /** forces usage of a connection which we do not own
            <p>To be used from within XInitialization::initialize only.</p>
        */
        void                    initializeConnection( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxForeignConn );

        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);

        // OGenericUnoController
        virtual void            onLoadedMenu( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >& _xLayoutManager );

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > getPrivateModel() const
        {
            return m_xModel;
        }

        virtual ~OApplicationController();

    public:
        OApplicationController(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB);

        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
        // need by registration
        static OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

        // ::com::sun::star::frame::XController
        virtual void SAL_CALL attachFrame(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & xFrame) throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL attachModel(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xModel) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >  SAL_CALL getModel(void) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::container::XContainerListener
        virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException);

        // XDatabaseDocumentUI
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource > SAL_CALL getDataSource() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL getApplicationMainWindow() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getActiveConnection() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > > SAL_CALL getSubComponents() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isConnected(  ) throw (::com::sun::star::uno::RuntimeException);
        // DO NOT CALL with getMutex() held!!
        virtual void SAL_CALL connect(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::beans::Pair< ::sal_Int32, OUString > SAL_CALL identifySubComponent( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& SubComponent ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL closeSubComponents(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > SAL_CALL loadComponent( ::sal_Int32 ObjectType, const OUString& ObjectName, ::sal_Bool ForEditing ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > SAL_CALL loadComponentWithArguments( ::sal_Int32 ObjectType, const OUString& ObjectName, ::sal_Bool ForEditing, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > SAL_CALL createComponent( ::sal_Int32 ObjectType, ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& o_DocumentDefinition ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > SAL_CALL createComponentWithArguments( ::sal_Int32 ObjectType, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments, ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& o_DocumentDefinition ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        // XContextMenuInterception
        virtual void SAL_CALL registerContextMenuInterceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XContextMenuInterceptor >& Interceptor ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL releaseContextMenuInterceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XContextMenuInterceptor >& Interceptor ) throw (::com::sun::star::uno::RuntimeException);

        // XSelectionSupplier
        virtual ::sal_Bool SAL_CALL select( const ::com::sun::star::uno::Any& xSelection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getSelection(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

        /** retrieves the current connection, creates it if necessary

            If an error occurs, then this is either stored in the location pointed to by <arg>_pErrorInfo</arg>,
            or, if <code>_pErrorInfo</code> is <NULL/>, then the error is displayed to the user.

            DO NOT CALL with getMutex() held!!
        */
        const SharedConnection& ensureConnection( ::dbtools::SQLExceptionInfo* _pErrorInfo = NULL );

        /** retrieves the current connection
        */
        const SharedConnection& getConnection() const { return m_xDataSourceConnection; }

        /// determines whether we're currently connected to the database
        bool isConnected() const { return m_xDataSourceConnection.is(); }

        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >&
            getConnectionMetaData() const { return m_xMetaData; }

        /** refreshes the tables
        */
        void refreshTables();

        // IApplicationController
        virtual bool onEntryDoubleClick(SvTreeListBox& _rTree);
        virtual sal_Bool onContainerSelect(ElementType _eType);
        virtual void onSelectionChanged();
        virtual void onCutEntry();
        virtual void onCopyEntry();
        virtual void onPasteEntry();
        virtual void onDeleteEntry();
        virtual void previewChanged( sal_Int32 _nMode);
        virtual void containerFound( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >& _xContainer);

        // IController (base of IApplicationController)
        virtual void        executeUnChecked(const ::com::sun::star::util::URL& _rCommand, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);
        virtual void        executeChecked(const ::com::sun::star::util::URL& _rCommand, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);
        virtual void        executeUnChecked(sal_uInt16 _nCommandId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);
        virtual void        executeChecked(sal_uInt16 _nCommandId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);
        virtual sal_Bool    isCommandEnabled(sal_uInt16 _nCommandId) const;
        virtual sal_Bool    isCommandEnabled( const OUString& _rCompleteCommandURL ) const;
        virtual sal_uInt16  registerCommandURL( const OUString& _rCompleteCommandURL );
        virtual void        notifyHiContrastChanged();
        virtual sal_Bool    isDataSourceReadOnly() const;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >
                            getXController(void) throw( ::com::sun::star::uno::RuntimeException );
        virtual bool        interceptUserInput( const NotifyEvent& _rEvent );

        // IControlActionListener overridables
        virtual sal_Bool        requestQuickHelp( const SvTreeListEntry* _pEntry, OUString& _rText ) const;
        virtual sal_Bool        requestDrag( sal_Int8 _nAction, const Point& _rPosPixel );
        virtual sal_Int8        queryDrop( const AcceptDropEvent& _rEvt, const DataFlavorExVector& _rFlavors );
        virtual sal_Int8        executeDrop( const ExecuteDropEvent& _rEvt );

        // IContextMenuProvider (base of IApplicationController)
        virtual PopupMenu*      getContextMenu( Control& _rControl ) const;
        virtual IController&    getCommandController();
        virtual ::cppu::OInterfaceContainerHelper*
                                getContextMenuInterceptors();
        virtual ::com::sun::star::uno::Any
                                getCurrentSelection( Control& _rControl ) const;

        void OnInvalidateClipboard();
        DECL_LINK( OnClipboardChanged, void* );
        DECL_LINK( OnAsyncDrop, void* );
        DECL_LINK( OnCreateWithPilot, void* );
        DECL_LINK( OnSelectContainer, void* );
        void OnFirstControllerConnected();

    protected:
        using OApplicationController_CBASE::connect;

        /** disconnects from our XConnection, and cleans up this connection
        */
        virtual void        disconnect();

        // late construction
        virtual sal_Bool    Construct(Window* pParent);
        virtual void        describeSupportedFeatures();

    protected:
        // XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

        // OComponentHelper
        virtual void SAL_CALL disposing();
    };

}   // namespace dbaui

#endif // DBAUI_APPCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
