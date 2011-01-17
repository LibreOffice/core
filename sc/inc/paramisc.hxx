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

#ifndef SC_PARAMISC_HXX
#define SC_PARAMISC_HXX

#include "address.hxx"
#include <tools/solar.h>

class String;

struct ScSolveParam
{
    ScAddress   aRefFormulaCell;
    ScAddress   aRefVariableCell;
    String*     pStrTargetVal;

    ScSolveParam();
    ScSolveParam( const ScSolveParam& r );
    ScSolveParam( const ScAddress&  rFormulaCell,
                  const ScAddress&  rVariableCell,
                  const String& rTargetValStr );
    ~ScSolveParam();

    ScSolveParam&   operator=   ( const ScSolveParam& r );
    sal_Bool            operator==  ( const ScSolveParam& r ) const;
};

//-----------------------------------------------------------------------

struct ScTabOpParam
{
    ScRefAddress    aRefFormulaCell;
    ScRefAddress    aRefFormulaEnd;
    ScRefAddress    aRefRowCell;
    ScRefAddress    aRefColCell;
    sal_uInt8           nMode;

    ScTabOpParam() {};
    ScTabOpParam( const ScTabOpParam& r );
    ScTabOpParam( const ScRefAddress& rFormulaCell,
                  const ScRefAddress& rFormulaEnd,
                  const ScRefAddress& rRowCell,
                  const ScRefAddress& rColCell,
                        sal_uInt8        nMd);
    ~ScTabOpParam() {};

    ScTabOpParam&   operator=       ( const ScTabOpParam& r );
    sal_Bool            operator==      ( const ScTabOpParam& r ) const;
};

#endif // SC_PARAMISC_HXX

