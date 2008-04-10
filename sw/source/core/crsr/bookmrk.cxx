/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bookmrk.cxx,v $
 * $Revision: 1.11 $
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


#include <bookmrk.hxx>
#include <swtypes.hxx>
#include <pam.hxx>
#include <swserv.hxx>
#include <errhdl.hxx>
#include <IDocumentBookmarkAccess.hxx>
#include "ndtxt.hxx"

SV_IMPL_REF( SwServerObject )

TYPEINIT1( SwBookmark, SwModify );  //rtti

void lcl_FixPosition( SwPosition& rPos )
{
    // make sure the position has 1) the proper node, and 2) a proper index
    SwTxtNode* pTxtNode = rPos.nNode.GetNode().GetTxtNode();

    if( rPos.nContent.GetIndex() > ( pTxtNode == NULL ? 0 : pTxtNode->Len() ) )
    {
        DBG_ERROR( "illegal position" );
        xub_StrLen nLen = rPos.nContent.GetIndex();
        if( pTxtNode == NULL )
            nLen = 0;
        else if( nLen >= pTxtNode->Len() )
            nLen = pTxtNode->Len();
        rPos.nContent.Assign( pTxtNode, nLen );
    }
}

SwBookmark::SwBookmark(const SwPosition& aPos )
    : SwModify( 0 ),
    pPos2( 0 ),
    eMarkType( IDocumentBookmarkAccess::BOOKMARK )
{
    pPos1 = new SwPosition( aPos );
}


SwBookmark::SwBookmark(const SwPosition& aPos, const KeyCode& rCode,
                        const String& rName, const String& rShortName )
    : SwModify( 0 ),
    pPos2( 0 ),
    aName(rName),
    aShortName(rShortName),
    aCode(rCode),
    eMarkType( IDocumentBookmarkAccess::BOOKMARK )
{
    pPos1 = new SwPosition(aPos);
    // --> OD 2007-09-26 #i81002#
    lcl_FixPosition( *pPos1 );
    // <--
}

SwBookmark::SwBookmark( const SwPaM& aPaM,
                        const KeyCode& rCode,
                        const String& rName, const String& rShortName)
    : SwModify( 0 ),
      pPos1( 0 ),
      pPos2( 0 ),
      refObj(),
      aName(rName),
      aShortName(rShortName),
      aCode(rCode),
      eMarkType( IDocumentBookmarkAccess::BOOKMARK )
{
    pPos1 = new SwPosition( *(aPaM.GetPoint()) );
    lcl_FixPosition( *pPos1 );
    if ( aPaM.HasMark() )
    {
        pPos2 = new SwPosition( *(aPaM.GetMark()) );
        lcl_FixPosition( *pPos2 );
    }
}

// Beim Loeschen von Text werden Bookmarks mitgeloescht!


SwBookmark::~SwBookmark()
{
    // falls wir noch der DDE-Bookmark sind, dann muss der aus dem
    // Clipboard ausgetragen werden. Wird automatisch ueber DataChanged
    // ausgeloest.
    if( refObj.Is() )
    {
        if( IDocumentBookmarkAccess::DDE_BOOKMARK == eMarkType && refObj->HasDataLinks() )
        {
            ::sfx2::SvLinkSource* p = &refObj;
            p->SendDataChanged();
        }
        refObj->SetNoServer();
    }

    delete pPos1;
    if( pPos2 )
        delete pPos2;
}

// Vergleiche auf Basis der Dokumentposition

BOOL SwBookmark::operator<(const SwBookmark &rBM) const
{
    // --> OD 2007-10-11 #i81002# - refactoring
    // simplification by using <BookmarkStart()>
//    const SwPosition* pThisPos = ( !pPos2 || *pPos1 <= *pPos2 ) ? pPos1 : pPos2;
//    const SwPosition* pBMPos = ( !rBM.pPos2 || *rBM.pPos1 <= *rBM.pPos2 )
//                                        ? rBM.pPos1 : rBM.pPos2;

//    return *pThisPos < *pBMPos;
    return *(BookmarkStart()) < *(rBM.BookmarkStart());
}

BOOL SwBookmark::operator==(const SwBookmark &rBM) const
{
    return (this == &rBM);
}

BOOL SwBookmark::IsEqualPos( const SwBookmark &rBM ) const
{
    // --> OD 2007-10-11 #i81002# - refactoring
    // simplification by using <BookmarkStart()>
//    const SwPosition* pThisPos = ( !pPos2 || *pPos1 <= *pPos2 ) ? pPos1 : pPos2;
//    const SwPosition* pBMPos = ( !rBM.pPos2 || *rBM.pPos1 <= *rBM.pPos2 )
//                                        ? rBM.pPos1 : rBM.pPos2;

//    return *pThisPos == *pBMPos;
    return *(BookmarkStart()) == *(rBM.BookmarkStart());
}

void SwBookmark::SetRefObject( SwServerObject* pObj )
{
    refObj = pObj;
}

// --> OD 2007-10-10 #i81002#
const SwPosition& SwBookmark::GetBookmarkPos() const
{
    return *pPos1;
}

const SwPosition* SwBookmark::GetOtherBookmarkPos() const
{
    return pPos2;
}

const SwPosition* SwBookmark::BookmarkStart() const
{
    return pPos2 ? (*pPos1 <= *pPos2 ? pPos1 : pPos2) : pPos1;
}

const SwPosition* SwBookmark::BookmarkEnd() const
{
    return pPos2 ? (*pPos1 >= *pPos2 ? pPos1 : pPos2) : pPos1;
}
// <--

// --> OD 2007-09-26 #i81002#
void SwBookmark::SetBookmarkPos( const SwPosition* pNewPos1 )
{
    ASSERT( pNewPos1 != 0,
            "<SwBookmark::SetBookmarkPos(..)> - Bookmark position 1 can't be NULL --> crash." );
    *pPos1 = *pNewPos1;
}
void SwBookmark::SetOtherBookmarkPos( const SwPosition* pNewPos2 )
{
    if ( pNewPos2 != 0 )
    {
        if ( pPos2 != 0 )
        {
            *pPos2 = *pNewPos2;
        }
        else
        {
            pPos2 = new SwPosition( *pNewPos2 );
        }
    }
    else
    {
        delete pPos2;
        pPos2 = 0;
    }
}
// <--

SwMark::SwMark( const SwPosition& aPos,
                const KeyCode& rCode,
                const String& rName,
                const String& rShortName )
    : SwBookmark( aPos, rCode, rName, rShortName )
{
    eMarkType = IDocumentBookmarkAccess::MARK;
}

// --> OD 2007-09-26 #i81002#
SwUNOMark::SwUNOMark( const SwPaM& aPaM,
                const KeyCode& rCode,
                const String& rName,
                const String& rShortName )
    : SwBookmark( aPaM, rCode, rName, rShortName )
{
    eMarkType = IDocumentBookmarkAccess::UNO_BOOKMARK;
}
// <--

