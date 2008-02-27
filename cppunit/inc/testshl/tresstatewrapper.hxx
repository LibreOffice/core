/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tresstatewrapper.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:20:33 $
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

#ifndef teststatewrapper_hxx
#define teststatewrapper_hxx

#include <cppunit/nocopy.hxx>
#include <sal/types.h>

#include <cppunit/simpleheader.hxx>
#include <cppunit/autoregister/callbackfunc_fktptr.h>

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

