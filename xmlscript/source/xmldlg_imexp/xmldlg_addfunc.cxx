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


#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>

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
    Reference< xml::sax::XWriter > xWriter = xml::sax::Writer::create(xContext);

    ByteSequence aBytes;
    xWriter->setOutputStream( createOutputStream( &aBytes ) );

    Reference< xml::sax::XExtendedDocumentHandler > xHandler(xWriter, UNO_QUERY_THROW);
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
    Reference< xml::sax::XParser > xParser = xml::sax::Parser::create( xContext );

    // error handler, entity resolver omitted for this helper function
    xParser->setDocumentHandler( importDialogModel( xDialogModel, xContext, xDocument ) );

    xml::sax::InputSource source;
    source.aInputStream = xInput;
    source.sSystemId = "virtual file";

    xParser->parseStream( source );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
