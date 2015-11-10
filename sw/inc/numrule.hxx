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
#ifndef INCLUDED_SW_INC_NUMRULE_HXX
#define INCLUDED_SW_INC_NUMRULE_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/numitem.hxx>
#include "swdllapi.h"
#include <swtypes.hxx>
#include <calbck.hxx>
#include <hints.hxx>
#include <SwNumberTreeTypes.hxx>
#include <ndarr.hxx>
#include <unordered_map>
#include <vector>
#include <charfmt.hxx>

class SwTextFormatColl;
class IDocumentListsAccess;
class SwNodeNum;
namespace vcl { class Font; }
class SvxBrushItem;
class SfxGrabBagItem;
class SvxNumRule;
class SwDoc;
class SwFormatVertOrient;
class SwTextNode;
class Size;

const sal_Unicode cBulletChar = 0x2022; ///< Character for lists.

class SW_DLLPUBLIC SwNumFormat : public SvxNumberFormat, public SwClient
{
    SwFormatVertOrient* m_pVertOrient;
    //For i120928,record the cp info of graphic within bullet
    sal_Unicode     m_cGrfBulletCP;
    SAL_DLLPRIVATE void UpdateNumNodes( SwDoc* pDoc );

    using SvxNumberFormat::operator ==;
    using SvxNumberFormat::operator !=;

protected:
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew ) override;

public:
    SwNumFormat();
    SwNumFormat( const SwNumFormat& );
    SwNumFormat( const SvxNumberFormat&, SwDoc* pDoc);

    virtual ~SwNumFormat();

    SwNumFormat& operator=( const SwNumFormat& );

    bool operator==( const SwNumFormat& ) const;
    bool operator!=( const SwNumFormat& r ) const { return !(*this == r); }

    SwCharFormat* GetCharFormat() const { return const_cast<SwCharFormat*>(static_cast<const SwCharFormat*>(GetRegisteredIn())); }
    void       SetCharFormat( SwCharFormat* );

    void                    SetCharFormatName(const OUString& rSet);
    virtual OUString        GetCharFormatName() const override;

    //For i120928,access the cp info of graphic within bullet
    void            SetGrfBulletCP(sal_Unicode cP){m_cGrfBulletCP = cP;}
    sal_Unicode     GetGrfBulletCP() const {return m_cGrfBulletCP;}

    virtual void    SetGraphicBrush( const SvxBrushItem* pBrushItem, const Size* pSize = nullptr, const sal_Int16* pOrient = nullptr) override;

    virtual void                SetVertOrient(sal_Int16 eSet) override;
    virtual sal_Int16   GetVertOrient() const override;
    const SwFormatVertOrient*      GetGraphicOrientation() const;

    bool IsEnumeration() const; // #i22362#
    bool IsItemize() const; // #i29560#
};

class SwPaM;
enum SwNumRuleType { OUTLINE_RULE = 0, NUM_RULE = 1, RULE_END = 2 };
class SW_DLLPUBLIC SwNumRule
{

public:
    typedef std::vector< SwTextNode* > tTextNodeList;
    typedef std::vector< SwTextFormatColl* > tParagraphStyleList;

    struct Extremities
    {
        sal_uInt16 nPrefixChars;
        sal_uInt16 nSuffixChars;
    };

private:
    friend void _FinitCore();

    static SwNumFormat* maBaseFormats [ RULE_END ][ MAXLEVEL ];
    static const sal_uInt16 maDefNumIndents[ MAXLEVEL ];
    /// default list level properties for position-and-space mode LABEL_ALIGNMENT
    static SwNumFormat* maLabelAlignmentBaseFormats [ RULE_END ][ MAXLEVEL ];
    static sal_uInt16 mnRefCount;

    SwNumFormat* maFormats[ MAXLEVEL ];

    /** container for associated text nodes */
    tTextNodeList maTextNodeList;

    /** container for associated paragraph styles */
    tParagraphStyleList maParagraphStyleList;

    /** unordered_map containing "name->rule" relation */
    std::unordered_map<OUString, SwNumRule *, OUStringHash> * mpNumRuleMap;

    OUString msName;
    SwNumRuleType meRuleType;
    sal_uInt16 mnPoolFormatId;      ///< Id-for NumRules created "automatically"
    sal_uInt16 mnPoolHelpId;     ///< HelpId for this Pool-style.
    sal_uInt8 mnPoolHlpFileId;   ///< FilePos at Doc on style helps.
    bool mbAutoRuleFlag : 1;
    bool mbInvalidRuleFlag : 1;
    bool mbContinusNum : 1;  ///< Continuous numbering without levels.
    bool mbAbsSpaces : 1;    ///< Levels represent absolute indents.
    bool mbHidden : 1;       ///< Is the numering rule to be hidden in the UI?
    bool mbCountPhantoms;

    const SvxNumberFormat::SvxNumPositionAndSpaceMode meDefaultNumberFormatPositionAndSpaceMode;
    OUString msDefaultListId;
    std::shared_ptr<SfxGrabBagItem> mpGrabBagItem; ///< Style InteropGrabBag.

public:
    /// add parameter <eDefaultNumberFormatPositionAndSpaceMode>
    SwNumRule( const OUString& rNm,
               const SvxNumberFormat::SvxNumPositionAndSpaceMode eDefaultNumberFormatPositionAndSpaceMode,
               SwNumRuleType = NUM_RULE,
               bool bAutoFlg = true );

    SwNumRule( const SwNumRule& );
    ~SwNumRule();

    SwNumRule& operator=( const SwNumRule& );
    bool operator==( const SwNumRule& ) const;
    bool operator!=( const SwNumRule& r ) const { return !(*this == r); }

    const SwNumFormat* GetNumFormat( sal_uInt16 i ) const;
    const SwNumFormat& Get( sal_uInt16 i ) const;

    bool IsHidden( ) const { return mbHidden; }
    void SetHidden( bool bValue ) { mbHidden = bValue; }

    void Set( sal_uInt16 i, const SwNumFormat* );
    void Set( sal_uInt16 i, const SwNumFormat& );
    OUString MakeNumString( const SwNodeNum&, bool bInclStrings = true,
                            bool bOnlyArabic = false ) const;
    /** - add optional parameter <_nRestrictToThisLevel> in order to
         restrict returned string to this level. */
    OUString MakeNumString( const SwNumberTree::tNumberVector & rNumVector,
                          const bool bInclStrings = true,
                          const bool bOnlyArabic = false,
                          const unsigned int _nRestrictToThisLevel = MAXLEVEL,
                          Extremities* pExtremities = nullptr ) const;
    OUString MakeRefNumString( const SwNodeNum& rNodeNum,
                             const bool bInclSuperiorNumLabels = false,
                             const sal_uInt8 nRestrictInclToThisLevel = 0 ) const;

    /**

       @return list of associated text nodes
    */
    void GetTextNodeList( SwNumRule::tTextNodeList& rTextNodeList ) const;
    SwNumRule::tTextNodeList::size_type GetTextNodeListSize() const;

    void AddTextNode( SwTextNode& rTextNode );
    void RemoveTextNode( SwTextNode& rTextNode );

    SwNumRule::tParagraphStyleList::size_type GetParagraphStyleListSize() const;
    void AddParagraphStyle( SwTextFormatColl& rTextFormatColl );
    void RemoveParagraphStyle( SwTextFormatColl& rTextFormatColl );

    inline void SetDefaultListId( const OUString& sDefaultListId )
    {
        msDefaultListId = sDefaultListId;
    }
    inline OUString GetDefaultListId() const
    {
        return msDefaultListId;
    }
    /**
       Register this rule in a "name->numrule" map.

       @param pNumRuleMap      map to register in
     */
    void SetNumRuleMap(
                std::unordered_map<OUString, SwNumRule *, OUStringHash>* pNumRuleMap );

    static OUString GetOutlineRuleName();

    static sal_uInt16 GetNumIndent( sal_uInt8 nLvl );
    static sal_uInt16 GetBullIndent( sal_uInt8 nLvl );

    SwNumRuleType GetRuleType() const           { return meRuleType; }
    void SetRuleType( SwNumRuleType eNew )      { meRuleType = eNew;
                                                  mbInvalidRuleFlag = true; }

    /** A kind of copy-constructor to make sure the num formats are
       attached to the correctCharFormats of a document!!
       (Copies the NumFormats and returns itself). */
    SwNumRule& CopyNumRule( SwDoc*, const SwNumRule& );

    /** Tests whether the CharFormats are from the given doc
       and copies them if appropriate. */
    void CheckCharFormats( SwDoc* pDoc );

    OUString GetName() const { return msName; }

    void SetName( const OUString& rNm,
                  IDocumentListsAccess& rDocListAccess );

    bool IsAutoRule() const             { return mbAutoRuleFlag; }
    void SetAutoRule( bool bFlag )      { mbAutoRuleFlag = bFlag; }

    bool IsInvalidRule() const          { return mbInvalidRuleFlag; }
    void SetInvalidRule( bool bFlag );

    bool IsContinusNum() const          { return mbContinusNum; }
    void SetContinusNum( bool bFlag )   { mbContinusNum = bFlag; }

    bool IsAbsSpaces() const            { return mbAbsSpaces; }
    void SetAbsSpaces( bool bFlag )     { mbAbsSpaces = bFlag; }

    bool IsOutlineRule() const { return meRuleType == OUTLINE_RULE; }

    bool IsCountPhantoms() const        { return mbCountPhantoms; }
    void SetCountPhantoms(bool bCountPhantoms);

    /// Query and set PoolFormat IDs.
    sal_uInt16 GetPoolFormatId() const         { return mnPoolFormatId; }
    void SetPoolFormatId( sal_uInt16 nId )     { mnPoolFormatId = nId; }

    /// Query and set Help-IDs for document styles.
    sal_uInt16 GetPoolHelpId() const        { return mnPoolHelpId; }
    void SetPoolHelpId( sal_uInt16 nId )    { mnPoolHelpId = nId; }
    sal_uInt8 GetPoolHlpFileId() const      { return mnPoolHlpFileId; }
    void SetPoolHlpFileId( sal_uInt8 nId )  { mnPoolHlpFileId = nId; }

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
    void dumpAsXml(xmlTextWriterPtr w) const;
    void GetGrabBagItem(css::uno::Any& rVal) const;
    void SetGrabBagItem(const css::uno::Any& rVal);
};

/// namespace for static functions and methods for numbering and bullets
namespace numfunc
{
    /** retrieve font family name used for the default bullet list characters

        @author OD
    */
    OUString GetDefBulletFontname();

    /** determine if default bullet font is user defined

        The default bullet font is user defined, if it is given in the user configuration

        @author OD
    */
    bool IsDefBulletFontUserDefined();

    /** retrieve font used for the default bullet list characters

        @author OD
    */
    SW_DLLPUBLIC const vcl::Font& GetDefBulletFont();

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
    bool ChangeIndentOnTabAtFirstPosOfFirstListItem();

    /**
        @author OD
    */
    SvxNumberFormat::SvxNumPositionAndSpaceMode GetDefaultPositionAndSpaceMode();
}

#endif // INCLUDED_SW_INC_NUMRULE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
