/*************************************************************************
 *
 *  $RCSfile: AppController.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:28:20 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/

#ifndef DBAUI_APPCONTROLLER_HXX
#define DBAUI_APPCONTROLLER_HXX

#ifndef DBAUI_GENERICCONTROLLER_HXX
#include "genericcontroller.hxx"
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif
#ifndef DBAUI_APPELEMENTTYPE_HXX
#include "AppElementType.hxx"
#endif
#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif
#ifndef DBAUI_IAPPELEMENTNOTIFICATION_HXX
#include "IAppElementNotification.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _DBACCESS_UI_CALLBACKS_HXX_
#include "callbacks.hxx"
#endif
#include <memory>

class ListBox;
class SvLBoxEntry;
class SvTreeListBox;
class TransferableHelper;
class TransferableClipboardListener;
class SfxFilter;

FORWARD_DECLARE_INTERFACE(container,XNameContainer)
FORWARD_DECLARE_INTERFACE(container,XContainer)
FORWARD_DECLARE_INTERFACE(util,XNumberFormatter)
FORWARD_DECLARE_INTERFACE(util,XCloseable)
FORWARD_DECLARE_INTERFACE(ucb,XContent)
//........................................................................
namespace dbaui
{
//........................................................................
    //====================================================================
    //= OApplicationController
    //====================================================================
    class OApplicationController;
    class OApplicationView;
    class OLinkedDocumentsAccess;
    typedef OGenericUnoController                   OApplicationController_CBASE;
    typedef ::cppu::ImplHelper2 <   ::com::sun::star::container::XContainerListener,
                                    ::com::sun::star::beans::XPropertyChangeListener
                                >   OApplicationController_Base;

    class OApplicationController
            :public OApplicationController_CBASE
            ,public OApplicationController_Base
            ,public IApplicationElementNotification
            ,public IControlActionListener
            ,public IContainerFoundListener
            ,public IViewChangeListener
    {
        friend class OConnectionChangeBroadcaster;
    public:
        typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer > > TContainerVector;
        typedef ::std::map< ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
                , ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > > TDocuments;
    protected:
        // is needed to describe the drop target
        struct DropDescriptor
        {
            ::svx::ODataAccessDescriptor    aDroppedData;
            String                          aUrl;
            SotStorageStreamRef             aHtmlRtfStorage;
            ElementType                     nType;
            sal_Int8                        nAction;
            sal_Bool                        bHtml;
            sal_Bool                        bError;

            DropDescriptor() : nType(E_TABLE),nAction(DND_ACTION_NONE) { }
        };
    private:

        DECLARE_STL_USTRINGACCESS_MAP(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >,TDataSourceConnections);

        DropDescriptor          m_aAsyncDrop;
        TDataSourceConnections  m_aDataSourceConnections;
        TransferableDataHelper  m_aSystemClipboard;     // content of the clipboard
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > m_xDataSource;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >
                                m_xCurrentContainer;        // the container we listen on at the moment
        TContainerVector        m_aCurrentSubContainers;    // the sub container of our current container
        TDocuments              m_aDocuments;
        TransferableClipboardListener*
                                m_pClipbordNotifier;        // notifier for changes in the clipboard
        mutable ::rtl::OUString m_sDatabaseName;
        Timer                   m_aRefreshMenu;             // the timer for the menu "refresh"
        Timer                   m_aPasteSpecialMenu;        // the timer for the menu "paste"
        sal_Int32               m_nAsyncDrop;
        PreviewMode             m_ePreviewMode;             // the mode of the preview
        ::std::map<sal_uInt16,sal_uInt16>
                                m_aLastSelectedPullDownActions;
        sal_Bool                m_bPreviewEnabled;          // true when the preview should enabled
        sal_Bool                m_bNeedToReconnect;         // true when the settings of the data source were modified and the connection is no longer up to date
        sal_Bool                m_bSuspended        : 1;    // is true when the controller was already suspended


        OApplicationView*       getContainer() const;

        /** returns the database name
            @return
                the database name
        */
        inline ::rtl::OUString getDatabaseName() const { return m_sDatabaseName; }

        /** returns the stripped database name.
            @return
                The stripped database name either the registered naem or if it is a file url the last segment.
        */
        ::rtl::OUString getStrippedDatabaseName() const;

        /** opens a new frame with either the table or the query or report or form or view
            @param  _sName
                The name of the object to open
            @param  _eType
                Defines the type to open
            @param  _bOpenDesignMode
                If <TRUE/> the design mode opens for the element
        */
        void openElement(const ::rtl::OUString& _sName,ElementType _eType = E_TABLE,sal_Bool _bOpenDesignMode = sal_False);

        /** opens a new frame for creation or auto pilot
            @param  _eType
                Defines the type to open
            @param  _bAutoPilot
                If <TRUE/> the auto pilot opens, otherwise not.
            @param  _bSQLView
                If <TRUE/> the query design will be opened in SQL view, otherwise not.
        */
        void newElement(ElementType _eType = E_TABLE,sal_Bool _bAutoPilot = sal_False,sal_Bool _bSQLView = sal_False);


        /** converts the query to a view
            @param  _sName
                The name of the query.
        */
        void convertToView(const ::rtl::OUString& _sName);

        /** checks if the selected data source is read only
            @return
                <TRUE/> if read only, otherwise <FALSE/>
        */
        virtual sal_Bool isDataSourceReadOnly() const;

        /** checks if the connection for the selected data source is read only. If the connection doesn't exist, <TRUE/> will be returned.
            @return
                <TRUE/> if read only or doesn't exist, otherwise <FALSE/>
        */
        sal_Bool isConnectionReadOnly() const;

        /** checks if the database allows the creation of relation
            @return
                <TRUE/> if read only, otherwise <FALSE/>
        */
        sal_Bool isRelationDesignAllowed() const;

        /** fills the list with the selected entries.
            @param  _rNames
        */
        void getSelectionElementNames(::std::vector< ::rtl::OUString>& _rNames);

        /// deletes the entries selected.
        void deleteEntries();

        /// renames the selected entry in the detail page
        void renameEntry();

        /** deletes queries.
            @param  _eType
                the type of the objects
            @param  _rList
                The list of queries.
            @param  _nTextResource
                The ID for the resource string.
        */
        void deleteObjects( ElementType _eType
                            ,const ::std::vector< ::rtl::OUString>& _rList
                            ,sal_uInt16 _nTextResource);


        /** deletes tables.
            @param  _rList
                The list of tables.
        */
        void deleteTables(const ::std::vector< ::rtl::OUString>& _rList);

        /// copies the current object into clipboard
        TransferableHelper* copyObject();

        /** fills all elements of the wanted type into the list.
            @param  _eType
                The type.
            @param  _rList
                The list to fill.
        */
        void getElements(ElementType _eType,::std::vector< ::rtl::OUString>& _rList);

        /// returns the nameaccess
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > getElements(ElementType _eType);

        ///

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
                <TRUE/> if the paste opertions was successfull, otherwise <FALSE/>.
        */
        sal_Bool paste( ElementType _eType,const ::svx::ODataAccessDescriptor& _rPasteData ,const String& _sParentFolder = String(),sal_Bool _bMove = sal_False);

        /** pastes a table into the data source
            @param  _rPasteData
                The data helper.
        */
        void pasteTable( const TransferableDataHelper& _rTransData );

        /** pastes a table into the data source
            @param  _nFormatId
                The format which should be copied.
            @param  _rPasteData
                The data helper.
        */
        void pasteTable( SotFormatStringId _nFormatId,const TransferableDataHelper& _rTransData );

        /** pastes a table into the data source
            @param  _rPasteData
                The data descriptor.
        */
        void pasteTable( const ::svx::ODataAccessDescriptor& _rPasteData );

        /** insert a table into the data source. The source can eihter be a table or a query
            @param  _nCommandType
                The command type.
            @param  _xSrcConnection
                The connection of the source.
            @param  _xSrcRs
                The ResultSet of the source.
            @param  _aSelection
                The selection of the rows to copy.
            @param  _bBookmarkSelection
                If <TRUE/> the selection is bookmark selection.
            @param  _sCommand
                The name of the query or table.
            @param  _sSrcDataSourceName
                The name of the source data source.
        */
        void insertTable( sal_Int32 _nCommandType
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xSrcConnection
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>&   _xSrcRs         // the source resultset may be empty
                        ,const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _aSelection
                        ,sal_Bool _bBookmarkSelection
                        ,const ::rtl::OUString& _sCommand
                        ,const ::rtl::OUString& _sSrcDataSourceName);

        /** copies a table which was constructed by tags like HTML or RTF
            @param  _rDesc
                The Drop descriptor
            @param  _bCheck
                If set to <TRUE/> than the controller checks only if a copy is possible.
        */
        sal_Bool copyTagTable(DropDescriptor& _rDesc, sal_Bool _bCheck);

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

        /** crates a number formatter
            @param  _rxConnection
                The connection is needed to create the formatter
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > getNumberFormatter(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection ) const;

        /** adds a listener to the current name access.
            @param  _xCollection
                The collection where we want to listen on.
        */
        void addContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xCollection);

        /** opens the popup for the toolbox item
            @param  _nSlotId
                The slot id for whom the popup shoudl be opened.
        */
        void openToolBoxPopup(USHORT _nSlotId);


        /** opens a uno dialog withthe currently selected data source as initialize argument
            @param  _sServiceName
                The serivce name of the dialog to be executed.
        */
        void openDialog(const ::rtl::OUString& _sServiceName);

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

        /** suspend all open documents.
            @return
                <TRUE/> if all documents could be suspended, otherwise <FALSE/>.
        */
        sal_Bool suspendDocuments(sal_Bool bSuspend);

        /** disposes all created connections
        */
        void clearConnections();

        /** returns the filter for this controller
            @retrun
                the filter
        */
        const SfxFilter* getStandardFilter();

        /** add event listener and remember the document
            @param  _xDocument
                the new document, may be <NULL/>
            @param  _xDefinition
                The defintion object.
        */
        void addDocumentListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _xDocument,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _xDefinition);

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
                <TRUE/> if the insert opertions was successfull, otherwise <FALSE/>.
        */
        sal_Bool insertHierachyElement(  ElementType _eType
                                    ,const String& _sParentFolder
                                    ,sal_Bool _bCollection = sal_True
                                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent>& _xContent = ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent>()
                                    ,sal_Bool _bMove = sal_False);
    protected:
        // ----------------------------------------------------------------
        // initalizing members
        /** forces usage of a connection which we do not own
            <p>To be used from within XInitialization::initialize only.</p>
        */
        void                    initializeConnection( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxForeignConn );

        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId);
        virtual ToolBox*        CreateToolBox(Window* pParent);

        // IControlActionListener overridables
        virtual sal_Bool        requestContextMenu( const CommandEvent& _rEvent );
        virtual sal_Bool        requestDrag( sal_Int8 _nAction, const Point& _rPosPixel );
        virtual sal_Int8        queryDrop( const AcceptDropEvent& _rEvt, const DataFlavorExVector& _rFlavors );
        virtual sal_Int8        executeDrop( const ExecuteDropEvent& _rEvt );

        // OGenericUnoController
        virtual void            updateTitle( );

        virtual void impl_initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments );

        virtual ~OApplicationController();
    public:
        OApplicationController(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);

        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
        // need by registration
        static ::rtl::OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

        // ::com::sun::star::frame::XController
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL attachModel(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xModel) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >  SAL_CALL getModel(void) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::frame::XFrameActionListener
        virtual void SAL_CALL frameAction(const ::com::sun::star::frame::FrameActionEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::container::XContainerListener
        virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& Event) throw( ::com::sun::star::uno::RuntimeException );

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException);

        /** called when an item in the toolbox has been selected
            <p>the default handling calls Execute with the id given.</p>
        */
        virtual void onToolBoxSelected( sal_uInt16 _nSelectedItem );
        /** called when an item in the toolbox has been clicked
            <p>the default handling does nothing.</p>
        */
        virtual void onToolBoxClicked( sal_uInt16 _nClickedItem );

        /** ensures that a connection for the selected data source exists
            @param  _xConnection
                The new connection
            @param  _bCreate
                If set to <TRUE/> than the connection will be created if it doesn't exist.
        */
        void ensureConnection(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,sal_Bool _bCreate = sal_True);

        /** returns the connection for the currently active data source
            @return
                The connection for the currently active data source
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > getActiveConnection() const;


        /** refreshes the tables
        */
        void refreshTables();

        /** closes the connection of the select data source
        */
        void closeConnection();

        /// @see <method>IApplicationElementNotification::onEntryDoubleClick</method>
        virtual void onEntryDoubleClick(SvTreeListBox* _pTree);
        /// @see <method>IApplicationElementNotification::onCreationClick</method>
        virtual void onCreationClick(sal_uInt16 _nId);
        /// @see <method>IApplicationElementNotification::onContainerSelect</method>
        virtual sal_Bool onContainerSelect(ElementType _eType);
        /// @see <method>IApplicationElementNotification::onEntrySelect</method>
        virtual void onEntrySelect(SvLBoxEntry* _pEntry);
        /// @see <method>IApplicationElementNotification::onEntryDeSelect</method>
        virtual void onEntryDeSelect(SvTreeListBox* _pTree);
        /// @see <method>IApplicationElementNotification::onCutEntry</method>
        virtual void onCutEntry(SvLBoxEntry* _pEntry);
        /// @see <method>IApplicationElementNotification::onCopyEntry</method>
        virtual void onCopyEntry(SvLBoxEntry* _pEntry);
        /// @see <method>IApplicationElementNotification::onPasteEntry</method>
        virtual void onPasteEntry(SvLBoxEntry* _pEntry);
        /// @see <method>IApplicationElementNotification::onDeleteEntry</method>
        virtual void onDeleteEntry(SvLBoxEntry* _pEntry);

        DECL_LINK( OnShowRefreshDropDown, void* );
        DECL_LINK( OnPasteSpecialDropDown, void* );

        // time to check the CUT/COPY/PASTE-slot-states
        DECL_LINK( OnInvalidateClipboard, void* );
        DECL_LINK( OnClipboardChanged, void* );
        DECL_LINK( OnAsyncDrop, void* );

        // IContainerFoundListener
        virtual void containerFound( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >& _xContainer);

        // IViewChangeListener
        virtual void previewChanged( sal_Int32 _nMode);
    protected:

        /** the connection will be disposed and set to NULL
            @param  _xConnection
                the connection to be disposed
        */
        virtual void        disconnect(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection);

        // late construction
        virtual sal_Bool    Construct(Window* pParent);
        virtual void        AddSupportedFeatures();

    protected:
        // XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

        // OComponentHelper
        virtual void SAL_CALL disposing();
    };

//........................................................................
}   // namespace dbaui
//........................................................................

#endif // DBAUI_APPCONTROLLER_HXX

