/*************************************************************************
 *
 *  $RCSfile: consdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:54 $
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

// System - Includes ---------------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef PCH
#include <segmentc.hxx>
#endif

#include <sfx2/dispatch.hxx>


// INCLUDE -------------------------------------------------------------------

#include "tabvwsh.hxx"
#include "uiitems.hxx"
#include "dbcolect.hxx"
#include "rangenam.hxx"
#include "rangeutl.hxx"
#include "reffact.hxx"
#include "document.hxx"
//#include "tabvwsh.hxx"
//#include "viewdata.hxx"
#include "scresid.hxx"

#include "globstr.hrc"
#include "consdlg.hrc"

#define _CONSDLG_CXX
#include "consdlg.hxx"
#undef _CONSDLG_CXX

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif


#define INFOBOX(id) InfoBox(this, ScGlobal::GetRscString(id)).Execute()

SEG_EOFGLOBALS()


//============================================================================
//  class ScAreaData

class ScAreaData
{
public:
    ScAreaData()  {}
    ~ScAreaData() {}

    void Set( const String& rName, const String& rArea, BOOL bDb )
                {
                    aStrName  = rName;
                    aStrArea  = rArea;
                    bIsDbArea = bDb;
                }

    String  aStrName;
    String  aStrArea;
    BOOL    bIsDbArea;
};


//============================================================================
//  class ScConsolidateDialog


ScConsolidateDlg::ScConsolidateDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                    const SfxItemSet&   rArgSet )

    :   ScAnyRefDlg ( pB, pCW, pParent, RID_SCDLG_CONSOLIDATE ),
        //
        aFtFunc         ( this, ScResId( FT_FUNC ) ),
        aLbFunc         ( this, ScResId( LB_FUNC ) ),

        aFtConsAreas    ( this, ScResId( FT_CONSAREAS ) ),
        aLbConsAreas    ( this, ScResId( LB_CONSAREAS ) ),

        aFtDataArea     ( this, ScResId( FT_DATA_AREA ) ),
        aLbDataArea     ( this, ScResId( LB_DATA_AREA ) ),
        aEdDataArea     ( this, ScResId( ED_DATA_AREA ) ),
        aRbDataArea     ( this, ScResId( RB_DATA_AREA ), &aEdDataArea ),

        aFtDestArea     ( this, ScResId( FT_DEST_AREA ) ),
        aLbDestArea     ( this, ScResId( LB_DEST_AREA ) ),
        aEdDestArea     ( this, ScResId( ED_DEST_AREA ) ),
        aRbDestArea     ( this, ScResId( RB_DEST_AREA ), &aEdDestArea ),

        aBtnByRow       ( this, ScResId( BTN_BYROW ) ),
        aBtnByCol       ( this, ScResId( BTN_BYCOL) ),

        aGbConsBy       ( this, ScResId( GB_CONSBY ) ),
        aBtnRefs        ( this, ScResId( BTN_REFS ) ),

        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        aBtnMore        ( this, ScResId( BTN_MORE ) ),
        aBtnAdd         ( this, ScResId( BTN_ADD ) ),
        aBtnRemove      ( this, ScResId( BTN_REMOVE ) ),

        aStrUndefined   ( ScResId( SCSTR_UNDEFINED ) ),
        //
        nWhichCons      ( rArgSet.GetPool()->GetWhich( SID_CONSOLIDATE ) ),
        theConsData     ( ((const ScConsolidateItem&)
                           rArgSet.Get( rArgSet.GetPool()->
                                            GetWhich( SID_CONSOLIDATE ) )
                                      ).GetData() ),

        pViewData       ( ((ScTabViewShell*)SfxViewShell::Current())->
                                GetViewData() ),
        pDoc            ( ((ScTabViewShell*)SfxViewShell::Current())->
                                GetViewData()->GetDocument() ),
        pRangeUtil      ( new ScRangeUtil ),
        pAreaData       ( NULL ),
        nAreaDataCount  ( 0 ),
        pRefInputEdit   ( &aEdDataArea )
{
    Init();
    FreeResource();
}


//----------------------------------------------------------------------------

__EXPORT ScConsolidateDlg::~ScConsolidateDlg()
{
    delete [] pAreaData;
    delete pRangeUtil;
}


//----------------------------------------------------------------------------

void ScConsolidateDlg::Init()
{
    DBG_ASSERT( pViewData && pDoc && pRangeUtil, "Error in Ctor" );

    ScArea aArea;
    String aStr;
    USHORT i=0;

    aEdDataArea .SetGetFocusHdl( LINK( this, ScConsolidateDlg, GetFocusHdl ) );
    aEdDestArea .SetGetFocusHdl( LINK( this, ScConsolidateDlg, GetFocusHdl ) );
    aLbDataArea .SetGetFocusHdl( LINK( this, ScConsolidateDlg, GetFocusHdl ) );
    aLbDestArea .SetGetFocusHdl( LINK( this, ScConsolidateDlg, GetFocusHdl ) );
    aEdDataArea .SetModifyHdl   ( LINK( this, ScConsolidateDlg, ModifyHdl ) );
    aEdDestArea .SetModifyHdl   ( LINK( this, ScConsolidateDlg, ModifyHdl ) );
    aLbConsAreas.SetSelectHdl   ( LINK( this, ScConsolidateDlg, SelectHdl ) );
    aLbDataArea .SetSelectHdl   ( LINK( this, ScConsolidateDlg, SelectHdl ) );
    aLbDestArea .SetSelectHdl   ( LINK( this, ScConsolidateDlg, SelectHdl ) );
    aBtnOk      .SetClickHdl    ( LINK( this, ScConsolidateDlg, OkHdl ) );
    aBtnCancel  .SetClickHdl    ( LINK( this, ScConsolidateDlg, ClickHdl ) );
    aBtnAdd     .SetClickHdl    ( LINK( this, ScConsolidateDlg, ClickHdl ) );
    aBtnRemove  .SetClickHdl    ( LINK( this, ScConsolidateDlg, ClickHdl ) );

    aBtnMore.AddWindow( &aGbConsBy );
    aBtnMore.AddWindow( &aBtnByRow );
    aBtnMore.AddWindow( &aBtnByCol );
    aBtnMore.AddWindow( &aBtnRefs );

    aBtnAdd.Disable();
    aBtnRemove.Disable();

    aBtnByRow.Check( theConsData.bByRow );
    aBtnByCol.Check( theConsData.bByCol );
    aBtnRefs .Check( theConsData.bReferenceData );

    aLbFunc.SelectEntryPos( FuncToLbPos( theConsData.eFunction ) );

    // Einlesen der Konsolidierungsbereiche
    aLbConsAreas.Clear();
    for ( i=0; i<theConsData.nDataAreaCount; i++ )
    {
        aArea = *(theConsData.ppDataAreas[i] );
        if ( aArea.nTab < pDoc->GetTableCount() )
        {
            pRangeUtil->MakeAreaString( aArea, aStr, pDoc );
            aLbConsAreas.InsertEntry( aStr );
        }
    }

    if ( theConsData.nTab < pDoc->GetTableCount() )
        aEdDestArea.SetText( ScRefTripel( theConsData.nCol,
                                          theConsData.nRow,
                                          theConsData.nTab,
                                          FALSE, FALSE, FALSE ).
                                                GetRefString( pDoc, MAXTAB+1 ) );
    else
        aEdDestArea.SetText( EMPTY_STRING );

    //----------------------------------------------------------

    /*
     * Aus den RangeNames und Datenbankbereichen werden sich
     * in der Hilfsklasse ScAreaData die Bereichsnamen gemerkt,
     * die in den ListBoxen erscheinen.
     */

    ScRangeName*    pRangeNames  = pDoc->GetRangeName();
    ScDBCollection* pDbNames     = pDoc->GetDBCollection();
    const USHORT    nRangeCount  = pRangeNames ? pRangeNames->GetCount() : 0;
    const USHORT    nDbCount     = pDbNames    ? pDbNames   ->GetCount() : 0;

    nAreaDataCount = nRangeCount+nDbCount;
    pAreaData      = NULL;

    if ( nAreaDataCount > 0 )
    {
        pAreaData = new ScAreaData[nAreaDataCount];

        String aStrName;
        String aStrArea;
        USHORT nAt = 0;
        ScRange aRange;
        ScAreaNameIterator aIter( pDoc );
        while ( aIter.Next( aStrName, aRange ) )
        {
            aRange.Format( aStrArea, SCA_ABS_3D, pDoc );
            pAreaData[nAt++].Set( aStrName, aStrArea, aIter.WasDBName() );
        }
    }

    FillAreaLists();
    ModifyHdl( &aEdDestArea );
    aLbDataArea.SelectEntryPos( 0 );
    aEdDataArea.SetText( EMPTY_STRING );
    aEdDataArea.GrabFocus();

    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
    //SFX_APPWINDOW->Enable();
}


//----------------------------------------------------------------------------
void ScConsolidateDlg::FillAreaLists()
{
    aLbDataArea.Clear();
    aLbDestArea.Clear();
    aLbDataArea.InsertEntry( aStrUndefined );
    aLbDestArea.InsertEntry( aStrUndefined );

    if ( pRangeUtil && pAreaData && (nAreaDataCount > 0) )
    {
        String aString;

        for ( USHORT i=0;
              (i<nAreaDataCount) && (pAreaData[i].aStrName.Len()>0);
              i++ )
        {
            aLbDataArea.InsertEntry( pAreaData[i].aStrName, i+1 );

//          if ( !pAreaData[i].bIsDbArea )
                aLbDestArea.InsertEntry( pAreaData[i].aStrName, i+1 );
        }
    }
}


//----------------------------------------------------------------------------
// Uebergabe eines mit der Maus selektierten Tabellenbereiches, der dann als
//  neue Selektion im Referenz-Fenster angezeigt wird.


void ScConsolidateDlg::SetReference( const ScRange& rRef, ScDocument* pDoc )
{
    if ( pRefInputEdit )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( pRefInputEdit );

        String      aStr;
        USHORT      nFmt = SCR_ABS_3D;       //!!! nCurTab fehlt noch

        if ( rRef.aStart.Tab() != rRef.aEnd.Tab() )
            nFmt |= SCA_TAB2_3D;

        if ( pRefInputEdit == &aEdDataArea)
            rRef.Format( aStr, nFmt, pDoc );
        else if ( pRefInputEdit == &aEdDestArea )
            rRef.aStart.Format( aStr, nFmt, pDoc );

        pRefInputEdit->SetRefString( aStr );
    }

    ModifyHdl( pRefInputEdit );
}


//----------------------------------------------------------------------------

BOOL __EXPORT ScConsolidateDlg::Close()
{
    return DoClose( ScConsolidateDlgWrapper::GetChildWindowId() );
}


//----------------------------------------------------------------------------

void ScConsolidateDlg::SetActive()
{
    if ( bDlgLostFocus )
    {
        bDlgLostFocus = FALSE;

        if ( pRefInputEdit )
        {
            pRefInputEdit->GrabFocus();
            ModifyHdl( pRefInputEdit );
        }
    }
    else
        GrabFocus();

    RefInputDone();
}


//----------------------------------------------------------------------------

void __EXPORT ScConsolidateDlg::Deactivate()
{
    bDlgLostFocus = TRUE;
}


//----------------------------------------------------------------------------

BOOL ScConsolidateDlg::VerifyEdit( ScRefEdit* pEd )
{
    if ( !pRangeUtil || !pDoc || !pViewData ||
         ((pEd != &aEdDataArea) && (pEd != &aEdDestArea)) )
        return FALSE;

    USHORT  nTab    = pViewData->GetTabNo();
    BOOL    bEditOk = FALSE;
    String  theCompleteStr;

    if ( pEd == &aEdDataArea )
    {
        bEditOk = pRangeUtil->IsAbsArea( pEd->GetText(), pDoc,
                                         nTab, &theCompleteStr );
    }
    else if ( pEd == &aEdDestArea )
    {
        String aPosStr;

        pRangeUtil->CutPosString( pEd->GetText(), aPosStr );
        bEditOk = pRangeUtil->IsAbsPos( aPosStr, pDoc,
                                        nTab, &theCompleteStr );
    }

    if ( bEditOk )
        pEd->SetText( theCompleteStr );

    return bEditOk;
}


//----------------------------------------------------------------------------
// Handler:
// ========

IMPL_LINK( ScConsolidateDlg, GetFocusHdl, Control*, pCtr )
{
    if ( pCtr ==(Control*)&aEdDataArea ||
         pCtr ==(Control*)&aEdDestArea)
    {
        pRefInputEdit = (ScRefEdit*)pCtr;
    }
    else if(pCtr ==(Control*)&aLbDataArea )
    {
        pRefInputEdit = &aEdDataArea;
    }
    else if(pCtr ==(Control*)&aLbDestArea )
    {
        pRefInputEdit = &aEdDestArea;
    }
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScConsolidateDlg, OkHdl, void*, p )
{
    USHORT nDataAreaCount = aLbConsAreas.GetEntryCount();

    if ( nDataAreaCount > 0 )
    {
        ScRefTripel aDestTripel;
        USHORT      nTab = pViewData->GetTabNo();
        String      aDestPosStr( aEdDestArea.GetText() );

        if ( pRangeUtil->IsAbsPos( aDestPosStr, pDoc, nTab, NULL, &aDestTripel ) )
        {
            ScConsolidateParam  theOutParam( theConsData );
            ScArea**            ppDataAreas = new ScArea*[nDataAreaCount];
            ScArea*             pArea;
            USHORT              i=0;

            for ( i=0; i<nDataAreaCount; i++ )
            {
                pArea = new ScArea;
                pRangeUtil->MakeArea( aLbConsAreas.GetEntry( i ),
                                      *pArea, pDoc, nTab );
                ppDataAreas[i] = pArea;
            }

            theOutParam.nCol            = aDestTripel.GetCol();
            theOutParam.nRow            = aDestTripel.GetRow();
            theOutParam.nTab            = aDestTripel.GetTab();
            theOutParam.eFunction       = LbPosToFunc( aLbFunc.GetSelectEntryPos() );
            theOutParam.bByCol          = aBtnByCol.IsChecked();
            theOutParam.bByRow          = aBtnByRow.IsChecked();
            theOutParam.bReferenceData  = aBtnRefs.IsChecked();
            theOutParam.SetAreas( ppDataAreas, nDataAreaCount );

            for ( i=0; i<nDataAreaCount; i++ )
                delete ppDataAreas[i];
            delete [] ppDataAreas;

            ScConsolidateItem aOutItem( nWhichCons, &theOutParam );

            SFX_APP()->LockDispatcher( FALSE );
            SwitchToDocument();
            SFX_DISPATCHER().Execute( SID_CONSOLIDATE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                                      &aOutItem, 0L, 0L );
            Close();
        }
        else
        {
            INFOBOX( STR_INVALID_TABREF );
            aEdDestArea.GrabFocus();
        }
    }
    else
        Close(); // keine Datenbereiche definiert -> Cancel
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScConsolidateDlg, ClickHdl, PushButton*, pBtn )
{
    if ( pBtn == &aBtnCancel )
        Close();
    else if ( pBtn == &aBtnAdd )
    {
        if ( aEdDataArea.GetText().Len() > 0 )
        {
            String      aNewEntry( aEdDataArea.GetText() );
            ScArea**    ppAreas = NULL;
            USHORT      nAreaCount = 0;

            if ( pRangeUtil->IsAbsTabArea( aNewEntry, pDoc, &ppAreas, &nAreaCount ) )
            {
                // IsAbsTabArea() legt ein Array von ScArea-Zeigern an,
                // welche ebenfalls dynamisch erzeugt wurden.
                // Diese Objekte muessen hier abgeraeumt werden.

                for ( USHORT i=0; i<nAreaCount; i++ )
                {
                    String aNewArea;

                    if ( ppAreas[i] )
                    {
                        pRangeUtil->MakeAreaString( *(ppAreas[i]),
                                                    aNewArea,
                                                    pDoc );

                        if ( aLbConsAreas.GetEntryPos( aNewArea )
                             == LISTBOX_ENTRY_NOTFOUND )
                        {
                            aLbConsAreas.InsertEntry( aNewArea );
                        }
                        delete ppAreas[i];
                    }
                }
                delete [] ppAreas;
            }
            else if ( VerifyEdit( &aEdDataArea ) )
            {
                String aNewArea( aEdDataArea.GetText() );

                if ( aLbConsAreas.GetEntryPos( aNewArea ) == LISTBOX_ENTRY_NOTFOUND )
                    aLbConsAreas.InsertEntry( aNewArea );
                else
                    INFOBOX( STR_AREA_ALREADY_INSERTED );
            }
            else
            {
                INFOBOX( STR_INVALID_TABREF );
                aEdDataArea.GrabFocus();
            }
        }
    }
    else if ( pBtn == &aBtnRemove )
    {
        while ( aLbConsAreas.GetSelectEntryCount() )
            aLbConsAreas.RemoveEntry( aLbConsAreas.GetSelectEntryPos() );
        aBtnRemove.Disable();
    }
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScConsolidateDlg, SelectHdl, ListBox*, pLb )
{
    if ( pLb == &aLbConsAreas )
    {
        if ( aLbConsAreas.GetSelectEntryCount() > 0 )
            aBtnRemove.Enable();
        else
            aBtnRemove.Disable();
    }
    else if ( (pLb == &aLbDataArea) || (pLb == &aLbDestArea) )
    {
        Edit*   pEd = (pLb == &aLbDataArea) ? &aEdDataArea : &aEdDestArea;
        USHORT  nSelPos = pLb->GetSelectEntryPos();

        if (    pRangeUtil
            && (nSelPos > 0)
            && (nAreaDataCount > 0)
            && (pAreaData != NULL) )
        {
            if ( nSelPos <= nAreaDataCount )
            {
                String aString( pAreaData[nSelPos-1].aStrArea );

                if ( pLb == &aLbDestArea )
                    pRangeUtil->CutPosString( aString, aString );

                pEd->SetText( aString );

                if ( pEd == &aEdDataArea )
                    aBtnAdd.Enable();
            }
        }
        else
        {
            pEd->SetText( EMPTY_STRING );
            if ( pEd == &aEdDataArea )
                aBtnAdd.Enable();
        }
    }
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScConsolidateDlg, ModifyHdl, ScRefEdit*, pEd )
{
    if ( pEd == &aEdDataArea )
    {
        String aAreaStr( pEd->GetText() );
        if ( aAreaStr.Len() > 0 )
        {
            aBtnAdd.Enable();
        }
        else
            aBtnAdd.Disable();
    }
    else if ( pEd == &aEdDestArea )
    {
        aLbDestArea.SelectEntryPos(0);
    }
    return 0;
}


//----------------------------------------------------------------------------
// Verallgemeinern!!! :
// Resource der ListBox und diese beiden Umrechnungsmethoden gibt es
// auch noch in tpsubt bzw. ueberall, wo StarCalc-Funktionen
// auswaehlbar sind.

ScSubTotalFunc ScConsolidateDlg::LbPosToFunc( USHORT nPos )
{
    switch ( nPos )
    {
        case  2:    return SUBTOTAL_FUNC_AVE;
        case  6:    return SUBTOTAL_FUNC_CNT;
        case  1:    return SUBTOTAL_FUNC_CNT2;
        case  3:    return SUBTOTAL_FUNC_MAX;
        case  4:    return SUBTOTAL_FUNC_MIN;
        case  5:    return SUBTOTAL_FUNC_PROD;
        case  7:    return SUBTOTAL_FUNC_STD;
        case  8:    return SUBTOTAL_FUNC_STDP;
        case  9:    return SUBTOTAL_FUNC_VAR;
        case 10:    return SUBTOTAL_FUNC_VARP;
        case  0:
        default:
            return SUBTOTAL_FUNC_SUM;
    }
}


//----------------------------------------------------------------------------

USHORT ScConsolidateDlg::FuncToLbPos( ScSubTotalFunc eFunc )
{
    switch ( eFunc )
    {
        case SUBTOTAL_FUNC_AVE:     return 2;
        case SUBTOTAL_FUNC_CNT:     return 6;
        case SUBTOTAL_FUNC_CNT2:    return 1;
        case SUBTOTAL_FUNC_MAX:     return 3;
        case SUBTOTAL_FUNC_MIN:     return 4;
        case SUBTOTAL_FUNC_PROD:    return 5;
        case SUBTOTAL_FUNC_STD:     return 7;
        case SUBTOTAL_FUNC_STDP:    return 8;
        case SUBTOTAL_FUNC_VAR:     return 9;
        case SUBTOTAL_FUNC_VARP:    return 10;
        case SUBTOTAL_FUNC_NONE:
        case SUBTOTAL_FUNC_SUM:
        default:
            return 0;
    }
}




