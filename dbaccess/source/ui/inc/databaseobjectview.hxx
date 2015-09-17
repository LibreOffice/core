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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_DATABASEOBJECTVIEW_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_DATABASEOBJECTVIEW_HXX

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

#include <memory>

namespace dbaui
{
    /** encapsulates access to the view of a database object.

        @todo
            this is to be merged with the OLinkedDocumentAccess class
    */
    class DatabaseObjectView
    {
    private:
        css::uno::Reference< css::uno::XComponentContext >
                        m_xORB;
        css::uno::Reference< css::frame::XFrame >
                        m_xParentFrame;
        css::uno::Reference< css::frame::XComponentLoader >
                        m_xFrameLoader;
        css::uno::Reference< css::sdb::application::XDatabaseDocumentUI >
                        m_xApplication;
        OUString        m_sComponentURL;

    private:
        css::uno::Reference< css::lang::XComponent >
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
        virtual css::uno::Reference< css::lang::XComponent > doCreateView(
            const css::uno::Any& _rDataSource,
            const OUString& _rObjectName,
            const ::comphelper::NamedValueCollection& i_rCreationArgs
        );

        virtual void fillDispatchArgs(
                          ::comphelper::NamedValueCollection& i_rDispatchArgs,
                    const css::uno::Any& _rDataSource,
                    const OUString& _rObjectName
                );

        const css::uno::Reference< css::sdb::application::XDatabaseDocumentUI >&
                getApplicationUI() const { return m_xApplication; }
        css::uno::Reference< css::sdbc::XConnection >
                getConnection() const;

    public:
        DatabaseObjectView(
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB,
            const css::uno::Reference< css::sdb::application::XDatabaseDocumentUI >& _rxApplication,
            const css::uno::Reference< css::frame::XFrame >& _rxParentFrame,
            const OUString& _rComponentURL
        );
        virtual ~DatabaseObjectView(){}

        /** sets the target frame into which the view should be loaded.

            By default, the view is loaded into a top-level frame not being part of the
            Desktop.
        */
        void setTargetFrame( const css::uno::Reference< css::frame::XFrame >& _rxFrame )
        {
            m_xFrameLoader.set( _rxFrame, css::uno::UNO_QUERY );
        }

        /** opens a view for a to-be-created object

            @param _xDataSource
                the data source for which a new object is to be created
            @return
                the controller of the newly created document
        */
        css::uno::Reference< css::lang::XComponent >
            createNew(
                const css::uno::Reference< css::sdbc::XDataSource >& _xDataSource,
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
        css::uno::Reference< css::lang::XComponent >
            openExisting(
                const css::uno::Any& _aDataSource,
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
            const css::uno::Any& _aDataSource,
            const OUString& _rObjectName
        ) SAL_OVERRIDE;

    public:
        QueryDesigner(
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB,
            const css::uno::Reference< css::sdb::application::XDatabaseDocumentUI >& _rxApplication,
            const css::uno::Reference< css::frame::XFrame >& _rxParentFrame,
            bool _bCreateView
        );
    };

    // TableDesigner
    class TableDesigner : public DatabaseObjectView
    {
    protected:
        virtual void fillDispatchArgs(
                  ::comphelper::NamedValueCollection& i_rDispatchArgs,
            const css::uno::Any& _aDataSource,
            const OUString& _rObjectName
        ) SAL_OVERRIDE;

        virtual css::uno::Reference< css::lang::XComponent > doCreateView(
            const css::uno::Any& _rDataSource,
            const OUString& _rObjectName,
            const ::comphelper::NamedValueCollection& i_rCreationArgs
        ) SAL_OVERRIDE;

    public:
        TableDesigner(
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB,
            const css::uno::Reference< css::sdb::application::XDatabaseDocumentUI >& _rxApplication,
            const css::uno::Reference< css::frame::XFrame >& _rxParentFrame
        );

    private:
        /** retrieves the table designer component as provided by the connection, if any
            @param  _rTableName
                the name of the table for which a designer is to be obtained
            @return
                the designer component, as provided by the connection, or <NULL/>, if the connection
                does not provide a specialized designer.
            @see css::sdb::application::XTableUIProvider
        */
        css::uno::Reference< css::uno::XInterface >
                impl_getConnectionProvidedDesigner_nothrow( const OUString& _rTableName );
    };

    // ResultSetBrowser
    class ResultSetBrowser : public DatabaseObjectView
    {
    private:
        bool    m_bTable;

    protected:
        virtual void fillDispatchArgs(
                  ::comphelper::NamedValueCollection& i_rDispatchArgs,
            const css::uno::Any& _aDataSource,
            const OUString& _rQualifiedName
        ) SAL_OVERRIDE;

    public:
        ResultSetBrowser(
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB,
            const css::uno::Reference< css::sdb::application::XDatabaseDocumentUI >& _rxApplication,
            const css::uno::Reference< css::frame::XFrame >& _rxParentFrame,
            bool _bTable
        );

    };
    // RelationDesigner
    class RelationDesigner : public DatabaseObjectView
    {
    public:
        RelationDesigner(
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB,
            const css::uno::Reference< css::sdb::application::XDatabaseDocumentUI >& _rxApplication,
            const css::uno::Reference< css::frame::XFrame >& _rxParentFrame
        );
    };
}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_DATABASEOBJECTVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
