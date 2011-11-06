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

#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "osl/diagnose.h"
#include "rtl/ref.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include "components.hxx"
#include "node.hxx"
#include "propertynode.hxx"
#include "type.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

}

PropertyNode::PropertyNode(
    int layer, Type staticType, bool nillable, css::uno::Any const & value,
    bool extension):
    Node(layer), staticType_(staticType), nillable_(nillable), value_(value),
    extension_(extension)
{}

rtl::Reference< Node > PropertyNode::clone(bool) const {
    return new PropertyNode(*this);
}

Type PropertyNode::getStaticType() const {
    return staticType_;
}

bool PropertyNode::isNillable() const {
    return nillable_;
}

css::uno::Any PropertyNode::getValue(Components & components) {
    if (externalDescriptor_.getLength() != 0) {
        css::beans::Optional< css::uno::Any > val(
            components.getExternalValue(externalDescriptor_));
        if (val.IsPresent) {
            value_ = val.Value; //TODO: check value type
        }
        externalDescriptor_ = rtl::OUString(); // must not throw
    }
    return value_;
}

void PropertyNode::setValue(int layer, css::uno::Any const & value) {
    setLayer(layer);
    value_ = value;
    externalDescriptor_ = rtl::OUString();
}

void PropertyNode::setExternal(int layer, rtl::OUString const & descriptor) {
    OSL_ASSERT(descriptor.getLength() != 0);
    setLayer(layer);
    externalDescriptor_ = descriptor;
}

bool PropertyNode::isExtension() const {
    return extension_;
}

PropertyNode::PropertyNode(PropertyNode const & other):
    Node(other), staticType_(other.staticType_), nillable_(other.nillable_),
    value_(other.value_), externalDescriptor_(other.externalDescriptor_),
    extension_(other.extension_)
{}

PropertyNode::~PropertyNode() {}

Node::Kind PropertyNode::kind() const {
    return KIND_PROPERTY;
}

}
