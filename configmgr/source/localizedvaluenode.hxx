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



#ifndef INCLUDED_CONFIGMGR_SOURCE_LOCALIZEDVALUENODE_HXX
#define INCLUDED_CONFIGMGR_SOURCE_LOCALIZEDVALUENODE_HXX

#include "sal/config.h"

#include "com/sun/star/uno/Any.hxx"
#include "rtl/ref.hxx"

#include "node.hxx"

namespace rtl { class OUString; }

namespace configmgr {

class LocalizedValueNode: public Node {
public:
    LocalizedValueNode(int layer, com::sun::star::uno::Any const & value);

    virtual rtl::Reference< Node > clone(bool keepTemplateName) const;

    virtual rtl::OUString getTemplateName() const;

    com::sun::star::uno::Any getValue() const;

    void setValue(int layer, com::sun::star::uno::Any const & value);

private:
    LocalizedValueNode(LocalizedValueNode const & other);

    virtual ~LocalizedValueNode();

    virtual Kind kind() const;

    com::sun::star::uno::Any value_;
};

}

#endif
