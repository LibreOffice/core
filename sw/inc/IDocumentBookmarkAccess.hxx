/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IDocumentBookmarkAccess.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 10:27:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef IDOCUMENTBOOKMARKACCESS_HXX_INCLUDED
#define IDOCUMENTBOOKMARKACCESS_HXX_INCLUDED

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

class SwBookmark;
class SwBookmarks;
class SwPaM;
class KeyCode;
class String;
class SwTxtNode;

/** Provides access to the bookmarks of a document.
*/
class IDocumentBookmarkAccess
{
public:
    enum BookmarkType
    {
        BOOKMARK,
        MARK,
        DDE_BOOKMARK,
        UNO_BOOKMARK,
        // --> OD 2007-10-11 #i81002# - bookmark type for cross-references
        CROSSREF_BOOKMARK
        // <--
    };

    // --> OD 2007-11-16 #i83479#
    enum CrossReferenceBookmarkSubType
    {
        HEADING,
        NUMITEM
    };
    // <--

public:
    /** Returns all bookmarks set at the document.

       @returns
       the bookmarks set at the document.
    */
    virtual const SwBookmarks& getBookmarks() const = 0;

    /** Generates a new bookmark in the document.

       @param rPaM
       [in] the location of the new bookmark.

       @param rKC
       [in] ???

       @param rName
       [in] the name of the new bookmark.

       @param rShortName
       [in] the short name of the new bookmark.

       @param eMark
       [in] the type of the new bookmark.

       @returns
       a pointer to the new bookmark.
    */
    virtual SwBookmark* makeBookmark( /*[in]*/const SwPaM& rPaM, /*[in]*/const KeyCode& rKC,
                                      /*[in]*/const String& rName, /*[in]*/const String& rShortName,
                                      /*[in]*/BookmarkType eMark ) = 0;

    /** Deletes a bookmark.

       @param nPos
       [in] the position of the bookmark to be deleted.
    */
    virtual void deleteBookmark( /*[in]*/sal_uInt16 nPos ) = 0;

    /** Deletes a bookmark.

       @param rName
       [in] the name of the bookmark to be deleted.
    */
    virtual void deleteBookmark( /*[in]*/const String& rName ) = 0;

    /** Checks, if the given name fits to the cross-reference bookmark
        name schema

        OD 2007-10-24 #i81002#

        @author OD

        @param rName
        [in] the name to be checked.

        @returns
        boolean indicating , if the name fits or not
    */
    virtual bool isCrossRefBookmarkName( /*[in]*/const String& rName ) = 0;

    /** Find a bookmark.

       @param rName
       [in] the name of the bookmark to be found.

       @returns
       the position of the bookmark in the bookmark container.
    */
    virtual sal_uInt16 findBookmark(  /*[in]*/const String& rName ) = 0;

    /** Generates a unique bookmark name. The name has to be passed to the
        function, a number will be added to the name if the name is already
        used.

       @param rName
       [in/out] the name of the bookmark.
    */
    virtual void makeUniqueBookmarkName( /*[in/out]*/String& rName ) = 0;

    /** Get the number of ::com::sun::star::text::Bookmarks.

       @param bBkmrk
       [in] if set, only "real" bookmarks are considered.

       @returns
       the number of bookmarks.
    */
    virtual sal_uInt16 getBookmarkCount( /*[in]*/bool bBkmrk) const = 0;

    /** Get a bookmark.

       @param nPos
       [in] the position of the bookmark in the bookmark container.

       @param bBkmrk
       [in] if set, only "real" bookmarks are considered.

       @returns
       the bookmark.
    */
    virtual SwBookmark& getBookmark(  /*[in]*/sal_uInt16 nPos, /*[in]*/bool bBkmrk) = 0;

    /** Get cross-reference bookmark name for certain text node

        OD 2007-11-16 #i83479#

        @author OD

        @param rTxtNode
        [in] reference to text node, whose cross-reference bookmark name has to be returned.

        @param nCrossRefType
        [in] sub type of cross-reference bookmark, whose name has to be returned.

        @returns
        name of cross-reference bookmark of given cross-reference sub type,
        if such a cross-reference bookmark exists at given textnode.
        otherwise, empty string
    */
    virtual String getCrossRefBookmarkName(
            /*[in]*/const SwTxtNode& rTxtNode,
            /*[in]*/const CrossReferenceBookmarkSubType nCrossRefType ) const = 0;

    /** Generates new cross-reference bookmark for given text node of given sub type

        OD 2007-11-16 #i83479#

        @author OD

        @param rTxtNode
        [in] reference to text node, at which the cross-reference bookmark has to be generated.

        @param nCrossRefType
        [in] sub type of cross-reference bookmark.

        @returns
        name of generated cross-reference bookmark.
        If empty, cross-reference bookmark is not generated.
    */
    virtual String makeCrossRefBookmark(
                /*[in]*/const SwTxtNode& rTxtNode,
                /*[in]*/const CrossReferenceBookmarkSubType nCrossRefType ) = 0;

protected:
    virtual ~IDocumentBookmarkAccess() {};
};

namespace bookmarkfunc
{
    /** return the prefix used for cross-reference bookmark for headings

        OD 2007-11-16 #i83479#

        @author OD
    */
    const String getHeadingCrossRefBookmarkNamePrefix();

    /** return the prefix used for cross-reference bookmark for numbered items

        OD 2007-11-16 #i83479#

        @author OD
    */
    const String getNumItemCrossRefBookmarkNamePrefix();

    /** Checks, if the given name fits to the heading cross-reference bookmark
        name schema

        OD 2007-11-09 #i81002#

        @author OD

        @param rName
        [in] the name to be checked.

        @returns
        boolean indicating , if the name fits or not
    */
    bool isHeadingCrossRefBookmarkName( /*[in]*/const String& rName );

    /** Checks, if the given name fits to the numbered item cross-reference
        bookmark name schema

        OD 2007-11-09 #i81002#

        @author OD

        @param rName
        [in] the name to be checked.

        @returns
        boolean indicating , if the name fits or not
    */
    bool isNumItemCrossRefBookmarkName( /*[in]*/const String& rName );

    /** generate new name for a cross-reference bookmark of given sub type

        OD 2007-11-16 #i83479#

        @author OD
    */
    String generateNewCrossRefBookmarkName(
            /*[in]*/const IDocumentBookmarkAccess::CrossReferenceBookmarkSubType nSubType );
}
#endif // IDOCUMENTBOOKMARKACCESS_HXX_INCLUDED
