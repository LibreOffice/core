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

#include <sal/config.h>

#include <cassert>
#include <cstdarg>

#include <unotxdoc.hxx>
#include <sfx2/app.hxx>
#include <sfx2/printer.hxx>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/VndSunStarPkgUrlReferenceFactory.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/TextConnectionSettings.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdb/XCompletedExecution.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/text/MailMergeEvent.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <vcl/errinf.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/viewfrm.hxx>
#include <dbconfig.hxx>
#include <pagedesc.hxx>
#include <vcl/lstbox.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/urihelper.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <vcl/oldprintadaptor.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/progress.hxx>
#include <sfx2/dispatch.hxx>
#include <cmdid.h>
#include <swmodule.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <edtwin.hxx>
#include <wrtsh.hxx>
#include <fldbas.hxx>
#include <initui.hxx>
#include <swundo.hxx>
#include <flddat.hxx>
#include <modcfg.hxx>
#include <shellio.hxx>
#include <dbui.hxx>
#include <dbmgr.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <swwait.hxx>
#include <swunohelper.hxx>
#include <dbui.hrc>
#include <globals.hrc>
#include <strings.hrc>
#include <mmconfigitem.hxx>
#include <sfx2/request.hxx>
#include <hintids.hxx>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/mail/MailAttachment.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include <mailmergehelper.hxx>
#include <maildispatcher.hxx>
#include <svtools/htmlcfg.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <editeng/langitem.hxx>
#include <svl/numuno.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbconversion.hxx>
#include <unotools/charclass.hxx>
#include <tools/diagnose_ex.h>

#include <unomailmerge.hxx>
#include <sfx2/event.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <osl/mutex.hxx>
#include <rtl/textenc.h>
#include <rtl/tencinfo.h>
#include <cppuhelper/implbase.hxx>
#include <ndindex.hxx>
#include <pam.hxx>
#include <swcrsr.hxx>
#include <swevent.hxx>
#include <osl/file.hxx>
#include <sal/log.hxx>
#include <swabstdlg.hxx>
#include <fmthdft.hxx>
#include <vector>
#include <unomid.h>
#include <section.hxx>
#include <rootfrm.hxx>
#include <fmtpdsc.hxx>
#include <ndtxt.hxx>
#include <calc.hxx>
#include <dbfld.hxx>
#include <IDocumentState.hxx>
#include <imaildsplistener.hxx>
#include <iodetect.hxx>
#include <IDocumentDeviceAccess.hxx>

#include <memory>
#include <comphelper/propertysequence.hxx>
#include <officecfg/Office/Common.hxx>

using namespace ::com::sun::star;
using namespace sw;

#define DB_SEP_SPACE    0
#define DB_SEP_TAB      1
#define DB_SEP_RETURN   2
#define DB_SEP_NEWLINE  3

namespace {

void lcl_emitEvent(SfxEventHintId nEventId, sal_Int32 nStrId, SfxObjectShell* pDocShell)
{
    SfxGetpApp()->NotifyEvent(SfxEventHint(nEventId,
                                           SwDocShell::GetEventName(nStrId),
                                           pDocShell));
}

}

std::vector<std::pair<SwDocShell*, OUString>> SwDBManager::m_aUncommittedRegistrations;

enum class SwDBNextRecord { NEXT, FIRST };
static bool lcl_ToNextRecord( SwDSParam* pParam, const SwDBNextRecord action = SwDBNextRecord::NEXT );

enum class WorkingDocType { SOURCE, TARGET, COPY };
static SfxObjectShell* lcl_CreateWorkingDocument(
    const WorkingDocType aType, const SwWrtShell &rSourceWrtShell,
    const vcl::Window *pSourceWindow,
    SwDBManager** const ppDBManager,
    SwView** const pView, SwWrtShell** const pWrtShell, SwDoc** const pDoc );

static bool lcl_getCountFromResultSet( sal_Int32& rCount, const SwDSParam* pParam )
{
    rCount = pParam->aSelection.getLength();
    if ( rCount > 0 )
        return true;

    uno::Reference<beans::XPropertySet> xPrSet(pParam->xResultSet, uno::UNO_QUERY);
    if ( xPrSet.is() )
    {
        try
        {
            bool bFinal = false;
            uno::Any aFinal = xPrSet->getPropertyValue("IsRowCountFinal");
            aFinal >>= bFinal;
            if(!bFinal)
            {
                pParam->xResultSet->last();
                pParam->xResultSet->first();
            }
            uno::Any aCount = xPrSet->getPropertyValue("RowCount");
            if( aCount >>= rCount )
                return true;
        }
        catch(const uno::Exception&)
        {
        }
    }
    return false;
}

class SwDBManager::ConnectionDisposedListener_Impl
    : public cppu::WeakImplHelper< lang::XEventListener >
{
private:
    SwDBManager * m_pDBManager;

    virtual void SAL_CALL disposing( const lang::EventObject& Source ) override;

public:
    explicit ConnectionDisposedListener_Impl(SwDBManager& rMgr);

    void Dispose() { m_pDBManager = nullptr; }

};

/// Listens to removed data sources, and if it's one that's embedded into this document, triggers embedding removal.
class SwDataSourceRemovedListener : public cppu::WeakImplHelper<sdb::XDatabaseRegistrationsListener>
{
    uno::Reference<sdb::XDatabaseContext> m_xDatabaseContext;
    SwDBManager* m_pDBManager;

public:
    explicit SwDataSourceRemovedListener(SwDBManager& rDBManager);
    virtual ~SwDataSourceRemovedListener() override;
    virtual void SAL_CALL registeredDatabaseLocation(const sdb::DatabaseRegistrationEvent& rEvent) override;
    virtual void SAL_CALL revokedDatabaseLocation(const sdb::DatabaseRegistrationEvent& rEvent) override;
    virtual void SAL_CALL changedDatabaseLocation(const sdb::DatabaseRegistrationEvent& rEvent) override;
    virtual void SAL_CALL disposing(const lang::EventObject& rObject) override;
    void Dispose();
};

SwDataSourceRemovedListener::SwDataSourceRemovedListener(SwDBManager& rDBManager)
    : m_pDBManager(&rDBManager)
{
    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    m_xDatabaseContext = sdb::DatabaseContext::create(xComponentContext);
    m_xDatabaseContext->addDatabaseRegistrationsListener(this);
}

SwDataSourceRemovedListener::~SwDataSourceRemovedListener()
{
    if (m_xDatabaseContext.is())
        m_xDatabaseContext->removeDatabaseRegistrationsListener(this);
}

void SAL_CALL SwDataSourceRemovedListener::registeredDatabaseLocation(const sdb::DatabaseRegistrationEvent& /*rEvent*/)
{
}

void SAL_CALL SwDataSourceRemovedListener::revokedDatabaseLocation(const sdb::DatabaseRegistrationEvent& rEvent)
{
    if (!m_pDBManager || m_pDBManager->getEmbeddedName().isEmpty())
        return;

    SwDoc* pDoc = m_pDBManager->getDoc();
    if (!pDoc)
        return;

    SwDocShell* pDocShell = pDoc->GetDocShell();
    if (!pDocShell)
        return;

    OUString aOwnURL = pDocShell->GetMedium()->GetURLObject().GetMainURL(INetURLObject::DecodeMechanism::WithCharset);
    OUString sTmpName = "vnd.sun.star.pkg://";
    sTmpName += INetURLObject::encode(aOwnURL, INetURLObject::PART_AUTHORITY, INetURLObject::EncodeMechanism::All);
    sTmpName += "/" + m_pDBManager->getEmbeddedName();

    if (sTmpName != rEvent.OldLocation)
        return;

    // The revoked database location is inside this document, then remove the
    // embedding, as otherwise it would be back on the next reload of the
    // document.
    pDocShell->GetStorage()->removeElement(m_pDBManager->getEmbeddedName());
    m_pDBManager->setEmbeddedName(OUString(), *pDocShell);
}

void SAL_CALL SwDataSourceRemovedListener::changedDatabaseLocation(const sdb::DatabaseRegistrationEvent& rEvent)
{
    if (rEvent.OldLocation != rEvent.NewLocation)
        revokedDatabaseLocation(rEvent);
}

void SwDataSourceRemovedListener::disposing(const lang::EventObject& /*rObject*/)
{
    m_xDatabaseContext.clear();
}

void SwDataSourceRemovedListener::Dispose()
{
    m_pDBManager = nullptr;
}

struct SwDBManager::SwDBManager_Impl
{
    std::unique_ptr<SwDSParam> pMergeData;
    VclPtr<AbstractMailMergeDlg>  pMergeDialog;
    rtl::Reference<SwDBManager::ConnectionDisposedListener_Impl> m_xDisposeListener;
    rtl::Reference<SwDataSourceRemovedListener> m_xDataSourceRemovedListener;
    osl::Mutex                    m_aAllEmailSendMutex;
    uno::Reference< mail::XMailMessage> m_xLastMessage;

    explicit SwDBManager_Impl(SwDBManager& rDBManager)
        : m_xDisposeListener(new ConnectionDisposedListener_Impl(rDBManager))
        {}

    ~SwDBManager_Impl()
    {
        m_xDisposeListener->Dispose();
        if (m_xDataSourceRemovedListener.is())
            m_xDataSourceRemovedListener->Dispose();
    }
};

static void lcl_InitNumberFormatter(SwDSParam& rParam, uno::Reference<sdbc::XDataSource> const & xSource)
{
    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    rParam.xFormatter.set(util::NumberFormatter::create(xContext), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSourceProps(
        (xSource.is()
         ? xSource
         : SwDBManager::getDataSourceAsParent(
             rParam.xConnection, rParam.sDataSource)),
        uno::UNO_QUERY);
    if(xSourceProps.is())
    {
        uno::Any aFormats = xSourceProps->getPropertyValue("NumberFormatsSupplier");
        if(aFormats.hasValue())
        {
            uno::Reference<util::XNumberFormatsSupplier> xSuppl;
            aFormats >>= xSuppl;
            if(xSuppl.is())
            {
                uno::Reference< beans::XPropertySet > xSettings = xSuppl->getNumberFormatSettings();
                uno::Any aNull = xSettings->getPropertyValue("NullDate");
                aNull >>= rParam.aNullDate;
                if(rParam.xFormatter.is())
                    rParam.xFormatter->attachNumberFormatsSupplier(xSuppl);
            }
        }
    }
}

static bool lcl_MoveAbsolute(SwDSParam* pParam, long nAbsPos)
{
    bool bRet = false;
    try
    {
        if(pParam->aSelection.getLength())
        {
            if(pParam->aSelection.getLength() <= nAbsPos)
            {
                pParam->bEndOfDB = true;
                bRet = false;
            }
            else
            {
                pParam->nSelectionIndex = nAbsPos;
                sal_Int32 nPos = 0;
                pParam->aSelection.getConstArray()[ pParam->nSelectionIndex ] >>= nPos;
                pParam->bEndOfDB = !pParam->xResultSet->absolute( nPos );
                bRet = !pParam->bEndOfDB;
            }
        }
        else if(pParam->bScrollable)
        {
            bRet = pParam->xResultSet->absolute( nAbsPos );
        }
        else
        {
            OSL_FAIL("no absolute positioning available");
        }
    }
    catch(const uno::Exception&)
    {
    }
    return bRet;
}

static void lcl_GetColumnCnt(SwDSParam *pParam,
                             const uno::Reference< beans::XPropertySet > &rColumnProps,
                             LanguageType nLanguage, OUString &rResult, double* pNumber)
{
    SwDBFormatData aFormatData;
    if(!pParam->xFormatter.is())
    {
        uno::Reference<sdbc::XDataSource> xSource = SwDBManager::getDataSourceAsParent(
                                    pParam->xConnection,pParam->sDataSource);
        lcl_InitNumberFormatter(*pParam, xSource );
    }
    aFormatData.aNullDate = pParam->aNullDate;
    aFormatData.xFormatter = pParam->xFormatter;

    aFormatData.aLocale = LanguageTag( nLanguage ).getLocale();

    rResult = SwDBManager::GetDBField( rColumnProps, aFormatData, pNumber);
}

static bool lcl_GetColumnCnt(SwDSParam* pParam, const OUString& rColumnName,
                             LanguageType nLanguage, OUString& rResult, double* pNumber)
{
    uno::Reference< sdbcx::XColumnsSupplier > xColsSupp( pParam->xResultSet, uno::UNO_QUERY );
    uno::Reference<container::XNameAccess> xCols;
    try
    {
        xCols = xColsSupp->getColumns();
    }
    catch(const lang::DisposedException&)
    {
    }
    if(!xCols.is() || !xCols->hasByName(rColumnName))
        return false;
    uno::Any aCol = xCols->getByName(rColumnName);
    uno::Reference< beans::XPropertySet > xColumnProps;
    aCol >>= xColumnProps;
    lcl_GetColumnCnt( pParam, xColumnProps, nLanguage, rResult, pNumber );
    return true;
};

// import data
bool SwDBManager::Merge( const SwMergeDescriptor& rMergeDesc )
{
    assert( !m_bInMerge && !m_pImpl->pMergeData && "merge already activated!" );

    SfxObjectShellLock  xWorkObjSh;
    SwWrtShell         *pWorkShell            = nullptr;
    SwDoc              *pWorkDoc              = nullptr;
    SwDBManager        *pWorkDocOrigDBManager = nullptr;

    switch( rMergeDesc.nMergeType )
    {
        case DBMGR_MERGE_PRINTER:
        case DBMGR_MERGE_EMAIL:
        case DBMGR_MERGE_FILE:
        case DBMGR_MERGE_SHELL:
        {
            SwDocShell *pSourceDocSh = rMergeDesc.rSh.GetView().GetDocShell();
            if( pSourceDocSh->IsModified() )
            {
                pWorkDocOrigDBManager = this;
                xWorkObjSh = lcl_CreateWorkingDocument(
                    WorkingDocType::SOURCE, rMergeDesc.rSh, nullptr,
                    &pWorkDocOrigDBManager, nullptr, &pWorkShell, &pWorkDoc );
            }
            [[fallthrough]];
        }

        default:
            if( !xWorkObjSh.Is() )
                pWorkShell = &rMergeDesc.rSh;
            break;
    }

    SwDBData aData;
    aData.nCommandType = sdb::CommandType::TABLE;
    uno::Reference<sdbc::XResultSet>  xResSet;
    uno::Sequence<uno::Any> aSelection;
    uno::Reference< sdbc::XConnection> xConnection;

    aData.sDataSource = rMergeDesc.rDescriptor.getDataSource();
    rMergeDesc.rDescriptor[svx::DataAccessDescriptorProperty::Command]      >>= aData.sCommand;
    rMergeDesc.rDescriptor[svx::DataAccessDescriptorProperty::CommandType]  >>= aData.nCommandType;

    if ( rMergeDesc.rDescriptor.has(svx::DataAccessDescriptorProperty::Cursor) )
        rMergeDesc.rDescriptor[svx::DataAccessDescriptorProperty::Cursor] >>= xResSet;
    if ( rMergeDesc.rDescriptor.has(svx::DataAccessDescriptorProperty::Selection) )
        rMergeDesc.rDescriptor[svx::DataAccessDescriptorProperty::Selection] >>= aSelection;
    if ( rMergeDesc.rDescriptor.has(svx::DataAccessDescriptorProperty::Connection) )
        rMergeDesc.rDescriptor[svx::DataAccessDescriptorProperty::Connection] >>= xConnection;

    if(aData.sDataSource.isEmpty() || aData.sCommand.isEmpty() || !xResSet.is())
    {
        return false;
    }

    m_pImpl->pMergeData.reset(new SwDSParam(aData, xResSet, aSelection));
    SwDSParam*  pTemp = FindDSData(aData, false);
    if(pTemp)
        *pTemp = *m_pImpl->pMergeData;
    else
    {
        // calls from the calculator may have added a connection with an invalid commandtype
        //"real" data base connections added here have to re-use the already available
        //DSData and set the correct CommandType
        SwDBData aTempData(aData);
        aData.nCommandType = -1;
        pTemp = FindDSData(aData, false);
        if(pTemp)
            *pTemp = *m_pImpl->pMergeData;
        else
        {
            m_DataSourceParams.push_back(std::make_unique<SwDSParam>(*m_pImpl->pMergeData));
            try
            {
                uno::Reference<lang::XComponent> xComponent(m_DataSourceParams.back()->xConnection, uno::UNO_QUERY);
                if(xComponent.is())
                    xComponent->addEventListener(m_pImpl->m_xDisposeListener.get());
            }
            catch(const uno::Exception&)
            {
            }
        }
    }
    if(!m_pImpl->pMergeData->xConnection.is())
        m_pImpl->pMergeData->xConnection = xConnection;
    // add an XEventListener

    lcl_ToNextRecord(m_pImpl->pMergeData.get(), SwDBNextRecord::FIRST);

    uno::Reference<sdbc::XDataSource> xSource = SwDBManager::getDataSourceAsParent(xConnection,aData.sDataSource);

    lcl_InitNumberFormatter(*m_pImpl->pMergeData, xSource);

    pWorkShell->ChgDBData(aData);
    m_bInMerge = true;

    if (IsInitDBFields())
    {
        // with database fields without DB-Name, use DB-Name from Doc
        std::vector<OUString> aDBNames;
        aDBNames.emplace_back();
        SwDBData aInsertData = pWorkShell->GetDBData();
        OUString sDBName = aInsertData.sDataSource
            + OUStringLiteral1(DB_DELIM) + aInsertData.sCommand
            + OUStringLiteral1(DB_DELIM)
            + OUString::number(aInsertData.nCommandType);
        pWorkShell->ChangeDBFields( aDBNames, sDBName);
        SetInitDBFields(false);
    }

    bool bRet = true;
    switch(rMergeDesc.nMergeType)
    {
        case DBMGR_MERGE:
            pWorkShell->StartAllAction();
            pWorkShell->SwViewShell::UpdateFields( true );
            pWorkShell->SetModified();
            pWorkShell->EndAllAction();
            break;

        case DBMGR_MERGE_PRINTER:
        case DBMGR_MERGE_EMAIL:
        case DBMGR_MERGE_FILE:
        case DBMGR_MERGE_SHELL:
            // save files and send them as e-Mail if required
            bRet = MergeMailFiles(pWorkShell, rMergeDesc);
            break;

        default:
            // insert selected entries
            // (was: InsertRecord)
            ImportFromConnection(pWorkShell);
            break;
    }

    m_pImpl->pMergeData.reset();

    if( xWorkObjSh.Is() )
    {
        pWorkDoc->SetDBManager( pWorkDocOrigDBManager );
        xWorkObjSh->DoClose();
    }

    m_bInMerge = false;

    return bRet;
}

void SwDBManager::ImportFromConnection(  SwWrtShell* pSh )
{
    if(m_pImpl->pMergeData && !m_pImpl->pMergeData->bEndOfDB)
    {
        pSh->StartAllAction();
        pSh->StartUndo();
        bool bGroupUndo(pSh->DoesGroupUndo());
        pSh->DoGroupUndo(false);

        if( pSh->HasSelection() )
            pSh->DelRight();

        std::unique_ptr<SwWait> pWait;

        {
            sal_uLong i = 0;
            do {

                ImportDBEntry(pSh);
                if( 10 == ++i )
                    pWait.reset(new SwWait( *pSh->GetView().GetDocShell(), true));

            } while(ToNextMergeRecord());
        }

        pSh->DoGroupUndo(bGroupUndo);
        pSh->EndUndo();
        pSh->EndAllAction();
    }
}

static OUString  lcl_FindColumn(const OUString& sFormatStr,sal_uInt16  &nUsedPos, sal_uInt8 &nSeparator)
{
    OUStringBuffer sReturn;
    sal_uInt16 nLen = sFormatStr.getLength();
    nSeparator = 0xff;
    while(nUsedPos < nLen && nSeparator == 0xff)
    {
        sal_Unicode cCurrent = sFormatStr[nUsedPos];
        switch(cCurrent)
        {
            case ',':
                nSeparator = DB_SEP_SPACE;
            break;
            case ';':
                nSeparator = DB_SEP_RETURN;
            break;
            case ':':
                nSeparator = DB_SEP_TAB;
            break;
            case '#':
                nSeparator = DB_SEP_NEWLINE;
            break;
            default:
                sReturn.append(cCurrent);
        }
        nUsedPos++;

    }
    return sReturn.makeStringAndClear();
}

void SwDBManager::ImportDBEntry(SwWrtShell* pSh)
{
    if(m_pImpl->pMergeData && !m_pImpl->pMergeData->bEndOfDB)
    {
        uno::Reference< sdbcx::XColumnsSupplier > xColsSupp( m_pImpl->pMergeData->xResultSet, uno::UNO_QUERY );
        uno::Reference<container::XNameAccess> xCols = xColsSupp->getColumns();
        OUString sFormatStr;
        sal_uInt16 nFormatLen = sFormatStr.getLength();
        if( nFormatLen )
        {
            const char cSpace = ' ';
            const char cTab = '\t';
            sal_uInt16 nUsedPos = 0;
            sal_uInt8   nSeparator;
            OUString sColumn = lcl_FindColumn(sFormatStr, nUsedPos, nSeparator);
            while( !sColumn.isEmpty() )
            {
                if(!xCols->hasByName(sColumn))
                    return;
                uno::Any aCol = xCols->getByName(sColumn);
                uno::Reference< beans::XPropertySet > xColumnProp;
                aCol >>= xColumnProp;
                if(xColumnProp.is())
                {
                    SwDBFormatData aDBFormat;
                    OUString sInsert = GetDBField( xColumnProp,   aDBFormat);
                    if( DB_SEP_SPACE == nSeparator )
                            sInsert += OUStringLiteral1(cSpace);
                    else if( DB_SEP_TAB == nSeparator)
                            sInsert += OUStringLiteral1(cTab);
                    pSh->Insert(sInsert);
                    if( DB_SEP_RETURN == nSeparator)
                        pSh->SplitNode();
                    else if(DB_SEP_NEWLINE == nSeparator)
                            pSh->InsertLineBreak();
                }
                else
                {
                    // column not found -> show error
                    OUStringBuffer sInsert;
                    sInsert.append('?').append(sColumn).append('?');
                    pSh->Insert(sInsert.makeStringAndClear());
                }
                sColumn = lcl_FindColumn(sFormatStr, nUsedPos, nSeparator);
            }
            pSh->SplitNode();
        }
        else
        {
            OUStringBuffer sStr;
            uno::Sequence<OUString> aColNames = xCols->getElementNames();
            const OUString* pColNames = aColNames.getConstArray();
            long nLength = aColNames.getLength();
            for(long i = 0; i < nLength; i++)
            {
                uno::Any aCol = xCols->getByName(pColNames[i]);
                uno::Reference< beans::XPropertySet > xColumnProp;
                aCol >>= xColumnProp;
                SwDBFormatData aDBFormat;
                sStr.append(GetDBField( xColumnProp, aDBFormat));
                if (i < nLength - 1)
                    sStr.append("\t");
            }
            pSh->SwEditShell::Insert2(sStr.makeStringAndClear());
            pSh->SwFEShell::SplitNode();    // line feed
        }
    }
}

bool SwDBManager::GetTableNames(weld::ComboBox& rBox, const OUString& rDBName)
{
    bool bRet = false;
    OUString sOldTableName(rBox.get_active_text());
    rBox.clear();
    SwDSParam* pParam = FindDSConnection(rDBName, false);
    uno::Reference< sdbc::XConnection> xConnection;
    if (pParam && pParam->xConnection.is())
        xConnection = pParam->xConnection;
    else
    {
        if ( !rDBName.isEmpty() )
            xConnection = RegisterConnection( rDBName );
    }
    if (xConnection.is())
    {
        uno::Reference<sdbcx::XTablesSupplier> xTSupplier(xConnection, uno::UNO_QUERY);
        if(xTSupplier.is())
        {
            uno::Reference<container::XNameAccess> xTables = xTSupplier->getTables();
            uno::Sequence<OUString> aTables = xTables->getElementNames();
            const OUString* pTables = aTables.getConstArray();
            for (sal_Int32 i = 0; i < aTables.getLength(); ++i)
                rBox.append("0", pTables[i]);
        }
        uno::Reference<sdb::XQueriesSupplier> xQSupplier(xConnection, uno::UNO_QUERY);
        if(xQSupplier.is())
        {
            uno::Reference<container::XNameAccess> xQueries = xQSupplier->getQueries();
            uno::Sequence<OUString> aQueries = xQueries->getElementNames();
            const OUString* pQueries = aQueries.getConstArray();
            for (sal_Int32 i = 0; i < aQueries.getLength(); i++)
                rBox.append("1", pQueries[i]);
        }
        if (!sOldTableName.isEmpty())
            rBox.set_active_text(sOldTableName);
        bRet = true;
    }
    return bRet;
}

// fill Listbox with column names of a database
void SwDBManager::GetColumnNames(ListBox* pListBox,
                             const OUString& rDBName, const OUString& rTableName)
{
    SwDBData aData;
    aData.sDataSource = rDBName;
    aData.sCommand = rTableName;
    aData.nCommandType = -1;
    SwDSParam* pParam = FindDSData(aData, false);
    uno::Reference< sdbc::XConnection> xConnection;
    if(pParam && pParam->xConnection.is())
        xConnection = pParam->xConnection;
    else
    {
        xConnection = RegisterConnection( rDBName );
    }
    GetColumnNames(pListBox, xConnection, rTableName);
}

void SwDBManager::GetColumnNames(weld::ComboBox& rBox,
                             const OUString& rDBName, const OUString& rTableName)
{
    SwDBData aData;
    aData.sDataSource = rDBName;
    aData.sCommand = rTableName;
    aData.nCommandType = -1;
    SwDSParam* pParam = FindDSData(aData, false);
    uno::Reference< sdbc::XConnection> xConnection;
    if(pParam && pParam->xConnection.is())
        xConnection = pParam->xConnection;
    else
    {
        xConnection = RegisterConnection( rDBName );
    }
    GetColumnNames(rBox, xConnection, rTableName);
}

void SwDBManager::GetColumnNames(ListBox* pListBox,
        uno::Reference< sdbc::XConnection> const & xConnection,
        const OUString& rTableName)
{
    pListBox->Clear();
    uno::Reference< sdbcx::XColumnsSupplier> xColsSupp = SwDBManager::GetColumnSupplier(xConnection, rTableName);
    if(xColsSupp.is())
    {
        uno::Reference<container::XNameAccess> xCols = xColsSupp->getColumns();
        const uno::Sequence<OUString> aColNames = xCols->getElementNames();
        const OUString* pColNames = aColNames.getConstArray();
        for(int nCol = 0; nCol < aColNames.getLength(); nCol++)
        {
            pListBox->InsertEntry(pColNames[nCol]);
        }
        ::comphelper::disposeComponent( xColsSupp );
    }
}

void SwDBManager::GetColumnNames(weld::ComboBox& rBox,
        uno::Reference< sdbc::XConnection> const & xConnection,
        const OUString& rTableName)
{
    rBox.clear();
    uno::Reference< sdbcx::XColumnsSupplier> xColsSupp = SwDBManager::GetColumnSupplier(xConnection, rTableName);
    if(xColsSupp.is())
    {
        uno::Reference<container::XNameAccess> xCols = xColsSupp->getColumns();
        const uno::Sequence<OUString> aColNames = xCols->getElementNames();
        const OUString* pColNames = aColNames.getConstArray();
        for (sal_Int32 nCol = 0; nCol < aColNames.getLength(); ++nCol)
        {
            rBox.append_text(pColNames[nCol]);
        }
        ::comphelper::disposeComponent( xColsSupp );
    }
}

SwDBManager::SwDBManager(SwDoc* pDoc)
    : m_aMergeStatus( MergeStatus::Ok )
    , m_bInitDBFields(false)
    , m_bInMerge(false)
    , m_bMergeSilent(false)
    , m_pImpl(new SwDBManager_Impl(*this))
    , m_pMergeEvtSrc(nullptr)
    , m_pDoc(pDoc)
{
}

SwDBManager::~SwDBManager() COVERITY_NOEXCEPT_FALSE
{
    RevokeLastRegistrations();

    // copy required, m_DataSourceParams can be modified while disposing components
    std::vector<uno::Reference<sdbc::XConnection>> aCopiedConnections;
    for (auto & pParam : m_DataSourceParams)
    {
        if(pParam->xConnection.is())
        {
            aCopiedConnections.push_back(pParam->xConnection);
        }
    }
    for (auto & xConnection : aCopiedConnections)
    {
        try
        {
            uno::Reference<lang::XComponent> xComp(xConnection, uno::UNO_QUERY);
            if(xComp.is())
                xComp->dispose();
        }
        catch(const uno::RuntimeException&)
        {
            //may be disposed already since multiple entries may have used the same connection
        }
    }
}

static void lcl_RemoveSectionLinks( SwWrtShell& rWorkShell )
{
    //reset all links of the sections of synchronized labels
    size_t nSections = rWorkShell.GetSectionFormatCount();
    for (size_t nSection = 0; nSection < nSections; ++nSection)
    {
        SwSectionData aSectionData( *rWorkShell.GetSectionFormat( nSection ).GetSection() );
        if( aSectionData.GetType() == FILE_LINK_SECTION )
        {
            aSectionData.SetType( CONTENT_SECTION );
            aSectionData.SetLinkFileName( OUString() );
            rWorkShell.UpdateSection( nSection, aSectionData );
        }
    }
    rWorkShell.SetLabelDoc( false );
}

static void lcl_SaveDebugDoc( SfxObjectShell *xTargetDocShell,
                              const char *name, int no = 0 )
{
    static OUString sTempDirURL;
    if( sTempDirURL.isEmpty() )
    {
        SvtPathOptions aPathOpt;
        utl::TempFile aTempDir( &aPathOpt.GetTempPath(), true );
        if( aTempDir.IsValid() )
        {
            INetURLObject aTempDirURL( aTempDir.GetURL() );
            sTempDirURL = aTempDirURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
            SAL_INFO( "sw.mailmerge", "Dump directory: " << sTempDirURL );
        }
    }
    if( sTempDirURL.isEmpty() )
        return;

    const OUString sExt( ".odt" );
    OUString basename = OUString::createFromAscii( name );
    if (no > 0)
        basename += OUString::number(no) + "-";
    // aTempFile is not deleted, but that seems to be intentional
    utl::TempFile aTempFile( basename, true, &sExt, &sTempDirURL );
    INetURLObject aTempFileURL( aTempFile.GetURL() );
    auto pDstMed = std::make_unique<SfxMedium>(
        aTempFileURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ),
        StreamMode::STD_READWRITE );
    bool bAnyError = !xTargetDocShell->DoSaveAs( *pDstMed );
    // xObjectShell->DoSaveCompleted crashes the mail merge unit tests, so skip it
    bAnyError |= (ERRCODE_NONE != xTargetDocShell->GetError());
    if( bAnyError )
        SAL_WARN( "sw.mailmerge", "Error saving: " << aTempFile.GetURL() );
    else
        SAL_INFO( "sw.mailmerge", "Saved doc as: " << aTempFile.GetURL() );
}

static bool lcl_SaveDoc(
    const INetURLObject* pFileURL,
    const std::shared_ptr<const SfxFilter>& pStoreToFilter,
    const OUString* pStoreToFilterOptions,
    const uno::Sequence< beans::PropertyValue >* pSaveToFilterData,
    const bool bIsPDFexport,
    SfxObjectShell* xObjectShell,
    SwWrtShell& rWorkShell,
    OUString * const decodedURL = nullptr )
{
    OUString url = pFileURL->GetMainURL( INetURLObject::DecodeMechanism::NONE );
    if( decodedURL )
        (*decodedURL) = url;

    SfxMedium* pDstMed = new SfxMedium( url, StreamMode::STD_READWRITE );
    pDstMed->SetFilter( pStoreToFilter );
    if( pDstMed->GetItemSet() )
    {
        if( pStoreToFilterOptions )
            pDstMed->GetItemSet()->Put( SfxStringItem(SID_FILE_FILTEROPTIONS,
                                        *pStoreToFilterOptions));
        if( pSaveToFilterData->getLength() )
            pDstMed->GetItemSet()->Put( SfxUnoAnyItem(SID_FILTER_DATA,
                                        uno::makeAny(*pSaveToFilterData)));
    }

    // convert fields to text if we are exporting to PDF.
    // this prevents a second merge while updating the fields
    // in SwXTextDocument::getRendererCount()
    if( bIsPDFexport )
        rWorkShell.ConvertFieldsToText();

    bool bAnyError = !xObjectShell->DoSaveAs(*pDstMed);
    // Actually this should be a bool... so in case of email and individual
    // files, where this is set, we skip the recently used handling
    bAnyError |= !xObjectShell->DoSaveCompleted( pDstMed, !decodedURL );
    bAnyError |= (ERRCODE_NONE != xObjectShell->GetError());
    if( bAnyError )
    {
        // error message ??
        ErrorHandler::HandleError( xObjectShell->GetError() );
    }
    return !bAnyError;
}

static void lcl_PreparePrinterOptions(
    const uno::Sequence< beans::PropertyValue >& rInPrintOptions,
    uno::Sequence< beans::PropertyValue >& rOutPrintOptions)
{
    // printing should be done synchronously otherwise the document
    // might already become invalid during the process

    const sal_Int32 nOffset = 1;
    rOutPrintOptions.realloc( nOffset );
    rOutPrintOptions[ 0 ].Name = "Wait";
    rOutPrintOptions[ 0 ].Value <<= true;

    // copy print options
    const beans::PropertyValue* pOptions = rInPrintOptions.getConstArray();
    for( sal_Int32 n = 0, nIndex = nOffset ; n < rInPrintOptions.getLength(); ++n)
    {
        if( pOptions[n].Name == "CopyCount" || pOptions[n].Name == "FileName"
            || pOptions[n].Name == "Collate" || pOptions[n].Name == "Pages"
            || pOptions[n].Name == "Wait" || pOptions[n].Name == "PrinterName" )
        {
            // add an option
            rOutPrintOptions.realloc( nIndex + 1 );
            rOutPrintOptions[ nIndex ].Name = pOptions[n].Name;
            rOutPrintOptions[ nIndex++ ].Value = pOptions[n].Value ;
        }
    }
}

static SfxObjectShell* lcl_CreateWorkingDocument(
    // input
    const WorkingDocType aType, const SwWrtShell &rSourceWrtShell,
    // optional input
    const vcl::Window *pSourceWindow,
    // optional in and output to swap the DB manager
    SwDBManager** const ppDBManager,
    // optional output
    SwView** const pView, SwWrtShell** const pWrtShell, SwDoc** const pDoc )
{
    const SwDoc *pSourceDoc = rSourceWrtShell.GetDoc();
    SfxObjectShellRef xWorkObjectShell = pSourceDoc->CreateCopy( true, (aType == WorkingDocType::TARGET) );
    SfxViewFrame* pWorkFrame = SfxViewFrame::LoadHiddenDocument( *xWorkObjectShell, SFX_INTERFACE_NONE );

    if( pSourceWindow )
    {
        // the created window has to be located at the same position as the source window
        vcl::Window& rTargetWindow = pWorkFrame->GetFrame().GetWindow();
        rTargetWindow.SetPosPixel( pSourceWindow->GetPosPixel() );
    }

    SwView* pWorkView = static_cast< SwView* >( pWorkFrame->GetViewShell() );
    SwWrtShell* pWorkWrtShell = pWorkView->GetWrtShellPtr();
    pWorkWrtShell->GetViewOptions()->SetIdle( false );
    pWorkView->AttrChangedNotify( pWorkWrtShell );// in order for SelectShell to be called
    SwDoc* pWorkDoc = pWorkWrtShell->GetDoc();
    pWorkDoc->GetIDocumentUndoRedo().DoUndo( false );
    pWorkDoc->ReplaceDocumentProperties( *pSourceDoc );

    if( aType == WorkingDocType::TARGET )
    {
        assert( !ppDBManager );
        pWorkDoc->SetInMailMerge( true );
        pWorkWrtShell->SetLabelDoc( false );
    }
    else
    {
        // We have to swap the DBmanager of the new doc, so we also need input
        assert(ppDBManager && *ppDBManager);
        SwDBManager *pWorkDBManager = pWorkDoc->GetDBManager();
        pWorkDoc->SetDBManager( *ppDBManager );
        *ppDBManager = pWorkDBManager;

        if( aType == WorkingDocType::SOURCE )
        {
            // the GetDBData call constructs the data, if it's missing - kind of const...
            pWorkWrtShell->ChgDBData( const_cast<SwDoc*>(pSourceDoc)->GetDBData() );
            // some DocumentSettings are currently not copied by SwDoc::CreateCopy
            pWorkWrtShell->SetLabelDoc( rSourceWrtShell.IsLabelDoc() );
            pWorkDoc->getIDocumentState().ResetModified();
        }
        else
            pWorkDoc->getIDocumentLinksAdministration().EmbedAllLinks();
    }

    if( pView )     *pView     = pWorkView;
    if( pWrtShell ) *pWrtShell = pWorkWrtShell;
    if( pDoc )      *pDoc      = pWorkDoc;

    return xWorkObjectShell.get();
}

static SwMailMessage* lcl_CreateMailFromDoc(
    const SwMergeDescriptor &rMergeDescriptor,
    const OUString &sFileURL, const OUString &sMailRecipient,
    const OUString &sMailBodyMimeType, rtl_TextEncoding sMailEncoding,
    const OUString &sAttachmentMimeType )
{
    SwMailMessage* pMessage = new SwMailMessage;
    if( rMergeDescriptor.pMailMergeConfigItem->IsMailReplyTo() )
        pMessage->setReplyToAddress(rMergeDescriptor.pMailMergeConfigItem->GetMailReplyTo());
    pMessage->addRecipient( sMailRecipient );
    pMessage->SetSenderAddress( rMergeDescriptor.pMailMergeConfigItem->GetMailAddress() );

    OUStringBuffer sBody;
    if( rMergeDescriptor.bSendAsAttachment )
    {
        sBody = rMergeDescriptor.sMailBody;
        mail::MailAttachment aAttach;
        aAttach.Data = new SwMailTransferable( sFileURL,
            rMergeDescriptor.sAttachmentName, sAttachmentMimeType );
        aAttach.ReadableName = rMergeDescriptor.sAttachmentName;
        pMessage->addAttachment( aAttach );
    }
    else
    {
        //read in the temporary file and use it as mail body
        SfxMedium aMedium( sFileURL, StreamMode::READ );
        SvStream* pInStream = aMedium.GetInStream();
        assert( pInStream && "no output file created?" );
        if( !pInStream )
            return pMessage;

        pInStream->SetStreamCharSet( sMailEncoding );
        OString sLine;
        while ( pInStream->ReadLine( sLine ) )
        {
            sBody.append(OStringToOUString( sLine, sMailEncoding ));
            sBody.append("\n");
        }
    }
    pMessage->setSubject( rMergeDescriptor.sSubject );
    uno::Reference< datatransfer::XTransferable> xBody =
                new SwMailTransferable( sBody.makeStringAndClear(), sMailBodyMimeType );
    pMessage->setBody( xBody );

    for( const OUString& sCcRecipient : rMergeDescriptor.aCopiesTo )
        pMessage->addCcRecipient( sCcRecipient );
    for( const OUString& sBccRecipient : rMergeDescriptor.aBlindCopiesTo )
        pMessage->addBccRecipient( sBccRecipient );

    return pMessage;
}

class SwDBManager::MailDispatcherListener_Impl : public IMailDispatcherListener
{
    SwDBManager &m_rDBManager;

public:
    explicit MailDispatcherListener_Impl( SwDBManager &rDBManager )
        : m_rDBManager( rDBManager ) {}

    virtual void started( ::rtl::Reference<MailDispatcher> ) override {};
    virtual void stopped( ::rtl::Reference<MailDispatcher> ) override {};
    virtual void idle( ::rtl::Reference<MailDispatcher> ) override {};

    virtual void mailDelivered( ::rtl::Reference<MailDispatcher>,
                 uno::Reference< mail::XMailMessage> xMessage ) override
    {
        osl::MutexGuard aGuard( m_rDBManager.m_pImpl->m_aAllEmailSendMutex );
        if ( m_rDBManager.m_pImpl->m_xLastMessage == xMessage )
            m_rDBManager.m_pImpl->m_xLastMessage.clear();
    }

    virtual void mailDeliveryError( ::rtl::Reference<MailDispatcher> xMailDispatcher,
                uno::Reference< mail::XMailMessage>, const OUString& ) override
    {
        osl::MutexGuard aGuard( m_rDBManager.m_pImpl->m_aAllEmailSendMutex );
        m_rDBManager.m_aMergeStatus = MergeStatus::Error;
        m_rDBManager.m_pImpl->m_xLastMessage.clear();
        xMailDispatcher->stop();
    }
};

/**
 * Please have a look at the README in the same directory, before you make
 * larger changes in this function!
 */
bool SwDBManager::MergeMailFiles(SwWrtShell* pSourceShell,
                                 const SwMergeDescriptor& rMergeDescriptor)
{
    // deconstruct mail merge type for better readability.
    // uppercase naming is intentional!
    const bool bMT_EMAIL   = rMergeDescriptor.nMergeType == DBMGR_MERGE_EMAIL;
    const bool bMT_SHELL   = rMergeDescriptor.nMergeType == DBMGR_MERGE_SHELL;
    const bool bMT_PRINTER = rMergeDescriptor.nMergeType == DBMGR_MERGE_PRINTER;
    const bool bMT_FILE    = rMergeDescriptor.nMergeType == DBMGR_MERGE_FILE;

    //check if the doc is synchronized and contains at least one linked section
    const bool bSynchronizedDoc = pSourceShell->IsLabelDoc() && pSourceShell->GetSectionFormatCount() > 1;
    const bool bNeedsTempFiles = ( bMT_EMAIL || bMT_FILE );
    const bool bIsMergeSilent = IsMergeSilent();

    bool bCheckSingleFile_ = rMergeDescriptor.bCreateSingleFile;
    OUString sPrefix_ = rMergeDescriptor.sPrefix;
    if( bMT_EMAIL )
    {
        assert( !rMergeDescriptor.bPrefixIsFilename );
        assert(!bCheckSingleFile_);
        bCheckSingleFile_ = false;
    }
    else if( bMT_SHELL || bMT_PRINTER )
    {
        assert(bCheckSingleFile_);
        bCheckSingleFile_ = true;
        assert(sPrefix_.isEmpty());
        sPrefix_.clear();
    }
    const bool bCreateSingleFile = bCheckSingleFile_;
    const OUString sDescriptorPrefix = sPrefix_;

    // Setup for dumping debugging documents
    static const sal_Int32 nMaxDumpDocs = []() {
        if (const char* sEnv = getenv("SW_DEBUG_MAILMERGE_DOCS"))
            return OUString(sEnv, strlen(sEnv), osl_getThreadTextEncoding()).toInt32();
        else
            return sal_Int32(0);
    }();

    ::rtl::Reference< MailDispatcher >          xMailDispatcher;
    ::rtl::Reference< IMailDispatcherListener > xMailListener;
    OUString                            sMailBodyMimeType;
    rtl_TextEncoding                    sMailEncoding = ::osl_getThreadTextEncoding();

    uno::Reference< beans::XPropertySet > xColumnProp;

    // Check for (mandatory) email or (optional) filename column
    SwDBFormatData aColumnDBFormat;
    bool bColumnName = !rMergeDescriptor.sDBcolumn.isEmpty();
    if( ! bColumnName )
    {
        if( bMT_EMAIL )
            return false;
    }
    else
    {
        uno::Reference< sdbcx::XColumnsSupplier > xColsSupp( m_pImpl->pMergeData->xResultSet, uno::UNO_QUERY );
        uno::Reference<container::XNameAccess> xCols = xColsSupp->getColumns();
        if( !xCols->hasByName( rMergeDescriptor.sDBcolumn ) )
            return false;
        uno::Any aCol = xCols->getByName( rMergeDescriptor.sDBcolumn );
        aCol >>= xColumnProp;

        aColumnDBFormat.xFormatter = m_pImpl->pMergeData->xFormatter;
        aColumnDBFormat.aNullDate  = m_pImpl->pMergeData->aNullDate;

        if( bMT_EMAIL )
        {
            // Reset internal mail accounting data
            m_pImpl->m_xLastMessage.clear();

            xMailDispatcher.set( new MailDispatcher(rMergeDescriptor.xSmtpServer) );
            xMailListener = new MailDispatcherListener_Impl( *this );
            xMailDispatcher->addListener( xMailListener );
            if(!rMergeDescriptor.bSendAsAttachment && rMergeDescriptor.bSendAsHTML)
            {
                sMailBodyMimeType = "text/html; charset=";
                sMailBodyMimeType += OUString::createFromAscii(
                                    rtl_getBestMimeCharsetFromTextEncoding( sMailEncoding ));
                SvxHtmlOptions& rHtmlOptions = SvxHtmlOptions::Get();
                sMailEncoding = rHtmlOptions.GetTextEncoding();
            }
            else
                sMailBodyMimeType = "text/plain; charset=UTF-8; format=flowed";
        }
    }

    SwDocShell  *pSourceDocSh = pSourceShell->GetView().GetDocShell();

    // setup the output format
    std::shared_ptr<const SfxFilter> pStoreToFilter = SwIoSystem::GetFileFilter(
        pSourceDocSh->GetMedium()->GetURLObject().GetMainURL(INetURLObject::DecodeMechanism::NONE));
    SfxFilterContainer* pFilterContainer = SwDocShell::Factory().GetFilterContainer();
    const OUString* pStoreToFilterOptions = nullptr;

    // if a save_to filter is set then use it - otherwise use the default
    if( bMT_EMAIL && !rMergeDescriptor.bSendAsAttachment )
    {
        OUString sExtension = rMergeDescriptor.bSendAsHTML ? OUString("html") : OUString("txt");
        pStoreToFilter = pFilterContainer->GetFilter4Extension(sExtension, SfxFilterFlags::EXPORT);
    }
    else if( !rMergeDescriptor.sSaveToFilter.isEmpty())
    {
        std::shared_ptr<const SfxFilter> pFilter =
                pFilterContainer->GetFilter4FilterName( rMergeDescriptor.sSaveToFilter );
        if(pFilter)
        {
            pStoreToFilter = pFilter;
            if(!rMergeDescriptor.sSaveToFilterOptions.isEmpty())
                pStoreToFilterOptions = &rMergeDescriptor.sSaveToFilterOptions;
        }
    }
    const bool bIsPDFexport = pStoreToFilter && pStoreToFilter->GetFilterName() == "writer_pdf_Export";

    m_aMergeStatus = MergeStatus::Ok;

    // in case of creating a single resulting file this has to be created here
    SwView*           pTargetView     = rMergeDescriptor.pMailMergeConfigItem ?
                                        rMergeDescriptor.pMailMergeConfigItem->GetTargetView() : nullptr;
    SwWrtShell*       pTargetShell    = nullptr;
    SwDoc*            pTargetDoc      = nullptr;
    SfxObjectShellRef xTargetDocShell;

    std::unique_ptr< utl::TempFile > aTempFile;
    sal_uInt16 nStartingPageNo = 0;

    vcl::Window *pSourceWindow = nullptr;
    std::shared_ptr<weld::GenericDialogController> xProgressDlg;

    try
    {
        if( !bIsMergeSilent )
        {
            // construct the process dialog
            pSourceWindow = &pSourceShell->GetView().GetEditWin();
            if (!bMT_PRINTER)
                xProgressDlg.reset(new CreateMonitor(pSourceWindow->GetFrameWeld()));
            else
            {
                xProgressDlg.reset(new PrintMonitor(pSourceWindow->GetFrameWeld()));
                static_cast<PrintMonitor*>(xProgressDlg.get())->set_title(
                    pSourceDocSh->GetTitle(22));
            }
            weld::DialogController::runAsync(xProgressDlg, [this, &xProgressDlg](sal_Int32 nResult){
                if (nResult == RET_CANCEL)
                    MergeCancel();
                xProgressDlg.reset();
            });

            Application::Reschedule( true );
        }

        if( bCreateSingleFile && !pTargetView )
        {
            // create a target docshell to put the merged document into
            xTargetDocShell = lcl_CreateWorkingDocument( WorkingDocType::TARGET,
                *pSourceShell, bMT_SHELL ? pSourceWindow : nullptr,
                nullptr, &pTargetView, &pTargetShell, &pTargetDoc );

            // import current print settings
            const SwPrintData &rPrintData = pSourceShell->getIDocumentDeviceAccess().getPrintData();
            pTargetDoc->getIDocumentDeviceAccess().setPrintData(rPrintData);

            if (nMaxDumpDocs)
                lcl_SaveDebugDoc( xTargetDocShell.get(), "MergeDoc" );
        }
        else if( pTargetView )
        {
            pTargetShell = pTargetView->GetWrtShellPtr();
            pTargetDoc = pTargetShell->GetDoc();
            xTargetDocShell = pTargetView->GetDocShell();
        }

        if( bCreateSingleFile )
        {
            // determine the page style and number used at the start of the source document
            pSourceShell->SttEndDoc(true);
            nStartingPageNo = pSourceShell->GetVirtPageNum();
        }

        // Progress, to prohibit KeyInputs
        SfxProgress aProgress(pSourceDocSh, OUString(), 1);

        // lock all dispatchers
        SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst(pSourceDocSh);
        while (pViewFrame)
        {
            pViewFrame->GetDispatcher()->Lock(true);
            pViewFrame = SfxViewFrame::GetNext(*pViewFrame, pSourceDocSh);
        }

        sal_Int32 nDocNo = 1;

        // For single file mode, the number of pages in the target document so far, which is used
        // by AppendDoc() to adjust position of page-bound objects. Getting this information directly
        // from the target doc would require repeated layouts of the doc, which is expensive, but
        // it can be manually computed from the source documents (for which we do layouts, so the page
        // count is known, and there is a blank page between each of them in the target document).
        int targetDocPageCount = 0;

        if( !bIsMergeSilent && !bMT_PRINTER )
        {
            sal_Int32 nRecordCount = 1;
            lcl_getCountFromResultSet( nRecordCount, m_pImpl->pMergeData.get() );

            // Synchronized docs don't auto-advance the record set, but there is a
            // "security" check, which will always advance the record set, if there
            // is no "next record" field in a synchronized doc => nRecordPerDoc > 0
            sal_Int32 nRecordPerDoc = pSourceShell->GetDoc()
                    ->getIDocumentFieldsAccess().GetRecordsPerDocument();
            if ( bSynchronizedDoc && (nRecordPerDoc > 1) )
                --nRecordPerDoc;
            assert( nRecordPerDoc > 0 );

            sal_Int32 nMaxDocs = nRecordCount / nRecordPerDoc;
            if ( 0 != nRecordCount % nRecordPerDoc )
                nMaxDocs += 1;
            static_cast<CreateMonitor*>(xProgressDlg.get())->SetTotalCount(nMaxDocs);
        }

        long nStartRow, nEndRow;
        bool bFreezedLayouts = false;
        // to collect temporary email files
        std::vector< OUString> aFilesToRemove;

        // The SfxObjectShell will be closed explicitly later but
        // it is more safe to use SfxObjectShellLock here
        SfxObjectShellLock xWorkDocSh;
        SwView*            pWorkView             = nullptr;
        SwDoc*             pWorkDoc              = nullptr;
        SwDBManager*       pWorkDocOrigDBManager = nullptr;
        SwWrtShell*        pWorkShell            = nullptr;
        bool               bWorkDocInitialized   = false;

        do
        {
            nStartRow = m_pImpl->pMergeData ? m_pImpl->pMergeData->xResultSet->getRow() : 0;

            OUString sColumnData;

            // Read the indicated data column, which should contain a valid mail
            // address or an optional file name
            if( bMT_EMAIL || bColumnName )
            {
                sColumnData = GetDBField( xColumnProp, aColumnDBFormat );
            }

            // create a new temporary file name - only done once in case of bCreateSingleFile
            if( bNeedsTempFiles && ( !bWorkDocInitialized || !bCreateSingleFile ))
            {
                OUString sPrefix = sDescriptorPrefix;
                OUString sLeading;

                //#i97667# if the name is from a database field then it will be used _as is_
                if( bColumnName && !bMT_EMAIL )
                {
                    if (!sColumnData.isEmpty())
                        sLeading = sColumnData;
                    else
                        sLeading = "_";
                }
                else
                {
                    INetURLObject aEntry( sPrefix );
                    sLeading = aEntry.GetBase();
                    aEntry.removeSegment();
                    sPrefix = aEntry.GetMainURL( INetURLObject::DecodeMechanism::NONE );
                }

                OUString sExt(comphelper::string::stripStart(pStoreToFilter->GetDefaultExtension(), '*'));
                aTempFile.reset( new utl::TempFile(sLeading, sColumnData.isEmpty(), &sExt, &sPrefix, true) );
                if( !aTempFile->IsValid() )
                {
                    ErrorHandler::HandleError( ERRCODE_IO_NOTSUPPORTED );
                    m_aMergeStatus = MergeStatus::Error;
                }
            }

            if( IsMergeOk() )
            {
                std::unique_ptr< INetURLObject > aTempFileURL;
                if( bNeedsTempFiles )
                    aTempFileURL.reset( new INetURLObject(aTempFile->GetURL()));
                if( !bIsMergeSilent ) {
                    if( !bMT_PRINTER )
                        static_cast<CreateMonitor*>(xProgressDlg.get())->SetCurrentPosition(nDocNo);
                    else {
                        PrintMonitor *pPrintMonDlg = static_cast<PrintMonitor*>(xProgressDlg.get());
                        pPrintMonDlg->m_xPrinter->set_label(bNeedsTempFiles
                            ? aTempFileURL->GetBase() : pSourceDocSh->GetTitle( 2));
                        OUString sStat( SwResId(STR_STATSTR_LETTER) );
                        sStat += " " + OUString::number( nDocNo );
                        pPrintMonDlg->m_xPrintInfo->set_label(sStat);
                    }
                    //TODO xProgressDlg->queue_draw();
                }

                Application::Reschedule( true );

                // Create a copy of the source document and work with that one instead of the source.
                // If we're not in the single file mode (which requires modifying the document for the merging),
                // it is enough to do this just once. Currently PDF also has to be treated special.
                if( !bWorkDocInitialized || bCreateSingleFile || bIsPDFexport )
                {
                    assert( !xWorkDocSh.Is());
                    pWorkDocOrigDBManager = this;
                    xWorkDocSh = lcl_CreateWorkingDocument( WorkingDocType::COPY,
                        *pSourceShell, nullptr, &pWorkDocOrigDBManager,
                        &pWorkView, &pWorkShell, &pWorkDoc );
                    if ( (nMaxDumpDocs < 0) || (nDocNo <= nMaxDumpDocs) )
                        lcl_SaveDebugDoc( xWorkDocSh, "WorkDoc", nDocNo );

                    // #i69458# lock fields to prevent access to the result set while calculating layout
                    // tdf#92324: and do not unlock: keep document locked during printing to avoid
                    // ExpFields update during printing, generation of preview, etc.
                    pWorkShell->LockExpFields();
                    pWorkShell->CalcLayout();
                    // tdf#121168: Now force correct page descriptions applied to page frames. Without
                    // this, e.g., page frames starting with sections could have page descriptions set
                    // wrong. This would lead to wrong page styles applied in SwDoc::AppendDoc below.
                    pWorkShell->GetViewOptions()->SetIdle(true);
                    for (auto aLayout : pWorkShell->GetDoc()->GetAllLayouts())
                    {
                        aLayout->FreezeLayout(false);
                        aLayout->AllCheckPageDescs();
                    }
                }

                lcl_emitEvent(SfxEventHintId::SwEventFieldMerge, STR_SW_EVENT_FIELD_MERGE, xWorkDocSh);

                // tdf#92324: Allow ExpFields update only by explicit instruction to avoid
                // database cursor movement on any other fields update, for example during
                // print preview and other operations
                if ( pWorkShell->IsExpFieldsLocked() )
                    pWorkShell->UnlockExpFields();
                pWorkShell->SwViewShell::UpdateFields();
                pWorkShell->LockExpFields();

                lcl_emitEvent(SfxEventHintId::SwEventFieldMergeFinished, STR_SW_EVENT_FIELD_MERGE_FINISHED, xWorkDocSh);

                // also emit MailMergeEvent on XInterface if possible
                const SwXMailMerge *pEvtSrc = GetMailMergeEvtSrc();
                if(pEvtSrc)
                {
                    uno::Reference< uno::XInterface > xRef(
                        static_cast<text::XMailMergeBroadcaster*>(const_cast<SwXMailMerge*>(pEvtSrc)) );
                    text::MailMergeEvent aEvt( xRef, xWorkDocSh->GetModel() );
                    pEvtSrc->LaunchMailMergeEvent( aEvt );
                }

                // working copy is merged - prepare final steps depending on merge options

                if( bCreateSingleFile )
                {
                    assert( pTargetShell && "no target shell available!" );

                    // prepare working copy and target to append

                    pWorkDoc->RemoveInvisibleContent();
                    // remove of invisible content has influence on page count and so on fields for page count,
                    // therefore layout has to be updated before fields are converted to text
                    pWorkShell->CalcLayout();
                    pWorkShell->ConvertFieldsToText();
                    pWorkShell->SetNumberingRestart();
                    if( bSynchronizedDoc )
                    {
                        lcl_RemoveSectionLinks( *pWorkShell );
                    }

                    if ( (nMaxDumpDocs < 0) || (nDocNo <= nMaxDumpDocs) )
                        lcl_SaveDebugDoc( xWorkDocSh, "WorkDoc", nDocNo );

                    // append the working document to the target document
                    if( targetDocPageCount % 2 == 1 )
                        ++targetDocPageCount; // Docs always start on odd pages (so offset must be even).
                    SwNodeIndex appendedDocStart = pTargetDoc->AppendDoc( *pWorkDoc,
                        nStartingPageNo, !bWorkDocInitialized, targetDocPageCount, nDocNo);
                    targetDocPageCount += pWorkShell->GetPageCnt();

                    if ( (nMaxDumpDocs < 0) || (nDocNo <= nMaxDumpDocs) )
                        lcl_SaveDebugDoc( xTargetDocShell.get(), "MergeDoc" );

                    if (bMT_SHELL)
                    {
                        SwDocMergeInfo aMergeInfo;
                        // Name of the mark is actually irrelevant, UNO bookmarks have internals names.
                        aMergeInfo.startPageInTarget = pTargetDoc->getIDocumentMarkAccess()->makeMark(
                            appendedDocStart, "", IDocumentMarkAccess::MarkType::UNO_BOOKMARK,
                            ::sw::mark::InsertMode::New);
                        aMergeInfo.nDBRow = nStartRow;
                        rMergeDescriptor.pMailMergeConfigItem->AddMergedDocument( aMergeInfo );
                    }
                }
                else
                {
                    assert( bNeedsTempFiles );
                    assert( pWorkShell->IsExpFieldsLocked() );

                    // fields are locked, so it's fine to
                    // restore the old / empty DB manager for save
                    pWorkDoc->SetDBManager( pWorkDocOrigDBManager );

                    // save merged document
                    OUString sFileURL;
                    if( !lcl_SaveDoc( aTempFileURL.get(), pStoreToFilter, pStoreToFilterOptions,
                                    &rMergeDescriptor.aSaveToFilterData, bIsPDFexport,
                                    xWorkDocSh, *pWorkShell, &sFileURL ) )
                    {
                        m_aMergeStatus = MergeStatus::Error;
                    }

                    // back to the MM DB manager
                    pWorkDoc->SetDBManager( this );

                    if( bMT_EMAIL && !IsMergeError() )
                    {
                        // schedule file for later removal
                        aFilesToRemove.push_back( sFileURL );

                        if( !SwMailMergeHelper::CheckMailAddress( sColumnData ) )
                        {
                            OSL_FAIL("invalid e-Mail address in database column");
                        }
                        else
                        {
                            uno::Reference< mail::XMailMessage > xMessage = lcl_CreateMailFromDoc(
                                rMergeDescriptor, sFileURL, sColumnData, sMailBodyMimeType,
                                sMailEncoding, pStoreToFilter->GetMimeType() );
                            if( xMessage.is() )
                            {
                                osl::MutexGuard aGuard( m_pImpl->m_aAllEmailSendMutex );
                                m_pImpl->m_xLastMessage.set( xMessage );
                                xMailDispatcher->enqueueMailMessage( xMessage );
                                if( !xMailDispatcher->isStarted() )
                                    xMailDispatcher->start();
                            }
                        }
                    }
                }
                if( bCreateSingleFile || bIsPDFexport )
                {
                    pWorkDoc->SetDBManager( pWorkDocOrigDBManager );
                    xWorkDocSh->DoClose();
                    xWorkDocSh = nullptr;
                }
            }

            bWorkDocInitialized = true;
            nDocNo++;
            nEndRow = m_pImpl->pMergeData ? m_pImpl->pMergeData->xResultSet->getRow() : 0;

            // Freeze the layouts of the target document after the first inserted
            // sub-document, to get the correct PageDesc.
            if(!bFreezedLayouts && bCreateSingleFile)
            {
                for ( auto aLayout : pTargetShell->GetDoc()->GetAllLayouts() )
                    aLayout->FreezeLayout(true);
                bFreezedLayouts = true;
            }
        } while( IsMergeOk() &&
            ((bSynchronizedDoc && (nStartRow != nEndRow)) ? IsValidMergeRecord() : ToNextMergeRecord()));

        if ( xWorkDocSh.Is() && pWorkView->GetWrtShell().IsExpFieldsLocked() )
        {
            // Unlock document fields after merge complete
            pWorkView->GetWrtShell().UnlockExpFields();
        }

        if( !bCreateSingleFile )
        {
            if( bMT_PRINTER )
                Printer::FinishPrintJob( pWorkView->GetPrinterController());
            if( !bIsPDFexport )
            {
                pWorkDoc->SetDBManager( pWorkDocOrigDBManager );
                xWorkDocSh->DoClose();
            }
        }
        else if( IsMergeOk() ) // && bCreateSingleFile
        {
            Application::Reschedule( true );

            // sw::DocumentLayoutManager::CopyLayoutFormat() did not generate
            // unique fly names, do it here once.
            pTargetDoc->SetInMailMerge(false);
            pTargetDoc->SetAllUniqueFlyNames();

            // Unfreeze target document layouts and correct all PageDescs.
            SAL_INFO( "sw.pageframe", "(MergeMailFiles pTargetShell->CalcLayout in" );
            pTargetShell->CalcLayout();
            SAL_INFO( "sw.pageframe", "MergeMailFiles pTargetShell->CalcLayout out)" );
            pTargetShell->GetViewOptions()->SetIdle( true );
            pTargetDoc->GetIDocumentUndoRedo().DoUndo( true );
            for ( auto aLayout : pTargetShell->GetDoc()->GetAllLayouts() )
            {
                aLayout->FreezeLayout(false);
                aLayout->AllCheckPageDescs();
            }

            Application::Reschedule( true );

            if( IsMergeOk() && bMT_FILE )
            {
                // save merged document
                assert( aTempFile.get() );
                INetURLObject aTempFileURL;
                if (sDescriptorPrefix.isEmpty() || !rMergeDescriptor.bPrefixIsFilename)
                    aTempFileURL.SetURL( aTempFile->GetURL() );
                else
                {
                    aTempFileURL.SetURL(sDescriptorPrefix);
                    // remove the unneeded temporary file
                    aTempFile->EnableKillingFile();
                }
                if( !lcl_SaveDoc( &aTempFileURL, pStoreToFilter,
                        pStoreToFilterOptions, &rMergeDescriptor.aSaveToFilterData,
                        bIsPDFexport, xTargetDocShell.get(), *pTargetShell ) )
                {
                    m_aMergeStatus = MergeStatus::Error;
                }
            }
            else if( IsMergeOk() && bMT_PRINTER )
            {
                // print the target document
                uno::Sequence< beans::PropertyValue > aOptions( rMergeDescriptor.aPrintOptions );
                lcl_PreparePrinterOptions( rMergeDescriptor.aPrintOptions, aOptions );
                pTargetView->ExecPrint( aOptions, bIsMergeSilent, false/*bPrintAsync*/ );
            }
        }

        // we also show canceled documents, as long as there was no error
        if( !IsMergeError() && bMT_SHELL )
            // leave docshell available for caller (e.g. MM wizard)
            rMergeDescriptor.pMailMergeConfigItem->SetTargetView( pTargetView );
        else if( xTargetDocShell.is() )
            xTargetDocShell->DoClose();

        Application::Reschedule( true );

        if (xProgressDlg)
        {
            xProgressDlg->response(RET_OK);
        }

        // unlock all dispatchers
        pViewFrame = SfxViewFrame::GetFirst(pSourceDocSh);
        while (pViewFrame)
        {
            pViewFrame->GetDispatcher()->Lock(false);
            pViewFrame = SfxViewFrame::GetNext(*pViewFrame, pSourceDocSh);
        }

        SW_MOD()->SetView(&pSourceShell->GetView());

        if( xMailDispatcher.is() )
        {
            if( IsMergeOk() )
            {
                // TODO: Instead of polling via an AutoTimer, post an Idle event,
                // if the main loop has been made thread-safe.
                AutoTimer aEmailDispatcherPollTimer;
                aEmailDispatcherPollTimer.SetDebugName(
                    "sw::SwDBManager aEmailDispatcherPollTimer" );
                aEmailDispatcherPollTimer.SetTimeout( 500 );
                aEmailDispatcherPollTimer.Start();
                while( IsMergeOk() && m_pImpl->m_xLastMessage.is() )
                    Application::Yield();
                aEmailDispatcherPollTimer.Stop();
            }
            xMailDispatcher->stop();
            xMailDispatcher->shutdown();
        }

        // remove the temporary files
        // has to be done after xMailDispatcher is finished, as mails may be
        // delivered as message attachments!
        for( const OUString &sFileURL : aFilesToRemove )
            SWUnoHelper::UCB_DeleteFile( sFileURL );
    }
    catch (const uno::Exception&)
    {
        if (xProgressDlg)
        {
            xProgressDlg->response(RET_CANCEL);
        }
    }

    return !IsMergeError();
}

void SwDBManager::MergeCancel()
{
    if (m_aMergeStatus < MergeStatus::Cancel)
        m_aMergeStatus = MergeStatus::Cancel;
}

// determine the column's Numberformat and transfer to the forwarded Formatter,
// if applicable.
sal_uLong SwDBManager::GetColumnFormat( const OUString& rDBName,
                                const OUString& rTableName,
                                const OUString& rColNm,
                                SvNumberFormatter* pNFormatr,
                                LanguageType nLanguage )
{
    sal_uLong nRet = 0;
    if(pNFormatr)
    {
        uno::Reference< sdbc::XDataSource> xSource;
        uno::Reference< sdbc::XConnection> xConnection;
        bool bUseMergeData = false;
        uno::Reference< sdbcx::XColumnsSupplier> xColsSupp;
        bool bDisposeConnection = false;
        if(m_pImpl->pMergeData &&
            ((m_pImpl->pMergeData->sDataSource == rDBName && m_pImpl->pMergeData->sCommand == rTableName) ||
            (rDBName.isEmpty() && rTableName.isEmpty())))
        {
            xConnection = m_pImpl->pMergeData->xConnection;
            xSource = SwDBManager::getDataSourceAsParent(xConnection,rDBName);
            bUseMergeData = true;
            xColsSupp.set(m_pImpl->pMergeData->xResultSet, css::uno::UNO_QUERY);
        }
        if(!xConnection.is())
        {
            SwDBData aData;
            aData.sDataSource = rDBName;
            aData.sCommand = rTableName;
            aData.nCommandType = -1;
            SwDSParam* pParam = FindDSData(aData, false);
            if(pParam && pParam->xConnection.is())
            {
                xConnection = pParam->xConnection;
                xColsSupp.set(pParam->xResultSet, css::uno::UNO_QUERY);
            }
            else
            {
                xConnection = RegisterConnection( rDBName );
                bDisposeConnection = true;
            }
            if(bUseMergeData)
                m_pImpl->pMergeData->xConnection = xConnection;
        }
        bool bDispose = !xColsSupp.is();
        if(bDispose)
        {
            xColsSupp = SwDBManager::GetColumnSupplier(xConnection, rTableName);
        }
        if(xColsSupp.is())
        {
            uno::Reference<container::XNameAccess> xCols;
            try
            {
                xCols = xColsSupp->getColumns();
            }
            catch (const uno::Exception& e)
            {
                SAL_WARN("sw.mailmerge", "Exception in getColumns(): " << e);
            }
            if(!xCols.is() || !xCols->hasByName(rColNm))
                return nRet;
            uno::Any aCol = xCols->getByName(rColNm);
            uno::Reference< beans::XPropertySet > xColumn;
            aCol >>= xColumn;
            nRet = GetColumnFormat(xSource, xConnection, xColumn, pNFormatr, nLanguage);
            if(bDispose)
            {
                ::comphelper::disposeComponent( xColsSupp );
            }
            if(bDisposeConnection)
            {
                ::comphelper::disposeComponent( xConnection );
            }
        }
        else
            nRet = pNFormatr->GetFormatIndex( NF_NUMBER_STANDARD, LANGUAGE_SYSTEM );
    }
    return nRet;
}

sal_uLong SwDBManager::GetColumnFormat( uno::Reference< sdbc::XDataSource> const & xSource_in,
                        uno::Reference< sdbc::XConnection> const & xConnection,
                        uno::Reference< beans::XPropertySet> const & xColumn,
                        SvNumberFormatter* pNFormatr,
                        LanguageType nLanguage )
{
    auto xSource = xSource_in;

    // set the NumberFormat in the doc if applicable
    sal_uLong nRet = 0;

    if(!xSource.is())
    {
        uno::Reference<container::XChild> xChild(xConnection, uno::UNO_QUERY);
        if ( xChild.is() )
            xSource.set(xChild->getParent(), uno::UNO_QUERY);
    }
    if(xSource.is() && xConnection.is() && xColumn.is() && pNFormatr)
    {
        SvNumberFormatsSupplierObj* pNumFormat = new SvNumberFormatsSupplierObj( pNFormatr );
        uno::Reference< util::XNumberFormatsSupplier >  xDocNumFormatsSupplier = pNumFormat;
        uno::Reference< util::XNumberFormats > xDocNumberFormats = xDocNumFormatsSupplier->getNumberFormats();
        uno::Reference< util::XNumberFormatTypes > xDocNumberFormatTypes(xDocNumberFormats, uno::UNO_QUERY);

        css::lang::Locale aLocale( LanguageTag( nLanguage ).getLocale());

        //get the number formatter of the data source
        uno::Reference<beans::XPropertySet> xSourceProps(xSource, uno::UNO_QUERY);
        uno::Reference< util::XNumberFormats > xNumberFormats;
        if(xSourceProps.is())
        {
            uno::Any aFormats = xSourceProps->getPropertyValue("NumberFormatsSupplier");
            if(aFormats.hasValue())
            {
                uno::Reference<util::XNumberFormatsSupplier> xSuppl;
                aFormats >>= xSuppl;
                if(xSuppl.is())
                {
                    xNumberFormats = xSuppl->getNumberFormats();
                }
            }
        }
        bool bUseDefault = true;
        try
        {
            uno::Any aFormatKey = xColumn->getPropertyValue("FormatKey");
            if(aFormatKey.hasValue())
            {
                sal_Int32 nFormat = 0;
                aFormatKey >>= nFormat;
                if(xNumberFormats.is())
                {
                    try
                    {
                        uno::Reference<beans::XPropertySet> xNumProps = xNumberFormats->getByKey( nFormat );
                        uno::Any aFormatString = xNumProps->getPropertyValue("FormatString");
                        uno::Any aLocaleVal = xNumProps->getPropertyValue("Locale");
                        OUString sFormat;
                        aFormatString >>= sFormat;
                        lang::Locale aLoc;
                        aLocaleVal >>= aLoc;
                        nFormat = xDocNumberFormats->queryKey( sFormat, aLoc, false );
                        if(NUMBERFORMAT_ENTRY_NOT_FOUND == sal::static_int_cast< sal_uInt32, sal_Int32>(nFormat))
                            nFormat = xDocNumberFormats->addNew( sFormat, aLoc );
                        nRet = nFormat;
                        bUseDefault = false;
                    }
                    catch (const uno::Exception& e)
                    {
                        SAL_WARN("sw.mailmerge", "illegal number format key: " << e);
                    }
                }
            }
        }
        catch(const uno::Exception&)
        {
            SAL_WARN("sw.mailmerge", "no FormatKey property found");
        }
        if(bUseDefault)
            nRet = dbtools::getDefaultNumberFormat(xColumn, xDocNumberFormatTypes,  aLocale);
    }
    return nRet;
}

sal_Int32 SwDBManager::GetColumnType( const OUString& rDBName,
                          const OUString& rTableName,
                          const OUString& rColNm )
{
    sal_Int32 nRet = sdbc::DataType::SQLNULL;
    SwDBData aData;
    aData.sDataSource = rDBName;
    aData.sCommand = rTableName;
    aData.nCommandType = -1;
    SwDSParam* pParam = FindDSData(aData, false);
    uno::Reference< sdbc::XConnection> xConnection;
    uno::Reference< sdbcx::XColumnsSupplier > xColsSupp;
    bool bDispose = false;
    if(pParam && pParam->xConnection.is())
    {
        xConnection = pParam->xConnection;
        xColsSupp.set( pParam->xResultSet, uno::UNO_QUERY );
    }
    else
    {
        xConnection = RegisterConnection( rDBName );
    }
    if( !xColsSupp.is() )
    {
        xColsSupp = SwDBManager::GetColumnSupplier(xConnection, rTableName);
        bDispose = true;
    }
    if(xColsSupp.is())
    {
        uno::Reference<container::XNameAccess> xCols = xColsSupp->getColumns();
        if(xCols->hasByName(rColNm))
        {
            uno::Any aCol = xCols->getByName(rColNm);
            uno::Reference<beans::XPropertySet> xCol;
            aCol >>= xCol;
            uno::Any aType = xCol->getPropertyValue("Type");
            aType >>= nRet;
        }
        if(bDispose)
            ::comphelper::disposeComponent( xColsSupp );
    }
    return nRet;
}

uno::Reference< sdbc::XConnection> SwDBManager::GetConnection(const OUString& rDataSource,
                                                              uno::Reference<sdbc::XDataSource>& rxSource, const SwView *pView)
{
    uno::Reference< sdbc::XConnection> xConnection;
    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    try
    {
        uno::Reference<sdb::XCompletedConnection> xComplConnection(dbtools::getDataSource(rDataSource, xContext), uno::UNO_QUERY);
        if ( xComplConnection.is() )
        {
            rxSource.set(xComplConnection, uno::UNO_QUERY);
            weld::Window* pWindow = pView ? pView->GetFrameWeld() : nullptr;
            uno::Reference< task::XInteractionHandler > xHandler( task::InteractionHandler::createWithParent(xContext, pWindow ? pWindow->GetXWindow() : nullptr), uno::UNO_QUERY_THROW );
            xConnection = xComplConnection->connectWithCompletion( xHandler );
        }
    }
    catch(const uno::Exception&)
    {
    }

    return xConnection;
}

uno::Reference< sdbcx::XColumnsSupplier> SwDBManager::GetColumnSupplier(uno::Reference<sdbc::XConnection> const & xConnection,
                                    const OUString& rTableOrQuery,
                                    SwDBSelect   eTableOrQuery)
{
    uno::Reference< sdbcx::XColumnsSupplier> xRet;
    try
    {
        if(eTableOrQuery == SwDBSelect::UNKNOWN)
        {
            //search for a table with the given command name
            uno::Reference<sdbcx::XTablesSupplier> xTSupplier(xConnection, uno::UNO_QUERY);
            if(xTSupplier.is())
            {
                uno::Reference<container::XNameAccess> xTables = xTSupplier->getTables();
                eTableOrQuery = xTables->hasByName(rTableOrQuery) ?
                            SwDBSelect::TABLE : SwDBSelect::QUERY;
            }
        }
        sal_Int32 nCommandType = SwDBSelect::TABLE == eTableOrQuery ?
                sdb::CommandType::TABLE : sdb::CommandType::QUERY;
        uno::Reference< lang::XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
        uno::Reference<sdbc::XRowSet> xRowSet(xMgr->createInstance("com.sun.star.sdb.RowSet"), uno::UNO_QUERY);

        OUString sDataSource;
        uno::Reference<sdbc::XDataSource> xSource = SwDBManager::getDataSourceAsParent(xConnection, sDataSource);
        uno::Reference<beans::XPropertySet> xSourceProperties(xSource, uno::UNO_QUERY);
        if(xSourceProperties.is())
        {
            xSourceProperties->getPropertyValue("Name") >>= sDataSource;
        }

        uno::Reference<beans::XPropertySet> xRowProperties(xRowSet, uno::UNO_QUERY);
        xRowProperties->setPropertyValue("DataSourceName", uno::makeAny(sDataSource));
        xRowProperties->setPropertyValue("Command", uno::makeAny(rTableOrQuery));
        xRowProperties->setPropertyValue("CommandType", uno::makeAny(nCommandType));
        xRowProperties->setPropertyValue("FetchSize", uno::makeAny(sal_Int32(10)));
        xRowProperties->setPropertyValue("ActiveConnection", uno::makeAny(xConnection));
        xRowSet->execute();
        xRet.set( xRowSet, uno::UNO_QUERY );
    }
    catch (const uno::Exception& e)
    {
        SAL_WARN("sw.mailmerge", "Exception in SwDBManager::GetColumnSupplier: " << e);
    }

    return xRet;
}

OUString SwDBManager::GetDBField(uno::Reference<beans::XPropertySet> const & xColumnProps,
                        const SwDBFormatData& rDBFormatData,
                        double* pNumber)
{
    uno::Reference< sdb::XColumn > xColumn(xColumnProps, uno::UNO_QUERY);
    OUString sRet;
    assert( xColumn.is() && "SwDBManager::::ImportDBField: illegal arguments" );
    if(!xColumn.is())
        return sRet;

    uno::Any aType = xColumnProps->getPropertyValue("Type");
    sal_Int32 eDataType = sdbc::DataType::SQLNULL;
    aType >>= eDataType;
    switch(eDataType)
    {
        case sdbc::DataType::CHAR:
        case sdbc::DataType::VARCHAR:
        case sdbc::DataType::LONGVARCHAR:
            try
            {
                sRet = xColumn->getString();
                sRet = sRet.replace( '\xb', '\n' ); // MSWord uses \xb as a newline
            }
            catch(const sdbc::SQLException&)
            {
            }
        break;
        case sdbc::DataType::BIT:
        case sdbc::DataType::BOOLEAN:
        case sdbc::DataType::TINYINT:
        case sdbc::DataType::SMALLINT:
        case sdbc::DataType::INTEGER:
        case sdbc::DataType::BIGINT:
        case sdbc::DataType::FLOAT:
        case sdbc::DataType::REAL:
        case sdbc::DataType::DOUBLE:
        case sdbc::DataType::NUMERIC:
        case sdbc::DataType::DECIMAL:
        case sdbc::DataType::DATE:
        case sdbc::DataType::TIME:
        case sdbc::DataType::TIMESTAMP:
        {

            try
            {
                sRet = dbtools::DBTypeConversion::getFormattedValue(
                    xColumnProps,
                    rDBFormatData.xFormatter,
                    rDBFormatData.aLocale,
                    rDBFormatData.aNullDate);
                if (pNumber)
                {
                    double fVal = xColumn->getDouble();
                    if(!xColumn->wasNull())
                    {
                        *pNumber = fVal;
                    }
                }
            }
            catch (const uno::Exception&)
            {
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN("sw.mailmerge", "exception caught: " << exceptionToString(ex));
            }

        }
        break;
    }

    return sRet;
}

// checks if a desired data source table or query is open
bool    SwDBManager::IsDataSourceOpen(const OUString& rDataSource,
                                  const OUString& rTableOrQuery, bool bMergeShell)
{
    if(m_pImpl->pMergeData)
    {
        return ((rDataSource == m_pImpl->pMergeData->sDataSource
                 && rTableOrQuery == m_pImpl->pMergeData->sCommand)
                || (rDataSource.isEmpty() && rTableOrQuery.isEmpty()))
               && m_pImpl->pMergeData->xResultSet.is();
    }
    else if(!bMergeShell)
    {
        SwDBData aData;
        aData.sDataSource = rDataSource;
        aData.sCommand = rTableOrQuery;
        aData.nCommandType = -1;
        SwDSParam* pFound = FindDSData(aData, false);
        return (pFound && pFound->xResultSet.is());
    }
    return false;
}

// read column data at a specified position
bool SwDBManager::GetColumnCnt(const OUString& rSourceName, const OUString& rTableName,
                           const OUString& rColumnName, sal_uInt32 nAbsRecordId,
                           LanguageType nLanguage,
                           OUString& rResult, double* pNumber)
{
    bool bRet = false;
    SwDSParam* pFound = nullptr;
    //check if it's the merge data source
    if(m_pImpl->pMergeData &&
        rSourceName == m_pImpl->pMergeData->sDataSource &&
        rTableName == m_pImpl->pMergeData->sCommand)
    {
        pFound = m_pImpl->pMergeData.get();
    }
    else
    {
        SwDBData aData;
        aData.sDataSource = rSourceName;
        aData.sCommand = rTableName;
        aData.nCommandType = -1;
        pFound = FindDSData(aData, false);
    }
    if (!pFound)
        return false;
    //check validity of supplied record Id
    if(pFound->aSelection.getLength())
    {
        //the destination has to be an element of the selection
        const uno::Any* pSelection = pFound->aSelection.getConstArray();
        bool bFound = false;
        for(sal_Int32 nPos = 0; !bFound && nPos < pFound->aSelection.getLength(); nPos++)
        {
            sal_Int32 nSelection = 0;
            pSelection[nPos] >>= nSelection;
            if(nSelection == static_cast<sal_Int32>(nAbsRecordId))
                bFound = true;
        }
        if(!bFound)
            return false;
    }
    if( pFound->HasValidRecord() )
    {
        sal_Int32 nOldRow = 0;
        try
        {
            nOldRow = pFound->xResultSet->getRow();
        }
        catch(const uno::Exception&)
        {
            return false;
        }
        //position to the desired index
        bool bMove = true;
        if ( nOldRow != static_cast<sal_Int32>(nAbsRecordId) )
            bMove = lcl_MoveAbsolute(pFound, nAbsRecordId);
        if(bMove)
            bRet = lcl_GetColumnCnt(pFound, rColumnName, nLanguage, rResult, pNumber);
        if ( nOldRow != static_cast<sal_Int32>(nAbsRecordId) )
            lcl_MoveAbsolute(pFound, nOldRow);
    }
    return bRet;
}

// reads the column data at the current position
bool    SwDBManager::GetMergeColumnCnt(const OUString& rColumnName, LanguageType nLanguage,
                                   OUString &rResult, double *pNumber)
{
    if( !IsValidMergeRecord() )
    {
        rResult.clear();
        return false;
    }

    bool bRet = lcl_GetColumnCnt(m_pImpl->pMergeData.get(), rColumnName, nLanguage, rResult, pNumber);
    return bRet;
}

bool SwDBManager::ToNextMergeRecord()
{
    assert( m_pImpl->pMergeData && m_pImpl->pMergeData->xResultSet.is() && "no data source in merge" );
    return lcl_ToNextRecord( m_pImpl->pMergeData.get() );
}

bool SwDBManager::FillCalcWithMergeData( SvNumberFormatter *pDocFormatter,
                                         LanguageType nLanguage, SwCalc &rCalc )
{
    if( !IsValidMergeRecord() )
        return false;

    uno::Reference< sdbcx::XColumnsSupplier > xColsSupp( m_pImpl->pMergeData->xResultSet, uno::UNO_QUERY );
    if( !xColsSupp.is() )
        return false;

    {
        uno::Reference<container::XNameAccess> xCols = xColsSupp->getColumns();
        const uno::Sequence<OUString> aColNames = xCols->getElementNames();
        const OUString* pColNames = aColNames.getConstArray();
        OUString aString;

        // add the "record number" variable, as SwCalc::VarLook would.
        rCalc.VarChange( GetAppCharClass().lowercase(
            SwFieldType::GetTypeStr(TYP_DBSETNUMBERFLD) ), GetSelectedRecordId() );

        for( int nCol = 0; nCol < aColNames.getLength(); nCol++ )
        {
            // get the column type
            sal_Int32 nColumnType = sdbc::DataType::SQLNULL;
            uno::Any aCol = xCols->getByName( pColNames[nCol] );
            uno::Reference<beans::XPropertySet> xColumnProps;
            aCol >>= xColumnProps;
            uno::Any aType = xColumnProps->getPropertyValue( "Type" );
            aType >>= nColumnType;
            double aNumber = DBL_MAX;

            lcl_GetColumnCnt( m_pImpl->pMergeData.get(), xColumnProps, nLanguage, aString, &aNumber );

            sal_uInt32 nFormat = GetColumnFormat( m_pImpl->pMergeData->sDataSource,
                                            m_pImpl->pMergeData->sCommand,
                                            pColNames[nCol], pDocFormatter, nLanguage );
            // aNumber is overwritten by SwDBField::FormatValue, so store initial status
            bool colIsNumber = aNumber != DBL_MAX;
            bool bValidValue = SwDBField::FormatValue( pDocFormatter, aString, nFormat,
                                                       aNumber, nColumnType );
            if( colIsNumber )
            {
                if( bValidValue )
                {
                    SwSbxValue aValue;
                    aValue.PutDouble( aNumber );
                    aValue.SetDBvalue( true );
                    SAL_INFO( "sw.ui", "'" << pColNames[nCol] << "': " << aNumber << " / " << aString );
                    rCalc.VarChange( pColNames[nCol], aValue );
                }
            }
            else
            {
                SwSbxValue aValue;
                aValue.PutString( aString );
                aValue.SetDBvalue( true );
                SAL_INFO( "sw.ui", "'" << pColNames[nCol] << "': " << aString );
                rCalc.VarChange( pColNames[nCol], aValue );
            }
        }
    }

    return true;
}

void SwDBManager::ToNextRecord(
    const OUString& rDataSource, const OUString& rCommand)
{
    SwDSParam* pFound = nullptr;
    if(m_pImpl->pMergeData &&
        rDataSource == m_pImpl->pMergeData->sDataSource &&
        rCommand == m_pImpl->pMergeData->sCommand)
    {
        pFound = m_pImpl->pMergeData.get();
    }
    else
    {
        SwDBData aData;
        aData.sDataSource = rDataSource;
        aData.sCommand = rCommand;
        aData.nCommandType = -1;
        pFound = FindDSData(aData, false);
    }
    lcl_ToNextRecord( pFound );
}

static bool lcl_ToNextRecord( SwDSParam* pParam, const SwDBNextRecord action )
{
    bool bRet = true;

    assert( SwDBNextRecord::NEXT == action ||
         (SwDBNextRecord::FIRST == action && pParam) );
    if( nullptr == pParam )
        return false;

    if( action == SwDBNextRecord::FIRST )
    {
        pParam->nSelectionIndex = 0;
        pParam->bEndOfDB        = false;
    }

    if( !pParam->HasValidRecord() )
        return false;

    try
    {
        if( pParam->aSelection.getLength() )
        {
            if( pParam->nSelectionIndex >= pParam->aSelection.getLength() )
                pParam->bEndOfDB = true;
            else
            {
                sal_Int32 nPos = 0;
                pParam->aSelection.getConstArray()[ pParam->nSelectionIndex ] >>= nPos;
                pParam->bEndOfDB = !pParam->xResultSet->absolute( nPos );
            }
        }
        else if( action == SwDBNextRecord::FIRST )
        {
            pParam->bEndOfDB = !pParam->xResultSet->first();
        }
        else
        {
            sal_Int32 nBefore = pParam->xResultSet->getRow();
            pParam->bEndOfDB = !pParam->xResultSet->next();
            if( !pParam->bEndOfDB && nBefore == pParam->xResultSet->getRow() )
            {
                // next returned true but it didn't move
                ::dbtools::throwFunctionSequenceException( pParam->xResultSet );
            }
        }

        ++pParam->nSelectionIndex;
        bRet = !pParam->bEndOfDB;
    }
    catch( const uno::Exception &e )
    {
        pParam->bEndOfDB = true;
        bRet = false;
        // we allow merging with empty databases, so don't warn on init
        SAL_WARN_IF(action == SwDBNextRecord::NEXT,
                    "sw.mailmerge", "exception in ToNextRecord(): " << e);
    }
    return bRet;
}

// synchronized labels contain a next record field at their end
// to assure that the next page can be created in mail merge
// the cursor position must be validated
bool SwDBManager::IsValidMergeRecord() const
{
    return( m_pImpl->pMergeData && m_pImpl->pMergeData->HasValidRecord() );
}

sal_uInt32  SwDBManager::GetSelectedRecordId()
{
    sal_uInt32  nRet = 0;
    assert( m_pImpl->pMergeData &&
            m_pImpl->pMergeData->xResultSet.is() && "no data source in merge" );
    if(!m_pImpl->pMergeData || !m_pImpl->pMergeData->xResultSet.is())
        return 0;
    try
    {
        nRet = m_pImpl->pMergeData->xResultSet->getRow();
    }
    catch(const uno::Exception&)
    {
    }
    return nRet;
}

bool SwDBManager::ToRecordId(sal_Int32 nSet)
{
    assert( m_pImpl->pMergeData &&
            m_pImpl->pMergeData->xResultSet.is() && "no data source in merge" );
    if(!m_pImpl->pMergeData || !m_pImpl->pMergeData->xResultSet.is()|| nSet < 0)
        return false;
    bool bRet = false;
    sal_Int32 nAbsPos = nSet;
    assert(nAbsPos >= 0);
    bRet = lcl_MoveAbsolute(m_pImpl->pMergeData.get(), nAbsPos);
    m_pImpl->pMergeData->bEndOfDB = !bRet;
    return bRet;
}

bool SwDBManager::OpenDataSource(const OUString& rDataSource, const OUString& rTableOrQuery)
{
    SwDBData aData;
    aData.sDataSource = rDataSource;
    aData.sCommand = rTableOrQuery;
    aData.nCommandType = -1;

    SwDSParam* pFound = FindDSData(aData, true);
    if(pFound->xResultSet.is())
        return true;
    SwDSParam* pParam = FindDSConnection(rDataSource, false);
    if(pParam && pParam->xConnection.is())
        pFound->xConnection = pParam->xConnection;
    if(pFound->xConnection.is())
    {
        try
        {
            uno::Reference< sdbc::XDatabaseMetaData >  xMetaData = pFound->xConnection->getMetaData();
            try
            {
                pFound->bScrollable = xMetaData
                        ->supportsResultSetType(sal_Int32(sdbc::ResultSetType::SCROLL_INSENSITIVE));
            }
            catch(const uno::Exception&)
            {
                // DB driver may not be ODBC 3.0 compliant
                pFound->bScrollable = true;
            }
            pFound->xStatement = pFound->xConnection->createStatement();
            OUString aQuoteChar = xMetaData->getIdentifierQuoteString();
            OUString sStatement("SELECT * FROM ");
            sStatement = "SELECT * FROM ";
            sStatement += aQuoteChar;
            sStatement += rTableOrQuery;
            sStatement += aQuoteChar;
            pFound->xResultSet = pFound->xStatement->executeQuery( sStatement );

            //after executeQuery the cursor must be positioned
            pFound->bEndOfDB = !pFound->xResultSet->next();
            ++pFound->nSelectionIndex;
        }
        catch (const uno::Exception&)
        {
            pFound->xResultSet = nullptr;
            pFound->xStatement = nullptr;
            pFound->xConnection = nullptr;
        }
    }
    return pFound->xResultSet.is();
}

uno::Reference< sdbc::XConnection> const & SwDBManager::RegisterConnection(OUString const& rDataSource)
{
    SwDSParam* pFound = SwDBManager::FindDSConnection(rDataSource, true);
    uno::Reference< sdbc::XDataSource> xSource;
    if(!pFound->xConnection.is())
    {
        SwView* pView = (m_pDoc && m_pDoc->GetDocShell()) ? m_pDoc->GetDocShell()->GetView() : nullptr;
        pFound->xConnection = SwDBManager::GetConnection(rDataSource, xSource, pView);
        try
        {
            uno::Reference<lang::XComponent> xComponent(pFound->xConnection, uno::UNO_QUERY);
            if(xComponent.is())
                xComponent->addEventListener(m_pImpl->m_xDisposeListener.get());
        }
        catch(const uno::Exception&)
        {
        }
    }
    return pFound->xConnection;
}

sal_uInt32      SwDBManager::GetSelectedRecordId(
    const OUString& rDataSource, const OUString& rTableOrQuery, sal_Int32 nCommandType)
{
    sal_uInt32 nRet = 0xffffffff;
    //check for merge data source first
    if(m_pImpl->pMergeData &&
        ((rDataSource == m_pImpl->pMergeData->sDataSource &&
        rTableOrQuery == m_pImpl->pMergeData->sCommand) ||
        (rDataSource.isEmpty() && rTableOrQuery.isEmpty())) &&
        (nCommandType == -1 || nCommandType == m_pImpl->pMergeData->nCommandType) &&
        m_pImpl->pMergeData->xResultSet.is())
    {
        nRet = GetSelectedRecordId();
    }
    else
    {
        SwDBData aData;
        aData.sDataSource = rDataSource;
        aData.sCommand = rTableOrQuery;
        aData.nCommandType = nCommandType;
        SwDSParam* pFound = FindDSData(aData, false);
        if(pFound && pFound->xResultSet.is())
        {
            try
            {   //if a selection array is set the current row at the result set may not be set yet
                if(pFound->aSelection.getLength())
                {
                    sal_Int32 nSelIndex = pFound->nSelectionIndex;
                    if(nSelIndex >= pFound->aSelection.getLength())
                        nSelIndex = pFound->aSelection.getLength() -1;
                    pFound->aSelection.getConstArray()[nSelIndex] >>= nRet;

                }
                else
                    nRet = pFound->xResultSet->getRow();
            }
            catch(const uno::Exception&)
            {
            }
        }
    }
    return nRet;
}

// close all data sources - after fields were updated
void    SwDBManager::CloseAll(bool bIncludingMerge)
{
    //the only thing done here is to reset the selection index
    //all connections stay open
    for (auto & pParam : m_DataSourceParams)
    {
        if (bIncludingMerge || pParam.get() != m_pImpl->pMergeData.get())
        {
            pParam->nSelectionIndex = 0;
            pParam->bEndOfDB = false;
            try
            {
                if(!m_bInMerge && pParam->xResultSet.is())
                    pParam->xResultSet->first();
            }
            catch(const uno::Exception&)
            {}
        }
    }
}

SwDSParam* SwDBManager::FindDSData(const SwDBData& rData, bool bCreate)
{
    //prefer merge data if available
    if(m_pImpl->pMergeData &&
        ((rData.sDataSource == m_pImpl->pMergeData->sDataSource &&
        rData.sCommand == m_pImpl->pMergeData->sCommand) ||
        (rData.sDataSource.isEmpty() && rData.sCommand.isEmpty())) &&
        (rData.nCommandType == -1 || rData.nCommandType == m_pImpl->pMergeData->nCommandType ||
        (bCreate && m_pImpl->pMergeData->nCommandType == -1)))
    {
        return m_pImpl->pMergeData.get();
    }

    SwDSParam* pFound = nullptr;
    for (size_t nPos = m_DataSourceParams.size(); nPos; nPos--)
    {
        SwDSParam* pParam = m_DataSourceParams[nPos - 1].get();
        if(rData.sDataSource == pParam->sDataSource &&
            rData.sCommand == pParam->sCommand &&
            (rData.nCommandType == -1 || rData.nCommandType == pParam->nCommandType ||
            (bCreate && pParam->nCommandType == -1)))
            {
                // calls from the calculator may add a connection with an invalid commandtype
                //later added "real" data base connections have to re-use the already available
                //DSData and set the correct CommandType
                if(bCreate && pParam->nCommandType == -1)
                    pParam->nCommandType = rData.nCommandType;
                pFound = pParam;
                break;
            }
    }
    if(bCreate && !pFound)
    {
        pFound = new SwDSParam(rData);
        m_DataSourceParams.push_back(std::unique_ptr<SwDSParam>(pFound));
        try
        {
            uno::Reference<lang::XComponent> xComponent(pFound->xConnection, uno::UNO_QUERY);
            if(xComponent.is())
                xComponent->addEventListener(m_pImpl->m_xDisposeListener.get());
        }
        catch(const uno::Exception&)
        {
        }
    }
    return pFound;
}

SwDSParam*  SwDBManager::FindDSConnection(const OUString& rDataSource, bool bCreate)
{
    //prefer merge data if available
    if(m_pImpl->pMergeData && rDataSource == m_pImpl->pMergeData->sDataSource )
    {
        SetAsUsed(rDataSource);
        return m_pImpl->pMergeData.get();
    }
    SwDSParam* pFound = nullptr;
    for (auto & pParam : m_DataSourceParams)
    {
        if(rDataSource == pParam->sDataSource)
        {
            SetAsUsed(rDataSource);
            pFound = pParam.get();
            break;
        }
    }
    if(bCreate && !pFound)
    {
        SwDBData aData;
        aData.sDataSource = rDataSource;
        pFound = new SwDSParam(aData);
        SetAsUsed(rDataSource);
        m_DataSourceParams.push_back(std::unique_ptr<SwDSParam>(pFound));
        try
        {
            uno::Reference<lang::XComponent> xComponent(pFound->xConnection, uno::UNO_QUERY);
            if(xComponent.is())
                xComponent->addEventListener(m_pImpl->m_xDisposeListener.get());
        }
        catch(const uno::Exception&)
        {
        }
    }
    return pFound;
}

const SwDBData& SwDBManager::GetAddressDBName()
{
    return SW_MOD()->GetDBConfig()->GetAddressSource();
}

uno::Sequence<OUString> SwDBManager::GetExistingDatabaseNames()
{
    uno::Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference<sdb::XDatabaseContext> xDBContext = sdb::DatabaseContext::create(xContext);
    return xDBContext->getElementNames();
}

namespace  sw
{
DBConnURIType GetDBunoType(const INetURLObject &rURL)
{
    OUString sExt(rURL.GetExtension());
    DBConnURIType type = DBConnURIType::UNKNOWN;

    if (sExt == "odb")
    {
        type = DBConnURIType::ODB;
    }
    else if (sExt.equalsIgnoreAsciiCase("sxc")
        || sExt.equalsIgnoreAsciiCase("ods")
        || sExt.equalsIgnoreAsciiCase("xls")
        || sExt.equalsIgnoreAsciiCase("xlsx"))
    {
        type = DBConnURIType::CALC;
    }
    else if (sExt.equalsIgnoreAsciiCase("sxw") || sExt.equalsIgnoreAsciiCase("odt") || sExt.equalsIgnoreAsciiCase("doc") || sExt.equalsIgnoreAsciiCase("docx"))
    {
        type = DBConnURIType::WRITER;
    }
    else if (sExt.equalsIgnoreAsciiCase("dbf"))
    {
        type = DBConnURIType::DBASE;
    }
    else if (sExt.equalsIgnoreAsciiCase("csv") || sExt.equalsIgnoreAsciiCase("txt"))
    {
        type = DBConnURIType::FLAT;
    }
#ifdef _WIN32
    else if (sExt.equalsIgnoreAsciiCase("mdb") || sExt.equalsIgnoreAsciiCase("mde"))
    {
        type = DBConnURIType::MSJET;
    }
    else if (sExt.equalsIgnoreAsciiCase("accdb") || sExt.equalsIgnoreAsciiCase("accde"))
    {
        type = DBConnURIType::MSACE;
    }
#endif
    return type;
}
}

namespace
{
uno::Any GetDBunoURI(const INetURLObject &rURL, DBConnURIType& rType)
{
    uno::Any aURLAny;

    if (rType == DBConnURIType::UNKNOWN)
        rType = GetDBunoType(rURL);

    switch (rType) {
    case DBConnURIType::UNKNOWN:
    case DBConnURIType::ODB:
        break;
    case DBConnURIType::CALC:
    {
        OUString sDBURL("sdbc:calc:");
        sDBURL += rURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);
        aURLAny <<= sDBURL;
    }
    break;
    case DBConnURIType::WRITER:
    {
        OUString sDBURL("sdbc:writer:");
        sDBURL += rURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);
        aURLAny <<= sDBURL;
    }
    break;
    case DBConnURIType::DBASE:
    {
        INetURLObject aUrlTmp(rURL);
        aUrlTmp.removeSegment();
        aUrlTmp.removeFinalSlash();
        OUString sDBURL("sdbc:dbase:");
        sDBURL += aUrlTmp.GetMainURL(INetURLObject::DecodeMechanism::NONE);
        aURLAny <<= sDBURL;
    }
    break;
    case DBConnURIType::FLAT:
    {
        INetURLObject aUrlTmp(rURL);
        aUrlTmp.removeSegment();
        aUrlTmp.removeFinalSlash();
        OUString sDBURL("sdbc:flat:");
        //only the 'path' has to be added
        sDBURL += aUrlTmp.GetMainURL(INetURLObject::DecodeMechanism::NONE);
        aURLAny <<= sDBURL;
    }
    break;
    case DBConnURIType::MSJET:
#ifdef _WIN32
    {
        OUString sDBURL("sdbc:ado:access:PROVIDER=Microsoft.Jet.OLEDB.4.0;DATA SOURCE=");
        sDBURL += rURL.PathToFileName();
        aURLAny <<= sDBURL;
    }
#endif
    break;
    case DBConnURIType::MSACE:
#ifdef _WIN32
    {
        OUString sDBURL("sdbc:ado:PROVIDER=Microsoft.ACE.OLEDB.12.0;DATA SOURCE=");
        sDBURL += rURL.PathToFileName();
        aURLAny <<= sDBURL;
    }
#endif
    break;
    }
    return aURLAny;
}

/// Returns the URL of this SwDoc.
OUString getOwnURL(SfxObjectShell const * pDocShell)
{
    OUString aRet;

    if (!pDocShell)
        return aRet;

    const INetURLObject& rURLObject = pDocShell->GetMedium()->GetURLObject();
    aRet = rURLObject.GetMainURL(INetURLObject::DecodeMechanism::NONE);
    return aRet;
}

/**
Loads a data source from file and registers it.

In case of success it returns the registered name, otherwise an empty string.
Optionally add a prefix to the registered DB name.
*/
OUString LoadAndRegisterDataSource_Impl(DBConnURIType type, const uno::Reference< beans::XPropertySet > *pSettings,
    const INetURLObject &rURL, const OUString *pDestDir, SfxObjectShell* pDocShell)
{
    OUString sExt(rURL.GetExtension());
    uno::Any aTableFilterAny;
    uno::Any aSuppressVersionsAny;
    uno::Any aInfoAny;
    bool bStore = true;
    OUString sFind;
    uno::Sequence<OUString> aFilters(1);

    uno::Any aURLAny = GetDBunoURI(rURL, type);
    switch (type) {
    case DBConnURIType::UNKNOWN:
    case DBConnURIType::CALC:
    case DBConnURIType::WRITER:
        break;
    case DBConnURIType::ODB:
        bStore = false;
        break;
    case DBConnURIType::FLAT:
    case DBConnURIType::DBASE:
        //set the filter to the file name without extension
        aFilters[0] = rURL.getBase();
        aTableFilterAny <<= aFilters;
        break;
    case DBConnURIType::MSJET:
    case DBConnURIType::MSACE:
        aSuppressVersionsAny <<= true;
        break;
    }

    try
    {
        uno::Reference<uno::XComponentContext> xContext(::comphelper::getProcessComponentContext());
        uno::Reference<sdb::XDatabaseContext> xDBContext = sdb::DatabaseContext::create(xContext);

        OUString sNewName = INetURLObject::decode(rURL.getName(),
            INetURLObject::DecodeMechanism::Unambiguous);
        sal_Int32 nExtLen = sExt.getLength();
        sNewName = sNewName.replaceAt(sNewName.getLength() - nExtLen - 1, nExtLen + 1, "");

        //find a unique name if sNewName already exists
        sFind = sNewName;
        sal_Int32 nIndex = 0;
        while (xDBContext->hasByName(sFind))
            sFind = sNewName + OUString::number(++nIndex);

        uno::Reference<uno::XInterface> xNewInstance;
        if (!bStore)
        {
            //odb-file
            uno::Any aDataSource = xDBContext->getByName(rURL.GetMainURL(INetURLObject::DecodeMechanism::NONE));
            aDataSource >>= xNewInstance;
        }
        else
        {
            xNewInstance = xDBContext->createInstance();
            uno::Reference<beans::XPropertySet> xDataProperties(xNewInstance, uno::UNO_QUERY);

            if (aURLAny.hasValue())
                xDataProperties->setPropertyValue("URL", aURLAny);
            if (aTableFilterAny.hasValue())
                xDataProperties->setPropertyValue("TableFilter", aTableFilterAny);
            if (aSuppressVersionsAny.hasValue())
                xDataProperties->setPropertyValue("SuppressVersionColumns", aSuppressVersionsAny);
            if (aInfoAny.hasValue())
                xDataProperties->setPropertyValue("Info", aInfoAny);

            if (DBConnURIType::FLAT == type && pSettings)
            {
                uno::Any aSettings = xDataProperties->getPropertyValue("Settings");
                uno::Reference < beans::XPropertySet > xDSSettings;
                aSettings >>= xDSSettings;
                ::comphelper::copyProperties(*pSettings, xDSSettings);
                xDSSettings->setPropertyValue("Extension", uno::makeAny(sExt));
            }

            uno::Reference<sdb::XDocumentDataSource> xDS(xNewInstance, uno::UNO_QUERY_THROW);
            uno::Reference<frame::XStorable> xStore(xDS->getDatabaseDocument(), uno::UNO_QUERY_THROW);
            OUString aOwnURL = getOwnURL(pDocShell);
            if (aOwnURL.isEmpty())
            {
                // Cannot embed, as embedded data source would need the URL of the parent document.
                OUString const sOutputExt = ".odb";
                OUString sHomePath(SvtPathOptions().GetWorkPath());
                utl::TempFile aTempFile(sNewName, true, &sOutputExt, pDestDir ? pDestDir : &sHomePath);
                const OUString& sTmpName = aTempFile.GetURL();
                xStore->storeAsURL(sTmpName, uno::Sequence<beans::PropertyValue>());
            }
            else
            {
                // Embed.
                OUString aStreamRelPath = "EmbeddedDatabase";
                uno::Reference<embed::XStorage> xStorage = pDocShell->GetStorage();

                // Refer to the sub-storage name in the document settings, so
                // we can load it again next time the file is imported.
                uno::Reference<lang::XMultiServiceFactory> xFactory(pDocShell->GetModel(), uno::UNO_QUERY);
                uno::Reference<beans::XPropertySet> xPropertySet(xFactory->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY);
                xPropertySet->setPropertyValue("EmbeddedDatabaseName", uno::makeAny(aStreamRelPath));

                // Store it only after setting the above property, so that only one data source gets registered.
                SwDBManager::StoreEmbeddedDataSource(xStore, xStorage, aStreamRelPath, aOwnURL);
            }
        }
        xDBContext->registerObject(sFind, xNewInstance);
    }
    catch (const uno::Exception&)
    {
        sFind.clear();
    }
    return sFind;
}

// Construct vnd.sun.star.pkg:// URL
OUString ConstructVndSunStarPkgUrl(const OUString& rMainURL, const OUString& rStreamRelPath)
{
    auto xContext(comphelper::getProcessComponentContext());
    auto xUri = css::uri::UriReferenceFactory::create(xContext)->parse(rMainURL);
    assert(xUri.is());
    xUri = css::uri::VndSunStarPkgUrlReferenceFactory::create(xContext)
        ->createVndSunStarPkgUrlReference(xUri);
    assert(xUri.is());
    return xUri->getUriReference() + "/"
        + INetURLObject::encode(
            rStreamRelPath, INetURLObject::PART_FPATH,
            INetURLObject::EncodeMechanism::All);
}
}

OUString SwDBManager::LoadAndRegisterDataSource(weld::Window* pParent, SwDocShell* pDocShell)
{
    sfx2::FileDialogHelper aDlgHelper(ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, FileDialogFlags::NONE, pParent);
    uno::Reference < ui::dialogs::XFilePicker3 > xFP = aDlgHelper.GetFilePicker();

    OUString sHomePath(SvtPathOptions().GetWorkPath());
    aDlgHelper.SetDisplayDirectory( sHomePath );

    uno::Reference<ui::dialogs::XFilterManager> xFltMgr(xFP, uno::UNO_QUERY);

    OUString sFilterAll(SwResId(STR_FILTER_ALL));
    OUString sFilterAllData(SwResId(STR_FILTER_ALL_DATA));
    OUString sFilterSXB(SwResId(STR_FILTER_SXB));
    OUString sFilterSXC(SwResId(STR_FILTER_SXC));
    OUString sFilterSXW(SwResId(STR_FILTER_SXW));
    OUString sFilterDBF(SwResId(STR_FILTER_DBF));
    OUString sFilterXLS(SwResId(STR_FILTER_XLS));
    OUString sFilterDOC(SwResId(STR_FILTER_DOC));
    OUString sFilterTXT(SwResId(STR_FILTER_TXT));
    OUString sFilterCSV(SwResId(STR_FILTER_CSV));
#ifdef _WIN32
    OUString sFilterMDB(SwResId(STR_FILTER_MDB));
    OUString sFilterACCDB(SwResId(STR_FILTER_ACCDB));
#endif
    xFltMgr->appendFilter( sFilterAll, "*" );
    xFltMgr->appendFilter( sFilterAllData, "*.ods;*.sxc;*.odt;*.sxw;*.dbf;*.xls;*.xlsx;*.doc;*.docx;*.txt;*.csv");

    xFltMgr->appendFilter( sFilterSXB, "*.odb" );
    xFltMgr->appendFilter( sFilterSXC, "*.ods;*.sxc" );
    xFltMgr->appendFilter( sFilterSXW, "*.odt;*.sxw" );
    xFltMgr->appendFilter( sFilterDBF, "*.dbf" );
    xFltMgr->appendFilter( sFilterXLS, "*.xls;*.xlsx" );
    xFltMgr->appendFilter( sFilterDOC, "*.doc;*.docx" );
    xFltMgr->appendFilter( sFilterTXT, "*.txt" );
    xFltMgr->appendFilter( sFilterCSV, "*.csv" );
#ifdef _WIN32
    xFltMgr->appendFilter(sFilterMDB, "*.mdb;*.mde");
    xFltMgr->appendFilter(sFilterACCDB, "*.accdb;*.accde");
#endif

    xFltMgr->setCurrentFilter( sFilterAll ) ;
    OUString sFind;
    if( ERRCODE_NONE == aDlgHelper.Execute() )
    {
        uno::Reference< beans::XPropertySet > aSettings;
        const INetURLObject aURL( xFP->getSelectedFiles().getConstArray()[0] );
        const DBConnURIType type = GetDBunoType( aURL );

        if( DBConnURIType::FLAT == type )
        {
            uno::Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
            uno::Reference < sdb::XTextConnectionSettings > xSettingsDlg = sdb::TextConnectionSettings::create(xContext);
            if( xSettingsDlg->execute() )
                aSettings.set( uno::Reference < beans::XPropertySet >( xSettingsDlg, uno::UNO_QUERY_THROW ) );
        }
        sFind = LoadAndRegisterDataSource_Impl( type, DBConnURIType::FLAT == type ? &aSettings : nullptr, aURL, nullptr, pDocShell );

        m_aUncommittedRegistrations.push_back(std::pair<SwDocShell*, OUString>(pDocShell, sFind));
    }
    return sFind;
}

void SwDBManager::StoreEmbeddedDataSource(const uno::Reference<frame::XStorable>& xStorable,
                                          const uno::Reference<embed::XStorage>& xStorage,
                                          const OUString& rStreamRelPath,
                                          const OUString& rOwnURL, bool bCopyTo)
{
    // Construct vnd.sun.star.pkg:// URL for later loading, and TargetStorage/StreamRelPath for storing.
    OUString const sTmpName = ConstructVndSunStarPkgUrl(rOwnURL, rStreamRelPath);

    uno::Sequence<beans::PropertyValue> aSequence = comphelper::InitPropertySequence(
    {
        {"TargetStorage", uno::makeAny(xStorage)},
        {"StreamRelPath", uno::makeAny(rStreamRelPath)},
        {"BaseURI", uno::makeAny(rOwnURL)}
    });
    if (bCopyTo)
        xStorable->storeToURL(sTmpName, aSequence);
    else
        xStorable->storeAsURL(sTmpName, aSequence);
}

OUString SwDBManager::LoadAndRegisterDataSource(const OUString &rURI, const OUString *pDestDir)
{
    return LoadAndRegisterDataSource_Impl( DBConnURIType::UNKNOWN, nullptr, INetURLObject(rURI), pDestDir, nullptr );
}

namespace
{
    // tdf#117824 switch the embedded database away from using its current storage and point it to temporary storage
    // which allows the original storage to be deleted
    void switchEmbeddedDatabaseStorage(const uno::Reference<sdb::XDatabaseContext>& rDatabaseContext, const OUString& rName)
    {
        uno::Reference<sdb::XDocumentDataSource> xDS(rDatabaseContext->getByName(rName), uno::UNO_QUERY);
        if (!xDS)
            return;
        uno::Reference<document::XStorageBasedDocument> xStorageDoc(xDS->getDatabaseDocument(), uno::UNO_QUERY);
        if (!xStorageDoc)
            return;
        xStorageDoc->switchToStorage(comphelper::OStorageHelper::GetTemporaryStorage());
    }
}

void SwDBManager::RevokeDataSource(const OUString& rName)
{
    uno::Reference<sdb::XDatabaseContext> xDatabaseContext = sdb::DatabaseContext::create(comphelper::getProcessComponentContext());
    if (xDatabaseContext->hasByName(rName))
    {
        switchEmbeddedDatabaseStorage(xDatabaseContext, rName);
        xDatabaseContext->revokeObject(rName);
    }
}

void SwDBManager::LoadAndRegisterEmbeddedDataSource(const SwDBData& rData, const SwDocShell& rDocShell)
{
    uno::Reference<sdb::XDatabaseContext> xDatabaseContext = sdb::DatabaseContext::create(comphelper::getProcessComponentContext());

    OUString sDataSource = rData.sDataSource;

    // Fallback, just in case the document would contain an embedded data source, but no DB fields.
    if (sDataSource.isEmpty())
        sDataSource = "EmbeddedDatabase";

    SwDBManager::RevokeDataSource( sDataSource );

    // Encode the stream name and the real path into a single URL.
    const INetURLObject& rURLObject = rDocShell.GetMedium()->GetURLObject();
    OUString const aURL = ConstructVndSunStarPkgUrl(
        rURLObject.GetMainURL(INetURLObject::DecodeMechanism::NONE),
        m_sEmbeddedName);

    uno::Reference<uno::XInterface> xDataSource(xDatabaseContext->getByName(aURL), uno::UNO_QUERY);
    xDatabaseContext->registerObject( sDataSource, xDataSource );

    // temp file - don't remember connection
    if (rData.sDataSource.isEmpty())
        m_aUncommittedRegistrations.push_back(std::pair<SwDocShell*, OUString>(nullptr, sDataSource));
}

void SwDBManager::ExecuteFormLetter( SwWrtShell& rSh,
                        const uno::Sequence<beans::PropertyValue>& rProperties)
{
    //prevent second call
    if(m_pImpl->pMergeDialog)
        return ;
    OUString sDataSource, sDataTableOrQuery;
    uno::Sequence<uno::Any> aSelection;

    sal_Int32 nCmdType = sdb::CommandType::TABLE;
    uno::Reference< sdbc::XConnection> xConnection;

    svx::ODataAccessDescriptor aDescriptor(rProperties);
    sDataSource = aDescriptor.getDataSource();
    OSL_VERIFY(aDescriptor[svx::DataAccessDescriptorProperty::Command]      >>= sDataTableOrQuery);
    OSL_VERIFY(aDescriptor[svx::DataAccessDescriptorProperty::CommandType]  >>= nCmdType);

    if ( aDescriptor.has(svx::DataAccessDescriptorProperty::Selection) )
        aDescriptor[svx::DataAccessDescriptorProperty::Selection] >>= aSelection;
    if ( aDescriptor.has(svx::DataAccessDescriptorProperty::Connection) )
        aDescriptor[svx::DataAccessDescriptorProperty::Connection] >>= xConnection;

    if(sDataSource.isEmpty() || sDataTableOrQuery.isEmpty())
    {
        OSL_FAIL("PropertyValues missing or unset");
        return;
    }

    //always create a connection for the dialog and dispose it after the dialog has been closed
    SwDSParam* pFound = nullptr;
    if(!xConnection.is())
    {
        xConnection = SwDBManager::RegisterConnection(sDataSource);
        pFound = FindDSConnection(sDataSource, true);
    }
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    m_pImpl->pMergeDialog = pFact->CreateMailMergeDlg( &rSh.GetView().GetViewFrame()->GetWindow(), rSh,
                                                     sDataSource,
                                                     sDataTableOrQuery,
                                                     nCmdType,
                                                     xConnection);
    if(m_pImpl->pMergeDialog->Execute() == RET_OK)
    {
        aDescriptor[svx::DataAccessDescriptorProperty::Selection] <<= m_pImpl->pMergeDialog->GetSelection();

        uno::Reference<sdbc::XResultSet> xResSet = m_pImpl->pMergeDialog->GetResultSet();
        if(xResSet.is())
            aDescriptor[svx::DataAccessDescriptorProperty::Cursor] <<= xResSet;

        // SfxObjectShellRef is ok, since there should be no control over the document lifetime here
        SfxObjectShellRef xDocShell = rSh.GetView().GetViewFrame()->GetObjectShell();

        lcl_emitEvent(SfxEventHintId::SwMailMerge, STR_SW_EVENT_MAIL_MERGE, xDocShell.get());

        // prepare mail merge descriptor
        SwMergeDescriptor aMergeDesc( m_pImpl->pMergeDialog->GetMergeType(), rSh, aDescriptor );
        aMergeDesc.sSaveToFilter = m_pImpl->pMergeDialog->GetSaveFilter();
        aMergeDesc.bCreateSingleFile = m_pImpl->pMergeDialog->IsSaveSingleDoc();
        aMergeDesc.bPrefixIsFilename = aMergeDesc.bCreateSingleFile;
        aMergeDesc.sPrefix = m_pImpl->pMergeDialog->GetTargetURL();
        if( !aMergeDesc.bCreateSingleFile && m_pImpl->pMergeDialog->IsGenerateFromDataBase() )
        {
            aMergeDesc.sDBcolumn = m_pImpl->pMergeDialog->GetColumnName();
        }

        Merge( aMergeDesc );

        lcl_emitEvent(SfxEventHintId::SwMailMergeEnd, STR_SW_EVENT_MAIL_MERGE_END, xDocShell.get());

        // reset the cursor inside
        xResSet = nullptr;
        aDescriptor[svx::DataAccessDescriptorProperty::Cursor] <<= xResSet;
    }
    if(pFound)
    {
        for (auto & pParam : m_DataSourceParams)
        {
            if (pParam.get() == pFound)
            {
                try
                {
                    uno::Reference<lang::XComponent> xComp(pParam->xConnection, uno::UNO_QUERY);
                    if(xComp.is())
                        xComp->dispose();
                }
                catch(const uno::RuntimeException&)
                {
                    //may be disposed already since multiple entries may have used the same connection
                }
                break;
            }
            //pFound doesn't need to be removed/deleted -
            //this has been done by the SwConnectionDisposedListener_Impl already
        }
    }
    m_pImpl->pMergeDialog.disposeAndClear();
}

void SwDBManager::InsertText(SwWrtShell& rSh,
                        const uno::Sequence< beans::PropertyValue>& rProperties)
{
    OUString sDataSource, sDataTableOrQuery;
    uno::Reference<sdbc::XResultSet>  xResSet;
    uno::Sequence<uno::Any> aSelection;
    sal_Int16 nCmdType = sdb::CommandType::TABLE;
    const beans::PropertyValue* pValues = rProperties.getConstArray();
    uno::Reference< sdbc::XConnection> xConnection;
    for(sal_Int32 nPos = 0; nPos < rProperties.getLength(); nPos++)
    {
        if ( pValues[nPos].Name == "DataSourceName" )
            pValues[nPos].Value >>= sDataSource;
        else if ( pValues[nPos].Name == "Command" )
            pValues[nPos].Value >>= sDataTableOrQuery;
        else if ( pValues[nPos].Name == "Cursor" )
            pValues[nPos].Value >>= xResSet;
        else if ( pValues[nPos].Name == "Selection" )
            pValues[nPos].Value >>= aSelection;
        else if ( pValues[nPos].Name == "CommandType" )
            pValues[nPos].Value >>= nCmdType;
        else if ( pValues[nPos].Name == "ActiveConnection" )
            pValues[nPos].Value >>= xConnection;
    }
    if(sDataSource.isEmpty() || sDataTableOrQuery.isEmpty() || !xResSet.is())
    {
        OSL_FAIL("PropertyValues missing or unset");
        return;
    }
    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference<sdbc::XDataSource> xSource;
    uno::Reference<container::XChild> xChild(xConnection, uno::UNO_QUERY);
    if(xChild.is())
        xSource.set(xChild->getParent(), uno::UNO_QUERY);
    if(!xSource.is())
        xSource = dbtools::getDataSource(sDataSource, xContext);
    uno::Reference< sdbcx::XColumnsSupplier > xColSupp( xResSet, uno::UNO_QUERY );
    SwDBData aDBData;
    aDBData.sDataSource = sDataSource;
    aDBData.sCommand = sDataTableOrQuery;
    aDBData.nCommandType = nCmdType;

    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSwInsertDBColAutoPilot> pDlg(pFact->CreateSwInsertDBColAutoPilot( rSh.GetView(),
                                                                                xSource,
                                                                                xColSupp,
                                                                                aDBData ));
    if( RET_OK == pDlg->Execute() )
    {
        OUString sDummy;
        if(!xConnection.is())
            xConnection = xSource->getConnection(sDummy, sDummy);
        try
        {
            pDlg->DataToDoc( aSelection , xSource, xConnection, xResSet);
        }
        catch (const uno::Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("sw.mailmerge", "exception caught: " << exceptionToString(ex));
        }
    }
}

uno::Reference<sdbc::XDataSource> SwDBManager::getDataSourceAsParent(const uno::Reference< sdbc::XConnection>& _xConnection,const OUString& _sDataSourceName)
{
    uno::Reference<sdbc::XDataSource> xSource;
    try
    {
        uno::Reference<container::XChild> xChild(_xConnection, uno::UNO_QUERY);
        if ( xChild.is() )
            xSource.set(xChild->getParent(), uno::UNO_QUERY);
        if ( !xSource.is() )
            xSource = dbtools::getDataSource(_sDataSourceName, ::comphelper::getProcessComponentContext());
    }
    catch (const uno::Exception&)
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("sw.mailmerge", "exception caught in getDataSourceAsParent(): " << exceptionToString(ex));
    }
    return xSource;
}

uno::Reference<sdbc::XResultSet> SwDBManager::createCursor(const OUString& _sDataSourceName,
                                       const OUString& _sCommand,
                                       sal_Int32 _nCommandType,
                                       const uno::Reference<sdbc::XConnection>& _xConnection,
                                       const SwView* pView)
{
    uno::Reference<sdbc::XResultSet> xResultSet;
    try
    {
        uno::Reference< lang::XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
        if( xMgr.is() )
        {
            uno::Reference<uno::XInterface> xInstance = xMgr->createInstance("com.sun.star.sdb.RowSet");
            uno::Reference<beans::XPropertySet> xRowSetPropSet(xInstance, uno::UNO_QUERY);
            if(xRowSetPropSet.is())
            {
                xRowSetPropSet->setPropertyValue("DataSourceName", uno::makeAny(_sDataSourceName));
                xRowSetPropSet->setPropertyValue("ActiveConnection", uno::makeAny(_xConnection));
                xRowSetPropSet->setPropertyValue("Command", uno::makeAny(_sCommand));
                xRowSetPropSet->setPropertyValue("CommandType", uno::makeAny(_nCommandType));

                uno::Reference< sdb::XCompletedExecution > xRowSet(xInstance, uno::UNO_QUERY);

                if ( xRowSet.is() )
                {
                    weld::Window* pWindow = pView ? pView->GetFrameWeld() : nullptr;
                    uno::Reference< task::XInteractionHandler > xHandler( task::InteractionHandler::createWithParent(comphelper::getComponentContext(xMgr), pWindow ? pWindow->GetXWindow() : nullptr), uno::UNO_QUERY_THROW );
                    xRowSet->executeWithCompletion(xHandler);
                }
                xResultSet.set(xRowSet, uno::UNO_QUERY);
            }
        }
    }
    catch (const uno::Exception&)
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("sw.mailmerge", "Caught exception while creating a new RowSet: " << exceptionToString(ex));
    }
    return xResultSet;
}

void SwDBManager::setEmbeddedName(const OUString& rEmbeddedName, SwDocShell& rDocShell)
{
    bool bLoad = m_sEmbeddedName != rEmbeddedName && !rEmbeddedName.isEmpty();
    bool bRegisterListener = m_sEmbeddedName.isEmpty() && !rEmbeddedName.isEmpty();

    m_sEmbeddedName = rEmbeddedName;

    if (bLoad)
    {
        uno::Reference<embed::XStorage> xStorage = rDocShell.GetStorage();
        // It's OK that we don't have the named sub-storage yet, in case
        // we're in the process of creating it.
        if (xStorage->hasByName(rEmbeddedName))
            LoadAndRegisterEmbeddedDataSource(rDocShell.GetDoc()->GetDBData(), rDocShell);
    }

    if (bRegisterListener)
        // Register a remove listener, so we know when the embedded data source is removed.
        m_pImpl->m_xDataSourceRemovedListener = new SwDataSourceRemovedListener(*this);
}

const OUString& SwDBManager::getEmbeddedName() const
{
    return m_sEmbeddedName;
}

SwDoc* SwDBManager::getDoc() const
{
    return m_pDoc;
}

void SwDBManager::releaseRevokeListener()
{
    if (m_pImpl->m_xDataSourceRemovedListener.is())
    {
        m_pImpl->m_xDataSourceRemovedListener->Dispose();
        m_pImpl->m_xDataSourceRemovedListener.clear();
    }
}

SwDBManager::ConnectionDisposedListener_Impl::ConnectionDisposedListener_Impl(SwDBManager& rManager)
    : m_pDBManager(&rManager)
{
}

void SwDBManager::ConnectionDisposedListener_Impl::disposing( const lang::EventObject& rSource )
{
    ::SolarMutexGuard aGuard;

    if (!m_pDBManager) return; // we're disposed too!

    uno::Reference<sdbc::XConnection> xSource(rSource.Source, uno::UNO_QUERY);
    for (size_t nPos = m_pDBManager->m_DataSourceParams.size(); nPos; nPos--)
    {
        SwDSParam* pParam = m_pDBManager->m_DataSourceParams[nPos - 1].get();
        if(pParam->xConnection.is() &&
                (xSource == pParam->xConnection))
        {
            m_pDBManager->m_DataSourceParams.erase(
                    m_pDBManager->m_DataSourceParams.begin() + nPos - 1);
        }
    }
}

std::shared_ptr<SwMailMergeConfigItem> SwDBManager::PerformMailMerge(SwView const * pView)
{
    std::shared_ptr<SwMailMergeConfigItem> xConfigItem = pView->GetMailMergeConfigItem();
    if (!xConfigItem)
        return xConfigItem;

    svx::ODataAccessDescriptor aDescriptor;
    aDescriptor.setDataSource(xConfigItem->GetCurrentDBData().sDataSource);
    aDescriptor[ svx::DataAccessDescriptorProperty::Connection ]  <<= xConfigItem->GetConnection().getTyped();
    aDescriptor[ svx::DataAccessDescriptorProperty::Cursor ]      <<= xConfigItem->GetResultSet();
    aDescriptor[ svx::DataAccessDescriptorProperty::Command ]     <<= xConfigItem->GetCurrentDBData().sCommand;
    aDescriptor[ svx::DataAccessDescriptorProperty::CommandType ] <<= xConfigItem->GetCurrentDBData().nCommandType;
    aDescriptor[ svx::DataAccessDescriptorProperty::Selection ]   <<= xConfigItem->GetSelection();

    SwWrtShell& rSh = pView->GetWrtShell();
    xConfigItem->SetTargetView(nullptr);

    SwMergeDescriptor aMergeDesc(DBMGR_MERGE_SHELL, rSh, aDescriptor);
    aMergeDesc.pMailMergeConfigItem = xConfigItem.get();
    aMergeDesc.bCreateSingleFile = true;
    rSh.GetDBManager()->Merge(aMergeDesc);

    return xConfigItem;
}

void SwDBManager::RevokeLastRegistrations()
{
    if (!m_aUncommittedRegistrations.empty())
    {
        SwView* pView = ( m_pDoc && m_pDoc->GetDocShell() ) ? m_pDoc->GetDocShell()->GetView() : nullptr;
        if (pView)
        {
            const std::shared_ptr<SwMailMergeConfigItem>& xConfigItem = pView->GetMailMergeConfigItem();
            if (xConfigItem)
            {
                xConfigItem->DisposeResultSet();
                xConfigItem->DocumentReloaded();
            }
        }

        for (auto it = m_aUncommittedRegistrations.begin(); it != m_aUncommittedRegistrations.end();)
        {
            if ((m_pDoc && it->first == m_pDoc->GetDocShell()) || it->first == nullptr)
            {
                RevokeDataSource(it->second);
                it = m_aUncommittedRegistrations.erase(it);
            }
            else
                ++it;
        }
    }
}

void SwDBManager::CommitLastRegistrations()
{
    for (auto aIt = m_aUncommittedRegistrations.begin(); aIt != m_aUncommittedRegistrations.end();)
    {
        if (aIt->first == m_pDoc->GetDocShell() || aIt->first == nullptr)
        {
            m_aNotUsedConnections.push_back(aIt->second);
            aIt = m_aUncommittedRegistrations.erase(aIt);
        }
        else
            aIt++;
    }
}

void SwDBManager::SetAsUsed(const OUString& rName)
{
    auto aFound = std::find(m_aNotUsedConnections.begin(), m_aNotUsedConnections.end(), rName);
    if (aFound != m_aNotUsedConnections.end())
        m_aNotUsedConnections.erase(aFound);
}

void SwDBManager::RevokeNotUsedConnections()
{
    for (auto aIt = m_aNotUsedConnections.begin(); aIt != m_aNotUsedConnections.end();)
    {
        RevokeDataSource(*aIt);
        aIt = m_aNotUsedConnections.erase(aIt);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
