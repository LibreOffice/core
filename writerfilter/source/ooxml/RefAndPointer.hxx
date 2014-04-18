/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_WRITERFILTER_SOURCE_OOXML_REFANDPOINTER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_OOXML_REFANDPOINTER_HXX

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
#ifdef DEBUG_MEMORY
        clog << "MEMORY:" << mpHandler->getInstanceNumber() << ":RefAndPointer"
             << endl;
#endif
    }

    RefAndPointer(uno::Reference<Interface> xRef)
    : mRef(xRef)
    {
        mpHandler = dynamic_cast<ChildClass *>(xRef.get());
#ifdef DEBUG_MEMORY
        if (mpHandler != NULL)
            clog << "MEMORY:" << mpHandler->getInstanceNumber()
                 << ":RefAndPointer" << endl;
#endif
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
#endif // INCLUDED_WRITERFILTER_SOURCE_OOXML_REFANDPOINTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
