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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlscript.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <xmlscript/xml_helper.hxx>
#include <xmlscript/xmldlg_imexp.hxx>


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

namespace xmlscript
{

//==================================================================================================
class InputStreamProvider
    : public ::cppu::WeakImplHelper1< io::XInputStreamProvider >
{
    ByteSequence _bytes;

public:
    inline InputStreamProvider( ByteSequence const & rBytes )
        : _bytes( rBytes )
        {}

    // XInputStreamProvider
    virtual Reference< io::XInputStream > SAL_CALL createInputStream()
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
Reference< io::XInputStream > InputStreamProvider::createInputStream()
    throw (RuntimeException)
{
    return ::xmlscript::createInputStream( _bytes );
}

//==================================================================================================
Reference< io::XInputStreamProvider > SAL_CALL exportDialogModel(
    Reference< container::XNameContainer > const & xDialogModel,
    Reference< XComponentContext > const & xContext,
    Reference< XModel > const & xDocument )
    SAL_THROW( (Exception) )
{
    Reference< lang::XMultiComponentFactory > xSMgr( xContext->getServiceManager() );
    if (! xSMgr.is())
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no service manager available!") ),
            Reference< XInterface >() );
    }

    Reference< xml::sax::XExtendedDocumentHandler > xHandler( xSMgr->createInstanceWithContext(
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer") ), xContext ), UNO_QUERY );
    OSL_ASSERT( xHandler.is() );
    if (! xHandler.is())
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("could not create sax-writer component!") ),
            Reference< XInterface >() );
    }

    ByteSequence aBytes;

    Reference< io::XActiveDataSource > xSource( xHandler, UNO_QUERY );
    xSource->setOutputStream( createOutputStream( &aBytes ) );
    exportDialogModel( xHandler, xDialogModel, xDocument );

    return new InputStreamProvider( aBytes );
}

//==================================================================================================
void SAL_CALL importDialogModel(
    Reference< io::XInputStream > const & xInput,
    Reference< container::XNameContainer > const & xDialogModel,
    Reference< XComponentContext > const & xContext,
    Reference< XModel > const & xDocument )
    SAL_THROW( (Exception) )
{
    Reference< lang::XMultiComponentFactory > xSMgr( xContext->getServiceManager() );
    if (! xSMgr.is())
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no service manager available!") ),
            Reference< XInterface >() );
    }

    Reference< xml::sax::XParser > xParser( xSMgr->createInstanceWithContext(
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser") ), xContext ), UNO_QUERY );
    OSL_ASSERT( xParser.is() );
    if (! xParser.is())
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("could not create sax-parser component!") ),
            Reference< XInterface >() );
    }

    // error handler, entity resolver omitted for this helper function
    xParser->setDocumentHandler( importDialogModel( xDialogModel, xContext, xDocument ) );

    xml::sax::InputSource source;
    source.aInputStream = xInput;
    source.sSystemId = OUString( RTL_CONSTASCII_USTRINGPARAM("virtual file") );

    xParser->parseStream( source );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
