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



#ifndef INCLUDED_CONFIGMGR_SOURCE_PROPERTYNODE_HXX
#define INCLUDED_CONFIGMGR_SOURCE_PROPERTYNODE_HXX

#include "sal/config.h"

#include "com/sun/star/uno/Any.hxx"
#include "rtl/ref.hxx"

#include "node.hxx"
#include "type.hxx"

namespace rtl { class OUString; }

namespace configmgr {

class Components;

class PropertyNode: public Node {
public:
    PropertyNode(
        int layer, Type staticType, bool nillable,
        com::sun::star::uno::Any const & value, bool extension);

    virtual rtl::Reference< Node > clone(bool keepTemplateName) const;

    Type getStaticType() const;

    bool isNillable() const;

    com::sun::star::uno::Any getValue(Components & components);

    void setValue(int layer, com::sun::star::uno::Any const & value);

    void setExternal(int layer, rtl::OUString const & descriptor);

    bool isExtension() const;

private:
    PropertyNode(PropertyNode const & other);

    virtual ~PropertyNode();

    virtual Kind kind() const;

    Type staticType_;
        // as specified in the component-schema (TYPE_ANY, ...,
        // TYPE_HEXBINARY_LIST; not TYPE_ERROR or TYPE_NIL)
    bool nillable_;
    com::sun::star::uno::Any value_;
    rtl::OUString externalDescriptor_;
    bool extension_;
};

}

#endif
