/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tres.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:43:23 $
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

#ifndef _RTL_TRES_HXX_
#define _RTL_TRES_HXX_


#ifndef _RTL_TRES_H_
#include <rtl/tres.h>
#endif

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





















