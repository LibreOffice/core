/*************************************************************************
 *
 *  $RCSfile: numrule.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2001-02-23 12:45:01 $
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
#ifndef _SVX_NUMITEM_HXX
#include <svx/numitem.hxx>
#endif

class Font;
class SvxBrushItem;
class SvxNumRule;
class SwCharFmt;
class SwDoc;
class SwFmtVertOrient;
class SwNodeNum;
class SwTxtNode;

extern char __FAR_DATA sOutlineStr[];   // SWG-Filter

inline BYTE GetRealLevel( const BYTE nLvl )
{
    return nLvl & (NO_NUMLEVEL - 1);
}
const sal_Unicode cBulletChar   = 0xF000 + 149; // Charakter fuer Aufzaehlungen

class SwNumFmt : public SvxNumberFormat, public SwClient
{
    SwFmtVertOrient* pVertOrient;

    void UpdateNumNodes( SwDoc* pDoc );
public:
    SwNumFmt();
    SwNumFmt( const SwNumFmt& );
    SwNumFmt( const SvxNumberFormat&, SwDoc* pDoc);

    virtual ~SwNumFmt();

    SwNumFmt& operator=( const SwNumFmt& );
    BOOL operator==( const SwNumFmt& ) const;
    BOOL operator!=( const SwNumFmt& r ) const { return !(*this == r); }

    const Graphic* GetGraphic() const;

    SwCharFmt* GetCharFmt() const { return (SwCharFmt*)pRegisteredIn; }
    void SetCharFmt( SwCharFmt* );
    virtual void Modify( SfxPoolItem* pOld, SfxPoolItem* pNew );

    virtual void            SetCharFmtName(const String& rSet);
    virtual const String&   GetCharFmtName()const;

    virtual void    SetGraphicBrush( const SvxBrushItem* pBrushItem, const Size* pSize = 0, const SvxFrameVertOrient* pOrient = 0);

    virtual void                SetVertOrient(SvxFrameVertOrient eSet);
    virtual SvxFrameVertOrient  GetVertOrient() const;
    const SwFmtVertOrient*      GetGraphicOrientation() const { return pVertOrient; }
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

    inline sal_Unicode GetBulletChar( const SwNodeNum& ) const;
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

inline sal_Unicode SwNumRule::GetBulletChar( const SwNodeNum& rNum ) const
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
