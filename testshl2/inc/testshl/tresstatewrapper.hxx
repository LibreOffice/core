/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
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

#ifndef teststatewrapper_hxx
#define teststatewrapper_hxx

#include <testshl/nocopy.hxx>
#include <sal/types.h>

#include <testshl/simpleheader.hxx>
#include <cppunit/callbackfunc_fktptr.h>

// This is a hack, because it's possible that the testshl directory doesn't exist.
#ifdef LOAD_TRESSTATEWRAPPER_LOCAL
#include "tresstatewrapper.h"
#else
#include <testshl/tresstatewrapper.h>
#endif

// -----------------------------------------------------------------------------
// helper class to mark the start off old test code
// the name is need in the test result generator
class rtl_tres_state_start : NOCOPY
{
    const sal_Char* m_pName;
    hTestResult     m_aResult;
public:
    rtl_tres_state_start(hTestResult _aResult, const sal_Char* _pName);
    ~rtl_tres_state_start();
};

#endif

