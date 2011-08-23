/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#ifdef DEBUG_MEMORY
        clog << "MEMORY:" << mpHandler->getInstanceNumber() << ":RefAndPointer"
             << endl;
#endif
    }

    RefAndPointer(ChildClass * pHandler)
    : mpHandler(pHandler), mRef(pHandler)
    {
#ifdef DEBUG_MEMORY
        clog << "MEMORY:" << mpHandler->getInstanceNumber() << ":RefAndPointer"
             << endl;
#endif
    }

    RefAndPointer(uno::Reference<Interface> xRef)
    : mRef(xRef)
    {
#if 0
        uno::Reference<lang::XUnoTunnel> xTunnel( xRef, uno::UNO_QUERY);
        
        if (xTunnel.is())
            mpHandler = reinterpret_cast<ChildClass *>(xTunnel->getSomething(ChildClass::getUnoTunnelId()));
#else
        mpHandler = dynamic_cast<ChildClass *>(xRef.get());
#endif
        if (mpHandler != NULL)
            clog << "MEMORY:" << mpHandler->getInstanceNumber() 
                 << ":RefAndPointer" << endl;
    }
    
    virtual ~RefAndPointer() 
    {
#ifdef DEBUG_MEMORY
        if (mpHandler != NULL)
            clog << "MEMORY:" << mpHandler->getInstanceNumber() 
                 << ":~RefAndPointer" << endl;
#endif
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
