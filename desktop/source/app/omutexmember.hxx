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

#ifndef __FRAMEWORK_OMUTEXMEMBER_HXX_
#define __FRAMEWORK_OMUTEXMEMBER_HXX_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#include <osl/mutex.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          definition of a public mutex member
    @descr          You can use this struct as baseclass to get a public mutex member for right initialization.
                    Don't use it as member. You can't guarantee the right order of initialization of baseclasses then!
                    And some other helper classes share the mutex with an implementation and must have a valid one.

    @seealso        See implementation of constructors in derived classes for further informations!

    @devstatus      ready
*//*-*************************************************************************************************************/

struct OMutexMember
{
    ::osl::Mutex    m_aMutex;
};

#endif  //  #ifndef __FRAMEWORK_OMUTEXMEMBER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
