/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef DBAUI_SINGLEDOCCONTROLLER_HXX
#define DBAUI_SINGLEDOCCONTROLLER_HXX

#include "genericcontroller.hxx"
#include "IEnvironment.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/util/XModifiable.hpp>
/** === end UNO includes === **/

#include <comphelper/broadcasthelper.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include <connectivity/dbmetadata.hxx>
#include <cppuhelper/implbase2.hxx>
#include <svl/undo.hxx>

#include <memory>

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= OSingleDocumentController
    //====================================================================
    class OSingleDocumentController;

    typedef ::cppu::ImplInheritanceHelper2  <   OGenericUnoController
                                            ,   ::com::sun::star::document::XScriptInvocationContext
                                            ,   ::com::sun::star::util::XModifiable
                                            >   OSingleDocumentController_Base;

    struct OSingleDocumentControllerImpl;
    class DBACCESS_DLLPUBLIC OSingleDocumentController
            :public OSingleDocumentController_Base
            ,public IEnvironment
    {
    private:
        ::std::auto_ptr<OSingleDocumentControllerImpl> m_pImpl;

    protected:
        SfxUndoManager  m_aUndoManager;

    private:
        /** forces usage of a connection which we do not own
            <p>To be used from within XInitialization::initialize, resp. impl_initialize, only.</p>
        */
        void        initializeConnection( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxForeignConn );

    protected:
        // OGenericUnoController - initialization
        virtual void impl_initialize();

        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > getPrivateModel() const;

                sal_Bool impl_isModified() const;
        virtual void     impl_onModifyChanged();

    public:

        sal_Bool        isReadOnly()            const;
        sal_Bool        isEditable()            const;
        void            setEditable(sal_Bool _bEditable);

        // need for undo's and redo's
        SfxUndoManager& GetUndoManager();

        /** addUndoActionAndInvalidate adds an undo action to the undoManager,
            additionally invalidates the UNDO and REDO slot
            @param  pAction the undo action to add
        */
        void addUndoActionAndInvalidate(SfxUndoAction *pAction);
        // ----------------------------------------------------------------
        // asking for connection-related stuff

        sal_Bool    isConnected() const;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >
                    getMetaData( ) const;

        // ----------------------------------------------------------------
        // access to the data source / document
        ::rtl::OUString getDataSourceName() const;
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >&
                    getDataSource() const;
        sal_Bool    haveDataSource() const;

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
                    getDatabaseDocument() const;

        /** provides access to the SDB-level database meta data of the current connection
        */
        const ::dbtools::DatabaseMetaData& getSdbMetaData() const;

        // IEnvironment
        /** appends an error in the current environment.
        */
        virtual void appendError(
                        const ::rtl::OUString& _rErrorMessage,
                        const ::dbtools::StandardSQLState _eSQLState = ::dbtools::SQL_GENERAL_ERROR,
                        const sal_Int32 _nErrorCode = 1000
                     );

        /** clears the error state.
        */
        virtual void clearError();

        /** @return
                <TRUE/> when an error was set otherwise <FALSE/>
        */
        virtual sal_Bool hasError() const;

        /** returns the current error
        */
        virtual const ::dbtools::SQLExceptionInfo& getError() const;

        /** displays the current error, or does nothing if there is no current error
        */
        virtual void displayError();

        /** shows an info box with the string conntection lost.
        */
        void connectionLostMessage() const;

        /** gives access to the currently used connection
            @return
                the currently used connection.
        */
        virtual const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >&
                    getConnection() const;

        /** returns the number formatter
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    getNumberFormatter() const;

        // ::com::sun::star::frame::XController
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL attachModel(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xModel) throw( ::com::sun::star::uno::RuntimeException );

        // XScriptInvocationContext
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedScripts > SAL_CALL getScriptContainer() throw (::com::sun::star::uno::RuntimeException);

        // XModifiable
        virtual ::sal_Bool SAL_CALL isModified(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setModified( ::sal_Bool bModified ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);

        // XModifyBroadcaster
        virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

        // XTitle
        virtual ::rtl::OUString SAL_CALL getTitle(  ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        OSingleDocumentController(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxORB);
        virtual ~OSingleDocumentController();

        virtual void        disconnect();
        virtual void        reconnect( sal_Bool _bUI );
                sal_Bool    ensureConnected( sal_Bool _bUI )    { if ( !isConnected() ) reconnect( _bUI ); return isConnected(); }

        /** called when our connection is beeing disposed
            <p>The default implementation does a reconnect</p>
        */
        virtual void losingConnection( );

        // late construction
        virtual sal_Bool Construct(Window* pParent);

    protected:
        // XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

        // OComponentHelper
        virtual void SAL_CALL disposing();

        // XInterface
        virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException);

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        sal_Int32 getCurrentStartNumber() const;

    private:
        OSingleDocumentController();    // never implemented
    };

//........................................................................
}   // namespace dbaui
//........................................................................

#endif // DBAUI_SINGLEDOCCONTROLLER_HXX

