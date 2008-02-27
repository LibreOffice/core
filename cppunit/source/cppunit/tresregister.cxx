/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tresregister.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:21:06 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppunit.hxx"

#include <stdio.h>
#include <sal/types.h>
#include <cppunit/autoregister/registerfunc.h>
#include <cppunit/autoregister/callbackstructure.h>
#include <cppunit/cmdlinebits.hxx>

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
