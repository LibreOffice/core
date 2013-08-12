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
#ifndef _NUMRULE_HXX
#define _NUMRULE_HXX

#include <sal/types.h>
#include <tools/string.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/numitem.hxx>
#include "swdllapi.h"
#include <swtypes.hxx>
#include <calbck.hxx>
#include <hints.hxx>
#include <boost/unordered_map.hpp>
#include <stringhash.hxx>
#include <SwNumberTreeTypes.hxx>
#include <vector>

class SwTxtFmtColl;
class IDocumentListsAccess;
class SwNodeNum;
class Font;
class SvxBrushItem;
class SvxNumRule;
class SwCharFmt;
class SwDoc;
class SwFmtVertOrient;
class SwTxtNode;
class Size;

const sal_Unicode cBulletChar = 0x2022; ///< Character for lists.

class SW_DLLPUBLIC SwNumFmt : public SvxNumberFormat, public SwClient
{
    SwFmtVertOrient* pVertOrient;
    //For i120928,record the cp info of graphic within bullet
    sal_Unicode     cGrfBulletCP;
    SW_DLLPRIVATE void UpdateNumNodes( SwDoc* pDoc );
    SW_DLLPRIVATE virtual void NotifyGraphicArrived();

    using SvxNumberFormat::operator ==;
    using SvxNumberFormat::operator !=;

protected:
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew );

public:
    SwNumFmt();
    SwNumFmt( const SwNumFmt& );
    SwNumFmt( const SvxNumberFormat&, SwDoc* pDoc);

    virtual ~SwNumFmt();

    SwNumFmt& operator=( const SwNumFmt& );

    sal_Bool operator==( const SwNumFmt& ) const;
    sal_Bool operator!=( const SwNumFmt& r ) const { return !(*this == r); }

    SwCharFmt* GetCharFmt() const { return (SwCharFmt*)GetRegisteredIn(); }
    void SetCharFmt( SwCharFmt* );

    virtual void            SetCharFmtName(const String& rSet);
    virtual OUString        GetCharFmtName()const;

    //For i120928,access the cp info of graphic within bullet
    void            SetGrfBulletCP(sal_Unicode cP){cGrfBulletCP = cP;}
    sal_Unicode     GetGrfBulletCP()const {return cGrfBulletCP;}

    virtual void    SetGraphicBrush( const SvxBrushItem* pBrushItem, const Size* pSize = 0, const sal_Int16* pOrient = 0);

    virtual void                SetVertOrient(sal_Int16 eSet);
    virtual sal_Int16   GetVertOrient() const;
    const SwFmtVertOrient*      GetGraphicOrientation() const;

    sal_Bool IsEnumeration() const; // #i22362#
    sal_Bool IsItemize() const; // #i29560#
};

class SwPaM;
enum SwNumRuleType { OUTLINE_RULE = 0, NUM_RULE = 1, RULE_END = 2 };
class SW_DLLPUBLIC SwNumRule
{

public:
    typedef std::vector< SwTxtNode* > tTxtNodeList;
    typedef std::vector< SwTxtFmtColl* > tParagraphStyleList;

    struct Extremities
    {
        sal_uInt16 nPrefixChars;
        sal_uInt16 nSuffixChars;
    };

private:
    friend void _FinitCore();

    static SwNumFmt* aBaseFmts [ RULE_END ][ MAXLEVEL ];
    static const sal_uInt16 aDefNumIndents[ MAXLEVEL ];
    /// default list level properties for position-and-space mode LABEL_ALIGNMENT
    static SwNumFmt* aLabelAlignmentBaseFmts [ RULE_END ][ MAXLEVEL ];
    static sal_uInt16 nRefCount;
    static char* pDefOutlineName;

    SwNumFmt* aFmts[ MAXLEVEL ];

    /** container for associated text nodes */
    tTxtNodeList maTxtNodeList;

    /** container for associated paragraph styles */
    tParagraphStyleList maParagraphStyleList;

    /** boost::unordered_map containing "name->rule" relation */
    boost::unordered_map<String, SwNumRule *, StringHash> * pNumRuleMap;

    String sName;
    SwNumRuleType eRuleType;
    sal_uInt16 nPoolFmtId;      ///< Id-for NumRules created "automatically"
    sal_uInt16 nPoolHelpId;     ///< HelpId for this Pool-style.
    sal_uInt8 nPoolHlpFileId;   ///< FilePos at Doc on style helps.
    sal_Bool bAutoRuleFlag : 1;
    sal_Bool bInvalidRuleFlag : 1;
    sal_Bool bContinusNum : 1;  ///< Continuous numbering without levels.
    sal_Bool bAbsSpaces : 1;    ///< Levels represent absolute indents.
    sal_Bool bHidden : 1;       ///< Is the numering rule to be hidden in the UI?
    bool mbCountPhantoms;

    const SvxNumberFormat::SvxNumPositionAndSpaceMode meDefaultNumberFormatPositionAndSpaceMode;
    String msDefaultListId;

public:
    /// add parameter <eDefaultNumberFormatPositionAndSpaceMode>
    SwNumRule( const String& rNm,
               const SvxNumberFormat::SvxNumPositionAndSpaceMode eDefaultNumberFormatPositionAndSpaceMode,
               SwNumRuleType = NUM_RULE,
               sal_Bool bAutoFlg = sal_True );

    SwNumRule( const SwNumRule& );
    ~SwNumRule();

    SwNumRule& operator=( const SwNumRule& );
    sal_Bool operator==( const SwNumRule& ) const;
    sal_Bool operator!=( const SwNumRule& r ) const { return !(*this == r); }

    const SwNumFmt* GetNumFmt( sal_uInt16 i ) const;
    const SwNumFmt& Get( sal_uInt16 i ) const;

    sal_Bool IsHidden( ) const { return bHidden; }
    void SetHidden( sal_Bool bValue ) { bHidden = bValue; }

    void Set( sal_uInt16 i, const SwNumFmt* );
    void Set( sal_uInt16 i, const SwNumFmt& );
    String MakeNumString( const SwNodeNum&, sal_Bool bInclStrings = sal_True,
                            sal_Bool bOnlyArabic = sal_False ) const;
    /** - add optional parameter <_nRestrictToThisLevel> in order to
         restrict returned string to this level. */
    String MakeNumString( const SwNumberTree::tNumberVector & rNumVector,
                          const sal_Bool bInclStrings = sal_True,
                          const sal_Bool bOnlyArabic = sal_False,
                          const unsigned int _nRestrictToThisLevel = MAXLEVEL,
                          Extremities* pExtremities = 0 ) const;
    String MakeRefNumString( const SwNodeNum& rNodeNum,
                             const bool bInclSuperiorNumLabels = false,
                             const sal_uInt8 nRestrictInclToThisLevel = 0 ) const;

    /**

       @return list of associated text nodes
    */
    void GetTxtNodeList( SwNumRule::tTxtNodeList& rTxtNodeList ) const;
    SwNumRule::tTxtNodeList::size_type GetTxtNodeListSize() const;

    void AddTxtNode( SwTxtNode& rTxtNode );
    void RemoveTxtNode( SwTxtNode& rTxtNode );

    SwNumRule::tParagraphStyleList::size_type GetParagraphStyleListSize() const;
    void AddParagraphStyle( SwTxtFmtColl& rTxtFmtColl );
    void RemoveParagraphStyle( SwTxtFmtColl& rTxtFmtColl );

    inline void SetDefaultListId( const String sDefaultListId )
    {
        msDefaultListId = sDefaultListId;
    }
    inline String GetDefaultListId() const
    {
        return msDefaultListId;
    }
    /**
       Register this rule in a "name->numrule" map.

       @param pNumRuleMap      map to register in
     */
    void SetNumRuleMap(
                boost::unordered_map<String, SwNumRule *, StringHash>* pNumRuleMap );

    static char* GetOutlineRuleName() { return pDefOutlineName; }

    static sal_uInt16 GetNumIndent( sal_uInt8 nLvl );
    static sal_uInt16 GetBullIndent( sal_uInt8 nLvl );

    SwNumRuleType GetRuleType() const           { return eRuleType; }
    void SetRuleType( SwNumRuleType eNew )      { eRuleType = eNew;
                                                  bInvalidRuleFlag = sal_True; }

    /** A kind of copy-constructor to make sure the num formats are
       attached to the correctCharFormats of a document!!
       (Copies the NumFormats and returns itself). */
    SwNumRule& CopyNumRule( SwDoc*, const SwNumRule& );

    /** Tests whether the CharFormats are from the given doc
       and copies them if appropriate. */
    void CheckCharFmts( SwDoc* pDoc );

    const String& GetName() const       { return sName; }

    void SetName( const String& rNm,
                  IDocumentListsAccess& rDocListAccess );

    sal_Bool IsAutoRule() const             { return bAutoRuleFlag; }
    void SetAutoRule( sal_Bool bFlag )      { bAutoRuleFlag = bFlag; }

    sal_Bool IsInvalidRule() const          { return bInvalidRuleFlag; }
    void SetInvalidRule( sal_Bool bFlag );

    sal_Bool IsContinusNum() const          { return bContinusNum; }
    void SetContinusNum( sal_Bool bFlag )   { bContinusNum = bFlag; }

    sal_Bool IsAbsSpaces() const            { return bAbsSpaces; }
    void SetAbsSpaces( sal_Bool bFlag )     { bAbsSpaces = bFlag; }

    sal_Bool IsOutlineRule() const { return eRuleType == OUTLINE_RULE; }

    bool IsCountPhantoms() const;
    void SetCountPhantoms(bool bCountPhantoms);

    /// Query and set PoolFormat IDs.
    sal_uInt16 GetPoolFmtId() const         { return nPoolFmtId; }
    void SetPoolFmtId( sal_uInt16 nId )     { nPoolFmtId = nId; }

    /// Query and set Help-IDs for document styles.
    sal_uInt16 GetPoolHelpId() const        { return nPoolHelpId; }
    void SetPoolHelpId( sal_uInt16 nId )    { nPoolHelpId = nId; }
    sal_uInt8 GetPoolHlpFileId() const      { return nPoolHlpFileId; }
    void SetPoolHlpFileId( sal_uInt8 nId )  { nPoolHlpFileId = nId; }

    void        SetSvxRule(const SvxNumRule&, SwDoc* pDoc);
    SvxNumRule  MakeSvxNumRule() const;

    /// change indent of all list levels by given difference
    void ChangeIndent( const short nDiff );
    /// set indent of certain list level to given value
    void SetIndent( const short nNewIndent,
                    const sal_uInt16 nListLevel );
    /** set indent of first list level to given value and change other list level's
       indents accordingly */
    void SetIndentOfFirstListLevelAndChangeOthers( const short nNewIndent );

    void Validate();
};

/// namespace for static functions and methods for numbering and bullets
namespace numfunc
{
    /** retrieve font family name used for the default bullet list characters

        @author OD
    */
    const String& GetDefBulletFontname();

    /** determine if default bullet font is user defined

        The default bullet font is user defined, if it is given in the user configuration

        @author OD
    */
    bool IsDefBulletFontUserDefined();

    /** retrieve font used for the default bullet list characters

        @author OD
    */
    SW_DLLPUBLIC const Font& GetDefBulletFont();

    /** retrieve unicode of character used for the default bullet list for the given list level

        @author OD
    */
    sal_Unicode GetBulletChar( sal_uInt8 nLevel );

    /** configuration, if at first position of the first list item the <TAB>-key
        increased the indent of the complete list or only demotes this list item.
        The same for <SHIFT-TAB>-key at the same position for decreasing the
        indent of the complete list or only promotes this list item.

        @author OD
    */
    sal_Bool ChangeIndentOnTabAtFirstPosOfFirstListItem();

    /**
        @author OD
    */
    SvxNumberFormat::SvxNumPositionAndSpaceMode GetDefaultPositionAndSpaceMode();
}

#endif  // _NUMRULE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
