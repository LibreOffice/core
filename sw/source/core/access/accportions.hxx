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
#include <memory>
#include <vector>

class SwTextFrame;
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
    // the frame this portion is referring to
    SwTextFrame const* m_pTextFrame;

    // variables used while collecting the data
    OUStringBuffer m_aBuffer;
    TextFrameIndex m_nViewPosition;
    const SwViewOption* m_pViewOptions;

    /// the accessible string
    /// note that the content is different both from the string in the text
    /// node(s) as well as the string in the text frame, so there are 3
    /// different index spaces involved.
    OUString m_sAccessibleString;

    // positions array
    // instances of Position_t must always include the minimum and
    // maximum positions as first/last elements (to simplify the
    // algorithms)
    typedef std::vector<sal_Int32> AccessiblePositions;
    typedef std::vector<TextFrameIndex> FramePositions;

    AccessiblePositions m_aLineBreaks; /// position of line breaks
    FramePositions m_ViewPositions; /// position of portion breaks in the core view
    AccessiblePositions m_aAccessiblePositions; /// portion breaks in m_sAccessibleString
    AccessiblePositions m_aFieldPosition;

    std::vector<sal_uInt8> m_aPortionAttrs;   /// additional portion attributes

    std::unique_ptr<AccessiblePositions> m_pSentences; /// positions of sentence breaks

    size_t m_nBeforePortions;     /// # of portions before first core character
    bool m_bFinished;

    /// fill the boundary with the values from rPositions[nPos]
    static void FillBoundary(css::i18n::Boundary& rBound,
                      const AccessiblePositions& rPositions,
                      size_t nPos );

    /// Access to portion attributes
    bool IsPortionAttrSet( size_t nPortionNo, sal_uInt8 nAttr ) const;
    bool IsSpecialPortion( size_t nPortionNo ) const;
    bool IsGrayPortionType( PortionType nType ) const;

    // helper method for GetEditableRange(...):
    void AdjustAndCheck( sal_Int32 nPos, size_t& nPortionNo,
                         TextFrameIndex& rCorePos, bool& bEdit) const;

public:
    SwAccessiblePortionData( const SwTextFrame* pTextFrame,
                             const SwViewOption* pViewOpt );
    virtual ~SwAccessiblePortionData() override;

    // SwPortionHandler methods
    virtual void Text(TextFrameIndex nLength, PortionType nType, sal_Int32 nHeight = 0, sal_Int32 nWidth = 0) override;
    virtual void Special(TextFrameIndex nLength, const OUString& rText, PortionType nType, sal_Int32 nHeight = 0, sal_Int32 nWidth = 0, const SwFont* pFont = nullptr) override;
    virtual void LineBreak(sal_Int32 nWidth) override;
    virtual void Skip(TextFrameIndex nLength) override;
    virtual void Finish() override;

    bool FillBoundaryIFDateField( css::i18n::Boundary& rBound, const sal_Int32 nPos );
    bool IsIndexInFootnode(sal_Int32 nIndex);
    bool IsInGrayPortion( sal_Int32 nPos );
    sal_Int32 GetFieldIndex(sal_Int32 nPos);

    bool IsZeroCorePositionData();

    // access to the portion data

    /// get the text string, as presented by the layout
    const OUString& GetAccessibleString() const;

    /// get the start & end positions of the sentence
    void GetLineBoundary( css::i18n::Boundary& rBound,
                          sal_Int32 nPos ) const;

    // get start and end position of the last line
    void GetLastLineBoundary( css::i18n::Boundary& rBound ) const;

    /// Determine whether this core position is valid for these portions.
    /// (A paragraph may be split into several frames, e.g. at page
    ///  boundaries. In this case, only part of a paragraph is represented
    ///  through this object. This method determines whether one particular
    ///  position is valid for this object or not.)
    bool IsValidCorePosition(TextFrameIndex nPos) const;
    TextFrameIndex GetFirstValidCorePosition() const;
    TextFrameIndex GetLastValidCorePosition() const;

    /// get the position in the accessibility string for a given view position
    sal_Int32 GetAccessiblePosition(TextFrameIndex nPos) const;

    // #i89175#
    sal_Int32 GetLineCount() const;
    sal_Int32 GetLineNo( const sal_Int32 nPos ) const;
    void GetBoundaryOfLine( const sal_Int32 nLineNo,
                            css::i18n::Boundary& rLineBound );

    /// get the position in the core view string for a given
    /// (accessibility) position
    TextFrameIndex GetCoreViewPosition(sal_Int32 nPos) const;

    /// fill a SwSpecialPos structure, suitable for calling
    /// SwTextFrame->GetCharRect
    /// Returns the core position, and fills rpPos either with NULL or
    /// with the &rPos, after putting the appropriate data into it.
    TextFrameIndex FillSpecialPos(sal_Int32 nPos,
                           SwSpecialPos& rPos,
                           SwSpecialPos*& rpPos ) const;

    // get boundaries of words/sentences. The data structures are
    // created on-demand.
    void GetSentenceBoundary( css::i18n::Boundary& rBound,
                              sal_Int32 nPos );

    // get (a) boundary for attribute change
    void GetAttributeBoundary( css::i18n::Boundary& rBound,
                               sal_Int32 nPos ) const;

    /// Convert start and end positions into core positions.
    /// @returns true if 'special' portions are included either completely
    ///          or not at all. This can be used to test whether editing
    ///          that range would be legal
    bool GetEditableRange( sal_Int32 nStart, sal_Int32 nEnd,
           TextFrameIndex& rCoreStart, TextFrameIndex& rCoreEnd) const;

private:
    std::vector< std::pair<sal_Int32,sal_Int32> > m_vecPairPos;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
