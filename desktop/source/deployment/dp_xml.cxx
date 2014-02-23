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


#include "dp_misc.h"
#include "dp_xml.h"
#include "rtl/ustrbuf.hxx"
#include "ucbhelper/content.hxx"
#include "com/sun/star/xml/sax/Parser.hpp"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace dp_misc
{


void xml_parse(
    Reference<xml::sax::XDocumentHandler> const & xDocHandler,
    ::ucbhelper::Content & ucb_content,
    Reference<XComponentContext> const & xContext )
{
    // raise sax parser:
    Reference<xml::sax::XParser> xParser = xml::sax::Parser::create(xContext);

    // error handler, entity resolver omitted
    xParser->setDocumentHandler( xDocHandler );
    xml::sax::InputSource source;
    source.aInputStream = ucb_content.openStream();
    source.sSystemId = ucb_content.getURL();
    xParser->parseStream( source );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
