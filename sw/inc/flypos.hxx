/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: flypos.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 15:22:49 $
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
#ifndef _FLYPOS_HXX
#define _FLYPOS_HXX


#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

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
