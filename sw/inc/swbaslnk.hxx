/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swbaslnk.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:10:43 $
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
#ifndef _SWBASLNK_HXX
#define _SWBASLNK_HXX

#ifndef _LNKBASE_HXX //autogen
#include <sfx2/lnkbase.hxx>
#endif

class SwNode;
class SwCntntNode;
class ReReadThread;
long GrfNodeChanged( void* pLink, void* pCaller );

class SwBaseLink : public ::sfx2::SvBaseLink
{
    friend long GrfNodeChanged( void* pLink, void* pCaller );

    SwCntntNode* pCntntNode;
    BOOL bSwapIn : 1;
    BOOL bNoDataFlag : 1;
    BOOL bIgnoreDataChanged : 1;
    ReReadThread* m_pReReadThread;

protected:
    SwBaseLink(): m_pReReadThread(0) {}

    SwBaseLink( const String& rNm, USHORT nObjectType, ::sfx2::SvLinkSource* pObj,
                 SwCntntNode* pNode = 0 )
        : ::sfx2::SvBaseLink( rNm, nObjectType, pObj ), pCntntNode( pNode ),
        bSwapIn( FALSE ), bNoDataFlag( FALSE ), bIgnoreDataChanged( FALSE ),
        m_pReReadThread(0)
    {}

public:
    TYPEINFO();

    SwBaseLink( USHORT nMode, USHORT nFormat, SwCntntNode* pNode = 0 )
        : ::sfx2::SvBaseLink( nMode, nFormat ), pCntntNode( pNode ),
        bSwapIn( FALSE ), bNoDataFlag( FALSE ), bIgnoreDataChanged( FALSE ),
        m_pReReadThread(0)
    {}
    virtual ~SwBaseLink();

    virtual void DataChanged( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue );

    virtual void Closed();

    virtual const SwNode* GetAnchor() const;

    SwCntntNode *GetCntntNode() { return pCntntNode; }

    // nur fuer Grafiken
    BOOL SwapIn( BOOL bWaitForData = FALSE, BOOL bNativFormat = FALSE );

    BOOL Connect() { return 0 != SvBaseLink::GetRealObject(); }

    // nur fuer Grafik-Links ( zum Umschalten zwischen DDE / Grf-Link)
    void SetObjType( USHORT nType ) { SvBaseLink::SetObjType( nType ); }

    BOOL IsRecursion( const SwBaseLink* pChkLnk ) const;
    virtual BOOL IsInRange( ULONG nSttNd, ULONG nEndNd, xub_StrLen nStt = 0,
                            xub_StrLen nEnd = STRING_NOTFOUND ) const;

    void SetNoDataFlag()    { bNoDataFlag = TRUE; }
    BOOL ChkNoDataFlag()    { BOOL bRet = bNoDataFlag; bNoDataFlag = FALSE; return bRet; }
    BOOL IsNoDataFlag() const           { return bNoDataFlag; }
};


#endif

