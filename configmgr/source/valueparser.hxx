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



#ifndef INCLUDED_CONFIGMGR_SOURCE_VALUEPARSER_HXX
#define INCLUDED_CONFIGMGR_SOURCE_VALUEPARSER_HXX

#include "sal/config.h"

#include <vector>

#include "boost/noncopyable.hpp"
#include "rtl/ref.hxx"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "xmlreader/pad.hxx"
#include "xmlreader/xmlreader.hxx"

#include "type.hxx"

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
} } } }
namespace xmlreader { struct Span; }

namespace configmgr {

class Node;

class ValueParser: private boost::noncopyable {
public:
    ValueParser(int layer);

    ~ValueParser();

    xmlreader::XmlReader::Text getTextMode() const;

    bool startElement(
        xmlreader::XmlReader & reader, int nsId, xmlreader::Span const & name);

    bool endElement();

    void characters(xmlreader::Span const & text);

    void start(
        rtl::Reference< Node > const & property,
        rtl::OUString const & localizedName = rtl::OUString());

    int getLayer() const;

    Type type_;
    rtl::OString separator_;

private:
    template< typename T > com::sun::star::uno::Any convertItems();

    enum State { STATE_TEXT, STATE_TEXT_UNICODE, STATE_IT, STATE_IT_UNICODE };

    int layer_;
    rtl::Reference< Node > node_;
    rtl::OUString localizedName_;
    State state_;
    xmlreader::Pad pad_;
    std::vector< com::sun::star::uno::Any > items_;
};

}

#endif
