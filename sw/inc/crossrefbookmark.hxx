/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: crossrefbookmark.hxx,v $
 * $Revision: 1.3 $
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
#ifndef _CROSSREFBOOKMRK_HXX
#define _CROSSREFBOOKMRK_HXX

#include <bookmrk.hxx>

/** Bookmark for cross-references

    OD 2007-10-11 #i81002#
    Bookmark, which bookmarks a complete text node (heading or numbered paragraph)
    for cross-referencing.
    <GetBookmarkPos()> is always at the beginning of this text node and
    <GetOtherBookmarkPos()> is always NULL.

    @author OD
*/
class SwCrossRefBookmark : public SwBookmark
{
public:
    TYPEINFO();

    SwCrossRefBookmark( const SwPosition& aPos,
                        const KeyCode& rCode,
                        const String& rName,
                        const String& rShortName );

    virtual ~SwCrossRefBookmark();

    virtual const SwPosition* GetOtherBookmarkPos() const;

    virtual void SetBookmarkPos( const SwPosition* pNewPos1 );
    virtual void SetOtherBookmarkPos( const SwPosition* /*pNewPos2*/ );

    IDocumentBookmarkAccess::CrossReferenceBookmarkSubType GetSubType() const;
private:
    // --> OD 2007-11-16 #i83479#
    IDocumentBookmarkAccess::CrossReferenceBookmarkSubType mnSubType;
    // <--

    // no copy-constructor and no assignment-operator
    SwCrossRefBookmark(const SwCrossRefBookmark &);
    SwCrossRefBookmark &operator=(const SwCrossRefBookmark &);
};
#endif
