/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef CONNECTIVITY_STATEMENTCOMPOSER_HXX
#define CONNECTIVITY_STATEMENTCOMPOSER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
/** === end UNO includes === **/

#include <boost/noncopyable.hpp>

#include <memory>
#include "connectivity/dbtoolsdllapi.hxx"

//........................................................................
namespace dbtools
{
//........................................................................

    //====================================================================
    //= StatementComposer
    //====================================================================
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
            const ::rtl::OUString&  _rCommand,
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
        bool    getDisposeComposer() const;

        void    setFilter( const ::rtl::OUString& _rFilter );
        void    setOrder( const ::rtl::OUString& _rOrder );

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
        ::rtl::OUString
                getQuery();

    private:
        StatementComposer();    // not implemented
    };

//........................................................................
} // namespace dbtools
//........................................................................

#endif // CONNECTIVITY_STATEMENTCOMPOSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
