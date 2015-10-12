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

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESS_ACCPORTIONS_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESS_ACCPORTIONS_HXX

#include <SwPortionHandler.hxx>
#include <sal/types.h>
#include <rtl/ustrbuf.hxx>
#include <vector>

class SwTextNode;
struct SwSpecialPos;
class SwViewOption;
namespace com { namespace sun { namespace star {
    namespace i18n { struct Boundary; }
} } }

/**
 * collect text portion data from the layout through SwPortionHandler interface
 */
class SwAccessiblePortionData : public SwPortionHandler
{
    // the node this portion is referring to
    const SwTextNode* pTextNode;

    // variables used while collecting the data
    OUStringBuffer aBuffer;
    sal_Int32 nModelPosition;
    const SwViewOption* pViewOptions;

    // the accessible string
    OUString sAccessibleString;

    // positions array
    // instances of Position_t must always include the minimum and
    // maximum positions as first/last elements (to simplify the
    // algorithms)
    typedef std::vector<sal_Int32> Positions_t;

    Positions_t aLineBreaks;        /// position of line breaks
    Positions_t aModelPositions;    /// position of portion breaks in the model
    Positions_t aAccessiblePositions;   /// portion breaks in sAccessibleString
    Positions_t aFieldPosition;
    Positions_t aAttrFieldType;

    typedef std::vector<sal_uInt8> PortionAttrs_t;
    PortionAttrs_t aPortionAttrs;   /// additional portion attributes

    Positions_t* pSentences;    /// positions of sentence breaks

    size_t nBeforePortions;     /// # of portions before first model character
    bool bFinished;
    bool bLastIsSpecial;    /// set if last portion was 'Special()'

    /// returns the index of the first position whose value is smaller
    /// or equal, and whose following value is equal or larger
    static size_t FindBreak( const Positions_t& rPositions, sal_Int32 nValue );

    /// like FindBreak, but finds the last equal or larger position
    static size_t FindLastBreak( const Positions_t& rPositions, sal_Int32 nValue );

    /// fill the boundary with the values from rPositions[nPos]
    static void FillBoundary(com::sun::star::i18n::Boundary& rBound,
                      const Positions_t& rPositions,
                      size_t nPos );

    /// Access to portion attributes
    bool IsPortionAttrSet( size_t nPortionNo, sal_uInt8 nAttr ) const;
    bool IsSpecialPortion( size_t nPortionNo ) const;
    bool IsReadOnlyPortion( size_t nPortionNo ) const;
    bool IsGrayPortionType( sal_uInt16 nType ) const;

    // helper method for GetEditableRange(...):
    void AdjustAndCheck( sal_Int32 nPos, size_t& nPortionNo,
                         sal_Int32& nCorePos, bool& bEdit ) const;

public:
    SwAccessiblePortionData( const SwTextNode* pTextNd,
                             const SwViewOption* pViewOpt = NULL );
    virtual ~SwAccessiblePortionData();

    // SwPortionHandler methods
    virtual void Text(sal_Int32 nLength, sal_uInt16 nType, sal_Int32 nHeight = 0, sal_Int32 nWidth = 0) override;
    virtual void Special(sal_Int32 nLength, const OUString& rText, sal_uInt16 nType, sal_Int32 nHeight = 0, sal_Int32 nWidth = 0, const SwFont* pFont = 0) override;
    virtual void LineBreak(sal_Int32 nWidth) override;
    virtual void Skip(sal_Int32 nLength) override;
    virtual void Finish() override;

    virtual void SetAttrFieldType( sal_uInt16 nAttrFieldType ) override;
    bool FillBoundaryIFDateField( com::sun::star::i18n::Boundary& rBound, const sal_Int32 nPos );
    bool IsIndexInFootnode(sal_Int32 nIndex);
    bool IsInGrayPortion( sal_Int32 nPos );
    sal_Int32 GetFieldIndex(sal_Int32 nPos);

    bool IsZeroCorePositionData();

    // access to the portion data

    /// get the text string, as presented by the layout
    const OUString& GetAccessibleString() const;

    /// get the start & end positions of the sentence
    void GetLineBoundary( com::sun::star::i18n::Boundary& rBound,
                          sal_Int32 nPos ) const;

    // get start and end position of the last line
    void GetLastLineBoundary( com::sun::star::i18n::Boundary& rBound ) const;

    /// Determine whether this core position is valid for these portions.
    /// (A paragraph may be split into several frames, e.g. at page
    ///  boundaries. In this case, only part of a paragraph is represented
    ///  through this object. This method determines whether one particular
    ///  position is valid for this object or not.)
    bool IsValidCorePosition( sal_Int32 nPos ) const;
    sal_Int32 GetFirstValidCorePosition() const;
    sal_Int32 GetLastValidCorePosition() const;

    /// get the position in the accessibility string for a given model position
    sal_Int32 GetAccessiblePosition( sal_Int32 nPos ) const;

    // #i89175#
    sal_Int32 GetLineCount() const;
    sal_Int32 GetLineNo( const sal_Int32 nPos ) const;
    void GetBoundaryOfLine( const sal_Int32 nLineNo,
                            com::sun::star::i18n::Boundary& rLineBound );

    /// get the position in the model string for a given
    /// (accessibility) position
    sal_Int32 GetModelPosition( sal_Int32 nPos ) const;

    /// fill a SwSpecialPos structure, suitable for calling
    /// SwTextFrm->GetCharRect
    /// Returns the core position, and fills rpPos either with NULL or
    /// with the &rPos, after putting the appropriate data into it.
    sal_Int32 FillSpecialPos( sal_Int32 nPos,
                           SwSpecialPos& rPos,
                           SwSpecialPos*& rpPos ) const;

    // get boundaries of words/sentences. The data structures are
    // created on-demand.
    void GetSentenceBoundary( com::sun::star::i18n::Boundary& rBound,
                              sal_Int32 nPos );

    // get (a) boundary for attribute change
    void GetAttributeBoundary( com::sun::star::i18n::Boundary& rBound,
                               sal_Int32 nPos ) const;

    /// Convert start and end positions into core positions.
    /// @returns true if 'special' portions are included either completely
    ///          or not at all. This can be used to test whether editing
    ///          that range would be legal
    bool GetEditableRange( sal_Int32 nStart, sal_Int32 nEnd,
                               sal_Int32& nCoreStart, sal_Int32& nCoreEnd ) const;
private:
    typedef std::pair<sal_Int32,sal_Int32> PAIR_POS;
    typedef std::vector<PAIR_POS> VEC_PAIR_POS;
    VEC_PAIR_POS m_vecPairPos;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
