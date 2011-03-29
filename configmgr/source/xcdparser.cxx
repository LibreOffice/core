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

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include <climits>

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "xmlreader/span.hxx"
#include "xmlreader/xmlreader.hxx"

#include "parsemanager.hxx"
#include "xcdparser.hxx"
#include "xcsparser.hxx"
#include "xcuparser.hxx"
#include "xmldata.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

}

XcdParser::XcdParser(int layer, Dependencies const & dependencies, Data & data):
    layer_(layer), dependencies_(dependencies), data_(data), state_(STATE_START)
{}

XcdParser::~XcdParser() {}

xmlreader::XmlReader::Text XcdParser::getTextMode() {
    return nestedParser_.is()
        ? nestedParser_->getTextMode() : xmlreader::XmlReader::TEXT_NONE;
}

bool XcdParser::startElement(
    xmlreader::XmlReader & reader, int nsId, xmlreader::Span const & name)
{
    if (nestedParser_.is()) {
        OSL_ASSERT(nesting_ != LONG_MAX);
        ++nesting_;
        return nestedParser_->startElement(reader, nsId, name);
    }
    switch (state_) {
    case STATE_START:
        if (nsId == ParseManager::NAMESPACE_OOR &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("data")))
        {
            state_ = STATE_DEPENDENCIES;
            return true;
        }
        break;
    case STATE_DEPENDENCIES:
        if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("dependency")))
        {
            if (dependency_.getLength() == 0) {
                xmlreader::Span attrFile;
                for (;;) {
                    int attrNsId;
                    xmlreader::Span attrLn;
                    if (!reader.nextAttribute(&attrNsId, &attrLn)) {
                        break;
                    }
                    if (attrNsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                            //TODO: _OOR
                        attrLn.equals(RTL_CONSTASCII_STRINGPARAM("file")))
                    {
                        attrFile = reader.getAttributeValue(false);
                    }
                }
                if (!attrFile.is()) {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "no dependency file attribute in ")) +
                         reader.getUrl()),
                        css::uno::Reference< css::uno::XInterface >());
                }
                dependency_ = attrFile.convertFromUtf8();
                if (dependency_.getLength() == 0) {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "bad dependency file attribute in ")) +
                         reader.getUrl()),
                        css::uno::Reference< css::uno::XInterface >());
                }
            }
            if (dependencies_.find(dependency_) == dependencies_.end()) {
                return false;
            }
            state_ = STATE_DEPENDENCY;
            dependency_ = rtl::OUString();
            return true;
        }
        state_ = STATE_COMPONENTS;
        // fall through
    case STATE_COMPONENTS:
        if (nsId == ParseManager::NAMESPACE_OOR &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("component-schema")))
        {
            nestedParser_ = new XcsParser(layer_, data_);
            nesting_ = 1;
            return nestedParser_->startElement(reader, nsId, name);
        }
        if (nsId == ParseManager::NAMESPACE_OOR &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("component-data")))
        {
            nestedParser_ = new XcuParser(layer_ + 1, data_, 0, 0, 0);
            nesting_ = 1;
            return nestedParser_->startElement(reader, nsId, name);
        }
        break;
    default: // STATE_DEPENDENCY
        OSL_ASSERT(false); // this cannot happen
        break;
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad member <")) +
         name.convertFromUtf8() +
         rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) + reader.getUrl()),
        css::uno::Reference< css::uno::XInterface >());
}

void XcdParser::endElement(xmlreader::XmlReader const & reader) {
    if (nestedParser_.is()) {
        nestedParser_->endElement(reader);
        if (--nesting_ == 0) {
            nestedParser_.clear();
        }
    } else {
        switch (state_) {
        case STATE_DEPENDENCY:
            state_ = STATE_DEPENDENCIES;
            break;
        case STATE_DEPENDENCIES:
        case STATE_COMPONENTS:
            break;
        default:
            OSL_ASSERT(false); // this cannot happen
            break;
        }
    }
}

void XcdParser::characters(xmlreader::Span const & text) {
    if (nestedParser_.is()) {
        nestedParser_->characters(text);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
