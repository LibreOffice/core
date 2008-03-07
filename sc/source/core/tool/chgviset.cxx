/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chgviset.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 12:18:01 $
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
#include "precompiled_sc.hxx"



#include <unotools/textsearch.hxx>

#include "chgviset.hxx"
#include "rechead.hxx"
#include "chgtrack.hxx"

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
    mbIsActionRange = r.mbIsActionRange;
    mnFirstAction   = r.mnFirstAction;
    mnLastAction    = r.mnLastAction;

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
    mbIsActionRange = r.mbIsActionRange;
    mnFirstAction   = r.mnFirstAction;
    mnLastAction    = r.mnLastAction;

    return *this;
}

BOOL ScChangeViewSettings::IsValidComment(const String* pCommentStr) const
{
    BOOL nTheFlag=TRUE;

    if(pCommentSearcher!=NULL)
    {
        xub_StrLen nStartPos = 0;
        xub_StrLen nEndPos = pCommentStr->Len();

        nTheFlag=sal::static_int_cast<BOOL>(pCommentSearcher->SearchFrwrd( *pCommentStr, &nStartPos, &nEndPos));
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


void ScChangeViewSettings::AdjustDateMode( const ScDocument& rDoc )
{
    switch ( eDateMode )
    {   // corresponds with ScViewUtil::IsActionShown
        case SCDM_DATE_EQUAL :
        case SCDM_DATE_NOTEQUAL :
            aFirstDateTime.SetTime( 0 );
            aLastDateTime = aFirstDateTime;
            aLastDateTime.SetTime( 23595999 );
            break;
        case SCDM_DATE_SAVE:
        {
            const ScChangeAction* pLast = 0;
            ScChangeTrack* pTrack = rDoc.GetChangeTrack();
            if ( pTrack )
            {
                pLast = pTrack->GetLastSaved();
                if ( pLast )
                {
                    aFirstDateTime = pLast->GetDateTime();
#if 0
// This would be the proper handling. But since the SvxTPFilter dialog uses
// DateField/TimeField, and the filter dialog is used in ScAcceptChgDlg as the
// controlling instance, and the TimeFields are used there without seconds or
// 100ths, we'd display some extra entries between the floor of the minute and
// the start of the next minute.
                    // add one 100th second to point past last saved
                    aFirstDateTime += Time( 0, 0, 0, 1 );
#else
                    // Set the next minute as the start time and assume that
                    // the document isn't saved, reloaded, edited and filter set
                    // all together during the gap between those two times.
                    aFirstDateTime += Time( 0, 1 );
                    aFirstDateTime.SetSec(0);
                    aFirstDateTime.Set100Sec(0);
#endif
                }
            }
            if ( !pLast )
            {
                aFirstDateTime.SetDate( 18990101 );
                aFirstDateTime.SetTime( 0 );
            }
            aLastDateTime = Date();
            aLastDateTime.SetYear( aLastDateTime.GetYear() + 100 );
        }
        break;
        default:
        {
            // added to avoid warnings
        }
    }
}

