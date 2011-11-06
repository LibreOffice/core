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

#include <algorithm>
#include <functional>
#include <vector>

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

#include "data.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "setnode.hxx"

namespace configmgr {

namespace {

// Work around some compilers' failure to accept
// std::binder1st(std::ptr_fun(&Data::equalTemplateNames), ...):
class EqualTemplateNames:
    public std::unary_function< rtl::OUString const &, bool >
{
public:
    inline explicit EqualTemplateNames(rtl::OUString const & shortName):
        shortName_(shortName) {}

    inline bool operator ()(rtl::OUString const & longName) const
    { return Data::equalTemplateNames(shortName_, longName); }

private:
    rtl::OUString const & shortName_;
};

}

SetNode::SetNode(
    int layer, rtl::OUString const & defaultTemplateName,
    rtl::OUString const & templateName):
    Node(layer), defaultTemplateName_(defaultTemplateName),
    templateName_(templateName), mandatory_(Data::NO_LAYER)
{}

rtl::Reference< Node > SetNode::clone(bool keepTemplateName) const {
    return new SetNode(*this, keepTemplateName);
}

NodeMap & SetNode::getMembers() {
    return members_;
}

rtl::OUString SetNode::getTemplateName() const {
    return templateName_;
}

void SetNode::setMandatory(int layer) {
    mandatory_ = layer;
}

int SetNode::getMandatory() const {
    return mandatory_;
}

rtl::OUString const & SetNode::getDefaultTemplateName() const {
    return defaultTemplateName_;
}

std::vector< rtl::OUString > & SetNode::getAdditionalTemplateNames() {
    return additionalTemplateNames_;
}

bool SetNode::isValidTemplate(rtl::OUString const & templateName) const {
    return Data::equalTemplateNames(templateName, defaultTemplateName_) ||
        (std::find_if(
            additionalTemplateNames_.begin(),
            additionalTemplateNames_.end(), EqualTemplateNames(templateName)) !=
         additionalTemplateNames_.end());
}

SetNode::SetNode(SetNode const & other, bool keepTemplateName):
    Node(other), defaultTemplateName_(other.defaultTemplateName_),
    additionalTemplateNames_(other.additionalTemplateNames_),
    mandatory_(other.mandatory_)
{
    cloneNodeMap(other.members_, &members_);
    if (keepTemplateName) {
        templateName_ = other.templateName_;
    }
}

SetNode::~SetNode() {}

Node::Kind SetNode::kind() const {
    return KIND_SET;
}

void SetNode::clear() {
    members_.clear();
}

}
