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
#include <tools/globname.hxx>
#include <svx/svxids.hrc>
#include <rtl/random.h>

#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/ForbiddenCharacters.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <comphelper/string.hxx>
#include <tools/urlobj.hxx>
#include <tools/poly.hxx>
#include <tools/multisel.hxx>
#include <rtl/ustring.hxx>
#include <vcl/virdev.hxx>
#include <svl/itemiter.hxx>
#include <svl/poolitem.hxx>
#include <unotools/syslocale.hxx>
#include <sfx2/printer.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/charsetcoloritem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <sfx2/linkmgr.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/pbinitem.hxx>
#include <unotools/charclass.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/timer.hxx>

#include <swatrset.hxx>
#include <swmodule.hxx>
#include <fmtpdsc.hxx>
#include <fmtanchr.hxx>
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
#include <swtable.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <swundo.hxx>
#include <UndoCore.hxx>
#include <UndoInsert.hxx>
#include <UndoSplitMove.hxx>
#include <UndoTable.hxx>
#include <pagedesc.hxx>
#include <ndole.hxx>
#include <ndgrf.hxx>
#include <rolbck.hxx>
#include <doctxm.hxx>
#include <grfatr.hxx>
#include <poolfmt.hxx>
#include <mvsave.hxx>
#include <SwGrammarMarkUp.hxx>
#include <scriptinfo.hxx>
#include <acorrect.hxx>
#include <mdiexp.hxx>
#include <docstat.hxx>
#include <docary.hxx>
#include <redline.hxx>
#include <fldupde.hxx>
#include <swbaslnk.hxx>
#include <printdata.hxx>
#include <cmdid.h>
#include <statstr.hrc>
#include <comcore.hrc>
#include <SwUndoTOXChange.hxx>
#include <unocrsr.hxx>
#include <docsh.hxx>
#include <viewopt.hxx>
#include <docfld.hxx>
#include <docufld.hxx>
#include <viewsh.hxx>
#include <shellres.hxx>
#include <txtfrm.hxx>
#include <attrhint.hxx>
#include <view.hxx>

#include <wdocsh.hxx>
#include <prtopt.hxx>
#include <wrtsh.hxx>

#include <vector>
#include <map>
#include <osl/diagnose.h>
#include <osl/interlck.h>
#include <vbahelper/vbaaccesshelper.hxx>
#include <calbck.hxx>

/* @@@MAINTAINABILITY-HORROR@@@
   Probably unwanted dependency on SwDocShell
*/
#include <layouter.hxx>

using namespace ::com::sun::star;

/* IInterface */
sal_Int32 SwDoc::acquire()
{
    OSL_ENSURE(mReferenceCount >= 0, "Negative reference count detected! This is a sign for unbalanced acquire/release calls.");
    return osl_atomic_increment(&mReferenceCount);
}

sal_Int32 SwDoc::release()
{
    OSL_PRECOND(mReferenceCount >= 1, "Object is already released! Releasing it again leads to a negative reference count.");
    return osl_atomic_decrement(&mReferenceCount);
}

sal_Int32 SwDoc::getReferenceCount() const
{
    OSL_ENSURE(mReferenceCount >= 0, "Negative reference count detected! This is a sign for unbalanced acquire/release calls.");
    return mReferenceCount;
}

::sw::MetaFieldManager & SwDoc::GetMetaFieldManager()
{
    return *m_pMetaFieldManager;
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
        static rtlRandomPool aPool = rtl_random_createPool();
        rtl_random_getBytes( aPool, &nIncrease, sizeof ( nIncrease ) );
        nIncrease &= ( 1<<17 ) - 1;
        nIncrease++; // make sure the new rsid is not the same
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
    }
    getIDocumentFieldsAccess().GetSysFieldType(RES_DBNAMEFLD)->UpdateFields();
}

struct _PostItField : public _SetGetExpField
{
    _PostItField( const SwNodeIndex& rNdIdx, const SwTextField* pField,  const SwIndex* pIdx = nullptr )
        : _SetGetExpField( rNdIdx, pField, pIdx ) {}

    sal_uInt16 GetPageNo( const StringRangeEnumerator &rRangeEnum,
            const std::set< sal_Int32 > &rPossiblePages,
            sal_uInt16& rVirtPgNo, sal_uInt16& rLineNo );

    const SwPostItField* GetPostIt() const
    {
        return static_cast<const SwPostItField*>( GetTextField()->GetFormatField().GetField() );
    }
};

sal_uInt16 _PostItField::GetPageNo(
    const StringRangeEnumerator &rRangeEnum,
    const std::set< sal_Int32 > &rPossiblePages,
    /* out */ sal_uInt16& rVirtPgNo, /* out */ sal_uInt16& rLineNo )
{
    //Problem: If a PostItField is contained in a Node that is represented
    //by more than one layout instance,
    //we have to decide whether it should be printed once or n-times.
    //Probably only once. For the page number we don't select a random one,
    //but the PostIt's first occurrence in the selected area.
    rVirtPgNo = 0;
    const sal_Int32 nPos = GetContent();
    SwIterator<SwTextFrame,SwTextNode> aIter( GetTextField()->GetTextNode() );
    for( SwTextFrame* pFrame = aIter.First(); pFrame;  pFrame = aIter.Next() )
    {
        if( pFrame->GetOfst() > nPos ||
            (pFrame->HasFollow() && pFrame->GetFollow()->GetOfst() <= nPos) )
            continue;
        sal_uInt16 nPgNo = pFrame->GetPhyPageNum();
        if( rRangeEnum.hasValue( nPgNo, &rPossiblePages ))
        {
            rLineNo = (sal_uInt16)(pFrame->GetLineCount( nPos ) +
                      pFrame->GetAllLines() - pFrame->GetThisLines());
            rVirtPgNo = pFrame->GetVirtPageNum();
            return nPgNo;
        }
    }
    return 0;
}

bool sw_GetPostIts(
    IDocumentFieldsAccess* pIDFA,
    _SetGetExpFields * pSrtLst )
{
    bool bHasPostIts = false;

    SwFieldType* pFieldType = pIDFA->GetSysFieldType( RES_POSTITFLD );
    OSL_ENSURE( pFieldType, "no PostItType ? ");

    if( pFieldType->HasWriterListeners() )
    {
        // Found modify object; insert all fields into the array
        SwIterator<SwFormatField,SwFieldType> aIter( *pFieldType );
        for( SwFormatField* pField = aIter.First(); pField;  pField = aIter.Next() )
        {
            const SwTextField* pTextField;
            if( nullptr != ( pTextField = pField->GetTextField() ) &&
                pTextField->GetTextNode().GetNodes().IsDocNodes() )
            {
                bHasPostIts = true;
                if (pSrtLst)
                {
                    SwNodeIndex aIdx( pTextField->GetTextNode() );
                    _PostItField* pNew = new _PostItField( aIdx, pTextField );
                    pSrtLst->insert( pNew );
                }
                else
                    break;  // we just wanted to check for the existence of postits ...
            }
        }
    }

    return bHasPostIts;
}

static void lcl_FormatPostIt(
    IDocumentContentOperations* pIDCO,
    SwPaM& aPam,
    const SwPostItField* pField,
    bool bNewPage, bool bIsFirstPostIt,
    sal_uInt16 nPageNo, sal_uInt16 nLineNo )
{
    static char const sTmp[] = " : ";

    OSL_ENSURE( SwViewShell::GetShellRes(), "missing ShellRes" );

    if (bNewPage)
    {
        pIDCO->InsertPoolItem( aPam, SvxFormatBreakItem( SVX_BREAK_PAGE_AFTER, RES_BREAK ) );
        pIDCO->SplitNode( *aPam.GetPoint(), false );
    }
    else if (!bIsFirstPostIt)
    {
        // add an empty line between different notes
        pIDCO->SplitNode( *aPam.GetPoint(), false );
        pIDCO->SplitNode( *aPam.GetPoint(), false );
    }

    OUString aStr( SwViewShell::GetShellRes()->aPostItPage );
    aStr += sTmp;

    aStr += OUString::number( nPageNo );
    aStr += " ";
    if( nLineNo )
    {
        aStr += SwViewShell::GetShellRes()->aPostItLine;
        aStr += sTmp;
        aStr += OUString::number( nLineNo );
        aStr += " ";
    }
    aStr += SwViewShell::GetShellRes()->aPostItAuthor;
    aStr += sTmp;
    aStr += pField->GetPar1();
    aStr += " ";
    SvtSysLocale aSysLocale;
    aStr += /*(LocaleDataWrapper&)*/aSysLocale.GetLocaleData().getDate( pField->GetDate() );
    pIDCO->InsertString( aPam, aStr );

    pIDCO->SplitNode( *aPam.GetPoint(), false );
    aStr = pField->GetPar2();
#if defined( WNT )
    // Throw out all CR in Windows
    aStr = comphelper::string::remove(aStr, '\r');
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
    std::set< sal_Int32 > &rValidPages = rData.GetValidPagesSet();
    rValidPages.clear();

    sal_Int32 nPageNum = 1;
    const SwPageFrame *pStPage = dynamic_cast<const SwPageFrame*>( rLayout.Lower() );
    while (pStPage && nPageNum <= nDocPageCount)
    {
        const bool bPrintThisPage =
            ( (bPrintRightPages && pStPage->OnRightPage()) ||
              (bPrintLeftPages && !pStPage->OnRightPage()) ) &&
            ( bPrintEmptyPages || pStPage->Frame().Height() );

        if (bPrintThisPage)
        {
            rValidPages.insert( nPageNum );
            rPrinterPaperTrays[ nPageNum ] = lcl_GetPaperBin( pStPage );
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
            aPageRange = rOptions.getStringValue( "PageRange", OUString() );
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
    rData.SetPageRange( aPageRange );

    // get vector of pages to print according to PageRange and valid pages set from above
    // (result may be an empty vector, for example if the range string is not correct)
    StringRangeEnumerator::getRangesFromString(
            aPageRange, rData.GetPagesToPrint(),
            1, nDocPageCount, 0, &rData.GetValidPagesSet() );
}

void SwDoc::UpdatePagesForPrintingWithPostItData(
    /* out */ SwRenderData &rData,
    const SwPrintUIOptions &rOptions,
    bool /*bIsPDFExport*/,
    sal_Int32 nDocPageCount )
{

    SwPostItMode nPostItMode = static_cast<SwPostItMode>( rOptions.getIntValue( "PrintAnnotationMode", 0 ) );
    OSL_ENSURE(nPostItMode == SwPostItMode::NONE || rData.HasPostItData(),
            "print post-its without post-it data?" );
    const _SetGetExpFields::size_type nPostItCount =
        rData.HasPostItData() ? rData.m_pPostItFields->size() : 0;
    if (nPostItMode != SwPostItMode::NONE && nPostItCount > 0)
    {
        SET_CURR_SHELL( rData.m_pPostItShell.get() );

        // clear document and move to end of it
        SwDoc & rPostItDoc(*rData.m_pPostItShell->GetDoc());
        SwPaM aPam(rPostItDoc.GetNodes().GetEndOfContent());
        aPam.Move( fnMoveBackward, fnGoDoc );
        aPam.SetMark();
        aPam.Move( fnMoveForward, fnGoDoc );
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
        sal_uInt16 nVirtPg = 0, nLineNo = 0, nLastPageNum = 0, nPhyPageNum = 0;
        bool bIsFirstPostIt = true;
        for (_SetGetExpFields::size_type i = 0; i < nPostItCount; ++i)
        {
            _PostItField& rPostIt = static_cast<_PostItField&>(*(*rData.m_pPostItFields)[ i ]);
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
            const SwPageFrame * pPageFrame = static_cast<SwPageFrame*>(rData.m_pPostItShell->GetLayout()->Lower());
            while( pPageFrame && nPageNum < nPostItDocPageCount )
            {
                OSL_ENSURE( pPageFrame, "Empty page frame. How are we going to print this?" );
                ++nPageNum;
                // negative page number indicates page is from the post-it doc
                rData.GetPagesToPrint().push_back( -nPageNum );
                OSL_ENSURE( pPageFrame, "pPageFrame is NULL!" );
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
                ::std::map<sal_Int32, sal_Int32>::const_iterator const iter(
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
}

void SwDoc::CalculatePagePairsForProspectPrinting(
    const SwRootFrame& rLayout,
    /* out */ SwRenderData &rData,
    const SwPrintUIOptions &rOptions,
    sal_Int32 nDocPageCount )
{
    std::map< sal_Int32, sal_Int32 > &rPrinterPaperTrays = rData.GetPrinterPaperTrays();
    std::set< sal_Int32 > &rValidPagesSet = rData.GetValidPagesSet();
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
        aPageRange = rOptions.getStringValue( "PageRange", OUString() );
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
        OSL_ENSURE( pPageFrame, "Empty page frame. How are we going to print this?" );
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
    for ( std::vector< sal_Int32 >::size_type i = 0; i < aPagesToPrint.size(); ++i)
    {
        const sal_Int32 nPage = aPagesToPrint[i];
        const SwPageFrame *pFrame = validStartFrames[ nPage ];
        aVec.push_back( pFrame );
    }

    // just one page is special ...
    if ( 1 == aVec.size() )
        aVec.insert( aVec.begin() + 1, nullptr ); // insert a second empty page
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
        rPagePairs.push_back( std::pair< sal_Int32, sal_Int32 >(nFirst, nSecond) );

        nSPg = nSPg + nStep;
        nEPg = nEPg - nStep;
    }
    OSL_ENSURE( size_t(nCntPage) == rPagePairs.size(), "size mismatch for number of page pairs" );

    // luckily prospect printing does not make use of post-its so far,
    // thus we are done here.
}

/// @return the reference in the doc for the name
const SwFormatRefMark* SwDoc::GetRefMark( const OUString& rName ) const
{
    sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_TXTATR_REFMARK );
    for( sal_uInt32 n = 0; n < nMaxItems; ++n )
    {
        const SfxPoolItem* pItem;
        if( nullptr == (pItem = GetAttrPool().GetItem2( RES_TXTATR_REFMARK, n ) ))
            continue;

        const SwFormatRefMark* pFormatRef = static_cast<const SwFormatRefMark*>(pItem);
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
    const SwTextRefMark* pTextRef;
    const SwFormatRefMark* pRet = nullptr;

    sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_TXTATR_REFMARK );
    sal_uInt32 nCount = 0;
    for( sal_uInt32 n = 0; n < nMaxItems; ++n )
    {
        const SfxPoolItem* pItem;

        if( nullptr != (pItem = GetAttrPool().GetItem2( RES_TXTATR_REFMARK, n )) &&
            nullptr != (pTextRef = static_cast<const SwFormatRefMark*>(pItem)->GetTextRefMark()) &&
            &pTextRef->GetTextNode().GetNodes() == &GetNodes() )
        {
            if(nCount == nIndex)
            {
                pRet = static_cast<const SwFormatRefMark*>(pItem);
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
    const SwTextRefMark* pTextRef;

    const sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_TXTATR_REFMARK );
    sal_uInt16 nCount = 0;
    for( sal_uInt32 n = 0; n < nMaxItems; ++n )
    {
        const SfxPoolItem* pItem;

        if( nullptr != (pItem = GetAttrPool().GetItem2( RES_TXTATR_REFMARK, n )) &&
            nullptr != (pTextRef = static_cast<const SwFormatRefMark*>(pItem)->GetTextRefMark()) &&
            &pTextRef->GetTextNode().GetNodes() == &GetNodes() )
        {
            if( pNames )
            {
                OUString aTmp(static_cast<const SwFormatRefMark*>(pItem)->GetRefName());
                pNames->insert(pNames->begin() + nCount, aTmp);
            }
            ++nCount;
        }
    }

    return nCount;
}

static bool lcl_SpellAndGrammarAgain( const SwNodePtr& rpNd, void* pArgs )
{
    SwTextNode *pTextNode = rpNd->GetTextNode();
    bool bOnlyWrong = *static_cast<sal_Bool*>(pArgs);
    if( pTextNode )
    {
        if( bOnlyWrong )
        {
            if( pTextNode->GetWrong() &&
                pTextNode->GetWrong()->InvalidateWrong() )
                pTextNode->SetWrongDirty(SwTextNode::WrongState::TODO);
            if( pTextNode->GetGrammarCheck() &&
                pTextNode->GetGrammarCheck()->InvalidateWrong() )
                pTextNode->SetGrammarCheckDirty( true );
        }
        else
        {
            pTextNode->SetWrongDirty(SwTextNode::WrongState::TODO);
            if( pTextNode->GetWrong() )
                pTextNode->GetWrong()->SetInvalid( 0, COMPLETE_STRING );
            pTextNode->SetGrammarCheckDirty( true );
            if( pTextNode->GetGrammarCheck() )
                pTextNode->GetGrammarCheck()->SetInvalid( 0, COMPLETE_STRING );
        }
    }
    return true;
}

static bool lcl_CheckSmartTagsAgain( const SwNodePtr& rpNd, void*  )
{
    SwTextNode *pTextNode = rpNd->GetTextNode();
    if( pTextNode )
    {
        pTextNode->SetSmartTagDirty( true );
        if( pTextNode->GetSmartTags() )
        {
            pTextNode->SetSmartTags( nullptr );
        }
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
    std::set<SwRootFrame*> aAllLayouts = GetAllLayouts();
    OSL_ENSURE( getIDocumentLayoutAccess().GetCurrentLayout(), "SpellAgain: Where's my RootFrame?" );
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
    if( pTmpRoot )
    {
        std::set<SwRootFrame*> aAllLayouts = GetAllLayouts();
        for( auto aLayout : aAllLayouts )
            aLayout->AllInvalidateAutoCompleteWords();
        for( sal_uLong nNd = 1, nCnt = GetNodes().Count(); nNd < nCnt; ++nNd )
        {
            SwTextNode* pTextNode = GetNodes()[ nNd ]->GetTextNode();
            if ( pTextNode ) pTextNode->SetAutoCompleteWordDirty( true );
        }

        for( auto aLayout : aAllLayouts )
            aLayout->SetIdleFlags();
    }
}

const SwFormatINetFormat* SwDoc::FindINetAttr( const OUString& rName ) const
{
    const SwFormatINetFormat* pItem;
    const SwTextINetFormat* pTextAttr;
    const SwTextNode* pTextNd;
    sal_uInt32 n, nMaxItems = GetAttrPool().GetItemCount2( RES_TXTATR_INETFMT );
    for( n = 0; n < nMaxItems; ++n )
    {
        pItem = static_cast<const SwFormatINetFormat*>( GetAttrPool().GetItem2( RES_TXTATR_INETFMT, n ) );
        if( nullptr != pItem &&
            pItem->GetName() == rName &&
            nullptr != ( pTextAttr = pItem->GetTextINetFormat()) &&
            nullptr != ( pTextNd = pTextAttr->GetpTextNode() ) &&
            &pTextNd->GetNodes() == &GetNodes() )
        {
            return pItem;
        }
    }
    return nullptr;
}

void SwDoc::Summary( SwDoc* pExtDoc, sal_uInt8 nLevel, sal_uInt8 nPara, bool bImpress )
{
    const SwOutlineNodes& rOutNds = GetNodes().GetOutLineNds();
    if( pExtDoc && !rOutNds.empty() )
    {
        ::StartProgress( STR_STATSTR_SUMMARY, 0, rOutNds.size(), GetDocShell() );
        SwNodeIndex aEndOfDoc( pExtDoc->GetNodes().GetEndOfContent(), -1 );
        for( SwOutlineNodes::size_type i = 0; i < rOutNds.size(); ++i )
        {
            ::SetProgressState( static_cast<long>(i), GetDocShell() );
            const sal_uLong nIndex = rOutNds[ i ]->GetIndex();

            const int nLvl = GetNodes()[ nIndex ]->GetTextNode()->GetAttrOutlineLevel()-1;
            if( nLvl > nLevel )
                continue;
            long nEndOfs = 1;
            sal_uInt8 nWish = nPara;
            sal_uLong nNextOutNd = i + 1 < rOutNds.size() ?
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

            SwNodeRange aRange( *rOutNds[ i ], 0, *rOutNds[ i ], nEndOfs );
            GetNodes()._Copy( aRange, aEndOfDoc );
        }
        const SwTextFormatColls *pColl = pExtDoc->GetTextFormatColls();
        for( SwTextFormatColls::size_type i = 0; i < pColl->size(); ++i )
            (*pColl)[ i ]->ResetFormatAttr( RES_PAGEDESC, RES_BREAK );
        SwNodeIndex aIndx( pExtDoc->GetNodes().GetEndOfExtras() );
        ++aEndOfDoc;
        while( aIndx < aEndOfDoc )
        {
            SwNode *pNode;
            bool bDelete = false;
            if( (pNode = &aIndx.GetNode())->IsTextNode() )
            {
                SwTextNode *pNd = pNode->GetTextNode();
                if( pNd->HasSwAttrSet() )
                    pNd->ResetAttr( RES_PAGEDESC, RES_BREAK );
                if( bImpress )
                {
                    SwTextFormatColl* pMyColl = pNd->GetTextColl();

                    const sal_uInt16 nHeadLine = static_cast<sal_uInt16>(
                                !pMyColl->IsAssignedToListLevelOfOutlineStyle()
                                ? RES_POOLCOLL_HEADLINE2
                                : RES_POOLCOLL_HEADLINE1 );
                    pMyColl = pExtDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( nHeadLine );
                    pNd->ChgFormatColl( pMyColl );
                }
                if( !pNd->Len() &&
                    pNd->StartOfSectionIndex()+2 < pNd->EndOfSectionIndex() )
                {
                    bDelete = true;
                    pExtDoc->GetNodes().Delete( aIndx );
                }
            }
            if( !bDelete )
                ++aIndx;
        }
        ::EndProgress( GetDocShell() );
    }
}

/// Remove the invisible content from the document e.g. hidden areas, hidden paragraphs
bool SwDoc::RemoveInvisibleContent()
{
    bool bRet = false;
    GetIDocumentUndoRedo().StartUndo( UNDO_UI_DELETE_INVISIBLECNTNT, nullptr );

    {
        SwTextNode* pTextNd;
        SwIterator<SwFormatField,SwFieldType> aIter( *getIDocumentFieldsAccess().GetSysFieldType( RES_HIDDENPARAFLD )  );
        for( SwFormatField* pFormatField = aIter.First(); pFormatField;  pFormatField = aIter.Next() )
        {
            if( pFormatField->GetTextField() &&
                nullptr != ( pTextNd = pFormatField->GetTextField()->GetpTextNode() ) &&
                pTextNd->GetpSwpHints() && pTextNd->HasHiddenParaField() &&
                &pTextNd->GetNodes() == &GetNodes() )
            {
                bRet = true;
                SwPaM aPam(*pTextNd, 0, *pTextNd, pTextNd->GetText().getLength());

                // Remove hidden paragraph or delete contents:
                // Delete contents if
                // 1. removing the paragraph would result in an empty section or
                // 2. if the paragraph is the last paragraph in the section and
                //    there is no paragraph in front of the paragraph:
                if ( ( 2 == pTextNd->EndOfSectionIndex() - pTextNd->StartOfSectionIndex() ) ||
                     ( 1 == pTextNd->EndOfSectionIndex() - pTextNd->GetIndex() &&
                       !GetNodes()[ pTextNd->GetIndex() - 1 ]->GetTextNode() ) )
                {
                    getIDocumentContentOperations().DeleteRange( aPam );
                }
                else
                {
                    aPam.DeleteMark();
                    getIDocumentContentOperations().DelFullPara( aPam );
                }
            }
        }
    }

    // Remove any hidden paragraph (hidden text attribute)
    for( sal_uLong n = GetNodes().Count(); n; )
    {
        SwTextNode* pTextNd = GetNodes()[ --n ]->GetTextNode();
        if ( pTextNd )
        {
            bool bRemoved = false;
            SwPaM aPam(*pTextNd, 0, *pTextNd, pTextNd->GetText().getLength());
            if ( pTextNd->HasHiddenCharAttribute( true ) )
            {
                bRemoved = true;
                bRet = true;

                // Remove hidden paragraph or delete contents:
                // Delete contents if
                // 1. removing the paragraph would result in an empty section or
                // 2. if the paragraph is the last paragraph in the section and
                //    there is no paragraph in front of the paragraph:
                if ( ( 2 == pTextNd->EndOfSectionIndex() - pTextNd->StartOfSectionIndex() ) ||
                     ( 1 == pTextNd->EndOfSectionIndex() - pTextNd->GetIndex() &&
                       !GetNodes()[ pTextNd->GetIndex() - 1 ]->GetTextNode() ) )
                {
                    getIDocumentContentOperations().DeleteRange( aPam );
                }
                else
                {
                    aPam.DeleteMark();
                    getIDocumentContentOperations().DelFullPara( aPam );
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
                n = aPam.GetPoint()->nNode.GetIndex();
        }
    }

    {
        // Delete/empty all hidden areas
        SwSectionFormats aSectFormats;
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

                SwSectionFormats::iterator it = std::find(
                        aSectFormats.begin(), aSectFormats.end(), pSect->GetFormat() );
                if (it == aSectFormats.end())
                    aSectFormats.insert( aSectFormats.begin(), pSect->GetFormat() );
            }
            if( !pSect->GetCondition().isEmpty() )
            {
                SwSectionData aSectionData( *pSect );
                aSectionData.SetCondition( OUString() );
                aSectionData.SetHidden( false );
                UpdateSection( n, aSectionData );
            }
        }

        SwSectionFormats::size_type n = aSectFormats.size();

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
                        SwContentNode* pCNd = GetNodes().GoNext(
                                                &aPam.GetPoint()->nNode );
                        aPam.GetPoint()->nContent.Assign( pCNd, 0 );
                        aPam.SetMark();
                        aPam.GetPoint()->nNode = *pSectNd->EndOfSectionNode();
                        pCNd = SwNodes::GoPrevious(
                                                &aPam.GetPoint()->nNode );
                        aPam.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );

                        getIDocumentContentOperations().DeleteRange( aPam );
                    }
                    else
                    {
                        // delete the whole section
                        aPam.SetMark();
                        aPam.GetPoint()->nNode = *pSectNd->EndOfSectionNode();
                        getIDocumentContentOperations().DelFullPara( aPam );
                    }

                }
            }
            aSectFormats.clear();
        }
    }

    if( bRet )
        getIDocumentState().SetModified();
    GetIDocumentUndoRedo().EndUndo( UNDO_UI_DELETE_INVISIBLECNTNT, nullptr );
    return bRet;
}

bool SwDoc::HasInvisibleContent() const
{
    if(SwIterator<SwFormatField,SwFieldType>(*getIDocumentFieldsAccess().GetSysFieldType( RES_HIDDENPARAFLD)).First())
        return true;

    // Search for any hidden paragraph (hidden text attribute)
    for( sal_uLong n = GetNodes().Count()-1; n; --n)
    {
        SwTextNode* pTextNd = GetNodes()[ n ]->GetTextNode();
        if ( pTextNd )
        {
            SwPaM aPam(*pTextNd, 0, *pTextNd, pTextNd->GetText().getLength());
            if( pTextNd->HasHiddenCharAttribute( true ) ||  ( pTextNd->HasHiddenCharAttribute( false ) ) )
                return true;
        }
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
    SwUndoId nLastUndoId(UNDO_EMPTY);
    if (GetIDocumentUndoRedo().GetLastUndoInfo(nullptr, & nLastUndoId)
        && (UNDO_UI_DELETE_INVISIBLECNTNT == nLastUndoId))
    {
        GetIDocumentUndoRedo().Undo();
        GetIDocumentUndoRedo().ClearRedo();
        return true;
    }
    return false;
}

bool SwDoc::ConvertFieldsToText()
{
    bool bRet = false;
    getIDocumentFieldsAccess().LockExpFields();
    GetIDocumentUndoRedo().StartUndo( UNDO_UI_REPLACE, nullptr );

    const SwFieldTypes* pMyFieldTypes = getIDocumentFieldsAccess().GetFieldTypes();
    const SwFieldTypes::size_type nCount = pMyFieldTypes->size();
    //go backward, field types are removed
    for(SwFieldTypes::size_type nType = nCount; nType > 0; --nType)
    {
        const SwFieldType *pCurType = (*pMyFieldTypes)[nType - 1];

        if ( RES_POSTITFLD == pCurType->Which() )
            continue;

        SwIterator<SwFormatField,SwFieldType> aIter( *pCurType );
        ::std::vector<const SwFormatField*> aFieldFormats;
        for( SwFormatField* pCurFieldFormat = aIter.First(); pCurFieldFormat; pCurFieldFormat = aIter.Next() )
            aFieldFormats.push_back(pCurFieldFormat);

        ::std::vector<const SwFormatField*>::iterator aBegin = aFieldFormats.begin();
        ::std::vector<const SwFormatField*>::iterator aEnd = aFieldFormats.end();
        while(aBegin != aEnd)
        {
            const SwTextField *pTextField = (*aBegin)->GetTextField();
            // skip fields that are currently not in the document
            // e.g. fields in undo or redo array

            bool bSkip = !pTextField ||
                         !pTextField->GetpTextNode()->GetNodes().IsDocNodes();

            if (!bSkip)
            {
                bool bInHeaderFooter = IsInHeaderFooter(SwNodeIndex(*pTextField->GetpTextNode()));
                const SwFormatField& rFormatField = pTextField->GetFormatField();
                const SwField*  pField = rFormatField.GetField();

                //#i55595# some fields have to be excluded in headers/footers
                sal_uInt16 nWhich = pField->GetTyp()->Which();
                if(!bInHeaderFooter ||
                        (nWhich != RES_PAGENUMBERFLD &&
                        nWhich != RES_CHAPTERFLD &&
                        nWhich != RES_GETEXPFLD&&
                        nWhich != RES_SETEXPFLD&&
                        nWhich != RES_INPUTFLD&&
                        nWhich != RES_REFPAGEGETFLD&&
                        nWhich != RES_REFPAGESETFLD))
                {
                    OUString sText = pField->ExpandField(true);

                    // database fields should not convert their command into text
                    if( RES_DBFLD == pCurType->Which() && !static_cast<const SwDBField*>(pField)->IsInitialized())
                        sText.clear();

                    SwPaM aInsertPam(*pTextField->GetpTextNode(), pTextField->GetStart());
                    aInsertPam.SetMark();

                    // go to the end of the field
                    const SwTextField *pFieldAtEnd = sw::DocumentFieldsManager::GetTextFieldAtPos(*aInsertPam.End());
                    if (pFieldAtEnd && pFieldAtEnd->Which() == RES_TXTATR_INPUTFIELD)
                    {
                        SwPosition &rEndPos = *aInsertPam.GetPoint();
                        rEndPos.nContent = SwCursorShell::EndOfInputFieldAtPos( *aInsertPam.End() );
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
            ++aBegin;
        }
    }

    if( bRet )
        getIDocumentState().SetModified();
    GetIDocumentUndoRedo().EndUndo( UNDO_UI_REPLACE, nullptr );
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

/// Set up the InsertDB as Undo table
void SwDoc::AppendUndoForInsertFromDB( const SwPaM& rPam, bool bIsTable )
{
    if( bIsTable )
    {
        const SwTableNode* pTableNd = rPam.GetPoint()->nNode.GetNode().FindTableNode();
        if( pTableNd )
        {
            SwUndoCpyTable* pUndo = new SwUndoCpyTable;
            pUndo->SetTableSttIdx( pTableNd->GetIndex() );
            GetIDocumentUndoRedo().AppendUndo( pUndo );
        }
    }
    else if( rPam.HasMark() )
    {
        SwUndoCpyDoc* pUndo = new SwUndoCpyDoc( rPam );
        pUndo->SetInsertRange( rPam, false );
        GetIDocumentUndoRedo().AppendUndo( pUndo );
    }
}

void SwDoc::ChgTOX(SwTOXBase & rTOX, const SwTOXBase & rNew)
{
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().DelAllUndoObj();

        SwUndo * pUndo = new SwUndoTOXChange(&rTOX, rNew);

        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    rTOX = rNew;

    if (dynamic_cast<const SwTOXBaseSection*>( &rTOX) !=  nullptr)
    {
        static_cast<SwTOXBaseSection &>(rTOX).Update();
        static_cast<SwTOXBaseSection &>(rTOX).UpdatePageNum();
    }
}

OUString SwDoc::GetPaMDescr(const SwPaM & rPam)
{
    if (&rPam.GetNode() == &rPam.GetNode(false))
    {
        SwTextNode * pTextNode = rPam.GetNode().GetTextNode();

        if (nullptr != pTextNode)
        {
            const sal_Int32 nStart = rPam.Start()->nContent.GetIndex();
            const sal_Int32 nEnd = rPam.End()->nContent.GetIndex();

            return SW_RESSTR(STR_START_QUOTE)
                + ShortenString(pTextNode->GetText().copy(nStart, nEnd - nStart),
                                nUndoStringLength,
                                SW_RESSTR(STR_LDOTS))
                + SW_RESSTR(STR_END_QUOTE);
        }
    }
    else
    {
        return SW_RESSTR(STR_PARAGRAPHS);
    }

    return OUString("??");
}

bool SwDoc::ContainsHiddenChars() const
{
    for( sal_uLong n = GetNodes().Count(); n; )
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
            pColl->CallSwClientNotify( SwAttrHint(RES_CONDTXTFMTCOLL) );
     }
}

uno::Reference< script::vba::XVBAEventProcessor >
SwDoc::GetVbaEventProcessor()
{
#if HAVE_FEATURE_SCRIPTING
    if( !mxVbaEvents.is() && mpDocShell && ooo::vba::isAlienWordDoc( *mpDocShell ) )
    {
        try
        {
            uno::Reference< frame::XModel > xModel( mpDocShell->GetModel(), uno::UNO_SET_THROW );
            uno::Sequence< uno::Any > aArgs(1);
            aArgs[0] <<= xModel;
            mxVbaEvents.set( ooo::vba::createVBAUnoAPIServiceWithArgs( mpDocShell, "com.sun.star.script.vba.VBATextEventProcessor" , aArgs ), uno::UNO_QUERY_THROW );
        }
        catch( uno::Exception& )
        {
        }
    }
#endif
    return mxVbaEvents;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
