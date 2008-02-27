/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: registerfunc.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:18:16 $
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

#ifndef _registerfunc_h
#define _registerfunc_h

#include <sal/types.h>
#include <cppunit/autoregister/htestresult.h>
#include <cppunit/autoregister/callbackfunc_fktptr.h>
#include <cppunit/autoregister/testfunc.h>

struct CallbackStructure;

// this function is used to register one function into a Container in the testshl tool.
extern "C" void SAL_CALL registerFunction(FktPtr aFunc, const char* aFuncName);

// this function is the register function for auto registering

extern "C" void SAL_CALL registerAllTestFunction(CallbackStructure* );
typedef void (* FktRegAllPtr)(CallbackStructure*);


#endif
