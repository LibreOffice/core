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

#ifndef SC_RANGESEQ_HXX
#define SC_RANGESEQ_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
namespace binfilter {

class SvNumberFormatter;
class ScDocument;
class ScRange;
class ScMatrix;

class ScRangeToSequence
{
public:
    static BOOL	FillLongArray( ::com::sun::star::uno::Any& rAny,
                                ScDocument* pDoc, const ScRange& rRange );
    static BOOL	FillLongArray( ::com::sun::star::uno::Any& rAny,
                                const ScMatrix* pMatrix );
    static BOOL	FillDoubleArray( ::com::sun::star::uno::Any& rAny,
                                ScDocument* pDoc, const ScRange& rRange );
    static BOOL	FillDoubleArray( ::com::sun::star::uno::Any& rAny,
                                const ScMatrix* pMatrix );
    static BOOL	FillStringArray( ::com::sun::star::uno::Any& rAny,
                                ScDocument* pDoc, const ScRange& rRange );
    static BOOL	FillStringArray( ::com::sun::star::uno::Any& rAny,
                                const ScMatrix* pMatrix, SvNumberFormatter* pFormatter );
    static BOOL	FillMixedArray( ::com::sun::star::uno::Any& rAny,
                                ScDocument* pDoc, const ScRange& rRange,
                                BOOL bAllowNV = FALSE );
    static BOOL	FillMixedArray( ::com::sun::star::uno::Any& rAny,
                                const ScMatrix* pMatrix );
};



} //namespace binfilter
#endif

