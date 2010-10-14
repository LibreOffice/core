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

#include <osl/diagnose.h>
#include <vos/object.hxx>
#include <vos/mutex.hxx>

using namespace vos;

/////////////////////////////////////////////////////////////////////////////
//
//  class Mutex
//

VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OMutex, vos), VOS_NAMESPACE(OMutex, vos), VOS_NAMESPACE(OObject, vos), 0);

IMutex& OMutex::getGlobalMutex()
{
    static OMutex theGlobalMutex;

    return theGlobalMutex;
}

OMutex::OMutex()
{
    m_Impl= osl_createMutex();
}

OMutex::~OMutex()
{
    osl_destroyMutex(m_Impl);
}

void OMutex::acquire()
{
    osl_acquireMutex(m_Impl);
}

sal_Bool OMutex::tryToAcquire()
{
    return osl_tryToAcquireMutex(m_Impl);
}

void OMutex::release()
{
    osl_releaseMutex(m_Impl);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
