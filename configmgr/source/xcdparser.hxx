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

#ifndef INCLUDED_CONFIGMGR_SOURCE_XCDPARSER_HXX
#define INCLUDED_CONFIGMGR_SOURCE_XCDPARSER_HXX

#include "sal/config.h"

#include <set>

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "xmlreader/xmlreader.hxx"

#include "parser.hxx"

namespace xmlreader { struct Span; }

namespace configmgr {

struct Data;

class XcdParser: public Parser {
public:
    typedef std::set< rtl::OUString > Dependencies;

    XcdParser(int layer, Dependencies const & dependencies, Data & data);

private:
    virtual ~XcdParser();

    virtual xmlreader::XmlReader::Text getTextMode();

    virtual bool startElement(
        xmlreader::XmlReader & reader, int nsId, xmlreader::Span const & name);

    virtual void endElement(xmlreader::XmlReader const & reader);

    virtual void characters(xmlreader::Span const & text);

    enum State {
        STATE_START, STATE_DEPENDENCIES, STATE_DEPENDENCY, STATE_COMPONENTS };

    int layer_;
    Dependencies const & dependencies_;
    Data & data_;
    State state_;
    rtl::OUString dependency_;
    rtl::Reference< Parser > nestedParser_;
    long nesting_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
