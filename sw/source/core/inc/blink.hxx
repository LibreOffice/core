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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_BLINK_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_BLINK_HXX

class SwLinePortion;
class SwRootFrm;
class SwTextFrm;

#include <vcl/timer.hxx>
#include <tools/gen.hxx>
#include <comphelper/stl_types.hxx>

#include <set>
#include <memory>

class SwBlinkPortion
{
    Point               aPos;
    const SwLinePortion *pPor;
    const SwRootFrm     *pFrm;
    sal_uInt16              nDir;

public:
    SwBlinkPortion(const SwLinePortion* pPortion, sal_uInt16 nDirection)
        : pPor(pPortion)
        , pFrm(nullptr)
        , nDir(nDirection)
    {
    }
    SwBlinkPortion(const SwBlinkPortion* pBlink, const SwLinePortion* pPort)
        : aPos(pBlink->aPos)
        , pPor(pPort)
        , pFrm(pBlink->pFrm)
        , nDir(pBlink->nDir)
    {
    }
    void SetPos( const Point& aNew ){ aPos = aNew; }
    const Point& GetPos() const{ return aPos; }
    void SetRootFrm( const SwRootFrm* pNew ){ pFrm = pNew; }
    const SwRootFrm* GetRootFrm() const{ return pFrm; }
    const SwLinePortion *GetPortion() const{ return pPor; }
    sal_uInt16 GetDirection() const { return nDir; }
    bool operator<( const SwBlinkPortion& rBlinkPortion ) const
    { return reinterpret_cast<sal_IntPtr>(pPor) < reinterpret_cast<sal_IntPtr>(rBlinkPortion.pPor); }
    bool operator==( const SwBlinkPortion& rBlinkPortion ) const
    { return reinterpret_cast<sal_IntPtr>(pPor) == reinterpret_cast<sal_IntPtr>(rBlinkPortion.pPor); }
};

typedef std::set<std::unique_ptr<SwBlinkPortion>,
            comphelper::UniquePtrValueLess<SwBlinkPortion>> SwBlinkSet;

class SwBlink
{
    SwBlinkSet      m_List;
    AutoTimer       aTimer;
    bool            bVisible;

public:
    SwBlink();
    ~SwBlink();

    DECL_LINK_TYPED( Blinker, Timer *, void );

    void Insert( const Point& rPoint, const SwLinePortion* pPor,
                 const SwTextFrm *pTextFrm, sal_uInt16 nDir );
    void Replace( const SwLinePortion* pOld, const SwLinePortion* pNew );
    void Delete( const SwLinePortion* pPor );
    void FrmDelete( const SwRootFrm* pRoot );
    bool IsVisible() const { return bVisible ; }
};

// Blink-Manager, global variable, see Blink.Cxx
extern SwBlink *pBlink;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
