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
#ifndef INCLUDED_SW_INC_TXTFLCNT_HXX
#define INCLUDED_SW_INC_TXTFLCNT_HXX

#include <txatbase.hxx>

class SwFlyInCntFrm;
class SwFrm;
class SwTxtNode;
class SwDoc;

// Attribute for line-bound frames (without end index).
class SwTxtFlyCnt : public SwTxtAttr
{
    SwFlyInCntFrm  *_GetFlyFrm( const SwFrm *pCurrFrm );
public:
    SwTxtFlyCnt( SwFmtFlyCnt& rAttr, sal_Int32 nStart );

    // Sets anchor in pFmt and
    void    SetAnchor( const SwTxtNode *pNode );

    inline        SwFlyInCntFrm  *GetFlyFrm( const SwFrm *pCurrFrm );
    inline const  SwFlyInCntFrm  *GetFlyFrm( const SwFrm *pCurrFrm ) const;

    // creates for itself a new FlyFrameFormat.
    void CopyFlyFmt( SwDoc* pDoc );
};

inline SwFlyInCntFrm *SwTxtFlyCnt::GetFlyFrm( const SwFrm *pCurrFrm )
{
    return _GetFlyFrm( pCurrFrm );
}

inline const SwFlyInCntFrm *SwTxtFlyCnt::GetFlyFrm( const SwFrm *pCurrFrm ) const
{
    return (const_cast<SwTxtFlyCnt*>(this))->_GetFlyFrm( pCurrFrm );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
