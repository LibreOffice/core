/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: singledoccontroller.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 07:50:28 $
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

#ifndef DBAUI_SINGLEDOCCONTROLLER_HXX
#define DBAUI_SINGLEDOCCONTROLLER_HXX

#ifndef DBAUI_GENERICCONTROLLER_HXX
#include "genericcontroller.hxx"
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COMPHELPER_PROPERTYCONTAINER_HXX_
#include <comphelper/propertycontainer.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef DBAUI_IENVIRONMENT_HXX
#include "IEnvironment.hxx"
#endif
#ifndef DBACCESS_SOURCE_UI_INC_DOCUMENTCONTROLLER_HXX
#include "documentcontroller.hxx"
#endif
#ifndef CONNECTIVITY_INC_CONNECTIVITY_DBMETADATA_HXX
#include <connectivity/dbmetadata.hxx>
#endif
#ifndef _UNDO_HXX
#include <svtools/undo.hxx>
#endif

#include <memory>

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= OSingleDocumentController
    //====================================================================
    class OSingleDocumentController;
    typedef OGenericUnoController                   OSingleDocumentController_CBASE;

    struct OSingleDocumentControllerImpl;
    class DBACCESS_DLLPUBLIC OSingleDocumentController
            :public OSingleDocumentController_CBASE
            ,public IEnvironment
    {
        friend class OConnectionChangeBroadcaster;
    private:
        ::std::auto_ptr<OSingleDocumentControllerImpl> m_pImpl;


    protected:
        SfxUndoManager  m_aUndoManager;

    protected:
        // ----------------------------------------------------------------
        // initalizing members
        /** forces usage of a connection which we do not own
            <p>To be used from within XInitialization::initialize only.</p>
        */
        void        initializeConnection( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxForeignConn );

        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);

    public:

        sal_Bool        isReadOnly()            const;
        sal_Bool        isEditable()            const;
        void            setEditable(sal_Bool _bEditable);
        sal_Bool        isModified()            const;

        virtual void setModified(sal_Bool _bModified=sal_True);


        // need for undo's and redo's
        SfxUndoManager* getUndoMgr();

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
        // access to the data source
        ::rtl::OUString getDataSourceName() const;
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >&
                    getDataSource() const;
        sal_Bool    haveDataSource() const;


        /** provides to the SDB-level database meta data of the current connection
        */
        const ::dbtools::DatabaseMetaData& getSdbMetaData() const;

        // IEnvironment
        /** appends an error in the current environment.
            @param  _aException
                contains a description of the error or the error directly
        */
        virtual void appendError(const ::com::sun::star::sdbc::SQLException& _aException);

        /** clears the error state.
        */
        virtual void clearError();

        /** set the current error in the given parameter.
            @param  _rException
                will contain the current error
        */
        virtual void getError(::com::sun::star::sdbc::SQLException& _rException ) const;

        /** @retrun
            returns <TRUE/> when an error was set otherwise <FALSE/>
        */
        virtual sal_Bool hasError() const;

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
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >  SAL_CALL getModel(void) throw( ::com::sun::star::uno::RuntimeException );

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
        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

        // OComponentHelper
        virtual void SAL_CALL disposing();
    };

//........................................................................
}   // namespace dbaui
//........................................................................

#endif // DBAUI_SINGLEDOCCONTROLLER_HXX

