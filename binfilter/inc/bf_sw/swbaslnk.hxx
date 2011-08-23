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
#ifndef _SWBASLNK_HXX
#define _SWBASLNK_HXX


#include <bf_so3/lnkbase.hxx>
namespace binfilter {

class SwNode;
class SwCntntNode;
class SwNodeIndex;

class SwBaseLink : public ::binfilter::SvBaseLink
{
    SwCntntNode* pCntntNode;
    BOOL bSwapIn : 1;
    BOOL bNoDataFlag : 1;
    BOOL bIgnoreDataChanged : 1;

public:
    TYPEINFO();

    SwBaseLink( USHORT nMode, USHORT nFormat, SwCntntNode* pNode = 0 )
        : ::binfilter::SvBaseLink( nMode, nFormat ), pCntntNode( pNode ),
        bSwapIn( FALSE ), bNoDataFlag( FALSE ), bIgnoreDataChanged( FALSE )
    {}
    virtual ~SwBaseLink();

    virtual void DataChanged( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue );



    SwCntntNode *GetCntntNode() { return pCntntNode; }

    // nur fuer Grafiken
    FASTBOOL SwapIn( BOOL bWaitForData = FALSE, BOOL bNativFormat = FALSE );

    FASTBOOL IsShowQuickDrawBmp() const;				// nur fuer Grafiken

    FASTBOOL Connect() { return 0 != SvBaseLink::GetRealObject(); }

    // nur fuer Grafik-Links ( zum Umschalten zwischen DDE / Grf-Link)
    void SetObjType( USHORT nType )	{ SvBaseLink::SetObjType( nType ); }

    BOOL IsRecursion( const SwBaseLink* pChkLnk ) const;
    virtual BOOL IsInRange( ULONG nSttNd, ULONG nEndNd, xub_StrLen nStt = 0,
                            xub_StrLen nEnd = STRING_NOTFOUND ) const;

    void SetNoDataFlag()	{ bNoDataFlag = TRUE; }
    BOOL ChkNoDataFlag()	{ return bNoDataFlag ? !(bNoDataFlag = FALSE) : FALSE; }
    BOOL IsNoDataFlag()	const			{ return bNoDataFlag; }
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
