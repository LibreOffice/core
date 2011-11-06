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



#ifndef INCLUDED_CONFIGMGR_SOURCE_GROUPNODE_HXX
#define INCLUDED_CONFIGMGR_SOURCE_GROUPNODE_HXX

#include "sal/config.h"

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

#include "node.hxx"
#include "nodemap.hxx"

namespace configmgr {

class GroupNode: public Node {
public:
    GroupNode(int layer, bool extensible, rtl::OUString const & templateName);

    virtual rtl::Reference< Node > clone(bool keepTemplateName) const;

    virtual NodeMap & getMembers();

    virtual rtl::OUString getTemplateName() const;

    virtual void setMandatory(int layer);

    virtual int getMandatory() const;

    bool isExtensible() const;

private:
    GroupNode(GroupNode const & other, bool keepTemplateName);

    virtual ~GroupNode();

    virtual Kind kind() const;

    virtual void clear();

    bool extensible_;
    NodeMap members_;
    rtl::OUString templateName_;
        // non-empty iff this node is a template, free node, or set member
    int mandatory_;
};

}

#endif
