/*************************************************************************
 *
 *  $RCSfile: dbmgr.cxx,v $
 *
 *  $Revision: 1.47 $
 *
 *  last change: $Author: os $ $Date: 2001-09-06 13:36:14 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#if STLPORT_VERSION>=321
#include <cstdarg>
#endif

#include <stdio.h>

#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_iXNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDCONNECTION_HPP_
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _DBCONFIG_HXX
#include <dbconfig.hxx>
#endif
#ifndef _SWDBTOOLSCLIENT_HXX
#include <swdbtoolsclient.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _ZFORMAT_HXX //autogen
#include <svtools/zformat.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_PROGRESS_HXX //autogen
#include <sfx2/progress.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _MAILENUM_HXX //autogen
#include <goodies/mailenum.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _DBINSDLG_HXX
#include <dbinsdlg.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif
#ifndef _SWPRTOPT_HXX
#include <swprtopt.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _DBUI_HXX
#include <dbui.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif

#ifndef _DBUI_HRC
#include <dbui.hrc>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>
#endif

#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#include <connectivity/dbconversion.hxx>
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOLUMN_HPP_
#include <com/sun/star/sdb/XColumn.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XSTATEMENT_HPP_
#include <com/sun/star/sdbc/XStatement.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_UNOMID_HXX
#include <svx/unomid.hxx>
#endif
#ifndef _NUMUNO_HXX
#include <svtools/numuno.hxx>
#endif
#include "mailmrge.hxx"


#ifndef _SFXEVENT_HXX
#include <sfx2/event.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdb;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace ::com::sun::star::ucb;
using namespace com::sun::star::task;

#define C2S(cChar) String::CreateFromAscii(cChar)
#define C2U(char) rtl::OUString::createFromAscii(char)

#define DB_SEP_SPACE    0
#define DB_SEP_TAB      1
#define DB_SEP_RETURN   2
#define DB_SEP_NEWLINE  3

SV_IMPL_PTRARR(SwDSParamArr, SwDSParamPtr);
const sal_Char cCursor[] = "Cursor";
const sal_Char cCommand[] = "Command";
const sal_Char cCommandType[] = "CommandType";
const sal_Char cDataSourceName[] = "DataSourceName";
const sal_Char cSelection[] = "Selection";
const sal_Char cActiveConnection[] = "ActiveConnection";

/* -----------------------------17.07.00 17:04--------------------------------

 ---------------------------------------------------------------------------*/
BOOL lcl_MoveAbsolute(SwDSParam* pParam, long nAbsPos)
{
    BOOL bRet = FALSE;
    try
    {
        if(pParam->bScrollable)
        {
            bRet = pParam->xResultSet->absolute( nAbsPos );
        }
        else
        {
            DBG_ERROR("no absolute positioning available")
        }
    }
    catch(Exception aExcept)
    {
        DBG_ERROR("exception caught")
    }
    return bRet;
}
/* -----------------------------17.07.00 17:23--------------------------------

 ---------------------------------------------------------------------------*/
BOOL lcl_GetColumnCnt(SwDSParam* pParam,
    const String& rColumnName, long nLanguage, String& rResult, double* pNumber)
{
    Reference< XColumnsSupplier > xColsSupp( pParam->xResultSet, UNO_QUERY );
    Reference <XNameAccess> xCols = xColsSupp->getColumns();
    if(!xCols->hasByName(rColumnName))
        return FALSE;
    Any aCol = xCols->getByName(rColumnName);
    Reference< XPropertySet > xColumnProps;
    aCol >>= xColumnProps;

    SwDBFormatData aFormatData;
    aFormatData.aNullDate = pParam->aNullDate;
    aFormatData.xFormatter = pParam->xFormatter;

    String sLanguage, sCountry;
    ::ConvertLanguageToIsoNames( (USHORT)nLanguage, sLanguage, sCountry );
    aFormatData.aLocale.Language = sLanguage;
    aFormatData.aLocale.Country = sCountry;

    rResult = SwNewDBMgr::GetDBField( xColumnProps, aFormatData, pNumber);
    return TRUE;
};
/*--------------------------------------------------------------------
    Beschreibung: Daten importieren
 --------------------------------------------------------------------*/
BOOL SwNewDBMgr::MergeNew(USHORT nOpt, SwWrtShell& rSh,
                        const Sequence<PropertyValue>& rProperties,
                        const String *pPrinter)
{

    DBG_ASSERT(!bInMerge && !pImpl->pMergeData, "merge already activated!")

    SwDBData aData;
    aData.nCommandType = CommandType::TABLE;
    Reference<XResultSet>  xResSet;
    Sequence<Any> aSelection;
    Sequence<sal_Int32> aDlgSelection;
    Reference< XConnection> xConnection;
    const PropertyValue* pValues = rProperties.getConstArray();
    for(sal_Int32 nPos = 0; nPos < rProperties.getLength(); nPos++)
    {
        if(pValues[nPos].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cDataSourceName)))
            pValues[nPos].Value >>= aData.sDataSource;
        else if(pValues[nPos].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cCommand)))
            pValues[nPos].Value >>= aData.sCommand;
        else if(pValues[nPos].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cCursor)))
            pValues[nPos].Value >>= xResSet;
        else if(pValues[nPos].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cSelection)))
        {
            //both types are possible here
            pValues[nPos].Value >>= aSelection;
            pValues[nPos].Value >>= aDlgSelection;
        }
        else if(pValues[nPos].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cCommandType)))
            pValues[nPos].Value >>= aData.nCommandType;
        else if(pValues[nPos].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cActiveConnection)))
            pValues[nPos].Value >>= xConnection;
    }
    if(!aData.sDataSource.getLength() || !aData.sCommand.getLength() || !xResSet.is())
    {
        return FALSE;
    }
    if(aSelection.getLength())
    {
        aDlgSelection.realloc(aSelection.getLength());
        sal_Int32* pDlgSelection = aDlgSelection.getArray();
        const Any* pGridSelection = aSelection.getConstArray();
        for(sal_Int32 nSel = 0; nSel < aSelection.getLength(); nSel++)
            pGridSelection[nSel] >>= pDlgSelection[nSel];
    }
    pImpl->pMergeData = new SwDSParam(aData, xResSet, aDlgSelection);
    SwDSParam*  pTemp = FindDSData(aData, FALSE);
    if(pTemp)
        *pTemp = *pImpl->pMergeData;
    else
        aDataSourceParams.Insert(new SwDSParam(*pImpl->pMergeData), aDataSourceParams.Count());
    if(!pImpl->pMergeData->xConnection.is())
        pImpl->pMergeData->xConnection = xConnection;

    try{
        //set to start position
        if(pImpl->pMergeData->aSelection.getLength())
        {
            pImpl->pMergeData->bEndOfDB = !pImpl->pMergeData->xResultSet->absolute(
                    (ULONG)pImpl->pMergeData->aSelection.getConstArray()[ pImpl->pMergeData->nSelectionIndex++ ] );
            pImpl->pMergeData->CheckEndOfDB();
            if(pImpl->pMergeData->nSelectionIndex >= pImpl->pMergeData->aSelection.getLength())
                pImpl->pMergeData->bEndOfDB = TRUE;
        }
        else
        {
            pImpl->pMergeData->bEndOfDB = !pImpl->pMergeData->xResultSet->first();
            pImpl->pMergeData->CheckEndOfDB();
        }
    }
    catch(Exception&)
    {
        pImpl->pMergeData->bEndOfDB = TRUE;
        pImpl->pMergeData->CheckEndOfDB();
        DBG_ERROR("exception in MergeNew()")
    }

    Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
    Reference<XDataSource> xSource = GetDbtoolsClient().getDataSource(aData.sDataSource, xMgr);
    if( xMgr.is() )
    {
        Reference<XInterface> xInstance = xMgr->createInstance( C2U( "com.sun.star.util.NumberFormatter" ));
        pImpl->pMergeData->xFormatter = Reference<util::XNumberFormatter>(xInstance, UNO_QUERY) ;
    }

    Reference<XPropertySet> xSourceProps(xSource, UNO_QUERY);
    if(xSourceProps.is())
    {
        Any aFormats = xSourceProps->getPropertyValue(C2U("NumberFormatsSupplier"));
        if(aFormats.hasValue())
        {
            Reference<XNumberFormatsSupplier> xSuppl;
            aFormats >>= xSuppl;
            if(xSuppl.is())
            {
                Reference< XPropertySet > xSettings = xSuppl->getNumberFormatSettings();
                Any aNull = xSettings->getPropertyValue(C2U("NullDate"));
                aNull >>= pImpl->pMergeData->aNullDate;
            }
        }
    }

    rSh.ChgDBData(aData);
    bInMerge = TRUE;

    if (IsInitDBFields())
    {
        // Bei Datenbankfeldern ohne DB-Name DB-Name von Dok einsetzen
        SvStringsDtor aDBNames(1, 1);
        aDBNames.Insert( new String(), 0);
        SwDBData aData = rSh.GetDBData();
        String sDBName = aData.sDataSource;
        sDBName += DB_DELIM;
        sDBName += (String)aData.sCommand;
        sDBName += DB_DELIM;
        sDBName += String::CreateFromInt32(aData.nCommandType);
        rSh.ChangeDBFields( aDBNames, sDBName);
        SetInitDBFields(FALSE);
    }

    BOOL bRet = TRUE;
    switch(nOpt)
    {
        case DBMGR_MERGE:
            bRet = Merge(&rSh);   // Mischen
            break;

        case DBMGR_MERGE_MAILMERGE: // Serienbrief
            {
            SfxDispatcher *pDis = rSh.GetView().GetViewFrame()->GetDispatcher();
            if (pPrinter)   // Aufruf kommt aus dem Basic
            {
                SfxBoolItem aSilent( SID_SILENT, TRUE );
                if (pPrinter)
                {
                    SfxStringItem aPrinterName(SID_PRINTER_NAME, *pPrinter);
                    pDis->Execute( SID_PRINTDOC, SFX_CALLMODE_SYNCHRON,
                                   &aPrinterName, &aSilent, 0L );
                }
                else
                {
                    pDis->Execute( SID_PRINTDOC, SFX_CALLMODE_SYNCHRON,
                                   &aSilent, 0L );
                }
            }
            else
                pDis->Execute(SID_PRINTDOC, SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD);
            }
            break;

        case DBMGR_MERGE_MAILING:
            bRet = MergeMailing(&rSh);  // Mailing
            break;

        case DBMGR_MERGE_MAILFILES:
            bRet = MergeMailFiles(&rSh);    // Serienbriefe als Dateien abspeichern
            break;

        default:        // Einfuegen der selektierten Eintraege
                        // (war: InsertRecord)
            ImportFromConnection(&rSh);
            break;
    }

    EndMerge();
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung: Daten importieren
 --------------------------------------------------------------------*/


BOOL SwNewDBMgr::Merge(SwWrtShell* pSh)
{
    pSh->StartAllAction();

    pSh->ViewShell::UpdateFlds(TRUE);
    pSh->SetModified();

    pSh->EndAllAction();

    return TRUE;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/
void SwNewDBMgr::ImportFromConnection(  SwWrtShell* pSh )
{
    if(pImpl->pMergeData && !pImpl->pMergeData->bEndOfDB)
    {
        {
            pSh->StartAllAction();
            pSh->StartUndo(0);
            BOOL bGroupUndo(pSh->DoesGroupUndo());
            pSh->DoGroupUndo(FALSE);

            if( pSh->HasSelection() )
                pSh->DelRight();

            SwWait *pWait = 0;

            {
                ULONG i = 0;
                do {

                    ImportDBEntry(pSh);
                    if( 10 == ++i )
                        pWait = new SwWait( *pSh->GetView().GetDocShell(), TRUE);

                } while(ToNextMergeRecord());
            }

            pSh->DoGroupUndo(bGroupUndo);
            pSh->EndUndo(0);
            pSh->EndAllAction();
            delete pWait;
        }
    }
}
/*-----------------24.02.97 10.30-------------------

--------------------------------------------------*/

String  lcl_FindColumn(const String& sFormatStr,USHORT  &nUsedPos, BYTE &nSeparator)
{
    String sReturn;
    USHORT nLen = sFormatStr.Len();
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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

inline String lcl_GetDBInsertMode( const SwDBData& rData )
{
    return aEmptyStr;
#if 0
//JP 13.11.00: must be change to the new configuration
    return  SFX_APP()->GetIniManager()->Get( String::CreateFromAscii(
                RTL_CONSTASCII_STRINGPARAM( "DataBaseFormatInfo" )),
                FALSE, FALSE, sDBName );
#endif
}


void SwNewDBMgr::ImportDBEntry(SwWrtShell* pSh)
{
    if(pImpl->pMergeData && !pImpl->pMergeData->bEndOfDB)
    {
        Reference< XColumnsSupplier > xColsSupp( pImpl->pMergeData->xResultSet, UNO_QUERY );
          Reference <XNameAccess> xCols = xColsSupp->getColumns();
        String sFormatStr( lcl_GetDBInsertMode( *pImpl->pMergeData ));
        USHORT nFmtLen = sFormatStr.Len();
        if( nFmtLen )
        {
            const char cSpace = ' ';
            const char cTab = '\t';
            USHORT nUsedPos = 0;
            BYTE    nSeparator;
            String sColumn = lcl_FindColumn(sFormatStr, nUsedPos, nSeparator);
            while( sColumn.Len() )
            {
                if(!xCols->hasByName(sColumn))
                    return;
                Any aCol = xCols->getByName(sColumn);
                Reference< XPropertySet > xColumnProp;
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
                    // Spalte nicht gefunden -> Fehler anzeigen
                    String sInsert = '?';
                    sInsert += sColumn;
                    sInsert += '?';
                    pSh->Insert(sInsert);
                }
                sColumn = lcl_FindColumn(sFormatStr, nUsedPos, nSeparator);
            }
            pSh->SplitNode();
        }
        else
        {
            String sStr;
            Sequence<rtl::OUString> aColNames = xCols->getElementNames();
            const rtl::OUString* pColNames = aColNames.getConstArray();
            long nLength = aColNames.getLength();
            for(long i = 0; i < nLength; i++)
            {
                Any aCol = xCols->getByName(pColNames[i]);
                Reference< XPropertySet > xColumnProp;
                aCol >>= xColumnProp;
                SwDBFormatData aDBFormat;
                sStr += GetDBField( xColumnProp, aDBFormat);
                if (i < nLength - 1)
                    sStr += '\t';
            }
            pSh->SwEditShell::Insert(sStr);
            pSh->SwFEShell::SplitNode();    // Zeilenvorschub
        }
    }
}
/*--------------------------------------------------------------------
    Beschreibung: Listbox mit Tabellenliste fuellen
 --------------------------------------------------------------------*/
BOOL SwNewDBMgr::GetTableNames(ListBox* pListBox, const String& rDBName)
{
    BOOL bRet = FALSE;
    String sOldTableName(pListBox->GetSelectEntry());
    pListBox->Clear();
    Reference< XDataSource> xSource;
    SwDSParam* pParam = FindDSConnection(rDBName, FALSE);
    Reference< XConnection> xConnection;
    if(pParam && pParam->xConnection.is())
        xConnection = pParam->xConnection;
    else
        xConnection = SwNewDBMgr::GetConnection(rDBName, xSource);
    if(xConnection.is())
    {
        Reference<XTablesSupplier> xTSupplier = Reference<XTablesSupplier>(xConnection, UNO_QUERY);
        if(xTSupplier.is())
        {
            Reference<XNameAccess> xTbls = xTSupplier->getTables();
            Sequence<rtl::OUString> aTbls = xTbls->getElementNames();
            const rtl::OUString* pTbls = aTbls.getConstArray();
            for(long i = 0; i < aTbls.getLength(); i++)
            {
                USHORT nEntry = pListBox->InsertEntry(pTbls[i]);
                pListBox->SetEntryData(nEntry, (void*)0);
            }
        }
        Reference<XQueriesSupplier> xQSupplier = Reference<XQueriesSupplier>(xConnection, UNO_QUERY);
        if(xQSupplier.is())
        {
            Reference<XNameAccess> xQueries = xQSupplier->getQueries();
            Sequence<rtl::OUString> aQueries = xQueries->getElementNames();
            const rtl::OUString* pQueries = aQueries.getConstArray();
            for(long i = 0; i < aQueries.getLength(); i++)
            {
                USHORT nEntry = pListBox->InsertEntry(pQueries[i]);
                pListBox->SetEntryData(nEntry, (void*)1);
            }
        }
        if (sOldTableName.Len())
            pListBox->SelectEntry(sOldTableName);
        bRet = TRUE;
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung: Listbox mit Spaltennamen einer Datenbank fuellen
 --------------------------------------------------------------------*/
BOOL SwNewDBMgr::GetColumnNames(ListBox* pListBox,
            const String& rDBName, const String& rTableName, BOOL bAppend)
{
    if (!bAppend)
        pListBox->Clear();
    Reference< XDataSource> xSource;
    SwDSParam* pParam = FindDSConnection(rDBName, FALSE);
    Reference< XConnection> xConnection;
    if(pParam && pParam->xConnection.is())
        xConnection = pParam->xConnection;
    else
        xConnection = SwNewDBMgr::GetConnection(rDBName, xSource);
    Reference< XColumnsSupplier> xColsSupp = SwNewDBMgr::GetColumnSupplier(xConnection, rTableName);
    if(xColsSupp.is())
    {
        Reference <XNameAccess> xCols = xColsSupp->getColumns();
        const Sequence<rtl::OUString> aColNames = xCols->getElementNames();
        const rtl::OUString* pColNames = aColNames.getConstArray();
        for(int nCol = 0; nCol < aColNames.getLength(); nCol++)
        {
            pListBox->InsertEntry(pColNames[nCol]);
        }
    }
    return(TRUE);
}
/* -----------------------------08.06.01 15:11--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwNewDBMgr::GetColumnNames(ListBox* pListBox,
        Reference< XConnection> xConnection,
        const String& rTableName, BOOL bAppend)
{
    if (!bAppend)
        pListBox->Clear();
    Reference< XColumnsSupplier> xColsSupp = SwNewDBMgr::GetColumnSupplier(xConnection, rTableName);
    if(xColsSupp.is())
    {
        Reference <XNameAccess> xCols = xColsSupp->getColumns();
        const Sequence<rtl::OUString> aColNames = xCols->getElementNames();
        const rtl::OUString* pColNames = aColNames.getConstArray();
        for(int nCol = 0; nCol < aColNames.getLength(); nCol++)
        {
            pListBox->InsertEntry(pColNames[nCol]);
        }
    }
    return(TRUE);
}

/*--------------------------------------------------------------------
    Beschreibung: CTOR
 --------------------------------------------------------------------*/

SwNewDBMgr::SwNewDBMgr() :
            pImpl(new SwNewDBMgr_Impl),
            bInMerge(FALSE),
            nMergeType(DBMGR_INSERT),
            bInitDBFields(FALSE)
{
}
/* -----------------------------18.07.00 08:56--------------------------------

 ---------------------------------------------------------------------------*/
SwNewDBMgr::~SwNewDBMgr()
{
    for(USHORT nPos = 0; nPos < aDataSourceParams.Count(); nPos++)
    {
        SwDSParam* pParam = aDataSourceParams[nPos];
        if(pParam->xConnection.is())
        {
            try
            {
                Reference<XComponent> xComp(pParam->xConnection, UNO_QUERY);
                if(xComp.is())
                    xComp->dispose();
            }
            catch(RuntimeException& rEx)
            {
                //may be disposed already since multiple entries may have used the same connection
            }
        }
    }
    delete pImpl->pDbtoolsClient;
    delete pImpl;
}
/*--------------------------------------------------------------------
    Beschreibung:   Serienbrief drucken
 --------------------------------------------------------------------*/


BOOL SwNewDBMgr::MergePrint( SwView& rView,
                             SwPrtOptions& rOpt, SfxProgress& rProgress )
{
    SwWrtShell* pSh = &rView.GetWrtShell();
    //check if the doc is synchronized and contains at least one linked section
    BOOL bSynchronizedDoc = pSh->IsLabelDoc() && pSh->GetSectionFmtCount() > 1;
    //merge source is already open
    rOpt.nMergeCnt = pImpl->pMergeData && pImpl->pMergeData->aSelection.getLength();

    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
    rOpt.bSinglePrtJobs = pModOpt->IsSinglePrintJob();

    SfxPrinter *pPrt = pSh->GetPrt();
    Link aSfxSaveLnk = pPrt->GetEndPrintHdl();
    if( rOpt.bSinglePrtJobs  )
        pPrt->SetEndPrintHdl( Link() );

    BOOL bNewJob = FALSE,
         bUserBreak = FALSE,
         bRet = FALSE;

    do {
        {
            pSh->ViewShell::UpdateFlds();
            ++rOpt.nMergeAct;
            rView.SfxViewShell::Print( rProgress ); // ggf Basic-Macro ausfuehren

            if( rOpt.bSinglePrtJobs && bRet )
            {
                //rOpt.bJobStartet = FALSE;
                bRet = FALSE;
            }

            if( pSh->Prt( rOpt, rProgress ) )
                bRet = TRUE;

            if( !pPrt->IsJobActive() )
            {
                bUserBreak = TRUE;
                bRet = FALSE;
                break;
            }
            if( !rOpt.bSinglePrtJobs )
            {
                String& rJNm = (String&)rOpt.GetJobName();
                rJNm.Erase();
            }
        }
    } while( bSynchronizedDoc ? ExistsNextRecord() : ToNextMergeRecord());

    if( rOpt.bSinglePrtJobs )
    {
        pSh->GetPrt()->SetEndPrintHdl( aSfxSaveLnk );
        if ( !bUserBreak && !pSh->GetPrt()->IsJobActive() )     //Schon zu spaet?
            aSfxSaveLnk.Call( pSh->GetPrt() );
    }

    rOpt.nMergeCnt = 0;
    rOpt.nMergeAct = 0;

    nMergeType = DBMGR_INSERT;

    SwDocShell* pDocSh = rView.GetDocShell();
    SfxViewFrame *pTmpFrm = SfxViewFrame::GetFirst(pDocSh);

    while (pTmpFrm)     // Alle Views Invalidieren
    {
        SwView *pVw = PTR_CAST(SwView, pTmpFrm->GetViewShell());
        if (pVw)
            pVw->GetEditWin().Invalidate();
        pTmpFrm = pTmpFrm->GetNext(*pTmpFrm, pDocSh);
    }

    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:   Serienbrief als Mail versenden
 --------------------------------------------------------------------*/


BOOL SwNewDBMgr::MergeMailing(SwWrtShell* pSh)
{
    //check if the doc is synchronized and contains at least one linked section
    BOOL bSynchronizedDoc = pSh->IsLabelDoc() && pSh->GetSectionFmtCount() > 1;
    BOOL bLoop = TRUE;

    {
        Reference< XColumnsSupplier > xColsSupp( pImpl->pMergeData->xResultSet, UNO_QUERY );
        Reference <XNameAccess> xCols = xColsSupp->getColumns();
        if(!xCols->hasByName(sEMailAddrFld))
            return FALSE;
        Any aCol = xCols->getByName(sEMailAddrFld);
        Reference< XPropertySet > xColumnProp;
        aCol >>= xColumnProp;

        SfxDispatcher* pSfxDispatcher = pSh->GetView().GetViewFrame()->GetDispatcher();
        if (!sSubject.Len())    // Kein leeres Subject wegen Automail (PB)
            sSubject = ' ';
        SfxStringItem aSubject(SID_MAIL_SUBJECT, sSubject);
        SfxStringItem aText(SID_MAIL_TEXT, ' ');    // Leerer Text ist nicht moeglich
        SfxStringItem aAttached(SID_MAIL_ATTACH_FILE, sAttached);
        SfxBoolItem aAttach(SID_MAIL_ATTACH, TRUE);

        SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
        BYTE nMailFmts = pModOpt->GetMailingFormats() | TXTFORMAT_ASCII;    // Immer Ascii
        SfxByteItem aTextFormats(SID_MAIL_TXTFORMAT, nMailFmts);

        pSfxDispatcher->Execute( SID_SAVEDOC, SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD);
        if( !pSh->IsModified() )
        {
            // Beim Speichern wurde kein Abbruch gedrueckt
            // neue DocShell erzeugen, alle gelinkten Bereiche embedden
            // und unter temporaerem Namen wieder speichern.
            String sTmpName;
            const SfxFilter* pSfxFlt;

            {
                SfxMedium* pOrig = pSh->GetView().GetDocShell()->GetMedium();
                String sMainURL( pOrig->GetURLObject().GetMainURL(
                                            INetURLObject::NO_DECODE ));
                pSfxFlt = SwIoSystem::GetFileFilter( sMainURL, ::aEmptyStr );

                sTmpName = utl::TempFile::CreateTempName(0);
                sTmpName = URIHelper::SmartRelToAbs(sTmpName);

                try
                {
                    String sMain(sTmpName);
                    sal_Unicode cSlash = '/';
                    xub_StrLen nSlashPos = sMain.SearchBackward(cSlash);
                    sMain.Erase(nSlashPos);
                    ::ucb::Content aNewContent( sMain, Reference< XCommandEnvironment > ());
                    Any aAny;
                    TransferInfo aInfo;
                    aInfo.NameClash = NameClash::ERROR;
                    aInfo.NewTitle = INetURLObject(sTmpName).GetName();
                    aInfo.SourceURL = sMainURL;
                    aInfo.MoveData  = FALSE;
                    aAny <<= aInfo;
                    aNewContent.executeCommand( C2U( "transfer" ), aAny);
                }
                catch( Exception& rEx)
                {
                }
            }
            String sAddress;
            ULONG nDocNo = 1;
            bCancel = FALSE;

            PrintMonitor aPrtMonDlg(&pSh->GetView().GetEditWin(), TRUE);
            aPrtMonDlg.aDocName.SetText(pSh->GetView().GetDocShell()->GetTitle(22));
            aPrtMonDlg.aCancel.SetClickHdl(LINK(this, SwNewDBMgr, PrtCancelHdl));
            aPrtMonDlg.Show();

            OfficeApplication* pOffApp = OFF_APP();
            SfxRequest aReq( SID_OPENDOC, SFX_CALLMODE_SYNCHRON, pOffApp->GetPool() );
            aReq.AppendItem( SfxStringItem( SID_FILE_NAME, sTmpName ));
            if(pSfxFlt)
                aReq.AppendItem( SfxStringItem( SID_FILTER_NAME, pSfxFlt->GetName() ));
            aReq.AppendItem( SfxBoolItem( SID_HIDDEN, TRUE ) );
            aReq.AppendItem( SfxStringItem( SID_REFERER, String::CreateFromAscii(URL_PREFIX_PRIV_SOFFICE )));

            pOffApp->ExecuteSlot( aReq, pOffApp->SfxApplication::GetInterface());
            const SfxPoolItem* pReturnValue = aReq.GetReturnValue();
            if(pReturnValue)
            {
                // DocShell besorgen
                SfxViewFrameItem* pVItem = (SfxViewFrameItem*)pReturnValue;
                SwView* pView = (SwView*) pVItem->GetFrame()->GetViewShell();
                SwWrtShell& rSh = pView->GetWrtShell();
                pView->AttrChangedNotify( &rSh );//Damit SelectShell gerufen wird.

                SwDoc* pDoc = rSh.GetDoc();
                SwNewDBMgr* pOldDBMgr = pDoc->GetNewDBMgr();
                pDoc->SetNewDBMgr( this );
                pDoc->EmbedAllLinks();
                String sTempStat(SW_RES(STR_DB_EMAIL));

                do
                {
                    if(UIUNDO_DELETE_INVISIBLECNTNT == rSh.GetUndoIds())
                        rSh.Undo();
                    rSh.ViewShell::UpdateFlds();

                    // alle versteckten Felder/Bereiche entfernen
                    rSh.RemoveInvisibleContent();

                    SfxFrameItem aFrame( SID_DOCFRAME, pVItem->GetFrame() );
                    SwDBFormatData aDBFormat;
                    sAddress = GetDBField( xColumnProp, aDBFormat);
                    if (!sAddress.Len())
                        sAddress = '_';

                    String sStat(sTempStat);
                    sStat += ' ';
                    sStat += String::CreateFromInt32( nDocNo++ );
                    aPrtMonDlg.aPrintInfo.SetText(sStat);
                    aPrtMonDlg.aPrinter.SetText( sAddress );

                    // Rechenzeit fuer EMail-Monitor:
                    for (USHORT i = 0; i < 25; i++)
                        Application::Reschedule();

                    sAddress.Insert(String::CreateFromAscii("mailto:"), 0);
                    SfxStringItem aRecipient( SID_MAIL_RECIPIENT, sAddress );

                    const SfxPoolItem* pRet = pSfxDispatcher->Execute(
                                SID_MAIL_SENDDOC,
                                SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD,
                                &aRecipient, &aSubject, &aAttach, &aAttached,
                                &aText, &aTextFormats, &aFrame,
                                0L );
                    //this must be done here because pRet may be destroyed in Reschedule (DeleteOnIdle)
                    BOOL bBreak = pRet && !( (SfxBoolItem*)pRet )->GetValue();

                    // Rechenzeit fuer EMail-Monitor:
                    for (i = 0; i < 25; i++)
                        Application::Reschedule();

                    if ( bBreak )
                        break; // das Verschicken wurde unterbrochen
                } while( !bCancel && (bSynchronizedDoc ? ExistsNextRecord() : ToNextMergeRecord()));
                pDoc->SetNewDBMgr( pOldDBMgr );
                pView->GetDocShell()->OwnerLock( FALSE );

            }
            // remove the temporary file
            try
            {
                ::ucb::Content aTempContent(
                    sTmpName,
                    Reference< XCommandEnvironment > ());
                aTempContent.executeCommand( C2U( "delete" ),
                                    makeAny( sal_Bool( sal_True ) ) );
            }
            catch( Exception& rEx)
            {
                DBG_ERRORFILE( "Exception" );
            }
            SW_MOD()->SetView(&pSh->GetView());
        }

        nMergeType = DBMGR_INSERT;
    }
    return bLoop;
}

/*--------------------------------------------------------------------
    Beschreibung:   Serienbriefe als einzelne Dokumente speichern
 --------------------------------------------------------------------*/

BOOL SwNewDBMgr::MergeMailFiles(SwWrtShell* pSh)
{
    //check if the doc is synchronized and contains at least one linked section
    BOOL bSynchronizedDoc = pSh->IsLabelDoc() && pSh->GetSectionFmtCount() > 1;
    BOOL bLoop = TRUE;

    Reference< XPropertySet > xColumnProp;
    {
        USHORT nColPos = 0;
        BOOL bColumnName = sEMailAddrFld.Len() > 0;

        if (bColumnName)
        {
            Reference< XColumnsSupplier > xColsSupp( pImpl->pMergeData->xResultSet, UNO_QUERY );
            Reference <XNameAccess> xCols = xColsSupp->getColumns();
            if(!xCols->hasByName(sEMailAddrFld))
                return FALSE;
            Any aCol = xCols->getByName(sEMailAddrFld);
            aCol >>= xColumnProp;
        }

        SfxDispatcher* pSfxDispatcher = pSh->GetView().GetViewFrame()->GetDispatcher();

        pSfxDispatcher->Execute( SID_SAVEDOC, SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD);
        if( !pSh->IsModified() )
        {
            // Beim Speichern wurde kein Abbruch gedrueckt
            SfxMedium* pOrig = pSh->GetView().GetDocShell()->GetMedium();
            String sOldName(pOrig->GetURLObject().GetMainURL( INetURLObject::NO_DECODE ));
            const SfxFilter* pSfxFlt = SwIoSystem::GetFileFilter(
                                                    sOldName, ::aEmptyStr );
            String sAddress;
            bCancel = FALSE;

            PrintMonitor aPrtMonDlg(&pSh->GetView().GetEditWin());
            aPrtMonDlg.aDocName.SetText(pSh->GetView().GetDocShell()->GetTitle(22));

            aPrtMonDlg.aCancel.SetClickHdl(LINK(this, SwNewDBMgr, PrtCancelHdl));
            aPrtMonDlg.Show();

            SwDocShell *pDocSh = pSh->GetView().GetDocShell();
            // Progress, um KeyInputs zu unterbinden
            SfxProgress aProgress(pDocSh, ::aEmptyStr, 1);

            // Alle Dispatcher sperren
            SfxViewFrame* pViewFrm = SfxViewFrame::GetFirst(pDocSh);
            while (pViewFrm)
            {
                pViewFrm->GetDispatcher()->Lock(TRUE);
                pViewFrm = SfxViewFrame::GetNext(*pViewFrm, pDocSh);
            }
            ULONG nDocNo = 1;
            ULONG nCounter = 0;
            String sExt( INetURLObject( sOldName ).GetExtension() );
            sExt.InsertAscii(".", 0);

            do {
                {
                    String sPath(sSubject);

                    if( bColumnName )
                    {
                        SwDBFormatData aDBFormat;
                        sAddress = GetDBField( xColumnProp, aDBFormat);
                        if (!sAddress.Len())
                            sAddress = '_';
                        sPath += sAddress;
                        nCounter = 0;
                    }

                    INetURLObject aEntry(sPath);
                    String sLeading(aEntry.GetBase());
                    aEntry.removeSegment();
                    sPath = aEntry.GetMainURL( INetURLObject::NO_DECODE );
                    utl::TempFile aTemp(sLeading,&sExt,&sPath );

                    if( !aTemp.IsValid() )
                    {
                        ErrorHandler::HandleError( ERRCODE_IO_NOTSUPPORTED );
                        bLoop = FALSE;
                        bCancel = TRUE;
                    }
                    else
                    {
                        INetURLObject aTempFile(aTemp.GetURL());
                        aPrtMonDlg.aPrinter.SetText( aTempFile.GetBase() );
                        String sStat(SW_RES(STR_STATSTR_LETTER));   // Brief
                        sStat += ' ';
                        sStat += String::CreateFromInt32( nDocNo++ );
                        aPrtMonDlg.aPrintInfo.SetText(sStat);

                        // Rechenzeit fuer Save-Monitor:
                        for (USHORT i = 0; i < 10; i++)
                            Application::Reschedule();

                        // Neues Dokument erzeugen und speichern
                        SfxObjectShellRef xDocSh( new SwDocShell( SFX_CREATE_MODE_INTERNAL ));
                        SfxMedium* pMed = new SfxMedium( sOldName, STREAM_STD_READ, TRUE );
                        pMed->SetFilter( pSfxFlt );

                        if (xDocSh->DoLoad(pMed))
                        {
                            SwDoc* pDoc = ((SwDocShell*)(&xDocSh))->GetDoc();
                            SwNewDBMgr* pOldDBMgr = pDoc->GetNewDBMgr();
                            pDoc->SetNewDBMgr( this );
                            pDoc->UpdateFlds(0);

                            // alle versteckten Felder/Bereiche entfernen
                            pDoc->RemoveInvisibleContent();

                            SfxMedium* pDstMed = new SfxMedium(
                                aTempFile.GetMainURL( INetURLObject::NO_DECODE ),
                                STREAM_STD_READWRITE, TRUE );
                            pDstMed->SetFilter( pSfxFlt );

                            xDocSh->DoSaveAs(*pDstMed);
                            xDocSh->DoSaveCompleted(pDstMed);
                            if( xDocSh->GetError() )
                            {
                                // error message ??
                                ErrorHandler::HandleError( xDocSh->GetError() );
                                bCancel = TRUE;
                                bLoop = FALSE;
                            }
                            pDoc->SetNewDBMgr( pOldDBMgr );
                        }
                        xDocSh->DoClose();
                    }
                }
            } while( !bCancel && bSynchronizedDoc ? ExistsNextRecord() : ToNextMergeRecord());
            // Alle Dispatcher freigeben
            pViewFrm = SfxViewFrame::GetFirst(pDocSh);
            while (pViewFrm)
            {
                pViewFrm->GetDispatcher()->Lock(FALSE);
                pViewFrm = SfxViewFrame::GetNext(*pViewFrm, pDocSh);
            }

            SW_MOD()->SetView(&pSh->GetView());
        }

        nMergeType = DBMGR_INSERT;
    }

    return bLoop;
}

/*--------------------------------------------------------------------
    Beschreibung:
  --------------------------------------------------------------------*/

IMPL_LINK_INLINE_START( SwNewDBMgr, PrtCancelHdl, Button *, pButton )
{
    pButton->GetParent()->Hide();
    bCancel = TRUE;
    return 0;
}
IMPL_LINK_INLINE_END( SwNewDBMgr, PrtCancelHdl, Button *, pButton )


/*--------------------------------------------------------------------
    Beschreibung: Numberformat der Spalte ermitteln und ggfs. in
                    den uebergebenen Formatter uebertragen
  --------------------------------------------------------------------*/

ULONG SwNewDBMgr::GetColumnFmt( const String& rDBName,
                                const String& rTableName,
                                const String& rColNm,
                                SvNumberFormatter* pNFmtr,
                                long nLanguage )
{
    ULONG nRet = 0;
    if(pNFmtr)
    {
        Reference< XDataSource> xSource;
        Reference< XConnection> xConnection;
        sal_Bool bUseMergeData = sal_False;
        if(pImpl->pMergeData &&
            pImpl->pMergeData->sDataSource.equals(rDBName) && pImpl->pMergeData->sCommand.equals(rTableName))
        {
            xConnection = pImpl->pMergeData->xConnection;
            Reference<XChild> xChild(xConnection, UNO_QUERY);
            if(xChild.is())
                xSource = Reference<XDataSource>(xChild->getParent(), UNO_QUERY);
            bUseMergeData = sal_True;
        }
        if(!xConnection.is() || !xSource.is())
        {
            SwDSParam* pParam = FindDSConnection(rDBName, FALSE);
            if(pParam && pParam->xConnection.is())
                xConnection = pParam->xConnection;
            else
                xConnection = SwNewDBMgr::GetConnection(rDBName, xSource);
            if(bUseMergeData)
                pImpl->pMergeData->xConnection = xConnection;
        }
        Reference< XColumnsSupplier> xColsSupp = SwNewDBMgr::GetColumnSupplier(xConnection, rTableName);
        if(xColsSupp.is())
        {
            Reference <XNameAccess> xCols;
            try
            {
                xCols = xColsSupp->getColumns();
            }
            catch(Exception&)
            {
                DBG_ERROR("Exception in getColumns()")
            }
            if(!xCols.is() || !xCols->hasByName(rColNm))
                return nRet;
            Any aCol = xCols->getByName(rColNm);
            Reference< XPropertySet > xColumn;
            aCol >>= xColumn;
            nRet = GetColumnFmt(xSource, xConnection, xColumn, pNFmtr, nLanguage);
        }
        else
            nRet = pNFmtr->GetFormatIndex( NF_NUMBER_STANDARD, LANGUAGE_SYSTEM );
    }
    return nRet;
}
/* -----------------------------07.06.01 15:43--------------------------------

 ---------------------------------------------------------------------------*/
ULONG SwNewDBMgr::GetColumnFmt( Reference< XDataSource> xSource,
                        Reference< XConnection> xConnection,
                        Reference< XPropertySet> xColumn,
                        SvNumberFormatter* pNFmtr,
                        long nLanguage )
{
    //JP 12.01.99: ggfs. das NumberFormat im Doc setzen
    ULONG nRet = 0;

    if(!xSource.is())
    {
        Reference<XChild> xChild(xConnection, UNO_QUERY);
        xSource = Reference<XDataSource>(xChild->getParent(), UNO_QUERY);
    }
    if(xSource.is() && xConnection.is() && xColumn.is() && pNFmtr)
    {
        SvNumberFormatsSupplierObj* pNumFmt = new SvNumberFormatsSupplierObj( pNFmtr );
        Reference< util::XNumberFormatsSupplier >  xDocNumFmtsSupplier = pNumFmt;
        Reference< XNumberFormats > xDocNumberFormats = xDocNumFmtsSupplier->getNumberFormats();
         Reference< XNumberFormatTypes > xDocNumberFormatTypes(xDocNumberFormats, UNO_QUERY);

        String sLanguage, sCountry;
        ::ConvertLanguageToIsoNames( (USHORT)nLanguage, sLanguage, sCountry );
        Locale aLocale;
        aLocale.Language = sLanguage;
        aLocale.Country = sCountry;

        //get the number formatter of the data source
        Reference<XPropertySet> xSourceProps(xSource, UNO_QUERY);
        Reference< XNumberFormats > xNumberFormats;
        if(xSourceProps.is())
        {
            Any aFormats = xSourceProps->getPropertyValue(C2U("NumberFormatsSupplier"));
            if(aFormats.hasValue())
            {
                Reference<XNumberFormatsSupplier> xSuppl;
                aFormats >>= xSuppl;
                if(xSuppl.is())
                {
                    xNumberFormats = xSuppl->getNumberFormats();
                }
            }
        }
        Any aFormat = xColumn->getPropertyValue(C2U("FormatKey"));
        if(aFormat.hasValue())
        {
            sal_Int32 nFmt;
            aFormat >>= nFmt;
            if(xNumberFormats.is())
            {
                try
                {
                    Reference<XPropertySet> xNumProps = xNumberFormats->getByKey( nFmt );
                    Any aFormat = xNumProps->getPropertyValue(C2U("FormatString"));
                    Any aLocale = xNumProps->getPropertyValue(C2U("Locale"));
                    rtl::OUString sFormat;
                    aFormat >>= sFormat;
                    com::sun::star::lang::Locale aLoc;
                    aLocale >>= aLoc;
                    nFmt = xDocNumberFormats->queryKey( sFormat, aLoc, sal_False );
                    if(NUMBERFORMAT_ENTRY_NOT_FOUND == nFmt)
                        nFmt = xDocNumberFormats->addNew( sFormat, aLoc );
                    nRet = nFmt;
                }
                catch(Exception&)
                {
                    DBG_ERROR("illegal number format key")
                }
            }
        }
        else
            nRet = GetDbtoolsClient().getDefaultNumberFormat(xColumn, xDocNumberFormatTypes,  aLocale);
    }
    return nRet;
}

/* -----------------------------17.07.00 09:47--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int32 SwNewDBMgr::GetColumnType( const String& rDBName,
                          const String& rTableName,
                          const String& rColNm )
{
    sal_Int32 nRet = DataType::SQLNULL;
    Reference< XDataSource> xSource;
    SwDSParam* pParam = FindDSConnection(rDBName, FALSE);
    Reference< XConnection> xConnection;
    if(pParam && pParam->xConnection.is())
        xConnection = pParam->xConnection;
    else
        xConnection = SwNewDBMgr::GetConnection(rDBName, xSource);
    Reference< XColumnsSupplier> xColsSupp = SwNewDBMgr::GetColumnSupplier(xConnection, rTableName);
    if(xColsSupp.is())
    {
          Reference <XNameAccess> xCols = xColsSupp->getColumns();
        if(xCols->hasByName(rColNm))
        {
            Any aCol = xCols->getByName(rColNm);
            Reference <XPropertySet> xCol;
            aCol >>= xCol;
            Any aType = xCol->getPropertyValue(C2S("Type"));
            aType >>= nRet;
        }
    }
    return nRet;
}

/* -----------------------------03.07.00 17:12--------------------------------

 ---------------------------------------------------------------------------*/
Reference< sdbc::XConnection> SwNewDBMgr::GetConnection(const String& rDataSource,
                                                    Reference<XDataSource>& rxSource)
{
    Reference< sdbc::XConnection> xConnection;
    Reference<XNameAccess> xDBContext;
    Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
    if( xMgr.is() )
    {
        Reference<XInterface> xInstance = xMgr->createInstance( C2U( "com.sun.star.sdb.DatabaseContext" ));
        xDBContext = Reference<XNameAccess>(xInstance, UNO_QUERY) ;
    }
    DBG_ASSERT(xDBContext.is(), "com.sun.star.sdb.DataBaseContext: service not available")
    if(xDBContext.is())
    {
        try
        {
            if(xDBContext->hasByName(rDataSource))
            {
                Reference<XCompletedConnection> xComplConnection;
                xDBContext->getByName(rDataSource) >>= xComplConnection;
                rxSource = Reference<XDataSource>(xComplConnection, UNO_QUERY);

                Reference< XInteractionHandler > xHandler(
                    xMgr->createInstance( C2U( "com.sun.star.sdb.InteractionHandler" )), UNO_QUERY);
                xConnection = xComplConnection->connectWithCompletion( xHandler );
            }
        }
        catch(Exception&) {}
    }
    return xConnection;
}
/* -----------------------------03.07.00 17:12--------------------------------

 ---------------------------------------------------------------------------*/
Reference< sdbcx::XColumnsSupplier> SwNewDBMgr::GetColumnSupplier(Reference<sdbc::XConnection> xConnection,
                                    const String& rTableOrQuery,
                                    BYTE    eTableOrQuery)
{
    Reference< sdbcx::XColumnsSupplier> xRet;
    if(SW_DB_SELECT_QUERY != eTableOrQuery)
    {
        Reference<XTablesSupplier> xTSupplier = Reference<XTablesSupplier>(xConnection, UNO_QUERY);
        if(xTSupplier.is())
        {
            Reference<XNameAccess> xTbls = xTSupplier->getTables();
            if(xTbls->hasByName(rTableOrQuery))
                try
                {
                    Any aTable = xTbls->getByName(rTableOrQuery);
                    Reference<XPropertySet> xPropSet;
                    aTable >>= xPropSet;
                    xRet = Reference<XColumnsSupplier>(xPropSet, UNO_QUERY);
                }
                catch(Exception&){}
        }
    }
    if(!xRet.is() && SW_DB_SELECT_QUERY != SW_DB_SELECT_TABLE)
    {
        Reference<XQueriesSupplier> xQSupplier = Reference<XQueriesSupplier>(xConnection, UNO_QUERY);
        if(xQSupplier.is())
        {
            Reference<XNameAccess> xQueries = xQSupplier->getQueries();
            if(xQueries->hasByName(rTableOrQuery))
                try
                {
                    Any aQuery = xQueries->getByName(rTableOrQuery);
                    Reference<XPropertySet> xPropSet;
                    aQuery >>= xPropSet;
                    xRet = Reference<XColumnsSupplier>(xPropSet, UNO_QUERY);
                }
                catch(Exception&){}
        }
    }
    return xRet;
}
/* -----------------------------05.07.00 13:44--------------------------------

 ---------------------------------------------------------------------------*/
String SwNewDBMgr::GetDBField(Reference<XPropertySet> xColumnProps,
                        const SwDBFormatData& rDBFormatData,
                        double* pNumber)
{
    Reference< XColumn > xColumn(xColumnProps, UNO_QUERY);
    String sRet;
    DBG_ASSERT(xColumn.is(), "SwNewDBMgr::::ImportDBField: illegal arguments")
    if(!xColumn.is())
        return sRet;

    Any aType = xColumnProps->getPropertyValue(C2U("Type"));
    sal_Int32 eDataType;
    aType >>= eDataType;
    switch(eDataType)
    {
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::LONGVARCHAR:
            sRet = xColumn->getString();
        break;
        case DataType::BIT:
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
//          ::Date aTempDate(rDBFormatData.aNullDate.Day,
//              rDBFormatData.aNullDate.Month, rDBFormatData.aNullDate.Year);

            try
            {
                SwDbtoolsClient aClient;
                sRet = aClient.getValue(
                    xColumnProps,
                    rDBFormatData.xFormatter,
                    rDBFormatData.aLocale,
                    rDBFormatData.aNullDate);
                double fVal = xColumn->getDouble();
                if (pNumber)
                    *pNumber = fVal;
            }
            catch(Exception& )
            {
                DBG_ERROR("exception caught")
            }

        }
        break;

//      case DataType::BINARY:
//      case DataType::VARBINARY:
//      case DataType::LONGVARBINARY:
//      case DataType::SQLNULL:
//      case DataType::OTHER:
//      case DataType::OBJECT:
//      case DataType::DISTINCT:
//      case DataType::STRUCT:
//      case DataType::ARRAY:
//      case DataType::BLOB:
//      case DataType::CLOB:
//      case DataType::REF:
//      default:
    }
//  if (pFormat)
//  {
//      SFX_ITEMSET_GET(*pCol, pFormatItem, SfxUInt32Item, SBA_DEF_FMTVALUE, sal_True);
//      *pFormat = pFormatItem->GetValue();
//  }

    return sRet;
}
/* -----------------------------06.07.00 14:28--------------------------------
    releases the merge data source table or query after merge is completed
 ---------------------------------------------------------------------------*/
void    SwNewDBMgr::EndMerge()
{
    DBG_ASSERT(bInMerge, "merge is not active")
    bInMerge = FALSE;
    delete pImpl->pMergeData;
    pImpl->pMergeData = 0;
}
/* -----------------------------06.07.00 14:28--------------------------------
    checks if a desired data source table or query is open
 ---------------------------------------------------------------------------*/
BOOL    SwNewDBMgr::IsDataSourceOpen(const String& rDataSource,
            const String& rTableOrQuery, sal_Bool bMergeOnly)
{
    if(pImpl->pMergeData)
    {
        return rDataSource == (String)pImpl->pMergeData->sDataSource &&
                    rTableOrQuery == (String)pImpl->pMergeData->sCommand &&
                    pImpl->pMergeData->xResultSet.is();
    }
    else if(!bMergeOnly)
    {
        SwDBData aData;
        aData.sDataSource = rDataSource;
        aData.sCommand = rTableOrQuery;
        aData.nCommandType = -1;
        SwDSParam* pFound = FindDSData(aData, FALSE);
        return (pFound && pFound->xResultSet.is());
    }
    return sal_False;
}
/* -----------------------------17.07.00 16:44--------------------------------
    read column data a a specified position
 ---------------------------------------------------------------------------*/
BOOL SwNewDBMgr::GetColumnCnt(const String& rSourceName, const String& rTableName,
                            const String& rColumnName, sal_uInt32 nAbsRecordId,
                            long nLanguage,
                            String& rResult, double* pNumber)
{
    BOOL bRet = FALSE;
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
        pFound = FindDSData(aData, FALSE);
    }
    if(pFound && pFound->xResultSet.is() && !pFound->bAfterSelection)
    {
        sal_Int32 nOldRow = pFound->xResultSet->getRow();
        //position to the desired index
        BOOL bMove = TRUE;
        if(nOldRow != nAbsRecordId)
            bMove = lcl_MoveAbsolute(pFound, nAbsRecordId);
        if(bMove)
        {
            bRet = lcl_GetColumnCnt(pFound, rColumnName, nLanguage, rResult, pNumber);
        }
        if(nOldRow != nAbsRecordId)
            bMove = lcl_MoveAbsolute(pFound, nOldRow);
    }
    return bRet;
}
/* -----------------------------06.07.00 16:47--------------------------------
    reads the column data at the current position
 ---------------------------------------------------------------------------*/
BOOL    SwNewDBMgr::GetMergeColumnCnt(const String& rColumnName, USHORT nLanguage,
                                String &rResult, double *pNumber, sal_uInt32 *pFormat)
{
    if(!pImpl->pMergeData || !pImpl->pMergeData->xResultSet.is() || pImpl->pMergeData->bAfterSelection )
    {
        rResult.Erase();
        return FALSE;
    }

    BOOL bRet = lcl_GetColumnCnt(pImpl->pMergeData, rColumnName, nLanguage, rResult, pNumber);
    return bRet;
}
/* -----------------------------07.07.00 14:28--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwNewDBMgr::ToNextMergeRecord()
{
    DBG_ASSERT(pImpl->pMergeData && pImpl->pMergeData->xResultSet.is(), "no data source in merge")
    return ToNextRecord(pImpl->pMergeData);
}
/* -----------------------------10.07.01 14:28--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwNewDBMgr::ToNextRecord(
    const String& rDataSource, const String& rCommand, sal_Int32 nCommandType)
{
    SwDSParam* pFound = 0;
    BOOL bRet = TRUE;
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
        pFound = FindDSData(aData, FALSE);
    }
    return ToNextRecord(pFound);
}
/* -----------------------------10.07.01 14:38--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwNewDBMgr::ToNextRecord(SwDSParam* pParam)
{
    BOOL bRet = TRUE;
    if(!pParam || !pParam->xResultSet.is() || pParam->bEndOfDB)
    {
        if(pParam)
            pParam->CheckEndOfDB();
        return FALSE;
    }
    try
    {
        if(pParam->aSelection.getLength())
        {
            pParam->bEndOfDB = !pParam->xResultSet->absolute(
                    (ULONG)pParam->aSelection.getConstArray()[ pParam->nSelectionIndex++ ] );
            pParam->CheckEndOfDB();
            bRet = !pParam->bEndOfDB;
            if(pParam->nSelectionIndex >= pParam->aSelection.getLength())
                pParam->bEndOfDB = TRUE;
        }
        else
        {
            pParam->bEndOfDB = !pParam->xResultSet->next();
            pParam->CheckEndOfDB();
            bRet = !pParam->bEndOfDB;
            ++pParam->nSelectionIndex;
        }
    }
    catch(Exception&)
    {
        DBG_ERROR("exception caught")
    }
    return bRet;
}

/* -----------------------------13.07.00 17:23--------------------------------
    synchronized labels contain a next record field at their end
    to assure that the next page can be created in mail merge
    the cursor position must be validated
 ---------------------------------------------------------------------------*/
BOOL SwNewDBMgr::ExistsNextRecord() const
{
    return pImpl->pMergeData && !pImpl->pMergeData->bEndOfDB;
}
/* -----------------------------13.07.00 10:41--------------------------------

 ---------------------------------------------------------------------------*/
sal_uInt32  SwNewDBMgr::GetSelectedRecordId()
{
    sal_uInt32  nRet = 0;
    DBG_ASSERT(pImpl->pMergeData && pImpl->pMergeData->xResultSet.is(), "no data source in merge")
    if(!pImpl->pMergeData || !pImpl->pMergeData->xResultSet.is())
        return FALSE;
    try
    {
        nRet = pImpl->pMergeData->xResultSet->getRow();
    }
    catch(Exception& )
    {
        DBG_ERROR("exception caught")
    }
    return nRet;
}
/* -----------------------------13.07.00 10:58--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwNewDBMgr::ToRecordId(sal_Int32 nSet)
{
    DBG_ASSERT(pImpl->pMergeData && pImpl->pMergeData->xResultSet.is(), "no data source in merge")
    if(!pImpl->pMergeData || !pImpl->pMergeData->xResultSet.is()|| nSet < 0)
        return FALSE;
    sal_Bool bRet = FALSE;
    sal_Int32 nAbsPos = nSet;

    if(nAbsPos >= 0)
    {
        bRet = lcl_MoveAbsolute(pImpl->pMergeData, nAbsPos);
        pImpl->pMergeData->bEndOfDB = !bRet;
        pImpl->pMergeData->CheckEndOfDB();
    }
    return bRet;
}
/* -----------------------------17.07.00 14:50--------------------------------

 ---------------------------------------------------------------------------*/
void lcl_ExtractMembers(const String& rDBName, String& sSource, String& sTable, String& sStatement)
{
    sSource = rDBName.GetToken(0, DB_DELIM);
    sTable = rDBName.GetToken(0).GetToken(1, DB_DELIM);
    sal_uInt16 nPos;
    if ((nPos = rDBName.Search(';')) != STRING_NOTFOUND)
        sStatement = rDBName.Copy(nPos + 1);
}
/* -----------------------------17.07.00 14:17--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwNewDBMgr::OpenDataSource(const String& rDataSource, const String& rTableOrQuery, sal_Int32 nCommandType)
{
    SwDBData aData;
    aData.sDataSource = rDataSource;
    aData.sCommand = rTableOrQuery;
    aData.nCommandType = nCommandType;

    SwDSParam* pFound = FindDSData(aData, TRUE);
    Reference< XDataSource> xSource;
    if(pFound->xResultSet.is())
        return TRUE;
    SwDSParam* pParam = FindDSConnection(rDataSource, FALSE);
    Reference< XConnection> xConnection;
    if(pParam && pParam->xConnection.is())
        pFound->xConnection = pParam->xConnection;
    else
        pFound->xConnection = SwNewDBMgr::GetConnection(rDataSource, xSource );
    if(pFound->xConnection.is())
    {
        try
        {
            Reference< sdbc::XDatabaseMetaData >  xMetaData = pFound->xConnection->getMetaData();
            pFound->bScrollable = xMetaData
                        ->supportsResultSetType((sal_Int32)ResultSetType::SCROLL_INSENSITIVE);
            pFound->xStatement = pFound->xConnection->createStatement();
            rtl::OUString aQuoteChar = xMetaData->getIdentifierQuoteString();
            rtl::OUString sStatement(C2U("SELECT * FROM "));
            sStatement = C2U("SELECT * FROM ");
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
        catch (Exception&)
        {
            pFound->xResultSet = 0;
            pFound->xStatement = 0;
            pFound->xConnection = 0;
        }
    }
    return pFound->xResultSet.is();
}
/* -----------------------------14.08.2001 10:26------------------------------

 ---------------------------------------------------------------------------*/
Reference< XConnection> SwNewDBMgr::RegisterConnection(rtl::OUString& rDataSource)
{
    SwDSParam* pFound = SwNewDBMgr::FindDSConnection(rDataSource, TRUE);
    Reference< XDataSource> xSource;
    if(!pFound->xConnection.is())
        pFound->xConnection = SwNewDBMgr::GetConnection(rDataSource, xSource );
    return pFound->xConnection;
}
/* -----------------------------17.07.00 15:55--------------------------------

 ---------------------------------------------------------------------------*/
sal_uInt32      SwNewDBMgr::GetSelectedRecordId(
    const String& rDataSource, const String& rTableOrQuery, sal_Int32 nCommandType)
{
    sal_uInt32 nRet = -1;
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
        SwDSParam* pFound = FindDSData(aData, FALSE);
        if(pFound && pFound->xResultSet.is())
        {
            try
            {
                nRet = pFound->xResultSet->getRow();
            }
            catch(Exception&){}
        }
    }
    return nRet;
}

/* -----------------------------17.07.00 14:18--------------------------------
    close all data sources - after fields were updated
 ---------------------------------------------------------------------------*/
void    SwNewDBMgr::CloseAll(BOOL bIncludingMerge)
{
    //do nothing - keep all connections until the destructor
//    for(USHORT nPos = 0; nPos < aDataSourceParams.Count(); nPos++)
//    {
//        SwDSParam* pParam = aDataSourceParams[nPos];
//        if(bIncludingMerge || pParam != pImpl->pMergeData)
//        {
//            pParam->xResultSet = 0;
//            pParam->xStatement = 0;
//            pParam->xConnection = 0;
//        }
//    }
}
/* -----------------------------17.07.00 14:54--------------------------------

 ---------------------------------------------------------------------------*/
SwDSParam* SwNewDBMgr::FindDSData(const SwDBData& rData, BOOL bCreate)
{
    SwDSParam* pFound = 0;
    for(USHORT nPos = 0; nPos < aDataSourceParams.Count(); nPos++)
    {
        SwDSParam* pParam = aDataSourceParams[nPos];
        if(rData.sDataSource == pParam->sDataSource &&
            rData.sCommand == pParam->sCommand &&
            (rData.nCommandType == -1 || rData.nCommandType == pParam->nCommandType))
            {
                pFound = pParam;
                break;
            }
    }
    if(bCreate)
    {
        if(!pFound)
        {
            pFound = new SwDSParam(rData);
            aDataSourceParams.Insert(pFound, aDataSourceParams.Count());
        }
    }
    return pFound;
}
/* -----------------------------14.08.2001 10:27------------------------------

 ---------------------------------------------------------------------------*/
SwDSParam*  SwNewDBMgr::FindDSConnection(const rtl::OUString& rDataSource, BOOL bCreate)
{
    SwDSParam* pFound = 0;
    for(USHORT nPos = 0; nPos < aDataSourceParams.Count(); nPos++)
    {
        SwDSParam* pParam = aDataSourceParams[nPos];
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
        aDataSourceParams.Insert(pFound, aDataSourceParams.Count());
    }
    return pFound;
}

/* -----------------------------17.07.00 14:31--------------------------------
    rDBName: <Source> + DB_DELIM + <Table>; + <Statement>
 ---------------------------------------------------------------------------*/
void    SwNewDBMgr::AddDSData(const SwDBData& rData, long nSelStart, long nSelEnd)
{
    SwDSParam* pFound = FindDSData(rData, TRUE);
    if(nSelStart > 0)
    {
        if(nSelEnd < nSelStart)
        {
            sal_uInt32 nZw = nSelEnd;
            nSelEnd = nSelStart;
            nSelStart = nZw;
        }

        pFound->aSelection.realloc(nSelEnd - nSelStart + 1);
        sal_Int32* pSelection = pFound->aSelection.getArray();
        sal_Int32 nIdx = 0;
        for (long i = nSelStart; i <= nSelEnd; i++, nIdx++)
            pSelection[nIdx] = i;
    }
    else
        pFound->aSelection.realloc(0);
}
/* -----------------------------17.07.00 14:31--------------------------------

 ---------------------------------------------------------------------------*/
void    SwNewDBMgr::GetDSSelection(const SwDBData& rData, long& rSelStart, long& rSelEnd)
{
    SwDSParam* pFound = FindDSData(rData, FALSE);
    if(!pFound || !pFound->aSelection.getLength())
        rSelStart = -1L;
    else
    {
        rSelStart = pFound->aSelection.getConstArray()[0];
        rSelEnd = pFound->aSelection.getConstArray()[pFound->aSelection.getLength() - 1];
    }
}
/* -----------------------------17.07.00 14:34--------------------------------

 ---------------------------------------------------------------------------*/
const SwDBData& SwNewDBMgr::GetAddressDBName()
{
    return SW_MOD()->GetDBConfig()->GetAddressSource();
}
/* -----------------------------18.07.00 13:13--------------------------------

 ---------------------------------------------------------------------------*/
Sequence<rtl::OUString> SwNewDBMgr::GetExistingDatabaseNames()
{
    Reference<XNameAccess> xDBContext;
    Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
    if( xMgr.is() )
    {
        Reference<XInterface> xInstance = xMgr->createInstance( C2U( "com.sun.star.sdb.DatabaseContext" ));
        xDBContext = Reference<XNameAccess>(xInstance, UNO_QUERY) ;
    }
    if(xDBContext.is())
    {
        return xDBContext->getElementNames();
    }
    return Sequence<rtl::OUString>();
}
/* -----------------------------10.11.00 17:10--------------------------------

 ---------------------------------------------------------------------------*/
void SwNewDBMgr::ExecuteFormLetter( SwWrtShell& rSh,
                        const Sequence<PropertyValue>& rProperties)
{
    //prevent second call
    if(pImpl->pMergeDialog)
        return ;
    rtl::OUString sDataSource, sDataTableOrQuery;
    Sequence<Any> aSelection;
    BOOL bHasSelectionProperty = FALSE;
    sal_Int32 nSelectionPos = 0;
    sal_Int32 nResultSetIdx = -1;
    sal_Int16 nCmdType = CommandType::TABLE;
    const PropertyValue* pValues = rProperties.getConstArray();
    Reference< XConnection> xConnection;
    for(sal_Int32 nPos = 0; nPos < rProperties.getLength(); nPos++)
    {
        if(pValues[nPos].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cDataSourceName)))
            pValues[nPos].Value >>= sDataSource;
        else if(pValues[nPos].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cCommand)))
            pValues[nPos].Value >>= sDataTableOrQuery;
        else if(pValues[nPos].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cSelection)))
        {
            bHasSelectionProperty = TRUE;
            nSelectionPos = nPos;
            pValues[nPos].Value >>= aSelection;
        }
        else if(pValues[nPos].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cCommandType)))
            pValues[nPos].Value >>= nCmdType;
        else if(pValues[nPos].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cCursor)))
            nResultSetIdx = nPos;
        else if(pValues[nPos].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cActiveConnection)))
            pValues[nPos].Value >>= xConnection;
    }
    if(!sDataSource.getLength() || !sDataTableOrQuery.getLength())
    {
        DBG_ERROR("PropertyValues missing or unset")
        return;
    }
    Sequence<sal_Int32> aDlgSelection(aSelection.getLength());
    if(aSelection.getLength())
    {
        sal_Int32* pDlgSelection = aDlgSelection.getArray();
        const Any* pGridSelection = aSelection.getConstArray();
        for(sal_Int32 nSel = 0; nSel < aSelection.getLength(); nSel++)
            pGridSelection[nSel] >>= pDlgSelection[nSel];

    }
    pImpl->pMergeDialog = new SwMailMergeDlg(
                    &rSh.GetView().GetViewFrame()->GetWindow(), rSh,
                    sDataSource,
                    sDataTableOrQuery,
                    nCmdType,
                    xConnection,
                    bHasSelectionProperty ? &aDlgSelection : 0 );

    if(pImpl->pMergeDialog->Execute() == RET_OK)
    {
        SetMergeType(  pImpl->pMergeDialog->GetMergeType() );

        Sequence<PropertyValue> aNewProperties = rProperties;
        if(!bHasSelectionProperty)
        {
            nSelectionPos = rProperties.getLength();
            aNewProperties.realloc(rProperties.getLength() + 1);
            aNewProperties[nSelectionPos].Name = C2U("Selection");
        }
        aNewProperties[nSelectionPos].Value <<= pImpl->pMergeDialog->GetSelection();

        Reference<XResultSet> xResSet = pImpl->pMergeDialog->GetResultSet();
        if(xResSet.is())
        {
            if(nResultSetIdx < 0)
            {
                nResultSetIdx = aNewProperties.getLength();
                aNewProperties.realloc(nResultSetIdx + 1);
                aNewProperties[nResultSetIdx].Name = C2U("Cursor");
            }
            aNewProperties[nResultSetIdx].Value <<= xResSet;
        }
        OFF_APP()->NotifyEvent(SfxEventHint(SW_EVENT_MAIL_MERGE, rSh.GetView().GetViewFrame()->GetObjectShell()));
        MergeNew(GetMergeType(),
                            rSh,
                            aNewProperties);
    }
    DELETEZ(pImpl->pMergeDialog);
}
/* -----------------------------13.11.00 08:20--------------------------------

 ---------------------------------------------------------------------------*/
void SwNewDBMgr::InsertText(SwWrtShell& rSh,
                        const Sequence< PropertyValue>& rProperties)
{
    rtl::OUString sDataSource, sDataTableOrQuery;
    Reference<XResultSet>  xResSet;
    Sequence<Any> aSelection;
    BOOL bHasSelectionProperty = FALSE;
    sal_Int32 nSelectionPos = 0;
    sal_Int16 nCmdType = CommandType::TABLE;
    const PropertyValue* pValues = rProperties.getConstArray();
    Reference< XConnection> xConnection;
    for(sal_Int32 nPos = 0; nPos < rProperties.getLength(); nPos++)
    {
        if(pValues[nPos].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cDataSourceName)))
            pValues[nPos].Value >>= sDataSource;
        else if(pValues[nPos].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cCommand)))
            pValues[nPos].Value >>= sDataTableOrQuery;
        else if(pValues[nPos].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cCursor)))
            pValues[nPos].Value >>= xResSet;
        else if(pValues[nPos].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cSelection)))
        {
            bHasSelectionProperty = TRUE;
            nSelectionPos = nPos;
            pValues[nPos].Value >>= aSelection;
        }
        else if(pValues[nPos].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cCommandType)))
            pValues[nPos].Value >>= nCmdType;
        else if(pValues[nPos].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(cActiveConnection)))
            pValues[nPos].Value >>= xConnection;
    }
    if(!sDataSource.getLength() || !sDataTableOrQuery.getLength() || !xResSet.is())
    {
        DBG_ERROR("PropertyValues missing or unset")
        return;
    }
    Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
    Reference<XDataSource> xSource;
    Reference<XChild> xChild(xConnection, UNO_QUERY);
    if(xChild.is())
        xSource = Reference<XDataSource>(xChild->getParent(), UNO_QUERY);
    if(!xSource.is())
        xSource = GetDbtoolsClient().getDataSource(sDataSource, xMgr);
    Reference< XColumnsSupplier > xColSupp( xResSet, UNO_QUERY );
    SwDBData aDBData;
    aDBData.sDataSource = sDataSource;
    aDBData.sCommand = sDataTableOrQuery;
    aDBData.nCommandType = nCmdType;

    SwInsertDBColAutoPilot *pDlg = new SwInsertDBColAutoPilot(
            rSh.GetView(),
            xSource,
            xColSupp,
            aDBData );
    if( RET_OK == pDlg->Execute() )
    {
        rtl::OUString sDummy;
        if(!xConnection.is())
            xConnection = xSource->getConnection(sDummy, sDummy);
        try
        {
            pDlg->DataToDoc( aSelection , xSource, xConnection, xResSet);
        }
        catch(Exception& )
        {
            DBG_ERROR("exception caught")
        }
    }
    delete pDlg;

}
/* -----------------------------30.08.2001 12:00------------------------------

 ---------------------------------------------------------------------------*/
SwDbtoolsClient& SwNewDBMgr::GetDbtoolsClient()
{
    if(!pImpl->pDbtoolsClient)
        pImpl->pDbtoolsClient = new SwDbtoolsClient;
    return *pImpl->pDbtoolsClient;
}
