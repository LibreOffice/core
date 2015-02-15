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

#include <stdio.h>
#include <unotxdoc.hxx>
#include <com/sun/star/text/NotePrintMode.hpp>
#include <sfx2/app.hxx>
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
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <sfx2/fcontnr.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/viewfrm.hxx>
#include <dbconfig.hxx>
#include <swdbtoolsclient.hxx>
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
#include <comphelper/componentcontext.hxx>
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

#include <unomailmerge.hxx>
#include <sfx2/event.hxx>
#include <vcl/msgbox.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <osl/mutex.hxx>
#include <rtl/textenc.h>
#include <ndindex.hxx>
#include <pam.hxx>
#include <swcrsr.hxx>
#include <swevent.hxx>
#include <osl/file.hxx>
#include <swabstdlg.hxx>
#include <fmthdft.hxx>
#include <envelp.hrc>
#include <memory>
#include <vector>
#include <unomid.h>
#include <section.hxx>
#include <rootfrm.hxx>
#include <fmtpdsc.hxx>
#include <ndtxt.hxx>
#include <calc.hxx>
#include <dbfld.hxx>

using namespace ::osl;
using namespace ::svx;
using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::ui::dialogs;

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

// -----------------------------------------------------------------------------
// Use nameless namespace to avoid to rubbish the global namespace
// -----------------------------------------------------------------------------
namespace
{

bool lcl_getCountFromResultSet( sal_Int32& rCount, const uno::Reference<XResultSet>& xResultSet )
{
    uno::Reference<XPropertySet> xPrSet(xResultSet, UNO_QUERY);
    if(xPrSet.is())
    {
        try
        {
            sal_Bool bFinal = sal_False;
            Any aFinal = xPrSet->getPropertyValue("IsRowCountFinal");
            aFinal >>= bFinal;
            if(!bFinal)
            {
                xResultSet->last();
                xResultSet->first();
            }
            Any aCount = xPrSet->getPropertyValue("RowCount");
            if( aCount >>= rCount )
                return true;
        }
        catch(const Exception&)
        {
        }
    }
    return false;
}
}

class SwConnectionDisposedListener_Impl : public cppu::WeakImplHelper1
< lang::XEventListener >
{
    SwNewDBMgr&     rDBMgr;

    virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException);
public:
    SwConnectionDisposedListener_Impl(SwNewDBMgr& rMgr);
    ~SwConnectionDisposedListener_Impl();

};

struct SwNewDBMgr_Impl
{
    SwDSParam*          pMergeData;
    AbstractMailMergeDlg*     pMergeDialog;
    uno::Reference<lang::XEventListener> xDisposeListener;

    SwNewDBMgr_Impl(SwNewDBMgr& rDBMgr)
       :pMergeData(0)
       ,pMergeDialog(0)
       ,xDisposeListener(new SwConnectionDisposedListener_Impl(rDBMgr))
        {}
};

static void lcl_InitNumberFormatter(SwDSParam& rParam, uno::Reference<XDataSource> xSource)
{
    uno::Reference<XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    rParam.xFormatter = uno::Reference<util::XNumberFormatter>(util::NumberFormatter::create(xContext), UNO_QUERY);
    if(!xSource.is())
        xSource = SwNewDBMgr::getDataSourceAsParent(rParam.xConnection, rParam.sDataSource);

    uno::Reference<XPropertySet> xSourceProps(xSource, UNO_QUERY);
    if(xSourceProps.is())
    {
        Any aFormats = xSourceProps->getPropertyValue("NumberFormatsSupplier");
        if(aFormats.hasValue())
        {
            uno::Reference<XNumberFormatsSupplier> xSuppl;
            aFormats >>= xSuppl;
            if(xSuppl.is())
            {
                uno::Reference< XPropertySet > xSettings = xSuppl->getNumberFormatSettings();
                Any aNull = xSettings->getPropertyValue("NullDate");
                aNull >>= rParam.aNullDate;
                if(rParam.xFormatter.is())
                    rParam.xFormatter->attachNumberFormatsSupplier(xSuppl);
            }
        }
    }
}

static sal_Bool lcl_MoveAbsolute(SwDSParam* pParam, long nAbsPos)
{
    sal_Bool bRet = sal_False;
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
    catch(const Exception&)
    {
    }
    return bRet;
}

static void lcl_GetColumnCnt(SwDSParam *pParam,
                             const uno::Reference< XPropertySet > &rColumnProps,
                             long nLanguage, OUString &rResult, double* pNumber)
{
    SwDBFormatData aFormatData;
    if(!pParam->xFormatter.is())
    {
        uno::Reference<XDataSource> xSource = SwNewDBMgr::getDataSourceAsParent(
                                    pParam->xConnection,pParam->sDataSource);
        lcl_InitNumberFormatter(*pParam, xSource );
    }
    aFormatData.aNullDate = pParam->aNullDate;
    aFormatData.xFormatter = pParam->xFormatter;

    aFormatData.aLocale = LanguageTag( (LanguageType)nLanguage ).getLocale();

    rResult = SwNewDBMgr::GetDBField( rColumnProps, aFormatData, pNumber);
}

static sal_Bool lcl_GetColumnCnt(SwDSParam* pParam, const String& rColumnName,
                             long nLanguage, OUString& rResult, double* pNumber)
{
    uno::Reference< XColumnsSupplier > xColsSupp( pParam->xResultSet, UNO_QUERY );
    uno::Reference<XNameAccess> xCols;
    try
    {
        xCols = xColsSupp->getColumns();
    }
    catch(const lang::DisposedException&)
    {
    }
    if(!xCols.is() || !xCols->hasByName(rColumnName))
        return sal_False;
    Any aCol = xCols->getByName(rColumnName);
    uno::Reference< XPropertySet > xColumnProps;
    aCol >>= xColumnProps;
    lcl_GetColumnCnt( pParam, xColumnProps, nLanguage, rResult, pNumber );
    return sal_True;
};

/*--------------------------------------------------------------------
    Description: import data
 --------------------------------------------------------------------*/
sal_Bool SwNewDBMgr::MergeNew( const SwMergeDescriptor& rMergeDesc )
{
    OSL_ENSURE(!bInMerge && !pImpl->pMergeData, "merge already activated!");

    SwDBData aData;
    aData.nCommandType = CommandType::TABLE;
    uno::Reference<XResultSet>  xResSet;
    Sequence<Any> aSelection;
    uno::Reference< XConnection> xConnection;

    aData.sDataSource = rMergeDesc.rDescriptor.getDataSource();
    rMergeDesc.rDescriptor[daCommand]      >>= aData.sCommand;
    rMergeDesc.rDescriptor[daCommandType]  >>= aData.nCommandType;

    if ( rMergeDesc.rDescriptor.has(daCursor) )
        rMergeDesc.rDescriptor[daCursor] >>= xResSet;
    if ( rMergeDesc.rDescriptor.has(daSelection) )
        rMergeDesc.rDescriptor[daSelection] >>= aSelection;
    if ( rMergeDesc.rDescriptor.has(daConnection) )
        rMergeDesc.rDescriptor[daConnection] >>= xConnection;

    if(aData.sDataSource.isEmpty() || aData.sCommand.isEmpty() || !xResSet.is())
    {
        return sal_False;
    }

    pImpl->pMergeData = new SwDSParam(aData, xResSet, aSelection);
    SwDSParam*  pTemp = FindDSData(aData, sal_False);
    if(pTemp)
        *pTemp = *pImpl->pMergeData;
    else
    {
        // calls from the calculator may have added a connection with an invalid commandtype
        //"real" data base connections added here have to re-use the already available
        //DSData and set the correct CommandType
        SwDBData aTempData(aData);
        aData.nCommandType = -1;
        pTemp = FindDSData(aData, sal_False);
        if(pTemp)
            *pTemp = *pImpl->pMergeData;
        else
        {
            SwDSParam* pInsert = new SwDSParam(*pImpl->pMergeData);
            aDataSourceParams.push_back(pInsert);
            try
            {
                uno::Reference<XComponent> xComponent(pInsert->xConnection, UNO_QUERY);
                if(xComponent.is())
                    xComponent->addEventListener(pImpl->xDisposeListener);
            }
            catch(const Exception&)
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
                pImpl->pMergeData->bEndOfDB = sal_True;
        }
        else
        {
            pImpl->pMergeData->bEndOfDB = !pImpl->pMergeData->xResultSet->first();
            pImpl->pMergeData->CheckEndOfDB();
        }
    }
    catch(const Exception&)
    {
        pImpl->pMergeData->bEndOfDB = sal_True;
        pImpl->pMergeData->CheckEndOfDB();
        OSL_FAIL("exception in MergeNew()");
    }

    uno::Reference<XDataSource> xSource = SwNewDBMgr::getDataSourceAsParent(xConnection,aData.sDataSource);

    lcl_InitNumberFormatter(*pImpl->pMergeData, xSource);

    rMergeDesc.rSh.ChgDBData(aData);
    bInMerge = sal_True;

    if (IsInitDBFields())
    {
        // with database fields without DB-Name, use DB-Name from Doc
        std::vector<String> aDBNames;
        aDBNames.push_back(String());
        SwDBData aInsertData = rMergeDesc.rSh.GetDBData();
        String sDBName = aInsertData.sDataSource;
        sDBName += DB_DELIM;
        sDBName += (String)aInsertData.sCommand;
        sDBName += DB_DELIM;
        sDBName += OUString::number(aInsertData.nCommandType);
        rMergeDesc.rSh.ChangeDBFields( aDBNames, sDBName);
        SetInitDBFields(sal_False);
    }

    sal_Bool bRet = sal_True;
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
                    rMergeDesc);
            break;

        default:
            // insert selected entries
            // (was: InsertRecord)
            ImportFromConnection(&rMergeDesc.rSh);
            break;
    }

    DELETEZ( pImpl->pMergeData );

    // Recalculate original section visibility states, as field changes aren't
    // tracked (not undo-able).  Has to be done, after pImpl->pMergeData is
    //  gone, otherwise merge data is used for calculation!
    rMergeDesc.rSh.ViewShell::UpdateFlds();

    bInMerge = sal_False;

    return bRet;
}

/*--------------------------------------------------------------------
    Description: import data
 --------------------------------------------------------------------*/
sal_Bool SwNewDBMgr::Merge(SwWrtShell* pSh)
{
    pSh->StartAllAction();

    pSh->ViewShell::UpdateFlds(sal_True);
    pSh->SetModified();

    pSh->EndAllAction();

    return sal_True;
}

void SwNewDBMgr::ImportFromConnection(  SwWrtShell* pSh )
{
    if(pImpl->pMergeData && !pImpl->pMergeData->bEndOfDB)
    {
        {
            pSh->StartAllAction();
            pSh->StartUndo(UNDO_EMPTY);
            sal_Bool bGroupUndo(pSh->DoesGroupUndo());
            pSh->DoGroupUndo(sal_False);

            if( pSh->HasSelection() )
                pSh->DelRight();

            SwWait *pWait = 0;

            {
                sal_uLong i = 0;
                do {

                    ImportDBEntry(pSh);
                    if( 10 == ++i )
                        pWait = new SwWait( *pSh->GetView().GetDocShell(), sal_True);

                } while(ToNextMergeRecord());
            }

            pSh->DoGroupUndo(bGroupUndo);
            pSh->EndUndo(UNDO_EMPTY);
            pSh->EndAllAction();
            delete pWait;
        }
    }
}

static String  lcl_FindColumn(const String& sFormatStr,sal_uInt16  &nUsedPos, sal_uInt8 &nSeparator)
{
    String sReturn;
    sal_uInt16 nLen = sFormatStr.Len();
    nSeparator = 0xff;
    while(nUsedPos < nLen && nSeparator == 0xff)
    {
        sal_Unicode cAkt = sFormatStr.GetChar(nUsedPos);
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
                sReturn += cAkt;
        }
        nUsedPos++;

    }
    return sReturn;
}

void SwNewDBMgr::ImportDBEntry(SwWrtShell* pSh)
{
    if(pImpl->pMergeData && !pImpl->pMergeData->bEndOfDB)
    {
        uno::Reference< XColumnsSupplier > xColsSupp( pImpl->pMergeData->xResultSet, UNO_QUERY );
        uno::Reference<XNameAccess> xCols = xColsSupp->getColumns();
        String sFormatStr;
        sal_uInt16 nFmtLen = sFormatStr.Len();
        if( nFmtLen )
        {
            const char cSpace = ' ';
            const char cTab = '\t';
            sal_uInt16 nUsedPos = 0;
            sal_uInt8   nSeparator;
            String sColumn = lcl_FindColumn(sFormatStr, nUsedPos, nSeparator);
            while( sColumn.Len() )
            {
                if(!xCols->hasByName(sColumn))
                    return;
                Any aCol = xCols->getByName(sColumn);
                uno::Reference< XPropertySet > xColumnProp;
                aCol >>= xColumnProp;
                if(xColumnProp.is())
                {
                    SwDBFormatData aDBFormat;
                    String sInsert = GetDBField( xColumnProp,   aDBFormat);
                    if( DB_SEP_SPACE == nSeparator )
                            sInsert += cSpace;
                    else if( DB_SEP_TAB == nSeparator)
                            sInsert += cTab;
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
            String sStr;
            Sequence<OUString> aColNames = xCols->getElementNames();
            const OUString* pColNames = aColNames.getConstArray();
            long nLength = aColNames.getLength();
            for(long i = 0; i < nLength; i++)
            {
                Any aCol = xCols->getByName(pColNames[i]);
                uno::Reference< XPropertySet > xColumnProp;
                aCol >>= xColumnProp;
                SwDBFormatData aDBFormat;
                sStr += GetDBField( xColumnProp, aDBFormat);
                if (i < nLength - 1)
                    sStr += '\t';
            }
            pSh->SwEditShell::Insert2(sStr);
            pSh->SwFEShell::SplitNode();    // line feed
        }
    }
}

/*--------------------------------------------------------------------
    Description: fill Listbox with tablelist
 --------------------------------------------------------------------*/
sal_Bool SwNewDBMgr::GetTableNames(ListBox* pListBox, const String& rDBName)
{
    sal_Bool bRet = sal_False;
    String sOldTableName(pListBox->GetSelectEntry());
    pListBox->Clear();
    SwDSParam* pParam = FindDSConnection(rDBName, sal_False);
    uno::Reference< XConnection> xConnection;
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
        uno::Reference<XTablesSupplier> xTSupplier = uno::Reference<XTablesSupplier>(xConnection, UNO_QUERY);
        if(xTSupplier.is())
        {
            uno::Reference<XNameAccess> xTbls = xTSupplier->getTables();
            Sequence<OUString> aTbls = xTbls->getElementNames();
            const OUString* pTbls = aTbls.getConstArray();
            for(long i = 0; i < aTbls.getLength(); i++)
            {
                sal_uInt16 nEntry = pListBox->InsertEntry(pTbls[i]);
                pListBox->SetEntryData(nEntry, (void*)0);
            }
        }
        uno::Reference<XQueriesSupplier> xQSupplier = uno::Reference<XQueriesSupplier>(xConnection, UNO_QUERY);
        if(xQSupplier.is())
        {
            uno::Reference<XNameAccess> xQueries = xQSupplier->getQueries();
            Sequence<OUString> aQueries = xQueries->getElementNames();
            const OUString* pQueries = aQueries.getConstArray();
            for(long i = 0; i < aQueries.getLength(); i++)
            {
                sal_uInt16 nEntry = pListBox->InsertEntry(pQueries[i]);
                pListBox->SetEntryData(nEntry, (void*)1);
            }
        }
        if (sOldTableName.Len())
            pListBox->SelectEntry(sOldTableName);
        bRet = sal_True;
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Description: fill Listbox with column names of a database
 --------------------------------------------------------------------*/
sal_Bool SwNewDBMgr::GetColumnNames(ListBox* pListBox,
            const String& rDBName, const String& rTableName, sal_Bool bAppend)
{
    if (!bAppend)
        pListBox->Clear();
    SwDBData aData;
    aData.sDataSource = rDBName;
    aData.sCommand = rTableName;
    aData.nCommandType = -1;
    SwDSParam* pParam = FindDSData(aData, sal_False);
    uno::Reference< XConnection> xConnection;
    if(pParam && pParam->xConnection.is())
        xConnection = pParam->xConnection;
    else
    {
        OUString sDBName(rDBName);
        xConnection = RegisterConnection( sDBName );
    }
    uno::Reference< XColumnsSupplier> xColsSupp = SwNewDBMgr::GetColumnSupplier(xConnection, rTableName);
    if(xColsSupp.is())
    {
        uno::Reference<XNameAccess> xCols = xColsSupp->getColumns();
        const Sequence<OUString> aColNames = xCols->getElementNames();
        const OUString* pColNames = aColNames.getConstArray();
        for(int nCol = 0; nCol < aColNames.getLength(); nCol++)
        {
            pListBox->InsertEntry(pColNames[nCol]);
        }
        ::comphelper::disposeComponent( xColsSupp );
    }
    return(sal_True);
}

sal_Bool SwNewDBMgr::GetColumnNames(ListBox* pListBox,
        uno::Reference< XConnection> xConnection,
        const String& rTableName, sal_Bool bAppend)
{
    if (!bAppend)
        pListBox->Clear();
    uno::Reference< XColumnsSupplier> xColsSupp = SwNewDBMgr::GetColumnSupplier(xConnection, rTableName);
    if(xColsSupp.is())
    {
        uno::Reference<XNameAccess> xCols = xColsSupp->getColumns();
        const Sequence<OUString> aColNames = xCols->getElementNames();
        const OUString* pColNames = aColNames.getConstArray();
        for(int nCol = 0; nCol < aColNames.getLength(); nCol++)
        {
            pListBox->InsertEntry(pColNames[nCol]);
        }
        ::comphelper::disposeComponent( xColsSupp );
    }
    return(sal_True);
}

/*--------------------------------------------------------------------
    Description: CTOR
 --------------------------------------------------------------------*/
SwNewDBMgr::SwNewDBMgr() :
            bInitDBFields(sal_False),
            bInMerge(sal_False),
            bMergeSilent(sal_False),
            bMergeLock(sal_False),
            pImpl(new SwNewDBMgr_Impl(*this)),
            pMergeEvtSrc(NULL)
{
}

SwNewDBMgr::~SwNewDBMgr()
{
    for(sal_uInt16 nPos = 0; nPos < aDataSourceParams.size(); nPos++)
    {
        SwDSParam* pParam = &aDataSourceParams[nPos];
        if(pParam->xConnection.is())
        {
            try
            {
                uno::Reference<XComponent> xComp(pParam->xConnection, UNO_QUERY);
                if(xComp.is())
                    xComp->dispose();
            }
            catch(const RuntimeException&)
            {
                //may be disposed already since multiple entries may have used the same connection
            }
        }
    }
    delete pImpl;
}

/*--------------------------------------------------------------------
    Description:    save bulk letters as single documents
 --------------------------------------------------------------------*/
static String lcl_FindUniqueName(SwWrtShell* pTargetShell, const String& rStartingPageDesc, sal_uLong nDocNo )
{
    do
    {
        String sTest = rStartingPageDesc;
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
    String sFollowPageDesc = pFollowPageDesc->GetName();
    if( sFollowPageDesc != rSourcePageDesc.GetName() )
    {
        SwDoc* pTargetDoc = rTargetShell.GetDoc();
        String sNewFollowPageDesc = lcl_FindUniqueName(&rTargetShell, sFollowPageDesc, nDocNo );
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
    sal_uInt16 nSections = rWorkShell.GetSectionFmtCount();
    for( sal_uInt16 nSection = 0; nSection < nSections; ++nSection )
    {
        SwSectionData aSectionData( *rWorkShell.GetSectionFmt( nSection ).GetSection() );
        if( aSectionData.GetType() == FILE_LINK_SECTION )
        {
            aSectionData.SetType( CONTENT_SECTION );
            aSectionData.SetLinkFileName( String() );
            rWorkShell.UpdateSection( nSection, aSectionData );
        }
    }
    rWorkShell.SetLabelDoc( sal_False );
}

// based on SwDoc::ReplaceDocumentProperties
static void lcl_CopyDocumentPorperties(
    const uno::Reference<document::XDocumentProperties> &xSourceDocProps,
    const SfxObjectShell *xTargetDocShell, SwDoc *pTargetDoc)
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        xTargetDocShell->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xTargetDocProps(
        xDPS->getDocumentProperties());
    OSL_ENSURE(xTargetDocProps.is(), "DocumentProperties is null");

    xTargetDocProps->setTitle( xSourceDocProps->getTitle() );
    xTargetDocProps->setSubject( xSourceDocProps->getSubject() );
    xTargetDocProps->setDescription( xSourceDocProps->getDescription() );
    xTargetDocProps->setKeywords( xSourceDocProps->getKeywords() );
    xTargetDocProps->setAuthor( xSourceDocProps->getAuthor() );
    xTargetDocProps->setGenerator( xSourceDocProps->getGenerator() );
    xTargetDocProps->setLanguage( xSourceDocProps->getLanguage() );

    // Manually set the creation date, otherwise author field isn't filled
    // during MM, as it's set when saving the document the first time.
    xTargetDocProps->setCreationDate( xSourceDocProps->getModificationDate() );

    pTargetDoc->ReplaceUserDefinedDocumentProperties( xSourceDocProps );
}

static void lcl_SaveDoc( SfxObjectShell *xTargetDocShell,
                         const char *name, int no = 0 )
{
    String sExt( ".odt" );
    String basename = OUString::createFromAscii( name );
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

sal_Bool SwNewDBMgr::MergeMailFiles(SwWrtShell* pSourceShell,
                                    const SwMergeDescriptor& rMergeDescriptor)
{
    //check if the doc is synchronized and contains at least one linked section
    bool bSynchronizedDoc = pSourceShell->IsLabelDoc() && pSourceShell->GetSectionFmtCount() > 1;
    sal_Bool bNoError = sal_True;
    const bool bEMail = rMergeDescriptor.nMergeType == DBMGR_MERGE_EMAIL;
    const bool bMergeShell = rMergeDescriptor.nMergeType == DBMGR_MERGE_SHELL;

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
            sBodyMimeType = OUString("text/html; charset=");
            sBodyMimeType += OUString::createFromAscii(
                                rtl_getBestMimeCharsetFromTextEncoding( eEncoding ));
            SvxHtmlOptions& rHtmlOptions = SvxHtmlOptions::Get();
            eEncoding = rHtmlOptions.GetTextEncoding();
        }
        else
            sBodyMimeType =
                OUString("text/plain; charset=UTF-8; format=flowed");
    }

    uno::Reference< XPropertySet > xColumnProp;
    {
        bool bColumnName = sEMailAddrFld.Len() > 0;

        if (bColumnName)
        {
            uno::Reference< XColumnsSupplier > xColsSupp( pImpl->pMergeData->xResultSet, UNO_QUERY );
            uno::Reference<XNameAccess> xCols = xColsSupp->getColumns();
            if(!xCols->hasByName(sEMailAddrFld))
                return sal_False;
            Any aCol = xCols->getByName(sEMailAddrFld);
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
            pSfxDispatcher->Execute( pSourceDocSh->HasName() ? SID_SAVEDOC : SID_SAVEASDOC, SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD);
        if( bMergeShell || !pSourceDocSh->IsModified() )
        {
            const SfxFilter* pStoreToFilter = SwIoSystem::GetFileFilter(
                pSourceDocSh->GetMedium()->GetURLObject().GetMainURL(INetURLObject::NO_DECODE));
            SfxFilterContainer* pFilterContainer = SwDocShell::Factory().GetFilterContainer();
            const String* pStoreToFilterOptions = 0;

            // if a save_to filter is set then use it - otherwise use the default
            if( bEMail && !rMergeDescriptor.bSendAsAttachment )
            {
                OUString sExtension = rMergeDescriptor.bSendAsHTML ? OUString("html") : OUString("txt");
                pStoreToFilter = pFilterContainer->GetFilter4Extension(sExtension, SFX_FILTER_EXPORT);
            }
            else if( rMergeDescriptor.sSaveToFilter.Len())
            {
                const SfxFilter* pFilter =
                        pFilterContainer->GetFilter4FilterName( rMergeDescriptor.sSaveToFilter );
                if(pFilter)
                {
                    pStoreToFilter = pFilter;
                    if(rMergeDescriptor.sSaveToFilterOptions.Len())
                        pStoreToFilterOptions = &rMergeDescriptor.sSaveToFilterOptions;
                }
            }
            bCancel = sal_False;

            // in case of creating a single resulting file this has to be created here
            SwWrtShell* pTargetShell = 0;
            SwDoc* pTargetDoc = 0;

            SfxObjectShellRef xTargetDocShell;

            SwView* pTargetView = 0;
            std::auto_ptr< utl::TempFile > aTempFile;
            bool createTempFile = ( rMergeDescriptor.nMergeType == DBMGR_MERGE_EMAIL || rMergeDescriptor.nMergeType == DBMGR_MERGE_FILE );
            String sModifiedStartingPageDesc;
            String sStartingPageDesc;
            sal_uInt16 nStartingPageNo = 0;
            bool bPageStylesWithHeaderFooter = false;

            Window *pSourceWindow = 0;
            CancelableModelessDialog *pProgressDlg = 0;

            if (!IsMergeSilent()) {
                pSourceWindow = &pSourceShell->GetView().GetEditWin();
                if( bMergeShell )
                    pProgressDlg = new CreateMonitor( pSourceWindow );
                else {
                    pProgressDlg = new PrintMonitor( pSourceWindow, PrintMonitor::MONITOR_TYPE_PRINT );
                    ((PrintMonitor*) pProgressDlg)->SetText(pSourceShell->GetView().GetDocShell()->GetTitle(22));
                }
                pProgressDlg->SetCancelHdl( LINK(this, SwNewDBMgr, PrtCancelHdl) );
                pProgressDlg->Show();

                for( sal_uInt16 i = 0; i < 25; i++)
                    Application::Reschedule();
            }

            if(rMergeDescriptor.bCreateSingleFile)
            {
                // create a target docshell to put the merged document into
                xTargetDocShell = new SwDocShell( SFX_CREATE_MODE_STANDARD );
                xTargetDocShell->DoInitNew( 0 );
                if (nMaxDumpDocs)
                    lcl_SaveDoc( xTargetDocShell, "MergeDoc" );
                SfxViewFrame* pTargetFrame = SfxViewFrame::LoadHiddenDocument( *xTargetDocShell, 0 );
                if (bMergeShell && pSourceWindow) {
                    //the created window has to be located at the same position as the source window
                    Window& rTargetWindow = pTargetFrame->GetFrame().GetWindow();
                    rTargetWindow.SetPosPixel(pSourceWindow->GetPosPixel());
                }

                pTargetView = static_cast<SwView*>( pTargetFrame->GetViewShell() );

                //initiate SelectShell() to create sub shells
                pTargetView->AttrChangedNotify( &pTargetView->GetWrtShell() );
                pTargetShell = pTargetView->GetWrtShellPtr();
                pTargetDoc = pTargetShell->GetDoc();
                pTargetDoc->SetInMailMerge(true);

                //copy the styles from the source to the target document
                pTargetView->GetDocShell()->_LoadStyles( *pSourceDocSh, sal_True );

                //determine the page style and number used at the start of the source document
                pSourceShell->SttEndDoc(sal_True);
                nStartingPageNo = pSourceShell->GetVirtPageNum();
                sStartingPageDesc = sModifiedStartingPageDesc = pSourceShell->GetPageDesc(
                                            pSourceShell->GetCurPageDesc()).GetName();

                // #i72517#
                const SwPageDesc* pSourcePageDesc = pSourceShell->FindPageDescByName( sStartingPageDesc );
                const SwFrmFmt& rMaster = pSourcePageDesc->GetMaster();
                bPageStylesWithHeaderFooter = rMaster.GetHeader().IsActive()  ||
                                                rMaster.GetFooter().IsActive();

                // copy compatibility options
                pTargetShell->GetDoc()->ReplaceCompatabilityOptions( *pSourceShell->GetDoc());
                // #72821# copy dynamic defaults
                pTargetShell->GetDoc()->ReplaceDefaults( *pSourceShell->GetDoc());

                lcl_CopyDocumentPorperties( xSourceDocProps, xTargetDocShell, pTargetDoc );
            }

            // Progress, to prohibit KeyInputs
            SfxProgress aProgress(pSourceDocSh, ::aEmptyStr, 1);

            // lock all dispatchers
            SfxViewFrame* pViewFrm = SfxViewFrame::GetFirst(pSourceDocSh);
            while (pViewFrm)
            {
                pViewFrm->GetDispatcher()->Lock(sal_True);
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
                ((CreateMonitor*) pProgressDlg)->SetTotalCount( nDocCount );

            long nStartRow, nEndRow;
            bool bFreezedLayouts = false;
            // collect temporary files
            ::std::vector< String> aFilesToRemove;

            // The SfxObjectShell will be closed explicitly later but it is more safe to use SfxObjectShellLock here
            SfxObjectShellLock xWorkDocSh;
            // a view frame for the document
            SwView* pWorkView = NULL;
            SwDoc* pWorkDoc = NULL;
            SwNewDBMgr* pOldDBManager = NULL;

            do
            {
                nStartRow = pImpl->pMergeData ? pImpl->pMergeData->xResultSet->getRow() : 0;
                {
                    String sPath(sSubject);

                    String sAddress;
                    if( !bEMail && bColumnName )
                    {
                        SwDBFormatData aDBFormat;
                        aDBFormat.xFormatter = pImpl->pMergeData->xFormatter;
                        aDBFormat.aNullDate = pImpl->pMergeData->aNullDate;
                        sAddress = GetDBField( xColumnProp, aDBFormat);
                        if (!sAddress.Len())
                            sAddress = '_';
                        sPath += sAddress;
                    }

                    // create a new temporary file name - only done once in case of bCreateSingleFile
                    if( createTempFile && ( 1 == nDocNo || !rMergeDescriptor.bCreateSingleFile ))
                    {
                        INetURLObject aEntry(sPath);
                        String sLeading;
                        //#i97667# if the name is from a database field then it will be used _as is_
                        if( sAddress.Len() )
                            sLeading = sAddress;
                        else
                            sLeading = aEntry.GetBase();
                        aEntry.removeSegment();
                        sPath = aEntry.GetMainURL( INetURLObject::NO_DECODE );
                        String sExt(comphelper::string::stripStart(pStoreToFilter->GetDefaultExtension(), '*'));
                        aTempFile = std::auto_ptr< utl::TempFile >(
                                new utl::TempFile(sLeading,&sExt,&sPath ));
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
                        boost::scoped_ptr< INetURLObject > aTempFileURL;
                        if( createTempFile )
                            aTempFileURL.reset( new INetURLObject(aTempFile->GetURL()));
                        if (!IsMergeSilent()) {
                            if( bMergeShell )
                                ((CreateMonitor*) pProgressDlg)->SetCurrentPosition( nDocNo );
                            else {
                                PrintMonitor *pPrintMonDlg = (PrintMonitor*) pProgressDlg;
                                pPrintMonDlg->aPrinter.SetText( createTempFile ? aTempFileURL->GetBase() : OUString( pSourceDocSh->GetTitle( 22 )));
                                OUString sStat(SW_RES(STR_STATSTR_LETTER));   // Brief
                                sStat += " ";
                                sStat += OUString::number( nDocNo );
                                pPrintMonDlg->aPrintInfo.SetText( sStat );
                            }
                            pProgressDlg->Update();
                        }

                        // Computation time for the GUI
                        for( sal_uInt16 i = 0; i < 25; i++ )
                            Application::Reschedule();

                        // Create a copy of the source document and work with that one instead of the source.
                        // If we're not in the single file mode (which requires modifying the document for the merging),
                        // it is enough to do this just once.
                        if( 1 == nDocNo || rMergeDescriptor.bCreateSingleFile )
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
                            lcl_CopyDocumentPorperties( xSourceDocProps, xWorkDocSh, pWorkDoc );
                            if ( (nMaxDumpDocs < 0) || (nDocNo <= nMaxDumpDocs) )
                                lcl_SaveDoc( xWorkDocSh, "WorkDoc", nDocNo );
                            pOldDBManager = pWorkDoc->GetNewDBMgr();
                            pWorkDoc->SetNewDBMgr( this );
                            pWorkDoc->EmbedAllLinks();

                            // #i69458# lock fields to prevent access to the result set while calculating layout
                            rWorkShell.LockExpFlds();
                            rWorkShell.CalcLayout();
                            rWorkShell.UnlockExpFlds();
                        }

                            SwWrtShell& rWorkShell = pWorkView->GetWrtShell();
                            SFX_APP()->NotifyEvent(SfxEventHint(SW_EVENT_FIELD_MERGE, SwDocShell::GetEventName(STR_SW_EVENT_FIELD_MERGE), xWorkDocSh));
                            rWorkShell.ViewShell::UpdateFlds();
                            SFX_APP()->NotifyEvent(SfxEventHint(SW_EVENT_FIELD_MERGE_FINISHED, SwDocShell::GetEventName(STR_SW_EVENT_FIELD_MERGE_FINISHED), xWorkDocSh));

                            // launch MailMergeEvent if required
                            const SwXMailMerge *pEvtSrc = GetMailMergeEvtSrc();
                            if(pEvtSrc)
                            {
                                uno::Reference< XInterface > xRef( (XMailMergeBroadcaster *) pEvtSrc );
                                text::MailMergeEvent aEvt( xRef, xWorkDocSh->GetModel() );
                                pEvtSrc->LaunchMailMergeEvent( aEvt );
                            }

                            if(rMergeDescriptor.bCreateSingleFile)
                            {
                                OSL_ENSURE( pTargetShell, "no target shell available!" );
                                // copy created file into the target document
                                pWorkDoc->RemoveInvisibleContent();
                                rWorkShell.ConvertFieldsToText();
                                rWorkShell.SetNumberingRestart();
                                if( bSynchronizedDoc )
                                {
                                    lcl_RemoveSectionLinks( rWorkShell );
                                }

                                // insert the document into the target document

                                //#i72517# put the styles to the target document
                                //if the source uses headers or footers each new copy need to copy a new page styles
                                SwPageDesc* pTargetPageDesc;
                                if(bPageStylesWithHeaderFooter)
                                {
                                    //create a new pagestyle
                                    //copy the pagedesc from the current document to the new document and change the name of the to-be-applied style
                                    String sNewPageDescName = lcl_FindUniqueName(pTargetShell, sStartingPageDesc, nDocNo );
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
                                        IDocumentMarkAccess::UNO_BOOKMARK );
                                    aMergeInfo.nDBRow = nStartRow;
                                    rMergeDescriptor.pMailMergeConfigItem->AddMergedDocument( aMergeInfo );
                                }
                            }
                            else
                            {
                                assert( createTempFile );
                                String sFileURL =  aTempFileURL->GetMainURL( INetURLObject::NO_DECODE );
                                SfxMedium* pDstMed = new SfxMedium(
                                    sFileURL,
                                    STREAM_STD_READWRITE );
                                pDstMed->SetFilter( pStoreToFilter );
                                if(pDstMed->GetItemSet())
                                {
                                    if(pStoreToFilterOptions )
                                        pDstMed->GetItemSet()->Put(SfxStringItem(SID_FILE_FILTEROPTIONS, *pStoreToFilterOptions));
                                    if(rMergeDescriptor.aSaveToFilterData.getLength())
                                        pDstMed->GetItemSet()->Put(SfxUsrAnyItem(SID_FILTER_DATA, makeAny(rMergeDescriptor.aSaveToFilterData)));
                                }

                                //convert fields to text if we are exporting to PDF
                                //this prevents a second merge while updating the fields in SwXTextDocument::getRendererCount()
                                if( pStoreToFilter && pStoreToFilter->GetFilterName().equalsAscii("writer_pdf_Export"))
                                    rWorkShell.ConvertFieldsToText();
                                xWorkDocSh->DoSaveAs(*pDstMed);
                                xWorkDocSh->DoSaveCompleted(pDstMed);
                                if( xWorkDocSh->GetError() )
                                {
                                    // error message ??
                                    ErrorHandler::HandleError( xWorkDocSh->GetError() );
                                    bCancel = sal_True;
                                    bNoError = sal_False;
                                }
                                if( bEMail )
                                {
                                    SwDBFormatData aDBFormat;
                                    aDBFormat.xFormatter = pImpl->pMergeData->xFormatter;
                                    aDBFormat.aNullDate = pImpl->pMergeData->aNullDate;
                                    String sMailAddress = GetDBField( xColumnProp, aDBFormat);
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
                                                SfxMedium aMedium( sFileURL, STREAM_READ);
                                                SvStream* pInStream = aMedium.GetInStream();
                                                OSL_ENSURE(pInStream, "no output file created?");
                                                if(pInStream)
                                                {
                                                    pInStream->SetStreamCharSet( eEncoding );
                                                    OString sLine;
                                                    sal_Bool bDone = pInStream->ReadLine( sLine );
                                                    while ( bDone )
                                                    {
                                                        sBody += OStringToOUString(sLine, eEncoding);
                                                        sBody += OUString('\n');
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
                        if( rMergeDescriptor.bCreateSingleFile )
                        {
                            pWorkDoc->SetNewDBMgr( pOldDBManager );
                            xWorkDocSh->DoClose();
                            xWorkDocSh = NULL;
                        }
                    }
                }
                nDocNo++;
                nEndRow = pImpl->pMergeData ? pImpl->pMergeData->xResultSet->getRow() : 0;

                // Freeze the layouts of the target document after the first inserted
                // sub-document, to get the correct PageDesc.
                if(!bFreezedLayouts && (rMergeDescriptor.bCreateSingleFile))
                {
                    std::set<SwRootFrm*> aAllLayouts = pTargetShell->GetDoc()->GetAllLayouts();
                    std::for_each( aAllLayouts.begin(), aAllLayouts.end(),
                        ::std::bind2nd(::std::mem_fun(&SwRootFrm::FreezeLayout), true));
                    bFreezedLayouts = true;
                }
            } while( !bCancel &&
                (bSynchronizedDoc && (nStartRow != nEndRow)? ExistsNextRecord() : ToNextMergeRecord()));

            if( !rMergeDescriptor.bCreateSingleFile )
            {
                pWorkDoc->SetNewDBMgr( pOldDBManager );
                xWorkDocSh->DoClose();
            }

            if (rMergeDescriptor.bCreateSingleFile)
            {
                // sw::DocumentLayoutManager::CopyLayoutFmt() did not generate
                // unique fly names, do it here once.
                pTargetDoc->SetInMailMerge(false);
                pTargetDoc->SetAllUniqueFlyNames();
            }

            for( sal_uInt16 i = 0; i < 25; i++)
                Application::Reschedule();

            // Unfreeze target document layouts and correct all PageDescs.
            if(rMergeDescriptor.bCreateSingleFile)
            {
                pTargetShell->CalcLayout();
                std::set<SwRootFrm*> aAllLayouts = pTargetShell->GetDoc()->GetAllLayouts();
                std::for_each( aAllLayouts.begin(), aAllLayouts.end(),
                    ::std::bind2nd(::std::mem_fun(&SwRootFrm::FreezeLayout), false));
                std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::mem_fun(&SwRootFrm::AllCheckPageDescs));
            }

            DELETEZ( pProgressDlg );

            // save the single output document
            if (bMergeShell)
            {
                rMergeDescriptor.pMailMergeConfigItem->SetTargetView( pTargetView );
            }
            else if(rMergeDescriptor.bCreateSingleFile)
            {
                if( rMergeDescriptor.nMergeType != DBMGR_MERGE_PRINTER )
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
                            pDstMed->GetItemSet()->Put(SfxUsrAnyItem(SID_FILTER_DATA, makeAny(rMergeDescriptor.aSaveToFilterData)));
                    }

                    xTargetDocShell->DoSaveAs(*pDstMed);
                    xTargetDocShell->DoSaveCompleted(pDstMed);
                    if( xTargetDocShell->GetError() )
                    {
                        // error message ??
                        ErrorHandler::HandleError( xTargetDocShell->GetError() );
                        bNoError = sal_False;
                    }
                }
                else if( pTargetView ) // must be available!
                {
                    //print the target document
        #if OSL_DEBUG_LEVEL > 1
                    sal_Bool  _bVal;
                    sal_Int16 _nVal;
                    OUString  _sVal;
                    const beans::PropertyValue* pDbgPrintOptions = rMergeDescriptor.aPrintOptions.getConstArray();
                    for( sal_Int32 nOption = 0; nOption < rMergeDescriptor.aPrintOptions.getLength(); ++nOption)
                    {
                        OUString aName( pDbgPrintOptions[nOption].Name );
                        uno::Any aVal( pDbgPrintOptions[nOption].Value );
                        aVal >>= _bVal;
                        aVal >>= _nVal;
                        aVal >>= _sVal;
                    }
        #endif
                    // printing should be done synchronously otherwise the document
                    // might already become invalid during the process
                    uno::Sequence< beans::PropertyValue > aOptions( rMergeDescriptor.aPrintOptions );

                    aOptions.realloc( 1 );
                    aOptions[ 0 ].Name = OUString("Wait");
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
            ::std::vector<String>::iterator aFileIter;
            for(aFileIter = aFilesToRemove.begin();
                        aFileIter != aFilesToRemove.end(); ++aFileIter)
                SWUnoHelper::UCB_DeleteFile( *aFileIter );

            // unlock all dispatchers
            pViewFrm = SfxViewFrame::GetFirst(pSourceDocSh);
            while (pViewFrm)
            {
                pViewFrm->GetDispatcher()->Lock(sal_False);
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

void SwNewDBMgr::MergeCancel()
{
    bCancel = sal_True;
}

IMPL_LINK_INLINE_START( SwNewDBMgr, PrtCancelHdl, Button *, pButton )
{
    pButton->GetParent()->Hide();
    MergeCancel();
    return 0;
}
IMPL_LINK_INLINE_END( SwNewDBMgr, PrtCancelHdl, Button *, pButton )

/*--------------------------------------------------------------------
    Description: determine the column's Numberformat and transfer
                    to the forwarded Formatter, if applicable.
  --------------------------------------------------------------------*/
sal_uLong SwNewDBMgr::GetColumnFmt( const String& rDBName,
                                const String& rTableName,
                                const String& rColNm,
                                SvNumberFormatter* pNFmtr,
                                long nLanguage )
{
    sal_uLong nRet = 0;
    if(pNFmtr)
    {
        uno::Reference< XDataSource> xSource;
        uno::Reference< XConnection> xConnection;
        bool bUseMergeData = false;
        uno::Reference< XColumnsSupplier> xColsSupp;
        bool bDisposeConnection = false;
        if(pImpl->pMergeData &&
            pImpl->pMergeData->sDataSource.equals(rDBName) && pImpl->pMergeData->sCommand.equals(rTableName))
        {
            xConnection = pImpl->pMergeData->xConnection;
            xSource = SwNewDBMgr::getDataSourceAsParent(xConnection,rDBName);
            bUseMergeData = true;
            xColsSupp = xColsSupp.query( pImpl->pMergeData->xResultSet );
        }
        if(!xConnection.is())
        {
            SwDBData aData;
            aData.sDataSource = rDBName;
            aData.sCommand = rTableName;
            aData.nCommandType = -1;
            SwDSParam* pParam = FindDSData(aData, sal_False);
            if(pParam && pParam->xConnection.is())
            {
                xConnection = pParam->xConnection;
                xColsSupp = xColsSupp.query( pParam->xResultSet );
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
            xColsSupp = SwNewDBMgr::GetColumnSupplier(xConnection, rTableName);
        }
        if(xColsSupp.is())
        {
            uno::Reference<XNameAccess> xCols;
            try
            {
                xCols = xColsSupp->getColumns();
            }
            catch(const Exception&)
            {
                OSL_FAIL("Exception in getColumns()");
            }
            if(!xCols.is() || !xCols->hasByName(rColNm))
                return nRet;
            Any aCol = xCols->getByName(rColNm);
            uno::Reference< XPropertySet > xColumn;
            aCol >>= xColumn;
            nRet = GetColumnFmt(xSource, xConnection, xColumn, pNFmtr, nLanguage);
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
            nRet = pNFmtr->GetFormatIndex( NF_NUMBER_STANDARD, LANGUAGE_SYSTEM );
    }
    return nRet;
}

sal_uLong SwNewDBMgr::GetColumnFmt( uno::Reference< XDataSource> xSource,
                        uno::Reference< XConnection> xConnection,
                        uno::Reference< XPropertySet> xColumn,
                        SvNumberFormatter* pNFmtr,
                        long nLanguage )
{
    // set the NumberFormat in the doc if applicable
    sal_uLong nRet = 0;

    if(!xSource.is())
    {
        uno::Reference<XChild> xChild(xConnection, UNO_QUERY);
        if ( xChild.is() )
            xSource = uno::Reference<XDataSource>(xChild->getParent(), UNO_QUERY);
    }
    if(xSource.is() && xConnection.is() && xColumn.is() && pNFmtr)
    {
        SvNumberFormatsSupplierObj* pNumFmt = new SvNumberFormatsSupplierObj( pNFmtr );
        uno::Reference< util::XNumberFormatsSupplier >  xDocNumFmtsSupplier = pNumFmt;
        uno::Reference< XNumberFormats > xDocNumberFormats = xDocNumFmtsSupplier->getNumberFormats();
        uno::Reference< XNumberFormatTypes > xDocNumberFormatTypes(xDocNumberFormats, UNO_QUERY);

        com::sun::star::lang::Locale aLocale( LanguageTag( (LanguageType)nLanguage ).getLocale());

        //get the number formatter of the data source
        uno::Reference<XPropertySet> xSourceProps(xSource, UNO_QUERY);
        uno::Reference< XNumberFormats > xNumberFormats;
        if(xSourceProps.is())
        {
            Any aFormats = xSourceProps->getPropertyValue("NumberFormatsSupplier");
            if(aFormats.hasValue())
            {
                uno::Reference<XNumberFormatsSupplier> xSuppl;
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
            Any aFormatKey = xColumn->getPropertyValue("FormatKey");
            if(aFormatKey.hasValue())
            {
                sal_Int32 nFmt = 0;
                aFormatKey >>= nFmt;
                if(xNumberFormats.is())
                {
                    try
                    {
                        uno::Reference<XPropertySet> xNumProps = xNumberFormats->getByKey( nFmt );
                        Any aFormatString = xNumProps->getPropertyValue("FormatString");
                        Any aLocaleVal = xNumProps->getPropertyValue("Locale");
                        OUString sFormat;
                        aFormatString >>= sFormat;
                        lang::Locale aLoc;
                        aLocaleVal >>= aLoc;
                        nFmt = xDocNumberFormats->queryKey( sFormat, aLoc, sal_False );
                        if(NUMBERFORMAT_ENTRY_NOT_FOUND == sal::static_int_cast< sal_uInt32, sal_Int32>(nFmt))
                            nFmt = xDocNumberFormats->addNew( sFormat, aLoc );
                        nRet = nFmt;
                        bUseDefault = false;
                    }
                    catch(const Exception&)
                    {
                        OSL_FAIL("illegal number format key");
                    }
                }
            }
        }
        catch(const Exception&)
        {
            OSL_FAIL("no FormatKey property found");
        }
        if(bUseDefault)
            nRet = SwNewDBMgr::GetDbtoolsClient().getDefaultNumberFormat(xColumn, xDocNumberFormatTypes,  aLocale);
    }
    return nRet;
}

sal_Int32 SwNewDBMgr::GetColumnType( const String& rDBName,
                          const String& rTableName,
                          const String& rColNm )
{
    sal_Int32 nRet = DataType::SQLNULL;
    SwDBData aData;
    aData.sDataSource = rDBName;
    aData.sCommand = rTableName;
    aData.nCommandType = -1;
    SwDSParam* pParam = FindDSData(aData, sal_False);
    uno::Reference< XConnection> xConnection;
    uno::Reference< XColumnsSupplier > xColsSupp;
    bool bDispose = false;
    if(pParam && pParam->xConnection.is())
    {
        xConnection = pParam->xConnection;
        xColsSupp = uno::Reference< XColumnsSupplier >( pParam->xResultSet, UNO_QUERY );
    }
    else
    {
        OUString sDBName(rDBName);
        xConnection = RegisterConnection( sDBName );
    }
    if( !xColsSupp.is() )
    {
        xColsSupp = SwNewDBMgr::GetColumnSupplier(xConnection, rTableName);
        bDispose = true;
    }
    if(xColsSupp.is())
    {
        uno::Reference<XNameAccess> xCols = xColsSupp->getColumns();
        if(xCols->hasByName(rColNm))
        {
            Any aCol = xCols->getByName(rColNm);
            uno::Reference<XPropertySet> xCol;
            aCol >>= xCol;
            Any aType = xCol->getPropertyValue(OUString("Type"));
            aType >>= nRet;
        }
        if(bDispose)
            ::comphelper::disposeComponent( xColsSupp );
    }
    return nRet;
}

uno::Reference< sdbc::XConnection> SwNewDBMgr::GetConnection(const String& rDataSource,
                                                    uno::Reference<XDataSource>& rxSource)
{
    Reference< sdbc::XConnection> xConnection;
    Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    try
    {
        Reference<XCompletedConnection> xComplConnection(SwNewDBMgr::GetDbtoolsClient().getDataSource(rDataSource, xContext),UNO_QUERY);
        if ( xComplConnection.is() )
        {
            rxSource.set(xComplConnection,UNO_QUERY);
            Reference< XInteractionHandler > xHandler( InteractionHandler::createWithParent(xContext, 0), UNO_QUERY_THROW );
            xConnection = xComplConnection->connectWithCompletion( xHandler );
        }
    }
    catch(const Exception&)
    {
    }

    return xConnection;
}

uno::Reference< sdbcx::XColumnsSupplier> SwNewDBMgr::GetColumnSupplier(uno::Reference<sdbc::XConnection> xConnection,
                                    const String& rTableOrQuery,
                                    sal_uInt8   eTableOrQuery)
{
    Reference< sdbcx::XColumnsSupplier> xRet;
    try
    {
        if(eTableOrQuery == SW_DB_SELECT_UNKNOWN)
        {
            //search for a table with the given command name
            Reference<XTablesSupplier> xTSupplier = Reference<XTablesSupplier>(xConnection, UNO_QUERY);
            if(xTSupplier.is())
            {
                Reference<XNameAccess> xTbls = xTSupplier->getTables();
                eTableOrQuery = xTbls->hasByName(rTableOrQuery) ?
                            SW_DB_SELECT_TABLE : SW_DB_SELECT_QUERY;
            }
        }
        sal_Int32 nCommandType = SW_DB_SELECT_TABLE == eTableOrQuery ?
                CommandType::TABLE : CommandType::QUERY;
        Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
        Reference<XRowSet> xRowSet(xMgr->createInstance("com.sun.star.sdb.RowSet"), UNO_QUERY);

        OUString sDataSource;
        Reference<XDataSource> xSource = SwNewDBMgr::getDataSourceAsParent(xConnection, sDataSource);
        Reference<XPropertySet> xSourceProperties(xSource, UNO_QUERY);
        if(xSourceProperties.is())
        {
            xSourceProperties->getPropertyValue("Name") >>= sDataSource;
        }

        Reference<XPropertySet> xRowProperties(xRowSet, UNO_QUERY);
        xRowProperties->setPropertyValue("DataSourceName", makeAny(sDataSource));
        xRowProperties->setPropertyValue("Command", makeAny(OUString(rTableOrQuery)));
        xRowProperties->setPropertyValue("CommandType", makeAny(nCommandType));
        xRowProperties->setPropertyValue("FetchSize", makeAny((sal_Int32)10));
        xRowProperties->setPropertyValue("ActiveConnection", makeAny(xConnection));
        xRowSet->execute();
        xRet = Reference<XColumnsSupplier>( xRowSet, UNO_QUERY );
    }
    catch(const uno::Exception&)
    {
        OSL_FAIL("Exception in SwDBMgr::GetColumnSupplier");
    }

    return xRet;
}

String SwNewDBMgr::GetDBField(uno::Reference<XPropertySet> xColumnProps,
                        const SwDBFormatData& rDBFormatData,
                        double* pNumber)
{
    uno::Reference< XColumn > xColumn(xColumnProps, UNO_QUERY);
    String sRet;
    OSL_ENSURE(xColumn.is(), "SwNewDBMgr::::ImportDBField: illegal arguments");
    if(!xColumn.is())
        return sRet;

    Any aType = xColumnProps->getPropertyValue("Type");
    sal_Int32 eDataType = 0;
    aType >>= eDataType;
    switch(eDataType)
    {
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::LONGVARCHAR:
            try
            {
                sRet = xColumn->getString();
            }
            catch(const SQLException&)
            {
            }
        break;
        case DataType::BIT:
        case DataType::BOOLEAN:
        case DataType::TINYINT:
        case DataType::SMALLINT:
        case DataType::INTEGER:
        case DataType::BIGINT:
        case DataType::FLOAT:
        case DataType::REAL:
        case DataType::DOUBLE:
        case DataType::NUMERIC:
        case DataType::DECIMAL:
        case DataType::DATE:
        case DataType::TIME:
        case DataType::TIMESTAMP:
        {

            try
            {
                SwDbtoolsClient& aClient = SwNewDBMgr::GetDbtoolsClient();
                sRet = aClient.getFormattedValue(
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
            catch(const Exception&)
            {
                OSL_FAIL("exception caught");
            }

        }
        break;
    }

    return sRet;
}

// checks if a desired data source table or query is open
sal_Bool    SwNewDBMgr::IsDataSourceOpen(const String& rDataSource,
            const String& rTableOrQuery, sal_Bool bMergeShell)
{
    if(pImpl->pMergeData)
    {
        return !bMergeLock &&
                ((rDataSource == (String)pImpl->pMergeData->sDataSource &&
                    rTableOrQuery == (String)pImpl->pMergeData->sCommand)
                    ||(!rDataSource.Len() && !rTableOrQuery.Len()))
                    &&
                    pImpl->pMergeData->xResultSet.is();
    }
    else if(!bMergeShell)
    {
        SwDBData aData;
        aData.sDataSource = rDataSource;
        aData.sCommand = rTableOrQuery;
        aData.nCommandType = -1;
        SwDSParam* pFound = FindDSData(aData, sal_False);
        return (pFound && pFound->xResultSet.is());
    }
    return sal_False;
}

// read column data at a specified position
sal_Bool SwNewDBMgr::GetColumnCnt(const String& rSourceName, const String& rTableName,
                            const String& rColumnName, sal_uInt32 nAbsRecordId,
                            long nLanguage,
                            OUString& rResult, double* pNumber)
{
    sal_Bool bRet = sal_False;
    SwDSParam* pFound = 0;
    //check if it's the merge data source
    if(pImpl->pMergeData &&
        rSourceName == (String)pImpl->pMergeData->sDataSource &&
        rTableName == (String)pImpl->pMergeData->sCommand)
    {
        pFound = pImpl->pMergeData;
    }
    else
    {
        SwDBData aData;
        aData.sDataSource = rSourceName;
        aData.sCommand = rTableName;
        aData.nCommandType = -1;
        pFound = FindDSData(aData, sal_False);
    }
    if (!pFound)
        return sal_False;
    //check validity of supplied record Id
    if(pFound->aSelection.getLength())
    {
        //the destination has to be an element of the selection
        const Any* pSelection = pFound->aSelection.getConstArray();
        bool bFound = false;
        for(sal_Int32 nPos = 0; !bFound && nPos < pFound->aSelection.getLength(); nPos++)
        {
            sal_Int32 nSelection = 0;
            pSelection[nPos] >>= nSelection;
            if(nSelection == static_cast<sal_Int32>(nAbsRecordId))
                bFound = true;
        }
        if(!bFound)
            return sal_False;
    }
    if(pFound->xResultSet.is() && !pFound->bAfterSelection)
    {
        sal_Int32 nOldRow = 0;
        try
        {
            nOldRow = pFound->xResultSet->getRow();
        }
        catch(const Exception&)
        {
            return sal_False;
        }
        //position to the desired index
        sal_Bool bMove = sal_True;
        if ( nOldRow != static_cast<sal_Int32>(nAbsRecordId) )
            bMove = lcl_MoveAbsolute(pFound, nAbsRecordId);
        if(bMove)
        {
            bRet = lcl_GetColumnCnt(pFound, rColumnName, nLanguage, rResult, pNumber);
        }
        if ( nOldRow != static_cast<sal_Int32>(nAbsRecordId) )
            bMove = lcl_MoveAbsolute(pFound, nOldRow);
    }
    return bRet;
}


// reads the column data at the current position
sal_Bool    SwNewDBMgr::GetMergeColumnCnt(const String& rColumnName, sal_uInt16 nLanguage,
                                OUString &rResult, double *pNumber)
{
    if(!pImpl->pMergeData || !pImpl->pMergeData->xResultSet.is() || pImpl->pMergeData->bAfterSelection )
    {
        rResult = OUString();
        return sal_False;
    }

    sal_Bool bRet = lcl_GetColumnCnt(pImpl->pMergeData, rColumnName, nLanguage, rResult, pNumber);
    return bRet;
}

sal_Bool SwNewDBMgr::ToNextMergeRecord()
{
    OSL_ENSURE(pImpl->pMergeData && pImpl->pMergeData->xResultSet.is(), "no data source in merge");
    return ToNextRecord(pImpl->pMergeData);
}

sal_Bool SwNewDBMgr::FillCalcWithMergeData( SvNumberFormatter *pDocFormatter,
                                            sal_uInt16 nLanguage, bool asString, SwCalc &rCalc )
{
    if (!(pImpl->pMergeData && pImpl->pMergeData->xResultSet.is()))
        return false;

    uno::Reference< XColumnsSupplier > xColsSupp( pImpl->pMergeData->xResultSet, UNO_QUERY );
    if(xColsSupp.is())
    {
        uno::Reference<XNameAccess> xCols = xColsSupp->getColumns();
        const Sequence<OUString> aColNames = xCols->getElementNames();
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
            sal_Int32 nColumnType = DataType::SQLNULL;
            Any aCol = xCols->getByName( pColNames[nCol] );
            uno::Reference<XPropertySet> xColumnProps;
            aCol >>= xColumnProps;
            Any aType = xColumnProps->getPropertyValue( "Type" );
            aType >>= nColumnType;
            double aNumber = DBL_MAX;

            lcl_GetColumnCnt( pImpl->pMergeData, xColumnProps, nLanguage, aString, &aNumber );

            sal_uInt32 nFmt = GetColumnFmt( pImpl->pMergeData->sDataSource,
                                            pImpl->pMergeData->sCommand,
                                            pColNames[nCol], pDocFormatter, nLanguage );
            // aNumber is overwritten by SwDBField::FormatValue, so store initial status
            bool colIsNumber = aNumber != DBL_MAX;
            bool bValidValue = SwDBField::FormatValue( pDocFormatter, aString, nFmt,
                                                       aNumber, nColumnType, NULL );
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

sal_Bool SwNewDBMgr::ToNextRecord(
    const String& rDataSource, const String& rCommand, sal_Int32 /*nCommandType*/)
{
    SwDSParam* pFound = 0;
    if(pImpl->pMergeData &&
        rDataSource == (String)pImpl->pMergeData->sDataSource &&
        rCommand == (String)pImpl->pMergeData->sCommand)
        pFound = pImpl->pMergeData;
    else
    {
        SwDBData aData;
        aData.sDataSource = rDataSource;
        aData.sCommand = rCommand;
        aData.nCommandType = -1;
        pFound = FindDSData(aData, sal_False);
    }
    return ToNextRecord(pFound);
}

sal_Bool SwNewDBMgr::ToNextRecord(SwDSParam* pParam)
{
    sal_Bool bRet = sal_True;
    if(!pParam || !pParam->xResultSet.is() || pParam->bEndOfDB ||
            (pParam->aSelection.getLength() && pParam->aSelection.getLength() <= pParam->nSelectionIndex))
    {
        if(pParam)
            pParam->CheckEndOfDB();
        return sal_False;
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
                pParam->bEndOfDB = sal_True;
        }
        else
        {
            sal_Int32 nBefore = pParam->xResultSet->getRow();
            pParam->bEndOfDB = !pParam->xResultSet->next();
            if( !pParam->bEndOfDB && nBefore == pParam->xResultSet->getRow())
            {
                //next returned true but it didn't move
                pParam->bEndOfDB = sal_True;
            }

            pParam->CheckEndOfDB();
            bRet = !pParam->bEndOfDB;
            ++pParam->nSelectionIndex;
        }
    }
    catch(const Exception&)
    {
    }
    return bRet;
}

/* ------------------------------------------------------------------------
    synchronized labels contain a next record field at their end
    to assure that the next page can be created in mail merge
    the cursor position must be validated
 ---------------------------------------------------------------------------*/
sal_Bool SwNewDBMgr::ExistsNextRecord() const
{
    return pImpl->pMergeData && !pImpl->pMergeData->bEndOfDB;
}

sal_uInt32  SwNewDBMgr::GetSelectedRecordId()
{
    sal_uInt32  nRet = 0;
    OSL_ENSURE(pImpl->pMergeData && pImpl->pMergeData->xResultSet.is(), "no data source in merge");
    if(!pImpl->pMergeData || !pImpl->pMergeData->xResultSet.is())
        return sal_False;
    try
    {
        nRet = pImpl->pMergeData->xResultSet->getRow();
    }
    catch(const Exception&)
    {
    }
    return nRet;
}

sal_Bool SwNewDBMgr::ToRecordId(sal_Int32 nSet)
{
    OSL_ENSURE(pImpl->pMergeData && pImpl->pMergeData->xResultSet.is(), "no data source in merge");
    if(!pImpl->pMergeData || !pImpl->pMergeData->xResultSet.is()|| nSet < 0)
        return sal_False;
    sal_Bool bRet = sal_False;
    sal_Int32 nAbsPos = nSet;

    if(nAbsPos >= 0)
    {
        bRet = lcl_MoveAbsolute(pImpl->pMergeData, nAbsPos);
        pImpl->pMergeData->bEndOfDB = !bRet;
        pImpl->pMergeData->CheckEndOfDB();
    }
    return bRet;
}

sal_Bool SwNewDBMgr::OpenDataSource(const String& rDataSource, const String& rTableOrQuery,
            sal_Int32 nCommandType, bool bCreate)
{
    SwDBData aData;
    aData.sDataSource = rDataSource;
    aData.sCommand = rTableOrQuery;
    aData.nCommandType = nCommandType;

    SwDSParam* pFound = FindDSData(aData, sal_True);
    uno::Reference< XDataSource> xSource;
    if(pFound->xResultSet.is())
        return sal_True;
    SwDSParam* pParam = FindDSConnection(rDataSource, sal_False);
    uno::Reference< XConnection> xConnection;
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
                        ->supportsResultSetType((sal_Int32)ResultSetType::SCROLL_INSENSITIVE);
            }
            catch(const Exception&)
            {
                // DB driver may not be ODBC 3.0 compliant
                pFound->bScrollable = sal_True;
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
            pFound->bAfterSelection = sal_False;
            pFound->CheckEndOfDB();
            ++pFound->nSelectionIndex;
        }
        catch (const Exception&)
        {
            pFound->xResultSet = 0;
            pFound->xStatement = 0;
            pFound->xConnection = 0;
        }
    }
    return pFound->xResultSet.is();
}

uno::Reference< XConnection> SwNewDBMgr::RegisterConnection(OUString& rDataSource)
{
    SwDSParam* pFound = SwNewDBMgr::FindDSConnection(rDataSource, sal_True);
    uno::Reference< XDataSource> xSource;
    if(!pFound->xConnection.is())
    {
        pFound->xConnection = SwNewDBMgr::GetConnection(rDataSource, xSource );
        try
        {
            uno::Reference<XComponent> xComponent(pFound->xConnection, UNO_QUERY);
            if(xComponent.is())
                xComponent->addEventListener(pImpl->xDisposeListener);
        }
        catch(const Exception&)
        {
        }
    }
    return pFound->xConnection;
}

sal_uInt32      SwNewDBMgr::GetSelectedRecordId(
    const String& rDataSource, const String& rTableOrQuery, sal_Int32 nCommandType)
{
    sal_uInt32 nRet = 0xffffffff;
    //check for merge data source first
    if(pImpl->pMergeData && rDataSource == (String)pImpl->pMergeData->sDataSource &&
                    rTableOrQuery == (String)pImpl->pMergeData->sCommand &&
                    (nCommandType == -1 || nCommandType == pImpl->pMergeData->nCommandType) &&
                    pImpl->pMergeData->xResultSet.is())
        nRet = GetSelectedRecordId();
    else
    {
        SwDBData aData;
        aData.sDataSource = rDataSource;
        aData.sCommand = rTableOrQuery;
        aData.nCommandType = nCommandType;
        SwDSParam* pFound = FindDSData(aData, sal_False);
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
            catch(const Exception&)
            {
            }
        }
    }
    return nRet;
}


// close all data sources - after fields were updated
void    SwNewDBMgr::CloseAll(sal_Bool bIncludingMerge)
{
    //the only thing done here is to reset the selection index
    //all connections stay open
    for(sal_uInt16 nPos = 0; nPos < aDataSourceParams.size(); nPos++)
    {
        SwDSParam* pParam = &aDataSourceParams[nPos];
        if(bIncludingMerge || pParam != pImpl->pMergeData)
        {
            pParam->nSelectionIndex = 0;
            pParam->bAfterSelection = sal_False;
            pParam->bEndOfDB = sal_False;
            try
            {
                if(!bInMerge && pParam->xResultSet.is())
                    pParam->xResultSet->first();
            }
            catch(const Exception&)
            {}
        }
    }
}

SwDSParam* SwNewDBMgr::FindDSData(const SwDBData& rData, sal_Bool bCreate)
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
    for(sal_uInt16 nPos = aDataSourceParams.size(); nPos; nPos--)
    {
        SwDSParam* pParam = &aDataSourceParams[nPos - 1];
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
            aDataSourceParams.push_back(pFound);
            try
            {
                uno::Reference<XComponent> xComponent(pFound->xConnection, UNO_QUERY);
                if(xComponent.is())
                    xComponent->addEventListener(pImpl->xDisposeListener);
            }
            catch(const Exception&)
            {
            }
        }
    }
    return pFound;
}

SwDSParam*  SwNewDBMgr::FindDSConnection(const OUString& rDataSource, sal_Bool bCreate)
{
    //prefer merge data if available
    if(pImpl->pMergeData && rDataSource == pImpl->pMergeData->sDataSource )
    {
         return pImpl->pMergeData;
    }
    SwDSParam* pFound = 0;
    for(sal_uInt16 nPos = 0; nPos < aDataSourceParams.size(); nPos++)
    {
        SwDSParam* pParam = &aDataSourceParams[nPos];
        if(rDataSource == pParam->sDataSource)
        {
            pFound = pParam;
            break;
        }
    }
    if(bCreate && !pFound)
    {
        SwDBData aData;
        aData.sDataSource = rDataSource;
        pFound = new SwDSParam(aData);
        aDataSourceParams.push_back(pFound);
        try
        {
            uno::Reference<XComponent> xComponent(pFound->xConnection, UNO_QUERY);
            if(xComponent.is())
                xComponent->addEventListener(pImpl->xDisposeListener);
        }
        catch(const Exception&)
        {
        }
    }
    return pFound;
}

const SwDBData& SwNewDBMgr::GetAddressDBName()
{
    return SW_MOD()->GetDBConfig()->GetAddressSource();
}

Sequence<OUString> SwNewDBMgr::GetExistingDatabaseNames()
{
    Reference<XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
    Reference<XDatabaseContext> xDBContext = DatabaseContext::create(xContext);
    return xDBContext->getElementNames();
}

String SwNewDBMgr::LoadAndRegisterDataSource()
{
    sfx2::FileDialogHelper aDlgHelper( TemplateDescription::FILEOPEN_SIMPLE, 0 );
    Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();

    String sHomePath(SvtPathOptions().GetWorkPath());
    aDlgHelper.SetDisplayDirectory( sHomePath );

    Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);

    String sFilterAll(SW_RES(STR_FILTER_ALL));
    String sFilterAllData(SW_RES(STR_FILTER_ALL_DATA));
    String sFilterSXB(SW_RES(STR_FILTER_SXB));
    String sFilterSXC(SW_RES(STR_FILTER_SXC));
    String sFilterDBF(SW_RES(STR_FILTER_DBF));
    String sFilterXLS(SW_RES(STR_FILTER_XLS));
    String sFilterTXT(SW_RES(STR_FILTER_TXT));
    String sFilterCSV(SW_RES(STR_FILTER_CSV));
#ifdef WNT
    String sFilterMDB(SW_RES(STR_FILTER_MDB));
    String sFilterACCDB(SW_RES(STR_FILTER_ACCDB));
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
    String sFind;
    if( ERRCODE_NONE == aDlgHelper.Execute() )
    {
        Any aURLAny;
        uno::Reference< beans::XPropertySet > aSettings;
        const OUString aURI( xFP->getFiles().getConstArray()[0] );
        const DBConnURITypes type = GetDBunoURI( aURI, aURLAny );

        if( DBCONN_FLAT == type )
        {
            Reference<XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
            uno::Reference < sdb::XTextConnectionSettings > xSettingsDlg = sdb::TextConnectionSettings::create(xContext);
            if( xSettingsDlg->execute() )
                aSettings.set( uno::Reference < beans::XPropertySet >( xSettingsDlg, uno::UNO_QUERY_THROW ) );
        }
        sFind = LoadAndRegisterDataSource( type, aURLAny, DBCONN_FLAT == type ? &aSettings : 0, aURI );
    }
    return sFind;
}

SwNewDBMgr::DBConnURITypes SwNewDBMgr::GetDBunoURI(const OUString &rURI, Any &aURLAny)
{
    INetURLObject aURL( rURI );
    String sExt( aURL.GetExtension() );
    DBConnURITypes type = DBCONN_UNKNOWN;

    if(sExt == "odb")
    {
        type = DBCONN_ODB;
    }
    else if(sExt.EqualsIgnoreCaseAscii("sxc")
        || sExt.EqualsIgnoreCaseAscii("ods")
            || sExt.EqualsIgnoreCaseAscii("xls"))
    {
        OUString sDBURL("sdbc:calc:");
        sDBURL += aURL.GetMainURL(INetURLObject::NO_DECODE);
        aURLAny <<= sDBURL;
        type = DBCONN_CALC;
    }
    else if(sExt.EqualsIgnoreCaseAscii("dbf"))
    {
        aURL.removeSegment();
        aURL.removeFinalSlash();
        OUString sDBURL("sdbc:dbase:");
        sDBURL += aURL.GetMainURL(INetURLObject::NO_DECODE);
        aURLAny <<= sDBURL;
        type = DBCONN_DBASE;
    }
    else if(sExt.EqualsIgnoreCaseAscii("csv") || sExt.EqualsIgnoreCaseAscii("txt"))
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
    else if(sExt.EqualsIgnoreCaseAscii("mdb"))
    {
        OUString sDBURL("sdbc:ado:access:PROVIDER=Microsoft.Jet.OLEDB.4.0;DATA SOURCE=");
        sDBURL += aURL.PathToFileName();
        aURLAny <<= sDBURL;
        type = DBCONN_MSJET;
    }
    else if(sExt.EqualsIgnoreCaseAscii("accdb"))
    {
        OUString sDBURL("sdbc:ado:PROVIDER=Microsoft.ACE.OLEDB.12.0;DATA SOURCE=");
        sDBURL += aURL.PathToFileName();
        aURLAny <<= sDBURL;
        type = DBCONN_MSACE;
    }
#endif
    return type;
}

OUString SwNewDBMgr::LoadAndRegisterDataSource(const DBConnURITypes type, const Any &aURLAny, const uno::Reference< beans::XPropertySet > *pSettings,
                                               const OUString &rURI, const OUString *pPrefix, const String *pDestDir)
{
        INetURLObject aURL( rURI );
        OUString sExt( aURL.GetExtension() );
        Any aTableFilterAny;
        Any aSuppressVersionsAny;
        Any aInfoAny;
        bool bStore = true;
        OUString sFind;
        Sequence<OUString> aFilters(1);

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
            aSuppressVersionsAny <<= makeAny(true);
            break;
        }

        try
        {
            Reference<XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
            Reference<XDatabaseContext> xDBContext = DatabaseContext::create(xContext);
            Reference<XSingleServiceFactory> xFact( xDBContext, UNO_QUERY);

            String sNewName = INetURLObject::decode( aURL.getName(),
                                                     INET_HEX_ESCAPE,
                                                     INetURLObject::DECODE_UNAMBIGUOUS,
                                                     RTL_TEXTENCODING_UTF8 );
            xub_StrLen nExtLen = static_cast< xub_StrLen >(aURL.GetExtension().getLength());
            sNewName.Erase( sNewName.Len() - nExtLen - 1, nExtLen + 1 );
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

            Reference<XInterface> xNewInstance;
            if(!bStore)
            {
                //odb-file
                Any aDataSource = xDBContext->getByName(aURL.GetMainURL(INetURLObject::NO_DECODE));
                aDataSource >>= xNewInstance;
            }
            else
            {
                xNewInstance = xFact->createInstance();
                Reference<XPropertySet> xDataProperties(xNewInstance, UNO_QUERY);

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
                        xDSSettings->setPropertyValue( "Extension", uno::makeAny( OUString( sExt )));
                }

                Reference<XDocumentDataSource> xDS(xNewInstance, UNO_QUERY_THROW);
                Reference<XStorable> xStore(xDS->getDatabaseDocument(), UNO_QUERY_THROW);
                String sOutputExt = OUString(".odb");
                String sTmpName;
                {
                    String sHomePath(SvtPathOptions().GetWorkPath());
                    utl::TempFile aTempFile(sNewName, &sOutputExt, pDestDir ? pDestDir : &sHomePath);
                    aTempFile.EnableKillingFile(sal_True);
                    sTmpName = aTempFile.GetURL();
                }
                xStore->storeAsURL(sTmpName, Sequence< PropertyValue >());
            }
            Reference<XNamingService> xNaming(xDBContext, UNO_QUERY);
            xNaming->registerObject( sFind, xNewInstance );
        }
        catch(const Exception&)
        {
            sFind = "";
        }
    return sFind;
}

OUString SwNewDBMgr::LoadAndRegisterDataSource(const OUString &rURI, const OUString *pPrefix, const String *pDestDir,
                                               const uno::Reference< beans::XPropertySet > *pSettings)
{
    Any aURLAny;
    DBConnURITypes type = GetDBunoURI( rURI, aURLAny );
    return LoadAndRegisterDataSource( type, aURLAny, pSettings, rURI, pPrefix, pDestDir );
}

void SwNewDBMgr::ExecuteFormLetter( SwWrtShell& rSh,
                        const Sequence<PropertyValue>& rProperties,
                        sal_Bool bWithDataSourceBrowser)
{
    //prevent second call
    if(pImpl->pMergeDialog)
        return ;
    OUString sDataSource, sDataTableOrQuery;
    Sequence<Any> aSelection;

    sal_Int32 nCmdType = CommandType::TABLE;
    uno::Reference< XConnection> xConnection;

    ODataAccessDescriptor aDescriptor(rProperties);
    sDataSource = aDescriptor.getDataSource();
    OSL_VERIFY(aDescriptor[daCommand]      >>= sDataTableOrQuery);
    OSL_VERIFY(aDescriptor[daCommandType]  >>= nCmdType);

    if ( aDescriptor.has(daSelection) )
        aDescriptor[daSelection] >>= aSelection;
    if ( aDescriptor.has(daConnection) )
        aDescriptor[daConnection] >>= xConnection;

    if(sDataSource.isEmpty() || sDataTableOrQuery.isEmpty())
    {
        OSL_FAIL("PropertyValues missing or unset");
        return;
    }

    //always create a connection for the dialog and dispose it after the dialog has been closed
    SwDSParam* pFound = 0;
    if(!xConnection.is())
    {
        xConnection = SwNewDBMgr::RegisterConnection(sDataSource);
        pFound = FindDSConnection(sDataSource, sal_True);
    }
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");
    pImpl->pMergeDialog = pFact->CreateMailMergeDlg( DLG_MAILMERGE,
                                                        &rSh.GetView().GetViewFrame()->GetWindow(), rSh,
                                                        sDataSource,
                                                        sDataTableOrQuery,
                                                        nCmdType,
                                                        xConnection,
                                                        bWithDataSourceBrowser ? 0 : &aSelection);
    OSL_ENSURE(pImpl->pMergeDialog, "Dialogdiet fail!");
    if(pImpl->pMergeDialog->Execute() == RET_OK)
    {
        aDescriptor[daSelection] <<= pImpl->pMergeDialog->GetSelection();

        uno::Reference<XResultSet> xResSet = pImpl->pMergeDialog->GetResultSet();
        if(xResSet.is())
            aDescriptor[daCursor] <<= xResSet;

        // SfxObjectShellRef is ok, since there should be no control over the document lifetime here
        SfxObjectShellRef xDocShell = rSh.GetView().GetViewFrame()->GetObjectShell();
        SFX_APP()->NotifyEvent(SfxEventHint(SW_EVENT_MAIL_MERGE, SwDocShell::GetEventName(STR_SW_EVENT_MAIL_MERGE), xDocShell));
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
                uno::Reference< XStorable > xStore( xDocShell->GetModel(), uno::UNO_QUERY);
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
                SfxObjectShellLock xWorkDocSh( new SwDocShell( SFX_CREATE_MODE_INTERNAL ));
                SfxMedium* pWorkMed = new SfxMedium( sTempURL, STREAM_STD_READ );
                pWorkMed->SetFilter( pSfxFlt );
                if( xWorkDocSh->DoLoad(pWorkMed) )
                {
                    SfxViewFrame *pFrame = SfxViewFrame::LoadHiddenDocument( *xWorkDocSh, 0 );
                    SwView *pView = (SwView*) pFrame->GetViewShell();
                    pView->AttrChangedNotify( &pView->GetWrtShell() );// in order for SelectShell to be called
                    //set the current DBMgr
                    SwDoc* pWorkDoc = pView->GetWrtShell().GetDoc();
                    SwNewDBMgr* pWorkDBMgr = pWorkDoc->GetNewDBMgr();
                    pWorkDoc->SetNewDBMgr( this );

                    SwMergeDescriptor aMergeDesc( pImpl->pMergeDialog->GetMergeType(), pView->GetWrtShell(), aDescriptor );
                    aMergeDesc.sSaveToFilter = pImpl->pMergeDialog->GetSaveFilter();
                    aMergeDesc.bCreateSingleFile = pImpl->pMergeDialog->IsSaveSingleDoc();
                    aMergeDesc.bSubjectIsFilename = aMergeDesc.bCreateSingleFile;
                    if( !aMergeDesc.bCreateSingleFile && pImpl->pMergeDialog->IsGenerateFromDataBase() )
                    {
                        aMergeDesc.sAddressFromColumn = pImpl->pMergeDialog->GetColumnName();
                        aMergeDesc.sSubject = pImpl->pMergeDialog->GetPath();
                    }

                    MergeNew(aMergeDesc);

                    pWorkDoc->SetNewDBMgr( pWorkDBMgr );
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
        SFX_APP()->NotifyEvent(SfxEventHint(SW_EVENT_MAIL_MERGE_END, SwDocShell::GetEventName(STR_SW_EVENT_MAIL_MERGE_END), rSh.GetView().GetViewFrame()->GetObjectShell()));

        // reset the cursor inside
        xResSet = NULL;
        aDescriptor[daCursor] <<= xResSet;
    }
    if(pFound)
    {
        for(sal_uInt16 nPos = 0; nPos < aDataSourceParams.size(); nPos++)
        {
            SwDSParam* pParam = &aDataSourceParams[nPos];
            if(pParam == pFound)
            {
                try
                {
                    uno::Reference<XComponent> xComp(pParam->xConnection, UNO_QUERY);
                    if(xComp.is())
                        xComp->dispose();
                }
                catch(const RuntimeException&)
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

void SwNewDBMgr::InsertText(SwWrtShell& rSh,
                        const Sequence< PropertyValue>& rProperties)
{
    OUString sDataSource, sDataTableOrQuery;
    uno::Reference<XResultSet>  xResSet;
    Sequence<Any> aSelection;
    sal_Int16 nCmdType = CommandType::TABLE;
    const PropertyValue* pValues = rProperties.getConstArray();
    uno::Reference< XConnection> xConnection;
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
    Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference<XDataSource> xSource;
    uno::Reference<XChild> xChild(xConnection, UNO_QUERY);
    if(xChild.is())
        xSource = uno::Reference<XDataSource>(xChild->getParent(), UNO_QUERY);
    if(!xSource.is())
        xSource = SwNewDBMgr::GetDbtoolsClient().getDataSource(sDataSource, xContext);
    uno::Reference< XColumnsSupplier > xColSupp( xResSet, UNO_QUERY );
    SwDBData aDBData;
    aDBData.sDataSource = sDataSource;
    aDBData.sCommand = sDataTableOrQuery;
    aDBData.nCommandType = nCmdType;

    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

    AbstractSwInsertDBColAutoPilot* pDlg = pFact->CreateSwInsertDBColAutoPilot( rSh.GetView(),
                                                                                xSource,
                                                                                xColSupp,
                                                                                aDBData,
                                                                                DLG_AP_INSERT_DB_SEL );
    OSL_ENSURE(pDlg, "Dialogdiet fail!");
    if( RET_OK == pDlg->Execute() )
    {
        OUString sDummy;
        if(!xConnection.is())
            xConnection = xSource->getConnection(sDummy, sDummy);
        try
        {
            pDlg->DataToDoc( aSelection , xSource, xConnection, xResSet);
        }
        catch(const Exception&)
        {
            OSL_FAIL("exception caught");
        }
    }
    delete pDlg;

}

SwDbtoolsClient* SwNewDBMgr::pDbtoolsClient = NULL;

SwDbtoolsClient& SwNewDBMgr::GetDbtoolsClient()
{
    if ( !pDbtoolsClient )
        pDbtoolsClient = new SwDbtoolsClient;
    return *pDbtoolsClient;
}

void SwNewDBMgr::RemoveDbtoolsClient()
{
    delete pDbtoolsClient;
    pDbtoolsClient = 0;
}

uno::Reference<XDataSource> SwNewDBMgr::getDataSourceAsParent(const uno::Reference< XConnection>& _xConnection,const OUString& _sDataSourceName)
{
    uno::Reference<XDataSource> xSource;
    try
    {
        uno::Reference<XChild> xChild(_xConnection, UNO_QUERY);
        if ( xChild.is() )
            xSource = uno::Reference<XDataSource>(xChild->getParent(), UNO_QUERY);
        if ( !xSource.is() )
            xSource = SwNewDBMgr::GetDbtoolsClient().getDataSource(_sDataSourceName, ::comphelper::getProcessComponentContext());
    }
    catch(const Exception&)
    {
        OSL_FAIL("exception in getDataSourceAsParent caught");
    }
    return xSource;
}

uno::Reference<XResultSet> SwNewDBMgr::createCursor(const OUString& _sDataSourceName,
                                       const OUString& _sCommand,
                                       sal_Int32 _nCommandType,
                                       const uno::Reference<XConnection>& _xConnection
                                      )
{
    uno::Reference<XResultSet> xResultSet;
    try
    {
        uno::Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
        if( xMgr.is() )
        {
            uno::Reference<XInterface> xInstance = xMgr->createInstance("com.sun.star.sdb.RowSet");
            uno::Reference<XPropertySet> xRowSetPropSet(xInstance, UNO_QUERY);
            if(xRowSetPropSet.is())
            {
                xRowSetPropSet->setPropertyValue("DataSourceName", makeAny(_sDataSourceName));
                xRowSetPropSet->setPropertyValue("ActiveConnection", makeAny(_xConnection));
                xRowSetPropSet->setPropertyValue("Command", makeAny(_sCommand));
                xRowSetPropSet->setPropertyValue("CommandType", makeAny(_nCommandType));

                uno::Reference< XCompletedExecution > xRowSet(xInstance, UNO_QUERY);

                if ( xRowSet.is() )
                {
                    uno::Reference< XInteractionHandler > xHandler( InteractionHandler::createWithParent(comphelper::getComponentContext(xMgr), 0), UNO_QUERY_THROW );
                    xRowSet->executeWithCompletion(xHandler);
                }
                xResultSet = uno::Reference<XResultSet>(xRowSet, UNO_QUERY);
            }
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("Caught exception while creating a new RowSet!");
    }
    return xResultSet;
}

SwConnectionDisposedListener_Impl::SwConnectionDisposedListener_Impl(SwNewDBMgr& rMgr) :
    rDBMgr(rMgr)
{
};

SwConnectionDisposedListener_Impl::~SwConnectionDisposedListener_Impl()
{
};

void SwConnectionDisposedListener_Impl::disposing( const EventObject& rSource )
        throw (RuntimeException)
{
    ::SolarMutexGuard aGuard;
    uno::Reference<XConnection> xSource(rSource.Source, UNO_QUERY);
    for(sal_uInt16 nPos = rDBMgr.aDataSourceParams.size(); nPos; nPos--)
    {
        SwDSParam* pParam = &rDBMgr.aDataSourceParams[nPos - 1];
        if(pParam->xConnection.is() &&
                (xSource == pParam->xConnection))
        {
            rDBMgr.aDataSourceParams.erase(rDBMgr.aDataSourceParams.begin() + nPos - 1);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
