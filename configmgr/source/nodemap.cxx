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

#include "osl/diagnose.h"
#include "rtl/ustring.hxx"

#include "node.hxx"
#include "nodemap.hxx"

namespace configmgr {

void cloneNodeMap(NodeMap const & source, NodeMap * target) {
    OSL_ASSERT(target != 0 && target->empty());
    NodeMap clone(source);
    for (NodeMap::iterator i(clone.begin()); i != clone.end(); ++i) {
        i->second = i->second->clone(true);
    }
    std::swap(clone, *target);
}

}
