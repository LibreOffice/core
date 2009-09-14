/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 * $Revision$
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
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
// #include "precompiled_cppunit.hxx"

#include <stdio.h>
#include <sal/types.h>
#include <testshl/autoregister/registerfunc.h>
#include <testshl/autoregister/callbackstructure.h>
#include <testshl/cmdlinebits.hxx>

//! is this global CallbackStructure need?
CallbackStructure aGlobalStructure;
CmdLineBits st_nBits;

FktPtr_CallbackDispatch  pCallbackFunc = NULL;

// This function is called from the TestShl tool.
extern "C" void SAL_CALL registerAllTestFunction(CallbackStructure* _pCallback)
{
    aGlobalStructure = *_pCallback;

        st_nBits = aGlobalStructure.nBits;

    pCallbackFunc = _pCallback->aCallbackDispatch;
    FktRegFuncPtr pFunc = _pCallback->aRegisterFunction;
    if (pFunc)
    {
        RegisterTestFunctions(pFunc);
    }
    else
    {
        // error, should be echoed
        fprintf(stderr, "error: Callback Structure doesn't contain a test function ptr.\n");
    }

    // all is done, copy the magic value
    // this is only a simple internal invariant check,
    // if the called registerAllTestFunctions Function do, what we want.
    _pCallback->nMagic2 = _pCallback->nMagic;
}
