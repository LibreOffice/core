/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tres.hxx,v $
 * $Revision: 1.14 $
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

#ifndef _RTL_TRES_HXX_
#define _RTL_TRES_HXX_


#include <rtl/tres.h>

//#include <stdarg.h>

// <namespace_rtl>
namespace rtl
{
/*==========================================================================*/

// <class_TestResult>
class TestResult
{
    // pointer to testresult structure
    rtl_TestResult* pData;

    // <private_ctors>
    TestResult();
    TestResult( const TestResult& oRes );
    // </private_ctors>

public:


    // <public_ctors>
    TestResult( const sal_Char* meth, sal_uInt32 flags = 0 )
    {
        pData = rtl_tres_create( meth, flags );
    } // </public_ctors>

    // <dtor>
    ~TestResult()
    {
        rtl_tres_destroy( pData );
    }
    // </dtor>

    // <public_methods>
    rtl_TestResult* getData()
    {
        return pData;
    }
    // <method_state>
    sal_Bool state(
                    sal_Bool tst_state,
                    const sal_Char* msg = 0,
                    const sal_Char* sub = 0,
                    sal_Bool verbose = sal_False
                    )
    {
        return pData->pFuncs->state_( pData, tst_state, msg, sub, verbose );
    } // </method_state>

    void end( sal_Char* msg = 0 )
    {
        pData->pFuncs->end_( pData, msg );
    } // </method_state>
}; // </class_TestResult>

} // </namespace_rtl>
#endif





















