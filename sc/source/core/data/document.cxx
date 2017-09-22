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

#include <scitems.hxx>
#include <editeng/eeitem.hxx>

#include <editeng/boxitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/editobj.hxx>
#include <svx/pageitem.hxx>
#include <editeng/editeng.hxx>
#include <svx/sdrundomanager.hxx>
#include <svx/svditer.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdocapt.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
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

#include <document.hxx>
#include <table.hxx>
#include <column.hxx>
#include <attrib.hxx>
#include <attarray.hxx>
#include <markarr.hxx>
#include <patattr.hxx>
#include <rangenam.hxx>
#include <poolhelp.hxx>
#include <docpool.hxx>
#include <stlpool.hxx>
#include <stlsheet.hxx>
#include <globstr.hrc>
#include <rechead.hxx>
#include <dbdata.hxx>
#include <pivot.hxx>
#include <chartlis.hxx>
#include <rangelst.hxx>
#include <markdata.hxx>
#include <drwlayer.hxx>
#include <conditio.hxx>
#include <colorscale.hxx>
#include <validat.hxx>
#include <prnsave.hxx>
#include <chgtrack.hxx>
#include <sc.hrc>
#include <scresid.hxx>
#include <hints.hxx>
#include <detdata.hxx>
#include <dpobject.hxx>
#include <detfunc.hxx>
#include <scmod.hxx>
#include <dociter.hxx>
#include <progress.hxx>
#include <autonamecache.hxx>
#include <bcaslot.hxx>
#include <postit.hxx>
#include <externalrefmgr.hxx>
#include <tabprotection.hxx>
#include <clipparam.hxx>
#include <defaultsoptions.hxx>
#include <editutil.hxx>
#include <stringutil.hxx>
#include <formulaiter.hxx>
#include <formulacell.hxx>
#include <clipcontext.hxx>
#include <listenercontext.hxx>
#include <scopetools.hxx>
#include <refupdatecontext.hxx>
#include <formulagroup.hxx>
#include <tokenarray.hxx>
#include <tokenstringcontext.hxx>

#include <formula/vectortoken.hxx>

#include <limits>
#include <map>
#include <memory>
#include <utility>

#include <comphelper/lok.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <mtvelements.hxx>

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
    explicit ScDefaultAttr(const ScPatternAttr* pPatAttr) : pAttr(pPatAttr), nFirst(0), nCount(0) {}
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

        aString += OUString::number(nTab+1);
        if ( _bNeedsNameCheck )
            CreateValidTabName( aString );  // no doubles
        if (nTab < static_cast<SCTAB>(maTabs.size()))
        {
            maTabs[nTab] = new ScTable(this, nTab, aString);
        }
        else
        {
            while(nTab > static_cast<SCTAB>(maTabs.size()))
                maTabs.push_back(nullptr);
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

bool ScDocument::GetHashCode( SCTAB nTab, sal_Int64& rHashCode ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
    {
        if (maTabs[nTab])
        {
            rHashCode = maTabs[nTab]->GetHashCode();
            return true;
        }
    }
    return false;
}

bool ScDocument::GetName( SCTAB nTab, OUString& rName ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
    {
        if (maTabs[nTab])
        {
            maTabs[nTab]->GetName( rName );
            return true;
        }
    }
    rName.clear();
    return false;
}

OUString ScDocument::GetCopyTabName( SCTAB nTab ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabNames.size()))
        return maTabNames[nTab];
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
    SAL_WARN("sc",  "can't set code name " << rName );
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
    rName.clear();
    return false;
}

bool ScDocument::GetTable( const OUString& rName, SCTAB& rTab ) const
{
    OUString aUpperName;
    static OUString aCacheName, aCacheUpperName;

    if (aCacheName != rName)
    {
        aCacheName = rName;
        // surprisingly slow ...
        aCacheUpperName = ScGlobal::pCharClass->uppercase(rName);
    }
    aUpperName = aCacheUpperName;

    for (SCTAB i=0; i< static_cast<SCTAB>(maTabs.size()); i++)
        if (maTabs[i])
        {
            if (aUpperName == maTabs[i]->GetUpperName())
            {
                rTab = i;
                return true;
            }
        }
    rTab = 0;
    return false;
}

std::vector<OUString> ScDocument::GetAllTableNames() const
{
    std::vector<OUString> aNames;
    aNames.reserve(maTabs.size());
    TableContainer::const_iterator it = maTabs.begin(), itEnd = maTabs.end();
    for (; it != itEnd; ++it)
    {
        // Positions need to be preserved for ScCompiler and address convention
        // context, so still push an empty string for NULL tabs.
        OUString aName;
        if (*it)
        {
            const ScTable& rTab = **it;
            rTab.GetName(aName);
        }
        aNames.push_back(aName);
    }

    return aNames;
}

ScDBData* ScDocument::GetAnonymousDBData(SCTAB nTab)
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetAnonymousDBData();
    return nullptr;
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

void ScDocument::SetAnonymousDBData( ScDBData* pDBData )
{
    delete mpAnonymousDBData;
    mpAnonymousDBData = pDBData;
}

ScDBData* ScDocument::GetAnonymousDBData()
{
    return mpAnonymousDBData;
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
        const OUString& aStrTable = rOpt.GetInitTabPrefix();

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
    const OUString& aStrTable = rOpt.GetInitTabPrefix();

    OUStringBuffer rName;

    // First test if the prefix is valid, if so only avoid doubles
    bool bPrefix = ValidTabName( aStrTable );
    OSL_ENSURE(bPrefix, "Invalid Table Name");
    SCTAB nDummy;
    SCTAB i = static_cast<SCTAB>(maTabs.size())+1;

    for (SCTAB j = 0; j < nCount; ++j)
    {
        bool bOk = false;
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

bool ScDocument::InsertTab(
    SCTAB nPos, const OUString& rName, bool bExternalDocument, bool bUndoDeleteTab )
{
    SCTAB   nTabCount = static_cast<SCTAB>(maTabs.size());
    bool    bValid = ValidTab(nTabCount);
    if ( !bExternalDocument )   // else test rName == "'Doc'!Tab" first
        bValid = (bValid && ValidNewTabName(rName));
    if (bValid)
    {
        if (nPos == SC_TAB_APPEND || nPos >= nTabCount)
        {
            nPos = maTabs.size();
            maTabs.push_back( new ScTable(this, nTabCount, rName) );
            if ( bExternalDocument )
                maTabs[nTabCount]->SetVisible( false );
        }
        else
        {
            if (ValidTab(nPos) && (nPos < nTabCount))
            {
                sc::RefUpdateInsertTabContext aCxt( *this, nPos, 1);

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
                maTabs.push_back(nullptr);
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

                StartAllListeners();

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
    {
        sc::SetFormulaDirtyContext aCxt;
        aCxt.mbClearTabDeletedFlag = bUndoDeleteTab;
        aCxt.mnTabDeletedStart = nPos;
        aCxt.mnTabDeletedEnd = nPos;
        SetAllFormulasDirty(aCxt);

        if (comphelper::LibreOfficeKit::isActive() && GetDrawLayer())
        {
            SfxViewShell* pViewShell = SfxViewShell::GetFirst();
            while (pViewShell)
            {
                pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_DOCUMENT_SIZE_CHANGED, "");
                pViewShell = SfxViewShell::GetNext(*pViewShell);
            }
        }
    }

    return bValid;
}

bool ScDocument::InsertTabs( SCTAB nPos, const std::vector<OUString>& rNames,
            bool bNamesValid )
{
    SCTAB   nNewSheets = static_cast<SCTAB>(rNames.size());
    SCTAB    nTabCount = static_cast<SCTAB>(maTabs.size());
    bool    bValid = bNamesValid || ValidTab(nTabCount+nNewSheets);

    if (bValid)
    {
        if (nPos == SC_TAB_APPEND || nPos >= nTabCount)
        {
            for ( SCTAB i = 0; i < nNewSheets; ++i )
            {
                maTabs.push_back( new ScTable(this, nTabCount + i, rNames.at(i)) );
            }
        }
        else
        {
            if (ValidTab(nPos) && (nPos < nTabCount))
            {
                sc::RefUpdateInsertTabContext aCxt( *this, nPos, nNewSheets);
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
                maTabs.insert(it+nPos,nNewSheets, nullptr);
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

                StartAllListeners();

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
    {
        sc::SetFormulaDirtyContext aCxt;
        SetAllFormulasDirty(aCxt);
    }

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
                sc::RefUpdateDeleteTabContext aCxt( *this, nTab, 1);

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

                for (ScTable* pTab : maTabs)
                    if (pTab)
                        pTab->UpdateDeleteTab(aCxt);

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
                    StartAllListeners();

                    sc::SetFormulaDirtyContext aFormulaDirtyCxt;
                    SetAllFormulasDirty(aFormulaDirtyCxt);
                }
                // sheet names of references are not valid until sheet is deleted
                pChartListenerCollection->UpdateScheduledSeriesRanges();

                if (comphelper::LibreOfficeKit::isActive())
                {
                    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
                    while (pViewShell)
                    {
                        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_DOCUMENT_SIZE_CHANGED, "");
                        pViewShell = SfxViewShell::GetNext(*pViewShell);
                    }
                }

                bValid = true;
            }
        }
    }
    return bValid;
}

void ScDocument::ClearTabs()
{
    for (auto& it: maTabs)
    {
        delete it;
    }
    maTabs.clear();
}

bool ScDocument::DeleteTabs( SCTAB nTab, SCTAB nSheets )
{
    bool bValid = false;
    if (ValidTab(nTab) && (nTab + nSheets) <= static_cast<SCTAB>(maTabs.size()))
    {
        if (maTabs[nTab])
        {
            SCTAB nTabCount = static_cast<SCTAB>(maTabs.size());
            if (nTabCount > nSheets)
            {
                sc::AutoCalcSwitch aACSwitch(*this, false);
                sc::RefUpdateDeleteTabContext aCxt( *this, nTab, nSheets);

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

                for (ScTable* pTab : maTabs)
                    if (pTab)
                        pTab->UpdateDeleteTab(aCxt);

                TableContainer::iterator it = maTabs.begin() + nTab;
                TableContainer::iterator itEnd = it + nSheets;
                std::for_each(it, itEnd, std::default_delete<ScTable>());
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
                    StartAllListeners();

                    sc::SetFormulaDirtyContext aFormulaDirtyCxt;
                    SetAllFormulasDirty(aFormulaDirtyCxt);
                }
                // sheet names of references are not valid until sheet is deleted
                pChartListenerCollection->UpdateScheduledSeriesRanges();

                if (comphelper::LibreOfficeKit::isActive())
                {
                    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
                    while (pViewShell)
                    {
                        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_DOCUMENT_SIZE_CHANGED, "");
                        pViewShell = SfxViewShell::GetNext(*pViewShell);
                    }
                }

                bValid = true;
            }
        }
    }
    return bValid;
}

bool ScDocument::RenameTab( SCTAB nTab, const OUString& rName, bool bExternalDocument )
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
                    if ( *it )
                        (*it)->SetStreamValid( false );

                if (comphelper::LibreOfficeKit::isActive() && GetDrawLayer())
                {
                    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
                    while (pViewShell)
                    {
                        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_DOCUMENT_SIZE_CHANGED, "");
                        pViewShell = SfxViewShell::GetNext(*pViewShell);
                    }
                }
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

        if (mpDrawLayer)
        {
            SdrPage* pPage = mpDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
            OSL_ENSURE(pPage,"Page ?");
            if (pPage)
            {
                SdrObjListIter aIter( *pPage, SdrIterMode::DeepNoGroups );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    //  objects with ScDrawObjData are re-positioned in SetPageSize,
                    //  don't mirror again
                    ScDrawObjData* pData = ScDrawLayer::GetObjData( pObject );
                    if ( !pData )
                        mpDrawLayer->MirrorRTL( pObject );

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
        SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow, bool bColumnsOnly,
        bool bStickyTopRow, bool bStickyLeftCol, bool bConsiderCellNotes ) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB> (maTabs.size()) || !maTabs[nTab])
    {
        o_bShrunk = false;
        return false;
    }
    return maTabs[nTab]->ShrinkToUsedDataArea( o_bShrunk, rStartCol, rStartRow, rEndCol, rEndRow,
            bColumnsOnly, bStickyTopRow, bStickyLeftCol, bConsiderCellNotes );
}

SCROW ScDocument::GetLastDataRow( SCTAB nTab, SCCOL nCol1, SCCOL nCol2, SCROW nLastRow ) const
{
    const ScTable* pTab = FetchTable(nTab);
    if (!pTab)
        return -1;

    return pTab->GetLastDataRow(nCol1, nCol2, nLastRow);
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
    if (rRangeList.is())
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

struct SetDirtyIfPostponedHandler
{
    void operator() (ScTable* p)
    {
        if (p)
            p->SetDirtyIfPostponed();
    }
};

struct BroadcastRecalcOnRefMoveHandler
{
    void operator() (ScTable* p)
    {
        if (p)
            p->BroadcastRecalcOnRefMove();
    }

    explicit BroadcastRecalcOnRefMoveHandler( ScDocument* pDoc ) :
        aSwitch( *pDoc, false),
        aBulk( pDoc->GetBASM(), SfxHintId::ScDataChanged)
    {
    }

private:
    sc::AutoCalcSwitch aSwitch; // first for ctor/dtor order, destroy second
    ScBulkBroadcast aBulk;      // second for ctor/dtor order, destroy first
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
    SetAutoCalc( false );   // avoid multiple calculations
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
        ScRange aShiftedRange(nStartCol, nStartRow, nTabRangeStart, nEndCol, MAXROW, nTabRangeEnd);
        sc::EndListeningContext aEndListenCxt(*this);

        std::vector<ScAddress> aGroupPos;
        do
        {
            aShiftedRange.aStart.SetTab(nTabRangeStart);
            aShiftedRange.aEnd.SetTab(nTabRangeEnd);

            // Collect all formula groups that will get split by the shifting,
            // and end all their listening.  Record the position of the top
            // cell of the topmost group, and the position of the bottom cell
            // of the bottommost group.
            EndListeningIntersectedGroups(aEndListenCxt, aShiftedRange, &aGroupPos);

            UpdateBroadcastAreas(URM_INSDEL, aShiftedRange, 0, static_cast<SCROW>(nSize), 0);
        }
        while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) ) );

        lcl_GetFirstTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) );

        sc::RefUpdateContext aCxt(*this);
        aCxt.meMode = URM_INSDEL;
        aCxt.maRange = aShiftedRange;
        aCxt.mnRowDelta = nSize;
        do
        {
            aCxt.maRange.aStart.SetTab(nTabRangeStart);
            aCxt.maRange.aEnd.SetTab(nTabRangeEnd);
            UpdateReference(aCxt, pRefUndoDoc, false);        // without drawing objects
        }
        while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) ) );

        // UpdateReference should have set "needs listening" flags to those
        // whose references have been modified.  We also need to set this flag
        // to those that were in the groups that got split by shifting.
        SetNeedsListeningGroups(aGroupPos);

        for (i=nStartTab; i<=nEndTab && i < static_cast<SCTAB>(maTabs.size()); i++)
            if (maTabs[i] && (!pTabMark || pTabMark->GetTableSelect(i)))
                maTabs[i]->InsertRow( nStartCol, nEndCol, nStartRow, nSize );

        //  UpdateRef for drawing layer must be after inserting,
        //  when the new row heights are known.
        for (i=nStartTab; i<=nEndTab && i < static_cast<SCTAB>(maTabs.size()); i++)
            if (maTabs[i] && (!pTabMark || pTabMark->GetTableSelect(i)))
                maTabs[i]->UpdateDrawRef( URM_INSDEL,
                            nStartCol, nStartRow, nStartTab, nEndCol, MAXROW, nEndTab,
                            0, static_cast<SCROW>(nSize), 0 );

        if ( pChangeTrack && pChangeTrack->IsInDeleteUndo() )
        {   // A new Listening is needed when references to deleted ranges are restored,
            // previous Listeners were removed in FormulaCell UpdateReference.
            StartAllListeners();
        }
        else
        {   // Listeners have been removed in UpdateReference
            StartNeededListeners();

            // At least all cells using range names pointing relative to the
            // moved range must be recalculated, and all cells marked postponed
            // dirty.
            TableContainer::iterator it = maTabs.begin();
            for (; it != maTabs.end(); ++it)
                if (*it)
                    (*it)->SetDirtyIfPostponed();

            std::for_each(maTabs.begin(), maTabs.end(), BroadcastRecalcOnRefMoveHandler( this));
        }
        bRet = true;
    }
    SetAutoCalc( bOldAutoCalc );
    if ( bRet )
        pChartListenerCollection->UpdateDirtyCharts();
    return bRet;
}

bool ScDocument::InsertRow( const ScRange& rRange )
{
    return InsertRow( rRange.aStart.Col(), rRange.aStart.Tab(),
                      rRange.aEnd.Col(),   rRange.aEnd.Tab(),
                      rRange.aStart.Row(), static_cast<SCSIZE>(rRange.aEnd.Row()-rRange.aStart.Row()+1) );
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

    sc::AutoCalcSwitch aACSwitch(*this, false); // avoid multiple calculations

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
                ScAddress( nEndCol, MAXROW, nTabRangeEnd )), 0, -static_cast<SCROW>(nSize), 0 );
        }
        else
            DelBroadcastAreasInRange( ScRange(
                ScAddress( nStartCol, nStartRow, nTabRangeStart ),
                ScAddress( nEndCol, MAXROW, nTabRangeEnd ) ) );
    }
    while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) ) );

    sc::RefUpdateContext aCxt(*this);
    const bool bLastRowIncluded = (nStartRow + nSize == MAXROWCOUNT && ValidRow(nStartRow));
    if ( ValidRow(nStartRow+nSize) || bLastRowIncluded )
    {
        lcl_GetFirstTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) );
        aCxt.meMode = URM_INSDEL;
        aCxt.mnRowDelta = -static_cast<SCROW>(nSize);
        if (bLastRowIncluded)
        {
            // Last row is included, shift a virtually non-existent row in.
            aCxt.maRange = ScRange( nStartCol, MAXROWCOUNT, nTabRangeStart, nEndCol, MAXROWCOUNT, nTabRangeEnd);
        }
        else
        {
            aCxt.maRange = ScRange( nStartCol, nStartRow+nSize, nTabRangeStart, nEndCol, MAXROW, nTabRangeEnd);
        }
        do
        {
            UpdateReference(aCxt, pRefUndoDoc, true, false);
        }
        while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) ) );
    }

    if (pUndoOutline)
        *pUndoOutline = false;

    // Keep track of the positions of all formula groups that have been joined
    // during row deletion.
    std::vector<ScAddress> aGroupPos;

    for ( i = nStartTab; i <= nEndTab && i < static_cast<SCTAB>(maTabs.size()); i++)
        if (maTabs[i] && (!pTabMark || pTabMark->GetTableSelect(i)))
            maTabs[i]->DeleteRow(aCxt.maRegroupCols, nStartCol, nEndCol, nStartRow, nSize, pUndoOutline, &aGroupPos);

    // Newly joined groups have some of their members still listening.  We
    // need to make sure none of them are listening.
    EndListeningGroups(aGroupPos);

    // Mark all joined groups for group listening.
    SetNeedsListeningGroups(aGroupPos);

    if ( ValidRow(nStartRow+nSize) || bLastRowIncluded )
    {
        // Listeners have been removed in UpdateReference
        StartNeededListeners();

        // At least all cells using range names pointing relative to the moved
        // range must be recalculated, and all cells marked postponed dirty.
        TableContainer::iterator it = maTabs.begin();
        for (; it != maTabs.end(); ++it)
            if (*it)
                (*it)->SetDirtyIfPostponed();

        std::for_each(maTabs.begin(), maTabs.end(), BroadcastRecalcOnRefMoveHandler( this));
    }

    pChartListenerCollection->UpdateDirtyCharts();
}

void ScDocument::DeleteRow( const ScRange& rRange )
{
    DeleteRow( rRange.aStart.Col(), rRange.aStart.Tab(),
               rRange.aEnd.Col(),   rRange.aEnd.Tab(),
               rRange.aStart.Row(), static_cast<SCSIZE>(rRange.aEnd.Row()-rRange.aStart.Row()+1) );
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
                ScAddress( MAXCOL, nEndRow, nTabRangeEnd )), static_cast<SCCOL>(nSize), 0, 0 );
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
                maTabs[i]->InsertCol(aCxt.maRegroupCols, nStartCol, nStartRow, nEndRow, nSize);

        if ( pChangeTrack && pChangeTrack->IsInDeleteUndo() )
        {   // A new Listening is needed when references to deleted ranges are restored,
            // previous Listeners were removed in FormulaCell UpdateReference.
            StartAllListeners();
        }
        else
        {
            // Listeners have been removed in UpdateReference
            StartNeededListeners();
            // At least all cells using range names pointing relative to the
            // moved range must be recalculated, and all cells marked postponed
            // dirty.
            std::for_each(maTabs.begin(), maTabs.end(), SetDirtyIfPostponedHandler());
            // Cells containing functions such as CELL, COLUMN or ROW may have
            // changed their values on relocation. Broadcast them.
            std::for_each(maTabs.begin(), maTabs.end(), BroadcastRecalcOnRefMoveHandler( this));
        }
        bRet = true;
    }
    SetAutoCalc( bOldAutoCalc );
    if ( bRet && pChartListenerCollection )
        pChartListenerCollection->UpdateDirtyCharts();
    return bRet;
}

bool ScDocument::InsertCol( const ScRange& rRange )
{
    return InsertCol( rRange.aStart.Row(), rRange.aStart.Tab(),
                      rRange.aEnd.Row(),   rRange.aEnd.Tab(),
                      rRange.aStart.Col(), static_cast<SCSIZE>(rRange.aEnd.Col()-rRange.aStart.Col()+1) );
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
                ScAddress( MAXCOL, nEndRow, nTabRangeEnd )), -static_cast<SCCOL>(nSize), 0, 0 );
        }
        else
            DelBroadcastAreasInRange( ScRange(
                ScAddress( nStartCol, nStartRow, nTabRangeStart ),
                ScAddress( MAXCOL, nEndRow, nTabRangeEnd ) ) );
    }
    while ( lcl_GetNextTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) ) );

    sc::RefUpdateContext aCxt(*this);
    const bool bLastColIncluded = (nStartCol + nSize == MAXCOLCOUNT && ValidCol(nStartCol));
    if ( ValidCol(sal::static_int_cast<SCCOL>(nStartCol+nSize)) || bLastColIncluded )
    {
        lcl_GetFirstTabRange( nTabRangeStart, nTabRangeEnd, pTabMark, static_cast<SCTAB>(maTabs.size()) );
        aCxt.meMode = URM_INSDEL;
        aCxt.mnColDelta = -static_cast<SCCOL>(nSize);
        if (bLastColIncluded)
        {
            // Last column is included, shift a virtually non-existent column in.
            aCxt.maRange = ScRange( MAXCOLCOUNT, nStartRow, nTabRangeStart, MAXCOLCOUNT, nEndRow, nTabRangeEnd);
        }
        else
        {
            aCxt.maRange = ScRange( sal::static_int_cast<SCCOL>(nStartCol+nSize), nStartRow, nTabRangeStart,
                    MAXCOL, nEndRow, nTabRangeEnd);
        }
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

    if ( ValidCol(sal::static_int_cast<SCCOL>(nStartCol+nSize)) || bLastColIncluded )
    {
        // Listeners have been removed in UpdateReference
        StartNeededListeners();

        // At least all cells using range names pointing relative to the moved
        // range must be recalculated, and all cells marked postponed dirty.
        TableContainer::iterator it = maTabs.begin();
        for (; it != maTabs.end(); ++it)
            if (*it)
                (*it)->SetDirtyIfPostponed();

        std::for_each(maTabs.begin(), maTabs.end(), BroadcastRecalcOnRefMoveHandler( this));
    }

    pChartListenerCollection->UpdateDirtyCharts();
}

void ScDocument::DeleteCol( const ScRange& rRange )
{
    DeleteCol( rRange.aStart.Row(), rRange.aStart.Tab(),
               rRange.aEnd.Row(),   rRange.aEnd.Tab(),
               rRange.aStart.Col(), static_cast<SCSIZE>(rRange.aEnd.Col()-rRange.aStart.Col()+1) );
}

//  for Area-Links: Insert/delete cells, when the range is changed.
//  (without Paint)

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

    // if more rows, columns are inserted/deleted at the old height.
    bool bGrowY = ( nNewEndY > nOldEndY );
    SCROW nColEndY = bGrowY ? nOldEndY : nNewEndY;
    SCCOL nRowEndX = bGrowY ? nNewEndX : nOldEndX;

    // Columns

    if ( nNewEndX > nOldEndX )          // Insert columns
    {
        rColRange = ScRange( nOldEndX+1, nStartY, nTab, nNewEndX, nColEndY, nTab );
        rInsCol = true;
    }
    else if ( nNewEndX < nOldEndX )     // Delete columns
    {
        rColRange = ScRange( nNewEndX+1, nStartY, nTab, nOldEndX, nColEndY, nTab );
        rDelCol = true;
    }

    // Rows

    if ( nNewEndY > nOldEndY )          // Insert rows
    {
        rRowRange = ScRange( nStartX, nOldEndY+1, nTab, nRowEndX, nNewEndY, nTab );
        rInsRow = true;
    }
    else if ( nNewEndY < nOldEndY )     // Delete rows
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
                        HasAttrFlags::Merged | HasAttrFlags::Overlapped ))
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

formula::VectorRefArray ScDocument::FetchVectorRefArray( const ScAddress& rPos, SCROW nLength )
{
    SCTAB nTab = rPos.Tab();
    if (!TableExists(nTab))
        return formula::VectorRefArray();

    return maTabs[nTab]->FetchVectorRefArray(rPos.Col(), rPos.Row(), rPos.Row()+nLength-1);
}

void ScDocument::UnlockAdjustHeight()
{
    assert(nAdjustHeightLock > 0);
    if(nAdjustHeightLock > 0)
        --nAdjustHeightLock;
}

bool ScDocument::HandleRefArrayForParallelism( const ScAddress& rPos, SCROW nLength )
{
    SCTAB nTab = rPos.Tab();
    if (!TableExists(nTab))
        return false;

    return maTabs[nTab]->HandleRefArrayForParallelism(rPos.Col(), rPos.Row(), rPos.Row()+nLength-1);
}

bool ScDocument::CanFitBlock( const ScRange& rOld, const ScRange& rNew )
{
    if ( rOld == rNew )
        return true;

    bool bOk = true;
    bool bInsCol,bDelCol,bInsRow,bDelRow;
    ScRange aColRange,aRowRange;
    lcl_GetInsDelRanges( rOld, rNew, aColRange,bInsCol,bDelCol, aRowRange,bInsRow,bDelRow );

    if ( bInsCol && !CanInsertCol( aColRange ) )            // Cells at the edge ?
        bOk = false;
    if ( bInsRow && !CanInsertRow( aRowRange ) )            // Cells at the edge ?
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
        DeleteAreaTab( rOld, InsertDeleteFlags::ALL );

    bool bInsCol,bDelCol,bInsRow,bDelRow;
    ScRange aColRange,aRowRange;
    lcl_GetInsDelRanges( rOld, rNew, aColRange,bInsCol,bDelCol, aRowRange,bInsRow,bDelRow );

    if ( bInsCol )
        InsertCol( aColRange );         // First insert columns
    if ( bInsRow )
        InsertRow( aRowRange );

    if ( bDelRow )
        DeleteRow( aRowRange );         // First delete rows
    if ( bDelCol )
        DeleteCol( aColRange );

    // Expand references to inserted rows

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

void ScDocument::DeleteArea(
    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, const ScMarkData& rMark,
    InsertDeleteFlags nDelFlag, bool bBroadcast, sc::ColumnSpanSet* pBroadcastSpans )
{
    sc::AutoCalcSwitch aACSwitch(*this, false);

    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );

    std::vector<ScAddress> aGroupPos;
    // Destroy and reconstruct listeners only if content is affected.
    bool bDelContent = ((nDelFlag & ~InsertDeleteFlags::CONTENTS) != nDelFlag);
    if (bDelContent)
    {
        // Record the positions of top and/or bottom formula groups that intersect
        // the area borders.
        sc::EndListeningContext aCxt(*this);
        ScRange aRange(nCol1, nRow1, 0, nCol2, nRow2, 0);
        for (SCTAB i = 0; i < static_cast<SCTAB>(maTabs.size()); i++)
        {
            if (rMark.GetTableSelect(i))
            {
                aRange.aStart.SetTab(i);
                aRange.aEnd.SetTab(i);

                EndListeningIntersectedGroups(aCxt, aRange, &aGroupPos);
            }
        }
        aCxt.purgeEmptyBroadcasters();
    }

    for (SCTAB i = 0; i < static_cast<SCTAB>(maTabs.size()); i++)
        if (maTabs[i])
            if ( rMark.GetTableSelect(i) || bIsUndo )
                maTabs[i]->DeleteArea(nCol1, nRow1, nCol2, nRow2, nDelFlag, bBroadcast, pBroadcastSpans);

    if (bDelContent)
    {
        // Re-start listeners on those top bottom groups that have been split.
        SetNeedsListeningGroups(aGroupPos);
        StartNeededListeners();

        // If formula groups were split their listeners were destroyed and may
        // need to be notified now that they're restored, ScTable::DeleteArea()
        // couldn't do that.
        if (!aGroupPos.empty())
        {
            ScRange aRange(nCol1, nRow1, 0, nCol2, nRow2, 0);
            for (SCTAB i = 0; i < static_cast<SCTAB>(maTabs.size()); i++)
            {
                if (rMark.GetTableSelect(i))
                {
                    aRange.aStart.SetTab(i);
                    aRange.aEnd.SetTab(i);
                    SetDirty( aRange, true);
                }
            }
        }
    }
}

void ScDocument::DeleteAreaTab(SCCOL nCol1, SCROW nRow1,
                                SCCOL nCol2, SCROW nRow2,
                                SCTAB nTab, InsertDeleteFlags nDelFlag)
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

void ScDocument::DeleteAreaTab( const ScRange& rRange, InsertDeleteFlags nDelFlag )
{
    for ( SCTAB nTab = rRange.aStart.Tab(); nTab <= rRange.aEnd.Tab(); nTab++ )
        DeleteAreaTab( rRange.aStart.Col(), rRange.aStart.Row(),
                       rRange.aEnd.Col(),   rRange.aEnd.Row(),
                       nTab, nDelFlag );
}

void ScDocument::InitUndoSelected( const ScDocument* pSrcDoc, const ScMarkData& rTabSelection,
                                bool bColInfo, bool bRowInfo )
{
    if (bIsUndo)
    {
        Clear();

        SharePooledResources(pSrcDoc);

        for (SCTAB nTab = 0; nTab <= rTabSelection.GetLastSelected(); nTab++)
            if ( rTabSelection.GetTableSelect( nTab ) )
            {
                ScTable* pTable = new ScTable(this, nTab, OUString(), bColInfo, bRowInfo);
                if (nTab < static_cast<SCTAB>(maTabs.size()))
                    maTabs[nTab] = pTable;
                else
                    maTabs.push_back(pTable);
            }
            else
            {
                if (nTab < static_cast<SCTAB>(maTabs.size()))
                    maTabs[nTab]=nullptr;
                else
                    maTabs.push_back(nullptr);
            }
    }
    else
    {
        OSL_FAIL("InitUndo");
    }
}

void ScDocument::InitUndo( const ScDocument* pSrcDoc, SCTAB nTab1, SCTAB nTab2,
                                bool bColInfo, bool bRowInfo )
{
    if (!bIsUndo)
    {
        OSL_FAIL("InitUndo");
        return;
    }

    Clear();

    // Undo document shares its pooled resources with the source document.
    SharePooledResources(pSrcDoc);

    if (pSrcDoc->mpShell->GetMedium())
        maFileURL = pSrcDoc->mpShell->GetMedium()->GetURLObject().GetMainURL(INetURLObject::DecodeMechanism::ToIUri);

    if ( nTab2 >= static_cast<SCTAB>(maTabs.size()))
        maTabs.resize(nTab2 + 1, nullptr);
    for (SCTAB nTab = nTab1; nTab <= nTab2; nTab++)
    {
        ScTable* pTable = new ScTable(this, nTab, OUString(), bColInfo, bRowInfo);
        maTabs[nTab] = pTable;
    }
}

void ScDocument::AddUndoTab( SCTAB nTab1, SCTAB nTab2, bool bColInfo, bool bRowInfo )
{
    if (!bIsUndo)
    {
        OSL_FAIL("AddUndoTab");
        return;
    }

    if (nTab2 >= static_cast<SCTAB>(maTabs.size()))
    {
        maTabs.resize(nTab2+1,nullptr);
    }

    for (SCTAB nTab = nTab1; nTab <= nTab2; nTab++)
        if (!maTabs[nTab])
        {
            maTabs[nTab] = new ScTable(this, nTab, OUString(), bColInfo, bRowInfo);
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
                            InsertDeleteFlags nFlags, bool bOnlyMarked, ScDocument& rDestDoc,
                            const ScMarkData* pMarks, bool bColRowFlags )
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    PutInOrder( nTab1, nTab2 );
    if (rDestDoc.aDocName.isEmpty())
        rDestDoc.aDocName = aDocName;
    if (ValidTab(nTab1) && ValidTab(nTab2))
    {
        sc::CopyToDocContext aCxt(rDestDoc);
        bool bOldAutoCalc = rDestDoc.GetAutoCalc();
        rDestDoc.SetAutoCalc( false );     // avoid multiple calculations
        SCTAB nMinSizeBothTabs = static_cast<SCTAB>(std::min(maTabs.size(), rDestDoc.maTabs.size()));
        for (SCTAB i = nTab1; i <= nTab2 && i < nMinSizeBothTabs; i++)
        {
            if (maTabs[i] && rDestDoc.maTabs[i])
                maTabs[i]->CopyToTable(aCxt, nCol1, nRow1, nCol2, nRow2, nFlags,
                                      bOnlyMarked, rDestDoc.maTabs[i], pMarks,
                                      false, bColRowFlags );
        }
        rDestDoc.SetAutoCalc(bOldAutoCalc);
    }
}

void ScDocument::UndoToDocument(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                            SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                            InsertDeleteFlags nFlags, bool bOnlyMarked, ScDocument& rDestDoc)
{
    PutInOrder( nCol1, nCol2 );
    PutInOrder( nRow1, nRow2 );
    PutInOrder( nTab1, nTab2 );
    if (ValidTab(nTab1) && ValidTab(nTab2))
    {
        sc::AutoCalcSwitch aACSwitch(rDestDoc, false); // avoid multiple calculations

        if (nTab1 > 0)
            CopyToDocument(0, 0, 0, MAXCOL, MAXROW, nTab1-1, InsertDeleteFlags::FORMULA, false, rDestDoc);

        sc::CopyToDocContext aCxt(rDestDoc);
        assert( nTab2 < static_cast<SCTAB>(maTabs.size()) && nTab2 < static_cast<SCTAB>(rDestDoc.maTabs.size()));
        for (SCTAB i = nTab1; i <= nTab2; i++)
        {
            if (maTabs[i] && rDestDoc.maTabs[i])
                maTabs[i]->UndoToTable(aCxt, nCol1, nRow1, nCol2, nRow2, nFlags,
                                    bOnlyMarked, rDestDoc.maTabs[i]);
        }

        if (nTab2 < MAXTAB)
            CopyToDocument(0, 0, nTab2+1, MAXCOL, MAXROW, MAXTAB, InsertDeleteFlags::FORMULA, false, rDestDoc);
    }
}

void ScDocument::CopyToDocument(const ScRange& rRange,
                            InsertDeleteFlags nFlags, bool bOnlyMarked, ScDocument& rDestDoc,
                            const ScMarkData* pMarks, bool bColRowFlags)
{
    ScRange aNewRange = rRange;
    aNewRange.PutInOrder();

    if (rDestDoc.aDocName.isEmpty())
        rDestDoc.aDocName = aDocName;

    sc::AutoCalcSwitch aACSwitch(rDestDoc, false); // avoid multiple calculations

    sc::CopyToDocContext aCxt(rDestDoc);
    aCxt.setStartListening(false);

    SCTAB nMinSizeBothTabs = static_cast<SCTAB>(std::min(maTabs.size(), rDestDoc.maTabs.size()));
    for (SCTAB i = aNewRange.aStart.Tab(); i <= aNewRange.aEnd.Tab() && i < nMinSizeBothTabs; i++)
    {
        ScTable* pTab = FetchTable(i);
        ScTable* pDestTab = rDestDoc.FetchTable(i);
        if (!pTab || !pDestTab)
            continue;

        pTab->CopyToTable(
            aCxt, aNewRange.aStart.Col(), aNewRange.aStart.Row(), aNewRange.aEnd.Col(), aNewRange.aEnd.Row(),
            nFlags, bOnlyMarked, pDestTab, pMarks, false, bColRowFlags);
    }

    rDestDoc.StartAllListeners(aNewRange);
}

void ScDocument::UndoToDocument(const ScRange& rRange,
                            InsertDeleteFlags nFlags, bool bOnlyMarked, ScDocument& rDestDoc)
{
    sc::AutoCalcSwitch aAutoCalcSwitch(*this, false);

    ScRange aNewRange = rRange;
    aNewRange.PutInOrder();
    SCTAB nTab1 = aNewRange.aStart.Tab();
    SCTAB nTab2 = aNewRange.aEnd.Tab();

    sc::CopyToDocContext aCxt(rDestDoc);
    if (nTab1 > 0)
        CopyToDocument(0, 0, 0, MAXCOL, MAXROW, nTab1-1, InsertDeleteFlags::FORMULA, false, rDestDoc);

    SCTAB nMinSizeBothTabs = static_cast<SCTAB>(std::min(maTabs.size(), rDestDoc.maTabs.size()));
    for (SCTAB i = nTab1; i <= nTab2 && i < nMinSizeBothTabs; i++)
    {
        if (maTabs[i] && rDestDoc.maTabs[i])
            maTabs[i]->UndoToTable(aCxt, aNewRange.aStart.Col(), aNewRange.aStart.Row(),
                                    aNewRange.aEnd.Col(), aNewRange.aEnd.Row(),
                                    nFlags, bOnlyMarked, rDestDoc.maTabs[i]);
    }

    if (nTab2 < static_cast<SCTAB>(maTabs.size()))
        CopyToDocument(0, 0 , nTab2+1, MAXCOL, MAXROW, maTabs.size(), InsertDeleteFlags::FORMULA, false, rDestDoc);
}

void ScDocument::CopyToClip(const ScClipParam& rClipParam,
                            ScDocument* pClipDoc, const ScMarkData* pMarks,
                            bool bKeepScenarioFlags, bool bIncludeObjects )
{
    OSL_ENSURE( pMarks, "CopyToClip: ScMarkData fails" );

    if (bIsClip)
        return;

    if (!pClipDoc)
    {
        SAL_WARN("sc", "CopyToClip: no ClipDoc");
        pClipDoc = ScModule::GetClipDoc();
    }

    if (mpShell->GetMedium())
    {
        pClipDoc->maFileURL = mpShell->GetMedium()->GetURLObject().GetMainURL(INetURLObject::DecodeMechanism::ToIUri);
        // for unsaved files use the title name and adjust during save of file
        if (pClipDoc->maFileURL.isEmpty())
            pClipDoc->maFileURL = mpShell->GetName();
    }
    else
    {
        pClipDoc->maFileURL = mpShell->GetName();
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
            pClipDoc->maTabNames.emplace_back();
    }

    pClipDoc->aDocName = aDocName;
    pClipDoc->SetClipParam(rClipParam);
    ScRange aClipRange = rClipParam.getWholeRange();
    SCTAB nEndTab =  static_cast<SCTAB>(maTabs.size());

    pClipDoc->ResetClip(this, pMarks);

    sc::CopyToClipContext aCxt(*pClipDoc, bKeepScenarioFlags);
    CopyRangeNamesToClip(pClipDoc, aClipRange, pMarks);

    for (SCTAB i = 0; i < nEndTab; ++i)
    {
        if (!maTabs[i] || i >= static_cast<SCTAB>(pClipDoc->maTabs.size()) || !pClipDoc->maTabs[i])
            continue;

        if ( pMarks && !pMarks->GetTableSelect(i) )
            continue;

        maTabs[i]->CopyToClip(aCxt, rClipParam.maRanges, pClipDoc->maTabs[i]);

        if (mpDrawLayer && bIncludeObjects)
        {
            //  also copy drawing objects
            tools::Rectangle aObjRect = GetMMRect(
                aClipRange.aStart.Col(), aClipRange.aStart.Row(), aClipRange.aEnd.Col(), aClipRange.aEnd.Row(), i);
            mpDrawLayer->CopyToClip(pClipDoc, i, aObjRect);
        }
    }

    // Make sure to mark overlapped cells.
    pClipDoc->ExtendMerge(aClipRange, true);
}

void ScDocument::CopyStaticToDocument(const ScRange& rSrcRange, SCTAB nDestTab, ScDocument* pDestDoc)
{
    if (!pDestDoc)
        return;

    ScTable* pSrcTab = rSrcRange.aStart.Tab() < static_cast<SCTAB>(maTabs.size()) ? maTabs[rSrcRange.aStart.Tab()] : nullptr;
    ScTable* pDestTab = nDestTab < static_cast<SCTAB>(pDestDoc->maTabs.size()) ? pDestDoc->maTabs[nDestTab] : nullptr;

    if (!pSrcTab || !pDestTab)
        return;

    pDestDoc->GetFormatTable()->MergeFormatter(*GetFormatTable());
    SvNumberFormatterMergeMap aMap = pDestDoc->GetFormatTable()->ConvertMergeTableToMap();

    pSrcTab->CopyStaticToDocument(
        rSrcRange.aStart.Col(), rSrcRange.aStart.Row(), rSrcRange.aEnd.Col(), rSrcRange.aEnd.Row(),
        aMap, pDestTab);
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
        if (!pClipDoc)
        {
            SAL_WARN("sc", "CopyTabToClip: no ClipDoc");
            pClipDoc = ScModule::GetClipDoc();
        }

        if (mpShell->GetMedium())
        {
            pClipDoc->maFileURL = mpShell->GetMedium()->GetURLObject().GetMainURL(INetURLObject::DecodeMechanism::ToIUri);
            // for unsaved files use the title name and adjust during save of file
            if (pClipDoc->maFileURL.isEmpty())
                pClipDoc->maFileURL = mpShell->GetName();
        }
        else
        {
            pClipDoc->maFileURL = mpShell->GetName();
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
                pClipDoc->maTabNames.emplace_back();
        }

        PutInOrder( nCol1, nCol2 );
        PutInOrder( nRow1, nRow2 );

        ScClipParam& rClipParam = pClipDoc->GetClipParam();
        pClipDoc->aDocName = aDocName;
        rClipParam.maRanges.RemoveAll();
        rClipParam.maRanges.Append(ScRange(nCol1, nRow1, 0, nCol2, nRow2, 0));
        pClipDoc->ResetClip( this, nTab );

        sc::CopyToClipContext aCxt(*pClipDoc, false);
        if (nTab < static_cast<SCTAB>(maTabs.size()) && nTab < static_cast<SCTAB>(pClipDoc->maTabs.size()))
            if (maTabs[nTab] && pClipDoc->maTabs[nTab])
                maTabs[nTab]->CopyToClip(aCxt, nCol1, nRow1, nCol2, nRow2, pClipDoc->maTabs[nTab]);

        pClipDoc->GetClipParam().mbCutMode = false;
    }
}

void ScDocument::TransposeClip( ScDocument* pTransClip, InsertDeleteFlags nFlags, bool bAsLink )
{
    OSL_ENSURE( bIsClip && pTransClip && pTransClip->bIsClip,
                    "TransposeClip with wrong Document" );

        // initialize
        // -> pTransClip has to be deleted before the original document!

    pTransClip->ResetClip(this, nullptr);     // all

        // Take over range

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

                if ( mpDrawLayer && ( nFlags & InsertDeleteFlags::OBJECTS ) )
                {
                    //  Drawing objects are copied to the new area without transposing.
                    //  CopyFromClip is used to adjust the objects to the transposed block's
                    //  cell range area.
                    //  (mpDrawLayer in the original clipboard document is set only if there
                    //  are drawing objects to copy)

                    pTransClip->InitDrawLayer();
                    tools::Rectangle aSourceRect = GetMMRect( aClipRange.aStart.Col(), aClipRange.aStart.Row(),
                                                        aClipRange.aEnd.Col(), aClipRange.aEnd.Row(), i );
                    tools::Rectangle aDestRect = pTransClip->GetMMRect( 0, 0,
                            static_cast<SCCOL>(aClipRange.aEnd.Row() - aClipRange.aStart.Row()),
                            static_cast<SCROW>(aClipRange.aEnd.Col() - aClipRange.aStart.Col()), i );
                    pTransClip->mpDrawLayer->CopyFromClip( mpDrawLayer, i, aSourceRect, ScAddress(0,0,i), aDestRect );
                }
            }

        pTransClip->SetClipParam(GetClipParam());
        pTransClip->GetClipParam().transpose();
    }
    else
    {
        SAL_WARN("sc", "TransposeClip: Too big");
    }

    // This happens only when inserting...

    GetClipParam().mbCutMode = false;
}

namespace {

void copyUsedNamesToClip(ScRangeName* pClipRangeName, ScRangeName* pRangeName,
        const sc::UpdatedRangeNames::NameIndicesType& rUsedNames)
{
    pClipRangeName->clear();
    ScRangeName::const_iterator itr = pRangeName->begin(), itrEnd = pRangeName->end();
    for (; itr != itrEnd; ++itr)        //TODO: also DB and Pivot regions!!!
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

void ScDocument::CopyRangeNamesToClip(ScDocument* pClipDoc, const ScRange& rClipRange, const ScMarkData* pMarks)
{
    if (!pRangeName || pRangeName->empty())
        return;

    sc::UpdatedRangeNames aUsedNames;        // indexes of named ranges that are used in the copied cells
    SCTAB nMinSizeBothTabs = static_cast<SCTAB>(std::min(maTabs.size(), pClipDoc->maTabs.size()));
    for (SCTAB i = 0; i < nMinSizeBothTabs; ++i)
        if (maTabs[i] && pClipDoc->maTabs[i])
            if ( !pMarks || pMarks->GetTableSelect(i) )
                maTabs[i]->FindRangeNamesInUse(
                    rClipRange.aStart.Col(), rClipRange.aStart.Row(),
                    rClipRange.aEnd.Col(), rClipRange.aEnd.Row(), aUsedNames);

    /* TODO: handle also sheet-local names */
    sc::UpdatedRangeNames::NameIndicesType aUsedGlobalNames( aUsedNames.getUpdatedNames(-1));
    copyUsedNamesToClip(pClipDoc->GetRangeName(), pRangeName, aUsedGlobalNames);
}

ScDocument::NumFmtMergeHandler::NumFmtMergeHandler(ScDocument* pDoc, const ScDocument* pSrcDoc) :
        mpDoc(pDoc)
{
    mpDoc->MergeNumberFormatter(pSrcDoc);
}

ScDocument::NumFmtMergeHandler::~NumFmtMergeHandler()
{
    mpDoc->pFormatExchangeList = nullptr;
}

void ScDocument::PrepareFormulaCalc()
{
    ScMutationGuard aGuard(this, ScMutationGuardFlags::CORE);
    mpFormulaGroupCxt.reset();
}

SvtBroadcaster* ScDocument::GetBroadcaster( const ScAddress& rPos )
{
    ScTable* pTab = FetchTable(rPos.Tab());
    if (!pTab)
        return nullptr;

    return pTab->GetBroadcaster(rPos.Col(), rPos.Row());
}

const SvtBroadcaster* ScDocument::GetBroadcaster( const ScAddress& rPos ) const
{
    const ScTable* pTab = FetchTable(rPos.Tab());
    if (!pTab)
        return nullptr;

    return pTab->GetBroadcaster(rPos.Col(), rPos.Row());
}

void ScDocument::DeleteBroadcasters( sc::ColumnBlockPosition& rBlockPos, const ScAddress& rTopPos, SCROW nLength )
{
    ScTable* pTab = FetchTable(rTopPos.Tab());
    if (!pTab || nLength <= 0)
        return;

    pTab->DeleteBroadcasters(rBlockPos, rTopPos.Col(), rTopPos.Row(), rTopPos.Row()+nLength-1);
}

#if DUMP_COLUMN_STORAGE
void ScDocument::DumpColumnStorage( SCTAB nTab, SCCOL nCol ) const
{
    const ScTable* pTab = FetchTable(nTab);
    if (!pTab)
        return;

    pTab->DumpColumnStorage(nCol);
}
#endif

#if DEBUG_AREA_BROADCASTER
void ScDocument::DumpAreaBroadcasters() const
{
    if (pBASM)
        pBASM->Dump();
}
#endif

bool ScDocument::TableExists( SCTAB nTab ) const
{
    return ValidTab(nTab) && static_cast<size_t>(nTab) < maTabs.size() && maTabs[nTab];
}

ScTable* ScDocument::FetchTable( SCTAB nTab )
{
    if (!TableExists(nTab))
        return nullptr;

    return maTabs[nTab];
}

const ScTable* ScDocument::FetchTable( SCTAB nTab ) const
{
    if (!TableExists(nTab))
        return nullptr;

    return maTabs[nTab];
}

ScColumnsRange ScDocument::GetColumnsRange( SCTAB nTab, SCCOL nColBegin, SCCOL nColEnd) const
{
    if (!TableExists(nTab))
    {
        std::vector<ScColumn*> aEmptyVector;
        return ScColumnsRange(ScColumnsRange::Iterator(aEmptyVector.begin()),
                              ScColumnsRange::Iterator(aEmptyVector.end()));
    }

    return maTabs[nTab]->GetColumnsRange(nColBegin, nColEnd);
}

void ScDocument::MergeNumberFormatter(const ScDocument* pSrcDoc)
{
    SvNumberFormatter* pThisFormatter = mxPoolHelper->GetFormTable();
    SvNumberFormatter* pOtherFormatter = pSrcDoc->mxPoolHelper->GetFormTable();
    if (pOtherFormatter && pOtherFormatter != pThisFormatter)
    {
        SvNumberFormatterIndexTable* pExchangeList =
                 pThisFormatter->MergeFormatter(*pOtherFormatter);
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
    if (bIsClip)
        return false;

    ScDocument* pClipDoc = ScModule::GetClipDoc();
    return pClipDoc && pClipDoc->bIsClip && pClipDoc->mxPoolHelper.is() && mxPoolHelper.is() &&
            mxPoolHelper->GetDocPool() == pClipDoc->mxPoolHelper->GetDocPool();
}

void ScDocument::StartListeningFromClip( SCCOL nCol1, SCROW nRow1,
                                        SCCOL nCol2, SCROW nRow2,
                                        const ScMarkData& rMark, InsertDeleteFlags nInsFlag )
{
    if (nInsFlag & InsertDeleteFlags::CONTENTS)
    {
        std::shared_ptr<sc::ColumnBlockPositionSet> pSet(
            new sc::ColumnBlockPositionSet(*this));

        sc::StartListeningContext aStartCxt(*this, pSet);
        sc::EndListeningContext aEndCxt(*this, pSet, nullptr);

        SCTAB nMax = static_cast<SCTAB>(maTabs.size());
        ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd && *itr < nMax; ++itr)
            if (maTabs[*itr])
                maTabs[*itr]->StartListeningFormulaCells(aStartCxt, aEndCxt, nCol1, nRow1, nCol2, nRow2);
    }
}

void ScDocument::SetDirtyFromClip(
    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, const ScMarkData& rMark,
    InsertDeleteFlags nInsFlag, sc::ColumnSpanSet& rBroadcastSpans )
{
    if (nInsFlag & InsertDeleteFlags::CONTENTS)
    {
        SCTAB nMax = static_cast<SCTAB>(maTabs.size());
        ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd && *itr < nMax; ++itr)
            if (maTabs[*itr])
                maTabs[*itr]->SetDirtyFromClip(nCol1, nRow1, nCol2, nRow2, rBroadcastSpans);
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
    const ScMarkData& rMark, SCCOL nDx, SCROW nDy )
{
    TableContainer& rClipTabs = rCxt.getClipDoc()->maTabs;
    SCTAB nTabEnd = rCxt.getTabEnd();
    SCTAB nClipTab = 0;
    for (SCTAB i = rCxt.getTabStart(); i <= nTabEnd && i < static_cast<SCTAB>(maTabs.size()); i++)
    {
        if (maTabs[i] && rMark.GetTableSelect(i) )
        {
            while (!rClipTabs[nClipTab]) nClipTab = (nClipTab+1) % static_cast<SCTAB>(rClipTabs.size());

            maTabs[i]->CopyFromClip(
                rCxt, nCol1, nRow1, nCol2, nRow2, nDx, nDy, rClipTabs[nClipTab]);

            if (rCxt.getClipDoc()->mpDrawLayer && (rCxt.getInsertFlag() & InsertDeleteFlags::OBJECTS))
            {
                //  also copy drawing objects

                // drawing layer must be created before calling CopyFromClip
                // (ScDocShell::MakeDrawLayer also does InitItems etc.)
                OSL_ENSURE( mpDrawLayer, "CopyBlockFromClip: No drawing layer" );
                if ( mpDrawLayer )
                {
                    //  For GetMMRect, the row heights in the target document must already be valid
                    //  (copied in an extra step before pasting, or updated after pasting cells, but
                    //  before pasting objects).

                    tools::Rectangle aSourceRect = rCxt.getClipDoc()->GetMMRect(
                                    nCol1-nDx, nRow1-nDy, nCol2-nDx, nRow2-nDy, nClipTab );
                    tools::Rectangle aDestRect = GetMMRect( nCol1, nRow1, nCol2, nRow2, i );
                    mpDrawLayer->CopyFromClip(rCxt.getClipDoc()->mpDrawLayer, nClipTab, aSourceRect,
                                                ScAddress( nCol1, nRow1, i ), aDestRect );
                }
            }

            nClipTab = (nClipTab+1) % static_cast<SCTAB>(rClipTabs.size());
        }
    }
    if (rCxt.getInsertFlag() & InsertDeleteFlags::CONTENTS)
    {
        nClipTab = 0;
        for (SCTAB i = rCxt.getTabStart(); i <= nTabEnd && i < static_cast<SCTAB>(maTabs.size()); i++)
        {
            if (maTabs[i] && rMark.GetTableSelect(i) )
            {
                while (!rClipTabs[nClipTab]) nClipTab = (nClipTab+1) % static_cast<SCTAB>(rClipTabs.size());
                SCTAB nDz = i - nClipTab;

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
                    // Update references only if cut originates from the same
                    // document we are pasting into.
                    if (rCxt.getClipDoc()->GetPool() == GetPool())
                    {
                        bool bOldInserting = IsInsertingFromOtherDoc();
                        SetInsertingFromOtherDoc( true);
                        aRefCxt.meMode = URM_MOVE;
                        UpdateReference(aRefCxt, rCxt.getUndoDoc(), false);

                        // For URM_MOVE group listeners may have been removed,
                        // re-establish them.
                        if (!aRefCxt.maRegroupCols.empty())
                        {
                            /* TODO: holding the ColumnSet in a shared_ptr at
                             * RefUpdateContext would eliminate the need of
                             * copying it here. */
                            std::shared_ptr<const sc::ColumnSet> pColSet( new sc::ColumnSet( aRefCxt.maRegroupCols));
                            StartNeededListeners( pColSet);
                        }

                        SetInsertingFromOtherDoc( bOldInserting);
                    }
                }
                else
                {
                    aRefCxt.meMode = URM_COPY;
                    UpdateReference(aRefCxt, rCxt.getUndoDoc(), false);
                }

                nClipTab = (nClipTab+nFollow+1) % static_cast<SCTAB>(rClipTabs.size());
                i = sal::static_int_cast<SCTAB>( i + nFollow );
            }
        }
    }
}

void ScDocument::CopyNonFilteredFromClip(
    sc::CopyFromClipContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
    const ScMarkData& rMark, SCCOL nDx, SCROW & rClipStartRow )
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
            (void)rCxt.getClipDoc()->RowFiltered(nSourceRow, nFlagTab, nullptr, &nLastRow);
            SCROW nFollow = nLastRow - nSourceRow;

            if (nFollow > nSourceEnd - nSourceRow)
                nFollow = nSourceEnd - nSourceRow;
            if (nFollow > nRow2 - nDestRow)
                nFollow = nRow2 - nDestRow;

            SCROW nNewDy = nDestRow - nSourceRow;
            CopyBlockFromClip(
                rCxt, nCol1, nDestRow, nCol2, nDestRow + nFollow, rMark, nDx, nNewDy);

            nSourceRow += nFollow + 1;
            nDestRow += nFollow + 1;
        }
    }
    rClipStartRow = nSourceRow;
}

namespace {

class BroadcastAction : public sc::ColumnSpanSet::ColumnAction
{
    ScDocument& mrDoc;
    ScColumn* mpCol;

public:
    explicit BroadcastAction( ScDocument& rDoc ) : mrDoc(rDoc), mpCol(nullptr) {}

    virtual void startColumn( ScColumn* pCol ) override
    {
        mpCol = pCol;
    }

    virtual void execute( SCROW nRow1, SCROW nRow2, bool bVal ) override
    {
        if (!bVal)
            return;

        assert(mpCol);
        ScRange aRange(mpCol->GetCol(), nRow1, mpCol->GetTab());
        aRange.aEnd.SetRow(nRow2);
        mrDoc.BroadcastCells(aRange, SfxHintId::ScDataChanged);
    };
};

}

void ScDocument::CopyFromClip( const ScRange& rDestRange, const ScMarkData& rMark,
                                InsertDeleteFlags nInsFlag,
                                ScDocument* pRefUndoDoc, ScDocument* pClipDoc, bool bResetCut,
                                bool bAsLink, bool bIncludeFiltered, bool bSkipAttrForEmpty,
                                const ScRangeList * pDestRanges )
{
    if (bIsClip)
        return;

    if (!pClipDoc)
    {
        OSL_FAIL("CopyFromClip: no ClipDoc");
        pClipDoc = ScModule::GetClipDoc();
    }

    if (!pClipDoc->bIsClip || !pClipDoc->GetTableCount())
        return;

    sc::AutoCalcSwitch aACSwitch(*this, false); // temporarily turn off auto calc.

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
        together with the special flag InsertDeleteFlags::ADDNOTES that states to not
        overwrite/delete existing cells but to insert the notes into
        these cells. In this case, just delete old notes from the
        destination area. */
    InsertDeleteFlags nDelFlag = InsertDeleteFlags::NONE;
    if ( (nInsFlag & (InsertDeleteFlags::CONTENTS | InsertDeleteFlags::ADDNOTES)) == (InsertDeleteFlags::NOTE | InsertDeleteFlags::ADDNOTES) )
        nDelFlag |= InsertDeleteFlags::NOTE;
    else if ( nInsFlag & InsertDeleteFlags::CONTENTS )
        nDelFlag |= InsertDeleteFlags::CONTENTS;

    if (nInsFlag & InsertDeleteFlags::ATTRIB)
        nDelFlag |= InsertDeleteFlags::ATTRIB;

    sc::CopyFromClipContext aCxt(*this, pRefUndoDoc, pClipDoc, nInsFlag, bAsLink, bSkipAttrForEmpty);
    std::pair<SCTAB,SCTAB> aTabRanges = getMarkedTableRange(maTabs, rMark);
    aCxt.setTabRange(aTabRanges.first, aTabRanges.second);
    aCxt.setDeleteFlag(nDelFlag);

    ScRangeList aLocalRangeList;
    if (!pDestRanges)
    {
        aLocalRangeList.Append( rDestRange);
        pDestRanges = &aLocalRangeList;
    }

    bInsertingFromOtherDoc = true;  // No Broadcast/Listener created at Insert

    sc::ColumnSpanSet aBroadcastSpans(false);

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

        if (bSkipAttrForEmpty)
        {
            // Delete cells in the destination only if their corresponding clip cells are not empty.
            aCxt.setDestRange(nCol1, nRow1, nCol2, nRow2);
            DeleteBeforeCopyFromClip(aCxt, rMark, aBroadcastSpans);
        }
        else
            DeleteArea(nCol1, nRow1, nCol2, nRow2, rMark, nDelFlag, false, &aBroadcastSpans);

        if (CopyOneCellFromClip(aCxt, nCol1, nRow1, nCol2, nRow2))
            continue;

        SCCOL nC1 = nCol1;
        SCROW nR1 = nRow1;
        SCCOL nC2 = nC1 + nXw;
        if (nC2 > nCol2)
            nC2 = nCol2;
        SCROW nR2 = nR1 + nYw;
        if (nR2 > nRow2)
            nR2 = nRow2;

        const SCCOLROW nThreshold = 8192;
        bool bPreallocatePattern = ((nInsFlag & InsertDeleteFlags::ATTRIB) && (nRow2 - nRow1 > nThreshold));
        std::vector< SCTAB > vTables;

        if (bPreallocatePattern)
        {
            for (SCTAB i = aCxt.getTabStart(); i <= aCxt.getTabEnd(); ++i)
                if (maTabs[i] && rMark.GetTableSelect( i ) )
                    vTables.push_back( i );
        }

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
                SCCOL nDx = nC1 - nClipStartCol;
                SCROW nDy = nR1 - nClipStartRow;
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

            // Preallocate pattern memory once if further chunks are to be pasted.
            if (bPreallocatePattern && (nR2+1) <= nRow2)
            {
                SCROW nR3 = nR2 + 1;
                for (SCTAB nTab : vTables)
                {
                    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
                    {
                        // Pattern count of the first chunk pasted.
                        SCSIZE nChunk = GetPatternCount( nTab, nCol, nR1, nR2);
                        // If it is only one pattern per chunk and chunks are
                        // pasted consecutively then it will get its range
                        // enlarged for each chunk and no further allocation
                        // happens. For non-consecutive chunks we're out of
                        // luck in this case.
                        if (nChunk > 1)
                        {
                            SCSIZE nNeeded = nChunk * (nRow2 - nR3 + 1) / (nYw + 1);
                            SCSIZE nRemain = GetPatternCount( nTab, nCol, nR3, nRow2);
                            if (nNeeded > nRemain)
                            {
                                SCSIZE nCurr = GetPatternCount( nTab, nCol);
                                ReservePatternCount( nTab, nCol, nCurr + nNeeded);
                            }
                        }
                    }
                }
                bPreallocatePattern = false;
            }

            nR1 = nR2 + 1;
            nR2 = std::min((SCROW)(nR1 + nYw), nRow2);
        } while (nR1 <= nRow2);
    }

    bInsertingFromOtherDoc = false;

    // Create Listener after everything has been inserted
    StartListeningFromClip( nAllCol1, nAllRow1, nAllCol2, nAllRow2, rMark, nInsFlag );

    {
        ScBulkBroadcast aBulkBroadcast( GetBASM(), SfxHintId::ScDataChanged);

        // Set all formula cells dirty, and collect non-empty non-formula cell
        // positions so that we can broadcast on them below.
        SetDirtyFromClip(nAllCol1, nAllRow1, nAllCol2, nAllRow2, rMark, nInsFlag, aBroadcastSpans);

        BroadcastAction aAction(*this);
        aBroadcastSpans.executeColumnAction(*this, aAction);
    }

    if (bResetCut)
        pClipDoc->GetClipParam().mbCutMode = false;
}

void ScDocument::CopyMultiRangeFromClip(
    const ScAddress& rDestPos, const ScMarkData& rMark, InsertDeleteFlags nInsFlag, ScDocument* pClipDoc,
    bool bResetCut, bool bAsLink, bool /*bIncludeFiltered*/, bool bSkipAttrForEmpty)
{
    if (bIsClip)
        return;

    if (!pClipDoc->bIsClip || !pClipDoc->GetTableCount())
        // There is nothing in the clip doc to copy.
        return;

    // Right now, we don't allow pasting into filtered rows, so we don't even handle it here.

    sc::AutoCalcSwitch aACSwitch(*this, false); // turn of auto calc temporarily.
    NumFmtMergeHandler aNumFmtMergeHdl(this, pClipDoc);

    ScRange aDestRange;
    rMark.GetMarkArea(aDestRange);

    bInsertingFromOtherDoc = true;  // No Broadcast/Listener created at Insert

    SCCOL nCol1 = rDestPos.Col();
    SCROW nRow1 = rDestPos.Row();
    ScClipParam& rClipParam = pClipDoc->GetClipParam();

    sc::ColumnSpanSet aBroadcastSpans(false);

    if (!bSkipAttrForEmpty)
    {
        // Do the deletion first.
        SCCOL nColSize = rClipParam.getPasteColSize();
        SCROW nRowSize = rClipParam.getPasteRowSize();

        DeleteArea(nCol1, nRow1, nCol1+nColSize-1, nRow1+nRowSize-1, rMark, InsertDeleteFlags::CONTENTS, false, &aBroadcastSpans);
    }

    sc::CopyFromClipContext aCxt(*this, nullptr, pClipDoc, nInsFlag, bAsLink, bSkipAttrForEmpty);
    std::pair<SCTAB,SCTAB> aTabRanges = getMarkedTableRange(maTabs, rMark);
    aCxt.setTabRange(aTabRanges.first, aTabRanges.second);

    for (size_t i = 0, n = rClipParam.maRanges.size(); i < n; ++i)
    {
        ScRange* p = rClipParam.maRanges[i];

        SCROW nRowCount = p->aEnd.Row() - p->aStart.Row() + 1;
        SCCOL nDx = static_cast<SCCOL>(nCol1 - p->aStart.Col());
        SCROW nDy = static_cast<SCROW>(nRow1 - p->aStart.Row());
        SCCOL nCol2 = nCol1 + p->aEnd.Col() - p->aStart.Col();
        SCROW nEndRow = nRow1 + nRowCount - 1;

        CopyBlockFromClip(aCxt, nCol1, nRow1, nCol2, nEndRow, rMark, nDx, nDy);

        switch (rClipParam.meDirection)
        {
            case ScClipParam::Row:
                // Begin row for the next range being pasted.
                nRow1 += nRowCount;
            break;
            case ScClipParam::Column:
                nCol1 += p->aEnd.Col() - p->aStart.Col() + 1;
            break;
            default:
                ;
        }
    }

    bInsertingFromOtherDoc = false;

    // Create Listener after everything has been inserted
    StartListeningFromClip(aDestRange.aStart.Col(), aDestRange.aStart.Row(),
                           aDestRange.aEnd.Col(), aDestRange.aEnd.Row(), rMark, nInsFlag );

    {
        ScBulkBroadcast aBulkBroadcast( GetBASM(), SfxHintId::ScDataChanged);

        // Set formula cells dirty and collect non-formula cells.
        SetDirtyFromClip(
                aDestRange.aStart.Col(), aDestRange.aStart.Row(), aDestRange.aEnd.Col(), aDestRange.aEnd.Row(),
                rMark, nInsFlag, aBroadcastSpans);

        BroadcastAction aAction(*this);
        aBroadcastSpans.executeColumnAction(*this, aAction);
    }

    if (bResetCut)
        pClipDoc->GetClipParam().mbCutMode = false;
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

void ScDocument::MixDocument( const ScRange& rRange, ScPasteFunc nFunction, bool bSkipEmpty,
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
                                InsertDeleteFlags nFlags, ScPasteFunc nFunction,
                                bool bSkipEmpty, bool bAsLink )
{
    InsertDeleteFlags nDelFlags = nFlags;
    if (nDelFlags & InsertDeleteFlags::CONTENTS)
        nDelFlags |= InsertDeleteFlags::CONTENTS;          // Either all contents or delete nothing!

    SCTAB nSrcTab = rSrcArea.aStart.Tab();

    if (ValidTab(nSrcTab) && nSrcTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nSrcTab])
    {
        SCCOL nStartCol = rSrcArea.aStart.Col();
        SCROW nStartRow = rSrcArea.aStart.Row();
        SCCOL nEndCol = rSrcArea.aEnd.Col();
        SCROW nEndRow = rSrcArea.aEnd.Row();
        ScDocumentUniquePtr pMixDoc;
        bool bDoMix = ( bSkipEmpty || nFunction != ScPasteFunc::NONE ) && ( nFlags & InsertDeleteFlags::CONTENTS );

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
                                            InsertDeleteFlags::CONTENTS, false, pMixDoc->maTabs[i] );
                }
                maTabs[i]->DeleteArea( nStartCol,nStartRow, nEndCol,nEndRow, nDelFlags);
                maTabs[nSrcTab]->CopyToTable(aCxt, nStartCol,nStartRow, nEndCol,nEndRow,
                                                 nFlags, false, maTabs[i], nullptr, bAsLink );

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
                                InsertDeleteFlags nFlags, ScPasteFunc nFunction,
                                bool bSkipEmpty, bool bAsLink )
{
    InsertDeleteFlags nDelFlags = nFlags;
    if (nDelFlags & InsertDeleteFlags::CONTENTS)
        nDelFlags |= InsertDeleteFlags::CONTENTS;          // Either all contents or delete nothing!

    if (ValidTab(nSrcTab) && nSrcTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nSrcTab])
    {
        ScDocumentUniquePtr pMixDoc;
        bool bDoMix = ( bSkipEmpty || nFunction != ScPasteFunc::NONE ) && ( nFlags & InsertDeleteFlags::CONTENTS );

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
                                            InsertDeleteFlags::CONTENTS, true, pMixDoc->maTabs[i], &rMark );
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
    ScTable* pTab = FetchTable(nTab);
    if (!pTab)
        return false;

    bool bNumFmtSet = false;

    const ScFormulaCell* pCurCellFormula = pTab->GetFormulaCell(nCol, nRow);
    if (pCurCellFormula && pCurCellFormula->IsShared())
    {
        // In case setting this string affects an existing formula group, record
        // its above and below position for later listening.

        std::vector<ScAddress> aGroupPos;
        sc::EndListeningContext aCxt(*this);
        ScAddress aPos(nCol, nRow, nTab);
        EndListeningIntersectedGroup(aCxt, aPos, &aGroupPos);
        aCxt.purgeEmptyBroadcasters();

        bNumFmtSet = pTab->SetString(nCol, nRow, nTab, rString, pParam);

        SetNeedsListeningGroups(aGroupPos);
        StartNeededListeners();

        // Listeners may just have been setup that are affected by the current
        // position thus were not notified by a ScColumn::BroadcastNewCell()
        // during ScTable::SetString(), so do it here.
        Broadcast( ScHint( SfxHintId::ScDataChanged, aPos));
    }
    else
    {
        bNumFmtSet = pTab->SetString(nCol, nRow, nTab, rString, pParam);
    }

    return bNumFmtSet;
}

bool ScDocument::SetString(
    const ScAddress& rPos, const OUString& rString, ScSetStringParam* pParam )
{
    return SetString(rPos.Col(), rPos.Row(), rPos.Tab(), rString, pParam);
}

bool ScDocument::SetEditText( const ScAddress& rPos, EditTextObject* pEditText )
{
    if (!TableExists(rPos.Tab()))
    {
        delete pEditText;
        return false;
    }

    return maTabs[rPos.Tab()]->SetEditText(rPos.Col(), rPos.Row(), pEditText);
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

SCROW ScDocument::GetFirstEditTextRow( const ScRange& rRange ) const
{
    const ScTable* pTab = FetchTable(rRange.aStart.Tab());
    if (!pTab)
        return -1;

    return pTab->GetFirstEditTextRow(rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(), rRange.aEnd.Row());
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
    SetValue(ScAddress(nCol, nRow, nTab), rVal);
}

void ScDocument::SetValue( const ScAddress& rPos, double fVal )
{
    ScTable* pTab = FetchTable(rPos.Tab());
    if (!pTab)
        return;

    const ScFormulaCell* pCurCellFormula = pTab->GetFormulaCell(rPos.Col(), rPos.Row());
    if (pCurCellFormula && pCurCellFormula->IsShared())
    {
        // In case setting this string affects an existing formula group, record
        // its above and below position for later listening.

        std::vector<ScAddress> aGroupPos;
        sc::EndListeningContext aCxt(*this);
        EndListeningIntersectedGroup(aCxt, rPos, &aGroupPos);
        aCxt.purgeEmptyBroadcasters();

        pTab->SetValue(rPos.Col(), rPos.Row(), fVal);

        SetNeedsListeningGroups(aGroupPos);
        StartNeededListeners();

        // Listeners may just have been setup that are affected by the current
        // position thus were not notified by a ScColumn::BroadcastNewCell()
        // during ScTable::SetValue(), so do it here.
        Broadcast( ScHint( SfxHintId::ScDataChanged, rPos));
    }
    else
    {
        pTab->SetValue(rPos.Col(), rPos.Row(), fVal);
    }
}

OUString ScDocument::GetString( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    if (TableExists(nTab))
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

double* ScDocument::GetValueCell( const ScAddress& rPos )
{
    if (!TableExists(rPos.Tab()))
        return nullptr;

    return maTabs[rPos.Tab()]->GetValueCell(rPos.Col(), rPos.Row());
}

svl::SharedString ScDocument::GetSharedString( const ScAddress& rPos ) const
{
    if (!TableExists(rPos.Tab()))
        return svl::SharedString();

    return maTabs[rPos.Tab()]->GetSharedString(rPos.Col(), rPos.Row());
}

std::shared_ptr<sc::FormulaGroupContext>& ScDocument::GetFormulaGroupContext()
{
    ScMutationGuard aGuard(this, ScMutationGuardFlags::CORE);
    if (!mpFormulaGroupCxt)
        mpFormulaGroupCxt.reset(new sc::FormulaGroupContext);

    return mpFormulaGroupCxt;
}

void ScDocument::GetInputString( SCCOL nCol, SCROW nRow, SCTAB nTab, OUString& rString )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->GetInputString( nCol, nRow, rString );
    else
        rString.clear();
}

FormulaError ScDocument::GetStringForFormula( const ScAddress& rPos, OUString& rString )
{
    // Used in formulas (add-in parameters etc), so it must use the same semantics as
    // ScInterpreter::GetCellString: always format values as numbers.
    // The return value is the error code.

    ScRefCellValue aCell(*this, rPos);
    if (aCell.isEmpty())
    {
        rString = EMPTY_OUSTRING;
        return FormulaError::NONE;
    }

    FormulaError nErr = FormulaError::NONE;
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
                                    css::util::NumberFormat::NUMBER,
                                    ScGlobal::eLnge);
                pFormatter->GetInputLineString(fVal, nIndex, aStr);
            }
            else
                aStr = pFCell->GetString().getString();
        }
        break;
        case CELLTYPE_VALUE:
        {
            double fVal = aCell.mfValue;
            sal_uInt32 nIndex = pFormatter->GetStandardFormat(
                                    css::util::NumberFormat::NUMBER,
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
        return nullptr;

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
    if (!TableExists(nTab))
        return 0;

    return maTabs[nTab]->GetNumberFormat( rPos );
}

void ScDocument::SetNumberFormat( const ScAddress& rPos, sal_uInt32 nNumberFormat )
{
    SCTAB nTab = rPos.Tab();
    if (!TableExists(nTab))
        return;

    maTabs[nTab]->SetNumberFormat(rPos.Col(), rPos.Row(), nNumberFormat);
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
        nType = css::util::NumberFormat::UNDEFINED;
        nIndex = 0;
    }
}

void ScDocument::GetFormula( SCCOL nCol, SCROW nRow, SCTAB nTab, OUString& rFormula ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
            maTabs[nTab]->GetFormula( nCol, nRow, rFormula );
    else
        rFormula.clear();
}

const ScFormulaCell* ScDocument::GetFormulaCell( const ScAddress& rPos ) const
{
    if (!TableExists(rPos.Tab()))
        return nullptr;

    return maTabs[rPos.Tab()]->GetFormulaCell(rPos.Col(), rPos.Row());
}

ScFormulaCell* ScDocument::GetFormulaCell( const ScAddress& rPos )
{
    if (!TableExists(rPos.Tab()))
        return nullptr;

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
    //  true, if String- or Edit cells in range

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

void ScDocument::CheckVectorizationState()
{
    bool bOldAutoCalc = GetAutoCalc();
    bAutoCalc = false;      // no multiple calculations

    TableContainer::iterator it = maTabs.begin();
    for (; it != maTabs.end(); ++it)
        if (*it)
            (*it)->CheckVectorizationState();

    SetAutoCalc(bOldAutoCalc);
}

void ScDocument::SetAllFormulasDirty( const sc::SetFormulaDirtyContext& rCxt )
{
    bool bOldAutoCalc = GetAutoCalc();
    bAutoCalc = false;      // no multiple calculations
    {   // scope for bulk broadcast
        ScBulkBroadcast aBulkBroadcast( GetBASM(), SfxHintId::ScDataChanged);
        TableContainer::iterator it = maTabs.begin();
        for (;it != maTabs.end(); ++it)
            if (*it)
                (*it)->SetAllFormulasDirty(rCxt);
    }

    // Although Charts are also set to dirty in Tracking without AutoCalc
    // if all formulas are dirty, the charts can no longer be caught
    //  (#45205#) - that is why all Charts have to be explicitly handled again
    if (pChartListenerCollection)
        pChartListenerCollection->SetDirty();

    SetAutoCalc( bOldAutoCalc );
}

void ScDocument::SetDirty( const ScRange& rRange, bool bIncludeEmptyCells )
{
    bool bOldAutoCalc = GetAutoCalc();
    bAutoCalc = false;      // no multiple calculations
    {   // scope for bulk broadcast
        ScBulkBroadcast aBulkBroadcast( GetBASM(), SfxHintId::ScDataChanged);
        SCTAB nTab2 = rRange.aEnd.Tab();
        for (SCTAB i=rRange.aStart.Tab(); i<=nTab2 && i < static_cast<SCTAB>(maTabs.size()); i++)
            if (maTabs[i]) maTabs[i]->SetDirty( rRange,
                    (bIncludeEmptyCells ? ScColumn::BROADCAST_BROADCASTERS : ScColumn::BROADCAST_DATA_POSITIONS));

        /* TODO: this now also notifies conditional formatting and does an UNO
         * broadcast, which wasn't done here before. Is that an actually
         * desired side effect, or should we come up with a method that
         * doesn't? */
        if (bIncludeEmptyCells)
            BroadcastCells( rRange, SfxHintId::ScDataChanged, false);
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

    PrepareFormulaCalc();

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

    ScMutationGuard aGuard(this, ScMutationGuardFlags::CORE);
    mpFormulaGroupCxt.reset();
}

void ScDocument::AddTableOpFormulaCell( ScFormulaCell* pCell )
{
    if (!m_TableOpList.empty())
    {
        ScInterpreterTableOpParams *const p = m_TableOpList.back().get();
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
    PrepareFormulaCalc();
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

    // In eternal hard recalc state caches were not added as listeners,
    // invalidate them so the next non-CalcAll() normal lookup will not be
    // presented with outdated data.
    if (GetHardRecalcState() == HardRecalcState::ETERNAL)
        ClearLookupCaches();
}

void ScDocument::CompileAll()
{
    sc::CompileFormulaContext aCxt(this);
    TableContainer::iterator it = maTabs.begin();
    for (; it != maTabs.end(); ++it)
        if (*it)
            (*it)->CompileAll(aCxt);

    sc::SetFormulaDirtyContext aFormulaDirtyCxt;
    SetAllFormulasDirty(aFormulaDirtyCxt);
}

void ScDocument::CompileXML()
{
    bool bOldAutoCalc = GetAutoCalc();
    SetAutoCalc( false );
    ScProgress aProgress( GetDocumentShell(), ScGlobal::GetRscString(
                STR_PROGRESS_CALCULATING ), GetXMLImportedFormulaCount(), true );

    sc::CompileFormulaContext aCxt(this);

    // set AutoNameCache to speed up automatic name lookup
    OSL_ENSURE( !pAutoNameCache, "AutoNameCache already set" );
    pAutoNameCache = new ScAutoNameCache( this );

    if (pRangeName)
        pRangeName->CompileUnresolvedXML(aCxt);

    std::for_each(maTabs.begin(), maTabs.end(),
        [&](ScTable* pTab)
        {
            if (pTab)
                pTab->CompileXML(aCxt, aProgress);
        }
    );
    StartAllListeners();

    DELETEZ( pAutoNameCache );  // valid only during CompileXML, where cell contents don't change

    if ( pValidationList )
        pValidationList->CompileXML();

    // Track all formula cells that were appended to the FormulaTrack during
    // import or CompileXML().
    TrackFormulas();

    SetAutoCalc( bOldAutoCalc );
}

bool ScDocument::CompileErrorCells(FormulaError nErrCode)
{
    bool bCompiled = false;
    sc::CompileFormulaContext aCxt(this);
    TableContainer::iterator it = maTabs.begin(), itEnd = maTabs.end();
    for (; it != itEnd; ++it)
    {
        ScTable* pTab = *it;
        if (!pTab)
            continue;

        if (pTab->CompileErrorCells(aCxt, nErrCode))
            bCompiled = true;
    }

    return bCompiled;
}

void ScDocument::CalcAfterLoad( bool bStartListening )
{
    if (bIsClip)    // Excel data is loaded from the Clipboard to a Clip-Doc
        return;     // the calculation is then only performed when inserting into the real document

    bCalcingAfterLoad = true;
    sc::CompileFormulaContext aCxt(this);
    {
        TableContainer::iterator it = maTabs.begin();
        for (; it != maTabs.end(); ++it)
            if (*it)
                (*it)->CalcAfterLoad(aCxt, bStartListening);
        for (it = maTabs.begin(); it != maTabs.end(); ++it)
            if (*it)
                (*it)->SetDirtyAfterLoad();
    }
    bCalcingAfterLoad = false;

    SetDetectiveDirty(false);   // No real changes yet

    // #i112436# If formula cells are already dirty, they don't broadcast further changes.
    // So the source ranges of charts must be interpreted even if they are not visible,
    // similar to ScMyShapeResizer::CreateChartListener for loading own files (i104899).
    if (pChartListenerCollection)
    {
        const ScChartListenerCollection::ListenersType& rListeners = pChartListenerCollection->getListeners();
        for (auto const& it : rListeners)
        {
            const ScChartListener *const p = it.second.get();
            InterpretDirtyCells(*p->GetRangeList());
        }
    }
}

FormulaError ScDocument::GetErrCode( const ScAddress& rPos ) const
{
    SCTAB nTab = rPos.Tab();
    if ( nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetErrCode( rPos );
    return FormulaError::NONE;
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

// Column widths / Row heights   --------------------------------------

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
            ( nStartRow, nEndRow, nNewHeight, 1.0 );
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

sal_uLong ScDocument::GetColWidth( SCCOL nStartCol, SCCOL nEndCol, SCTAB nTab ) const
{
    const ScTable* pTab = FetchTable(nTab);
    if (!pTab)
        return 0;

    return pTab->GetColWidth(nStartCol, nEndCol);
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
        return maTabs[nTab]->GetRowHeight( nRow, nullptr, nullptr, bHiddenAsZero );
    OSL_FAIL("Wrong sheet number");
    return 0;
}

sal_uInt16 ScDocument::GetRowHeight( SCROW nRow, SCTAB nTab, SCROW* pStartRow, SCROW* pEndRow ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetRowHeight( nRow, pStartRow, pEndRow );
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

bool ScDocument::SetOptimalHeight( sc::RowHeightContext& rCxt, SCROW nStartRow, SCROW nEndRow, SCTAB nTab )
{
    ScTable* pTab = FetchTable(nTab);
    if (!pTab)
        return false;

    return pTab->SetOptimalHeight(rCxt, nStartRow, nEndRow);
}

void ScDocument::UpdateAllRowHeights( sc::RowHeightContext& rCxt, const ScMarkData* pTabMark )
{
    // one progress across all (selected) sheets

    sal_uLong nCellCount = 0;
    for ( SCTAB nTab=0; nTab< static_cast<SCTAB>(maTabs.size()); nTab++ )
        if ( maTabs[nTab] && ( !pTabMark || pTabMark->GetTableSelect(nTab) ) )
            nCellCount += maTabs[nTab]->GetWeightedCount();

    ScProgress aProgress( GetDocumentShell(), ScGlobal::GetRscString(STR_PROGRESS_HEIGHTING), nCellCount, true );

    sal_uLong nProgressStart = 0;
    for ( SCTAB nTab=0; nTab< static_cast<SCTAB>(maTabs.size()); nTab++ )
        if ( maTabs[nTab] && ( !pTabMark || pTabMark->GetTableSelect(nTab) ) )
        {
            maTabs[nTab]->SetOptimalHeightOnly(rCxt, 0, MAXROW, &aProgress, nProgressStart);
            maTabs[nTab]->SetDrawPageSize();
            nProgressStart += maTabs[nTab]->GetWeightedCount();
        }
}

// Column/Row - Flags   ----------------------------------------------

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

void ScDocument::SetRowFlags( SCROW nRow, SCTAB nTab, CRFlags nNewFlags )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->SetRowFlags( nRow, nNewFlags );
}

void ScDocument::SetRowFlags( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, CRFlags nNewFlags )
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        maTabs[nTab]->SetRowFlags( nStartRow, nEndRow, nNewFlags );
}

CRFlags ScDocument::GetColFlags( SCCOL nCol, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetColFlags( nCol );
    OSL_FAIL("wrong table number");
    return CRFlags::NONE;
}

CRFlags ScDocument::GetRowFlags( SCROW nRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetRowFlags( nRow );
    OSL_FAIL("wrong table number");
    return CRFlags::NONE;
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
    ScBreakType nType = ScBreakType::NONE;
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab] || !ValidRow(nRow))
        return nType;

    if (maTabs[nTab]->HasRowPageBreak(nRow))
        nType |= ScBreakType::Page;

    if (maTabs[nTab]->HasRowManualBreak(nRow))
        nType |= ScBreakType::Manual;

    return nType;
}

ScBreakType ScDocument::HasColBreak(SCCOL nCol, SCTAB nTab) const
{
    ScBreakType nType = ScBreakType::NONE;
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab] || !ValidCol(nCol))
        return nType;

    if (maTabs[nTab]->HasColPageBreak(nCol))
        nType |= ScBreakType::Page;

    if (maTabs[nTab]->HasColManualBreak(nCol))
        nType |= ScBreakType::Manual;

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
        return ::std::numeric_limits<SCROW>::max();

    return maTabs[nTab]->FirstVisibleRow(nStartRow, nEndRow);
}

SCROW ScDocument::LastVisibleRow(SCROW nStartRow, SCROW nEndRow, SCTAB nTab) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return ::std::numeric_limits<SCROW>::max();

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

bool ScDocument::ColFiltered(SCCOL nCol, SCTAB nTab) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return false;

    return maTabs[nTab]->ColFiltered(nCol);
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
        return ::std::numeric_limits<SCROW>::max();

    return maTabs[nTab]->FirstNonFilteredRow(nStartRow, nEndRow);
}

SCROW ScDocument::LastNonFilteredRow(SCROW nStartRow, SCROW nEndRow, SCTAB nTab) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return ::std::numeric_limits<SCROW>::max();

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
        CRFlags nStartFlags = maTabs[nTab]->GetColFlags(nStart);
        sal_uInt16 nStartWidth = maTabs[nTab]->GetOriginalWidth(nStart);
        for (SCCOL nCol : maTabs[nTab]->GetColumnsRange( nStart + 1, MAXCOL))
        {
            if (((nStartFlags & CRFlags::ManualBreak) != (maTabs[nTab]->GetColFlags(nCol) & CRFlags::ManualBreak)) ||
                (nStartWidth != maTabs[nTab]->GetOriginalWidth(nCol)) ||
                ((nStartFlags & CRFlags::Hidden) != (maTabs[nTab]->GetColFlags(nCol) & CRFlags::Hidden)) )
                return nCol;
        }
        return MAXCOL+1;
    }
    return 0;
}

SCROW ScDocument::GetNextDifferentChangedRow( SCTAB nTab, SCROW nStart) const
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return 0;

    const ScBitMaskCompressedArray<SCROW, CRFlags>* pRowFlagsArray = maTabs[nTab]->GetRowFlagsArray();
    if (!pRowFlagsArray)
        return 0;

    if (!maTabs[nTab]->mpRowHeights || !maTabs[nTab]->mpHiddenRows)
        return 0;

    size_t nIndex;          // ignored
    SCROW nFlagsEndRow;
    SCROW nHiddenEndRow;
    SCROW nHeightEndRow;
    CRFlags nFlags;
    bool bHidden;
    sal_uInt16 nHeight;
    CRFlags nStartFlags = nFlags = pRowFlagsArray->GetValue( nStart, nIndex, nFlagsEndRow);
    bool bStartHidden = bHidden = maTabs[nTab]->RowHidden( nStart, nullptr, &nHiddenEndRow);
    sal_uInt16 nStartHeight = nHeight = maTabs[nTab]->GetRowHeight( nStart, nullptr, &nHeightEndRow, false);
    SCROW nRow;
    while ((nRow = std::min( nHiddenEndRow, std::min( nFlagsEndRow, nHeightEndRow)) + 1) <= MAXROW)
    {
        if (nFlagsEndRow < nRow)
            nFlags = pRowFlagsArray->GetValue( nRow, nIndex, nFlagsEndRow);
        if (nHiddenEndRow < nRow)
            bHidden = maTabs[nTab]->RowHidden( nRow, nullptr, &nHiddenEndRow);
        if (nHeightEndRow < nRow)
            nHeight = maTabs[nTab]->GetRowHeight( nRow, nullptr, &nHeightEndRow, false);

        if (((nStartFlags & CRFlags::ManualBreak) != (nFlags & CRFlags::ManualBreak)) ||
            ((nStartFlags & CRFlags::ManualSize) != (nFlags & CRFlags::ManualSize)) ||
            (bStartHidden != bHidden) ||
            (nStartHeight != nHeight))
            return nRow;
    }

    return MAXROW+1;
}

void ScDocument::GetColDefault( SCTAB nTab, SCCOL nCol, SCROW nLastRow, SCROW& nDefault)
{
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
    }
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

//  Attribute   ----------------------------------------------------------

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
    return &mxPoolHelper->GetDocPool()->GetDefaultItem( nWhich );
}

const SfxPoolItem* ScDocument::GetAttr( const ScAddress& rPos, sal_uInt16 nWhich ) const
{
    return GetAttr(rPos.Col(), rPos.Row(), rPos.Tab(), nWhich);
}

const ScPatternAttr* ScDocument::GetPattern( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    if (TableExists(nTab))
        return maTabs[nTab]->GetPattern( nCol, nRow );
    return nullptr;
}

const ScPatternAttr* ScDocument::GetPattern( const ScAddress& rPos ) const
{
    if (TableExists(rPos.Tab()))
        return maTabs[rPos.Tab()]->GetPattern(rPos.Col(), rPos.Row());

    return nullptr;
}

const ScPatternAttr* ScDocument::GetMostUsedPattern( SCCOL nCol, SCROW nStartRow, SCROW nEndRow, SCTAB nTab ) const
{
    if ( ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] )
        return maTabs[nTab]->GetMostUsedPattern( nCol, nStartRow, nEndRow );
    return nullptr;
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
                        ScEditDataArray* pDataArray,
                        bool* const pIsChanged )
{
    SCTAB nMax = static_cast<SCTAB>(maTabs.size());
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nMax; ++itr)
        if (maTabs[*itr])
            maTabs[*itr]->ApplyPatternArea( nStartCol, nStartRow, nEndCol, nEndRow, rAttr, pDataArray, pIsChanged );
}

void ScDocument::ApplyPatternAreaTab( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow, SCTAB nTab, const ScPatternAttr& rAttr )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
            maTabs[nTab]->ApplyPatternArea( nStartCol, nStartRow, nEndCol, nEndRow, rAttr );
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
            maTabs[nTab]->ApplyStyle( nCol, nRow, &rStyle );
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
        return nullptr;
}

const ScStyleSheet* ScDocument::GetSelectionStyle( const ScMarkData& rMark ) const
{
    bool    bEqual = true;
    bool    bFound;

    const ScStyleSheet* pStyle = nullptr;
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
                        bEqual = false;                            // different
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
                        bEqual = false;                                // different
                    pStyle = pNewStyle;
                }
            }
    }

    return bEqual ? pStyle : nullptr;
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

bool ScDocument::IsStyleSheetUsed( const ScStyleSheet& rStyle ) const
{
    if ( bStyleSheetUsageInvalid || rStyle.GetUsage() == ScStyleSheet::UNKNOWN )
    {
        SfxStyleSheetIterator aIter( mxPoolHelper->GetStylePool(),
                    SfxStyleFamily::Para );
        for ( const SfxStyleSheetBase* pStyle = aIter.First(); pStyle;
                                       pStyle = aIter.Next() )
        {
            const ScStyleSheet* pScStyle = dynamic_cast<const ScStyleSheet*>( pStyle  );
            if ( pScStyle )
                pScStyle->SetUsage( ScStyleSheet::NOTUSED );
        }

        bool bIsUsed = false;

        TableContainer::const_iterator it = maTabs.begin();
        for (; it != maTabs.end(); ++it)
            if (*it)
            {
                if ( (*it)->IsStyleSheetUsed( rStyle ) )
                {
                    bIsUsed = true;
                }
            }

        bStyleSheetUsageInvalid = false;

        return bIsUsed;
    }

    return rStyle.GetUsage() == ScStyleSheet::USED;
}

bool ScDocument::ApplyFlagsTab( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow, SCTAB nTab, ScMF nFlags )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
            return maTabs[nTab]->ApplyFlags( nStartCol, nStartRow, nEndCol, nEndRow, nFlags );

    OSL_FAIL("ApplyFlags: wrong table");
    return false;
}

bool ScDocument::RemoveFlagsTab( SCCOL nStartCol, SCROW nStartRow,
                        SCCOL nEndCol, SCROW nEndRow, SCTAB nTab, ScMF nFlags )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
            return maTabs[nTab]->RemoveFlags( nStartCol, nStartRow, nEndCol, nEndRow, nFlags );

    OSL_FAIL("RemoveFlags: wrong table");
    return false;
}

void ScDocument::SetPattern( SCCOL nCol, SCROW nRow, SCTAB nTab, const ScPatternAttr& rAttr )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        if (maTabs[nTab])
            maTabs[nTab]->SetPattern( nCol, nRow, rAttr );
}

void ScDocument::SetPattern( const ScAddress& rPos, const ScPatternAttr& rAttr )
{
    SCTAB nTab = rPos.Tab();
    if ( nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->SetPattern( rPos, rAttr );
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
    if ( rMark.IsMarked() )                                     // single selection
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
    {
        ScPatternAttr* pPattern = new ScPatternAttr( std::move(aState.pItemSet) );
        if (aState.mbValidPatternId)
            pPattern->SetKey(aState.mnPatternId);

        return pPattern;
    }
    else
        return new ScPatternAttr( GetPool() );      // empty
}

const ScPatternAttr* ScDocument::GetSelectionPattern( const ScMarkData& rMark )
{
    delete pSelectionAttr;
    pSelectionAttr = CreateSelectionPattern( rMark );
    return pSelectionAttr;
}

void ScDocument::GetSelectionFrame( const ScMarkData& rMark,
                                    SvxBoxItem&     rLineOuter,
                                    SvxBoxInfoItem& rLineInner )
{
    rLineOuter.SetLine(nullptr, SvxBoxItemLine::TOP);
    rLineOuter.SetLine(nullptr, SvxBoxItemLine::BOTTOM);
    rLineOuter.SetLine(nullptr, SvxBoxItemLine::LEFT);
    rLineOuter.SetLine(nullptr, SvxBoxItemLine::RIGHT);
    rLineOuter.SetAllDistances(0);

    rLineInner.SetLine(nullptr, SvxBoxInfoItemLine::HORI);
    rLineInner.SetLine(nullptr, SvxBoxInfoItemLine::VERT);
    rLineInner.SetTable(true);
    rLineInner.SetDist(true);
    rLineInner.SetMinDist(false);

    ScLineFlags aFlags;

    if( rMark.IsMultiMarked() )
    {
        ScRangeList aRangeList;
        rMark.FillRangeListWithMarks( &aRangeList, false );
        size_t nRangeCount = aRangeList.size();
        bool bMultipleRows = false, bMultipleCols = false;
        for( size_t nRangeIdx = 0; nRangeIdx < nRangeCount; ++nRangeIdx )
        {
            const ScRange* pRange = aRangeList[ nRangeIdx ];
            bMultipleRows = ( bMultipleRows || ( pRange->aStart.Row() != pRange->aEnd.Row() ) );
            bMultipleCols = ( bMultipleCols || ( pRange->aStart.Col() != pRange->aEnd.Col() ) );
            SCTAB nMax = static_cast<SCTAB>(maTabs.size());
            ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
            for (; itr != itrEnd && *itr < nMax; ++itr)
                if (maTabs[*itr])
                    maTabs[*itr]->MergeBlockFrame( &rLineOuter, &rLineInner, aFlags,
                                          pRange->aStart.Col(), pRange->aStart.Row(),
                                          pRange->aEnd.Col(),   pRange->aEnd.Row() );
        }
        rLineInner.EnableHor( bMultipleRows );
        rLineInner.EnableVer( bMultipleCols );
    }
    else if( rMark.IsMarked() )
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

        // Evaluate don't care Status

    rLineInner.SetValid( SvxBoxInfoItemValidFlags::LEFT,   ( aFlags.nLeft != SC_LINE_DONTCARE ) );
    rLineInner.SetValid( SvxBoxInfoItemValidFlags::RIGHT,  ( aFlags.nRight != SC_LINE_DONTCARE ) );
    rLineInner.SetValid( SvxBoxInfoItemValidFlags::TOP,    ( aFlags.nTop != SC_LINE_DONTCARE ) );
    rLineInner.SetValid( SvxBoxInfoItemValidFlags::BOTTOM, ( aFlags.nBottom != SC_LINE_DONTCARE ) );
    rLineInner.SetValid( SvxBoxInfoItemValidFlags::HORI,   ( aFlags.nHori != SC_LINE_DONTCARE ) );
    rLineInner.SetValid( SvxBoxInfoItemValidFlags::VERT,   ( aFlags.nVert != SC_LINE_DONTCARE ) );
}

bool ScDocument::HasAttrib( SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                            SCCOL nCol2, SCROW nRow2, SCTAB nTab2, HasAttrFlags nMask ) const
{
    if ( nMask & HasAttrFlags::Rotate )
    {
        //  Is attribute used in document?
        //  (as in fillinfo)

        ScDocumentPool* pPool = mxPoolHelper->GetDocPool();

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
            nMask &= ~HasAttrFlags::Rotate;
    }

    if (nMask == HasAttrFlags::NONE)
        return false;

    bool bFound = false;
    for (SCTAB i=nTab1; i<=nTab2 && !bFound && i < static_cast<SCTAB>(maTabs.size()); i++)
        if (maTabs[i])
        {
            if ( nMask & HasAttrFlags::RightOrCenter )
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

bool ScDocument::HasAttrib( const ScRange& rRange, HasAttrFlags nMask ) const
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
    //TODO: consider page limits for printing !!!!!

    const SvxBoxItem* pThisAttr = static_cast<const SvxBoxItem*>( GetEffItem( nCol, nRow, nTab, ATTR_BORDER ) );
    OSL_ENSURE(pThisAttr,"where is the attribute?");

    const SvxBorderLine* pLeftLine   = pThisAttr->GetLeft();
    const SvxBorderLine* pTopLine    = pThisAttr->GetTop();
    const SvxBorderLine* pRightLine  = pThisAttr->GetRight();
    const SvxBorderLine* pBottomLine = pThisAttr->GetBottom();

    if ( nCol > 0 )
    {
        const SvxBorderLine* pOther = static_cast<const SvxBoxItem*>(
                                GetEffItem( nCol-1, nRow, nTab, ATTR_BORDER ))->GetRight();
        if ( ScHasPriority( pOther, pLeftLine ) )
            pLeftLine = pOther;
    }
    if ( nRow > 0 )
    {
        const SvxBorderLine* pOther = static_cast<const SvxBoxItem*>(
                                GetEffItem( nCol, nRow-1, nTab, ATTR_BORDER ))->GetBottom();
        if ( ScHasPriority( pOther, pTopLine ) )
            pTopLine = pOther;
    }
    if ( nCol < MAXCOL )
    {
        const SvxBorderLine* pOther = static_cast<const SvxBoxItem*>(
                                GetEffItem( nCol+1, nRow, nTab, ATTR_BORDER ))->GetLeft();
        if ( ScHasPriority( pOther, pRightLine ) )
            pRightLine = pOther;
    }
    if ( nRow < MAXROW )
    {
        const SvxBorderLine* pOther = static_cast<const SvxBoxItem*>(
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
    if (!bImportingXML && !mbChangeReadOnlyEnabled && mpShell && mpShell->IsReadOnly())
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
    if ( !bImportingXML && !mbChangeReadOnlyEnabled && mpShell && mpShell->IsReadOnly() )
    {
        if ( pOnlyNotBecauseOfMatrix )
            *pOnlyNotBecauseOfMatrix = false;
        return false;
    }

    ScRange aRange;
    rMark.GetMarkArea(aRange);

    bool bOk = true;
    bool bMatrix = ( pOnlyNotBecauseOfMatrix != nullptr );
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

void ScDocument::ExtendOverlapped( SCCOL& rStartCol, SCROW& rStartRow,
                                SCCOL nEndCol, SCROW nEndRow, SCTAB nTab ) const
{
    if ( ValidColRow(rStartCol,rStartRow) && ValidColRow(nEndCol,nEndRow) && ValidTab(nTab) )
    {
        if (nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        {
            SCCOL nCol;
            SCCOL nOldCol = rStartCol;
            SCROW nOldRow = rStartRow;
            for (nCol=nOldCol; nCol<=nEndCol; nCol++)
                while (static_cast<const ScMergeFlagAttr*>(GetAttr(nCol,rStartRow,nTab,ATTR_MERGE_FLAG))->
                            IsVerOverlapped())
                    --rStartRow;

            //TODO: pass on ?

            ScAttrArray* pAttrArray = maTabs[nTab]->aCol[nOldCol].pAttrArray;
            SCSIZE nIndex;
            if ( pAttrArray->Count() )
                pAttrArray->Search( nOldRow, nIndex );
            else
                nIndex = 0;
            SCROW nAttrPos = nOldRow;
            while (nAttrPos<=nEndRow)
            {
                OSL_ENSURE( nIndex < pAttrArray->Count(), "Wrong index in AttrArray" );

                bool bHorOverlapped;
                if ( pAttrArray->Count() )
                    bHorOverlapped = static_cast<const ScMergeFlagAttr&>(pAttrArray->mvData[nIndex].pPattern->
                                                                         GetItem(ATTR_MERGE_FLAG)).IsHorOverlapped();
                else
                    bHorOverlapped = static_cast<const ScMergeFlagAttr&>(GetDefPattern()->
                                                                         GetItem(ATTR_MERGE_FLAG)).IsHorOverlapped();
                if ( bHorOverlapped )
                {
                    SCROW nEndRowSeg = (pAttrArray->Count()) ? pAttrArray->mvData[nIndex].nEndRow : MAXROW;
                    SCROW nLoopEndRow = std::min( nEndRow, nEndRowSeg );
                    for (SCROW nAttrRow = nAttrPos; nAttrRow <= nLoopEndRow; nAttrRow++)
                    {
                        SCCOL nTempCol = nOldCol;
                        do
                            --nTempCol;
                        while (static_cast<const ScMergeFlagAttr*>(GetAttr(nTempCol,nAttrRow,nTab,ATTR_MERGE_FLAG))
                                ->IsHorOverlapped());
                        if (nTempCol < rStartCol)
                            rStartCol = nTempCol;
                    }
                }
                if ( pAttrArray->Count() )
                {
                    nAttrPos = pAttrArray->mvData[nIndex].nEndRow + 1;
                    ++nIndex;
                }
                else
                    nAttrPos = MAXROW + 1;
            }
        }
    }
    else
    {
        OSL_FAIL("ExtendOverlapped: invalid range");
    }
}

void ScDocument::ExtendMergeSel( SCCOL nStartCol, SCROW nStartRow,
                              SCCOL& rEndCol, SCROW& rEndRow,
                              const ScMarkData& rMark, bool bRefresh )
{
    // use all selected sheets from rMark

    SCCOL nOldEndCol = rEndCol;
    SCROW nOldEndRow = rEndRow;

    SCTAB nMax = static_cast<SCTAB>(maTabs.size());
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nMax; ++itr)
        if ( maTabs[*itr] )
        {
            SCCOL nThisEndCol = nOldEndCol;
            SCROW nThisEndRow = nOldEndRow;
            ExtendMerge( nStartCol, nStartRow, nThisEndCol, nThisEndRow, *itr, bRefresh );
            if ( nThisEndCol > rEndCol )
                rEndCol = nThisEndCol;
            if ( nThisEndRow > rEndRow )
                rEndRow = nThisEndRow;
        }
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

void ScDocument::ExtendTotalMerge( ScRange& rRange ) const
{
    // Extend range to merged cells without including any new non-overlapped cells
    ScRange aExt = rRange;
    // ExtendMerge() is non-const, but called withouth refresh.
    if (const_cast<ScDocument*>(this)->ExtendMerge( aExt ))
    {
        if ( aExt.aEnd.Row() > rRange.aEnd.Row() )
        {
            ScRange aTest = aExt;
            aTest.aStart.SetRow( rRange.aEnd.Row() + 1 );
            if ( HasAttrib( aTest, HasAttrFlags::NotOverlapped ) )
                aExt.aEnd.SetRow(rRange.aEnd.Row());
        }
        if ( aExt.aEnd.Col() > rRange.aEnd.Col() )
        {
            ScRange aTest = aExt;
            aTest.aStart.SetCol( rRange.aEnd.Col() + 1 );
            if ( HasAttrib( aTest, HasAttrFlags::NotOverlapped ) )
                aExt.aEnd.SetCol(rRange.aEnd.Col());
        }

        rRange = aExt;
    }
}

void ScDocument::ExtendOverlapped( ScRange& rRange ) const
{
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
        }
        if (nExtendRow < nStartRow)
        {
            nStartRow = nExtendRow;
        }
    }

    rRange.aStart.SetCol(nStartCol);
    rRange.aStart.SetRow(nStartRow);
}

bool ScDocument::RefreshAutoFilter( SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow, SCTAB nTab )
{
    SCTAB nDBTab;
    SCCOL nDBStartCol;
    SCROW nDBStartRow;
    SCCOL nDBEndCol;
    SCROW nDBEndRow;

    //      Delete Autofilter

    bool bChange = RemoveFlagsTab( nStartCol,nStartRow, nEndCol,nEndRow, nTab, ScMF::Auto );

    //      Set Autofilter

    const ScDBData* pData = nullptr;
    ScDBCollection::NamedDBs& rDBs = pDBCollection->getNamedDBs();
    ScDBCollection::NamedDBs::const_iterator itr = rDBs.begin(), itrEnd = rDBs.end();
    for (; itr != itrEnd; ++itr)
    {
        if ((*itr)->HasAutoFilter())
        {
            (*itr)->GetArea(nDBTab, nDBStartCol,nDBStartRow, nDBEndCol,nDBEndRow);
            if ( nDBTab==nTab && nDBStartRow<=nEndRow && nDBEndRow>=nStartRow &&
                                    nDBStartCol<=nEndCol && nDBEndCol>=nStartCol )
            {
                if (ApplyFlagsTab( nDBStartCol,nDBStartRow, nDBEndCol,nDBStartRow,
                                    nDBTab, ScMF::Auto ))
                    bChange = true;
            }
        }
    }
    if (nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        pData = maTabs[nTab]->GetAnonymousDBData();
    else
        pData=nullptr;
    if (pData)
    {
        if (pData->HasAutoFilter())
        {
            pData->GetArea( nDBTab, nDBStartCol,nDBStartRow, nDBEndCol,nDBEndRow );
            if ( nDBTab==nTab && nDBStartRow<=nEndRow && nDBEndRow>=nStartRow &&
                                    nDBStartCol<=nEndCol && nDBEndCol>=nStartCol )
            {
                if (ApplyFlagsTab( nDBStartCol,nDBStartRow, nDBEndCol,nDBStartRow,
                                    nDBTab, ScMF::Auto ))
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
    const ScMergeFlagAttr* pAttr = static_cast<const ScMergeFlagAttr*>(
                                        GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ));
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
    const ScMergeFlagAttr* pAttr = static_cast<const ScMergeFlagAttr*>(
                                        GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ));
    if (pAttr)
        return pAttr->IsVerOverlapped();
    else
    {
        OSL_FAIL("Overlapped: Attr==0");
        return false;
    }
}

void ScDocument::ApplySelectionFrame( const ScMarkData& rMark,
                                      const SvxBoxItem& rLineOuter,
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
                maTabs[*itr]->ApplyBlockFrame( rLineOuter, pLineInner,
                    aRange.aStart.Col(), aRange.aStart.Row(),
                    aRange.aEnd.Col(),   aRange.aEnd.Row() );
            }
        }
    }
    if (rLineOuter.IsRemoveAdjacentCellBorder())
    {
        SvxBoxItem aTmp0(rLineOuter);
        aTmp0.SetLine( nullptr, SvxBoxItemLine::TOP );
        aTmp0.SetLine( nullptr, SvxBoxItemLine::BOTTOM );
        aTmp0.SetLine( nullptr, SvxBoxItemLine::LEFT );
        aTmp0.SetLine( nullptr, SvxBoxItemLine::RIGHT );
        SvxBoxItem aLeft( aTmp0 );
        SvxBoxItem aRight( aTmp0 );
        SvxBoxItem aTop( aTmp0 );
        SvxBoxItem aBottom( aTmp0 );

        SvxBoxInfoItem aTmp1( *pLineInner );
        aTmp1.SetTable( false );
        aTmp1.SetLine( nullptr, SvxBoxInfoItemLine::HORI );
        aTmp1.SetLine( nullptr, SvxBoxInfoItemLine::VERT );
        aTmp1.SetValid( SvxBoxInfoItemValidFlags::ALL, false );
        aTmp1.SetValid( SvxBoxInfoItemValidFlags::DISTANCE );
        SvxBoxInfoItem aLeftInfo( aTmp1 );
        SvxBoxInfoItem aRightInfo( aTmp1 );
        SvxBoxInfoItem aTopInfo( aTmp1 );
        SvxBoxInfoItem aBottomInfo( aTmp1 );

        if (pLineInner->IsValid( SvxBoxInfoItemValidFlags::TOP ) && !rLineOuter.GetTop())
            aTopInfo.SetValid( SvxBoxInfoItemValidFlags::BOTTOM );

        if (pLineInner->IsValid( SvxBoxInfoItemValidFlags::BOTTOM ) && !rLineOuter.GetBottom())
            aBottomInfo.SetValid( SvxBoxInfoItemValidFlags::TOP );

        if (pLineInner->IsValid( SvxBoxInfoItemValidFlags::LEFT ) && !rLineOuter.GetLeft())
            aLeftInfo.SetValid( SvxBoxInfoItemValidFlags::RIGHT );

        if (pLineInner->IsValid( SvxBoxInfoItemValidFlags::RIGHT ) && !rLineOuter.GetRight())
            aRightInfo.SetValid( SvxBoxInfoItemValidFlags::LEFT );

        const ScRangeList& rRangeListTopEnvelope = rMark.GetTopEnvelope();
        const ScRangeList& rRangeListBottomEnvelope = rMark.GetBottomEnvelope();
        const ScRangeList& rRangeListLeftEnvelope = rMark.GetLeftEnvelope();
        const ScRangeList& rRangeListRightEnvelope = rMark.GetRightEnvelope();

        ScMarkData::const_iterator itr1 = rMark.begin(), itrEnd1 = rMark.end();
        for ( ; itr1 != itrEnd1 && *itr1 < nMax; ++itr1 )
        {
            if ( maTabs[*itr1] )
            {
                size_t nEnvelopeRangeCount = rRangeListTopEnvelope.size();
                for ( size_t j=0; j < nEnvelopeRangeCount; j++ )
                {
                    const ScRange* pRange = rRangeListTopEnvelope[ j ];
                    maTabs[*itr1]->ApplyBlockFrame( aTop, &aTopInfo,
                                                    pRange->aStart.Col(), pRange->aStart.Row(),
                                                    pRange->aEnd.Col(),   pRange->aEnd.Row() );
                }
                nEnvelopeRangeCount = rRangeListBottomEnvelope.size();
                for ( size_t j=0; j < nEnvelopeRangeCount; j++ )
                {
                    const ScRange* pRange = rRangeListBottomEnvelope[ j ];
                    maTabs[*itr1]->ApplyBlockFrame( aBottom, &aBottomInfo,
                                                    pRange->aStart.Col(), pRange->aStart.Row(),
                                                    pRange->aEnd.Col(),   pRange->aEnd.Row() );
                }
                nEnvelopeRangeCount = rRangeListLeftEnvelope.size();
                for ( size_t j=0; j < nEnvelopeRangeCount; j++ )
                {
                    const ScRange* pRange = rRangeListLeftEnvelope[ j ];
                    maTabs[*itr1]->ApplyBlockFrame( aLeft, &aLeftInfo,
                                                    pRange->aStart.Col(), pRange->aStart.Row(),
                                                    pRange->aEnd.Col(),   pRange->aEnd.Row() );
                }
                nEnvelopeRangeCount = rRangeListRightEnvelope.size();
                for ( size_t j=0; j < nEnvelopeRangeCount; j++ )
                {
                    const ScRange* pRange = rRangeListRightEnvelope[ j ];
                    maTabs[*itr1]->ApplyBlockFrame( aRight, &aRightInfo,
                                                    pRange->aStart.Col(), pRange->aStart.Row(),
                                                    pRange->aEnd.Col(),   pRange->aEnd.Row() );
                }
            }
        }
    }
}

void ScDocument::ApplyFrameAreaTab(const ScRange& rRange,
                                   const SvxBoxItem& rLineOuter,
                                   const SvxBoxInfoItem& rLineInner)
{
    SCTAB nStartTab = rRange.aStart.Tab();
    SCTAB nEndTab = rRange.aStart.Tab();
    for (SCTAB nTab=nStartTab; nTab<=nEndTab && nTab < static_cast<SCTAB>(maTabs.size()); nTab++)
        if (maTabs[nTab])
            maTabs[nTab]->ApplyBlockFrame(rLineOuter, &rLineInner,
                                          rRange.aStart.Col(), rRange.aStart.Row(),
                                          rRange.aEnd.Col(),   rRange.aEnd.Row());
}

void ScDocument::ApplySelectionPattern( const ScPatternAttr& rAttr, const ScMarkData& rMark, ScEditDataArray* pDataArray, bool* const pIsChanged )
{
    const SfxItemSet* pSet = &rAttr.GetItemSet();
    bool bSet = false;
    sal_uInt16 i;
    for (i=ATTR_PATTERN_START; i<=ATTR_PATTERN_END && !bSet; i++)
        if (pSet->GetItemState(i) == SfxItemState::SET)
            bSet = true;

    if (bSet)
    {
        // ApplySelectionCache needs multi mark
        if ( rMark.IsMarked() && !rMark.IsMultiMarked() )
        {
            ScRange aRange;
            rMark.GetMarkArea( aRange );
            ApplyPatternArea( aRange.aStart.Col(), aRange.aStart.Row(),
                              aRange.aEnd.Col(), aRange.aEnd.Row(), rMark, rAttr, pDataArray, pIsChanged );
        }
        else
        {
            SfxItemPoolCache aCache( mxPoolHelper->GetDocPool(), pSet );
            SCTAB nMax = static_cast<SCTAB>(maTabs.size());
            ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
            for (; itr != itrEnd && *itr < nMax; ++itr)
                if (maTabs[*itr])
                    maTabs[*itr]->ApplySelectionCache( &aCache, rMark, pDataArray, pIsChanged );
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

void ScDocument::DeleteSelection( InsertDeleteFlags nDelFlag, const ScMarkData& rMark, bool bBroadcast )
{
    sc::AutoCalcSwitch aACSwitch(*this, false);

    std::vector<ScAddress> aGroupPos;
    // Destroy and reconstruct listeners only if content is affected.
    bool bDelContent = ((nDelFlag & ~InsertDeleteFlags::CONTENTS) != nDelFlag);
    if (bDelContent)
    {
        // Record the positions of top and/or bottom formula groups that
        // intersect the area borders.
        sc::EndListeningContext aCxt(*this);
        ScRangeList aRangeList;
        rMark.FillRangeListWithMarks( &aRangeList, false);
        for (size_t i = 0; i < aRangeList.size(); ++i)
        {
            const ScRange* pRange = aRangeList[i];
            if (pRange)
                EndListeningIntersectedGroups( aCxt, *pRange, &aGroupPos);
        }
        aCxt.purgeEmptyBroadcasters();
    }

    SCTAB nMax = static_cast<SCTAB>(maTabs.size());
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nMax; ++itr)
        if (maTabs[*itr])
            maTabs[*itr]->DeleteSelection(nDelFlag, rMark, bBroadcast);

    if (bDelContent)
    {
        // Re-start listeners on those top bottom groups that have been split.
        SetNeedsListeningGroups(aGroupPos);
        StartNeededListeners();

        // If formula groups were split their listeners were destroyed and may
        // need to be notified now that they're restored,
        // ScTable::DeleteSelection() couldn't do that.
        if (!aGroupPos.empty())
        {
            ScRangeList aRangeList;
            rMark.FillRangeListWithMarks( &aRangeList, false);
            for (size_t i = 0; i < aRangeList.size(); ++i)
            {
                const ScRange* pRange = aRangeList[i];
                if (pRange)
                    SetDirty( *pRange, true);
            }
            //Notify listeners on top and bottom of the group that has been split
            for (size_t i = 0; i < aGroupPos.size(); ++i) {
                ScFormulaCell *pFormulaCell = GetFormulaCell(aGroupPos[i]);
                if (pFormulaCell)
                    pFormulaCell->SetDirty(true);
            }
        }
    }
}

void ScDocument::DeleteSelectionTab(
    SCTAB nTab, InsertDeleteFlags nDelFlag, const ScMarkData& rMark )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
    {
        sc::AutoCalcSwitch aACSwitch(*this, false);

        std::vector<ScAddress> aGroupPos;
        // Destroy and reconstruct listeners only if content is affected.
        bool bDelContent = ((nDelFlag & ~InsertDeleteFlags::CONTENTS) != nDelFlag);
        if (bDelContent)
        {
            // Record the positions of top and/or bottom formula groups that
            // intersect the area borders.
            sc::EndListeningContext aCxt(*this);
            ScRangeList aRangeList;
            rMark.FillRangeListWithMarks( &aRangeList, false);
            for (size_t i = 0; i < aRangeList.size(); ++i)
            {
                const ScRange* pRange = aRangeList[i];
                if (pRange && pRange->aStart.Tab() <= nTab && nTab <= pRange->aEnd.Tab())
                {
                    ScRange aRange( *pRange);
                    aRange.aStart.SetTab( nTab);
                    aRange.aEnd.SetTab( nTab);
                    EndListeningIntersectedGroups( aCxt, aRange, &aGroupPos);
                }
            }
            aCxt.purgeEmptyBroadcasters();
        }

        maTabs[nTab]->DeleteSelection(nDelFlag, rMark);

        if (bDelContent)
        {
            // Re-start listeners on those top bottom groups that have been split.
            SetNeedsListeningGroups(aGroupPos);
            StartNeededListeners();

            // If formula groups were split their listeners were destroyed and may
            // need to be notified now that they're restored,
            // ScTable::DeleteSelection() couldn't do that.
            if (!aGroupPos.empty())
            {
                ScRangeList aRangeList;
                rMark.FillRangeListWithMarks( &aRangeList, false);
                for (size_t i = 0; i < aRangeList.size(); ++i)
                {
                    const ScRange* pRange = aRangeList[i];
                    if (pRange && pRange->aStart.Tab() <= nTab && nTab <= pRange->aEnd.Tab())
                    {
                        ScRange aRange( *pRange);
                        aRange.aStart.SetTab( nTab);
                        aRange.aEnd.SetTab( nTab);
                        SetDirty( aRange, true);
                    }
                }
            }
        }
    }
    else
    {
        OSL_FAIL("wrong table");
    }
}

ScPatternAttr* ScDocument::GetDefPattern() const
{
    return const_cast<ScPatternAttr*>(static_cast<const ScPatternAttr*>(&mxPoolHelper->GetDocPool()->GetDefaultItem(ATTR_PATTERN)));
}

ScDocumentPool* ScDocument::GetPool()
{
    return mxPoolHelper->GetDocPool();
}

ScStyleSheetPool* ScDocument::GetStyleSheetPool() const
{
    return mxPoolHelper->GetStylePool();
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

void ScDocument::GetNextPos( SCCOL& rCol, SCROW& rRow, SCTAB nTab, SCCOL nMovX, SCROW nMovY,
                                bool bMarked, bool bUnprotected, const ScMarkData& rMark ) const
{
    OSL_ENSURE( !nMovX || !nMovY, "GetNextPos: only X or Y" );

    ScMarkData aCopyMark = rMark;
    aCopyMark.SetMarking(false);
    aCopyMark.MarkToMulti();

    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->GetNextPos( rCol, rRow, nMovX, nMovY, bMarked, bUnprotected, aCopyMark );
}

//  Data operations

void ScDocument::UpdStlShtPtrsFrmNms()
{
    ScDocumentPool* pPool = mxPoolHelper->GetDocPool();

    sal_uInt32 nCount = pPool->GetItemCount2(ATTR_PATTERN);
    for (sal_uInt32 i=0; i<nCount; i++)
    {
        ScPatternAttr* pPattern = const_cast<ScPatternAttr*>(static_cast<const ScPatternAttr*>(pPool->GetItem2(ATTR_PATTERN, i)));
        if (pPattern)
            pPattern->UpdateStyleSheet(this);
    }
    const_cast<ScPatternAttr&>(static_cast<const ScPatternAttr&>(pPool->GetDefaultItem(ATTR_PATTERN))).UpdateStyleSheet(this);
}

void ScDocument::StylesToNames()
{
    ScDocumentPool* pPool = mxPoolHelper->GetDocPool();

    sal_uInt32 nCount = pPool->GetItemCount2(ATTR_PATTERN);
    for (sal_uInt32 i=0; i<nCount; i++)
    {
        ScPatternAttr* pPattern = const_cast<ScPatternAttr*>(static_cast<const ScPatternAttr*>(pPool->GetItem2(ATTR_PATTERN, i)));
        if (pPattern)
            pPattern->StyleToName();
    }
    const_cast<ScPatternAttr&>(static_cast<const ScPatternAttr&>(pPool->GetDefaultItem(ATTR_PATTERN))).StyleToName();
}

sal_uLong ScDocument::GetCellCount() const
{
    sal_uLong nCellCount = 0;

    TableContainer::const_iterator it = maTabs.begin();
    for (; it != maTabs.end(); ++it)
        if ( *it )
            nCellCount += (*it)->GetCellCount();

    return nCellCount;
}

sal_uLong ScDocument::GetFormulaGroupCount() const
{
    sal_uLong nFormulaGroupCount = 0;

    ScFormulaGroupIterator aIter( const_cast<ScDocument*>(this) );
    for ( sc::FormulaGroupEntry* ptr = aIter.first(); ptr; ptr = aIter.next())
    {
         nFormulaGroupCount++;
    }

    return nFormulaGroupCount;
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
    rDocStat.nFormulaCount = GetFormulaGroupCount();
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

    return nullptr;
}

const ScRange* ScDocument::GetRepeatColRange( SCTAB nTab )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetRepeatColRange();

    return nullptr;
}

const ScRange* ScDocument::GetRepeatRowRange( SCTAB nTab )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetRepeatRowRange();

    return nullptr;
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
    // The page number count restarts at a sheet, if another template is set at
    // the preceding one (only compare names) and if a pagenumber is specified (not 0)

    if ( nTab + 1 < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab] && maTabs[nTab+1] )
    {
        OUString aNew = maTabs[nTab+1]->GetPageStyle();
        if ( aNew != maTabs[nTab]->GetPageStyle() )
        {
            SfxStyleSheetBase* pStyle = mxPoolHelper->GetStylePool()->Find( aNew, SfxStyleFamily::Page );
            if ( pStyle )
            {
                const SfxItemSet& rSet = pStyle->GetItemSet();
                sal_uInt16 nFirst = static_cast<const SfxUInt16Item&>(rSet.Get(ATTR_PAGE_FIRSTPAGENO)).GetValue();
                if ( nFirst != 0 )
                    return true;        // Specify page number in new template
            }
        }
    }

    return false;       // otherwise not
}

SfxUndoManager* ScDocument::GetUndoManager()
{
    if (!mpUndoManager)
    {
        // to support enhanced text edit for draw objects, use an SdrUndoManager
        ScMutationGuard aGuard(this, ScMutationGuardFlags::CORE);
        mpUndoManager = new SdrUndoManager;
    }

    return mpUndoManager;
}

ScRowBreakIterator* ScDocument::GetRowBreakIterator(SCTAB nTab) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return new ScRowBreakIterator(maTabs[nTab]->maRowPageBreaks);
    return nullptr;
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

sal_uInt16 ScDocument::GetTextWidth( const ScAddress& rPos ) const
{
    SCTAB nTab = rPos.Tab();
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetTextWidth(rPos.Col(), rPos.Row());

    return 0;
}

SvtScriptType ScDocument::GetScriptType( const ScAddress& rPos ) const
{
    SCTAB nTab = rPos.Tab();
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetScriptType(rPos.Col(), rPos.Row());

    return SvtScriptType::NONE;
}

void ScDocument::SetScriptType( const ScAddress& rPos, SvtScriptType nType )
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
        if( mpDrawLayer ) mpDrawLayer->EnableUndo(bVal);
    }

    mbUndoEnabled = bVal;
}

void ScDocument::EnableUserInteraction( bool bVal )
{
    mbUserInteractionEnabled = bVal;
}

bool ScDocument::IsInVBAMode() const
{
    if (!mpShell)
        return false;

    try
    {
        uno::Reference<script::vba::XVBACompatibility> xVBA(
            mpShell->GetBasicContainer(), uno::UNO_QUERY);

        return xVBA.is() && xVBA->getVBACompatibilityMode();
    }
    catch (const lang::NotInitializedException&) {}

    return false;
}

ScPostIt* ScDocument::GetNote(const ScAddress& rPos)
{
    return GetNote(rPos.Col(), rPos.Row(), rPos.Tab());
}

ScPostIt* ScDocument::GetNote(SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()))
        return maTabs[nTab]->aCol[nCol].GetCellNote(nRow);
    else
        return nullptr;

}

void ScDocument::SetNote(const ScAddress& rPos, ScPostIt* pNote)
{
    return SetNote(rPos.Col(), rPos.Row(), rPos.Tab(), pNote);
}

void ScDocument::SetNote(SCCOL nCol, SCROW nRow, SCTAB nTab, ScPostIt* pNote)
{
    return maTabs[nTab]->aCol[nCol].SetCellNote(nRow, pNote);
}

bool ScDocument::HasNote(const ScAddress& rPos) const
{
    return HasNote(rPos.Col(), rPos.Row(), rPos.Tab());
}

bool ScDocument::HasNote(SCCOL nCol, SCROW nRow, SCTAB nTab) const
{
    if (!ValidColRow(nCol, nRow))
        return false;

    const ScTable* pTab = FetchTable(nTab);
    if (!pTab)
        return false;

    const ScPostIt* pNote = pTab->aCol[nCol].GetCellNote(nRow);
    return pNote != nullptr;
}

bool ScDocument::HasColNotes(SCCOL nCol, SCTAB nTab) const
{
    if (!ValidCol(nCol))
        return false;

    const ScTable* pTab = FetchTable(nTab);
    if (!pTab)
        return false;

    return pTab->aCol[nCol].HasCellNotes();
}

bool ScDocument::HasTabNotes(SCTAB nTab) const
{
    const ScTable* pTab = FetchTable(nTab);

    if ( !pTab )
        return false;

    for (SCCOL nCol=0, nColSize = pTab->aCol.size(); nCol < nColSize; ++nCol)
        if ( HasColNotes(nCol, nTab) )
            return true;

    return false;
}

bool ScDocument::HasNotes() const
{
    for (SCTAB i = 0; i <= MAXTAB; ++i)
    {
        if (HasTabNotes(i))
            return true;
    }
    return false;
}

ScPostIt* ScDocument::ReleaseNote(const ScAddress& rPos)
{
    ScTable* pTab = FetchTable(rPos.Tab());
    if (!pTab)
        return nullptr;

    return pTab->ReleaseNote(rPos.Col(), rPos.Row());
}

ScPostIt* ScDocument::GetOrCreateNote(const ScAddress& rPos)
{
    if (HasNote(rPos))
        return GetNote(rPos);
    else
        return CreateNote(rPos);
}
ScPostIt* ScDocument::CreateNote(const ScAddress& rPos)
{
    ScPostIt* pPostIt = new ScPostIt(*this, rPos);
    SetNote(rPos, pPostIt);
    return pPostIt;
}

size_t ScDocument::GetNoteCount( SCTAB nTab, SCCOL nCol ) const
{
    const ScTable* pTab = FetchTable(nTab);
    if (!pTab)
        return 0;

    return pTab->GetNoteCount(nCol);
}

void ScDocument::CreateAllNoteCaptions()
{
    TableContainer::iterator it = maTabs.begin(), itEnd = maTabs.end();
    for (; it != itEnd; ++it)
    {
        ScTable* p = *it;
        if (p)
            p->CreateAllNoteCaptions();
    }
}

void ScDocument::ForgetNoteCaptions( const ScRangeList& rRanges, bool bPreserveData )
{
    for (size_t i = 0, n = rRanges.size(); i < n; ++i)
    {
        const ScRange* p = rRanges[i];
        const ScAddress& s = p->aStart;
        const ScAddress& e = p->aEnd;
        for (SCTAB nTab = s.Tab(); nTab <= e.Tab(); ++nTab)
        {
            ScTable* pTab = FetchTable(nTab);
            if (!pTab)
                continue;

            pTab->ForgetNoteCaptions(s.Col(), s.Row(), e.Col(), e.Row(), bPreserveData);
        }
    }
}

CommentCaptionState ScDocument::GetAllNoteCaptionsState( const ScRangeList& rRanges )
{
    CommentCaptionState aTmpState = CommentCaptionState::ALLHIDDEN;
    CommentCaptionState aState = CommentCaptionState::ALLHIDDEN;
    bool bFirstControl = true;
    std::vector<sc::NoteEntry> aNotes;

    for (size_t i = 0, n = rRanges.size(); i < n; ++i)
    {
        const ScRange* pRange = rRanges[i];

        for( SCTAB nTab = pRange->aStart.Tab(); nTab <= pRange->aEnd.Tab(); ++nTab )
        {
            aState = maTabs[nTab]->GetAllNoteCaptionsState( *pRange, aNotes );

            if (aState == CommentCaptionState::MIXED)
                return aState;

            if (bFirstControl)                      // it is possible that a range is ALLSHOWN, another range is ALLHIDDEN,
            {                                       // we have to detect that situation as mixed.
                aTmpState = aState;
                bFirstControl = false;
            }
            else if(aTmpState != aState)
            {
                aState = CommentCaptionState::MIXED;
                return aState;
            }
        }
    }
    return aState;
}

ScAddress ScDocument::GetNotePosition( size_t nIndex ) const
{
    for (size_t nTab = 0; nTab < maTabs.size(); ++nTab)
    {
        for (SCCOL nCol : GetColumnsRange(nTab, 0, MAXCOL))
        {
            size_t nColNoteCount = GetNoteCount(nTab, nCol);
            if (!nColNoteCount)
                continue;

            if (nIndex >= nColNoteCount)
            {
                nIndex -= nColNoteCount;
                continue;
            }

            SCROW nRow = GetNotePosition(nTab, nCol, nIndex);
            if (nRow >= 0)
                return ScAddress(nCol, nRow, nTab);

            OSL_FAIL("note not found");
            return ScAddress::INITIALIZE_INVALID;
        }
    }

    OSL_FAIL("note not found");
    return ScAddress::INITIALIZE_INVALID;
}

ScAddress ScDocument::GetNotePosition( size_t nIndex, SCTAB nTab ) const
{
    for (SCCOL nCol : GetColumnsRange(nTab, 0, MAXCOL))
    {
        size_t nColNoteCount = GetNoteCount(nTab, nCol);
        if (!nColNoteCount)
            continue;

        if (nIndex >= nColNoteCount)
        {
            nIndex -= nColNoteCount;
            continue;
        }

        SCROW nRow = GetNotePosition(nTab, nCol, nIndex);
        if (nRow >= 0)
            return ScAddress(nCol, nRow, nTab);

        OSL_FAIL("note not found");
        return ScAddress::INITIALIZE_INVALID;
    }

    OSL_FAIL("note not found");
    return ScAddress::INITIALIZE_INVALID;
}

SCROW ScDocument::GetNotePosition( SCTAB nTab, SCCOL nCol, size_t nIndex ) const
{
    const ScTable* pTab = FetchTable(nTab);
    if (!pTab)
        return -1;

    return pTab->GetNotePosition(nCol, nIndex);
}

void ScDocument::GetAllNoteEntries( std::vector<sc::NoteEntry>& rNotes ) const
{
    for (ScTable* pTab : maTabs)
    {
        if (!pTab)
            continue;

        pTab->GetAllNoteEntries(rNotes);
    }
}

void ScDocument::GetAllNoteEntries( SCTAB nTab, std::vector<sc::NoteEntry>& rNotes ) const
{
    const ScTable* pTab = FetchTable(nTab);
    if (!pTab)
        return;

    return pTab->GetAllNoteEntries( rNotes );
}

void ScDocument::GetNotesInRange( const ScRangeList& rRange, std::vector<sc::NoteEntry>& rNotes ) const
{
    for( size_t i = 0; i < rRange.size(); ++i)
    {
        const ScRange* pRange = rRange[i];
        for( SCTAB nTab = pRange->aStart.Tab(); nTab <= pRange->aEnd.Tab(); ++nTab )
        {
            maTabs[nTab]->GetNotesInRange( *pRange, rNotes );
        }
    }
}

void ScDocument::GetUnprotectedCells( ScRangeList& rRangeList, SCTAB nTab ) const
{
    maTabs[nTab]->GetUnprotectedCells( rRangeList );
}

bool ScDocument::ContainsNotesInRange( const ScRangeList& rRange ) const
{
    for( size_t i = 0; i < rRange.size(); ++i)
    {
        const ScRange* pRange = rRange[i];
        for( SCTAB nTab = pRange->aStart.Tab(); nTab < pRange->aEnd.Tab(); ++nTab )
        {
            bool bContainsNote = maTabs[nTab]->ContainsNotesInRange( *pRange );
            if(bContainsNote)
                return true;
        }
    }

    return false;
}

void ScDocument::SetAutoNameCache(  ScAutoNameCache* pCache )
{
    delete pAutoNameCache;
    pAutoNameCache = pCache;
}

ScMutationGuard::ScMutationGuard(ScDocument* pDocument, ScMutationGuardFlags nFlags) :
    mpDocument(pDocument),
    mnFlags(nFlags)
{
    for (auto b = 0; b < static_cast<std::size_t>(ScMutationGuardFlags::N); b++)
    {
        if (static_cast<std::size_t>(mnFlags) & (1 << b))
        {
            assert(mpDocument->maMutationGuard[b].try_lock());
        }
    }
}

ScMutationGuard::~ScMutationGuard()
{
#ifndef NDEBUG
    for (auto b = 0; b < static_cast<std::size_t>(ScMutationGuardFlags::N); b++)
    {
        if (static_cast<std::size_t>(mnFlags) & (1 << b))
        {
            mpDocument->maMutationGuard[b].unlock();
        }
    }
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
