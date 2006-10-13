/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tablemgr.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-13 11:12:50 $
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



#include <sot/storage.hxx>
#ifndef _SCH_DLL_HXX
#include <sch/schdll.hxx>
#endif
#ifndef _SCH_MEMCHRT_HXX
#include <sch/memchrt.hxx>
#endif

#include <sot/clsids.hxx>

#include "errhdl.hxx"
#include "wrtsh.hxx"
#include "cmdid.h"
#include "frmatr.hxx"
#include "view.hxx"
#include "basesh.hxx"
#include "swundo.hxx"
#include "tablemgr.hxx"
#include "frmfmt.hxx"
#include "instable.hxx"
//CHINA001 #include "colwd.hxx"
#include "swerror.h"
#include "table.hrc"
#include "swabstdlg.hxx" //CHINA001


/*------------------------------------------------------------------------
 Beschreibung:  Zeilenhoehe einstellen (Dialog)
------------------------------------------------------------------------*/


void SwTableFUNC::ColWidthDlg( Window *pParent )
{
    InitTabCols();
    //CHINA001 SwTableWidthDlg *pDlg = new SwTableWidthDlg( pParent, *this );
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();//CHINA001
    DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");//CHINA001

    VclAbstractDialog* pDlg = pFact->CreateSwTableWidthDlg( pParent, *this ,ResId( DLG_COL_WIDTH ));
    DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
    pDlg->Execute();
    delete pDlg;
}

/*--------------------------------------------------------------------
    Beschreibung: Breite ermitteln
 --------------------------------------------------------------------*/


SwTwips SwTableFUNC::GetColWidth(USHORT nNum) const
{
    SwTwips nWidth = 0;

    if( aCols.Count() > 0 )
    {
        if(aCols.Count() == GetColCount())
        {
            nWidth = (SwTwips)((nNum == aCols.Count()) ?
                    aCols.GetRight() - aCols[nNum-1] :
                    nNum == 0 ? aCols[nNum] - aCols.GetLeft() :
                                aCols[nNum] - aCols[nNum-1]);
        }
        else
        {
            SwTwips nRValid = nNum < GetColCount() ?
                            aCols[(USHORT)GetRightSeparator((int)nNum)]:
                                    aCols.GetRight();
            SwTwips nLValid = nNum ?
                            aCols[(USHORT)GetRightSeparator((int)nNum - 1)]:
                                    aCols.GetLeft();
            nWidth = nRValid - nLValid;
        }
    }
    else
        nWidth = aCols.GetRight();

    return nWidth;
}



SwTwips SwTableFUNC::GetMaxColWidth( USHORT nNum ) const
{
    ASSERT(nNum <= aCols.Count(), "Index out of Area");

    if ( GetColCount() > 0 )
    {
        // Die max. Breite ergibt sich aus der eigenen Breite und
        // der Breite der Nachbarzellen um je MINLAY verringert
        SwTwips nMax =  nNum == 0 ?
            GetColWidth(1) - MINLAY :
                nNum == GetColCount() ?
                    GetColWidth( nNum-1 ) - MINLAY :
                        GetColWidth(nNum - 1) + GetColWidth( nNum + 1 ) - 2 * MINLAY;

        return nMax + GetColWidth(nNum) ;
    }
    else
        return GetColWidth(nNum);
}



void SwTableFUNC::SetColWidth(USHORT nNum, SwTwips nNewWidth )
{
    // aktuelle Breite setzen
    // alle folgenden Verschieben
    BOOL bCurrentOnly = FALSE;
    SwTwips nWidth = 0;

    if ( aCols.Count() > 0 )
    {
        if(aCols.Count() != GetColCount())
            bCurrentOnly = TRUE;
        nWidth = GetColWidth(nNum);

        int nDiff = (int)(nNewWidth - nWidth);
        if( !nNum )
            aCols[GetRightSeparator(0)] += nDiff;
        else if( nNum < GetColCount()  )
        {
            if(nDiff < GetColWidth(nNum + 1) - MINLAY)
                aCols[GetRightSeparator(nNum)] += nDiff;
            else
            {
                int nDiffLeft = nDiff - (int)GetColWidth(nNum + 1) + (int)MINLAY;
                aCols[GetRightSeparator(nNum)] += (nDiff - nDiffLeft);
                aCols[GetRightSeparator(nNum - 1)] -= nDiffLeft;
            }
        }
        else
            aCols[GetRightSeparator(nNum-1)] -= nDiff;
    }
    else
        aCols.SetRight( Min( nNewWidth, aCols.GetRightMax()) );

    pSh->StartAllAction();
    pSh->SetTabCols( aCols, bCurrentOnly );
    pSh->EndAllAction();
}



void SwTableFUNC::InitTabCols()
{
    ASSERT(pSh, keine Shell);

    if( pFmt && pSh)
        pSh->GetTabCols( aCols );
}



SwTableFUNC::SwTableFUNC(SwWrtShell *pShell, BOOL bCopyFmt)
    : pFmt(pShell->GetTableFmt()),
      pSh(pShell),
      bCopy(bCopyFmt)
{
        // gfs. das Format fuer die Bearbeitung kopieren
    if( pFmt && bCopy )
        pFmt = new SwFrmFmt( *pFmt );
}



SwTableFUNC::~SwTableFUNC()
{
    if(bCopy)
        delete pFmt;
}

//Sonst GPF mit W95,MSVC40, non product
#pragma optimize("",off)



void SwTableFUNC::InsertChart( SchMemChart& rData, const SfxItemSet *pSet )
{
    pSh->StartAllAction();
    pSh->StartUndo( UIUNDO_INSERT_CHART );

    String aName( pSh->GetTableFmt()->GetName() );

    //Vor die Tabelle gehen und einen Node einfuegen.
    pSh->MoveTable( fnTableCurr, fnTableStart );
    pSh->Up();
    if ( pSh->IsCrsrInTbl() )
    {
        if ( aName != pSh->GetTableFmt()->GetName() )
            pSh->Down();    //Zwei Tabellen direkt uebereinander.
    }
    pSh->SplitNode();

    //Jetzt das CharObject einfuegen.
    //Wer das nicht versteht ist selber schuld ;-)
    ::rtl::OUString aObjName;
    comphelper::EmbeddedObjectContainer aCnt;
    com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > xObj =
            aCnt.CreateEmbeddedObject( SvGlobalName( SO3_SCH_CLASSID ).GetByteSequence(), aObjName );
    if ( xObj.is() )
    {
        pSh->InsertOleObject( ::svt::EmbeddedObjectRef( xObj, ::com::sun::star::embed::Aspects::MSOLE_CONTENT ) );

        //Den Namen der Table am OleNode setzen
        pSh->SetChartName( aName );

        //Und die Daten in's Objekt uebertragen.
        // TODO/LATER: Looks like there is no need here to update the replacement. But it should be checked.
        if( pSet )
            SchDLL::Update( xObj, &rData, *pSet );
        else
            SchDLL::Update( xObj, &rData );
    }
    pSh->EndUndo( UIUNDO_INSERT_CHART );
    pSh->EndAllAction();
}

#pragma optimize("",on)



void SwTableFUNC::UpdateChart()
{
    //Update der Felder in der Tabelle vom User ausgeloesst, alle
    //Charts zu der Tabelle werden auf den neuesten Stand gebracht.
    SwFrmFmt *pFmt = pSh->GetTableFmt();
    if ( pFmt && pSh->HasOLEObj( pFmt->GetName() ) )
    {
        pSh->StartAllAction();
        pSh->UpdateCharts( pFmt->GetName() );
        pSh->EndAllAction();
    }
}



USHORT  SwTableFUNC::GetCurColNum() const
{
    int nPos = pSh->GetCurTabColNum();
    int nCount = 0;
    for(int i = 0; i < nPos; i++ )
        if(aCols.IsHidden(i))
            nCount ++;
    return nPos - nCount;
}




USHORT  SwTableFUNC::GetColCount() const
{
    USHORT nCount = 0;
    for(int i=0; i < (int)aCols.Count(); i++ )
        if(aCols.IsHidden(i))
            nCount ++;
    return aCols.Count() - nCount;
}



int SwTableFUNC::GetRightSeparator(int nNum) const
{
    DBG_ASSERT( nNum < (int)GetColCount() ,"Index out of range")
    int i = 0;
    while( nNum >= 0 )
    {
        if( !aCols.IsHidden(i) )
            nNum--;
        i++;
    }
    return i - 1;
}



