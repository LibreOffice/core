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



#ifndef INCLUDED_REF_AND_POINTER_HXX
#define INCLUDED_REF_AND_POINTER_HXX

#include <iostream>
#include <com/sun/star/lang/XUnoTunnel.hpp>

namespace writerfilter {
namespace ooxml
{
using namespace ::com::sun::star;
using namespace ::std;

template <class Interface, class ChildClass>
class RefAndPointer
{
    mutable ChildClass * mpHandler;
    mutable uno::Reference<Interface> mRef;

public:
    RefAndPointer()
    : mpHandler(NULL)
    {
    }

    RefAndPointer(ChildClass * pHandler)
    : mpHandler(pHandler), mRef(pHandler)
    {
    }

    RefAndPointer(uno::Reference<Interface> xRef)
    : mRef(xRef)
    {
        mpHandler = dynamic_cast<ChildClass *>(xRef.get());
    }

    virtual ~RefAndPointer()
    {
    }

    void set(ChildClass * pHandler)
    {
        mpHandler = pHandler;
        mRef = pHandler;
    }

    void set(uno::Reference<Interface> xHandler)
    {
        mpHandler = dynamic_cast<ChildClass*>(xHandler.get());
        mRef = xHandler;
    }

    ChildClass * getPointer() const { return mpHandler; }
    const uno::Reference<Interface> getRef() const { return mRef; }

    RefAndPointer & operator=
    (const RefAndPointer & rSrc)
    {
        set(rSrc.getHandler());

        return *this;
    }

    bool is() { return getRef().is(); }

    operator ChildClass* () { return getPointer(); }
    operator uno::Reference<Interface> () { return getRef(); }
};
}}
#endif // INCLUDED_REF_AND_POINTER_HXX
