/*************************************************************************
 *
 *  $RCSfile: numrule.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:27 $
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
#ifndef _NUMRULE_HXX
#define _NUMRULE_HXX


#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif
#ifndef _SV_GEN_HXX //autogen wg. Size
#include <tools/gen.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX //autogen
#include <svx/svxenum.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _CALBCK_HXX
#include <calbck.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>       // Fuer die inline-ASSERTs
#endif
#ifndef _SWERROR_H
#include <error.h>          // Fuer die inline-ASSERTs
#endif


class Font;
class SvxBrushItem;
class SvxNumberFormat;
class SvxNumRule;
class SwCharFmt;
class SwDoc;
class SwFmtVertOrient;
class SwNodeNum;
class SwTxtNode;

extern char __FAR_DATA sOutlineStr[];   // SWG-Filter

const sal_Unicode cBulletChar   = 0xF000 + 149; // Charakter fuer Aufzaehlungen

inline BYTE GetRealLevel( const BYTE nLvl )
{
    return nLvl & (NO_NUMLEVEL - 1);
}

class SwNumType
{
    void GetRomanStr( ULONG nNo, String& rStr ) const;
    void GetCharStr( ULONG nNo, String& rStr ) const;
    void GetCharStrN( ULONG nNo, String& rStr ) const;
public:
        /* Reihenfolge der Elemente des enums ist fuer die
         * UI-Seite interesssant -- bitte nicht ohne trifftigen
         * Grund und Nachricht an die UI-Seite aendern.
         */
    SvxExtNumType     eType;

    SwNumType() { eType = SVX_NUM_ARABIC; }
    SwNumType( const SwNumType& rType ) : eType( rType.eType ) {}

    String GetNumStr( ULONG nNo ) const;
};


class SwNumFmt : public SwClient, public SwNumType
{
    String      aPrefix;                // vorangestellter Text
    String      aPostfix;               // nachfolgender Text

    // bei eType == USER_BITMAP ist folgendes ganz interessant
    Size        aGrfSize;
    SvxBrushItem* pGrfBrush;
    SwFmtVertOrient* pVertOrient;
    // bei eType == CHAR_SPECIAL ist folgendes ganz interessant
    Font*       pBulletFont;            // Pointer auf den BulletFont

    SvxAdjust   eNumAdjust;             // Ausrichtung (Links/rechts/zent.)
    short       nFirstLineOffset;       // Abstand zwischen Linken Rand und Text
    short       nLSpace;                // relativer linker Rand
    USHORT      nAbsLSpace;             // absoluter linker Rand
    USHORT      nCharTextOffset;        // Abstand zwischen Zeichen und Text
    USHORT      nStart;                 // Startwert fuer die Nummer

    BYTE        nInclUpperLevel;        // wieviele Levels
    sal_Unicode cBullet;                // das Bullet Char
    BOOL        bRelLSpace : 1;         // LSpace als relative Angabe (UI)

    void UpdateNumNodes( SwDoc* pDoc );
    DECL_STATIC_LINK( SwNumFmt, GraphicArrived, void * );

public:
    SwNumFmt();
    SwNumFmt( const SwNumFmt& );
    SwNumFmt( const SvxNumberFormat&, SwDoc* pDoc);
    SvxNumberFormat MakeSvxFormat() const;
    virtual ~SwNumFmt();

    SwNumFmt& operator=( const SwNumFmt& );
    BOOL operator==( const SwNumFmt& ) const;
    BOOL operator!=( const SwNumFmt& r ) const { return !(*this == r); }

    SwCharFmt* GetCharFmt() const { return (SwCharFmt*)pRegisteredIn; }
    void SetCharFmt( SwCharFmt* );
    virtual void Modify( SfxPoolItem* pOld, SfxPoolItem* pNew );

    // ist der Font* == 0, wird der Font nicht gewechselt
    void SetBulletFont(const Font*);
    const Font* GetBulletFont() const           { return pBulletFont; }

    sal_Unicode GetBulletChar() const           { return cBullet; }
    void SetBulletChar( sal_Unicode c )         { cBullet = c; }

//------
// ALT
    BOOL IsInclUpperLevel() const               { return 1 < nInclUpperLevel; }
    void SetInclUpperLevel( BOOL b )            { nInclUpperLevel = b ? MAXLEVEL : 1; }
// ALT
//------
    BYTE GetUpperLevel() const                  { return nInclUpperLevel; }
    void SetUpperLevel( BYTE nValue )           { nInclUpperLevel = nValue; }

    BOOL IsRelLSpace() const                    { return bRelLSpace; }
    void SetRelLSpace( BOOL b )                 { bRelLSpace = b; }

    SvxAdjust GetAdjust() const                 { return eNumAdjust; }
    void SetAdjust( SvxAdjust eAdj )            { eNumAdjust = eAdj; }

    short GetLSpace() const                     { return nLSpace; }
    void SetLSpace( short n )                   { nLSpace = n; }

    USHORT GetAbsLSpace() const                 { return nAbsLSpace; }
    void SetAbsLSpace( USHORT n )               { nAbsLSpace = n; }

    short GetFirstLineOffset() const            { return nFirstLineOffset; }
    void SetFirstLineOffset( short n )          { nFirstLineOffset = n; }

    USHORT GetCharTextOffset() const            { return nCharTextOffset; }
    void SetCharTextOffset( USHORT n )          { nCharTextOffset = n; }

    USHORT GetStartValue() const                { return nStart; }
    void SetStartValue( USHORT n )              { nStart = n; }

    const String& GetPrefix() const             { return aPrefix; }
    void SetPrefix( const String& rS )          { aPrefix = rS; }

    const String& GetPostfix() const            { return aPostfix; }
    void SetPostfix( const String& rS )         { aPostfix = rS; }

    const SvxBrushItem* GetGrfBrush() const     { return pGrfBrush; }
    const SwFmtVertOrient* GetGrfOrient() const { return pVertOrient; }
    const Size& GetGrfSize() const              { return aGrfSize; }
    void SetGrfBrush( const SvxBrushItem* pGrfBr, const Size* pSz,
                        const SwFmtVertOrient* pVOrient );
    void SetGraphic( const String& rName );
    // Graphic ggfs. reinswappen
    const Graphic* GetGraphic() const;
    // kann fuer das Format ein Text erzeugt werden?
    inline BOOL IsTxtFmt() const;
};


enum SwNumRuleType { OUTLINE_RULE = 0, NUM_RULE = 1, RULE_END = 2 };
class SwNumRule
{
    friend void _FinitCore();

    static SwNumFmt* aBaseFmts [ RULE_END ][ MAXLEVEL ];
    static USHORT aDefNumIndents[ MAXLEVEL ];
    static USHORT nRefCount;
    static Font* pDefBulletFont;
    static char* pDefOutlineName;

    SwNumFmt* aFmts[ MAXLEVEL ];
    String sName;
    SwNumRuleType eRuleType;
    USHORT nPoolFmtId;      // Id-fuer "automatich" erzeugte NumRules
    USHORT nPoolHelpId;     // HelpId fuer diese Pool-Vorlage
    BYTE nPoolHlpFileId;    // FilePos ans Doc auf die Vorlagen-Hilfen
    BOOL bAutoRuleFlag : 1;
    BOOL bInvalidRuleFlag : 1;
    BOOL bContinusNum : 1;  // Fortlaufende Numerierung - ohne Ebenen
    BOOL bAbsSpaces : 1;    // die Ebenen repraesentieren absol. Einzuege

    static void _MakeDefBulletFont();

public:
    SwNumRule( const String& rNm, SwNumRuleType = NUM_RULE,
                BOOL bAutoFlg = TRUE );

    SwNumRule( const SwNumRule& );
    ~SwNumRule();

    SwNumRule& operator=( const SwNumRule& );
    BOOL operator==( const SwNumRule& ) const;
    BOOL operator!=( const SwNumRule& r ) const { return !(*this == r); }

    inline const SwNumFmt* GetNumFmt( USHORT i ) const;
    inline const SwNumFmt& Get( USHORT i ) const;
    void Set( USHORT i, const SwNumFmt* );
    void Set( USHORT i, const SwNumFmt& );

    String MakeNumString( const SwNodeNum&, BOOL bInclStrings = TRUE,
                            BOOL bOnlyArabic = FALSE ) const;

    inline unsigned char GetBulletChar( const SwNodeNum& ) const;
    inline const Font* GetBulletFont( const SwNodeNum& ) const;
    static inline const Font& GetDefBulletFont();

    static char* GetOutlineRuleName() { return pDefOutlineName; }

    static inline USHORT GetNumIndent( BYTE nLvl );
    static inline USHORT GetBullIndent( BYTE nLvl );

    SwNumRuleType GetRuleType() const           { return eRuleType; }
    void SetRuleType( SwNumRuleType eNew )      { eRuleType = eNew;
                                                  bInvalidRuleFlag = TRUE; }

    // eine Art Copy-Constructor, damit die Num-Formate auch an den
    // richtigen CharFormaten eines Dokumentes haengen !!
    // (Kopiert die NumFormate und returnt sich selbst)
    SwNumRule& CopyNumRule( SwDoc*, const SwNumRule& );

    // testet ob die CharFormate aus dem angegeben Doc sind und kopiert
    // die gegebenfalls
    void CheckCharFmts( SwDoc* pDoc );

    // test ob der Einzug von dieser Numerierung kommt.
    BOOL IsRuleLSpace( SwTxtNode& rNd ) const;

    const String& GetName() const       { return sName; }
    void SetName( const String& rNm )   { sName = rNm; }

    BOOL IsAutoRule() const             { return bAutoRuleFlag; }
    void SetAutoRule( BOOL bFlag )      { bAutoRuleFlag = bFlag; }

    BOOL IsInvalidRule() const          { return bInvalidRuleFlag; }
    void SetInvalidRule( BOOL bFlag )   { bInvalidRuleFlag = bFlag; }

    BOOL IsContinusNum() const          { return bContinusNum; }
    void SetContinusNum( BOOL bFlag )   { bContinusNum = bFlag; }

    BOOL IsAbsSpaces() const            { return bAbsSpaces; }
    void SetAbsSpaces( BOOL bFlag )     { bAbsSpaces = bFlag; }

    // erfragen und setzen der Poolvorlagen-Id's
    USHORT GetPoolFmtId() const         { return nPoolFmtId; }
    void SetPoolFmtId( USHORT nId )     { nPoolFmtId = nId; }

    // erfragen und setzen der Hilfe-Id's fuer die Document-Vorlagen
    USHORT GetPoolHelpId() const        { return nPoolHelpId; }
    void SetPoolHelpId( USHORT nId )    { nPoolHelpId = nId; }
    BYTE GetPoolHlpFileId() const       { return nPoolHlpFileId; }
    void SetPoolHlpFileId( BYTE nId )   { nPoolHlpFileId = nId; }

    void        SetSvxRule(const SvxNumRule&, SwDoc* pDoc);
    SvxNumRule  MakeSvxNumRule() const;
};


class SwNodeNum
{
    USHORT nLevelVal[ MAXLEVEL ];       // Nummern aller Levels
    USHORT nSetValue;                   // vorgegeben Nummer
    BYTE nMyLevel;                      // akt. Level
    BOOL bStartNum;                     // Numerierung neu starten

public:
    inline SwNodeNum( BYTE nLevel = NO_NUM, USHORT nSetVal = USHRT_MAX );
    inline SwNodeNum& operator=( const SwNodeNum& rCpy );

    BOOL operator==( const SwNodeNum& ) const;

    BYTE GetLevel() const                   { return nMyLevel; }
    void SetLevel( BYTE nVal )              { nMyLevel = nVal; }

    BOOL IsStart() const                    { return bStartNum; }
    void SetStart( BOOL bFlag = TRUE )      { bStartNum = bFlag; }

    USHORT GetSetValue() const              { return nSetValue; }
    void SetSetValue( USHORT nVal )         { nSetValue = nVal; }

    const USHORT* GetLevelVal() const       { return nLevelVal; }
          USHORT* GetLevelVal()             { return nLevelVal; }
};




// ------------ inline Methoden ----------------------------

inline BOOL SwNumFmt::IsTxtFmt() const
{
    return SVX_NUM_NUMBER_NONE != eType &&
           SVX_NUM_CHAR_SPECIAL != eType &&
           SVX_NUM_BITMAP != eType;
}

inline const SwNumFmt& SwNumRule::Get( USHORT i ) const
{
    ASSERT_ID( i < MAXLEVEL && eRuleType < RULE_END, ERR_NUMLEVEL);
    return aFmts[ i ] ? *aFmts[ i ]
                      : *aBaseFmts[ eRuleType ][ i ];
}

inline const SwNumFmt* SwNumRule::GetNumFmt( USHORT i ) const
{
    ASSERT_ID( i < MAXLEVEL && eRuleType < RULE_END, ERR_NUMLEVEL);
    return aFmts[ i ];
}

inline const Font& SwNumRule::GetDefBulletFont()
{
    if( !pDefBulletFont )
        SwNumRule::_MakeDefBulletFont();
    return *pDefBulletFont;
}

inline USHORT SwNumRule::GetNumIndent( BYTE nLvl )
{
    ASSERT( MAXLEVEL > nLvl, "NumLevel is out of range" );
    return aDefNumIndents[ nLvl ];
}
inline USHORT SwNumRule::GetBullIndent( BYTE nLvl )
{
    ASSERT( MAXLEVEL > nLvl, "NumLevel is out of range" );
    return aDefNumIndents[ nLvl ];
}

inline unsigned char SwNumRule::GetBulletChar( const SwNodeNum& rNum ) const
{
    return Get( rNum.GetLevel() & ~NO_NUMLEVEL ).GetBulletChar();
}
inline const Font* SwNumRule::GetBulletFont( const SwNodeNum& rNum ) const
{
    return Get( rNum.GetLevel() & ~NO_NUMLEVEL ).GetBulletFont();
}



SwNodeNum::SwNodeNum( BYTE nLevel, USHORT nSetVal )
    : nMyLevel( nLevel ), nSetValue( nSetVal ), bStartNum( FALSE )
{
    memset( nLevelVal, 0, sizeof( nLevelVal ) );
}

inline SwNodeNum& SwNodeNum::operator=( const SwNodeNum& rCpy )
{
    nSetValue = rCpy.nSetValue;
    nMyLevel = rCpy.nMyLevel;
    bStartNum = rCpy.bStartNum;

    memcpy( nLevelVal, rCpy.nLevelVal, sizeof( nLevelVal ) );
    return *this;
}


#endif  // _NUMRULE_HXX
