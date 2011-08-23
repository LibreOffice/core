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
#ifndef _TXTFLCNT_HXX
#define _TXTFLCNT_HXX

#include <txatbase.hxx>
namespace binfilter {

class SwFlyInCntFrm;
class SwFrm;
class SwTxtNode;
class SwDoc;

// SWTXT_FLYCNT ********************************
// Attribut fuer zeilengebundene Frames (ohne Endindex)

class SwTxtFlyCnt : public SwTxtAttr
{
    SwFlyInCntFrm  *_GetFlyFrm( const SwFrm *pCurrFrm );

public:
    SwTxtFlyCnt( const SwFmtFlyCnt& rAttr, xub_StrLen nStart );

    // Setzt den Anker im pFmt und
    void	SetAnchor( const SwTxtNode *pNode );

    inline		  SwFlyInCntFrm  *GetFlyFrm( const SwFrm *pCurrFrm );
    inline const  SwFlyInCntFrm  *GetFlyFrm( const SwFrm *pCurrFrm ) const;

    // erzeugt sich ein neues FlyFrameFormat
    void CopyFlyFmt( SwDoc* pDoc );
};

inline SwFlyInCntFrm *SwTxtFlyCnt::GetFlyFrm( const SwFrm *pCurrFrm )
{
    return _GetFlyFrm( pCurrFrm );
}

inline const SwFlyInCntFrm *SwTxtFlyCnt::GetFlyFrm( const SwFrm *pCurrFrm ) const
{
    return ((SwTxtFlyCnt*)this)->_GetFlyFrm( pCurrFrm );
}

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
