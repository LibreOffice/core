/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RefAndPointer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 12:01:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
