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


