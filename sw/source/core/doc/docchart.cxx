/*************************************************************************
 *
 *  $RCSfile: docchart.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:15 $
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

#include <float.h>

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _IPOBJ_HXX //autogen
#include <so3/ipobj.hxx>
#endif
#ifndef _SCH_DLL_HXX
#include <sch/schdll.hxx>
#endif
#ifndef _SCH_MEMCHRT_HXX
#include <sch/memchrt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _NDINDEX_HXX //autogen
#include <ndindex.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _NDTXT_HXX //autogen
#include <ndtxt.hxx>
#endif
#ifndef _CALC_HXX
#include <calc.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _CELLFML_HXX //autogen
#include <cellfml.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _NDOLE_HXX //autogen
#include <ndole.hxx>
#endif
#ifndef _CALBCK_HXX //autogen
#include <calbck.hxx>
#endif
#ifndef _CNTFRM_HXX //autogen
#include <cntfrm.hxx>
#endif
#ifndef _SWTBLFMT_HXX //autogen
#include <swtblfmt.hxx>
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>
#endif
#ifndef _CELLATR_HXX //autogen wg. SwTblBoxNumFormat
#include <cellatr.hxx>
#endif


SchMemChart *SwTable::UpdateData( SchMemChart* pData,
                                const String* pSelection ) const
{
    SwCalc aCalc( *GetFrmFmt()->GetDoc() );
    SwTblCalcPara aCalcPara( aCalc, *this );
    String aSelection;

    // worauf bezieht sich das Chart?
    if( pData && pData->SomeData1().Len() )
        aSelection = pData->SomeData1();
    else if( pSelection )
        aSelection = *pSelection;

    SwChartLines aLines;
    if( !IsTblComplexForChart( aSelection, &aLines ))
    {
        USHORT nLines = aLines.Count(), nBoxes = aLines[0]->Count();

        if( !pData )
        {
            //JP 08.02.99: als default wird mit Spalten/Zeilenueberschrift
            //              eingefuegt, deshalb das -1
            pData = SchDLL::NewMemChart( nBoxes-1, nLines-1 );
            pData->SetSubTitle( aEmptyStr );
            pData->SetXAxisTitle( aEmptyStr );
            pData->SetYAxisTitle( aEmptyStr );
            pData->SetZAxisTitle( aEmptyStr );
        }

        if( !pData->SomeData1().Len() )
        {
            pData->SomeData2().AssignAscii( RTL_CONSTASCII_STRINGPARAM("11") );

            //Den Title nur beim erzeugen setzen. Hinterher darf der Titel nicht
            //mehr angepasst werden, weil der Anwender diesen evtl. Geaendert hat.
            pData->SetMainTitle( GetFrmFmt()->GetName() );

            // dann mal den Namen setzen:
            const SwTableBox* pBox = (*aLines[0])[ 0 ];

            String &rStr = pData->SomeData1();
            rStr.Assign( '<' ).Append( pBox->GetName() ).Append( ':' );

            pBox = (*aLines[ nLines - 1 ])[ nBoxes - 1 ];
            rStr.Append( pBox->GetName() ).Append( '>' );
        }

        USHORT nRowStt = 0, nColStt = 0;
        if( pData->SomeData2().Len() )
        {
            if( '1' == pData->SomeData2().GetChar( 0 ))
                ++nRowStt;
            if( '1' == pData->SomeData2().GetChar( 1 ))
                ++nColStt;
        }

        if( (nBoxes - nColStt) > pData->GetColCount() )
            pData->InsertCols( 0, (nBoxes - nColStt) - pData->GetColCount() );
        else if( (nBoxes - nColStt) < pData->GetColCount() )
            pData->RemoveCols( 0, pData->GetColCount() - (nBoxes - nColStt) );

        if( (nLines - nRowStt) > pData->GetRowCount() )
            pData->InsertRows( 0, (nLines - nRowStt) - pData->GetRowCount() );
        else if( (nLines - nRowStt) < pData->GetRowCount() )
            pData->RemoveRows( 0, pData->GetRowCount() - (nLines - nRowStt) );


        ASSERT( pData->GetRowCount() >= (nLines - nRowStt ) &&
                pData->GetColCount() >= (nBoxes - nColStt ),
                    "Die Struktur fuers Chart ist zu klein,\n"
                    "es wird irgendwo in den Speicher geschrieben!" );

        // Row-Texte setzen
        USHORT n;
        if( nRowStt )
            for( n = nColStt; n < nBoxes; ++n )
            {
                const SwTableBox *pBox = (*aLines[ 0 ])[ n ];
                ASSERT( pBox->GetSttNd(), "Box without SttIdx" );
                SwNodeIndex aIdx( *pBox->GetSttNd(), 1 );
                const SwTxtNode* pTNd = aIdx.GetNode().GetTxtNode();
                if( !pTNd )
                    pTNd = aIdx.GetNodes().GoNextSection( &aIdx, TRUE, FALSE )
                                ->GetTxtNode();

                pData->SetColText( n - nColStt, pTNd->GetExpandTxt() );
            }
        else
        {
            String aText;
            for( n = nColStt; n < nBoxes; ++n )
            {
                SchDLL::GetDefaultForColumnText( *pData, n - nColStt, aText );
                pData->SetColText( n - nColStt, aText );
            }
        }

        // Col-Texte setzen
        if( nColStt )
            for( n = nRowStt; n < nLines; ++n )
            {
                const SwTableBox *pBox = (*aLines[ n ])[ 0 ];
                ASSERT( pBox->GetSttNd(), "Box without SttIdx" );
                SwNodeIndex aIdx( *pBox->GetSttNd(), 1 );
                const SwTxtNode* pTNd = aIdx.GetNode().GetTxtNode();
                if( !pTNd )
                    pTNd = aIdx.GetNodes().GoNextSection( &aIdx, TRUE, FALSE )
                                ->GetTxtNode();

                pData->SetRowText( n - nRowStt, pTNd->GetExpandTxt() );
            }
        else
        {
            String aText;
            for( n = nRowStt; n < nLines; ++n )
            {
                SchDLL::GetDefaultForRowText( *pData, n - nRowStt, aText );
                pData->SetRowText( n - nRowStt, aText );
            }
        }

        // und dann fehlen nur noch die Daten
        const SwTblBoxNumFormat& rDfltNumFmt = *(SwTblBoxNumFormat*)
                                        GetDfltAttr( RES_BOXATR_FORMAT );
        pData->SetNumberFormatter( GetFrmFmt()->GetDoc()->GetNumberFormatter());
        int bFirstRow = TRUE;
        for( n = nRowStt; n < nLines; ++n )
        {
            for( USHORT i = nColStt; i < nBoxes; ++i )
            {
                const SwTableBox* pBox = (*aLines[ n ])[ i ];
                ASSERT( pBox->GetSttNd(), "Box without SttIdx" );
                SwNodeIndex aIdx( *pBox->GetSttNd(), 1 );
                const SwTxtNode* pTNd = aIdx.GetNode().GetTxtNode();
                if( !pTNd )
                    pTNd = aIdx.GetNodes().GoNextSection( &aIdx, TRUE, FALSE )
                                ->GetTxtNode();

                pData->SetData( short( i - nColStt ),
                                short( n - nRowStt ),
                                pTNd->GetTxt().Len()
                                        ? pBox->GetValue( aCalcPara )
                                        : DBL_MIN );

                if( i == nColStt || bFirstRow )
                {
                    // first box of row set the numberformat
                    const SwTblBoxNumFormat& rNumFmt = pBox->GetFrmFmt()->
                                                        GetTblBoxNumFmt();
                    if( rNumFmt != rDfltNumFmt )
                    {
                        pData->SetNumFormatIdCol( i, rNumFmt.GetValue() );
                        if( bFirstRow )
                            pData->SetNumFormatIdRow( n, rNumFmt.GetValue() );
                    }
                }

            }
            bFirstRow = FALSE;
        }
    }
    else if( pData )
    {
        if( pData->GetColCount() )
            pData->RemoveCols( 0, pData->GetColCount() );
        if( pData->GetRowCount() )
            pData->RemoveRows( 0, pData->GetRowCount() );
    }

    return pData;
}

BOOL SwTable::IsTblComplexForChart( const String& rSelection,
                                    SwChartLines* pGetCLines ) const
{
    const SwTableBox* pSttBox, *pEndBox;
    if( 2 < rSelection.Len() )
    {
        // spitze Klammern am Anfang & Ende enfernen
        String sBox( rSelection );
        if( '<' == sBox.GetChar( 0  ) ) sBox.Erase( 0, 1 );
        if( '>' == sBox.GetChar( sBox.Len()-1  ) ) sBox.Erase( sBox.Len()-1 );

        xub_StrLen nTrenner = sBox.Search( ':' );
        ASSERT( STRING_NOTFOUND != nTrenner, "keine gueltige Selektion" );

        pSttBox = GetTblBox( sBox.Copy( 0, nTrenner ));
        pEndBox = GetTblBox( sBox.Copy( nTrenner+1 ));
    }
    else
    {
        const SwTableLines* pLns = &GetTabLines();
        pSttBox = (*pLns)[ 0 ]->GetTabBoxes()[ 0 ];
        while( !pSttBox->GetSttNd() )
            // bis zur Content Box!
            pSttBox = pSttBox->GetTabLines()[ 0 ]->GetTabBoxes()[ 0 ];

        const SwTableBoxes* pBoxes = &(*pLns)[ pLns->Count()-1 ]->GetTabBoxes();
        pEndBox = (*pBoxes)[ pBoxes->Count()-1 ];
        while( !pEndBox->GetSttNd() )
        {
            // bis zur Content Box!
            pLns = &pEndBox->GetTabLines();
            pBoxes = &(*pLns)[ pLns->Count()-1 ]->GetTabBoxes();
            pEndBox = (*pBoxes)[ pBoxes->Count()-1 ];
        }
    }

    return !pSttBox || !pEndBox || !::ChkChartSel( *pSttBox->GetSttNd(),
                                        *pEndBox->GetSttNd(), pGetCLines );
}



IMPL_LINK( SwDoc, DoUpdateAllCharts, Timer *, pTimer )
{
    ViewShell* pVSh;
    GetEditShell( &pVSh );
    if( pVSh )
    {
        const SwFrmFmts& rTblFmts = *GetTblFrmFmts();
        for( USHORT n = 0; n < rTblFmts.Count(); ++n )
        {
            SwTable* pTmpTbl;
            const SwTableNode* pTblNd;
            SwFrmFmt* pFmt = rTblFmts[ n ];

            if( 0 != ( pTmpTbl = SwTable::FindTable( pFmt ) ) &&
                0 != ( pTblNd = pTmpTbl->GetTableNode() ) &&
                pTblNd->GetNodes().IsDocNodes() )
            {
                _UpdateCharts( *pTmpTbl, *pVSh );
            }
        }
    }
    return 0;
}

void SwDoc::_UpdateCharts( const SwTable& rTbl, ViewShell& rVSh ) const
{
    String aName( rTbl.GetFrmFmt()->GetName() );
    SwOLENode *pONd;
    SwStartNode *pStNd;
    SwNodeIndex aIdx( *GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
    while( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
    {
        aIdx++;
        SwFrm* pFrm;
        if( 0 != ( pONd = aIdx.GetNode().GetOLENode() ) &&
            aName.Equals( pONd->GetChartTblName() ) &&
            0 != ( pFrm = pONd->GetFrm() ) )
        {
            SwOLEObj& rOObj = pONd->GetOLEObj();

            SchMemChart *pData = SchDLL::GetChartData( rOObj.GetOleRef() );
            FASTBOOL bDelData = 0 == pData;

            ASSERT( pData, "UpdateChart ohne irgendwelche Daten?" );
            pData = rTbl.UpdateData( pData );

            if( !rTbl.IsTblComplexForChart( pData->SomeData1() ) )
            {
                SchDLL::Update( rOObj.GetOleRef(), pData, rVSh.GetWin() );

                SwClientIter aIter( *pONd );
                for( pFrm = (SwFrm*)aIter.First( TYPE(SwFrm) ); pFrm;
                        pFrm = (SwFrm*)aIter.Next() )
                {
                    if( pFrm->Frm().HasArea() )
                        rVSh.InvalidateWindows( pFrm->Frm() );
                }
            }

            if ( bDelData )
                delete pData;
        }
        aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
    }
}

void SwDoc::UpdateCharts( const String &rName ) const
{
    SwTable* pTmpTbl = SwTable::FindTable( FindTblFmtByName( rName ) );
    if( pTmpTbl )
    {
        ViewShell* pVSh;
        GetEditShell( &pVSh );

        if( pVSh )
            _UpdateCharts( *pTmpTbl, *pVSh );
    }
}

void SwDoc::SetTableName( SwFrmFmt& rTblFmt, const String &rNewName )
{
    const String aOldName( rTblFmt.GetName() );

    BOOL bNameFound = 0 == rNewName.Len();
    if( !bNameFound )
    {
        SwFrmFmt* pFmt;
        const SwFrmFmts& rTbl = *GetTblFrmFmts();
        for( USHORT i = rTbl.Count(); i; )
            if( !( pFmt = rTbl[ --i ] )->IsDefault() &&
                pFmt->GetName() == rNewName && IsUsed( *pFmt ) )
            {
                bNameFound = TRUE;
                break;
            }
    }

    if( !bNameFound )
        rTblFmt.SetName( rNewName );
    else
        rTblFmt.SetName( GetUniqueTblName() );

    SwStartNode *pStNd;
    SwNodeIndex aIdx( *GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
    while ( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
    {
        aIdx++;
        SwOLENode *pNd = aIdx.GetNode().GetOLENode();
        if( pNd && aOldName == pNd->GetChartTblName() )
        {
            pNd->SetChartTblName( rNewName );

            SwOLEObj& rOObj = pNd->GetOLEObj();
            SchMemChart *pData = SchDLL::GetChartData( rOObj.GetOleRef() );
            if( pData )
            {
                ViewShell* pVSh;
                GetEditShell( &pVSh );

                if( aOldName == pData->GetMainTitle() )
                {
                    pData->SetMainTitle( rNewName );
                    if( pVSh )
                        SchDLL::Update( rOObj.GetOleRef(), pData, pVSh->GetWin() );
                }

                if( pVSh )
                {
                    SwFrm *pFrm;
                    SwClientIter aIter( *pNd );
                    for( pFrm = (SwFrm*)aIter.First( TYPE(SwFrm) ); pFrm;
                            pFrm = (SwFrm*)aIter.Next() )
                    {
                        if( pFrm->Frm().HasArea() )
                            pVSh->InvalidateWindows( pFrm->Frm() );
                    }
                }
            }
        }
        aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
    }
    SetModified();
}


