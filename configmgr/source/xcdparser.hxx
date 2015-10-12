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

#ifndef INCLUDED_CONFIGMGR_SOURCE_XCDPARSER_HXX
#define INCLUDED_CONFIGMGR_SOURCE_XCDPARSER_HXX

#include <sal/config.h>

#include <set>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <xmlreader/xmlreader.hxx>

#include "parser.hxx"

namespace xmlreader { struct Span; }

namespace configmgr {

struct Data;

class XcdParser: public Parser {
public:
    XcdParser(
        int layer, std::set< OUString > const & processedDependencies,
        Data & data);

private:
    virtual ~XcdParser();

    virtual xmlreader::XmlReader::Text getTextMode() override;

    virtual bool startElement(
        xmlreader::XmlReader & reader, int nsId, xmlreader::Span const & name,
        std::set< OUString > const * existingDependencies) override;

    virtual void endElement(xmlreader::XmlReader const & reader) override;

    virtual void characters(xmlreader::Span const & text) override;

    enum State {
        STATE_START, STATE_DEPENDENCIES, STATE_DEPENDENCY, STATE_COMPONENTS };

    int layer_;
    std::set< OUString > const & processedDependencies_;
    Data & data_;
    State state_;
    OUString dependencyFile_;
    bool dependencyOptional_;
    rtl::Reference< Parser > nestedParser_;
    long nesting_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
