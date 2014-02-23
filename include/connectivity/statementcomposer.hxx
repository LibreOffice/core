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

#ifndef INCLUDED_CONNECTIVITY_STATEMENTCOMPOSER_HXX
#define INCLUDED_CONNECTIVITY_STATEMENTCOMPOSER_HXX

#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>

#include <boost/noncopyable.hpp>

#include <memory>
#include <connectivity/dbtoolsdllapi.hxx>


namespace dbtools
{



    //= StatementComposer

    struct StatementComposer_Data;
    /** a class which is able to compose queries (SELECT statements) from a command and a command type
    */
    class OOO_DLLPUBLIC_DBTOOLS StatementComposer : public ::boost::noncopyable
    {
        ::std::auto_ptr< StatementComposer_Data >   m_pData;

    public:
        /** constructs an instance

            @param _rxConnection
                the connection to work with. Must not be <NULL/>.
        */
        StatementComposer(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const OUString&  _rCommand,
            const sal_Int32         _nCommandType,
            const sal_Bool          _bEscapeProcessing
        );

        ~StatementComposer();

        /** controls whether or not the instance disposes its XSingleSelectQueryComposer upon
            destruction

            Unless you explicitly call this method with the parameter being <TRUE/>,
            the XSingleSelectQueryComposer will be disposed when the StatementComposer
            instance is destroyed.
        */
        void    setDisposeComposer( bool _bDoDispose );

        void    setFilter( const OUString& _rFilter );
        void    setOrder( const OUString& _rOrder );

        /** returns the composer which has been fed with the current settings

            @throws ::com::sun::star::sdbc::SQLException
                if such an exception occurs while creating the composer
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer >
                getComposer();

        /** returns the composer statement

            Effectively, this is equivalent to calling getComposer, and asking the composer
            for its Query attribute.

            @throws ::com::sun::star::sdbc::SQLException
                if such an exception occurs while creating the composer
        */
        OUString
                getQuery();

    private:
        StatementComposer();    // not implemented
    };


} // namespace dbtools


#endif // INCLUDED_CONNECTIVITY_STATEMENTCOMPOSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
