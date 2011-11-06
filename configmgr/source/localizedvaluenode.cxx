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

#include "com/sun/star/uno/Any.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include "localizedvaluenode.hxx"
#include "node.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

}

LocalizedValueNode::LocalizedValueNode(int layer, css::uno::Any const & value):
    Node(layer), value_(value)
{}

rtl::Reference< Node > LocalizedValueNode::clone(bool) const {
    return new LocalizedValueNode(*this);
}

rtl::OUString LocalizedValueNode::getTemplateName() const {
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("*"));
}

css::uno::Any LocalizedValueNode::getValue() const {
    return value_;
}

void LocalizedValueNode::setValue(int layer, css::uno::Any const & value) {
    setLayer(layer);
    value_ = value;
}

LocalizedValueNode::LocalizedValueNode(LocalizedValueNode const & other):
    Node(other), value_(other.value_)
{}

LocalizedValueNode::~LocalizedValueNode() {}

Node::Kind LocalizedValueNode::kind() const {
    return KIND_LOCALIZED_VALUE;
}

}
