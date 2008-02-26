/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bookmrk.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 10:33:40 $
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


#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SWSERV_HXX
#include <swserv.hxx>
#endif
#ifndef _ERRHDL_HXX //autogen
#include <errhdl.hxx>
#endif
#include <IDocumentBookmarkAccess.hxx>
#ifndef _NDTXT_HXX
#include "ndtxt.hxx"
#endif

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

