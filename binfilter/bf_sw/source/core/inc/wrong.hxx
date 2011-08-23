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

#ifndef	_WRONG_HXX
#define	_WRONG_HXX

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_XUB_STRLEN
#define _SVSTDARR_ULONGS
#include <bf_svtools/svstdarr.hxx>
#endif
/*N*/ #include <tools/debug.hxx> //for stripping
namespace binfilter {
#define WRPOS( nIdx ) ((xub_StrLen)( GetObject( nIdx ) ))
#define WRLEN( nIdx ) ((xub_StrLen)( aLen.GetObject( nIdx ) ))

class SwWrongList : public SvXub_StrLens
{
    SvXub_StrLens aLen;
    xub_StrLen nBeginInvalid;	// Start des ungueltigen Bereichs
    xub_StrLen nEndInvalid;		// Ende des ungueltigen Bereichs
    void ShiftLeft( xub_StrLen &rPos, xub_StrLen nStart, xub_StrLen nEnd )
    { if( rPos > nStart ) rPos = rPos > nEnd ? rPos - nEnd + nStart : nStart; }
    void ShiftRight( xub_StrLen &rPos, xub_StrLen nStart, xub_StrLen nEnd )
    { if( rPos >= nStart ) rPos += nStart - nEnd; }
    void _Invalidate( xub_StrLen nBegin, xub_StrLen nEnd );
public:
    inline SwWrongList() :
        SvXub_StrLens(5,5), aLen(5,5), nBeginInvalid( STRING_LEN ){}
    inline xub_StrLen GetBeginInv() const { return nBeginInvalid; }
    inline xub_StrLen GetEndInv() const { return nEndInvalid; }
    inline BOOL InsideInvalid( xub_StrLen nChk ) const
        { return nChk >= nBeginInvalid && nChk <= nEndInvalid; }
    inline void SetInvalid( xub_StrLen nBegin, xub_StrLen nEnd )
        { nBeginInvalid = nBegin; nEndInvalid = nEnd; }
    inline void Validate(){ nBeginInvalid = STRING_LEN; }
    inline void Invalidate( xub_StrLen nBegin, xub_StrLen nEnd )
        { if( STRING_LEN == GetBeginInv() ) SetInvalid( nBegin, nEnd );
          else _Invalidate( nBegin, nEnd ); }
    BOOL InvalidateWrong();
    void Move( xub_StrLen nPos, long nDiff ){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void Move( xub_StrLen nPos, long nDiff );

    // Divide the list into two part, the wrong words until nSplitPos will be
    // removed and transferred to a new SwWrongList.
    // Join the next SwWrongList, nInsertPos is my own text length, where
    // the other wrong list has to be inserted.

    inline xub_StrLen Len( USHORT nIdx ) const { return WRLEN( nIdx );	}
    inline xub_StrLen Pos( USHORT nIdx ) const { return WRPOS( nIdx );	}
    inline void Insert( xub_StrLen nNewPos, xub_StrLen nNewLen, USHORT nWhere )
    {SvXub_StrLens::Insert( nNewPos, nWhere ); aLen.Insert( nNewLen, nWhere );}

// Wer braucht dies?
    void Insert( ULONG nNew )
    ;//	{ SvULongs::Insert( nNew, GetPos( (USHORT)( nNew & WRMASK ) ) ); }
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
