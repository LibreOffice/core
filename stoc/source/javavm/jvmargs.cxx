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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_stoc.hxx"

#include "jvmargs.hxx"
#include <rtl/ustring.hxx>


#define OUSTR(x) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( x ))

using namespace rtl;

namespace stoc_javavm {

JVM::JVM() throw()//: _enabled(sal_False)
{
}

void JVM::pushProp(const OUString & property)
{
    sal_Int32 index = property.indexOf((sal_Unicode)'=');
    if(index > 0)
    {
        OUString left = property.copy(0, index).trim();
        OUString right(property.copy(index + 1).trim());
        _props.push_back(property);
    }
    else
    { // no '=', could be -X
        _props.push_back(property);
    }
}


const ::std::vector< ::rtl::OUString > & JVM::getProperties() const
{
    return _props;
}

}
