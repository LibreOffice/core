/*************************************************************************
 *
 *  $RCSfile: bookmrk.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:16 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "swtypes.hxx"
#include "doc.hxx"
#include "pam.hxx"

#include "bookmrk.hxx"
#include "swserv.hxx"

#ifndef _ERRHDL_HXX //autogen
#include <errhdl.hxx>
#endif

SO2_IMPL_REF( SwServerObject )

TYPEINIT1( SwBookmark, SwModify );  //rtti


SwBookmark::SwBookmark(const SwPosition& aPos)
    : SwModify( 0 ),
    aStartMacro( aEmptyStr, aEmptyStr ),
    aEndMacro  ( aEmptyStr, aEmptyStr ),
    pPos2( 0 ),
    eMarkType( BOOKMARK )
{
    pPos1 = new SwPosition( aPos );
}


SwBookmark::SwBookmark(const SwPosition& aPos, const KeyCode& rCode,
                        const String& rName, const String& rShortName )
    : SwModify( 0 ),
    aStartMacro( aEmptyStr, aEmptyStr ),
    aEndMacro  ( aEmptyStr, aEmptyStr ),
    aCode(rCode),
    aName(rName),
    aShortName(rShortName),
    pPos2( 0 ),
    eMarkType( BOOKMARK )
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
        if( DDE_BOOKMARK == eMarkType && refObj->GetSelectorCount() )
        {
            SvData aSvData;
            refObj->DataChanged( aSvData );
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

void SwBookmark::SetRefObject( SvPseudoObject* pObj )
{
    refObj = pObj;
}


SwMark::SwMark( const SwPosition& aPos,
                const KeyCode& rCode,
                const String& rName,
                const String& rShortName )
    : SwBookmark( aPos, rCode, rName, rShortName )
{
    eMarkType = MARK;
}

SwUNOMark::SwUNOMark( const SwPosition& aPos,
                const KeyCode& rCode,
                const String& rName,
                const String& rShortName )
    : SwBookmark( aPos, rCode, rName, rShortName )
{
    eMarkType = UNO_BOOKMARK;
}

