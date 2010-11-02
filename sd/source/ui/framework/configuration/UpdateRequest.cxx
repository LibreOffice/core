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

#include "precompiled_sd.hxx"

#include "UpdateRequest.hxx"

#include "framework/FrameworkHelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;

namespace sd { namespace framework {

UpdateRequest::UpdateRequest (void)
    throw()
    : UpdateRequestInterfaceBase(MutexOwner::maMutex)
{
}




UpdateRequest::~UpdateRequest (void) throw()
{
}




void SAL_CALL UpdateRequest::execute (const Reference<XConfiguration>& rxConfiguration)
    throw (RuntimeException)
{
    (void)rxConfiguration;
    // Do nothing here.  The configuration is updated when the request queue
    // becomes empty.
}




OUString SAL_CALL UpdateRequest::getName (void)
    throw (RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("UpdateRequest"));
}




void SAL_CALL UpdateRequest::setName (const OUString& rsName)
    throw (RuntimeException)
{
    (void)rsName;
    // Ignored.
}

} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
