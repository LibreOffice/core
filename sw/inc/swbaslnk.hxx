/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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

    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const OUString& rMimeType, const ::com::sun::star::uno::Any & rValue );

    virtual void Closed();

    virtual const SwNode* GetAnchor() const;

    SwCntntNode *GetCntntNode() { return pCntntNode; }

    // For graphics only.
    sal_Bool SwapIn( sal_Bool bWaitForData = sal_False, sal_Bool bNativFormat = sal_False );

    sal_Bool Connect() { return 0 != SvBaseLink::GetRealObject(); }

    // Only for graphics-links (for switching between DDE / Grf-link).
    void SetObjType( sal_uInt16 nType ) { SvBaseLink::SetObjType( nType ); }

    sal_Bool IsRecursion( const SwBaseLink* pChkLnk ) const;
    virtual sal_Bool IsInRange( sal_uLong nSttNd, sal_uLong nEndNd, xub_StrLen nStt = 0,
                            xub_StrLen nEnd = STRING_NOTFOUND ) const;

    void SetNoDataFlag()    { bNoDataFlag = sal_True; }
    sal_Bool ChkNoDataFlag()    { sal_Bool bRet = bNoDataFlag; bNoDataFlag = sal_False; return bRet; }
    sal_Bool IsNoDataFlag() const           { return bNoDataFlag; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
