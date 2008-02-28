/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: crossrefbookmark.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-28 11:15:30 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <crossrefbookmark.hxx>

#include <ndtxt.hxx>

TYPEINIT1( SwCrossRefBookmark, SwBookmark );  //rtti

SwCrossRefBookmark::SwCrossRefBookmark( const SwPosition& aPos,
                                        const KeyCode& rCode,
                                        const String& rName,
                                        const String& rShortName )
    : SwBookmark( SwPosition( aPos ), rCode, rName, rShortName ),
      // --> OD 2007-11-16 #i83479#
      mnSubType( bookmarkfunc::isHeadingCrossRefBookmarkName( rName )
                 ? IDocumentBookmarkAccess::HEADING
                 : IDocumentBookmarkAccess::NUMITEM )
      // <--
{
    eMarkType = IDocumentBookmarkAccess::CROSSREF_BOOKMARK;

    ASSERT( GetBookmarkPos().nNode.GetNode().GetTxtNode(),
            "<SwCrossRefBookmark::SwCrossRefBookmark(..)> - cross-reference bookmark doesn't mark text node." )
    ASSERT( GetBookmarkPos().nContent.GetIndex() == 0,
            "<SwCrossRefBookmark::SwCrossRefBookmark(..)> - cross-reference bookmark doesn't mark start of text node." )
    ASSERT( mnSubType == IDocumentBookmarkAccess::HEADING ||
            bookmarkfunc::isNumItemCrossRefBookmarkName( rName ),
            "<SwCrossRefBookmark::SwCrossRefBookmark(..)> - name doesn't fit. Serious issue, please inform OD!" );
}

SwCrossRefBookmark::~SwCrossRefBookmark()
{
}

IDocumentBookmarkAccess::CrossReferenceBookmarkSubType SwCrossRefBookmark::GetSubType() const
{
    return mnSubType;
}

const SwPosition* SwCrossRefBookmark::GetOtherBookmarkPos() const
{
    return 0;
}

void SwCrossRefBookmark::SetBookmarkPos( const SwPosition* pNewPos1 )
{
    ASSERT( pNewPos1->nNode.GetNode().GetTxtNode(),
            "<SwCrossRefBookmark::SetBookmarkPos(..)> - new bookmark position for cross-reference bookmark doesn't mark text node" );
    ASSERT( pNewPos1->nContent.GetIndex() == 0,
            "<SwCrossRefBookmark::SetBookmarkPos(..)> - new bookmark position for cross-reference bookmark doesn't mark start of text node" );

    SwBookmark::SetBookmarkPos( pNewPos1 );
}

void SwCrossRefBookmark::SetOtherBookmarkPos( const SwPosition* /*pNewPos2*/ )
{
    // the other bookmark position for a cross-reference bookmark is allowed
    // to be set.
    ASSERT( false,
            "<SwCrossRefBookmark::SetOtherBookmarkPos(..)> - misusage of SwCrossRefBookmark: other bookmark position isn't allowed to be set." );
}

