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
#include "dp_parceldesc.hxx"



using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;

namespace css = ::com::sun::star;
namespace dp_registry
{
namespace backend
{
namespace sfwk
{


// XDocumentHandler
void SAL_CALL
ParcelDescDocHandler::startDocument()
throw ( xml::sax::SAXException, RuntimeException )
{
    m_bIsParsed = false;
}

void SAL_CALL
ParcelDescDocHandler::endDocument()
throw ( xml::sax::SAXException, RuntimeException )
{
    m_bIsParsed = true;
}

void SAL_CALL
ParcelDescDocHandler::characters( const OUString & )
    throw ( xml::sax::SAXException, RuntimeException )
{
}

void SAL_CALL
ParcelDescDocHandler::ignorableWhitespace( const OUString & )
    throw ( xml::sax::SAXException, RuntimeException )
{
}

void SAL_CALL
ParcelDescDocHandler::processingInstruction(
    const OUString &, const OUString & )
        throw ( xml::sax::SAXException, RuntimeException )
{
}

void SAL_CALL
ParcelDescDocHandler::setDocumentLocator(
    const Reference< xml::sax::XLocator >& )
        throw ( xml::sax::SAXException, RuntimeException )
{
}

void SAL_CALL
ParcelDescDocHandler::startElement( const OUString& aName,
    const Reference< xml::sax::XAttributeList > & xAttribs )
        throw ( xml::sax::SAXException,
            RuntimeException )
{

    dp_misc::TRACE(OUSTR("ParcelDescDocHandler::startElement() for ") +
        aName + OUSTR("\n"));
    if ( !skipIndex )
    {
        if ( aName.equals( OUString::createFromAscii( "parcel" ) ) )
        {
            m_sLang = xAttribs->getValueByName( OUString::createFromAscii( "language" ) );
        }
        ++skipIndex;
    }
    else
    {
        dp_misc::TRACE(OUSTR("ParcelDescDocHandler::startElement() skipping for ")
            + aName + OUSTR("\n"));
    }

}

void SAL_CALL ParcelDescDocHandler::endElement( const OUString & aName )
   throw ( xml::sax::SAXException, RuntimeException )
{
    if ( skipIndex )
    {
        --skipIndex;
        dp_misc::TRACE(OUSTR("ParcelDescDocHandler::endElement() skipping for ")
            + aName + OUSTR("\n"));
    }
}


}
}
}

