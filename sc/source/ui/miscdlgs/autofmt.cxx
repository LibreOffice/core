/*************************************************************************
 *
 *  $RCSfile: autofmt.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-20 09:35:14 $
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

//------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <svx/boxitem.hxx>
#include <svx/brshitem.hxx>
#include <svx/cntritem.hxx>
#include <svx/colritem.hxx>
#include <svx/crsditem.hxx>
#include <svx/fontitem.hxx>
#include <svx/postitem.hxx>
#include <svx/shdditem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/wghtitem.hxx>
#include <svtools/zforlist.hxx>
#include <vcl/msgbox.hxx>
#include <comphelper/processfactory.hxx>

#include "sc.hrc"
#include "attrib.hxx"
#include "zforauto.hxx"
#include "scitems.hxx"
#include "global.hxx"
#include "globstr.hrc"
#include "autoform.hxx"
#include "sclnlnk.hxx"
#include "strindlg.hxx"
#include "miscdlgs.hrc"
#include "autofmt.hxx"
#include "scresid.hxx"

#define FRAME_OFFSET 4

USHORT AutoFmtPreview::aFmtMap[] = { 0,  1,  2,  1,  3, // Zuordnung:
                                     4,  5,  6,  5,  7, // Zelle->Format
                                     8,  9, 10,  9, 11,
                                     4,  5,  6,  5,  7,
                                    12, 13, 14, 13, 15  };

BOOL bIsOlk = FALSE;


//========================================================================
// AutoFormat-Dialog:

ScAutoFormatDlg::ScAutoFormatDlg( Window*                   pParent,
                                  ScAutoFormat*             pAutoFormat,
                                  const ScAutoFormatData*   pSelFormatData ) :

    ModalDialog     ( pParent, ScResId( RID_SCDLG_AUTOFORMAT ) ),
    //
    aLbFormat       ( this, ScResId( LB_FORMAT ) ),
    aFtFormat       ( this, ScResId( FT_FORMAT ) ),
    aGbPreview      ( this, ScResId( GB_PREVIEW ) ),
    pWndPreview     ( new AutoFmtPreview( this, ScResId( WND_PREVIEW ) ) ),
    aBtnNumFormat   ( this, ScResId( BTN_NUMFORMAT ) ),
    aBtnBorder      ( this, ScResId( BTN_BORDER ) ),
    aBtnFont        ( this, ScResId( BTN_FONT ) ),
    aBtnPattern     ( this, ScResId( BTN_PATTERN ) ),
    aBtnAlignment   ( this, ScResId( BTN_ALIGNMENT ) ),
    aBtnAdjust      ( this, ScResId( BTN_ADJUST ) ),
    aGbFormat       ( this, ScResId( GB_FORMAT ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) ),
    aBtnAdd         ( this, ScResId( BTN_ADD ) ),
    aBtnRemove      ( this, ScResId( BTN_REMOVE ) ),
    aBtnMore        ( this, ScResId( BTN_MORE ) ),
    aBtnRename      ( this, ScResId( BTN_RENAME ) ),
    aStrTitle       ( String( ScResId( STR_ADD_TITLE ) ) ),
    aStrLabel       ( String( ScResId( STR_ADD_LABEL ) ) ),
    aStrRename      ( String( ScResId( STR_RENAME_TITLE) ) ),
    aStrClose       ( String( ScResId( STR_BTN_CLOSE ) ) ),
    aStrDelTitle    ( String( ScResId( STR_DEL_TITLE ) ) ),
    aStrDelMsg      ( String( ScResId( STR_DEL_MSG ) ) ),
    //
    nIndex          ( 0 ),
    bFmtInserted    ( FALSE ),
    bCoreDataChanged( FALSE ),
    pFormat         ( pAutoFormat ),
    pSelFmtData     ( pSelFormatData )
{
    Init();
    pWndPreview->NotifyChange( (*pFormat)[0] );
    FreeResource();
}

//------------------------------------------------------------------------

__EXPORT ScAutoFormatDlg::~ScAutoFormatDlg()
{
    delete pWndPreview;
}

//------------------------------------------------------------------------

void ScAutoFormatDlg::Init()
{
    USHORT nCount;
    String aEntry;

    aLbFormat    .SetSelectHdl( LINK( this, ScAutoFormatDlg, SelFmtHdl ) );
    aBtnNumFormat.SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    aBtnBorder   .SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    aBtnFont     .SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    aBtnPattern  .SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    aBtnAlignment.SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    aBtnAdjust   .SetClickHdl ( LINK( this, ScAutoFormatDlg, CheckHdl ) );
    aBtnAdd      .SetClickHdl ( LINK( this, ScAutoFormatDlg, AddHdl ) );
    aBtnRemove   .SetClickHdl ( LINK( this, ScAutoFormatDlg, RemoveHdl ) );
    aBtnOk       .SetClickHdl ( LINK( this, ScAutoFormatDlg, CloseHdl ) );
    aBtnCancel   .SetClickHdl ( LINK( this, ScAutoFormatDlg, CloseHdl ) );
    aBtnRename   .SetClickHdl ( LINK( this, ScAutoFormatDlg, RenameHdl ) );
    aLbFormat    .SetDoubleClickHdl( LINK( this, ScAutoFormatDlg, DblClkHdl ) );

    aBtnMore.AddWindow( &aBtnRename );
    aBtnMore.AddWindow( &aBtnNumFormat );
    aBtnMore.AddWindow( &aBtnBorder );
    aBtnMore.AddWindow( &aBtnFont );
    aBtnMore.AddWindow( &aBtnPattern );
    aBtnMore.AddWindow( &aBtnAlignment );
    aBtnMore.AddWindow( &aBtnAdjust );
    aBtnMore.AddWindow( &aGbFormat );

    nCount = pFormat->GetCount();

    for ( USHORT i = 0; i < nCount; i++ )
    {
        ((*pFormat)[i])->GetName( aEntry );
        aLbFormat.InsertEntry( aEntry );
    }

    if ( nCount == 1 )
        aBtnRemove.Disable();

    aLbFormat.SelectEntryPos( 0 );
    aBtnRename.Disable();
    aBtnRemove.Disable();

    nIndex = 0;
    UpdateChecks();

    if ( !pSelFmtData )
    {
        aBtnAdd.Disable();
        aBtnRemove.Disable();
        bFmtInserted = TRUE;
    }
}

//------------------------------------------------------------------------

void ScAutoFormatDlg::UpdateChecks()
{
    ScAutoFormatData* pData = (*pFormat)[nIndex];

    aBtnNumFormat.Check( pData->GetIncludeValueFormat() );
    aBtnBorder   .Check( pData->GetIncludeFrame() );
    aBtnFont     .Check( pData->GetIncludeFont() );
    aBtnPattern  .Check( pData->GetIncludeBackground() );
    aBtnAlignment.Check( pData->GetIncludeJustify() );
    aBtnAdjust   .Check( pData->GetIncludeWidthHeight() );
}

//------------------------------------------------------------------------
// Handler:
//---------

IMPL_LINK( ScAutoFormatDlg, CloseHdl, PushButton *, pBtn )
{
    if ( pBtn == &aBtnOk || pBtn == &aBtnCancel )
    {
        if ( bCoreDataChanged )
            ScGlobal::GetAutoFormat()->Save();

        EndDialog( (pBtn == &aBtnOk) ? RET_OK : RET_CANCEL );
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( ScAutoFormatDlg, DblClkHdl, void *, EMPTYARG )
{
    if ( bCoreDataChanged )
        ScGlobal::GetAutoFormat()->Save();

    EndDialog( RET_OK );
    return 0;
}
IMPL_LINK_INLINE_END( ScAutoFormatDlg, DblClkHdl, void *, EMPTYARG )

//------------------------------------------------------------------------

IMPL_LINK( ScAutoFormatDlg, CheckHdl, Button *, pBtn )
{
    ScAutoFormatData* pData  = (*pFormat)[nIndex];
    BOOL              bCheck = ((CheckBox*)pBtn)->IsChecked();

    if ( pBtn == &aBtnNumFormat )
        pData->SetIncludeValueFormat( bCheck );
    else if ( pBtn == &aBtnBorder )
        pData->SetIncludeFrame( bCheck );
    else if ( pBtn == &aBtnFont )
        pData->SetIncludeFont( bCheck );
    else if ( pBtn == &aBtnPattern )
        pData->SetIncludeBackground( bCheck );
    else if ( pBtn == &aBtnAlignment )
        pData->SetIncludeJustify( bCheck );
    else if ( pBtn == &aBtnAdjust )
        pData->SetIncludeWidthHeight( bCheck );

    if ( !bCoreDataChanged )
    {
        aBtnCancel.SetText( aStrClose );
        bCoreDataChanged = TRUE;
    }

    pWndPreview->NotifyChange( pData );

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( ScAutoFormatDlg, AddHdl, void *, EMPTYARG )
{
    if ( !bFmtInserted && pSelFmtData )
    {
        String              aStrStandard( ScResId(STR_STANDARD) );
        String              aFormatName;
        ScStringInputDlg*   pDlg;
        BOOL                bOk = FALSE;

        while ( !bOk )
        {
            pDlg = new ScStringInputDlg( this,
                                         aStrTitle,
                                         aStrLabel,
                                         aFormatName,
                                         HID_SC_ADD_AUTOFMT );

            if ( pDlg->Execute() == RET_OK )
            {
                pDlg->GetInputString( aFormatName );

                if ( (aFormatName.Len() > 0) && (aFormatName != aStrStandard) )
                {
                    ScAutoFormatData* pNewData
                        = new ScAutoFormatData( *pSelFmtData );

                    pNewData->SetName( aFormatName );
                    bFmtInserted = pFormat->Insert( pNewData );

                    if ( bFmtInserted )
                    {
                        USHORT nAt = pFormat->IndexOf( pNewData );

                        aLbFormat.InsertEntry( aFormatName, nAt );
                        aLbFormat.SelectEntry( aFormatName );
                        aBtnAdd.Disable();

                        if ( !bCoreDataChanged )
                        {
                            aBtnCancel.SetText( aStrClose );
                            bCoreDataChanged = TRUE;
                        }

                        SelFmtHdl( 0 );
                        bOk = TRUE;
                    }
                    else
                        delete pNewData;

                }

                if ( !bFmtInserted )
                {
                    USHORT nRet = ErrorBox( this,
                                            WinBits( WB_OK_CANCEL | WB_DEF_OK),
                                            ScGlobal::GetRscString(STR_INVALID_AFNAME)
                                          ).Execute();

                    bOk = ( nRet == RET_CANCEL );
                }
            }
            else
                bOk = TRUE;

            delete pDlg;
        }
    }

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( ScAutoFormatDlg, RemoveHdl, void *, EMPTYARG )
{
    if ( (nIndex > 0) && (aLbFormat.GetEntryCount() > 0) )
    {
        String aMsg( aStrDelMsg.GetToken( 0, '#' ) );

        aMsg += aLbFormat.GetSelectEntry();
        aMsg += aStrDelMsg.GetToken( 1, '#' );

        if ( RET_YES ==
             QueryBox( this, WinBits( WB_YES_NO | WB_DEF_YES ), aMsg ).Execute() )
        {
            aLbFormat.RemoveEntry( nIndex );
            aLbFormat.SelectEntryPos( nIndex-1 );

            if ( nIndex-1 == 0 )
                aBtnRemove.Disable();

            if ( !bCoreDataChanged )
            {
                aBtnCancel.SetText( aStrClose );
                bCoreDataChanged = TRUE;
            }

            pFormat->AtFree( nIndex ); // in der Core loeschen
            nIndex--;

            SelFmtHdl( 0 );
        }
    }

    SelFmtHdl( 0 );

    return 0;
}

IMPL_LINK( ScAutoFormatDlg, RenameHdl, void *, pBtn)
{
    BOOL bOk = FALSE;
    while( !bOk )
    {

        String aFormatName=aLbFormat.GetSelectEntry();
        String aEntry;

        ScStringInputDlg* pDlg = new ScStringInputDlg( this,
                                         aStrRename,
                                         aStrLabel,
                                         aFormatName,
                                         HID_SC_RENAME_AUTOFMT );
        if( pDlg->Execute() == RET_OK )
        {
            BOOL bFmtRenamed = FALSE;
            pDlg->GetInputString( aFormatName );

            if ( aFormatName.Len() > 0 )
            {
                for( USHORT n = 0; n < pFormat->GetCount(); ++n )
                {
                    (*pFormat)[n]->GetName(aEntry);
                    if ( aEntry== aFormatName)
                        break;
                }
                if( n >= pFormat->GetCount() )
                {
                    // Format mit dem Namen noch nicht vorhanden, also
                    // umbenennen

                    aLbFormat.RemoveEntry(nIndex );
                    ScAutoFormatData* p=(*pFormat)[ nIndex ];
                    ScAutoFormatData* pNewData
                        = new ScAutoFormatData(*p);

                    pFormat->AtFree( nIndex );

                    pNewData->SetName( aFormatName );

                    pFormat->Insert( pNewData);

                    USHORT nCount = pFormat->GetCount();

                    aLbFormat.SetUpdateMode(FALSE);
                    aLbFormat.Clear();
                    for ( USHORT i = 0; i < nCount; i++ )
                    {
                        ((*pFormat)[i])->GetName( aEntry );
                        aLbFormat.InsertEntry( aEntry );
                    }

                    aLbFormat.SetUpdateMode( TRUE);
                    aLbFormat.SelectEntry( aFormatName);

                    if ( !bCoreDataChanged )
                    {
                        aBtnCancel.SetText( aStrClose );
                        bCoreDataChanged = TRUE;
                    }


                    SelFmtHdl( 0 );
                    bOk = TRUE;
                    bFmtRenamed = TRUE;
                }
            }
            if( !bFmtRenamed )
            {
                bOk = RET_CANCEL == ErrorBox( this,
                                    WinBits( WB_OK_CANCEL | WB_DEF_OK),
                                    ScGlobal::GetRscString(STR_INVALID_AFNAME)
                                    ).Execute();
            }
        }
        else
            bOk = TRUE;
        delete pDlg;
    }

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( ScAutoFormatDlg, SelFmtHdl, void *, EMPTYARG )
{
    nIndex = aLbFormat.GetSelectEntryPos();
    UpdateChecks();

    if ( nIndex == 0 )
    {
        aBtnRename.Disable();
        aBtnRemove.Disable();
    }
    else
    {
        aBtnRename.Enable();
        aBtnRemove.Enable();
    }

    pWndPreview->NotifyChange( (*pFormat)[nIndex] );

    return 0;
}

//------------------------------------------------------------------------

String __EXPORT ScAutoFormatDlg::GetCurrFormatName()
{
    String  aResult;

    ((*pFormat)[nIndex])->GetName( aResult );

    return aResult;
}

//========================================================================
// AutoFmtPreview

AutoFmtPreview::AutoFmtPreview( Window* pParent, const ResId& rRes ) :
        Window          ( pParent, rRes ),
        aVD             ( *this ),
        pCurData        ( NULL ),
        bFitWidth       ( FALSE ),
        aStrJan         ( String( ScResId( STR_JAN ) ) ),
        aStrFeb         ( String( ScResId( STR_FEB ) ) ),
        aStrMar         ( String( ScResId( STR_MAR ) ) ),
        aStrNorth       ( String( ScResId( STR_NORTH ) ) ),
        aStrMid         ( String( ScResId( STR_MID ) ) ),
        aStrSouth       ( String( ScResId( STR_SOUTH ) ) ),
        aStrSum         ( String( ScResId( STR_SUM ) ) ),
        aPrvSize        ( GetSizePixel().Width()  - 6,
                          GetSizePixel().Height() - 30 ),
        nLabelColWidth  ( (USHORT)(((aPrvSize.Width()-4)/4)-12) ),
        nDataColWidth1  ( (USHORT)(((aPrvSize.Width()-4)-(nLabelColWidth*2)) / 3) ),
        nDataColWidth2  ( (USHORT)(((aPrvSize.Width()-4)-(nLabelColWidth*2)) / 4) ),
        nRowHeight      ( (USHORT)((aPrvSize.Height()-4) / 5) ),
        pNumFmt         ( new SvNumberFormatter( ::comphelper::getProcessServiceFactory(), ScGlobal::eLnge ) )
{
    Init();
    if( bIsOlk )
        ((String&)aStrMar).AssignAscii(RTL_CONSTASCII_STRINGPARAM( "Olk" ));
}

//------------------------------------------------------------------------

__EXPORT AutoFmtPreview::~AutoFmtPreview()
{
    for ( USHORT i=0; i<=35; i++ )
    {
        delete aLinePtrArray[i];
    }
    delete pNumFmt;
}

//------------------------------------------------------------------------

void lcl_AssignLine( SvxBorderLine&      dest,
                    const SvxBorderLine* src )
{
    dest.SetColor    ( (src) ? src->GetColor()    : Color( COL_WHITE ) );
    dest.SetOutWidth ( (src) ? src->GetOutWidth() : 0 );
    dest.SetInWidth  ( (src) ? src->GetInWidth()  : 0 );
    dest.SetDistance ( (src) ? src->GetDistance() : 0 );
}

//------------------------------------------------------------------------

void lcl_GetLineStruct( ScLineStruct&       rLine,
                       const SvxBorderLine& rBoxLine )
{
    if ( rBoxLine.GetOutWidth() > 0 )
    {
        rLine.nLeft     = rBoxLine.GetOutWidth();
        rLine.nMiddle   = rBoxLine.GetDistance();
        rLine.nRight    = rBoxLine.GetInWidth();

        /* Linienstaerke auf dick/duenn abbilden:
         *
         * (in TWIPS, 1pt = 20 TWIPS = duenn)
         * alles was <=0  ist -> (0,0,0)
         * alles was <=20 ist -> (1,0,0)/(1,1,1)
         * alles andere       -> (3,0,0)/(2,1,2)
         */

        if ( rLine.nMiddle == 0 ) // einfache Linie?
        {
            rLine.nRight = 0; // sicher ist sicher

            if ( rLine.nLeft <= 20 )
            {
                rLine.nLeft = 1;
            }
            else if ( rLine.nLeft > 20 )
            {
                rLine.nLeft = 3;
            }
        }
        else // doppelte Linie
        {
            rLine.nMiddle = 1;
            if ( rLine.nLeft <= 20 )
            {
                rLine.nLeft = rLine.nRight = 1;
            }
            else if ( rLine.nLeft > 20 )
            {
                rLine.nLeft = rLine.nRight = 2;
            }
        }
    }
    else
    {
        rLine.nLeft     = 0;
        rLine.nMiddle   = 0;
        rLine.nRight    = 0;
    }
}

//------------------------------------------------------------------------

void AutoFmtPreview::CheckPriority( USHORT          nCurLine,
                                    AutoFmtLine     eLine,
                                    SvxBorderLine&  rLine )
{
    const SvxBorderLine*    pDrawLine;
    USHORT                  nOther = 0;

    switch ( eLine )
    {
        case TOP_LINE:
            {
                lcl_AssignLine( rLine, aLinePtrArray[nCurLine]->GetTop() );
                nOther = nCurLine-7;
                pDrawLine = aLinePtrArray[nOther]->GetBottom();

                if ( HasPriority( pDrawLine, &rLine ) )
                    lcl_AssignLine( rLine, pDrawLine );
            }
            break;

        case BOTTOM_LINE:
            {
                lcl_AssignLine( rLine, aLinePtrArray[nCurLine]->GetBottom() );
                nOther = nCurLine+7;
                pDrawLine = aLinePtrArray[nOther]->GetTop();

                if ( HasPriority( pDrawLine, &rLine ) )
                    lcl_AssignLine( rLine, pDrawLine );
            }
            break;

        case LEFT_LINE:
            {
                lcl_AssignLine( rLine, aLinePtrArray[nCurLine]->GetLeft() );
                nOther = nCurLine-1;
                pDrawLine = aLinePtrArray[nOther]->GetRight();

                if ( HasPriority( pDrawLine, &rLine ) )
                    lcl_AssignLine( rLine, pDrawLine );
            }
            break;

        case RIGHT_LINE:
            {
                lcl_AssignLine( rLine, aLinePtrArray[nCurLine]->GetRight() );
                nOther = nCurLine+1;
                pDrawLine = aLinePtrArray[nOther]->GetLeft();

                if ( HasPriority( pDrawLine, &rLine ) )
                    lcl_AssignLine( rLine, pDrawLine );
            }
            break;
    }
}

//------------------------------------------------------------------------

void AutoFmtPreview::GetLines( USHORT nIndex, AutoFmtLine eLine,
                               SvxBorderLine&   rLineD,
                               SvxBorderLine&   rLineLT,
                               SvxBorderLine&   rLineL,
                               SvxBorderLine&   rLineLB,
                               SvxBorderLine&   rLineRT,
                               SvxBorderLine&   rLineR,
                               SvxBorderLine&   rLineRB )
{
    {
        SvxBorderLine aNullLine;

        lcl_AssignLine( rLineD,  &aNullLine);
        lcl_AssignLine( rLineLT, &aNullLine);
        lcl_AssignLine( rLineL,  &aNullLine);
        lcl_AssignLine( rLineLB, &aNullLine);
        lcl_AssignLine( rLineRT, &aNullLine);
        lcl_AssignLine( rLineR,  &aNullLine);
        lcl_AssignLine( rLineRB, &aNullLine);
    }

    if ( pCurData )
    {
        USHORT  nCurLine   = nIndex + 8 + ((nIndex/5)*2);
        USHORT  nOther     = 0;

        switch ( eLine )
        {
            case TOP_LINE:
                {
                    // obere Linien werden nur in der
                    // ersten Zeile gemalt:
                    if ( (nIndex >= 0) && (nIndex <= 4) )
                    {
                        // links
                        CheckPriority( nCurLine, eLine, rLineD );
                        nOther = nCurLine-7;
                        CheckPriority( nOther, LEFT_LINE, rLineLT );
                        nOther = nCurLine-1;
                        CheckPriority( nOther, TOP_LINE, rLineL );
                        CheckPriority( nCurLine, LEFT_LINE, rLineLB );
                        // rechts
                        nOther = nCurLine-7;
                        CheckPriority( nOther, RIGHT_LINE, rLineRT );
                        nOther = nCurLine+1;
                        CheckPriority( nOther, TOP_LINE, rLineR );
                        CheckPriority( nCurLine, RIGHT_LINE, rLineRB );
                    }
                }
                break;

            case BOTTOM_LINE:
                {
                    // links
                    CheckPriority( nCurLine, eLine, rLineD );
                    CheckPriority( nCurLine, LEFT_LINE, rLineLT );
                    nOther = nCurLine-1;
                    CheckPriority( nOther, BOTTOM_LINE, rLineL );
                    nOther = nCurLine+7;
                    CheckPriority( nOther, LEFT_LINE, rLineLB );
                    // rechts
                    CheckPriority( nCurLine, RIGHT_LINE, rLineRT );
                    nOther = nCurLine+1;
                    CheckPriority( nOther, BOTTOM_LINE, rLineR );
                    nOther = nCurLine+7;
                    CheckPriority( nOther, RIGHT_LINE, rLineRB );
                }
                break;

            case LEFT_LINE:
                {
                    // linke Linien werden nur in der
                    // ersten Spalte gemalt:
                    if ( (nIndex%5) == 0 )
                    {
                        // oben
                        CheckPriority( nCurLine, eLine, rLineD );
                        CheckPriority( nCurLine, TOP_LINE, rLineLT );
                        nOther = nCurLine-7;
                        CheckPriority( nOther, LEFT_LINE, rLineL );
                        nOther = nCurLine-1;
                        CheckPriority( nOther, TOP_LINE, rLineLB );
                        // unten
                        CheckPriority( nCurLine, BOTTOM_LINE, rLineRT );
                        nOther = nCurLine+7;
                        CheckPriority( nOther, LEFT_LINE, rLineR );
                        nOther = nCurLine-1;
                        CheckPriority( nOther, BOTTOM_LINE, rLineRB );
                    }
                }
                break;

            case RIGHT_LINE:
                {
                    // oben
                    CheckPriority( nCurLine, eLine, rLineD );
                    nOther = nCurLine+1;
                    CheckPriority( nOther, TOP_LINE, rLineLT );
                    nOther = nCurLine-7;
                    CheckPriority( nOther, RIGHT_LINE, rLineL );
                    CheckPriority( nCurLine, TOP_LINE, rLineLB );
                    // unten
                    nOther = nCurLine+1;
                    CheckPriority( nOther, BOTTOM_LINE, rLineRT );
                    nOther = nCurLine+7;
                    CheckPriority( nOther, RIGHT_LINE, rLineR );
                    CheckPriority( nCurLine, BOTTOM_LINE, rLineRB );
                }
                break;
        }
    }
}

//------------------------------------------------------------------------

void AutoFmtPreview::DrawFrameLine( const SvxBorderLine&    rLineD,
                                    Point                   from,
                                    Point                   to,
                                    BOOL                    bHorizontal,
                                    const SvxBorderLine&    rLineLT,
                                    const SvxBorderLine&    rLineL,
                                    const SvxBorderLine&    rLineLB,
                                    const SvxBorderLine&    rLineRT,
                                    const SvxBorderLine&    rLineR,
                                    const SvxBorderLine&    rLineRB )
{
    if ( pCurData )
    {
        ScLineStruct    dLine;
        ScLineStruct    ltLine;
        ScLineStruct    lLine;
        ScLineStruct    lbLine;
        ScLineStruct    rtLine;
        ScLineStruct    rLine;
        ScLineStruct    rbLine;
        short           dxArr[4];

        lcl_GetLineStruct( dLine,   rLineD  );
        lcl_GetLineStruct( ltLine,  rLineLT );
        lcl_GetLineStruct( lLine,   rLineL  );
        lcl_GetLineStruct( lbLine,  rLineLB );
        lcl_GetLineStruct( rtLine,  rLineRT );
        lcl_GetLineStruct( rLine,   rLineR  );
        lcl_GetLineStruct( rbLine,  rLineRB );

        if ( dLine.nLeft > 0 )
        {
            BOOL bHadLine  = aVD.IsLineColor();
            Color aOldLine = aVD.GetLineColor();
            BOOL bHadFill  = aVD.IsFillColor();
            Color aOldFill = aVD.GetFillColor();

            USHORT  nHeight  = dLine.nLeft + dLine.nMiddle + dLine.nRight;
            Point   from2    = from;
            Point   to2      = to;

            aVD.SetLineColor();
            aVD.SetFillColor( rLineD.GetColor() );

            ScLinkLine( dLine,
                        ltLine, lLine, lbLine,
                        rtLine, rLine, rbLine,
                        dxArr );

            if ( bHorizontal )
            {
                from.Y() -= nHeight/2;
                to.Y()   -= nHeight/2;
                to.Y()   += (dLine.nLeft-1);

                from.X() += dxArr[0];
                to.X()   += dxArr[2];

                aVD.DrawRect( Rectangle( from, to ) );

                // noch eine zweite Linie zu malen?
                if ( dLine.nRight != 0 )
                {
                    from2.Y() -= nHeight/2;
                    from2.Y() += dLine.nLeft+dLine.nMiddle;
                    to2.Y()   -= nHeight/2;
                    to2.Y()   += dLine.nMiddle+dLine.nLeft;
                    to2.Y()   += (dLine.nRight-1);

                    from2.X() += dxArr[1];
                    to2.X()   += dxArr[3];

                    aVD.DrawRect( Rectangle( from2, to2 ) );
                }
            }
            else
            {
                from.X() += nHeight/2;
                from.X() -= (dLine.nLeft-1);
                to.X()   += nHeight/2;

                from.Y() += dxArr[0];
                to.Y()   += dxArr[2];

                aVD.DrawRect( Rectangle( from, to ) );

                // noch eine zweite Linie zu malen?
                if ( dLine.nRight != 0 )
                {
                    from2.X() -= nHeight/2;
                    to2.X()   -= nHeight/2;
                    to2.X()   += (dLine.nRight-1);

                    from2.Y() += dxArr[1];
                    to2.Y()   += dxArr[3];

                    aVD.DrawRect( Rectangle( from2, to2 ) );
                }
            }

            if (bHadFill) aVD.SetFillColor(aOldFill); else aVD.SetFillColor();
            if (bHadLine) aVD.SetLineColor(aOldLine); else aVD.SetLineColor();
        }
    }
}

//------------------------------------------------------------------------

void AutoFmtPreview::DrawFrame( USHORT nIndex )
{
    if ( pCurData )
    {
        //----------------------
        // Malen des Zellrahmens
        //----------------------

        SvxBorderLine   aLineD;
        SvxBorderLine   aLineLT;
        SvxBorderLine   aLineL;
        SvxBorderLine   aLineLB;
        SvxBorderLine   aLineRT;
        SvxBorderLine   aLineR;
        SvxBorderLine   aLineRB;
        Rectangle       cellRect    = aCellArray[nIndex];

        //---------
        // TopLine ---------------------------------------------
        //---------
        GetLines( nIndex, TOP_LINE, aLineD,
                  aLineLT, aLineL, aLineLB,
                  aLineRT, aLineR, aLineRB );

        DrawFrameLine( aLineD,
                       cellRect.TopLeft(), cellRect.TopRight(), TRUE,
                       aLineLT,  aLineL, aLineLB,
                       aLineRT,  aLineR, aLineRB );

        //------------
        // BottomLine ------------------------------------------
        //------------
        GetLines( nIndex, BOTTOM_LINE, aLineD,
                  aLineLT, aLineL, aLineLB,
                  aLineRT, aLineR, aLineRB );

        DrawFrameLine( aLineD,
                       cellRect.BottomLeft(), cellRect.BottomRight(), TRUE,
                       aLineLT,  aLineL, aLineLB,
                       aLineRT,  aLineR, aLineRB );

        //----------
        // LeftLine --------------------------------------------
        //----------
        GetLines( nIndex, LEFT_LINE, aLineD,
                  aLineLT, aLineL, aLineLB,
                  aLineRT, aLineR, aLineRB );

        DrawFrameLine( aLineD,
                       cellRect.TopLeft(), cellRect.BottomLeft(), FALSE,
                       aLineLT,  aLineL, aLineLB,
                       aLineRT,  aLineR, aLineRB );

        //-----------
        // RightLine -------------------------------------------
        //-----------
        GetLines( nIndex, RIGHT_LINE, aLineD,
                  aLineLT, aLineL, aLineLB,
                  aLineRT, aLineR, aLineRB );

        DrawFrameLine( aLineD,
                       cellRect.TopRight(), cellRect.BottomRight(), FALSE,
                       aLineLT,  aLineL, aLineLB,
                       aLineRT,  aLineR, aLineRB );
    }
}

//------------------------------------------------------------------------

void AutoFmtPreview::MakeFont( USHORT nIndex, Font& rFont )
{
    if ( pCurData )
    {
        Size                aSize;
        SvxFontItem         aFontItem;
        SvxWeightItem       aFontWeightItem;
        SvxPostureItem      aFontPostureItem;
        SvxUnderlineItem    aFontUnderlineItem;
        SvxCrossedOutItem   aFontCrossedOutItem;
        SvxContourItem      aFontContourItem;
        SvxShadowedItem     aFontShadowedItem;
        SvxColorItem        aFontColorItem;

        rFont = GetFont();
        aSize = rFont.GetSize();
        aSize.Height() = 10;

        pCurData->GetFont           ( nIndex, aFontItem );
        pCurData->GetFontWeight     ( nIndex, aFontWeightItem );
        pCurData->GetFontPosture    ( nIndex, aFontPostureItem );
        pCurData->GetFontUnderline  ( nIndex, aFontUnderlineItem );
        pCurData->GetFontCrossedOut ( nIndex, aFontCrossedOutItem );
        pCurData->GetFontContour    ( nIndex, aFontContourItem );
        pCurData->GetFontShadowed   ( nIndex, aFontShadowedItem );
        pCurData->GetFontColor      ( nIndex, aFontColorItem );

        rFont.SetFamily     ( aFontItem.GetFamily() );
        rFont.SetName       ( aFontItem.GetFamilyName() );
        rFont.SetStyleName  ( aFontItem.GetStyleName() );
        rFont.SetCharSet    ( aFontItem.GetCharSet() );
        rFont.SetPitch      ( aFontItem.GetPitch() );
        rFont.SetWeight     ( (FontWeight)aFontWeightItem.GetValue() );
        rFont.SetUnderline  ( (FontUnderline)aFontUnderlineItem.GetValue() );
        rFont.SetStrikeout  ( (FontStrikeout)aFontCrossedOutItem.GetValue() );
        rFont.SetItalic     ( (FontItalic)aFontPostureItem.GetValue() );
        rFont.SetOutline    ( aFontContourItem.GetValue() );
        rFont.SetShadow     ( aFontShadowedItem.GetValue() );
        rFont.SetColor      ( aFontColorItem.GetValue() );
        rFont.SetSize       ( aSize );
        rFont.SetTransparent (TRUE);
    }
}

//------------------------------------------------------------------------

void AutoFmtPreview::DrawString( USHORT nIndex )
{
    if ( pCurData )
    {
        //------------------------
        // Ausgabe des Zelltextes:
        //------------------------

        String  cellString;
        BOOL    bNumFormat  = pCurData->GetIncludeValueFormat();
        ULONG   nNum;
        double  nVal;
        Color*  pDummy = NULL;

        switch( nIndex )
        {
            case  1: cellString = aStrJan;          break;
            case  2: cellString = aStrFeb;          break;
            case  3: cellString = aStrMar;          break;
            case  5: cellString = aStrNorth;        break;
            case 10: cellString = aStrMid;          break;
            case 15: cellString = aStrSouth;        break;
            case  4:
            case 20: cellString = aStrSum;          break;

            case  6:
            case  8:
            case 16:
            case 18: nVal = nIndex;
                     nNum = 5;
                     goto mknum;
            case 17:
            case  7: nVal = nIndex;
                     nNum = 6;
                     goto mknum;
            case 11:
            case 12:
            case 13: nVal = nIndex;
                     nNum = 12 == nIndex ? 10 : 9;
                     goto mknum;

            case  9: nVal = 21; nNum = 7; goto mknum;
            case 14: nVal = 36; nNum = 11; goto mknum;
            case 19: nVal = 51; nNum = 7; goto mknum;
            case 21: nVal = 33; nNum = 13; goto mknum;
            case 22: nVal = 36; nNum = 14; goto mknum;
            case 23: nVal = 39; nNum = 13; goto mknum;
            case 24: nVal = 108; nNum = 15;
            mknum:
                if( bNumFormat )
                {
                    ScNumFormatAbbrev aFmt;
                    pCurData->GetNumFormat( (USHORT) nNum, aFmt );
                    nNum = aFmt.GetFormatIndex( *pNumFmt );
                }
                else
                    nNum = 0;
                pNumFmt->GetOutputString( nVal, nNum, cellString, &pDummy );
                break;
        }

        if ( cellString.Len() > 0 )
        {
            Font                oldFont;
            Size                aStrSize;
            USHORT              nFmtIndex       = aFmtMap[nIndex];
            Rectangle           cellRect        = aCellArray[nIndex];
            Point               aPos            = aCellArray[nIndex].TopLeft();
            USHORT              nRightX         = 0;
            BOOL                bJustify        = pCurData->GetIncludeJustify();
            SvxHorJustifyItem    aHorJustifyItem;
            SvxCellHorJustify      eJustification;

            //-------------
            // Ausrichtung:
            //-------------
            if ( bJustify )
            {
                pCurData->GetHorJustify( nFmtIndex, aHorJustifyItem );
                eJustification = (SvxCellHorJustify)aHorJustifyItem.GetValue();
            }
            else
            {
                eJustification = SVX_HOR_JUSTIFY_STANDARD;
            }

            if ( pCurData->GetIncludeFont() )
            {
                Font aFont;
                Size theMaxStrSize;

                MakeFont( nFmtIndex, aFont );
                oldFont = aVD.GetFont();
                aVD.SetFont( aFont );
                theMaxStrSize           = cellRect.GetSize();
                theMaxStrSize.Width()  -= FRAME_OFFSET;
                theMaxStrSize.Height() -= FRAME_OFFSET;
                aStrSize.Width()  = aVD.GetTextWidth( cellString );
                aStrSize.Height() = aVD.GetTextHeight();

                if ( theMaxStrSize.Height() < aStrSize.Height() )
                {
                    // wenn der String in diesem Font nicht
                    // in die Zelle passt, wird wieder der
                    // Standard-Font genommen:
                    aVD.SetFont( oldFont );
                    aStrSize.Width()  = aVD.GetTextWidth( cellString );
                    aStrSize.Height() = aVD.GetTextHeight();
                }
                while ( ( theMaxStrSize.Width() <= aStrSize.Width() )
                    && ( cellString.Len() > 1 ) )
                {
                    if( eJustification == SVX_HOR_JUSTIFY_RIGHT )
                        cellString.Erase( 0, 1 );
                    else
                        cellString.Erase( cellString.Len() - 1 );
                    aStrSize.Width()  = aVD.GetTextWidth( cellString );
                    aStrSize.Height() = aVD.GetTextHeight();
                }
            }
            else
            {
                aStrSize.Width()  = aVD.GetTextWidth( cellString );
                aStrSize.Height() = aVD.GetTextHeight();
            }

            nRightX  = (USHORT)(  cellRect.GetWidth()
                                  - aStrSize.Width()
                                  - FRAME_OFFSET );

            //-----------------------------
            // vertikal (immer zentrieren):
            //-----------------------------
            aPos.Y() += (nRowHeight - (USHORT)aStrSize.Height()) / 2;

            //-----------
            // horizontal
            //-----------
            if ( eJustification != SVX_HOR_JUSTIFY_STANDARD )
            {
                USHORT nHorPos = (USHORT)
                                 ((cellRect.GetWidth()-aStrSize.Width())/2);

                switch ( eJustification )
                {
                    case SVX_HOR_JUSTIFY_LEFT:
                        aPos.X() += FRAME_OFFSET;
                        break;
                    case SVX_HOR_JUSTIFY_RIGHT:
                        aPos.X() += nRightX;
                        break;
                    case SVX_HOR_JUSTIFY_BLOCK:
                    case SVX_HOR_JUSTIFY_REPEAT:
                    case SVX_HOR_JUSTIFY_CENTER:
                        aPos.X() += nHorPos;
                        break;
                    case SVX_HOR_JUSTIFY_STANDARD:
                    default:
                        // Standard wird hier nicht behandelt
                        break;
                }
            }
            else
            {
                //---------------------
                // Standardausrichtung:
                //---------------------
                if ( ((nIndex%5) == 0) || (nIndex == 4) )
                {
                    // Text-Label links oder Summe linksbuendig
                    aPos.X() += FRAME_OFFSET;
                }
                else
                {
                     // Zahlen/Datum rechtsbuendig
                    aPos.X() += nRightX;
                }
            }

            //-------------------------------
            aVD.DrawText( aPos, cellString );
            //-------------------------------

            if ( pCurData->GetIncludeFont() )
                aVD.SetFont( oldFont );
        }
    }
}

#undef FRAME_OFFSET

//------------------------------------------------------------------------

void AutoFmtPreview::DrawBackground( USHORT nIndex )
{
    if ( pCurData )
    {
        USHORT          nFmtIndex = aFmtMap[nIndex];
        Rectangle       cellRect  = aCellArray[nIndex];

        SvxBrushItem    aBrushItem;
        pCurData->GetBackground( nFmtIndex, aBrushItem );

        BOOL bHadLine  = aVD.IsLineColor();
        Color aOldLine = aVD.GetLineColor();
        BOOL bHadFill  = aVD.IsFillColor();
        Color aOldFill = aVD.GetFillColor();

        aVD.SetFillColor( aBrushItem.GetColor() );
        aVD.SetLineColor();
        //-----------------------
        aVD.DrawRect( cellRect );
        //-----------------------
        if (bHadFill) aVD.SetFillColor(aOldFill); else aVD.SetFillColor();
        if (bHadLine) aVD.SetLineColor(aOldLine); else aVD.SetLineColor();
    }
}

//------------------------------------------------------------------------

void AutoFmtPreview::PaintCells()
{
    if ( pCurData )
    {
        USHORT i = 0;

        //---------------
        // 1. Hintergrund
        //---------------
        if ( pCurData->GetIncludeBackground() )
        {
            for ( i=0; i<=24; i++ )
            {
                DrawBackground( i );
            }
        }

        //----------
        // 2. Rahmen
        //----------
        if ( pCurData->GetIncludeFrame() )
        {
            for ( i=0; i<=24; i++ )
            {
                DrawFrame( i );
            }
        }

        //---------
        // 3. Werte
        //---------
        for ( i = 0; i<=24; i++ )
        {
            DrawString( i );
        }
    }
}

//------------------------------------------------------------------------

void __EXPORT AutoFmtPreview::Init()
{
    SvxBoxItem aEmptyBoxItem;
    aEmptyBoxItem.SetLine( NULL, BOX_LINE_TOP );
    aEmptyBoxItem.SetLine( NULL, BOX_LINE_BOTTOM );
    aEmptyBoxItem.SetLine( NULL, BOX_LINE_LEFT );
    aEmptyBoxItem.SetLine( NULL, BOX_LINE_RIGHT );
    aEmptyBoxItem.SetDistance( 0 );

    //------------------------
    // Linienattribut-Feld mit
    // Null-BoxItems fuellen
    //------------------------
    for ( USHORT i=0; i<=48; i++ )
    {
        aLinePtrArray[i] = new SvxBoxItem( aEmptyBoxItem );
    }

    CalcCellArray( FALSE );
    CalcLineMap();
}

//------------------------------------------------------------------------

void AutoFmtPreview::CalcCellArray( BOOL bFitWidth )
{
    // Initialisieren des Zellfeldes (5x5 Zellen).
    // Rectangles enthalten Position und Groesse einer Zelle.
    // abhaengig, ob Zellbreite an Format angepasst werden soll (bFitWidth)
    USHORT  nRow;
    USHORT  nDataColWidth = (bFitWidth) ? nDataColWidth2 : nDataColWidth1;

    for ( nRow=0; nRow<=4; nRow++ )
    {
        Point   topLeftPos ( 2, (nRowHeight * nRow) + 2 );
        Size    rectSize   ( 0, nRowHeight );
        USHORT  nCell;
        USHORT  nFirst = nRow*5;
        USHORT  nLast  = nFirst+4;

        for ( nCell=nFirst; nCell<=nLast; nCell++ )
        {
            //--------------------
            // Position berechnen:
            //--------------------
            if ( nCell == 0 )
            {
                // erste Zelle benoetigt keine Sonderbehandlung
            }
            else if ( (nCell <= 4) && (nCell != 0) )
            {
                // Zelle aus der ersten Zeile
                topLeftPos = aCellArray[nCell-1].TopRight();
            }
            else
            {
                topLeftPos = aCellArray[nCell-5].BottomLeft();
            }

            //-------------------
            // Groesse berechnen:
            //-------------------
            if ( (nCell == nFirst) || (nCell == nLast) )
                rectSize.Width() = nLabelColWidth;
            else
                rectSize.Width() = nDataColWidth;

            aCellArray[nCell] = Rectangle( topLeftPos, rectSize );
        }
    }
    aPrvSize.Width()  = aCellArray[24].BottomRight().X() + 3;
    aPrvSize.Height() = aCellArray[24].BottomRight().Y() + 3;
}

//------------------------------------------------------------------------

void AutoFmtPreview::CalcLineMap()
{
    //----------------------------------------------------
    // Initialisieren des Linenattributfeldes (7x7 Zellen)
    // Ein Eintrag dieses Feldes enthaelt einen Verweis
    // auf die Linienattribute des zugehoerigen Feldes.
    // Dieses Feld "umschliesst" das Zellfeld mit einer
    // Reihe "leerer" Zellen.
    //----------------------------------------------------

    if ( pCurData )
    {
        SvxBoxItem  aFrameItem;
        USHORT      nRow;

        for ( nRow=0; nRow<=4; nRow++ )
        {
            USHORT  nLine;
            USHORT  nCell;
            USHORT  nFirst = (nRow*5);
            USHORT  nLast  = nFirst+4;

            for ( nCell=nFirst; nCell<=nLast; nCell++ )
            {
                nLine = nCell + 8 + ((nCell/5)*2);
                pCurData->GetBox( aFmtMap[nCell], aFrameItem );
                *(aLinePtrArray[nLine]) = aFrameItem;
            }
        }
    }
}

//------------------------------------------------------------------------

void AutoFmtPreview::NotifyChange( ScAutoFormatData* pNewData )
{
    if ( pNewData != pCurData )
    {
        pCurData  = pNewData;
        bFitWidth = pNewData->GetIncludeWidthHeight();
        CalcCellArray( bFitWidth );
        CalcLineMap();
    }
    else if ( bFitWidth != pNewData->GetIncludeWidthHeight() )
    {
        bFitWidth = !bFitWidth;
        CalcCellArray( bFitWidth );
    }

    DoPaint( Rectangle( Point(0,0), GetSizePixel() ) );
}

//------------------------------------------------------------------------

void AutoFmtPreview::DoPaint( const Rectangle& rRect )
{
    Bitmap  thePreview;
    Point   aCenterPos;
    Size    theWndSize = GetSizePixel();
    Size    thePrevSize;
    Font    aFont;

    aFont = aVD.GetFont();
    aFont.SetTransparent( TRUE );

    aVD.SetFont( aFont );
    aVD.SetLineColor();
    aVD.SetFillColor( Color( COL_WHITE ) );
    aVD.SetOutputSizePixel( aPrvSize );

    //--------------------------------
    // Zellen auf virtual Device malen
    // und Ergebnis sichern
    //--------------------------------
    PaintCells();
    thePreview = aVD.GetBitmap( Point(0,0), aPrvSize );

    //--------------------------------------
    // Rahmen malen und Vorschau zentrieren:
    // (virtual Device fuer Fensterausgabe)
    //--------------------------------------
    aVD.SetOutputSizePixel( theWndSize );
    BOOL bHadLine  = aVD.IsLineColor();
    Color aOldLine = aVD.GetLineColor();
    aVD.SetLineColor( Color( COL_BLACK ) );
    aVD.DrawRect( Rectangle( Point(0,0), theWndSize ) );
    if (bHadLine) aVD.SetLineColor(aOldLine); else aVD.SetLineColor();
    aCenterPos  = Point( (theWndSize.Width()  - aPrvSize.Width() ) / 2,
                         (theWndSize.Height() - aPrvSize.Height()) / 2 );
    aVD.DrawBitmap( aCenterPos, thePreview );

    //----------------------------
    // Ausgabe im Vorschaufenster:
    //----------------------------
    DrawBitmap( Point(0,0), aVD.GetBitmap( Point(0,0), theWndSize ) );
}

//------------------------------------------------------------------------

void __EXPORT AutoFmtPreview::Paint( const Rectangle& rRect )
{
    DoPaint( rRect );
}



