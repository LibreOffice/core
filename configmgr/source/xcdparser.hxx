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
