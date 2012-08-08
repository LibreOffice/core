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

#ifndef __FRAMEWORK_THREADHELP_IGATE_H_
#define __FRAMEWORK_THREADHELP_IGATE_H_

#include <osl/time.h>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr          We need this interface to support using of different gate implementations in a generic way.
*//*-*************************************************************************************************************/
class IGate
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        /*-****************************************************************************************************//**
            @descr      These functions must be supported by a derived class!
                            open()      -open access for all waiting threads
                            close()     -close access for all further coming threads
                            openGap()   -open access for current waiting threads only
                            wait()      -must be called to pass the gate
        *//*-*****************************************************************************************************/
        virtual void     open    (                                   ) = 0;
        virtual void     close   (                                   ) = 0;
        virtual void     openGap (                                   ) = 0;
        virtual sal_Bool wait    ( const TimeValue* pTimeOut = NULL  ) = 0;

    protected:
        ~IGate() {}
};      //  class IGate

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_IGATE_H_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
