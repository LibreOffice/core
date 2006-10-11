/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IDocumentBookmarkAccess.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-11 08:41:22 $
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
        HIDDEN_BOOKMARK
    };

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

protected:
    virtual ~IDocumentBookmarkAccess() {};
 };

 #endif // IDOCUMENTBOOKMARKACCESS_HXX_INCLUDED
