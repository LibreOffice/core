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
#include "precompiled_desktop.hxx"

#include "dp_misc.h"
#include "dp_xml.h"
#include "rtl/ustrbuf.hxx"
#include "ucbhelper/content.hxx"
#include "com/sun/star/xml/sax/XParser.hpp"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace dp_misc
{

//==============================================================================
void xml_parse(
    Reference<xml::sax::XDocumentHandler> const & xDocHandler,
    ::ucbhelper::Content & ucb_content,
    Reference<XComponentContext> const & xContext )
{
    // raise sax parser:
    Reference<xml::sax::XParser> xParser(
        xContext->getServiceManager()->createInstanceWithContext(
            OUSTR("com.sun.star.xml.sax.Parser"), xContext ), UNO_QUERY_THROW );

    // error handler, entity resolver omitted
    xParser->setDocumentHandler( xDocHandler );
    xml::sax::InputSource source;
    source.aInputStream = ucb_content.openStream();
    source.sSystemId = ucb_content.getURL();
    xParser->parseStream( source );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
