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

#include <sfx2/lnkbase.hxx>

class SwNode;
class SwCntntNode;
class ReReadThread;
long GrfNodeChanged( void* pLink, void* pCaller );

class SwBaseLink : public ::sfx2::SvBaseLink
{
    friend long GrfNodeChanged( void* pLink, void* pCaller );

    SwCntntNode* pCntntNode;
    sal_Bool bSwapIn : 1;
    sal_Bool bNoDataFlag : 1;
    sal_Bool bIgnoreDataChanged : 1;
    ReReadThread* m_pReReadThread;

protected:
    SwBaseLink(): m_pReReadThread(0) {}

    SwBaseLink( const String& rNm, sal_uInt16 nObjectType, ::sfx2::SvLinkSource* pObj,
                 SwCntntNode* pNode = 0 )
        : ::sfx2::SvBaseLink( rNm, nObjectType, pObj ), pCntntNode( pNode ),
        bSwapIn( sal_False ), bNoDataFlag( sal_False ), bIgnoreDataChanged( sal_False ),
        m_pReReadThread(0)
    {}

public:
    TYPEINFO();

    SwBaseLink( sal_uInt16 nMode, sal_uInt16 nFormat, SwCntntNode* pNode = 0 )
        : ::sfx2::SvBaseLink( nMode, nFormat ), pCntntNode( pNode ),
        bSwapIn( sal_False ), bNoDataFlag( sal_False ), bIgnoreDataChanged( sal_False ),
        m_pReReadThread(0)
    {}
    virtual ~SwBaseLink();

    virtual void DataChanged( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue );

    virtual void Closed();

    virtual const SwNode* GetAnchor() const;

    SwCntntNode *GetCntntNode() { return pCntntNode; }

    // nur fuer Grafiken
    sal_Bool SwapIn( sal_Bool bWaitForData = sal_False, sal_Bool bNativFormat = sal_False );

    sal_Bool Connect() { return 0 != SvBaseLink::GetRealObject(); }

    // nur fuer Grafik-Links ( zum Umschalten zwischen DDE / Grf-Link)
    void SetObjType( sal_uInt16 nType ) { SvBaseLink::SetObjType( nType ); }

    sal_Bool IsRecursion( const SwBaseLink* pChkLnk ) const;
    virtual sal_Bool IsInRange( sal_uLong nSttNd, sal_uLong nEndNd, xub_StrLen nStt = 0,
                            xub_StrLen nEnd = STRING_NOTFOUND ) const;

    void SetNoDataFlag()    { bNoDataFlag = sal_True; }
    sal_Bool ChkNoDataFlag()    { sal_Bool bRet = bNoDataFlag; bNoDataFlag = sal_False; return bRet; }
    sal_Bool IsNoDataFlag() const           { return bNoDataFlag; }
};


#endif

