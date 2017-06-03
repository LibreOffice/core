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

#include <connectivity/dbtools.hxx>
#include <tools/link.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>


namespace pcr
{


    class ISQLCommandAdapter;

    //= SQLCommandDesigner

    typedef ::cppu::WeakImplHelper <   css::beans::XPropertyChangeListener
                                    >   SQLCommandDesigner_Base;
    /** encapsulates the code for calling and managing a query design frame, used
        for interactively designing the Command property of a ->RowSet
    */
    class SQLCommandDesigner : public SQLCommandDesigner_Base
    {
    private:
        css::uno::Reference< css::uno::XComponentContext >        m_xContext;
        css::uno::Reference< css::lang::XMultiComponentFactory >  m_xORB;
        ::dbtools::SharedConnection                               m_xConnection;
        css::uno::Reference< css::frame::XController >            m_xDesigner;
        ::rtl::Reference< ISQLCommandAdapter >                    m_xObjectAdapter;
        Link<SQLCommandDesigner&,void>                            m_aCloseLink;

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
        @throws css::lang::NullPointerException
            if any of the arguments (except ->_rCloseLink) is <NULL/>, or if the component context
            does not provide a valid component factory.
        */
        SQLCommandDesigner(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            const ::rtl::Reference< ISQLCommandAdapter >& _rxPropertyAdapter,
            const ::dbtools::SharedConnection& _rConnection,
            const Link<SQLCommandDesigner&,void>& _rCloseLink
        );

        /** determines whether the SQL Command designer is currently active, i.e.
            if there currently exists a frame which allows the user entering the SQL command
        */
        bool isActive() const { return m_xDesigner.is(); }

        /** returns the property adapter used by the instance
        */
        const ::rtl::Reference< ISQLCommandAdapter >& getPropertyAdapter() const { return m_xObjectAdapter; }

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
        virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& evt ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    protected:
        virtual ~SQLCommandDesigner() override;

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
            @throws css::lang::DisposedException
                if we in fact are disposed
        */
        void impl_checkDisposed_throw() const;

        /** create an empty top-level frame, which does not belong to the desktop's frame list
            @precond
                ->m_xORB is not <NULL/>
        */
        css::uno::Reference< css::frame::XFrame >
            impl_createEmptyParentlessTask_nothrow() const;

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
        SQLCommandDesigner( const SQLCommandDesigner& ) = delete;
        SQLCommandDesigner& operator=( const SQLCommandDesigner& ) = delete;
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

        virtual ~ISQLCommandAdapter() override;
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_SQLCOMMANDDESIGN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
