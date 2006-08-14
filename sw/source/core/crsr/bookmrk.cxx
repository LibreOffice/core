/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bookmrk.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 15:50:26 $
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

#pragma hdrstop

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

SV_IMPL_REF( SwServerObject )

TYPEINIT1( SwBookmark, SwModify );  //rtti


SwBookmark::SwBookmark(const SwPosition& aPos)
    : SwModify( 0 ),
    pPos2( 0 ),
    aStartMacro( aEmptyStr, aEmptyStr ),
    aEndMacro  ( aEmptyStr, aEmptyStr ),
    eMarkType( IDocumentBookmarkAccess::BOOKMARK )
{
    pPos1 = new SwPosition( aPos );
}


SwBookmark::SwBookmark(const SwPosition& aPos, const KeyCode& rCode,
                        const String& rName, const String& rShortName )
    : SwModify( 0 ),
    pPos2( 0 ),
    aStartMacro( aEmptyStr, aEmptyStr ),
    aEndMacro  ( aEmptyStr, aEmptyStr ),
    aName(rName),
    aShortName(rShortName),
    aCode(rCode),
    eMarkType( IDocumentBookmarkAccess::BOOKMARK )
{
    pPos1 = new SwPosition(aPos);
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
    const SwPosition* pThisPos = ( !pPos2 || *pPos1 <= *pPos2 ) ? pPos1 : pPos2;
    const SwPosition* pBMPos = ( !rBM.pPos2 || *rBM.pPos1 <= *rBM.pPos2 )
                                        ? rBM.pPos1 : rBM.pPos2;

    return *pThisPos < *pBMPos;
}

BOOL SwBookmark::operator==(const SwBookmark &rBM) const
{
    return (this == &rBM);
}

BOOL SwBookmark::IsEqualPos( const SwBookmark &rBM ) const
{
    const SwPosition* pThisPos = ( !pPos2 || *pPos1 <= *pPos2 ) ? pPos1 : pPos2;
    const SwPosition* pBMPos = ( !rBM.pPos2 || *rBM.pPos1 <= *rBM.pPos2 )
                                        ? rBM.pPos1 : rBM.pPos2;

    return *pThisPos == *pBMPos;
}

void SwBookmark::SetRefObject( SwServerObject* pObj )
{
    refObj = pObj;
}


SwMark::SwMark( const SwPosition& aPos,
                const KeyCode& rCode,
                const String& rName,
                const String& rShortName )
    : SwBookmark( aPos, rCode, rName, rShortName )
{
    eMarkType = IDocumentBookmarkAccess::MARK;
}

SwUNOMark::SwUNOMark( const SwPosition& aPos,
                const KeyCode& rCode,
                const String& rName,
                const String& rShortName )
    : SwBookmark( aPos, rCode, rName, rShortName )
{
    eMarkType = IDocumentBookmarkAccess::UNO_BOOKMARK;
}

