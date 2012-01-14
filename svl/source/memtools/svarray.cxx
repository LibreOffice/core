/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_STRINGSSORTDTOR

#include <svl/svstdarr.hxx>
#include <tools/debug.hxx>

SV_IMPL_VARARR(SvPtrarr,VoidPtr)

sal_uInt16 SvPtrarr::GetPos( const VoidPtr& aElement ) const
{   sal_uInt16 n;
    for( n=0; n < nA && *(GetData()+n) != aElement; ) n++;
    return ( n >= nA ? USHRT_MAX : n );
}

// ---------------- strings -------------------------------------

// Array with different Seek method
_SV_IMPL_SORTAR_ALG( SvStringsISortDtor, StringPtr )
void SvStringsISortDtor::DeleteAndDestroy( sal_uInt16 nP, sal_uInt16 nL )
{
    if( nL )
    {
        DBG_ASSERT( nP < nA && nP + nL <= nA, "ERR_VAR_DEL" );
        for( sal_uInt16 n=nP; n < nP + nL; n++ )
            delete *((StringPtr*)pData+n);
        SvPtrarr::Remove( nP, nL );
    }
}
sal_Bool SvStringsISortDtor::Seek_Entry( const StringPtr aE, sal_uInt16* pP ) const
{
    register sal_uInt16 nO  = SvStringsISortDtor_SAR::Count(),
            nM,
            nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            StringCompare eCmp = (*((StringPtr*)pData + nM))->
                                    CompareIgnoreCaseToAscii( *(aE) );
            if( COMPARE_EQUAL == eCmp )
            {
                if( pP ) *pP = nM;
                return sal_True;
            }
            else if( COMPARE_LESS == eCmp )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pP ) *pP = nU;
                return sal_False;
            }
            else
                nO = nM - 1;
        }
    }
    if( pP ) *pP = nU;
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
