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

#pragma once

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <memory>

#include "abptypes.hxx"


namespace com::sun::star {
    namespace beans {
        class XPropertySet;
    }
}

namespace weld { class Window; }


namespace abp
{

    struct ODataSourceContextImpl;
    class ODataSource;
    /// a non-UNO wrapper for the data source context
    class ODataSourceContext
    {
    private:
        std::unique_ptr<ODataSourceContextImpl>   m_pImpl;

    public:
        explicit ODataSourceContext(
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB
        );
        ~ODataSourceContext();

        /// retrieves the names of all data sources
        void    getDataSourceNames( StringBag& _rNames ) const;

        /// disambiguates the given name by appending successive numbers
        void disambiguate(OUString& _rDataSourceName);

        /// creates a new Thunderbird data source
        ODataSource createNewThunderbird( const OUString& _rName );

        /// creates a new Evolution local data source
        ODataSource createNewEvolution( const OUString& _rName );

        /// creates a new Evolution LDAP data source
        ODataSource createNewEvolutionLdap( const OUString& _rName );

        /// creates a new Evolution GROUPWISE data source
        ODataSource createNewEvolutionGroupwise( const OUString& _rName );

        /// creates a new KDE address book data source
        ODataSource createNewKab( const OUString& _rName );

        /// creates a new macOS address book data source
        ODataSource createNewMacab( const OUString& _rName );

        /// creates a new Other data source; tdf117101: Spreadsheet by default
        ODataSource createNewOther( const OUString& _rName );
    };

    struct ODataSourceImpl;
    struct AddressSettings;
    /** a non-UNO wrapper for a data source
        <p>This class allows to access data sources without the need to compile the respective file with
        exception handling enabled (hopefully :).</p>
        <p>In addition to wrapping a UNO data source, an instance of this class can handle at most
        one valid connection, as obtained from the data source.</p>
    */
    class ODataSource
    {
    private:
        std::unique_ptr<ODataSourceImpl>    m_pImpl;

    public:

        // - ctor/dtor/assignment

        /// constructs an object which is initially invalid
        explicit ODataSource(
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB
        );

        /// copy ctor
        ODataSource( const ODataSource& _rSource );

        /// dtor
        ~ODataSource( );

        /// copy assignment
        ODataSource& operator=( const ODataSource& _rSource );

        /// move assignment
        ODataSource& operator=(ODataSource&& _rSource) noexcept;

        /// checks whether or not the object represents a valid data source
        bool    isValid() const;


        /// removes the data source represented by the object from the data source context
        void        remove();
            // TODO: put this into the context class

        /// returns the name of the data source
        OUString
                    getName() const;

        /// renames the data source
        bool    rename( const OUString& _rName );
            // TODO: put this into the context class


        // - connection handling

        /** connects to the data source represented by this object
            @param _pMessageParent
                the window to use as parent for any error messages. If this is <NULL/>, no messages are displayed
                at all.
            @see isConnected
        */
        bool    connect(weld::Window* _pMessageParent);

        /// returns <TRUE/> if the object has a valid connection, obtained from its data source
        bool    isConnected( ) const;

        /// disconnects from the data source (i.e. disposes the UNO connection hold internally)
        void        disconnect( );

        /// stores the database file
        void        store(const AddressSettings& rSettings);

        /// register the data source under the given name in the configuration
        void        registerDataSource( const OUString& _sRegisteredDataSourceName );


        /** retrieves the tables names from the connection
            <p>to be called when <method>isConnected</method> returns <TRUE/> only</p>
        */
        const StringBag&    getTableNames() const;

        /** determines whether a given table exists
        */
        bool    hasTable( const OUString& _rTableName ) const;

        /// return the intern data source object
        css::uno::Reference< css::beans::XPropertySet > getDataSource() const;


        /** set a new data source.
            <p>Available to selected clients only</p>
        */
        void        setDataSource(
             const css::uno::Reference< css::beans::XPropertySet >& _rxDS
            ,const OUString& _sName
        );
    };


}   // namespace abp
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
