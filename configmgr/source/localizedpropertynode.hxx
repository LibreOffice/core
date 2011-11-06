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



#ifndef INCLUDED_CONFIGMGR_SOURCE_LOCALIZEDPROPERTYNODE_HXX
#define INCLUDED_CONFIGMGR_SOURCE_LOCALIZEDPROPERTYNODE_HXX

#include "sal/config.h"

#include "rtl/ref.hxx"

#include "node.hxx"
#include "nodemap.hxx"
#include "type.hxx"

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
} } } }
namespace rtl { class OUString; }

namespace configmgr {

class LocalizedPropertyNode: public Node {
public:
    LocalizedPropertyNode(int layer, Type staticType, bool nillable);

    virtual rtl::Reference< Node > clone(bool keepTemplateName) const;

    virtual NodeMap & getMembers();

    Type getStaticType() const;

    bool isNillable() const;

private:
    LocalizedPropertyNode(LocalizedPropertyNode const & other);

    virtual ~LocalizedPropertyNode();

    virtual Kind kind() const;

    virtual void clear();

    Type staticType_;
        // as specified in the component-schema (TYPE_ANY, ...,
        // TYPE_HEXBINARY_LIST; not TYPE_ERROR or TYPE_NIL)
    bool nillable_;
    NodeMap members_;
};

}

#endif
