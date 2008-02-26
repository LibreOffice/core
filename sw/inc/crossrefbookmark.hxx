/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: crossrefbookmark.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 10:29:03 $
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
