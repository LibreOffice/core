/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"
#include "cachemapobjectcontainer2.hxx"
#include "cachemapobject2.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XWeak.hpp"
#include "cppuhelper/weakref.hxx"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

using ucb::cachemap::Object2;
using ucb::cachemap::ObjectContainer2;
using namespace com::sun::star;

ObjectContainer2::ObjectContainer2()
{}

ObjectContainer2::~ObjectContainer2() SAL_THROW(())
{}

rtl::Reference< Object2 > ObjectContainer2::get(rtl::OUString const & rKey)
{
    rtl::Reference< Object2 > xElement;
    {
        osl::MutexGuard aGuard(m_aMutex);
        Map::iterator aIt(m_aMap.find(rKey));
        if (aIt != m_aMap.end())
            xElement = static_cast< Object2 * >(
                           uno::Reference< uno::XWeak >(
                                   aIt->second.get(), uno::UNO_QUERY).
                               get());
        if (!xElement.is())
        {
            xElement = new Object2;
            m_aMap[rKey]
                = uno::WeakReference< Object2 >(xElement.get());
        }
    }
    return xElement;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
