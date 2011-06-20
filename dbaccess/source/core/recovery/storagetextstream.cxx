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

#include "storagetextstream.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/io/XTextOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <tools/diagnose_ex.h>

//......................................................................................................................
namespace dbaccess
{
//......................................................................................................................

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
    using ::com::sun::star::io::XTextOutputStream;
    using ::com::sun::star::io::XActiveDataSource;
    /** === end UNO using === **/

    //==================================================================================================================
    //= StorageTextOutputStream_Data
    //==================================================================================================================
    struct StorageTextOutputStream_Data
    {
        Reference< XTextOutputStream >  xTextOutput;
    };

    //==================================================================================================================
    //= helper
    //==================================================================================================================
    namespace
    {
        //--------------------------------------------------------------------------------------------------------------
        static const ::rtl::OUString& lcl_getTextStreamEncodingName()
        {
            static const ::rtl::OUString s_sMapStreamEncodingName( RTL_CONSTASCII_USTRINGPARAM( "UTF-8" ) );
            return s_sMapStreamEncodingName;
        }

        //--------------------------------------------------------------------------------------------------------------
        static const ::rtl::OUString& lcl_getLineFeed()
        {
            static const ::rtl::OUString s_sLineFeed( sal_Unicode( '\n' ) );
            return s_sLineFeed;
        }
    }

    //==================================================================================================================
    //= StorageTextOutputStream
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    StorageTextOutputStream::StorageTextOutputStream(   const ::comphelper::ComponentContext& i_rContext,
                                                        const Reference< XStorage >& i_rParentStorage,
                                                        const ::rtl::OUString& i_rStreamName
                                                    )
        :StorageOutputStream( i_rContext, i_rParentStorage, i_rStreamName )
        ,m_pData( new StorageTextOutputStream_Data )
    {
        m_pData->xTextOutput.set( i_rContext.createComponent( "com.sun.star.io.TextOutputStream" ), UNO_QUERY_THROW );
        m_pData->xTextOutput->setEncoding( lcl_getTextStreamEncodingName() );

        Reference< XActiveDataSource > xDataSource( m_pData->xTextOutput, UNO_QUERY_THROW );
        xDataSource->setOutputStream( getOutputStream() );
    }

    //------------------------------------------------------------------------------------------------------------------
    StorageTextOutputStream::~StorageTextOutputStream()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void StorageTextOutputStream::writeLine( const ::rtl::OUString& i_rLine )
    {
        ENSURE_OR_RETURN_VOID( m_pData->xTextOutput.is(), "no text output" );

        m_pData->xTextOutput->writeString( i_rLine );
        m_pData->xTextOutput->writeString( lcl_getLineFeed() );
    }

    //------------------------------------------------------------------------------------------------------------------
    void StorageTextOutputStream::writeLine()
    {
        ENSURE_OR_RETURN_VOID( m_pData->xTextOutput.is(), "no text output" );

        m_pData->xTextOutput->writeString( lcl_getLineFeed() );
    }

//......................................................................................................................
} // namespace dbaccess
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
