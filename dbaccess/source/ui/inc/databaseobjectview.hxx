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

#ifndef DBACCESS_DATABASE_OBJECT_VIEW_HXX
#define DBACCESS_DATABASE_OBJECT_VIEW_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/namedvaluecollection.hxx>

#include <boost/shared_ptr.hpp>

namespace dbaui
{
    /** encapsulates access to the view of a database object.

        @todo
            this is to be merged with the OLinkedDocumentAccess class
    */
    class DatabaseObjectView
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                        m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                        m_xParentFrame;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XComponentLoader >
                        m_xFrameLoader;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >
                        m_xApplication;
        OUString m_sComponentURL;

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
                doDispatch(
                    const ::comphelper::NamedValueCollection& i_rDispatchArgs
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
            const OUString& _rObjectName,
            const ::comphelper::NamedValueCollection& i_rCreationArgs
        );

        virtual void fillDispatchArgs(
                          ::comphelper::NamedValueCollection& i_rDispatchArgs,
                    const ::com::sun::star::uno::Any& _rDataSource,
                    const OUString& _rObjectName
                );

        const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >&
                getApplicationUI() const { return m_xApplication; }
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                getConnection() const;

    public:
        DatabaseObjectView(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& _rxApplication,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxParentFrame,
            const OUString& _rComponentURL
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
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource >& _xDataSource,
                const ::comphelper::NamedValueCollection& i_rDispatchArgs = ::comphelper::NamedValueCollection()
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
                const OUString& _rName,
                const ::comphelper::NamedValueCollection& i_rDispatchArgs
            );
    };

    // QueryDesigner
    class QueryDesigner : public DatabaseObjectView
    {
    protected:
        sal_Int32                           m_nCommandType;

    protected:
        virtual void fillDispatchArgs(
                  ::comphelper::NamedValueCollection& i_rDispatchArgs,
            const ::com::sun::star::uno::Any& _aDataSource,
            const OUString& _rObjectName
        );

    public:
        QueryDesigner(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& _rxApplication,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxParentFrame,
            bool _bCreateView
        );
    };

    // TableDesigner
    class TableDesigner : public DatabaseObjectView
    {
    protected:
        virtual void fillDispatchArgs(
                  ::comphelper::NamedValueCollection& i_rDispatchArgs,
            const ::com::sun::star::uno::Any& _aDataSource,
            const OUString& _rObjectName
        );

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > doCreateView(
            const ::com::sun::star::uno::Any& _rDataSource,
            const OUString& _rObjectName,
            const ::comphelper::NamedValueCollection& i_rCreationArgs
        );

    public:
        TableDesigner(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB,
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
                impl_getConnectionProvidedDesigner_nothrow( const OUString& _rTableName );
    };

    // ResultSetBrowser
    class ResultSetBrowser : public DatabaseObjectView
    {
    private:
        sal_Bool    m_bTable;

    protected:
        virtual void fillDispatchArgs(
                  ::comphelper::NamedValueCollection& i_rDispatchArgs,
            const ::com::sun::star::uno::Any& _aDataSource,
            const OUString& _rQualifiedName
        );

    public:
        ResultSetBrowser(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& _rxApplication,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxParentFrame,
            sal_Bool _bTable
        );

    };
    // RelationDesigner
    class RelationDesigner : public DatabaseObjectView
    {
    public:
        RelationDesigner(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& _rxApplication,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxParentFrame
        );
    };
}   // namespace dbaui

#endif // DBACCESS_DATABASE_OBJECT_VIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
