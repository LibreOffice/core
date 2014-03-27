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
#ifndef INCLUDED_SW_INC_NDTXT_HXX
#define INCLUDED_SW_INC_NDTXT_HXX

#include <cppuhelper/weakref.hxx>

#include "swdllapi.h"
#include <node.hxx>
#include <hintids.hxx>
#include <ndhints.hxx>
#include <modeltoviewhelper.hxx>
#include <SwNumberTreeTypes.hxx>
#include <IDocumentContentOperations.hxx>

#include <sfx2/Metadatable.hxx>

#include <vector>
#include <set>

class SfxHint;
class SwNumRule;
class SwNodeNum;
class SwList;
class SvxLRSpaceItem;

namespace utl {
    class TransliterationWrapper;
}

class SwTxtFmtColl;
class SwCntntFrm;
class SwTxtFld;
class SwTxtInputFld;
class SfxItemSet;
class SwUndoTransliterate;
struct SwSpellArgs;
struct SwConversionArgs;
class SwInterHyphInfo;
class SwWrongList;
class SwGrammarMarkUp;
class OutputDevice;
class SwScriptInfo;
struct SwDocStat;
struct SwParaIdleData_Impl;

namespace com { namespace sun { namespace star {
    namespace uno {
        template < class > class Sequence;
    }
    namespace text { class XTextContent; }
} } }

typedef std::set< sal_Int32 > SwSoftPageBreakList;

/// SwTxtNode is a paragraph in the document model.
class SW_DLLPUBLIC SwTxtNode: public SwCntntNode, public ::sfx2::Metadatable
{

    /// For creating the first TextNode.
    friend class SwDoc;         ///< CTOR and AppendTxtNode()
    friend class SwNodes;
    friend class SwTxtFrm;
    friend class SwScriptInfo;

    /** May be 0. It is only then not 0 if it contains hard attributes.
       Therefore: never access directly! */
    SwpHints    *m_pSwpHints;

    mutable SwNodeNum* mpNodeNum;  ///< Numbering for this paragraph.

    OUString m_Text;

    SwParaIdleData_Impl* m_pParaIdleData_Impl;

    /** Some of the chars this para are hidden. Paragraph has to be reformatted
       on changing the view to print preview. */
    mutable bool m_bContainsHiddenChars : 1;
    /// The whole paragraph is hidden because of the hidden text attribute
    mutable bool m_bHiddenCharsHidePara : 1;
    /// The last two flags have to be recalculated if this flag is set:
    mutable bool m_bRecalcHiddenCharFlags : 1;

    mutable bool m_bLastOutlineState : 1;
    bool m_bNotifiable;

    bool mbEmptyListStyleSetDueToSetOutlineLevelAttr;

    /** boolean, indicating that a <SetAttr(..)> or <ResetAttr(..)> or
       <ResetAllAttr(..)> method is running.
       Needed to avoid duplicate handling of attribute change actions. */
    bool mbInSetOrResetAttr;

    /// pointer to the list, to whose the text node is added to
    SwList* mpList;

    ::std::auto_ptr< OUString > m_pNumStringCache;

    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::text::XTextContent> m_wXParagraph;

    SAL_DLLPRIVATE SwTxtNode( const SwNodeIndex &rWhere, SwTxtFmtColl *pTxtColl,
                             const SfxItemSet* pAutoAttr = 0 );

    /// Copies the attributes at nStart to pDest.
    SAL_DLLPRIVATE void CopyAttr( SwTxtNode *pDest, const sal_Int32 nStart, const sal_Int32 nOldPos);

    SAL_DLLPRIVATE SwTxtNode* _MakeNewTxtNode( const SwNodeIndex&, bool bNext = true,
                                bool bChgFollow = true );

    SAL_DLLPRIVATE void CutImpl(
          SwTxtNode * const pDest, const SwIndex & rDestStart,
          const SwIndex & rStart, /*const*/ sal_Int32 nLen,
          const bool bUpdate = true );

    /// Move all comprising hard attributes to the AttrSet of the paragraph.
    SAL_DLLPRIVATE void MoveTxtAttr_To_AttrSet();  // Called by SplitNode.

    /// Create the specific AttrSet.
    SAL_DLLPRIVATE virtual void NewAttrSet( SwAttrPool& ) SAL_OVERRIDE;

    /// Optimization: Asking for information about hidden characters at SwScriptInfo
    /// updates these flags.
    inline bool IsCalcHiddenCharFlags() const
        { return m_bRecalcHiddenCharFlags; }
    inline void SetHiddenCharAttribute( bool bNewHiddenCharsHidePara, bool bNewContainsHiddenChars ) const
    {
        m_bHiddenCharsHidePara = bNewHiddenCharsHidePara;
        m_bContainsHiddenChars = bNewContainsHiddenChars;
        m_bRecalcHiddenCharFlags = false;
    }

    SAL_DLLPRIVATE void CalcHiddenCharFlags() const;

    SAL_DLLPRIVATE SwNumRule * _GetNumRule(bool bInParent = true) const;

    SAL_DLLPRIVATE void SetLanguageAndFont( const SwPaM &rPaM,
            LanguageType nLang, sal_uInt16 nLangWhichId,
            const Font *pFont,  sal_uInt16 nFontWhichId );

    /// Start: Data collected during idle time

    SAL_DLLPRIVATE void SetParaNumberOfWords( sal_uLong nTmpWords ) const;
    SAL_DLLPRIVATE sal_uLong GetParaNumberOfWords() const;
    SAL_DLLPRIVATE void SetParaNumberOfAsianWords( sal_uLong nTmpAsianWords ) const;
    SAL_DLLPRIVATE sal_uLong GetParaNumberOfAsianWords() const;
    SAL_DLLPRIVATE void SetParaNumberOfChars( sal_uLong nTmpChars ) const;
    SAL_DLLPRIVATE sal_uLong GetParaNumberOfChars() const;
    SAL_DLLPRIVATE void SetParaNumberOfCharsExcludingSpaces( sal_uLong nTmpChars ) const;
    SAL_DLLPRIVATE sal_uLong GetParaNumberOfCharsExcludingSpaces() const;
    SAL_DLLPRIVATE void InitSwParaStatistics( bool bNew );

    /** create number for this text node, if not already existing

        @return number of this node
    */
    SwNodeNum* CreateNum() const;

    inline void TryDeleteSwpHints();

    SAL_DLLPRIVATE void impl_FmtToTxtAttr(const SfxItemSet& i_rAttrSet);

    const SwTxtInputFld* GetOverlappingInputFld( const SwTxtAttr& rTxtAttr ) const;

public:
    bool IsWordCountDirty() const;
    bool IsWrongDirty() const;
    bool IsGrammarCheckDirty() const;
    bool IsSmartTagDirty() const;
    bool IsAutoCompleteWordDirty() const;
    void SetWordCountDirty( bool bNew ) const;
    void SetWrongDirty( bool bNew ) const;
    void SetGrammarCheckDirty( bool bNew ) const;
    void SetSmartTagDirty( bool bNew ) const;
    void SetAutoCompleteWordDirty( bool bNew ) const;
    void SetWrong( SwWrongList* pNew, bool bDelete = true );
    SwWrongList* GetWrong();
    const SwWrongList* GetWrong() const;
    void SetGrammarCheck( SwGrammarMarkUp* pNew, bool bDelete = true );
    SwGrammarMarkUp* GetGrammarCheck();
    void SetSmartTags( SwWrongList* pNew, bool bDelete = true );
    SwWrongList* GetSmartTags();
    bool TryCharSetExpandToNum(const SfxItemSet& pCharSet);

    /// End: Data collected during idle time

protected:
    /// for hanging TxtFmtCollections somewhere else (Outline-Numbering!)
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) SAL_OVERRIDE;
    virtual void SwClientNotify( const SwModify&, const SfxHint& ) SAL_OVERRIDE;

public:
    using SwCntntNode::GetAttr;

    const OUString& GetTxt() const { return m_Text; }

    // returns the maximum number of characters that can still be added to the node
    inline sal_Int32 GetSpaceLeft() const;

    /// getters for SwpHints
    inline       SwpHints &GetSwpHints();
    inline const SwpHints &GetSwpHints() const;
    inline       SwpHints *GetpSwpHints()       { return m_pSwpHints; }
    inline const SwpHints *GetpSwpHints() const { return m_pSwpHints; }
    inline       bool   HasHints() const { return m_pSwpHints ? true : false; }
    inline       SwpHints &GetOrCreateSwpHints();

    virtual ~SwTxtNode();

    virtual sal_Int32 Len() const SAL_OVERRIDE;

    /// Is in itratr.
    void GetMinMaxSize( sal_uLong nIndex, sal_uLong& rMin, sal_uLong &rMax, sal_uLong &rAbs,
                        OutputDevice* pOut = 0 ) const;

    /// overriding to handle change of certain paragraph attributes
    virtual sal_Bool SetAttr( const SfxPoolItem& ) SAL_OVERRIDE;
    virtual sal_Bool SetAttr( const SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual sal_Bool ResetAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 = 0 ) SAL_OVERRIDE;
    virtual sal_Bool ResetAttr( const std::vector<sal_uInt16>& rWhichArr ) SAL_OVERRIDE;
    virtual sal_uInt16 ResetAllAttr() SAL_OVERRIDE;

    /// insert text content
    /// @param rStr text to insert; in case it does not fit into the capacity
    ///             of the node, the longest prefix that fits is inserted
    /// @return the prefix of rStr that was actually inserted
    OUString InsertText( const OUString & rStr, const SwIndex & rIdx,
                     const enum IDocumentContentOperations::InsertFlags nMode
                         = IDocumentContentOperations::INS_DEFAULT );

    /** delete text content
        ATTENTION: must not be called with a range that overlaps the start of
                   an attribute with both extent and dummy char
     */
    void EraseText ( const SwIndex &rIdx, const sal_Int32 nCount = SAL_MAX_INT32,
                     const enum IDocumentContentOperations::InsertFlags nMode
                         = IDocumentContentOperations::INS_DEFAULT );

    /** delete all attributes.
        If neither pSet nor nWhich is given, delete all attributes (except
        refmarks, toxmarks, meta) in range.
        @param rIdx     start position
        @param nLen     range in which attributes will be deleted
        @param pSet     if not 0, delete only attributes contained in pSet
        @param nWhich   if not 0, delete only attributes with matching which
        @param bInclRefToxMark
            refmarks, toxmarks, and metas will be ignored unless this is true
        ATTENTION: setting bInclRefToxMark is only allowed from UNDO!
     */
    void RstTxtAttr(
        const SwIndex &rIdx,
        const sal_Int32 nLen,
        const sal_uInt16 nWhich = 0,
        const SfxItemSet* pSet = 0,
        const bool bInclRefToxMark = false );
    void    GCAttr();

    // Delete text attribute (needs to be deregistered at Pool!)
    void    DestroyAttr( SwTxtAttr* pAttr );

    // delete all attributes from SwpHintsArray.
    void    ClearSwpHintsArr( bool bDelFields );

    /// Insert pAttr into hints array. @return true iff inserted successfully
    bool    InsertHint( SwTxtAttr * const pAttr,
                  const SetAttrMode nMode = nsSetAttrMode::SETATTR_DEFAULT );
    /// create new text attribute from rAttr and insert it
    /// @return     inserted hint; 0 if not sure the hint is inserted
    SwTxtAttr* InsertItem( SfxPoolItem& rAttr,
                  const sal_Int32 nStart, const sal_Int32 nEnd,
                  const SetAttrMode nMode = nsSetAttrMode::SETATTR_DEFAULT );

    /** Set these attributes at TextNode. If the whole range is comprised
       set them only in AutoAttrSet (SwCntntNode::SetAttr). */
    bool SetAttr( const SfxItemSet& rSet,
                  sal_Int32 nStt, sal_Int32 nEnd,
                  const SetAttrMode nMode = nsSetAttrMode::SETATTR_DEFAULT );
    /** Query the attributes of textnode over the range.
       Introduce 4th optional parameter <bMergeIndentValuesOfNumRule>.
       If <bMergeIndentValuesOfNumRule> == true, the indent attributes of
       the corresponding list level of an applied list style is merged into
       the requested item set as a LR-SPACE item, if <bOnlyTxtAttr> == false,
       corresponding node has not its own indent attributes and the
       position-and-space mode of the list level is SvxNumberFormat::LABEL_ALIGNMENT. */
    bool GetAttr( SfxItemSet& rSet, sal_Int32 nStt, sal_Int32 nEnd,
                  const bool bOnlyTxtAttr  = false,
                  const bool bGetFromChrFmt = true,
                  const bool bMergeIndentValuesOfNumRule = false ) const;

    /// Convey attributes of an AttrSet (AutoFmt) to SwpHintsArray.
    void FmtToTxtAttr( SwTxtNode* pNd );

    /// delete all attributes of type nWhich at nStart (opt. end nEnd)
    void DeleteAttributes( const sal_uInt16 nWhich,
                  const sal_Int32 nStart, const sal_Int32 nEnd = 0 );
    /// delete the attribute pTxtAttr
    void DeleteAttribute ( SwTxtAttr * const pTxtAttr );

    /** Actions on text and attributes.
       introduce optional parameter to control, if all attributes have to be copied. */
    void CopyText( SwTxtNode * const pDest,
               const SwIndex &rStart,
               const sal_Int32 nLen,
               const bool bForceCopyOfAllAttrs = false );
    void CopyText( SwTxtNode * const pDest,
               const SwIndex &rDestStart,
               const SwIndex &rStart,
               sal_Int32 nLen,
               const bool bForceCopyOfAllAttrs = false );

    void        CutText(SwTxtNode * const pDest,
                    const SwIndex & rStart, const sal_Int32 nLen);
    inline void CutText(SwTxtNode * const pDest, const SwIndex &rDestStart,
                    const SwIndex & rStart, const sal_Int32 nLen);

    /// replace nDelLen characters at rStart with rText
    /// in case the replacement does not fit, it is partially inserted up to
    /// the capacity of the node
    void ReplaceText( const SwIndex& rStart, const sal_Int32 nDelLen,
            const OUString & rText );
    void ReplaceTextOnly( sal_Int32 nPos, sal_Int32 nLen,
            const OUString& rText,
            const ::com::sun::star::uno::Sequence<sal_Int32>& rOffsets );

    /// Virtual methods from CntntNode.
    virtual SwCntntFrm *MakeFrm( SwFrm* ) SAL_OVERRIDE;
    virtual SwCntntNode *SplitCntntNode( const SwPosition & ) SAL_OVERRIDE;
    virtual SwCntntNode *JoinNext() SAL_OVERRIDE;
    virtual SwCntntNode *JoinPrev() SAL_OVERRIDE;

    SwCntntNode *AppendNode( const SwPosition & );

    /// When appropriate set DontExpand-flag at INet or character styles respectively.
    bool DontExpandFmt( const SwIndex& rIdx, bool bFlag = true,
                        bool bFmtToTxtAttributes = true );

    enum GetTxtAttrMode {
        DEFAULT,    /// DEFAULT: (Start <= nIndex <  End)
        EXPAND,     /// EXPAND : (Start <  nIndex <= End)
        PARENT,     /// PARENT : (Start <  nIndex <  End)
    };

    /** get the innermost text attribute covering position nIndex.
        @param nWhich   only attribute with this id is returned.
        @param eMode    the predicate for matching (@see GetTxtAttrMode).

        ATTENTION: this function is not well-defined for those
        hints of which several may cover a single position, like
        RES_TXTATR_CHARFMT, RES_TXTATR_REFMARK, RES_TXTATR_TOXMARK
     */
    SwTxtAttr *GetTxtAttrAt(
        sal_Int32 const nIndex,
        RES_TXTATR const nWhich,
        enum GetTxtAttrMode const eMode = DEFAULT ) const;

    /** get the innermost text attributes covering position nIndex.
        @param nWhich   only attributes with this id are returned.
        @param eMode    the predicate for matching (@see GetTxtAttrMode).
     */
    ::std::vector<SwTxtAttr *> GetTxtAttrsAt(
        sal_Int32 const nIndex,
        RES_TXTATR const nWhich,
        enum GetTxtAttrMode const eMode = DEFAULT ) const;

    /** get the text attribute at position nIndex which owns
        the dummy character CH_TXTATR_* at that position, if one exists.
        @param nIndex   the position in the text
        @param nWhich   if different from RES_TXTATR_END, return only
                        attribute with given which id
        @return the text attribute at nIndex of type nWhich, if it exists
    */
    SwTxtAttr *GetTxtAttrForCharAt(
        const sal_Int32 nIndex,
        const RES_TXTATR nWhich = RES_TXTATR_END ) const;

    SwTxtFld* GetFldTxtAttrAt(
        const sal_Int32 nIndex,
        const bool bIncludeInputFldAtStart = false ) const;

    OUString GetCurWord(sal_Int32) const;
    sal_uInt16 Spell(SwSpellArgs*);
    sal_uInt16 Convert( SwConversionArgs & );

    inline SwTxtFmtColl *GetTxtColl() const;
    virtual SwFmtColl *ChgFmtColl( SwFmtColl* ) SAL_OVERRIDE;
    void _ChgTxtCollUpdateNum( const SwTxtFmtColl* pOld,
                                const SwTxtFmtColl* pNew );

    /** Copy collection with all auto formats to dest-node.
        The latter might be in an other document!
       (Method in ndcopy.cxx!!). */
    void CopyCollFmt( SwTxtNode& rDestNd );

    // BEGIN OF BULLET/NUMBERING/OUTLINE STUFF:

    /**
       Returns numbering rule of this text node.

       @param bInParent     serach in parent attributes, too

       @return numbering rule of this text node or NULL if none is set
     */
    SwNumRule *GetNumRule(bool bInParent = true) const;

    inline const SwNodeNum* GetNum() const
    {
        return mpNodeNum;
    }

    SwNumberTree::tNumberVector GetNumberVector() const;

    /**
       Returns if this text node is an outline.

       @retval true      this text node is an outline
       @retval false     else
     */
    bool IsOutline() const;

    bool IsOutlineStateChanged() const;

    void UpdateOutlineState();

    /**
       Notify this textnode that its numbering rule has changed.
     */
    void NumRuleChgd();

    /** Returns outline of numbering string

        Introduce parameter <_bInclPrefixAndSuffixStrings> in order to control,
        if the prefix and the suffix strings have to been included or not.

        @param _bInclPrefixAndSuffixStrings
        optional input parameter - boolean indicating, if the prefix and the
        suffix strings have to been included or not. default value = <true>

        @param _nRestrictToThisLevel
        optional input parameter - unsigned integer indicating the maximum outline
        level to which the output string must be restricted to. Default value is
        MAXLEVEL
    */
    OUString GetNumString( const bool _bInclPrefixAndSuffixStrings = true,
            const unsigned int _nRestrictToThisLevel = MAXLEVEL ) const;

    /**
       Returns the additional indents of this text node and its numbering.

       @param bTxtLeft  ???

       @return additional indents
     */
     long GetLeftMarginWithNum( bool bTxtLeft = false ) const;

    /**
       Returns the combined first line indent of this text node and
       its numbering.

       @param the first line indent of this text node taking the
               numbering into account (return parameter)

       @retval true   this node has SwNodeNum and has numbering rule
       @retval false  else
     */
    bool GetFirstLineOfsWithNum( short& rFirstOffset ) const;

    SwTwips GetAdditionalIndentForStartingNewList() const;

    void ClearLRSpaceItemDueToListLevelIndents( SvxLRSpaceItem& o_rLRSpaceItem ) const;

    /** return left margin for tab stop position calculation

        Needed for text formatting
        Method considers new list level attributes, which also can provide a left margin value
    */
    long GetLeftMarginForTabCalculation() const;

    /** Returns if this text node has a number.

        This text node has a number if it has a SwNodeNum and a
        numbering rule and the numbering format specified for the
        level of the SwNodeNum is of an enumeration type.

        @retval true    This text node has a number.
        @retval false   else
     */
    bool HasNumber() const;

    /** Returns if this text node has a bullet.

        This text node has a bullet if it has a SwNodeNum and a
        numbering rule and the numbering format specified for the
        level of the SwNodeNum is of a bullet type.

        @retval true    This text node has a bullet.
        @retval false   else
     */
    bool HasBullet() const;

    /** Returns is this text node is numbered.

        This node is numbered if it has a SwNodeNum and it has a
        numbering rule and has not a hidden SwNodeNum.

        ATTENTION: Returns true even if the SwNumFmt has type
        SVX_NUM_NUMBER_NONE.

        @retval true      This node is numbered.
        @retval false     else
     */
    bool IsNumbered() const;

    /** Returns if this text node has a marked label.

        @retval true       This text node has a marked label.
        @retval false      else
     */
    bool HasMarkedLabel() const;

    /** Sets the list level of this text node.

        Side effect, when the text node is a list item:
        The text node's representation in the list tree (<SwNodeNum> instance)
        is updated.

        @param nLevel level to set
    */
    void SetAttrListLevel(int nLevel);

    bool HasAttrListLevel() const;

    int GetAttrListLevel() const;

    /** Returns the actual list level of this text node, when it is a list item

        @return the actual list level of this text node, if it is a list item,
               -1 otherwise
    */
    int GetActualListLevel() const;

    /**
       Returns outline level of this text node.

       If a text node has an outline number (i.e. it has an SwNodeNum
       and a outline numbering rule) the outline level is the level of
       this SwNodeNum.

       If a text node has no outline number and has a paragraph style
       attached the outline level is the outline level of the
       paragraph style.

       Otherwise the text node has no outline level (NO_NUMBERING).

       NOTE: The outline level of text nodes is subject to change. The
       plan is to have an SwTxtNode::nOutlineLevel member that is
       updated from a paragraph style upon appliance of that paragraph
       style.

       @return outline level or NO_NUMBERING if there is no outline level
     */
    int GetAttrOutlineLevel() const;

    /**
       Sets the out line level *at* a text node.

       @param nLevel     the level to be set

       If the text node has an outline number the level is set at the
       outline number.

       If the text node has no outline number but has a paragraph
       style applied the outline level is set at the paragraph style.

       NOTE: This is subject to change, see GetOutlineLevel.
     */
    void SetAttrOutlineLevel(int nLevel);

    bool IsEmptyListStyleDueToSetOutlineLevelAttr();
    void SetEmptyListStyleDueToSetOutlineLevelAttr();
    void ResetEmptyListStyleDueToResetOutlineLevelAttr();

    /**
       Returns the width of leading tabs/blanks in this paragraph.
       This space will be converted into numbering indent if the paragraph
       is set to be numbered.

       @return     the width of the leading whitespace
     */
    sal_uInt16 GetWidthOfLeadingTabs() const;

    /**
       Returns if the paragraph has a visible numbering or bullet.
       This includes all kinds of numbering/bullet/outlines.
       Note: This function returns false, if the numbering format is
       SVX_NUM_NUMBER_NONE or if the numbering/bullet has been deleted.

       @return     true if the paragraph has a visible numbering/bullet/outline
     */
    bool HasVisibleNumberingOrBullet() const;

    void SetListId(OUString const& rListId);
    OUString GetListId() const;

    /** Determines, if the list level indent attributes can be applied to the
        paragraph.

        The list level indents can be applied to the paragraph under the one
        of following conditions:
        - the list style is directly applied to the paragraph and the paragraph
          has no own indent attributes.
        - the list style is applied to the paragraph through one of its paragraph
          styles, the paragraph has no own indent attributes and on the paragraph
          style hierarchy from the paragraph to the paragraph style with the
          list style no indent attributes are found.

        @return boolean
    */
    bool AreListLevelIndentsApplicable() const;

    /** Retrieves the list tab stop position, if the paragraph's list level defines
        one and this list tab stop has to merged into the tap stops of the paragraph

        @param nListTabStopPosition
        output parameter - containing the list tab stop position

        @return boolean - indicating, if a list tab stop position is provided
    */
    bool GetListTabStopPosition( long& nListTabStopPosition ) const;

    /** Retrieves the character following the list label, if the paragraph's
        list level defines one.

        @return the list tab stop position as string
    */
    OUString GetLabelFollowedBy() const;

    // END OF BULLET/NUMBERING/OUTLINE STUFF:

    void fillSoftPageBreakList( SwSoftPageBreakList& rBreak ) const;

    sal_uInt16 GetLang( const sal_Int32 nBegin, const sal_Int32 nLen = 0,
                    sal_uInt16 nScript = 0 ) const;

    /// in ndcopy.cxx
    bool IsSymbol( const sal_Int32 nBegin ) const; // In itratr.cxx.
    virtual SwCntntNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const SAL_OVERRIDE;

    /// Interactive hyphenation: we find TxtFrm and call its CalcHyph.
    bool Hyphenate( SwInterHyphInfo &rHyphInf );
    void DelSoftHyph( const sal_Int32 nStart, const sal_Int32 nEnd );

    /** add 4th optional parameter <bAddSpaceAfterListLabelStr> indicating,
       when <bWithNum = true> that a space is inserted after the string for
       the list label.
       add 5th optional parameter <bWithSpacesForLevel> indicating, if additional
       spaces are inserted in front of the expanded text string depending on
       the list level. */
    OUString GetExpandTxt(  const sal_Int32 nIdx = 0,
                            const sal_Int32 nLen = -1,
                            const bool bWithNum = false,
                            const bool bAddSpaceAfterListLabelStr = false,
                            const bool bWithSpacesForLevel = false,
                            const bool bWithFtn = true ) const;
    bool GetExpandTxt( SwTxtNode& rDestNd, const SwIndex* pDestIdx = 0,
                           sal_Int32 nIdx = 0, sal_Int32 nLen = -1,
                           bool bWithNum = false, bool bWithFtn = true,
                           bool bReplaceTabsWithSpaces = false ) const;

    OUString GetRedlineTxt( sal_Int32 nIdx = 0,
                          sal_Int32 nLen = SAL_MAX_INT32,
                          bool bExpandFlds = false,
                          bool bWithNum = false ) const;

    /** @return actual count of initial chars for initial-function.
       If nWishLen == 0 that of first word. */
    sal_uInt16 GetDropLen( sal_uInt16 nWishLen) const;

    /// Passes back info needed on the dropcap dimensions
    bool GetDropSize(int& rFontHeight, int& rDropHeight, int& rDropDescent) const;

    /// Hidden Paragraph Field:
    inline bool CalcHiddenParaField()
        { return m_pSwpHints ? m_pSwpHints->CalcHiddenParaField() : false; }
    /// set CalcVisible flags
    inline void SetCalcHiddenParaField()
        { if (m_pSwpHints) m_pSwpHints->SetCalcHiddenParaField(); }

    /// is the paragraph visible?
    inline bool HasHiddenParaField() const
        { return m_pSwpHints ? m_pSwpHints->HasHiddenParaField()  : false; }

    /// Hidden Paragraph Field:

    inline bool HasHiddenCharAttribute( bool bWholePara ) const
    {
        if ( m_bRecalcHiddenCharFlags )
            CalcHiddenCharFlags();
        return bWholePara ? m_bHiddenCharsHidePara : m_bContainsHiddenChars;
    }

    inline void SetCalcHiddenCharFlags() const
        { m_bRecalcHiddenCharFlags = true; }

    /** @return if the node is hidden due to
       1. HiddenParaField
       2. HiddenCharAttribute
       3. HiddenSection */

    bool IsHidden() const;

    TYPEINFO_OVERRIDE(); // fuer rtti

    /// override SwIndexReg
    virtual void Update(
        SwIndex const & rPos,
        const sal_Int32 nChangeLen,
        const bool bNegative = false,
        const bool bDelete = false ) SAL_OVERRIDE;

    /// change text to Upper/Lower/Hiragana/Katagana/...
    void TransliterateText( utl::TransliterationWrapper& rTrans,
                            sal_Int32 nStart, sal_Int32 nEnd,
                            SwUndoTransliterate* pUndo = 0 );

    /// count words in given range - returns true if we refreshed out count
    bool CountWords( SwDocStat& rStat, sal_Int32 nStart, sal_Int32 nEnd ) const;

    /** Checks some global conditions like loading or destruction of document
       to economize notifications */
    bool IsNotificationEnabled() const;

    /// Checks a temporary notification blocker and the global conditons of IsNotificationEnabled()
    bool IsNotifiable() const;

    void SetListRestart( bool bRestart );
    bool IsListRestart() const;

    void SetAttrListRestartValue( SwNumberTree::tSwNumTreeNumber nNum );
    bool HasAttrListRestartValue() const;
    SwNumberTree::tSwNumTreeNumber GetAttrListRestartValue() const;
    SwNumberTree::tSwNumTreeNumber GetActualListStartValue() const;

    void SetCountedInList( bool bCounted );
    bool IsCountedInList() const;

    void AddToList();
    void RemoveFromList();
    bool IsInList() const;

    bool IsFirstOfNumRule() const;

    sal_uInt16 GetScalingOfSelectedText( sal_Int32 nStt, sal_Int32 nEnd ) const;

    SAL_DLLPRIVATE ::com::sun::star::uno::WeakReference<
        ::com::sun::star::text::XTextContent> const& GetXParagraph() const
            { return m_wXParagraph; }
    SAL_DLLPRIVATE void SetXParagraph(::com::sun::star::uno::Reference<
                    ::com::sun::star::text::XTextContent> const& xParagraph)
            { m_wXParagraph = xParagraph; }

    /// sfx2::Metadatable
    virtual ::sfx2::IXmlIdRegistry& GetRegistry() SAL_OVERRIDE;
    virtual bool IsInClipboard() const SAL_OVERRIDE;
    virtual bool IsInUndo() const SAL_OVERRIDE;
    virtual bool IsInContent() const SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::rdf::XMetadatable > MakeUnoObject() SAL_OVERRIDE;

    bool IsCollapse() const;

    virtual void dumpAsXml( xmlTextWriterPtr writer = NULL ) SAL_OVERRIDE;

    sal_uInt32 GetRsid( sal_Int32 nStt, sal_Int32 nEnd ) const;
    sal_uInt32 GetParRsid() const;

    bool CompareRsid( const SwTxtNode &rTxtNode, sal_Int32 nStt1, sal_Int32 nStt2,
            sal_Int32 nEnd1 = 0,  sal_Int32 nEnd2 = 0 ) const;
    bool CompareParRsid( const SwTxtNode &rTxtNode ) const;

    DECL_FIXEDMEMPOOL_NEWDEL(SwTxtNode)
};

inline SwpHints & SwTxtNode::GetSwpHints()
{
    assert( m_pSwpHints );
    return *m_pSwpHints;
}
inline const SwpHints &SwTxtNode::GetSwpHints() const
{
    assert( m_pSwpHints );
    return *m_pSwpHints;
}

inline SwpHints& SwTxtNode::GetOrCreateSwpHints()
{
    if ( !m_pSwpHints )
    {
        m_pSwpHints = new SwpHints;
    }
    return *m_pSwpHints;
}

inline void SwTxtNode::TryDeleteSwpHints()
{
    if ( m_pSwpHints && m_pSwpHints->CanBeDeleted() )
    {
        DELETEZ( m_pSwpHints );
    }
}

inline SwTxtFmtColl* SwTxtNode::GetTxtColl() const
{
    return static_cast<SwTxtFmtColl*>(const_cast<SwModify*>(GetRegisteredIn()));
}

/// Inline methods from Node.hxx
inline SwTxtNode *SwNode::GetTxtNode()
{
     return ND_TEXTNODE == nNodeType ? static_cast<SwTxtNode*>(this) : 0;
}

inline const SwTxtNode *SwNode::GetTxtNode() const
{
     return ND_TEXTNODE == nNodeType ? static_cast<const SwTxtNode*>(this) : 0;
}

inline void
SwTxtNode::CutText(SwTxtNode * const pDest, const SwIndex & rDestStart,
                    const SwIndex & rStart, const sal_Int32 nLen)
{
    CutImpl( pDest, rDestStart, rStart, nLen, true );
}

inline sal_Int32 SwTxtNode::GetSpaceLeft() const
{
    // do not fill the String up to the max - need to be able to have a
    // SwPosition "behind" the last character, i.e., at index TXTNODE_MAX + 1
    const sal_Int32 TXTNODE_MAX = SAL_MAX_INT32 - 2;
    return TXTNODE_MAX-m_Text.getLength();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
