/*************************************************************************
 *
 *  $RCSfile: htmlnum.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _HTMLNUM_HXX
#define _HTMLNUM_HXX

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
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
#ifndef NUM_RELSPACE
    sal_Bool        bUpdateWholeNum : 1;    // Import: Muss die NumRule
                                        // vollstaendig aktualisiert werden
#endif

public:

    inline void Set( const SwHTMLNumRuleInfo& rInf );
    void Set( const SwTxtNode& rTxtNd );

    SwHTMLNumRuleInfo() :
        pNumRule( 0 ), nDeep( 0 ),
#ifndef NUM_RELSPACE
        bUpdateWholeNum( sal_False ),
#endif
        bRestart( sal_False ), bNumbered( sal_False )
    {
        memset( &aNumStarts, 0xff, sizeof( aNumStarts ) );
    }

    SwHTMLNumRuleInfo( const SwHTMLNumRuleInfo& rInf ) :
        pNumRule( rInf.pNumRule ), nDeep( rInf.nDeep ),
#ifndef NUM_RELSPACE
        bUpdateWholeNum( rInf.bUpdateWholeNum ),
#endif
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

#ifndef NUM_RELSPACE
    void SetUpdateWholeNum( sal_Bool bSet ) { bUpdateWholeNum = bSet; }
    sal_Bool IsUpdateWholeNum() const { return bUpdateWholeNum; }
#endif

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
#ifndef NUM_RELSPACE
    bUpdateWholeNum = rInf.bUpdateWholeNum;
#endif
    bRestart = rInf.bRestart;
    bNumbered = rInf.bNumbered;
    memcpy( &aNumStarts, &rInf.aNumStarts, sizeof( aNumStarts ) );
}

inline void SwHTMLNumRuleInfo::Clear()
{
    pNumRule = 0;
    nDeep = 0;
#ifndef NUM_RELSPACE
    bUpdateWholeNum = sal_False;
#endif
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


