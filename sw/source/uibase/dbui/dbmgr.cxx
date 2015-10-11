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

#include <cstdarg>

#include <unotxdoc.hxx>
#include <com/sun/star/text/NotePrintMode.hpp>
#include <sfx2/app.hxx>
#include <sfx2/printer.hxx>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
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
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
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
#include <svl/mailenum.hxx>
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
#include <swwait.hxx>
#include <swunohelper.hxx>
#include <dbui.hrc>
#include <globals.hrc>
#include <statstr.hrc>
#include <mmconfigitem.hxx>
#include <sfx2/request.hxx>
#include <hintids.hxx>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/mail/MailAttachment.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
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

#include <unomailmerge.hxx>
#include <sfx2/event.hxx>
#include <vcl/msgbox.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <osl/mutex.hxx>
#include <rtl/textenc.h>
#include <cppuhelper/implbase.hxx>
#include <ndindex.hxx>
#include <pam.hxx>
#include <swcrsr.hxx>
#include <swevent.hxx>
#include <osl/file.hxx>
#include <sal/log.hxx>
#include <swabstdlg.hxx>
#include <fmthdft.hxx>
#include <envelp.hrc>
#include <vector>
#include <unomid.h>
#include <section.hxx>
#include <rootfrm.hxx>
#include <fmtpdsc.hxx>
#include <ndtxt.hxx>
#include <calc.hxx>
#include <dbfld.hxx>

#include <memory>
#include <config_cups.h>
#if ENABLE_CUPS && !defined(MACOSX)
#include <vcl/printerinfomanager.hxx>
#endif
#include <comphelper/propertysequence.hxx>
#include <officecfg/Office/Common.hxx>

using namespace ::com::sun::star;

#define DB_SEP_SPACE    0
#define DB_SEP_TAB      1
#define DB_SEP_RETURN   2
#define DB_SEP_NEWLINE  3

const sal_Char cCursor[] = "Cursor";
const sal_Char cCommand[] = "Command";
const sal_Char cCommandType[] = "CommandType";
const sal_Char cDataSourceName[] = "DataSourceName";
const sal_Char cSelection[] = "Selection";
const sal_Char cActiveConnection[] = "ActiveConnection";

namespace
{

bool lcl_getCountFromResultSet( sal_Int32& rCount, const uno::Reference<sdbc::XResultSet>& xResultSet )
{
    uno::Reference<beans::XPropertySet> xPrSet(xResultSet, uno::UNO_QUERY);
    if(xPrSet.is())
    {
        try
        {
            bool bFinal = false;
            uno::Any aFinal = xPrSet->getPropertyValue("IsRowCountFinal");
            aFinal >>= bFinal;
            if(!bFinal)
            {
                xResultSet->last();
                xResultSet->first();
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
}

class SwConnectionDisposedListener_Impl : public cppu::WeakImplHelper
< lang::XEventListener >
{
private:
    SwDBManager * m_pDBManager;

    virtual void SAL_CALL disposing( const lang::EventObject& Source ) throw (uno::RuntimeException, std::exception) override;

public:
    explicit SwConnectionDisposedListener_Impl(SwDBManager& rMgr);
    virtual ~SwConnectionDisposedListener_Impl();

    void Dispose() { m_pDBManager = 0; }

};

/// Listens to removed data sources, and if it's one that's embedded into this document, triggers embedding removal.
class SwDataSourceRemovedListener : public cppu::WeakImplHelper<sdb::XDatabaseRegistrationsListener>
{
    uno::Reference<sdb::XDatabaseContext> m_xDatabaseContext;
    SwDBManager* m_pDBManager;

public:
    explicit SwDataSourceRemovedListener(SwDBManager& rDBManager);
    virtual ~SwDataSourceRemovedListener();
    virtual void SAL_CALL registeredDatabaseLocation(const sdb::DatabaseRegistrationEvent& rEvent) throw (uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL revokedDatabaseLocation(const sdb::DatabaseRegistrationEvent& rEvent) throw (uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL changedDatabaseLocation(const sdb::DatabaseRegistrationEvent& rEvent) throw (uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL disposing(const lang::EventObject& rObject) throw (uno::RuntimeException, std::exception) override;
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

void SAL_CALL SwDataSourceRemovedListener::registeredDatabaseLocation(const sdb::DatabaseRegistrationEvent& /*rEvent*/) throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL SwDataSourceRemovedListener::revokedDatabaseLocation(const sdb::DatabaseRegistrationEvent& rEvent) throw (uno::RuntimeException, std::exception)
{
    if (!m_pDBManager || m_pDBManager->getEmbeddedName().isEmpty())
        return;

    SwDoc* pDoc = m_pDBManager->getDoc();
    if (!pDoc)
        return;

    SwDocShell* pDocShell = pDoc->GetDocShell();
    if (!pDocShell)
        return;

    OUString aOwnURL = pDocShell->GetMedium()->GetURLObject().GetMainURL(INetURLObject::DECODE_WITH_CHARSET);
    OUString sTmpName = "vnd.sun.star.pkg://";
    sTmpName += INetURLObject::encode(aOwnURL, INetURLObject::PART_AUTHORITY, INetURLObject::ENCODE_ALL);
    sTmpName += "/" + m_pDBManager->getEmbeddedName();

    if (sTmpName != rEvent.OldLocation)
        return;

    // The revoked database location is inside this document, then remove the
    // embedding, as otherwise it would be back on the next reload of the
    // document.
    pDocShell->GetStorage()->removeElement(m_pDBManager->getEmbeddedName());
    m_pDBManager->setEmbeddedName(OUString(), *pDocShell);
}

void SAL_CALL SwDataSourceRemovedListener::changedDatabaseLocation(const sdb::DatabaseRegistrationEvent& rEvent) throw (uno::RuntimeException, std::exception)
{
    if (rEvent.OldLocation != rEvent.NewLocation)
        revokedDatabaseLocation(rEvent);
}

void SwDataSourceRemovedListener::disposing(const lang::EventObject& /*rObject*/) throw (uno::RuntimeException, std::exception)
{
    m_xDatabaseContext.clear();
}

void SwDataSourceRemovedListener::Dispose()
{
    m_pDBManager = 0;
}

struct SwDBManager_Impl
{
    SwDSParam*          pMergeData;
    AbstractMailMergeDlg*     pMergeDialog;
    ::rtl::Reference<SwConnectionDisposedListener_Impl> m_xDisposeListener;
    rtl::Reference<SwDataSourceRemovedListener> m_xDataSourceRemovedListener;

    explicit SwDBManager_Impl(SwDBManager& rDBManager)
       :pMergeData(0)
       ,pMergeDialog(0)
       , m_xDisposeListener(new SwConnectionDisposedListener_Impl(rDBManager))
        {}

    ~SwDBManager_Impl()
    {
        m_xDisposeListener->Dispose();
        if (m_xDataSourceRemovedListener.is())
            m_xDataSourceRemovedListener->Dispose();
    }
};

static void lcl_InitNumberFormatter(SwDSParam& rParam, uno::Reference<sdbc::XDataSource> xSource)
{
    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    rParam.xFormatter = uno::Reference<util::XNumberFormatter>(util::NumberFormatter::create(xContext), uno::UNO_QUERY);
    if(!xSource.is())
        xSource = SwDBManager::getDataSourceAsParent(rParam.xConnection, rParam.sDataSource);

    uno::Reference<beans::XPropertySet> xSourceProps(xSource, uno::UNO_QUERY);
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
        if(pParam->bScrollable)
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
                             long nLanguage, OUString &rResult, double* pNumber)
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

    aFormatData.aLocale = LanguageTag( (LanguageType)nLanguage ).getLocale();

    rResult = SwDBManager::GetDBField( rColumnProps, aFormatData, pNumber);
}

static bool lcl_GetColumnCnt(SwDSParam* pParam, const OUString& rColumnName,
                             long nLanguage, OUString& rResult, double* pNumber)
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
bool SwDBManager::MergeNew( const SwMergeDescriptor& rMergeDesc, vcl::Window* pParent )
{
    OSL_ENSURE(!bInMerge && !pImpl->pMergeData, "merge already activated!");

    SwDBData aData;
    aData.nCommandType = sdb::CommandType::TABLE;
    uno::Reference<sdbc::XResultSet>  xResSet;
    uno::Sequence<uno::Any> aSelection;
    uno::Reference< sdbc::XConnection> xConnection;

    aData.sDataSource = rMergeDesc.rDescriptor.getDataSource();
    rMergeDesc.rDescriptor[svx::daCommand]      >>= aData.sCommand;
    rMergeDesc.rDescriptor[svx::daCommandType]  >>= aData.nCommandType;

    if ( rMergeDesc.rDescriptor.has(svx::daCursor) )
        rMergeDesc.rDescriptor[svx::daCursor] >>= xResSet;
    if ( rMergeDesc.rDescriptor.has(svx::daSelection) )
        rMergeDesc.rDescriptor[svx::daSelection] >>= aSelection;
    if ( rMergeDesc.rDescriptor.has(svx::daConnection) )
        rMergeDesc.rDescriptor[svx::daConnection] >>= xConnection;

    if(aData.sDataSource.isEmpty() || aData.sCommand.isEmpty() || !xResSet.is())
    {
        return false;
    }

    pImpl->pMergeData = new SwDSParam(aData, xResSet, aSelection);
    SwDSParam*  pTemp = FindDSData(aData, false);
    if(pTemp)
        *pTemp = *pImpl->pMergeData;
    else
    {
        // calls from the calculator may have added a connection with an invalid commandtype
        //"real" data base connections added here have to re-use the already available
        //DSData and set the correct CommandType
        SwDBData aTempData(aData);
        aData.nCommandType = -1;
        pTemp = FindDSData(aData, false);
        if(pTemp)
            *pTemp = *pImpl->pMergeData;
        else
        {
            std::unique_ptr<SwDSParam> pInsert(new SwDSParam(*pImpl->pMergeData));
            m_DataSourceParams.push_back(std::move(pInsert));
            try
            {
                uno::Reference<lang::XComponent> xComponent(pInsert->xConnection, uno::UNO_QUERY);
                if(xComponent.is())
                    xComponent->addEventListener(pImpl->m_xDisposeListener.get());
            }
            catch(const uno::Exception&)
            {
            }
        }
    }
    if(!pImpl->pMergeData->xConnection.is())
        pImpl->pMergeData->xConnection = xConnection;
    // add an XEventListener

    try{
        //set to start position
        if(pImpl->pMergeData->aSelection.getLength())
        {
            sal_Int32 nPos = 0;
            pImpl->pMergeData->aSelection.getConstArray()[ pImpl->pMergeData->nSelectionIndex++ ] >>= nPos;
            pImpl->pMergeData->bEndOfDB = !pImpl->pMergeData->xResultSet->absolute( nPos );
            pImpl->pMergeData->CheckEndOfDB();
            if(pImpl->pMergeData->nSelectionIndex >= pImpl->pMergeData->aSelection.getLength())
                pImpl->pMergeData->bEndOfDB = true;
        }
        else
        {
            pImpl->pMergeData->bEndOfDB = !pImpl->pMergeData->xResultSet->first();
            pImpl->pMergeData->CheckEndOfDB();
        }
    }
    catch (const uno::Exception& e)
    {
        pImpl->pMergeData->bEndOfDB = true;
        pImpl->pMergeData->CheckEndOfDB();
        SAL_WARN("sw.mailmerge", "exception in MergeNew(): " << e.Message);
    }

    uno::Reference<sdbc::XDataSource> xSource = SwDBManager::getDataSourceAsParent(xConnection,aData.sDataSource);

    lcl_InitNumberFormatter(*pImpl->pMergeData, xSource);

    rMergeDesc.rSh.ChgDBData(aData);
    bInMerge = true;

    if (IsInitDBFields())
    {
        // with database fields without DB-Name, use DB-Name from Doc
        std::vector<OUString> aDBNames;
        aDBNames.push_back(OUString());
        SwDBData aInsertData = rMergeDesc.rSh.GetDBData();
        OUString sDBName = aInsertData.sDataSource;
        sDBName += OUString(DB_DELIM);
        sDBName += aInsertData.sCommand;
        sDBName += OUString(DB_DELIM);
        sDBName += OUString::number(aInsertData.nCommandType);
        rMergeDesc.rSh.ChangeDBFields( aDBNames, sDBName);
        SetInitDBFields(false);
    }

    bool bRet = true;
    switch(rMergeDesc.nMergeType)
    {
        case DBMGR_MERGE:
            bRet = Merge(&rMergeDesc.rSh);
            break;

        case DBMGR_MERGE_PRINTER:
        case DBMGR_MERGE_EMAIL:
        case DBMGR_MERGE_FILE:
        case DBMGR_MERGE_SHELL:
            // save files and send them as e-Mail if required
            bRet = MergeMailFiles(&rMergeDesc.rSh,
                    rMergeDesc, pParent);
            break;

        default:
            // insert selected entries
            // (was: InsertRecord)
            ImportFromConnection(&rMergeDesc.rSh);
            break;
    }

    DELETEZ( pImpl->pMergeData );

    bInMerge = false;

    return bRet;
}

// import data
bool SwDBManager::Merge(SwWrtShell* pSh)
{
    pSh->StartAllAction();

    pSh->SwViewShell::UpdateFields(true);
    pSh->SetModified();

    pSh->EndAllAction();

    return true;
}

void SwDBManager::ImportFromConnection(  SwWrtShell* pSh )
{
    if(pImpl->pMergeData && !pImpl->pMergeData->bEndOfDB)
    {
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
}

static OUString  lcl_FindColumn(const OUString& sFormatStr,sal_uInt16  &nUsedPos, sal_uInt8 &nSeparator)
{
    OUString sReturn;
    sal_uInt16 nLen = sFormatStr.getLength();
    nSeparator = 0xff;
    while(nUsedPos < nLen && nSeparator == 0xff)
    {
        sal_Unicode cAkt = sFormatStr[nUsedPos];
        switch(cAkt)
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
                sReturn += OUString(cAkt);
        }
        nUsedPos++;

    }
    return sReturn;
}

void SwDBManager::ImportDBEntry(SwWrtShell* pSh)
{
    if(pImpl->pMergeData && !pImpl->pMergeData->bEndOfDB)
    {
        uno::Reference< sdbcx::XColumnsSupplier > xColsSupp( pImpl->pMergeData->xResultSet, uno::UNO_QUERY );
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
                            sInsert += OUStringLiteral1<cSpace>();
                    else if( DB_SEP_TAB == nSeparator)
                            sInsert += OUStringLiteral1<cTab>();
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
            OUString sStr;
            uno::Sequence<OUString> aColNames = xCols->getElementNames();
            const OUString* pColNames = aColNames.getConstArray();
            long nLength = aColNames.getLength();
            for(long i = 0; i < nLength; i++)
            {
                uno::Any aCol = xCols->getByName(pColNames[i]);
                uno::Reference< beans::XPropertySet > xColumnProp;
                aCol >>= xColumnProp;
                SwDBFormatData aDBFormat;
                sStr += GetDBField( xColumnProp, aDBFormat);
                if (i < nLength - 1)
                    sStr += "\t";
            }
            pSh->SwEditShell::Insert2(sStr);
            pSh->SwFEShell::SplitNode();    // line feed
        }
    }
}

// fill Listbox with tablelist
bool SwDBManager::GetTableNames(ListBox* pListBox, const OUString& rDBName)
{
    bool bRet = false;
    OUString sOldTableName(pListBox->GetSelectEntry());
    pListBox->Clear();
    SwDSParam* pParam = FindDSConnection(rDBName, false);
    uno::Reference< sdbc::XConnection> xConnection;
    if(pParam && pParam->xConnection.is())
        xConnection = pParam->xConnection;
    else
    {
        OUString sDBName(rDBName);
        if ( !sDBName.isEmpty() )
            xConnection = RegisterConnection( sDBName );
    }
    if(xConnection.is())
    {
        uno::Reference<sdbcx::XTablesSupplier> xTSupplier = uno::Reference<sdbcx::XTablesSupplier>(xConnection, uno::UNO_QUERY);
        if(xTSupplier.is())
        {
            uno::Reference<container::XNameAccess> xTables = xTSupplier->getTables();
            uno::Sequence<OUString> aTables = xTables->getElementNames();
            const OUString* pTables = aTables.getConstArray();
            for(long i = 0; i < aTables.getLength(); i++)
            {
                const sal_Int32 nEntry = pListBox->InsertEntry(pTables[i]);
                pListBox->SetEntryData(nEntry, nullptr);
            }
        }
        uno::Reference<sdb::XQueriesSupplier> xQSupplier = uno::Reference<sdb::XQueriesSupplier>(xConnection, uno::UNO_QUERY);
        if(xQSupplier.is())
        {
            uno::Reference<container::XNameAccess> xQueries = xQSupplier->getQueries();
            uno::Sequence<OUString> aQueries = xQueries->getElementNames();
            const OUString* pQueries = aQueries.getConstArray();
            for(long i = 0; i < aQueries.getLength(); i++)
            {
                const sal_Int32 nEntry = pListBox->InsertEntry(pQueries[i]);
                pListBox->SetEntryData(nEntry, reinterpret_cast<void*>(1));
            }
        }
        if (!sOldTableName.isEmpty())
            pListBox->SelectEntry(sOldTableName);
        bRet = true;
    }
    return bRet;
}

// fill Listbox with column names of a database
void SwDBManager::GetColumnNames(ListBox* pListBox,
                             const OUString& rDBName, const OUString& rTableName, bool bAppend)
{
    if (!bAppend)
        pListBox->Clear();
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
        OUString sDBName(rDBName);
        xConnection = RegisterConnection( sDBName );
    }
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

void SwDBManager::GetColumnNames(ListBox* pListBox,
        uno::Reference< sdbc::XConnection> xConnection,
        const OUString& rTableName, bool bAppend)
{
    if (!bAppend)
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

SwDBManager::SwDBManager(SwDoc* pDoc)
    : bCancel(false)
    , bInitDBFields(false)
    , bInMerge(false)
    , bMergeSilent(false)
    , bMergeLock(false)
    , pImpl(new SwDBManager_Impl(*this))
    , pMergeEvtSrc(NULL)
    , m_pDoc(pDoc)
{
}

SwDBManager::~SwDBManager()
{
    for (auto & pParam : m_DataSourceParams)
    {
        if(pParam->xConnection.is())
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
        }
    }
    delete pImpl;
}

// save bulk letters as single documents
static OUString lcl_FindUniqueName(SwWrtShell* pTargetShell, const OUString& rStartingPageDesc, sal_uLong nDocNo )
{
    do
    {
        OUString sTest = rStartingPageDesc;
        sTest += OUString::number( nDocNo );
        if( !pTargetShell->FindPageDescByName( sTest ) )
            return sTest;
        ++nDocNo;
    }while(true);
}

static void lcl_CopyFollowPageDesc(
                            SwWrtShell& rTargetShell,
                            const SwPageDesc& rSourcePageDesc,
                            const SwPageDesc& rTargetPageDesc,
                            const sal_uLong nDocNo )
{
    //now copy the follow page desc, too
    const SwPageDesc* pFollowPageDesc = rSourcePageDesc.GetFollow();
    OUString sFollowPageDesc = pFollowPageDesc->GetName();
    if( sFollowPageDesc != rSourcePageDesc.GetName() )
    {
        SwDoc* pTargetDoc = rTargetShell.GetDoc();
        OUString sNewFollowPageDesc = lcl_FindUniqueName(&rTargetShell, sFollowPageDesc, nDocNo );
        SwPageDesc* pTargetFollowPageDesc = pTargetDoc->MakePageDesc(sNewFollowPageDesc);

        pTargetDoc->CopyPageDesc(*pFollowPageDesc, *pTargetFollowPageDesc, false);
        SwPageDesc aDesc(rTargetPageDesc);
        aDesc.SetFollow(pTargetFollowPageDesc);
        pTargetDoc->ChgPageDesc(rTargetPageDesc.GetName(), aDesc);
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

static void lcl_SaveDoc( SfxObjectShell *xTargetDocShell,
                         const char *name, int no = 0 )
{
    OUString sExt( ".odt" );
    OUString basename = OUString::createFromAscii( name );
    if (no > 0)
        basename += OUString::number(no) + "-";
    // aTempFile is not deleted, but that seems to be intentional
    utl::TempFile aTempFile(basename, true, &sExt);
    INetURLObject aTempFileURL( aTempFile.GetURL() );
    SfxMedium* pDstMed = new SfxMedium(
        aTempFileURL.GetMainURL( INetURLObject::NO_DECODE ),
        STREAM_STD_READWRITE );
    if( !xTargetDocShell->DoSaveAs( *pDstMed ) )
        SAL_WARN( "sw.mailmerge", "Error saving: " << aTempFile.GetURL() );
    else
        SAL_INFO( "sw.mailmerge", "Saved doc as: " << aTempFile.GetURL() );
    delete pDstMed;
}

bool SwDBManager::MergeMailFiles(SwWrtShell* pSourceShell,
                                 const SwMergeDescriptor& rMergeDescriptor,
                                 vcl::Window* pParent)
{
    //check if the doc is synchronized and contains at least one linked section
    bool bSynchronizedDoc = pSourceShell->IsLabelDoc() && pSourceShell->GetSectionFormatCount() > 1;
    bool bNoError = true;
    const bool bEMail = rMergeDescriptor.nMergeType == DBMGR_MERGE_EMAIL;
    const bool bMergeShell = rMergeDescriptor.nMergeType == DBMGR_MERGE_SHELL;
    bool bCreateSingleFile = rMergeDescriptor.bCreateSingleFile;

    if( rMergeDescriptor.nMergeType == DBMGR_MERGE_PRINTER )
    {
        // It is possible to do MM printing in both modes for the same result, but the singlefile mode
        // is slower because of all the temporary document copies and merging them together
        // into the single file, while the other mode simply updates fields and prints for every record.
        // However, this would cause one print job for every record, and e.g. CUPS refuses new jobs
        // if it has many jobs enqueued (500 by default), and with the current printing framework
        // (which uses a pull model) it's rather complicated to create a single print job
        // in steps.
        // To handle this, CUPS backend has been changed to cache all the documents to print
        // and send them to CUPS only as one job at the very end. Therefore, with CUPS, it's ok
        // to use the faster mode. As I have no idea about other platforms, keep them using
        // the slower singlefile mode (or feel free to check them, or rewrite the printing code).
#if ENABLE_CUPS && !defined(MACOSX)
        bCreateSingleFile = !psp::PrinterInfoManager::get().supportsBatchPrint();
#else
        bCreateSingleFile = true;
#endif
    }

    ::rtl::Reference< MailDispatcher >          xMailDispatcher;
    OUString sBodyMimeType;
    rtl_TextEncoding eEncoding = ::osl_getThreadTextEncoding();

    static const char *sMaxDumpDocs = 0;
    static sal_Int32 nMaxDumpDocs = 0;
    if (!sMaxDumpDocs)
    {
        sMaxDumpDocs = getenv("SW_DEBUG_MAILMERGE_DOCS");
        if (!sMaxDumpDocs)
            sMaxDumpDocs = "";
        else
            nMaxDumpDocs = rtl_ustr_toInt32(reinterpret_cast<const sal_Unicode*>( sMaxDumpDocs ), 10);
    }

    if(bEMail)
    {
        xMailDispatcher.set( new MailDispatcher(rMergeDescriptor.xSmtpServer));
        if(!rMergeDescriptor.bSendAsAttachment && rMergeDescriptor.bSendAsHTML)
        {
            sBodyMimeType = "text/html; charset=";
            sBodyMimeType += OUString::createFromAscii(
                                rtl_getBestMimeCharsetFromTextEncoding( eEncoding ));
            SvxHtmlOptions& rHtmlOptions = SvxHtmlOptions::Get();
            eEncoding = rHtmlOptions.GetTextEncoding();
        }
        else
            sBodyMimeType = "text/plain; charset=UTF-8; format=flowed";
    }

    uno::Reference< beans::XPropertySet > xColumnProp;
    {
        bool bColumnName = !sEMailAddrField.isEmpty();

        if (bColumnName)
        {
            uno::Reference< sdbcx::XColumnsSupplier > xColsSupp( pImpl->pMergeData->xResultSet, uno::UNO_QUERY );
            uno::Reference<container::XNameAccess> xCols = xColsSupp->getColumns();
            if(!xCols->hasByName(sEMailAddrField))
                return false;
            uno::Any aCol = xCols->getByName(sEMailAddrField);
            aCol >>= xColumnProp;
        }

        // Try saving the source document
        SfxDispatcher* pSfxDispatcher = pSourceShell->GetView().GetViewFrame()->GetDispatcher();
        SwDocShell* pSourceDocSh = pSourceShell->GetView().GetDocShell();

        uno::Reference<document::XDocumentProperties> xSourceDocProps;
        {
            uno::Reference<document::XDocumentPropertiesSupplier>
                xDPS(pSourceDocSh->GetModel(), uno::UNO_QUERY);
            xSourceDocProps.set(xDPS->getDocumentProperties());
            OSL_ENSURE(xSourceDocProps.is(), "DocumentProperties is null");
        }

        if( !bMergeShell && pSourceDocSh->IsModified() )
            pSfxDispatcher->Execute( pSourceDocSh->HasName() ? SID_SAVEDOC : SID_SAVEASDOC, SfxCallMode::SYNCHRON|SfxCallMode::RECORD);
        if( bMergeShell || !pSourceDocSh->IsModified() )
        {
            const SfxFilter* pStoreToFilter = SwIoSystem::GetFileFilter(
                pSourceDocSh->GetMedium()->GetURLObject().GetMainURL(INetURLObject::NO_DECODE));
            SfxFilterContainer* pFilterContainer = SwDocShell::Factory().GetFilterContainer();
            const OUString* pStoreToFilterOptions = 0;

            // if a save_to filter is set then use it - otherwise use the default
            if( bEMail && !rMergeDescriptor.bSendAsAttachment )
            {
                OUString sExtension = rMergeDescriptor.bSendAsHTML ? OUString("html") : OUString("txt");
                pStoreToFilter = pFilterContainer->GetFilter4Extension(sExtension, SfxFilterFlags::EXPORT);
            }
            else if( !rMergeDescriptor.sSaveToFilter.isEmpty())
            {
                const SfxFilter* pFilter =
                        pFilterContainer->GetFilter4FilterName( rMergeDescriptor.sSaveToFilter );
                if(pFilter)
                {
                    pStoreToFilter = pFilter;
                    if(!rMergeDescriptor.sSaveToFilterOptions.isEmpty())
                        pStoreToFilterOptions = &rMergeDescriptor.sSaveToFilterOptions;
                }
            }
            bCancel = false;

            // in case of creating a single resulting file this has to be created here
            SwWrtShell* pTargetShell = 0;
            SwDoc* pTargetDoc = 0;

            SfxObjectShellRef xTargetDocShell;

            SwView* pTargetView = 0;
            std::unique_ptr< utl::TempFile > aTempFile;
            bool createTempFile = ( rMergeDescriptor.nMergeType == DBMGR_MERGE_EMAIL || rMergeDescriptor.nMergeType == DBMGR_MERGE_FILE );
            OUString sModifiedStartingPageDesc;
            OUString sStartingPageDesc;
            sal_uInt16 nStartingPageNo = 0;
            bool bPageStylesWithHeaderFooter = false;

            vcl::Window *pSourceWindow = 0;
            VclPtr<CancelableDialog> pProgressDlg;

            if (!IsMergeSilent()) {
                pSourceWindow = &pSourceShell->GetView().GetEditWin();
                if( ! pParent )
                    pParent = pSourceWindow;
                if( bMergeShell )
                    pProgressDlg = VclPtr<CreateMonitor>::Create( pParent, pParent != pSourceWindow );
                else {
                    pProgressDlg = VclPtr<PrintMonitor>::Create( pParent, pParent != pSourceWindow, PrintMonitor::MONITOR_TYPE_PRINT );
                    static_cast<PrintMonitor*>( pProgressDlg.get() )->SetText(pSourceShell->GetView().GetDocShell()->GetTitle(22));
                }
                pProgressDlg->SetCancelHdl( LINK(this, SwDBManager, PrtCancelHdl) );
                pProgressDlg->Show();

                for( sal_uInt16 i = 0; i < 25; i++)
                    Application::Reschedule();
            }

            if(bCreateSingleFile)
            {
                // create a target docshell to put the merged document into
                xTargetDocShell = new SwDocShell( SfxObjectCreateMode::STANDARD );
                xTargetDocShell->DoInitNew();
                if (nMaxDumpDocs)
                    lcl_SaveDoc( xTargetDocShell, "MergeDoc" );
                SfxViewFrame* pTargetFrame = SfxViewFrame::LoadHiddenDocument( *xTargetDocShell, 0 );
                if (bMergeShell && pSourceWindow) {
                    //the created window has to be located at the same position as the source window
                    vcl::Window& rTargetWindow = pTargetFrame->GetFrame().GetWindow();
                    rTargetWindow.SetPosPixel(pSourceWindow->GetPosPixel());
                }

                pTargetView = static_cast<SwView*>( pTargetFrame->GetViewShell() );

                //initiate SelectShell() to create sub shells
                pTargetView->AttrChangedNotify( &pTargetView->GetWrtShell() );
                pTargetShell = pTargetView->GetWrtShellPtr();
                pTargetDoc = pTargetShell->GetDoc();
                pTargetDoc->SetInMailMerge(true);

                //copy the styles from the source to the target document
                pTargetView->GetDocShell()->_LoadStyles( *pSourceDocSh, true );

                //determine the page style and number used at the start of the source document
                pSourceShell->SttEndDoc(true);
                nStartingPageNo = pSourceShell->GetVirtPageNum();
                sStartingPageDesc = sModifiedStartingPageDesc = pSourceShell->GetPageDesc(
                                            pSourceShell->GetCurPageDesc()).GetName();

                // #i72517#
                const SwPageDesc* pSourcePageDesc = pSourceShell->FindPageDescByName( sStartingPageDesc );
                const SwFrameFormat& rMaster = pSourcePageDesc->GetMaster();
                bPageStylesWithHeaderFooter = rMaster.GetHeader().IsActive()  ||
                                                rMaster.GetFooter().IsActive();

                // copy compatibility options
                pTargetShell->GetDoc()->ReplaceCompatibilityOptions( *pSourceShell->GetDoc());
                // #72821# copy dynamic defaults
                pTargetShell->GetDoc()->ReplaceDefaults( *pSourceShell->GetDoc());

                pTargetShell->GetDoc()->ReplaceDocumentProperties( *pSourceShell->GetDoc());
            }

            // Progress, to prohibit KeyInputs
            SfxProgress aProgress(pSourceDocSh, ::aEmptyOUStr, 1);

            // lock all dispatchers
            SfxViewFrame* pViewFrm = SfxViewFrame::GetFirst(pSourceDocSh);
            while (pViewFrm)
            {
                pViewFrm->GetDispatcher()->Lock(true);
                pViewFrm = SfxViewFrame::GetNext(*pViewFrm, pSourceDocSh);
            }

            sal_Int32 nDocNo = 1;
            sal_Int32 nDocCount = 0;
            // For single file mode, the number of pages in the target document so far, which is used
            // by AppendDoc() to adjust position of page-bound objects. Getting this information directly
            // from the target doc would require repeated layouts of the doc, which is expensive, but
            // it can be manually computed from the source documents (for which we do layouts, so the page
            // count is known, and there is a blank page between each of them in the target document).
            int targetDocPageCount = 0;
            if( !IsMergeSilent() && bMergeShell &&
                    lcl_getCountFromResultSet( nDocCount, pImpl->pMergeData->xResultSet ) )
                static_cast<CreateMonitor*>( pProgressDlg.get() )->SetTotalCount( nDocCount );

            long nStartRow, nEndRow;
            bool bFreezedLayouts = false;
            // collect temporary files
            ::std::vector< OUString> aFilesToRemove;

            // The SfxObjectShell will be closed explicitly later but it is more safe to use SfxObjectShellLock here
            SfxObjectShellLock xWorkDocSh;
            // a view frame for the document
            SwView* pWorkView = NULL;
            SwDoc* pWorkDoc = NULL;
            SwDBManager* pOldDBManager = NULL;

            do
            {
                nStartRow = pImpl->pMergeData ? pImpl->pMergeData->xResultSet->getRow() : 0;
                {
                    OUString sPath(sSubject);

                    OUString sAddress;
                    if( !bEMail && bColumnName )
                    {
                        SwDBFormatData aDBFormat;
                        aDBFormat.xFormatter = pImpl->pMergeData->xFormatter;
                        aDBFormat.aNullDate = pImpl->pMergeData->aNullDate;
                        sAddress = GetDBField( xColumnProp, aDBFormat);
                        if (sAddress.isEmpty())
                            sAddress = "_";
                        sPath += sAddress;
                    }

                    // create a new temporary file name - only done once in case of bCreateSingleFile
                    if( createTempFile && ( 1 == nDocNo || !bCreateSingleFile ))
                    {
                        INetURLObject aEntry(sPath);
                        OUString sLeading;
                        //#i97667# if the name is from a database field then it will be used _as is_
                        if( !sAddress.isEmpty() )
                            sLeading = sAddress;
                        else
                            sLeading = aEntry.GetBase();
                        aEntry.removeSegment();
                        sPath = aEntry.GetMainURL( INetURLObject::NO_DECODE );
                        OUString sExt(comphelper::string::stripStart(pStoreToFilter->GetDefaultExtension(), '*'));
                        aTempFile.reset(
                            new utl::TempFile(sLeading, true, &sExt, &sPath));
                        if( rMergeDescriptor.bSubjectIsFilename )
                            aTempFile->EnableKillingFile();
                        if( !aTempFile->IsValid() )
                        {
                            ErrorHandler::HandleError( ERRCODE_IO_NOTSUPPORTED );
                            bNoError = false;
                            bCancel = true;
                        }
                    }

                    if( !bCancel )
                    {
                        std::unique_ptr< INetURLObject > aTempFileURL;
                        if( createTempFile )
                            aTempFileURL.reset( new INetURLObject(aTempFile->GetURL()));
                        if (!IsMergeSilent()) {
                            if( bMergeShell )
                                static_cast<CreateMonitor*>( pProgressDlg.get() )->SetCurrentPosition( nDocNo );
                            else {
                                PrintMonitor *pPrintMonDlg = static_cast<PrintMonitor*>( pProgressDlg.get() );
                                pPrintMonDlg->m_pPrinter->SetText( createTempFile ? aTempFileURL->GetBase() : OUString( pSourceDocSh->GetTitle( 22 )));
                                OUString sStat(SW_RES(STR_STATSTR_LETTER));   // Brief
                                sStat += " ";
                                sStat += OUString::number( nDocNo );
                                pPrintMonDlg->m_pPrintInfo->SetText( sStat );
                            }
                            pProgressDlg->Update();
                        }

                        // Computation time for the GUI
                        for( sal_uInt16 i = 0; i < 25; i++ )
                            Application::Reschedule();

                        // Create a copy of the source document and work with that one instead of the source.
                        // If we're not in the single file mode (which requires modifying the document for the merging),
                        // it is enough to do this just once.
                        if( 1 == nDocNo || bCreateSingleFile )
                        {
                            assert( !xWorkDocSh.Is());
                            // copy the source document
                            xWorkDocSh = pSourceDocSh->GetDoc()->CreateCopy( true );

                            //create a view frame for the document
                            pWorkView = static_cast< SwView* >( SfxViewFrame::LoadHiddenDocument( *xWorkDocSh, 0 )->GetViewShell() );
                            //request the layout calculation
                            SwWrtShell& rWorkShell = pWorkView->GetWrtShell();
                            pWorkView->AttrChangedNotify( &rWorkShell );// in order for SelectShell to be called

                            pWorkDoc = rWorkShell.GetDoc();
                            pWorkDoc->ReplaceDocumentProperties( *pSourceDocSh->GetDoc());
                            if ( (nMaxDumpDocs < 0) || (nDocNo <= nMaxDumpDocs) )
                                lcl_SaveDoc( xWorkDocSh, "WorkDoc", nDocNo );
                            pOldDBManager = pWorkDoc->GetDBManager();
                            pWorkDoc->SetDBManager( this );
                            pWorkDoc->getIDocumentLinksAdministration().EmbedAllLinks();

                            // #i69458# lock fields to prevent access to the result set while calculating layout
                            // tdf#92324: and do not unlock: keep document locked during printing to avoid
                            // ExpFields update during printing, generation of preview, etc.
                            rWorkShell.LockExpFields();
                            rWorkShell.CalcLayout();
                        }

                        SwWrtShell& rWorkShell = pWorkView->GetWrtShell();
                        SfxGetpApp()->NotifyEvent(SfxEventHint(SW_EVENT_FIELD_MERGE, SwDocShell::GetEventName(STR_SW_EVENT_FIELD_MERGE), xWorkDocSh));
                        // tdf#92324: Allow ExpFields update only by explicit instruction to avoid
                        // database cursor movement on any other fields update, for example during
                        // print preview and other operations
                        if ( rWorkShell.IsExpFieldsLocked() )
                            rWorkShell.UnlockExpFields();
                        rWorkShell.SwViewShell::UpdateFields();
                        rWorkShell.LockExpFields();
                        SfxGetpApp()->NotifyEvent(SfxEventHint(SW_EVENT_FIELD_MERGE_FINISHED, SwDocShell::GetEventName(STR_SW_EVENT_FIELD_MERGE_FINISHED), xWorkDocSh));

                        // launch MailMergeEvent if required
                        const SwXMailMerge *pEvtSrc = GetMailMergeEvtSrc();
                        if(pEvtSrc)
                        {
                            uno::Reference< uno::XInterface > xRef( const_cast<text::XMailMergeBroadcaster *>(static_cast<text::XMailMergeBroadcaster const *>(pEvtSrc)) );
                            text::MailMergeEvent aEvt( xRef, xWorkDocSh->GetModel() );
                            pEvtSrc->LaunchMailMergeEvent( aEvt );
                        }

                        if(bCreateSingleFile)
                        {
                            pWorkDoc->RemoveInvisibleContent();

                            OSL_ENSURE( pTargetShell, "no target shell available!" );
                            // copy created file into the target document
                            rWorkShell.ConvertFieldsToText();
                            rWorkShell.SetNumberingRestart();
                            if( bSynchronizedDoc )
                            {
                                lcl_RemoveSectionLinks( rWorkShell );
                            }

                            // insert the document into the target document

                            //#i72517# put the styles to the target document
                            //if the source uses headers or footers each new copy need to copy a new page styles
                            SwPageDesc* pTargetPageDesc(NULL);
                            if(bPageStylesWithHeaderFooter)
                            {
                                //create a new pagestyle
                                //copy the pagedesc from the current document to the new document and change the name of the to-be-applied style
                                OUString sNewPageDescName = lcl_FindUniqueName(pTargetShell, sStartingPageDesc, nDocNo );
                                pTargetPageDesc = pTargetDoc->MakePageDesc( sNewPageDescName );
                                const SwPageDesc* pWorkPageDesc = rWorkShell.FindPageDescByName( sStartingPageDesc );

                                if(pWorkPageDesc && pTargetPageDesc)
                                {
                                    pTargetDoc->CopyPageDesc( *pWorkPageDesc, *pTargetPageDesc, false );
                                    sModifiedStartingPageDesc = sNewPageDescName;
                                    lcl_CopyFollowPageDesc( *pTargetShell, *pWorkPageDesc, *pTargetPageDesc, nDocNo );
                                }
                            }
                            else
                                pTargetPageDesc = pTargetShell->FindPageDescByName( sModifiedStartingPageDesc );

                            if ( (nMaxDumpDocs < 0) || (nDocNo <= nMaxDumpDocs) )
                                lcl_SaveDoc( xWorkDocSh, "WorkDoc", nDocNo );
                            if( targetDocPageCount % 2 == 1 )
                                ++targetDocPageCount; // Docs always start on odd pages (so offset must be even).
                            SwNodeIndex appendedDocStart = pTargetDoc->AppendDoc(*rWorkShell.GetDoc(),
                                nStartingPageNo, pTargetPageDesc, nDocNo == 1, targetDocPageCount);
                            targetDocPageCount += rWorkShell.GetPageCnt();
                            if ( (nMaxDumpDocs < 0) || (nDocNo <= nMaxDumpDocs) )
                                lcl_SaveDoc( xTargetDocShell, "MergeDoc" );
                            if (bMergeShell)
                            {
                                SwDocMergeInfo aMergeInfo;
                                // Name of the mark is actually irrelevant, UNO bookmarks have internals names.
                                aMergeInfo.startPageInTarget = pTargetDoc->getIDocumentMarkAccess()->makeMark( appendedDocStart, "",
                                    IDocumentMarkAccess::MarkType::UNO_BOOKMARK );
                                aMergeInfo.nDBRow = nStartRow;
                                rMergeDescriptor.pMailMergeConfigItem->AddMergedDocument( aMergeInfo );
                            }
                        }
                        else if( rMergeDescriptor.nMergeType == DBMGR_MERGE_PRINTER )
                        {
                            assert(!bCreateSingleFile);
                            if( 1 == nDocNo ) // set up printing only once at the beginning
                            {
                                // printing should be done synchronously otherwise the document
                                // might already become invalid during the process
                                uno::Sequence< beans::PropertyValue > aOptions( rMergeDescriptor.aPrintOptions );

                                aOptions.realloc( 2 );
                                aOptions[ 0 ].Name = "Wait";
                                aOptions[ 0 ].Value <<= sal_True;
                                aOptions[ 1 ].Name = "MonitorVisible";
                                aOptions[ 1 ].Value <<= sal_False;
                                // move print options
                                const beans::PropertyValue* pPrintOptions = rMergeDescriptor.aPrintOptions.getConstArray();
                                for( sal_Int32 nOption = 0, nIndex = 1 ; nOption < rMergeDescriptor.aPrintOptions.getLength(); ++nOption)
                                {
                                    if( pPrintOptions[nOption].Name == "CopyCount" || pPrintOptions[nOption].Name == "FileName"
                                        || pPrintOptions[nOption].Name == "Collate" || pPrintOptions[nOption].Name == "Pages"
                                        || pPrintOptions[nOption].Name == "Wait" || pPrintOptions[nOption].Name == "PrinterName" )
                                    {
                                        // add an option
                                        aOptions.realloc( nIndex + 1 );
                                        aOptions[ nIndex ].Name = pPrintOptions[nOption].Name;
                                        aOptions[ nIndex++ ].Value = pPrintOptions[nOption].Value ;
                                    }
                                }
                                pWorkView->StartPrint( aOptions, IsMergeSilent(), rMergeDescriptor.bPrintAsync );
                                SfxPrinter* pDocPrt = pWorkView->GetPrinter();
                                JobSetup aJobSetup = pDocPrt ? pDocPrt->GetJobSetup() : SfxViewShell::GetJobSetup();
                                bCancel = !Printer::PreparePrintJob( pWorkView->GetPrinterController(), aJobSetup );
#if ENABLE_CUPS && !defined(MACOSX)
                                if( !bCancel )
                                    psp::PrinterInfoManager::get().startBatchPrint();
#endif
                            }
                            if( !bCancel && !Printer::ExecutePrintJob( pWorkView->GetPrinterController()))
                                bCancel = true;
                        }
                        else
                        {
                            assert( createTempFile );
                            OUString sFileURL =  aTempFileURL->GetMainURL( INetURLObject::NO_DECODE );
                            SfxMedium* pDstMed = new SfxMedium(
                                sFileURL,
                                STREAM_STD_READWRITE );
                            pDstMed->SetFilter( pStoreToFilter );
                            if(pDstMed->GetItemSet())
                            {
                                if(pStoreToFilterOptions )
                                    pDstMed->GetItemSet()->Put(SfxStringItem(SID_FILE_FILTEROPTIONS, *pStoreToFilterOptions));
                                if(rMergeDescriptor.aSaveToFilterData.getLength())
                                    pDstMed->GetItemSet()->Put(SfxUsrAnyItem(SID_FILTER_DATA, uno::makeAny(rMergeDescriptor.aSaveToFilterData)));
                            }

                            //convert fields to text if we are exporting to PDF
                            //this prevents a second merge while updating the fields in SwXTextDocument::getRendererCount()
                            if( pStoreToFilter && pStoreToFilter->GetFilterName() == "writer_pdf_Export")
                                rWorkShell.ConvertFieldsToText();
                            xWorkDocSh->DoSaveAs(*pDstMed);
                            xWorkDocSh->DoSaveCompleted(pDstMed);
                            if( xWorkDocSh->GetError() )
                            {
                                // error message ??
                                ErrorHandler::HandleError( xWorkDocSh->GetError() );
                                bCancel = true;
                                bNoError = false;
                            }
                            if( bEMail )
                            {
                                SwDBFormatData aDBFormat;
                                aDBFormat.xFormatter = pImpl->pMergeData->xFormatter;
                                aDBFormat.aNullDate = pImpl->pMergeData->aNullDate;
                                OUString sMailAddress = GetDBField( xColumnProp, aDBFormat);
                                if(!SwMailMergeHelper::CheckMailAddress( sMailAddress ))
                                {
                                    OSL_FAIL("invalid e-Mail address in database column");
                                }
                                else
                                {
                                    SwMailMessage* pMessage = new SwMailMessage;
                                    uno::Reference< mail::XMailMessage > xMessage = pMessage;
                                    if(rMergeDescriptor.pMailMergeConfigItem->IsMailReplyTo())
                                        pMessage->setReplyToAddress(rMergeDescriptor.pMailMergeConfigItem->GetMailReplyTo());
                                    pMessage->addRecipient( sMailAddress );
                                    pMessage->SetSenderAddress( rMergeDescriptor.pMailMergeConfigItem->GetMailAddress() );
                                    OUString sBody;
                                    if(rMergeDescriptor.bSendAsAttachment)
                                    {
                                        sBody = rMergeDescriptor.sMailBody;
                                        mail::MailAttachment aAttach;
                                        aAttach.Data = new SwMailTransferable(
                                                sFileURL,
                                                rMergeDescriptor.sAttachmentName,
                                                pStoreToFilter->GetMimeType());
                                        aAttach.ReadableName = rMergeDescriptor.sAttachmentName;
                                        pMessage->addAttachment( aAttach );
                                    }
                                    else
                                    {
                                        {
                                            //read in the temporary file and use it as mail body
                                            SfxMedium aMedium( sFileURL, StreamMode::READ);
                                            SvStream* pInStream = aMedium.GetInStream();
                                            OSL_ENSURE(pInStream, "no output file created?");
                                            if(pInStream)
                                            {
                                                pInStream->SetStreamCharSet( eEncoding );
                                                OString sLine;
                                                bool bDone = pInStream->ReadLine( sLine );
                                                while ( bDone )
                                                {
                                                    sBody += OStringToOUString(sLine, eEncoding);
                                                    sBody += "\n";
                                                    bDone = pInStream->ReadLine( sLine );
                                                }
                                            }
                                        }
                                    }
                                    pMessage->setSubject( rMergeDescriptor.sSubject );
                                    uno::Reference< datatransfer::XTransferable> xBody =
                                                new SwMailTransferable(
                                                    sBody,
                                                    sBodyMimeType);
                                    pMessage->setBody( xBody );

                                    if(rMergeDescriptor.aCopiesTo.getLength())
                                    {
                                        const OUString* pCopies = rMergeDescriptor.aCopiesTo.getConstArray();
                                        for( sal_Int32 nToken = 0; nToken < rMergeDescriptor.aCopiesTo.getLength(); ++nToken)
                                            pMessage->addCcRecipient( pCopies[nToken] );
                                    }
                                    if(rMergeDescriptor.aBlindCopiesTo.getLength())
                                    {
                                        const OUString* pCopies = rMergeDescriptor.aBlindCopiesTo.getConstArray();
                                        for( sal_Int32 nToken = 0; nToken < rMergeDescriptor.aBlindCopiesTo.getLength(); ++nToken)
                                            pMessage->addBccRecipient( pCopies[nToken] );
                                    }
                                    xMailDispatcher->enqueueMailMessage( xMessage );
                                    if(!xMailDispatcher->isStarted())
                                            xMailDispatcher->start();
                                    //schedule for removal
                                    aFilesToRemove.push_back(sFileURL);
                                }
                            }
                        }
                        if( bCreateSingleFile )
                        {
                            pWorkDoc->SetDBManager( pOldDBManager );
                            xWorkDocSh->DoClose();
                            xWorkDocSh = NULL;
                        }
                    }
                }
                nDocNo++;
                nEndRow = pImpl->pMergeData ? pImpl->pMergeData->xResultSet->getRow() : 0;

                // Freeze the layouts of the target document after the first inserted
                // sub-document, to get the correct PageDesc.
                if(!bFreezedLayouts && bCreateSingleFile)
                {
                    std::set<SwRootFrm*> aAllLayouts = pTargetShell->GetDoc()->GetAllLayouts();
                    std::for_each( aAllLayouts.begin(), aAllLayouts.end(),
                        [](SwRootFrm* pLayout) { pLayout->FreezeLayout(true); });
                    bFreezedLayouts = true;
                }
            } while( !bCancel &&
                (bSynchronizedDoc && (nStartRow != nEndRow)? ExistsNextRecord() : ToNextMergeRecord()));

            if ( xWorkDocSh.Is() && pWorkView->GetWrtShell().IsExpFieldsLocked() )
            {
                // Unlock document fields after merge complete
                pWorkView->GetWrtShell().UnlockExpFields();
            }

            if( !bCreateSingleFile )
            {
                if( rMergeDescriptor.nMergeType == DBMGR_MERGE_PRINTER )
                {
                    Printer::FinishPrintJob( pWorkView->GetPrinterController());
#if ENABLE_CUPS && !defined(MACOSX)
                    psp::PrinterInfoManager::get().flushBatchPrint();
#endif
                }
                pWorkDoc->SetDBManager( pOldDBManager );
                xWorkDocSh->DoClose();
            }

            if (bCreateSingleFile)
            {
                // sw::DocumentLayoutManager::CopyLayoutFormat() did not generate
                // unique fly names, do it here once.
                pTargetDoc->SetInMailMerge(false);
                pTargetDoc->SetAllUniqueFlyNames();
            }

            for( sal_uInt16 i = 0; i < 25; i++)
                Application::Reschedule();

            // Unfreeze target document layouts and correct all PageDescs.
            if(bCreateSingleFile)
            {
                pTargetShell->CalcLayout();
                std::set<SwRootFrm*> aAllLayouts = pTargetShell->GetDoc()->GetAllLayouts();
                std::for_each( aAllLayouts.begin(), aAllLayouts.end(), [](SwRootFrm* pLayout)
                {
                    pLayout->FreezeLayout(false);
                    pLayout->AllCheckPageDescs();
                });
            }

            pProgressDlg.disposeAndClear();

            // save the single output document
            if (bMergeShell)
            {
                rMergeDescriptor.pMailMergeConfigItem->SetTargetView( pTargetView );
            }
            else if(bCreateSingleFile)
            {
                if( rMergeDescriptor.nMergeType != DBMGR_MERGE_PRINTER )
                {
                    if( !bCancel )
                    {
                        assert( aTempFile.get());
                        INetURLObject aTempFileURL( rMergeDescriptor.bSubjectIsFilename ? sSubject : aTempFile->GetURL());
                        SfxMedium* pDstMed = new SfxMedium(
                            aTempFileURL.GetMainURL( INetURLObject::NO_DECODE ),
                            STREAM_STD_READWRITE );
                        pDstMed->SetFilter( pStoreToFilter );
                        if(pDstMed->GetItemSet())
                        {
                            if(pStoreToFilterOptions )
                                pDstMed->GetItemSet()->Put(SfxStringItem(SID_FILE_FILTEROPTIONS, *pStoreToFilterOptions));
                            if(rMergeDescriptor.aSaveToFilterData.getLength())
                                pDstMed->GetItemSet()->Put(SfxUsrAnyItem(SID_FILTER_DATA, uno::makeAny(rMergeDescriptor.aSaveToFilterData)));
                        }

                        xTargetDocShell->DoSaveAs(*pDstMed);
                        xTargetDocShell->DoSaveCompleted(pDstMed);
                        if( xTargetDocShell->GetError() )
                        {
                            // error message ??
                            ErrorHandler::HandleError( xTargetDocShell->GetError() );
                            bNoError = false;
                        }
                    }
                }
                else if( pTargetView ) // must be available!
                {
                    //print the target document
                    // printing should be done synchronously otherwise the document
                    // might already become invalid during the process
                    uno::Sequence< beans::PropertyValue > aOptions( rMergeDescriptor.aPrintOptions );

                    aOptions.realloc( 1 );
                    aOptions[ 0 ].Name = "Wait";
                    aOptions[ 0 ].Value <<= sal_True ;
                    // move print options
                    const beans::PropertyValue* pPrintOptions = rMergeDescriptor.aPrintOptions.getConstArray();
                    for( sal_Int32 nOption = 0, nIndex = 1 ; nOption < rMergeDescriptor.aPrintOptions.getLength(); ++nOption)
                    {
                        if( pPrintOptions[nOption].Name == "CopyCount" || pPrintOptions[nOption].Name == "FileName"
                            || pPrintOptions[nOption].Name == "Collate" || pPrintOptions[nOption].Name == "Pages"
                            || pPrintOptions[nOption].Name == "Wait" || pPrintOptions[nOption].Name == "PrinterName" )
                        {
                            // add an option
                            aOptions.realloc( nIndex + 1 );
                            aOptions[ nIndex ].Name = pPrintOptions[nOption].Name;
                            aOptions[ nIndex++ ].Value = pPrintOptions[nOption].Value ;
                        }
                    }

                        pTargetView->ExecPrint( aOptions, IsMergeSilent(), rMergeDescriptor.bPrintAsync );
                }

                // Leave docshell available for caller (e.g. MM wizard)
                if (!bMergeShell)
                    xTargetDocShell->DoClose();
            }

            //remove the temporary files
            ::std::vector<OUString>::iterator aFileIter;
            for(aFileIter = aFilesToRemove.begin();
                        aFileIter != aFilesToRemove.end(); ++aFileIter)
                SWUnoHelper::UCB_DeleteFile( *aFileIter );

            // unlock all dispatchers
            pViewFrm = SfxViewFrame::GetFirst(pSourceDocSh);
            while (pViewFrm)
            {
                pViewFrm->GetDispatcher()->Lock(false);
                pViewFrm = SfxViewFrame::GetNext(*pViewFrm, pSourceDocSh);
            }

            SW_MOD()->SetView(&pSourceShell->GetView());
        }
    }

    if(bEMail)
    {
        xMailDispatcher->stop();
        xMailDispatcher->shutdown();
    }

    return bNoError;
}

void SwDBManager::MergeCancel()
{
    bCancel = true;
}

IMPL_LINK_TYPED( SwDBManager, PrtCancelHdl, Button *, pButton, void )
{
    pButton->GetParent()->Hide();
    MergeCancel();
}

// determine the column's Numberformat and transfer to the forwarded Formatter,
// if applicable.
sal_uLong SwDBManager::GetColumnFormat( const OUString& rDBName,
                                const OUString& rTableName,
                                const OUString& rColNm,
                                SvNumberFormatter* pNFormatr,
                                long nLanguage )
{
    sal_uLong nRet = 0;
    if(pNFormatr)
    {
        uno::Reference< sdbc::XDataSource> xSource;
        uno::Reference< sdbc::XConnection> xConnection;
        bool bUseMergeData = false;
        uno::Reference< sdbcx::XColumnsSupplier> xColsSupp;
        bool bDisposeConnection = false;
        if(pImpl->pMergeData &&
            pImpl->pMergeData->sDataSource.equals(rDBName) && pImpl->pMergeData->sCommand.equals(rTableName))
        {
            xConnection = pImpl->pMergeData->xConnection;
            xSource = SwDBManager::getDataSourceAsParent(xConnection,rDBName);
            bUseMergeData = true;
            xColsSupp.set(pImpl->pMergeData->xResultSet, css::uno::UNO_QUERY);
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
                OUString sDBName(rDBName);
                xConnection = RegisterConnection( sDBName );
                bDisposeConnection = true;
            }
            if(bUseMergeData)
                pImpl->pMergeData->xConnection = xConnection;
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
                SAL_WARN("sw.mailmerge", "Exception in getColumns(): " << e.Message);
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

sal_uLong SwDBManager::GetColumnFormat( uno::Reference< sdbc::XDataSource> xSource,
                        uno::Reference< sdbc::XConnection> xConnection,
                        uno::Reference< beans::XPropertySet> xColumn,
                        SvNumberFormatter* pNFormatr,
                        long nLanguage )
{
    // set the NumberFormat in the doc if applicable
    sal_uLong nRet = 0;

    if(!xSource.is())
    {
        uno::Reference<container::XChild> xChild(xConnection, uno::UNO_QUERY);
        if ( xChild.is() )
            xSource = uno::Reference<sdbc::XDataSource>(xChild->getParent(), uno::UNO_QUERY);
    }
    if(xSource.is() && xConnection.is() && xColumn.is() && pNFormatr)
    {
        SvNumberFormatsSupplierObj* pNumFormat = new SvNumberFormatsSupplierObj( pNFormatr );
        uno::Reference< util::XNumberFormatsSupplier >  xDocNumFormatsSupplier = pNumFormat;
        uno::Reference< util::XNumberFormats > xDocNumberFormats = xDocNumFormatsSupplier->getNumberFormats();
        uno::Reference< util::XNumberFormatTypes > xDocNumberFormatTypes(xDocNumberFormats, uno::UNO_QUERY);

        com::sun::star::lang::Locale aLocale( LanguageTag( (LanguageType)nLanguage ).getLocale());

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
                        nFormat = xDocNumberFormats->queryKey( sFormat, aLoc, sal_False );
                        if(NUMBERFORMAT_ENTRY_NOT_FOUND == sal::static_int_cast< sal_uInt32, sal_Int32>(nFormat))
                            nFormat = xDocNumberFormats->addNew( sFormat, aLoc );
                        nRet = nFormat;
                        bUseDefault = false;
                    }
                    catch (const uno::Exception& e)
                    {
                        SAL_WARN("sw.mailmerge", "illegal number format key: " << e.Message);
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
        xColsSupp = uno::Reference< sdbcx::XColumnsSupplier >( pParam->xResultSet, uno::UNO_QUERY );
    }
    else
    {
        OUString sDBName(rDBName);
        xConnection = RegisterConnection( sDBName );
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
                                                    uno::Reference<sdbc::XDataSource>& rxSource)
{
    uno::Reference< sdbc::XConnection> xConnection;
    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    try
    {
        uno::Reference<sdb::XCompletedConnection> xComplConnection(dbtools::getDataSource(rDataSource, xContext), uno::UNO_QUERY);
        if ( xComplConnection.is() )
        {
            rxSource.set(xComplConnection, uno::UNO_QUERY);
            uno::Reference< task::XInteractionHandler > xHandler( task::InteractionHandler::createWithParent(xContext, 0), uno::UNO_QUERY_THROW );
            xConnection = xComplConnection->connectWithCompletion( xHandler );
        }
    }
    catch(const uno::Exception&)
    {
    }

    return xConnection;
}

uno::Reference< sdbcx::XColumnsSupplier> SwDBManager::GetColumnSupplier(uno::Reference<sdbc::XConnection> xConnection,
                                    const OUString& rTableOrQuery,
                                    SwDBSelect   eTableOrQuery)
{
    uno::Reference< sdbcx::XColumnsSupplier> xRet;
    try
    {
        if(eTableOrQuery == SwDBSelect::UNKNOWN)
        {
            //search for a table with the given command name
            uno::Reference<sdbcx::XTablesSupplier> xTSupplier = uno::Reference<sdbcx::XTablesSupplier>(xConnection, uno::UNO_QUERY);
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
        xRowProperties->setPropertyValue("Command", uno::makeAny(OUString(rTableOrQuery)));
        xRowProperties->setPropertyValue("CommandType", uno::makeAny(nCommandType));
        xRowProperties->setPropertyValue("FetchSize", uno::makeAny((sal_Int32)10));
        xRowProperties->setPropertyValue("ActiveConnection", uno::makeAny(xConnection));
        xRowSet->execute();
        xRet = uno::Reference<sdbcx::XColumnsSupplier>( xRowSet, uno::UNO_QUERY );
    }
    catch (const uno::Exception& e)
    {
        SAL_WARN("sw.mailmerge", "Exception in SwDBManager::GetColumnSupplier: " << e.Message);
    }

    return xRet;
}

OUString SwDBManager::GetDBField(uno::Reference<beans::XPropertySet> xColumnProps,
                        const SwDBFormatData& rDBFormatData,
                        double* pNumber)
{
    uno::Reference< sdb::XColumn > xColumn(xColumnProps, uno::UNO_QUERY);
    OUString sRet;
    OSL_ENSURE(xColumn.is(), "SwDBManager::::ImportDBField: illegal arguments");
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
            catch (const uno::Exception& e)
            {
                SAL_WARN("sw.mailmerge", "exception caught: " << e.Message);
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
    if(pImpl->pMergeData)
    {
        return !bMergeLock &&
                ((rDataSource == pImpl->pMergeData->sDataSource &&
                    rTableOrQuery == pImpl->pMergeData->sCommand)
                    ||(rDataSource.isEmpty() && rTableOrQuery.isEmpty()))
                    &&
                    pImpl->pMergeData->xResultSet.is();
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
                           long nLanguage,
                           OUString& rResult, double* pNumber)
{
    bool bRet = false;
    SwDSParam* pFound = 0;
    //check if it's the merge data source
    if(pImpl->pMergeData &&
        rSourceName == pImpl->pMergeData->sDataSource &&
        rTableName == pImpl->pMergeData->sCommand)
    {
        pFound = pImpl->pMergeData;
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
    if(pFound->xResultSet.is() && !pFound->bAfterSelection)
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
bool    SwDBManager::GetMergeColumnCnt(const OUString& rColumnName, sal_uInt16 nLanguage,
                                   OUString &rResult, double *pNumber)
{
    if(!pImpl->pMergeData || !pImpl->pMergeData->xResultSet.is() || pImpl->pMergeData->bAfterSelection )
    {
        rResult.clear();
        return false;
    }

    bool bRet = lcl_GetColumnCnt(pImpl->pMergeData, rColumnName, nLanguage, rResult, pNumber);
    return bRet;
}

bool SwDBManager::ToNextMergeRecord()
{
    OSL_ENSURE(pImpl->pMergeData && pImpl->pMergeData->xResultSet.is(), "no data source in merge");
    return ToNextRecord(pImpl->pMergeData);
}

bool SwDBManager::FillCalcWithMergeData( SvNumberFormatter *pDocFormatter,
                                         sal_uInt16 nLanguage, bool asString, SwCalc &rCalc )
{
    if (!(pImpl->pMergeData && pImpl->pMergeData->xResultSet.is()))
        return false;

    uno::Reference< sdbcx::XColumnsSupplier > xColsSupp( pImpl->pMergeData->xResultSet, uno::UNO_QUERY );
    if(xColsSupp.is())
    {
        uno::Reference<container::XNameAccess> xCols = xColsSupp->getColumns();
        const uno::Sequence<OUString> aColNames = xCols->getElementNames();
        const OUString* pColNames = aColNames.getConstArray();
        OUString aString;

        const bool bExistsNextRecord = ExistsNextRecord();

        for( int nCol = 0; nCol < aColNames.getLength(); nCol++ )
        {
            const OUString &rColName = pColNames[nCol];

            // empty variables, if no more records;
            if( !bExistsNextRecord )
            {
                rCalc.VarChange( rColName, 0 );
                continue;
            }

            // get the column type
            sal_Int32 nColumnType = sdbc::DataType::SQLNULL;
            uno::Any aCol = xCols->getByName( pColNames[nCol] );
            uno::Reference<beans::XPropertySet> xColumnProps;
            aCol >>= xColumnProps;
            uno::Any aType = xColumnProps->getPropertyValue( "Type" );
            aType >>= nColumnType;
            double aNumber = DBL_MAX;

            lcl_GetColumnCnt( pImpl->pMergeData, xColumnProps, nLanguage, aString, &aNumber );

            sal_uInt32 nFormat = GetColumnFormat( pImpl->pMergeData->sDataSource,
                                            pImpl->pMergeData->sCommand,
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
                    if( !asString )
                        aValue.PutDouble( aNumber );
                    else
                        aValue.PutString( aString );
                    SAL_INFO( "sw.dbmgr", "'" << pColNames[nCol] << "': " << aNumber << " / " << aString );
                    rCalc.VarChange( pColNames[nCol], aValue );
                }
            }
            else
            {
                SwSbxValue aValue;
                aValue.PutString( aString );
                SAL_INFO( "sw.dbmgr", "'" << pColNames[nCol] << "': " << aString );
                rCalc.VarChange( pColNames[nCol], aValue );
            }
        }
        return bExistsNextRecord;
    }
    return false;
}

bool SwDBManager::ToNextRecord(
    const OUString& rDataSource, const OUString& rCommand, sal_Int32 /*nCommandType*/)
{
    SwDSParam* pFound = 0;
    if(pImpl->pMergeData &&
        rDataSource == pImpl->pMergeData->sDataSource &&
        rCommand == pImpl->pMergeData->sCommand)
        pFound = pImpl->pMergeData;
    else
    {
        SwDBData aData;
        aData.sDataSource = rDataSource;
        aData.sCommand = rCommand;
        aData.nCommandType = -1;
        pFound = FindDSData(aData, false);
    }
    return ToNextRecord(pFound);
}

bool SwDBManager::ToNextRecord(SwDSParam* pParam)
{
    bool bRet = true;
    if(!pParam || !pParam->xResultSet.is() || pParam->bEndOfDB ||
            (pParam->aSelection.getLength() && pParam->aSelection.getLength() <= pParam->nSelectionIndex))
    {
        if(pParam)
            pParam->CheckEndOfDB();
        return false;
    }
    try
    {
        if(pParam->aSelection.getLength())
        {
            sal_Int32 nPos = 0;
            pParam->aSelection.getConstArray()[ pParam->nSelectionIndex++ ] >>= nPos;
            pParam->bEndOfDB = !pParam->xResultSet->absolute( nPos );
            pParam->CheckEndOfDB();
            bRet = !pParam->bEndOfDB;
            if(pParam->nSelectionIndex >= pParam->aSelection.getLength())
                pParam->bEndOfDB = true;
        }
        else
        {
            sal_Int32 nBefore = pParam->xResultSet->getRow();
            pParam->bEndOfDB = !pParam->xResultSet->next();
            if( !pParam->bEndOfDB && nBefore == pParam->xResultSet->getRow())
            {
                //next returned true but it didn't move
                pParam->bEndOfDB = true;
            }

            pParam->CheckEndOfDB();
            bRet = !pParam->bEndOfDB;
            ++pParam->nSelectionIndex;
        }
    }
    catch(const uno::Exception&)
    {
    }
    return bRet;
}

// synchronized labels contain a next record field at their end
// to assure that the next page can be created in mail merge
// the cursor position must be validated
bool SwDBManager::ExistsNextRecord() const
{
    return pImpl->pMergeData && !pImpl->pMergeData->bEndOfDB;
}

sal_uInt32  SwDBManager::GetSelectedRecordId()
{
    sal_uInt32  nRet = 0;
    OSL_ENSURE(pImpl->pMergeData && pImpl->pMergeData->xResultSet.is(), "no data source in merge");
    if(!pImpl->pMergeData || !pImpl->pMergeData->xResultSet.is())
        return 0;
    try
    {
        nRet = pImpl->pMergeData->xResultSet->getRow();
    }
    catch(const uno::Exception&)
    {
    }
    return nRet;
}

bool SwDBManager::ToRecordId(sal_Int32 nSet)
{
    OSL_ENSURE(pImpl->pMergeData && pImpl->pMergeData->xResultSet.is(), "no data source in merge");
    if(!pImpl->pMergeData || !pImpl->pMergeData->xResultSet.is()|| nSet < 0)
        return false;
    bool bRet = false;
    sal_Int32 nAbsPos = nSet;

    if(nAbsPos >= 0)
    {
        bRet = lcl_MoveAbsolute(pImpl->pMergeData, nAbsPos);
        pImpl->pMergeData->bEndOfDB = !bRet;
        pImpl->pMergeData->CheckEndOfDB();
    }
    return bRet;
}

bool SwDBManager::OpenDataSource(const OUString& rDataSource, const OUString& rTableOrQuery,
            sal_Int32 nCommandType, bool bCreate)
{
    SwDBData aData;
    aData.sDataSource = rDataSource;
    aData.sCommand = rTableOrQuery;
    aData.nCommandType = nCommandType;

    SwDSParam* pFound = FindDSData(aData, true);
    uno::Reference< sdbc::XDataSource> xSource;
    if(pFound->xResultSet.is())
        return true;
    SwDSParam* pParam = FindDSConnection(rDataSource, false);
    uno::Reference< sdbc::XConnection> xConnection;
    if(pParam && pParam->xConnection.is())
        pFound->xConnection = pParam->xConnection;
    else if(bCreate)
    {
        OUString sDataSource(rDataSource);
        pFound->xConnection = RegisterConnection( sDataSource );
    }
    if(pFound->xConnection.is())
    {
        try
        {
            uno::Reference< sdbc::XDatabaseMetaData >  xMetaData = pFound->xConnection->getMetaData();
            try
            {
                pFound->bScrollable = xMetaData
                        ->supportsResultSetType((sal_Int32)sdbc::ResultSetType::SCROLL_INSENSITIVE);
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
            pFound->bAfterSelection = false;
            pFound->CheckEndOfDB();
            ++pFound->nSelectionIndex;
        }
        catch (const uno::Exception&)
        {
            pFound->xResultSet = 0;
            pFound->xStatement = 0;
            pFound->xConnection = 0;
        }
    }
    return pFound->xResultSet.is();
}

uno::Reference< sdbc::XConnection> SwDBManager::RegisterConnection(OUString& rDataSource)
{
    SwDSParam* pFound = SwDBManager::FindDSConnection(rDataSource, true);
    uno::Reference< sdbc::XDataSource> xSource;
    if(!pFound->xConnection.is())
    {
        pFound->xConnection = SwDBManager::GetConnection(rDataSource, xSource );
        try
        {
            uno::Reference<lang::XComponent> xComponent(pFound->xConnection, uno::UNO_QUERY);
            if(xComponent.is())
                xComponent->addEventListener(pImpl->m_xDisposeListener.get());
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
    if(pImpl->pMergeData && rDataSource == pImpl->pMergeData->sDataSource &&
                    rTableOrQuery == pImpl->pMergeData->sCommand &&
                    (nCommandType == -1 || nCommandType == pImpl->pMergeData->nCommandType) &&
                    pImpl->pMergeData->xResultSet.is())
        nRet = GetSelectedRecordId();
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
        if(bIncludingMerge || pParam.get() != pImpl->pMergeData)
        {
            pParam->nSelectionIndex = 0;
            pParam->bAfterSelection = false;
            pParam->bEndOfDB = false;
            try
            {
                if(!bInMerge && pParam->xResultSet.is())
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
    if(pImpl->pMergeData && rData.sDataSource == pImpl->pMergeData->sDataSource &&
        rData.sCommand == pImpl->pMergeData->sCommand &&
        (rData.nCommandType == -1 || rData.nCommandType == pImpl->pMergeData->nCommandType ||
        (bCreate && pImpl->pMergeData->nCommandType == -1)))
    {
         return pImpl->pMergeData;
    }

    SwDSParam* pFound = 0;
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
    if(bCreate)
    {
        if(!pFound)
        {
            pFound = new SwDSParam(rData);
            m_DataSourceParams.push_back(std::unique_ptr<SwDSParam>(pFound));
            try
            {
                uno::Reference<lang::XComponent> xComponent(pFound->xConnection, uno::UNO_QUERY);
                if(xComponent.is())
                    xComponent->addEventListener(pImpl->m_xDisposeListener.get());
            }
            catch(const uno::Exception&)
            {
            }
        }
    }
    return pFound;
}

SwDSParam*  SwDBManager::FindDSConnection(const OUString& rDataSource, bool bCreate)
{
    //prefer merge data if available
    if(pImpl->pMergeData && rDataSource == pImpl->pMergeData->sDataSource )
    {
         return pImpl->pMergeData;
    }
    SwDSParam* pFound = 0;
    for (auto & pParam : m_DataSourceParams)
    {
        if(rDataSource == pParam->sDataSource)
        {
            pFound = pParam.get();
            break;
        }
    }
    if(bCreate && !pFound)
    {
        SwDBData aData;
        aData.sDataSource = rDataSource;
        pFound = new SwDSParam(aData);
        m_DataSourceParams.push_back(std::unique_ptr<SwDSParam>(pFound));
        try
        {
            uno::Reference<lang::XComponent> xComponent(pFound->xConnection, uno::UNO_QUERY);
            if(xComponent.is())
                xComponent->addEventListener(pImpl->m_xDisposeListener.get());
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

OUString SwDBManager::LoadAndRegisterDataSource(SwDocShell* pDocShell)
{
    sfx2::FileDialogHelper aDlgHelper( ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );
    uno::Reference < ui::dialogs::XFilePicker2 > xFP = aDlgHelper.GetFilePicker();

    OUString sHomePath(SvtPathOptions().GetWorkPath());
    aDlgHelper.SetDisplayDirectory( sHomePath );

    uno::Reference<ui::dialogs::XFilterManager> xFltMgr(xFP, uno::UNO_QUERY);

    OUString sFilterAll(SW_RES(STR_FILTER_ALL));
    OUString sFilterAllData(SW_RES(STR_FILTER_ALL_DATA));
    OUString sFilterSXB(SW_RES(STR_FILTER_SXB));
    OUString sFilterSXC(SW_RES(STR_FILTER_SXC));
    OUString sFilterDBF(SW_RES(STR_FILTER_DBF));
    OUString sFilterXLS(SW_RES(STR_FILTER_XLS));
    OUString sFilterTXT(SW_RES(STR_FILTER_TXT));
    OUString sFilterCSV(SW_RES(STR_FILTER_CSV));
#ifdef WNT
    OUString sFilterMDB(SW_RES(STR_FILTER_MDB));
    OUString sFilterACCDB(SW_RES(STR_FILTER_ACCDB));
#endif
    xFltMgr->appendFilter( sFilterAll, "*" );
    xFltMgr->appendFilter( sFilterAllData, "*.ods;*.sxc;*.dbf;*.xls;*.txt;*.csv");

    xFltMgr->appendFilter( sFilterSXB, "*.odb" );
    xFltMgr->appendFilter( sFilterSXC, "*.ods;*.sxc" );
    xFltMgr->appendFilter( sFilterDBF, "*.dbf" );
    xFltMgr->appendFilter( sFilterXLS, "*.xls" );
    xFltMgr->appendFilter( sFilterTXT, "*.txt" );
    xFltMgr->appendFilter( sFilterCSV, "*.csv" );
#ifdef WNT
    xFltMgr->appendFilter( sFilterMDB, "*.mdb" );
    xFltMgr->appendFilter( sFilterACCDB, "*.accdb" );
#endif

    xFltMgr->setCurrentFilter( sFilterAll ) ;
    OUString sFind;
    if( ERRCODE_NONE == aDlgHelper.Execute() )
    {
        uno::Any aURLAny;
        uno::Reference< beans::XPropertySet > aSettings;
        const OUString aURI( xFP->getSelectedFiles().getConstArray()[0] );
        const DBConnURITypes type = GetDBunoURI( aURI, aURLAny );

        if( DBCONN_FLAT == type )
        {
            uno::Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
            uno::Reference < sdb::XTextConnectionSettings > xSettingsDlg = sdb::TextConnectionSettings::create(xContext);
            if( xSettingsDlg->execute() )
                aSettings.set( uno::Reference < beans::XPropertySet >( xSettingsDlg, uno::UNO_QUERY_THROW ) );
        }
        sFind = LoadAndRegisterDataSource( type, aURLAny, DBCONN_FLAT == type ? &aSettings : 0, aURI, 0, 0, pDocShell );
    }
    return sFind;
}

SwDBManager::DBConnURITypes SwDBManager::GetDBunoURI(const OUString &rURI, uno::Any &aURLAny)
{
    INetURLObject aURL( rURI );
    OUString sExt( aURL.GetExtension() );
    DBConnURITypes type = DBCONN_UNKNOWN;

    if(sExt == "odb")
    {
        type = DBCONN_ODB;
    }
    else if(sExt.equalsIgnoreAsciiCase("sxc")
        || sExt.equalsIgnoreAsciiCase("ods")
            || sExt.equalsIgnoreAsciiCase("xls"))
    {
        OUString sDBURL("sdbc:calc:");
        sDBURL += aURL.GetMainURL(INetURLObject::NO_DECODE);
        aURLAny <<= sDBURL;
        type = DBCONN_CALC;
    }
    else if(sExt.equalsIgnoreAsciiCase("dbf"))
    {
        aURL.removeSegment();
        aURL.removeFinalSlash();
        OUString sDBURL("sdbc:dbase:");
        sDBURL += aURL.GetMainURL(INetURLObject::NO_DECODE);
        aURLAny <<= sDBURL;
        type = DBCONN_DBASE;
    }
    else if(sExt.equalsIgnoreAsciiCase("csv") || sExt.equalsIgnoreAsciiCase("txt"))
    {
        aURL.removeSegment();
        aURL.removeFinalSlash();
        OUString sDBURL("sdbc:flat:");
        //only the 'path' has to be added
        sDBURL += aURL.GetMainURL(INetURLObject::NO_DECODE);
        aURLAny <<= sDBURL;
        type = DBCONN_FLAT;
    }
#ifdef WNT
    else if(sExt.equalsIgnoreAsciiCase("mdb"))
    {
        OUString sDBURL("sdbc:ado:access:PROVIDER=Microsoft.Jet.OLEDB.4.0;DATA SOURCE=");
        sDBURL += aURL.PathToFileName();
        aURLAny <<= sDBURL;
        type = DBCONN_MSJET;
    }
    else if(sExt.equalsIgnoreAsciiCase("accdb"))
    {
        OUString sDBURL("sdbc:ado:PROVIDER=Microsoft.ACE.OLEDB.12.0;DATA SOURCE=");
        sDBURL += aURL.PathToFileName();
        aURLAny <<= sDBURL;
        type = DBCONN_MSACE;
    }
#endif
    return type;
}

/// Returns the URL of this SwDoc.
OUString lcl_getOwnURL(SwDocShell* pDocShell)
{
    OUString aRet;

    if (!pDocShell)
        return aRet;

    const INetURLObject& rURLObject = pDocShell->GetMedium()->GetURLObject();
    aRet = rURLObject.GetMainURL(INetURLObject::DECODE_WITH_CHARSET);
    return aRet;
}

OUString SwDBManager::LoadAndRegisterDataSource(const DBConnURITypes type, const uno::Any &aURLAny, const uno::Reference< beans::XPropertySet > *pSettings,
                                                const OUString &rURI, const OUString *pPrefix, const OUString *pDestDir, SwDocShell* pDocShell)
{
    INetURLObject aURL( rURI );
    OUString sExt( aURL.GetExtension() );
    uno::Any aTableFilterAny;
    uno::Any aSuppressVersionsAny;
    uno::Any aInfoAny;
    bool bStore = true;
    OUString sFind;
    uno::Sequence<OUString> aFilters(1);

    switch (type) {
    case DBCONN_UNKNOWN:
    case DBCONN_CALC:
        break;
    case DBCONN_ODB:
        bStore = false;
        break;
    case DBCONN_FLAT:
    case DBCONN_DBASE:
        //set the filter to the file name without extension
        aFilters[0] = aURL.getBase();
        aTableFilterAny <<= aFilters;
        break;
    case DBCONN_MSJET:
    case DBCONN_MSACE:
        aSuppressVersionsAny <<= uno::makeAny(true);
        break;
    }

    try
    {
        uno::Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
        uno::Reference<sdb::XDatabaseContext> xDBContext = sdb::DatabaseContext::create(xContext);

        OUString sNewName = INetURLObject::decode( aURL.getName(),
                                                 INetURLObject::DECODE_UNAMBIGUOUS );
        sal_Int32 nExtLen = aURL.GetExtension().getLength();
        sNewName = sNewName.replaceAt( sNewName.getLength() - nExtLen - 1, nExtLen + 1, "" );
        if (pPrefix)
            sNewName = *pPrefix + sNewName;

        //find a unique name if sNewName already exists
        sFind = sNewName;
        sal_Int32 nIndex = 0;
        while(xDBContext->hasByName(sFind))
        {
            sFind = sNewName;
            sFind += OUString::number(++nIndex);
        }

        uno::Reference<uno::XInterface> xNewInstance;
        if(!bStore)
        {
            //odb-file
            uno::Any aDataSource = xDBContext->getByName(aURL.GetMainURL(INetURLObject::NO_DECODE));
            aDataSource >>= xNewInstance;
        }
        else
        {
            xNewInstance = xDBContext->createInstance();
            uno::Reference<beans::XPropertySet> xDataProperties(xNewInstance, uno::UNO_QUERY);

            if(aURLAny.hasValue())
                xDataProperties->setPropertyValue("URL", aURLAny);
            if(aTableFilterAny.hasValue())
                xDataProperties->setPropertyValue("TableFilter", aTableFilterAny);
            if(aSuppressVersionsAny.hasValue())
                xDataProperties->setPropertyValue("SuppressVersionColumns", aSuppressVersionsAny);
            if(aInfoAny.hasValue())
                xDataProperties->setPropertyValue("Info", aInfoAny);

            if( DBCONN_FLAT == type && pSettings )
            {
                    uno::Any aSettings = xDataProperties->getPropertyValue( "Settings" );
                    uno::Reference < beans::XPropertySet > xDSSettings;
                    aSettings >>= xDSSettings;
                    ::comphelper::copyProperties( *pSettings, xDSSettings );
                    xDSSettings->setPropertyValue( "Extension", uno::makeAny( sExt ));
            }

            uno::Reference<sdb::XDocumentDataSource> xDS(xNewInstance, uno::UNO_QUERY_THROW);
            uno::Reference<frame::XStorable> xStore(xDS->getDatabaseDocument(), uno::UNO_QUERY_THROW);
            OUString sOutputExt = ".odb";
            OUString aOwnURL = lcl_getOwnURL(pDocShell);
            if (aOwnURL.isEmpty())
            {
                // Cannot embed, as embedded data source would need the URL of the parent document.
                OUString sHomePath(SvtPathOptions().GetWorkPath());
                utl::TempFile aTempFile(sNewName, true, &sOutputExt, pDestDir ? pDestDir : &sHomePath);
                aTempFile.EnableKillingFile();
                OUString sTmpName = aTempFile.GetURL();
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
        xDBContext->registerObject( sFind, xNewInstance );
    }
    catch(const uno::Exception&)
    {
        sFind.clear();
    }
    return sFind;
}

void SwDBManager::StoreEmbeddedDataSource(const uno::Reference<frame::XStorable>& xStorable,
                                          const uno::Reference<embed::XStorage>& xStorage,
                                          const OUString& rStreamRelPath,
                                          const OUString& rOwnURL)
{
    // Construct vnd.sun.star.pkg:// URL for later loading, and TargetStorage/StreamRelPath for storing.
    OUString sTmpName = "vnd.sun.star.pkg://";
    sTmpName += INetURLObject::encode(rOwnURL, INetURLObject::PART_AUTHORITY, INetURLObject::ENCODE_ALL);
    sTmpName += "/" + rStreamRelPath;

    uno::Sequence<beans::PropertyValue> aSequence = comphelper::InitPropertySequence(
    {
        {"TargetStorage", uno::makeAny(xStorage)},
        {"StreamRelPath", uno::makeAny(rStreamRelPath)},
        {"BaseURI", uno::makeAny(rOwnURL)}
    });
    xStorable->storeAsURL(sTmpName, aSequence);
}

OUString SwDBManager::LoadAndRegisterDataSource(const OUString &rURI, const OUString *pPrefix, const OUString *pDestDir,
                                                const uno::Reference< beans::XPropertySet > *pSettings)
{
    uno::Any aURLAny;
    DBConnURITypes type = GetDBunoURI( rURI, aURLAny );
    return LoadAndRegisterDataSource( type, aURLAny, pSettings, rURI, pPrefix, pDestDir );
}

void SwDBManager::RevokeDataSource(const OUString& rName)
{
    uno::Reference<sdb::XDatabaseContext> xDatabaseContext = sdb::DatabaseContext::create(comphelper::getProcessComponentContext());
    if (xDatabaseContext->hasByName(rName))
        xDatabaseContext->revokeObject(rName);
}

void SwDBManager::LoadAndRegisterEmbeddedDataSource(const SwDBData& rData, const SwDocShell& rDocShell)
{
    uno::Reference<sdb::XDatabaseContext> xDatabaseContext = sdb::DatabaseContext::create(comphelper::getProcessComponentContext());

    OUString sDataSource = rData.sDataSource;

    // Fallback, just in case the document would contain an embedded data source, but no DB fields.
    if (sDataSource.isEmpty())
        sDataSource = "EmbeddedDatabase";

    SwDBManager::RevokeDataSource(rData.sDataSource);

    // Encode the stream name and the real path into a single URL.
    const INetURLObject& rURLObject = rDocShell.GetMedium()->GetURLObject();
    OUString aURL = "vnd.sun.star.pkg://";
    aURL += INetURLObject::encode(rURLObject.GetMainURL(INetURLObject::DECODE_WITH_CHARSET), INetURLObject::PART_AUTHORITY, INetURLObject::ENCODE_ALL);
    aURL += "/" + INetURLObject::encode(m_sEmbeddedName, INetURLObject::PART_FPATH, INetURLObject::ENCODE_ALL);

    uno::Reference<uno::XInterface> xDataSource(xDatabaseContext->getByName(aURL), uno::UNO_QUERY);
    xDatabaseContext->registerObject(rData.sDataSource, xDataSource);
}

void SwDBManager::ExecuteFormLetter( SwWrtShell& rSh,
                        const uno::Sequence<beans::PropertyValue>& rProperties,
                        bool bWithDataSourceBrowser)
{
    //prevent second call
    if(pImpl->pMergeDialog)
        return ;
    OUString sDataSource, sDataTableOrQuery;
    uno::Sequence<uno::Any> aSelection;

    sal_Int32 nCmdType = sdb::CommandType::TABLE;
    uno::Reference< sdbc::XConnection> xConnection;

    svx::ODataAccessDescriptor aDescriptor(rProperties);
    sDataSource = aDescriptor.getDataSource();
    OSL_VERIFY(aDescriptor[svx::daCommand]      >>= sDataTableOrQuery);
    OSL_VERIFY(aDescriptor[svx::daCommandType]  >>= nCmdType);

    if ( aDescriptor.has(svx::daSelection) )
        aDescriptor[svx::daSelection] >>= aSelection;
    if ( aDescriptor.has(svx::daConnection) )
        aDescriptor[svx::daConnection] >>= xConnection;

    if(sDataSource.isEmpty() || sDataTableOrQuery.isEmpty())
    {
        OSL_FAIL("PropertyValues missing or unset");
        return;
    }

    //always create a connection for the dialog and dispose it after the dialog has been closed
    SwDSParam* pFound = 0;
    if(!xConnection.is())
    {
        xConnection = SwDBManager::RegisterConnection(sDataSource);
        pFound = FindDSConnection(sDataSource, true);
    }
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialog creation failed!");
    pImpl->pMergeDialog = pFact->CreateMailMergeDlg( DLG_MAILMERGE,
                                                        &rSh.GetView().GetViewFrame()->GetWindow(), rSh,
                                                        sDataSource,
                                                        sDataTableOrQuery,
                                                        nCmdType,
                                                        xConnection,
                                                        bWithDataSourceBrowser ? 0 : &aSelection);
    OSL_ENSURE(pImpl->pMergeDialog, "Dialog creation failed!");
    if(pImpl->pMergeDialog->Execute() == RET_OK)
    {
        aDescriptor[svx::daSelection] <<= pImpl->pMergeDialog->GetSelection();

        uno::Reference<sdbc::XResultSet> xResSet = pImpl->pMergeDialog->GetResultSet();
        if(xResSet.is())
            aDescriptor[svx::daCursor] <<= xResSet;

        // SfxObjectShellRef is ok, since there should be no control over the document lifetime here
        SfxObjectShellRef xDocShell = rSh.GetView().GetViewFrame()->GetObjectShell();
        SfxGetpApp()->NotifyEvent(SfxEventHint(SW_EVENT_MAIL_MERGE, SwDocShell::GetEventName(STR_SW_EVENT_MAIL_MERGE), xDocShell));
        {
            //copy rSh to aTempFile
            OUString sTempURL;
            const SfxFilter *pSfxFlt = SwIoSystem::GetFilterOfFormat(
                        OUString(FILTER_XML),
                        SwDocShell::Factory().GetFilterContainer() );
            try
            {

                uno::Sequence< beans::PropertyValue > aValues(1);
                beans::PropertyValue* pValues = aValues.getArray();
                pValues[0].Name = "FilterName";
                pValues[0].Value <<= OUString(pSfxFlt->GetFilterName());
                uno::Reference< frame::XStorable > xStore( xDocShell->GetModel(), uno::UNO_QUERY);
                sTempURL = URIHelper::SmartRel2Abs( INetURLObject(), utl::TempFile::CreateTempName() );
                xStore->storeToURL( sTempURL, aValues );
            }
            catch(const uno::Exception&)
            {
            }
            if( xDocShell->GetError() )
            {
                // error message ??
                ErrorHandler::HandleError( xDocShell->GetError() );
            }
            else
            {
                // the shell will be explicitly closed, but it is more safe to use SfxObjectShellLock here
                // especially for the case that the loading has failed
                SfxObjectShellLock xWorkDocSh( new SwDocShell( SfxObjectCreateMode::INTERNAL ));
                SfxMedium* pWorkMed = new SfxMedium( sTempURL, STREAM_STD_READ );
                pWorkMed->SetFilter( pSfxFlt );
                if( xWorkDocSh->DoLoad(pWorkMed) )
                {
                    SfxViewFrame *pFrame = SfxViewFrame::LoadHiddenDocument( *xWorkDocSh, 0 );
                    SwView *pView = static_cast<SwView*>( pFrame->GetViewShell() );
                    pView->AttrChangedNotify( &pView->GetWrtShell() );// in order for SelectShell to be called
                    //set the current DBManager
                    SwDoc* pWorkDoc = pView->GetWrtShell().GetDoc();
                    SwDBManager* pWorkDBManager = pWorkDoc->GetDBManager();
                    pWorkDoc->SetDBManager( this );

                    SwMergeDescriptor aMergeDesc( pImpl->pMergeDialog->GetMergeType(), pView->GetWrtShell(), aDescriptor );
                    aMergeDesc.sSaveToFilter = pImpl->pMergeDialog->GetSaveFilter();
                    aMergeDesc.bCreateSingleFile = pImpl->pMergeDialog->IsSaveSingleDoc() && pImpl->pMergeDialog->GetMergeType() != DBMGR_MERGE_PRINTER;
                    aMergeDesc.bSubjectIsFilename = aMergeDesc.bCreateSingleFile;
                    if( !aMergeDesc.bCreateSingleFile && pImpl->pMergeDialog->IsGenerateFromDataBase() )
                    {
                        aMergeDesc.sAddressFromColumn = pImpl->pMergeDialog->GetColumnName();
                        aMergeDesc.sSubject = pImpl->pMergeDialog->GetPath();
                    }

                    MergeNew(aMergeDesc);

                    pWorkDoc->SetDBManager( pWorkDBManager );
                    //close the temporary file
                    uno::Reference< util::XCloseable > xClose( xWorkDocSh->GetModel(), uno::UNO_QUERY );
                    if (xClose.is())
                    {
                        try
                        {
                            //! 'sal_True' -> transfer ownership to vetoing object if vetoed!
                            //! I.e. now that object is responsible for closing the model and doc shell.
                            xClose->close( sal_True );
                        }
                        catch (const uno::Exception&)
                        {
                        }
                    }
                }
            }
            //remove the temporary file
            SWUnoHelper::UCB_DeleteFile( sTempURL );
        }
        SfxGetpApp()->NotifyEvent(SfxEventHint(SW_EVENT_MAIL_MERGE_END, SwDocShell::GetEventName(STR_SW_EVENT_MAIL_MERGE_END), rSh.GetView().GetViewFrame()->GetObjectShell()));

        // reset the cursor inside
        xResSet = NULL;
        aDescriptor[svx::daCursor] <<= xResSet;
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
    DELETEZ(pImpl->pMergeDialog);
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
        if ( pValues[nPos].Name == cDataSourceName )
            pValues[nPos].Value >>= sDataSource;
        else if ( pValues[nPos].Name == cCommand )
            pValues[nPos].Value >>= sDataTableOrQuery;
        else if ( pValues[nPos].Name == cCursor )
            pValues[nPos].Value >>= xResSet;
        else if ( pValues[nPos].Name == cSelection )
            pValues[nPos].Value >>= aSelection;
        else if ( pValues[nPos].Name == cCommandType )
            pValues[nPos].Value >>= nCmdType;
        else if ( pValues[nPos].Name == cActiveConnection )
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
        xSource = uno::Reference<sdbc::XDataSource>(xChild->getParent(), uno::UNO_QUERY);
    if(!xSource.is())
        xSource = dbtools::getDataSource(sDataSource, xContext);
    uno::Reference< sdbcx::XColumnsSupplier > xColSupp( xResSet, uno::UNO_QUERY );
    SwDBData aDBData;
    aDBData.sDataSource = sDataSource;
    aDBData.sCommand = sDataTableOrQuery;
    aDBData.nCommandType = nCmdType;

    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

    std::unique_ptr<AbstractSwInsertDBColAutoPilot> pDlg(pFact->CreateSwInsertDBColAutoPilot( rSh.GetView(),
                                                                                xSource,
                                                                                xColSupp,
                                                                                aDBData ));
    OSL_ENSURE(pDlg, "Dialog creation failed!");
    if( RET_OK == pDlg->Execute() )
    {
        OUString sDummy;
        if(!xConnection.is())
            xConnection = xSource->getConnection(sDummy, sDummy);
        try
        {
            pDlg->DataToDoc( aSelection , xSource, xConnection, xResSet);
        }
        catch (const uno::Exception& e)
        {
            SAL_WARN("sw.mailmerge", "exception caught: " << e.Message);
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
            xSource = uno::Reference<sdbc::XDataSource>(xChild->getParent(), uno::UNO_QUERY);
        if ( !xSource.is() )
            xSource = dbtools::getDataSource(_sDataSourceName, ::comphelper::getProcessComponentContext());
    }
    catch (const uno::Exception& e)
    {
        SAL_WARN("sw.mailmerge", "exception caught in getDataSourceAsParent(): " << e.Message);
    }
    return xSource;
}

uno::Reference<sdbc::XResultSet> SwDBManager::createCursor(const OUString& _sDataSourceName,
                                       const OUString& _sCommand,
                                       sal_Int32 _nCommandType,
                                       const uno::Reference<sdbc::XConnection>& _xConnection
                                      )
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
                    uno::Reference< task::XInteractionHandler > xHandler( task::InteractionHandler::createWithParent(comphelper::getComponentContext(xMgr), 0), uno::UNO_QUERY_THROW );
                    xRowSet->executeWithCompletion(xHandler);
                }
                xResultSet = uno::Reference<sdbc::XResultSet>(xRowSet, uno::UNO_QUERY);
            }
        }
    }
    catch (const uno::Exception& e)
    {
        SAL_WARN("sw.mailmerge", "Caught exception while creating a new RowSet: " << e.Message);
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
        pImpl->m_xDataSourceRemovedListener = new SwDataSourceRemovedListener(*this);
}

OUString SwDBManager::getEmbeddedName() const
{
    return m_sEmbeddedName;
}

SwDoc* SwDBManager::getDoc() const
{
    return m_pDoc;
}

void SwDBManager::releaseRevokeListener()
{
    pImpl->m_xDataSourceRemovedListener->Dispose();
    pImpl->m_xDataSourceRemovedListener.clear();
}

SwConnectionDisposedListener_Impl::SwConnectionDisposedListener_Impl(SwDBManager& rManager)
    : m_pDBManager(&rManager)
{
}

SwConnectionDisposedListener_Impl::~SwConnectionDisposedListener_Impl()
{
}

void SwConnectionDisposedListener_Impl::disposing( const lang::EventObject& rSource )
        throw (uno::RuntimeException, std::exception)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
