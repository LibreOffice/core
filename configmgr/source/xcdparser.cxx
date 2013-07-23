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

#include "sal/config.h"

#include <cassert>
#include <climits>
#include <set>

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "rtl/ustring.hxx"
#include "xmlreader/span.hxx"
#include "xmlreader/xmlreader.hxx"

#include "parsemanager.hxx"
#include "xcdparser.hxx"
#include "xcsparser.hxx"
#include "xcuparser.hxx"
#include "xmldata.hxx"

namespace configmgr {

XcdParser::XcdParser(
    int layer, std::set< OUString > const & processedDependencies, Data & data):
    layer_(layer), processedDependencies_(processedDependencies), data_(data),
    state_(STATE_START), dependencyOptional_(), nesting_()
{}

XcdParser::~XcdParser() {}

xmlreader::XmlReader::Text XcdParser::getTextMode() {
    return nestedParser_.is()
        ? nestedParser_->getTextMode() : xmlreader::XmlReader::TEXT_NONE;
}

bool XcdParser::startElement(
    xmlreader::XmlReader & reader, int nsId, xmlreader::Span const & name,
    std::set< OUString > const * existingDependencies)
{
    if (nestedParser_.is()) {
        assert(nesting_ != LONG_MAX);
        ++nesting_;
        return nestedParser_->startElement(
            reader, nsId, name, existingDependencies);
    }
    switch (state_) {
    case STATE_START:
        if (nsId == ParseManager::NAMESPACE_OOR && name.equals("data")) {
            state_ = STATE_DEPENDENCIES;
            return true;
        }
        break;
    case STATE_DEPENDENCIES:
        if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
            name.equals("dependency"))
        {
            if (dependencyFile_.isEmpty()) {
                dependencyOptional_ = false;
                xmlreader::Span attrFile;
                for (;;) {
                    int attrNsId;
                    xmlreader::Span attrLn;
                    if (!reader.nextAttribute(&attrNsId, &attrLn)) {
                        break;
                    }
                    if (attrNsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                            //TODO: _OOR
                        attrLn.equals("file"))
                    {
                        attrFile = reader.getAttributeValue(false);
                    } else if ((attrNsId ==
                                xmlreader::XmlReader::NAMESPACE_NONE) &&
                               attrLn.equals("optional"))
                    {
                        dependencyOptional_ = xmldata::parseBoolean(
                            reader.getAttributeValue(true));
                    }
                }
                if (!attrFile.is()) {
                    throw css::uno::RuntimeException(
                        "no dependency file attribute in " + reader.getUrl(),
                        css::uno::Reference< css::uno::XInterface >());
                }
                dependencyFile_ = attrFile.convertFromUtf8();
                if (dependencyFile_.isEmpty()) {
                    throw css::uno::RuntimeException(
                        "bad dependency file attribute in " + reader.getUrl(),
                        css::uno::Reference< css::uno::XInterface >());
                }
            }
            if ((processedDependencies_.find(dependencyFile_) ==
                 processedDependencies_.end()) &&
                (!dependencyOptional_ || existingDependencies == 0 ||
                 (existingDependencies->find(dependencyFile_) !=
                  existingDependencies->end())))
            {
                return false;
            }
            state_ = STATE_DEPENDENCY;
            dependencyFile_ = "";
            return true;
        }
        state_ = STATE_COMPONENTS;
        // fall through
    case STATE_COMPONENTS:
        if (nsId == ParseManager::NAMESPACE_OOR &&
            name.equals("component-schema"))
        {
            nestedParser_ = new XcsParser(layer_, data_);
            nesting_ = 1;
            return nestedParser_->startElement(
                reader, nsId, name, existingDependencies);
        }
        if (nsId == ParseManager::NAMESPACE_OOR &&
            name.equals("component-data"))
        {
            nestedParser_ = new XcuParser(layer_ + 1, data_, 0, 0, 0);
            nesting_ = 1;
            return nestedParser_->startElement(
                reader, nsId, name, existingDependencies);
        }
        break;
    default: // STATE_DEPENDENCY
        assert(false); // this cannot happen
        break;
    }
    throw css::uno::RuntimeException(
        "bad member <" + name.convertFromUtf8() + "> in " + reader.getUrl(),
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
            assert(false); // this cannot happen
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
