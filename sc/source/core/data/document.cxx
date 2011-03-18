/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#define _ZFORLIST_DECLARE_TABLE
#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <editeng/boxitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/pageitem.hxx>
#include <editeng/editeng.hxx>
#include <svx/svditer.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdocapt.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <svl/poolcach.hxx>
#include <unotools/saveopt.hxx>
#include <svl/zforlist.hxx>
#include <unotools/charclass.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <tools/tenccvt.hxx>

#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/sheet/TablePageBreakData.hpp>

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
#include "detfunc.hxx"      // for UpdateAllComments
#include "scmod.hxx"
#include "dociter.hxx"
#include "progress.hxx"
#include "autonamecache.hxx"
#include "bcaslot.hxx"
#include "postit.hxx"
#include "externalrefmgr.hxx"
#include "tabprotection.hxx"
#include "clipparam.hxx"

#include <map>
#include <limits>

using namespace ::com::sun::star;

namespace WritingMode2 = ::com::sun::star::text::WritingMode2;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::sheet::TablePageBreakData;
using ::std::set;

// The constant parameters to CopyBlockFromClip
struct ScCopyBlockFromClipParams
{
    ScDocument* pRefUndoDoc;
    ScDocument* pClipDoc;
    sal_uInt16      nInsFlag;
    SCTAB       nTabStart;
    SCTAB       nTabEnd;
    sal_Bool        bAsLink;
    sal_Bool        bSkipAttrForEmpty;
};

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

void ScDocument::MakeTable( SCTAB nTab,bool _bNeedsNameCheck )
{
    if ( ValidTab(nTab) && !pTab[nTab] )
    {
        String aString = ScGlobal::GetRscString(STR_TABLE_DEF); //"Table"
        aString += String::CreateFromInt32(nTab+1);
        if ( _bNeedsNameCheck )
            CreateValidTabName( aString );  // no doubles

        pTab[nTab] = new ScTable(this, nTab, aString);
        pTab[nTab]->SetLoadingMedium(bLoadingMedium);
        ++nMaxTableNumber;
    }
}


sal_Bool ScDocument::HasTable( SCTAB nTab ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return sal_True;

    return false;
}


sal_Bool ScDocument::GetName( SCTAB nTab, String& rName ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
        {
            pTab[nTab]->GetName( rName );
            return sal_True;
        }
    rName.Erase();
    return false;
}

sal_Bool ScDocument::SetCodeName( SCTAB nTab, const String& rName )
{
    if (VALIDTAB(nTab))
    {
        if (pTab[nTab])
        {
            pTab[nTab]->SetCodeName( rName );
            return sal_True;
        }
    }
    OSL_TRACE( "**** can't set code name %s", rtl::OUStringToOString( rName, RTL_TEXTENCODING_UTF8 ).getStr() );
    return false;
}

sal_Bool ScDocument::GetCodeName( SCTAB nTab, String& rName ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
        {
            pTab[nTab]->GetCodeName( rName );
            return sal_True;
        }
    rName.Erase();
    return false;
}

sal_Bool ScDocument::GetTable( const String& rName, SCTAB& rTab ) const
{
    String aUpperName = rName;
    ScGlobal::pCharClass->toUpper(aUpperName);

    for (SCTAB i=0; i<=MAXTAB; i++)
        if (pTab[i])
        {
            if ( pTab[i]->GetUpperName() == aUpperName )
            {
                rTab = i;
                return sal_True;
            }
        }
    rTab = 0;
    return false;
}


bool ScDocument::ValidTabName( const String& rName )
{
    xub_StrLen nLen = rName.Len();
    if (!nLen)
        return false;

#if 1
    // Restrict sheet names to what Excel accepts.
    /* TODO: We may want to remove this restriction for full ODFF compliance.
     * Merely loading and calculating ODF documents using these characters in
     * sheet names is not affected by this, but all sheet name editing and
     * copying functionality is, maybe falling back to "Sheet4" or similar. */
    for (xub_StrLen i = 0; i < nLen; ++i)
    {
        const sal_Unicode c = rName.GetChar(i);
        switch (c)
        {
            case ':':
            case '\\':
            case '/':
            case '?':
            case '*':
            case '[':
            case ']':
                // these characters are not allowed to match XL's convention.
                return false;
            case '\'':
                if (i == 0 || i == nLen - 1)
                    // single quote is not allowed at the first or last
                    // character position.
                    return false;
            break;
        }
    }
#endif

    return true;
}


sal_Bool ScDocument::ValidNewTabName( const String& rName ) const
{
    sal_Bool bValid = ValidTabName(rName);
    for (SCTAB i=0; (i<=MAXTAB) && bValid; i++)
        if (pTab[i])
        {
            String aOldName;
            pTab[i]->GetName(aOldName);
            bValid = !ScGlobal::GetpTransliteration()->isEqual( rName, aOldName );
        }
    return bValid;
}


void ScDocument::CreateValidTabName(String& rName) const
{
    if ( !ValidTabName(rName) )
    {
        // Find new one

        const String aStrTable( ScResId(SCSTR_TABLE) );
        sal_Bool         bOk   = false;

        // First test if the prefix is valid, if so only avoid doubles
        sal_Bool bPrefix = ValidTabName( aStrTable );
        OSL_ENSURE(bPrefix, "Invalid Table Name");
        SCTAB nDummy;

        SCTAB nLoops = 0;       // "for safety messures"
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

        OSL_ENSURE(bOk, "No Valid Table name found.");
        if ( !bOk )
            rName = aStrTable;
    }
    else
    {
        // testing the supplied Name

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


sal_Bool ScDocument::InsertTab( SCTAB nPos, const String& rName,
            sal_Bool bExternalDocument )
{
    SCTAB   nTabCount = GetTableCount();
    sal_Bool    bValid = ValidTab(nTabCount);
    if ( !bExternalDocument )   // else test rName == "'Doc'!Tab" first
        bValid = (bValid && ValidNewTabName(rName));
    if (bValid)
    {
        if (nPos == SC_TAB_APPEND || nPos == nTabCount)
        {
            pTab[nTabCount] = new ScTable(this, nTabCount, rName);
            pTab[nTabCount]->SetCodeName( rName );
            ++nMaxTableNumber;
            if ( bExternalDocument )
                pTab[nTabCount]->SetVisible( false );
        }
        else
        {
            if (VALIDTAB(nPos) && (nPos < nTabCount))
            {
                ScRange aRange( 0,0,nPos, MAXCOL,MAXROW,MAXTAB );
                xColNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,1 );
                xRowNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,1 );
                if (pRangeName)
                    pRangeName->UpdateTabRef( nPos, 1 );
                pDBCollection->UpdateReference(
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
                {
                    pTab[i] = pTab[i - 1];
                }

                pTab[nPos] = new ScTable(this, nPos, rName);
                pTab[nPos]->SetCodeName( rName );
                ++nMaxTableNumber;

                // UpdateBroadcastAreas must be called between UpdateInsertTab,
                // which ends listening, and StartAllListeners, to not modify
                // areas that are to be inserted by starting listeners.
                UpdateBroadcastAreas( URM_INSDEL, aRange, 0,0,1);
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->UpdateCompile();
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->StartAllListeners();

                //  update conditional formats after table is inserted
                if ( pCondFormList )
                    pCondFormList->UpdateReference( URM_INSDEL, aRange, 0,0,1 );
                if ( pValidationList )
                    pValidationList->UpdateReference( URM_INSDEL, aRange, 0,0,1 );
                // sheet names of references are not valid until sheet is inserted
                if ( pChartListenerCollection )
                    pChartListenerCollection->UpdateScheduledSeriesRanges();

                SetDirty();
                bValid = sal_True;
            }
            else
                bValid = false;
        }
    }
    return bValid;
}


sal_Bool ScDocument::DeleteTab( SCTAB nTab, ScDocument* pRefUndoDoc )
{
    sal_Bool bValid = false;
    if (VALIDTAB(nTab))
    {
        if (pTab[nTab])
        {
            SCTAB nTabCount = GetTableCount();
            if (nTabCount > 1)
            {
                sal_Bool bOldAutoCalc = GetAutoCalc();
                SetAutoCalc( false );   // avoid multiple calculations
                ScRange aRange( 0, 0, nTab, MAXCOL, MAXROW, nTab );
                DelBroadcastAreasInRange( aRange );

                // #i8180# remove database ranges etc. that are on the deleted tab
                // (restored in undo with ScRefUndoData)

                xColNameRanges->DeleteOnTab( nTab );
                xRowNameRanges->DeleteOnTab( nTab );
                pDBCollection->DeleteOnTab( nTab );
                if (pDPCollection)
                    pDPCollection->DeleteOnTab( nTab );
                if (pDetOpList)
                    pDetOpList->DeleteOnTab( nTab );
                DeleteAreaLinksOnTab( nTab );

                // normal reference update

                aRange.aEnd.SetTab( MAXTAB );
                xColNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,-1 );
                xRowNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,-1 );
                if (pRangeName)
                    pRangeName->UpdateTabRef( nTab, 2 );
                pDBCollection->UpdateReference(
                                    URM_INSDEL, 0,0,nTab, MAXCOL,MAXROW,MAXTAB, 0,0,-1 );
                if (pDPCollection)
                    pDPCollection->UpdateReference( URM_INSDEL, aRange, 0,0,-1 );
                if (pDetOpList)
                    pDetOpList->UpdateReference( this, URM_INSDEL, aRange, 0,0,-1 );
                UpdateChartRef( URM_INSDEL, 0,0,nTab, MAXCOL,MAXROW,MAXTAB, 0,0,-1 );
                UpdateRefAreaLinks( URM_INSDEL, aRange, 0,0,-1 );
                if ( pCondFormList )
                    pCondFormList->UpdateReference( URM_INSDEL, aRange, 0,0,-1 );
                if ( pValidationList )
                    pValidationList->UpdateReference( URM_INSDEL, aRange, 0,0,-1 );
                if ( pUnoBroadcaster )
                    pUnoBroadcaster->Broadcast( ScUpdateRefHint( URM_INSDEL, aRange, 0,0,-1 ) );

                SCTAB i;
                for (i=0; i<=MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->UpdateDeleteTab(nTab,false,
                                    pRefUndoDoc ? pRefUndoDoc->pTab[i] : 0);
                delete pTab[nTab];
                for (i=nTab + 1; i < nTabCount; i++)
                    pTab[i - 1] = pTab[i];

                pTab[nTabCount - 1] = NULL;
                --nMaxTableNumber;
                // UpdateBroadcastAreas must be called between UpdateDeleteTab,
                // which ends listening, and StartAllListeners, to not modify
                // areas that are to be inserted by starting listeners.
                UpdateBroadcastAreas( URM_INSDEL, aRange, 0,0,-1);
                for (i = 0; i <= MAXTAB; i++)
                    if (pTab[i])
                        pTab[i]->UpdateCompile();
                // Excel-Filter deletes some Tables while loading, Listeners will
                // only be triggered after the loading is done.
                if ( !bInsertingFromOtherDoc )
                {
                    for (i = 0; i <= MAXTAB; i++)
                        if (pTab[i])
                            pTab[i]->StartAllListeners();
                    SetDirty();
                }
                // sheet names of references are not valid until sheet is deleted
                pChartListenerCollection->UpdateScheduledSeriesRanges();

                SetAutoCalc( bOldAutoCalc );
                bValid = sal_True;
            }
        }
    }
    return bValid;
}


sal_Bool ScDocument::RenameTab( SCTAB nTab, const String& rName, sal_Bool /* bUpdateRef */,
        sal_Bool bExternalDocument )
{
    sal_Bool    bValid = false;
    SCTAB   i;
    if VALIDTAB(nTab)
        if (pTab[nTab])
        {
            if ( bExternalDocument )
                bValid = true;      // composed name
            else
                bValid = ValidTabName(rName);
            for (i=0; (i<=MAXTAB) && bValid; i++)
                if (pTab[i] && (i != nTab))
                {
                    String aOldName;
                    pTab[i]->GetName(aOldName);
                    bValid = !ScGlobal::GetpTransliteration()->isEqual( rName, aOldName );
                }
            if (bValid)
            {
                // #i75258# update charts before renaming, so they can get their live data objects.
                // Once the charts are live, the sheet can be renamed without problems.
                if ( pChartListenerCollection )
                    pChartListenerCollection->UpdateChartsContainingTab( nTab );
                pTab[nTab]->SetName(rName);

                // If formulas refer to the renamed sheet, the TokenArray remains valid,
                // but the XML stream must be re-generated.
                for (i=0; i<=MAXTAB; ++i)
                    if (pTab[i] && pTab[i]->IsStreamValid())
                        pTab[i]->SetStreamValid( false );
            }
        }
    return bValid;
}


void ScDocument::SetVisible( SCTAB nTab, sal_Bool bVisible )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->SetVisible(bVisible);
}


sal_Bool ScDocument::IsVisible( SCTAB nTab ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->IsVisible();

    return false;
}


sal_Bool ScDocument::IsStreamValid( SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->IsStreamValid();

    return false;
}


void ScDocument::SetStreamValid( SCTAB nTab, sal_Bool bSet, sal_Bool bIgnoreLock )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->SetStreamValid( bSet, bIgnoreLock );
}


void ScDocument::LockStreamValid( bool bLock )
{
    mbStreamValidLocked = bLock;
}


sal_Bool ScDocument::IsPendingRowHeights( SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->IsPendingRowHeights();

    return false;
}


void ScDocument::SetPendingRowHeights( SCTAB nTab, sal_Bool bSet )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->SetPendingRowHeights( bSet );
}


void ScDocument::SetLayoutRTL( SCTAB nTab, sal_Bool bRTL )
{
    if ( ValidTab(nTab)  && pTab[nTab] )
    {
        if ( bImportingXML )
        {
            // #i57869# only set the LoadingRTL flag, the real setting (including mirroring)
            // is applied in SetImportingXML(sal_False). This is so the shapes can be loaded in
            // normal LTR mode.

            pTab[nTab]->SetLoadingRTL( bRTL );
            return;
        }

        pTab[nTab]->SetLayoutRTL( bRTL );       // only sets the flag
        pTab[nTab]->SetDrawPageSize();

        //  mirror existing objects:

        if (pDrawLayer)
        {
            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
            OSL_ENSURE(pPage,"Page ?");
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

                    pObject->SetContextWritingMode( bRTL ? WritingMode2::RL_TB : WritingMode2::LR_TB );

                    pObject = aIter.Next();
                }
            }
        }
    }
}


sal_Bool ScDocument::IsLayoutRTL( SCTAB nTab ) const
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        return pTab[nTab]->IsLayoutRTL();

    return false;
}


sal_Bool ScDocument::IsNegativePage( SCTAB nTab ) const
{
    //  Negative page area is always used for RTL layout.
    //  The separate method is used to find all RTL handling of drawing objects.
    return IsLayoutRTL( nTab );
}


/* ----------------------------------------------------------------------------
    used search area:

    GetCellArea  - Only Data
    GetTableArea - Data / Attributes
    GetPrintArea - intended for character objects,
                    sweeps attributes all the way to bottom / right
---------------------------------------------------------------------------- */


sal_Bool ScDocument::GetCellArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->GetCellArea( rEndCol, rEndRow );

    rEndCol = 0;
    rEndRow = 0;
    return false;
}


sal_Bool ScDocument::GetTableArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->GetTableArea( rEndCol, rEndRow );

    rEndCol = 0;
    rEndRow = 0;
    return false;
}

bool ScDocument::ShrinkToDataArea(SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow) const
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return false;

    SCCOL nCol1, nCol2;
    SCROW nRow1, nRow2;
    pTab[nTab]->GetFirstDataPos(nCol1, nRow1);
    pTab[nTab]->GetLastDataPos(nCol2, nRow2);

    if (nCol1 > nCol2 || nRow1 > nRow2)
        // invalid range.
        return false;

    // Make sure the area only shrinks, and doesn't grow.
    if (rStartCol < nCol1)
        rStartCol = nCol1;
    if (nCol2 < rEndCol)
        rEndCol = nCol2;
    if (rStartRow < nRow1)
        rStartRow = nRow1;
    if (nRow2 < rEndRow)
        rEndRow = nRow2;

    if (rStartCol > rEndCol || rStartRow > rEndRow)
        // invalid range.
        return false;

    return true;  // success!
}

bool ScDocument::ShrinkToUsedDataArea( bool& o_bShrunk, SCTAB nTab, SCCOL& rStartCol,
        SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow, bool bColumnsOnly ) const
{
    if (!ValidTab(nTab) || !pTab[nTab])
    {
        o_bShrunk = false;
        return false;
    }
    return pTab[nTab]->ShrinkToUsedDataArea( o_bShrunk, rStartCol, rStartRow, rEndCol, rEndRow, bColumnsOnly);
}

// connected area

void ScDocument::GetDataArea( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow,
                              SCCOL& rEndCol, SCROW& rEndRow, sal_Bool bIncludeOld, bool bOnlyDown ) const
{
    if (ValidTab(nTab) && pTab[nTab])
        pTab[nTab]->GetDataArea( rStartCol, rStartRow, rEndCol, rEndRow, bIncludeOld, bOnlyDown );
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
        for ( size_t i = 0, nCount = rRangeList->size(); i < nCount; i++ )
        {
            ScRange aRange( *(*rRangeList)[i] );
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
    {
        OSL_FAIL("LimitChartIfAll: Ref==0");
    }
    rRangeList = aNew;
}


void lcl_GetFirstTabRange( SCTAB& rTabRangeStart, SCTAB& rTabRangeEnd, const ScMarkData* pTabMark )
{
    // without ScMarkData, leave start/end unchanged
    if ( pTabMark )
    {
        for (SCTAB nTab=0; nTab<=MAXTAB; ++nTab)
            if (pTabMark->GetTableSelect(nTab))
            {
                // find first range of consecutive selected sheets
                rTabRangeStart = nTab;
                while ( nTab+1 <= MAXTAB && pTabMark->GetTableSelect(nTab+1) )
                    ++nTab;
                rTabRangeEnd = nTab;
                return;
            }
    }
}

bool lcl_GetNextTabRange( SCTAB& rTabRangeStart, SCTAB& rTabRangeEnd, const ScMarkData* pTabMark )
{
    if ( pTabMark )
    {
        // find next range of consecutive selected sheets after rTabRangeEnd
        for (SCTAB nTab=rTabRangeEnd+1; nTab<=MAXTAB; ++nTab)
            if (pTabMark->GetTableSelect(nTab))
            {
                rTabRangeStart = nTab;
                while ( nTab+1 <= MAXTAB && pTabMark->GetTableSelect(nTab+1) )
                    ++nTab;
                rTabRangeEnd = nTab;
                return true;
            }
    }
    return false;
}


sal_Bool ScDocument::CanInsertRow( const ScRange& rRange ) const
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

    sal_Bool bTest = sal_True;
    for (SCTAB i=nStartTab; i<=nEndTab && bTest; i++)
        if (pTab[i])
            bTest &= pTab[i]->TestInsertRow( nStartCol, nEndCol, nSize );

    return bTest;
}


sal_Bool ScDocument::InsertRow( SCCOL nStartCol, SCTAB nStartTab,
                            SCCOL nEndCol,   SCTAB nEndTab,
                            SCROW nStartRow, SCSIZE nSize, ScDocument* pRefUndoDoc,
                            const ScMarkData* pTabMark )
{
    SCTAB i;

    PutInOrder( nStartCol, nEndCol );
    PutInOrder( nStartTab, nEndTab );
    if ( pTabMark )
    {
        nStartTab = 0;
        nEndTab = MAXTAB;
    }

    sal_Bool bTest = true;
    sal_Bool bRet = false;
    sal_Bool bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( false );   // avoid mulitple calculations
    for ( i = nStartTab; i <= nEndTab && bTest; i++)
        if (pTab[i] && (!pTabMark || pTabMark->GetTableSelect(i)))
            bTest &= pTab[i]->TestInsertRow( nStartCol, nEndCol, nSize );
    if (bTest)
    {
        // UpdateBroadcastAreas have to be called before UpdateReference, so that entries
        // aren't shifted that would be rebuild at UpdateReference

        // handle chunks of consecutive selected sheets together
        SCTAB nTabRangeStart = nStartTab;
        SCTAB nTabRangeEnd = nEndTab;
        lcl_GetFirstTabRange( nTabRangeStart, nTabRangeEnd, pTabMark );
        do
        {
            UpdateBroadcastAreas( URM_INSDEL, ScRange(
                ScAddress( nStartCol, nStartRow, nTabRangeStart ),
                ScAddress( nEndCol, MAXROW, nTabRangeEnd )), 0, static_cast<SCsROW>(nSize), 0 );
        }
        while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark ) );

        lcl_GetFirstTabRange( nTabRangeStart, nTabRangeEnd, pTabMark );
        do
        {
            UpdateReference( URM_INSDEL, nStartCol, nStartRow, nTabRangeStart,
                             nEndCol, MAXROW, nTabRangeEnd,
                             0, static_cast<SCsROW>(nSize), 0, pRefUndoDoc, false );        // without drawing objects
        }
        while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark ) );

        for (i=nStartTab; i<=nEndTab; i++)
            if (pTab[i] && (!pTabMark || pTabMark->GetTableSelect(i)))
                pTab[i]->InsertRow( nStartCol, nEndCol, nStartRow, nSize );

        //  UpdateRef for drawing layer must be after inserting,
        //  when the new row heights are known.
        for (i=nStartTab; i<=nEndTab; i++)
            if (pTab[i] && (!pTabMark || pTabMark->GetTableSelect(i)))
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
        {   // Listeners have been removed in UpdateReference
            for (i=0; i<=MAXTAB; i++)
                if (pTab[i])
                    pTab[i]->StartNeededListeners();
            // at least all cells using range names pointing relative
            // to the moved range must recalculate
            for (i=0; i<=MAXTAB; i++)
                if (pTab[i])
                    pTab[i]->SetRelNameDirty();
        }
        bRet = sal_True;
    }
    SetAutoCalc( bOldAutoCalc );
    if ( bRet )
        pChartListenerCollection->UpdateDirtyCharts();
    return bRet;
}


sal_Bool ScDocument::InsertRow( const ScRange& rRange, ScDocument* pRefUndoDoc )
{
    return InsertRow( rRange.aStart.Col(), rRange.aStart.Tab(),
                      rRange.aEnd.Col(),   rRange.aEnd.Tab(),
                      rRange.aStart.Row(), static_cast<SCSIZE>(rRange.aEnd.Row()-rRange.aStart.Row()+1),
                      pRefUndoDoc );
}


void ScDocument::DeleteRow( SCCOL nStartCol, SCTAB nStartTab,
                            SCCOL nEndCol,   SCTAB nEndTab,
                            SCROW nStartRow, SCSIZE nSize,
                            ScDocument* pRefUndoDoc, sal_Bool* pUndoOutline,
                            const ScMarkData* pTabMark )
{
    SCTAB i;

    PutInOrder( nStartCol, nEndCol );
    PutInOrder( nStartTab, nEndTab );
    if ( pTabMark )
    {
        nStartTab = 0;
        nEndTab = MAXTAB;
    }

    sal_Bool bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( false );   // avoid multiple calculations

    // handle chunks of consecutive selected sheets together
    SCTAB nTabRangeStart = nStartTab;
    SCTAB nTabRangeEnd = nEndTab;
    lcl_GetFirstTabRange( nTabRangeStart, nTabRangeEnd, pTabMark );
    do
    {
        if ( ValidRow(nStartRow+nSize) )
        {
            DelBroadcastAreasInRange( ScRange(
                ScAddress( nStartCol, nStartRow, nTabRangeStart ),
                ScAddress( nEndCol, nStartRow+nSize-1, nTabRangeEnd ) ) );
            UpdateBroadcastAreas( URM_INSDEL, ScRange(
                ScAddress( nStartCol, nStartRow+nSize, nTabRangeStart ),
                ScAddress( nEndCol, MAXROW, nTabRangeEnd )), 0, -(static_cast<SCsROW>(nSize)), 0 );
        }
        else
            DelBroadcastAreasInRange( ScRange(
                ScAddress( nStartCol, nStartRow, nTabRangeStart ),
                ScAddress( nEndCol, MAXROW, nTabRangeEnd ) ) );
    }
    while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark ) );

    if ( ValidRow(nStartRow+nSize) )
    {
        lcl_GetFirstTabRange( nTabRangeStart, nTabRangeEnd, pTabMark );
        do
        {
            UpdateReference( URM_INSDEL, nStartCol, nStartRow+nSize, nTabRangeStart,
                             nEndCol, MAXROW, nTabRangeEnd,
                             0, -(static_cast<SCsROW>(nSize)), 0, pRefUndoDoc, sal_True, false );
        }
        while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark ) );
    }

    if (pUndoOutline)
        *pUndoOutline = false;

    for ( i = nStartTab; i <= nEndTab; i++)
        if (pTab[i] && (!pTabMark || pTabMark->GetTableSelect(i)))
            pTab[i]->DeleteRow( nStartCol, nEndCol, nStartRow, nSize, pUndoOutline );

    if ( ValidRow(nStartRow+nSize) )
    {   // Listeners have been removed in UpdateReference
        for (i=0; i<=MAXTAB; i++)
            if (pTab[i])
                pTab[i]->StartNeededListeners();
        // at least all cells using range names pointing relative to
        // the moved range must recalculate
        for (i=0; i<=MAXTAB; i++)
            if (pTab[i])
                pTab[i]->SetRelNameDirty();
    }

    SetAutoCalc( bOldAutoCalc );
    pChartListenerCollection->UpdateDirtyCharts();
}


void ScDocument::DeleteRow( const ScRange& rRange, ScDocument* pRefUndoDoc, sal_Bool* pUndoOutline )
{
    DeleteRow( rRange.aStart.Col(), rRange.aStart.Tab(),
               rRange.aEnd.Col(),   rRange.aEnd.Tab(),
               rRange.aStart.Row(), static_cast<SCSIZE>(rRange.aEnd.Row()-rRange.aStart.Row()+1),
               pRefUndoDoc, pUndoOutline );
}


sal_Bool ScDocument::CanInsertCol( const ScRange& rRange ) const
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

    sal_Bool bTest = sal_True;
    for (SCTAB i=nStartTab; i<=nEndTab && bTest; i++)
        if (pTab[i])
            bTest &= pTab[i]->TestInsertCol( nStartRow, nEndRow, nSize );

    return bTest;
}


sal_Bool ScDocument::InsertCol( SCROW nStartRow, SCTAB nStartTab,
                            SCROW nEndRow,   SCTAB nEndTab,
                            SCCOL nStartCol, SCSIZE nSize, ScDocument* pRefUndoDoc,
                            const ScMarkData* pTabMark )
{
    SCTAB i;

    PutInOrder( nStartRow, nEndRow );
    PutInOrder( nStartTab, nEndTab );
    if ( pTabMark )
    {
        nStartTab = 0;
        nEndTab = MAXTAB;
    }

    sal_Bool bTest = true;
    sal_Bool bRet = false;
    sal_Bool bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( false );   // avoid multiple calculations
    for ( i = nStartTab; i <= nEndTab && bTest; i++)
        if (pTab[i] && (!pTabMark || pTabMark->GetTableSelect(i)))
            bTest &= pTab[i]->TestInsertCol( nStartRow, nEndRow, nSize );
    if (bTest)
    {
        // handle chunks of consecutive selected sheets together
        SCTAB nTabRangeStart = nStartTab;
        SCTAB nTabRangeEnd = nEndTab;
        lcl_GetFirstTabRange( nTabRangeStart, nTabRangeEnd, pTabMark );
        do
        {
            UpdateBroadcastAreas( URM_INSDEL, ScRange(
                ScAddress( nStartCol, nStartRow, nTabRangeStart ),
                ScAddress( MAXCOL, nEndRow, nTabRangeEnd )), static_cast<SCsCOL>(nSize), 0, 0 );
        }
        while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark ) );

        lcl_GetFirstTabRange( nTabRangeStart, nTabRangeEnd, pTabMark );
        do
        {
            UpdateReference( URM_INSDEL, nStartCol, nStartRow, nTabRangeStart,
                             MAXCOL, nEndRow, nTabRangeEnd,
                             static_cast<SCsCOL>(nSize), 0, 0, pRefUndoDoc, sal_True, false );
        }
        while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark ) );

        for (i=nStartTab; i<=nEndTab; i++)
            if (pTab[i] && (!pTabMark || pTabMark->GetTableSelect(i)))
                pTab[i]->InsertCol( nStartCol, nStartRow, nEndRow, nSize );

        if ( pChangeTrack && pChangeTrack->IsInDeleteUndo() )
        {   // durch Restaurierung von Referenzen auf geloeschte Bereiche ist
            // ein neues Listening faellig, bisherige Listener wurden in
            // FormulaCell UpdateReference abgehaengt
            StartAllListeners();
        }
        else
        {   // Listeners have been removed in UpdateReference
            for (i=0; i<=MAXTAB; i++)
                if (pTab[i])
                    pTab[i]->StartNeededListeners();
            // at least all cells using range names pointing relative
            // to the moved range must recalculate
            for (i=0; i<=MAXTAB; i++)
                if (pTab[i])
                    pTab[i]->SetRelNameDirty();
        }
        bRet = sal_True;
    }
    SetAutoCalc( bOldAutoCalc );
    if ( bRet )
        pChartListenerCollection->UpdateDirtyCharts();
    return bRet;
}


sal_Bool ScDocument::InsertCol( const ScRange& rRange, ScDocument* pRefUndoDoc )
{
    return InsertCol( rRange.aStart.Row(), rRange.aStart.Tab(),
                      rRange.aEnd.Row(),   rRange.aEnd.Tab(),
                      rRange.aStart.Col(), static_cast<SCSIZE>(rRange.aEnd.Col()-rRange.aStart.Col()+1),
                      pRefUndoDoc );
}


void ScDocument::DeleteCol(SCROW nStartRow, SCTAB nStartTab, SCROW nEndRow, SCTAB nEndTab,
                                SCCOL nStartCol, SCSIZE nSize, ScDocument* pRefUndoDoc,
                                sal_Bool* pUndoOutline, const ScMarkData* pTabMark )
{
    SCTAB i;

    PutInOrder( nStartRow, nEndRow );
    PutInOrder( nStartTab, nEndTab );
    if ( pTabMark )
    {
        nStartTab = 0;
        nEndTab = MAXTAB;
    }

    sal_Bool bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( false );   // avoid multiple calculations

    // handle chunks of consecutive selected sheets together
    SCTAB nTabRangeStart = nStartTab;
    SCTAB nTabRangeEnd = nEndTab;
    lcl_GetFirstTabRange( nTabRangeStart, nTabRangeEnd, pTabMark );
    do
    {
        if ( ValidCol(sal::static_int_cast<SCCOL>(nStartCol+nSize)) )
        {
            DelBroadcastAreasInRange( ScRange(
                ScAddress( nStartCol, nStartRow, nTabRangeStart ),
                ScAddress( sal::static_int_cast<SCCOL>(nStartCol+nSize-1), nEndRow, nTabRangeEnd ) ) );
            UpdateBroadcastAreas( URM_INSDEL, ScRange(
                ScAddress( sal::static_int_cast<SCCOL>(nStartCol+nSize), nStartRow, nTabRangeStart ),
                ScAddress( MAXCOL, nEndRow, nTabRangeEnd )), -static_cast<SCsCOL>(nSize), 0, 0 );
        }
        else
            DelBroadcastAreasInRange( ScRange(
                ScAddress( nStartCol, nStartRow, nTabRangeStart ),
                ScAddress( MAXCOL, nEndRow, nTabRangeEnd ) ) );
    }
    while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark ) );

    if ( ValidCol(sal::static_int_cast<SCCOL>(nStartCol+nSize)) )
    {
        lcl_GetFirstTabRange( nTabRangeStart, nTabRangeEnd, pTabMark );
        do
        {
            UpdateReference( URM_INSDEL, sal::static_int_cast<SCCOL>(nStartCol+nSize), nStartRow, nTabRangeStart,
                             MAXCOL, nEndRow, nTabRangeEnd,
                             -static_cast<SCsCOL>(nSize), 0, 0, pRefUndoDoc, sal_True, false );
        }
        while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark ) );
    }

    if (pUndoOutline)
        *pUndoOutline = false;

    for ( i = nStartTab; i <= nEndTab; i++)
        if (pTab[i] && (!pTabMark || pTabMark->GetTableSelect(i)))
            pTab[i]->DeleteCol( nStartCol, nStartRow, nEndRow, nSize, pUndoOutline );

    if ( ValidCol(sal::static_int_cast<SCCOL>(nStartCol+nSize)) )
    {   // Listeners have been removed in UpdateReference
        for (i=0; i<=MAXTAB; i++)
            if (pTab[i])
                pTab[i]->StartNeededListeners();
        // at least all cells using range names pointing relative to
        // the moved range must recalculate
        for (i=0; i<=MAXTAB; i++)
            if (pTab[i])
                pTab[i]->SetRelNameDirty();
    }

    SetAutoCalc( bOldAutoCalc );
    pChartListenerCollection->UpdateDirtyCharts();
}


void ScDocument::DeleteCol( const ScRange& rRange, ScDocument* pRefUndoDoc, sal_Bool* pUndoOutline )
{
    DeleteCol( rRange.aStart.Row(), rRange.aStart.Tab(),
               rRange.aEnd.Row(),   rRange.aEnd.Tab(),
               rRange.aStart.Col(), static_cast<SCSIZE>(rRange.aEnd.Col()-rRange.aStart.Col()+1),
               pRefUndoDoc, pUndoOutline );
}


//  fuer Area-Links: Zellen einuegen/loeschen, wenn sich der Bereich veraendert
//  (ohne Paint)


void lcl_GetInsDelRanges( const ScRange& rOld, const ScRange& rNew,
                            ScRange& rColRange, sal_Bool& rInsCol, sal_Bool& rDelCol,
                            ScRange& rRowRange, sal_Bool& rInsRow, sal_Bool& rDelRow )
{
    OSL_ENSURE( rOld.aStart == rNew.aStart, "FitBlock: Beginning is different" );

    rInsCol = rDelCol = rInsRow = rDelRow = false;

    SCCOL nStartX = rOld.aStart.Col();
    SCROW nStartY = rOld.aStart.Row();
    SCCOL nOldEndX = rOld.aEnd.Col();
    SCROW nOldEndY = rOld.aEnd.Row();
    SCCOL nNewEndX = rNew.aEnd.Col();
    SCROW nNewEndY = rNew.aEnd.Row();
    SCTAB nTab = rOld.aStart.Tab();

    //  wenn es mehr Zeilen werden, werden Spalten auf der alten Hoehe eingefuegt/geloescht
    sal_Bool bGrowY = ( nNewEndY > nOldEndY );
    SCROW nColEndY = bGrowY ? nOldEndY : nNewEndY;
    SCCOL nRowEndX = bGrowY ? nNewEndX : nOldEndX;

    //  Spalten

    if ( nNewEndX > nOldEndX )          // Spalten einfuegen
    {
        rColRange = ScRange( nOldEndX+1, nStartY, nTab, nNewEndX, nColEndY, nTab );
        rInsCol = sal_True;
    }
    else if ( nNewEndX < nOldEndX )     // Spalten loeschen
    {
        rColRange = ScRange( nNewEndX+1, nStartY, nTab, nOldEndX, nColEndY, nTab );
        rDelCol = sal_True;
    }

    //  Zeilen

    if ( nNewEndY > nOldEndY )          // Zeilen einfuegen
    {
        rRowRange = ScRange( nStartX, nOldEndY+1, nTab, nRowEndX, nNewEndY, nTab );
        rInsRow = sal_True;
    }
    else if ( nNewEndY < nOldEndY )     // Zeilen loeschen
    {
        rRowRange = ScRange( nStartX, nNewEndY+1, nTab, nRowEndX, nOldEndY, nTab );
        rDelRow = sal_True;
    }
}


sal_Bool ScDocument::HasPartOfMerged( const ScRange& rRange )
{
    sal_Bool bPart = false;
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


sal_Bool ScDocument::CanFitBlock( const ScRange& rOld, const ScRange& rNew )
{
    if ( rOld == rNew )
        return sal_True;

    sal_Bool bOk = sal_True;
    sal_Bool bInsCol,bDelCol,bInsRow,bDelRow;
    ScRange aColRange,aRowRange;
    lcl_GetInsDelRanges( rOld, rNew, aColRange,bInsCol,bDelCol, aRowRange,bInsRow,bDelRow );

    if ( bInsCol && !CanInsertCol( aColRange ) )            // Zellen am Rand ?
        bOk = false;
    if ( bInsRow && !CanInsertRow( aRowRange ) )            // Zellen am Rand ?
        bOk = false;

    if ( bInsCol || bDelCol )
    {
        aColRange.aEnd.SetCol(MAXCOL);
        if ( HasPartOfMerged(aColRange) )
            bOk = false;
    }
    if ( bInsRow || bDelRow )
    {
        aRowRange.aEnd.SetRow(MAXROW);
        if ( HasPartOfMerged(aRowRange) )
            bOk = false;
    }

    return bOk;
}


void ScDocument::FitBlock( const ScRange& rOld, const ScRange& rNew, sal_Bool bClear )
{
    if (bClear)
        DeleteAreaTab( rOld, IDF_ALL );

    sal_Bool bInsCol,bDelCol,bInsRow,bDelRow;
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
                            const ScMarkData& rMark, sal_uInt16 nDelFlag)
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    sal_Bool bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( false );   // avoid multiple calculations
    for (SCTAB i = 0; i <= MAXTAB; i++)
        if (pTab[i])
            if ( rMark.GetTableSelect(i) || bIsUndo )
                pTab[i]->DeleteArea(nCol1, nRow1, nCol2, nRow2, nDelFlag);
    SetAutoCalc( bOldAutoCalc );
}


void ScDocument::DeleteAreaTab(SCCOL nCol1, SCROW nRow1,
                                SCCOL nCol2, SCROW nRow2,
                                SCTAB nTab, sal_uInt16 nDelFlag)
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    if ( VALIDTAB(nTab) && pTab[nTab] )
    {
        sal_Bool bOldAutoCalc = GetAutoCalc();
        SetAutoCalc( false );   // avoid multiple calculations
        pTab[nTab]->DeleteArea(nCol1, nRow1, nCol2, nRow2, nDelFlag);
        SetAutoCalc( bOldAutoCalc );
    }
}


void ScDocument::DeleteAreaTab( const ScRange& rRange, sal_uInt16 nDelFlag )
{
    for ( SCTAB nTab = rRange.aStart.Tab(); nTab <= rRange.aEnd.Tab(); nTab++ )
        DeleteAreaTab( rRange.aStart.Col(), rRange.aStart.Row(),
                       rRange.aEnd.Col(),   rRange.aEnd.Row(),
                       nTab, nDelFlag );
}


void ScDocument::InitUndoSelected( ScDocument* pSrcDoc, const ScMarkData& rTabSelection,
                                sal_Bool bColInfo, sal_Bool bRowInfo )
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
        {
        OSL_FAIL("InitUndo");
        }
}


void ScDocument::InitUndo( ScDocument* pSrcDoc, SCTAB nTab1, SCTAB nTab2,
                                sal_Bool bColInfo, sal_Bool bRowInfo )
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
    {
        OSL_FAIL("InitUndo");
    }
}


void ScDocument::AddUndoTab( SCTAB nTab1, SCTAB nTab2, sal_Bool bColInfo, sal_Bool bRowInfo )
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
    {
        OSL_FAIL("InitUndo");
    }
}


void ScDocument::SetCutMode( sal_Bool bVal )
{
    if (bIsClip)
        GetClipParam().mbCutMode = bVal;
    else
    {
        OSL_FAIL("SetCutMode without bIsClip");
    }
}


sal_Bool ScDocument::IsCutMode()
{
    if (bIsClip)
        return GetClipParam().mbCutMode;
    else
    {
        OSL_FAIL("IsCutMode ohne bIsClip");
        return false;
    }
}


void ScDocument::CopyToDocument(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                            SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                            sal_uInt16 nFlags, sal_Bool bOnlyMarked, ScDocument* pDestDoc,
                            const ScMarkData* pMarks, sal_Bool bColRowFlags )
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    PutInOrder( nTab1, nTab2 );
    if( !pDestDoc->aDocName.Len() )
        pDestDoc->aDocName = aDocName;
    if (VALIDTAB(nTab1) && VALIDTAB(nTab2))
    {
        sal_Bool bOldAutoCalc = pDestDoc->GetAutoCalc();
        pDestDoc->SetAutoCalc( false );     // avoid multiple calculations
        for (SCTAB i = nTab1; i <= nTab2; i++)
        {
            if (pTab[i] && pDestDoc->pTab[i])
                pTab[i]->CopyToTable( nCol1, nRow1, nCol2, nRow2, nFlags,
                                      bOnlyMarked, pDestDoc->pTab[i], pMarks,
                                      false, bColRowFlags );
        }
        pDestDoc->SetAutoCalc( bOldAutoCalc );
    }
}


void ScDocument::UndoToDocument(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                            SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                            sal_uInt16 nFlags, sal_Bool bOnlyMarked, ScDocument* pDestDoc,
                            const ScMarkData* pMarks)
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    PutInOrder( nTab1, nTab2 );
    if (VALIDTAB(nTab1) && VALIDTAB(nTab2))
    {
        sal_Bool bOldAutoCalc = pDestDoc->GetAutoCalc();
        pDestDoc->SetAutoCalc( false );     // avoid multiple calculations
        if (nTab1 > 0)
            CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTab1-1, IDF_FORMULA, false, pDestDoc, pMarks );

        for (SCTAB i = nTab1; i <= nTab2; i++)
        {
            if (pTab[i] && pDestDoc->pTab[i])
                pTab[i]->UndoToTable(nCol1, nRow1, nCol2, nRow2, nFlags,
                                    bOnlyMarked, pDestDoc->pTab[i], pMarks);
        }

        if (nTab2 < MAXTAB)
            CopyToDocument( 0,0,nTab2+1, MAXCOL,MAXROW,MAXTAB, IDF_FORMULA, false, pDestDoc, pMarks );
        pDestDoc->SetAutoCalc( bOldAutoCalc );
    }
}


void ScDocument::CopyToDocument(const ScRange& rRange,
                            sal_uInt16 nFlags, sal_Bool bOnlyMarked, ScDocument* pDestDoc,
                            const ScMarkData* pMarks, sal_Bool bColRowFlags)
{
    ScRange aNewRange = rRange;
    aNewRange.Justify();

    if( !pDestDoc->aDocName.Len() )
        pDestDoc->aDocName = aDocName;
    sal_Bool bOldAutoCalc = pDestDoc->GetAutoCalc();
    pDestDoc->SetAutoCalc( false );     // avoid multiple calculations
    for (SCTAB i = aNewRange.aStart.Tab(); i <= aNewRange.aEnd.Tab(); i++)
        if (pTab[i] && pDestDoc->pTab[i])
            pTab[i]->CopyToTable(aNewRange.aStart.Col(), aNewRange.aStart.Row(),
                                 aNewRange.aEnd.Col(), aNewRange.aEnd.Row(),
                                 nFlags, bOnlyMarked, pDestDoc->pTab[i],
                                 pMarks, false, bColRowFlags);
    pDestDoc->SetAutoCalc( bOldAutoCalc );
}


void ScDocument::UndoToDocument(const ScRange& rRange,
                            sal_uInt16 nFlags, sal_Bool bOnlyMarked, ScDocument* pDestDoc,
                            const ScMarkData* pMarks)
{
    ScRange aNewRange = rRange;
    aNewRange.Justify();
    SCTAB nTab1 = aNewRange.aStart.Tab();
    SCTAB nTab2 = aNewRange.aEnd.Tab();

    sal_Bool bOldAutoCalc = pDestDoc->GetAutoCalc();
    pDestDoc->SetAutoCalc( false );     // avoid multiple calculations
    if (nTab1 > 0)
        CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTab1-1, IDF_FORMULA, false, pDestDoc, pMarks );

    for (SCTAB i = nTab1; i <= nTab2; i++)
    {
        if (pTab[i] && pDestDoc->pTab[i])
            pTab[i]->UndoToTable(aNewRange.aStart.Col(), aNewRange.aStart.Row(),
                                    aNewRange.aEnd.Col(), aNewRange.aEnd.Row(),
                                    nFlags, bOnlyMarked, pDestDoc->pTab[i], pMarks);
    }

    if (nTab2 < MAXTAB)
        CopyToDocument( 0,0,nTab2+1, MAXCOL,MAXROW,MAXTAB, IDF_FORMULA, false, pDestDoc, pMarks );
    pDestDoc->SetAutoCalc( bOldAutoCalc );
}

void ScDocument::CopyToClip(const ScClipParam& rClipParam,
                            ScDocument* pClipDoc, const ScMarkData* pMarks,
                            bool bAllTabs, bool bKeepScenarioFlags, bool bIncludeObjects, bool bCloneNoteCaptions)
{
    OSL_ENSURE( bAllTabs || pMarks, "CopyToClip: ScMarkData fails" );

    if (bIsClip)
        return;

    if (!pClipDoc)
    {
        OSL_TRACE("CopyToClip: no ClipDoc");
        pClipDoc = SC_MOD()->GetClipDoc();
    }

    pClipDoc->aDocName = aDocName;
    pClipDoc->SetClipParam(rClipParam);
    pClipDoc->ResetClip(this, pMarks);

    ScRange aClipRange = rClipParam.getWholeRange();
    CopyRangeNamesToClip(pClipDoc, aClipRange, pMarks, bAllTabs);

    for (SCTAB i = 0; i <= MAXTAB; ++i)
    {
        if (!pTab[i] || !pClipDoc->pTab[i])
            continue;

        if (pMarks && !pMarks->GetTableSelect(i))
            continue;

        pTab[i]->CopyToClip(rClipParam.maRanges, pClipDoc->pTab[i], bKeepScenarioFlags, bCloneNoteCaptions);

        if (pDrawLayer && bIncludeObjects)
        {
            //  also copy drawing objects
            Rectangle aObjRect = GetMMRect(
                aClipRange.aStart.Col(), aClipRange.aStart.Row(), aClipRange.aEnd.Col(), aClipRange.aEnd.Row(), i);
            pDrawLayer->CopyToClip(pClipDoc, i, aObjRect);
        }
    }

    // Make sure to mark overlapped cells.
    pClipDoc->ExtendMerge(aClipRange, true);
}

// Copy the content of the Range into clipboard. Adding this method for VBA API: Range.Copy().
void ScDocument::CopyToClip4VBA(const ScClipParam& rClipParam, ScDocument* pClipDoc, bool bKeepScenarioFlags, bool bIncludeObjects, bool bCloneNoteCaptions)
{
    if ( !bIsClip )
    {
        pClipDoc = pClipDoc ? pClipDoc : SC_MOD()->GetClipDoc();
        if ( !pClipDoc )
        {
            return;
        }
        ScRange aClipRange = rClipParam.getWholeRange();
        SCTAB nTab = aClipRange.aStart.Tab();
        pClipDoc->aDocName = aDocName;
        pClipDoc->SetClipParam( rClipParam );
        pClipDoc->ResetClip( this, nTab );

        CopyRangeNamesToClip( pClipDoc, aClipRange, nTab );

        if ( pTab[nTab] && pClipDoc->pTab[nTab] )
        {
            pTab[nTab]->CopyToClip( rClipParam.maRanges, pClipDoc->pTab[nTab], bKeepScenarioFlags, bCloneNoteCaptions );
            if ( pDrawLayer && bIncludeObjects )
            {
                // Also copy drawing objects.
                Rectangle aObjRect = GetMMRect( aClipRange.aStart.Col(), aClipRange.aStart.Row(), aClipRange.aEnd.Col(), aClipRange.aEnd.Row(), nTab );
                pDrawLayer->CopyToClip( pClipDoc, nTab, aObjRect );
            }
        }

        // Make sure to mark overlapped cells.
        pClipDoc->ExtendMerge( aClipRange, true );
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
            OSL_TRACE("CopyTabToClip: no ClipDoc");
            pClipDoc = SC_MOD()->GetClipDoc();
        }

        ScClipParam& rClipParam = pClipDoc->GetClipParam();
        pClipDoc->aDocName = aDocName;
        rClipParam.maRanges.RemoveAll();
        rClipParam.maRanges.Append(ScRange(nCol1, nRow1, 0, nCol2, nRow2, 0));
        pClipDoc->ResetClip( this, nTab );

        if (pTab[nTab] && pClipDoc->pTab[nTab])
            pTab[nTab]->CopyToClip(nCol1, nRow1, nCol2, nRow2, pClipDoc->pTab[nTab], false, sal_True);

        pClipDoc->GetClipParam().mbCutMode = false;
    }
}


void ScDocument::TransposeClip( ScDocument* pTransClip, sal_uInt16 nFlags, sal_Bool bAsLink )
{
    OSL_ENSURE( bIsClip && pTransClip && pTransClip->bIsClip,
                    "TransposeClip with wrong Document" );

        //  initialisieren
        //  -> pTransClip muss vor dem Original-Dokument geloescht werden!

    pTransClip->ResetClip(this, (ScMarkData*)NULL);     // alle

        //  Bereiche uebernehmen

    if (pRangeName)
    {
        pTransClip->GetRangeName()->clear();
        ScRangeName::const_iterator itr = pRangeName->begin(), itrEnd = pRangeName->end();
        for (; itr != itrEnd; ++itr)
        {
            sal_uInt16 nIndex = itr->GetIndex();
            ScRangeData* pData = new ScRangeData(*itr);
            if (!pTransClip->pRangeName->insert(pData))
                delete pData;
            else
                pData->SetIndex(nIndex);
        }
    }

    // The data

    ScRange aClipRange = GetClipParam().getWholeRange();
    if ( ValidRow(aClipRange.aEnd.Row()-aClipRange.aStart.Row()) )
    {
        for (SCTAB i=0; i<=MAXTAB; i++)
            if (pTab[i])
            {
                OSL_ENSURE( pTransClip->pTab[i], "TransposeClip: Table not there" );
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

        pTransClip->SetClipParam(GetClipParam());
        pTransClip->GetClipParam().transpose();
    }
    else
    {
        OSL_TRACE("TransposeClip: Too big");
    }

        //  Dies passiert erst beim Einfuegen...

    GetClipParam().mbCutMode = false;
}

namespace {

void copyUsedNamesToClip(ScRangeName* pClipRangeName, ScRangeName* pRangeName, const std::set<sal_uInt16>& rUsedNames)
{
    pClipRangeName->clear();
    ScRangeName::const_iterator itr = pRangeName->begin(), itrEnd = pRangeName->end();
    for (; itr != itrEnd; ++itr)        //! DB-Bereiche Pivot-Bereiche auch !!!
    {
        sal_uInt16 nIndex = itr->GetIndex();
        bool bInUse = (rUsedNames.count(nIndex) > 0);
        if (!bInUse)
            continue;

        ScRangeData* pData = new ScRangeData(*itr);
        if (!pClipRangeName->insert(pData))
            delete pData;
        else
            pData->SetIndex(nIndex);
    }
}

}

void ScDocument::CopyRangeNamesToClip(ScDocument* pClipDoc, const ScRange& rClipRange, const ScMarkData* pMarks, bool bAllTabs)
{
    if (!pRangeName || pRangeName->empty())
        return;

    std::set<sal_uInt16> aUsedNames;        // indexes of named ranges that are used in the copied cells
    for (SCTAB i = 0; i <= MAXTAB; ++i)
        if (pTab[i] && pClipDoc->pTab[i])
            if ( bAllTabs || !pMarks || pMarks->GetTableSelect(i) )
                pTab[i]->FindRangeNamesInUse(
                    rClipRange.aStart.Col(), rClipRange.aStart.Row(),
                    rClipRange.aEnd.Col(), rClipRange.aEnd.Row(), aUsedNames);

    copyUsedNamesToClip(pClipDoc->GetRangeName(), pRangeName, aUsedNames);
}

void ScDocument::CopyRangeNamesToClip(ScDocument* pClipDoc, const ScRange& rClipRange, SCTAB nTab)
{
    if (!pRangeName || pRangeName->empty())
        return;

    // Indexes of named ranges that are used in the copied cells
    std::set<sal_uInt16> aUsedNames;
    if ( pTab[nTab] && pClipDoc->pTab[nTab] )
    {
        pTab[nTab]->FindRangeNamesInUse(
            rClipRange.aStart.Col(), rClipRange.aStart.Row(),
            rClipRange.aEnd.Col(), rClipRange.aEnd.Row(), aUsedNames );
    }

    copyUsedNamesToClip(pClipDoc->GetRangeName(), pRangeName, aUsedNames);
}

ScDocument::NumFmtMergeHandler::NumFmtMergeHandler(ScDocument* pDoc, ScDocument* pSrcDoc) :
        mpDoc(pDoc)
{
    mpDoc->MergeNumberFormatter(pSrcDoc);
}

ScDocument::NumFmtMergeHandler::~NumFmtMergeHandler()
{
    mpDoc->pFormatExchangeList = NULL;
}

void ScDocument::MergeNumberFormatter(ScDocument* pSrcDoc)
{
    SvNumberFormatter* pThisFormatter = xPoolHelper->GetFormTable();
    SvNumberFormatter* pOtherFormatter = pSrcDoc->xPoolHelper->GetFormTable();
    if (pOtherFormatter && pOtherFormatter != pThisFormatter)
    {
        SvNumberFormatterIndexTable* pExchangeList =
                 pThisFormatter->MergeFormatter(*(pOtherFormatter));
        if (pExchangeList->Count() > 0)
            pFormatExchangeList = pExchangeList;
    }
}

void ScDocument::CopyRangeNamesFromClip(ScDocument* pClipDoc, ScClipRangeNameData& rRangeNames)
{
    if (!pClipDoc->pRangeName)
        return;

    ScClipRangeNameData aClipRangeNames;

    ScRangeName::const_iterator itr = pClipDoc->pRangeName->begin();
    ScRangeName::const_iterator itrEnd = pClipDoc->pRangeName->end();
    for (; itr != itrEnd; ++itr)        //! DB-Bereiche Pivot-Bereiche auch
    {
        /*  Copy only if the name doesn't exist in this document.
            If it exists we use the already existing name instead,
            another possibility could be to create new names if
            documents differ.
            A proper solution would ask the user how to proceed.
            The adjustment of the indices in the formulas is done later.
        */
        const ScRangeData* pExistingData = GetRangeName()->findByName(itr->GetName());
        if (pExistingData)
        {
            sal_uInt16 nOldIndex = itr->GetIndex();
            sal_uInt16 nNewIndex = pExistingData->GetIndex();
            aClipRangeNames.insert(nOldIndex, nNewIndex);
            if ( !aClipRangeNames.mbReplace )
                aClipRangeNames.mbReplace = ( nOldIndex != nNewIndex );
        }
        else
        {
            ScRangeData* pData = new ScRangeData( *itr );
            pData->SetDocument(this);
            if ( pRangeName->findByIndex( pData->GetIndex() ) )
                pData->SetIndex(0);     // need new index, done in Insert
            if ( pRangeName->insert(pData) )
            {
                aClipRangeNames.mpRangeNames.push_back(pData);
                sal_uInt16 nOldIndex = itr->GetIndex();
                sal_uInt16 nNewIndex = pData->GetIndex();
                aClipRangeNames.insert(nOldIndex, nNewIndex);
                if ( !aClipRangeNames.mbReplace )
                    aClipRangeNames.mbReplace = ( nOldIndex != nNewIndex );
            }
            else
            {   // must be an overflow
                delete pData;
                aClipRangeNames.insert(itr->GetIndex(), 0);
                aClipRangeNames.mbReplace = true;
            }
        }
    }
    rRangeNames = aClipRangeNames;
}

void ScDocument::UpdateRangeNamesInFormulas(
    ScClipRangeNameData& rRangeNames, const ScRangeList& rDestRanges, const ScMarkData& rMark,
    SCCOL nXw, SCROW nYw)
{
    // nXw and nYw are the extra width and height of the destination range
    // extended due to presence of merged cell(s).

    if (!rRangeNames.mbReplace)
        return;

    // first update all inserted named formulas if they contain other
    // range names and used indices changed
    for (size_t i = 0, n = rRangeNames.mpRangeNames.size(); i < n; ++i)        //! DB-Bereiche Pivot-Bereiche auch
    {
        rRangeNames.mpRangeNames[i]->ReplaceRangeNamesInUse(rRangeNames.maRangeMap);
    }
    // then update the formulas, they might need just the updated range names
    for ( size_t nRange = 0, n = rDestRanges.size(); nRange < n; ++nRange )
    {
        const ScRange* pRange = rDestRanges[nRange];
        SCCOL nCol1 = pRange->aStart.Col();
        SCROW nRow1 = pRange->aStart.Row();
        SCCOL nCol2 = pRange->aEnd.Col();
        SCROW nRow2 = pRange->aEnd.Row();

        SCCOL nC1 = nCol1;
        SCROW nR1 = nRow1;
        SCCOL nC2 = nC1 + nXw;
        if (nC2 > nCol2)
            nC2 = nCol2;
        SCROW nR2 = nR1 + nYw;
        if (nR2 > nRow2)
            nR2 = nRow2;
        do
        {
            do
            {
                for (SCTAB k = 0; k <= MAXTAB; k++)
                {
                    if ( pTab[k] && rMark.GetTableSelect(k) )
                        pTab[k]->ReplaceRangeNamesInUse(nC1, nR1,
                            nC2, nR2, rRangeNames.maRangeMap);
                }
                nC1 = nC2 + 1;
                nC2 = Min((SCCOL)(nC1 + nXw), nCol2);
            } while (nC1 <= nCol2);
            nC1 = nCol1;
            nC2 = nC1 + nXw;
            if (nC2 > nCol2)
                nC2 = nCol2;
            nR1 = nR2 + 1;
            nR2 = Min((SCROW)(nR1 + nYw), nRow2);
        } while (nR1 <= nRow2);
    }
}

ScClipParam& ScDocument::GetClipParam()
{
    if (!mpClipParam.get())
        mpClipParam.reset(new ScClipParam);

    return *mpClipParam;
}

void ScDocument::SetClipParam(const ScClipParam& rParam)
{
    mpClipParam.reset(new ScClipParam(rParam));
}

sal_Bool ScDocument::IsClipboardSource() const
{
    ScDocument* pClipDoc = SC_MOD()->GetClipDoc();
    return pClipDoc && pClipDoc->xPoolHelper.is() &&
            xPoolHelper->GetDocPool() == pClipDoc->xPoolHelper->GetDocPool();
}


void ScDocument::StartListeningFromClip( SCCOL nCol1, SCROW nRow1,
                                        SCCOL nCol2, SCROW nRow2,
                                        const ScMarkData& rMark, sal_uInt16 nInsFlag )
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
                                    const ScMarkData& rMark, sal_uInt16 nInsFlag )
{
    if (nInsFlag & IDF_CONTENTS)
    {
        ScBulkBroadcast aBulkBroadcast( GetBASM());
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
                OSL_ENSURE( pDrawLayer, "CopyBlockFromClip: No drawing layer" );
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

                //  ranges of consecutive selected tables (in clipboard and dest. doc)
                //  must be handled in one UpdateReference call
                SCTAB nFollow = 0;
                while ( i + nFollow < nTabEnd
                        && rMark.GetTableSelect( i + nFollow + 1 )
                        && nClipTab + nFollow < MAXTAB
                        && ppClipTab[nClipTab + nFollow + 1] )
                    ++nFollow;

                if ( pCBFCP->pClipDoc->GetClipParam().mbCutMode )
                {
                    sal_Bool bOldInserting = IsInsertingFromOtherDoc();
                    SetInsertingFromOtherDoc( sal_True);
                    UpdateReference( URM_MOVE,
                        nCol1, nRow1, i, nCol2, nRow2, i+nFollow,
                        nDx, nDy, nDz, pCBFCP->pRefUndoDoc, false );
                    SetInsertingFromOtherDoc( bOldInserting);
                }
                else
                    UpdateReference( URM_COPY,
                        nCol1, nRow1, i, nCol2, nRow2, i+nFollow,
                        nDx, nDy, nDz, pCBFCP->pRefUndoDoc, false );

                nClipTab = (nClipTab+nFollow+1) % (MAXTAB+1);
                i = sal::static_int_cast<SCTAB>( i + nFollow );
            }
        }
    }
}


void ScDocument::CopyNonFilteredFromClip( SCCOL nCol1, SCROW nRow1,
                                    SCCOL nCol2, SCROW nRow2,
                                    const ScMarkData& rMark,
                                    SCsCOL nDx, SCsROW /* nDy */,
                                    const ScCopyBlockFromClipParams* pCBFCP,
                                    SCROW & rClipStartRow )
{
    //  call CopyBlockFromClip for ranges of consecutive non-filtered rows
    //  nCol1/nRow1 etc. is in target doc

    //  filtered state is taken from first used table in clipboard (as in GetClipArea)
    SCTAB nFlagTab = 0;
    ScTable** ppClipTab = pCBFCP->pClipDoc->pTab;
    while ( nFlagTab < MAXTAB && !ppClipTab[nFlagTab] )
        ++nFlagTab;

    SCROW nSourceRow = rClipStartRow;
    SCROW nSourceEnd = 0;
    if ( !pCBFCP->pClipDoc->GetClipParam().maRanges.empty() )
        nSourceEnd = pCBFCP->pClipDoc->GetClipParam().maRanges.front()->aEnd.Row();
    SCROW nDestRow = nRow1;

    while ( nSourceRow <= nSourceEnd && nDestRow <= nRow2 )
    {
        // skip filtered rows
        nSourceRow = pCBFCP->pClipDoc->FirstNonFilteredRow(nSourceRow, nSourceEnd, nFlagTab);

        if ( nSourceRow <= nSourceEnd )
        {
            // look for more non-filtered rows following
            SCROW nLastRow = nSourceRow;
            pCBFCP->pClipDoc->RowFiltered(nSourceRow, nFlagTab, NULL, &nLastRow);
            SCROW nFollow = nLastRow - nSourceRow;

            if (nFollow > nSourceEnd - nSourceRow)
                nFollow = nSourceEnd - nSourceRow;
            if (nFollow > nRow2 - nDestRow)
                nFollow = nRow2 - nDestRow;

            SCsROW nNewDy = ((SCsROW)nDestRow) - nSourceRow;
            CopyBlockFromClip( nCol1, nDestRow, nCol2, nDestRow + nFollow, rMark, nDx, nNewDy, pCBFCP );

            nSourceRow += nFollow + 1;
            nDestRow += nFollow + 1;
        }
    }
    rClipStartRow = nSourceRow;
}


void ScDocument::CopyFromClip( const ScRange& rDestRange, const ScMarkData& rMark,
                                sal_uInt16 nInsFlag,
                                ScDocument* pRefUndoDoc, ScDocument* pClipDoc, sal_Bool bResetCut,
                                sal_Bool bAsLink, sal_Bool bIncludeFiltered, sal_Bool bSkipAttrForEmpty,
                                const ScRangeList * pDestRanges )
{
    if (!bIsClip)
    {
        if (!pClipDoc)
        {
            OSL_FAIL("CopyFromClip: no ClipDoc");
            pClipDoc = SC_MOD()->GetClipDoc();
        }
        if (pClipDoc->bIsClip && pClipDoc->GetTableCount())
        {
            sal_Bool bOldAutoCalc = GetAutoCalc();
            SetAutoCalc( false );   // avoid multiple recalculations

            NumFmtMergeHandler aNumFmtMergeHdl(this, pClipDoc);

            ScClipRangeNameData aClipRangeNames;
            CopyRangeNamesFromClip(pClipDoc, aClipRangeNames);

            SCCOL nAllCol1 = rDestRange.aStart.Col();
            SCROW nAllRow1 = rDestRange.aStart.Row();
            SCCOL nAllCol2 = rDestRange.aEnd.Col();
            SCROW nAllRow2 = rDestRange.aEnd.Row();

            SCCOL nXw = 0;
            SCROW nYw = 0;
            ScRange aClipRange = pClipDoc->GetClipParam().getWholeRange();
            for (SCTAB nTab = 0; nTab <= MAXTAB; nTab++)    // find largest merge overlap
                if (pClipDoc->pTab[nTab])                   // all sheets of the clipboard content
                {
                    SCCOL nThisEndX = aClipRange.aEnd.Col();
                    SCROW nThisEndY = aClipRange.aEnd.Row();
                    pClipDoc->ExtendMerge( aClipRange.aStart.Col(),
                                            aClipRange.aStart.Row(),
                                            nThisEndX, nThisEndY, nTab );
                    // only extra value from ExtendMerge
                    nThisEndX = sal::static_int_cast<SCCOL>( nThisEndX - aClipRange.aEnd.Col() );
                    nThisEndY = sal::static_int_cast<SCROW>( nThisEndY - aClipRange.aEnd.Row() );
                    if ( nThisEndX > nXw )
                        nXw = nThisEndX;
                    if ( nThisEndY > nYw )
                        nYw = nThisEndY;
                }

            SCCOL nDestAddX;
            SCROW nDestAddY;
            pClipDoc->GetClipArea( nDestAddX, nDestAddY, bIncludeFiltered );
            nXw = sal::static_int_cast<SCCOL>( nXw + nDestAddX );
            nYw = sal::static_int_cast<SCROW>( nYw + nDestAddY );   // ClipArea, plus ExtendMerge value

            /*  Decide which contents to delete before copying. Delete all
                contents if nInsFlag contains any real content flag.
                #i102056# Notes are pasted from clipboard in a second pass,
                together with the special flag IDF_ADDNOTES that states to not
                overwrite/delete existing cells but to insert the notes into
                these cells. In this case, just delete old notes from the
                destination area. */
            sal_uInt16 nDelFlag = IDF_NONE;
            if ( (nInsFlag & (IDF_CONTENTS | IDF_ADDNOTES)) == (IDF_NOTE | IDF_ADDNOTES) )
                nDelFlag |= IDF_NOTE;
            else if ( nInsFlag & IDF_CONTENTS )
                nDelFlag |= IDF_CONTENTS;
            //  With bSkipAttrForEmpty, don't remove attributes, copy
            //  on top of existing attributes instead.
            if ( ( nInsFlag & IDF_ATTRIB ) && !bSkipAttrForEmpty )
                nDelFlag |= IDF_ATTRIB;

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

            ScRangeList aLocalRangeList;
            if (!pDestRanges)
            {
                aLocalRangeList.Append( rDestRange);
                pDestRanges = &aLocalRangeList;
            }

            bInsertingFromOtherDoc = sal_True;  // kein Broadcast/Listener aufbauen bei Insert

            // bei mindestens 64 Zeilen wird in ScColumn::CopyFromClip voralloziert
            sal_Bool bDoDouble = ( nYw < 64 && nAllRow2 - nAllRow1 > 64);
            sal_Bool bOldDouble = ScColumn::bDoubleAlloc;
            if (bDoDouble)
                ScColumn::bDoubleAlloc = sal_True;

            SCCOL nClipStartCol = aClipRange.aStart.Col();
            SCROW nClipStartRow = aClipRange.aStart.Row();
            SCROW nClipEndRow = aClipRange.aEnd.Row();
            for ( size_t nRange = 0; nRange < pDestRanges->size(); ++nRange )
            {
                const ScRange* pRange = (*pDestRanges)[nRange];
                SCCOL nCol1 = pRange->aStart.Col();
                SCROW nRow1 = pRange->aStart.Row();
                SCCOL nCol2 = pRange->aEnd.Col();
                SCROW nRow2 = pRange->aEnd.Row();

                DeleteArea(nCol1, nRow1, nCol2, nRow2, rMark, nDelFlag);

                SCCOL nC1 = nCol1;
                SCROW nR1 = nRow1;
                SCCOL nC2 = nC1 + nXw;
                if (nC2 > nCol2)
                    nC2 = nCol2;
                SCROW nR2 = nR1 + nYw;
                if (nR2 > nRow2)
                    nR2 = nRow2;

                do
                {
                    // Pasting is done column-wise, when pasting to a filtered
                    // area this results in partitioning and we have to
                    // remember and reset the start row for each column until
                    // it can be advanced for the next chunk of unfiltered
                    // rows.
                    SCROW nSaveClipStartRow = nClipStartRow;
                    do
                    {
                        nClipStartRow = nSaveClipStartRow;
                        SCsCOL nDx = ((SCsCOL)nC1) - nClipStartCol;
                        SCsROW nDy = ((SCsROW)nR1) - nClipStartRow;
                        if ( bIncludeFiltered )
                        {
                            CopyBlockFromClip( nC1, nR1, nC2, nR2, rMark, nDx,
                                    nDy, &aCBFCP );
                            nClipStartRow += nR2 - nR1 + 1;
                        }
                        else
                        {
                            CopyNonFilteredFromClip( nC1, nR1, nC2, nR2, rMark,
                                    nDx, nDy, &aCBFCP, nClipStartRow );
                        }
                        nC1 = nC2 + 1;
                        nC2 = Min((SCCOL)(nC1 + nXw), nCol2);
                    } while (nC1 <= nCol2);
                    if (nClipStartRow > nClipEndRow)
                        nClipStartRow = aClipRange.aStart.Row();
                    nC1 = nCol1;
                    nC2 = nC1 + nXw;
                    if (nC2 > nCol2)
                        nC2 = nCol2;
                    nR1 = nR2 + 1;
                    nR2 = Min((SCROW)(nR1 + nYw), nRow2);
                } while (nR1 <= nRow2);
            }

            ScColumn::bDoubleAlloc = bOldDouble;

            for (SCTAB k = 0; k <= MAXTAB; k++)
                if (pTab[k] && rMark.GetTableSelect(k))
                    pTab[k]->DecRecalcLevel();

            bInsertingFromOtherDoc = false;

            UpdateRangeNamesInFormulas(aClipRangeNames, *pDestRanges, rMark, nXw, nYw);

            // Listener aufbauen nachdem alles inserted wurde
            StartListeningFromClip( nAllCol1, nAllRow1, nAllCol2, nAllRow2, rMark, nInsFlag );
            // nachdem alle Listener aufgebaut wurden, kann gebroadcastet werden
            BroadcastFromClip( nAllCol1, nAllRow1, nAllCol2, nAllRow2, rMark, nInsFlag );
            if (bResetCut)
                pClipDoc->GetClipParam().mbCutMode = false;
            SetAutoCalc( bOldAutoCalc );
        }
    }
}

static SCROW lcl_getLastNonFilteredRow(
    const ScBitMaskCompressedArray<SCROW, sal_uInt8>& rFlags, SCROW nBegRow, SCROW nEndRow,
    SCROW nRowCount)
{
    SCROW nFilteredRow = rFlags.GetFirstForCondition(
        nBegRow, nEndRow, CR_FILTERED, CR_FILTERED);

    SCROW nRow = nFilteredRow - 1;
    if (nRow - nBegRow + 1 > nRowCount)
        // make sure the row range stays within the data size.
        nRow = nBegRow + nRowCount - 1;

    return nRow;
}

void ScDocument::CopyMultiRangeFromClip(
    const ScAddress& rDestPos, const ScMarkData& rMark, sal_uInt16 nInsFlag, ScDocument* pClipDoc,
    bool bResetCut, bool bAsLink, bool /*bIncludeFiltered*/, bool bSkipAttrForEmpty)
{
    if (bIsClip)
        return;

    if (!pClipDoc->bIsClip || !pClipDoc->GetTableCount())
        // There is nothing in the clip doc to copy.
        return;

    sal_Bool bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( false );   // avoid multiple recalculations

    NumFmtMergeHandler aNumFmtMergeHdl(this, pClipDoc);

    ScClipRangeNameData aClipRangeNames;
    CopyRangeNamesFromClip(pClipDoc, aClipRangeNames);

    SCCOL nCol1 = rDestPos.Col();
    SCROW nRow1 = rDestPos.Row();
    ScClipParam& rClipParam = pClipDoc->GetClipParam();

    ScCopyBlockFromClipParams aCBFCP;
    aCBFCP.pRefUndoDoc = NULL;
    aCBFCP.pClipDoc = pClipDoc;
    aCBFCP.nInsFlag = nInsFlag;
    aCBFCP.bAsLink  = bAsLink;
    aCBFCP.bSkipAttrForEmpty = bSkipAttrForEmpty;
    aCBFCP.nTabStart = MAXTAB;
    aCBFCP.nTabEnd = 0;

    for (SCTAB j = 0; j <= MAXTAB; ++j)
    {
        if (pTab[j] && rMark.GetTableSelect(j))
        {
            if ( j < aCBFCP.nTabStart )
                aCBFCP.nTabStart = j;
            aCBFCP.nTabEnd = j;
            pTab[j]->IncRecalcLevel();
        }
    }

    ScRange aDestRange;
    rMark.GetMarkArea(aDestRange);
    SCROW nLastMarkedRow = aDestRange.aEnd.Row();

    bInsertingFromOtherDoc = sal_True;  // kein Broadcast/Listener aufbauen bei Insert

    SCROW nBegRow = nRow1;
    sal_uInt16 nDelFlag = IDF_CONTENTS;
    const ScBitMaskCompressedArray<SCROW, sal_uInt8>& rFlags = GetRowFlagsArray(aCBFCP.nTabStart);

    for ( size_t i = 0, n = rClipParam.maRanges.size(); i < n; ++i )
    {
        ScRange* p = rClipParam.maRanges[ i ];
        // The begin row must not be filtered.

        SCROW nRowCount = p->aEnd.Row() - p->aStart.Row() + 1;

        SCsCOL nDx = static_cast<SCsCOL>(nCol1 - p->aStart.Col());
        SCsROW nDy = static_cast<SCsROW>(nBegRow - p->aStart.Row());
        SCCOL nCol2 = nCol1 + p->aEnd.Col() - p->aStart.Col();

        SCROW nEndRow = lcl_getLastNonFilteredRow(rFlags, nBegRow, nLastMarkedRow, nRowCount);

        if (!bSkipAttrForEmpty)
            DeleteArea(nCol1, nBegRow, nCol2, nEndRow, rMark, nDelFlag);

        CopyBlockFromClip(nCol1, nBegRow, nCol2, nEndRow, rMark, nDx, nDy, &aCBFCP);
        nRowCount -= nEndRow - nBegRow + 1;

        while (nRowCount > 0)
        {
            // Get the first non-filtered row.
            SCROW nNonFilteredRow = rFlags.GetFirstForCondition(nEndRow+1, nLastMarkedRow, CR_FILTERED, 0);
            if (nNonFilteredRow > nLastMarkedRow)
                return;

            SCROW nRowsSkipped = nNonFilteredRow - nEndRow - 1;
            nDy += nRowsSkipped;

            nBegRow = nNonFilteredRow;
            nEndRow = lcl_getLastNonFilteredRow(rFlags, nBegRow, nLastMarkedRow, nRowCount);

            if (!bSkipAttrForEmpty)
                DeleteArea(nCol1, nBegRow, nCol2, nEndRow, rMark, nDelFlag);

            CopyBlockFromClip(nCol1, nBegRow, nCol2, nEndRow, rMark, nDx, nDy, &aCBFCP);
            nRowCount -= nEndRow - nBegRow + 1;
        }

        if (rClipParam.meDirection == ScClipParam::Row)
            // Begin row for the next range being pasted.
            nBegRow = rFlags.GetFirstForCondition(nEndRow+1, nLastMarkedRow, CR_FILTERED, 0);
        else
            nBegRow = nRow1;

        if (rClipParam.meDirection == ScClipParam::Column)
            nCol1 += p->aEnd.Col() - p->aStart.Col() + 1;
    }

    for (SCTAB i = 0; i <= MAXTAB; i++)
        if (pTab[i] && rMark.GetTableSelect(i))
            pTab[i]->DecRecalcLevel();

    bInsertingFromOtherDoc = false;

    ScRangeList aRanges;
    aRanges.Append(aDestRange);
    SCCOL nCols = aDestRange.aEnd.Col() - aDestRange.aStart.Col() + 1;
    SCROW nRows = aDestRange.aEnd.Row() - aDestRange.aStart.Row() + 1;
    UpdateRangeNamesInFormulas(aClipRangeNames, aRanges, rMark, nCols-1, nRows-1);

    // Listener aufbauen nachdem alles inserted wurde
    StartListeningFromClip(aDestRange.aStart.Col(), aDestRange.aStart.Row(),
                           aDestRange.aEnd.Col(), aDestRange.aEnd.Row(), rMark, nInsFlag );
    // nachdem alle Listener aufgebaut wurden, kann gebroadcastet werden
    BroadcastFromClip(aDestRange.aStart.Col(), aDestRange.aStart.Row(),
                      aDestRange.aEnd.Col(), aDestRange.aEnd.Row(), rMark, nInsFlag );

    if (bResetCut)
        pClipDoc->GetClipParam().mbCutMode = false;
    SetAutoCalc( bOldAutoCalc );
}

void ScDocument::SetClipArea( const ScRange& rArea, sal_Bool bCut )
{
    if (bIsClip)
    {
        ScClipParam& rClipParam = GetClipParam();
        rClipParam.maRanges.RemoveAll();
        rClipParam.maRanges.Append(rArea);
        rClipParam.mbCutMode = bCut;
    }
    else
    {
        OSL_FAIL("SetClipArea: kein Clip");
    }
}


void ScDocument::GetClipArea(SCCOL& nClipX, SCROW& nClipY, sal_Bool bIncludeFiltered)
{
    if (!bIsClip)
    {
        OSL_FAIL("GetClipArea: kein Clip");
        return;
    }

    ScRangeList& rClipRanges = GetClipParam().maRanges;
    if (rClipRanges.empty())
        // No clip range.  Bail out.
        return;

    ScRange* p = rClipRanges.front();
    SCCOL nStartCol = p->aStart.Col();
    SCCOL nEndCol   = p->aEnd.Col();
    SCROW nStartRow = p->aStart.Row();
    SCROW nEndRow   = p->aEnd.Row();
    for ( size_t i = 1, n = rClipRanges.size(); i < n; ++i )
    {
        p = rClipRanges[ i ];
        if (p->aStart.Col() < nStartCol)
            nStartCol = p->aStart.Col();
        if (p->aStart.Row() < nStartRow)
            nStartRow = p->aStart.Row();
        if (p->aEnd.Col() > nEndCol)
            nEndCol = p->aEnd.Col();
        if (p->aEnd.Row() < nEndRow)
            nEndRow = p->aEnd.Row();
    }

    nClipX = nEndCol - nStartCol;

    if ( bIncludeFiltered )
        nClipY = nEndRow - nStartRow;
    else
    {
        //  count non-filtered rows
        //  count on first used table in clipboard
        SCTAB nCountTab = 0;
        while ( nCountTab < MAXTAB && !pTab[nCountTab] )
            ++nCountTab;

        SCROW nResult = CountNonFilteredRows(nStartRow, nEndRow, nCountTab);

        if ( nResult > 0 )
            nClipY = nResult - 1;
        else
            nClipY = 0;                 // always return at least 1 row
    }
}


void ScDocument::GetClipStart(SCCOL& nClipX, SCROW& nClipY)
{
    if (bIsClip)
    {
        ScRangeList& rClipRanges = GetClipParam().maRanges;
        if ( !rClipRanges.empty() )
        {
            nClipX = rClipRanges.front()->aStart.Col();
            nClipY = rClipRanges.front()->aStart.Row();
        }
    }
    else
    {
        OSL_FAIL("GetClipStart: kein Clip");
    }
}


sal_Bool ScDocument::HasClipFilteredRows()
{
    //  count on first used table in clipboard
    SCTAB nCountTab = 0;
    while ( nCountTab < MAXTAB && !pTab[nCountTab] )
        ++nCountTab;

    ScRangeList& rClipRanges = GetClipParam().maRanges;
    if ( rClipRanges.empty() )
        return false;

    for ( size_t i = 0, n = rClipRanges.size(); i < n; ++i )
    {
        ScRange* p = rClipRanges[ i ];
        bool bAnswer = pTab[nCountTab]->HasFilteredRows(p->aStart.Row(), p->aEnd.Row());
        if (bAnswer)
            return true;
    }
    return false;
}


void ScDocument::MixDocument( const ScRange& rRange, sal_uInt16 nFunction, sal_Bool bSkipEmpty,
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
                                sal_uInt16 nFlags, sal_uInt16 nFunction,
                                sal_Bool bSkipEmpty, sal_Bool bAsLink )
{
    sal_uInt16 nDelFlags = nFlags;
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
        sal_Bool bDoMix = ( bSkipEmpty || nFunction ) && ( nFlags & IDF_CONTENTS );

        sal_Bool bOldAutoCalc = GetAutoCalc();
        SetAutoCalc( false );                   // avoid multiple calculations

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
                                            IDF_CONTENTS, false, pMixDoc->pTab[i] );
                }
                pTab[i]->DeleteArea( nStartCol,nStartRow, nEndCol,nEndRow, nDelFlags);
                pTab[nSrcTab]->CopyToTable( nStartCol,nStartRow, nEndCol,nEndRow,
                                                 nFlags, false, pTab[i], NULL, bAsLink );

                if (bDoMix)
                    pTab[i]->MixData( nStartCol,nStartRow, nEndCol,nEndRow,
                                        nFunction, bSkipEmpty, pMixDoc->pTab[i] );
            }

        delete pMixDoc;

        SetAutoCalc( bOldAutoCalc );
    }
    else
    {
        OSL_FAIL("falsche Tabelle");
    }
}


void ScDocument::FillTabMarked( SCTAB nSrcTab, const ScMarkData& rMark,
                                sal_uInt16 nFlags, sal_uInt16 nFunction,
                                sal_Bool bSkipEmpty, sal_Bool bAsLink )
{
    sal_uInt16 nDelFlags = nFlags;
    if (nDelFlags & IDF_CONTENTS)
        nDelFlags |= IDF_CONTENTS;          // immer alle Inhalte oder keine loeschen!

    if (ValidTab(nSrcTab)  && pTab[nSrcTab])
    {
        ScDocument* pMixDoc = NULL;
        sal_Bool bDoMix = ( bSkipEmpty || nFunction ) && ( nFlags & IDF_CONTENTS );

        sal_Bool bOldAutoCalc = GetAutoCalc();
        SetAutoCalc( false );                   // avoid multiple calculations

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
                                            IDF_CONTENTS, sal_True, pMixDoc->pTab[i], &rMark );
                }

                pTab[i]->DeleteSelection( nDelFlags, rMark );
                pTab[nSrcTab]->CopyToTable( nStartCol,nStartRow, nEndCol,nEndRow,
                                             nFlags, sal_True, pTab[i], &rMark, bAsLink );

                if (bDoMix)
                    pTab[i]->MixMarked( rMark, nFunction, bSkipEmpty, pMixDoc->pTab[i] );
            }

        delete pMixDoc;

        SetAutoCalc( bOldAutoCalc );
    }
    else
    {
        OSL_FAIL("falsche Tabelle");
    }
}


void ScDocument::PutCell( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell* pCell, sal_Bool bForceTab )
{
    if (VALIDTAB(nTab))
    {
        if ( bForceTab && !pTab[nTab] )
        {
            sal_Bool bExtras = !bIsUndo;        // Spaltenbreiten, Zeilenhoehen, Flags

            pTab[nTab] = new ScTable(this, nTab,
                            String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("temp")),
                            bExtras, bExtras);
            ++nMaxTableNumber;
        }

        if (pTab[nTab])
            pTab[nTab]->PutCell( nCol, nRow, pCell );
    }
}


void ScDocument::PutCell( const ScAddress& rPos, ScBaseCell* pCell, sal_Bool bForceTab )
{
    SCTAB nTab = rPos.Tab();
    if ( bForceTab && !pTab[nTab] )
    {
        sal_Bool bExtras = !bIsUndo;        // Spaltenbreiten, Zeilenhoehen, Flags

        pTab[nTab] = new ScTable(this, nTab,
                        String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("temp")),
                        bExtras, bExtras);
        ++nMaxTableNumber;
    }

    if (pTab[nTab])
        pTab[nTab]->PutCell( rPos, pCell );
}


sal_Bool ScDocument::SetString( SCCOL nCol, SCROW nRow, SCTAB nTab, const String& rString,
                            ScSetStringParam* pParam )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->SetString( nCol, nRow, nTab, rString, pParam );
    else
        return false;
}


void ScDocument::SetValue( SCCOL nCol, SCROW nRow, SCTAB nTab, const double& rVal )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->SetValue( nCol, nRow, rVal );
}


void ScDocument::GetString( SCCOL nCol, SCROW nRow, SCTAB nTab, String& rString )
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
        pTab[nTab]->GetString( nCol, nRow, rString );
    else
        rString.Erase();
}

void ScDocument::GetString( SCCOL nCol, SCROW nRow, SCTAB nTab, rtl::OUString& rString )
{
    String aStr;
    GetString(nCol, nRow, nTab, aStr);
    rString = aStr;
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
                                  sal_uInt32& rFormat ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
        {
            rFormat = pTab[nTab]->GetNumberFormat( nCol, nRow );
            return ;
        }
    rFormat = 0;
}

sal_uInt32 ScDocument::GetNumberFormat( const ScRange& rRange ) const
{
    SCTAB nTab1 = rRange.aStart.Tab(), nTab2 = rRange.aEnd.Tab();
    SCCOL nCol1 = rRange.aStart.Col(), nCol2 = rRange.aEnd.Col();
    SCROW nRow1 = rRange.aStart.Row(), nRow2 = rRange.aEnd.Row();

    if (!ValidTab(nTab1) || !ValidTab(nTab2) || !pTab[nTab1] || !pTab[nTab2])
        return 0;

    sal_uInt32 nFormat = 0;
    bool bFirstItem = true;
    for (SCTAB nTab = nTab1; nTab <= nTab2; ++nTab)
        for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        {
            sal_uInt32 nThisFormat = pTab[nTab]->GetNumberFormat(nCol, nRow1, nRow2);
            if (bFirstItem)
            {
                nFormat = nThisFormat;
                bFirstItem = false;
            }
            else if (nThisFormat != nFormat)
                return 0;
        }

    return nFormat;
}

sal_uInt32 ScDocument::GetNumberFormat( const ScAddress& rPos ) const
{
    SCTAB nTab = rPos.Tab();
    if ( pTab[nTab] )
        return pTab[nTab]->GetNumberFormat( rPos );
    return 0;
}


void ScDocument::GetNumberFormatInfo( short& nType, sal_uLong& nIndex,
            const ScAddress& rPos, const ScBaseCell* pCell ) const
{
    SCTAB nTab = rPos.Tab();
    if ( pTab[nTab] )
    {
        nIndex = pTab[nTab]->GetNumberFormat( rPos );
        if ( (nIndex % SV_COUNTRY_LANGUAGE_OFFSET) == 0 && pCell &&
                pCell->GetCellType() == CELLTYPE_FORMULA )
            static_cast<const ScFormulaCell*>(pCell)->GetFormatInfo( nType, nIndex );
        else
            nType = GetFormatTable()->GetType( nIndex );
    }
    else
    {
        nType = NUMBERFORMAT_UNDEFINED;
        nIndex = 0;
    }
}


void ScDocument::GetFormula( SCCOL nCol, SCROW nRow, SCTAB nTab, String& rFormula ) const
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
            pTab[nTab]->GetFormula( nCol, nRow, rFormula );
    else
        rFormula.Erase();
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
        OSL_FAIL("GetCell ohne Tabelle");
        rpCell = NULL;
    }
}


ScBaseCell* ScDocument::GetCell( const ScAddress& rPos ) const
{
    SCTAB nTab = rPos.Tab();
    if (ValidTab(nTab) && pTab[nTab])
        return pTab[nTab]->GetCell( rPos );

    OSL_FAIL("GetCell ohne Tabelle");
    return NULL;
}


sal_Bool ScDocument::HasStringData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
            return pTab[nTab]->HasStringData( nCol, nRow );
    else
        return false;
}


sal_Bool ScDocument::HasValueData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    if ( VALIDTAB(nTab) && pTab[nTab] )
            return pTab[nTab]->HasValueData( nCol, nRow );
    else
        return false;
}


sal_Bool ScDocument::HasStringCells( const ScRange& rRange ) const
{
    //  sal_True, wenn String- oder Editzellen im Bereich

    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    for ( SCTAB nTab=nStartTab; nTab<=nEndTab; nTab++ )
        if ( pTab[nTab] && pTab[nTab]->HasStringCells( nStartCol, nStartRow, nEndCol, nEndRow ) )
            return sal_True;

    return false;
}


sal_Bool ScDocument::HasSelectionData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    sal_uInt32 nValidation = static_cast< const SfxUInt32Item* >( GetAttr( nCol, nRow, nTab, ATTR_VALIDDATA ) )->GetValue();
    if( nValidation )
    {
        const ScValidationData* pData = GetValidationEntry( nValidation );
        if( pData && pData->HasSelectionList() )
            return sal_True;
    }
    return HasStringCells( ScRange( nCol, 0, nTab, nCol, MAXROW, nTab ) );
}


ScPostIt* ScDocument::GetNote( const ScAddress& rPos )
{
    ScTable* pTable = ValidTab( rPos.Tab() ) ? pTab[ rPos.Tab() ] : 0;
    return pTable ? pTable->GetNote( rPos.Col(), rPos.Row() ) : 0;
}


void ScDocument::TakeNote( const ScAddress& rPos, ScPostIt*& rpNote )
{
    if( ValidTab( rPos.Tab() ) && pTab[ rPos.Tab() ] )
        pTab[ rPos.Tab() ]->TakeNote( rPos.Col(), rPos.Row(), rpNote );
    else
        DELETEZ( rpNote );
}


ScPostIt* ScDocument::ReleaseNote( const ScAddress& rPos )
{
    ScTable* pTable = ValidTab( rPos.Tab() ) ? pTab[ rPos.Tab() ] : 0;
    return pTable ? pTable->ReleaseNote( rPos.Col(), rPos.Row() ) : 0;
}


ScPostIt* ScDocument::GetOrCreateNote( const ScAddress& rPos )
{
    ScPostIt* pNote = GetNote( rPos );
    if( !pNote )
    {
        pNote = new ScPostIt( *this, rPos, false );
        TakeNote( rPos, pNote );
    }
    return pNote;
}


void ScDocument::DeleteNote( const ScAddress& rPos )
{
    if( ValidTab( rPos.Tab() ) && pTab[ rPos.Tab() ] )
        pTab[ rPos.Tab() ]->DeleteNote( rPos.Col(), rPos.Row() );
}


void ScDocument::InitializeNoteCaptions( SCTAB nTab, bool bForced )
{
    if( ValidTab( nTab ) && pTab[ nTab ] )
        pTab[ nTab ]->InitializeNoteCaptions( bForced );
}

void ScDocument::InitializeAllNoteCaptions( bool bForced )
{
    for( SCTAB nTab = 0; nTab < GetTableCount(); ++nTab )
        InitializeNoteCaptions( nTab, bForced );
}

void ScDocument::SetDirty()
{
    sal_Bool bOldAutoCalc = GetAutoCalc();
    bAutoCalc = false;      // keine Mehrfachberechnung
    {   // scope for bulk broadcast
        ScBulkBroadcast aBulkBroadcast( GetBASM());
        for (SCTAB i=0; i<=MAXTAB; i++)
            if (pTab[i]) pTab[i]->SetDirty();
    }

    //  Charts werden zwar auch ohne AutoCalc im Tracking auf Dirty gesetzt,
    //  wenn alle Formeln dirty sind, werden die Charts aber nicht mehr erwischt
    //  (#45205#) - darum alle Charts nochmal explizit
    if (pChartListenerCollection)
        pChartListenerCollection->SetDirty();

    SetAutoCalc( bOldAutoCalc );
}


void ScDocument::SetDirty( const ScRange& rRange )
{
    sal_Bool bOldAutoCalc = GetAutoCalc();
    bAutoCalc = false;      // keine Mehrfachberechnung
    {   // scope for bulk broadcast
        ScBulkBroadcast aBulkBroadcast( GetBASM());
        SCTAB nTab2 = rRange.aEnd.Tab();
        for (SCTAB i=rRange.aStart.Tab(); i<=nTab2; i++)
            if (pTab[i]) pTab[i]->SetDirty( rRange );
    }
    SetAutoCalc( bOldAutoCalc );
}


void ScDocument::SetTableOpDirty( const ScRange& rRange )
{
    sal_Bool bOldAutoCalc = GetAutoCalc();
    bAutoCalc = false;      // no multiple recalculation
    SCTAB nTab2 = rRange.aEnd.Tab();
    for (SCTAB i=rRange.aStart.Tab(); i<=nTab2; i++)
        if (pTab[i]) pTab[i]->SetTableOpDirty( rRange );
    SetAutoCalc( bOldAutoCalc );
}


void ScDocument::InterpretDirtyCells( const ScRangeList& rRanges )
{
    for (size_t nPos=0, nRangeCount = rRanges.size(); nPos < nRangeCount; nPos++)
    {
        ScCellIterator aIter( this, *rRanges[ nPos ] );
        ScBaseCell* pCell = aIter.GetFirst();
        while (pCell)
        {
            if (pCell->GetCellType() == CELLTYPE_FORMULA)
            {
                if ( static_cast<ScFormulaCell*>(pCell)->GetDirty() && GetAutoCalc() )
                    static_cast<ScFormulaCell*>(pCell)->Interpret();
            }
            pCell = aIter.GetNext();
        }
    }
}


void ScDocument::AddTableOpFormulaCell( ScFormulaCell* pCell )
{
    if ( !aTableOpList.empty() )
    {
        ScInterpreterTableOpParams* p = &aTableOpList.back();
        if ( p->bCollectNotifications )
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
}


void ScDocument::CalcAll()
{
    ClearLookupCaches();    // Ensure we don't deliver zombie data.
    sal_Bool bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( sal_True );
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
    sal_Bool bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( false );
    ScProgress aProgress( GetDocumentShell(), ScGlobal::GetRscString(
                STR_PROGRESS_CALCULATING ), GetXMLImportedFormulaCount() );

    // set AutoNameCache to speed up automatic name lookup
    DBG_ASSERT( !pAutoNameCache, "AutoNameCache already set" );
    pAutoNameCache = new ScAutoNameCache( this );

    for (SCTAB i=0; i<=MAXTAB; i++)
        if (pTab[i]) pTab[i]->CompileXML( aProgress );

    DELETEZ( pAutoNameCache );  // valid only during CompileXML, where cell contents don't change

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

    bCalcingAfterLoad = sal_True;
    for ( i = 0; i <= MAXTAB; i++)
        if (pTab[i]) pTab[i]->CalcAfterLoad();
    for (i=0; i<=MAXTAB; i++)
        if (pTab[i]) pTab[i]->SetDirtyAfterLoad();
    bCalcingAfterLoad = false;

    SetDetectiveDirty(false);   // noch keine wirklichen Aenderungen

    // #i112436# If formula cells are already dirty, they don't broadcast further changes.
    // So the source ranges of charts must be interpreted even if they are not visible,
    // similar to ScMyShapeResizer::CreateChartListener for loading own files (i104899).
    if (pChartListenerCollection)
    {
        sal_uInt16 nChartCount = pChartListenerCollection->GetCount();
        for ( sal_uInt16 nIndex = 0; nIndex < nChartCount; nIndex++ )
        {
            ScChartListener* pChartListener = static_cast<ScChartListener*>(pChartListenerCollection->At(nIndex));
            InterpretDirtyCells(*pChartListener->GetRangeList());
        }
    }
}


sal_uInt16 ScDocument::GetErrCode( const ScAddress& rPos ) const
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


void ScDocument::SetColWidth( SCCOL nCol, SCTAB nTab, sal_uInt16 nNewWidth )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->SetColWidth( nCol, nNewWidth );
}

void ScDocument::SetColWidthOnly( SCCOL nCol, SCTAB nTab, sal_uInt16 nNewWidth )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->SetColWidthOnly( nCol, nNewWidth );
}

void ScDocument::SetRowHeight( SCROW nRow, SCTAB nTab, sal_uInt16 nNewHeight )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->SetRowHeight( nRow, nNewHeight );
}


void ScDocument::SetRowHeightRange( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, sal_uInt16 nNewHeight )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->SetRowHeightRange
            ( nStartRow, nEndRow, nNewHeight, 1.0, 1.0 );
}

void ScDocument::SetRowHeightOnly( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, sal_uInt16 nNewHeight )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->SetRowHeightOnly( nStartRow, nEndRow, nNewHeight );
}

void ScDocument::SetManualHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, sal_Bool bManual )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->SetManualHeight( nStartRow, nEndRow, bManual );
}


sal_uInt16 ScDocument::GetColWidth( SCCOL nCol, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetColWidth( nCol );
    OSL_FAIL("Falsche Tabellennummer");
    return 0;
}


sal_uInt16 ScDocument::GetOriginalWidth( SCCOL nCol, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetOriginalWidth( nCol );
    OSL_FAIL("Falsche Tabellennummer");
    return 0;
}


sal_uInt16 ScDocument::GetCommonWidth( SCCOL nEndCol, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetCommonWidth( nEndCol );
    OSL_FAIL("Wrong table number");
    return 0;
}


sal_uInt16 ScDocument::GetOriginalHeight( SCROW nRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetOriginalHeight( nRow );
    OSL_FAIL("Wrong table number");
    return 0;
}


sal_uInt16 ScDocument::GetRowHeight( SCROW nRow, SCTAB nTab, bool bHiddenAsZero ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetRowHeight( nRow, NULL, NULL, bHiddenAsZero );
    OSL_FAIL("Wrong sheet number");
    return 0;
}


sal_uInt16 ScDocument::GetRowHeight( SCROW nRow, SCTAB nTab, SCROW* pStartRow, SCROW* pEndRow, bool bHiddenAsZero ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetRowHeight( nRow, pStartRow, pEndRow, bHiddenAsZero );
    OSL_FAIL("Wrong sheet number");
    return 0;
}


sal_uLong ScDocument::GetRowHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab ) const
{
    if (nStartRow == nEndRow)
        return GetRowHeight( nStartRow, nTab);  // faster for a single row

    // check bounds because this method replaces former for(i=start;i<=end;++i) loops
    if (nStartRow > nEndRow)
        return 0;

    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetRowHeight( nStartRow, nEndRow);

    OSL_FAIL("wrong sheet number");
    return 0;
}

SCROW ScDocument::GetRowForHeight( SCTAB nTab, sal_uLong nHeight ) const
{
    return pTab[nTab]->GetRowForHeight(nHeight);
}

sal_uLong ScDocument::GetScaledRowHeight( SCROW nStartRow, SCROW nEndRow,
        SCTAB nTab, double fScale ) const
{
    // faster for a single row
    if (nStartRow == nEndRow)
        return (sal_uLong) (GetRowHeight( nStartRow, nTab) * fScale);

    // check bounds because this method replaces former for(i=start;i<=end;++i) loops
    if (nStartRow > nEndRow)
        return 0;

    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetScaledRowHeight( nStartRow, nEndRow, fScale);

    OSL_FAIL("wrong sheet number");
    return 0;
}

SCROW ScDocument::GetHiddenRowCount( SCROW nRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetHiddenRowCount( nRow );
    OSL_FAIL("Falsche Tabellennummer");
    return 0;
}


sal_uLong ScDocument::GetColOffset( SCCOL nCol, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetColOffset( nCol );
    OSL_FAIL("Falsche Tabellennummer");
    return 0;
}


sal_uLong ScDocument::GetRowOffset( SCROW nRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetRowOffset( nRow );
    OSL_FAIL("Falsche Tabellennummer");
    return 0;
}


sal_uInt16 ScDocument::GetOptimalColWidth( SCCOL nCol, SCTAB nTab, OutputDevice* pDev,
                                       double nPPTX, double nPPTY,
                                       const Fraction& rZoomX, const Fraction& rZoomY,
                                       sal_Bool bFormula, const ScMarkData* pMarkData,
                                       const ScColWidthParam* pParam )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetOptimalColWidth( nCol, pDev, nPPTX, nPPTY,
            rZoomX, rZoomY, bFormula, pMarkData, pParam );
    OSL_FAIL("Falsche Tabellennummer");
    return 0;
}


long ScDocument::GetNeededSize( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    sal_Bool bWidth, sal_Bool bTotalSize )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetNeededSize
                ( nCol, nRow, pDev, nPPTX, nPPTY, rZoomX, rZoomY, bWidth, bTotalSize );
    OSL_FAIL("Falsche Tabellennummer");
    return 0;
}


sal_Bool ScDocument::SetOptimalHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, sal_uInt16 nExtra,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    sal_Bool bShrink )
{
//! MarkToMulti();
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->SetOptimalHeight( nStartRow, nEndRow, nExtra,
                                                pDev, nPPTX, nPPTY, rZoomX, rZoomY, bShrink );
    OSL_FAIL("Falsche Tabellennummer");
    return false;
}


void ScDocument::UpdateAllRowHeights( OutputDevice* pDev, double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY, const ScMarkData* pTabMark )
{
    // one progress across all (selected) sheets

    sal_uLong nCellCount = 0;
    for ( SCTAB nTab=0; nTab<=MAXTAB; nTab++ )
        if ( pTab[nTab] && ( !pTabMark || pTabMark->GetTableSelect(nTab) ) )
            nCellCount += pTab[nTab]->GetWeightedCount();

    ScProgress aProgress( GetDocumentShell(), ScGlobal::GetRscString(STR_PROGRESS_HEIGHTING), nCellCount );

    sal_uLong nProgressStart = 0;
    for ( SCTAB nTab=0; nTab<=MAXTAB; nTab++ )
        if ( pTab[nTab] && ( !pTabMark || pTabMark->GetTableSelect(nTab) ) )
        {
            pTab[nTab]->SetOptimalHeightOnly( 0, MAXROW, 0,
                        pDev, nPPTX, nPPTY, rZoomX, rZoomY, false, &aProgress, nProgressStart );
            pTab[nTab]->SetDrawPageSize(true, true);
            nProgressStart += pTab[nTab]->GetWeightedCount();
        }
}


//
//  Spalten-/Zeilen-Flags   ----------------------------------------------
//

void ScDocument::ShowCol(SCCOL nCol, SCTAB nTab, sal_Bool bShow)
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->ShowCol( nCol, bShow );
}


void ScDocument::ShowRow(SCROW nRow, SCTAB nTab, sal_Bool bShow)
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->ShowRow( nRow, bShow );
}


void ScDocument::ShowRows(SCROW nRow1, SCROW nRow2, SCTAB nTab, sal_Bool bShow)
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->ShowRows( nRow1, nRow2, bShow );
}


void ScDocument::SetRowFlags( SCROW nRow, SCTAB nTab, sal_uInt8 nNewFlags )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->SetRowFlags( nRow, nNewFlags );
}


void ScDocument::SetRowFlags( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, sal_uInt8 nNewFlags )
{
    if ( ValidTab(nTab) && pTab[nTab] )
        pTab[nTab]->SetRowFlags( nStartRow, nEndRow, nNewFlags );
}


sal_uInt8 ScDocument::GetColFlags( SCCOL nCol, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetColFlags( nCol );
    OSL_FAIL("Falsche Tabellennummer");
    return 0;
}

sal_uInt8 ScDocument::GetRowFlags( SCROW nRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && pTab[nTab] )
        return pTab[nTab]->GetRowFlags( nRow );
    OSL_FAIL("Falsche Tabellennummer");
    return 0;
}

ScBitMaskCompressedArray< SCROW, sal_uInt8> & ScDocument::GetRowFlagsArrayModifiable(
        SCTAB nTab )
{
    return const_cast< ScBitMaskCompressedArray< SCROW, sal_uInt8> & >(
            GetRowFlagsArray( nTab));
}

const ScBitMaskCompressedArray< SCROW, sal_uInt8> & ScDocument::GetRowFlagsArray(
        SCTAB nTab ) const
{
    const ScBitMaskCompressedArray< SCROW, sal_uInt8> * pFlags;
    if ( ValidTab(nTab) && pTab[nTab] )
        pFlags = pTab[nTab]->GetRowFlagsArray();
    else
    {
        OSL_FAIL("wrong sheet number");
        pFlags = 0;
    }
    if (!pFlags)
    {
        OSL_FAIL("no row flags at sheet");
        static ScBitMaskCompressedArray< SCROW, sal_uInt8> aDummy( MAXROW, 0);
        pFlags = &aDummy;
    }
    return *pFlags;
}

void ScDocument::GetAllRowBreaks(set<SCROW>& rBreaks, SCTAB nTab, bool bPage, bool bManual) const
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return;

    pTab[nTab]->GetAllRowBreaks(rBreaks, bPage, bManual);
}

void ScDocument::GetAllColBreaks(set<SCCOL>& rBreaks, SCTAB nTab, bool bPage, bool bManual) const
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return;

    pTab[nTab]->GetAllColBreaks(rBreaks, bPage, bManual);
}

ScBreakType ScDocument::HasRowBreak(SCROW nRow, SCTAB nTab) const
{
    ScBreakType nType = BREAK_NONE;
    if (!ValidTab(nTab) || !pTab[nTab] || !ValidRow(nRow))
        return nType;

    if (pTab[nTab]->HasRowPageBreak(nRow))
        nType |= BREAK_PAGE;

    if (pTab[nTab]->HasRowManualBreak(nRow))
        nType |= BREAK_MANUAL;

    return nType;
}

ScBreakType ScDocument::HasColBreak(SCCOL nCol, SCTAB nTab) const
{
    ScBreakType nType = BREAK_NONE;
    if (!ValidTab(nTab) || !pTab[nTab] || !ValidCol(nCol))
        return nType;

    if (pTab[nTab]->HasColPageBreak(nCol))
        nType |= BREAK_PAGE;

    if (pTab[nTab]->HasColManualBreak(nCol))
        nType |= BREAK_MANUAL;

    return nType;
}

void ScDocument::SetRowBreak(SCROW nRow, SCTAB nTab, bool bPage, bool bManual)
{
    if (!ValidTab(nTab) || !pTab[nTab] || !ValidRow(nRow))
        return;

    pTab[nTab]->SetRowBreak(nRow, bPage, bManual);
}

void ScDocument::SetColBreak(SCCOL nCol, SCTAB nTab, bool bPage, bool bManual)
{
    if (!ValidTab(nTab) || !pTab[nTab] || !ValidCol(nCol))
        return;

    pTab[nTab]->SetColBreak(nCol, bPage, bManual);
}

void ScDocument::RemoveRowBreak(SCROW nRow, SCTAB nTab, bool bPage, bool bManual)
{
    if (!ValidTab(nTab) || !pTab[nTab] || !ValidRow(nRow))
        return;

    pTab[nTab]->RemoveRowBreak(nRow, bPage, bManual);
}

void ScDocument::RemoveColBreak(SCCOL nCol, SCTAB nTab, bool bPage, bool bManual)
{
    if (!ValidTab(nTab) || !pTab[nTab] || !ValidCol(nCol))
        return;

    pTab[nTab]->RemoveColBreak(nCol, bPage, bManual);
}

Sequence<TablePageBreakData> ScDocument::GetRowBreakData(SCTAB nTab) const
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return Sequence<TablePageBreakData>();

    return pTab[nTab]->GetRowBreakData();
}

bool ScDocument::RowHidden(SCROW nRow, SCTAB nTab, SCROW* pFirstRow, SCROW* pLastRow)
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return false;

    return pTab[nTab]->RowHidden(nRow, pFirstRow, pLastRow);
}

bool ScDocument::HasHiddenRows(SCROW nStartRow, SCROW nEndRow, SCTAB nTab)
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return false;

    return pTab[nTab]->HasHiddenRows(nStartRow, nEndRow);
}

bool ScDocument::ColHidden(SCCOL nCol, SCTAB nTab, SCCOL* pFirstCol, SCCOL* pLastCol)
{
    if (!ValidTab(nTab) || !pTab[nTab])
    {
        if (pFirstCol)
            *pFirstCol = nCol;
        if (pLastCol)
            *pLastCol = nCol;
        return false;
    }

    return pTab[nTab]->ColHidden(nCol, pFirstCol, pLastCol);
}

void ScDocument::SetRowHidden(SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bHidden)
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return;

    pTab[nTab]->SetRowHidden(nStartRow, nEndRow, bHidden);
}

void ScDocument::SetColHidden(SCCOL nStartCol, SCCOL nEndCol, SCTAB nTab, bool bHidden)
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return;

    pTab[nTab]->SetColHidden(nStartCol, nEndCol, bHidden);
}

SCROW ScDocument::FirstVisibleRow(SCROW nStartRow, SCROW nEndRow, SCTAB nTab)
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return ::std::numeric_limits<SCROW>::max();;

    return pTab[nTab]->FirstVisibleRow(nStartRow, nEndRow);
}

SCROW ScDocument::LastVisibleRow(SCROW nStartRow, SCROW nEndRow, SCTAB nTab)
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return ::std::numeric_limits<SCROW>::max();;

    return pTab[nTab]->LastVisibleRow(nStartRow, nEndRow);
}

SCROW ScDocument::CountVisibleRows(SCROW nStartRow, SCROW nEndRow, SCTAB nTab)
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return 0;

    return pTab[nTab]->CountVisibleRows(nStartRow, nEndRow);
}

bool ScDocument::RowFiltered(SCROW nRow, SCTAB nTab, SCROW* pFirstRow, SCROW* pLastRow)
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return false;

    return pTab[nTab]->RowFiltered(nRow, pFirstRow, pLastRow);
}

bool ScDocument::HasFilteredRows(SCROW nStartRow, SCROW nEndRow, SCTAB nTab)
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return false;

    return pTab[nTab]->HasFilteredRows(nStartRow, nEndRow);
}

bool ScDocument::ColFiltered(SCCOL nCol, SCTAB nTab, SCCOL* pFirstCol, SCCOL* pLastCol)
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return false;

    return pTab[nTab]->ColFiltered(nCol, pFirstCol, pLastCol);
}

void ScDocument::SetRowFiltered(SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bFiltered)
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return;

    pTab[nTab]->SetRowFiltered(nStartRow, nEndRow, bFiltered);
}


SCROW ScDocument::FirstNonFilteredRow(SCROW nStartRow, SCROW nEndRow, SCTAB nTab)
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return ::std::numeric_limits<SCROW>::max();;

    return pTab[nTab]->FirstNonFilteredRow(nStartRow, nEndRow);
}

SCROW ScDocument::LastNonFilteredRow(SCROW nStartRow, SCROW nEndRow, SCTAB nTab)
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return ::std::numeric_limits<SCROW>::max();;

    return pTab[nTab]->LastNonFilteredRow(nStartRow, nEndRow);
}

SCROW ScDocument::CountNonFilteredRows(SCROW nStartRow, SCROW nEndRow, SCTAB nTab)
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return 0;

    return pTab[nTab]->CountNonFilteredRows(nStartRow, nEndRow);
}

void ScDocument::SyncColRowFlags()
{
    for (SCTAB i = 0; i <= nMaxTableNumber; ++i)
    {
        if (!ValidTab(i) || !pTab[i])
            continue;

        pTab[i]->SyncColRowFlags();
    }
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
        sal_uInt8 nStartFlags = pTab[nTab]->GetColFlags(nStart);
        sal_uInt16 nStartWidth = pTab[nTab]->GetOriginalWidth(nStart);
        for (SCCOL nCol = nStart + 1; nCol <= MAXCOL; nCol++)
        {
            if (((nStartFlags & CR_MANUALBREAK) != (pTab[nTab]->GetColFlags(nCol) & CR_MANUALBREAK)) ||
                (nStartWidth != pTab[nTab]->GetOriginalWidth(nCol)) ||
                ((nStartFlags & CR_HIDDEN) != (pTab[nTab]->GetColFlags(nCol) & CR_HIDDEN)) )
                return nCol;
        }
        return MAXCOL+1;
    }
    return 0;
}

SCROW ScDocument::GetNextDifferentChangedRow( SCTAB nTab, SCROW nStart, bool bCareManualSize) const
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return 0;

    const ScBitMaskCompressedArray<SCROW, sal_uInt8>* pRowFlagsArray = pTab[nTab]->GetRowFlagsArray();
    if (!pRowFlagsArray)
        return 0;

    if (!pTab[nTab]->mpRowHeights || !pTab[nTab]->mpHiddenRows)
        return 0;

    size_t nIndex;          // ignored
    SCROW nFlagsEndRow;
    SCROW nHiddenEndRow;
    SCROW nHeightEndRow;
    sal_uInt8 nFlags;
    bool bHidden;
    sal_uInt16 nHeight;
    sal_uInt8 nStartFlags = nFlags = pRowFlagsArray->GetValue( nStart, nIndex, nFlagsEndRow);
    bool bStartHidden = bHidden = pTab[nTab]->RowHidden( nStart, NULL, &nHiddenEndRow);
    sal_uInt16 nStartHeight = nHeight = pTab[nTab]->GetRowHeight( nStart, NULL, &nHeightEndRow, false);
    SCROW nRow;
    while ((nRow = std::min( nHiddenEndRow, std::min( nFlagsEndRow, nHeightEndRow)) + 1) <= MAXROW)
    {
        if (nFlagsEndRow < nRow)
            nFlags = pRowFlagsArray->GetValue( nRow, nIndex, nFlagsEndRow);
        if (nHiddenEndRow < nRow)
            bHidden = pTab[nTab]->RowHidden( nRow, NULL, &nHiddenEndRow);
        if (nHeightEndRow < nRow)
            nHeight = pTab[nTab]->GetRowHeight( nRow, NULL, &nHeightEndRow, false);

        if (((nStartFlags & CR_MANUALBREAK) != (nFlags & CR_MANUALBREAK)) ||
            ((nStartFlags & CR_MANUALSIZE) != (nFlags & CR_MANUALSIZE)) ||
            (bStartHidden != bHidden) ||
            (bCareManualSize && (nStartFlags & CR_MANUALSIZE) && (nStartHeight != nHeight)) ||
            (!bCareManualSize && ((nStartHeight != nHeight))))
            return nRow;
    }

    return MAXROW+1;
}

sal_Bool ScDocument::GetColDefault( SCTAB nTab, SCCOL nCol, SCROW nLastRow, SCROW& nDefault)
{
    sal_Bool bRet(false);
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
        ++aItr;
        while (aItr != aSet.end())
        {
            // for entries with equal count, use the one with the lowest start row,
            // don't use the random order of pointer comparisons
            if ( aItr->nCount > aDefaultItr->nCount ||
                 ( aItr->nCount == aDefaultItr->nCount && aItr->nFirst < aDefaultItr->nFirst ) )
                aDefaultItr = aItr;
            ++aItr;
        }
        nDefault = aDefaultItr->nFirst;
        bRet = sal_True;
    }
    else
        bRet = sal_True;
    return bRet;
}

sal_Bool ScDocument::GetRowDefault( SCTAB /* nTab */, SCROW /* nRow */, SCCOL /* nLastCol */, SCCOL& /* nDefault */ )
{
    sal_Bool bRet(false);
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

const SfxPoolItem* ScDocument::GetAttr( SCCOL nCol, SCROW nRow, SCTAB nTab, sal_uInt16 nWhich ) const
{
    if ( ValidTab(nTab)  && pTab[nTab] )
    {
        const SfxPoolItem* pTemp = pTab[nTab]->GetAttr( nCol, nRow, nWhich );
        if (pTemp)
            return pTemp;
        else
        {
            OSL_FAIL( "Attribut Null" );
        }
    }
    return &xPoolHelper->GetDocPool()->GetDefaultItem( nWhich );
}


const ScPatternAttr* ScDocument::GetPattern( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        return pTab[nTab]->GetPattern( nCol, nRow );
    return NULL;
}


const ScPatternAttr* ScDocument::GetMostUsedPattern( SCCOL nCol, SCROW nStartRow, SCROW nEndRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        return pTab[nTab]->GetMostUsedPattern( nCol, nStartRow, nEndRow );
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
                        const ScPatternAttr& rAttr,
                        ScEditDataArray* pDataArray )
{
    for (SCTAB i=0; i <= MAXTAB; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                pTab[i]->ApplyPatternArea( nStartCol, nStartRow, nEndCol, nEndRow, rAttr, pDataArray );
}


void ScDocument::ApplyPatternAreaTab( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow, SCTAB nTab, const ScPatternAttr& rAttr )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->ApplyPatternArea( nStartCol, nStartRow, nEndCol, nEndRow, rAttr );
}

bool ScDocument::SetAttrEntries(SCCOL nCol, SCTAB nTab, ScAttrEntry* pData, SCSIZE nSize)
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return false;

    return pTab[nTab]->SetAttrEntries(nCol, pData, nSize);
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
                    const SvxBorderLine* pLine, sal_Bool bColorOnly )
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
    sal_Bool    bEqual = sal_True;
    sal_Bool    bFound;

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
                        bEqual = false;                                             // unterschiedliche
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
                        bEqual = false;                                             // unterschiedliche
                    pStyle = pNewStyle;
                }
            }
    }

    return bEqual ? pStyle : NULL;
}


void ScDocument::StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, sal_Bool bRemoved,
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
        ScDetectiveFunc::UpdateAllComments( *this );
    }
}


sal_Bool ScDocument::IsStyleSheetUsed( const ScStyleSheet& rStyle, sal_Bool bGatherAllStyles ) const
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

        sal_Bool bIsUsed = false;

        for ( SCTAB i=0; i<=MAXTAB; i++ )
        {
            if ( pTab[i] )
            {
                if ( pTab[i]->IsStyleSheetUsed( rStyle, bGatherAllStyles ) )
                {
                    if ( !bGatherAllStyles )
                        return sal_True;
                    bIsUsed = sal_True;
                }
            }
        }

        if ( bGatherAllStyles )
            bStyleSheetUsageInvalid = false;

        return bIsUsed;
    }

    return rStyle.GetUsage() == ScStyleSheet::USED;
}


sal_Bool ScDocument::ApplyFlagsTab( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow, SCTAB nTab, sal_Int16 nFlags )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->ApplyFlags( nStartCol, nStartRow, nEndCol, nEndRow, nFlags );

    OSL_FAIL("ApplyFlags: falsche Tabelle");
    return false;
}


sal_Bool ScDocument::RemoveFlagsTab( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow, SCTAB nTab, sal_Int16 nFlags )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->RemoveFlags( nStartCol, nStartRow, nEndCol, nEndRow, nFlags );

    OSL_FAIL("RemoveFlags: falsche Tabelle");
    return false;
}


void ScDocument::SetPattern( SCCOL nCol, SCROW nRow, SCTAB nTab, const ScPatternAttr& rAttr,
                                sal_Bool bPutToPool )
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            pTab[nTab]->SetPattern( nCol, nRow, rAttr, bPutToPool );
}


void ScDocument::SetPattern( const ScAddress& rPos, const ScPatternAttr& rAttr,
                                sal_Bool bPutToPool )
{
    SCTAB nTab = rPos.Tab();
    if (pTab[nTab])
        pTab[nTab]->SetPattern( rPos, rAttr, bPutToPool );
}


ScPatternAttr* ScDocument::CreateSelectionPattern( const ScMarkData& rMark, sal_Bool bDeep )
{
    ScMergePatternState aState;

    if ( rMark.IsMultiMarked() )                                // multi selection
    {
        for (SCTAB i=0; i<=MAXTAB; i++)
            if (pTab[i] && rMark.GetTableSelect(i))
                pTab[i]->MergeSelectionPattern( aState, rMark, bDeep );
    }
    if ( rMark.IsMarked() )                                     // simle selection
    {
        ScRange aRange;
        rMark.GetMarkArea(aRange);
        for (SCTAB i=0; i<=MAXTAB; i++)
            if (pTab[i] && rMark.GetTableSelect(i))
                pTab[i]->MergePatternArea( aState,
                                aRange.aStart.Col(), aRange.aStart.Row(),
                                aRange.aEnd.Col(), aRange.aEnd.Row(), bDeep );
    }

    DBG_ASSERT( aState.pItemSet, "SelectionPattern Null" );
    if (aState.pItemSet)
        return new ScPatternAttr( aState.pItemSet );
    else
        return new ScPatternAttr( GetPool() );      // empty
}


const ScPatternAttr* ScDocument::GetSelectionPattern( const ScMarkData& rMark, sal_Bool bDeep )
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
    rLineInner.SetTable(sal_True);
    rLineInner.SetDist(sal_True);
    rLineInner.SetMinDist(false);

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


bool ScDocument::HasAttrib( SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                            SCCOL nCol2, SCROW nRow2, SCTAB nTab2, sal_uInt16 nMask )
{
    if ( nMask & HASATTR_ROTATE )
    {
        //  Attribut im Dokument ueberhaupt verwendet?
        //  (wie in fillinfo)

        ScDocumentPool* pPool = xPoolHelper->GetDocPool();

        sal_Bool bAnyItem = false;
        sal_uInt32 nRotCount = pPool->GetItemCount2( ATTR_ROTATE_VALUE );
        for (sal_uInt32 nItem=0; nItem<nRotCount; nItem++)
        {
            const SfxPoolItem* pItem = pPool->GetItem2( ATTR_ROTATE_VALUE, nItem );
            if ( pItem )
            {
                // 90 or 270 degrees is former SvxOrientationItem - only look for other values
                // (see ScPatternAttr::GetCellOrientation)
                sal_Int32 nAngle = static_cast<const SfxInt32Item*>(pItem)->GetValue();
                if ( nAngle != 0 && nAngle != 9000 && nAngle != 27000 )
                {
                    bAnyItem = sal_True;
                    break;
                }
            }
        }
        if (!bAnyItem)
            nMask &= ~HASATTR_ROTATE;
    }

    if ( nMask & HASATTR_RTL )
    {
        //  first check if right-to left is in the pool at all
        //  (the same item is used in cell and page format)

        ScDocumentPool* pPool = xPoolHelper->GetDocPool();

        sal_Bool bHasRtl = false;
        sal_uInt32 nDirCount = pPool->GetItemCount2( ATTR_WRITINGDIR );
        for (sal_uInt32 nItem=0; nItem<nDirCount; nItem++)
        {
            const SfxPoolItem* pItem = pPool->GetItem2( ATTR_WRITINGDIR, nItem );
            if ( pItem && ((const SvxFrameDirectionItem*)pItem)->GetValue() == FRMDIR_HORI_RIGHT_TOP )
            {
                bHasRtl = sal_True;
                break;
            }
        }
        if (!bHasRtl)
            nMask &= ~HASATTR_RTL;
    }

    if (!nMask)
        return false;

    bool bFound = false;
    for (SCTAB i=nTab1; i<=nTab2 && !bFound; i++)
        if (pTab[i])
        {
            if ( nMask & HASATTR_RTL )
            {
                if ( GetEditTextDirection(i) == EE_HTEXTDIR_R2L )       // sheet default
                    bFound = true;
            }
            if ( nMask & HASATTR_RIGHTORCENTER )
            {
                //  On a RTL sheet, don't start to look for the default left value
                //  (which is then logically right), instead always assume sal_True.
                //  That way, ScAttrArray::HasAttrib doesn't have to handle RTL sheets.

                if ( IsLayoutRTL(i) )
                    bFound = true;
            }

            if ( !bFound )
                bFound = pTab[i]->HasAttrib( nCol1, nRow1, nCol2, nRow2, nMask );
        }

    return bFound;
}

bool ScDocument::HasAttrib( const ScRange& rRange, sal_uInt16 nMask )
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
    {
        DBG_ERRORFILE("FindMaxRotCol: falsche Tabelle");
    }
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
        if ( ScHasPriority( pOther, pLeftLine ) )
            pLeftLine = pOther;
    }
    if ( nRow > 0 )
    {
        const SvxBorderLine* pOther = ((const SvxBoxItem*)
                                GetEffItem( nCol, nRow-1, nTab, ATTR_BORDER ))->GetBottom();
        if ( ScHasPriority( pOther, pTopLine ) )
            pTopLine = pOther;
    }
    if ( nCol < MAXCOL )
    {
        const SvxBorderLine* pOther = ((const SvxBoxItem*)
                                GetEffItem( nCol+1, nRow, nTab, ATTR_BORDER ))->GetLeft();
        if ( ScHasPriority( pOther, pRightLine ) )
            pRightLine = pOther;
    }
    if ( nRow < MAXROW )
    {
        const SvxBorderLine* pOther = ((const SvxBoxItem*)
                                GetEffItem( nCol, nRow+1, nTab, ATTR_BORDER ))->GetTop();
        if ( ScHasPriority( pOther, pBottomLine ) )
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

sal_Bool ScDocument::IsBlockEmpty( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow, bool bIgnoreNotes ) const
{
    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->IsBlockEmpty( nStartCol, nStartRow, nEndCol, nEndRow, bIgnoreNotes );

    OSL_FAIL("Falsche Tabellennummer");
    return false;
}


void ScDocument::LockTable(SCTAB nTab)
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        pTab[nTab]->LockTable();
    else
    {
        OSL_FAIL("Falsche Tabellennummer");
    }
}


void ScDocument::UnlockTable(SCTAB nTab)
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        pTab[nTab]->UnlockTable();
    else
    {
        OSL_FAIL("Falsche Tabellennummer");
    }
}


sal_Bool ScDocument::IsBlockEditable( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow,
                                        sal_Bool* pOnlyNotBecauseOfMatrix /* = NULL */ ) const
{
    // import into read-only document is possible
    if ( !bImportingXML && !mbChangeReadOnlyEnabled && pShell && pShell->IsReadOnly() )
    {
        if ( pOnlyNotBecauseOfMatrix )
            *pOnlyNotBecauseOfMatrix = false;
        return false;
    }

    if (VALIDTAB(nTab))
        if (pTab[nTab])
            return pTab[nTab]->IsBlockEditable( nStartCol, nStartRow, nEndCol,
                nEndRow, pOnlyNotBecauseOfMatrix );

    OSL_FAIL("Falsche Tabellennummer");
    if ( pOnlyNotBecauseOfMatrix )
        *pOnlyNotBecauseOfMatrix = false;
    return false;
}


sal_Bool ScDocument::IsSelectionEditable( const ScMarkData& rMark,
            sal_Bool* pOnlyNotBecauseOfMatrix /* = NULL */ ) const
{
    // import into read-only document is possible
    if ( !bImportingXML && !mbChangeReadOnlyEnabled && pShell && pShell->IsReadOnly() )
    {
        if ( pOnlyNotBecauseOfMatrix )
            *pOnlyNotBecauseOfMatrix = false;
        return false;
    }

    ScRange aRange;
    rMark.GetMarkArea(aRange);

    sal_Bool bOk = sal_True;
    sal_Bool bMatrix = ( pOnlyNotBecauseOfMatrix != NULL );
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
                    bOk = false;
                    if ( pOnlyNotBecauseOfMatrix )
                        bMatrix = *pOnlyNotBecauseOfMatrix;
                }
            }
            if (rMark.IsMultiMarked())
            {
                if ( !pTab[i]->IsSelectionEditable( rMark, pOnlyNotBecauseOfMatrix ) )
                {
                    bOk = false;
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


sal_Bool ScDocument::HasSelectedBlockMatrixFragment( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow,
                                const ScMarkData& rMark ) const
{
    sal_Bool bOk = sal_True;
    for (SCTAB i=0; i<=MAXTAB && bOk; i++)
        if (pTab[i])
            if (rMark.GetTableSelect(i))
                if (pTab[i]->HasBlockMatrixFragment( nStartCol, nStartRow, nEndCol, nEndRow ))
                    bOk = false;

    return !bOk;
}

sal_Bool ScDocument::HasSelectedBlockMatrixFragment( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, SCTAB nTab ) const
{
    sal_Bool bOk = true;
    if ( pTab[nTab] && pTab[nTab]->HasBlockMatrixFragment( nStartCol, nStartRow, nEndCol, nEndRow ) )
    {
        bOk = false;
    }
    return !bOk;
}

sal_Bool ScDocument::GetMatrixFormulaRange( const ScAddress& rCellPos, ScRange& rMatrix )
{
    //  if rCell is part of a matrix formula, return its complete range

    sal_Bool bRet = false;
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
                    bRet = sal_True;
                }
            }
        }
    }
    return bRet;
}


sal_Bool ScDocument::ExtendOverlapped( SCCOL& rStartCol, SCROW& rStartRow,
                                SCCOL nEndCol, SCROW nEndRow, SCTAB nTab )
{
    sal_Bool bFound = false;
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
    {
        OSL_FAIL("ExtendOverlapped: falscher Bereich");
    }

    return bFound;
}


sal_Bool ScDocument::ExtendMergeSel( SCCOL nStartCol, SCROW nStartRow,
                              SCCOL& rEndCol, SCROW& rEndRow,
                              const ScMarkData& rMark, sal_Bool bRefresh, sal_Bool bAttrs )
{
    // use all selected sheets from rMark

    sal_Bool bFound = false;
    SCCOL nOldEndCol = rEndCol;
    SCROW nOldEndRow = rEndRow;

    for (SCTAB nTab = 0; nTab <= MAXTAB; nTab++)
        if ( pTab[nTab] && rMark.GetTableSelect(nTab) )
        {
            SCCOL nThisEndCol = nOldEndCol;
            SCROW nThisEndRow = nOldEndRow;
            if ( ExtendMerge( nStartCol, nStartRow, nThisEndCol, nThisEndRow, nTab, bRefresh, bAttrs ) )
                bFound = sal_True;
            if ( nThisEndCol > rEndCol )
                rEndCol = nThisEndCol;
            if ( nThisEndRow > rEndRow )
                rEndRow = nThisEndRow;
        }

    return bFound;
}


sal_Bool ScDocument::ExtendMerge( SCCOL nStartCol, SCROW nStartRow,
                              SCCOL& rEndCol,  SCROW& rEndRow,
                              SCTAB nTab, sal_Bool bRefresh, sal_Bool bAttrs )
{
    sal_Bool bFound = false;
    if ( ValidColRow(nStartCol,nStartRow) && ValidColRow(rEndCol,rEndRow) && ValidTab(nTab) )
    {
        if (pTab[nTab])
            bFound = pTab[nTab]->ExtendMerge( nStartCol, nStartRow, rEndCol, rEndRow, bRefresh, bAttrs );

        if (bRefresh)
            RefreshAutoFilter( nStartCol, nStartRow, rEndCol, rEndRow, nTab );
    }
    else
    {
        OSL_FAIL("ExtendMerge: falscher Bereich");
    }

    return bFound;
}


sal_Bool ScDocument::ExtendMerge( ScRange& rRange, sal_Bool bRefresh, sal_Bool bAttrs )
{
    sal_Bool bFound = false;
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
            bFound = sal_True;
            if (nExtendCol > nEndCol) nEndCol = nExtendCol;
            if (nExtendRow > nEndRow) nEndRow = nExtendRow;
        }
    }

    rRange.aEnd.SetCol(nEndCol);
    rRange.aEnd.SetRow(nEndRow);

    return bFound;
}

sal_Bool ScDocument::ExtendTotalMerge( ScRange& rRange )
{
    //  Bereich genau dann auf zusammengefasste Zellen erweitern, wenn
    //  dadurch keine neuen nicht-ueberdeckten Zellen getroffen werden

    sal_Bool bRet = false;
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

sal_Bool ScDocument::ExtendOverlapped( ScRange& rRange )
{
    sal_Bool bFound = false;
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
            bFound = sal_True;
        }
        if (nExtendRow < nStartRow)
        {
            nStartRow = nExtendRow;
            bFound = sal_True;
        }
    }

    rRange.aStart.SetCol(nStartCol);
    rRange.aStart.SetRow(nStartRow);

    return bFound;
}

sal_Bool ScDocument::RefreshAutoFilter( SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow, SCTAB nTab )
{
    sal_uInt16 nCount = pDBCollection->GetCount();
    sal_uInt16 i;
    ScDBData* pData;
    SCTAB nDBTab;
    SCCOL nDBStartCol;
    SCROW nDBStartRow;
    SCCOL nDBEndCol;
    SCROW nDBEndRow;

    //      Autofilter loeschen

    sal_Bool bChange = RemoveFlagsTab( nStartCol,nStartRow, nEndCol,nEndRow, nTab, SC_MF_AUTO );

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
                    bChange = sal_True;
            }
        }
    }
    return bChange;
}

void ScDocument::SkipOverlapped( SCCOL& rCol, SCROW& rRow, SCTAB nTab ) const
{
    while (IsHorOverlapped(rCol, rRow, nTab))
        --rCol;
    while (IsVerOverlapped(rCol, rRow, nTab))
        --rRow;
}

sal_Bool ScDocument::IsHorOverlapped( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    const ScMergeFlagAttr* pAttr = (const ScMergeFlagAttr*)
                                        GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG );
    if (pAttr)
        return pAttr->IsHorOverlapped();
    else
    {
        OSL_FAIL("Overlapped: Attr==0");
        return false;
    }
}


sal_Bool ScDocument::IsVerOverlapped( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    const ScMergeFlagAttr* pAttr = (const ScMergeFlagAttr*)
                                        GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG );
    if (pAttr)
        return pAttr->IsVerOverlapped();
    else
    {
        OSL_FAIL("Overlapped: Attr==0");
        return false;
    }
}


void ScDocument::ApplySelectionFrame( const ScMarkData& rMark,
                                      const SvxBoxItem* pLineOuter,
                                      const SvxBoxInfoItem* pLineInner )
{
    ScRangeList aRangeList;
    rMark.FillRangeListWithMarks( &aRangeList, false );
    size_t nRangeCount = aRangeList.size();
    for (SCTAB i=0; i<=MAXTAB; i++)
    {
        if (pTab[i] && rMark.GetTableSelect(i))
        {
            for ( size_t j=0; j < nRangeCount; j++ )
            {
                ScRange aRange = *aRangeList[ j ];
                pTab[i]->ApplyBlockFrame( pLineOuter, pLineInner,
                    aRange.aStart.Col(), aRange.aStart.Row(),
                    aRange.aEnd.Col(),   aRange.aEnd.Row() );
            }
        }
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


void ScDocument::ApplySelectionPattern( const ScPatternAttr& rAttr, const ScMarkData& rMark, ScEditDataArray* pDataArray )
{
    const SfxItemSet* pSet = &rAttr.GetItemSet();
    sal_Bool bSet = false;
    sal_uInt16 i;
    for (i=ATTR_PATTERN_START; i<=ATTR_PATTERN_END && !bSet; i++)
        if (pSet->GetItemState(i) == SFX_ITEM_SET)
            bSet = sal_True;

    if (bSet)
    {
        // ApplySelectionCache needs multi mark
        if ( rMark.IsMarked() && !rMark.IsMultiMarked() )
        {
            ScRange aRange;
            rMark.GetMarkArea( aRange );
            ApplyPatternArea( aRange.aStart.Col(), aRange.aStart.Row(),
                              aRange.aEnd.Col(), aRange.aEnd.Row(), rMark, rAttr, pDataArray );
        }
        else
        {
            SfxItemPoolCache aCache( xPoolHelper->GetDocPool(), pSet );
            for (SCTAB nTab=0; nTab<=MAXTAB; nTab++)
                if (pTab[nTab])
                    if (rMark.GetTableSelect(nTab))
                        pTab[nTab]->ApplySelectionCache( &aCache, rMark, pDataArray );
        }
    }
}


void ScDocument::ChangeSelectionIndent( sal_Bool bIncrement, const ScMarkData& rMark )
{
    for (SCTAB i=0; i<=MAXTAB; i++)
        if (pTab[i] && rMark.GetTableSelect(i))
            pTab[i]->ChangeSelectionIndent( bIncrement, rMark );
}


void ScDocument::ClearSelectionItems( const sal_uInt16* pWhich, const ScMarkData& rMark )
{
    for (SCTAB i=0; i<=MAXTAB; i++)
        if (pTab[i] && rMark.GetTableSelect(i))
            pTab[i]->ClearSelectionItems( pWhich, rMark );
}


void ScDocument::DeleteSelection( sal_uInt16 nDelFlag, const ScMarkData& rMark )
{
    for (SCTAB i=0; i<=MAXTAB; i++)
        if (pTab[i] && rMark.GetTableSelect(i))
            pTab[i]->DeleteSelection( nDelFlag, rMark );
}


void ScDocument::DeleteSelectionTab( SCTAB nTab, sal_uInt16 nDelFlag, const ScMarkData& rMark )
{
    if (ValidTab(nTab)  && pTab[nTab])
        pTab[nTab]->DeleteSelection( nDelFlag, rMark );
    else
    {
        OSL_FAIL("Falsche Tabelle");
    }
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
                                sal_Bool bMarked, sal_Bool bUnprotected, const ScMarkData& rMark )
{
    DBG_ASSERT( !nMovX || !nMovY, "GetNextPos: nur X oder Y" );

    ScMarkData aCopyMark = rMark;
    aCopyMark.SetMarking(false);
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

    sal_uInt32 nCount = pPool->GetItemCount2(ATTR_PATTERN);
    ScPatternAttr* pPattern;
    for (sal_uInt32 i=0; i<nCount; i++)
    {
        pPattern = (ScPatternAttr*)pPool->GetItem2(ATTR_PATTERN, i);
        if (pPattern)
            pPattern->UpdateStyleSheet();
    }
    ((ScPatternAttr&)pPool->GetDefaultItem(ATTR_PATTERN)).UpdateStyleSheet();
}


void ScDocument::StylesToNames()
{
    ScPatternAttr::pDoc = this;

    ScDocumentPool* pPool = xPoolHelper->GetDocPool();

    sal_uInt32 nCount = pPool->GetItemCount2(ATTR_PATTERN);
    ScPatternAttr* pPattern;
    for (sal_uInt32 i=0; i<nCount; i++)
    {
        pPattern = (ScPatternAttr*)pPool->GetItem2(ATTR_PATTERN, i);
        if (pPattern)
            pPattern->StyleToName();
    }
    ((ScPatternAttr&)pPool->GetDefaultItem(ATTR_PATTERN)).StyleToName();
}


sal_uLong ScDocument::GetCellCount() const
{
    sal_uLong nCellCount = 0L;

    for ( SCTAB nTab=0; nTab<=MAXTAB; nTab++ )
        if ( pTab[nTab] )
            nCellCount += pTab[nTab]->GetCellCount();

    return nCellCount;
}

SCSIZE ScDocument::GetCellCount(SCTAB nTab, SCCOL nCol) const
{
    if (!ValidTab(nTab) || !pTab[nTab])
        return 0;

    return pTab[nTab]->GetCellCount(nCol);
}

sal_uLong ScDocument::GetCodeCount() const
{
    sal_uLong nCodeCount = 0;

    for ( SCTAB nTab=0; nTab<=MAXTAB; nTab++ )
        if ( pTab[nTab] )
            nCodeCount += pTab[nTab]->GetCodeCount();

    return nCodeCount;
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

    OSL_FAIL("falsche Tab");
    return Size();
}


void ScDocument::SetRepeatArea( SCTAB nTab, SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCROW nEndRow )
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        pTab[nTab]->SetRepeatArea( nStartCol, nEndCol, nStartRow, nEndRow );
}

void ScDocument::InvalidatePageBreaks(SCTAB nTab)
{
    if (ValidTab(nTab) && pTab[nTab])
        pTab[nTab]->InvalidatePageBreaks();
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

sal_Bool ScDocument::HasManualBreaks( SCTAB nTab ) const
{
    if ( ValidTab(nTab)  && pTab[nTab] )
        return pTab[nTab]->HasManualBreaks();

    OSL_FAIL("falsche Tab");
    return false;
}


void ScDocument::GetDocStat( ScDocStat& rDocStat )
{
    rDocStat.nTableCount = GetTableCount();
    rDocStat.aDocName    = aDocName;
    rDocStat.nCellCount  = GetCellCount();
}


sal_Bool ScDocument::HasPrintRange()
{
    sal_Bool bResult = false;

    for ( SCTAB i=0; !bResult && i<nMaxTableNumber; i++ )
        if ( pTab[i] )
            bResult = pTab[i]->IsPrintEntireSheet() || (pTab[i]->GetPrintRangeCount() > 0);

    return bResult;
}


sal_Bool ScDocument::IsPrintEntireSheet( SCTAB nTab ) const
{
    return (ValidTab(nTab) ) && pTab[nTab] && pTab[nTab]->IsPrintEntireSheet();
}


sal_uInt16 ScDocument::GetPrintRangeCount( SCTAB nTab )
{
    if (ValidTab(nTab) && pTab[nTab])
        return pTab[nTab]->GetPrintRangeCount();

    return 0;
}


const ScRange* ScDocument::GetPrintRange( SCTAB nTab, sal_uInt16 nPos )
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


sal_Bool ScDocument::NeedPageResetAfterTab( SCTAB nTab ) const
{
    //  Die Seitennummern-Zaehlung faengt bei einer Tabelle neu an, wenn eine
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
                sal_uInt16 nFirst = ((const SfxUInt16Item&)rSet.Get(ATTR_PAGE_FIRSTPAGENO)).GetValue();
                if ( nFirst != 0 )
                    return sal_True;        // Seitennummer in neuer Vorlage angegeben
            }
        }
    }

    return false;       // sonst nicht
}

SfxUndoManager* ScDocument::GetUndoManager()
{
    if (!mpUndoManager)
        mpUndoManager = new SfxUndoManager;
    return mpUndoManager;
}

ScRowBreakIterator* ScDocument::GetRowBreakIterator(SCTAB nTab) const
{
    if (ValidTab(nTab) && pTab[nTab])
        return new ScRowBreakIterator(pTab[nTab]->maRowPageBreaks);
    return NULL;
}

void ScDocument::AddSubTotalCell(ScFormulaCell* pCell)
{
    maSubTotalCells.insert(pCell);
}

void ScDocument::RemoveSubTotalCell(ScFormulaCell* pCell)
{
    maSubTotalCells.erase(pCell);
}

namespace {

bool lcl_hasDirtyRange(ScFormulaCell* pCell, const ScRange& rDirtyRange)
{
    ScDetectiveRefIter aRefIter(pCell);
    ScRange aRange;
    while (aRefIter.GetNextRef(aRange))
    {
        if (aRange.Intersects(rDirtyRange))
            return true;
    }
    return false;
}

}

void ScDocument::SetSubTotalCellsDirty(const ScRange& rDirtyRange)
{
    // to update the list by skipping cells that no longer contain subtotal function.
    set<ScFormulaCell*> aNewSet;

    bool bOldRecalc = GetAutoCalc();
    SetAutoCalc(false);
    set<ScFormulaCell*>::iterator itr = maSubTotalCells.begin(), itrEnd = maSubTotalCells.end();
    for (; itr != itrEnd; ++itr)
    {
        ScFormulaCell* pCell = *itr;
        if (pCell->IsSubTotal())
        {
            aNewSet.insert(pCell);
            if (lcl_hasDirtyRange(pCell, rDirtyRange))
                pCell->SetDirty();
        }
    }

    SetAutoCalc(bOldRecalc);
    maSubTotalCells.swap(aNewSet); // update the list.
}

void ScDocument::EnableUndo( bool bVal )
{
    GetUndoManager()->EnableUndo(bVal);
    mbUndoEnabled = bVal;
}

bool ScDocument::IsInVBAMode() const
{
    bool bResult = false;
    if ( pShell )
    {
        com::sun::star::uno::Reference< com::sun::star::script::vba::XVBACompatibility > xVBA( pShell->GetBasicContainer(), com::sun::star::uno::UNO_QUERY );
        bResult = xVBA.is() && xVBA->getVBACompatibilityMode();
    }
    return bResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
