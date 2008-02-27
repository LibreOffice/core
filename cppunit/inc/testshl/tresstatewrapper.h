/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tresstatewrapper.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:19:45 $
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

#ifndef TESTSTATEWRAPPER_H
#define TESTSTATEWRAPPER_H

#ifdef _RTL_TRES_H_
#error "This is old code, please remove the include of rtl/tres.h"
#endif

#include <sal/types.h>
#include "cppunit/autoregister/registertestfunction.h"

#ifdef __cplusplus
extern "C" {
#endif

// typedef void* hTestResult;
typedef hTestResult rtlTestResult;

// -----------------------------------------------------------------------------
void c_rtl_tres_state_start(hTestResult _aResult, const sal_Char* _pName);
void c_rtl_tres_state_end(hTestResult _aResult, const sal_Char* _pName);

// -----------------------------------------------------------------------------
// the old test state function, which here convert all values for using with the
// new testshl tool

sal_Bool SAL_CALL c_rtl_tres_state(
    hTestResult pResult,
    sal_Bool bSuccess,
    char const * pTestMsg,
    char const * pFuncName);

#ifdef __cplusplus
}
#endif

#endif
