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



#ifndef INCLUDED_CONFIGMGR_SOURCE_NODE_HXX
#define INCLUDED_CONFIGMGR_SOURCE_NODE_HXX

#include "sal/config.h"

#include "rtl/ref.hxx"
#include "salhelper/simplereferenceobject.hxx"

#include "nodemap.hxx"

namespace rtl { class OUString; }

namespace configmgr {

class Node: public salhelper::SimpleReferenceObject {
public:
    enum Kind {
        KIND_PROPERTY, KIND_LOCALIZED_PROPERTY, KIND_LOCALIZED_VALUE,
        KIND_GROUP, KIND_SET };

    virtual Kind kind() const = 0;

    virtual rtl::Reference< Node > clone(bool keepTemplateName) const = 0;

    virtual NodeMap & getMembers();

    virtual rtl::OUString getTemplateName() const;

    virtual void setMandatory(int layer);

    virtual int getMandatory() const;

    void setLayer(int layer);

    int getLayer() const;

    void setFinalized(int layer);

    int getFinalized() const;

    rtl::Reference< Node > getMember(rtl::OUString const & name);

protected:
    explicit Node(int layer);

    Node(const Node & other);

    virtual ~Node();

    virtual void clear();

    int layer_;
    int finalized_;
};

}

#endif
