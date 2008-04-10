/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: crossrefbookmark.cxx,v $
 * $Revision: 1.4 $
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

