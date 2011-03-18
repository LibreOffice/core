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
#ifndef _ACCPORTIONS_HXX
#define _ACCPORTIONS_HXX
#include <SwPortionHandler.hxx>
#include <sal/types.h>
#include <rtl/ustrbuf.hxx>
#include <vector>

class String;
class SwTxtNode;
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
    const SwTxtNode* pTxtNode;

    // variables used while collecting the data
    rtl::OUStringBuffer aBuffer;
    sal_Int32 nModelPosition;
    sal_Bool bFinished;
    const SwViewOption* pViewOptions;

    // the accessible string
    rtl::OUString sAccessibleString;

    // positions array
    // instances of Position_t must always include the minimum and
    // maximum positions as first/last elements (to simplify the
    // algorithms)
    typedef std::vector<sal_Int32> Positions_t;

    Positions_t aLineBreaks;        /// position of line breaks
    Positions_t aModelPositions;    /// position of portion breaks in the model
    Positions_t aAccessiblePositions;   /// portion breaks in sAccessibleString

    typedef std::vector<sal_uInt8> PortionAttrs_t;
    PortionAttrs_t aPortionAttrs;   /// additional portion attributes

    Positions_t* pSentences;    /// positions of sentence breaks

    size_t nBeforePortions;     /// # of portions before first model character
    sal_Bool bLastIsSpecial;    /// set if last portion was 'Special()'

    /// returns the index of the first position whose value is smaller
    /// or equal, and whose following value is equal or larger
    size_t FindBreak( const Positions_t& rPositions, sal_Int32 nValue ) const;

    /// like FindBreak, but finds the last equal or larger position
    size_t FindLastBreak( const Positions_t& rPositions, sal_Int32 nValue ) const;

    /// fill the boundary with the values from rPositions[nPos]
    void FillBoundary(com::sun::star::i18n::Boundary& rBound,
                      const Positions_t& rPositions,
                      size_t nPos ) const;

    /// Access to portion attributes
    sal_Bool IsPortionAttrSet( size_t nPortionNo, sal_uInt8 nAttr ) const;
    sal_Bool IsSpecialPortion( size_t nPortionNo ) const;
    sal_Bool IsReadOnlyPortion( size_t nPortionNo ) const;
    sal_Bool IsGrayPortionType( sal_uInt16 nType ) const;

    // helper method for GetEditableRange(...):
    void AdjustAndCheck( sal_Int32 nPos, size_t& nPortionNo,
                         sal_uInt16& nCorePos, sal_Bool& bEdit ) const;

public:
    SwAccessiblePortionData( const SwTxtNode* pTxtNd,
                             const SwViewOption* pViewOpt = NULL );
    virtual ~SwAccessiblePortionData();

    // SwPortionHandler methods
    virtual void Text(sal_uInt16 nLength, sal_uInt16 nType);
    virtual void Special(sal_uInt16 nLength, const String& rText, sal_uInt16 nType);
    virtual void LineBreak();
    virtual void Skip(sal_uInt16 nLength);
    virtual void Finish();


    // access to the portion data

    /// get the text string, as presented by the layout
    const rtl::OUString& GetAccessibleString() const;

    /// get the start & end positions of the sentence
    void GetLineBoundary( com::sun::star::i18n::Boundary& rBound,
                          sal_Int32 nPos ) const;

    // get start and end position of the last line
    void GetLastLineBoundary( com::sun::star::i18n::Boundary& rBound ) const;

    // #i89175#
    sal_Int32 GetLineCount() const;
    sal_Int32 GetLineNo( const sal_Int32 nPos ) const;
    void GetBoundaryOfLine( const sal_Int32 nLineNo,
                            com::sun::star::i18n::Boundary& rLineBound );
    // <--

    /// get the position in the model string for a given
    /// (accessibility) position
    sal_uInt16 GetModelPosition( sal_Int32 nPos ) const;

    /// get the position in the accessibility string for a given model position
    sal_Int32 GetAccessiblePosition( sal_uInt16 nPos ) const;

    /// fill a SwSpecialPos structure, suitable for calling
    /// SwTxtFrm->GetCharRect
    /// Returns the core position, and fills thr rpPos either with NULL or
    /// with the &rPos, after putting the appropriate data into it.
    sal_uInt16 FillSpecialPos( sal_Int32 nPos,
                           SwSpecialPos& rPos,
                           SwSpecialPos*& rpPos ) const;


    // get boundaries of words/sentences. The data structures are
    // created on-demand.
    void GetSentenceBoundary( com::sun::star::i18n::Boundary& rBound,
                              sal_Int32 nPos );

    // get (a) boundary for attribut change
    void GetAttributeBoundary( com::sun::star::i18n::Boundary& rBound,
                               sal_Int32 nPos ) const;

    /// Convert start and end positions into core positions.
    /// @returns true if 'special' portions are included either completely
    ///          or not at all. This can be used to test whether editing
    ///          that range would be legal
    sal_Bool GetEditableRange( sal_Int32 nStart, sal_Int32 nEnd,
                               sal_uInt16& nCoreStart, sal_uInt16& nCoreEnd ) const;

    /// Determine whether this core position is valid for these portions.
    /// (A paragraph may be split into several frames, e.g. at page
    ///  boundaries. In this case, only part of a paragraph is represented
    ///  through this object. This method determines whether one particular
    ///  position is valid for this object or not.)
    sal_Bool IsValidCorePosition( sal_uInt16 nPos ) const;
    sal_uInt16 GetFirstValidCorePosition() const;
    sal_uInt16 GetLastValidCorePosition() const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
