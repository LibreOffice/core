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

#include <cppuhelper/implbase.hxx>
#include <xmlscript/xml_helper.hxx>
#include <xmlscript/xmldlg_imexp.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;

namespace xmlscript
{

class InputStreamProvider
    : public ::cppu::WeakImplHelper< io::XInputStreamProvider >
{
    std::vector<sal_Int8> const _bytes;

public:
    explicit InputStreamProvider( std::vector<sal_Int8> const & rBytes )
        : _bytes( rBytes )
    {
    }

    // XInputStreamProvider
    virtual uno::Reference< io::XInputStream > SAL_CALL createInputStream() override;
};
uno::Reference< io::XInputStream > InputStreamProvider::createInputStream()
{
    return ::xmlscript::createInputStream( _bytes );
}

uno::Reference< io::XInputStreamProvider > exportDialogModel(
    uno::Reference< container::XNameContainer > const & xDialogModel,
    uno::Reference< uno::XComponentContext > const & xContext,
    uno::Reference< XModel > const & xDocument )
{
    uno::Reference< xml::sax::XWriter > xWriter = xml::sax::Writer::create(xContext);

    std::vector<sal_Int8> aBytes;
    xWriter->setOutputStream( createOutputStream( &aBytes ) );

    uno::Reference< xml::sax::XExtendedDocumentHandler > xHandler(xWriter, uno::UNO_QUERY_THROW);
    exportDialogModel( xHandler, xDialogModel, xDocument );

    return new InputStreamProvider( aBytes );
}

void importDialogModel(
    uno::Reference< io::XInputStream > const & xInput,
    uno::Reference< container::XNameContainer > const & xDialogModel,
    uno::Reference< uno::XComponentContext > const & xContext,
    uno::Reference< XModel > const & xDocument )
{
    uno::Reference< xml::sax::XParser > xParser = xml::sax::Parser::create( xContext );

    // error handler, entity resolver omitted for this helper function
    xParser->setDocumentHandler( importDialogModel( xDialogModel, xContext, xDocument ) );

    xml::sax::InputSource source;
    source.aInputStream = xInput;
    source.sSystemId = "virtual file";

    xParser->parseStream( source );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
