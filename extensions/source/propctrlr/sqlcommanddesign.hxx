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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_SQLCOMMANDDESIGN_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_SQLCOMMANDDESIGN_HXX

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/inspection/XObjectInspectorUI.hpp>

#include <connectivity/dbtools.hxx>
#include <tools/link.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>


namespace pcr
{


    class ISQLCommandAdapter;

    //= SQLCommandDesigner

    typedef ::cppu::WeakImplHelper <   ::com::sun::star::beans::XPropertyChangeListener
                                    >   SQLCommandDesigner_Base;
    /** encapsulates the code for calling and managing a query design frame, used
        for interactively designing the Command property of a ->RowSet
    */
    class SQLCommandDesigner : public SQLCommandDesigner_Base
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiComponentFactory >  m_xORB;
        ::dbtools::SharedConnection                                                         m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >            m_xDesigner;
        ::rtl::Reference< ISQLCommandAdapter >                                              m_xObjectAdapter;
        Link<SQLCommandDesigner&,void>                                                      m_aCloseLink;

    public:
        /** creates the instance, and immediately opens the SQL command design frame

        @param  _rxContext
            our component context. Must not be <NULL/>, and must provide a non-<NULL/> XMultiComponentFactory
        @param  _rxPropertyAdapter
            an adapter to the object's SQL command related properties
        @param  _rConnection
            the current connection of ->_rxRowSet. Must not be <NULL/>.
        @param _rCloseLink
            link to call when the component has been closed
        @throws ::com::sun::star::lang::NullPointerException
            if any of the arguments (except ->_rCloseLink) is <NULL/>, or if the component context
            does not provide a valid component factory.
        */
        SQLCommandDesigner(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            const ::rtl::Reference< ISQLCommandAdapter >& _rxPropertyAdapter,
            const ::dbtools::SharedConnection& _rConnection,
            const Link<SQLCommandDesigner&,void>& _rCloseLink
        );

        /** determines whether the SQL Command designer is currently active, i.e.
            if there currently exists a frame which allows the user entering the SQL command
        */
        inline bool isActive() const { return m_xDesigner.is(); }

        /** returns the property adapter used by the instance
        */
        inline const ::rtl::Reference< ISQLCommandAdapter >& getPropertyAdapter() const { return m_xObjectAdapter; }

        /** raises the designer window to top
            @precond
                the designer is active (->isActive)
            @precond
                the instance is not disposed
        */
        void    raise() const;

        /** suspends the designer
            @precond
                the designer is active (->isActive)
            @precond
                the instance is not disposed
        */
        bool    suspend() const;

        /** disposes the instance so that it becomes non-functional
        */
        void    dispose();

    protected:
        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    protected:
        virtual ~SQLCommandDesigner();

        /** opens a new frame for interactively designing an SQL command
            @precond
                the designer is not currently active (see ->isActive)
            @precond
                ->m_xConnection is not <NULL/>
        */
        void impl_doOpenDesignerFrame_nothrow();

        /** impl-version of ->raise
        */
        void impl_raise_nothrow() const;

        /** determines whether we are already disposed
        */
        bool impl_isDisposed() const
        {
            return !m_xContext.is();
        }
        /** checks whether we are already disposed
            @throws ::com::sun::star::lang::DisposedException
                if we in fact are disposed
        */
        void impl_checkDisposed_throw() const;

        /** create an empty top-level frame, which does not belong to the desktop's frame list
            @precond
                ->m_xORB is not <NULL/>
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
            impl_createEmptyParentlessTask_nothrow() const;

        /** called whenever the component denoted by m_xDesigner has been closed
            <em>by an external instance</em>
        */
        void impl_designerClosed_nothrow();

        /** closes the component denoted by m_xDesigner
            @precond
                our designer component is actually active (->isActive)
            @precond
                we're not disposed already
        */
        void impl_closeDesigner_nothrow();

        /** suspends our designer component
            @precond
                the designer component is actually active (->isActive)
            @return
                <TRUE/> if the suspension was successful, <FALSE/> if it was vetoed
        */
        bool impl_trySuspendDesigner_nothrow() const;

    private:
        SQLCommandDesigner( const SQLCommandDesigner& ) SAL_DELETED_FUNCTION;
        SQLCommandDesigner& operator=( const SQLCommandDesigner& ) SAL_DELETED_FUNCTION;
    };


    //= ISQLCommandAdapter

    /** an adapter to forward changed SQL command property values to a component
    */
    class ISQLCommandAdapter : public salhelper::SimpleReferenceObject
    {
    public:
        /// retrieves the current SQL command of the component
        virtual OUString getSQLCommand() const = 0;
        /// retrieves the current value of the EscapeProcessing property of the component
        virtual bool     getEscapeProcessing() const = 0;

        /// sets a new SQL command
        virtual void     setSQLCommand( const OUString& _rCommand ) const = 0;
        /// sets a new EscapeProcessing property value
        virtual void     setEscapeProcessing( const bool _bEscapeProcessing ) const = 0;

        virtual ~ISQLCommandAdapter();
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_SQLCOMMANDDESIGN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
