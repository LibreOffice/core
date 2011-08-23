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

#ifndef SC_SUBTOTAL_HXX
#define SC_SUBTOTAL_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef MSC
    #include <setjmp.h>
#else
    extern "C"
    {
    #define _JBLEN  9  /* bp, di, si, sp, ret addr, ds */
    typedef  int  jmp_buf[_JBLEN];
    #define _JMP_BUF_DEFINED
    #define setjmp  _setjmp
    int  __cdecl _setjmp(jmp_buf);
    void __cdecl longjmp(jmp_buf, int);
    };
#endif

namespace binfilter {

class SubTotal
{
private:
    long	nCount;
    long	nCount2;
    double	nSum;
    double	nSumSqr;
    double	nMax;
    double	nMin;
    double	nProduct;
    BOOL	bSumOk;
    BOOL	bSumSqrOk;
    BOOL	bProductOk;
    USHORT  nProgress;

public:
    USHORT	nIndex;			// Test
    static jmp_buf aGlobalJumpBuf;

public:
            SubTotal();
            ~SubTotal();

    void	Update( double nVal );
    static  BOOL SafePlus( double& fVal1, double fVal2);
    static  BOOL SafeMult( double& fVal1, double fVal2);
};


struct ScFunctionData					// zum Berechnen von einzelnen Funktionen
{
    ScSubTotalFunc	eFunc;
    double			nVal;
    long			nCount;
    BOOL			bError;

    ScFunctionData( ScSubTotalFunc eFn ) :
        eFunc(eFn), nVal(0.0), nCount(0), bError(FALSE) {}
};


} //namespace binfilter
#endif


