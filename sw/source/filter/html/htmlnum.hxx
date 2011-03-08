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
#ifndef _HTMLNUM_HXX
#define _HTMLNUM_HXX

#include <swtypes.hxx>
#include <string.h>

#define HTML_NUMBUL_MARGINLEFT  (MM50*2 + MM50/2)
#define HTML_NUMBUL_INDENT      (-MM50)

class SwTxtNode;
class SwNumRule;

class SwHTMLNumRuleInfo
{
    sal_uInt16      aNumStarts[MAXLEVEL];
    SwNumRule   *   pNumRule;       // Aktuelle Numerierung
    sal_uInt16      nDeep;          // aktuelle Num-Tiefe (1, 2, 3, ...)
    sal_Bool        bRestart : 1;   // Export: Numerierung neu starten
    sal_Bool        bNumbered : 1;  // Export: Absatz ist numeriert

public:

    inline void Set( const SwHTMLNumRuleInfo& rInf );
    void Set( const SwTxtNode& rTxtNd );

    SwHTMLNumRuleInfo() :
        pNumRule( 0 ), nDeep( 0 ),
        bRestart( sal_False ), bNumbered( sal_False )
    {
        memset( &aNumStarts, 0xff, sizeof( aNumStarts ) );
    }

    SwHTMLNumRuleInfo( const SwHTMLNumRuleInfo& rInf ) :
        pNumRule( rInf.pNumRule ), nDeep( rInf.nDeep ),
        bRestart( rInf.bRestart ), bNumbered( rInf.bNumbered )
    {
        memcpy( &aNumStarts, &rInf.aNumStarts, sizeof( aNumStarts ) );
    }

    SwHTMLNumRuleInfo( const SwTxtNode& rTxtNd ) { Set( rTxtNd ); }
    inline SwHTMLNumRuleInfo& operator=( const SwHTMLNumRuleInfo& rInf );

    inline void Clear();

    void SetNumRule( const SwNumRule *pRule ) { pNumRule = (SwNumRule *)pRule; }
    SwNumRule *GetNumRule() { return pNumRule; }
    const SwNumRule *GetNumRule() const { return pNumRule; }

    void SetDepth( sal_uInt16 nDepth ) { nDeep = nDepth; }
    sal_uInt16 GetDepth() const { return nDeep; }
    sal_uInt16 IncDepth() { return ++nDeep; }
    sal_uInt16 DecDepth() { return nDeep==0 ? 0 : --nDeep; }
    inline sal_uInt8 GetLevel() const;

    void SetRestart( sal_Bool bSet ) { bRestart = bSet; }
    sal_Bool IsRestart() const { return bRestart; }

    void SetNumbered( sal_Bool bSet ) { bNumbered = bSet; }
    sal_Bool IsNumbered() const { return bNumbered; }

    inline void SetNodeStartValue( sal_uInt8 nLvl, sal_uInt16 nVal=USHRT_MAX );
    sal_uInt16 GetNodeStartValue( sal_uInt8 nLvl ) const { return aNumStarts[nLvl]; }
};

inline SwHTMLNumRuleInfo& SwHTMLNumRuleInfo::operator=(
    const SwHTMLNumRuleInfo& rInf )
{
    Set( rInf );
    return *this;
}

inline void SwHTMLNumRuleInfo::Set( const SwHTMLNumRuleInfo& rInf )
{
    pNumRule = rInf.pNumRule;
    nDeep = rInf.nDeep;
    bRestart = rInf.bRestart;
    bNumbered = rInf.bNumbered;
    memcpy( &aNumStarts, &rInf.aNumStarts, sizeof( aNumStarts ) );
}

inline void SwHTMLNumRuleInfo::Clear()
{
    pNumRule = 0;
    nDeep = 0;
    bRestart = bNumbered = sal_False;
    memset( &aNumStarts, 0xff, sizeof( aNumStarts ) );
}

inline sal_uInt8 SwHTMLNumRuleInfo::GetLevel() const
{
    return
        (sal_uInt8)( pNumRule!=0 && nDeep != 0
            ? ( nDeep<=MAXLEVEL ? nDeep-1 : MAXLEVEL - 1 )
            : 0 );
}

inline void SwHTMLNumRuleInfo::SetNodeStartValue( sal_uInt8 nLvl, sal_uInt16 nVal )
{
    aNumStarts[nLvl] = nVal;
}


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
