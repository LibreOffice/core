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



#ifndef INCLUDED_CONFIGMGR_SOURCE_SETNODE_HXX
#define INCLUDED_CONFIGMGR_SOURCE_SETNODE_HXX

#include "sal/config.h"

#include <vector>

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

#include "node.hxx"
#include "nodemap.hxx"

namespace configmgr {

class SetNode: public Node {
public:
    SetNode(
        int layer, rtl::OUString const & defaultTemplateName,
        rtl::OUString const & templateName);

    virtual rtl::Reference< Node > clone(bool keepTemplateName) const;

    virtual NodeMap & getMembers();

    virtual rtl::OUString getTemplateName() const;

    virtual void setMandatory(int layer);

    virtual int getMandatory() const;

    rtl::OUString const & getDefaultTemplateName() const;

    std::vector< rtl::OUString > & getAdditionalTemplateNames();

    bool isValidTemplate(rtl::OUString const & templateName) const;

private:
    SetNode(SetNode const & other, bool keepTemplateName);

    virtual ~SetNode();

    virtual Kind kind() const;

    virtual void clear();

    rtl::OUString defaultTemplateName_;
    std::vector< rtl::OUString > additionalTemplateNames_;
    NodeMap members_;
    rtl::OUString templateName_;
        // non-empty iff this node is a template, free node, or set member
    int mandatory_;
};

}

#endif
