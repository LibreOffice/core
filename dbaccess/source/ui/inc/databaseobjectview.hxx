/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: databaseobjectview.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 16:03:08 $
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

#ifndef DBACCESS_DATABASE_OBJECT_VIEW_HXX
#define DBACCESS_DATABASE_OBJECT_VIEW_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_APPLICATION_XDATABASEDOCUMENTUI_HPP_
#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#include <boost/shared_ptr.hpp>


// .........................................................................
namespace dbaui
{
// .........................................................................
    /** encapsulates access to the view of a database object.

        @todo
            this is to be merged with the OLinkedDocumentAccess class
    */
    class DatabaseObjectView
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                        m_xParentFrame;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XComponentLoader >
                        m_xFrameLoader;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >
                        m_xApplication;
        ::rtl::OUString m_sComponentURL;

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
                doDispatch(
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rDispatchArguments
                );

    protected:
        /** creates the desired view

            The default implementation will call <member>fillDispatchArgs</member>, followed
            by <member>doDispatch</member>.

            @param _rDataSource
                the data source, as passed to the <member>createNew</member> or <member>openExisting</member> method.
            @param _rObjectName
                the name of the object for which the view is to be opened,
                or an empty string if a view for a new object should be created.
            @param _rCreationArgs
                the arguments for the view's creation
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > doCreateView(
            const ::com::sun::star::uno::Any& _rDataSource,
            const ::rtl::OUString& _rObjectName,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rCreationArgs
        );

        virtual void fillDispatchArgs(
                    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rDispatchArguments,
                    const ::com::sun::star::uno::Any& _rDataSource,
                    const ::rtl::OUString& _rObjectName
                );

        const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >&
                getApplicationUI() const { return m_xApplication; }
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                getConnection() const;

    public:
        DatabaseObjectView(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& _rxApplication,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxParentFrame,
            const ::rtl::OUString& _rComponentURL
        );
        virtual ~DatabaseObjectView(){}

        /** sets the target frame into which the view should be loaded.

            By default, the view is loaded into a top-level frame not being part of the
            Desktop.
        */
        void setTargetFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame )
        {
            m_xFrameLoader.set( _rxFrame, ::com::sun::star::uno::UNO_QUERY );
        }

        /** opens a view for a to-be-created object

            @param _xDataSource
                the data source for which a new object is to be created
            @return
                the controller of the newly created document
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
            createNew(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource >& _xDataSource
            );

        /** opens a view for an existent object

            @param _xDataSource
                the data source for which a new object is to be created
            @param _rObjectName
                the name of the object to be edited
            @param  _rArgs
                Additional settings which should be forwarded to the frame
            @return
                the frame into which the view has been loaded
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
            openExisting(
                const ::com::sun::star::uno::Any& _aDataSource,
                const ::rtl::OUString& _rName,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rArgs
            );
    };

    //======================================================================
    //= QueryDesigner
    //======================================================================
    class QueryDesigner : public DatabaseObjectView
    {
    protected:
        sal_Int32   m_nCommandType;
        sal_Bool    m_bPreferSQLView;

    protected:
        virtual void fillDispatchArgs(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rDispatchArguments,
            const ::com::sun::star::uno::Any& _aDataSource,
            const ::rtl::OUString& _rObjectName
        );

    public:
        QueryDesigner(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& _rxApplication,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxParentFrame,
            bool _bCreateView,
            sal_Bool _bPreferSQLView
        );
    };

    //======================================================================
    //= TableDesigner
    //======================================================================
    class TableDesigner : public DatabaseObjectView
    {
    protected:
        virtual void fillDispatchArgs(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rDispatchArguments,
            const ::com::sun::star::uno::Any& _aDataSource,
            const ::rtl::OUString& _rObjectName
        );

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > doCreateView(
            const ::com::sun::star::uno::Any& _rDataSource,
            const ::rtl::OUString& _rObjectName,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rCreationArgs
        );

    public:
        TableDesigner(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& _rxApplication,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxParentFrame
        );

    private:
        /** retrieves the table designer component as provided by the connection, if any
            @param  _rTableName
                the name of the table for which a designer is to be obtained
            @return
                the designer component, as provided by the connection, or <NULL/>, if the connection
                does not provide a specialized designer.
            @see com::sun::star::sdb::application::XTableUIProvider
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                impl_getConnectionProvidedDesigner_nothrow( const ::rtl::OUString& _rTableName );
    };

    //======================================================================
    //= ResultSetBrowser
    //======================================================================
    class ResultSetBrowser : public DatabaseObjectView
    {
    private:
        sal_Bool    m_bTable;

    protected:
        virtual void fillDispatchArgs(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rDispatchArguments,
            const ::com::sun::star::uno::Any& _aDataSource,
            const ::rtl::OUString& _rQualifiedName
        );

    public:
        ResultSetBrowser(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& _rxApplication,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxParentFrame,
            sal_Bool _bTable
        );

    };
    //======================================================================
    //= RelationDesigner
    //======================================================================
    class RelationDesigner : public DatabaseObjectView
    {
    public:
        RelationDesigner(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& _rxApplication,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxParentFrame
        );
    };
    //======================================================================
    //= ReportDesigner
    //======================================================================
    class ReportDesigner : public DatabaseObjectView
    {
    public:
        ReportDesigner(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& _rxApplication,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxParentFrame
        );
    };
// .........................................................................
}   // namespace dbaui
// .........................................................................

#endif // DBACCESS_DATABASE_OBJECT_VIEW_HXX

