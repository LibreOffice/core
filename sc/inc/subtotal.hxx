/*************************************************************************
 *
 *  $RCSfile: subtotal.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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



class SubTotal
{
private:
    long    nCount;
    long    nCount2;
    double  nSum;
    double  nSumSqr;
    double  nMax;
    double  nMin;
    double  nProduct;
    BOOL    bSumOk;
    BOOL    bSumSqrOk;
    BOOL    bProductOk;
    USHORT  nProgress;

public:
    USHORT  nIndex;         // Test
    static jmp_buf aGlobalJumpBuf;

public:
            SubTotal();
            ~SubTotal();

    void    UpdateNoVal();
    void    Update( double nVal );
    void    Update( const SubTotal& rVal );

    short   Valid( USHORT nFunction ) const;
                            // return 0 => Fehler, -1 => kein Wert, 1 => ok
    double  Result( USHORT nFunction ) const;
    static  BOOL SafePlus( double& fVal1, double fVal2);
    static  BOOL SafeMult( double& fVal1, double fVal2);
    static  BOOL SafeDiv( double& fVal1, double fVal2);
};


struct ScFunctionData                   // zum Berechnen von einzelnen Funktionen
{
    ScSubTotalFunc  eFunc;
    double          nVal;
    long            nCount;
    BOOL            bError;

    ScFunctionData( ScSubTotalFunc eFn ) :
        eFunc(eFn), nVal(0.0), nCount(0), bError(FALSE) {}
};


#endif


