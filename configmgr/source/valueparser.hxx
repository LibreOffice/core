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

#pragma once

#include <sal/config.h>

#include <vector>

#include <rtl/ref.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <xmlreader/pad.hxx>
#include <xmlreader/xmlreader.hxx>

#include "type.hxx"

namespace com::sun::star::uno {
    class Any;
}
namespace xmlreader { struct Span; }

namespace configmgr {

class Node;

class ValueParser {
public:
    explicit ValueParser(int layer);

    ~ValueParser();

    xmlreader::XmlReader::Text getTextMode() const;

    bool startElement(
        xmlreader::XmlReader & reader, int nsId, xmlreader::Span const & name);

    bool endElement();

    void characters(xmlreader::Span const & text);

    void start(
        rtl::Reference< Node > const & property,
        OUString const & localizedName = OUString());

    int getLayer() const { return layer_;}

    Type type_;
    OString separator_;

private:
    ValueParser(const ValueParser&) = delete;
    ValueParser& operator=(const ValueParser&) = delete;

    template< typename T > css::uno::Any convertItems();

    enum class State { Text, TextUnicode, IT, ITUnicode };

    int layer_;
    rtl::Reference< Node > node_;
    OUString localizedName_;
    State state_;
    xmlreader::Pad pad_;
    std::vector< css::uno::Any > items_;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
