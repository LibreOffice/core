/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: flypos.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _FLYPOS_HXX
#define _FLYPOS_HXX


#include <svtools/svarray.hxx>

class SwFrmFmt;
class SwNodeIndex;

// Struktur zum Erfragen der akt. freifliegenden Rahmen am Dokument.
class SwPosFlyFrm
{
    const SwFrmFmt* pFrmFmt;    // das FlyFrmFmt
//  SwPosition* pPos;           // Position in den ContentNode
    SwNodeIndex* pNdIdx;        // es reicht ein Index auf den Node
    UINT32 nOrdNum;
public:
    SwPosFlyFrm( const SwNodeIndex& , const SwFrmFmt*, USHORT nArrPos );
    virtual ~SwPosFlyFrm(); // virtual fuer die Writer (DLL !!)

    // operatoren fuer das Sort-Array
    BOOL operator==( const SwPosFlyFrm& );
    BOOL operator<( const SwPosFlyFrm& );

    const SwFrmFmt& GetFmt() const { return *pFrmFmt; }
    const SwNodeIndex& GetNdIndex() const { return *pNdIdx; }
    UINT32 GetOrdNum() const { return nOrdNum; }
};

typedef SwPosFlyFrm* SwPosFlyFrmPtr;
SV_DECL_PTRARR_SORT( SwPosFlyFrms, SwPosFlyFrmPtr, 0, 40 )

#endif // _FLYPOS_HXX
