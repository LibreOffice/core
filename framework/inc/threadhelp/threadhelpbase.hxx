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

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#define __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_

#include <threadhelp/lockhelper.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          "baseclass" to make own classes threadsafe
    @descr          Sometimes you must share your lock- or mutex member with any other baseclasses.
                    And baseclasses are initialized erlier then members! That's why you should use
                    this struct as first of your baseclasses!!!
                    Then you will get a public member "m_aLock" which can be used by special guard implementations
                    to make your code threadsafe.

    @seealso        class LockHelper

    @implements     -
    @base           -

    @devstatus      ready to use
*//*-*************************************************************************************************************/
struct ThreadHelpBase
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:
        ThreadHelpBase( ::osl::SolarMutex* pSolarMutex = NULL )
                :   m_aLock( pSolarMutex )
        {
        }

    //-------------------------------------------------------------------------------------------------------------
    //  public member
    //  Make it mutable for using in const functions!
    //-------------------------------------------------------------------------------------------------------------
    public:

        mutable LockHelper m_aLock;
};

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
