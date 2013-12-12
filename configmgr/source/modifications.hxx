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



#ifndef INCLUDED_CONFIGMGR_SOURCE_MODIFICATIONS_HXX
#define INCLUDED_CONFIGMGR_SOURCE_MODIFICATIONS_HXX

#include "sal/config.h"

#include <boost/unordered_map.hpp> // using the boost container because it explicitly allows recursive types

#include "boost/noncopyable.hpp"

#include "path.hxx"
#include "rtl/ustring.hxx"

namespace configmgr {

class Modifications: private boost::noncopyable {
public:
    struct Node {
        typedef boost::unordered_map< rtl::OUString, Node, rtl::OUStringHash > Children;

        Children children;
    };

    Modifications();

    ~Modifications();

    void add(Path const & path);

    void remove(Path const & path);

    Node const & getRoot() const;

private:
    Node root_;
};

}

#endif
