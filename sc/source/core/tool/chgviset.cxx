/*************************************************************************
 *
 *  $RCSfile: chgviset.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-20 10:29:51 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <unotools/textsearch.hxx>

#include "chgviset.hxx"
#include "rechead.hxx"

// -----------------------------------------------------------------------
ScChangeViewSettings::~ScChangeViewSettings()
{
    if(pCommentSearcher!=NULL)
        delete pCommentSearcher;
}

ScChangeViewSettings::ScChangeViewSettings( const ScChangeViewSettings& r )
{
    SetTheComment(r.aComment);

    aFirstDateTime  =r.aFirstDateTime;
    aLastDateTime   =r.aLastDateTime;
    aAuthorToShow   =r.aAuthorToShow;
    aRangeList      =r.aRangeList;
    eDateMode       =r.eDateMode;
    bShowIt         =r.bShowIt;
    bIsDate         =r.bIsDate;
    bIsAuthor       =r.bIsAuthor;
    bIsComment      =r.bIsComment;
    bIsRange        =r.bIsRange;
    bEveryoneButMe  =r.bEveryoneButMe;
    bShowAccepted   =r.bShowAccepted;
    bShowRejected   =r.bShowRejected;

}

ScChangeViewSettings& ScChangeViewSettings::operator=( const ScChangeViewSettings& r )
{
    SetTheComment(r.aComment);

    aFirstDateTime  =r.aFirstDateTime;
    aLastDateTime   =r.aLastDateTime;
    aAuthorToShow   =r.aAuthorToShow;
    aRangeList      =r.aRangeList;
    eDateMode       =r.eDateMode;
    bShowIt         =r.bShowIt;
    bIsDate         =r.bIsDate;
    bIsAuthor       =r.bIsAuthor;
    bIsComment      =r.bIsComment;
    bIsRange        =r.bIsRange;
    bEveryoneButMe  =r.bEveryoneButMe;
    bShowAccepted   =r.bShowAccepted;
    bShowRejected   =r.bShowRejected;

    return *this;
}

BOOL ScChangeViewSettings::IsValidComment(const String* pCommentStr) const
{
    BOOL nTheFlag=TRUE;

    if(pCommentSearcher!=NULL)
    {
        xub_StrLen nStartPos = 0;
        xub_StrLen nEndPos = pCommentStr->Len();

        nTheFlag=pCommentSearcher->SearchFrwrd( *pCommentStr, &nStartPos, &nEndPos);
    }
    return nTheFlag;
}

void ScChangeViewSettings::SetTheComment(const String& rString)
{
    aComment=rString;
    if(pCommentSearcher!=NULL)
    {
        delete pCommentSearcher;
        pCommentSearcher=NULL;
    }

    if(rString.Len()>0)
    {
        utl::SearchParam aSearchParam( rString,
            utl::SearchParam::SRCH_REGEXP,FALSE,FALSE,FALSE );

        pCommentSearcher = new utl::TextSearch( aSearchParam, *ScGlobal::pCharClass );
    }
}

void ScChangeViewSettings::Load( SvStream& rStream, USHORT nVer )
{
    ScReadHeader aHdr( rStream );

    BYTE nByte;
    UINT32 nDT;
    rStream >> bShowIt;
    rStream >> bIsDate;
    rStream >> nByte; eDateMode = (ScChgsDateMode)nByte;
    rStream >> nDT; aFirstDateTime.SetDate( nDT );
    rStream >> nDT; aFirstDateTime.SetTime( nDT );
    rStream >> nDT; aLastDateTime.SetDate( nDT );
    rStream >> nDT; aLastDateTime.SetTime( nDT );
    rStream >> bIsAuthor;
    rStream >> bEveryoneButMe;
    rStream.ReadByteString( aAuthorToShow, rStream.GetStreamCharSet() );
    rStream >> bIsRange;
    aRangeList.Load( rStream, nVer );
    if ( aHdr.BytesLeft() )
    {
        rStream >> bShowAccepted;
        rStream >> bShowRejected;
    }
    else
    {
        bShowAccepted = FALSE;
        bShowRejected = FALSE;
    }

    // Zusaetzlich Kommentar-Informationen lesen (src509)
    if ( aHdr.BytesLeft() )  //#59103#
    {
        rStream >> bIsComment;
        rStream.ReadByteString( aComment, rStream.GetStreamCharSet() );
    }
    else
    {
        bIsComment = FALSE;
        aComment.Erase();
    }
    SetTheComment(aComment);
}

void ScChangeViewSettings::Store( SvStream& rStream ) const
{
    ScWriteHeader aHdr( rStream, 42 );      // Groesse, wenn String und RangeList leer sind

    rStream << bShowIt;
    rStream << bIsDate;
    rStream << (BYTE) eDateMode;
    rStream << (UINT32) aFirstDateTime.GetDate();
    rStream << (UINT32) aFirstDateTime.GetTime();
    rStream << (UINT32) aLastDateTime.GetDate();
    rStream << (UINT32) aLastDateTime.GetTime();
    rStream << bIsAuthor;
    rStream << bEveryoneButMe;
    rStream.WriteByteString( aAuthorToShow, rStream.GetStreamCharSet() );
    rStream << bIsRange;
    aRangeList.Store( rStream );
    rStream << bShowAccepted;
    rStream << bShowRejected;

    // Zusaetzlich Kommentar-Informationen schreiben (src509)
    if(bIsComment || aComment.Len()>0) //#59103#
    {
        rStream << bIsComment;
        rStream.WriteByteString( aComment, rStream.GetStreamCharSet() );
    }
}





