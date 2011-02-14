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

#include <tools/solar.h>
#include <com/sun/star/uno/Any.h>
#include "scmatrix.hxx"

class SvNumberFormatter;
class ScDocument;
class ScRange;

class ScRangeToSequence
{
public:
    static sal_Bool FillLongArray( com::sun::star::uno::Any& rAny,
                                ScDocument* pDoc, const ScRange& rRange );
    static sal_Bool FillLongArray( com::sun::star::uno::Any& rAny,
                                const ScMatrix* pMatrix );
    static sal_Bool FillDoubleArray( com::sun::star::uno::Any& rAny,
                                ScDocument* pDoc, const ScRange& rRange );
    static sal_Bool FillDoubleArray( com::sun::star::uno::Any& rAny,
                                const ScMatrix* pMatrix );
    static sal_Bool FillStringArray( com::sun::star::uno::Any& rAny,
                                ScDocument* pDoc, const ScRange& rRange );
    static sal_Bool FillStringArray( com::sun::star::uno::Any& rAny,
                                const ScMatrix* pMatrix, SvNumberFormatter* pFormatter );
    static sal_Bool FillMixedArray( com::sun::star::uno::Any& rAny,
                                ScDocument* pDoc, const ScRange& rRange,
                                sal_Bool bAllowNV = sal_False );

    /** @param bDataTypes
            Additionally to the differentiation between string and double allow
            differentiation between other types such as as boolean. Needed for
            implementation of XFormulaParser. If <FALSE/>, boolean values are
            treated as ordinary double values 1 (true) and 0 (false).
     */
    static sal_Bool FillMixedArray( com::sun::star::uno::Any& rAny,
                                const ScMatrix* pMatrix, bool bDataTypes = false );
};


class ScApiTypeConversion
{
public:

    /** Convert an uno::Any to double if possible, including integer types.
        @param o_fVal
            Out: the double value on successful conversion.
        @param o_eClass
            Out: the uno::TypeClass of rAny.
        @returns <TRUE/> if successfully converted.
     */
    static  bool        ConvertAnyToDouble(
                            double & o_fVal,
                            com::sun::star::uno::TypeClass & o_eClass,
                            const com::sun::star::uno::Any & rAny );

};


class ScSequenceToMatrix
{
public:

    /** Convert a sequence of mixed elements to ScMatrix.

        Precondition: rAny.getValueType().equals( getCppuType( (uno::Sequence< uno::Sequence< uno::Any > > *)0))

        @returns a new'd ScMatrix as ScMatrixRef, NULL if rAny couldn't be read
        as type Sequence<Sequence<Any>>
     */
    static  ScMatrixRef CreateMixedMatrix( const com::sun::star::uno::Any & rAny );

};


class ScByteSequenceToString
{
public:
    //  rAny must contain Sequence<sal_Int8>,
    //  may or may not contain 0-bytes at the end
    static sal_Bool GetString( String& rString, const com::sun::star::uno::Any& rAny,
                            sal_uInt16 nEncoding );
};

#endif

