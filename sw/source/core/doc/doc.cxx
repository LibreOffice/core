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

#include <config_features.h>

#include <doc.hxx>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <DocumentFieldsManager.hxx>
#include <DocumentSettingManager.hxx>
#include <DocumentDrawModelManager.hxx>
#include <DocumentTimerManager.hxx>
#include <DocumentDeviceManager.hxx>
#include <DocumentChartDataProviderManager.hxx>
#include <DocumentLinksAdministrationManager.hxx>
#include <DocumentListItemsManager.hxx>
#include <DocumentListsManager.hxx>
#include <DocumentOutlineNodesManager.hxx>
#include <DocumentContentOperationsManager.hxx>
#include <DocumentRedlineManager.hxx>
#include <DocumentStatisticsManager.hxx>
#include <DocumentStateManager.hxx>
#include <DocumentStylePoolManager.hxx>
#include <DocumentLayoutManager.hxx>
#include <DocumentExternalDataManager.hxx>
#include <UndoManager.hxx>
#include <dbmgr.hxx>
#include <hintids.hxx>

#include <comphelper/random.hxx>
#include <tools/multisel.hxx>
#include <rtl/ustring.hxx>
#include <svl/poolitem.hxx>
#include <unotools/syslocale.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/pbinitem.hxx>
#include <unotools/localedatawrapper.hxx>

#include <officecfg/Office/Writer.hxx>

#include <swatrset.hxx>
#include <swmodule.hxx>
#include <fmtrfmrk.hxx>
#include <fmtinfmt.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <dbfld.hxx>
#include <txtinet.hxx>
#include <txtrfmrk.hxx>
#include <frmatr.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <swundo.hxx>
#include <rolbck.hxx>
#include <UndoAttribute.hxx>
#include <UndoCore.hxx>
#include <UndoTable.hxx>
#include <pagedesc.hxx>
#include <doctxm.hxx>
#include <poolfmt.hxx>
#include <SwGrammarMarkUp.hxx>
#include <scriptinfo.hxx>
#include <mdiexp.hxx>
#include <docary.hxx>
#include <printdata.hxx>
#include <strings.hrc>
#include <SwUndoTOXChange.hxx>
#include <unocrsr.hxx>
#include <docfld.hxx>
#include <docufld.hxx>
#include <viewsh.hxx>
#include <shellres.hxx>
#include <txtfrm.hxx>
#include <attrhint.hxx>

#include <vector>
#include <map>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <osl/interlck.h>
#include <vbahelper/vbaaccesshelper.hxx>
#include <editeng/langitem.hxx>
#include <calbck.hxx>
#include <crsrsh.hxx>

/* @@@MAINTAINABILITY-HORROR@@@
   Probably unwanted dependency on SwDocShell
*/
#include <docsh.hxx>

#include <com/sun/star/text/XTextRange.hpp>
#include <editeng/unoprnms.hxx>
#include <unotextrange.hxx>
#include <unoprnms.hxx>
#include <unomap.hxx>

using namespace ::com::sun::star;

sal_Int32 SwDoc::acquire()
{
    assert(mReferenceCount >= 0);
    return osl_atomic_increment(&mReferenceCount);
}

sal_Int32 SwDoc::release()
{
    assert(mReferenceCount >= 1);
    auto x = osl_atomic_decrement(&mReferenceCount);
    if (x == 0)
        delete this;
    return x;
}

sal_Int32 SwDoc::getReferenceCount() const
{
    assert(mReferenceCount >= 0);
    return mReferenceCount;
}

::sw::MetaFieldManager & SwDoc::GetMetaFieldManager()
{
    return *m_pMetaFieldManager;
}

::SwContentControlManager& SwDoc::GetContentControlManager()
{
    return *m_pContentControlManager;
}

::sw::UndoManager & SwDoc::GetUndoManager()
{
    return *m_pUndoManager;
}

::sw::UndoManager const & SwDoc::GetUndoManager() const
{
    return *m_pUndoManager;
}


IDocumentUndoRedo & SwDoc::GetIDocumentUndoRedo()
{
    return *m_pUndoManager;
}

IDocumentUndoRedo const & SwDoc::GetIDocumentUndoRedo() const
{
    return *m_pUndoManager;
}

/* IDocumentDrawModelAccess */
IDocumentDrawModelAccess const & SwDoc::getIDocumentDrawModelAccess() const
{
    return GetDocumentDrawModelManager();
}

IDocumentDrawModelAccess & SwDoc::getIDocumentDrawModelAccess()
{
    return GetDocumentDrawModelManager();
}

::sw::DocumentDrawModelManager const & SwDoc::GetDocumentDrawModelManager() const
{
    return *m_pDocumentDrawModelManager;
}

::sw::DocumentDrawModelManager & SwDoc::GetDocumentDrawModelManager()
{
    return *m_pDocumentDrawModelManager;
}

/* IDocumentSettingAccess */
IDocumentSettingAccess const & SwDoc::getIDocumentSettingAccess() const
{
    return GetDocumentSettingManager();
}

IDocumentSettingAccess & SwDoc::getIDocumentSettingAccess()
{
    return GetDocumentSettingManager();
}

::sw::DocumentSettingManager & SwDoc::GetDocumentSettingManager()
{
    return *m_pDocumentSettingManager;
}

::sw::DocumentSettingManager const & SwDoc::GetDocumentSettingManager() const
{
    return *m_pDocumentSettingManager;
}

sal_uInt32 SwDoc::getRsid() const
{
    return mnRsid;
}

void SwDoc::setRsid( sal_uInt32 nVal )
{
    static bool bHack = (getenv("LIBO_ONEWAY_STABLE_ODF_EXPORT") != nullptr);

    sal_uInt32 nIncrease = 0;
    if (!bHack)
    {
        // Increase the rsid with a random number smaller than 2^17. This way we
        // expect to be able to edit a document 2^12 times before rsid overflows.
        // start from 1 to ensure the new rsid is not the same
        nIncrease = comphelper::rng::uniform_uint_distribution(1, (1 << 17) - 1);
    }
    mnRsid = nVal + nIncrease;
}

sal_uInt32 SwDoc::getRsidRoot() const
{
    return mnRsidRoot;
}

void SwDoc::setRsidRoot( sal_uInt32 nVal )
{
    mnRsidRoot = nVal;
}

/* IDocumentChartDataProviderAccess */
IDocumentChartDataProviderAccess const & SwDoc::getIDocumentChartDataProviderAccess() const
{
    return *m_pDocumentChartDataProviderManager;
}

IDocumentChartDataProviderAccess & SwDoc::getIDocumentChartDataProviderAccess()
{
    return *m_pDocumentChartDataProviderManager;
}

// IDocumentDeviceAccess
IDocumentDeviceAccess const & SwDoc::getIDocumentDeviceAccess() const
{
    return *m_pDeviceAccess;
}

IDocumentDeviceAccess & SwDoc::getIDocumentDeviceAccess()
{
    return *m_pDeviceAccess;
}

//IDocumentTimerAccess
IDocumentTimerAccess const & SwDoc::getIDocumentTimerAccess() const
{
    return *m_pDocumentTimerManager;
}

IDocumentTimerAccess & SwDoc::getIDocumentTimerAccess()
{
    return *m_pDocumentTimerManager;
}

// IDocumentLinksAdministration
IDocumentLinksAdministration const & SwDoc::getIDocumentLinksAdministration() const
{
    return *m_pDocumentLinksAdministrationManager;
}

IDocumentLinksAdministration & SwDoc::getIDocumentLinksAdministration()
{
    return *m_pDocumentLinksAdministrationManager;
}

::sw::DocumentLinksAdministrationManager const & SwDoc::GetDocumentLinksAdministrationManager() const
{
    return *m_pDocumentLinksAdministrationManager;
}

::sw::DocumentLinksAdministrationManager & SwDoc::GetDocumentLinksAdministrationManager()
{
    return *m_pDocumentLinksAdministrationManager;
}

//IDocumentListItems
IDocumentListItems const & SwDoc::getIDocumentListItems() const
{
    return *m_pDocumentListItemsManager;
}

//IDocumentListItems
IDocumentListItems & SwDoc::getIDocumentListItems()
{
    return *m_pDocumentListItemsManager;
}

//IDocumentListsAccess
IDocumentListsAccess const & SwDoc::getIDocumentListsAccess() const
{
    return *m_pDocumentListsManager;
}

IDocumentListsAccess & SwDoc::getIDocumentListsAccess()
{
    return *m_pDocumentListsManager;
}

//IDocumentOutlinesNodes
IDocumentOutlineNodes const & SwDoc::getIDocumentOutlineNodes() const
{
    return *m_pDocumentOutlineNodesManager;
}

IDocumentOutlineNodes & SwDoc::getIDocumentOutlineNodes()
{
    return *m_pDocumentOutlineNodesManager;
}

//IDocumentContentOperations
IDocumentContentOperations const & SwDoc::getIDocumentContentOperations() const
{
    return *m_pDocumentContentOperationsManager;
}

IDocumentContentOperations & SwDoc::getIDocumentContentOperations()
{
    return *m_pDocumentContentOperationsManager;
}

::sw::DocumentContentOperationsManager const & SwDoc::GetDocumentContentOperationsManager() const
{
    return *m_pDocumentContentOperationsManager;
}
::sw::DocumentContentOperationsManager & SwDoc::GetDocumentContentOperationsManager()
{
    return *m_pDocumentContentOperationsManager;
}

//IDocumentRedlineAccess
IDocumentRedlineAccess const & SwDoc::getIDocumentRedlineAccess() const
{
    return *m_pDocumentRedlineManager;
}

IDocumentRedlineAccess& SwDoc::getIDocumentRedlineAccess()
{
    return *m_pDocumentRedlineManager;
}

::sw::DocumentRedlineManager const & SwDoc::GetDocumentRedlineManager() const
{
    return *m_pDocumentRedlineManager;
}

::sw::DocumentRedlineManager& SwDoc::GetDocumentRedlineManager()
{
    return *m_pDocumentRedlineManager;
}

//IDocumentFieldsAccess

IDocumentFieldsAccess const & SwDoc::getIDocumentFieldsAccess() const
{
    return *m_pDocumentFieldsManager;
}

IDocumentFieldsAccess & SwDoc::getIDocumentFieldsAccess()
{
    return *m_pDocumentFieldsManager;
}

::sw::DocumentFieldsManager & SwDoc::GetDocumentFieldsManager()
{
    return *m_pDocumentFieldsManager;
}

//IDocumentStatistics
IDocumentStatistics const & SwDoc::getIDocumentStatistics() const
{
    return *m_pDocumentStatisticsManager;
}

IDocumentStatistics & SwDoc::getIDocumentStatistics()
{
    return *m_pDocumentStatisticsManager;
}

::sw::DocumentStatisticsManager const & SwDoc::GetDocumentStatisticsManager() const
{
    return *m_pDocumentStatisticsManager;
}

::sw::DocumentStatisticsManager & SwDoc::GetDocumentStatisticsManager()
{
    return *m_pDocumentStatisticsManager;
}

//IDocumentState
IDocumentState const & SwDoc::getIDocumentState() const
{
    return *m_pDocumentStateManager;
}

IDocumentState & SwDoc::getIDocumentState()
{
    return *m_pDocumentStateManager;
}

//IDocumentLayoutAccess
IDocumentLayoutAccess const & SwDoc::getIDocumentLayoutAccess() const
{
    return *m_pDocumentLayoutManager;
}

IDocumentLayoutAccess & SwDoc::getIDocumentLayoutAccess()
{
    return *m_pDocumentLayoutManager;
}

::sw::DocumentLayoutManager const & SwDoc::GetDocumentLayoutManager() const
{
    return *m_pDocumentLayoutManager;
}

::sw::DocumentLayoutManager & SwDoc::GetDocumentLayoutManager()
{
    return *m_pDocumentLayoutManager;
}

//IDocumentStylePoolAccess
IDocumentStylePoolAccess const & SwDoc::getIDocumentStylePoolAccess() const
{
    return *m_pDocumentStylePoolManager;
}

IDocumentStylePoolAccess & SwDoc::getIDocumentStylePoolAccess()
{
    return *m_pDocumentStylePoolManager;
}

//IDocumentExternalData
IDocumentExternalData const & SwDoc::getIDocumentExternalData() const
{
    return *m_pDocumentExternalDataManager;
}

IDocumentExternalData & SwDoc::getIDocumentExternalData()
{
    return *m_pDocumentExternalDataManager;
}

/* Implementations the next Interface here */

/*
 * Document editing (Doc-SS) to fill the document
 * by the RTF parser and for the EditShell.
 */
void SwDoc::ChgDBData(const SwDBData& rNewData)
{
    if( rNewData != maDBData )
    {
        maDBData = rNewData;
        getIDocumentState().SetModified();
        if (m_pDBManager)
            m_pDBManager->CommitLastRegistrations();
    }
    getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::DatabaseName)->UpdateFields();
}

namespace {

struct PostItField_ : public SetGetExpField
{
    PostItField_( const SwNode& rNd, const SwTextField* pField )
        : SetGetExpField( rNd, pField, std::nullopt ) {}

    sal_uInt16 GetPageNo( const StringRangeEnumerator &rRangeEnum,
            const o3tl::sorted_vector< sal_Int32 > &rPossiblePages,
            sal_uInt16& rVirtPgNo, sal_Int32& rLineNo );

    const SwPostItField* GetPostIt() const
    {
        return static_cast<const SwPostItField*>( GetTextField()->GetFormatField().GetField() );
    }
};

}

sal_uInt16 PostItField_::GetPageNo(
    const StringRangeEnumerator &rRangeEnum,
    const o3tl::sorted_vector< sal_Int32 > &rPossiblePages,
    /* out */ sal_uInt16& rVirtPgNo, /* out */ sal_Int32& rLineNo )
{
    //Problem: If a PostItField is contained in a Node that is represented
    //by more than one layout instance,
    //we have to decide whether it should be printed once or n-times.
    //Probably only once. For the page number we don't select a random one,
    //but the PostIt's first occurrence in the selected area.
    rVirtPgNo = 0;
    SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(GetTextField()->GetTextNode());
    for( SwTextFrame* pFrame = aIter.First(); pFrame;  pFrame = aIter.Next() )
    {
        TextFrameIndex const nPos = pFrame->MapModelToView(
                &GetTextField()->GetTextNode(), GetContent());
        if( pFrame->GetOffset() > nPos ||
            (pFrame->HasFollow() && pFrame->GetFollow()->GetOffset() <= nPos) )
            continue;
        sal_uInt16 nPgNo = pFrame->GetPhyPageNum();
        if( rRangeEnum.hasValue( nPgNo, &rPossiblePages ))
        {
            rLineNo = o3tl::narrowing<sal_Int32>(pFrame->GetLineCount( nPos ) +
                      pFrame->GetAllLines() - pFrame->GetThisLines());
            rVirtPgNo = pFrame->GetVirtPageNum();
            return nPgNo;
        }
    }
    return 0;
}

bool sw_GetPostIts(const IDocumentFieldsAccess& rIDFA, SetGetExpFields* pSrtLst)
{
    SwFieldType* pFieldType = rIDFA.GetSysFieldType(SwFieldIds::Postit);
    assert(pFieldType);

    std::vector<SwFormatField*> vFields;
    pFieldType->GatherFields(vFields);
    if(pSrtLst)
        for(auto pField: vFields)
        {
            auto pTextField = pField->GetTextField();
            std::unique_ptr<PostItField_> pNew(new PostItField_(pTextField->GetTextNode(), pTextField));
            pSrtLst->insert(std::move(pNew));

        }
    return vFields.size()>0;
}

static void lcl_FormatPostIt(
    IDocumentContentOperations* pIDCO,
    SwPaM& aPam,
    const SwPostItField* pField,
    bool bNewPage, bool bIsFirstPostIt,
    sal_uInt16 nPageNo, sal_Int32 nLineNo )
{
    static char const sTmp[] = " : ";

    assert(SwViewShell::GetShellRes());

    if (bNewPage)
    {
        pIDCO->InsertPoolItem( aPam, SvxFormatBreakItem( SvxBreak::PageAfter, RES_BREAK ) );
        pIDCO->SplitNode( *aPam.GetPoint(), false );
    }
    else if (!bIsFirstPostIt)
    {
        // add an empty line between different notes
        pIDCO->SplitNode( *aPam.GetPoint(), false );
        pIDCO->SplitNode( *aPam.GetPoint(), false );
    }

    OUString aStr = SwViewShell::GetShellRes()->aPostItPage +
        sTmp +
        OUString::number( nPageNo ) +
        " ";
    if( nLineNo )
    {
        aStr += SwViewShell::GetShellRes()->aPostItLine +
            sTmp +
            OUString::number( nLineNo ) +
            " ";
    }
    SvtSysLocale aSysLocale;
    aStr += SwViewShell::GetShellRes()->aPostItAuthor +
        sTmp + pField->GetPar1() + " " +
        /*(LocaleDataWrapper&)*/aSysLocale.GetLocaleData().getDate( pField->GetDate() );
    if(pField->GetResolved())
        aStr += " " + SwResId(STR_RESOLVED);
    pIDCO->InsertString( aPam, aStr );

    pIDCO->SplitNode( *aPam.GetPoint(), false );
    aStr = pField->GetPar2();
#if defined(_WIN32)
    // Throw out all CR in Windows
    aStr = aStr.replaceAll("\r", "");
#endif
    pIDCO->InsertString( aPam, aStr );
}

/// provide the paper tray to use according to the page style in use,
/// but do that only if the respective item is NOT just the default item
static sal_Int32 lcl_GetPaperBin( const SwPageFrame *pStartFrame )
{
    sal_Int32 nRes = -1;

    const SwFrameFormat &rFormat = pStartFrame->GetPageDesc()->GetMaster();
    const SfxPoolItem *pItem = nullptr;
    SfxItemState eState = rFormat.GetItemState( RES_PAPER_BIN, false, &pItem );
    const SvxPaperBinItem *pPaperBinItem = dynamic_cast< const SvxPaperBinItem * >(pItem);
    if (eState > SfxItemState::DEFAULT && pPaperBinItem)
        nRes = pPaperBinItem->GetValue();

    return nRes;
}

namespace
{
// tdf#:114663 Translates a range string from user input (with page numbering possibly not
// taking blank pages into account) to equivalent string which references physical page numbers.
// rUIPages2PhyPagesMap must contain a contiguous sequence of UI page numbers
OUString UIPages2PhyPages(const OUString& rUIPageRange, const std::map< sal_Int32, sal_Int32 >& rUIPages2PhyPagesMap)
{
    if (rUIPages2PhyPagesMap.empty())
        return OUString();
    auto iMin = rUIPages2PhyPagesMap.begin();
    const sal_Int32 nUIPageMin = iMin->first, nPhyPageMin = iMin->second;
    auto iMax = rUIPages2PhyPagesMap.rbegin();
    const sal_Int32 nUIPageMax = iMax->first, nPhyPageMax = iMax->second;
    OUStringBuffer aOut(rUIPageRange.getLength());
    OUStringBuffer aNumber(16);
    const sal_Unicode* pInput = rUIPageRange.getStr();
    while (*pInput)
    {
        while (*pInput >= '0' && *pInput <= '9')
            aNumber.append(*pInput++);
        if (!aNumber.isEmpty())
        {
            sal_Int32 nNumber = o3tl::toInt32(aNumber);
            aNumber.setLength(0);
            if (nNumber < nUIPageMin)
                nNumber = nPhyPageMin-1;
            else if (nNumber > nUIPageMax)
                nNumber = nPhyPageMax+1;
            else
                nNumber = rUIPages2PhyPagesMap.at(nNumber);
            aOut.append(nNumber);
        }

        while (*pInput && (*pInput < '0' || *pInput > '9'))
            aOut.append(*pInput++);
    }

    return aOut.makeStringAndClear();
}
}

// tdf#52316 remove blank pages from page count and actual page number
void SwDoc::CalculateNonBlankPages(
    const SwRootFrame& rLayout,
    sal_uInt16& nDocPageCount,
    sal_uInt16& nActualPage)
{
    sal_uInt16 nDocPageCountWithBlank = nDocPageCount;
    sal_uInt16 nActualPageWithBlank = nActualPage;
    sal_uInt16 nPageNum = 1;
    const SwPageFrame *pStPage = dynamic_cast<const SwPageFrame*>( rLayout.Lower() );
    while (pStPage && nPageNum <= nDocPageCountWithBlank)
    {
        if ( pStPage->getFrameArea().Height() == 0 )
        {
            --nDocPageCount;
            if (nPageNum <= nActualPageWithBlank)
                --nActualPage;
        }
        ++nPageNum;
        pStPage = static_cast<const SwPageFrame*>(pStPage->GetNext());
    }
}

void SwDoc::CalculatePagesForPrinting(
    const SwRootFrame& rLayout,
    /* out */ SwRenderData &rData,
    const SwPrintUIOptions &rOptions,
    bool bIsPDFExport,
    sal_Int32 nDocPageCount )
{
    const sal_Int64 nContent = rOptions.getIntValue( "PrintContent", 0 );
    const bool bPrintSelection = nContent == 2;

    // properties to take into account when calculating the set of pages
    // (PDF export UI does not allow for selecting left or right pages only)
    bool bPrintLeftPages    = bIsPDFExport || rOptions.IsPrintLeftPages();
    bool bPrintRightPages   = bIsPDFExport || rOptions.IsPrintRightPages();
    // #i103700# printing selections should not allow for automatic inserting empty pages
    bool bPrintEmptyPages   = !bPrintSelection && rOptions.IsPrintEmptyPages( bIsPDFExport );

    std::map< sal_Int32, sal_Int32 > &rPrinterPaperTrays = rData.GetPrinterPaperTrays();
    o3tl::sorted_vector< sal_Int32 > &rValidPages = rData.GetValidPagesSet();
    // Map page numbers from user input (possibly ignoring blanks) to physical page numbers
    std::map< sal_Int32, sal_Int32 > aUIPages2PhyPagesMap;
    rValidPages.clear();

    sal_Int32 nPageNum = 1, nUIPageNum = 1;
    const SwPageFrame *pStPage = dynamic_cast<const SwPageFrame*>( rLayout.Lower() );
    while (pStPage && nPageNum <= nDocPageCount)
    {
        const bool bNonEmptyPage = pStPage->getFrameArea().Height() != 0;
        const bool bPrintThisPage =
            ( (bPrintRightPages && pStPage->OnRightPage()) ||
              (bPrintLeftPages && !pStPage->OnRightPage()) ) &&
            ( bPrintEmptyPages || bNonEmptyPage );

        if (bPrintThisPage)
        {
            rValidPages.insert( nPageNum );
            rPrinterPaperTrays[ nPageNum ] = lcl_GetPaperBin( pStPage );
        }

        if ( bPrintEmptyPages || bNonEmptyPage )
        {
            aUIPages2PhyPagesMap[nUIPageNum++] = nPageNum;
        }
        ++nPageNum;
        pStPage = static_cast<const SwPageFrame*>(pStPage->GetNext());
    }

    // now that we have identified the valid pages for printing according
    // to the print settings we need to get the PageRange to use and
    // use both results to get the actual pages to be printed
    // (post-it settings need to be taken into account later on!)

    // get PageRange value to use
    OUString aPageRange;
    // #i116085# - adjusting fix for i113919
    if ( !bIsPDFExport )
    {
        // PageContent :
        // 0 -> print all pages (default if aPageRange is empty)
        // 1 -> print range according to PageRange
        // 2 -> print selection
        if (1 == nContent)
            aPageRange = rOptions.getStringValue( "PageRange" );

        if (2 == nContent)
        {
            // note that printing selections is actually implemented by copying
            // the selection to a new temporary document and printing all of that one.
            // Thus for Writer "PrintContent" must never be 2.
            // See SwXTextDocument::GetRenderDoc for evaluating if a selection is to be
            // printed and for creating the temporary document.
        }

        // please note
    }
    if (aPageRange.isEmpty())    // empty string -> print all
    {
        // set page range to print to 'all pages'
        aPageRange = OUString::number( 1 ) + "-" + OUString::number( nDocPageCount );
    }
    else
    {
        // Convert page numbers from user input to physical page numbers
        aPageRange = UIPages2PhyPages(aPageRange, aUIPages2PhyPagesMap);
    }
    rData.SetPageRange( aPageRange );

    // get vector of pages to print according to PageRange and valid pages set from above
    // (result may be an empty vector, for example if the range string is not correct)
    // If excluding empty pages, allow range to specify range of printable pages
    StringRangeEnumerator::getRangesFromString( aPageRange, rData.GetPagesToPrint(),
                                                1, nDocPageCount, 0, &rData.GetValidPagesSet() );
}

void SwDoc::UpdatePagesForPrintingWithPostItData(
    /* out */ SwRenderData &rData,
    const SwPrintUIOptions &rOptions,
    sal_Int32 nDocPageCount )
{

    SwPostItMode nPostItMode = static_cast<SwPostItMode>( rOptions.getIntValue( "PrintAnnotationMode", 0 ) );
    assert((nPostItMode == SwPostItMode::NONE || rData.HasPostItData())
            && "print post-its without post-it data?");
    const SetGetExpFields::size_type nPostItCount =
        rData.HasPostItData() ? rData.m_pPostItFields->size() : 0;
    if (nPostItMode == SwPostItMode::NONE || nPostItCount <= 0)
        return;

    CurrShell aCurr( rData.m_pPostItShell.get() );

    // clear document and move to end of it
    SwDoc & rPostItDoc(*rData.m_pPostItShell->GetDoc());
    SwPaM aPam(rPostItDoc.GetNodes().GetEndOfContent());
    aPam.Move( fnMoveBackward, GoInDoc );
    aPam.SetMark();
    aPam.Move( fnMoveForward, GoInDoc );
    rPostItDoc.getIDocumentContentOperations().DeleteRange( aPam );

    const StringRangeEnumerator aRangeEnum( rData.GetPageRange(), 1, nDocPageCount, 0 );

    // For mode SwPostItMode::EndPage:
    // maps a physical page number to the page number in post-it document that holds
    // the first post-it for that physical page . Needed to relate the correct start frames
    // from the post-it doc to the physical page of the document
    std::map< sal_Int32, sal_Int32 >  aPostItLastStartPageNum;

    // add all post-its on valid pages within the page range to the
    // temporary post-it document.
    // Since the array of post-it fields is sorted by page and line number we will
    // already get them in the correct order
    sal_uInt16 nVirtPg = 0, nLastPageNum = 0, nPhyPageNum = 0;
    sal_Int32 nLineNo = 0;
    bool bIsFirstPostIt = true;
    for (SetGetExpFields::size_type i = 0; i < nPostItCount; ++i)
    {
        PostItField_& rPostIt = static_cast<PostItField_&>(*(*rData.m_pPostItFields)[ i ]);
        nLastPageNum = nPhyPageNum;
        nPhyPageNum = rPostIt.GetPageNo(
                aRangeEnum, rData.GetValidPagesSet(), nVirtPg, nLineNo );
        if (nPhyPageNum)
        {
            // need to insert a page break?
            // In SwPostItMode::EndPage mode for each document page the following
            // post-it page needs to start on a new page
            const bool bNewPage = nPostItMode == SwPostItMode::EndPage &&
                    !bIsFirstPostIt && nPhyPageNum != nLastPageNum;

            lcl_FormatPostIt( &rData.m_pPostItShell->GetDoc()->getIDocumentContentOperations(), aPam,
                    rPostIt.GetPostIt(), bNewPage, bIsFirstPostIt, nVirtPg, nLineNo );
            bIsFirstPostIt = false;

            if (nPostItMode == SwPostItMode::EndPage)
            {
                // get the correct number of current pages for the post-it document
                rData.m_pPostItShell->CalcLayout();
                const sal_Int32 nPages = rData.m_pPostItShell->GetPageCount();
                aPostItLastStartPageNum[ nPhyPageNum ] = nPages;
            }
        }
    }

    // format post-it doc to get correct number of pages
    rData.m_pPostItShell->CalcLayout();

    SwRootFrame* pPostItRoot = rData.m_pPostItShell->GetLayout();
    //tdf#103313 print dialog maxes out cpu as Idles never get to
    //complete this postitshell's desire to complete formatting
    pPostItRoot->ResetIdleFormat();

    const sal_Int32 nPostItDocPageCount = rData.m_pPostItShell->GetPageCount();

    if (nPostItMode == SwPostItMode::Only || nPostItMode == SwPostItMode::EndDoc)
    {
        // now add those post-it pages to the vector of pages to print
        // or replace them if only post-its should be printed

        if (nPostItMode == SwPostItMode::Only)
        {
            // no document page to be printed
            rData.GetPagesToPrint().clear();
        }

        // now we just need to add the post-it pages to be printed to the
        // end of the vector of pages to print
        sal_Int32 nPageNum = 0;
        const SwPageFrame * pPageFrame = static_cast<SwPageFrame*>(pPostItRoot->Lower());
        while( pPageFrame && nPageNum < nPostItDocPageCount )
        {
            ++nPageNum;
            // negative page number indicates page is from the post-it doc
            rData.GetPagesToPrint().push_back( -nPageNum );
            pPageFrame = static_cast<const SwPageFrame*>(pPageFrame->GetNext());
        }
        OSL_ENSURE( nPageNum == nPostItDocPageCount, "unexpected number of pages" );
    }
    else if (nPostItMode == SwPostItMode::EndPage)
    {
        // the next step is to find all the pages from the post-it
        // document that should be printed for a given physical page
        // of the document

        std::vector< sal_Int32 >            aTmpPagesToPrint;
        sal_Int32 nLastPostItPage(0);
        const size_t nNum = rData.GetPagesToPrint().size();
        for (size_t i = 0 ;  i < nNum;  ++i)
        {
            // add the physical page to print from the document
            const sal_Int32 nPhysPage = rData.GetPagesToPrint()[i];
            aTmpPagesToPrint.push_back( nPhysPage );

            // add the post-it document pages to print, i.e those
            // post-it pages that have the data for the above physical page
            std::map<sal_Int32, sal_Int32>::const_iterator const iter(
                    aPostItLastStartPageNum.find(nPhysPage));
            if (iter != aPostItLastStartPageNum.end())
            {
                for (sal_Int32 j = nLastPostItPage + 1;
                        j <= iter->second; ++j)
                {
                    // negative page number indicates page is from the
                    aTmpPagesToPrint.push_back(-j); // post-it document
                }
                nLastPostItPage = iter->second;
            }
        }

        // finally we need to assign those vectors to the resulting ones.
        // swapping the data should be more efficient than assigning since
        // we won't need the temporary vectors anymore
        rData.GetPagesToPrint().swap( aTmpPagesToPrint );
    }

}

void SwDoc::CalculatePagePairsForProspectPrinting(
    const SwRootFrame& rLayout,
    /* out */ SwRenderData &rData,
    const SwPrintUIOptions &rOptions,
    sal_Int32 nDocPageCount )
{
    std::map< sal_Int32, sal_Int32 > &rPrinterPaperTrays = rData.GetPrinterPaperTrays();
    o3tl::sorted_vector< sal_Int32 > &rValidPagesSet = rData.GetValidPagesSet();
    std::vector< std::pair< sal_Int32, sal_Int32 > > &rPagePairs = rData.GetPagePairsForProspectPrinting();
    std::map< sal_Int32, const SwPageFrame * > validStartFrames;

    rPagePairs.clear();
    rValidPagesSet.clear();

    OUString aPageRange;
    // PageContent :
    // 0 -> print all pages (default if aPageRange is empty)
    // 1 -> print range according to PageRange
    // 2 -> print selection
    const sal_Int64 nContent = rOptions.getIntValue( "PrintContent", 0 );
    if (nContent == 1)
        aPageRange = rOptions.getStringValue( "PageRange" );
    if (aPageRange.isEmpty())    // empty string -> print all
    {
        // set page range to print to 'all pages'
        aPageRange = OUString::number( 1 ) + "-" + OUString::number( nDocPageCount );
    }
    StringRangeEnumerator aRange( aPageRange, 1, nDocPageCount, 0 );

    if ( aRange.size() <= 0)
        return;

    const SwPageFrame *pStPage  = dynamic_cast<const SwPageFrame*>( rLayout.Lower() );
    for ( sal_Int32 i = 1; pStPage && i < nDocPageCount; ++i )
        pStPage = static_cast<const SwPageFrame*>(pStPage->GetNext());
    if ( !pStPage )          // Then it was that
        return;

    // currently for prospect printing all pages are valid to be printed
    // thus we add them all to the respective map and set for later use
    sal_Int32 nPageNum = 0;
    const SwPageFrame *pPageFrame  = dynamic_cast<const SwPageFrame*>( rLayout.Lower() );
    while( pPageFrame && nPageNum < nDocPageCount )
    {
        ++nPageNum;
        rValidPagesSet.insert( nPageNum );
        validStartFrames[ nPageNum ] = pPageFrame;
        pPageFrame = static_cast<const SwPageFrame*>(pPageFrame->GetNext());

        rPrinterPaperTrays[ nPageNum ] = lcl_GetPaperBin( pStPage );
    }
    OSL_ENSURE( nPageNum == nDocPageCount, "unexpected number of pages" );

    // properties to take into account when calculating the set of pages
    // Note: here bPrintLeftPages and bPrintRightPages refer to the (virtual) resulting pages
    //      of the prospect!
    bool bPrintLeftPages     = rOptions.IsPrintLeftPages();
    bool bPrintRightPages    = rOptions.IsPrintRightPages();
    bool bPrintProspectRTL = rOptions.getIntValue( "PrintProspectRTL",  0 ) != 0;

    // get pages for prospect printing according to the 'PageRange'
    // (duplicates and any order allowed!)
    std::vector< sal_Int32 > aPagesToPrint;
    StringRangeEnumerator::getRangesFromString(
            aPageRange, aPagesToPrint, 1, nDocPageCount, 0 );

    if (aPagesToPrint.empty())
        return;

    // now fill the vector for calculating the page pairs with the start frames
    // from the above obtained vector
    std::vector< const SwPageFrame * > aVec;
    for (sal_Int32 nPage : aPagesToPrint)
    {
        const SwPageFrame *pFrame = validStartFrames[ nPage ];
        aVec.push_back( pFrame );
    }

    // just one page is special ...
    if ( 1 == aVec.size() )
    {
#if defined __GNUC__ && !defined __clang__ && __GNUC__ <= 13 && __cplusplus == 202002L
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif
        aVec.insert( aVec.begin() + 1, nullptr ); // insert a second empty page
#if defined __GNUC__ && !defined __clang__ && __GNUC__ <= 13 && __cplusplus == 202002L
#pragma GCC diagnostic pop
#endif
    }
    else
    {
        // now extend the number of pages to fit a multiple of 4
        // (4 'normal' pages are needed for a single prospect paper
        //  with back and front)
        while( aVec.size() & 3 )
            aVec.push_back( nullptr );
    }

    // make sure that all pages are in correct order
    std::vector< const SwPageFrame * >::size_type nSPg = 0;
    std::vector< const SwPageFrame * >::size_type nEPg = aVec.size();
    sal_Int32 nStep = 1;
    if ( 0 == (nEPg & 1 ))      // there are no uneven ones!
        --nEPg;

    if ( !bPrintLeftPages )
        ++nStep;
    else if ( !bPrintRightPages )
    {
        ++nStep;
        ++nSPg;
        --nEPg;
    }

    // the number of 'virtual' pages to be printed
    sal_Int32 nCntPage = (( nEPg - nSPg ) / ( 2 * nStep )) + 1;

    for ( sal_Int32 nPrintCount = 0; nSPg < nEPg &&
            nPrintCount < nCntPage; ++nPrintCount )
    {
        pStPage = aVec[ nSPg ];
        const SwPageFrame* pNxtPage = nEPg < aVec.size() ? aVec[ nEPg ] : nullptr;

        short nRtlOfs = bPrintProspectRTL ? 1 : 0;
        if ( 0 == (( nSPg + nRtlOfs) & 1 ) )     // switch for odd number in LTR, even number in RTL
        {
            const SwPageFrame* pTmp = pStPage;
            pStPage = pNxtPage;
            pNxtPage = pTmp;
        }

        sal_Int32 nFirst = -1, nSecond = -1;
        for ( int nC = 0; nC < 2; ++nC )
        {
            sal_Int32 nPage = -1;
            if ( pStPage )
                nPage = pStPage->GetPhyPageNum();
            if (nC == 0)
                nFirst  = nPage;
            else
                nSecond = nPage;

            pStPage = pNxtPage;
        }
        rPagePairs.emplace_back(nFirst, nSecond );

        nSPg = nSPg + nStep;
        nEPg = nEPg - nStep;
    }
    OSL_ENSURE( size_t(nCntPage) == rPagePairs.size(), "size mismatch for number of page pairs" );

    // luckily prospect printing does not make use of post-its so far,
    // thus we are done here.
}

/// @return the reference in the doc for the name
const SwFormatRefMark* SwDoc::GetRefMark( std::u16string_view rName ) const
{
    ItemSurrogates aSurrogates;
    GetAttrPool().GetItemSurrogates(aSurrogates, RES_TXTATR_REFMARK);
    for (const SfxPoolItem* pItem : aSurrogates)
    {
        auto pFormatRef = dynamic_cast<const SwFormatRefMark*>(pItem);
        if(!pFormatRef)
            continue;

        const SwTextRefMark* pTextRef = pFormatRef->GetTextRefMark();
        if( pTextRef && &pTextRef->GetTextNode().GetNodes() == &GetNodes() &&
            rName == pFormatRef->GetRefName() )
            return pFormatRef;
    }
    return nullptr;
}

/// @return the RefMark per index - for Uno
const SwFormatRefMark* SwDoc::GetRefMark( sal_uInt16 nIndex ) const
{
    const SwFormatRefMark* pRet = nullptr;

    sal_uInt32 nCount = 0;
    ItemSurrogates aSurrogates;
    GetAttrPool().GetItemSurrogates(aSurrogates, RES_TXTATR_REFMARK);
    for (const SfxPoolItem* pItem : aSurrogates)
    {
        auto pRefMark = dynamic_cast<const SwFormatRefMark*>(pItem);
        if( !pRefMark )
            continue;
        const SwTextRefMark* pTextRef = pRefMark->GetTextRefMark();
        if( pTextRef && &pTextRef->GetTextNode().GetNodes() == &GetNodes() )
        {
            if(nCount == nIndex)
            {
                pRet = pRefMark;
                break;
            }
            nCount++;
        }
    }
    return pRet;
}

/// @return the names of all set references in the Doc
//JP 24.06.96: If the array pointer is 0, then just return whether a RefMark is set in the Doc
// OS 25.06.96: From now on we always return the reference count
sal_uInt16 SwDoc::GetRefMarks( std::vector<OUString>* pNames ) const
{
    sal_uInt16 nCount = 0;
    ItemSurrogates aSurrogates;
    GetAttrPool().GetItemSurrogates(aSurrogates, RES_TXTATR_REFMARK);
    for (const SfxPoolItem* pItem : aSurrogates)
    {
        auto pRefMark = dynamic_cast<const SwFormatRefMark*>(pItem);
        if( !pRefMark )
            continue;
        const SwTextRefMark* pTextRef = pRefMark->GetTextRefMark();
        if( pTextRef && &pTextRef->GetTextNode().GetNodes() == &GetNodes() )
        {
            if( pNames )
            {
                OUString aTmp(pRefMark->GetRefName());
                pNames->insert(pNames->begin() + nCount, aTmp);
            }
            ++nCount;
        }
    }

    return nCount;
}

void SwDoc::DeleteFormatRefMark(const SwFormatRefMark* pFormatRefMark)
{
    const SwTextRefMark* pTextRefMark = pFormatRefMark->GetTextRefMark();
    SwTextNode& rTextNd = const_cast<SwTextNode&>(pTextRefMark->GetTextNode());
    std::unique_ptr<SwRegHistory> aRegHistory;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndoResetAttr* pUndo = new SwUndoResetAttr(SwPosition(rTextNd, pTextRefMark->GetStart()),
                                                     RES_TXTATR_REFMARK);
        GetIDocumentUndoRedo().AppendUndo(std::unique_ptr<SwUndo>(pUndo));
        aRegHistory.reset(new SwRegHistory(rTextNd, &pUndo->GetHistory()));
        rTextNd.GetpSwpHints()->Register(aRegHistory.get());
    }
    rTextNd.DeleteAttribute(const_cast<SwTextRefMark*>(pTextRefMark));
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        if (rTextNd.GetpSwpHints())
            rTextNd.GetpSwpHints()->DeRegister();
    }
    getIDocumentState().SetModified();
}

static bool lcl_SpellAndGrammarAgain( SwNode* pNd, void* pArgs )
{
    SwTextNode *pTextNode = pNd->GetTextNode();
    bool bOnlyWrong = *static_cast<sal_Bool*>(pArgs);
    if( pTextNode )
    {
        if( bOnlyWrong )
        {
            if( pTextNode->GetWrong() &&
                pTextNode->GetWrong()->InvalidateWrong() )
                pTextNode->SetWrongDirty(sw::WrongState::TODO);
            if( pTextNode->GetGrammarCheck() &&
                pTextNode->GetGrammarCheck()->InvalidateWrong() )
                pTextNode->SetGrammarCheckDirty( true );
        }
        else
        {
            pTextNode->SetWrongDirty(sw::WrongState::TODO);
            if( pTextNode->GetWrong() )
                pTextNode->GetWrong()->SetInvalid( 0, COMPLETE_STRING );
            pTextNode->SetGrammarCheckDirty( true );
            if( pTextNode->GetGrammarCheck() )
                pTextNode->GetGrammarCheck()->SetInvalid( 0, COMPLETE_STRING );
        }
    }
    return true;
}

static bool lcl_CheckSmartTagsAgain( SwNode* pNd, void*  )
{
    SwTextNode *pTextNode = pNd->GetTextNode();
    if( pTextNode )
    {
        pTextNode->SetSmartTagDirty( true );
        pTextNode->ClearSmartTags();
    }
    return true;
}

/**
 * Re-trigger spelling in the idle handler.
 *
 * @param bInvalid if <true>, the WrongLists in all nodes are invalidated
 *                 and the SpellInvalid flag is set on all pages.
 * @param bOnlyWrong controls whether only the areas with wrong words are
 *                   checked or the whole area.
 * @param bSmartTags ???
 */
void SwDoc::SpellItAgainSam( bool bInvalid, bool bOnlyWrong, bool bSmartTags )
{
    o3tl::sorted_vector<SwRootFrame*> aAllLayouts = GetAllLayouts();
    assert(getIDocumentLayoutAccess().GetCurrentLayout() && "SpellAgain: Where's my RootFrame?");
    if( bInvalid )
    {
        for ( auto aLayout : aAllLayouts )
        {
            aLayout->AllInvalidateSmartTagsOrSpelling(bSmartTags);
            aLayout->SetNeedGrammarCheck(true);
        }
        if ( bSmartTags )
            GetNodes().ForEach( lcl_CheckSmartTagsAgain, &bOnlyWrong );
        GetNodes().ForEach( lcl_SpellAndGrammarAgain, &bOnlyWrong );
    }

    for ( auto aLayout : aAllLayouts )
        aLayout->SetIdleFlags();
}

void SwDoc::InvalidateAutoCompleteFlag()
{
    SwRootFrame* pTmpRoot = getIDocumentLayoutAccess().GetCurrentLayout();
    if( !pTmpRoot )
        return;

    o3tl::sorted_vector<SwRootFrame*> aAllLayouts = GetAllLayouts();
    for( auto aLayout : aAllLayouts )
        aLayout->AllInvalidateAutoCompleteWords();
    for( SwNodeOffset nNd(1), nCnt = GetNodes().Count(); nNd < nCnt; ++nNd )
    {
        SwTextNode* pTextNode = GetNodes()[ nNd ]->GetTextNode();
        if ( pTextNode ) pTextNode->SetAutoCompleteWordDirty( true );
    }

    for( auto aLayout : aAllLayouts )
        aLayout->SetIdleFlags();
}

const SwFormatINetFormat* SwDoc::FindINetAttr( std::u16string_view rName ) const
{
    ItemSurrogates aSurrogates;
    GetAttrPool().GetItemSurrogates(aSurrogates, RES_TXTATR_INETFMT);
    for (const SfxPoolItem* pItem : aSurrogates)
    {
        auto pFormatItem = dynamic_cast<const SwFormatINetFormat*>(pItem);
        if( !pFormatItem || pFormatItem->GetName() != rName )
            continue;
        const SwTextINetFormat* pTextAttr = pFormatItem->GetTextINetFormat();
        if( !pTextAttr )
            continue;
        const SwTextNode* pTextNd = pTextAttr->GetpTextNode();
        if( pTextNd && &pTextNd->GetNodes() == &GetNodes() )
        {
            return pFormatItem;
        }
    }
    return nullptr;
}

void SwDoc::Summary(SwDoc& rExtDoc, sal_uInt8 nLevel, sal_uInt8 nPara, bool bImpress)
{
    const SwOutlineNodes& rOutNds = GetNodes().GetOutLineNds();
    if (rOutNds.empty())
        return;

    ::StartProgress( STR_STATSTR_SUMMARY, 0, rOutNds.size(), GetDocShell() );
    SwNodeIndex aEndOfDoc( rExtDoc.GetNodes().GetEndOfContent(), -1 );
    for( SwOutlineNodes::size_type i = 0; i < rOutNds.size(); ++i )
    {
        ::SetProgressState( static_cast<tools::Long>(i), GetDocShell() );
        const SwNodeOffset nIndex = rOutNds[ i ]->GetIndex();

        const int nLvl = GetNodes()[ nIndex ]->GetTextNode()->GetAttrOutlineLevel()-1;
        if( nLvl > nLevel )
            continue;
        SwNodeOffset nEndOfs(1);
        sal_uInt8 nWish = nPara;
        SwNodeOffset nNextOutNd = i + 1 < rOutNds.size() ?
            rOutNds[ i + 1 ]->GetIndex() : GetNodes().Count();
        bool bKeep = false;
        while( ( nWish || bKeep ) && nIndex + nEndOfs < nNextOutNd &&
               GetNodes()[ nIndex + nEndOfs ]->IsTextNode() )
        {
            SwTextNode* pTextNode = GetNodes()[ nIndex+nEndOfs ]->GetTextNode();
            if (pTextNode->GetText().getLength() && nWish)
                --nWish;
            bKeep = pTextNode->GetSwAttrSet().GetKeep().GetValue();
            ++nEndOfs;
        }

        SwNodeRange aRange( *rOutNds[ i ], SwNodeOffset(0), *rOutNds[ i ], nEndOfs );
        GetNodes().Copy_( aRange, aEndOfDoc.GetNode() );
    }
    const SwTextFormatColls *pColl = rExtDoc.GetTextFormatColls();
    for( SwTextFormatColls::size_type i = 0; i < pColl->size(); ++i )
        (*pColl)[ i ]->ResetFormatAttr( RES_PAGEDESC, RES_BREAK );
    SwNodeIndex aIndx( rExtDoc.GetNodes().GetEndOfExtras() );
    ++aEndOfDoc;
    while( aIndx < aEndOfDoc )
    {
        bool bDelete = false;
        SwNode *pNode = &aIndx.GetNode();
        if( pNode->IsTextNode() )
        {
            SwTextNode *pNd = pNode->GetTextNode();
            if( pNd->HasSwAttrSet() )
                pNd->ResetAttr( RES_PAGEDESC, RES_BREAK );
            if( bImpress )
            {
                SwTextFormatColl* pMyColl = pNd->GetTextColl();

                const sal_uInt16 nHeadLine = o3tl::narrowing<sal_uInt16>(
                            !pMyColl->IsAssignedToListLevelOfOutlineStyle()
                            ? RES_POOLCOLL_HEADLINE2
                            : RES_POOLCOLL_HEADLINE1 );
                pMyColl = rExtDoc.getIDocumentStylePoolAccess().GetTextCollFromPool( nHeadLine );
                pNd->ChgFormatColl( pMyColl );
            }
            if( !pNd->Len() &&
                pNd->StartOfSectionIndex()+SwNodeOffset(2) < pNd->EndOfSectionIndex() )
            {
                bDelete = true;
                rExtDoc.GetNodes().Delete( aIndx );
            }
        }
        if( !bDelete )
            ++aIndx;
    }
    ::EndProgress( GetDocShell() );
}

namespace
{
void RemoveOrDeleteContents(SwTextNode* pTextNd, IDocumentContentOperations& xOperations)
{
    SwPaM aPam(*pTextNd, 0, *pTextNd, pTextNd->GetText().getLength());

    // Remove hidden paragraph or delete contents:
    // Delete contents if
    // 1. removing the paragraph would result in an empty section or
    // 2. if the paragraph is the last paragraph in the section and
    //    there is no paragraph in front of the paragraph:
    if ((SwNodeOffset(2) == pTextNd->EndOfSectionIndex() - pTextNd->StartOfSectionIndex())
        || (SwNodeOffset(1) == pTextNd->EndOfSectionIndex() - pTextNd->GetIndex()
            && !pTextNd->GetNodes()[pTextNd->GetIndex() - 1]->GetTextNode()))
    {
        xOperations.DeleteRange(aPam);
    }
    else
    {
        aPam.DeleteMark();
        xOperations.DelFullPara(aPam);
    }
}
// Returns if the data was actually modified
bool HandleHidingField(SwFormatField& rFormatField, const SwNodes& rNodes,
                       IDocumentContentOperations& xOperations)
{
    if( !rFormatField.GetTextField() )
        return false;
    SwTextNode* pTextNd = rFormatField.GetTextField()->GetpTextNode();
    if( pTextNd
        && pTextNd->GetpSwpHints() && pTextNd->IsHiddenByParaField()
        && &pTextNd->GetNodes() == &rNodes)
    {
        RemoveOrDeleteContents(pTextNd, xOperations);
        return true;
    }
    return false;
}
}

// The greater the returned value, the more weight has this field type on deciding the final
// paragraph state
int SwDoc::FieldCanHideParaWeight(SwFieldIds eFieldId) const
{
    switch (eFieldId)
    {
        case SwFieldIds::HiddenPara:
            return 20;
        case SwFieldIds::Database:
            return GetDocumentSettingManager().get(DocumentSettingId::EMPTY_DB_FIELD_HIDES_PARA)
                       ? 10
                       : 0;
        default:
            return 0;
    }
}

bool SwDoc::FieldHidesPara(const SwField& rField) const
{
    switch (rField.GetTyp()->Which())
    {
        case SwFieldIds::HiddenPara:
            return static_cast<const SwHiddenParaField&>(rField).IsHidden();
        case SwFieldIds::Database:
            return FieldCanHideParaWeight(SwFieldIds::Database)
                   && rField.ExpandField(true, nullptr).isEmpty();
        default:
            return false;
    }
}

/// Remove the invisible content from the document e.g. hidden areas, hidden paragraphs
// Returns if the data was actually modified
bool SwDoc::RemoveInvisibleContent()
{
    bool bRet = false;
    GetIDocumentUndoRedo().StartUndo( SwUndoId::UI_DELETE_INVISIBLECNTNT, nullptr );

    {
        class FieldTypeGuard : public SwClient
        {
        public:
            explicit FieldTypeGuard(SwFieldType* pType)
                : SwClient(pType)
            {
            }
            const SwFieldType* get() const
            {
                return static_cast<const SwFieldType*>(GetRegisteredIn());
            }
        };
        // Removing some nodes for one SwFieldIds::Database type might remove the type from
        // document's field types, invalidating iterators. So, we need to create own list of
        // matching types prior to processing them.
        std::vector<std::unique_ptr<FieldTypeGuard>> aHidingFieldTypes;
        for (std::unique_ptr<SwFieldType> const & pType : *getIDocumentFieldsAccess().GetFieldTypes())
        {
            if (FieldCanHideParaWeight(pType->Which()))
                aHidingFieldTypes.push_back(std::make_unique<FieldTypeGuard>(pType.get()));
        }
        for (const auto& pTypeGuard : aHidingFieldTypes)
        {
            if (const SwFieldType* pType = pTypeGuard->get())
            {
                std::vector<SwFormatField*> vFields;
                pType->GatherFields(vFields);
                for(auto pFormatField: vFields)
                    bRet |= HandleHidingField(*pFormatField, GetNodes(), getIDocumentContentOperations());
            }
        }
    }

    // Remove any hidden paragraph (hidden text attribute)
    for( SwNodeOffset n = GetNodes().Count(); n; )
    {
        SwTextNode* pTextNd = GetNodes()[ --n ]->GetTextNode();
        if ( pTextNd )
        {
            bool bRemoved = false;
            if ( pTextNd->HasHiddenCharAttribute( true ) )
            {
                bRemoved = true;
                bRet = true;

                if (SwNodeOffset(2) == pTextNd->EndOfSectionIndex() - pTextNd->StartOfSectionIndex())
                {
                    SwFrameFormat *const pFormat = pTextNd->StartOfSectionNode()->GetFlyFormat();
                    if (nullptr != pFormat)
                    {
                        // remove hidden text frame
                        getIDocumentLayoutAccess().DelLayoutFormat(pFormat);
                    }
                    else
                    {
                        // default, remove hidden paragraph
                        RemoveOrDeleteContents(pTextNd, getIDocumentContentOperations());
                    }
                }
                else
                {
                    // default, remove hidden paragraph
                    RemoveOrDeleteContents(pTextNd, getIDocumentContentOperations());
                }
            }
            else if ( pTextNd->HasHiddenCharAttribute( false ) )
            {
                bRemoved = true;
                bRet = true;
                SwScriptInfo::DeleteHiddenRanges( *pTextNd );
            }

            // Footnotes/Frames may have been removed, therefore we have
            // to reset n:
            if ( bRemoved )
            {
                // [n] has to be inside [0 .. GetNodes().Count()] range
                if (n > GetNodes().Count())
                    n = GetNodes().Count();
            }
        }
    }

    {
        // Delete/empty all hidden areas
        o3tl::sorted_vector<SwSectionFormat*> aSectFormats;
        SwSectionFormats& rSectFormats = GetSections();

        for( SwSectionFormats::size_type n = rSectFormats.size(); n; )
        {
            SwSectionFormat* pSectFormat = rSectFormats[ --n ];
            // don't add sections in Undo/Redo
            if( !pSectFormat->IsInNodesArr())
                continue;
            SwSection* pSect = pSectFormat->GetSection();
            if( pSect->CalcHiddenFlag() )
            {
                SwSection* pParent = pSect, *pTmp;
                while( nullptr != (pTmp = pParent->GetParent() ))
                {
                    if( pTmp->IsHiddenFlag() )
                        pSect = pTmp;
                    pParent = pTmp;
                }

                aSectFormats.insert( pSect->GetFormat() );
            }
            if( !pSect->GetCondition().isEmpty() )
            {
                SwSectionData aSectionData( *pSect );
                aSectionData.SetCondition( OUString() );
                aSectionData.SetHidden( false );
                UpdateSection( n, aSectionData );
            }
        }

        auto n = aSectFormats.size();

        if( 0 != n )
        {
            while( n )
            {
                SwSectionFormat* pSectFormat = aSectFormats[ --n ];
                SwSectionNode* pSectNd = pSectFormat->GetSectionNode();
                if( pSectNd )
                {
                    bRet = true;
                    SwPaM aPam( *pSectNd );

                    if( pSectNd->StartOfSectionNode()->StartOfSectionIndex() ==
                        pSectNd->GetIndex() - 1 &&
                        pSectNd->StartOfSectionNode()->EndOfSectionIndex() ==
                        pSectNd->EndOfSectionIndex() + 1 )
                    {
                        // only delete the content
                        SwContentNode* pCNd = SwNodes::GoNext(aPam.GetPoint());
                        aPam.SetMark();
                        aPam.GetPoint()->Assign( *pSectNd->EndOfSectionNode() );
                        pCNd = SwNodes::GoPrevious( aPam.GetPoint() );
                        assert(pCNd); // keep coverity happy
                        aPam.GetPoint()->SetContent( pCNd->Len() );

                        getIDocumentContentOperations().DeleteRange( aPam );
                    }
                    else
                    {
                        // delete the whole section
                        aPam.SetMark();
                        aPam.GetPoint()->Assign( *pSectNd->EndOfSectionNode() );
                        getIDocumentContentOperations().DelFullPara( aPam );
                    }

                }
            }
        }
    }

    if( bRet )
        getIDocumentState().SetModified();
    GetIDocumentUndoRedo().EndUndo( SwUndoId::UI_DELETE_INVISIBLECNTNT, nullptr );
    return bRet;
}

bool SwDoc::HasInvisibleContent() const
{
    std::vector<SwFormatField*> vFields;
    getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::HiddenPara)->GatherFields(vFields);
    if(vFields.size())
        return true;

    // Search for any hidden paragraph (hidden text attribute)
    for( SwNodeOffset n = GetNodes().Count()-SwNodeOffset(1); n; --n)
    {
        SwTextNode* pTextNd = GetNodes()[ n ]->GetTextNode();
        if ( pTextNd &&
             ( pTextNd->HasHiddenCharAttribute( true ) || pTextNd->HasHiddenCharAttribute( false ) ) )
            return true;
    }

    for(auto pSectFormat : GetSections())
    {
        // don't add sections in Undo/Redo
        if( !pSectFormat->IsInNodesArr())
            continue;
        SwSection* pSect = pSectFormat->GetSection();
        if( pSect->IsHidden() )
            return true;
    }
    return false;
}

bool SwDoc::RestoreInvisibleContent()
{
    SwUndoId nLastUndoId(SwUndoId::EMPTY);
    if (GetIDocumentUndoRedo().GetLastUndoInfo(nullptr, & nLastUndoId)
        && (SwUndoId::UI_DELETE_INVISIBLECNTNT == nLastUndoId))
    {
        GetIDocumentUndoRedo().Undo();
        GetIDocumentUndoRedo().ClearRedo();
        return true;
    }
    return false;
}

static bool IsMailMergeField(SwFieldIds fieldId)
{
    switch (fieldId)
    {
        case SwFieldIds::Database: // Mail merge fields
        case SwFieldIds::DatabaseName: // Database name
        case SwFieldIds::HiddenText: // Hidden text may use database fields in condition
        case SwFieldIds::HiddenPara: // Hidden paragraph may use database fields in condition
        case SwFieldIds::DbNextSet: // Moving to next mail merge record
        case SwFieldIds::DbNumSet: // Moving to a specific mail merge record
        case SwFieldIds::DbSetNumber: // Number of current mail merge record
            return true;
        default:
            return false;
    }
}

bool SwDoc::ConvertFieldsToText(SwRootFrame const& rLayout)
{
    bool bRet = false;
    getIDocumentFieldsAccess().LockExpFields();
    GetIDocumentUndoRedo().StartUndo( SwUndoId::UI_REPLACE, nullptr );

    const bool bOnlyConvertDBFields
        = officecfg::Office::Writer::FormLetter::ConvertToTextOnlyMMFields::get();

    const SwFieldTypes* pMyFieldTypes = getIDocumentFieldsAccess().GetFieldTypes();
    const SwFieldTypes::size_type nCount = pMyFieldTypes->size();
    //go backward, field types are removed
    for(SwFieldTypes::size_type nType = nCount; nType > 0; --nType)
    {
        const SwFieldType *pCurType = (*pMyFieldTypes)[nType - 1].get();

        if ( SwFieldIds::Postit == pCurType->Which() )
            continue;

        if (bOnlyConvertDBFields && !IsMailMergeField(pCurType->Which()))
            continue;

        std::vector<SwFormatField*> vFieldFormats;
        pCurType->GatherFields(vFieldFormats, false);
        for(const auto& rpFieldFormat : vFieldFormats)
        {
            const SwTextField *pTextField = rpFieldFormat->GetTextField();
            // skip fields that are currently not in the document
            // e.g. fields in undo or redo array

            bool bSkip = !pTextField ||
                         !pTextField->GetpTextNode()->GetNodes().IsDocNodes();
            if (bSkip)
                continue;

            bool bInHeaderFooter = IsInHeaderFooter(*pTextField->GetpTextNode());
            const SwFormatField& rFormatField = pTextField->GetFormatField();
            const SwField*  pField = rFormatField.GetField();

            //#i55595# some fields have to be excluded in headers/footers
            SwFieldIds nWhich = pField->GetTyp()->Which();
            if(!bInHeaderFooter ||
                    (nWhich != SwFieldIds::PageNumber &&
                    nWhich != SwFieldIds::Chapter &&
                    nWhich != SwFieldIds::GetExp&&
                    nWhich != SwFieldIds::SetExp&&
                    nWhich != SwFieldIds::Input&&
                    nWhich != SwFieldIds::RefPageGet&&
                    nWhich != SwFieldIds::RefPageSet))
            {
                OUString sText = pField->ExpandField(true, &rLayout);

                // database fields should not convert their command into text
                if( SwFieldIds::Database == pCurType->Which() && !static_cast<const SwDBField*>(pField)->IsInitialized())
                    sText.clear();

                SwPaM aInsertPam(*pTextField->GetpTextNode(), pTextField->GetStart());
                aInsertPam.SetMark();

                // go to the end of the field
                const SwTextField *pFieldAtEnd = sw::DocumentFieldsManager::GetTextFieldAtPos(*aInsertPam.End());
                if (pFieldAtEnd && pFieldAtEnd->Which() == RES_TXTATR_INPUTFIELD)
                {
                    SwPosition &rEndPos = *aInsertPam.GetPoint();
                    rEndPos.SetContent( SwCursorShell::EndOfInputFieldAtPos( *aInsertPam.End() ) );
                }
                else
                {
                    aInsertPam.Move();
                }

                // first insert the text after field to keep the field's attributes,
                // then delete the field
                if (!sText.isEmpty())
                {
                    // to keep the position after insert
                    SwPaM aDelPam( *aInsertPam.GetMark(), *aInsertPam.GetPoint() );
                    aDelPam.Move( fnMoveBackward );
                    aInsertPam.DeleteMark();

                    getIDocumentContentOperations().InsertString( aInsertPam, sText );

                    aDelPam.Move();
                    // finally remove the field
                    getIDocumentContentOperations().DeleteAndJoin( aDelPam );
                }
                else
                {
                    getIDocumentContentOperations().DeleteAndJoin( aInsertPam );
                }

                bRet = true;
            }
        }
    }

    if( bRet )
        getIDocumentState().SetModified();
    GetIDocumentUndoRedo().EndUndo( SwUndoId::UI_REPLACE, nullptr );
    getIDocumentFieldsAccess().UnlockExpFields();
    return bRet;

}

bool SwDoc::IsInsTableFormatNum() const
{
    return SW_MOD()->IsInsTableFormatNum(GetDocumentSettingManager().get(DocumentSettingId::HTML_MODE));
}

bool SwDoc::IsInsTableChangeNumFormat() const
{
    return SW_MOD()->IsInsTableChangeNumFormat(GetDocumentSettingManager().get(DocumentSettingId::HTML_MODE));
}

bool SwDoc::IsInsTableAlignNum() const
{
    return SW_MOD()->IsInsTableAlignNum(GetDocumentSettingManager().get(DocumentSettingId::HTML_MODE));
}

bool SwDoc::IsSplitVerticalByDefault() const
{
    return SW_MOD()->IsSplitVerticalByDefault(GetDocumentSettingManager().get(DocumentSettingId::HTML_MODE));
}

void SwDoc::SetSplitVerticalByDefault(bool value)
{
    SW_MOD()->SetSplitVerticalByDefault(GetDocumentSettingManager().get(DocumentSettingId::HTML_MODE), value);
}

/// Set up the InsertDB as Undo table
void SwDoc::AppendUndoForInsertFromDB( const SwPaM& rPam, bool bIsTable )
{
    if( bIsTable )
    {
        const SwTableNode* pTableNd = rPam.GetPoint()->GetNode().FindTableNode();
        if( pTableNd )
        {
            std::unique_ptr<SwUndoCpyTable> pUndo(new SwUndoCpyTable(*this));
            pUndo->SetTableSttIdx( pTableNd->GetIndex() );
            GetIDocumentUndoRedo().AppendUndo( std::move(pUndo) );
        }
    }
    else if( rPam.HasMark() )
    {
        std::unique_ptr<SwUndoCpyDoc> pUndo(new SwUndoCpyDoc( rPam ));
        pUndo->SetInsertRange( rPam, false );
        GetIDocumentUndoRedo().AppendUndo( std::move(pUndo) );
    }
}

void SwDoc::ChangeTOX(SwTOXBase & rTOX, const SwTOXBase & rNew)
{
    assert(dynamic_cast<const SwTOXBaseSection*>(&rTOX));
    SwTOXBaseSection& rTOXSect(static_cast<SwTOXBaseSection&>(rTOX));

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo(
            std::make_unique<SwUndoTOXChange>(*this, rTOXSect, rNew));
    }

    rTOX = rNew;

    // note: do not Update the ToX here - the caller will do it, with a ViewShell!
}

OUString SwDoc::GetPaMDescr(const SwPaM & rPam)
{
    if (&rPam.GetPointNode() == &rPam.GetMarkNode())
    {
        SwTextNode * pTextNode = rPam.GetPointNode().GetTextNode();

        if (nullptr != pTextNode)
        {
            const sal_Int32 nStart = rPam.Start()->GetContentIndex();
            const sal_Int32 nEnd = rPam.End()->GetContentIndex();

            return SwResId(STR_START_QUOTE)
                + ShortenString(pTextNode->GetText().copy(nStart, nEnd - nStart),
                                nUndoStringLength,
                                SwResId(STR_LDOTS))
                + SwResId(STR_END_QUOTE);
        }
    }
    else
    {
        return SwResId(STR_PARAGRAPHS);
    }

    return "??";
}

bool SwDoc::ContainsHiddenChars() const
{
    for( SwNodeOffset n = GetNodes().Count(); n; )
    {
        SwNode* pNd = GetNodes()[ --n ];
        if ( pNd->IsTextNode() && pNd->GetTextNode()->HasHiddenCharAttribute( false ) )
            return true;
    }

    return false;
}

std::shared_ptr<SwUnoCursor> SwDoc::CreateUnoCursor( const SwPosition& rPos, bool bTableCursor )
{
    std::shared_ptr<SwUnoCursor> pNew;
    if( bTableCursor )
        pNew = std::make_shared<SwUnoTableCursor>(rPos);
    else
        pNew = std::make_shared<SwUnoCursor>(rPos);

    mvUnoCursorTable.push_back( pNew );
    return pNew;
}

void SwDoc::ChkCondColls()
{
     for (SwTextFormatColls::size_type n = 0; n < mpTextFormatCollTable->size(); ++n)
     {
        SwTextFormatColl *pColl = (*mpTextFormatCollTable)[n];
        if (RES_CONDTXTFMTCOLL == pColl->Which())
            pColl->CallSwClientNotify( SwAttrHint() );
     }
}

uno::Reference< script::vba::XVBAEventProcessor > const &
SwDoc::GetVbaEventProcessor()
{
    return mxVbaEvents;
}

void SwDoc::SetVbaEventProcessor()
{
#if HAVE_FEATURE_SCRIPTING
    if (mpDocShell && ooo::vba::isAlienWordDoc(*mpDocShell))
    {
        try
        {
            uno::Reference< frame::XModel > xModel( mpDocShell->GetModel(), uno::UNO_SET_THROW );
            uno::Sequence< uno::Any > aArgs{ uno::Any(xModel) };
            mxVbaEvents.set( ooo::vba::createVBAUnoAPIServiceWithArgs( mpDocShell, "com.sun.star.script.vba.VBATextEventProcessor" , aArgs ), uno::UNO_QUERY_THROW );
        }
        catch( uno::Exception& )
        {
        }
    }
#endif
}

void SwDoc::SetMissingDictionaries( bool bIsMissing )
{
    if (!bIsMissing)
        meDictionaryMissing = MissingDictionary::False;
    else if (meDictionaryMissing == MissingDictionary::Undefined)
        meDictionaryMissing = MissingDictionary::True;
};

void SwDoc::SetLanguage(const LanguageType eLang, const sal_uInt16 nId)
{
    mpAttrPool->SetUserDefaultItem(SvxLanguageItem(eLang, nId));
}

bool SwDoc::HasParagraphDirectFormatting(const SwPosition& rPos)
{
    rtl::Reference<SwXTextRange> xRange(SwXTextRange::CreateXTextRange(rPos.GetDoc(), rPos,
                                                                           &rPos));
    uno::Reference<container::XEnumeration> xParaEnum = xRange->createEnumeration();
    uno::Reference<text::XTextRange> xThisParagraphRange(xParaEnum->nextElement(), uno::UNO_QUERY);
    if (xThisParagraphRange.is())
    {
        const std::vector<OUString> aHiddenProperties{ UNO_NAME_RSID,
                    UNO_NAME_PARA_IS_NUMBERING_RESTART,
                    UNO_NAME_PARA_STYLE_NAME,
                    UNO_NAME_PARA_CONDITIONAL_STYLE_NAME,
                    UNO_NAME_PAGE_STYLE_NAME,
                    UNO_NAME_NUMBERING_START_VALUE,
                    UNO_NAME_NUMBERING_IS_NUMBER,
                    UNO_NAME_PARA_CONTINUEING_PREVIOUS_SUB_TREE,
                    UNO_NAME_CHAR_STYLE_NAME,
                    UNO_NAME_NUMBERING_LEVEL,
                    UNO_NAME_SORTED_TEXT_ID,
                    UNO_NAME_PARRSID,
                    UNO_NAME_CHAR_COLOR_THEME,
                    UNO_NAME_CHAR_COLOR_TINT_OR_SHADE };

        SfxItemPropertySet const& rPropSet(*aSwMapProvider.GetPropertySet(
                                               PROPERTY_MAP_PARA_AUTO_STYLE));
        SfxItemPropertyMap const& rMap(rPropSet.getPropertyMap());

        uno::Reference<beans::XPropertySet> xPropertySet(xThisParagraphRange,
                                                         uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertyState> xPropertyState(xThisParagraphRange,
                                                             uno::UNO_QUERY_THROW);
        const uno::Sequence<beans::Property> aProperties
                = xPropertySet->getPropertySetInfo()->getProperties();
        for (const beans::Property& rProperty : aProperties)
        {
            const OUString& rPropName = rProperty.Name;
            if (!rMap.hasPropertyByName(rPropName))
                continue;
            if (std::find(aHiddenProperties.begin(), aHiddenProperties.end(), rPropName)
                    != aHiddenProperties.end())
                continue;
            if (xPropertyState->getPropertyState(rPropName) == beans::PropertyState_DIRECT_VALUE)
                return true;
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
