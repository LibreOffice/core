/*************************************************************************
 *
 *  $RCSfile: document.cxx,v $
 *
 *  $Revision: 1.58 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:54:49 $
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

// INCLUDE ---------------------------------------------------------------

#define _ZFORLIST_DECLARE_TABLE
#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/boxitem.hxx>
#include <svx/frmdiritem.hxx>
#include <svx/pageitem.hxx>
#include <svx/editeng.hxx>
#include <svx/svditer.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdocapt.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <svtools/poolcach.hxx>
#include <svtools/saveopt.hxx>
#include <svtools/zforlist.hxx>
#include <unotools/charclass.hxx>
#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif
#ifndef _TOOLS_TENCCVT_HXX
#include <tools/tenccvt.hxx>
#endif

#include "document.hxx"
#include "table.hxx"
#include "attrib.hxx"
#include "attarray.hxx"
#include "markarr.hxx"
#include "patattr.hxx"
#include "rangenam.hxx"
#include "poolhelp.hxx"
#include "docpool.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "globstr.hrc"
#include "rechead.hxx"
#include "dbcolect.hxx"
#include "pivot.hxx"
#include "chartlis.hxx"
#include "rangelst.hxx"
#include "markdata.hxx"
#include "drwlayer.hxx"
#include "conditio.hxx"
#include "validat.hxx"
#include "prnsave.hxx"
#include "chgtrack.hxx"
#include "sc.hrc"
#include "scresid.hxx"
#include "hints.hxx"
#include "detdata.hxx"
#include "cell.hxx"
#include "dpobject.hxx"
#include "indexmap.hxx"
#include "detfunc.hxx"      // for UpdateAllComments
#include "scmod.hxx"
#ifndef SC_DOCITER_HXX
#include "dociter.hxx"
#endif
#include "progress.hxx"
#ifndef __SGI_STL_SET
#include <set>
#endif

struct ScDefaultAttr
{
    const ScPatternAttr*    pAttr;
    SCROW                   nFirst;
    SCSIZE                  nCount;
    ScDefaultAttr(const ScPatternAttr* pPatAttr) : pAttr(pPatAttr), nFirst(0), nCount(0) {}
};

struct ScLessDefaultAttr
{
    sal_Bool operator() (const ScDefaultAttr& rValue1, const ScDefaultAttr& rValue2) const
    {
        return rValue1.pAttr < rValue2.pAttr;
    }
};

typedef std::set<ScDefaultAttr, ScLessDefaultAttr>  ScDefaultAttrSet;

void ScDocument::MakeTable( SCTAB nTab )
{
    if ( ValidTab(nTab) && !pTab[nTab] )
    {
        String aString = ScGlobal::GetRscString(STR_TABLE_DEF); //"Tabelle"
        aString += String::CreateFromInt32(nTab+1);
        CreateValidTabName( aString );  // keine doppelten

        pTab[nTab] = new ScTable(this, nTab, aString);
        ++nMaxTableNumber;
    }
}


BOOL ScDocument::HasTable( SCTAB nTab ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return TRUE;

    return FALSE;
}


BOOL ScDocument::GetName( SCTAB nTab, String& rName ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
        {
            pTab[nTab]->GetName( rName );
            return TRUE;
        }
    rName.Erase();
    return FALSE;
}


BOOL ScDocument::GetTable( const String& rName, SCTAB& rTab ) const
{
    String aUpperName = rName;
    ScGlobal::pCharClass->toUpper(aUpperName);
    String aCompName;

    for (SCTAB i=0; i<=MAXTAB; i++)
        if (pTab[i])
        {
            pTab[i]->GetName( aCompName );
            ScGlobal::pCharClass->toUpper(aCompName);
            if (aUpperName == aCompName)
            {
                rTab = i;
                return TRUE;
            }
        }
    rTab = 0;
    return FALSE;
}


BOOL ScDocument::ValidTabName( const String& rName ) const
{
    /*  If changed, ScfTools::ConvertToScSheetName (sc/source/filter/ftools/ftools.cxx)
        needs to be changed too. */
    using namespace ::com::sun::star::i18n;
    sal_Int32 nStartFlags = KParseTokens::ANY_LETTER_OR_NUMBER |
        KParseTokens::ASC_UNDERSCORE;
    sal_Int32 nContFlags = nStartFlags;
    String aContChars( RTL_CONSTASCII_USTRINGPARAM(" ") );
    ParseResult rRes = ScGlobal::pCharClass->parsePredefinedToken( KParseType::IDENTNAME, rName, 0,
        nStartFlags, EMPTY_STRING, nContFlags, aContChars );
    return (rRes.TokenType & KParseType::IDENTNAME) && rRes.EndPos == rName.Len();
}


BOOL ScDocument::ValidNewTabName( const String& rName ) const
{
    BOOL bValid = ValidTabName(rName);
    for (SCTAB i=0; (i<=MAXTAB) && bValid; i++)
        if (pTab[i])
        {
            String aOldName;
            pTab[i]->GetName(aOldName);
            bValid = !ScGlobal::pTransliteration->isEqual( rName, aOldName );
        }
    return bValid;
}


void ScDocument::CreateValidTabName(String& rName) const
{
    if ( !ValidTabName(rName) )
    {
        // neu erzeugen

        const String aStrTable( ScResId(SCSTR_TABLE) );
        BOOL         bOk   = FALSE;

        //  vorneweg testen, ob der Prefix als gueltig erkannt wird
        //  wenn nicht, nur doppelte vermeiden
        BOOL bPrefix = ValidTabName( aStrTable );
        DBG_ASSERT(bPrefix, "ungueltiger Tabellenname");
        SCTAB nDummy;

        SCTAB nLoops = 0;       // "zur Sicherheit"
        for ( SCTAB i = nMaxTableNumber+1; !bOk && nLoops <= MAXTAB; i++ )
        {
            rName  = aStrTable;
            rName += String::CreateFromInt32(i);
            if (bPrefix)
                bOk = ValidNewTabName( rName );
            else
                bOk = !GetTable( rName, nDummy );
            ++nLoops;
        }

        DBG_ASSERT(bOk, "kein gueltiger Tabellenname gefunden");
        if ( !bOk )
            rName = aStrTable;
    }
    else
    {
        // uebergebenen Namen ueberpruefen

        if ( !ValidNewTabName(rName) )
        {
            SCTAB i = 1;
            String aName;
            do
            {
                i++;
                aName = rName;
                aName += '_';
                aName += String::CreateFromInt32(static_cast<sal_Int32>(i));
            }
            while (!ValidNewTabName(aName) && (i < MAXTAB+1));
            rName = aName;
        }
    }
}


BOOL ScDocument::InsertTab( SCTAB nPos, const String& rName,
            BOOL bExternalDocument )
{
    SCTAB   nTabCount = GetTableCount();
    BOOL    bValid = ValidTab(nTabCount);
    if ( !bExternalDocument )   // sonst rName == "'Doc'!Tab", vorher pruefen
        bValid = (bValid && ValidNewTabName(rName));
    if (bValid)
    {
        if (nPos == SC_TAB_APPEND || nPos == nTabCount)
        {
            pTab[nTabCount] = new ScTable(this, nTabCount, rName);
            ++nMaxTableNumber;
            if ( bExternalDocument )
                pTab[nTabCount]->SetVisible( FALSE );
        }
        else
        {
            if (VALIDTAB(nPos) && (nPos < nTabCount))
            {
                ScRange aRange( 0,0,nPos, MAXCOL,MAXROW,MAXTAB );
                xColNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,1 );
                xRowNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,1 );
                pRangeName->UpdateTabRef( nPos, 1 );
                pDBCollection->UpdateReference(
                                    URM_INSDEL, 0,0,nPos, MAXCOL,MAXROW,MAXTAB, 0,0,1 );
                if (pPivotCollection)
                    pPivotCollection->UpdateReference(
                                    URM_INSDEL, 0,0,nPos, MAXCOL,MAXROW,MAXTAB, 0,0,1 );
                if (pDPCollection)
                    pDPCollection->UpdateReference( URM_INSDEL, aRange, 0,0,1 );
                if (pDetOpList)
                    pDetOpList->UpdateReference( this, URM_INSDEL, aRange, 0,0,1 );
                UpdateChartRef( URM_INSDEL, 0,0,nPos, MAXCOL,MAXROW,MAXTAB, 0,0,1 );
                UpdateRefAreaLinks( URM_INSDEL, aRange, 0,0,1 );
                if ( pUnoBroadcaster )
                    pUnoBroadcaster->Broadcast( ScUpdateRefHint( URM_INSDEL, aRange, 0,0,1 ) );

                SCTAB i;
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->UpdateInsertTab(nPos);
                for (i = nTabCount; i > nPos; i--)
                    pTab[i] = pTab[i - 1];
                pTab[nPos] = new ScTable(this, nPos, rName);
                ++nMaxTableNumber;
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->UpdateCompile();
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->StartAllListeners();

                //  update conditional formats after table is inserted
                if ( pCondFormList )
                    pCondFormList->UpdateReference( URM_INSDEL, aRange, 0,0,1 );
                // #81844# sheet names of references are not valid until sheet is inserted
                if ( pChartListenerCollection )
                    pChartListenerCollection->UpdateScheduledSeriesRanges();

                SetDirty();
                bValid = TRUE;
            }
            else
                bValid = FALSE;
        }
    }
    return bValid;
}


BOOL ScDocument::DeleteTab( SCTAB nTab, ScDocument* pRefUndoDoc )
{
    BOOL bValid = FALSE;
    if (VALIDTAB(nTab))
    {
        if (pTab[nTab])
        {
            SCTAB nTabCount = GetTableCount();
            if (nTabCount > 1)
            {
                BOOL bOldAutoCalc = GetAutoCalc();
                SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden
                ScRange aRange( 0, 0, nTab, MAXCOL, MAXROW, nTab );
                DelBroadcastAreasInRange( aRange );

                aRange.aEnd.SetTab( MAXTAB );
                xColNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,-1 );
                xRowNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,-1 );
                pRangeName->UpdateTabRef( nTab, 2 );
                pDBCollection->UpdateReference(
                                    URM_INSDEL, 0,0,nTab, MAXCOL,MAXROW,MAXTAB, 0,0,-1 );
                if (pPivotCollection)
                    pPivotCollection->UpdateReference(
                                    URM_INSDEL, 0,0,nTab, MAXCOL,MAXROW,MAXTAB, 0,0,-1 );
                if (pDPCollection)
                    pDPCollection->UpdateReference( URM_INSDEL, aRange, 0,0,-1 );
                if (pDetOpList)
                    pDetOpList->UpdateReference( this, URM_INSDEL, aRange, 0,0,-1 );
                UpdateChartRef( URM_INSDEL, 0,0,nTab, MAXCOL,MAXROW,MAXTAB, 0,0,-1 );
                UpdateRefAreaLinks( URM_INSDEL, aRange, 0,0,-1 );
                if ( pCondFormList )
                    pCondFormList->UpdateReference( URM_INSDEL, aRange, 0,0,-1 );
                if ( pUnoBroadcaster )
                    pUnoBroadcaster->Broadcast( ScUpdateRefHint( URM_INSDEL, aRange, 0,0,-1 ) );

                SCTAB i;
                for (i=0; i<=MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->UpdateDeleteTab(nTab,FALSE,
                                    pRefUndoDoc ? pRefUndoDoc->pTab[i] : 0);
                delete pTab[nTab];
                for (i=nTab + 1; i < nTabCount; i++)
                    pTab[i - 1] = pTab[i];
                pTab[nTabCount - 1] = NULL;
                --nMaxTableNumber;
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->UpdateCompile();
                // Excel-Filter loescht einige Tables waehrend des Ladens,
                // Listener werden erst nach dem Laden aufgesetzt
                if ( !bInsertingFromOtherDoc )
                {
                    for (i = 0; i <= MAXTAB; i++)
                        if (pTab[i])
                            pTab[i]->StartAllListeners();
                    SetDirty();
                }
                // #81844# sheet names of references are not valid until sheet is deleted
                pChartListenerCollection->UpdateScheduledSeriesRanges();
                SetAutoCalc( bOldAutoCalc );
                bValid = TRUE;
            }
        }
    }
    return bValid;
}


BOOL ScDocument::RenameTab( SCTAB nTab, const String& rName, BOOL bUpdateRef,
        BOOL bExternalDocument )
{
    BOOL    bValid = FALSE;
    SCTAB   i;
    if VALIDTAB(nTab)
        if (pTab[nTab])
        {
            if ( bExternalDocument )
                bValid = TRUE;      // zusammengesetzter Name
            else
                bValid = ValidTabName(rName);
            for (i=0; (i<=MAXTAB) && bValid; i++)
                if (pTab[i] && (i != nTab))
                {
                    String aOldName;
                    pTab[i]->GetName(aOldName);
                    bValid = !ScGlobal::pTransliteration->isEqual( rName, aOldName );
                }
            if (bValid)
            {
                pTab[nTab]->SetName(rName);
                if ( pChartListenerCollection )
                    pChartListenerCollection->UpdateSeriesRangesContainingTab( nTab );
            }
        }
    return bValid;
}


void ScDocument::SetVisible( SCTAB nTab, BOOL bVisible )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->SetVisible(bVisible);
}


BOOL ScDocument::IsVisible( SCTAB nTab ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->IsVisible();

    return FALSE;
}


void ScDocument::SetLayoutRTL( SCTAB nTab, BOOL bRTL )
{
    if ( ValidTab(nTab)  && pTab[nTab] )
    {
        pTab[nTab]->SetLayoutRTL( bRTL );       // only sets the flag
        pTab[nTab]->SetDrawPageSize();

        //  mirror existing objects:

        if (pDrawLayer)
        {
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
            DBG_ASSERT(pPage,"Page ?");
            if (pPage)
            {
                SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    //  objects with ScDrawObjData are re-positioned in SetPageSize,
                    //  don't mirror again
                    ScDrawObjData* pData = ScDrawLayer::GetObjData( pObject );
                    if ( !pData )
                        pDrawLayer->MirrorRTL( pObject );
                    else if ( pObject->ISA( SdrCaptionObj ) )
                    {
                        //  Re-positioning only moves the whole object so the tail
                        //  points to the cell. The position of the text area relative
                        //  to the tail must be mirrored here.

                        SdrCaptionObj* pCaption = (SdrCaptionObj*) pObject;
                        Rectangle aOldRect = pCaption->GetLogicRect();
                        long nTail = pCaption->GetTailPos().X();
                        Rectangle aNewRect = aOldRect;

                        aNewRect.Left() = 2 * nTail - aOldRect.Right();
                        aNewRect.Right() = 2 * nTail - aOldRect.Left();

                        pCaption->SetLogicRect( aNewRect );
                    }

                    pObject = aIter.Next();
                }
            }
        }
    }
}


BOOL ScDocument::IsLayoutRTL( SCTAB nTab ) const
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        return pTab[nTab]->IsLayoutRTL();

    return FALSE;
}


BOOL ScDocument::IsNegativePage( SCTAB nTab ) const
{
    //  Negative page area is always used for RTL layout.
    //  The separate method is used to find all RTL handling of drawing objects.
    return IsLayoutRTL( nTab );
}


/* ----------------------------------------------------------------------------
    benutzten Bereich suchen:

    GetCellArea  - nur Daten
    GetTableArea - Daten / Attribute
    GetPrintArea - beruecksichtigt auch Zeichenobjekte,
                    streicht Attribute bis ganz rechts / unten
---------------------------------------------------------------------------- */


BOOL ScDocument::GetCellArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->GetCellArea( rEndCol, rEndRow );

    rEndCol = 0;
    rEndRow = 0;
    return FALSE;
}


BOOL ScDocument::GetTableArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->GetTableArea( rEndCol, rEndRow );

    rEndCol = 0;
    rEndRow = 0;
    return FALSE;
}


//  zusammenhaengender Bereich

void ScDocument::GetDataArea( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow,
                                SCCOL& rEndCol, SCROW& rEndRow, BOOL bIncludeOld )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->GetDataArea( rStartCol, rStartRow, rEndCol, rEndRow, bIncludeOld );
}


void ScDocument::LimitChartArea( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow,
                                    SCCOL& rEndCol, SCROW& rEndRow )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->LimitChartArea( rStartCol, rStartRow, rEndCol, rEndRow );
}


void ScDocument::LimitChartIfAll( ScRangeListRef& rRangeList )
{
    ScRangeListRef aNew = new ScRangeList;
    if (rRangeList.Is())
    {
        ULONG nCount = rRangeList->Count();
        for (ULONG i=0; i<nCount; i++)
        {
            ScRange aRange(*rRangeList->GetObject( i ));
            if ( ( aRange.aStart.Col() == 0 && aRange.aEnd.Col() == MAXCOL ) ||
                 ( aRange.aStart.Row() == 0 && aRange.aEnd.Row() == MAXROW ) )
            {
                SCCOL nStartCol = aRange.aStart.Col();
                SCROW nStartRow = aRange.aStart.Row();
                SCCOL nEndCol = aRange.aEnd.Col();
                SCROW nEndRow = aRange.aEnd.Row();
                SCTAB nTab = aRange.aStart.Tab();
                if (pTab[nTab])
                    pTab[nTab]->LimitChartArea(nStartCol, nStartRow, nEndCol, nEndRow);
                aRange.aStart.SetCol( nStartCol );
                aRange.aStart.SetRow( nStartRow );
                aRange.aEnd.SetCol( nEndCol );
                aRange.aEnd.SetRow( nEndRow );
            }
            aNew->Append(aRange);
        }
    }
    else
        DBG_ERROR("LimitChartIfAll: Ref==0");
    rRangeList = aNew;
}


BOOL ScDocument::CanInsertRow( const ScRange& rRange ) const
{
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();
    PutInOrder( nStartCol, nEndCol );
    PutInOrder( nStartRow, nEndRow );
    PutInOrder( nStartTab, nEndTab );
    SCSIZE nSize = static_cast<SCSIZE>(nEndRow - nStartRow + 1);

    BOOL bTest = TRUE;
    for (SCTAB i=nStartTab; i<=nEndTab && bTest; i++)
        if (pTab[i])
            bTest &= pTab[i]->TestInsertRow( nStartCol, nEndCol, nSize );

    return bTest;
}


BOOL ScDocument::InsertRow( SCCOL nStartCol, SCTAB nStartTab,
                            SCCOL nEndCol,   SCTAB nEndTab,
                            SCROW nStartRow, SCSIZE nSize, ScDocument* pRefUndoDoc )
{
    SCTAB i;

    PutInOrder( nStartCol, nEndCol );
    PutInOrder( nStartTab, nEndTab );

    BOOL bTest = TRUE;
    BOOL bRet = FALSE;
    BOOL bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden
    for ( i = nStartTab; i <= nEndTab && bTest; i++)
        if (pTab[i])
            bTest &= pTab[i]->TestInsertRow( nStartCol, nEndCol, nSize );
    if (bTest)
    {
        // UpdateBroadcastAreas muss vor UpdateReference gerufen werden, damit nicht
        // Eintraege verschoben werden, die erst bei UpdateReference neu erzeugt werden

        UpdateBroadcastAreas( URM_INSDEL, ScRange(
            ScAddress( nStartCol, nStartRow, nStartTab ),
            ScAddress( nEndCol, MAXROW, nEndTab )), 0, static_cast<SCsROW>(nSize), 0 );
        UpdateReference( URM_INSDEL, nStartCol, nStartRow, nStartTab,
                         nEndCol, MAXROW, nEndTab,
                         0, static_cast<SCsROW>(nSize), 0, pRefUndoDoc, FALSE );        // without drawing objects
        for (i=nStartTab; i<=nEndTab; i++)
            if (pTab[i])
                pTab[i]->InsertRow( nStartCol, nEndCol, nStartRow, nSize );

        //  #82991# UpdateRef for drawing layer must be after inserting,
        //  when the new row heights are known.
        for (i=nStartTab; i<=nEndTab; i++)
            if (pTab[i])
                pTab[i]->UpdateDrawRef( URM_INSDEL,
                            nStartCol, nStartRow, nStartTab, nEndCol, MAXROW, nEndTab,
                            0, static_cast<SCsROW>(nSize), 0 );

        if ( pChangeTrack && pChangeTrack->IsInDeleteUndo() )
        {   // durch Restaurierung von Referenzen auf geloeschte Bereiche ist
            // ein neues Listening faellig, bisherige Listener wurden in
            // FormulaCell UpdateReference abgehaengt
            StartAllListeners();
        }
        else
        {   // RelName listeners have been removed in UpdateReference
            for (i=0; i<=MAXTAB; i++)
                if (pTab[i])
                    pTab[i]->StartNameListeners( TRUE );
            // #69592# at least all cells using range names pointing relative
            // to the moved range must recalculate
            for (i=0; i<=MAXTAB; i++)
                if (pTab[i])
                    pTab[i]->SetRelNameDirty();
        }
        bRet = TRUE;
    }
    SetAutoCalc( bOldAutoCalc );
    if ( bRet )
        pChartListenerCollection->UpdateDirtyCharts();
    return bRet;
}


BOOL ScDocument::InsertRow( const ScRange& rRange, ScDocument* pRefUndoDoc )
{
    return InsertRow( rRange.aStart.Col(), rRange.aStart.Tab(),
                      rRange.aEnd.Col(),   rRange.aEnd.Tab(),
                      rRange.aStart.Row(), static_cast<SCSIZE>(rRange.aEnd.Row()-rRange.aStart.Row()+1),
                      pRefUndoDoc );
}


void ScDocument::DeleteRow( SCCOL nStartCol, SCTAB nStartTab,
                            SCCOL nEndCol,   SCTAB nEndTab,
                            SCROW nStartRow, SCSIZE nSize,
                            ScDocument* pRefUndoDoc, BOOL* pUndoOutline )
{
    SCTAB i;

    PutInOrder( nStartCol, nEndCol );
    PutInOrder( nStartTab, nEndTab );

    BOOL bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden

    if ( ValidRow(nStartRow+nSize) )
    {
        DelBroadcastAreasInRange( ScRange(
            ScAddress( nStartCol, nStartRow, nStartTab ),
            ScAddress( nEndCol, nStartRow+nSize-1, nEndTab ) ) );
        UpdateBroadcastAreas( URM_INSDEL, ScRange(
            ScAddress( nStartCol, nStartRow+nSize, nStartTab ),
            ScAddress( nEndCol, MAXROW, nEndTab )), 0, -(static_cast<SCsROW>(nSize)), 0 );
    }
    else
        DelBroadcastAreasInRange( ScRange(
            ScAddress( nStartCol, nStartRow, nStartTab ),
            ScAddress( nEndCol, MAXROW, nEndTab ) ) );

    if ( ValidRow(nStartRow+nSize) )
    {
        UpdateReference( URM_INSDEL, nStartCol, nStartRow+nSize, nStartTab,
                         nEndCol, MAXROW, nEndTab,
                         0, -(static_cast<SCsROW>(nSize)), 0, pRefUndoDoc );
    }

    if (pUndoOutline)
        *pUndoOutline = FALSE;

    for ( i = nStartTab; i <= nEndTab; i++)
        if (pTab[i])
            pTab[i]->DeleteRow( nStartCol, nEndCol, nStartRow, nSize, pUndoOutline );

    if ( ValidRow(nStartRow+nSize) )
    {   // Name listeners have been removed in UpdateReference
        for (i=0; i<=MAXTAB; i++)
            if (pTab[i])
                pTab[i]->StartNameListeners( FALSE );
        // #69592# at least all cells using range names pointing relative to
        // the moved range must recalculate
        for (i=0; i<=MAXTAB; i++)
            if (pTab[i])
                pTab[i]->SetRelNameDirty();
    }

    SetAutoCalc( bOldAutoCalc );
    pChartListenerCollection->UpdateDirtyCharts();
}


void ScDocument::DeleteRow( const ScRange& rRange, ScDocument* pRefUndoDoc, BOOL* pUndoOutline )
{
    DeleteRow( rRange.aStart.Col(), rRange.aStart.Tab(),
               rRange.aEnd.Col(),   rRange.aEnd.Tab(),
               rRange.aStart.Row(), static_cast<SCSIZE>(rRange.aEnd.Row()-rRange.aStart.Row()+1),
               pRefUndoDoc, pUndoOutline );
}


BOOL ScDocument::CanInsertCol( const ScRange& rRange ) const
{
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();
    PutInOrder( nStartCol, nEndCol );
    PutInOrder( nStartRow, nEndRow );
    PutInOrder( nStartTab, nEndTab );
    SCSIZE nSize = static_cast<SCSIZE>(nEndCol - nStartCol + 1);

    BOOL bTest = TRUE;
    for (SCTAB i=nStartTab; i<=nEndTab && bTest; i++)
        if (pTab[i])
            bTest &= pTab[i]->TestInsertCol( nStartRow, nEndRow, nSize );

    return bTest;
}


BOOL ScDocument::InsertCol( SCROW nStartRow, SCTAB nStartTab,
                            SCROW nEndRow,   SCTAB nEndTab,
                            SCCOL nStartCol, SCSIZE nSize, ScDocument* pRefUndoDoc )
{
    SCTAB i;

    PutInOrder( nStartRow, nEndRow );
    PutInOrder( nStartTab, nEndTab );

    BOOL bTest = TRUE;
    BOOL bRet = FALSE;
    BOOL bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden
    for ( i = nStartTab; i <= nEndTab && bTest; i++)
        if (pTab[i])
            bTest &= pTab[i]->TestInsertCol( nStartRow, nEndRow, nSize );
    if (bTest)
    {
        UpdateBroadcastAreas( URM_INSDEL, ScRange(
            ScAddress( nStartCol, nStartRow, nStartTab ),
            ScAddress( MAXCOL, nEndRow, nEndTab )), static_cast<SCsCOL>(nSize), 0, 0 );
        UpdateReference( URM_INSDEL, nStartCol, nStartRow, nStartTab,
                         MAXCOL, nEndRow, nEndTab,
                         static_cast<SCsCOL>(nSize), 0, 0, pRefUndoDoc );
        for (i=nStartTab; i<=nEndTab; i++)
            if (pTab[i])
                pTab[i]->InsertCol( nStartCol, nStartRow, nEndRow, nSize );

        if ( pChangeTrack && pChangeTrack->IsInDeleteUndo() )
        {   // durch Restaurierung von Referenzen auf geloeschte Bereiche ist
            // ein neues Listening faellig, bisherige Listener wurden in
            // FormulaCell UpdateReference abgehaengt
            StartAllListeners();
        }
        else
        {   // RelName listeners have been removed in UpdateReference
            for (i=0; i<=MAXTAB; i++)
                if (pTab[i])
                    pTab[i]->StartNameListeners( TRUE );
            // #69592# at least all cells using range names pointing relative
            // to the moved range must recalculate
            for (i=0; i<=MAXTAB; i++)
                if (pTab[i])
                    pTab[i]->SetRelNameDirty();
        }
        bRet = TRUE;
    }
    SetAutoCalc( bOldAutoCalc );
    if ( bRet )
        pChartListenerCollection->UpdateDirtyCharts();
    return bRet;
}


BOOL ScDocument::InsertCol( const ScRange& rRange, ScDocument* pRefUndoDoc )
{
    return InsertCol( rRange.aStart.Row(), rRange.aStart.Tab(),
                      rRange.aEnd.Row(),   rRange.aEnd.Tab(),
                      rRange.aStart.Col(), static_cast<SCSIZE>(rRange.aEnd.Col()-rRange.aStart.Col()+1),
                      pRefUndoDoc );
}


void ScDocument::DeleteCol(SCROW nStartRow, SCTAB nStartTab, SCROW nEndRow, SCTAB nEndTab,
                                SCCOL nStartCol, SCSIZE nSize, ScDocument* pRefUndoDoc,
                                BOOL* pUndoOutline )
{
    SCTAB i;

    PutInOrder( nStartRow, nEndRow );
    PutInOrder( nStartTab, nEndTab );

    BOOL bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden

    if ( ValidCol(nStartCol+nSize) )
    {
        DelBroadcastAreasInRange( ScRange(
            ScAddress( nStartCol, nStartRow, nStartTab ),
            ScAddress( nStartCol+nSize-1, nEndRow, nEndTab ) ) );
        UpdateBroadcastAreas( URM_INSDEL, ScRange(
            ScAddress( nStartCol+nSize, nStartRow, nStartTab ),
            ScAddress( MAXCOL, nEndRow, nEndTab )), -static_cast<SCsCOL>(nSize), 0, 0 );
    }
    else
        DelBroadcastAreasInRange( ScRange(
            ScAddress( nStartCol, nStartRow, nStartTab ),
            ScAddress( MAXCOL, nEndRow, nEndTab ) ) );

    if ( ValidCol(nStartCol+nSize) )
    {
        UpdateReference( URM_INSDEL, nStartCol+nSize, nStartRow, nStartTab,
                         MAXCOL, nEndRow, nEndTab,
                         -static_cast<SCsCOL>(nSize), 0, 0, pRefUndoDoc );
    }

    if (pUndoOutline)
        *pUndoOutline = FALSE;

    for ( i = nStartTab; i <= nEndTab; i++)
        if (pTab[i])
            pTab[i]->DeleteCol( nStartCol, nStartRow, nEndRow, nSize, pUndoOutline );

    if ( ValidCol(nStartCol+nSize) )
    {   // Name listeners have been removed in UpdateReference
        for (i=0; i<=MAXTAB; i++)
            if (pTab[i])
                pTab[i]->StartNameListeners( FALSE );
        // #69592# at least all cells using range names pointing relative to
        // the moved range must recalculate
        for (i=0; i<=MAXTAB; i++)
            if (pTab[i])
                pTab[i]->SetRelNameDirty();
    }

    SetAutoCalc( bOldAutoCalc );
    pChartListenerCollection->UpdateDirtyCharts();
}


void ScDocument::DeleteCol( const ScRange& rRange, ScDocument* pRefUndoDoc, BOOL* pUndoOutline )
{
    DeleteCol( rRange.aStart.Row(), rRange.aStart.Tab(),
               rRange.aEnd.Row(),   rRange.aEnd.Tab(),
               rRange.aStart.Col(), static_cast<SCSIZE>(rRange.aEnd.Col()-rRange.aStart.Col()+1),
               pRefUndoDoc, pUndoOutline );
}


//  fuer Area-Links: Zellen einuegen/loeschen, wenn sich der Bereich veraendert
//  (ohne Paint)


void lcl_GetInsDelRanges( const ScRange& rOld, const ScRange& rNew,
                            ScRange& rColRange, BOOL& rInsCol, BOOL& rDelCol,
                            ScRange& rRowRange, BOOL& rInsRow, BOOL& rDelRow )
{
    DBG_ASSERT( rOld.aStart == rNew.aStart, "FitBlock: Anfang unterschiedlich" );

    rInsCol = rDelCol = rInsRow = rDelRow = FALSE;

    SCCOL nStartX = rOld.aStart.Col();
    SCROW nStartY = rOld.aStart.Row();
    SCCOL nOldEndX = rOld.aEnd.Col();
    SCROW nOldEndY = rOld.aEnd.Row();
    SCCOL nNewEndX = rNew.aEnd.Col();
    SCROW nNewEndY = rNew.aEnd.Row();
    SCTAB nTab = rOld.aStart.Tab();

    //  wenn es mehr Zeilen werden, werden Spalten auf der alten Hoehe eingefuegt/geloescht
    BOOL bGrowY = ( nNewEndY > nOldEndY );
    SCROW nColEndY = bGrowY ? nOldEndY : nNewEndY;
    SCCOL nRowEndX = bGrowY ? nNewEndX : nOldEndX;

    //  Spalten

    if ( nNewEndX > nOldEndX )          // Spalten einfuegen
    {
        rColRange = ScRange( nOldEndX+1, nStartY, nTab, nNewEndX, nColEndY, nTab );
        rInsCol = TRUE;
    }
    else if ( nNewEndX < nOldEndX )     // Spalten loeschen
    {
        rColRange = ScRange( nNewEndX+1, nStartY, nTab, nOldEndX, nColEndY, nTab );
        rDelCol = TRUE;
    }

    //  Zeilen

    if ( nNewEndY > nOldEndY )          // Zeilen einfuegen
    {
        rRowRange = ScRange( nStartX, nOldEndY+1, nTab, nRowEndX, nNewEndY, nTab );
        rInsRow = TRUE;
    }
    else if ( nNewEndY < nOldEndY )     // Zeilen loeschen
    {
        rRowRange = ScRange( nStartX, nNewEndY+1, nTab, nRowEndX, nOldEndY, nTab );
        rDelRow = TRUE;
    }
}


BOOL ScDocument::HasPartOfMerged( const ScRange& rRange )
{
    BOOL bPart = FALSE;
    SCTAB nTab = rRange.aStart.Tab();

    SCCOL nStartX = rRange.aStart.Col();
    SCROW nStartY = rRange.aStart.Row();
    SCCOL nEndX = rRange.aEnd.Col();
    SCROW nEndY = rRange.aEnd.Row();

    if (HasAttrib( nStartX, nStartY, nTab, nEndX, nEndY, nTab,
                        HASATTR_MERGED | HASATTR_OVERLAPPED ))
    {
        ExtendMerge( nStartX, nStartY, nEndX, nEndY, nTab );
        ExtendOverlapped( nStartX, nStartY, nEndX, nEndY, nTab );

        bPart = ( nStartX != rRange.aStart.Col() || nEndX != rRange.aEnd.Col() ||
                  nStartY != rRange.aStart.Row() || nEndY != rRange.aEnd.Row() );
    }
    return bPart;
}


BOOL ScDocument::CanFitBlock( const ScRange& rOld, const ScRange& rNew )
{
    if ( rOld == rNew )
        return TRUE;

    SCTAB nTab = rOld.aStart.Tab();
    BOOL bOk = TRUE;
    BOOL bInsCol,bDelCol,bInsRow,bDelRow;
    ScRange aColRange,aRowRange;
    lcl_GetInsDelRanges( rOld, rNew, aColRange,bInsCol,bDelCol, aRowRange,bInsRow,bDelRow );

    if ( bInsCol && !CanInsertCol( aColRange ) )            // Zellen am Rand ?
        bOk = FALSE;
    if ( bInsRow && !CanInsertRow( aRowRange ) )            // Zellen am Rand ?
        bOk = FALSE;

    if ( bInsCol || bDelCol )
    {
        aColRange.aEnd.SetCol(MAXCOL);
        if ( HasPartOfMerged(aColRange) )
            bOk = FALSE;
    }
    if ( bInsRow || bDelRow )
    {
        aRowRange.aEnd.SetRow(MAXROW);
        if ( HasPartOfMerged(aRowRange) )
            bOk = FALSE;
    }

    return bOk;
}


void ScDocument::FitBlock( const ScRange& rOld, const ScRange& rNew, BOOL bClear )
{
    if (bClear)
        DeleteAreaTab( rOld, IDF_ALL );

    BOOL bInsCol,bDelCol,bInsRow,bDelRow;
    ScRange aColRange,aRowRange;
    lcl_GetInsDelRanges( rOld, rNew, aColRange,bInsCol,bDelCol, aRowRange,bInsRow,bDelRow );

    if ( bInsCol )
        InsertCol( aColRange );         // Spalten zuerst einfuegen
    if ( bInsRow )
        InsertRow( aRowRange );

    if ( bDelRow )
        DeleteRow( aRowRange );         // Zeilen zuerst loeschen
    if ( bDelCol )
        DeleteCol( aColRange );

    //  Referenzen um eingefuegte Zeilen erweitern

    if ( bInsCol || bInsRow )
    {
        ScRange aGrowSource = rOld;
        aGrowSource.aEnd.SetCol(Min( rOld.aEnd.Col(), rNew.aEnd.Col() ));
        aGrowSource.aEnd.SetRow(Min( rOld.aEnd.Row(), rNew.aEnd.Row() ));
        SCCOL nGrowX = bInsCol ? ( rNew.aEnd.Col() - rOld.aEnd.Col() ) : 0;
        SCROW nGrowY = bInsRow ? ( rNew.aEnd.Row() - rOld.aEnd.Row() ) : 0;
        UpdateGrow( aGrowSource, nGrowX, nGrowY );
    }
}


void ScDocument::DeleteArea(SCCOL nCol1, SCROW nRow1,
                            SCCOL nCol2, SCROW nRow2,
                            const ScMarkData& rMark, USHORT nDelFlag)
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    BOOL bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden
    for (SCTAB i = 0; i <= MAXTAB; i++)
        if (pTab[i])
            if ( rMark.GetTableSelect(i) || bIsUndo )
                pTab[i]->DeleteArea(nCol1, nRow1, nCol2, nRow2, nDelFlag);
    SetAutoCalc( bOldAutoCalc );
}


void ScDocument::DeleteAreaTab(SCCOL nCol1, SCROW nRow1,
                                SCCOL nCol2, SCROW nRow2,
                                SCTAB nTab, USHORT nDelFlag)
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    if ( VALIDTAB(nTab) && pTab[nTab] )
    {
        BOOL bOldAutoCalc = GetAutoCalc();
        SetAutoCalc( FALSE );   // Mehrfachberechnungen vermeiden
        pTab[nTab]->DeleteArea(nCol1, nRow1, nCol2, nRow2, nDelFlag);
        SetAutoCalc( bOldAutoCalc );
    }
}


void ScDocument::DeleteAreaTab( const ScRange& rRange, USHORT nDelFlag )
{
    for ( SCTAB nTab = rRange.aStart.Tab(); nTab <= rRange.aEnd.Tab(); nTab++ )
        DeleteAreaTab( rRange.aStart.Col(), rRange.aStart.Row(),
                       rRange.aEnd.Col(),   rRange.aEnd.Row(),
                       nTab, nDelFlag );
}


void ScDocument::InitUndoSelected( ScDocument* pSrcDoc, const ScMarkData& rTabSelection,
                                BOOL bColInfo, BOOL bRowInfo )
{
    if (bIsUndo)
    {
        Clear();

        xPoolHelper = pSrcDoc->xPoolHelper;

        String aString;
        for (SCTAB nTab = 0; nTab <= MAXTAB; nTab++)
            if ( rTabSelection.GetTableSelect( nTab ) )
            {
                pTab[nTab] = new ScTable(this, nTab, aString, bColInfo, bRowInfo);
                nMaxTableNumber = nTab + 1;
            }
    }
    else
        DBG_ERROR("InitUndo");
}


void ScDocument::InitUndo( ScDocument* pSrcDoc, SCTAB nTab1, SCTAB nTab2,
                                BOOL bColInfo, BOOL bRowInfo )
{
    if (bIsUndo)
    {
        Clear();

        xPoolHelper = pSrcDoc->xPoolHelper;

        String aString;
        for (SCTAB nTab = nTab1; nTab <= nTab2; nTab++)
            pTab[nTab] = new ScTable(this, nTab, aString, bColInfo, bRowInfo);

        nMaxTableNumber = nTab2 + 1;
    }
    else
        DBG_ERROR("InitUndo");
}


void ScDocument::AddUndoTab( SCTAB nTab1, SCTAB nTab2, BOOL bColInfo, BOOL bRowInfo )
{
    if (bIsUndo)
    {
        String aString;
        for (SCTAB nTab = nTab1; nTab <= nTab2; nTab++)
            if (!pTab[nTab])
                pTab[nTab] = new ScTable(this, nTab, aString, bColInfo, bRowInfo);

        if ( nMaxTableNumber <= nTab2 )
            nMaxTableNumber = nTab2 + 1;
    }
    else
        DBG_ERROR("InitUndo");
}


void ScDocument::SetCutMode( BOOL bVal )
{
    if (bIsClip)
        bCutMode = bVal;
    else
    {
        DBG_ERROR("SetCutMode without bIsClip");
    }
}


BOOL ScDocument::IsCutMode()
{
    if (bIsClip)
        return bCutMode;
    else
    {
        DBG_ERROR("IsCutMode ohne bIsClip");
        return FALSE;
    }
}


void ScDocument::CopyToDocument(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                            SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                            USHORT nFlags, BOOL bOnlyMarked, ScDocument* pDestDoc,
                            const ScMarkData* pMarks, BOOL bColRowFlags )
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    PutInOrder( nTab1, nTab2 );
    if( !pDestDoc->aDocName.Len() )
        pDestDoc->aDocName = aDocName;
    if (VALIDTAB(nTab1) && VALIDTAB(nTab2))
    {
        BOOL bOldAutoCalc = pDestDoc->GetAutoCalc();
        pDestDoc->SetAutoCalc( FALSE );     // Mehrfachberechnungen vermeiden
        for (SCTAB i = nTab1; i <= nTab2; i++)
        {
            if (pTab[i] && pDestDoc->pTab[i])
                pTab[i]->CopyToTable( nCol1, nRow1, nCol2, nRow2, nFlags,
                                      bOnlyMarked, pDestDoc->pTab[i], pMarks,
                                      FALSE, bColRowFlags );
        }
        pDestDoc->SetAutoCalc( bOldAutoCalc );
    }
}


void ScDocument::UndoToDocument(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                            SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                            USHORT nFlags, BOOL bOnlyMarked, ScDocument* pDestDoc,
                            const ScMarkData* pMarks)
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    PutInOrder( nTab1, nTab2 );
    if (VALIDTAB(nTab1) && VALIDTAB(nTab2))
    {
        BOOL bOldAutoCalc = pDestDoc->GetAutoCalc();
        pDestDoc->SetAutoCalc( FALSE );     // Mehrfachberechnungen vermeiden
        if (nTab1 > 0)
            CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTab1-1, IDF_FORMULA, FALSE, pDestDoc, pMarks );

        for (SCTAB i = nTab1; i <= nTab2; i++)
        {
            if (pTab[i] && pDestDoc->pTab[i])
                pTab[i]->UndoToTable(nCol1, nRow1, nCol2, nRow2, nFlags,
                                    bOnlyMarked, pDestDoc->pTab[i], pMarks);
        }

        if (nTab2 < MAXTAB)
            CopyToDocument( 0,0,nTab2+1, MAXCOL,MAXROW,MAXTAB, IDF_FORMULA, FALSE, pDestDoc, pMarks );
        pDestDoc->SetAutoCalc( bOldAutoCalc );
    }
}


void ScDocument::CopyToDocument(const ScRange& rRange,
                            USHORT nFlags, BOOL bOnlyMarked, ScDocument* pDestDoc,
                            const ScMarkData* pMarks, BOOL bColRowFlags)
{
    ScRange aNewRange = rRange;
    aNewRange.Justify();

    if( !pDestDoc->aDocName.Len() )
        pDestDoc->aDocName = aDocName;
    BOOL bOldAutoCalc = pDestDoc->GetAutoCalc();
    pDestDoc->SetAutoCalc( FALSE );     // Mehrfachberechnungen vermeiden
    for (SCTAB i = aNewRange.aStart.Tab(); i <= aNewRange.aEnd.Tab(); i++)
        if (pTab[i] && pDestDoc->pTab[i])
            pTab[i]->CopyToTable(aNewRange.aStart.Col(), aNewRange.aStart.Row(),
                                 aNewRange.aEnd.Col(), aNewRange.aEnd.Row(),
                                 nFlags, bOnlyMarked, pDestDoc->pTab[i],
                                 pMarks, FALSE, bColRowFlags);
    pDestDoc->SetAutoCalc( bOldAutoCalc );
}


void ScDocument::UndoToDocument(const ScRange& rRange,
                            USHORT nFlags, BOOL bOnlyMarked, ScDocument* pDestDoc,
                            const ScMarkData* pMarks)
{
    ScRange aNewRange = rRange;
    aNewRange.Justify();
    SCTAB nTab1 = aNewRange.aStart.Tab();
    SCTAB nTab2 = aNewRange.aEnd.Tab();

    BOOL bOldAutoCalc = pDestDoc->GetAutoCalc();
    pDestDoc->SetAutoCalc( FALSE );     // Mehrfachberechnungen vermeiden
    if (nTab1 > 0)
        CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTab1-1, IDF_FORMULA, FALSE, pDestDoc, pMarks );

    for (SCTAB i = nTab1; i <= nTab2; i++)
    {
        if (pTab[i] && pDestDoc->pTab[i])
            pTab[i]->UndoToTable(aNewRange.aStart.Col(), aNewRange.aStart.Row(),
                                    aNewRange.aEnd.Col(), aNewRange.aEnd.Row(),
                                    nFlags, bOnlyMarked, pDestDoc->pTab[i], pMarks);
    }

    if (nTab2 < MAXTAB)
        CopyToDocument( 0,0,nTab2+1, MAXCOL,MAXROW,MAXTAB, IDF_FORMULA, FALSE, pDestDoc, pMarks );
    pDestDoc->SetAutoCalc( bOldAutoCalc );
}


void ScDocument::CopyToClip(SCCOL nCol1, SCROW nRow1,
                            SCCOL nCol2, SCROW nRow2,
                            BOOL bCut, ScDocument* pClipDoc,
                            BOOL bAllTabs, const ScMarkData* pMarks,
                            BOOL bKeepScenarioFlags, BOOL bIncludeObjects)
{
    DBG_ASSERT( bAllTabs || pMarks, "CopyToClip: ScMarkData fehlt" );

    if (!bIsClip)
    {
        PutInOrder( nCol1, nCol2 );
        PutInOrder( nRow1, nRow2 );
        if (!pClipDoc)
        {
            DBG_ERROR("CopyToClip: no ClipDoc");
            pClipDoc = SC_MOD()->GetClipDoc();
        }

        pClipDoc->aDocName = aDocName;
        pClipDoc->aClipRange = ScRange( nCol1,nRow1,0, nCol2,nRow2,0 );
        pClipDoc->ResetClip( this, pMarks );
        USHORT i;
        SCTAB j;
        pClipDoc->pRangeName->FreeAll();
        for (i = 0; i < pRangeName->GetCount(); i++)        //! DB-Bereiche Pivot-Bereiche auch !!!
        {
            USHORT nIndex = ((ScRangeData*)((*pRangeName)[i]))->GetIndex();
            BOOL bInUse = FALSE;
            for (j = 0; !bInUse && (j <= MAXTAB); j++)
            {
                if (pTab[j])
                    bInUse = pTab[j]->IsRangeNameInUse(nCol1, nRow1, nCol2, nRow2,
                                                       nIndex);
            }
            if (bInUse)
            {
                ScRangeData* pData = new ScRangeData(*((*pRangeName)[i]));
                if (!pClipDoc->pRangeName->Insert(pData))
                    delete pData;
                else
                    pData->SetIndex(nIndex);
            }
        }
        for (j = 0; j <= MAXTAB; j++)
            if (pTab[j] && pClipDoc->pTab[j])
                if ( bAllTabs || !pMarks || pMarks->GetTableSelect(j) )
                {
                    pTab[j]->CopyToClip(nCol1, nRow1, nCol2, nRow2, pClipDoc->pTab[j], bKeepScenarioFlags);

                    if ( pDrawLayer && bIncludeObjects )
                    {
                        //  also copy drawing objects

                        Rectangle aObjRect = GetMMRect( nCol1, nRow1, nCol2, nRow2, j );
                        pDrawLayer->CopyToClip( pClipDoc, j, aObjRect );
                    }
                }

        pClipDoc->bCutMode = bCut;
    }
}


void ScDocument::CopyTabToClip(SCCOL nCol1, SCROW nRow1,
                                SCCOL nCol2, SCROW nRow2,
                                SCTAB nTab, ScDocument* pClipDoc)
{
    if (!bIsClip)
    {
        PutInOrder( nCol1, nCol2 );
        PutInOrder( nRow1, nRow2 );
        if (!pClipDoc)
        {
            DBG_ERROR("CopyTabToClip: no ClipDoc");
            pClipDoc = SC_MOD()->GetClipDoc();
        }

        pClipDoc->aDocName = aDocName;
        pClipDoc->aClipRange = ScRange( nCol1,nRow1,0, nCol2,nRow2,0 );
        pClipDoc->ResetClip( this, nTab );

        if (pTab[nTab] && pClipDoc->pTab[nTab])
            pTab[nTab]->CopyToClip(nCol1, nRow1, nCol2, nRow2, pClipDoc->pTab[nTab], FALSE);

        pClipDoc->bCutMode = FALSE;
    }
}


void ScDocument::TransposeClip( ScDocument* pTransClip, USHORT nFlags, BOOL bAsLink )
{
    DBG_ASSERT( bIsClip && pTransClip && pTransClip->bIsClip,
                    "TransposeClip mit falschem Dokument" );

        //  initialisieren
        //  -> pTransClip muss vor dem Original-Dokument geloescht werden!

    pTransClip->ResetClip(this, (ScMarkData*)NULL);     // alle

        //  Bereiche uebernehmen

    pTransClip->pRangeName->FreeAll();
    for (USHORT i = 0; i < pRangeName->GetCount(); i++)     //! DB-Bereiche Pivot-Bereiche auch !!!
    {
        USHORT nIndex = ((ScRangeData*)((*pRangeName)[i]))->GetIndex();
        ScRangeData* pData = new ScRangeData(*((*pRangeName)[i]));
        if (!pTransClip->pRangeName->Insert(pData))
            delete pData;
        else
            pData->SetIndex(nIndex);
    }

        //  Daten

    if ( ValidRow(aClipRange.aEnd.Row()-aClipRange.aStart.Row()) )
    {
        for (SCTAB i=0; i<=MAXTAB; i++)
            if (pTab[i])
            {
                DBG_ASSERT( pTransClip->pTab[i], "TransposeClip: Tabelle nicht da" );
                pTab[i]->TransposeClip( aClipRange.aStart.Col(), aClipRange.aStart.Row(),
                                            aClipRange.aEnd.Col(), aClipRange.aEnd.Row(),
                                            pTransClip->pTab[i], nFlags, bAsLink );

                if ( pDrawLayer && ( nFlags & IDF_OBJECTS ) )
                {
                    //  Drawing objects are copied to the new area without transposing.
                    //  CopyFromClip is used to adjust the objects to the transposed block's
                    //  cell range area.
                    //  (pDrawLayer in the original clipboard document is set only if there
                    //  are drawing objects to copy)

                    pTransClip->InitDrawLayer();
                    Rectangle aSourceRect = GetMMRect( aClipRange.aStart.Col(), aClipRange.aStart.Row(),
                                                        aClipRange.aEnd.Col(), aClipRange.aEnd.Row(), i );
                    Rectangle aDestRect = pTransClip->GetMMRect( 0, 0,
                            static_cast<SCCOL>(aClipRange.aEnd.Row() - aClipRange.aStart.Row()),
                            static_cast<SCROW>(aClipRange.aEnd.Col() - aClipRange.aStart.Col()), i );
                    pTransClip->pDrawLayer->CopyFromClip( pDrawLayer, i, aSourceRect, ScAddress(0,0,i), aDestRect );
                }
            }

        pTransClip->aClipRange = ScRange( 0, 0, aClipRange.aStart.Tab(),
                static_cast<SCCOL>(aClipRange.aEnd.Row() - aClipRange.aStart.Row()),
                static_cast<SCROW>(aClipRange.aEnd.Col() - aClipRange.aStart.Col()),
                                    aClipRange.aEnd.Tab() );
    }
    else
        DBG_ERROR("TransposeClip: zu gross");

        //  Dies passiert erst beim Einfuegen...

    bCutMode = FALSE;
}


BOOL ScDocument::IsClipboardSource() const
{
    ScDocument* pClipDoc = SC_MOD()->GetClipDoc();
    return pClipDoc && pClipDoc->xPoolHelper.isValid() &&
            xPoolHelper->GetDocPool() == pClipDoc->xPoolHelper->GetDocPool();
}


void ScDocument::StartListeningFromClip( SCCOL nCol1, SCROW nRow1,
                                        SCCOL nCol2, SCROW nRow2,
                                        const ScMarkData& rMark, USHORT nInsFlag )
{
    if (nInsFlag & IDF_CONTENTS)
    {
        for (SCTAB i = 0; i <= MAXTAB; i++)
            if (pTab[i])
                if (rMark.GetTableSelect(i))
                    pTab[i]->StartListeningInArea( nCol1, nRow1, nCol2, nRow2 );
    }
}


void ScDocument::BroadcastFromClip( SCCOL nCol1, SCROW nRow1,
                                    SCCOL nCol2, SCROW nRow2,
                                    const ScMarkData& rMark, USHORT nInsFlag )
{
    if (nInsFlag & IDF_CONTENTS)
    {
        SCTAB nClipTab = 0;
        for (SCTAB i = 0; i <= MAXTAB; i++)
            if (pTab[i])
                if (rMark.GetTableSelect(i))
                    pTab[i]->BroadcastInArea( nCol1, nRow1, nCol2, nRow2 );
    }
}


void ScDocument::CopyBlockFromClip( SCCOL nCol1, SCROW nRow1,
                                    SCCOL nCol2, SCROW nRow2,
                                    const ScMarkData& rMark,
                                    SCsCOL nDx, SCsROW nDy,
                                    const ScCopyBlockFromClipParams* pCBFCP )
{
    ScTable** ppClipTab = pCBFCP->pClipDoc->pTab;
    SCTAB nTabEnd = pCBFCP->nTabEnd;
    SCTAB nClipTab = 0;
    for (SCTAB i = pCBFCP->nTabStart; i <= nTabEnd; i++)
    {
        if (pTab[i] && rMark.GetTableSelect(i) )
        {
            while (!ppClipTab[nClipTab]) nClipTab = (nClipTab+1) % (MAXTAB+1);

            pTab[i]->CopyFromClip( nCol1, nRow1, nCol2, nRow2, nDx, nDy,
                pCBFCP->nInsFlag, pCBFCP->bAsLink, pCBFCP->bSkipAttrForEmpty, ppClipTab[nClipTab] );

            if ( pCBFCP->pClipDoc->pDrawLayer && ( pCBFCP->nInsFlag & IDF_OBJECTS ) )
            {
                //  also copy drawing objects

                // drawing layer must be created before calling CopyFromClip
                // (ScDocShell::MakeDrawLayer also does InitItems etc.)
                DBG_ASSERT( pDrawLayer, "CopyBlockFromClip: No drawing layer" );
                if ( pDrawLayer )
                {
                    //  For GetMMRect, the row heights in the target document must already be valid
                    //  (copied in an extra step before pasting, or updated after pasting cells, but
                    //  before pasting objects).

                    Rectangle aSourceRect = pCBFCP->pClipDoc->GetMMRect(
                                    nCol1-nDx, nRow1-nDy, nCol2-nDx, nRow2-nDy, nClipTab );
                    Rectangle aDestRect = GetMMRect( nCol1, nRow1, nCol2, nRow2, i );
                    pDrawLayer->CopyFromClip( pCBFCP->pClipDoc->pDrawLayer, nClipTab, aSourceRect,
                                                ScAddress( nCol1, nRow1, i ), aDestRect );
                }
            }

            nClipTab = (nClipTab+1) % (MAXTAB+1);
        }
    }
    if ( pCBFCP->nInsFlag & IDF_CONTENTS )
    {
        nClipTab = 0;
        for (SCTAB i = pCBFCP->nTabStart; i <= nTabEnd; i++)
        {
            if (pTab[i] && rMark.GetTableSelect(i) )
            {
                while (!ppClipTab[nClipTab]) nClipTab = (nClipTab+1) % (MAXTAB+1);
                SCsTAB nDz = ((SCsTAB)i) - nClipTab;

                //  #89081# ranges of consecutive selected tables (in clipboard and dest. doc)
                //  must be handled in one UpdateReference call
                SCTAB nFollow = 0;
                while ( i + nFollow < nTabEnd
                        && rMark.GetTableSelect( i + nFollow + 1 )
                        && nClipTab + nFollow < MAXTAB
                        && ppClipTab[nClipTab + nFollow + 1] )
                    ++nFollow;

                if ( pCBFCP->pClipDoc->bCutMode )
                    UpdateReference( URM_MOVE,
                        nCol1, nRow1, i, nCol2, nRow2, i+nFollow,
                        nDx, nDy, nDz, pCBFCP->pRefUndoDoc );
                else
                    UpdateReference( URM_COPY,
                        nCol1, nRow1, i, nCol2, nRow2, i+nFollow,
                        nDx, nDy, nDz, pCBFCP->pRefUndoDoc, FALSE );

                nClipTab = (nClipTab+nFollow+1) % (MAXTAB+1);
                i += nFollow;
            }
        }
    }
}


void ScDocument::CopyNonFilteredFromClip( SCCOL nCol1, SCROW nRow1,
                                    SCCOL nCol2, SCROW nRow2,
                                    const ScMarkData& rMark,
                                    SCsCOL nDx, SCsROW nDy,
                                    const ScCopyBlockFromClipParams* pCBFCP )
{
    //  call CopyBlockFromClip for ranges of consecutive non-filtered rows
    //  nCol1/nRow1 etc. is in target doc

    //  filtered state is taken from first used table in clipboard (as in GetClipArea)
    SCTAB nFlagTab = 0;
    ScTable** ppClipTab = pCBFCP->pClipDoc->pTab;
    while ( nFlagTab < MAXTAB && !ppClipTab[nFlagTab] )
        ++nFlagTab;

    SCROW nSourceRow = pCBFCP->pClipDoc->aClipRange.aStart.Row();
    SCROW nSourceEnd = pCBFCP->pClipDoc->aClipRange.aEnd.Row();
    SCROW nDestRow = nRow1;

    while ( nSourceRow <= nSourceEnd && nDestRow <= nRow2 )
    {
        // skip filtered rows
        while ( nSourceRow <= nSourceEnd &&
                ( pCBFCP->pClipDoc->GetRowFlags( nSourceRow, nFlagTab ) & CR_FILTERED ) != 0 )
            ++nSourceRow;

        if ( nSourceRow <= nSourceEnd )
        {
            // look for more non-filtered rows following
            SCROW nFollow = 0;
            while ( nSourceRow + nFollow < nSourceEnd && nDestRow + nFollow < nRow2 &&
                    ( pCBFCP->pClipDoc->GetRowFlags( nSourceRow + nFollow + 1, nFlagTab ) & CR_FILTERED ) == 0 )
                ++nFollow;

            SCsROW nNewDy = ((SCsROW)nDestRow) - nSourceRow;
            CopyBlockFromClip( nCol1, nDestRow, nCol2, nDestRow + nFollow, rMark, nDx, nNewDy, pCBFCP );

            nSourceRow += nFollow + 1;
            nDestRow += nFollow + 1;
        }
    }
}


void ScDocument::CopyFromClip( const ScRange& rDestRange, const ScMarkData& rMark,
                                USHORT nInsFlag,
                                ScDocument* pRefUndoDoc, ScDocument* pClipDoc, BOOL bResetCut,
                                BOOL bAsLink, BOOL bIncludeFiltered, BOOL bSkipAttrForEmpty )
{
    if (!bIsClip)
    {
        if (!pClipDoc)
        {
            DBG_ERROR("CopyFromClip: no ClipDoc");
            pClipDoc = SC_MOD()->GetClipDoc();
        }
        if (pClipDoc->bIsClip && pClipDoc->GetTableCount())
        {
            BOOL bOldAutoCalc = GetAutoCalc();
            SetAutoCalc( FALSE );   // avoid multiple recalculations

            SvNumberFormatter* pThisFormatter = xPoolHelper->GetFormTable();
            SvNumberFormatter* pOtherFormatter = pClipDoc->xPoolHelper->GetFormTable();
            if (pOtherFormatter && pOtherFormatter != pThisFormatter)
            {
                SvULONGTable* pExchangeList =
                         pThisFormatter->MergeFormatter(*(pOtherFormatter));
                if (pExchangeList->Count() > 0)
                    pFormatExchangeList = pExchangeList;
            }

            USHORT nClipRangeNames = pClipDoc->pRangeName->GetCount();
            // array containing range names which might need update of indices
            ScRangeData** pClipRangeNames = nClipRangeNames ? new ScRangeData* [nClipRangeNames] : NULL;
            // the index mapping thereof
            ScIndexMap aClipRangeMap( nClipRangeNames );
            BOOL bRangeNameReplace = FALSE;

            for (USHORT i = 0; i < nClipRangeNames; i++)        //! DB-Bereiche Pivot-Bereiche auch
            {
                /*  Copy only if the name doesn't exist in this document.
                    If it exists we use the already existing name instead,
                    another possibility could be to create new names if
                    documents differ.
                    A proper solution would ask the user how to proceed.
                    The adjustment of the indices in the formulas is done later.
                */
                ScRangeData* pClipData = (*pClipDoc->pRangeName)[i];
                USHORT k;
                if ( pRangeName->SearchName( pClipData->GetName(), k ) )
                {
                    pClipRangeNames[i] = NULL;  // range name not inserted
                    USHORT nOldIndex = pClipData->GetIndex();
                    USHORT nNewIndex = ((*pRangeName)[k])->GetIndex();
                    aClipRangeMap.SetPair( i, nOldIndex, nNewIndex );
                    if ( !bRangeNameReplace )
                        bRangeNameReplace = ( nOldIndex != nNewIndex );
                }
                else
                {
                    ScRangeData* pData = new ScRangeData( *pClipData );
                    pData->SetDocument(this);
                    if ( pRangeName->FindIndex( pData->GetIndex() ) )
                        pData->SetIndex(0);     // need new index, done in Insert
                    if ( pRangeName->Insert( pData ) )
                    {
                        pClipRangeNames[i] = pData;
                        USHORT nOldIndex = pClipData->GetIndex();
                        USHORT nNewIndex = pData->GetIndex();
                        aClipRangeMap.SetPair( i, nOldIndex, nNewIndex );
                        if ( !bRangeNameReplace )
                            bRangeNameReplace = ( nOldIndex != nNewIndex );
                    }
                    else
                    {   // must be an overflow
                        delete pData;
                        pClipRangeNames[i] = NULL;
                        aClipRangeMap.SetPair( i, pClipData->GetIndex(), 0 );
                        bRangeNameReplace = TRUE;
                    }
                }
            }
            SCCOL nCol1 = rDestRange.aStart.Col();
            SCROW nRow1 = rDestRange.aStart.Row();
            SCCOL nCol2 = rDestRange.aEnd.Col();
            SCROW nRow2 = rDestRange.aEnd.Row();

            SCCOL nXw = pClipDoc->aClipRange.aEnd.Col();
            SCROW nYw = pClipDoc->aClipRange.aEnd.Row();
            pClipDoc->ExtendMerge( pClipDoc->aClipRange.aStart.Col(),
                                    pClipDoc->aClipRange.aStart.Row(),
                                    nXw, nYw, 0 );
            nXw -= pClipDoc->aClipRange.aEnd.Col();
            nYw -= pClipDoc->aClipRange.aEnd.Row();         // only extra value from ExtendMerge
            SCCOL nDestAddX;
            SCROW nDestAddY;
            pClipDoc->GetClipArea( nDestAddX, nDestAddY, bIncludeFiltered );
            nXw += nDestAddX;
            nYw += nDestAddY;                               // ClipArea, plus ExtendMerge value

            //  Inhalte entweder komplett oder gar nicht loeschen:
            USHORT nDelFlag = IDF_NONE;
            if ( nInsFlag & IDF_CONTENTS )
                nDelFlag |= IDF_CONTENTS;
            //  With bSkipAttrForEmpty, don't remove attributes, copy
            //  on top of existing attributes instead.
            if ( ( nInsFlag & IDF_ATTRIB ) && !bSkipAttrForEmpty )
                nDelFlag |= IDF_ATTRIB;
            DeleteArea(nCol1, nRow1, nCol2, nRow2, rMark, nDelFlag);

            bInsertingFromOtherDoc = TRUE;  // kein Broadcast/Listener aufbauen bei Insert
            SCCOL nC1 = nCol1;
            SCROW nR1 = nRow1;
            SCCOL nC2 = nC1 + nXw;
            SCROW nR2 = nR1 + nYw;
            SCCOL nClipStartCol = pClipDoc->aClipRange.aStart.Col();
            SCROW nClipStartRow = pClipDoc->aClipRange.aStart.Row();

            ScCopyBlockFromClipParams aCBFCP;
            aCBFCP.pRefUndoDoc = pRefUndoDoc;
            aCBFCP.pClipDoc = pClipDoc;
            aCBFCP.nInsFlag = nInsFlag;
            aCBFCP.bAsLink  = bAsLink;
            aCBFCP.bSkipAttrForEmpty = bSkipAttrForEmpty;
            aCBFCP.nTabStart = MAXTAB;      // wird in der Schleife angepasst
            aCBFCP.nTabEnd = 0;             // wird in der Schleife angepasst

            //  Inc/DecRecalcLevel einmal aussen, damit nicht fuer jeden Block
            //  die Draw-Seitengroesse neu berechnet werden muss
            //! nur wenn ganze Zeilen/Spalten kopiert werden?

            for (SCTAB j = 0; j <= MAXTAB; j++)
                if (pTab[j] && rMark.GetTableSelect(j))
                {
                    if ( j < aCBFCP.nTabStart )
                        aCBFCP.nTabStart = j;
                    aCBFCP.nTabEnd = j;
                    pTab[j]->IncRecalcLevel();
                }

            // bei mindestens 64 Zeilen wird in ScColumn::CopyFromClip voralloziert
            BOOL bDoDouble = ( nYw < 64 && nRow2 - nRow1 > 64);
            BOOL bOldDouble = ScColumn::bDoubleAlloc;
            if (bDoDouble)
                ScColumn::bDoubleAlloc = TRUE;

            do
            {
                do
                {
                    SCsCOL nDx = ((SCsCOL)nC1) - nClipStartCol;
                    SCsROW nDy = ((SCsROW)nR1) - nClipStartRow;
                    if ( bIncludeFiltered )
                        CopyBlockFromClip( nC1, nR1, nC2, nR2, rMark, nDx, nDy, &aCBFCP );
                    else
                        CopyNonFilteredFromClip( nC1, nR1, nC2, nR2, rMark, nDx, nDy, &aCBFCP );
                    nC1 = nC2 + 1;
                    nC2 = Min((SCCOL)(nC1 + nXw), nCol2);
                }
                while (nC1 <= nCol2);
                nC1 = nCol1;
                nC2 = nC1 + nXw;
                nR1 = nR2 + 1;
                nR2 = Min((SCROW)(nR1 + nYw), nRow2);
            }
            while (nR1 <= nRow2);

            ScColumn::bDoubleAlloc = bOldDouble;

            for (SCTAB k = 0; k <= MAXTAB; k++)
                if (pTab[k] && rMark.GetTableSelect(k))
                    pTab[k]->DecRecalcLevel();

            bInsertingFromOtherDoc = FALSE;
            pFormatExchangeList = NULL;
            if ( bRangeNameReplace )
            {
                // first update all inserted named formulas if they contain other
                // range names and used indices changed
                for (USHORT i = 0; i < nClipRangeNames; i++)        //! DB-Bereiche Pivot-Bereiche auch
                {
                    if ( pClipRangeNames[i] )
                        pClipRangeNames[i]->ReplaceRangeNamesInUse( aClipRangeMap );
                }
                // then update the formulas, they might need the just updated range names
                SCCOL nC1 = nCol1;
                SCROW nR1 = nRow1;
                SCCOL nC2 = nC1 + nXw;
                SCROW nR2 = nR1 + nYw;
                do
                {
                    do
                    {
                        for (SCTAB k = 0; k <= MAXTAB; k++)
                        {
                            if ( pTab[k] && rMark.GetTableSelect(k) )
                                pTab[k]->ReplaceRangeNamesInUse(nC1, nR1,
                                    nC2, nR2, aClipRangeMap );
                        }
                        nC1 = nC2 + 1;
                        nC2 = Min((SCCOL)(nC1 + nXw), nCol2);
                    } while (nC1 <= nCol2);
                    nC1 = nCol1;
                    nC2 = nC1 + nXw;
                    nR1 = nR2 + 1;
                    nR2 = Min((SCROW)(nR1 + nYw), nRow2);
                } while (nR1 <= nRow2);
            }
            if ( pClipRangeNames )
                delete [] pClipRangeNames;
            // Listener aufbauen nachdem alles inserted wurde
            StartListeningFromClip( nCol1, nRow1, nCol2, nRow2, rMark, nInsFlag );
            // nachdem alle Listener aufgebaut wurden, kann gebroadcastet werden
            BroadcastFromClip( nCol1, nRow1, nCol2, nRow2, rMark, nInsFlag );
            if (bResetCut)
                pClipDoc->bCutMode = FALSE;
            SetAutoCalc( bOldAutoCalc );
        }
    }
}


void ScDocument::SetClipArea( const ScRange& rArea, BOOL bCut )
{
    if (bIsClip)
    {
        aClipRange = rArea;
        bCutMode = bCut;
    }
    else
        DBG_ERROR("SetClipArea: kein Clip");
}


void ScDocument::GetClipArea(SCCOL& nClipX, SCROW& nClipY, BOOL bIncludeFiltered)
{
    if (bIsClip)
    {
        nClipX = aClipRange.aEnd.Col() - aClipRange.aStart.Col();

        if ( bIncludeFiltered )
            nClipY = aClipRange.aEnd.Row() - aClipRange.aStart.Row();
        else
        {
            //  count non-filtered rows
            //  count on first used table in clipboard
            SCTAB nCountTab = 0;
            while ( nCountTab < MAXTAB && !pTab[nCountTab] )
                ++nCountTab;

            SCROW nEndRow = aClipRange.aEnd.Row();
            USHORT nResult = 0;
            for (SCROW nRow = aClipRange.aStart.Row(); nRow <= nEndRow; nRow++)
                if ( ( GetRowFlags( nRow, nCountTab ) & CR_FILTERED ) == 0 )
                    ++nResult;

            if ( nResult > 0 )
                nClipY = nResult - 1;
            else
                nClipY = 0;                 // always return at least 1 row
        }
    }
    else
        DBG_ERROR("GetClipArea: kein Clip");
}


void ScDocument::GetClipStart(SCCOL& nClipX, SCROW& nClipY)
{
    if (bIsClip)
    {
        nClipX = aClipRange.aStart.Col();
        nClipY = aClipRange.aStart.Row();
    }
    else
        DBG_ERROR("GetClipStart: kein Clip");
}


BOOL ScDocument::HasClipFilteredRows()
{
    //  count on first used table in clipboard
    SCTAB nCountTab = 0;
    while ( nCountTab < MAXTAB && !pTab[nCountTab] )
        ++nCountTab;

    SCROW nEndRow = aClipRange.aEnd.Row();
    for (SCROW nRow = aClipRange.aStart.Row(); nRow <= nEndRow; nRow++)
        if ( ( GetRowFlags( nRow, nCountTab ) & CR_FILTERED ) != 0 )
            return TRUE;

    return FALSE;
}


void ScDocument::MixDocument( const ScRange& rRange, USHORT nFunction, BOOL bSkipEmpty,
                                    ScDocument* pSrcDoc )
{
    SCTAB nTab1 = rRange.aStart.Tab();
    SCTAB nTab2 = rRange.aEnd.Tab();
    for (SCTAB i = nTab1; i <= nTab2; i++)
        if (pTab[i] && pSrcDoc->pTab[i])
            pTab[i]->MixData( rRange.aStart.Col(), rRange.aStart.Row(),
                                rRange.aEnd.Col(), rRange.aEnd.Row(),
                                nFunction, bSkipEmpty, pSrcDoc->pTab[i] );
}


void ScDocument::FillTab( const ScRange& rSrcArea, const ScMarkData& rMark,
                                USHORT nFlags, USHORT nFunction,
                                BOOL bSkipEmpty, BOOL bAsLink )
{
    USHORT nDelFlags = nFlags;
    if (nDelFlags & IDF_CONTENTS)
        nDelFlags |= IDF_CONTENTS;          // immer alle Inhalte oder keine loeschen!

    SCTAB nSrcTab = rSrcArea.aStart.Tab();

    if (ValidTab(nSrcTab)  && pTab[nSrcTab])
    {
        SCCOL nStartCol = rSrcArea.aStart.Col();
        SCROW nStartRow = rSrcArea.aStart.Row();
        SCCOL nEndCol = rSrcArea.aEnd.Col();
        SCROW nEndRow = rSrcArea.aEnd.Row();
        ScDocument* pMixDoc = NULL;
        BOOL bDoMix = ( bSkipEmpty || nFunction ) && ( nFlags & IDF_CONTENTS );

        BOOL bOldAutoCalc = GetAutoCalc();
        SetAutoCalc( FALSE );                   // Mehrfachberechnungen vermeiden

        SCTAB nCount = GetTableCount();
        for (SCTAB i=0; i<nCount; i++)
            if ( i!=nSrcTab && pTab[i] && rMark.GetTableSelect(i) )
            {
                if (bDoMix)
                {
                    if (!pMixDoc)
                    {
                        pMixDoc = new ScDocument( SCDOCMODE_UNDO );
                        pMixDoc->InitUndo( this, i, i );
                    }
                    else
                        pMixDoc->AddUndoTab( i, i );
                    pTab[i]->CopyToTable( nStartCol,nStartRow, nEndCol,nEndRow,
                                            IDF_CONTENTS, FALSE, pMixDoc->pTab[i] );
                }
                pTab[i]->DeleteArea( nStartCol,nStartRow, nEndCol,nEndRow, nDelFlags);
                pTab[nSrcTab]->CopyToTable( nStartCol,nStartRow, nEndCol,nEndRow,
                                                 nFlags, FALSE, pTab[i], NULL, bAsLink );

                if (bDoMix)
                    pTab[i]->MixData( nStartCol,nStartRow, nEndCol,nEndRow,
                                        nFunction, bSkipEmpty, pMixDoc->pTab[i] );
            }

        delete pMixDoc;

        SetAutoCalc( bOldAutoCalc );
    }
    else
        DBG_ERROR("falsche Tabelle");
}


void ScDocument::FillTabMarked( SCTAB nSrcTab, const ScMarkData& rMark,
                                USHORT nFlags, USHORT nFunction,
                                BOOL bSkipEmpty, BOOL bAsLink )
{
    USHORT nDelFlags = nFlags;
    if (nDelFlags & IDF_CONTENTS)
        nDelFlags |= IDF_CONTENTS;          // immer alle Inhalte oder keine loeschen!

    if (ValidTab(nSrcTab)  && pTab[nSrcTab])
    {
        ScDocument* pMixDoc = NULL;
        BOOL bDoMix = ( bSkipEmpty || nFunction ) && ( nFlags & IDF_CONTENTS );

        BOOL bOldAutoCalc = GetAutoCalc();
        SetAutoCalc( FALSE );                   // Mehrfachberechnungen vermeiden

        ScRange aArea;
        rMark.GetMultiMarkArea( aArea );
        SCCOL nStartCol = aArea.aStart.Col();
        SCROW nStartRow = aArea.aStart.Row();
        SCCOL nEndCol = aArea.aEnd.Col();
        SCROW nEndRow = aArea.aEnd.Row();

        SCTAB nCount = GetTableCount();
        for (SCTAB i=0; i<nCount; i++)
            if ( i!=nSrcTab && pTab[i] && rMark.GetTableSelect(i) )
            {
                if (bDoMix)
                {
                    if (!pMixDoc)
                    {
                        pMixDoc = new ScDocument( SCDOCMODE_UNDO );
                        pMixDoc->InitUndo( this, i, i );
                    }
                    else
                        pMixDoc->AddUndoTab( i, i );
                    pTab[i]->CopyToTable( nStartCol,nStartRow, nEndCol,nEndRow,
                                            IDF_CONTENTS, TRUE, pMixDoc->pTab[i], &rMark );
                }

                pTab[i]->DeleteSelection( nDelFlags, rMark );
                pTab[nSrcTab]->CopyToTable( nStartCol,nStartRow, nEndCol,nEndRow,
                                             nFlags, TRUE, pTab[i], &rMark, bAsLink );

                if (bDoMix)
                    pTab[i]->MixMarked( rMark, nFunction, bSkipEmpty, pMixDoc->pTab[i] );
            }

        delete pMixDoc;

        SetAutoCalc( bOldAutoCalc );
    }
    else
        DBG_ERROR("falsche Tabelle");
}


void ScDocument::PutCell( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell* pCell, BOOL bForceTab )
{
    if (VALIDTAB(nTab))
    {
        if ( bForceTab && !pTab[nTab] )
        {
            BOOL bExtras = !bIsUndo;        // Spaltenbreiten, Zeilenhoehen, Flags

            pTab[nTab] = new ScTable(this, nTab,
                            String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("temp")),
                            bExtras, bExtras);
            ++nMaxTableNumber;
        }

        if (pTab[nTab])
            pTab[nTab]->PutCell( nCol, nRow, pCell );
    }
}


void ScDocument::PutCell( const ScAddress& rPos, ScBaseCell* pCell, BOOL bForceTab )
{
    SCTAB nTab = rPos.Tab();
    if ( bForceTab && !pTab[nTab] )
    {
        BOOL bExtras = !bIsUndo;        // Spaltenbreiten, Zeilenhoehen, Flags

        pTab[nTab] = new ScTable(this, nTab,
                        String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("temp")),
                        bExtras, bExtras);
        ++nMaxTableNumber;
    }

    if (pTab[nTab])
        pTab[nTab]->PutCell( rPos, pCell );
}


BOOL ScDocument::SetString( SCCOL nCol, SCROW nRow, SCTAB nTab, const String& rString )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->SetString( nCol, nRow, nTab, rString );
    else
        return FALSE;
}


void ScDocument::SetValue( SCCOL nCol, SCROW nRow, SCTAB nTab, const double& rVal )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->SetValue( nCol, nRow, rVal );
}


void ScDocument::SetNote( SCCOL nCol, SCROW nRow, SCTAB nTab, const ScPostIt& rNote )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->SetNote( nCol, nRow, rNote );
}


void ScDocument::GetString( SCCOL nCol, SCROW nRow, SCTAB nTab, String& rString )
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
        pTab[nTab]->GetString( nCol, nRow, rString );
    else
        rString.Erase();
}


void ScDocument::GetInputString( SCCOL nCol, SCROW nRow, SCTAB nTab, String& rString )
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
        pTab[nTab]->GetInputString( nCol, nRow, rString );
    else
        rString.Erase();
}


void ScDocument::GetValue( SCCOL nCol, SCROW nRow, SCTAB nTab, double& rValue )
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
        rValue = pTab[nTab]->GetValue( nCol, nRow );
    else
        rValue = 0.0;
}


double ScDocument::GetValue( const ScAddress& rPos )
{
    SCTAB nTab = rPos.Tab();
    if ( pTab[nTab] )
        return pTab[nTab]->GetValue( rPos );
    return 0.0;
}


void ScDocument::GetNumberFormat( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                  ULONG& rFormat )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
        {
            rFormat = pTab[nTab]->GetNumberFormat( nCol, nRow );
            return ;
        }
    rFormat = 0;
}


ULONG ScDocument::GetNumberFormat( const ScAddress& rPos ) const
{
    SCTAB nTab = rPos.Tab();
    if ( pTab[nTab] )
        return pTab[nTab]->GetNumberFormat( rPos );
    return 0;
}


void ScDocument::GetNumberFormatInfo( short& nType, ULONG& nIndex,
            const ScAddress& rPos, const ScFormulaCell& rFCell ) const
{
    SCTAB nTab = rPos.Tab();
    if ( pTab[nTab] )
    {
        nIndex = pTab[nTab]->GetNumberFormat( rPos );
        if ( (nIndex % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
            rFCell.GetFormatInfo( nType, nIndex );
        else
            nType = GetFormatTable()->GetType( nIndex );
    }
    else
    {
        nType = NUMBERFORMAT_UNDEFINED;
        nIndex = 0;
    }
}


void ScDocument::GetFormula( SCCOL nCol, SCROW nRow, SCTAB nTab, String& rFormula,
                             BOOL bAsciiExport ) const
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
            pTab[nTab]->GetFormula( nCol, nRow, rFormula, bAsciiExport );
    else
        rFormula.Erase();
}


BOOL ScDocument::GetNote( SCCOL nCol, SCROW nRow, SCTAB nTab, ScPostIt& rNote )
{
    BOOL bHasNote = FALSE;

    if ( VALIDTAB(nTab) && pTab[nTab] )
        bHasNote = pTab[nTab]->GetNote( nCol, nRow, rNote );
    else
        rNote.Clear();

    return bHasNote;
}


CellType ScDocument::GetCellType( const ScAddress& rPos ) const
{
    SCTAB nTab = rPos.Tab();
    if ( pTab[nTab] )
        return pTab[nTab]->GetCellType( rPos );
    return CELLTYPE_NONE;
}


void ScDocument::GetCellType( SCCOL nCol, SCROW nRow, SCTAB nTab,
        CellType& rCellType ) const
{
    if (ValidTab(nTab) && pTab[nTab])
        rCellType = pTab[nTab]->GetCellType( nCol, nRow );
    else
        rCellType = CELLTYPE_NONE;
}


void ScDocument::GetCell( SCCOL nCol, SCROW nRow, SCTAB nTab,
        ScBaseCell*& rpCell ) const
{
    if (ValidTab(nTab) && pTab[nTab])
        rpCell = pTab[nTab]->GetCell( nCol, nRow );
    else
    {
        DBG_ERROR("GetCell ohne Tabelle");
        rpCell = NULL;
    }
}


ScBaseCell* ScDocument::GetCell( const ScAddress& rPos ) const
{
    SCTAB nTab = rPos.Tab();
    if ( pTab[nTab] )
        return pTab[nTab]->GetCell( rPos );

    DBG_ERROR("GetCell ohne Tabelle");
    return NULL;
}


BOOL ScDocument::HasStringData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
            return pTab[nTab]->HasStringData( nCol, nRow );
    else
        return FALSE;
}


BOOL ScDocument::HasValueData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
            return pTab[nTab]->HasValueData( nCol, nRow );
    else
        return FALSE;
}


BOOL ScDocument::HasStringCells( const ScRange& rRange ) const
{
    //  TRUE, wenn String- oder Editzellen im Bereich

    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    for ( SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++ )
        if ( pTab[nTab] && pTab[nTab]->HasStringCells( nStartCol, nStartRow, nEndCol, nEndRow ) )
            return TRUE;

    return FALSE;
}


BOOL ScDocument::HasSelectionData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    sal_uInt32 nValidation = static_cast< const SfxUInt32Item* >( GetAttr( nCol, nRow, nTab, ATTR_VALIDDATA ) )->GetValue();
    if( nValidation )
    {
        const ScValidationData* pData = GetValidationEntry( nValidation );
        if( pData && pData->HasSelectionList() )
            return TRUE;
    }
    return HasStringCells( ScRange( nCol, 0, nTab, nCol, MAXROW, nTab ) );
}


void ScDocument::SetDirtyVar()
{
    for (SCTAB i=0; i<=MAXTAB; i++)
        if (pTab[i]) pTab[i]->SetDirtyVar();
}


void ScDocument::SetDirty()
{
    BOOL bOldAutoCalc = GetAutoCalc();
    bAutoCalc = FALSE;      // keine Mehrfachberechnung
    for (SCTAB i=0; i<=MAXTAB; i++)
        if (pTab[i]) pTab[i]->SetDirty();

    //  Charts werden zwar auch ohne AutoCalc im Tracking auf Dirty gesetzt,
    //  wenn alle Formeln dirty sind, werden die Charts aber nicht mehr erwischt
    //  (#45205#) - darum alle Charts nochmal explizit
    if (pChartListenerCollection)
        pChartListenerCollection->SetDirty();

    SetAutoCalc( bOldAutoCalc );
}


void ScDocument::SetDirty( const ScRange& rRange )
{
    BOOL bOldAutoCalc = GetAutoCalc();
    bAutoCalc = FALSE;      // keine Mehrfachberechnung
    SCTAB nTab2 = rRange.aEnd.Tab();
    for (SCTAB i=rRange.aStart.Tab(); i<=nTab2; i++)
        if (pTab[i]) pTab[i]->SetDirty( rRange );
    SetAutoCalc( bOldAutoCalc );
}


void ScDocument::SetTableOpDirty( const ScRange& rRange )
{
    BOOL bOldAutoCalc = GetAutoCalc();
    bAutoCalc = FALSE;      // no multiple recalculation
    SCTAB nTab2 = rRange.aEnd.Tab();
    for (SCTAB i=rRange.aStart.Tab(); i<=nTab2; i++)
        if (pTab[i]) pTab[i]->SetTableOpDirty( rRange );
    SetAutoCalc( bOldAutoCalc );
}


void ScDocument::AddTableOpFormulaCell( ScFormulaCell* pCell )
{
    ScInterpreterTableOpParams* p = aTableOpList.Last();
    if ( p && p->bCollectNotifications )
    {
        if ( p->bRefresh )
        {   // refresh pointers only
            p->aNotifiedFormulaCells.push_back( pCell );
        }
        else
        {   // init both, address and pointer
            p->aNotifiedFormulaCells.push_back( pCell );
            p->aNotifiedFormulaPos.push_back( pCell->aPos );
        }
    }
}


void ScDocument::CalcAll()
{
    BOOL bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( TRUE );
    SCTAB i;
    for (i=0; i<=MAXTAB; i++)
        if (pTab[i]) pTab[i]->SetDirtyVar();
    for (i=0; i<=MAXTAB; i++)
        if (pTab[i]) pTab[i]->CalcAll();
    ClearFormulaTree();
    SetAutoCalc( bOldAutoCalc );
}


void ScDocument::CompileAll()
{
    if ( pCondFormList )
        pCondFormList->CompileAll();

    for (SCTAB i=0; i<=MAXTAB; i++)
        if (pTab[i]) pTab[i]->CompileAll();
    SetDirty();
}


void ScDocument::CompileXML()
{
    BOOL bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( FALSE );
    ScProgress aProgress( GetDocumentShell(), ScGlobal::GetRscString(
                STR_PROGRESS_CALCULATING ), GetXMLImportedFormulaCount() );

    for (SCTAB i=0; i<=MAXTAB; i++)
        if (pTab[i]) pTab[i]->CompileXML( aProgress );

    if ( pCondFormList )
        pCondFormList->CompileXML();
    if ( pValidationList )
        pValidationList->CompileXML();

    SetDirty();
    SetAutoCalc( bOldAutoCalc );
}


void ScDocument::CalcAfterLoad()
{
    SCTAB i;

    if (bIsClip)    // Excel-Dateien werden aus dem Clipboard in ein Clip-Doc geladen
        return;     // dann wird erst beim Einfuegen in das richtige Doc berechnet

    bCalcingAfterLoad = TRUE;
    for ( i = 0; i <= MAXTAB; i++)
        if (pTab[i]) pTab[i]->CalcAfterLoad();
    for (i=0; i<=MAXTAB; i++)
        if (pTab[i]) pTab[i]->SetDirtyAfterLoad();
    bCalcingAfterLoad = FALSE;

    SetDetectiveDirty(FALSE);   // noch keine wirklichen Aenderungen
}


void ScDocument::GetErrCode( SCCOL nCol, SCROW nRow, SCTAB nTab, USHORT& rErrCode )
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
        rErrCode = pTab[nTab]->GetErrCode( nCol, nRow );
    else
        rErrCode = 0;
}


USHORT ScDocument::GetErrCode( const ScAddress& rPos ) const
{
    SCTAB nTab = rPos.Tab();
    if ( pTab[nTab] )
        return pTab[nTab]->GetErrCode( rPos );
    return 0;
}


void ScDocument::ResetChanged( const ScRange& rRange )
{
    SCTAB nStartTab = rRange.aStart.Tab();
    SCTAB nEndTab = rRange.aEnd.Tab();
    for (SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++)
        if (pTab[nTab])
            pTab[nTab]->ResetChanged( rRange );
}

//
//  Spaltenbreiten / Zeilenhoehen   --------------------------------------
//


void ScDocument::SetColWidth( SCCOL nCol, SCTAB nTab, USHORT nNewWidth )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->SetColWidth( nCol, nNewWidth );
}


void ScDocument::SetRowHeight( SCROW nRow, SCTAB nTab, USHORT nNewHeight )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->SetRowHeight( nRow, nNewHeight );
}


void ScDocument::SetRowHeightRange( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, USHORT nNewHeight )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->SetRowHeightRange
            ( nStartRow, nEndRow, nNewHeight, 1.0, 1.0 );
}


void ScDocument::SetManualHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, BOOL bManual )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->SetManualHeight( nStartRow, nEndRow, bManual );
}


USHORT ScDocument::GetColWidth( SCCOL nCol, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetColWidth( nCol );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}


USHORT ScDocument::GetOriginalWidth( SCCOL nCol, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetOriginalWidth( nCol );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}


USHORT ScDocument::GetCommonWidth( SCCOL nEndCol, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetCommonWidth( nEndCol );
    DBG_ERROR("Wrong table number");
    return 0;
}


USHORT ScDocument::GetOriginalHeight( SCROW nRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetOriginalHeight( nRow );
    DBG_ERROR("Wrong table number");
    return 0;
}


USHORT ScDocument::GetRowHeight( SCROW nRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetRowHeight( nRow );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}


SCROW ScDocument::GetHiddenRowCount( SCROW nRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetHiddenRowCount( nRow );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}


ULONG ScDocument::GetColOffset( SCCOL nCol, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetColOffset( nCol );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}


ULONG ScDocument::GetRowOffset( SCROW nRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetRowOffset( nRow );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}


USHORT ScDocument::GetOptimalColWidth( SCCOL nCol, SCTAB nTab, OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY,
                                        BOOL bFormula, const ScMarkData* pMarkData,
                                        BOOL bSimpleTextImport )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetOptimalColWidth( nCol, pDev, nPPTX, nPPTY,
            rZoomX, rZoomY, bFormula, pMarkData, bSimpleTextImport );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}


long ScDocument::GetNeededSize( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bWidth, BOOL bTotalSize )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetNeededSize
                ( nCol, nRow, pDev, nPPTX, nPPTY, rZoomX, rZoomY, bWidth, bTotalSize );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}


BOOL ScDocument::SetOptimalHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, USHORT nExtra,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bShrink )
{
//! MarkToMulti();
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->SetOptimalHeight( nStartRow, nEndRow, nExtra,
                                                pDev, nPPTX, nPPTY, rZoomX, rZoomY, bShrink );
    DBG_ERROR("Falsche Tabellennummer");
    return FALSE;
}


//
//  Spalten-/Zeilen-Flags   ----------------------------------------------
//

void ScDocument::ShowCol(SCCOL nCol, SCTAB nTab, BOOL bShow)
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->ShowCol( nCol, bShow );
}


void ScDocument::ShowRow(SCROW nRow, SCTAB nTab, BOOL bShow)
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->ShowRow( nRow, bShow );
}


void ScDocument::ShowRows(SCROW nRow1, SCROW nRow2, SCTAB nTab, BOOL bShow)
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->ShowRows( nRow1, nRow2, bShow );
}


void ScDocument::SetColFlags( SCCOL nCol, SCTAB nTab, BYTE nNewFlags )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->SetColFlags( nCol, nNewFlags );
}


void ScDocument::SetRowFlags( SCROW nRow, SCTAB nTab, BYTE nNewFlags )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->SetRowFlags( nRow, nNewFlags );
}


BYTE ScDocument::GetColFlags( SCCOL nCol, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetColFlags( nCol );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}

BYTE ScDocument::GetRowFlags( SCROW nRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetRowFlags( nRow );
    DBG_ERROR("Falsche Tabellennummer");
    return 0;
}


SCCOL ScDocument::GetLastFlaggedCol( SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetLastFlaggedCol();
    return 0;
}

SCROW ScDocument::GetLastFlaggedRow( SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetLastFlaggedRow();
    return 0;
}


SCCOL ScDocument::GetLastChangedCol( SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetLastChangedCol();
    return 0;
}

SCROW ScDocument::GetLastChangedRow( SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetLastChangedRow();
    return 0;
}


SCCOL ScDocument::GetNextDifferentChangedCol( SCTAB nTab, SCCOL nStart) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
    {
        BYTE nStartFlags = pTab[nTab]->GetColFlags(nStart);
        USHORT nStartWidth = pTab[nTab]->GetOriginalWidth(nStart);
        for (SCCOL nCol = nStart + 1; nCol <= MAXCOL; nCol++)
        {
            if (((nStartFlags & CR_MANUALBREAK) != (pTab[nTab]->GetColFlags(nCol) & CR_MANUALBREAK)) ||
                (nStartWidth != pTab[nTab]->GetOriginalWidth(nCol)) ||
                ((nStartFlags & CR_HIDDEN) != (pTab[nTab]->GetColFlags(nCol) & CR_HIDDEN)) )
                return nCol;
        }
        return MAXCOL;
    }
    return 0;
}

SCROW ScDocument::GetNextDifferentChangedRow( SCTAB nTab, SCROW nStart, bool bCareManualSize) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
    {
        BYTE nStartFlags = pTab[nTab]->GetRowFlags(nStart);
        USHORT nStartHeight = pTab[nTab]->GetOriginalHeight(nStart);
        for (SCROW nRow = nStart + 1; nRow <= MAXROW; nRow++)
        {
            if (((nStartFlags & CR_MANUALBREAK) != (pTab[nTab]->GetRowFlags(nRow) & CR_MANUALBREAK)) ||
                ((nStartFlags & CR_MANUALSIZE) != (pTab[nTab]->GetRowFlags(nRow) & CR_MANUALSIZE)) ||
                (bCareManualSize && (nStartFlags & CR_MANUALSIZE) && (nStartHeight != pTab[nTab]->GetOriginalHeight(nRow))) ||
                (!bCareManualSize && ((nStartHeight != pTab[nTab]->GetOriginalHeight(nRow)))))
                return nRow;
        }
        return MAXROW;
    }
    return 0;
}

BOOL ScDocument::GetColDefault( SCTAB nTab, SCCOL nCol, SCROW nLastRow, SCROW& nDefault)
{
    BOOL bRet(FALSE);
    nDefault = 0;
    ScDocAttrIterator aDocAttrItr(this, nTab, nCol, 0, nCol, nLastRow);
    SCCOL nColumn;
    SCROW nStartRow;
    SCROW nEndRow;
    const ScPatternAttr* pAttr = aDocAttrItr.GetNext(nColumn, nStartRow, nEndRow);
    if (nEndRow < nLastRow)
    {
        ScDefaultAttrSet aSet;
        ScDefaultAttrSet::iterator aItr = aSet.end();
        while (pAttr)
        {
            ScDefaultAttr aAttr(pAttr);
            aItr = aSet.find(aAttr);
            if (aItr == aSet.end())
            {
                aAttr.nCount = static_cast<SCSIZE>(nEndRow - nStartRow + 1);
                aAttr.nFirst = nStartRow;
                aSet.insert(aAttr);
            }
            else
            {
                aAttr.nCount = aItr->nCount + static_cast<SCSIZE>(nEndRow - nStartRow + 1);
                aAttr.nFirst = aItr->nFirst;
                aSet.erase(aItr);
                aSet.insert(aAttr);
            }
            pAttr = aDocAttrItr.GetNext(nColumn, nStartRow, nEndRow);
        }
        ScDefaultAttrSet::iterator aDefaultItr = aSet.begin();
        aItr = aDefaultItr;
        aItr++;
        while (aItr != aSet.end())
        {
            if (aItr->nCount > aDefaultItr->nCount)
                aDefaultItr = aItr;
            aItr++;
        }
        nDefault = aDefaultItr->nFirst;
        bRet = TRUE;
    }
    else
        bRet = TRUE;
    return bRet;
}

BOOL ScDocument::GetRowDefault( SCTAB nTab, SCROW nRow, SCCOL nLastCol, SCCOL& nDefault)
{
    BOOL bRet(FALSE);
    return bRet;
}

void ScDocument::StripHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2, SCTAB nTab )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->StripHidden( rX1, rY1, rX2, rY2 );
}


void ScDocument::ExtendHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2, SCTAB nTab )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->ExtendHidden( rX1, rY1, rX2, rY2 );
}

//
//  Attribute   ----------------------------------------------------------
//

const SfxPoolItem* ScDocument::GetAttr( SCCOL nCol, SCROW nRow, SCTAB nTab, USHORT nWhich ) const
{
    if ( ValidTab(nTab)  && pTab[nTab] )
    {
        const SfxPoolItem* pTemp = pTab[nTab]->GetAttr( nCol, nRow, nWhich );
        if (pTemp)
            return pTemp;
        else
            DBG_ERROR( "Attribut Null" );
    }
    return &xPoolHelper->GetDocPool()->GetDefaultItem( nWhich );
}


const ScPatternAttr* ScDocument::GetPattern( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        return pTab[nTab]->GetPattern( nCol, nRow );
    return NULL;
}


void ScDocument::ApplyAttr( SCCOL nCol, SCROW nRow, SCTAB nTab, const SfxPoolItem& rAttr )
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        pTab[nTab]->ApplyAttr( nCol, nRow, rAttr );
}


void ScDocument::ApplyPattern( SCCOL nCol, SCROW nRow, SCTAB nTab, const ScPatternAttr& rAttr )
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        pTab[nTab]->ApplyPattern( nCol, nRow, rAttr );
}


void ScDocument::ApplyPatternArea( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark,
                        const ScPatternAttr& rAttr )
{
    for (SCTAB i=0; i <= MAXTAB; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                pTab[i]->ApplyPatternArea( nStartCol, nStartRow, nEndCol, nEndRow, rAttr );
}


void ScDocument::ApplyPatternAreaTab( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow, SCTAB nTab, const ScPatternAttr& rAttr )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->ApplyPatternArea( nStartCol, nStartRow, nEndCol, nEndRow, rAttr );
}

void ScDocument::ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
        const ScMarkData& rMark, const ScPatternAttr& rPattern, short nNewType )
{
    for (SCTAB i=0; i <= MAXTAB; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                pTab[i]->ApplyPatternIfNumberformatIncompatible( rRange, rPattern, nNewType );
}


void ScDocument::ApplyStyle( SCCOL nCol, SCROW nRow, SCTAB nTab, const ScStyleSheet& rStyle)
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->ApplyStyle( nCol, nRow, rStyle );
}


void ScDocument::ApplyStyleArea( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark,
                        const ScStyleSheet& rStyle)
{
    for (SCTAB i=0; i <= MAXTAB; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                pTab[i]->ApplyStyleArea( nStartCol, nStartRow, nEndCol, nEndRow, rStyle );
}


void ScDocument::ApplyStyleAreaTab( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow, SCTAB nTab, const ScStyleSheet& rStyle)
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->ApplyStyleArea( nStartCol, nStartRow, nEndCol, nEndRow, rStyle );
}


void ScDocument::ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark)
{
    // ApplySelectionStyle needs multi mark
    if ( rMark.IsMarked() && !rMark.IsMultiMarked() )
    {
        ScRange aRange;
        rMark.GetMarkArea( aRange );
        ApplyStyleArea( aRange.aStart.Col(), aRange.aStart.Row(),
                          aRange.aEnd.Col(), aRange.aEnd.Row(), rMark, rStyle );
    }
    else
    {
        for (SCTAB i=0; i<=MAXTAB; i++)
            if ( pTab[i] && rMark.GetTableSelect(i) )
                    pTab[i]->ApplySelectionStyle( rStyle, rMark );
    }
}


void ScDocument::ApplySelectionLineStyle( const ScMarkData& rMark,
                    const SvxBorderLine* pLine, BOOL bColorOnly )
{
    if ( bColorOnly && !pLine )
        return;

    for (SCTAB i=0; i<=MAXTAB; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                pTab[i]->ApplySelectionLineStyle( rMark, pLine, bColorOnly );
}


const ScStyleSheet* ScDocument::GetStyle( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
        return pTab[nTab]->GetStyle(nCol, nRow);
    else
        return NULL;
}


const ScStyleSheet* ScDocument::GetSelectionStyle( const ScMarkData& rMark ) const
{
    BOOL    bEqual = TRUE;
    BOOL    bFound;

    const ScStyleSheet* pStyle = NULL;
    const ScStyleSheet* pNewStyle;

    if ( rMark.IsMultiMarked() )
        for (SCTAB i=0; i<=MAXTAB && bEqual; i++)
            if (pTab[i] && rMark.GetTableSelect(i))
            {
                pNewStyle = pTab[i]->GetSelectionStyle( rMark, bFound );
                if (bFound)
                {
                    if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
                        bEqual = FALSE;                                             // unterschiedliche
                    pStyle = pNewStyle;
                }
            }
    if ( rMark.IsMarked() )
    {
        ScRange aRange;
        rMark.GetMarkArea( aRange );
        for (SCTAB i=aRange.aStart.Tab(); i<=aRange.aEnd.Tab() && bEqual; i++)
            if (pTab[i] && rMark.GetTableSelect(i))
            {
                pNewStyle = pTab[i]->GetAreaStyle( bFound,
                                        aRange.aStart.Col(), aRange.aStart.Row(),
                                        aRange.aEnd.Col(),   aRange.aEnd.Row()   );
                if (bFound)
                {
                    if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
                        bEqual = FALSE;                                             // unterschiedliche
                    pStyle = pNewStyle;
                }
            }
    }

    return bEqual ? pStyle : NULL;
}


void ScDocument::StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, BOOL bRemoved,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY )
{
    for (SCTAB i=0; i <= MAXTAB; i++)
        if (pTab[i])
            pTab[i]->StyleSheetChanged
                ( pStyleSheet, bRemoved, pDev, nPPTX, nPPTY, rZoomX, rZoomY );

    if ( pStyleSheet && pStyleSheet->GetName() == ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
    {
        //  update attributes for all note objects

        ScDetectiveFunc aFunc( this, 0 );
        aFunc.UpdateAllComments();
    }
}


BOOL ScDocument::IsStyleSheetUsed( const ScStyleSheet& rStyle, BOOL bGatherAllStyles ) const
{
    if ( bStyleSheetUsageInvalid || rStyle.GetUsage() == ScStyleSheet::UNKNOWN )
    {
        if ( bGatherAllStyles )
        {
            SfxStyleSheetIterator aIter( xPoolHelper->GetStylePool(),
                    SFX_STYLE_FAMILY_PARA );
            for ( const SfxStyleSheetBase* pStyle = aIter.First(); pStyle;
                                           pStyle = aIter.Next() )
            {
                const ScStyleSheet* pScStyle = PTR_CAST( ScStyleSheet, pStyle );
                if ( pScStyle )
                    pScStyle->SetUsage( ScStyleSheet::NOTUSED );
            }
        }

        BOOL bIsUsed = FALSE;

        for ( SCTAB i=0; i<=MAXTAB; i++ )
        {
            if ( pTab[i] )
            {
                if ( pTab[i]->IsStyleSheetUsed( rStyle, bGatherAllStyles ) )
                {
                    if ( !bGatherAllStyles )
                        return TRUE;
                    bIsUsed = TRUE;
                }
            }
        }

        if ( bGatherAllStyles )
            bStyleSheetUsageInvalid = FALSE;

        return bIsUsed;
    }

    return rStyle.GetUsage() == ScStyleSheet::USED;
}



BOOL ScDocument::ApplyFlags( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark,
                        INT16 nFlags )
{
    BOOL bChanged = FALSE;
    for (SCTAB i=0; i <= MAXTAB; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                bChanged |= pTab[i]->ApplyFlags( nStartCol, nStartRow, nEndCol, nEndRow, nFlags );
    return bChanged;
}


BOOL ScDocument::ApplyFlagsTab( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow, SCTAB nTab, INT16 nFlags )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->ApplyFlags( nStartCol, nStartRow, nEndCol, nEndRow, nFlags );

    DBG_ERROR("ApplyFlags: falsche Tabelle");
    return FALSE;
}


BOOL ScDocument::RemoveFlags( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark,
                        INT16 nFlags )
{
    BOOL bChanged = FALSE;
    for (SCTAB i=0; i <= MAXTAB; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                bChanged |= pTab[i]->RemoveFlags( nStartCol, nStartRow, nEndCol, nEndRow, nFlags );
    return bChanged;
}


BOOL ScDocument::RemoveFlagsTab( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow, SCTAB nTab, INT16 nFlags )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->RemoveFlags( nStartCol, nStartRow, nEndCol, nEndRow, nFlags );

    DBG_ERROR("RemoveFlags: falsche Tabelle");
    return FALSE;
}


void ScDocument::SetPattern( SCCOL nCol, SCROW nRow, SCTAB nTab, const ScPatternAttr& rAttr,
                                BOOL bPutToPool )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->SetPattern( nCol, nRow, rAttr, bPutToPool );
}


void ScDocument::SetPattern( const ScAddress& rPos, const ScPatternAttr& rAttr,
                                BOOL bPutToPool )
{
    SCTAB nTab = rPos.Tab();
    if (pTab[nTab])
        pTab[nTab]->SetPattern( rPos, rAttr, bPutToPool );
}


ScPatternAttr* ScDocument::CreateSelectionPattern( const ScMarkData& rMark, BOOL bDeep )
{
    SfxItemSet* pSet = NULL;

    if ( rMark.IsMultiMarked() )                                // multi selection
    {
        for (SCTAB i=0; i<=MAXTAB; i++)
            if (pTab[i] && rMark.GetTableSelect(i))
                pTab[i]->MergeSelectionPattern( &pSet, rMark, bDeep );
    }
    if ( rMark.IsMarked() )                                     // simle selection
    {
        ScRange aRange;
        rMark.GetMarkArea(aRange);
        for (SCTAB i=0; i<=MAXTAB; i++)
            if (pTab[i] && rMark.GetTableSelect(i))
                pTab[i]->MergePatternArea( &pSet,
                                aRange.aStart.Col(), aRange.aStart.Row(),
                                aRange.aEnd.Col(), aRange.aEnd.Row(), bDeep );
    }

    DBG_ASSERT( pSet, "SelectionPattern Null" );
    if (pSet)
        return new ScPatternAttr( pSet );
    else
        return new ScPatternAttr( GetPool() );      // empty
}


const ScPatternAttr* ScDocument::GetSelectionPattern( const ScMarkData& rMark, BOOL bDeep )
{
    delete pSelectionAttr;
    pSelectionAttr = CreateSelectionPattern( rMark, bDeep );
    return pSelectionAttr;
}


void ScDocument::GetSelectionFrame( const ScMarkData& rMark,
                                    SvxBoxItem&     rLineOuter,
                                    SvxBoxInfoItem& rLineInner )
{
    rLineOuter.SetLine(NULL, BOX_LINE_TOP);
    rLineOuter.SetLine(NULL, BOX_LINE_BOTTOM);
    rLineOuter.SetLine(NULL, BOX_LINE_LEFT);
    rLineOuter.SetLine(NULL, BOX_LINE_RIGHT);
    rLineOuter.SetDistance(0);

    rLineInner.SetLine(NULL, BOXINFO_LINE_HORI);
    rLineInner.SetLine(NULL, BOXINFO_LINE_VERT);
    rLineInner.SetTable(TRUE);
    rLineInner.SetDist(TRUE);
    rLineInner.SetMinDist(FALSE);

    ScLineFlags aFlags;

    if (rMark.IsMarked())
    {
        ScRange aRange;
        rMark.GetMarkArea(aRange);
        rLineInner.EnableHor( aRange.aStart.Row() != aRange.aEnd.Row() );
        rLineInner.EnableVer( aRange.aStart.Col() != aRange.aEnd.Col() );
        for (SCTAB i=0; i<=MAXTAB; i++)
            if (pTab[i] && rMark.GetTableSelect(i))
                pTab[i]->MergeBlockFrame( &rLineOuter, &rLineInner, aFlags,
                                          aRange.aStart.Col(), aRange.aStart.Row(),
                                          aRange.aEnd.Col(),   aRange.aEnd.Row() );
    }

        //  Don't care Status auswerten

    rLineInner.SetValid( VALID_LEFT,   ( aFlags.nLeft != SC_LINE_DONTCARE ) );
    rLineInner.SetValid( VALID_RIGHT,  ( aFlags.nRight != SC_LINE_DONTCARE ) );
    rLineInner.SetValid( VALID_TOP,    ( aFlags.nTop != SC_LINE_DONTCARE ) );
    rLineInner.SetValid( VALID_BOTTOM, ( aFlags.nBottom != SC_LINE_DONTCARE ) );
    rLineInner.SetValid( VALID_HORI,   ( aFlags.nHori != SC_LINE_DONTCARE ) );
    rLineInner.SetValid( VALID_VERT,   ( aFlags.nVert != SC_LINE_DONTCARE ) );
}


BOOL ScDocument::HasAttrib( SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                            SCCOL nCol2, SCROW nRow2, SCTAB nTab2, USHORT nMask )
{
    if ( nMask & HASATTR_ROTATE )
    {
        //  Attribut im Dokument ueberhaupt verwendet?
        //  (wie in fillinfo)

        ScDocumentPool* pPool = xPoolHelper->GetDocPool();

        BOOL bAnyItem = FALSE;
        USHORT nRotCount = pPool->GetItemCount( ATTR_ROTATE_VALUE );
        for (USHORT nItem=0; nItem<nRotCount; nItem++)
            if (pPool->GetItem( ATTR_ROTATE_VALUE, nItem ))
            {
                bAnyItem = TRUE;
                break;
            }
        if (!bAnyItem)
            nMask &= ~HASATTR_ROTATE;
    }

    if ( nMask & HASATTR_RTL )
    {
        //  first check if right-to left is in the pool at all
        //  (the same item is used in cell and page format)

        ScDocumentPool* pPool = xPoolHelper->GetDocPool();

        BOOL bHasRtl = FALSE;
        USHORT nDirCount = pPool->GetItemCount( ATTR_WRITINGDIR );
        for (USHORT nItem=0; nItem<nDirCount; nItem++)
        {
            const SfxPoolItem* pItem = pPool->GetItem( ATTR_WRITINGDIR, nItem );
            if ( pItem && ((const SvxFrameDirectionItem*)pItem)->GetValue() == FRMDIR_HORI_RIGHT_TOP )
            {
                bHasRtl = TRUE;
                break;
            }
        }
        if (!bHasRtl)
            nMask &= ~HASATTR_RTL;
    }

    if (!nMask)
        return FALSE;

    BOOL bFound = FALSE;
    for (SCTAB i=nTab1; i<=nTab2 && !bFound; i++)
        if (pTab[i])
        {
            if ( nMask & HASATTR_RTL )
            {
                if ( GetEditTextDirection(i) == EE_HTEXTDIR_R2L )       // sheet default
                    bFound = TRUE;
            }
            if ( nMask & HASATTR_RIGHTORCENTER )
            {
                //  On a RTL sheet, don't start to look for the default left value
                //  (which is then logically right), instead always assume TRUE.
                //  That way, ScAttrArray::HasAttrib doesn't have to handle RTL sheets.

                if ( IsLayoutRTL(i) )
                    bFound = TRUE;
            }

            if ( !bFound )
                bFound = pTab[i]->HasAttrib( nCol1, nRow1, nCol2, nRow2, nMask );
        }

    return bFound;
}

BOOL ScDocument::HasAttrib( const ScRange& rRange, USHORT nMask )
{
    return HasAttrib( rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
                      rRange.aEnd.Col(),   rRange.aEnd.Row(),   rRange.aEnd.Tab(),
                      nMask );
}

void ScDocument::FindMaxRotCol( SCTAB nTab, RowInfo* pRowInfo, SCSIZE nArrCount,
                                SCCOL nX1, SCCOL nX2 ) const
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        pTab[nTab]->FindMaxRotCol( pRowInfo, nArrCount, nX1, nX2 );
    else
        DBG_ERRORFILE("FindMaxRotCol: falsche Tabelle");
}

BOOL ScDocument::HasLines( const ScRange& rRange, Rectangle& rSizes ) const
{
    SCTAB nTab1 = rRange.aStart.Tab();
    SCTAB nTab2 = rRange.aEnd.Tab();
    PutInOrder( nTab1, nTab2 );
    BOOL bFound = FALSE;
    rSizes = Rectangle(0,0,0,0);

    for (SCTAB i=nTab1; i<=nTab2; i++)
        if (pTab[i])
            if (pTab[i]->HasLines( rRange, rSizes ))
                bFound = TRUE;

    return bFound;
}

void ScDocument::GetBorderLines( SCCOL nCol, SCROW nRow, SCTAB nTab,
                        const SvxBorderLine** ppLeft, const SvxBorderLine** ppTop,
                        const SvxBorderLine** ppRight, const SvxBorderLine** ppBottom ) const
{
    //! Seitengrenzen fuer Druck beruecksichtigen !!!!!

    const SvxBoxItem* pThisAttr = (const SvxBoxItem*) GetEffItem( nCol, nRow, nTab, ATTR_BORDER );
    DBG_ASSERT(pThisAttr,"wo ist das Attribut?");

    const SvxBorderLine* pLeftLine   = pThisAttr->GetLeft();
    const SvxBorderLine* pTopLine    = pThisAttr->GetTop();
    const SvxBorderLine* pRightLine  = pThisAttr->GetRight();
    const SvxBorderLine* pBottomLine = pThisAttr->GetBottom();

    if ( nCol > 0 )
    {
        const SvxBorderLine* pOther = ((const SvxBoxItem*)
                                GetEffItem( nCol-1, nRow, nTab, ATTR_BORDER ))->GetRight();
        if ( HasPriority( pOther, pLeftLine ) )
            pLeftLine = pOther;
    }
    if ( nRow > 0 )
    {
        const SvxBorderLine* pOther = ((const SvxBoxItem*)
                                GetEffItem( nCol, nRow-1, nTab, ATTR_BORDER ))->GetBottom();
        if ( HasPriority( pOther, pTopLine ) )
            pTopLine = pOther;
    }
    if ( nCol < MAXCOL )
    {
        const SvxBorderLine* pOther = ((const SvxBoxItem*)
                                GetEffItem( nCol+1, nRow, nTab, ATTR_BORDER ))->GetLeft();
        if ( HasPriority( pOther, pRightLine ) )
            pRightLine = pOther;
    }
    if ( nRow < MAXROW )
    {
        const SvxBorderLine* pOther = ((const SvxBoxItem*)
                                GetEffItem( nCol, nRow+1, nTab, ATTR_BORDER ))->GetTop();
        if ( HasPriority( pOther, pBottomLine ) )
            pBottomLine = pOther;
    }

    if (ppLeft)
        *ppLeft = pLeftLine;
    if (ppTop)
        *ppTop = pTopLine;
    if (ppRight)
        *ppRight = pRightLine;
    if (ppBottom)
        *ppBottom = pBottomLine;
}

BOOL ScDocument::IsBlockEmpty( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->IsBlockEmpty( nStartCol, nStartRow, nEndCol, nEndRow );

    DBG_ERROR("Falsche Tabellennummer");
    return FALSE;
}


void ScDocument::LockTable(SCTAB nTab)
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        pTab[nTab]->LockTable();
    else
        DBG_ERROR("Falsche Tabellennummer");
}


void ScDocument::UnlockTable(SCTAB nTab)
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        pTab[nTab]->UnlockTable();
    else
        DBG_ERROR("Falsche Tabellennummer");
}


BOOL ScDocument::IsBlockEditable( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow,
                                        BOOL* pOnlyNotBecauseOfMatrix /* = NULL */ ) const
{
    // import into read-only document is possible - must be extended if other filters use api
    if ( pShell && pShell->IsReadOnly() && !bImportingXML )
    {
        if ( pOnlyNotBecauseOfMatrix )
            *pOnlyNotBecauseOfMatrix = FALSE;
        return FALSE;
    }

    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->IsBlockEditable( nStartCol, nStartRow, nEndCol,
                nEndRow, pOnlyNotBecauseOfMatrix );

    DBG_ERROR("Falsche Tabellennummer");
    if ( pOnlyNotBecauseOfMatrix )
        *pOnlyNotBecauseOfMatrix = FALSE;
    return FALSE;
}


BOOL ScDocument::IsSelectedBlockEditable( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow,
                                            const ScMarkData& rMark ) const
{
    // import into read-only document is possible - must be extended if other filters use api
    if ( pShell && pShell->IsReadOnly() && !bImportingXML )
        return FALSE;

    BOOL bOk = TRUE;
    for (SCTAB i=0; i<=MAXTAB && bOk; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                if (!pTab[i]->IsBlockEditable( nStartCol, nStartRow, nEndCol, nEndRow ))
                    bOk = FALSE;

    return bOk;
}


BOOL ScDocument::IsSelectionEditable( const ScMarkData& rMark,
            BOOL* pOnlyNotBecauseOfMatrix /* = NULL */ ) const
{
    // import into read-only document is possible - must be extended if other filters use api
    if ( pShell && pShell->IsReadOnly() && !bImportingXML )
    {
        if ( pOnlyNotBecauseOfMatrix )
            *pOnlyNotBecauseOfMatrix = FALSE;
        return FALSE;
    }

    ScRange aRange;
    rMark.GetMarkArea(aRange);

    BOOL bOk = TRUE;
    BOOL bMatrix = ( pOnlyNotBecauseOfMatrix != NULL );
    for ( SCTAB i=0; i<=MAXTAB && (bOk || bMatrix); i++ )
    {
        if ( pTab[i] && rMark.GetTableSelect(i) )
        {
            if (rMark.IsMarked())
            {
                if ( !pTab[i]->IsBlockEditable( aRange.aStart.Col(),
                        aRange.aStart.Row(), aRange.aEnd.Col(),
                        aRange.aEnd.Row(), pOnlyNotBecauseOfMatrix ) )
                {
                    bOk = FALSE;
                    if ( pOnlyNotBecauseOfMatrix )
                        bMatrix = *pOnlyNotBecauseOfMatrix;
                }
            }
            if (rMark.IsMultiMarked())
            {
                if ( !pTab[i]->IsSelectionEditable( rMark, pOnlyNotBecauseOfMatrix ) )
                {
                    bOk = FALSE;
                    if ( pOnlyNotBecauseOfMatrix )
                        bMatrix = *pOnlyNotBecauseOfMatrix;
                }
            }
        }
    }

    if ( pOnlyNotBecauseOfMatrix )
        *pOnlyNotBecauseOfMatrix = ( !bOk && bMatrix );

    return bOk;
}


BOOL ScDocument::IsSelectionOrBlockEditable( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow,
                                        const ScMarkData& rMark ) const
{
    // import into read-only document is possible - must be extended if other filters use api
    if ( pShell && pShell->IsReadOnly() && !bImportingXML )
        return FALSE;

    BOOL bOk = TRUE;
    if (VALIDTAB(nTab))
    {
        if (pTab[nTab])
        {
            if (rMark.IsMarked())
            {
                ScRange aRange;
                rMark.GetMarkArea(aRange);
                bOk = pTab[nTab]->IsBlockEditable( aRange.aStart.Col(), aRange.aStart.Row(),
                                                   aRange.aEnd.Col(),   aRange.aEnd.Row() );
            }

            if (bOk && rMark.IsMultiMarked())
                bOk = pTab[nTab]->IsSelectionEditable( rMark );
            if ( bOk && !rMark.IsMarked() && !rMark.IsMultiMarked() )
                bOk = pTab[nTab]->IsBlockEditable( nStartCol, nStartRow, nEndCol, nEndRow );
            return bOk;
        }
    }
    DBG_ERROR("Falsche Tabellennummer");
    return FALSE;
}


BOOL ScDocument::IsSelectedOrBlockEditable( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow,
                                            const ScMarkData& rMark ) const
{
    // import into read-only document is possible - must be extended if other filters use api
    if ( pShell && pShell->IsReadOnly() && !bImportingXML )
        return FALSE;

    BOOL bOk = TRUE;
    for (SCTAB i=0; i<=MAXTAB && bOk; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
            {
                if (rMark.IsMarked())
                {
                    ScRange aRange;
                    rMark.GetMarkArea(aRange);
                    bOk = pTab[i]->IsBlockEditable( aRange.aStart.Col(), aRange.aStart.Row(),
                                                    aRange.aEnd.Col(),   aRange.aEnd.Row() );
                }
                if ( bOk && !rMark.IsMarked() )
                    bOk = pTab[i]->IsBlockEditable( nStartCol, nStartRow, nEndCol, nEndRow );
            }

    return bOk;
}


BOOL ScDocument::HasSelectedBlockMatrixFragment( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow,
                                const ScMarkData& rMark ) const
{
    BOOL bOk = TRUE;
    for (SCTAB i=0; i<=MAXTAB && bOk; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                if (pTab[i]->HasBlockMatrixFragment( nStartCol, nStartRow, nEndCol, nEndRow ))
                    bOk = FALSE;

    return !bOk;
}


BOOL ScDocument::GetMatrixFormulaRange( const ScAddress& rCellPos, ScRange& rMatrix )
{
    //  if rCell is part of a matrix formula, return its complete range

    BOOL bRet = FALSE;
    ScBaseCell* pCell = GetCell( rCellPos );
    if (pCell && pCell->GetCellType() == CELLTYPE_FORMULA)
    {
        ScAddress aOrigin = rCellPos;
        if ( ((ScFormulaCell*)pCell)->GetMatrixOrigin( aOrigin ) )
        {
            if ( aOrigin != rCellPos )
                pCell = GetCell( aOrigin );
            if (pCell && pCell->GetCellType() == CELLTYPE_FORMULA)
            {
                SCCOL nSizeX;
                SCROW nSizeY;
                ((ScFormulaCell*)pCell)->GetMatColsRows(nSizeX,nSizeY);
                if ( !(nSizeX > 0 && nSizeY > 0) )
                {
                    // GetMatrixEdge computes also dimensions of the matrix
                    // if not already done (may occur if document is loaded
                    // from old file format).
                    // Needs an "invalid" initialized address.
                    aOrigin.SetInvalid();
                    ((ScFormulaCell*)pCell)->GetMatrixEdge(aOrigin);
                    ((ScFormulaCell*)pCell)->GetMatColsRows(nSizeX,nSizeY);
                }
                if ( nSizeX > 0 && nSizeY > 0 )
                {
                    ScAddress aEnd( aOrigin.Col() + nSizeX - 1,
                                    aOrigin.Row() + nSizeY - 1,
                                    aOrigin.Tab() );

                    rMatrix.aStart = aOrigin;
                    rMatrix.aEnd = aEnd;
                    bRet = TRUE;
                }
            }
        }
    }
    return bRet;
}


BOOL ScDocument::ExtendOverlapped( SCCOL& rStartCol, SCROW& rStartRow,
                                SCCOL nEndCol, SCROW nEndRow, SCTAB nTab )
{
    BOOL bFound = FALSE;
    if ( ValidColRow(rStartCol,rStartRow) && ValidColRow(nEndCol,nEndRow) && ValidTab(nTab) )
    {
        if (pTab[nTab])
        {
            SCCOL nCol;
            SCCOL nOldCol = rStartCol;
            SCROW nOldRow = rStartRow;
            for (nCol=nOldCol; nCol<=nEndCol; nCol++)
                while (((ScMergeFlagAttr*)GetAttr(nCol,rStartRow,nTab,ATTR_MERGE_FLAG))->
                            IsVerOverlapped())
                    --rStartRow;

            //!     weiterreichen ?

            ScAttrArray* pAttrArray = pTab[nTab]->aCol[nOldCol].pAttrArray;
            SCSIZE nIndex;
            pAttrArray->Search( nOldRow, nIndex );
            SCROW nAttrPos = nOldRow;
            while (nAttrPos<=nEndRow)
            {
                DBG_ASSERT( nIndex < pAttrArray->nCount, "Falscher Index im AttrArray" );

                if (((ScMergeFlagAttr&)pAttrArray->pData[nIndex].pPattern->
                        GetItem(ATTR_MERGE_FLAG)).IsHorOverlapped())
                {
                    SCROW nLoopEndRow = Min( nEndRow, pAttrArray->pData[nIndex].nRow );
                    for (SCROW nAttrRow = nAttrPos; nAttrRow <= nLoopEndRow; nAttrRow++)
                    {
                        SCCOL nTempCol = nOldCol;
                        do
                            --nTempCol;
                        while (((ScMergeFlagAttr*)GetAttr(nTempCol,nAttrRow,nTab,ATTR_MERGE_FLAG))
                                ->IsHorOverlapped());
                        if (nTempCol < rStartCol)
                            rStartCol = nTempCol;
                    }
                }
                nAttrPos = pAttrArray->pData[nIndex].nRow + 1;
                ++nIndex;
            }
        }
    }
    else
        DBG_ERROR("ExtendOverlapped: falscher Bereich");

    return bFound;
}


BOOL ScDocument::ExtendMerge( SCCOL nStartCol, SCROW nStartRow,
                              SCCOL& rEndCol,  SCROW& rEndRow,
                              SCTAB nTab, BOOL bRefresh, BOOL bAttrs )
{
    BOOL bFound = FALSE;
    if ( ValidColRow(nStartCol,nStartRow) && ValidColRow(rEndCol,rEndRow) && ValidTab(nTab) )
    {
        if (pTab[nTab])
            bFound = pTab[nTab]->ExtendMerge( nStartCol, nStartRow, rEndCol, rEndRow, bRefresh, bAttrs );

        if (bRefresh)
            RefreshAutoFilter( nStartCol, nStartRow, rEndCol, rEndRow, nTab );
    }
    else
        DBG_ERROR("ExtendMerge: falscher Bereich");

    return bFound;
}


BOOL ScDocument::ExtendMerge( ScRange& rRange, BOOL bRefresh, BOOL bAttrs )
{
    BOOL bFound = FALSE;
    SCTAB nStartTab = rRange.aStart.Tab();
    SCTAB nEndTab   = rRange.aEnd.Tab();
    SCCOL nEndCol   = rRange.aEnd.Col();
    SCROW nEndRow   = rRange.aEnd.Row();

    PutInOrder( nStartTab, nEndTab );
    for (SCTAB nTab = nStartTab; nTab <= nEndTab; nTab++ )
    {
        SCCOL nExtendCol = rRange.aEnd.Col();
        SCROW nExtendRow = rRange.aEnd.Row();
        if (ExtendMerge( rRange.aStart.Col(), rRange.aStart.Row(),
                         nExtendCol,          nExtendRow,
                         nTab, bRefresh, bAttrs ) )
        {
            bFound = TRUE;
            if (nExtendCol > nEndCol) nEndCol = nExtendCol;
            if (nExtendRow > nEndRow) nEndRow = nExtendRow;
        }
    }

    rRange.aEnd.SetCol(nEndCol);
    rRange.aEnd.SetRow(nEndRow);

    return bFound;
}

BOOL ScDocument::ExtendTotalMerge( ScRange& rRange )
{
    //  Bereich genau dann auf zusammengefasste Zellen erweitern, wenn
    //  dadurch keine neuen nicht-ueberdeckten Zellen getroffen werden

    BOOL bRet = FALSE;
    ScRange aExt = rRange;
    if (ExtendMerge(aExt))
    {
        if ( aExt.aEnd.Row() > rRange.aEnd.Row() )
        {
            ScRange aTest = aExt;
            aTest.aStart.SetRow( rRange.aEnd.Row() + 1 );
            if ( HasAttrib( aTest, HASATTR_NOTOVERLAPPED ) )
                aExt.aEnd.SetRow(rRange.aEnd.Row());
        }
        if ( aExt.aEnd.Col() > rRange.aEnd.Col() )
        {
            ScRange aTest = aExt;
            aTest.aStart.SetCol( rRange.aEnd.Col() + 1 );
            if ( HasAttrib( aTest, HASATTR_NOTOVERLAPPED ) )
                aExt.aEnd.SetCol(rRange.aEnd.Col());
        }

        bRet = ( aExt.aEnd != rRange.aEnd );
        rRange = aExt;
    }
    return bRet;
}

BOOL ScDocument::ExtendOverlapped( ScRange& rRange )
{
    BOOL bFound = FALSE;
    SCTAB nStartTab = rRange.aStart.Tab();
    SCTAB nEndTab   = rRange.aEnd.Tab();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();

    PutInOrder( nStartTab, nEndTab );
    for (SCTAB nTab = nStartTab; nTab <= nEndTab; nTab++ )
    {
        SCCOL nExtendCol = rRange.aStart.Col();
        SCROW nExtendRow = rRange.aStart.Row();
        ExtendOverlapped( nExtendCol, nExtendRow,
                                rRange.aEnd.Col(), rRange.aEnd.Row(), nTab );
        if (nExtendCol < nStartCol)
        {
            nStartCol = nExtendCol;
            bFound = TRUE;
        }
        if (nExtendRow < nStartRow)
        {
            nStartRow = nExtendRow;
            bFound = TRUE;
        }
    }

    rRange.aStart.SetCol(nStartCol);
    rRange.aStart.SetRow(nStartRow);

    return bFound;
}

BOOL ScDocument::RefreshAutoFilter( SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow, SCTAB nTab )
{
    USHORT nCount = pDBCollection->GetCount();
    USHORT i;
    ScDBData* pData;
    SCTAB nDBTab;
    SCCOL nDBStartCol;
    SCROW nDBStartRow;
    SCCOL nDBEndCol;
    SCROW nDBEndRow;

    //      Autofilter loeschen

    BOOL bChange = RemoveFlagsTab( nStartCol,nStartRow, nEndCol,nEndRow, nTab, SC_MF_AUTO );

    //      Autofilter setzen

    for (i=0; i<nCount; i++)
    {
        pData = (*pDBCollection)[i];
        if (pData->HasAutoFilter())
        {
            pData->GetArea( nDBTab, nDBStartCol,nDBStartRow, nDBEndCol,nDBEndRow );
            if ( nDBTab==nTab && nDBStartRow<=nEndRow && nDBEndRow>=nStartRow &&
                                    nDBStartCol<=nEndCol && nDBEndCol>=nStartCol )
            {
                if (ApplyFlagsTab( nDBStartCol,nDBStartRow, nDBEndCol,nDBStartRow,
                                    nDBTab, SC_MF_AUTO ))
                    bChange = TRUE;
            }
        }
    }
    return bChange;
}


void ScDocument::SetAutoFilterFlags()
{
    USHORT nCount = pDBCollection->GetCount();
    for (USHORT i=0; i<nCount; i++)
    {
        ScDBData* pData = (*pDBCollection)[i];
        SCTAB nDBTab;
        SCCOL nDBStartCol;
        SCROW nDBStartRow;
        SCCOL nDBEndCol;
        SCROW nDBEndRow;
        pData->GetArea( nDBTab, nDBStartCol,nDBStartRow, nDBEndCol,nDBEndRow );
        pData->SetAutoFilter( HasAttrib( nDBStartCol,nDBStartRow,nDBTab,
                                nDBEndCol,nDBStartRow,nDBTab, HASATTR_AUTOFILTER ) );
    }
}


BOOL ScDocument::IsOverlapped( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    const ScMergeFlagAttr* pAttr = (const ScMergeFlagAttr*)
                                        GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG );
    if (pAttr)
        return pAttr->IsOverlapped();
    else
    {
        DBG_ERROR("Overlapped: Attr==0");
        return FALSE;
    }
}


BOOL ScDocument::IsHorOverlapped( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    const ScMergeFlagAttr* pAttr = (const ScMergeFlagAttr*)
                                        GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG );
    if (pAttr)
        return pAttr->IsHorOverlapped();
    else
    {
        DBG_ERROR("Overlapped: Attr==0");
        return FALSE;
    }
}


BOOL ScDocument::IsVerOverlapped( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    const ScMergeFlagAttr* pAttr = (const ScMergeFlagAttr*)
                                        GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG );
    if (pAttr)
        return pAttr->IsVerOverlapped();
    else
    {
        DBG_ERROR("Overlapped: Attr==0");
        return FALSE;
    }
}


void ScDocument::ApplySelectionFrame( const ScMarkData& rMark,
                                      const SvxBoxItem* pLineOuter,
                                      const SvxBoxInfoItem* pLineInner )
{
    if (rMark.IsMarked())
    {
        ScRange aRange;
        rMark.GetMarkArea(aRange);
        for (SCTAB i=0; i<=MAXTAB; i++)
            if (pTab[i])
                if (rMark.GetTableSelect(i))
                    pTab[i]->ApplyBlockFrame( pLineOuter, pLineInner,
                                                aRange.aStart.Col(), aRange.aStart.Row(),
                                                aRange.aEnd.Col(),   aRange.aEnd.Row() );
    }
}


void ScDocument::ApplyFrameAreaTab( const ScRange& rRange,
                                    const SvxBoxItem* pLineOuter,
                                    const SvxBoxInfoItem* pLineInner )
{
    SCTAB nStartTab = rRange.aStart.Tab();
    SCTAB nEndTab = rRange.aStart.Tab();
    for (SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++)
        if (pTab[nTab])
            pTab[nTab]->ApplyBlockFrame( pLineOuter, pLineInner,
                                         rRange.aStart.Col(), rRange.aStart.Row(),
                                         rRange.aEnd.Col(),   rRange.aEnd.Row() );
}


void ScDocument::ApplySelectionPattern( const ScPatternAttr& rAttr, const ScMarkData& rMark )
{
    const SfxItemSet* pSet = &rAttr.GetItemSet();
    BOOL bSet = FALSE;
    USHORT i;
    for (i=ATTR_PATTERN_START; i<=ATTR_PATTERN_END && !bSet; i++)
        if (pSet->GetItemState(i) == SFX_ITEM_SET)
            bSet = TRUE;

    if (bSet)
    {
        // ApplySelectionCache needs multi mark
        if ( rMark.IsMarked() && !rMark.IsMultiMarked() )
        {
            ScRange aRange;
            rMark.GetMarkArea( aRange );
            ApplyPatternArea( aRange.aStart.Col(), aRange.aStart.Row(),
                              aRange.aEnd.Col(), aRange.aEnd.Row(), rMark, rAttr );
        }
        else
        {
            SfxItemPoolCache aCache( xPoolHelper->GetDocPool(), pSet );
            for (SCTAB i=0; i<=MAXTAB; i++)
                if (pTab[i])
                    if (rMark.GetTableSelect(i))
                        pTab[i]->ApplySelectionCache( &aCache, rMark );
        }
    }
}


void ScDocument::ChangeSelectionIndent( BOOL bIncrement, const ScMarkData& rMark )
{
    for (SCTAB i=0; i<=MAXTAB; i++)
        if (pTab[i] && rMark.GetTableSelect(i))
            pTab[i]->ChangeSelectionIndent( bIncrement, rMark );
}


void ScDocument::ClearSelectionItems( const USHORT* pWhich, const ScMarkData& rMark )
{
    for (SCTAB i=0; i<=MAXTAB; i++)
        if (pTab[i] && rMark.GetTableSelect(i))
            pTab[i]->ClearSelectionItems( pWhich, rMark );
}


void ScDocument::DeleteSelection( USHORT nDelFlag, const ScMarkData& rMark )
{
    for (SCTAB i=0; i<=MAXTAB; i++)
        if (pTab[i] && rMark.GetTableSelect(i))
            pTab[i]->DeleteSelection( nDelFlag, rMark );
}


void ScDocument::DeleteSelectionTab( SCTAB nTab, USHORT nDelFlag, const ScMarkData& rMark )
{
    if (ValidTab(nTab)  && pTab[nTab])
        pTab[nTab]->DeleteSelection( nDelFlag, rMark );
    else
        DBG_ERROR("Falsche Tabelle");
}


ScPatternAttr* ScDocument::GetDefPattern() const
{
    return (ScPatternAttr*) &xPoolHelper->GetDocPool()->GetDefaultItem(ATTR_PATTERN);
}


ScDocumentPool* ScDocument::GetPool()
{
    return xPoolHelper->GetDocPool();
}



ScStyleSheetPool* ScDocument::GetStyleSheetPool() const
{
    return xPoolHelper->GetStylePool();
}


SCSIZE ScDocument::GetEmptyLinesInBlock( SCCOL nStartCol, SCROW nStartRow, SCTAB nStartTab,
                            SCCOL nEndCol, SCROW nEndRow, SCTAB nEndTab, ScDirection eDir )
{
    PutInOrder(nStartCol, nEndCol);
    PutInOrder(nStartRow, nEndRow);
    PutInOrder(nStartTab, nEndTab);
    if (VALIDTAB(nStartTab))
    {
        if (pTab[nStartTab])
            return pTab[nStartTab]->GetEmptyLinesInBlock(nStartCol, nStartRow, nEndCol, nEndRow, eDir);
        else
            return 0;
    }
    else
        return 0;
}


void ScDocument::FindAreaPos( SCCOL& rCol, SCROW& rRow, SCTAB nTab, SCsCOL nMovX, SCsROW nMovY )
{
    if (ValidTab(nTab) && pTab[nTab])
        pTab[nTab]->FindAreaPos( rCol, rRow, nMovX, nMovY );
}


void ScDocument::GetNextPos( SCCOL& rCol, SCROW& rRow, SCTAB nTab, SCsCOL nMovX, SCsROW nMovY,
                                BOOL bMarked, BOOL bUnprotected, const ScMarkData& rMark )
{
    DBG_ASSERT( !nMovX || !nMovY, "GetNextPos: nur X oder Y" );

    ScMarkData aCopyMark = rMark;
    aCopyMark.SetMarking(FALSE);
    aCopyMark.MarkToMulti();

    if (ValidTab(nTab) && pTab[nTab])
        pTab[nTab]->GetNextPos( rCol, rRow, nMovX, nMovY, bMarked, bUnprotected, aCopyMark );
}

//
//  Datei-Operationen
//


void ScDocument::UpdStlShtPtrsFrmNms()
{
    ScPatternAttr::pDoc = this;

    ScDocumentPool* pPool = xPoolHelper->GetDocPool();

    USHORT nCount = pPool->GetItemCount(ATTR_PATTERN);
    ScPatternAttr* pPattern;
    for (USHORT i=0; i<nCount; i++)
    {
        pPattern = (ScPatternAttr*)pPool->GetItem(ATTR_PATTERN, i);
        if (pPattern)
            pPattern->UpdateStyleSheet();
    }
    ((ScPatternAttr&)pPool->GetDefaultItem(ATTR_PATTERN)).UpdateStyleSheet();
}


void ScDocument::StylesToNames()
{
    ScPatternAttr::pDoc = this;

    ScDocumentPool* pPool = xPoolHelper->GetDocPool();

    USHORT nCount = pPool->GetItemCount(ATTR_PATTERN);
    ScPatternAttr* pPattern;
    for (USHORT i=0; i<nCount; i++)
    {
        pPattern = (ScPatternAttr*)pPool->GetItem(ATTR_PATTERN, i);
        if (pPattern)
            pPattern->StyleToName();
    }
    ((ScPatternAttr&)pPool->GetDefaultItem(ATTR_PATTERN)).StyleToName();
}


void lcl_RemoveMergeFromStyles( ScStyleSheetPool* pStylePool )
{
    pStylePool->SetSearchMask( SFX_STYLE_FAMILY_ALL );

    USHORT nCount = pStylePool->Count();
    for (USHORT i=0; i<nCount; i++)
    {
        //  in alten Versionen wurden statt SFXSTYLEBIT_USERDEF alle Bits gesetzt
        SfxStyleSheetBase* pStyle = (*pStylePool)[i];
        if ( pStyle->GetMask() & SFXSTYLEBIT_READONLY )
            pStyle->SetMask( pStyle->GetMask() & ~SFXSTYLEBIT_READONLY );

        SfxItemSet& rSet = pStyle->GetItemSet();
        rSet.ClearItem( ATTR_MERGE );
        rSet.ClearItem( ATTR_MERGE_FLAG );

        //  Das SvxBoxInfoItem wurde bis zur 358 falsch geladen, so dass
        //  Seitenvorlagen falsche Items mit bDist = FALSE enthalten koennen
        if ( pStyle->GetFamily() == SFX_STYLE_FAMILY_PAGE )
        {
            const SvxBoxInfoItem& rPageInfo = (const SvxBoxInfoItem&)rSet.Get(ATTR_BORDER_INNER);
            if ( !rPageInfo.IsDist() )
            {
                DBG_WARNING("altes SvxBoxInfoItem muss korrigiert werden");
                SvxBoxInfoItem aNew( rPageInfo );
                aNew.SetDist( TRUE );
                rSet.Put( aNew );
            }
            //  Das gilt fuer alle Hdr/Ftr-SetItems, darum kann das SetItem auch
            //  direkt im Pool geaendert werden (const weggecastet):
            SfxItemSet& rHdrSet = ((SvxSetItem&)rSet.Get(ATTR_PAGE_HEADERSET)).GetItemSet();
            const SvxBoxInfoItem& rHdrInfo = (const SvxBoxInfoItem&)rHdrSet.Get(ATTR_BORDER_INNER);
            if ( !rHdrInfo.IsDist() )
            {
                DBG_WARNING("altes SvxBoxInfoItem muss korrigiert werden");
                SvxBoxInfoItem aNew( rHdrInfo );
                aNew.SetDist( TRUE );
                rHdrSet.Put( aNew );
            }
            SfxItemSet& rFtrSet = ((SvxSetItem&)rSet.Get(ATTR_PAGE_FOOTERSET)).GetItemSet();
            const SvxBoxInfoItem& rFtrInfo = (const SvxBoxInfoItem&)rFtrSet.Get(ATTR_BORDER_INNER);
            if ( !rFtrInfo.IsDist() )
            {
                DBG_WARNING("altes SvxBoxInfoItem muss korrigiert werden");
                SvxBoxInfoItem aNew( rFtrInfo );
                aNew.SetDist( TRUE );
                rFtrSet.Put( aNew );
            }
            const SfxUInt16Item& rScaleItem = (const SfxUInt16Item&)rSet.Get(ATTR_PAGE_SCALE);
            USHORT nScale = rScaleItem.GetValue();
            //! Extra-Konstanten fuer Seitenformat?
            //  0 ist erlaubt (wird gesetzt bei Scale To Pages)
            if ( nScale != 0 && ( nScale < MINZOOM || nScale > MAXZOOM ) )
            {
                //  konnte anscheinend mal irgendwie kaputtgehen (#34508#)
                DBG_WARNING("kaputter Zoom im Seitenformat muss korrigiert werden");
                rSet.Put( SfxUInt16Item( ATTR_PAGE_SCALE, 100 ) );
            }
        }
    }
}


BOOL ScDocument::LoadPool( SvStream& rStream, BOOL bLoadRefCounts )
{
    //  bLoadingDone wird beim Laden des StylePools (ScStyleSheet::GetItemSet) gebraucht
    bLoadingDone = FALSE;

    USHORT nOldBufSize = rStream.GetBufferSize();
    rStream.SetBufferSize( 32768 );
    CharSet eOldSet = rStream.GetStreamCharSet();

    SetPrinter( NULL );

    ScPatternAttr::pDoc = this;

    if ( xPoolHelper.isValid() && !bIsClip )
        xPoolHelper->SourceDocumentGone();

    xPoolHelper = new ScPoolHelper( this );

    xPoolHelper->GetDocPool()->SetFileFormatVersion( (USHORT)rStream.GetVersion() );
    BOOL bStylesFound = FALSE;

    BOOL bRet = FALSE;
    USHORT nID;
    rStream >> nID;
    if (nID == SCID_POOLS || nID == SCID_NEWPOOLS)
    {
        ScReadHeader aHdr( rStream );
        while (aHdr.BytesLeft())
        {
            USHORT nSubID;
            rStream >> nSubID;
            ScReadHeader aSubHdr( rStream );
            switch (nSubID)
            {
                case SCID_CHARSET:
                    {
                        BYTE cSet, cGUI;    // cGUI is dummy, old GUIType
                        rStream >> cGUI >> cSet;
                        eSrcSet = (CharSet) cSet;
                        rStream.SetStreamCharSet( ::GetSOLoadTextEncoding(
                            eSrcSet, (USHORT)rStream.GetVersion() ) );
                    }
                    break;
                case SCID_DOCPOOL:
                    xPoolHelper->GetDocPool()->Load( rStream );
                    break;
                case SCID_STYLEPOOL:
                    {
                        //  StylePool konvertiert beim Laden selber
                        CharSet eOld = rStream.GetStreamCharSet();
                        rStream.SetStreamCharSet( gsl_getSystemTextEncoding() );    //! ???
                        xPoolHelper->GetStylePool()->Load( rStream );
                        rStream.SetStreamCharSet( eOld );
                        lcl_RemoveMergeFromStyles( xPoolHelper->GetStylePool() );   // setzt auch ReadOnly zurueck
                        bStylesFound = TRUE;
                    }
                    break;
                case SCID_EDITPOOL :
                    xPoolHelper->GetEditPool()->Load( rStream );
                    break;
                default:
                    DBG_ERROR("unbekannter Sub-Record in ScDocument::LoadPool");
            }
        }

        UpdStlShtPtrsFrmNms();
        bRet = TRUE;
    }
    else
        DBG_ERROR("LoadPool: SCID_POOLS nicht gefunden");

    if (!bStylesFound)
        xPoolHelper->GetStylePool()->CreateStandardStyles();

    rStream.SetStreamCharSet( eOldSet );
    rStream.SetBufferSize( nOldBufSize );

    bLoadingDone = TRUE;

    //  Das Uno-Objekt merkt sich einen Pointer auf den NumberFormatter
    //  -> mitteilen, dass der alte Pointer ungueltig geworden ist
    BroadcastUno( ScPointerChangedHint(SC_POINTERCHANGED_NUMFMT) );

    return bRet;
}


BOOL ScDocument::SavePool( SvStream& rStream ) const
{
    xPoolHelper->GetDocPool()->SetFileFormatVersion( (USHORT)rStream.GetVersion() );

    USHORT nOldBufSize = rStream.GetBufferSize();
    rStream.SetBufferSize( 32768 );
    CharSet eOldSet = rStream.GetStreamCharSet();
    CharSet eStoreCharSet = ::GetSOStoreTextEncoding(
        gsl_getSystemTextEncoding(), (USHORT)rStream.GetVersion() );
    rStream.SetStreamCharSet( eStoreCharSet );

    //  Compress-Mode fuer Grafiken in Brush-Items (Hintergrund im Seitenformat)

    USHORT nComprMode = rStream.GetCompressMode() & ~(COMPRESSMODE_ZBITMAP | COMPRESSMODE_NATIVE);
    SvtSaveOptions aSaveOpt;
    SvtSaveOptions::SaveGraphicsMode eMode = aSaveOpt.GetSaveGraphicsMode();
    BOOL bNative = ( eMode == SvtSaveOptions::SaveGraphicsOriginal );
    BOOL bCompr = bNative || ( eMode == SvtSaveOptions::SaveGraphicsCompressed );

    if ( rStream.GetVersion() >= SOFFICE_FILEFORMAT_40 && bCompr )
        nComprMode |= COMPRESSMODE_ZBITMAP;             //  komprimiert ab 4.0
    if ( rStream.GetVersion() > SOFFICE_FILEFORMAT_40 && bNative )
        nComprMode |= COMPRESSMODE_NATIVE;              //  Originalformat ab 5.0
    rStream.SetCompressMode( nComprMode );

    {
        rStream << (USHORT) SCID_NEWPOOLS;
        ScWriteHeader aHdr( rStream );

        {
            rStream << (USHORT) SCID_CHARSET;
            ScWriteHeader aSetHdr( rStream, 2 );
            rStream << (BYTE) 0     // dummy, old System::GetGUIType()
                    << (BYTE) eStoreCharSet;
        }

        //  Force the default style's name to be "Standard" for all languages in the file.
        //  This is needed for versions up to 5.1, to find the default pattern's style in
        //  the UpdateStyleSheet call.
        //  #89078# this has to be set for the DocPool save, too, so the default style name
        //  is adjusted for the patterns, or a wrong style would be used if other styles
        //  match the default style's name after CharacterSet conversion.

        String aFileStdName = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(STRING_STANDARD));
        if ( aFileStdName != ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
            xPoolHelper->GetStylePool()->SetForceStdName( &aFileStdName );

        {
            rStream << (USHORT) SCID_DOCPOOL;
            ScWriteHeader aDocPoolHdr( rStream );
            xPoolHelper->GetDocPool()->Store( rStream );
        }

        {
            rStream << (USHORT) SCID_STYLEPOOL;
            ScWriteHeader aStylePoolHdr( rStream );
            xPoolHelper->GetStylePool()->SetSearchMask( SFX_STYLE_FAMILY_ALL );

            xPoolHelper->GetStylePool()->Store( rStream, FALSE );
        }

        xPoolHelper->GetStylePool()->SetForceStdName( NULL );

        if ( rStream.GetVersion() >= SOFFICE_FILEFORMAT_50 )
        {
            rStream << (USHORT) SCID_EDITPOOL;
            ScWriteHeader aEditPoolHdr( rStream );
            xPoolHelper->GetEditPool()->SetFileFormatVersion( (USHORT)rStream.GetVersion() );
            xPoolHelper->GetEditPool()->Store( rStream );
        }
    }

    rStream.SetStreamCharSet( eOldSet );
    rStream.SetBufferSize( nOldBufSize );
    return TRUE;
}


ULONG ScDocument::GetCellCount() const
{
    ULONG nCellCount = 0L;

    for ( SCTAB nTab=0; nTab<=MAXTAB; nTab++ )
        if ( pTab[nTab] )
            nCellCount += pTab[nTab]->GetCellCount();

    return nCellCount;
}


ULONG ScDocument::GetCodeCount() const
{
    ULONG nCodeCount = 0;

    for ( SCTAB nTab=0; nTab<=MAXTAB; nTab++ )
        if ( pTab[nTab] )
            nCodeCount += pTab[nTab]->GetCodeCount();

    return nCodeCount;
}


ULONG ScDocument::GetWeightedCount() const
{
    ULONG nCellCount = 0L;

    for ( SCTAB nTab=0; nTab<=MAXTAB; nTab++ )
        if ( pTab[nTab] )
            nCellCount += pTab[nTab]->GetWeightedCount();

    return nCellCount;
}


void ScDocument::PageStyleModified( SCTAB nTab, const String& rNewName )
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        pTab[nTab]->PageStyleModified( rNewName );
}


void ScDocument::SetPageStyle( SCTAB nTab, const String& rName )
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        pTab[nTab]->SetPageStyle( rName );
}


const String& ScDocument::GetPageStyle( SCTAB nTab ) const
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        return pTab[nTab]->GetPageStyle();

    return EMPTY_STRING;
}


void ScDocument::SetPageSize( SCTAB nTab, const Size& rSize )
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        pTab[nTab]->SetPageSize( rSize );
}

Size ScDocument::GetPageSize( SCTAB nTab ) const
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        return pTab[nTab]->GetPageSize();

    DBG_ERROR("falsche Tab");
    return Size();
}


void ScDocument::SetRepeatArea( SCTAB nTab, SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCROW nEndRow )
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        pTab[nTab]->SetRepeatArea( nStartCol, nEndCol, nStartRow, nEndRow );
}


void ScDocument::UpdatePageBreaks()
{
    for (SCTAB i=0; i<=MAXTAB; i++)
        if (pTab[i])
            pTab[i]->UpdatePageBreaks( NULL );
}


void ScDocument::UpdatePageBreaks( SCTAB nTab, const ScRange* pUserArea )
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        pTab[nTab]->UpdatePageBreaks( pUserArea );
}

void ScDocument::RemoveManualBreaks( SCTAB nTab )
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        pTab[nTab]->RemoveManualBreaks();
}

BOOL ScDocument::HasManualBreaks( SCTAB nTab ) const
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        return pTab[nTab]->HasManualBreaks();

    DBG_ERROR("falsche Tab");
    return FALSE;
}


void ScDocument::GetDocStat( ScDocStat& rDocStat )
{
    rDocStat.nTableCount = GetTableCount();
    rDocStat.aDocName    = aDocName;
    rDocStat.nCellCount  = GetCellCount();
}


BOOL ScDocument::HasPrintRange()
{
    BOOL bResult = FALSE;

    for ( SCTAB i=0; !bResult && i<nMaxTableNumber; i++ )
        if ( pTab[i] )
            bResult = pTab[i]->IsPrintEntireSheet() || (pTab[i]->GetPrintRangeCount() > 0);

    return bResult;
}


BOOL ScDocument::IsPrintEntireSheet( SCTAB nTab ) const
{
    return (ValidTab(nTab) ) && pTab[nTab] && pTab[nTab]->IsPrintEntireSheet();
}


USHORT ScDocument::GetPrintRangeCount( SCTAB nTab )
{
    if (ValidTab(nTab) && pTab[nTab])
        return pTab[nTab]->GetPrintRangeCount();

    return 0;
}


const ScRange* ScDocument::GetPrintRange( SCTAB nTab, USHORT nPos )
{
    if (ValidTab(nTab) && pTab[nTab])
        return pTab[nTab]->GetPrintRange(nPos);

    return NULL;
}


const ScRange* ScDocument::GetRepeatColRange( SCTAB nTab )
{
    if (ValidTab(nTab) && pTab[nTab])
        return pTab[nTab]->GetRepeatColRange();

    return NULL;
}


const ScRange* ScDocument::GetRepeatRowRange( SCTAB nTab )
{
    if (ValidTab(nTab) && pTab[nTab])
        return pTab[nTab]->GetRepeatRowRange();

    return NULL;
}


void ScDocument::ClearPrintRanges( SCTAB nTab )
{
    if (ValidTab(nTab) && pTab[nTab])
        pTab[nTab]->ClearPrintRanges();
}


void ScDocument::AddPrintRange( SCTAB nTab, const ScRange& rNew )
{
    if (ValidTab(nTab) && pTab[nTab])
        pTab[nTab]->AddPrintRange( rNew );
}


void ScDocument::SetPrintRange( SCTAB nTab, const ScRange& rNew )
{
    if (ValidTab(nTab) && pTab[nTab])
        pTab[nTab]->SetPrintRange( rNew );
}


void ScDocument::SetPrintEntireSheet( SCTAB nTab )
{
    if (ValidTab(nTab) && pTab[nTab])
        pTab[nTab]->SetPrintEntireSheet();
}


void ScDocument::SetRepeatColRange( SCTAB nTab, const ScRange* pNew )
{
    if (ValidTab(nTab) && pTab[nTab])
        pTab[nTab]->SetRepeatColRange( pNew );
}


void ScDocument::SetRepeatRowRange( SCTAB nTab, const ScRange* pNew )
{
    if (ValidTab(nTab) && pTab[nTab])
        pTab[nTab]->SetRepeatRowRange( pNew );
}


ScPrintRangeSaver* ScDocument::CreatePrintRangeSaver() const
{
    SCTAB nCount = GetTableCount();
    ScPrintRangeSaver* pNew = new ScPrintRangeSaver( nCount );
    for (SCTAB i=0; i<nCount; i++)
        if (pTab[i])
            pTab[i]->FillPrintSaver( pNew->GetTabData(i) );
    return pNew;
}


void ScDocument::RestorePrintRanges( const ScPrintRangeSaver& rSaver )
{
    SCTAB nCount = rSaver.GetTabCount();
    for (SCTAB i=0; i<nCount; i++)
        if (pTab[i])
            pTab[i]->RestorePrintRanges( rSaver.GetTabData(i) );
}


BOOL ScDocument::NeedPageResetAfterTab( SCTAB nTab ) const
{
    //  Die Seitennummern-Zaehlung fängt bei einer Tabelle neu an, wenn eine
    //  andere Vorlage als bei der vorherigen gesetzt ist (nur Namen vergleichen)
    //  und eine Seitennummer angegeben ist (nicht 0)

    if ( nTab < MAXTAB && pTab[nTab] && pTab[nTab+1] )
    {
        String aNew = pTab[nTab+1]->GetPageStyle();
        if ( aNew != pTab[nTab]->GetPageStyle() )
        {
            SfxStyleSheetBase* pStyle = xPoolHelper->GetStylePool()->Find( aNew, SFX_STYLE_FAMILY_PAGE );
            if ( pStyle )
            {
                const SfxItemSet& rSet = pStyle->GetItemSet();
                USHORT nFirst = ((const SfxUInt16Item&)rSet.Get(ATTR_PAGE_FIRSTPAGENO)).GetValue();
                if ( nFirst != 0 )
                    return TRUE;        // Seitennummer in neuer Vorlage angegeben
            }
        }
    }

    return FALSE;       // sonst nicht
}



