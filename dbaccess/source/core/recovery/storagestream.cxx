/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
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
************************************************************************/

#include "precompiled_dbaccess.hxx"

#include "storagestream.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/embed/ElementModes.hpp>
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>

//........................................................................
namespace dbaccess
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::embed::XStorage;
    using ::com::sun::star::io::XStream;
    /** === end UNO using === **/
    namespace ElementModes = ::com::sun::star::embed::ElementModes;

    //====================================================================
    //= StorageOutputStream
    //====================================================================
    //--------------------------------------------------------------------
    StorageOutputStream::StorageOutputStream(   const ::comphelper::ComponentContext& i_rContext,
                                                const Reference< XStorage >& i_rParentStorage,
                                                const ::rtl::OUString& i_rStreamName
                                             )
        :m_rContext( i_rContext )
    {
        ENSURE_OR_THROW( i_rParentStorage.is(), "illegal stream" );

        const Reference< XStream > xStream(
            i_rParentStorage->openStreamElement( i_rStreamName, ElementModes::READWRITE ), UNO_QUERY_THROW );
        m_xOutputStream.set( xStream->getOutputStream(), UNO_SET_THROW );
    }

    //--------------------------------------------------------------------
    StorageOutputStream::~StorageOutputStream()
    {
    }

    //--------------------------------------------------------------------
    void StorageOutputStream::close()
    {
        ENSURE_OR_RETURN_VOID( m_xOutputStream.is(), "already closed" );
        m_xOutputStream->closeOutput();
        m_xOutputStream.clear();

        // if you add additional functionality here, be aware that there are derived classes which
        // (legitimately) do not call this method here.
    }

    //====================================================================
    //= StorageInputStream
    //====================================================================
    //--------------------------------------------------------------------
    StorageInputStream::StorageInputStream( const ::comphelper::ComponentContext& i_rContext,
                                            const Reference< XStorage >& i_rParentStorage,
                                            const ::rtl::OUString& i_rStreamName
                                          )
        :m_rContext( i_rContext )
    {
        ENSURE_OR_THROW( i_rParentStorage.is(), "illegal stream" );

        const Reference< XStream > xStream(
            i_rParentStorage->openStreamElement( i_rStreamName, ElementModes::READ ), UNO_QUERY_THROW );
        m_xInputStream.set( xStream->getInputStream(), UNO_SET_THROW );
    }

    //--------------------------------------------------------------------
    StorageInputStream::~StorageInputStream()
    {
    }

    //--------------------------------------------------------------------
    void StorageInputStream::close()
    {
        ENSURE_OR_RETURN_VOID( m_xInputStream.is(), "already closed" );
        m_xInputStream->closeInput();
        m_xInputStream.clear();

        // if you add additional functionality here, be aware that there are derived classes which
        // (legitimately) do not call this method here.
    }

//........................................................................
} // namespace dbaccess
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
