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
#ifndef _NUMRULE_HXX
#define _NUMRULE_HXX


#include <tools/link.hxx>
#include <tools/gen.hxx>
#include <tools/string.hxx>
#include <bf_svx/svxenum.hxx>

#include <swtypes.hxx>
#include <calbck.hxx>
#include <errhdl.hxx> 		// Fuer die inline-ASSERTs
#include <error.h>			// Fuer die inline-ASSERTs
#include <bf_svx/numitem.hxx>
class Font; 
namespace binfilter {


class SvxBrushItem;
class SvxNumRule;
class SwCharFmt;
class SwDoc;
class SwFmtVertOrient;
class SwNodeNum;
class SwTxtNode;

extern char __FAR_DATA sOutlineStr[];	// SWG-Filter

inline BYTE GetRealLevel( const BYTE nLvl )
{
    return nLvl & (NO_NUMLEVEL - 1);
}
const sal_Unicode cBulletChar	= 0x2022;	// Charakter fuer Aufzaehlungen

class SwNumFmt : public SvxNumberFormat, public SwClient
{
    SwFmtVertOrient* pVertOrient;

    void UpdateNumNodes( SwDoc* pDoc );
    virtual void NotifyGraphicArrived();
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

    virtual void			SetCharFmtName(const String& rSet);
    virtual const String&	GetCharFmtName()const;

    virtual void	SetGraphicBrush( const SvxBrushItem* pBrushItem, const Size* pSize = 0, const SvxFrameVertOrient* pOrient = 0);

    virtual void				SetVertOrient(SvxFrameVertOrient eSet);
    virtual SvxFrameVertOrient 	GetVertOrient() const;
    const SwFmtVertOrient*      GetGraphicOrientation() const;
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
    USHORT nPoolFmtId;		// Id-fuer "automatich" erzeugte NumRules
    USHORT nPoolHelpId;		// HelpId fuer diese Pool-Vorlage
    BYTE nPoolHlpFileId; 	// FilePos ans Doc auf die Vorlagen-Hilfen
    BOOL bAutoRuleFlag : 1;
    BOOL bInvalidRuleFlag : 1;
    BOOL bContinusNum : 1;	// Fortlaufende Numerierung - ohne Ebenen
    BOOL bAbsSpaces : 1;	// die Ebenen repraesentieren absol. Einzuege

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

    SwNumRuleType GetRuleType() const 			{ return eRuleType; }
    void SetRuleType( SwNumRuleType eNew ) 		{ eRuleType = eNew;
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

    const String& GetName() const 		{ return sName; }
    void SetName( const String& rNm )	{ sName = rNm; }

    BOOL IsAutoRule() const 			{ return bAutoRuleFlag; }
    void SetAutoRule( BOOL bFlag )		{ bAutoRuleFlag = bFlag; }

    BOOL IsInvalidRule() const 			{ return bInvalidRuleFlag; }
    void SetInvalidRule( BOOL bFlag )	{ bInvalidRuleFlag = bFlag; }

    BOOL IsContinusNum() const 			{ return bContinusNum; }
    void SetContinusNum( BOOL bFlag )	{ bContinusNum = bFlag; }

    BOOL IsAbsSpaces() const 			{ return bAbsSpaces; }
    void SetAbsSpaces( BOOL bFlag )		{ bAbsSpaces = bFlag; }

    // erfragen und setzen der Poolvorlagen-Id's
    USHORT GetPoolFmtId() const			{ return nPoolFmtId; }
    void SetPoolFmtId( USHORT nId ) 	{ nPoolFmtId = nId; }

    // erfragen und setzen der Hilfe-Id's fuer die Document-Vorlagen
    USHORT GetPoolHelpId() const 		{ return nPoolHelpId; }
    void SetPoolHelpId( USHORT nId ) 	{ nPoolHelpId = nId; }
    BYTE GetPoolHlpFileId() const 		{ return nPoolHlpFileId; }
    void SetPoolHlpFileId( BYTE nId ) 	{ nPoolHlpFileId = nId; }

    /**  
        #109308# Sets adjustment in all formats of the numbering rule. 

        @param eNum adjustment to be set
    */
    void SetNumAdjust(SvxAdjust eNum);

    void		SetSvxRule(const SvxNumRule&, SwDoc* pDoc);
    SvxNumRule	MakeSvxNumRule() const;
};


class SwNodeNum
{
    USHORT nLevelVal[ MAXLEVEL ];		// Nummern aller Levels
    USHORT nSetValue;					// vorgegeben Nummer
    BYTE nMyLevel;						// akt. Level
    BOOL bStartNum;						// Numerierung neu starten

public:
    inline SwNodeNum( BYTE nLevel = NO_NUM, USHORT nSetVal = USHRT_MAX );
    inline SwNodeNum& operator=( const SwNodeNum& rCpy );

    BOOL operator==( const SwNodeNum& ) const;

    BYTE GetLevel() const 					{ return nMyLevel; }
    void SetLevel( BYTE nVal )  			{ nMyLevel = nVal; }

    BOOL IsStart() const					{ return bStartNum; }
    void SetStart( BOOL bFlag = TRUE ) 		{ bStartNum = bFlag; }

    USHORT GetSetValue() const 				{ return nSetValue; }
    void SetSetValue( USHORT nVal )  		{ nSetValue = nVal; }

    const USHORT* GetLevelVal() const 		{ return nLevelVal; }
          USHORT* GetLevelVal() 	 		{ return nLevelVal; }
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
    : nSetValue( nSetVal ), nMyLevel( nLevel ), bStartNum( FALSE )
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


} //namespace binfilter
#endif	// _NUMRULE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
