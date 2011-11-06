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



#ifndef INCLUDED_idlc_inc_idlc_inheritedinterface_hxx
#define INCLUDED_idlc_inc_idlc_inheritedinterface_hxx

#include "idlc/idlc.hxx"

#include "rtl/ustring.hxx"

class AstInterface;
class AstType;

class InheritedInterface {
public:
    InheritedInterface(
        AstType const * theInterface, bool theOptional,
        rtl::OUString const & theDocumentation):
        interface(theInterface), optional(theOptional),
        documentation(theDocumentation) {}

    AstType const * getInterface() const { return interface; }

    AstInterface const * getResolved() const
    { return resolveInterfaceTypedefs(interface); }

    bool isOptional() const { return optional; }

    rtl::OUString getDocumentation() const { return documentation; }

private:
    AstType const * interface;
    bool optional;
    rtl::OUString documentation;
};

#endif
