/*************************************************************************
 *
 *  $RCSfile: accportions.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:11:44 $
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
#ifndef _ACCPORTIONS_HXX
#define _ACCPORTIONS_HXX

#include <SwPortionHandler.hxx>
#include <sal/types.h>

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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
    size_t FindBreak( const Positions_t& rPositions, sal_Int32 nValue );

    /// like FindBreak, but finds the last equal or larger position
    size_t FindLastBreak( const Positions_t& rPositions, sal_Int32 nValue );

    /// fill the boundary with the values from rPositions[nPos]
    void FillBoundary(com::sun::star::i18n::Boundary& rBound,
                      const Positions_t& rPositions,
                      size_t nPos );

    /// Access to portion attributes
    sal_Bool IsPortionAttrSet( size_t nPortionNo, sal_uInt8 nAttr );
    inline sal_Bool IsSpecialPortion( size_t nPortionNo );
    inline sal_Bool IsReadOnlyPortion( size_t nPortionNo );
    inline sal_Bool IsGrayPortion( size_t nPortionNo );

    // Helper methods for SwPortionHandler:

    /// Is a portion of this type gray?
    sal_Bool IsGrayPortionType( USHORT nType );

    // helper method for GetEditableRange(...):
    void AdjustAndCheck( sal_Int32 nPos, size_t& nPortionNo,
                         USHORT& nCorePos, sal_Bool& bEdit );

public:
    SwAccessiblePortionData( const SwTxtNode* pTxtNd,
                             const SwViewOption* pViewOpt = NULL );
    virtual ~SwAccessiblePortionData();

    // SwPortionHandler methods
    virtual void Text(USHORT nLength, USHORT nType);
    virtual void Special(USHORT nLength, const String& rText, USHORT nType);
    virtual void LineBreak();
    virtual void Skip(USHORT nLength);
    virtual void Finish();


    // access to the portion data

    /// get the text string, as presented by the layout
    const rtl::OUString& GetAccessibleString();

    /// get the start & end positions of the sentence
    void GetLineBoundary( com::sun::star::i18n::Boundary& rBound,
                          sal_Int32 nPos );

    // get start and end position of the last line
    void GetLastLineBoundary( com::sun::star::i18n::Boundary& rBound );

    /// get the position in the model string for a given
    /// (accessibility) position
    USHORT GetModelPosition( sal_Int32 nPos );

    /// get the position in the accessibility string for a given model position
    sal_Int32 GetAccessiblePosition( USHORT nPos );

    /// fill a SwSpecialPos structure, suitable for calling
    /// SwTxtFrm->GetCharRect
    /// Returns the core position, and fills thr rpPos either with NULL or
    /// with the &rPos, after putting the appropriate data into it.
    USHORT FillSpecialPos( sal_Int32 nPos,
                           SwSpecialPos& rPos,
                           SwSpecialPos*& rpPos );


    // get boundaries of words/sentences. The data structures are
    // created on-demand.
    void GetSentenceBoundary( com::sun::star::i18n::Boundary& rBound,
                              sal_Int32 nPos );

    // get (a) boundary for attribut change
    void GetAttributeBoundary( com::sun::star::i18n::Boundary& rBound,
                               sal_Int32 nPos );

    /// Determine whether this portion should have a gray background
    /// accoridng to the view options
    sal_Bool IsInGrayPortion( sal_Int32 nPos );


    /// Convert start and end positions into core positions.
    /// @returns true if 'special' portions are included either completely
    ///          or not at all. This can be used to test whether editing
    ///          that range would be legal
    sal_Bool GetEditableRange( sal_Int32 nStart, sal_Int32 nEnd,
                               USHORT& nCoreStart, USHORT& nCoreEnd );

    /// Determine whether this core position is valid for these portions.
    /// (A paragraph may be split into several frames, e.g. at page
    ///  boundaries. In this case, only part of a paragraph is represented
    ///  through this object. This method determines whether one particular
    ///  position is valid for this object or not.)
    sal_Bool IsValidCorePosition( USHORT nPos );
    USHORT GetFirstValidCorePosition();
    USHORT GetLastValidCorePosition();
};


#endif

