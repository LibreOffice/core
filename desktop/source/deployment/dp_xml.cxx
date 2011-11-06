/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    Reference<xml::input::XRoot> const & xRoot,
    ::ucbhelper::Content & ucb_content,
    Reference<XComponentContext> const & xContext )
{
    const Any arg(xRoot);
    const Reference<xml::sax::XDocumentHandler> xDocHandler(
        xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            OUSTR("com.sun.star.xml.input.SaxDocumentHandler"),
            Sequence<Any>( &arg, 1 ), xContext ), UNO_QUERY_THROW );
    xml_parse( xDocHandler, ucb_content, xContext );
 }

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
