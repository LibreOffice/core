/*************************************************************************
 *
 *  $RCSfile: viewutil.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:10 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------
#include <tools/list.hxx>
#include "scitems.hxx"
#include <svx/charmap.hxx>
#include <svx/fontitem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/wrkwin.hxx>

#include "viewutil.hxx"
#include "global.hxx"
#include "chgtrack.hxx"
#include "chgviset.hxx"

// STATIC DATA -----------------------------------------------------------

//==================================================================

//  static
BOOL ScViewUtil::IsActionShown( const ScChangeAction& rAction,
                                const ScChangeViewSettings& rSettings,
                                ScDocument& rDocument )
{
    // abgelehnte werden durch eine invertierende akzeptierte Action dargestellt,
    // die Reihenfolge von ShowRejected/ShowAccepted ist deswegen wichtig

    if ( !rSettings.IsShowRejected() && rAction.IsRejecting() )
        return FALSE;

    if ( !rSettings.IsShowAccepted() && rAction.IsAccepted() && !rAction.IsRejecting() )
        return FALSE;

    if ( rSettings.HasAuthor() )
    {
        if ( rSettings.IsEveryoneButMe() )
        {
            //  GetUser() am ChangeTrack ist der aktuelle Benutzer
            ScChangeTrack* pTrack = rDocument.GetChangeTrack();
            if ( !pTrack || rAction.GetUser() == pTrack->GetUser() )
                return FALSE;
        }
        else if ( rAction.GetUser() != rSettings.GetTheAuthorToShow() )
            return FALSE;
    }

    if ( rSettings.HasComment() )
    {
        String aComStr=rAction.GetComment();
        aComStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " (" ));
        rAction.GetDescription( aComStr, &rDocument );
        aComStr+=')';

        if(!rSettings.IsValidComment(&aComStr))
            return FALSE;
    }

    if ( rSettings.HasRange() )
        if ( !rSettings.GetTheRangeList().Intersects( rAction.GetBigRange().MakeRange() ) )
            return FALSE;

    if ( rSettings.HasDate() && rSettings.GetTheDateMode() != SCDM_NO_DATEMODE )
    {
        DateTime aDateTime = rAction.GetDateTime();
        const DateTime& rFirst = rSettings.GetTheFirstDateTime();
        const DateTime& rLast  = rSettings.GetTheLastDateTime();
        switch ( rSettings.GetTheDateMode() )
        {   // korrespondiert mit ScHighlightChgDlg::OKBtnHdl
            case SCDM_DATE_BEFORE:
                if ( aDateTime > rFirst )
                    return FALSE;
                break;

            case SCDM_DATE_SINCE:
                if ( aDateTime < rFirst )
                    return FALSE;
                break;

            case SCDM_DATE_EQUAL:
            case SCDM_DATE_BETWEEN:
                if ( aDateTime < rFirst || aDateTime > rLast )
                    return FALSE;
                break;

            case SCDM_DATE_NOTEQUAL:
                if ( aDateTime >= rFirst && aDateTime <= rLast )
                    return FALSE;
                break;

            case SCDM_DATE_SAVE:
                {
                ScChangeTrack* pTrack = rDocument.GetChangeTrack();
                if ( !pTrack || pTrack->GetLastSavedActionNumber() >=
                        rAction.GetActionNumber() )
                    return FALSE;
                }
                break;
        }
    }

    return TRUE;
}

//==================================================================

BOOL ScViewUtil::ExecuteCharMap( const SvxFontItem& rOldFont,
                                 SvxFontItem&       rNewFont,
                                 String&            rString )
{
    BOOL bRet = FALSE;

    Font aFont;
    aFont.SetName    ( rOldFont.GetFamilyName() );
    aFont.SetStyleName( rOldFont.GetStyleName() );
    aFont.SetFamily  ( rOldFont.GetFamily() );
    aFont.SetCharSet  ( rOldFont.GetCharSet() );
    aFont.SetPitch   ( rOldFont.GetPitch() );

    SvxCharacterMap* pDlg = new SvxCharacterMap( NULL, FALSE );
    pDlg->SetCharFont( aFont );

    if ( pDlg->Execute() == RET_OK )
    {
        rString  = pDlg->GetCharacters();
        aFont    = pDlg->GetCharFont();
        rNewFont = SvxFontItem( aFont.GetFamily(), aFont.GetName(),
                                aFont.GetStyleName(), aFont.GetPitch(),
                                aFont.GetCharSet() );                   //  nId egal

        bRet = TRUE;
    }
    delete pDlg;

    return bRet;
}

//------------------------------------------------------------------

ScUpdateRect::ScUpdateRect( USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2 )
{
    PutInOrder( nX1, nX2 );
    PutInOrder( nY1, nY2 );

    nOldStartX = nX1;
    nOldStartY = nY1;
    nOldEndX = nX2;
    nOldEndY = nY2;
}

void ScUpdateRect::SetNew( USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2 )
{
    PutInOrder( nX1, nX2 );
    PutInOrder( nY1, nY2 );

    nNewStartX = nX1;
    nNewStartY = nY1;
    nNewEndX = nX2;
    nNewEndY = nY2;
}

BOOL ScUpdateRect::GetDiff( USHORT& rX1, USHORT& rY1, USHORT& rX2, USHORT& rY2 )
{
    if ( nNewStartX == nOldStartX && nNewEndX == nOldEndX &&
         nNewStartY == nOldStartY && nNewEndY == nOldEndY )
    {
        rX1 = nNewStartX;
        rY1 = nNewStartY;
        rX2 = nNewStartX;
        rY2 = nNewStartY;
        return FALSE;
    }

    rX1 = Min(nNewStartX,nOldStartX);
    rY1 = Min(nNewStartY,nOldStartY);
    rX2 = Max(nNewEndX,nOldEndX);
    rY2 = Max(nNewEndY,nOldEndY);

    if ( nNewStartX == nOldStartX && nNewEndX == nOldEndX )
    {
        if ( nNewStartY == nOldStartY )
        {
            rY1 = Min( nNewEndY, nOldEndY );
            rY2 = Max( nNewEndY, nOldEndY );
        }
        else if ( nNewEndY == nOldEndY )
        {
            rY1 = Min( nNewStartY, nOldStartY );
            rY2 = Max( nNewStartY, nOldStartY );
        }
    }
    else if ( nNewStartY == nOldStartY && nNewEndY == nOldEndY )
    {
        if ( nNewStartX == nOldStartX )
        {
            rX1 = Min( nNewEndX, nOldEndX );
            rX2 = Max( nNewEndX, nOldEndX );
        }
        else if ( nNewEndX == nOldEndX )
        {
            rX1 = Min( nNewStartX, nOldStartX );
            rX2 = Max( nNewStartX, nOldStartX );
        }
    }

    return TRUE;
}

BOOL ScUpdateRect::GetXorDiff( USHORT& rX1, USHORT& rY1, USHORT& rX2, USHORT& rY2, BOOL& rCont )
{
    rCont = FALSE;

    if ( nNewStartX == nOldStartX && nNewEndX == nOldEndX &&
         nNewStartY == nOldStartY && nNewEndY == nOldEndY )
    {
        rX1 = nNewStartX;
        rY1 = nNewStartY;
        rX2 = nNewStartX;
        rY2 = nNewStartY;
        return FALSE;
    }

    rX1 = Min(nNewStartX,nOldStartX);
    rY1 = Min(nNewStartY,nOldStartY);
    rX2 = Max(nNewEndX,nOldEndX);
    rY2 = Max(nNewEndY,nOldEndY);

    if ( nNewStartX == nOldStartX && nNewEndX == nOldEndX )             // nur vertikal
    {
        if ( nNewStartY == nOldStartY )
        {
            rY1 = Min( nNewEndY, nOldEndY ) + 1;
            rY2 = Max( nNewEndY, nOldEndY );
        }
        else if ( nNewEndY == nOldEndY )
        {
            rY1 = Min( nNewStartY, nOldStartY );
            rY2 = Max( nNewStartY, nOldStartY ) - 1;
        }
        else
        {
            rY1 = Min( nNewStartY, nOldStartY );
            rY2 = Max( nNewStartY, nOldStartY ) - 1;
            rCont = TRUE;
            nContY1 = Min( nNewEndY, nOldEndY ) + 1;
            nContY2 = Max( nNewEndY, nOldEndY );
            nContX1 = rX1;
            nContX2 = rX2;
        }
    }
    else if ( nNewStartY == nOldStartY && nNewEndY == nOldEndY )        // nur horizontal
    {
        if ( nNewStartX == nOldStartX )
        {
            rX1 = Min( nNewEndX, nOldEndX ) + 1;
            rX2 = Max( nNewEndX, nOldEndX );
        }
        else if ( nNewEndX == nOldEndX )
        {
            rX1 = Min( nNewStartX, nOldStartX );
            rX2 = Max( nNewStartX, nOldStartX ) - 1;
        }
        else
        {
            rX1 = Min( nNewStartX, nOldStartX );
            rX2 = Max( nNewStartX, nOldStartX ) - 1;
            rCont = TRUE;
            nContX1 = Min( nNewEndX, nOldEndX ) + 1;
            nContX2 = Max( nNewEndX, nOldEndX );
            nContY1 = rY1;
            nContY2 = rY2;
        }
    }
    else if ( nNewEndX == nOldEndX && nNewEndY == nOldEndY )            // links oben
    {
        if ( (nNewStartX<nOldStartX) == (nNewStartY<nOldStartY) )
            rX1 = Min( nNewStartX, nOldStartX );
        else
            rX1 = Max( nNewStartX, nOldStartX );            // Ecke weglassen
        rX2 = nOldEndX;
        rY1 = Min( nNewStartY, nOldStartY );                // oben
        rY2 = Max( nNewStartY, nOldStartY ) - 1;
        rCont = TRUE;
        nContY1 = rY2+1;
        nContY2 = nOldEndY;
        nContX1 = Min( nNewStartX, nOldStartX );            // links
        nContX2 = Max( nNewStartX, nOldStartX ) - 1;
    }
    else if ( nNewStartX == nOldStartX && nNewEndY == nOldEndY )        // rechts oben
    {
        if ( (nNewEndX<nOldEndX) != (nNewStartY<nOldStartY) )
            rX2 = Max( nNewEndX, nOldEndX );
        else
            rX2 = Min( nNewEndX, nOldEndX );                // Ecke weglassen
        rX1 = nOldStartX;
        rY1 = Min( nNewStartY, nOldStartY );                // oben
        rY2 = Max( nNewStartY, nOldStartY ) - 1;
        rCont = TRUE;
        nContY1 = rY2+1;
        nContY2 = nOldEndY;
        nContX1 = Min( nNewEndX, nOldEndX ) + 1;            // rechts
        nContX2 = Max( nNewEndX, nOldEndX );
    }
    else if ( nNewEndX == nOldEndX && nNewStartY == nOldStartY )        // links unten
    {
        if ( (nNewStartX<nOldStartX) != (nNewEndY<nOldEndY) )
            rX1 = Min( nNewStartX, nOldStartX );
        else
            rX1 = Max( nNewStartX, nOldStartX );            // Ecke weglassen
        rX2 = nOldEndX;
        rY1 = Min( nNewEndY, nOldEndY ) + 1;                // unten
        rY2 = Max( nNewEndY, nOldEndY );
        rCont = TRUE;
        nContY1 = nOldStartY;
        nContY2 = rY1-1;
        nContX1 = Min( nNewStartX, nOldStartX );            // links
        nContX2 = Max( nNewStartX, nOldStartX ) - 1;
    }
    else if ( nNewStartX == nOldStartX && nNewStartY == nOldStartY )    // rechts unten
    {
        if ( (nNewEndX<nOldEndX) == (nNewEndY<nOldEndY) )
            rX2 = Max( nNewEndX, nOldEndX );
        else
            rX2 = Min( nNewEndX, nOldEndX );                // Ecke weglassen
        rX1 = nOldStartX;
        rY1 = Min( nNewEndY, nOldEndY ) + 1;                // unten
        rY2 = Max( nNewEndY, nOldEndY );
        rCont = TRUE;
        nContY1 = nOldStartY;
        nContY2 = rY1-1;
        nContX1 = Min( nNewEndX, nOldEndX ) + 1;            // rechts
        nContX2 = Max( nNewEndX, nOldEndX );
    }
    else                                                                // Ueberschlag
    {
        rX1 = nOldStartX;
        rY1 = nOldStartY;
        rX2 = nOldEndX;
        rY2 = nOldEndY;
        rCont = TRUE;
        nContX1 = nNewStartX;
        nContY1 = nNewStartY;
        nContX2 = nNewEndX;
        nContY2 = nNewEndY;
    }

    return TRUE;
}

void ScUpdateRect::GetContDiff( USHORT& rX1, USHORT& rY1, USHORT& rX2, USHORT& rY2 )
{
    rX1 = nContX1;
    rY1 = nContY1;
    rX2 = nContX2;
    rY2 = nContY2;
}






