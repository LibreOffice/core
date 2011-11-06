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



#ifndef INCLUDED_CONFIGMGR_SOURCE_XCSPARSER_HXX
#define INCLUDED_CONFIGMGR_SOURCE_XCSPARSER_HXX

#include "sal/config.h"

#include <stack>

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "xmlreader/xmlreader.hxx"

#include "node.hxx"
#include "parser.hxx"
#include "valueparser.hxx"

namespace xmlreader { struct Span; }

namespace configmgr {

class SetNode;
struct Data;

class XcsParser: public Parser {
public:
    XcsParser(int layer, Data & data);

private:
    virtual ~XcsParser();

    virtual xmlreader::XmlReader::Text getTextMode();

    virtual bool startElement(
        xmlreader::XmlReader & reader, int nsId, xmlreader::Span const & name);

    virtual void endElement(xmlreader::XmlReader const & reader);

    virtual void characters(xmlreader::Span const & text);

    void handleComponentSchema(xmlreader::XmlReader & reader);

    void handleNodeRef(xmlreader::XmlReader & reader);

    void handleProp(xmlreader::XmlReader & reader);

    void handlePropValue(
        xmlreader::XmlReader & reader, rtl::Reference< Node > const & property);

    void handleGroup(xmlreader::XmlReader & reader, bool isTemplate);

    void handleSet(xmlreader::XmlReader & reader, bool isTemplate);

    void handleSetItem(xmlreader::XmlReader & reader, SetNode * set);

    enum State {
        STATE_START, STATE_COMPONENT_SCHEMA, STATE_TEMPLATES,
        STATE_TEMPLATES_DONE, STATE_COMPONENT, STATE_COMPONENT_DONE };

    struct Element {
        rtl::Reference< Node > node;
        rtl::OUString name;

        inline Element(
            rtl::Reference< Node > const & theNode,
            rtl::OUString const & theName):
            node(theNode), name(theName) {}
    };

    typedef std::stack< Element > ElementStack;

    ValueParser valueParser_;
    Data & data_;
    rtl::OUString componentName_;
    State state_;
    long ignoring_;
    ElementStack elements_;
};

}

#endif
