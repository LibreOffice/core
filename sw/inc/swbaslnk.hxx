/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

