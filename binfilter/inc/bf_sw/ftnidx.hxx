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
#ifndef _FTNIDX_HXX
#define _FTNIDX_HXX


#define _SVSTDARR_USHORTS
#include <bf_svtools/svstdarr.hxx>
namespace binfilter {

class SwTxtFtn;
class SwNodeIndex;
class SwSectionNode;

// ueberall, wo der NodeIndex gebraucht wird, werden die hier fehlenden
// Headerfiles schon includes. Darum hier nur als define und nicht als
// inline Methode (spart Compile-Zeit)
#define _SwTxtFtn_GetIndex( pFIdx )	(pFIdx->GetTxtNode().GetIndex())


typedef SwTxtFtn* SwTxtFtnPtr;
SV_DECL_PTRARR_SORT( _SwFtnIdxs, SwTxtFtnPtr, 0, 10 )

class SwFtnIdxs : public _SwFtnIdxs
{
public:
    SwFtnIdxs() {}

    void UpdateFtn( const SwNodeIndex& rStt );   	// ab Pos. alle Updaten
    void UpdateAllFtn();					// alle Fussnoten updaten

    SwTxtFtn* SeekEntry( const SwNodeIndex& rIdx, USHORT* pPos = 0 ) const;
};


class SwUpdFtnEndNtAtEnd
{
    SvPtrarr aFtnSects, aEndSects;
    SvUShorts aFtnNums, aEndNums;

public:
    SwUpdFtnEndNtAtEnd() : aFtnSects( 0, 4 ), aEndSects( 0, 4 ),
                           aFtnNums( 0, 4 ), aEndNums( 0, 4 )
        {}

    static const SwSectionNode* FindSectNdWithEndAttr(
                                            const SwTxtFtn& rTxtFtn );

    USHORT GetNumber( const SwTxtFtn& rTxtFtn, const SwSectionNode& rNd );
    USHORT ChkNumber( const SwTxtFtn& rTxtFtn );
};



} //namespace binfilter
#endif // _FTNIDX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
