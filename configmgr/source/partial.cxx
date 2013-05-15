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

#include <map>
#include <set>

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "data.hxx"
#include "partial.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

bool parseSegment(
    rtl::OUString const & path, sal_Int32 * index, rtl::OUString * segment)
{
    OSL_ASSERT(
        index != 0 && *index >= 0 && *index <= path.getLength() &&
        segment != 0);
    if (path[(*index)++] == '/') {
        rtl::OUString name;
        bool setElement;
        rtl::OUString templateName;
        *index = Data::parseSegment(
            path, *index, &name, &setElement, &templateName);
        if (*index != -1) {
            *segment = Data::createSegment(templateName, name);
            return *index == path.getLength();
        }
    }
    throw css::uno::RuntimeException(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) + path,
        css::uno::Reference< css::uno::XInterface >());
}

}

Partial::Partial(
    std::set< rtl::OUString > const & includedPaths,
    std::set< rtl::OUString > const & excludedPaths)
{
    for (std::set< rtl::OUString >::const_iterator i(includedPaths.begin());
         i != includedPaths.end(); ++i)
    {
        sal_Int32 n = 0;
        for (Node * p = &root_;;) {
            rtl::OUString seg;
            bool end = parseSegment(*i, &n, &seg);
            p = &p->children[seg];
            if (p->startInclude) {
                break;
            }
            if (end) {
                p->children.clear();
                p->startInclude = true;
                break;
            }
        }
    }
    for (std::set< rtl::OUString >::const_iterator i(excludedPaths.begin());
         i != excludedPaths.end(); ++i)
    {
        sal_Int32 n = 0;
        for (Node * p = &root_;;) {
            rtl::OUString seg;
            bool end = parseSegment(*i, &n, &seg);
            if (end) {
                p->children[seg].clear();
                break;
            }
            Node::Children::iterator j(p->children.find(seg));
            if (j == p->children.end()) {
                break;
            }
            p = &j->second;
        }
    }
}

Partial::~Partial() {}

Partial::Containment Partial::contains(Path const & path) const {
    //TODO: For set elements, the segment names recorded in the node tree need
    // not match the corresponding path segments, so this function can fail.
    Node const * p = &root_;
    bool includes = false;
    for (Path::const_iterator i(path.begin()); i != path.end(); ++i) {
        Node::Children::const_iterator j(p->children.find(*i));
        if (j == p->children.end()) {
            break;
        }
        p = &j->second;
        includes |= p->startInclude;
    }
    return p->children.empty() && !p->startInclude
        ? CONTAINS_NOT
        : includes ? CONTAINS_NODE : CONTAINS_SUBNODES;
}

}
