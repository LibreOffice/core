/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <editeng/boxitem.hxx>
#include <editeng/frmdiritem.hxx>
#include "editeng/editobj.hxx"
#include <svx/pageitem.hxx>
#include <editeng/editeng.hxx>
#include <svx/sdrundomanager.hxx>
#include <svx/svditer.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdocapt.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <svl/poolcach.hxx>
#include <unotools/saveopt.hxx>
#include <svl/zforlist.hxx>
#include <unotools/charclass.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <tools/tenccvt.hxx>
#include <tools/urlobj.hxx>

#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/sheet/TablePageBreakData.hpp>
#include <com/sun/star/lang/NotInitializedException.hpp>

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
#include "dbdata.hxx"
#include "pivot.hxx"
#include "chartlis.hxx"
#include "rangelst.hxx"
#include "markdata.hxx"
#include "drwlayer.hxx"
#include "conditio.hxx"
#include "colorscale.hxx"
#include "validat.hxx"
#include "prnsave.hxx"
#include "chgtrack.hxx"
#include "sc.hrc"
#include "scresid.hxx"
#include "hints.hxx"
#include "detdata.hxx"
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
#include "stlalgorithm.hxx"
#include "defaultsoptions.hxx"
#include "editutil.hxx"
#include "stringutil.hxx"
#include "formulaiter.hxx"
#include "formulacell.hxx"
#include "clipcontext.hxx"
#include "listenercontext.hxx"
#include "scopetools.hxx"
#include "refupdatecontext.hxx"

#include <map>
#include <limits>
#include <boost/scoped_ptr.hpp>

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;

namespace WritingMode2 = ::com::sun::star::text::WritingMode2;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::sheet::TablePageBreakData;
using ::std::set;

namespace {

std::pair<SCTAB,SCTAB> getMarkedTableRange(const std::vector<ScTable*>& rTables, const ScMarkData& rMark)
{
    SCTAB nTabStart = MAXTAB;
    SCTAB nTabEnd = 0;
    SCTAB nMax = static_cast<SCTAB>(rTables.size());
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nMax; ++itr)
    {
        if (!rTables[*itr])
            continue;

        if (*itr < nTabStart)
            nTabStart = *itr;
        nTabEnd = *itr;
    }

    return std::pair<SCTAB,SCTAB>(nTabStart,nTabEnd);
}

}

struct ScDefaultAttr
{
    const ScPatternAttr*    pAttr;
    SCROW                   nFirst;
    SCSIZE                  nCount;
    ScDefaultAttr(const ScPatternAttr* pPatAttr) : pAttr(pPatAttr), nFirst(0), nCount(0) {}
};

struct ScLessDefaultAttr
{
    bool operator() (const ScDefaultAttr& rValue1, const ScDefaultAttr& rValue2) const
    {
        return rValue1.pAttr < rValue2.pAttr;
    }
};

typedef std::set<ScDefaultAttr, ScLessDefaultAttr>  ScDefaultAttrSet;

void ScDocument::MakeTable( SCTAB nTab,bool _bNeedsNameCheck )
{
    if ( ValidTab(nTab) && ( nTab >= static_cast<SCTAB>(maTabs.size()) ||!maTabs[nTab]) )
    {
        // Get Custom prefix
        const ScDefaultsOptions& rOpt = SC_MOD()->GetDefaultsOptions();
        OUString aString = rOpt.GetInitTabPrefix();

        aString += OUString::valueOf(static_cast<sal_Int32>(nTab+1));
        if ( _bNeedsNameCheck )
            CreateValidTabName( aString );  // no doubles
        if (nTab < static_cast<SCTAB>(maTabs.size()))
        {
            maTabs[nTab] = new ScTable(this, nTab, aString);
        }
        else
        {
            while(nTab > static_cast<SCTAB>(maTabs.size()))
                maTabs.push_back(NULL);
            maTabs.push_back( new ScTable(this, nTab, aString) );
        }
        maTabs[nTab]->SetLoadingMedium(bLoadingMedium);
    }
}


bool ScDocument::HasTable( SCTAB nTab ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
            return true;

    return false;
}

bool ScDocument::GetName( SCTAB nTab, OUString& rName ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
        {
            maTabs[nTab]->GetName( rName );
            return true;
        }
    rName = OUString();
    return false;
}

OUString ScDocument::GetCopyTabName( SCTAB nTab ) const
{
    if (nTab < static_cast<SCTAB>(maTabNames.size()))
        return maTabNames[nTab];
    else
        return OUString();
}

bool ScDocument::SetCodeName( SCTAB nTab, const OUString& rName )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
    {
        if (maTabs[nTab])
        {
            maTabs[nTab]->SetCodeName( rName );
            return true;
        }
    }
    OSL_TRACE( "**** can't set code name %s", OUStringToOString( rName, RTL_TEXTENCODING_UTF8 ).getStr() );
    return false;
}

bool ScDocument::GetCodeName( SCTAB nTab, OUString& rName ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
        {
            maTabs[nTab]->GetCodeName( rName );
            return true;
        }
    rName = OUString();
    return false;
}

bool ScDocument::GetTable( const OUString& rName, SCTAB& rTab ) const
{
    OUString aUpperName = ScGlobal::pCharClass->uppercase(rName);

    for (SCTAB i=0; i< static_cast<SCTAB>(maTabs.size()); i++)
        if (maTabs[i])
        {
            if (aUpperName.equals(maTabs[i]->GetUpperName()))
            {
                rTab = i;
                return true;
            }
        }
    rTab = 0;
    return false;
}

ScDBData* ScDocument::GetAnonymousDBData(SCTAB nTab)
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetAnonymousDBData();
    return NULL;
}

SCTAB ScDocument::GetTableCount() const
{
    return static_cast<SCTAB>(maTabs.size());
}

void ScDocument::SetAnonymousDBData(SCTAB nTab, ScDBData* pDBData)
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->SetAnonymousDBData(pDBData);
}


bool ScDocument::ValidTabName( const OUString& rName )
{
    if (rName.isEmpty())
        return false;
    sal_Int32 nLen = rName.getLength();

#if 1
    // Restrict sheet names to what Excel accepts.
    /* TODO: We may want to remove this restriction for full ODFF compliance.
     * Merely loading and calculating ODF documents using these characters in
     * sheet names is not affected by this, but all sheet name editing and
     * copying functionality is, maybe falling back to "Sheet4" or similar. */
    for (sal_Int32 i = 0; i < nLen; ++i)
    {
        const sal_Unicode c = rName[i];
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


bool ScDocument::ValidNewTabName( const OUString& rName ) const
{
    bool bValid = ValidTabName(rName);
    TableContainer::const_iterator it = maTabs.begin();
    for (; it != maTabs.end() && bValid; ++it)
        if ( *it )
        {
            OUString aOldName;
            (*it)->GetName(aOldName);
            bValid = !ScGlobal::GetpTransliteration()->isEqual( rName, aOldName );
        }
    return bValid;
}


void ScDocument::CreateValidTabName(OUString& rName) const
{
    if ( !ValidTabName(rName) )
    {
        // Find new one

        // Get Custom prefix
        const ScDefaultsOptions& rOpt = SC_MOD()->GetDefaultsOptions();
        OUString aStrTable = rOpt.GetInitTabPrefix();

        bool         bOk   = false;

        // First test if the prefix is valid, if so only avoid doubles
        bool bPrefix = ValidTabName( aStrTable );
        OSL_ENSURE(bPrefix, "Invalid Table Name");
        SCTAB nDummy;

        for ( SCTAB i = static_cast<SCTAB>(maTabs.size())+1; !bOk ; i++ )
        {
            OUStringBuffer aBuf;
            aBuf.append(aStrTable);
            aBuf.append(static_cast<sal_Int32>(i));
            rName = aBuf.makeStringAndClear();
            if (bPrefix)
                bOk = ValidNewTabName( rName );
            else
                bOk = !GetTable( rName, nDummy );
        }
    }
    else
    {
        // testing the supplied Name

        if ( !ValidNewTabName(rName) )
        {
            SCTAB i = 1;
            OUStringBuffer aName;
            do
            {
                i++;
                aName = rName;
                aName.append('_');
                aName.append(static_cast<sal_Int32>(i));
            }
            while (!ValidNewTabName(aName.toString()) && (i < MAXTAB+1));
            rName = aName.makeStringAndClear();
        }
    }
}

void ScDocument::CreateValidTabNames(std::vector<OUString>& aNames, SCTAB nCount) const
{
    aNames.clear();//ensure that the vector is empty

    // Get Custom prefix
    const ScDefaultsOptions& rOpt = SC_MOD()->GetDefaultsOptions();
    OUString aStrTable = rOpt.GetInitTabPrefix();

    OUStringBuffer rName;
    bool         bOk   = false;

    // First test if the prefix is valid, if so only avoid doubles
    bool bPrefix = ValidTabName( aStrTable );
    OSL_ENSURE(bPrefix, "Invalid Table Name");
    SCTAB nDummy;
    SCTAB i = static_cast<SCTAB>(maTabs.size())+1;

    for (SCTAB j = 0; j < nCount; ++j)
    {
        bOk = false;
        while(!bOk)
        {
            rName = aStrTable;
            rName.append(static_cast<sal_Int32>(i));
            if (bPrefix)
                bOk = ValidNewTabName( rName.toString() );
            else
                bOk = !GetTable( rName.toString(), nDummy );
            i++;
        }
        aNames.push_back(rName.makeStringAndClear());
    }
}

void ScDocument::AppendTabOnLoad(const OUString& rName)
{
    SCTAB nTabCount = static_cast<SCTAB>(maTabs.size());
    if (!ValidTab(nTabCount))
        // max table count reached.  No more tables.
        return;

    OUString aName = rName;
    CreateValidTabName(aName);
    maTabs.push_back( new ScTable(this, nTabCount, aName) );
}

void ScDocument::SetTabNameOnLoad(SCTAB nTab, const OUString& rName)
{
    if (!ValidTab(nTab) || static_cast<SCTAB>(maTabs.size()) <= nTab)
        return;

    if (!ValidTabName(rName))
        return;

    maTabs[nTab]->SetName(rName);
}

void ScDocument::InvalidateStreamOnSave()
{
    TableContainer::iterator it = maTabs.begin(), itEnd = maTabs.end();
    for (; it != itEnd; ++it)
    {
        ScTable* pTab = *it;
        if (pTab)
            pTab->SetStreamValid(false);
    }
}

bool ScDocument::InsertTab( SCTAB nPos, const OUString& rName,
            bool bExternalDocument )
{
    SCTAB   nTabCount = static_cast<SCTAB>(maTabs.size());
    bool    bValid = ValidTab(nTabCount);
    if ( !bExternalDocument )   // else test rName == "'Doc'!Tab" first
        bValid = (bValid && ValidNewTabName(rName));
    if (bValid)
    {
        if (nPos == SC_TAB_APPEND || nPos >= nTabCount)
        {
            maTabs.push_back( new ScTable(this, nTabCount, rName) );
            if ( bExternalDocument )
                maTabs[nTabCount]->SetVisible( false );
        }
        else
        {
            if (ValidTab(nPos) && (nPos < nTabCount))
            {
                sc::RefUpdateInsertTabContext aCxt(nPos, 1);

                ScRange aRange( 0,0,nPos, MAXCOL,MAXROW,MAXTAB );
                xColNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,1 );
                xRowNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,1 );
                if (pRangeName)
                    pRangeName->UpdateInsertTab(aCxt);
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
                TableContainer::iterator it = maTabs.begin();
                for (; it != maTabs.end(); ++it)
                    if ( *it )
                        (*it)->UpdateInsertTab(aCxt);
                maTabs.push_back(NULL);
                for (i = nTabCount; i > nPos; i--)
                {
                    maTabs[i] = maTabs[i - 1];
                }

                maTabs[nPos] = new ScTable(this, nPos, rName);

                // UpdateBroadcastAreas must be called between UpdateInsertTab,
                // which ends listening, and StartAllListeners, to not modify
                // areas that are to be inserted by starting listeners.
                UpdateBroadcastAreas( URM_INSDEL, aRange, 0,0,1);
                it = maTabs.begin();
                for (; it != maTabs.end(); ++it)
                    if ( *it )
                        (*it)->UpdateCompile();
                it = maTabs.begin();
                for (; it != maTabs.end(); ++it)
                    if ( *it )
                        (*it)->StartAllListeners();

                if (pValidationList)
                    pValidationList->UpdateInsertTab(aCxt);

                // sheet names of references are not valid until sheet is inserted
                if ( pChartListenerCollection )
                    pChartListenerCollection->UpdateScheduledSeriesRanges();

                bValid = true;
            }
            else
                bValid = false;
        }
    }

    if (bValid)
        SetDirty();

    return bValid;
}


bool ScDocument::InsertTabs( SCTAB nPos, const std::vector<OUString>& rNames,
            bool bExternalDocument, bool bNamesValid )
{
    SCTAB   nNewSheets = static_cast<SCTAB>(rNames.size());
    SCTAB    nTabCount = static_cast<SCTAB>(maTabs.size());
    bool    bValid = bNamesValid || ValidTab(nTabCount+nNewSheets);
//    if ( !bExternalDocument )    // else test rName == "'Doc'!Tab" first
//        bValid = (bValid && ValidNewTabName(rNames));
    if (bValid)
    {
        if (nPos == SC_TAB_APPEND || nPos >= nTabCount)
        {
            for ( SCTAB i = 0; i < nNewSheets; ++i )
            {
                maTabs.push_back( new ScTable(this, nTabCount + i, rNames.at(i)) );
                if ( bExternalDocument )
                    maTabs[nTabCount+i]->SetVisible( false );
            }
        }
        else
        {
            if (ValidTab(nPos) && (nPos < nTabCount))
            {
                sc::RefUpdateInsertTabContext aCxt(nPos, nNewSheets);
                ScRange aRange( 0,0,nPos, MAXCOL,MAXROW,MAXTAB );
                xColNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,nNewSheets );
                xRowNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,nNewSheets );
                if (pRangeName)
                    pRangeName->UpdateInsertTab(aCxt);
                pDBCollection->UpdateReference(
                                    URM_INSDEL, 0,0,nPos, MAXCOL,MAXROW,MAXTAB, 0,0,nNewSheets );
                if (pDPCollection)
                    pDPCollection->UpdateReference( URM_INSDEL, aRange, 0,0,nNewSheets );
                if (pDetOpList)
                    pDetOpList->UpdateReference( this, URM_INSDEL, aRange, 0,0,nNewSheets );
                UpdateChartRef( URM_INSDEL, 0,0,nPos, MAXCOL,MAXROW,MAXTAB, 0,0,nNewSheets );
                UpdateRefAreaLinks( URM_INSDEL, aRange, 0,0, nNewSheets );
                if ( pUnoBroadcaster )
                    pUnoBroadcaster->Broadcast( ScUpdateRefHint( URM_INSDEL, aRange, 0,0,nNewSheets ) );

                TableContainer::iterator it = maTabs.begin();
                for (; it != maTabs.end(); ++it)
                    if ( *it )
                        (*it)->UpdateInsertTab(aCxt);
                it = maTabs.begin();
                maTabs.insert(it+nPos,nNewSheets, NULL);
                for (SCTAB i = 0; i < nNewSheets; ++i)
                {
                    maTabs[nPos + i] = new ScTable(this, nPos + i, rNames.at(i));
                }

                // UpdateBroadcastAreas must be called between UpdateInsertTab,
                // which ends listening, and StartAllListeners, to not modify
                // areas that are to be inserted by starting listeners.
                UpdateBroadcastAreas( URM_INSDEL, aRange, 0,0,nNewSheets);
                it = maTabs.begin();
                for (; it != maTabs.end(); ++it)
                {
                    if ( *it )
                        (*it)->UpdateCompile();
                }
                it = maTabs.begin();
                for (; it != maTabs.end(); ++it)
                    if ( *it )
                        (*it)->StartAllListeners();

                if (pValidationList)
                    pValidationList->UpdateInsertTab(aCxt);

                // sheet names of references are not valid until sheet is inserted
                if ( pChartListenerCollection )
                    pChartListenerCollection->UpdateScheduledSeriesRanges();

                bValid = true;
            }
            else
                bValid = false;
        }
    }

    if (bValid)
        SetDirty();

    return bValid;
}


bool ScDocument::DeleteTab( SCTAB nTab )
{
    bool bValid = false;
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
    {
        if (maTabs[nTab])
        {
            SCTAB nTabCount = static_cast<SCTAB>(maTabs.size());
            if (nTabCount > 1)
            {
                sc::AutoCalcSwitch aACSwitch(*this, false);
                sc::RefUpdateDeleteTabContext aCxt(nTab, 1);

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

                aRange.aEnd.SetTab( static_cast<SCTAB>(maTabs.size())-1 );
                xColNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,-1 );
                xRowNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,-1 );
                if (pRangeName)
                    pRangeName->UpdateDeleteTab(aCxt);
                pDBCollection->UpdateReference(
                                    URM_INSDEL, 0,0,nTab, MAXCOL,MAXROW,MAXTAB, 0,0,-1 );
                if (pDPCollection)
                    pDPCollection->UpdateReference( URM_INSDEL, aRange, 0,0,-1 );
                if (pDetOpList)
                    pDetOpList->UpdateReference( this, URM_INSDEL, aRange, 0,0,-1 );
                UpdateChartRef( URM_INSDEL, 0,0,nTab, MAXCOL,MAXROW,MAXTAB, 0,0,-1 );
                UpdateRefAreaLinks( URM_INSDEL, aRange, 0,0,-1 );
                if (pValidationList)
                    pValidationList->UpdateDeleteTab(aCxt);
                if ( pUnoBroadcaster )
                    pUnoBroadcaster->Broadcast( ScUpdateRefHint( URM_INSDEL, aRange, 0,0,-1 ) );

                for (SCTAB i = 0, n = static_cast<SCTAB>(maTabs.size()); i < n; ++i)
                    if (maTabs[i])
                        maTabs[i]->UpdateDeleteTab(aCxt);

                TableContainer::iterator it = maTabs.begin() + nTab;
                delete *it;
                maTabs.erase(it);
                // UpdateBroadcastAreas must be called between UpdateDeleteTab,
                // which ends listening, and StartAllListeners, to not modify
                // areas that are to be inserted by starting listeners.
                UpdateBroadcastAreas( URM_INSDEL, aRange, 0,0,-1);
                it = maTabs.begin();
                for (; it != maTabs.end(); ++it)
                    if ( *it )
                        (*it)->UpdateCompile();
                // Excel-Filter deletes some Tables while loading, Listeners will
                // only be triggered after the loading is done.
                if ( !bInsertingFromOtherDoc )
                {
                    it = maTabs.begin();
                    for (; it != maTabs.end(); ++it)
                        if ( *it )
                            (*it)->StartAllListeners();
                    SetDirty();
                }
                // sheet names of references are not valid until sheet is deleted
                pChartListenerCollection->UpdateScheduledSeriesRanges();

                bValid = true;
            }
        }
    }
    return bValid;
}


bool ScDocument::DeleteTabs( SCTAB nTab, SCTAB nSheets )
{
    bool bValid = false;
    if (ValidTab(nTab) && (nTab + nSheets) < static_cast<SCTAB>(maTabs.size()))
    {
        if (maTabs[nTab])
        {
            SCTAB nTabCount = static_cast<SCTAB>(maTabs.size());
            if (nTabCount > nSheets)
            {
                sc::AutoCalcSwitch aACSwitch(*this, false);
                sc::RefUpdateDeleteTabContext aCxt(nTab, nSheets);

                for (SCTAB aTab = 0; aTab < nSheets; ++aTab)
                {
                    ScRange aRange( 0, 0, nTab, MAXCOL, MAXROW, nTab + aTab );
                    DelBroadcastAreasInRange( aRange );

                    // #i8180# remove database ranges etc. that are on the deleted tab
                    // (restored in undo with ScRefUndoData)

                    xColNameRanges->DeleteOnTab( nTab + aTab );
                    xRowNameRanges->DeleteOnTab( nTab + aTab );
                    pDBCollection->DeleteOnTab( nTab + aTab );
                    if (pDPCollection)
                        pDPCollection->DeleteOnTab( nTab + aTab );
                    if (pDetOpList)
                        pDetOpList->DeleteOnTab( nTab + aTab );
                    DeleteAreaLinksOnTab( nTab + aTab );
                }

                if (pRangeName)
                    pRangeName->UpdateDeleteTab(aCxt);

                // normal reference update

                ScRange aRange( 0, 0, nTab, MAXCOL, MAXROW, nTabCount - 1 );
                xColNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,-1*nSheets );
                xRowNameRanges->UpdateReference( URM_INSDEL, this, aRange, 0,0,-1*nSheets );
                pDBCollection->UpdateReference(
                                    URM_INSDEL, 0,0,nTab, MAXCOL,MAXROW,MAXTAB, 0,0,-1*nSheets );
                if (pDPCollection)
                    pDPCollection->UpdateReference( URM_INSDEL, aRange, 0,0,-1*nSheets );
                if (pDetOpList)
                    pDetOpList->UpdateReference( this, URM_INSDEL, aRange, 0,0,-1*nSheets );
                UpdateChartRef( URM_INSDEL, 0,0,nTab, MAXCOL,MAXROW,MAXTAB, 0,0,-1*nSheets );
                UpdateRefAreaLinks( URM_INSDEL, aRange, 0,0,-1*nSheets );
                if (pValidationList)
                    pValidationList->UpdateDeleteTab(aCxt);
                if ( pUnoBroadcaster )
                    pUnoBroadcaster->Broadcast( ScUpdateRefHint( URM_INSDEL, aRange, 0,0,-1*nSheets ) );

                for (SCTAB i = 0, n = static_cast<SCTAB>(maTabs.size()); i < n; ++i)
                    if (maTabs[i])
                        maTabs[i]->UpdateDeleteTab(aCxt);

                TableContainer::iterator it = maTabs.begin() + nTab;
                TableContainer::iterator itEnd = it + nSheets;
                std::for_each(it, itEnd, ScDeleteObjectByPtr<ScTable>());
                maTabs.erase(it, itEnd);
                // UpdateBroadcastAreas must be called between UpdateDeleteTab,
                // which ends listening, and StartAllListeners, to not modify
                // areas that are to be inserted by starting listeners.
                UpdateBroadcastAreas( URM_INSDEL, aRange, 0,0,-1*nSheets);
                it = maTabs.begin();
                for (; it != maTabs.end(); ++it)
                    if ( *it )
                        (*it)->UpdateCompile();
                // Excel-Filter deletes some Tables while loading, Listeners will
                // only be triggered after the loading is done.
                if ( !bInsertingFromOtherDoc )
                {
                    it = maTabs.begin();
                    for (; it != maTabs.end(); ++it)
                        if ( *it )
                            (*it)->StartAllListeners();
                    SetDirty();
                }
                // sheet names of references are not valid until sheet is deleted
                pChartListenerCollection->UpdateScheduledSeriesRanges();

                bValid = true;
            }
        }
    }
    return bValid;
}


bool ScDocument::RenameTab( SCTAB nTab, const OUString& rName, bool /* bUpdateRef */,
        bool bExternalDocument )
{
    bool bValid = false;
    SCTAB i;
    if (ValidTab(nTab))
    {
        if (maTabs[nTab])
        {
            if ( bExternalDocument )
                bValid = true;      // composed name
            else
                bValid = ValidTabName(rName);
            for (i=0; (i< static_cast<SCTAB>(maTabs.size())) && bValid; i++)
                if (maTabs[i] && (i != nTab))
                {
                    OUString aOldName;
                    maTabs[i]->GetName(aOldName);
                    bValid = !ScGlobal::GetpTransliteration()->isEqual( rName, aOldName );
                }
            if (bValid)
            {
                // #i75258# update charts before renaming, so they can get their live data objects.
                // Once the charts are live, the sheet can be renamed without problems.
                if ( pChartListenerCollection )
                    pChartListenerCollection->UpdateChartsContainingTab( nTab );
                maTabs[nTab]->SetName(rName);

                // If formulas refer to the renamed sheet, the TokenArray remains valid,
                // but the XML stream must be re-generated.
                TableContainer::iterator it = maTabs.begin();
                for (; it != maTabs.end(); ++it)
                    if ( *it && (*it)->IsStreamValid())
                        (*it)->SetStreamValid( false );
            }
        }
    }
    return bValid;
}


void ScDocument::SetVisible( SCTAB nTab, bool bVisible )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB> (maTabs.size()))
        if (maTabs[nTab])
            maTabs[nTab]->SetVisible(bVisible);
}


bool ScDocument::IsVisible( SCTAB nTab ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB> (maTabs.size()))
        if (maTabs[nTab])
            return maTabs[nTab]->IsVisible();

    return false;
}


bool ScDocument::IsStreamValid( SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB> (maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->IsStreamValid();

    return false;
}


void ScDocument::SetStreamValid( SCTAB nTab, bool bSet, bool bIgnoreLock )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB> (maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->SetStreamValid( bSet, bIgnoreLock );
}


void ScDocument::LockStreamValid( bool bLock )
{
    mbStreamValidLocked = bLock;
}


bool ScDocument::IsPendingRowHeights( SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB> (maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->IsPendingRowHeights();

    return false;
}


void ScDocument::SetPendingRowHeights( SCTAB nTab, bool bSet )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB> (maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->SetPendingRowHeights( bSet );
}


void ScDocument::SetLayoutRTL( SCTAB nTab, bool bRTL )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB> (maTabs.size()) && maTabs[nTab] )
    {
        if ( bImportingXML )
        {
            // #i57869# only set the LoadingRTL flag, the real setting (including mirroring)
            // is applied in SetImportingXML(false). This is so the shapes can be loaded in
            // normal LTR mode.

            maTabs[nTab]->SetLoadingRTL( bRTL );
            return;
        }

        maTabs[nTab]->SetLayoutRTL( bRTL );     // only sets the flag
        maTabs[nTab]->SetDrawPageSize();

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


bool ScDocument::IsLayoutRTL( SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB> (maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->IsLayoutRTL();

    return false;
}


bool ScDocument::IsNegativePage( SCTAB nTab ) const
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


bool ScDocument::GetCellArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB> (maTabs.size()))
        if (maTabs[nTab])
            return maTabs[nTab]->GetCellArea( rEndCol, rEndRow );

    rEndCol = 0;
    rEndRow = 0;
    return false;
}


bool ScDocument::GetTableArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB> (maTabs.size()))
        if (maTabs[nTab])
            return maTabs[nTab]->GetTableArea( rEndCol, rEndRow );

    rEndCol = 0;
    rEndRow = 0;
    return false;
}

bool ScDocument::ShrinkToDataArea(SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB> (maTabs.size()) || !maTabs[nTab])
        return false;

    SCCOL nCol1, nCol2;
    SCROW nRow1, nRow2;
    maTabs[nTab]->GetFirstDataPos(nCol1, nRow1);
    maTabs[nTab]->GetLastDataPos(nCol2, nRow2);

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
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB> (maTabs.size()) || !maTabs[nTab])
    {
        o_bShrunk = false;
        return false;
    }
    return maTabs[nTab]->ShrinkToUsedDataArea( o_bShrunk, rStartCol, rStartRow, rEndCol, rEndRow, bColumnsOnly);
}

// connected area

void ScDocument::GetDataArea( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow,
                              SCCOL& rEndCol, SCROW& rEndRow, bool bIncludeOld, bool bOnlyDown ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB> (maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->GetDataArea( rStartCol, rStartRow, rEndCol, rEndRow, bIncludeOld, bOnlyDown );
}


void ScDocument::LimitChartArea( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow,
                                    SCCOL& rEndCol, SCROW& rEndRow )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB> (maTabs.size()))
        if (maTabs[nTab])
            maTabs[nTab]->LimitChartArea( rStartCol, rStartRow, rEndCol, rEndRow );
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
                if ( nTab < static_cast<SCTAB> (maTabs.size()) && maTabs[nTab])
                    maTabs[nTab]->LimitChartArea(nStartCol, nStartRow, nEndCol, nEndRow);
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


static void lcl_GetFirstTabRange( SCTAB& rTabRangeStart, SCTAB& rTabRangeEnd, const ScMarkData* pTabMark, SCTAB aMaxTab )
{
    // without ScMarkData, leave start/end unchanged
    if ( pTabMark )
    {
        for (SCTAB nTab=0; nTab< aMaxTab; ++nTab)
            if (pTabMark->GetTableSelect(nTab))
            {
                // find first range of consecutive selected sheets
                rTabRangeStart = pTabMark->GetFirstSelected();
                while ( nTab+1 < aMaxTab && pTabMark->GetTableSelect(nTab+1) )
                    ++nTab;
                rTabRangeEnd = nTab;
                return;
            }
    }
}

static bool lcl_GetNextTabRange( SCTAB& rTabRangeStart, SCTAB& rTabRangeEnd, const ScMarkData* pTabMark, SCTAB aMaxTab )
{
    if ( pTabMark )
    {
        // find next range of consecutive selected sheets after rTabRangeEnd
        for (SCTAB nTab=rTabRangeEnd+1; nTab< aMaxTab; ++nTab)
            if (pTabMark->GetTableSelect(nTab))
            {
                rTabRangeStart = nTab;
                while ( nTab+1 < aMaxTab && pTabMark->GetTableSelect(nTab+1) )
                    ++nTab;
                rTabRangeEnd = nTab;
                return true;
            }
    }
    return false;
}


bool ScDocument::CanInsertRow( const ScRange& rRange ) const
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

    bool bTest = true;
    for (SCTAB i=nStartTab; i<=nEndTab && bTest && i < static_cast<SCTAB>(maTabs.size()); i++)
        if (maTabs[i])
            bTest &= maTabs[i]->TestInsertRow( nStartCol, nEndCol, nStartRow, nSize );

    return bTest;
}

namespace {

struct StartNeededListenersHandler : std::unary_function<ScTable*, void>
{
    void operator() (ScTable* p)
    {
        if (p)
            p->StartNeededListeners();
    }
};

struct SetRelNameDirtyHandler : std::unary_function<ScTable*, void>
{
    void operator() (ScTable* p)
    {
        if (p)
            p->SetRelNameDirty();
    }
};

struct BroadcastRecalcOnRefMoveHandler : std::unary_function<ScTable*, void>
{
    void operator() (ScTable* p)
    {
        if (p)
            p->BroadcastRecalcOnRefMove();
    }
};

}

bool ScDocument::InsertRow( SCCOL nStartCol, SCTAB nStartTab,
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
        nEndTab = static_cast<SCTAB>(maTabs.size()) -1;
    }

    bool bTest = true;
    bool bRet = false;
    bool bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( false );   // avoid mulitple calculations
    for ( i = nStartTab; i <= nEndTab && bTest && i < static_cast<SCTAB>(maTabs.size()); i++)
        if (maTabs[i] && (!pTabMark || pTabMark->GetTableSelect(i)))
            bTest &= maTabs[i]->TestInsertRow(nStartCol, nEndCol, nStartRow, nSize);
    if (bTest)
    {
        // UpdateBroadcastAreas have to be called before UpdateReference, so that entries
        // aren't shifted that would be rebuild at UpdateReference

        // handle chunks of consecutive selected sheets together
        SCTAB nTabRangeStart = nStartTab;
        SCTAB nTabRangeEnd = nEndTab;
        lcl_GetFirstTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) );
        do
        {
            UpdateBroadcastAreas( URM_INSDEL, ScRange(
                ScAddress( nStartCol, nStartRow, nTabRangeStart ),
                ScAddress( nEndCol, MAXROW, nTabRangeEnd )), 0, static_cast<SCsROW>(nSize), 0 );
        }
        while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) ) );

        lcl_GetFirstTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) );

        sc::RefUpdateContext aCxt(*this);
        aCxt.meMode = URM_INSDEL;
        aCxt.maRange = ScRange(nStartCol, nStartRow, nTabRangeStart, nEndCol, MAXROW, nTabRangeEnd);
        aCxt.mnRowDelta = nSize;
        do
        {
            UpdateReference(aCxt, pRefUndoDoc, false);        // without drawing objects
        }
        while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) ) );

        for (i=nStartTab; i<=nEndTab && i < static_cast<SCTAB>(maTabs.size()); i++)
            if (maTabs[i] && (!pTabMark || pTabMark->GetTableSelect(i)))
                maTabs[i]->InsertRow( nStartCol, nEndCol, nStartRow, nSize );

        //  UpdateRef for drawing layer must be after inserting,
        //  when the new row heights are known.
        for (i=nStartTab; i<=nEndTab && i < static_cast<SCTAB>(maTabs.size()); i++)
            if (maTabs[i] && (!pTabMark || pTabMark->GetTableSelect(i)))
                maTabs[i]->UpdateDrawRef( URM_INSDEL,
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
            TableContainer::iterator it = maTabs.begin();
            for (; it != maTabs.end(); ++it)
                if (*it)
                    (*it)->StartNeededListeners();
            // at least all cells using range names pointing relative
            // to the moved range must recalculate
            it = maTabs.begin();
            for (; it != maTabs.end(); ++it)
                if (*it)
                    (*it)->SetRelNameDirty();

            std::for_each(maTabs.begin(), maTabs.end(), BroadcastRecalcOnRefMoveHandler());
        }
        bRet = true;
    }
    SetAutoCalc( bOldAutoCalc );
    if ( bRet )
        pChartListenerCollection->UpdateDirtyCharts();
    return bRet;
}


bool ScDocument::InsertRow( const ScRange& rRange, ScDocument* pRefUndoDoc )
{
    return InsertRow( rRange.aStart.Col(), rRange.aStart.Tab(),
                      rRange.aEnd.Col(),   rRange.aEnd.Tab(),
                      rRange.aStart.Row(), static_cast<SCSIZE>(rRange.aEnd.Row()-rRange.aStart.Row()+1),
                      pRefUndoDoc );
}


void ScDocument::DeleteRow( SCCOL nStartCol, SCTAB nStartTab,
                            SCCOL nEndCol,   SCTAB nEndTab,
                            SCROW nStartRow, SCSIZE nSize,
                            ScDocument* pRefUndoDoc, bool* pUndoOutline,
                            const ScMarkData* pTabMark )
{
    SCTAB i;

    PutInOrder( nStartCol, nEndCol );
    PutInOrder( nStartTab, nEndTab );
    if ( pTabMark )
    {
        nStartTab = 0;
        nEndTab = static_cast<SCTAB>(maTabs.size())-1;
    }

    bool bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( false );   // avoid multiple calculations

    // handle chunks of consecutive selected sheets together
    SCTAB nTabRangeStart = nStartTab;
    SCTAB nTabRangeEnd = nEndTab;
    lcl_GetFirstTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) );
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
    while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) ) );

    if ( ValidRow(nStartRow+nSize) )
    {
        lcl_GetFirstTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) );
        sc::RefUpdateContext aCxt(*this);
        aCxt.meMode = URM_INSDEL;
        aCxt.maRange = ScRange(nStartCol, nStartRow+nSize, nTabRangeStart, nEndCol, MAXROW, nTabRangeEnd);
        aCxt.mnRowDelta = -(static_cast<SCROW>(nSize));
        do
        {
            UpdateReference(aCxt, pRefUndoDoc, true, false);
        }
        while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) ) );
    }

    if (pUndoOutline)
        *pUndoOutline = false;

    for ( i = nStartTab; i <= nEndTab && i < static_cast<SCTAB>(maTabs.size()); i++)
        if (maTabs[i] && (!pTabMark || pTabMark->GetTableSelect(i)))
            maTabs[i]->DeleteRow( nStartCol, nEndCol, nStartRow, nSize, pUndoOutline );

    if ( ValidRow(nStartRow+nSize) )
    {   // Listeners have been removed in UpdateReference
        TableContainer::iterator it = maTabs.begin();
        for (; it != maTabs.end(); ++it)
            if (*it)
                (*it)->StartNeededListeners();
        // at least all cells using range names pointing relative
        // to the moved range must recalculate
        it = maTabs.begin();
        for (; it != maTabs.end(); ++it)
            if (*it)
                (*it)->SetRelNameDirty();

        std::for_each(maTabs.begin(), maTabs.end(), BroadcastRecalcOnRefMoveHandler());
    }

    SetAutoCalc( bOldAutoCalc );
    pChartListenerCollection->UpdateDirtyCharts();
}


void ScDocument::DeleteRow( const ScRange& rRange, ScDocument* pRefUndoDoc, bool* pUndoOutline )
{
    DeleteRow( rRange.aStart.Col(), rRange.aStart.Tab(),
               rRange.aEnd.Col(),   rRange.aEnd.Tab(),
               rRange.aStart.Row(), static_cast<SCSIZE>(rRange.aEnd.Row()-rRange.aStart.Row()+1),
               pRefUndoDoc, pUndoOutline );
}


bool ScDocument::CanInsertCol( const ScRange& rRange ) const
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

    bool bTest = true;
    for (SCTAB i=nStartTab; i<=nEndTab && bTest && i < static_cast<SCTAB>(maTabs.size()); i++)
        if (maTabs[i])
            bTest &= maTabs[i]->TestInsertCol( nStartRow, nEndRow, nSize );

    return bTest;
}

bool ScDocument::InsertCol( SCROW nStartRow, SCTAB nStartTab,
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
        nEndTab = static_cast<SCTAB>(maTabs.size())-1;
    }

    bool bTest = true;
    bool bRet = false;
    bool bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( false );   // avoid multiple calculations
    for ( i = nStartTab; i <= nEndTab && bTest && i < static_cast<SCTAB>(maTabs.size()); i++)
        if (maTabs[i] && (!pTabMark || pTabMark->GetTableSelect(i)))
            bTest &= maTabs[i]->TestInsertCol( nStartRow, nEndRow, nSize );
    if (bTest)
    {
        // handle chunks of consecutive selected sheets together
        SCTAB nTabRangeStart = nStartTab;
        SCTAB nTabRangeEnd = nEndTab;
        lcl_GetFirstTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) );
        do
        {
            UpdateBroadcastAreas( URM_INSDEL, ScRange(
                ScAddress( nStartCol, nStartRow, nTabRangeStart ),
                ScAddress( MAXCOL, nEndRow, nTabRangeEnd )), static_cast<SCsCOL>(nSize), 0, 0 );
        }
        while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) ) );

        lcl_GetFirstTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) );

        sc::RefUpdateContext aCxt(*this);
        aCxt.meMode = URM_INSDEL;
        aCxt.maRange = ScRange(nStartCol, nStartRow, nTabRangeStart, MAXCOL, nEndRow, nTabRangeEnd);
        aCxt.mnColDelta = nSize;
        do
        {
            UpdateReference(aCxt, pRefUndoDoc, true, false);
        }
        while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) ) );

        for (i=nStartTab; i<=nEndTab && i < static_cast<SCTAB>(maTabs.size()); i++)
            if (maTabs[i] && (!pTabMark || pTabMark->GetTableSelect(i)))
                maTabs[i]->InsertCol( nStartCol, nStartRow, nEndRow, nSize );

        if ( pChangeTrack && pChangeTrack->IsInDeleteUndo() )
        {   // durch Restaurierung von Referenzen auf geloeschte Bereiche ist
            // ein neues Listening faellig, bisherige Listener wurden in
            // FormulaCell UpdateReference abgehaengt
            StartAllListeners();
        }
        else
        {
            // Listeners have been removed in UpdateReference
            std::for_each(maTabs.begin(), maTabs.end(), StartNeededListenersHandler());
            // at least all cells using range names pointing relative to the
            // moved range must recalculate.
            std::for_each(maTabs.begin(), maTabs.end(), SetRelNameDirtyHandler());
            // Cells containing functions such as CELL, COLUMN or ROW may have
            // changed their values on relocation. Broadcast them.
            std::for_each(maTabs.begin(), maTabs.end(), BroadcastRecalcOnRefMoveHandler());
        }
        bRet = true;
    }
    SetAutoCalc( bOldAutoCalc );
    if ( bRet )
        pChartListenerCollection->UpdateDirtyCharts();
    return bRet;
}


bool ScDocument::InsertCol( const ScRange& rRange, ScDocument* pRefUndoDoc )
{
    return InsertCol( rRange.aStart.Row(), rRange.aStart.Tab(),
                      rRange.aEnd.Row(),   rRange.aEnd.Tab(),
                      rRange.aStart.Col(), static_cast<SCSIZE>(rRange.aEnd.Col()-rRange.aStart.Col()+1),
                      pRefUndoDoc );
}


void ScDocument::DeleteCol(SCROW nStartRow, SCTAB nStartTab, SCROW nEndRow, SCTAB nEndTab,
                                SCCOL nStartCol, SCSIZE nSize, ScDocument* pRefUndoDoc,
                                bool* pUndoOutline, const ScMarkData* pTabMark )
{
    SCTAB i;

    PutInOrder( nStartRow, nEndRow );
    PutInOrder( nStartTab, nEndTab );
    if ( pTabMark )
    {
        nStartTab = 0;
        nEndTab = static_cast<SCTAB>(maTabs.size())-1;
    }

    sc::AutoCalcSwitch aACSwitch(*this, false); // avoid multiple calculations

    // handle chunks of consecutive selected sheets together
    SCTAB nTabRangeStart = nStartTab;
    SCTAB nTabRangeEnd = nEndTab;
    lcl_GetFirstTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) );
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
    while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) ) );

    sc::RefUpdateContext aCxt(*this);
    if ( ValidCol(sal::static_int_cast<SCCOL>(nStartCol+nSize)) )
    {
        lcl_GetFirstTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) );
        aCxt.meMode = URM_INSDEL;
        aCxt.maRange = ScRange(sal::static_int_cast<SCCOL>(nStartCol+nSize), nStartRow, nTabRangeStart, MAXCOL, nEndRow, nTabRangeEnd);
        aCxt.mnColDelta = -(static_cast<SCCOL>(nSize));
        do
        {
            UpdateReference(aCxt, pRefUndoDoc, true, false);
        }
        while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) ) );
    }

    if (pUndoOutline)
        *pUndoOutline = false;

    for (i = nStartTab; i <= nEndTab && i < static_cast<SCTAB>(maTabs.size()); ++i)
    {
        if (maTabs[i] && (!pTabMark || pTabMark->GetTableSelect(i)))
            maTabs[i]->DeleteCol(aCxt.maRegroupCols, nStartCol, nStartRow, nEndRow, nSize, pUndoOutline);
    }

    if ( ValidCol(sal::static_int_cast<SCCOL>(nStartCol+nSize)) )
    {// Listeners have been removed in UpdateReference
        TableContainer::iterator it = maTabs.begin();
        for (; it != maTabs.end(); ++it)
            if (*it)
                (*it)->StartNeededListeners();
        // at least all cells using range names pointing relative
        // to the moved range must recalculate
        it = maTabs.begin();
        for (; it != maTabs.end(); ++it)
            if (*it)
                (*it)->SetRelNameDirty();

        std::for_each(maTabs.begin(), maTabs.end(), BroadcastRecalcOnRefMoveHandler());
    }

    pChartListenerCollection->UpdateDirtyCharts();
}


void ScDocument::DeleteCol( const ScRange& rRange, ScDocument* pRefUndoDoc, bool* pUndoOutline )
{
    DeleteCol( rRange.aStart.Row(), rRange.aStart.Tab(),
               rRange.aEnd.Row(),   rRange.aEnd.Tab(),
               rRange.aStart.Col(), static_cast<SCSIZE>(rRange.aEnd.Col()-rRange.aStart.Col()+1),
               pRefUndoDoc, pUndoOutline );
}


//  fuer Area-Links: Zellen einuegen/loeschen, wenn sich der Bereich veraendert
//  (ohne Paint)


static void lcl_GetInsDelRanges( const ScRange& rOld, const ScRange& rNew,
                            ScRange& rColRange, bool& rInsCol, bool& rDelCol,
                            ScRange& rRowRange, bool& rInsRow, bool& rDelRow )
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
    bool bGrowY = ( nNewEndY > nOldEndY );
    SCROW nColEndY = bGrowY ? nOldEndY : nNewEndY;
    SCCOL nRowEndX = bGrowY ? nNewEndX : nOldEndX;

    //  Spalten

    if ( nNewEndX > nOldEndX )          // Spalten einfuegen
    {
        rColRange = ScRange( nOldEndX+1, nStartY, nTab, nNewEndX, nColEndY, nTab );
        rInsCol = true;
    }
    else if ( nNewEndX < nOldEndX )     // Spalten loeschen
    {
        rColRange = ScRange( nNewEndX+1, nStartY, nTab, nOldEndX, nColEndY, nTab );
        rDelCol = true;
    }

    //  Zeilen

    if ( nNewEndY > nOldEndY )          // Zeilen einfuegen
    {
        rRowRange = ScRange( nStartX, nOldEndY+1, nTab, nRowEndX, nNewEndY, nTab );
        rInsRow = true;
    }
    else if ( nNewEndY < nOldEndY )     // Zeilen loeschen
    {
        rRowRange = ScRange( nStartX, nNewEndY+1, nTab, nRowEndX, nOldEndY, nTab );
        rDelRow = true;
    }
}


bool ScDocument::HasPartOfMerged( const ScRange& rRange )
{
    bool bPart = false;
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

size_t ScDocument::GetFormulaHash( const ScAddress& rPos ) const
{
    SCTAB nTab = rPos.Tab();
    if (!ValidTab(nTab) || static_cast<size_t>(nTab) >= maTabs.size() || !maTabs[nTab])
        return 0;

    return maTabs[nTab]->GetFormulaHash(rPos.Col(), rPos.Row());
}

ScFormulaVectorState ScDocument::GetFormulaVectorState( const ScAddress& rPos ) const
{
    SCTAB nTab = rPos.Tab();
    if (!ValidTab(nTab) || static_cast<size_t>(nTab) >= maTabs.size() || !maTabs[nTab])
        return FormulaVectorUnknown;

    return maTabs[nTab]->GetFormulaVectorState(rPos.Col(), rPos.Row());
}

formula::FormulaTokenRef ScDocument::ResolveStaticReference( const ScAddress& rPos )
{
    SCTAB nTab = rPos.Tab();
    if (!TableExists(nTab))
        return formula::FormulaTokenRef();

    return maTabs[nTab]->ResolveStaticReference(rPos.Col(), rPos.Row());
}

formula::FormulaTokenRef ScDocument::ResolveStaticReference( const ScRange& rRange )
{
    SCTAB nTab = rRange.aStart.Tab();
    if (nTab != rRange.aEnd.Tab() || !TableExists(nTab))
        return formula::FormulaTokenRef();

    return maTabs[nTab]->ResolveStaticReference(
        rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(), rRange.aEnd.Row());
}

const double* ScDocument::FetchDoubleArray(
    sc::FormulaGroupContext& rCxt, const ScAddress& rPos, SCROW nLength )
{
    SCTAB nTab = rPos.Tab();
    if (!TableExists(nTab))
        return NULL;

    return maTabs[nTab]->FetchDoubleArray(rCxt, rPos.Col(), rPos.Row(), rPos.Row()+nLength-1);
}

bool ScDocument::CanFitBlock( const ScRange& rOld, const ScRange& rNew )
{
    if ( rOld == rNew )
        return true;

    bool bOk = true;
    bool bInsCol,bDelCol,bInsRow,bDelRow;
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


void ScDocument::FitBlock( const ScRange& rOld, const ScRange& rNew, bool bClear )
{
    if (bClear)
        DeleteAreaTab( rOld, IDF_ALL );

    bool bInsCol,bDelCol,bInsRow,bDelRow;
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
        aGrowSource.aEnd.SetCol(std::min( rOld.aEnd.Col(), rNew.aEnd.Col() ));
        aGrowSource.aEnd.SetRow(std::min( rOld.aEnd.Row(), rNew.aEnd.Row() ));
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
    bool bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( false );   // avoid multiple calculations
    for (SCTAB i = 0; i < static_cast<SCTAB>(maTabs.size()); i++)
        if (maTabs[i])
            if ( rMark.GetTableSelect(i) || bIsUndo )
                maTabs[i]->DeleteArea(nCol1, nRow1, nCol2, nRow2, nDelFlag);
    SetAutoCalc( bOldAutoCalc );
}


void ScDocument::DeleteAreaTab(SCCOL nCol1, SCROW nRow1,
                                SCCOL nCol2, SCROW nRow2,
                                SCTAB nTab, sal_uInt16 nDelFlag)
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
    {
        bool bOldAutoCalc = GetAutoCalc();
        SetAutoCalc( false );   // avoid multiple calculations
        maTabs[nTab]->DeleteArea(nCol1, nRow1, nCol2, nRow2, nDelFlag);
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
                                bool bColInfo, bool bRowInfo )
{
    if (bIsUndo)
    {
        Clear();

        xPoolHelper = pSrcDoc->xPoolHelper;


        OUString aString;
        for (SCTAB nTab = 0; nTab <= rTabSelection.GetLastSelected(); nTab++)
            if ( rTabSelection.GetTableSelect( nTab ) )
            {
                ScTable* pTable = new ScTable(this, nTab, aString, bColInfo, bRowInfo);
                if (nTab < static_cast<SCTAB>(maTabs.size()))
                    maTabs[nTab] = pTable;
                else
                    maTabs.push_back(pTable);
            }
            else
            {
                if (nTab < static_cast<SCTAB>(maTabs.size()))
                    maTabs[nTab]=NULL;
                else
                    maTabs.push_back(NULL);
            }
    }
    else
    {
        OSL_FAIL("InitUndo");
    }
}


void ScDocument::InitUndo( ScDocument* pSrcDoc, SCTAB nTab1, SCTAB nTab2,
                                bool bColInfo, bool bRowInfo )
{
    if (bIsUndo)
    {
        Clear();

        xPoolHelper = pSrcDoc->xPoolHelper;
        if (pSrcDoc->pShell->GetMedium())
            maFileURL = pSrcDoc->pShell->GetMedium()->GetURLObject().GetMainURL(INetURLObject::DECODE_TO_IURI);

        OUString aString;
        if ( nTab2 >= static_cast<SCTAB>(maTabs.size()))
            maTabs.resize(nTab2 + 1, NULL);
        for (SCTAB nTab = nTab1; nTab <= nTab2; nTab++)
        {
            ScTable* pTable = new ScTable(this, nTab, aString, bColInfo, bRowInfo);
            maTabs[nTab] = pTable;
        }
    }
    else
    {
        OSL_FAIL("InitUndo");
    }
}


void ScDocument::AddUndoTab( SCTAB nTab1, SCTAB nTab2, bool bColInfo, bool bRowInfo )
{
    if (bIsUndo)
    {
        OUString aString;
        if (nTab2 >= static_cast<SCTAB>(maTabs.size()))
        {
            maTabs.resize(nTab2+1,NULL);
        }
        for (SCTAB nTab = nTab1; nTab <= nTab2; nTab++)
            if (!maTabs[nTab])
            {
                maTabs[nTab] = new ScTable(this, nTab, aString, bColInfo, bRowInfo);
            }

    }
    else
    {
        OSL_FAIL("InitUndo");
    }
}


void ScDocument::SetCutMode( bool bVal )
{
    if (bIsClip)
        GetClipParam().mbCutMode = bVal;
    else
    {
        OSL_FAIL("SetCutMode without bIsClip");
    }
}


bool ScDocument::IsCutMode()
{
    if (bIsClip)
        return GetClipParam().mbCutMode;
    else
    {
        OSL_FAIL("IsCutMode without bIsClip");
        return false;
    }
}


void ScDocument::CopyToDocument(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                            SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                            sal_uInt16 nFlags, bool bOnlyMarked, ScDocument* pDestDoc,
                            const ScMarkData* pMarks, bool bColRowFlags )
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    PutInOrder( nTab1, nTab2 );
    if( pDestDoc->aDocName.isEmpty() )
        pDestDoc->aDocName = aDocName;
    if (ValidTab(nTab1) && ValidTab(nTab2))
    {
        sc::CopyToDocContext aCxt(*pDestDoc);
        bool bOldAutoCalc = pDestDoc->GetAutoCalc();
        pDestDoc->SetAutoCalc( false );     // avoid multiple calculations
        SCTAB nMinSizeBothTabs = static_cast<SCTAB>(std::min(maTabs.size(), pDestDoc->maTabs.size()));
        for (SCTAB i = nTab1; i <= nTab2 && i < nMinSizeBothTabs; i++)
        {
            if (maTabs[i] && pDestDoc->maTabs[i])
                maTabs[i]->CopyToTable(aCxt, nCol1, nRow1, nCol2, nRow2, nFlags,
                                      bOnlyMarked, pDestDoc->maTabs[i], pMarks,
                                      false, bColRowFlags );
        }
        pDestDoc->SetAutoCalc( bOldAutoCalc );
    }
}


void ScDocument::UndoToDocument(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                            SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                            sal_uInt16 nFlags, bool bOnlyMarked, ScDocument* pDestDoc,
                            const ScMarkData* pMarks)
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    PutInOrder( nTab1, nTab2 );
    if (ValidTab(nTab1) && ValidTab(nTab2))
    {
        bool bOldAutoCalc = pDestDoc->GetAutoCalc();
        pDestDoc->SetAutoCalc( false );     // avoid multiple calculations
        if (nTab1 > 0)
            CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTab1-1, IDF_FORMULA, false, pDestDoc, pMarks );

        sc::CopyToDocContext aCxt(*pDestDoc);
        OSL_ASSERT( nTab2 < static_cast<SCTAB>(maTabs.size()) && nTab2 < static_cast<SCTAB>(pDestDoc->maTabs.size()));
        for (SCTAB i = nTab1; i <= nTab2; i++)
        {
            if (maTabs[i] && pDestDoc->maTabs[i])
                maTabs[i]->UndoToTable(aCxt, nCol1, nRow1, nCol2, nRow2, nFlags,
                                    bOnlyMarked, pDestDoc->maTabs[i], pMarks);
        }

        if (nTab2 < MAXTAB)
            CopyToDocument( 0,0,nTab2+1, MAXCOL,MAXROW,MAXTAB, IDF_FORMULA, false, pDestDoc, pMarks );
        pDestDoc->SetAutoCalc( bOldAutoCalc );
    }
}


void ScDocument::CopyToDocument(const ScRange& rRange,
                            sal_uInt16 nFlags, bool bOnlyMarked, ScDocument* pDestDoc,
                            const ScMarkData* pMarks, bool bColRowFlags)
{
    ScRange aNewRange = rRange;
    aNewRange.Justify();

    if( pDestDoc->aDocName.isEmpty() )
        pDestDoc->aDocName = aDocName;
    bool bOldAutoCalc = pDestDoc->GetAutoCalc();
    pDestDoc->SetAutoCalc( false );     // avoid multiple calculations
    sc::CopyToDocContext aCxt(*pDestDoc);
    SCTAB nMinSizeBothTabs = static_cast<SCTAB>(std::min(maTabs.size(), pDestDoc->maTabs.size()));
    for (SCTAB i = aNewRange.aStart.Tab(); i <= aNewRange.aEnd.Tab() && i < nMinSizeBothTabs; i++)
    {
        if (!TableExists(i) || !pDestDoc->TableExists(i))
            continue;

        maTabs[i]->CopyToTable(aCxt, aNewRange.aStart.Col(), aNewRange.aStart.Row(),
                               aNewRange.aEnd.Col(), aNewRange.aEnd.Row(),
                               nFlags, bOnlyMarked, pDestDoc->maTabs[i],
                               pMarks, false, bColRowFlags);
    }
    pDestDoc->SetAutoCalc( bOldAutoCalc );
}


void ScDocument::UndoToDocument(const ScRange& rRange,
                            sal_uInt16 nFlags, bool bOnlyMarked, ScDocument* pDestDoc,
                            const ScMarkData* pMarks)
{
    sc::AutoCalcSwitch aAutoCalcSwitch(*this, false);

    ScRange aNewRange = rRange;
    aNewRange.Justify();
    SCTAB nTab1 = aNewRange.aStart.Tab();
    SCTAB nTab2 = aNewRange.aEnd.Tab();

    sc::CopyToDocContext aCxt(*pDestDoc);
    if (nTab1 > 0)
        CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTab1-1, IDF_FORMULA, false, pDestDoc, pMarks );

    SCTAB nMinSizeBothTabs = static_cast<SCTAB>(std::min(maTabs.size(), pDestDoc->maTabs.size()));
    for (SCTAB i = nTab1; i <= nTab2 && i < nMinSizeBothTabs; i++)
    {
        if (maTabs[i] && pDestDoc->maTabs[i])
            maTabs[i]->UndoToTable(aCxt, aNewRange.aStart.Col(), aNewRange.aStart.Row(),
                                    aNewRange.aEnd.Col(), aNewRange.aEnd.Row(),
                                    nFlags, bOnlyMarked, pDestDoc->maTabs[i], pMarks);
    }

    if (nTab2 < static_cast<SCTAB>(maTabs.size()))
        CopyToDocument( 0,0,nTab2+1, MAXCOL,MAXROW,maTabs.size(), IDF_FORMULA, false, pDestDoc, pMarks );
}

// bUseRangeForVBA added for VBA api support to allow content of a specified
// range to be copied ( e.g. don't use marked data but the just the range
// specified by rClipParam
void ScDocument::CopyToClip(const ScClipParam& rClipParam,
                            ScDocument* pClipDoc, const ScMarkData* pMarks,
                            bool bAllTabs, bool bKeepScenarioFlags, bool bIncludeObjects, bool bCloneNoteCaptions, bool bUseRangeForVBA )
{
    OSL_ENSURE( !bUseRangeForVBA && ( bAllTabs ||  pMarks ), "CopyToClip: ScMarkData fails" );

    if (bIsClip)
        return;

    if (!pClipDoc)
    {
        OSL_TRACE("CopyToClip: no ClipDoc");
        pClipDoc = SC_MOD()->GetClipDoc();
    }

    if (pShell->GetMedium())
    {
        pClipDoc->maFileURL = pShell->GetMedium()->GetURLObject().GetMainURL(INetURLObject::DECODE_TO_IURI);
        // for unsaved files use the title name and adjust during save of file
        if (pClipDoc->maFileURL.isEmpty())
            pClipDoc->maFileURL = pShell->GetName();
    }
    else
    {
        pClipDoc->maFileURL = pShell->GetName();
    }

    //init maTabNames
    for (TableContainer::iterator itr = maTabs.begin(); itr != maTabs.end(); ++itr)
    {
        if( *itr )
        {
            OUString aTabName;
            (*itr)->GetName(aTabName);
            pClipDoc->maTabNames.push_back(aTabName);
        }
        else
            pClipDoc->maTabNames.push_back(OUString());
    }

    pClipDoc->aDocName = aDocName;
    pClipDoc->SetClipParam(rClipParam);
    ScRange aClipRange = rClipParam.getWholeRange();
    SCTAB nTab = aClipRange.aStart.Tab();
    SCTAB i = 0;
    SCTAB nEndTab =  static_cast<SCTAB>(maTabs.size());

    if ( bUseRangeForVBA )
    {
        pClipDoc->ResetClip( this, nTab );
        i = nTab;
        nEndTab = nTab + 1;
    }
    else
        pClipDoc->ResetClip(this, pMarks);

    sc::CopyToClipContext aCxt(*pClipDoc, bKeepScenarioFlags, bCloneNoteCaptions);
    CopyRangeNamesToClip(pClipDoc, aClipRange, pMarks, bAllTabs);

    for ( ; i < nEndTab; ++i)
    {
        if (!maTabs[i] || i >= static_cast<SCTAB>(pClipDoc->maTabs.size()) || !pClipDoc->maTabs[i])
            continue;

        if ( !bUseRangeForVBA && ( pMarks && !pMarks->GetTableSelect(i) ) )
            continue;

        maTabs[i]->CopyToClip(aCxt, rClipParam.maRanges, pClipDoc->maTabs[i]);

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

void ScDocument::CopyStaticToDocument(const ScRange& rSrcRange, SCTAB nDestTab, ScDocument* pDestDoc)
{
    if (!pDestDoc)
        return;

    ScTable* pSrcTab = rSrcRange.aStart.Tab() < static_cast<SCTAB>(maTabs.size()) ? maTabs[rSrcRange.aStart.Tab()] : NULL;
    ScTable* pDestTab = nDestTab < static_cast<SCTAB>(pDestDoc->maTabs.size()) ? pDestDoc->maTabs[nDestTab] : NULL;

    if (!pSrcTab || !pDestTab)
        return;

    pSrcTab->CopyStaticToDocument(
        rSrcRange.aStart.Col(), rSrcRange.aStart.Row(), rSrcRange.aEnd.Col(), rSrcRange.aEnd.Row(), pDestTab);
}

void ScDocument::CopyCellToDocument( const ScAddress& rSrcPos, const ScAddress& rDestPos, ScDocument& rDestDoc )
{
    if (!TableExists(rSrcPos.Tab()) || !rDestDoc.TableExists(rDestPos.Tab()))
        return;

    ScTable& rSrcTab = *maTabs[rSrcPos.Tab()];
    ScTable& rDestTab = *rDestDoc.maTabs[rDestPos.Tab()];

    rSrcTab.CopyCellToDocument(rSrcPos.Col(), rSrcPos.Row(), rDestPos.Col(), rDestPos.Row(), rDestTab);
}

void ScDocument::CopyTabToClip(SCCOL nCol1, SCROW nRow1,
                                SCCOL nCol2, SCROW nRow2,
                                SCTAB nTab, ScDocument* pClipDoc)
{
    if (!bIsClip)
    {
        if (pShell->GetMedium())
        {
            pClipDoc->maFileURL = pShell->GetMedium()->GetURLObject().GetMainURL(INetURLObject::DECODE_TO_IURI);
            // for unsaved files use the title name and adjust during save of file
            if (pClipDoc->maFileURL.isEmpty())
                pClipDoc->maFileURL = pShell->GetName();
        }
        else
        {
            pClipDoc->maFileURL = pShell->GetName();
        }

        //init maTabNames
        for (TableContainer::iterator itr = maTabs.begin(); itr != maTabs.end(); ++itr)
        {
            if( *itr )
            {
                OUString aTabName;
                (*itr)->GetName(aTabName);
                pClipDoc->maTabNames.push_back(aTabName);
            }
            else
                pClipDoc->maTabNames.push_back(OUString());
        }

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

        sc::CopyToClipContext aCxt(*pClipDoc, false, true);
        if (nTab < static_cast<SCTAB>(maTabs.size()) && nTab < static_cast<SCTAB>(pClipDoc->maTabs.size()))
            if (maTabs[nTab] && pClipDoc->maTabs[nTab])
                maTabs[nTab]->CopyToClip(aCxt, nCol1, nRow1, nCol2, nRow2, pClipDoc->maTabs[nTab]);

        pClipDoc->GetClipParam().mbCutMode = false;
    }
}


void ScDocument::TransposeClip( ScDocument* pTransClip, sal_uInt16 nFlags, bool bAsLink )
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
            sal_uInt16 nIndex = itr->second->GetIndex();
            ScRangeData* pData = new ScRangeData(*itr->second);
            if (pTransClip->pRangeName->insert(pData))
                pData->SetIndex(nIndex);
        }
    }

    // The data

    ScRange aClipRange = GetClipParam().getWholeRange();
    if ( ValidRow(aClipRange.aEnd.Row()-aClipRange.aStart.Row()) )
    {
        for (SCTAB i=0; i< static_cast<SCTAB>(maTabs.size()); i++)
            if (maTabs[i])
            {
                OSL_ENSURE( pTransClip->maTabs[i], "TransposeClip: Table not there" );
                maTabs[i]->TransposeClip( aClipRange.aStart.Col(), aClipRange.aStart.Row(),
                                            aClipRange.aEnd.Col(), aClipRange.aEnd.Row(),
                                            pTransClip->maTabs[i], nFlags, bAsLink );

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
        sal_uInt16 nIndex = itr->second->GetIndex();
        bool bInUse = (rUsedNames.count(nIndex) > 0);
        if (!bInUse)
            continue;

        ScRangeData* pData = new ScRangeData(*itr->second);
        if (pClipRangeName->insert(pData))
            pData->SetIndex(nIndex);
    }
}

}

void ScDocument::CopyRangeNamesToClip(ScDocument* pClipDoc, const ScRange& rClipRange, const ScMarkData* pMarks, bool bAllTabs)
{
    if (!pRangeName || pRangeName->empty())
        return;

    std::set<sal_uInt16> aUsedNames;        // indexes of named ranges that are used in the copied cells
    SCTAB nMinSizeBothTabs = static_cast<SCTAB>(std::min(maTabs.size(), pClipDoc->maTabs.size()));
    for (SCTAB i = 0; i < nMinSizeBothTabs; ++i)
        if (maTabs[i] && pClipDoc->maTabs[i])
            if ( bAllTabs || !pMarks || pMarks->GetTableSelect(i) )
                maTabs[i]->FindRangeNamesInUse(
                    rClipRange.aStart.Col(), rClipRange.aStart.Row(),
                    rClipRange.aEnd.Col(), rClipRange.aEnd.Row(), aUsedNames);

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

SvtBroadcaster* ScDocument::GetBroadcaster( const ScAddress& rPos )
{
    ScTable* pTab = FetchTable(rPos.Tab());
    if (!pTab)
        return NULL;

    return pTab->GetBroadcaster(rPos.Col(), rPos.Row());
}

const SvtBroadcaster* ScDocument::GetBroadcaster( const ScAddress& rPos ) const
{
    const ScTable* pTab = FetchTable(rPos.Tab());
    if (!pTab)
        return NULL;

    return pTab->GetBroadcaster(rPos.Col(), rPos.Row());
}

void ScDocument::DeleteBroadcasters( sc::ColumnBlockPosition& rBlockPos, const ScAddress& rTopPos, SCROW nLength )
{
    ScTable* pTab = FetchTable(rTopPos.Tab());
    if (!pTab || nLength <= 0)
        return;

    pTab->DeleteBroadcasters(rBlockPos, rTopPos.Col(), rTopPos.Row(), rTopPos.Row()+nLength-1);
}

bool ScDocument::HasBroadcaster( SCTAB nTab, SCCOL nCol ) const
{
    const ScTable* pTab = FetchTable(nTab);
    if (!pTab)
        return false;

    return pTab->HasBroadcaster(nCol);
}

#if DEBUG_COLUMN_STORAGE
void ScDocument::DumpFormulaGroups( SCTAB nTab, SCCOL nCol ) const
{
    const ScTable* pTab = FetchTable(nTab);
    if (!pTab)
        return;

    pTab->DumpFormulaGroups(nCol);
}
#endif

bool ScDocument::TableExists( SCTAB nTab ) const
{
    return ValidTab(nTab) && static_cast<size_t>(nTab) < maTabs.size() && maTabs[nTab];
}

ScTable* ScDocument::FetchTable( SCTAB nTab )
{
    if (!TableExists(nTab))
        return NULL;

    return maTabs[nTab];
}

const ScTable* ScDocument::FetchTable( SCTAB nTab ) const
{
    if (!TableExists(nTab))
        return NULL;

    return maTabs[nTab];
}

void ScDocument::MergeNumberFormatter(ScDocument* pSrcDoc)
{
    SvNumberFormatter* pThisFormatter = xPoolHelper->GetFormTable();
    SvNumberFormatter* pOtherFormatter = pSrcDoc->xPoolHelper->GetFormTable();
    if (pOtherFormatter && pOtherFormatter != pThisFormatter)
    {
        SvNumberFormatterIndexTable* pExchangeList =
                 pThisFormatter->MergeFormatter(*(pOtherFormatter));
        if (!pExchangeList->empty())
            pFormatExchangeList = pExchangeList;
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

bool ScDocument::IsClipboardSource() const
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
        sc::StartListeningContext aCxt(*this);
        SCTAB nMax = static_cast<SCTAB>(maTabs.size());
        ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd && *itr < nMax; ++itr)
            if (maTabs[*itr])
                maTabs[*itr]->StartListeningInArea(aCxt, nCol1, nRow1, nCol2, nRow2);
    }
}


void ScDocument::BroadcastFromClip( SCCOL nCol1, SCROW nRow1,
                                    SCCOL nCol2, SCROW nRow2,
                                    const ScMarkData& rMark, sal_uInt16 nInsFlag )
{
    if (nInsFlag & IDF_CONTENTS)
    {
        ScBulkBroadcast aBulkBroadcast( GetBASM());
        SCTAB nMax = static_cast<SCTAB>(maTabs.size());
        ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd && *itr < nMax; ++itr)
            if (maTabs[*itr])
                maTabs[*itr]->BroadcastInArea( nCol1, nRow1, nCol2, nRow2 );
    }
}

bool ScDocument::InitColumnBlockPosition( sc::ColumnBlockPosition& rBlockPos, SCTAB nTab, SCCOL nCol )
{
    if (!TableExists(nTab))
        return false;

    return maTabs[nTab]->InitColumnBlockPosition(rBlockPos, nCol);
}

void ScDocument::CopyBlockFromClip(
    sc::CopyFromClipContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
    const ScMarkData& rMark, SCsCOL nDx, SCsROW nDy )
{
    TableContainer& rClipTabs = rCxt.getClipDoc()->maTabs;
    SCTAB nTabEnd = rCxt.getTabEnd();
    SCTAB nClipTab = 0;
    for (SCTAB i = rCxt.getTabStart(); i <= nTabEnd && i < static_cast<SCTAB>(maTabs.size()); i++)
    {
        if (maTabs[i] && rMark.GetTableSelect(i) )
        {
            while (!rClipTabs[nClipTab]) nClipTab = (nClipTab+1) % (static_cast<SCTAB>(rClipTabs.size()));

            maTabs[i]->CopyFromClip(
                rCxt, nCol1, nRow1, nCol2, nRow2, nDx, nDy, rClipTabs[nClipTab]);

            if (rCxt.getClipDoc()->pDrawLayer && (rCxt.getInsertFlag() & IDF_OBJECTS))
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

                    Rectangle aSourceRect = rCxt.getClipDoc()->GetMMRect(
                                    nCol1-nDx, nRow1-nDy, nCol2-nDx, nRow2-nDy, nClipTab );
                    Rectangle aDestRect = GetMMRect( nCol1, nRow1, nCol2, nRow2, i );
                    pDrawLayer->CopyFromClip(rCxt.getClipDoc()->pDrawLayer, nClipTab, aSourceRect,
                                                ScAddress( nCol1, nRow1, i ), aDestRect );
                }
            }

            nClipTab = (nClipTab+1) % (static_cast<SCTAB>(rClipTabs.size()));
        }
    }
    if (rCxt.getInsertFlag() & IDF_CONTENTS)
    {
        nClipTab = 0;
        for (SCTAB i = rCxt.getTabStart(); i <= nTabEnd && i < static_cast<SCTAB>(maTabs.size()); i++)
        {
            if (maTabs[i] && rMark.GetTableSelect(i) )
            {
                while (!rClipTabs[nClipTab]) nClipTab = (nClipTab+1) % (static_cast<SCTAB>(rClipTabs.size()));
                SCsTAB nDz = ((SCsTAB)i) - nClipTab;

                //  ranges of consecutive selected tables (in clipboard and dest. doc)
                //  must be handled in one UpdateReference call
                SCTAB nFollow = 0;
                while ( i + nFollow < nTabEnd
                        && rMark.GetTableSelect( i + nFollow + 1 )
                        && nClipTab + nFollow < MAXTAB
                        && rClipTabs[(nClipTab + nFollow + 1) % static_cast<SCTAB>(rClipTabs.size())] )
                    ++nFollow;

                sc::RefUpdateContext aRefCxt(*this);
                aRefCxt.maRange = ScRange(nCol1, nRow1, i, nCol2, nRow2, i+nFollow);
                aRefCxt.mnColDelta = nDx;
                aRefCxt.mnRowDelta = nDy;
                aRefCxt.mnTabDelta = nDz;
                if (rCxt.getClipDoc()->GetClipParam().mbCutMode)
                {
                    bool bOldInserting = IsInsertingFromOtherDoc();
                    SetInsertingFromOtherDoc( true);
                    aRefCxt.meMode = URM_MOVE;
                    UpdateReference(aRefCxt, rCxt.getUndoDoc(), false);
                    SetInsertingFromOtherDoc( bOldInserting);
                }
                else
                {
                    aRefCxt.meMode = URM_COPY;
                    UpdateReference(aRefCxt, rCxt.getUndoDoc(), false);
                }

                nClipTab = (nClipTab+nFollow+1) % (static_cast<SCTAB>(rClipTabs.size()));
                i = sal::static_int_cast<SCTAB>( i + nFollow );
            }
        }
    }
}


void ScDocument::CopyNonFilteredFromClip(
    sc::CopyFromClipContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
    const ScMarkData& rMark, SCsCOL nDx, SCROW & rClipStartRow )
{
    //  call CopyBlockFromClip for ranges of consecutive non-filtered rows
    //  nCol1/nRow1 etc. is in target doc

    //  filtered state is taken from first used table in clipboard (as in GetClipArea)
    SCTAB nFlagTab = 0;
    TableContainer& rClipTabs = rCxt.getClipDoc()->maTabs;
    while ( nFlagTab < static_cast<SCTAB>(rClipTabs.size()) && !rClipTabs[nFlagTab] )
        ++nFlagTab;

    SCROW nSourceRow = rClipStartRow;
    SCROW nSourceEnd = 0;
    if (!rCxt.getClipDoc()->GetClipParam().maRanges.empty())
        nSourceEnd = rCxt.getClipDoc()->GetClipParam().maRanges.front()->aEnd.Row();
    SCROW nDestRow = nRow1;

    while ( nSourceRow <= nSourceEnd && nDestRow <= nRow2 )
    {
        // skip filtered rows
        nSourceRow = rCxt.getClipDoc()->FirstNonFilteredRow(nSourceRow, nSourceEnd, nFlagTab);

        if ( nSourceRow <= nSourceEnd )
        {
            // look for more non-filtered rows following
            SCROW nLastRow = nSourceRow;
            rCxt.getClipDoc()->RowFiltered(nSourceRow, nFlagTab, NULL, &nLastRow);
            SCROW nFollow = nLastRow - nSourceRow;

            if (nFollow > nSourceEnd - nSourceRow)
                nFollow = nSourceEnd - nSourceRow;
            if (nFollow > nRow2 - nDestRow)
                nFollow = nRow2 - nDestRow;

            SCsROW nNewDy = ((SCsROW)nDestRow) - nSourceRow;
            CopyBlockFromClip(
                rCxt, nCol1, nDestRow, nCol2, nDestRow + nFollow, rMark, nDx, nNewDy);

            nSourceRow += nFollow + 1;
            nDestRow += nFollow + 1;
        }
    }
    rClipStartRow = nSourceRow;
}


void ScDocument::CopyFromClip( const ScRange& rDestRange, const ScMarkData& rMark,
                                sal_uInt16 nInsFlag,
                                ScDocument* pRefUndoDoc, ScDocument* pClipDoc, bool bResetCut,
                                bool bAsLink, bool bIncludeFiltered, bool bSkipAttrForEmpty,
                                const ScRangeList * pDestRanges )
{
    if (bIsClip)
        return;

    if (!pClipDoc)
    {
        OSL_FAIL("CopyFromClip: no ClipDoc");
        pClipDoc = SC_MOD()->GetClipDoc();
    }

    if (!pClipDoc->bIsClip || !pClipDoc->GetTableCount())
        return;

    bool bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( false );   // avoid multiple recalculations

    NumFmtMergeHandler aNumFmtMergeHdl(this, pClipDoc);

    SCCOL nAllCol1 = rDestRange.aStart.Col();
    SCROW nAllRow1 = rDestRange.aStart.Row();
    SCCOL nAllCol2 = rDestRange.aEnd.Col();
    SCROW nAllRow2 = rDestRange.aEnd.Row();

    SCCOL nXw = 0;
    SCROW nYw = 0;
    ScRange aClipRange = pClipDoc->GetClipParam().getWholeRange();
    for (SCTAB nTab = 0; nTab < static_cast<SCTAB>(pClipDoc->maTabs.size()); nTab++)    // find largest merge overlap
        if (pClipDoc->maTabs[nTab])                   // all sheets of the clipboard content
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

    sc::CopyFromClipContext aCxt(*this, pRefUndoDoc, pClipDoc, nInsFlag, bAsLink, bSkipAttrForEmpty);
    std::pair<SCTAB,SCTAB> aTabRanges = getMarkedTableRange(maTabs, rMark);
    aCxt.setTabRange(aTabRanges.first, aTabRanges.second);

    ScRangeList aLocalRangeList;
    if (!pDestRanges)
    {
        aLocalRangeList.Append( rDestRange);
        pDestRanges = &aLocalRangeList;
    }

    bInsertingFromOtherDoc = true;  // kein Broadcast/Listener aufbauen bei Insert

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
                    CopyBlockFromClip(
                        aCxt, nC1, nR1, nC2, nR2, rMark, nDx, nDy);
                    nClipStartRow += nR2 - nR1 + 1;
                }
                else
                {
                    CopyNonFilteredFromClip(
                        aCxt, nC1, nR1, nC2, nR2, rMark, nDx, nClipStartRow);
                }
                nC1 = nC2 + 1;
                nC2 = std::min((SCCOL)(nC1 + nXw), nCol2);
            } while (nC1 <= nCol2);
            if (nClipStartRow > nClipEndRow)
                nClipStartRow = aClipRange.aStart.Row();
            nC1 = nCol1;
            nC2 = nC1 + nXw;
            if (nC2 > nCol2)
                nC2 = nCol2;
            nR1 = nR2 + 1;
            nR2 = std::min((SCROW)(nR1 + nYw), nRow2);
        } while (nR1 <= nRow2);
    }

    bInsertingFromOtherDoc = false;

    // Listener aufbauen nachdem alles inserted wurde
    StartListeningFromClip( nAllCol1, nAllRow1, nAllCol2, nAllRow2, rMark, nInsFlag );
    // nachdem alle Listener aufgebaut wurden, kann gebroadcastet werden
    BroadcastFromClip( nAllCol1, nAllRow1, nAllCol2, nAllRow2, rMark, nInsFlag );
    if (bResetCut)
        pClipDoc->GetClipParam().mbCutMode = false;
    SetAutoCalc( bOldAutoCalc );
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

    bool bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( false );   // avoid multiple recalculations

    NumFmtMergeHandler aNumFmtMergeHdl(this, pClipDoc);

    SCCOL nCol1 = rDestPos.Col();
    SCROW nRow1 = rDestPos.Row();
    ScClipParam& rClipParam = pClipDoc->GetClipParam();

    sc::CopyFromClipContext aCxt(*this, NULL, pClipDoc, nInsFlag, bAsLink, bSkipAttrForEmpty);
    std::pair<SCTAB,SCTAB> aTabRanges = getMarkedTableRange(maTabs, rMark);
    aCxt.setTabRange(aTabRanges.first, aTabRanges.second);

    ScRange aDestRange;
    rMark.GetMarkArea(aDestRange);
    SCROW nLastMarkedRow = aDestRange.aEnd.Row();

    bInsertingFromOtherDoc = true;  // kein Broadcast/Listener aufbauen bei Insert

    SCROW nBegRow = nRow1;
    sal_uInt16 nDelFlag = IDF_CONTENTS;
    const ScBitMaskCompressedArray<SCROW, sal_uInt8>& rFlags = GetRowFlagsArray(aCxt.getTabStart());

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

        CopyBlockFromClip(aCxt, nCol1, nBegRow, nCol2, nEndRow, rMark, nDx, nDy);
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

            CopyBlockFromClip(aCxt, nCol1, nBegRow, nCol2, nEndRow, rMark, nDx, nDy);
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

    bInsertingFromOtherDoc = false;

    ScRangeList aRanges;
    aRanges.Append(aDestRange);

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

void ScDocument::SetClipArea( const ScRange& rArea, bool bCut )
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
        OSL_FAIL("SetClipArea: No Clip");
    }
}


void ScDocument::GetClipArea(SCCOL& nClipX, SCROW& nClipY, bool bIncludeFiltered)
{
    if (!bIsClip)
    {
        OSL_FAIL("GetClipArea: No Clip");
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
        while ( nCountTab < static_cast<SCTAB>(maTabs.size()) && !maTabs[nCountTab] )
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
        OSL_FAIL("GetClipStart: No Clip");
    }
}


bool ScDocument::HasClipFilteredRows()
{
    //  count on first used table in clipboard
    SCTAB nCountTab = 0;
    while ( nCountTab < static_cast<SCTAB>(maTabs.size()) && !maTabs[nCountTab] )
        ++nCountTab;

    ScRangeList& rClipRanges = GetClipParam().maRanges;
    if ( rClipRanges.empty() )
        return false;

    for ( size_t i = 0, n = rClipRanges.size(); i < n; ++i )
    {
        ScRange* p = rClipRanges[ i ];
        bool bAnswer = maTabs[nCountTab]->HasFilteredRows(p->aStart.Row(), p->aEnd.Row());
        if (bAnswer)
            return true;
    }
    return false;
}


void ScDocument::MixDocument( const ScRange& rRange, sal_uInt16 nFunction, bool bSkipEmpty,
                                    ScDocument* pSrcDoc )
{
    SCTAB nTab1 = rRange.aStart.Tab();
    SCTAB nTab2 = rRange.aEnd.Tab();
    sc::MixDocContext aCxt(*this);
    SCTAB nMinSizeBothTabs = static_cast<SCTAB>(std::min(maTabs.size(), pSrcDoc->maTabs.size()));
    for (SCTAB i = nTab1; i <= nTab2 && i < nMinSizeBothTabs; i++)
    {
        ScTable* pTab = FetchTable(i);
        const ScTable* pSrcTab = pSrcDoc->FetchTable(i);
        if (!pTab || !pSrcTab)
            continue;

        pTab->MixData(
            aCxt, rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(), rRange.aEnd.Row(),
            nFunction, bSkipEmpty, pSrcTab);
    }
}


void ScDocument::FillTab( const ScRange& rSrcArea, const ScMarkData& rMark,
                                sal_uInt16 nFlags, sal_uInt16 nFunction,
                                bool bSkipEmpty, bool bAsLink )
{
    sal_uInt16 nDelFlags = nFlags;
    if (nDelFlags & IDF_CONTENTS)
        nDelFlags |= IDF_CONTENTS;          // immer alle Inhalte oder keine loeschen!

    SCTAB nSrcTab = rSrcArea.aStart.Tab();

    if (ValidTab(nSrcTab) && nSrcTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nSrcTab])
    {
        SCCOL nStartCol = rSrcArea.aStart.Col();
        SCROW nStartRow = rSrcArea.aStart.Row();
        SCCOL nEndCol = rSrcArea.aEnd.Col();
        SCROW nEndRow = rSrcArea.aEnd.Row();
        boost::scoped_ptr<ScDocument> pMixDoc;
        bool bDoMix = ( bSkipEmpty || nFunction ) && ( nFlags & IDF_CONTENTS );

        bool bOldAutoCalc = GetAutoCalc();
        SetAutoCalc( false );                   // avoid multiple calculations

        sc::CopyToDocContext aCxt(*this);
        sc::MixDocContext aMixDocCxt(*this);

        SCTAB nCount = static_cast<SCTAB>(maTabs.size());
        ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd && *itr < nCount; ++itr)
            if ( *itr!=nSrcTab && maTabs[*itr])
            {
                SCTAB i = *itr;
                if (bDoMix)
                {
                    if (!pMixDoc)
                    {
                        pMixDoc.reset(new ScDocument(SCDOCMODE_UNDO));
                        pMixDoc->InitUndo( this, i, i );
                    }
                    else
                        pMixDoc->AddUndoTab( i, i );

                    // context used for copying content to the temporary mix document.
                    sc::CopyToDocContext aMixCxt(*pMixDoc);
                    maTabs[i]->CopyToTable(aMixCxt, nStartCol,nStartRow, nEndCol,nEndRow,
                                            IDF_CONTENTS, false, pMixDoc->maTabs[i] );
                }
                maTabs[i]->DeleteArea( nStartCol,nStartRow, nEndCol,nEndRow, nDelFlags);
                maTabs[nSrcTab]->CopyToTable(aCxt, nStartCol,nStartRow, nEndCol,nEndRow,
                                                 nFlags, false, maTabs[i], NULL, bAsLink );

                if (bDoMix)
                    maTabs[i]->MixData(aMixDocCxt, nStartCol,nStartRow, nEndCol,nEndRow,
                                        nFunction, bSkipEmpty, pMixDoc->maTabs[i] );
            }

        SetAutoCalc( bOldAutoCalc );
    }
    else
    {
        OSL_FAIL("wrong table");
    }
}


void ScDocument::FillTabMarked( SCTAB nSrcTab, const ScMarkData& rMark,
                                sal_uInt16 nFlags, sal_uInt16 nFunction,
                                bool bSkipEmpty, bool bAsLink )
{
    sal_uInt16 nDelFlags = nFlags;
    if (nDelFlags & IDF_CONTENTS)
        nDelFlags |= IDF_CONTENTS;          // immer alle Inhalte oder keine loeschen!

    if (ValidTab(nSrcTab) && nSrcTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nSrcTab])
    {
        boost::scoped_ptr<ScDocument> pMixDoc;
        bool bDoMix = ( bSkipEmpty || nFunction ) && ( nFlags & IDF_CONTENTS );

        bool bOldAutoCalc = GetAutoCalc();
        SetAutoCalc( false );                   // avoid multiple calculations

        ScRange aArea;
        rMark.GetMultiMarkArea( aArea );
        SCCOL nStartCol = aArea.aStart.Col();
        SCROW nStartRow = aArea.aStart.Row();
        SCCOL nEndCol = aArea.aEnd.Col();
        SCROW nEndRow = aArea.aEnd.Row();

        sc::CopyToDocContext aCxt(*this);
        sc::MixDocContext aMixDocCxt(*this);
        SCTAB nCount = static_cast<SCTAB>(maTabs.size());
        ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd && *itr < nCount; ++itr)
            if ( *itr!=nSrcTab && maTabs[*itr] )
            {
                SCTAB i = *itr;
                if (bDoMix)
                {
                    if (!pMixDoc)
                    {
                        pMixDoc.reset(new ScDocument(SCDOCMODE_UNDO));
                        pMixDoc->InitUndo( this, i, i );
                    }
                    else
                        pMixDoc->AddUndoTab( i, i );

                    sc::CopyToDocContext aMixCxt(*pMixDoc);
                    maTabs[i]->CopyToTable(aMixCxt, nStartCol,nStartRow, nEndCol,nEndRow,
                                            IDF_CONTENTS, true, pMixDoc->maTabs[i], &rMark );
                }

                maTabs[i]->DeleteSelection( nDelFlags, rMark );
                maTabs[nSrcTab]->CopyToTable(aCxt, nStartCol,nStartRow, nEndCol,nEndRow,
                                             nFlags, true, maTabs[i], &rMark, bAsLink );

                if (bDoMix)
                    maTabs[i]->MixMarked(aMixDocCxt, rMark, nFunction, bSkipEmpty, pMixDoc->maTabs[i]);
            }

        SetAutoCalc( bOldAutoCalc );
    }
    else
    {
        OSL_FAIL("wrong table");
    }
}


bool ScDocument::SetString( SCCOL nCol, SCROW nRow, SCTAB nTab, const OUString& rString,
                            ScSetStringParam* pParam )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->SetString( nCol, nRow, nTab, rString, pParam );
    else
        return false;
}

bool ScDocument::SetString(
    const ScAddress& rPos, const OUString& rString, ScSetStringParam* pParam )
{
    return SetString(rPos.Col(), rPos.Row(), rPos.Tab(), rString, pParam);
}

void ScDocument::SetEditText( const ScAddress& rPos, EditTextObject* pEditText )
{
    if (!TableExists(rPos.Tab()))
    {
        delete pEditText;
        return;
    }

    maTabs[rPos.Tab()]->SetEditText(rPos.Col(), rPos.Row(), pEditText);
}

void ScDocument::SetEditText( const ScAddress& rPos, const EditTextObject& rEditText, const SfxItemPool* pEditPool )
{
    if (!TableExists(rPos.Tab()))
        return;

    maTabs[rPos.Tab()]->SetEditText(rPos.Col(), rPos.Row(), rEditText, pEditPool);
}

void ScDocument::SetEditText( const ScAddress& rPos, const OUString& rStr )
{
    if (!TableExists(rPos.Tab()))
        return;

    ScFieldEditEngine& rEngine = GetEditEngine();
    rEngine.SetText(rStr);
    maTabs[rPos.Tab()]->SetEditText(rPos.Col(), rPos.Row(), rEngine.CreateTextObject());
}

void ScDocument::SetTextCell( const ScAddress& rPos, const OUString& rStr )
{
    if (!TableExists(rPos.Tab()))
        return;

    if (ScStringUtil::isMultiline(rStr))
    {
        ScFieldEditEngine& rEngine = GetEditEngine();
        rEngine.SetText(rStr);
        maTabs[rPos.Tab()]->SetEditText(rPos.Col(), rPos.Row(), rEngine.CreateTextObject());
    }
    else
    {
        ScSetStringParam aParam;
        aParam.setTextInput();
        maTabs[rPos.Tab()]->SetString(rPos.Col(), rPos.Row(), rPos.Tab(), rStr, &aParam);
    }
}

void ScDocument::SetEmptyCell( const ScAddress& rPos )
{
    if (!TableExists(rPos.Tab()))
        return;

    maTabs[rPos.Tab()]->SetEmptyCell(rPos.Col(), rPos.Row());
}

void ScDocument::SetValue( SCCOL nCol, SCROW nRow, SCTAB nTab, const double& rVal )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
            maTabs[nTab]->SetValue( nCol, nRow, rVal );
}

void ScDocument::SetValue( const ScAddress& rPos, double fVal )
{
    if (!TableExists(rPos.Tab()))
        return;

    maTabs[rPos.Tab()]->SetValue(rPos.Col(), rPos.Row(), fVal);
}

OUString ScDocument::GetString( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
    {
        OUString aStr;
        maTabs[nTab]->GetString(nCol, nRow, aStr);
        return aStr;
    }
    else
        return EMPTY_OUSTRING;
}

OUString ScDocument::GetString( const ScAddress& rPos ) const
{
    if (!TableExists(rPos.Tab()))
        return EMPTY_OUSTRING;

    OUString aStr;
    maTabs[rPos.Tab()]->GetString(rPos.Col(), rPos.Row(), aStr);
    return aStr;
}

const OUString* ScDocument::GetStringCell( const ScAddress& rPos ) const
{
    if (!TableExists(rPos.Tab()))
        return NULL;

    return maTabs[rPos.Tab()]->GetStringCell(rPos.Col(), rPos.Row());
}

double* ScDocument::GetValueCell( const ScAddress& rPos )
{
    if (!TableExists(rPos.Tab()))
        return NULL;

    return maTabs[rPos.Tab()]->GetValueCell(rPos.Col(), rPos.Row());
}

void ScDocument::GetInputString( SCCOL nCol, SCROW nRow, SCTAB nTab, OUString& rString )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->GetInputString( nCol, nRow, rString );
    else
        rString = OUString();
}

void ScDocument::GetInputString( SCCOL nCol, SCROW nRow, SCTAB nTab, String& rString )
{
    OUString aString;
    GetInputString( nCol, nRow, nTab, aString);
    rString = aString;
}

sal_uInt16 ScDocument::GetStringForFormula( const ScAddress& rPos, OUString& rString )
{
    // Used in formulas (add-in parameters etc), so it must use the same semantics as
    // ScInterpreter::GetCellString: always format values as numbers.
    // The return value is the error code.

    ScRefCellValue aCell;
    aCell.assign(*this, rPos);
    if (aCell.isEmpty())
    {
        rString = EMPTY_OUSTRING;
        return 0;
    }

    sal_uInt16 nErr = 0;
    OUString aStr;
    SvNumberFormatter* pFormatter = GetFormatTable();
    switch (aCell.meType)
    {
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
            aStr = aCell.getString(this);
        break;
        case CELLTYPE_FORMULA:
        {
            ScFormulaCell* pFCell = aCell.mpFormula;
            nErr = pFCell->GetErrCode();
            if (pFCell->IsValue())
            {
                double fVal = pFCell->GetValue();
                sal_uInt32 nIndex = pFormatter->GetStandardFormat(
                                    NUMBERFORMAT_NUMBER,
                                    ScGlobal::eLnge);
                pFormatter->GetInputLineString(fVal, nIndex, aStr);
            }
            else
                aStr = pFCell->GetString();
        }
        break;
        case CELLTYPE_VALUE:
        {
            double fVal = aCell.mfValue;
            sal_uInt32 nIndex = pFormatter->GetStandardFormat(
                                    NUMBERFORMAT_NUMBER,
                                    ScGlobal::eLnge);
            pFormatter->GetInputLineString(fVal, nIndex, aStr);
        }
        break;
        default:
            ;
    }

    rString = aStr;
    return nErr;
}


void ScDocument::GetValue( SCCOL nCol, SCROW nRow, SCTAB nTab, double& rValue ) const
{
    if (TableExists(nTab))
        rValue = maTabs[nTab]->GetValue( nCol, nRow );
    else
        rValue = 0.0;
}

const EditTextObject* ScDocument::GetEditText( const ScAddress& rPos ) const
{
    SCTAB nTab = rPos.Tab();
    if (!TableExists(nTab))
        return NULL;

    return maTabs[nTab]->GetEditText(rPos.Col(), rPos.Row());
}

void ScDocument::RemoveEditTextCharAttribs( const ScAddress& rPos, const ScPatternAttr& rAttr )
{
    if (!TableExists(rPos.Tab()))
        return;

    return maTabs[rPos.Tab()]->RemoveEditTextCharAttribs(rPos.Col(), rPos.Row(), rAttr);
}

double ScDocument::GetValue( const ScAddress& rPos ) const
{
    SCTAB nTab = rPos.Tab();
    if ( nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetValue(rPos.Col(), rPos.Row());
    return 0.0;
}

double ScDocument::GetValue( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    ScAddress aAdr(nCol, nRow, nTab); return GetValue(aAdr);
}

void ScDocument::GetNumberFormat( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                  sal_uInt32& rFormat ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
        {
            rFormat = maTabs[nTab]->GetNumberFormat( nCol, nRow );
            return ;
        }
    rFormat = 0;
}

sal_uInt32 ScDocument::GetNumberFormat( const ScRange& rRange ) const
{
    SCTAB nTab1 = rRange.aStart.Tab(), nTab2 = rRange.aEnd.Tab();
    SCCOL nCol1 = rRange.aStart.Col(), nCol2 = rRange.aEnd.Col();
    SCROW nRow1 = rRange.aStart.Row(), nRow2 = rRange.aEnd.Row();

    if (!ValidTab(nTab1) || !ValidTab(nTab2) || !maTabs[nTab1] || !maTabs[nTab2])
        return 0;

    sal_uInt32 nFormat = 0;
    bool bFirstItem = true;
    for (SCTAB nTab = nTab1; nTab <= nTab2 && nTab < static_cast<SCTAB>(maTabs.size()) ; ++nTab)
        for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        {
            sal_uInt32 nThisFormat = maTabs[nTab]->GetNumberFormat(nCol, nRow1, nRow2);
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
    if ( maTabs[nTab] )
        return maTabs[nTab]->GetNumberFormat( rPos );
    return 0;
}

void ScDocument::SetNumberFormat( const ScAddress& rPos, sal_uInt32 nNumberFormat )
{
    if (!TableExists(rPos.Tab()))
        return;

    maTabs[rPos.Tab()]->SetNumberFormat(rPos.Col(), rPos.Row(), nNumberFormat);
}

void ScDocument::GetNumberFormatInfo( short& nType, sal_uLong& nIndex,
            const ScAddress& rPos ) const
{
    SCTAB nTab = rPos.Tab();
    if ( nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
    {
        nIndex = maTabs[nTab]->GetNumberFormat( rPos );
        nType = GetFormatTable()->GetType( nIndex );
    }
    else
    {
        nType = NUMBERFORMAT_UNDEFINED;
        nIndex = 0;
    }
}


void ScDocument::GetFormula( SCCOL nCol, SCROW nRow, SCTAB nTab, OUString& rFormula ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
            maTabs[nTab]->GetFormula( nCol, nRow, rFormula );
    else
        rFormula = OUString();
}


void ScDocument::GetFormula( SCCOL nCol, SCROW nRow, SCTAB nTab, String& rFormula ) const
{
    OUString aString;
    GetFormula( nCol, nRow, nTab, aString);
    rFormula = aString;
}

const ScTokenArray* ScDocument::GetFormulaTokens( const ScAddress& rPos ) const
{
    if (!TableExists(rPos.Tab()))
        return NULL;

    return maTabs[rPos.Tab()]->GetFormulaTokens(rPos.Col(), rPos.Row());
}

const ScFormulaCell* ScDocument::GetFormulaCell( const ScAddress& rPos ) const
{
    if (!TableExists(rPos.Tab()))
        return NULL;

    return maTabs[rPos.Tab()]->GetFormulaCell(rPos.Col(), rPos.Row());
}

ScFormulaCell* ScDocument::GetFormulaCell( const ScAddress& rPos )
{
    if (!TableExists(rPos.Tab()))
        return NULL;

    return maTabs[rPos.Tab()]->GetFormulaCell(rPos.Col(), rPos.Row());
}

CellType ScDocument::GetCellType( const ScAddress& rPos ) const
{
    SCTAB nTab = rPos.Tab();
    if ( nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetCellType( rPos );
    return CELLTYPE_NONE;
}


void ScDocument::GetCellType( SCCOL nCol, SCROW nRow, SCTAB nTab,
        CellType& rCellType ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        rCellType = maTabs[nTab]->GetCellType( nCol, nRow );
    else
        rCellType = CELLTYPE_NONE;
}

bool ScDocument::HasStringData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
            return maTabs[nTab]->HasStringData( nCol, nRow );
    else
        return false;
}


bool ScDocument::HasValueData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
            return maTabs[nTab]->HasValueData( nCol, nRow );
    else
        return false;
}

bool ScDocument::HasValueData( const ScAddress& rPos ) const
{
    return HasValueData(rPos.Col(), rPos.Row(), rPos.Tab());
}

bool ScDocument::HasStringCells( const ScRange& rRange ) const
{
    //  true, wenn String- oder Editzellen im Bereich

    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCTAB nStartTab = rRange.aStart.Tab();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();
    SCTAB nEndTab = rRange.aEnd.Tab();

    for ( SCTAB nTab=nStartTab; nTab<=nEndTab && nTab < static_cast<SCTAB>(maTabs.size()); nTab++ )
        if ( maTabs[nTab] && maTabs[nTab]->HasStringCells( nStartCol, nStartRow, nEndCol, nEndRow ) )
            return true;

    return false;
}


bool ScDocument::HasSelectionData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    sal_uInt32 nValidation = static_cast< const SfxUInt32Item* >( GetAttr( nCol, nRow, nTab, ATTR_VALIDDATA ) )->GetValue();
    if( nValidation )
    {
        const ScValidationData* pData = GetValidationEntry( nValidation );
        if( pData && pData->HasSelectionList() )
            return true;
    }
    return HasStringCells( ScRange( nCol, 0, nTab, nCol, MAXROW, nTab ) );
}


void ScDocument::InitializeNoteCaptions( SCTAB nTab, bool bForced )
{
    if( ValidTab( nTab ) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[ nTab ] )
        maTabs[ nTab ]->InitializeNoteCaptions( bForced );
}

void ScDocument::SetDirty()
{
    bool bOldAutoCalc = GetAutoCalc();
    bAutoCalc = false;      // keine Mehrfachberechnung
    {   // scope for bulk broadcast
        ScBulkBroadcast aBulkBroadcast( GetBASM());
        TableContainer::iterator it = maTabs.begin();
        for (;it != maTabs.end(); ++it)
            if (*it)
                (*it)->SetDirty();
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
    bool bOldAutoCalc = GetAutoCalc();
    bAutoCalc = false;      // keine Mehrfachberechnung
    {   // scope for bulk broadcast
        ScBulkBroadcast aBulkBroadcast( GetBASM());
        SCTAB nTab2 = rRange.aEnd.Tab();
        for (SCTAB i=rRange.aStart.Tab(); i<=nTab2 && i < static_cast<SCTAB>(maTabs.size()); i++)
            if (maTabs[i]) maTabs[i]->SetDirty( rRange );
    }
    SetAutoCalc( bOldAutoCalc );
}


void ScDocument::SetTableOpDirty( const ScRange& rRange )
{
    bool bOldAutoCalc = GetAutoCalc();
    bAutoCalc = false;      // no multiple recalculation
    SCTAB nTab2 = rRange.aEnd.Tab();
    for (SCTAB i=rRange.aStart.Tab(); i<=nTab2 && i < static_cast<SCTAB>(maTabs.size()); i++)
        if (maTabs[i]) maTabs[i]->SetTableOpDirty( rRange );
    SetAutoCalc( bOldAutoCalc );
}

void ScDocument::InterpretDirtyCells( const ScRangeList& rRanges )
{
    if (!GetAutoCalc())
        return;

    for (size_t nPos=0, nRangeCount = rRanges.size(); nPos < nRangeCount; nPos++)
    {
        const ScRange& rRange = *rRanges[nPos];
        for (SCTAB nTab = rRange.aStart.Tab(); nTab <= rRange.aEnd.Tab(); ++nTab)
        {
            ScTable* pTab = FetchTable(nTab);
            if (!pTab)
                return;

            pTab->InterpretDirtyCells(
                rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(), rRange.aEnd.Row());
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
    sc::AutoCalcSwitch aSwitch(*this, true);
    TableContainer::iterator it = maTabs.begin();
    for (; it != maTabs.end(); ++it)
        if (*it)
            (*it)->SetDirtyVar();
    for (it = maTabs.begin(); it != maTabs.end(); ++it)
        if (*it)
            (*it)->CalcAll();
    ClearFormulaTree();
}


void ScDocument::CompileAll()
{
    TableContainer::iterator it = maTabs.begin();
    for (; it != maTabs.end(); ++it)
        if (*it)
            (*it)->CompileAll();
    SetDirty();
}


void ScDocument::CompileXML()
{
    bool bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( false );
    ScProgress aProgress( GetDocumentShell(), ScGlobal::GetRscString(
                STR_PROGRESS_CALCULATING ), GetXMLImportedFormulaCount() );

    // set AutoNameCache to speed up automatic name lookup
    OSL_ENSURE( !pAutoNameCache, "AutoNameCache already set" );
    pAutoNameCache = new ScAutoNameCache( this );

    if (pRangeName)
        pRangeName->CompileUnresolvedXML();

    TableContainer::iterator it = maTabs.begin();
    for (; it != maTabs.end(); ++it)
        if (*it)
            (*it)->CompileXML( aProgress );

    DELETEZ( pAutoNameCache );  // valid only during CompileXML, where cell contents don't change

    if ( pValidationList )
        pValidationList->CompileXML();

    SetAutoCalc( bOldAutoCalc );
}

bool ScDocument::CompileErrorCells(sal_uInt16 nErrCode)
{
    bool bCompiled = false;
    TableContainer::iterator it = maTabs.begin(), itEnd = maTabs.end();
    for (; it != itEnd; ++it)
    {
        ScTable* pTab = *it;
        if (!pTab)
            continue;

        if (pTab->CompileErrorCells(nErrCode))
            bCompiled = true;
    }

    return bCompiled;
}

void ScDocument::CalcAfterLoad()
{
    if (bIsClip)    // Excel-Dateien werden aus dem Clipboard in ein Clip-Doc geladen
        return;     // dann wird erst beim Einfuegen in das richtige Doc berechnet

    bCalcingAfterLoad = true;
    {
        TableContainer::iterator it = maTabs.begin();
        for (; it != maTabs.end(); ++it)
            if (*it)
                (*it)->CalcAfterLoad();
        for (it = maTabs.begin(); it != maTabs.end(); ++it)
            if (*it)
                (*it)->SetDirtyAfterLoad();
    }
    bCalcingAfterLoad = false;

    SetDetectiveDirty(false);   // noch keine wirklichen Aenderungen

    // #i112436# If formula cells are already dirty, they don't broadcast further changes.
    // So the source ranges of charts must be interpreted even if they are not visible,
    // similar to ScMyShapeResizer::CreateChartListener for loading own files (i104899).
    if (pChartListenerCollection)
    {
        const ScChartListenerCollection::ListenersType& rListeners = pChartListenerCollection->getListeners();
        ScChartListenerCollection::ListenersType::const_iterator it = rListeners.begin(), itEnd = rListeners.end();
        for (; it != itEnd; ++it)
        {
            const ScChartListener* p = it->second;
            InterpretDirtyCells(*p->GetRangeList());
        }
    }
}


sal_uInt16 ScDocument::GetErrCode( const ScAddress& rPos ) const
{
    SCTAB nTab = rPos.Tab();
    if ( nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetErrCode( rPos );
    return 0;
}


void ScDocument::ResetChanged( const ScRange& rRange )
{
    SCTAB nTabSize = static_cast<SCTAB>(maTabs.size());
    SCTAB nTab1 = rRange.aStart.Tab();
    SCTAB nTab2 = rRange.aEnd.Tab();
    for (SCTAB nTab = nTab1; nTab1 <= nTab2 && nTab < nTabSize; ++nTab)
        if (maTabs[nTab])
            maTabs[nTab]->ResetChanged(rRange);
}

//
//  Spaltenbreiten / Zeilenhoehen   --------------------------------------
//


void ScDocument::SetColWidth( SCCOL nCol, SCTAB nTab, sal_uInt16 nNewWidth )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->SetColWidth( nCol, nNewWidth );
}

void ScDocument::SetColWidthOnly( SCCOL nCol, SCTAB nTab, sal_uInt16 nNewWidth )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->SetColWidthOnly( nCol, nNewWidth );
}

void ScDocument::SetRowHeight( SCROW nRow, SCTAB nTab, sal_uInt16 nNewHeight )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->SetRowHeight( nRow, nNewHeight );
}


void ScDocument::SetRowHeightRange( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, sal_uInt16 nNewHeight )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->SetRowHeightRange
            ( nStartRow, nEndRow, nNewHeight, 1.0, 1.0 );
}

void ScDocument::SetRowHeightOnly( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, sal_uInt16 nNewHeight )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->SetRowHeightOnly( nStartRow, nEndRow, nNewHeight );
}

void ScDocument::SetManualHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bManual )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->SetManualHeight( nStartRow, nEndRow, bManual );
}


sal_uInt16 ScDocument::GetColWidth( SCCOL nCol, SCTAB nTab, bool bHiddenAsZero ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetColWidth( nCol, bHiddenAsZero );
    OSL_FAIL("wrong table number");
    return 0;
}


sal_uInt16 ScDocument::GetOriginalWidth( SCCOL nCol, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetOriginalWidth( nCol );
    OSL_FAIL("wrong table number");
    return 0;
}


sal_uInt16 ScDocument::GetCommonWidth( SCCOL nEndCol, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetCommonWidth( nEndCol );
    OSL_FAIL("Wrong table number");
    return 0;
}


sal_uInt16 ScDocument::GetOriginalHeight( SCROW nRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetOriginalHeight( nRow );
    OSL_FAIL("Wrong table number");
    return 0;
}


sal_uInt16 ScDocument::GetRowHeight( SCROW nRow, SCTAB nTab, bool bHiddenAsZero ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetRowHeight( nRow, NULL, NULL, bHiddenAsZero );
    OSL_FAIL("Wrong sheet number");
    return 0;
}


sal_uInt16 ScDocument::GetRowHeight( SCROW nRow, SCTAB nTab, SCROW* pStartRow, SCROW* pEndRow, bool bHiddenAsZero ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetRowHeight( nRow, pStartRow, pEndRow, bHiddenAsZero );
    OSL_FAIL("Wrong sheet number");
    return 0;
}


sal_uLong ScDocument::GetRowHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bHiddenAsZero ) const
{
    if (nStartRow == nEndRow)
        return GetRowHeight( nStartRow, nTab, bHiddenAsZero );  // faster for a single row

    // check bounds because this method replaces former for(i=start;i<=end;++i) loops
    if (nStartRow > nEndRow)
        return 0;

    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetRowHeight( nStartRow, nEndRow, bHiddenAsZero );

    OSL_FAIL("wrong sheet number");
    return 0;
}

SCROW ScDocument::GetRowForHeight( SCTAB nTab, sal_uLong nHeight ) const
{
    return maTabs[nTab]->GetRowForHeight(nHeight);
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

    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetScaledRowHeight( nStartRow, nEndRow, fScale);

    OSL_FAIL("wrong sheet number");
    return 0;
}

SCROW ScDocument::GetHiddenRowCount( SCROW nRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetHiddenRowCount( nRow );
    OSL_FAIL("wrong table number");
    return 0;
}


sal_uLong ScDocument::GetColOffset( SCCOL nCol, SCTAB nTab, bool bHiddenAsZero ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetColOffset( nCol, bHiddenAsZero );
    OSL_FAIL("wrong table number");
    return 0;
}


sal_uLong ScDocument::GetRowOffset( SCROW nRow, SCTAB nTab, bool bHiddenAsZero ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetRowOffset( nRow, bHiddenAsZero );
    OSL_FAIL("wrong table number");
    return 0;
}


sal_uInt16 ScDocument::GetOptimalColWidth( SCCOL nCol, SCTAB nTab, OutputDevice* pDev,
                                       double nPPTX, double nPPTY,
                                       const Fraction& rZoomX, const Fraction& rZoomY,
                                       bool bFormula, const ScMarkData* pMarkData,
                                       const ScColWidthParam* pParam )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetOptimalColWidth( nCol, pDev, nPPTX, nPPTY,
            rZoomX, rZoomY, bFormula, pMarkData, pParam );
    OSL_FAIL("wrong table number");
    return 0;
}


long ScDocument::GetNeededSize( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    bool bWidth, bool bTotalSize )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetNeededSize
                ( nCol, nRow, pDev, nPPTX, nPPTY, rZoomX, rZoomY, bWidth, bTotalSize );
    OSL_FAIL("wrong table number");
    return 0;
}


bool ScDocument::SetOptimalHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, sal_uInt16 nExtra,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    bool bShrink )
{
//! MarkToMulti();
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->SetOptimalHeight( nStartRow, nEndRow, nExtra,
                                                pDev, nPPTX, nPPTY, rZoomX, rZoomY, bShrink );
    OSL_FAIL("wrong table number");
    return false;
}


void ScDocument::UpdateAllRowHeights( OutputDevice* pDev, double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY, const ScMarkData* pTabMark )
{
    // one progress across all (selected) sheets

    sal_uLong nCellCount = 0;
    for ( SCTAB nTab=0; nTab< static_cast<SCTAB>(maTabs.size()); nTab++ )
        if ( maTabs[nTab] && ( !pTabMark || pTabMark->GetTableSelect(nTab) ) )
            nCellCount += maTabs[nTab]->GetWeightedCount();

    ScProgress aProgress( GetDocumentShell(), ScGlobal::GetRscString(STR_PROGRESS_HEIGHTING), nCellCount );

    sal_uLong nProgressStart = 0;
    for ( SCTAB nTab=0; nTab< static_cast<SCTAB>(maTabs.size()); nTab++ )
        if ( maTabs[nTab] && ( !pTabMark || pTabMark->GetTableSelect(nTab) ) )
        {
            maTabs[nTab]->SetOptimalHeightOnly( 0, MAXROW, 0,
                        pDev, nPPTX, nPPTY, rZoomX, rZoomY, false, &aProgress, nProgressStart );
            maTabs[nTab]->SetDrawPageSize(true, true);
            nProgressStart += maTabs[nTab]->GetWeightedCount();
        }
}


//
//  Spalten-/Zeilen-Flags   ----------------------------------------------
//

void ScDocument::ShowCol(SCCOL nCol, SCTAB nTab, bool bShow)
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->ShowCol( nCol, bShow );
}


void ScDocument::ShowRow(SCROW nRow, SCTAB nTab, bool bShow)
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->ShowRow( nRow, bShow );
}


void ScDocument::ShowRows(SCROW nRow1, SCROW nRow2, SCTAB nTab, bool bShow)
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->ShowRows( nRow1, nRow2, bShow );
}


void ScDocument::SetRowFlags( SCROW nRow, SCTAB nTab, sal_uInt8 nNewFlags )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->SetRowFlags( nRow, nNewFlags );
}


void ScDocument::SetRowFlags( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, sal_uInt8 nNewFlags )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->SetRowFlags( nStartRow, nEndRow, nNewFlags );
}


sal_uInt8 ScDocument::GetColFlags( SCCOL nCol, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetColFlags( nCol );
    OSL_FAIL("wrong table number");
    return 0;
}

sal_uInt8 ScDocument::GetRowFlags( SCROW nRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetRowFlags( nRow );
    OSL_FAIL("wrong table number");
    return 0;
}

const ScBitMaskCompressedArray< SCROW, sal_uInt8> & ScDocument::GetRowFlagsArray(
        SCTAB nTab ) const
{
    const ScBitMaskCompressedArray< SCROW, sal_uInt8> * pFlags;
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        pFlags = maTabs[nTab]->GetRowFlagsArray();
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
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return;
    maTabs[nTab]->GetAllRowBreaks(rBreaks, bPage, bManual);
}

void ScDocument::GetAllColBreaks(set<SCCOL>& rBreaks, SCTAB nTab, bool bPage, bool bManual) const
{
    if (!ValidTab(nTab) || !maTabs[nTab])
        return;

    maTabs[nTab]->GetAllColBreaks(rBreaks, bPage, bManual);
}

ScBreakType ScDocument::HasRowBreak(SCROW nRow, SCTAB nTab) const
{
    ScBreakType nType = BREAK_NONE;
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab] || !ValidRow(nRow))
        return nType;

    if (maTabs[nTab]->HasRowPageBreak(nRow))
        nType |= BREAK_PAGE;

    if (maTabs[nTab]->HasRowManualBreak(nRow))
        nType |= BREAK_MANUAL;

    return nType;
}

ScBreakType ScDocument::HasColBreak(SCCOL nCol, SCTAB nTab) const
{
    ScBreakType nType = BREAK_NONE;
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab] || !ValidCol(nCol))
        return nType;

    if (maTabs[nTab]->HasColPageBreak(nCol))
        nType |= BREAK_PAGE;

    if (maTabs[nTab]->HasColManualBreak(nCol))
        nType |= BREAK_MANUAL;

    return nType;
}

void ScDocument::SetRowBreak(SCROW nRow, SCTAB nTab, bool bPage, bool bManual)
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab] || !ValidRow(nRow))
        return;

    maTabs[nTab]->SetRowBreak(nRow, bPage, bManual);
}

void ScDocument::SetColBreak(SCCOL nCol, SCTAB nTab, bool bPage, bool bManual)
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab] || !ValidCol(nCol))
        return;

    maTabs[nTab]->SetColBreak(nCol, bPage, bManual);
}

void ScDocument::RemoveRowBreak(SCROW nRow, SCTAB nTab, bool bPage, bool bManual)
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab] || !ValidRow(nRow))
        return;

    maTabs[nTab]->RemoveRowBreak(nRow, bPage, bManual);
}

void ScDocument::RemoveColBreak(SCCOL nCol, SCTAB nTab, bool bPage, bool bManual)
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab] || !ValidCol(nCol))
        return;

    maTabs[nTab]->RemoveColBreak(nCol, bPage, bManual);
}

Sequence<TablePageBreakData> ScDocument::GetRowBreakData(SCTAB nTab) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return Sequence<TablePageBreakData>();

    return maTabs[nTab]->GetRowBreakData();
}

bool ScDocument::RowHidden(SCROW nRow, SCTAB nTab, SCROW* pFirstRow, SCROW* pLastRow) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return false;

    return maTabs[nTab]->RowHidden(nRow, pFirstRow, pLastRow);
}

bool ScDocument::HasHiddenRows(SCROW nStartRow, SCROW nEndRow, SCTAB nTab) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return false;

    return maTabs[nTab]->HasHiddenRows(nStartRow, nEndRow);
}

bool ScDocument::ColHidden(SCCOL nCol, SCTAB nTab, SCCOL* pFirstCol, SCCOL* pLastCol) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
    {
        if (pFirstCol)
            *pFirstCol = nCol;
        if (pLastCol)
            *pLastCol = nCol;
        return false;
    }

    return maTabs[nTab]->ColHidden(nCol, pFirstCol, pLastCol);
}

void ScDocument::SetRowHidden(SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bHidden)
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return;

    maTabs[nTab]->SetRowHidden(nStartRow, nEndRow, bHidden);
}

void ScDocument::SetColHidden(SCCOL nStartCol, SCCOL nEndCol, SCTAB nTab, bool bHidden)
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return;

    maTabs[nTab]->SetColHidden(nStartCol, nEndCol, bHidden);
}

SCROW ScDocument::FirstVisibleRow(SCROW nStartRow, SCROW nEndRow, SCTAB nTab) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return ::std::numeric_limits<SCROW>::max();;

    return maTabs[nTab]->FirstVisibleRow(nStartRow, nEndRow);
}

SCROW ScDocument::LastVisibleRow(SCROW nStartRow, SCROW nEndRow, SCTAB nTab) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return ::std::numeric_limits<SCROW>::max();;

    return maTabs[nTab]->LastVisibleRow(nStartRow, nEndRow);
}

SCROW ScDocument::CountVisibleRows(SCROW nStartRow, SCROW nEndRow, SCTAB nTab) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return 0;

    return maTabs[nTab]->CountVisibleRows(nStartRow, nEndRow);
}

bool ScDocument::RowFiltered(SCROW nRow, SCTAB nTab, SCROW* pFirstRow, SCROW* pLastRow) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return false;

    return maTabs[nTab]->RowFiltered(nRow, pFirstRow, pLastRow);
}

bool ScDocument::HasFilteredRows(SCROW nStartRow, SCROW nEndRow, SCTAB nTab) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return false;

    return maTabs[nTab]->HasFilteredRows(nStartRow, nEndRow);
}

bool ScDocument::ColFiltered(SCCOL nCol, SCTAB nTab, SCCOL* pFirstCol, SCCOL* pLastCol) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return false;

    return maTabs[nTab]->ColFiltered(nCol, pFirstCol, pLastCol);
}

void ScDocument::SetRowFiltered(SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bFiltered)
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return;

    maTabs[nTab]->SetRowFiltered(nStartRow, nEndRow, bFiltered);
}


SCROW ScDocument::FirstNonFilteredRow(SCROW nStartRow, SCROW nEndRow, SCTAB nTab) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return ::std::numeric_limits<SCROW>::max();;

    return maTabs[nTab]->FirstNonFilteredRow(nStartRow, nEndRow);
}

SCROW ScDocument::LastNonFilteredRow(SCROW nStartRow, SCROW nEndRow, SCTAB nTab) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return ::std::numeric_limits<SCROW>::max();;

    return maTabs[nTab]->LastNonFilteredRow(nStartRow, nEndRow);
}

SCROW ScDocument::CountNonFilteredRows(SCROW nStartRow, SCROW nEndRow, SCTAB nTab) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return 0;

    return maTabs[nTab]->CountNonFilteredRows(nStartRow, nEndRow);
}

bool ScDocument::IsManualRowHeight(SCROW nRow, SCTAB nTab) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return false;

    return maTabs[nTab]->IsManualRowHeight(nRow);
}

void ScDocument::SyncColRowFlags()
{
    TableContainer::iterator it = maTabs.begin();
    for (; it != maTabs.end(); ++it)
    {
        if (*it)
            (*it)->SyncColRowFlags();
    }
}

SCROW ScDocument::GetLastFlaggedRow( SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetLastFlaggedRow();
    return 0;
}


SCCOL ScDocument::GetLastChangedCol( SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetLastChangedCol();
    return 0;
}

SCROW ScDocument::GetLastChangedRow( SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetLastChangedRow();
    return 0;
}


SCCOL ScDocument::GetNextDifferentChangedCol( SCTAB nTab, SCCOL nStart) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
    {
        sal_uInt8 nStartFlags = maTabs[nTab]->GetColFlags(nStart);
        sal_uInt16 nStartWidth = maTabs[nTab]->GetOriginalWidth(nStart);
        for (SCCOL nCol = nStart + 1; nCol <= MAXCOL; nCol++)
        {
            if (((nStartFlags & CR_MANUALBREAK) != (maTabs[nTab]->GetColFlags(nCol) & CR_MANUALBREAK)) ||
                (nStartWidth != maTabs[nTab]->GetOriginalWidth(nCol)) ||
                ((nStartFlags & CR_HIDDEN) != (maTabs[nTab]->GetColFlags(nCol) & CR_HIDDEN)) )
                return nCol;
        }
        return MAXCOL+1;
    }
    return 0;
}

SCROW ScDocument::GetNextDifferentChangedRow( SCTAB nTab, SCROW nStart, bool bCareManualSize) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return 0;

    const ScBitMaskCompressedArray<SCROW, sal_uInt8>* pRowFlagsArray = maTabs[nTab]->GetRowFlagsArray();
    if (!pRowFlagsArray)
        return 0;

    if (!maTabs[nTab]->mpRowHeights || !maTabs[nTab]->mpHiddenRows)
        return 0;

    size_t nIndex;          // ignored
    SCROW nFlagsEndRow;
    SCROW nHiddenEndRow;
    SCROW nHeightEndRow;
    sal_uInt8 nFlags;
    bool bHidden;
    sal_uInt16 nHeight;
    sal_uInt8 nStartFlags = nFlags = pRowFlagsArray->GetValue( nStart, nIndex, nFlagsEndRow);
    bool bStartHidden = bHidden = maTabs[nTab]->RowHidden( nStart, NULL, &nHiddenEndRow);
    sal_uInt16 nStartHeight = nHeight = maTabs[nTab]->GetRowHeight( nStart, NULL, &nHeightEndRow, false);
    SCROW nRow;
    while ((nRow = std::min( nHiddenEndRow, std::min( nFlagsEndRow, nHeightEndRow)) + 1) <= MAXROW)
    {
        if (nFlagsEndRow < nRow)
            nFlags = pRowFlagsArray->GetValue( nRow, nIndex, nFlagsEndRow);
        if (nHiddenEndRow < nRow)
            bHidden = maTabs[nTab]->RowHidden( nRow, NULL, &nHiddenEndRow);
        if (nHeightEndRow < nRow)
            nHeight = maTabs[nTab]->GetRowHeight( nRow, NULL, &nHeightEndRow, false);

        if (((nStartFlags & CR_MANUALBREAK) != (nFlags & CR_MANUALBREAK)) ||
            ((nStartFlags & CR_MANUALSIZE) != (nFlags & CR_MANUALSIZE)) ||
            (bStartHidden != bHidden) ||
            (bCareManualSize && (nStartFlags & CR_MANUALSIZE) && (nStartHeight != nHeight)) ||
            (!bCareManualSize && ((nStartHeight != nHeight))))
            return nRow;
    }

    return MAXROW+1;
}

bool ScDocument::GetColDefault( SCTAB nTab, SCCOL nCol, SCROW nLastRow, SCROW& nDefault)
{
    bool bRet(false);
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
        bRet = true;
    }
    else
        bRet = true;
    return bRet;
}

bool ScDocument::GetRowDefault( SCTAB /* nTab */, SCROW /* nRow */, SCCOL /* nLastCol */, SCCOL& /* nDefault */ )
{
    return false;
}

void ScDocument::StripHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2, SCTAB nTab )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->StripHidden( rX1, rY1, rX2, rY2 );
}


void ScDocument::ExtendHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2, SCTAB nTab )
{
    if ( ValidTab(nTab) && maTabs[nTab] )
        maTabs[nTab]->ExtendHidden( rX1, rY1, rX2, rY2 );
}

//
//  Attribute   ----------------------------------------------------------
//

const SfxPoolItem* ScDocument::GetAttr( SCCOL nCol, SCROW nRow, SCTAB nTab, sal_uInt16 nWhich ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
    {
        const SfxPoolItem* pTemp = maTabs[nTab]->GetAttr( nCol, nRow, nWhich );
        if (pTemp)
            return pTemp;
        else
        {
            OSL_FAIL( "Attribut Null" );
        }
    }
    return &xPoolHelper->GetDocPool()->GetDefaultItem( nWhich );
}

const SfxPoolItem* ScDocument::GetAttr( const ScAddress& rPos, sal_uInt16 nWhich ) const
{
    return GetAttr(rPos.Col(), rPos.Row(), rPos.Tab(), nWhich);
}

const ScPatternAttr* ScDocument::GetPattern( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    if (TableExists(nTab))
        return maTabs[nTab]->GetPattern( nCol, nRow );
    return NULL;
}

const ScPatternAttr* ScDocument::GetPattern( const ScAddress& rPos ) const
{
    if (TableExists(rPos.Tab()))
        return maTabs[rPos.Tab()]->GetPattern(rPos.Col(), rPos.Row());

    return NULL;
}

const ScPatternAttr* ScDocument::GetMostUsedPattern( SCCOL nCol, SCROW nStartRow, SCROW nEndRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetMostUsedPattern( nCol, nStartRow, nEndRow );
    return NULL;
}


void ScDocument::ApplyAttr( SCCOL nCol, SCROW nRow, SCTAB nTab, const SfxPoolItem& rAttr )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->ApplyAttr( nCol, nRow, rAttr );
}


void ScDocument::ApplyPattern( SCCOL nCol, SCROW nRow, SCTAB nTab, const ScPatternAttr& rAttr )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->ApplyPattern( nCol, nRow, rAttr );
}


void ScDocument::ApplyPatternArea( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark,
                        const ScPatternAttr& rAttr,
                        ScEditDataArray* pDataArray )
{
    SCTAB nMax = static_cast<SCTAB>(maTabs.size());
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nMax; ++itr)
        if (maTabs[*itr])
            maTabs[*itr]->ApplyPatternArea( nStartCol, nStartRow, nEndCol, nEndRow, rAttr, pDataArray );
}


void ScDocument::ApplyPatternAreaTab( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow, SCTAB nTab, const ScPatternAttr& rAttr )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
            maTabs[nTab]->ApplyPatternArea( nStartCol, nStartRow, nEndCol, nEndRow, rAttr );
}

bool ScDocument::SetAttrEntries(SCCOL nCol, SCTAB nTab, ScAttrEntry* pData, SCSIZE nSize)
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return false;

    return maTabs[nTab]->SetAttrEntries(nCol, pData, nSize);
}

void ScDocument::ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
        const ScMarkData& rMark, const ScPatternAttr& rPattern, short nNewType )
{
    SCTAB nMax = static_cast<SCTAB>(maTabs.size());
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nMax; ++itr)
        if (maTabs[*itr])
            maTabs[*itr]->ApplyPatternIfNumberformatIncompatible( rRange, rPattern, nNewType );
}

void ScDocument::AddCondFormatData( const ScRangeList& rRange, SCTAB nTab, sal_uInt32 nIndex )
{
    if(!(static_cast<size_t>(nTab) < maTabs.size()))
        return;

    if(!maTabs[nTab])
        return;

    maTabs[nTab]->AddCondFormatData(rRange, nIndex);
}

void ScDocument::RemoveCondFormatData( const ScRangeList& rRange, SCTAB nTab, sal_uInt32 nIndex )
{
    if(!(static_cast<size_t>(nTab) < maTabs.size()))
        return;

    if(!maTabs[nTab])
        return;

    maTabs[nTab]->RemoveCondFormatData(rRange, nIndex);
}


void ScDocument::ApplyStyle( SCCOL nCol, SCROW nRow, SCTAB nTab, const ScStyleSheet& rStyle)
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
            maTabs[nTab]->ApplyStyle( nCol, nRow, rStyle );
}


void ScDocument::ApplyStyleArea( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow,
                        const ScMarkData& rMark,
                        const ScStyleSheet& rStyle)
{
    SCTAB nMax = static_cast<SCTAB>(maTabs.size());
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nMax; ++itr)
        if (maTabs[*itr])
            maTabs[*itr]->ApplyStyleArea( nStartCol, nStartRow, nEndCol, nEndRow, rStyle );
}


void ScDocument::ApplyStyleAreaTab( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow, SCTAB nTab, const ScStyleSheet& rStyle)
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
            maTabs[nTab]->ApplyStyleArea( nStartCol, nStartRow, nEndCol, nEndRow, rStyle );
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
        SCTAB nMax = static_cast<SCTAB>(maTabs.size());
        ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd && *itr < nMax; ++itr)
            if ( maTabs[*itr] )
                    maTabs[*itr]->ApplySelectionStyle( rStyle, rMark );
    }
}


void ScDocument::ApplySelectionLineStyle( const ScMarkData& rMark,
                    const SvxBorderLine* pLine, bool bColorOnly )
{
    if ( bColorOnly && !pLine )
        return;

    SCTAB nMax = static_cast<SCTAB>(maTabs.size());
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nMax; ++itr)
        if (maTabs[*itr])
            maTabs[*itr]->ApplySelectionLineStyle( rMark, pLine, bColorOnly );
}


const ScStyleSheet* ScDocument::GetStyle( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetStyle(nCol, nRow);
    else
        return NULL;
}


const ScStyleSheet* ScDocument::GetSelectionStyle( const ScMarkData& rMark ) const
{
    bool    bEqual = true;
    bool    bFound;

    const ScStyleSheet* pStyle = NULL;
    const ScStyleSheet* pNewStyle;

    if ( rMark.IsMultiMarked() )
    {
        SCTAB nMax = static_cast<SCTAB>(maTabs.size());
        ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd && *itr < nMax; ++itr)
            if (maTabs[*itr])
            {
                pNewStyle = maTabs[*itr]->GetSelectionStyle( rMark, bFound );
                if (bFound)
                {
                    if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
                        bEqual = false;                                             // unterschiedliche
                    pStyle = pNewStyle;
                }
            }
    }
    if ( rMark.IsMarked() )
    {
        ScRange aRange;
        rMark.GetMarkArea( aRange );
        for (SCTAB i=aRange.aStart.Tab(); i<=aRange.aEnd.Tab() && bEqual && i < static_cast<SCTAB>(maTabs.size()); i++)
            if (maTabs[i] && rMark.GetTableSelect(i))
            {
                pNewStyle = maTabs[i]->GetAreaStyle( bFound,
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


void ScDocument::StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, bool bRemoved,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY )
{
    TableContainer::iterator it = maTabs.begin();
    for (; it != maTabs.end(); ++it)
        if (*it)
            (*it)->StyleSheetChanged
                ( pStyleSheet, bRemoved, pDev, nPPTX, nPPTY, rZoomX, rZoomY );

    if ( pStyleSheet && pStyleSheet->GetName() == ScGlobal::GetRscString(STR_STYLENAME_STANDARD) )
    {
        //  update attributes for all note objects
        ScDetectiveFunc::UpdateAllComments( *this );
    }
}


bool ScDocument::IsStyleSheetUsed( const ScStyleSheet& rStyle, bool bGatherAllStyles ) const
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

        bool bIsUsed = false;

        TableContainer::const_iterator it = maTabs.begin();
        for (; it != maTabs.end(); ++it)
            if (*it)
            {
                if ( (*it)->IsStyleSheetUsed( rStyle, bGatherAllStyles ) )
                {
                    if ( !bGatherAllStyles )
                        return true;
                    bIsUsed = true;
                }
            }

        if ( bGatherAllStyles )
            bStyleSheetUsageInvalid = false;

        return bIsUsed;
    }

    return rStyle.GetUsage() == ScStyleSheet::USED;
}


bool ScDocument::ApplyFlagsTab( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow, SCTAB nTab, sal_Int16 nFlags )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
            return maTabs[nTab]->ApplyFlags( nStartCol, nStartRow, nEndCol, nEndRow, nFlags );

    OSL_FAIL("ApplyFlags: wrong table");
    return false;
}


bool ScDocument::RemoveFlagsTab( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow, SCTAB nTab, sal_Int16 nFlags )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
            return maTabs[nTab]->RemoveFlags( nStartCol, nStartRow, nEndCol, nEndRow, nFlags );

    OSL_FAIL("RemoveFlags: wrong table");
    return false;
}


void ScDocument::SetPattern( SCCOL nCol, SCROW nRow, SCTAB nTab, const ScPatternAttr& rAttr,
                                bool bPutToPool )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
            maTabs[nTab]->SetPattern( nCol, nRow, rAttr, bPutToPool );
}


void ScDocument::SetPattern( const ScAddress& rPos, const ScPatternAttr& rAttr,
                                bool bPutToPool )
{
    SCTAB nTab = rPos.Tab();
    if ( nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->SetPattern( rPos, rAttr, bPutToPool );
}


ScPatternAttr* ScDocument::CreateSelectionPattern( const ScMarkData& rMark, bool bDeep )
{
    ScMergePatternState aState;

    if ( rMark.IsMultiMarked() )                                // multi selection
    {
        SCTAB nMax = static_cast<SCTAB>(maTabs.size());
        ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd && *itr < nMax; ++itr)
            if (maTabs[*itr])
                maTabs[*itr]->MergeSelectionPattern( aState, rMark, bDeep );
    }
    if ( rMark.IsMarked() )                                     // simle selection
    {
        ScRange aRange;
        rMark.GetMarkArea(aRange);
        SCTAB nMax = static_cast<SCTAB>(maTabs.size());
        ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd && *itr < nMax; ++itr)
            if (maTabs[*itr])
                maTabs[*itr]->MergePatternArea( aState,
                                aRange.aStart.Col(), aRange.aStart.Row(),
                                aRange.aEnd.Col(), aRange.aEnd.Row(), bDeep );
    }

    OSL_ENSURE( aState.pItemSet, "SelectionPattern Null" );
    if (aState.pItemSet)
        return new ScPatternAttr( aState.pItemSet );
    else
        return new ScPatternAttr( GetPool() );      // empty
}


const ScPatternAttr* ScDocument::GetSelectionPattern( const ScMarkData& rMark, bool bDeep )
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
    rLineInner.SetTable(true);
    rLineInner.SetDist(true);
    rLineInner.SetMinDist(false);

    ScLineFlags aFlags;

    if (rMark.IsMarked())
    {
        ScRange aRange;
        rMark.GetMarkArea(aRange);
        rLineInner.EnableHor( aRange.aStart.Row() != aRange.aEnd.Row() );
        rLineInner.EnableVer( aRange.aStart.Col() != aRange.aEnd.Col() );
        SCTAB nMax = static_cast<SCTAB>(maTabs.size());
        ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd && *itr < nMax; ++itr)
            if (maTabs[*itr])
                maTabs[*itr]->MergeBlockFrame( &rLineOuter, &rLineInner, aFlags,
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
                            SCCOL nCol2, SCROW nRow2, SCTAB nTab2, sal_uInt16 nMask ) const
{
    if ( nMask & HASATTR_ROTATE )
    {
        //  Attribut im Dokument ueberhaupt verwendet?
        //  (wie in fillinfo)

        ScDocumentPool* pPool = xPoolHelper->GetDocPool();

        bool bAnyItem = false;
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
                    bAnyItem = true;
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

        bool bHasRtl = false;
        sal_uInt32 nDirCount = pPool->GetItemCount2( ATTR_WRITINGDIR );
        for (sal_uInt32 nItem=0; nItem<nDirCount; nItem++)
        {
            const SfxPoolItem* pItem = pPool->GetItem2( ATTR_WRITINGDIR, nItem );
            if ( pItem && ((const SvxFrameDirectionItem*)pItem)->GetValue() == FRMDIR_HORI_RIGHT_TOP )
            {
                bHasRtl = true;
                break;
            }
        }
        if (!bHasRtl)
            nMask &= ~HASATTR_RTL;
    }

    if (!nMask)
        return false;

    bool bFound = false;
    for (SCTAB i=nTab1; i<=nTab2 && !bFound && i < static_cast<SCTAB>(maTabs.size()); i++)
        if (maTabs[i])
        {
            if ( nMask & HASATTR_RTL )
            {
                if ( GetEditTextDirection(i) == EE_HTEXTDIR_R2L )       // sheet default
                    bFound = true;
            }
            if ( nMask & HASATTR_RIGHTORCENTER )
            {
                //  On a RTL sheet, don't start to look for the default left value
                //  (which is then logically right), instead always assume true.
                //  That way, ScAttrArray::HasAttrib doesn't have to handle RTL sheets.

                if ( IsLayoutRTL(i) )
                    bFound = true;
            }

            if ( !bFound )
                bFound = maTabs[i]->HasAttrib( nCol1, nRow1, nCol2, nRow2, nMask );
        }

    return bFound;
}

bool ScDocument::HasAttrib( const ScRange& rRange, sal_uInt16 nMask ) const
{
    return HasAttrib( rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
                      rRange.aEnd.Col(),   rRange.aEnd.Row(),   rRange.aEnd.Tab(),
                      nMask );
}

void ScDocument::FindMaxRotCol( SCTAB nTab, RowInfo* pRowInfo, SCSIZE nArrCount,
                                SCCOL nX1, SCCOL nX2 ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->FindMaxRotCol( pRowInfo, nArrCount, nX1, nX2 );
    else
    {
        OSL_FAIL("FindMaxRotCol: wrong table");
    }
}

void ScDocument::GetBorderLines( SCCOL nCol, SCROW nRow, SCTAB nTab,
                        const SvxBorderLine** ppLeft, const SvxBorderLine** ppTop,
                        const SvxBorderLine** ppRight, const SvxBorderLine** ppBottom ) const
{
    //! Seitengrenzen fuer Druck beruecksichtigen !!!!!

    const SvxBoxItem* pThisAttr = (const SvxBoxItem*) GetEffItem( nCol, nRow, nTab, ATTR_BORDER );
    OSL_ENSURE(pThisAttr,"where is the attribute?");

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

bool ScDocument::IsBlockEmpty( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow, bool bIgnoreNotes ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
            return maTabs[nTab]->IsBlockEmpty( nStartCol, nStartRow, nEndCol, nEndRow, bIgnoreNotes );

    OSL_FAIL("wrong table number");
    return false;
}


void ScDocument::LockTable(SCTAB nTab)
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->LockTable();
    else
    {
        OSL_FAIL("wrong table number");
    }
}


void ScDocument::UnlockTable(SCTAB nTab)
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->UnlockTable();
    else
    {
        OSL_FAIL("wrong table number");
    }
}


bool ScDocument::IsBlockEditable( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow,
                                        bool* pOnlyNotBecauseOfMatrix /* = NULL */ ) const
{
    // import into read-only document is possible
    if (!bImportingXML && !mbChangeReadOnlyEnabled && pShell && pShell->IsReadOnly())
    {
        if ( pOnlyNotBecauseOfMatrix )
            *pOnlyNotBecauseOfMatrix = false;
        return false;
    }

    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
            return maTabs[nTab]->IsBlockEditable( nStartCol, nStartRow, nEndCol,
                nEndRow, pOnlyNotBecauseOfMatrix );

    OSL_FAIL("wrong table number");
    if ( pOnlyNotBecauseOfMatrix )
        *pOnlyNotBecauseOfMatrix = false;
    return false;
}


bool ScDocument::IsSelectionEditable( const ScMarkData& rMark,
            bool* pOnlyNotBecauseOfMatrix /* = NULL */ ) const
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

    bool bOk = true;
    bool bMatrix = ( pOnlyNotBecauseOfMatrix != NULL );
    SCTAB nMax = static_cast<SCTAB>(maTabs.size());
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nMax && (bOk || bMatrix); ++itr)
    {
        if ( maTabs[*itr] )
        {
            if (rMark.IsMarked())
            {
                if ( !maTabs[*itr]->IsBlockEditable( aRange.aStart.Col(),
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
                if ( !maTabs[*itr]->IsSelectionEditable( rMark, pOnlyNotBecauseOfMatrix ) )
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


bool ScDocument::HasSelectedBlockMatrixFragment( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow,
                                const ScMarkData& rMark ) const
{
    bool bOk = true;
    SCTAB nMax = static_cast<SCTAB>(maTabs.size());
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nMax && bOk; ++itr)
        if (maTabs[*itr])
            if (maTabs[*itr]->HasBlockMatrixFragment( nStartCol, nStartRow, nEndCol, nEndRow ))
                bOk = false;

    return !bOk;
}

bool ScDocument::GetMatrixFormulaRange( const ScAddress& rCellPos, ScRange& rMatrix )
{
    //  if rCell is part of a matrix formula, return its complete range

    ScFormulaCell* pFCell = GetFormulaCell(rCellPos);
    if (!pFCell)
        // not a formula cell.  Bail out.
        return false;

    ScAddress aOrigin = rCellPos;
    if (!pFCell->GetMatrixOrigin(aOrigin))
        // Failed to get the address of the matrix origin.
        return false;

    if (aOrigin != rCellPos)
    {
        pFCell = GetFormulaCell(aOrigin);
        if (!pFCell)
            // The matrix origin cell is not a formula cell !?  Something is up...
            return false;
    }

    SCCOL nSizeX;
    SCROW nSizeY;
    pFCell->GetMatColsRows(nSizeX, nSizeY);
    if (nSizeX <= 0 || nSizeY <= 0)
    {
        // GetMatrixEdge computes also dimensions of the matrix
        // if not already done (may occur if document is loaded
        // from old file format).
        // Needs an "invalid" initialized address.
        aOrigin.SetInvalid();
        pFCell->GetMatrixEdge(aOrigin);
        pFCell->GetMatColsRows(nSizeX, nSizeY);
    }

    if (nSizeX <= 0 || nSizeY <= 0)
        // Matrix size is still invalid. Give up.
        return false;

    ScAddress aEnd( aOrigin.Col() + nSizeX - 1,
                    aOrigin.Row() + nSizeY - 1,
                    aOrigin.Tab() );

    rMatrix.aStart = aOrigin;
    rMatrix.aEnd = aEnd;

    return true;
}


bool ScDocument::ExtendOverlapped( SCCOL& rStartCol, SCROW& rStartRow,
                                SCCOL nEndCol, SCROW nEndRow, SCTAB nTab ) const
{
    bool bFound = false;
    if ( ValidColRow(rStartCol,rStartRow) && ValidColRow(nEndCol,nEndRow) && ValidTab(nTab) )
    {
        if (nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        {
            SCCOL nCol;
            SCCOL nOldCol = rStartCol;
            SCROW nOldRow = rStartRow;
            for (nCol=nOldCol; nCol<=nEndCol; nCol++)
                while (((ScMergeFlagAttr*)GetAttr(nCol,rStartRow,nTab,ATTR_MERGE_FLAG))->
                            IsVerOverlapped())
                    --rStartRow;

            //!     weiterreichen ?

            ScAttrArray* pAttrArray = maTabs[nTab]->aCol[nOldCol].pAttrArray;
            SCSIZE nIndex;
            pAttrArray->Search( nOldRow, nIndex );
            SCROW nAttrPos = nOldRow;
            while (nAttrPos<=nEndRow)
            {
                OSL_ENSURE( nIndex < pAttrArray->nCount, "Wrong index in AttrArray" );

                if (((ScMergeFlagAttr&)pAttrArray->pData[nIndex].pPattern->
                        GetItem(ATTR_MERGE_FLAG)).IsHorOverlapped())
                {
                    SCROW nLoopEndRow = std::min( nEndRow, pAttrArray->pData[nIndex].nRow );
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
        OSL_FAIL("ExtendOverlapped: invalid range");
    }

    return bFound;
}


bool ScDocument::ExtendMergeSel( SCCOL nStartCol, SCROW nStartRow,
                              SCCOL& rEndCol, SCROW& rEndRow,
                              const ScMarkData& rMark, bool bRefresh )
{
    // use all selected sheets from rMark

    bool bFound = false;
    SCCOL nOldEndCol = rEndCol;
    SCROW nOldEndRow = rEndRow;

    SCTAB nMax = static_cast<SCTAB>(maTabs.size());
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nMax; ++itr)
        if ( maTabs[*itr] )
        {
            SCCOL nThisEndCol = nOldEndCol;
            SCROW nThisEndRow = nOldEndRow;
            if ( ExtendMerge( nStartCol, nStartRow, nThisEndCol, nThisEndRow, *itr, bRefresh ) )
                bFound = true;
            if ( nThisEndCol > rEndCol )
                rEndCol = nThisEndCol;
            if ( nThisEndRow > rEndRow )
                rEndRow = nThisEndRow;
        }

    return bFound;
}


bool ScDocument::ExtendMerge( SCCOL nStartCol, SCROW nStartRow,
                              SCCOL& rEndCol,  SCROW& rEndRow,
                              SCTAB nTab, bool bRefresh )
{
    bool bFound = false;
    if ( ValidColRow(nStartCol,nStartRow) && ValidColRow(rEndCol,rEndRow) && ValidTab(nTab) )
    {
        if (nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
            bFound = maTabs[nTab]->ExtendMerge( nStartCol, nStartRow, rEndCol, rEndRow, bRefresh );

        if (bRefresh)
            RefreshAutoFilter( nStartCol, nStartRow, rEndCol, rEndRow, nTab );
    }
    else
    {
        OSL_FAIL("ExtendMerge: invalid range");
    }

    return bFound;
}


bool ScDocument::ExtendMerge( ScRange& rRange, bool bRefresh )
{
    bool bFound = false;
    SCTAB nStartTab = rRange.aStart.Tab();
    SCTAB nEndTab   = rRange.aEnd.Tab();
    SCCOL nEndCol   = rRange.aEnd.Col();
    SCROW nEndRow   = rRange.aEnd.Row();

    PutInOrder( nStartTab, nEndTab );
    for (SCTAB nTab = nStartTab; nTab <= nEndTab && nTab < static_cast<SCTAB>(maTabs.size()); nTab++ )
    {
        SCCOL nExtendCol = rRange.aEnd.Col();
        SCROW nExtendRow = rRange.aEnd.Row();
        if (ExtendMerge( rRange.aStart.Col(), rRange.aStart.Row(),
                         nExtendCol,          nExtendRow,
                         nTab, bRefresh ) )
        {
            bFound = true;
            if (nExtendCol > nEndCol) nEndCol = nExtendCol;
            if (nExtendRow > nEndRow) nEndRow = nExtendRow;
        }
    }

    rRange.aEnd.SetCol(nEndCol);
    rRange.aEnd.SetRow(nEndRow);

    return bFound;
}

bool ScDocument::ExtendTotalMerge( ScRange& rRange ) const
{
    //  Bereich genau dann auf zusammengefasste Zellen erweitern, wenn
    //  dadurch keine neuen nicht-ueberdeckten Zellen getroffen werden

    bool bRet = false;
    ScRange aExt = rRange;
    // ExtendMerge() is non-const, but called withouth refresh.
    if (const_cast<ScDocument*>(this)->ExtendMerge( aExt, false))
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

bool ScDocument::ExtendOverlapped( ScRange& rRange ) const
{
    bool bFound = false;
    SCTAB nStartTab = rRange.aStart.Tab();
    SCTAB nEndTab   = rRange.aEnd.Tab();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();

    PutInOrder( nStartTab, nEndTab );
    for (SCTAB nTab = nStartTab; nTab <= nEndTab && nTab < static_cast<SCTAB>(maTabs.size()); nTab++ )
    {
        SCCOL nExtendCol = rRange.aStart.Col();
        SCROW nExtendRow = rRange.aStart.Row();
        ExtendOverlapped( nExtendCol, nExtendRow,
                                rRange.aEnd.Col(), rRange.aEnd.Row(), nTab );
        if (nExtendCol < nStartCol)
        {
            nStartCol = nExtendCol;
            bFound = true;
        }
        if (nExtendRow < nStartRow)
        {
            nStartRow = nExtendRow;
            bFound = true;
        }
    }

    rRange.aStart.SetCol(nStartCol);
    rRange.aStart.SetRow(nStartRow);

    return bFound;
}

bool ScDocument::RefreshAutoFilter( SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow, SCTAB nTab )
{
    SCTAB nDBTab;
    SCCOL nDBStartCol;
    SCROW nDBStartRow;
    SCCOL nDBEndCol;
    SCROW nDBEndRow;

    //      Autofilter loeschen

    bool bChange = RemoveFlagsTab( nStartCol,nStartRow, nEndCol,nEndRow, nTab, SC_MF_AUTO );

    //      Autofilter setzen

    const ScDBData* pData = NULL;
    ScDBCollection::NamedDBs& rDBs = pDBCollection->getNamedDBs();
    ScDBCollection::NamedDBs::const_iterator itr = rDBs.begin(), itrEnd = rDBs.end();
    for (; itr != itrEnd; ++itr)
    {
        if (itr->HasAutoFilter())
        {
            itr->GetArea( nDBTab, nDBStartCol,nDBStartRow, nDBEndCol,nDBEndRow );
            if ( nDBTab==nTab && nDBStartRow<=nEndRow && nDBEndRow>=nStartRow &&
                                    nDBStartCol<=nEndCol && nDBEndCol>=nStartCol )
            {
                if (ApplyFlagsTab( nDBStartCol,nDBStartRow, nDBEndCol,nDBStartRow,
                                    nDBTab, SC_MF_AUTO ))
                    bChange = true;
            }
        }
    }
    if (nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        pData = maTabs[nTab]->GetAnonymousDBData();
    else
        pData=NULL;
    if (pData)
    {
        if (pData->HasAutoFilter())
        {
            pData->GetArea( nDBTab, nDBStartCol,nDBStartRow, nDBEndCol,nDBEndRow );
            if ( nDBTab==nTab && nDBStartRow<=nEndRow && nDBEndRow>=nStartRow &&
                                    nDBStartCol<=nEndCol && nDBEndCol>=nStartCol )
            {
                if (ApplyFlagsTab( nDBStartCol,nDBStartRow, nDBEndCol,nDBStartRow,
                                    nDBTab, SC_MF_AUTO ))
                    bChange = true;
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

bool ScDocument::IsHorOverlapped( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
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


bool ScDocument::IsVerOverlapped( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
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
    SCTAB nMax = static_cast<SCTAB>(maTabs.size());
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nMax; ++itr)
    {
        if (maTabs[*itr])
        {
            for ( size_t j=0; j < nRangeCount; j++ )
            {
                ScRange aRange = *aRangeList[ j ];
                maTabs[*itr]->ApplyBlockFrame( pLineOuter, pLineInner,
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
    for (SCTAB nTab=nStartTab; nTab<=nEndTab && nTab < static_cast<SCTAB>(maTabs.size()); nTab++)
        if (maTabs[nTab])
            maTabs[nTab]->ApplyBlockFrame( pLineOuter, pLineInner,
                                         rRange.aStart.Col(), rRange.aStart.Row(),
                                         rRange.aEnd.Col(),   rRange.aEnd.Row() );
}


void ScDocument::ApplySelectionPattern( const ScPatternAttr& rAttr, const ScMarkData& rMark, ScEditDataArray* pDataArray )
{
    const SfxItemSet* pSet = &rAttr.GetItemSet();
    bool bSet = false;
    sal_uInt16 i;
    for (i=ATTR_PATTERN_START; i<=ATTR_PATTERN_END && !bSet; i++)
        if (pSet->GetItemState(i) == SFX_ITEM_SET)
            bSet = true;

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
            SCTAB nMax = static_cast<SCTAB>(maTabs.size());
            ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
            for (; itr != itrEnd && *itr < nMax; ++itr)
                if (maTabs[*itr])
                    maTabs[*itr]->ApplySelectionCache( &aCache, rMark, pDataArray );
        }
    }
}


void ScDocument::ChangeSelectionIndent( bool bIncrement, const ScMarkData& rMark )
{
    SCTAB nMax = static_cast<SCTAB>(maTabs.size());
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nMax; ++itr)
        if (maTabs[*itr])
            maTabs[*itr]->ChangeSelectionIndent( bIncrement, rMark );
}


void ScDocument::ClearSelectionItems( const sal_uInt16* pWhich, const ScMarkData& rMark )
{
    SCTAB nMax = static_cast<SCTAB>(maTabs.size());
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nMax; ++itr)
        if (maTabs[*itr])
            maTabs[*itr]->ClearSelectionItems( pWhich, rMark );
}


void ScDocument::DeleteSelection( sal_uInt16 nDelFlag, const ScMarkData& rMark )
{
    SCTAB nMax = static_cast<SCTAB>(maTabs.size());
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nMax; ++itr)
        if (maTabs[*itr])
            maTabs[*itr]->DeleteSelection( nDelFlag, rMark );
}


void ScDocument::DeleteSelectionTab( SCTAB nTab, sal_uInt16 nDelFlag, const ScMarkData& rMark )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->DeleteSelection( nDelFlag, rMark );
    else
    {
        OSL_FAIL("wrong table");
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
    if (ValidTab(nStartTab) && nStartTab < static_cast<SCTAB>(maTabs.size()))
    {
        if (maTabs[nStartTab])
            return maTabs[nStartTab]->GetEmptyLinesInBlock(nStartCol, nStartRow, nEndCol, nEndRow, eDir);
        else
            return 0;
    }
    else
        return 0;
}


void ScDocument::FindAreaPos( SCCOL& rCol, SCROW& rRow, SCTAB nTab, ScMoveDirection eDirection ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->FindAreaPos( rCol, rRow, eDirection );
}


void ScDocument::GetNextPos( SCCOL& rCol, SCROW& rRow, SCTAB nTab, SCsCOL nMovX, SCsROW nMovY,
                                bool bMarked, bool bUnprotected, const ScMarkData& rMark ) const
{
    OSL_ENSURE( !nMovX || !nMovY, "GetNextPos: only X or Y" );

    ScMarkData aCopyMark = rMark;
    aCopyMark.SetMarking(false);
    aCopyMark.MarkToMulti();

    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->GetNextPos( rCol, rRow, nMovX, nMovY, bMarked, bUnprotected, aCopyMark );
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

    TableContainer::const_iterator it = maTabs.begin();
    for (; it != maTabs.end(); ++it)
        if ( *it )
            nCellCount += (*it)->GetCellCount();

    return nCellCount;
}

SCSIZE ScDocument::GetCellCount(SCTAB nTab, SCCOL nCol) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return 0;

    return maTabs[nTab]->GetCellCount(nCol);
}

sal_uLong ScDocument::GetCodeCount() const
{
    sal_uLong nCodeCount = 0;

    TableContainer::const_iterator it = maTabs.begin();
    for (; it != maTabs.end(); ++it)
        if ( *it )
            nCodeCount += (*it)->GetCodeCount();

    return nCodeCount;
}


void ScDocument::PageStyleModified( SCTAB nTab, const OUString& rNewName )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->PageStyleModified( rNewName );
}


void ScDocument::SetPageStyle( SCTAB nTab, const OUString& rName )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->SetPageStyle( rName );
}


const OUString ScDocument::GetPageStyle( SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetPageStyle();

    return OUString();
}


void ScDocument::SetPageSize( SCTAB nTab, const Size& rSize )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->SetPageSize( rSize );
}

Size ScDocument::GetPageSize( SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetPageSize();

    OSL_FAIL("invalid tab");
    return Size();
}


void ScDocument::SetRepeatArea( SCTAB nTab, SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCROW nEndRow )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->SetRepeatArea( nStartCol, nEndCol, nStartRow, nEndRow );
}

void ScDocument::InvalidatePageBreaks(SCTAB nTab)
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->InvalidatePageBreaks();
}

void ScDocument::UpdatePageBreaks( SCTAB nTab, const ScRange* pUserArea )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->UpdatePageBreaks( pUserArea );
}

void ScDocument::RemoveManualBreaks( SCTAB nTab )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->RemoveManualBreaks();
}

bool ScDocument::HasManualBreaks( SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->HasManualBreaks();

    OSL_FAIL("invalid tab");
    return false;
}


void ScDocument::GetDocStat( ScDocStat& rDocStat )
{
    rDocStat.nTableCount = GetTableCount();
    rDocStat.aDocName    = aDocName;
    rDocStat.nCellCount  = GetCellCount();
}


bool ScDocument::HasPrintRange()
{
    bool bResult = false;

    TableContainer::iterator it = maTabs.begin();
    for (; it != maTabs.end() && !bResult; ++it)
        if ( *it )
            bResult = (*it)->IsPrintEntireSheet() || ((*it)->GetPrintRangeCount() > 0);

    return bResult;
}


bool ScDocument::IsPrintEntireSheet( SCTAB nTab ) const
{
    return (ValidTab(nTab) ) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] && maTabs[nTab]->IsPrintEntireSheet();
}


sal_uInt16 ScDocument::GetPrintRangeCount( SCTAB nTab )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetPrintRangeCount();

    return 0;
}


const ScRange* ScDocument::GetPrintRange( SCTAB nTab, sal_uInt16 nPos )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetPrintRange(nPos);

    return NULL;
}


const ScRange* ScDocument::GetRepeatColRange( SCTAB nTab )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetRepeatColRange();

    return NULL;
}


const ScRange* ScDocument::GetRepeatRowRange( SCTAB nTab )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetRepeatRowRange();

    return NULL;
}


void ScDocument::ClearPrintRanges( SCTAB nTab )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->ClearPrintRanges();
}


void ScDocument::AddPrintRange( SCTAB nTab, const ScRange& rNew )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->AddPrintRange( rNew );
}


void ScDocument::SetPrintEntireSheet( SCTAB nTab )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->SetPrintEntireSheet();
}


void ScDocument::SetRepeatColRange( SCTAB nTab, const ScRange* pNew )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->SetRepeatColRange( pNew );
}


void ScDocument::SetRepeatRowRange( SCTAB nTab, const ScRange* pNew )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->SetRepeatRowRange( pNew );
}


ScPrintRangeSaver* ScDocument::CreatePrintRangeSaver() const
{
    SCTAB nCount = static_cast<SCTAB>(maTabs.size());
    ScPrintRangeSaver* pNew = new ScPrintRangeSaver( nCount );
    for (SCTAB i=0; i<nCount; i++)
        if (maTabs[i])
            maTabs[i]->FillPrintSaver( pNew->GetTabData(i) );
    return pNew;
}


void ScDocument::RestorePrintRanges( const ScPrintRangeSaver& rSaver )
{
    SCTAB nCount = rSaver.GetTabCount();
    for (SCTAB i=0; i<nCount && i < static_cast<SCTAB>(maTabs.size()); i++)
        if (maTabs[i])
            maTabs[i]->RestorePrintRanges( rSaver.GetTabData(i) );
}


bool ScDocument::NeedPageResetAfterTab( SCTAB nTab ) const
{
    //  Die Seitennummern-Zaehlung faengt bei einer Tabelle neu an, wenn eine
    //  andere Vorlage als bei der vorherigen gesetzt ist (nur Namen vergleichen)
    //  und eine Seitennummer angegeben ist (nicht 0)

    if ( nTab + 1 < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] && maTabs[nTab+1] )
    {
        OUString aNew = maTabs[nTab+1]->GetPageStyle();
        if ( aNew != maTabs[nTab]->GetPageStyle() )
        {
            SfxStyleSheetBase* pStyle = xPoolHelper->GetStylePool()->Find( aNew, SFX_STYLE_FAMILY_PAGE );
            if ( pStyle )
            {
                const SfxItemSet& rSet = pStyle->GetItemSet();
                sal_uInt16 nFirst = ((const SfxUInt16Item&)rSet.Get(ATTR_PAGE_FIRSTPAGENO)).GetValue();
                if ( nFirst != 0 )
                    return true;        // Seitennummer in neuer Vorlage angegeben
            }
        }
    }

    return false;       // sonst nicht
}

SfxUndoManager* ScDocument::GetUndoManager()
{
    if (!mpUndoManager)
    {
        // to support enhanced text edit for draw objects, use an SdrUndoManager
        mpUndoManager = new SdrUndoManager;
    }

    return mpUndoManager;
}

ScRowBreakIterator* ScDocument::GetRowBreakIterator(SCTAB nTab) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return new ScRowBreakIterator(maTabs[nTab]->maRowPageBreaks);
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

void ScDocument::MarkSubTotalCells( sc::ColumnSpanSet& rSet, const ScRange& rRange, bool bVal ) const
{
    for (SCTAB nTab = rRange.aStart.Tab(); nTab <= rRange.aEnd.Tab(); ++nTab)
    {
        const ScTable* pTab = FetchTable(nTab);
        if (!pTab)
            continue;

        pTab->MarkSubTotalCells(
            rSet, rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(), rRange.aEnd.Row(), bVal);
    }
}

sal_uInt16 ScDocument::GetTextWidth( const ScAddress& rPos ) const
{
    SCTAB nTab = rPos.Tab();
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetTextWidth(rPos.Col(), rPos.Row());

    return 0;
}

sal_uInt8 ScDocument::GetScriptType( const ScAddress& rPos ) const
{
    SCTAB nTab = rPos.Tab();
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetScriptType(rPos.Col(), rPos.Row());

    return 0;
}

void ScDocument::SetScriptType( const ScAddress& rPos, sal_uInt8 nType )
{
    SCTAB nTab = rPos.Tab();
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->SetScriptType(rPos.Col(), rPos.Row(), nType);
}

void ScDocument::EnableUndo( bool bVal )
{
    // The undo manager increases lock count every time undo is disabled.
    // Because of this, we shouldn't disable undo unless it's currently
    // enabled, or else re-enabling it may not actually re-enable undo unless
    // the lock count becomes zero.

    if (bVal != GetUndoManager()->IsUndoEnabled())
    {
        GetUndoManager()->EnableUndo(bVal);
        if( pDrawLayer ) pDrawLayer->EnableUndo(bVal);
    }

    mbUndoEnabled = bVal;
}

bool ScDocument::IsUserInteractionEnabled() const
{
    return mbUserInteractionEnabled;
}

void ScDocument::EnableUserInteraction( bool bVal )
{
    mbUserInteractionEnabled = bVal;
}

bool ScDocument::IsInVBAMode() const
{
    if (!pShell)
        return false;

    try
    {
        uno::Reference<script::vba::XVBACompatibility> xVBA(
            pShell->GetBasicContainer(), uno::UNO_QUERY);

        return xVBA.is() && xVBA->getVBACompatibilityMode();
    }
    catch (const lang::NotInitializedException&) {}

    return false;
}

ScNotes* ScDocument::GetNotes(SCTAB nTab)
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        return maTabs[nTab]->GetNotes();

    return NULL;
}

void ScDocument::SetAutoNameCache(  ScAutoNameCache* pCache )
{
    delete pAutoNameCache;
    pAutoNameCache = pCache;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
