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



#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "osl/diagnose.h"
#include "sal/types.h"
#include "xmlreader/span.hxx"
#include "xmlreader/xmlreader.hxx"

#include "parsemanager.hxx"
#include "parser.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

}

ParseManager::ParseManager(
    rtl::OUString const & url, rtl::Reference< Parser > const & parser)
    SAL_THROW((
        css::container::NoSuchElementException, css::uno::RuntimeException)):
    reader_(url), parser_(parser)
{
    OSL_ASSERT(parser.is());
    int id;
    id = reader_.registerNamespaceIri(
        xmlreader::Span(
            RTL_CONSTASCII_STRINGPARAM("http://openoffice.org/2001/registry")));
    OSL_ASSERT(id == NAMESPACE_OOR);
    id = reader_.registerNamespaceIri(
        xmlreader::Span(
            RTL_CONSTASCII_STRINGPARAM("http://www.w3.org/2001/XMLSchema")));
    OSL_ASSERT(id == NAMESPACE_XS);
    id = reader_.registerNamespaceIri(
        xmlreader::Span(
            RTL_CONSTASCII_STRINGPARAM(
                "http://www.w3.org/2001/XMLSchema-instance")));
    OSL_ASSERT(id == NAMESPACE_XSI);
}

bool ParseManager::parse() {
    for (;;) {
        switch (itemData_.is()
                ? xmlreader::XmlReader::RESULT_BEGIN
                : reader_.nextItem(
                    parser_->getTextMode(), &itemData_, &itemNamespaceId_))
        {
        case xmlreader::XmlReader::RESULT_BEGIN:
            if (!parser_->startElement(reader_, itemNamespaceId_, itemData_))
            {
                return false;
            }
            break;
        case xmlreader::XmlReader::RESULT_END:
            parser_->endElement(reader_);
            break;
        case xmlreader::XmlReader::RESULT_TEXT:
            parser_->characters(itemData_);
            break;
        case xmlreader::XmlReader::RESULT_DONE:
            return true;
        }
        itemData_.clear();
    }
}

ParseManager::~ParseManager() {}

}
