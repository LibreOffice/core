/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
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

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "osl/diagnose.h"
#include "sal/types.h"

#include "parsemanager.hxx"
#include "parser.hxx"
#include "xmlreader.hxx"

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
}

bool ParseManager::parse() {
    for (;;) {
        switch (itemData_.is()
                ? XmlReader::RESULT_BEGIN
                : reader_.nextItem(
                    parser_->getTextMode(), &itemData_, &itemNamespace_))
        {
        case XmlReader::RESULT_BEGIN:
            if (!parser_->startElement(reader_, itemNamespace_, itemData_))
            {
                return false;
            }
            break;
        case XmlReader::RESULT_END:
            parser_->endElement(reader_);
            break;
        case XmlReader::RESULT_TEXT:
            parser_->characters(itemData_);
            break;
        case XmlReader::RESULT_DONE:
            return true;
        }
        itemData_.clear();
    }
}

ParseManager::~ParseManager() {}

}
