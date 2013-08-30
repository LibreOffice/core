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

#include "dbinsdlg.hxx"

#include <memory>

#include <float.h>

#include <hintids.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdb/XDatabaseAccess.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <comphelper/processfactory.hxx>
#include <editeng/langitem.hxx>
#include <svl/numuno.hxx>
#include <svl/stritem.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <svl/style.hxx>
#include <svl/zformat.hxx>
#include <sfx2/htmlmode.hxx>
#include <editeng/unolingu.hxx>
#include <sfx2/app.hxx>
#include <svl/itemset.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/boxitem.hxx>
#include <svx/rulritem.hxx>
#include <swdbtoolsclient.hxx>
#include <tabledlg.hxx>
#include <fmtclds.hxx>
#include <tabcol.hxx>
#include <uiitems.hxx>
#include <viewopt.hxx>
#include <uitool.hxx>
#include <wrtsh.hxx>
#include <wview.hxx>
#include <docsh.hxx>
#include <dbmgr.hxx>
#include <tblafmt.hxx>
#include <cellatr.hxx>
#include <swtable.hxx>
#include <dbfld.hxx>
#include <fmtcol.hxx>
#include <section.hxx>
#include <swwait.hxx>
#include <modcfg.hxx>
#include <swmodule.hxx>
#include <poolfmt.hxx>
#include <crsskip.hxx>

#include <dbui.hrc>

#include <cmdid.h>
#include <helpid.h>
#include <cfgid.h>
#include <SwStyleNameMapper.hxx>
#include <comphelper/uno3.hxx>
#include "tabsh.hxx"
#include "swabstdlg.hxx"
#include "table.hrc"
#include <unomid.h>
#include <IDocumentMarkAccess.hxx>

namespace swui
{
    SwAbstractDialogFactory * GetFactory();
}

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::uno;

const char cDBFldStart  = '<';
const char cDBFldEnd    = '>';

// Helper structure for adding database rows as fields or text
struct _DB_Column
{
    enum ColType { DB_FILLTEXT, DB_COL_FIELD, DB_COL_TEXT, DB_SPLITPARA } eColType;

    union {
        String* pText;
        SwField* pField;
        sal_uLong nFormat;
    } DB_ColumnData;
    const SwInsDBColumn* pColInfo;

    _DB_Column()
    {
        pColInfo = 0;
        DB_ColumnData.pText = 0;
        eColType = DB_SPLITPARA;
    }

    _DB_Column( const String& rTxt )
    {
        pColInfo = 0;
        DB_ColumnData.pText = new String( rTxt );
        eColType = DB_FILLTEXT;
    }

    _DB_Column( const SwInsDBColumn& rInfo, sal_uLong nFormat )
    {
        pColInfo = &rInfo;
        DB_ColumnData.nFormat = nFormat;
        eColType = DB_COL_TEXT;
    }

    _DB_Column( const SwInsDBColumn& rInfo, SwDBField& rFld )
    {
        pColInfo = &rInfo;
        DB_ColumnData.pField = &rFld;
        eColType = DB_COL_FIELD;
    }

    ~_DB_Column()
    {
        if( DB_COL_FIELD == eColType )
            delete DB_ColumnData.pField;
        else if( DB_FILLTEXT == eColType )
            delete DB_ColumnData.pText;
    }
};


struct _DB_ColumnConfigData
{
    SwInsDBColumns aDBColumns;
    OUString sSource, sTable, sEdit, sTblList, sTmplNm, sTAutoFmtNm;
    sal_Bool bIsTable : 1,
         bIsField : 1,
         bIsHeadlineOn : 1,
         bIsEmptyHeadln : 1;

    _DB_ColumnConfigData()
    {
        bIsTable = bIsHeadlineOn = sal_True;
        bIsField = bIsEmptyHeadln = sal_False;
    }

    ~_DB_ColumnConfigData();
private:
    _DB_ColumnConfigData( const _DB_ColumnConfigData& );
    _DB_ColumnConfigData& operator =( const _DB_ColumnConfigData& );
};

int SwInsDBColumn::operator<( const SwInsDBColumn& rCmp ) const
{
    return 0 > GetAppCollator().compareString( sColumn, rCmp.sColumn );
}

SwInsertDBColAutoPilot::SwInsertDBColAutoPilot( SwView& rView,
        Reference<XDataSource> xDataSource,
        Reference<sdbcx::XColumnsSupplier> xColSupp,
        const SwDBData& rData )
    : SfxModalDialog(rView.GetWindow(), "InsertDbColumnsDialog",
        "modules/swriter/ui/insertdbcolumnsdialog.ui")
    , ConfigItem("Office.Writer/InsertData/DataSet",
        CONFIG_MODE_DELAYED_UPDATE)
    , aDBData(rData)
    , sNoTmpl(SW_RESSTR(SW_STR_NONE))
    , pView(&rView)
    , pTAutoFmt(0)
    , pTblSet(0)
    , pRep(0)
{
    get(m_pRbAsTable, "astable");
    get(m_pRbAsField, "asfields");
    get(m_pRbAsText, "astext");
    get(m_pHeadFrame, "dbframe");
    get(m_pLbTblDbColumn, "tabledbcols");
    get(m_pLbTxtDbColumn, "tabletxtcols");
    m_pLbTblDbColumn->SetDropDownLineCount(8);
    m_pLbTxtDbColumn->SetDropDownLineCount(8);
    get(m_pFormatFrame, "formatframe");
    get(m_pRbDbFmtFromDb, "fromdatabase");
    get(m_pRbDbFmtFromUsr, "userdefined");
    get(m_pLbDbFmtFromUsr, "numformat");
    aOldNumFmtLnk = m_pLbDbFmtFromUsr->GetSelectHdl();
    get(m_pIbDbcolToEdit, "toedit");
    get(m_pEdDbText, "textview");
    m_pEdDbText->set_width_request(m_pEdDbText->approximate_char_width() * 46);
    get(m_pFtDbParaColl, "parastylelabel");
    get(m_pLbDbParaColl, "parastyle");
    m_pLbDbParaColl->SetStyle(m_pLbDbParaColl->GetStyle() | WB_SORT);
    get(m_pIbDbcolAllTo, "oneright");
    get(m_pIbDbcolOneTo, "allright");
    get(m_pIbDbcolOneFrom, "oneleft");
    get(m_pIbDbcolAllFrom, "allleft");
    get(m_pFtTableCol, "tablecolft");
    get(m_pLbTableCol, "tablecols");
    m_pLbTableCol->SetDropDownLineCount(8);
    get(m_pCbTableHeadon, "tableheading");
    get(m_pRbHeadlColnms, "columnname");
    get(m_pRbHeadlEmpty, "rowonly");
    get(m_pPbTblFormat, "tableformat");
    get(m_pPbTblAutofmt, "autoformat");

    nGBFmtLen = m_pFormatFrame->get_label().getLength();

    if(xColSupp.is())
    {
        SwWrtShell& rSh = pView->GetWrtShell();
        SvNumberFormatter* pNumFmtr = rSh.GetNumberFormatter();
        SvNumberFormatsSupplierObj* pNumFmt = new SvNumberFormatsSupplierObj( pNumFmtr );
        Reference< util::XNumberFormatsSupplier >  xDocNumFmtsSupplier = pNumFmt;
        Reference< util::XNumberFormats > xDocNumberFormats = xDocNumFmtsSupplier->getNumberFormats();
        Reference< util::XNumberFormatTypes > xDocNumberFormatTypes(xDocNumberFormats, UNO_QUERY);

        Reference<XPropertySet> xSourceProps(xDataSource, UNO_QUERY);
        Reference< util::XNumberFormats > xNumberFormats;
        if(xSourceProps.is())
        {
            Any aFormats = xSourceProps->getPropertyValue("NumberFormatsSupplier");
            if(aFormats.hasValue())
            {
                Reference< util::XNumberFormatsSupplier> xSuppl;
                aFormats >>= xSuppl;
                if(xSuppl.is())
                {
                    xNumberFormats = xSuppl->getNumberFormats(  );
                }
            }
        }
        Reference <XNameAccess> xCols = xColSupp->getColumns();
        Sequence<OUString> aColNames = xCols->getElementNames();
        const OUString* pColNames = aColNames.getConstArray();
        long nCount = aColNames.getLength();
        for (long n = 0; n < nCount; n++)
        {
            SwInsDBColumn* pNew = new SwInsDBColumn( pColNames[n], (sal_uInt16)n );
            Any aCol = xCols->getByName(pColNames[n]);
            Reference <XPropertySet> xCol;
            aCol >>= xCol;
            Any aType = xCol->getPropertyValue("Type");
            sal_Int32 eDataType = 0;
            aType >>= eDataType;
            switch(eDataType)
            {
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
                    pNew->bHasFmt = sal_True;
                    Any aFormat = xCol->getPropertyValue("FormatKey");
                    if(aFormat.hasValue())
                    {
                        sal_Int32 nFmt = 0;
                        aFormat >>= nFmt;
                        if(xNumberFormats.is())
                        {
                            try
                            {
                                Reference<XPropertySet> xNumProps = xNumberFormats->getByKey( nFmt );
                                Any aFormatVal = xNumProps->getPropertyValue("FormatString");
                                Any aLocale = xNumProps->getPropertyValue("Locale");
                                OUString sFormat;
                                aFormatVal >>= sFormat;
                                lang::Locale aLoc;
                                aLocale >>= aLoc;
                                long nKey = xDocNumberFormats->queryKey( sFormat, aLoc, sal_True);
                                if(nKey < 0)
                                {
                                    nKey = xDocNumberFormats->addNew( sFormat, aLoc );
                                }
                                pNew->nDBNumFmt = nKey;
                            }
                            catch (const Exception&)
                            {
                                OSL_FAIL("illegal number format key");
                            }
                        }
                    }
                    else
                    {
                        pNew->nDBNumFmt = SwNewDBMgr::GetDbtoolsClient().getDefaultNumberFormat(xCol,
                                xDocNumberFormatTypes, LanguageTag( rSh.GetCurLang() ).getLocale());
                    }

                }
                break;
            }
            if( !aDBColumns.insert( pNew ).second )
            {
                OSL_ENSURE( !this, "Spaltenname mehrfach vergeben?" );
                delete pNew;
            }
        }
    }

    // fill paragraph templates-ListBox
    {
        SfxStyleSheetBasePool* pPool = pView->GetDocShell()->GetStyleSheetPool();
        pPool->SetSearchMask( SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_ALL );
        m_pLbDbParaColl->InsertEntry( sNoTmpl );

        const SfxStyleSheetBase* pBase = pPool->First();
        while( pBase )
        {
            m_pLbDbParaColl->InsertEntry( pBase->GetName() );
            pBase = pPool->Next();
        }
        m_pLbDbParaColl->SelectEntryPos( 0 );
    }

    // when the cursor is inside of a table, table must NEVER be selectable
    if( pView->GetWrtShell().GetTableFmt() )
    {
        m_pRbAsTable->Enable( sal_False );
        m_pRbAsField->Check( sal_True );
        m_pRbDbFmtFromDb->Check( sal_True );
    }
    else
    {
        m_pRbAsTable->Check( sal_True );
        m_pRbDbFmtFromDb->Check( sal_True );
        m_pIbDbcolOneFrom->Enable( sal_False );
        m_pIbDbcolAllFrom->Enable( sal_False );
    }

    m_pRbAsTable->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, PageHdl ));
    m_pRbAsField->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, PageHdl ));
    m_pRbAsText->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, PageHdl ));

    m_pRbDbFmtFromDb->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, DBFormatHdl ));
    m_pRbDbFmtFromUsr->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, DBFormatHdl ));

    m_pPbTblFormat->SetClickHdl(LINK(this, SwInsertDBColAutoPilot, TblFmtHdl ));
    m_pPbTblAutofmt->SetClickHdl(LINK(this, SwInsertDBColAutoPilot, AutoFmtHdl ));

    m_pIbDbcolAllTo->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, TblToFromHdl ));
    m_pIbDbcolOneTo->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, TblToFromHdl ));
    m_pIbDbcolOneFrom->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, TblToFromHdl ));
    m_pIbDbcolAllFrom->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, TblToFromHdl ));
    m_pIbDbcolToEdit->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, TblToFromHdl ));

    m_pCbTableHeadon->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, HeaderHdl ));
    m_pRbHeadlColnms->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, HeaderHdl ));
    m_pRbHeadlEmpty->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, HeaderHdl ));

    m_pLbTxtDbColumn->SetSelectHdl( LINK( this, SwInsertDBColAutoPilot, SelectHdl ));
    m_pLbTblDbColumn->SetSelectHdl( LINK( this, SwInsertDBColAutoPilot, SelectHdl ));
    m_pLbDbFmtFromUsr->SetSelectHdl( LINK( this, SwInsertDBColAutoPilot, SelectHdl ));
    m_pLbTableCol->SetSelectHdl( LINK( this, SwInsertDBColAutoPilot, SelectHdl ));

    m_pLbTxtDbColumn->SetDoubleClickHdl( LINK( this, SwInsertDBColAutoPilot, DblClickHdl ));
    m_pLbTblDbColumn->SetDoubleClickHdl( LINK( this, SwInsertDBColAutoPilot, DblClickHdl ));
    m_pLbTableCol->SetDoubleClickHdl( LINK( this, SwInsertDBColAutoPilot, DblClickHdl ));

    for( sal_uInt16 n = 0; n < aDBColumns.size(); ++n )
    {
        const String& rS = aDBColumns[ n ]->sColumn;
        m_pLbTblDbColumn->InsertEntry( rS, n );
        m_pLbTxtDbColumn->InsertEntry( rS, n );
    }
    m_pLbTxtDbColumn->SelectEntryPos( 0 );
    m_pLbTblDbColumn->SelectEntryPos( 0 );

    // read configuration
    Load();

    // lock size to widest config
    m_pHeadFrame->set_width_request(m_pHeadFrame->get_preferred_size().Width());
    // initialise Controls:
    PageHdl( m_pRbAsTable->IsChecked() ? m_pRbAsTable : m_pRbAsField );
}

SwInsertDBColAutoPilot::~SwInsertDBColAutoPilot()
{
    delete pTblSet;
    delete pRep;

    delete pTAutoFmt;
}

IMPL_LINK( SwInsertDBColAutoPilot, PageHdl, Button*, pButton )
{
    bool bShowTbl = pButton == m_pRbAsTable;

    OUString sTxt(pButton->GetText());
    m_pHeadFrame->set_label(MnemonicGenerator::EraseAllMnemonicChars(sTxt));

    m_pLbTxtDbColumn->Show( !bShowTbl );
    m_pIbDbcolToEdit->Show( !bShowTbl );
    m_pEdDbText->Show( !bShowTbl );
    m_pFtDbParaColl->Show( !bShowTbl );
    m_pLbDbParaColl->Show( !bShowTbl );

    m_pLbTblDbColumn->Show( bShowTbl );
    m_pIbDbcolAllTo->Show( bShowTbl );
    m_pIbDbcolOneTo->Show( bShowTbl );
    m_pIbDbcolOneFrom->Show( bShowTbl );
    m_pIbDbcolAllFrom->Show( bShowTbl );
    m_pFtTableCol->Show( bShowTbl );
    m_pLbTableCol->Show( bShowTbl );
    m_pCbTableHeadon->Show( bShowTbl );
    m_pRbHeadlColnms->Show( bShowTbl );
    m_pRbHeadlEmpty->Show( bShowTbl );
    m_pPbTblFormat->Show( bShowTbl );
    m_pPbTblAutofmt->Show( bShowTbl );

    if( bShowTbl )
        m_pPbTblFormat->Enable( 0 != m_pLbTableCol->GetEntryCount() );

    SelectHdl( bShowTbl ? m_pLbTblDbColumn : m_pLbTxtDbColumn );

    return 0;
}

IMPL_LINK( SwInsertDBColAutoPilot, DBFormatHdl, Button*, pButton )
{
    ListBox& rBox = m_pRbAsTable->IsChecked()
                        ? ( 0 == m_pLbTableCol->GetEntryData( 0 )
                            ? *m_pLbTblDbColumn
                            : *m_pLbTableCol )
                        : *m_pLbTxtDbColumn;

    SwInsDBColumn aSrch( rBox.GetSelectEntry(), 0 );
    SwInsDBColumns::const_iterator it = aDBColumns.find( &aSrch );

    sal_Bool bFromDB = m_pRbDbFmtFromDb == pButton;
    (*it)->bIsDBFmt = bFromDB;
    m_pLbDbFmtFromUsr->Enable( !bFromDB );

    return 0;
}

IMPL_LINK( SwInsertDBColAutoPilot, TblToFromHdl, Button*, pButton )
{
    sal_Bool bChgEnable = sal_True, bEnableTo = sal_True, bEnableFrom = sal_True;
    m_pLbTblDbColumn->SetUpdateMode( sal_False );
    m_pLbTableCol->SetUpdateMode( sal_False );

    if( pButton == m_pIbDbcolAllTo )
    {
        bEnableTo = sal_False;

        sal_uInt16 n, nInsPos = m_pLbTableCol->GetSelectEntryPos(),
               nCnt = m_pLbTblDbColumn->GetEntryCount();
        if( LISTBOX_APPEND == nInsPos )
            for( n = 0; n < nCnt; ++n )
                m_pLbTableCol->InsertEntry( m_pLbTblDbColumn->GetEntry( n ),
                                            LISTBOX_APPEND );
        else
            for( n = 0; n < nCnt; ++n, ++nInsPos )
                m_pLbTableCol->InsertEntry( m_pLbTblDbColumn->GetEntry( n ), nInsPos );
        m_pLbTblDbColumn->Clear();
        m_pLbTableCol->SelectEntryPos( nInsPos );
        m_pLbTblDbColumn->SelectEntryPos( LISTBOX_APPEND );
    }
    else if( pButton == m_pIbDbcolOneTo &&
            LISTBOX_ENTRY_NOTFOUND != m_pLbTblDbColumn->GetSelectEntryPos() )
    {
        sal_uInt16 nInsPos = m_pLbTableCol->GetSelectEntryPos(),
               nDelPos = m_pLbTblDbColumn->GetSelectEntryPos(),
               nTopPos = m_pLbTblDbColumn->GetTopEntry();
        m_pLbTableCol->InsertEntry( m_pLbTblDbColumn->GetEntry( nDelPos ), nInsPos );
        m_pLbTblDbColumn->RemoveEntry( nDelPos );

        m_pLbTableCol->SelectEntryPos( nInsPos );
        if( nDelPos >= m_pLbTblDbColumn->GetEntryCount() )
            nDelPos = m_pLbTblDbColumn->GetEntryCount() - 1;
        m_pLbTblDbColumn->SelectEntryPos( nDelPos );
        m_pLbTblDbColumn->SetTopEntry( nTopPos );

        bEnableTo = 0 != m_pLbTblDbColumn->GetEntryCount();
    }
    else if( pButton == m_pIbDbcolOneFrom )
    {
        if( LISTBOX_ENTRY_NOTFOUND != m_pLbTableCol->GetSelectEntryPos() )
        {
            sal_uInt16 nInsPos,
                    nDelPos = m_pLbTableCol->GetSelectEntryPos(),
                    nTopPos = m_pLbTableCol->GetTopEntry();

            // look for the right InsertPos!!
            SwInsDBColumn aSrch( m_pLbTableCol->GetEntry( nDelPos ), 0 );
            SwInsDBColumns::const_iterator it = aDBColumns.find( &aSrch );
            if( it == aDBColumns.begin() || (it+1) == aDBColumns.end() )
                nInsPos = it - aDBColumns.begin();
            else
            {
                nInsPos = LISTBOX_ENTRY_NOTFOUND;
                while( ++it != aDBColumns.end() &&
                        LISTBOX_ENTRY_NOTFOUND == (nInsPos = m_pLbTblDbColumn->
                        GetEntryPos( String( (*it)->sColumn ))) )
                    ;
            }

            m_pLbTblDbColumn->InsertEntry( aSrch.sColumn, nInsPos );
            m_pLbTableCol->RemoveEntry( nDelPos );

            if( nInsPos >= m_pLbTblDbColumn->GetEntryCount() )
                nInsPos = m_pLbTblDbColumn->GetEntryCount() - 1;
            m_pLbTblDbColumn->SelectEntryPos( nInsPos );

            if( nDelPos >= m_pLbTableCol->GetEntryCount() )
                nDelPos = m_pLbTableCol->GetEntryCount() - 1;
            m_pLbTableCol->SelectEntryPos( nDelPos );
            m_pLbTableCol->SetTopEntry( nTopPos );
        }
        else
            bEnableTo = 0 != m_pLbTblDbColumn->GetEntryCount();

        bEnableFrom = 0 != m_pLbTableCol->GetEntryCount();
    }
    else if( pButton == m_pIbDbcolAllFrom )
    {
        bEnableFrom = sal_False;

        m_pLbTblDbColumn->Clear();
        m_pLbTableCol->Clear();
        for( sal_uInt16 n = 0; n < aDBColumns.size(); ++n )
            m_pLbTblDbColumn->InsertEntry( aDBColumns[ n ]->sColumn, n );
        m_pLbTblDbColumn->SelectEntryPos( 0 );
    }
    else if( pButton == m_pIbDbcolToEdit )
    {
        bChgEnable = sal_False;
        // move data to Edit:
        String aFld( m_pLbTxtDbColumn->GetSelectEntry() );
        if( aFld.Len() )
        {
            String aStr( m_pEdDbText->GetText() );
            sal_uInt16 nPos = (sal_uInt16)m_pEdDbText->GetSelection().Min();
            sal_uInt16 nSel = sal_uInt16(m_pEdDbText->GetSelection().Max()) - nPos;
            if( nSel )
                // first delete the existing selection
                aStr.Erase( nPos, nSel );

            aFld.Insert( cDBFldStart, 0 );
            aFld += cDBFldEnd;
            if( aStr.Len() )
            {
                if( nPos )                          // one blank in front
                {
                    sal_Unicode c = aStr.GetChar( nPos-1 );
                    if( '\n' != c && '\r' != c )
                        aFld.Insert( ' ', 0 );
                }
                if( nPos < aStr.Len() )             // one blank behind
                {
                    sal_Unicode c = aStr.GetChar( nPos );
                    if( '\n' != c && '\r' != c )
                        aFld += ' ';
                }
            }

            aStr.Insert( aFld, nPos );
            m_pEdDbText->SetText( aStr );
            nPos = nPos + aFld.Len();
            m_pEdDbText->SetSelection( Selection( nPos ));
        }
    }

    if( bChgEnable )
    {
        m_pIbDbcolOneTo->Enable( bEnableTo );
        m_pIbDbcolAllTo->Enable( bEnableTo );
        m_pIbDbcolOneFrom->Enable( bEnableFrom );
        m_pIbDbcolAllFrom->Enable( bEnableFrom );

        m_pRbDbFmtFromDb->Enable( sal_False );
        m_pRbDbFmtFromUsr->Enable( sal_False );
        m_pLbDbFmtFromUsr->Enable( sal_False );

        m_pPbTblFormat->Enable( bEnableFrom );
    }
    m_pLbTblDbColumn->SetUpdateMode( sal_True );
    m_pLbTableCol->SetUpdateMode( sal_True );

    return 0;
}

IMPL_LINK( SwInsertDBColAutoPilot, DblClickHdl, ListBox*, pBox )
{
    Button* pButton = 0;
    if( pBox == m_pLbTxtDbColumn )
        pButton = m_pIbDbcolToEdit;
    else if( pBox == m_pLbTblDbColumn && m_pIbDbcolOneTo->IsEnabled() )
        pButton = m_pIbDbcolOneTo;
    else if( pBox == m_pLbTableCol && m_pIbDbcolOneFrom->IsEnabled() )
        pButton = m_pIbDbcolOneFrom;

    if( pButton )
        TblToFromHdl( pButton );

    return 0;
}

IMPL_LINK( SwInsertDBColAutoPilot, TblFmtHdl, PushButton*, pButton )
{
    SwWrtShell& rSh = pView->GetWrtShell();
    bool bNewSet = false;
    if( !pTblSet )
    {
        bNewSet = true;
        pTblSet = new SfxItemSet( rSh.GetAttrPool(), SwuiGetUITableAttrRange() );

        // At first acquire the simple attributes
        pTblSet->Put( SfxStringItem( FN_PARAM_TABLE_NAME, rSh.GetUniqueTblName() ));
        pTblSet->Put( SfxUInt16Item( FN_PARAM_TABLE_HEADLINE, 1 ) );

        pTblSet->Put( SfxUInt16Item( SID_BACKGRND_DESTINATION,
                                    rSh.GetViewOptions()->GetTblDest() ));

        SvxBrushItem aBrush( RES_BACKGROUND );
        pTblSet->Put( aBrush );
        pTblSet->Put( aBrush, SID_ATTR_BRUSH_ROW );
        pTblSet->Put( aBrush, SID_ATTR_BRUSH_TABLE );

        SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
            // table variant, when multiple table cells are selected
        aBoxInfo.SetTable( sal_True );
            // always show gap field
        aBoxInfo.SetDist( sal_True);
            // set minimum size in tables and paragraphs
        aBoxInfo.SetMinDist( sal_False );
            // always set default-gap
        aBoxInfo.SetDefDist( MIN_BORDER_DIST );
            // Single lines can have DontCare-status only in tables
        aBoxInfo.SetValid( VALID_DISABLE, sal_True );
        pTblSet->Put( aBoxInfo );

        SwGetCurColNumPara aPara;
        const sal_uInt16 nNum = rSh.GetCurColNum( &aPara );
        long nWidth;

        if( nNum )
        {
            nWidth = aPara.pPrtRect->Width();
            const SwFmtCol& rCol = aPara.pFrmFmt->GetCol();
            const SwColumns& rCols = rCol.GetColumns();

            // initialise nStart und nEnd for nNum == 0
            long nWidth1 = 0,
                nStart1 = 0,
                nEnd1 = nWidth;
            for( sal_uInt16 i = 0; i < nNum; ++i )
            {
                const SwColumn* pCol = &rCols[i];
                nStart1 = pCol->GetLeft() + nWidth1;
                nWidth1 += (long)rCol.CalcColWidth( i, (sal_uInt16)nWidth );
                nEnd1 = nWidth1 - pCol->GetRight();
            }
            if(nStart1 || nEnd1 != nWidth)
                nWidth = nEnd1 - nStart1;
        }
        else
            nWidth = rSh.GetAnyCurRect(
                                FRMTYPE_FLY_ANY & rSh.GetFrmType( 0, sal_True )
                                              ? RECT_FLY_PRT_EMBEDDED
                                              : RECT_PAGE_PRT ).Width();

        SwTabCols aTabCols;
        aTabCols.SetRight( nWidth );
        aTabCols.SetRightMax( nWidth );
        pRep = new SwTableRep( aTabCols );
        pRep->SetAlign( text::HoriOrientation::NONE );
        pRep->SetSpace( nWidth );
        pRep->SetWidth( nWidth );
        pRep->SetWidthPercent( 100 );
        pTblSet->Put( SwPtrItem( FN_TABLE_REP, pRep ));

        pTblSet->Put( SfxUInt16Item( SID_HTML_MODE,
                    ::GetHtmlMode( pView->GetDocShell() )));
    }

    if( m_pLbTableCol->GetEntryCount() != pRep->GetAllColCount() )
    {
        // Number of columns has changed: then the TabCols have to be adjusted
        long nWidth = pRep->GetWidth();
        sal_uInt16 nCols = m_pLbTableCol->GetEntryCount() - 1;
        SwTabCols aTabCols( nCols );
        aTabCols.SetRight( nWidth  );
        aTabCols.SetRightMax( nWidth );
        if( nCols )
            for( sal_uInt16 n = 0, nStep = (sal_uInt16)(nWidth / (nCols+1)), nW = nStep;
                    n < nCols; ++n, nW = nW + nStep )
            {
                aTabCols.Insert( nW, sal_False, n );
            }
        delete pRep;
        pRep = new SwTableRep( aTabCols );
        pRep->SetAlign( text::HoriOrientation::NONE );
        pRep->SetSpace( nWidth );
        pRep->SetWidth( nWidth );
        pRep->SetWidthPercent( 100 );
        pTblSet->Put( SwPtrItem( FN_TABLE_REP, pRep ));
    }

    SwAbstractDialogFactory* pFact = swui::GetFactory();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

    SfxAbstractTabDialog* pDlg = pFact->CreateSwTableTabDlg(pButton, rSh.GetAttrPool(), pTblSet, &rSh);
    OSL_ENSURE(pDlg, "Dialogdiet fail!");
    if( RET_OK == pDlg->Execute() )
        pTblSet->Put( *pDlg->GetOutputItemSet() );
    else if( bNewSet )
    {
        delete pTblSet, pTblSet = 0;
        delete pRep, pRep = 0;
    }
    delete pDlg;

    return 0;
}

IMPL_LINK( SwInsertDBColAutoPilot, AutoFmtHdl, PushButton*, pButton )
{
    SwAbstractDialogFactory* pFact = swui::GetFactory();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

    AbstractSwAutoFormatDlg* pDlg = pFact->CreateSwAutoFormatDlg(pButton, pView->GetWrtShellPtr(), sal_False, pTAutoFmt);
    OSL_ENSURE(pDlg, "Dialogdiet fail!");
    if( RET_OK == pDlg->Execute())
        pDlg->FillAutoFmtOfIndex( pTAutoFmt );
    delete pDlg;
    return 0;
}

IMPL_LINK( SwInsertDBColAutoPilot, SelectHdl, ListBox*, pBox )
{
    ListBox* pGetBox = pBox == m_pLbDbFmtFromUsr
                            ? ( m_pRbAsTable->IsChecked()
                                    ? ( 0 == m_pLbTableCol->GetEntryData( 0 )
                                        ? m_pLbTblDbColumn
                                        : m_pLbTableCol )
                                    : m_pLbTxtDbColumn )
                            : pBox;

    SwInsDBColumn aSrch( pGetBox->GetSelectEntry(), 0 );
    SwInsDBColumns::const_iterator it = aDBColumns.find( &aSrch );

    if( pBox == m_pLbDbFmtFromUsr )
    {
        if( !aSrch.sColumn.isEmpty() )
        {
            aOldNumFmtLnk.Call( pBox );
            (*it)->nUsrNumFmt = m_pLbDbFmtFromUsr->GetFormat();
        }
    }
    else
    {
        // set the selected FieldName at the FormatGroupBox, so that
        // it's clear what field is configured by the format!
        String sTxt( m_pFormatFrame->get_label().copy( 0, nGBFmtLen ));
        if( aSrch.sColumn.isEmpty() )
        {
            m_pRbDbFmtFromDb->Enable( sal_False );
            m_pRbDbFmtFromUsr->Enable( sal_False );
            m_pLbDbFmtFromUsr->Enable( sal_False );
        }
        else
        {
            sal_Bool bEnableFmt = (*it)->bHasFmt;
            m_pRbDbFmtFromDb->Enable( bEnableFmt );
            m_pRbDbFmtFromUsr->Enable( bEnableFmt );

            if( bEnableFmt )
            {
                (( sTxt += OUString(" (" )) += String(aSrch.sColumn) ) += (sal_Unicode)')';
            }

            sal_Bool bIsDBFmt = (*it)->bIsDBFmt;
            m_pRbDbFmtFromDb->Check( bIsDBFmt );
            m_pRbDbFmtFromUsr->Check( !bIsDBFmt );
            m_pLbDbFmtFromUsr->Enable( !bIsDBFmt );
            if( !bIsDBFmt )
                m_pLbDbFmtFromUsr->SetDefFormat( (*it)->nUsrNumFmt );
        }

        m_pFormatFrame->set_label(sTxt);

        // to know later on, what ListBox was the "active", a Flag
        // is remembered in the 1st entry
        void* pPtr = pBox == m_pLbTableCol ? m_pLbTableCol : 0;
        m_pLbTableCol->SetEntryData( 0, pPtr );
    }
    return 0;
}

IMPL_LINK( SwInsertDBColAutoPilot, HeaderHdl, Button*, pButton )
{
    if( pButton == m_pCbTableHeadon )
    {
        sal_Bool bEnable = m_pCbTableHeadon->IsChecked();

        m_pRbHeadlColnms->Enable( bEnable );
        m_pRbHeadlEmpty->Enable( bEnable );
    }
    return 0;
}

static void lcl_InsTextInArr( const String& rTxt, _DB_Columns& rColArr )
{
    _DB_Column* pNew;
    sal_uInt16 nSttPos = 0, nFndPos;
    while( STRING_NOTFOUND != ( nFndPos = rTxt.Search( '\x0A', nSttPos )) )
    {
        if( 1 < nFndPos )
        {
            pNew = new _DB_Column( rTxt.Copy( nSttPos, nFndPos -1 ) );
            rColArr.push_back( pNew );
        }
        pNew = new _DB_Column;
        rColArr.push_back( pNew );
        nSttPos = nFndPos + 1;
    }
    if( nSttPos < rTxt.Len() )
    {
        pNew = new _DB_Column( rTxt.Copy( nSttPos ) );
        rColArr.push_back( pNew );
    }
}

bool SwInsertDBColAutoPilot::SplitTextToColArr( const String& rTxt,
                                _DB_Columns& rColArr,
                                sal_Bool bInsField )
{
    // create each of the database columns from the text again
    // and then save in an array
    // database columns are in <> and must be present in the columns' array:
    String sTxt( rTxt );
    sal_uInt16 nFndPos, nEndPos, nSttPos = 0;

    while( STRING_NOTFOUND != ( nFndPos = sTxt.Search( cDBFldStart, nSttPos )))
    {
        nSttPos = nFndPos + 1;
        if( STRING_NOTFOUND != ( nEndPos = sTxt.Search( cDBFldEnd, nSttPos+1 )))
        {
            // Text in <> brackets found: what is it:
            SwInsDBColumn aSrch( sTxt.Copy( nSttPos, nEndPos - nSttPos ), 0);
            SwInsDBColumns::const_iterator it = aDBColumns.find( &aSrch );
            if( it != aDBColumns.end() )
            {
                // that is a valid field
                // so surely the text "before":
                const SwInsDBColumn& rFndCol = **it;

                _DB_Column* pNew;

                if( 1 < nSttPos )
                {
                    ::lcl_InsTextInArr( sTxt.Copy( 0, nSttPos-1 ), rColArr );
                    sTxt.Erase( 0, nSttPos-1 );
                }

                sTxt.Erase( 0, (xub_StrLen)(rFndCol.sColumn.getLength() + 2) );
                nSttPos = 0;

                sal_uInt16 nSubType = 0;
                sal_uLong nFormat;
                if( rFndCol.bHasFmt )
                {
                    if( rFndCol.bIsDBFmt )
                        nFormat =  rFndCol.nDBNumFmt;
                    else
                    {
                        nFormat = rFndCol.nUsrNumFmt;
                        nSubType = nsSwExtendedSubType::SUB_OWN_FMT;
                    }
                }
                else
                    nFormat = 0;

                if( bInsField )
                {
                    SwWrtShell& rSh = pView->GetWrtShell();
                    SwDBFieldType aFldType( rSh.GetDoc(), aSrch.sColumn,
                                            aDBData );
                    pNew = new _DB_Column( rFndCol, *new SwDBField(
                            (SwDBFieldType*)rSh.InsertFldType( aFldType ),
                                                            nFormat ) );
                    if( nSubType )
                        pNew->DB_ColumnData.pField->SetSubType( nSubType );
                }
                else
                    pNew = new _DB_Column( rFndCol, nFormat );

                rColArr.push_back( pNew );
            }
        }
    }

    // don't forget the last text
    if( sTxt.Len() )
        ::lcl_InsTextInArr( sTxt, rColArr );

    return !rColArr.empty();
}

void SwInsertDBColAutoPilot::DataToDoc( const Sequence<Any>& rSelection,
    Reference< XDataSource> xSource,
    Reference< XConnection> xConnection,
    Reference< sdbc::XResultSet > xResultSet )
{
    const Any* pSelection = rSelection.getLength() ? rSelection.getConstArray() : 0;
    SwWrtShell& rSh = pView->GetWrtShell();

    //with the drag and drop interface no result set is initially available
    sal_Bool bDisposeResultSet = sal_False;
    // we don't have a cursor, so we have to create our own RowSet
    if ( !xResultSet.is() )
    {
        xResultSet = SwNewDBMgr::createCursor(aDBData.sDataSource,aDBData.sCommand,aDBData.nCommandType,xConnection);
        bDisposeResultSet = xResultSet.is();
    }

    Reference< sdbc::XRow > xRow(xResultSet, UNO_QUERY);
    if ( !xRow.is() )
        return;

    rSh.StartAllAction();
    sal_Bool bUndo = rSh.DoesUndo();
    if( bUndo )
        rSh.StartUndo( UNDO_EMPTY );

    sal_Bool bAsTable = m_pRbAsTable->IsChecked();
    SvNumberFormatter& rNumFmtr = *rSh.GetNumberFormatter();

    if( rSh.HasSelection() )
        rSh.DelRight();

    ::std::auto_ptr<SwWait> pWait;

    Reference< XColumnsSupplier > xColsSupp( xResultSet, UNO_QUERY );
    Reference <XNameAccess> xCols = xColsSupp->getColumns();

    do{                                 // middle checked loop!!
    if( bAsTable )          // Daten als Tabelle einfuegen
    {
        rSh.DoUndo( sal_False );

        sal_uInt16 n, nRows = 0, nCols = m_pLbTableCol->GetEntryCount();
        if( m_pCbTableHeadon->IsChecked() )
            nRows++;

        if( pSelection )
            nRows = nRows + (sal_uInt16)rSelection.getLength();
        else
            ++nRows;

        // prepare the array for the selected columns
        std::vector<SwInsDBColumn*> aColFlds;
        for( n = 0; n < nCols; ++n )
        {
            SwInsDBColumn aSrch( m_pLbTableCol->GetEntry( n ), 0 );
            SwInsDBColumns::const_iterator it = aDBColumns.find( &aSrch );
            if (it != aDBColumns.end())
                aColFlds.push_back(*it);
            else {
                OSL_ENSURE( !this, "database column not found" );
            }
        }

        if( nCols != aColFlds.size() )
        {
            OSL_ENSURE( !this, "not all database columns found" );
            nCols = aColFlds.size();
        }

        if(!nRows || !nCols)
        {
            OSL_ENSURE( !this, "wrong parameters" );
            break;
        }

        const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

        sal_Bool bHTML = 0 != (::GetHtmlMode( pView->GetDocShell() ) & HTMLMODE_ON);
        rSh.InsertTable(
            pModOpt->GetInsTblFlags(bHTML),
            nRows, nCols, text::HoriOrientation::FULL, (pSelection ? pTAutoFmt : 0) );
        rSh.MoveTable( GetfnTablePrev(), GetfnTableStart() );

        if( pSelection && pTblSet )
            SetTabSet();

        SfxItemSet aTblSet( rSh.GetAttrPool(), RES_BOXATR_FORMAT,
                                                RES_BOXATR_VALUE );
        sal_Bool bIsAutoUpdateCells = rSh.IsAutoUpdateCells();
        rSh.SetAutoUpdateCells( sal_False );


        if( m_pCbTableHeadon->IsChecked() )
        {
            for( n = 0; n < nCols; ++n )
            {
                if( m_pRbHeadlColnms->IsChecked() )
                {
                    rSh.SwEditShell::Insert2( aColFlds[ n ]->sColumn );
                }
                rSh.GoNextCell();
            }
        }
        else
            rSh.SetRowsToRepeat( 0 );

        for( sal_Int32 i = 0 ; ; ++i )
        {
            bool bBreak = false;
            try
            {
                if(pSelection)
                {
                    sal_Int32 nPos = 0;
                    pSelection[i] >>= nPos;
                    bBreak = !xResultSet->absolute(nPos);
                }
                else if(!i)
                    bBreak = !xResultSet->first();
            }
            catch (const Exception&)
            {
                bBreak = true;
            }
            if(bBreak)
                break;

            for( n = 0; n < nCols; ++n )
            {
                // at the very first time, NO GoNextCell, because we're
                // already in it. Also no GoNextCell after the Insert,
                // because an empty line is added at the end.
                if( i || n )
                    rSh.GoNextCell();

                const SwInsDBColumn* pEntry = aColFlds[ n ];

                Reference< XColumn > xColumn;
                xCols->getByName(pEntry->sColumn) >>= xColumn;
                Reference< XPropertySet > xColumnProps( xColumn, UNO_QUERY );
                sal_Int32 eDataType = 0;
                if( xColumnProps.is() )
                {
                    Any aType = xColumnProps->getPropertyValue("Type");
                    aType >>= eDataType;
                }
                try
                {
                    if( pEntry->bHasFmt )
                    {
                        SwTblBoxNumFormat aNumFmt(
                                        pEntry->bIsDBFmt ? pEntry->nDBNumFmt
                                                         : pEntry->nUsrNumFmt );
                        aTblSet.Put(aNumFmt);
                        if( xColumn.is() )
                        {
                            double fVal = xColumn->getDouble();
                            if( xColumn->wasNull() )
                                aTblSet.ClearItem( RES_BOXATR_VALUE );
                            else
                            {
                                if(rNumFmtr.GetType(aNumFmt.GetValue()) & NUMBERFORMAT_DATE)
                                {
                                    ::Date aStandard(1,1,1900);
                                    if (*rNumFmtr.GetNullDate() != aStandard)
                                        fVal += (aStandard - *rNumFmtr.GetNullDate());
                                }
                                aTblSet.Put( SwTblBoxValue( fVal ));
                            }
                        }
                        else
                            aTblSet.ClearItem( RES_BOXATR_VALUE );
                        rSh.SetTblBoxFormulaAttrs( aTblSet );
                    }
                    //#i60207# don't insert binary data as string - creates a loop
                    else if( DataType::BINARY       == eDataType ||
                             DataType::VARBINARY    == eDataType ||
                             DataType::LONGVARBINARY== eDataType ||
                             DataType::SQLNULL      == eDataType ||
                             DataType::OTHER        == eDataType ||
                             DataType::OBJECT       == eDataType ||
                             DataType::DISTINCT     == eDataType ||
                             DataType::STRUCT       == eDataType ||
                             DataType::ARRAY        == eDataType ||
                             DataType::BLOB         == eDataType ||
                             DataType::CLOB         == eDataType ||
                             DataType::REF          == eDataType
                             )
                    {
                        // do nothing
                    }
                    else
                    {
                        OUString sVal =  xColumn->getString();
                        if(!xColumn->wasNull())
                        {
                            rSh.SwEditShell::Insert2( sVal );
                        }
                    }
                }
                catch (const Exception& rExcept)
                {
                    OSL_FAIL(OUStringToOString(rExcept.Message, osl_getThreadTextEncoding()).getStr());
                    (void)rExcept;
                }
            }

            if( !pSelection )
            {
                if ( !xResultSet->next() )
                    break;
            }
            else if( i+1 >= rSelection.getLength() )
                break;

            if( 10 == i )
                pWait = ::std::auto_ptr<SwWait>(new SwWait( *pView->GetDocShell(), sal_True ));
        }

        rSh.MoveTable( GetfnTableCurr(), GetfnTableStart() );
        if( !pSelection && ( pTblSet || pTAutoFmt ))
        {
            if( pTblSet )
                SetTabSet();

            if( pTAutoFmt )
                rSh.SetTableAutoFmt( *pTAutoFmt );
        }
        rSh.SetAutoUpdateCells( bIsAutoUpdateCells );
    }
    else                            // add data as fields/text
    {
        _DB_Columns aColArr;
        if( SplitTextToColArr( m_pEdDbText->GetText(), aColArr, m_pRbAsField->IsChecked() ) )
        {
            // now for each data set, we can iterate over the array
            // and add the data

            if( !rSh.IsSttPara() )
                rSh.SwEditShell::SplitNode();
            if( !rSh.IsEndPara() )
            {
                rSh.SwEditShell::SplitNode();
                rSh.SwCrsrShell::Left(1,CRSR_SKIP_CHARS);
            }

            rSh.DoUndo( sal_False );

            SwTxtFmtColl* pColl = 0;
            {
                String sTmplNm( m_pLbDbParaColl->GetSelectEntry() );
                if( sNoTmpl != sTmplNm )
                {
                    pColl = rSh.FindTxtFmtCollByName( sTmplNm );
                    if( !pColl )
                    {
                        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( sTmplNm, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                        if( USHRT_MAX != nId )
                            pColl = rSh.GetTxtCollFromPool( nId );
                        else
                            pColl = rSh.MakeTxtFmtColl( sTmplNm );
                    }
                    rSh.SetTxtFmtColl( pColl );
                }
            }

            // for adding as fields -> insert a "NextField" after
            // every data set
            SwDBFormatData aDBFormatData;
            Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
            aDBFormatData.xFormatter = Reference<util::XNumberFormatter>(util::NumberFormatter::create(xContext), UNO_QUERY_THROW) ;

            Reference<XPropertySet> xSourceProps(xSource, UNO_QUERY);
            if(xSourceProps.is())
            {
              Any aFormats = xSourceProps->getPropertyValue("NumberFormatsSupplier");
              if(aFormats.hasValue())
              {
                  Reference< util::XNumberFormatsSupplier> xSuppl;
                  aFormats >>= xSuppl;
                  if(xSuppl.is())
                  {
                        Reference< XPropertySet > xSettings = xSuppl->getNumberFormatSettings();
                        Any aNull = xSettings->getPropertyValue("NullDate");
                        aNull >>= aDBFormatData.aNullDate;
                        if(aDBFormatData.xFormatter.is())
                            aDBFormatData.xFormatter->attachNumberFormatsSupplier(xSuppl);
                  }
              }
            }
            aDBFormatData.aLocale = LanguageTag( rSh.GetCurLang() ).getLocale();
            SwDBNextSetField aNxtDBFld( (SwDBNextSetFieldType*)rSh.
                                        GetFldType( 0, RES_DBNEXTSETFLD ),
                                        OUString("1"), aEmptyStr, aDBData );


            bool bSetCrsr = true;
            sal_uInt16 n = 0, nCols = aColArr.size();
            ::sw::mark::IMark* pMark = NULL;
            for( sal_Int32 i = 0 ; ; ++i )
            {
                bool bBreak = false;
                try
                {
                    if(pSelection)
                    {
                        sal_Int32 nPos = 0;
                        pSelection[i] >>= nPos;
                        bBreak = !xResultSet->absolute(nPos);
                    }
                    else if(!i)
                        bBreak = !xResultSet->first();
                }
                catch (const Exception&)
                {
                    bBreak = true;
                }

                if(bBreak)
                    break;


                for( n = 0; n < nCols; ++n )
                {
                    _DB_Column* pDBCol = &aColArr[ n ];
                    OUString sIns;
                    switch( pDBCol->eColType )
                    {
                    case _DB_Column::DB_FILLTEXT:
                        sIns =  *pDBCol->DB_ColumnData.pText;
                        break;

                    case _DB_Column::DB_SPLITPARA:
                        rSh.SplitNode();
                        // when the template is not the same as the follow template,
                        // the selected has to be set newly
                        if( pColl && &pColl->GetNextTxtFmtColl() != pColl )
                            rSh.SetTxtFmtColl( pColl );
                        break;

                    case _DB_Column::DB_COL_FIELD:
                        {
                            SwDBField *const pFld = static_cast<SwDBField *>(
                                pDBCol->DB_ColumnData.pField->CopyField());
                            double nValue = DBL_MAX;

                            Reference< XPropertySet > xColumnProps;
                            xCols->getByName(pDBCol->pColInfo->sColumn) >>= xColumnProps;

                            pFld->SetExpansion( SwNewDBMgr::GetDBField(
                                                xColumnProps,
                                                aDBFormatData,
                                                &nValue ) );
                            if( DBL_MAX != nValue )
                            {
                                Any aType = xColumnProps->getPropertyValue("Type");
                                sal_Int32 eDataType = 0;
                                aType >>= eDataType;
                                if( DataType::DATE == eDataType  || DataType::TIME == eDataType  ||
                                    DataType::TIMESTAMP  == eDataType)

                                {
                                    ::Date aStandard(1,1,1900);
                                    ::Date aCompare(aDBFormatData.aNullDate.Day ,
                                                    aDBFormatData.aNullDate.Month,
                                                    aDBFormatData.aNullDate.Year);
                                    if(aStandard != aCompare)
                                        nValue += (aStandard - aCompare);
                                }
                                pFld->ChgValue( nValue, sal_True );
                            }
                            pFld->SetInitialized();

                            rSh.Insert( *pFld );
                            delete pFld;
                        }
                        break;

                    case _DB_Column::DB_COL_TEXT:
                        {
                            double nValue = DBL_MAX;
                            Reference< XPropertySet > xColumnProps;
                            xCols->getByName(pDBCol->pColInfo->sColumn) >>= xColumnProps;
                            sIns = SwNewDBMgr::GetDBField(
                                                xColumnProps,
                                                aDBFormatData,
                                                &nValue );
                            if( pDBCol->DB_ColumnData.nFormat &&
                                DBL_MAX != nValue )
                            {
                                Color* pCol;
                                if(rNumFmtr.GetType(pDBCol->DB_ColumnData.nFormat) & NUMBERFORMAT_DATE)
                                {
                                    ::Date aStandard(1,1,1900);
                                    if (*rNumFmtr.GetNullDate() != aStandard)
                                        nValue += (aStandard - *rNumFmtr.GetNullDate());
                                }
                                rNumFmtr.GetOutputString( nValue,
                                            pDBCol->DB_ColumnData.nFormat,
                                            sIns, &pCol );
                            }
                        }
                        break;
                    }

                    if( !sIns.isEmpty() )
                    {
                        rSh.Insert( sIns );

                        if( bSetCrsr)
                        {
                            // to the beginning and set a mark, so that
                            // the cursor can be set to the initial position
                            // at the end.

                            rSh.SwCrsrShell::MovePara(
                                    GetfnParaCurr(), GetfnParaStart() );
                            pMark = rSh.SetBookmark(
                                    KeyCode(),
                                    OUString(),
                                    OUString(), IDocumentMarkAccess::UNO_BOOKMARK );
                            rSh.SwCrsrShell::MovePara(
                                    GetfnParaCurr(), GetfnParaEnd() );
                            bSetCrsr = false;
                        }
                    }
                }

                if( !pSelection )
                {
                    sal_Bool bNext = xResultSet->next();
                    if(!bNext)
                        break;
                }
                else if( i+1 >= rSelection.getLength() )
                    break;

                if( m_pRbAsField->IsChecked() )
                    rSh.Insert( aNxtDBFld );

                if( !rSh.IsSttPara() )
                    rSh.SwEditShell::SplitNode();

                if( 10 == i )
                    pWait = ::std::auto_ptr<SwWait>(new SwWait( *pView->GetDocShell(), sal_True ));
            }

            if( !bSetCrsr && pMark != NULL)
            {
                rSh.SetMark();
                rSh.GotoMark( pMark );
                rSh.getIDocumentMarkAccess()->deleteMark( pMark );
                break;
            }
        }
    }
    // write configuration
    Commit();
    }while( false );                    // middle checked loop

    if( bUndo )
    {
        rSh.DoUndo( sal_True );
        rSh.AppendUndoForInsertFromDB( bAsTable );
        rSh.EndUndo( UNDO_EMPTY );
    }
    rSh.ClearMark();
    rSh.EndAllAction();

    if ( bDisposeResultSet )
        ::comphelper::disposeComponent(xResultSet);
}

void SwInsertDBColAutoPilot::SetTabSet()
{
    SwWrtShell& rSh = pView->GetWrtShell();
    const SfxPoolItem* pItem;

    if( pTAutoFmt )
    {
        if( pTAutoFmt->IsFrame() )
        {
            // border is from AutoFormat
            pTblSet->ClearItem( RES_BOX );
            pTblSet->ClearItem( SID_ATTR_BORDER_INNER );
        }
        if( pTAutoFmt->IsBackground() )
        {
            pTblSet->ClearItem( RES_BACKGROUND );
            pTblSet->ClearItem( SID_ATTR_BRUSH_ROW );
            pTblSet->ClearItem( SID_ATTR_BRUSH_TABLE );
        }
    }
    else
    {
        // remove the defaults again, it makes no sense to set them
        SvxBrushItem aBrush( RES_BACKGROUND );
        static const sal_uInt16 aIds[3] =
            { RES_BACKGROUND, SID_ATTR_BRUSH_ROW, SID_ATTR_BRUSH_TABLE };
        for( int i = 0; i < 3; ++i )
            if( SFX_ITEM_SET == pTblSet->GetItemState( aIds[ i ],
                sal_False, &pItem ) && *pItem == aBrush )
                pTblSet->ClearItem( aIds[ i ] );
    }

    if( SFX_ITEM_SET == pTblSet->GetItemState( FN_PARAM_TABLE_NAME, sal_False,
        &pItem ) && ((const SfxStringItem*)pItem)->GetValue() ==
                    rSh.GetTableFmt()->GetName() )
        pTblSet->ClearItem( FN_PARAM_TABLE_NAME );

    rSh.MoveTable( GetfnTableCurr(), GetfnTableStart() );
    rSh.SetMark();
    rSh.MoveTable( GetfnTableCurr(), GetfnTableEnd() );

    ItemSetToTableParam( *pTblSet, rSh );

    rSh.ClearMark();
    rSh.MoveTable( GetfnTableCurr(), GetfnTableStart() );
}

_DB_ColumnConfigData::~_DB_ColumnConfigData() {}

static Sequence<OUString> lcl_createSourceNames(const OUString& rNodeName)
{
    Sequence<OUString> aSourceNames(11);
    OUString* pNames = aSourceNames.getArray();
    pNames[0] = rNodeName + "/DataSource";
    pNames[1] = rNodeName + "/Command";
    pNames[2] = rNodeName + "/CommandType";
    pNames[3] = rNodeName + "/ColumnsToText";
    pNames[4] = rNodeName + "/ColumnsToTable";
    pNames[5] = rNodeName + "/ParaStyle";
    pNames[6] = rNodeName + "/TableAutoFormat";
    pNames[7] = rNodeName + "/IsTable";
    pNames[8] = rNodeName + "/IsField";
    pNames[9] = rNodeName + "/IsHeadlineOn";
    pNames[10] = rNodeName + "/IsEmptyHeadline";
    return aSourceNames;
}

static Sequence<OUString> lcl_CreateSubNames(const OUString& rSubNodeName)
{
    Sequence<OUString> aSubSourceNames(6);
    OUString* pNames = aSubSourceNames.getArray();
    pNames[0] = rSubNodeName + "/ColumnName";
    pNames[1] = rSubNodeName + "/ColumnIndex";
    pNames[2] = rSubNodeName + "/IsNumberFormat";
    pNames[3] = rSubNodeName + "/IsNumberFormatFromDataBase";
    pNames[4] = rSubNodeName + "/NumberFormat";
    pNames[5] = rSubNodeName + "/NumberFormatLocale";
    return aSubSourceNames;
}

static OUString lcl_CreateUniqueName(const Sequence<OUString>& aNames)
{
    sal_Int32 nIdx = aNames.getLength();
    const OUString* pNames = aNames.getConstArray();
    OUString sTest("_");
    OUString sRet;
    while(true)
    {
        sRet = sTest; sRet += OUString::number(nIdx++);
        bool bFound = false;
        for(sal_Int32 i = 0; i < aNames.getLength(); i++)
        {
            if(pNames[i] == sRet)
            {
                bFound = true;
                break;
            }
        }
        if(!bFound)
            break;
    }
    return sRet;
}

void SwInsertDBColAutoPilot::Notify( const ::com::sun::star::uno::Sequence< OUString >&  ) {}

void SwInsertDBColAutoPilot::Commit()
{
    Sequence <OUString> aNames = GetNodeNames(OUString());
    const OUString* pNames = aNames.getArray();
    //remove entries that contain this data source + table at first
    for(sal_Int32 nNode = 0; nNode < aNames.getLength(); nNode++)
    {
        Sequence<OUString> aSourceNames(2);
        OUString* pSourceNames = aSourceNames.getArray();
        pSourceNames[0] = pNames[nNode];
        pSourceNames[0] += "/DataSource";
        pSourceNames[1] = pNames[nNode];
        pSourceNames[1] += "/Command";
        Sequence<Any> aSourceProperties = GetProperties(aSourceNames);
        const Any* pSourceProps = aSourceProperties.getArray();
        OUString sSource, sCommand;
        pSourceProps[0] >>= sSource;
        pSourceProps[1] >>= sCommand;
        if(sSource.equals(aDBData.sDataSource) && sCommand.equals(aDBData.sCommand))
        {
            Sequence<OUString> aElements(1);
            aElements.getArray()[0] = pNames[nNode];
            ClearNodeElements(OUString(), aElements);
        }
    }

    aNames = GetNodeNames(OUString());
    OUString sNewNode = lcl_CreateUniqueName(aNames);
    Sequence<OUString> aNodeNames = lcl_createSourceNames(sNewNode);
    Sequence<PropertyValue> aValues(aNodeNames.getLength());
    PropertyValue* pValues = aValues.getArray();
    const OUString* pNodeNames = aNodeNames.getConstArray();
    OUString sSlash("/");
    for(sal_Int32 i = 0; i < aNodeNames.getLength(); i++)
    {
        pValues[i].Name = sSlash;
        pValues[i].Name += pNodeNames[i];
    }

    pValues[0].Value <<= OUString(aDBData.sDataSource);
    pValues[1].Value <<= OUString(aDBData.sCommand);
    pValues[2].Value <<= aDBData.nCommandType;
    pValues[3].Value <<= OUString(m_pEdDbText->GetText());

    OUString sTmp;
    for( sal_uInt16 n = 0, nCnt = m_pLbTableCol->GetEntryCount(); n < nCnt; ++n )
        (sTmp += m_pLbTableCol->GetEntry(n)) += "\x0a";

    if (!sTmp.isEmpty())
        pValues[4].Value <<= sTmp;

    if( sNoTmpl != (sTmp = m_pLbDbParaColl->GetSelectEntry()) )
        pValues[5].Value <<= sTmp;

    if( pTAutoFmt )
        pValues[6].Value <<= OUString(pTAutoFmt->GetName());

    const Type& rBoolType = ::getBooleanCppuType();
    sal_Bool bTmp = m_pRbAsTable->IsChecked();
    pValues[7].Value.setValue(&bTmp, rBoolType);

    bTmp = m_pRbAsField->IsChecked();
    pValues[8].Value.setValue(&bTmp, rBoolType);

    bTmp = m_pCbTableHeadon->IsChecked();
    pValues[9].Value.setValue(&bTmp, rBoolType);

    bTmp = m_pRbHeadlEmpty->IsChecked();
    pValues[10].Value.setValue(&bTmp, rBoolType);

    SetSetProperties(OUString(), aValues);

    sNewNode += "/ColumnSet";
    OUString sDelim("/__");

    LanguageType ePrevLang = (LanguageType)-1;
    OUString sPrevLang;

    SvNumberFormatter& rNFmtr = *pView->GetWrtShell().GetNumberFormatter();
    for(sal_uInt16 nCol = 0; nCol < aDBColumns.size(); nCol++)
    {
        OUString sColumnNode = sNewNode;
         SwInsDBColumn* pColumn = aDBColumns[nCol];
        String sColumnInsertNode(sColumnNode);
        sColumnInsertNode += sDelim;
        if( nCol < 100 )
            sColumnInsertNode += '0';
        if( nCol < 10 )
            sColumnInsertNode += '0';
        sColumnInsertNode += OUString::number(  nCol );

        Sequence <OUString> aSubNodeNames = lcl_CreateSubNames(sColumnInsertNode);
        Sequence<PropertyValue> aSubValues(aSubNodeNames.getLength());
        PropertyValue* pSubValues = aSubValues.getArray();
        const OUString* pSubNodeNames = aSubNodeNames.getConstArray();
        sal_Int32 i;

        for( i = 0; i < aSubNodeNames.getLength(); i++)
            pSubValues[i].Name = pSubNodeNames[i];
        pSubValues[0].Value <<= pColumn->sColumn;
        pSubValues[1].Value <<= i;

        sal_Bool bVal = pColumn->bHasFmt;
        pSubValues[2].Value.setValue(&bVal, rBoolType);
        bVal = pColumn->bIsDBFmt;
        pSubValues[3].Value.setValue(&bVal, rBoolType);

        SwStyleNameMapper::FillUIName( RES_POOLCOLL_STANDARD, sTmp );
        const SvNumberformat* pNF = rNFmtr.GetEntry( pColumn->nUsrNumFmt );
        LanguageType eLang;
        if( pNF )
        {
            pSubValues[4].Value <<= OUString(pNF->GetFormatstring());
            eLang = pNF->GetLanguage();
        }
        else
        {
            pSubValues[4].Value <<= sTmp;
            eLang = GetAppLanguage();
        }

        if( eLang != ePrevLang )
        {
            sPrevLang = LanguageTag::convertToBcp47( eLang );
            ePrevLang = eLang;
        }

        pSubValues[5].Value <<=  sPrevLang;
        SetSetProperties(sColumnNode, aSubValues);
    }
}

void SwInsertDBColAutoPilot::Load()
{
    Sequence <OUString> aNames = GetNodeNames(OUString());
    const OUString* pNames = aNames.getArray();
    SvNumberFormatter& rNFmtr = *pView->GetWrtShell().GetNumberFormatter();
    for(sal_Int32 nNode = 0; nNode < aNames.getLength(); nNode++)
    {
        //search for entries with the appropriate data source and table
        Sequence<OUString> aSourceNames = lcl_createSourceNames(pNames[nNode]);

        Sequence< Any> aDataSourceProps = GetProperties(aSourceNames);
        const Any* pDataSourceProps = aDataSourceProps.getConstArray();
        OUString sSource, sCommand;
        sal_Int16 nCommandType;
        pDataSourceProps[0] >>= sSource;
        pDataSourceProps[1] >>= sCommand;
        pDataSourceProps[2] >>= nCommandType;
        if(sSource.equals(aDBData.sDataSource) && sCommand.equals(aDBData.sCommand))
        {
            _DB_ColumnConfigData* pNewData = new _DB_ColumnConfigData;
            pNewData->sSource = sSource;
            pNewData->sTable = sCommand;

            pDataSourceProps[3] >>= pNewData->sEdit;
            pDataSourceProps[4] >>= pNewData->sTblList;
            pDataSourceProps[5] >>= pNewData->sTmplNm;
            pDataSourceProps[6] >>= pNewData->sTAutoFmtNm;
            if(pDataSourceProps[7].hasValue())
                pNewData->bIsTable = *(sal_Bool*)pDataSourceProps[7].getValue();
            if(pDataSourceProps[8].hasValue())
                 pNewData->bIsField = *(sal_Bool*)pDataSourceProps[8].getValue();
            if(pDataSourceProps[9].hasValue())
                 pNewData->bIsHeadlineOn = *(sal_Bool*)pDataSourceProps[9].getValue();
            if(pDataSourceProps[10].hasValue())
                 pNewData->bIsEmptyHeadln = *(sal_Bool*)pDataSourceProps[10].getValue();

            OUString sSubNodeName(pNames[nNode]);
            sSubNodeName += "/ColumnSet/";
            Sequence <OUString> aSubNames = GetNodeNames(sSubNodeName);
            const OUString* pSubNames = aSubNames.getConstArray();
            for(sal_Int32 nSub = 0; nSub < aSubNames.getLength(); nSub++)
            {
                OUString sSubSubNodeName(sSubNodeName);
                sSubSubNodeName += pSubNames[nSub];
                Sequence <OUString> aSubNodeNames = lcl_CreateSubNames(sSubSubNodeName);
                Sequence< Any> aSubProps = GetProperties(aSubNodeNames);
                const Any* pSubProps = aSubProps.getConstArray();

                OUString sColumn;
                pSubProps[0] >>= sColumn;
                //check for existance of the loaded column name
                bool bFound = false;
                for(sal_uInt16 nRealColumn = 0; nRealColumn < aDBColumns.size(); nRealColumn++)
                {
                    if(aDBColumns[nRealColumn]->sColumn == sColumn)
                    {
                        bFound = true;
                        break;
                    }
                }
                if(!bFound)
                    continue;
                sal_Int16 nIndex = 0;
                pSubProps[1] >>= nIndex;
                SwInsDBColumn* pInsDBColumn = new SwInsDBColumn(sColumn, nIndex);
                if(pSubProps[2].hasValue())
                    pInsDBColumn->bHasFmt = *(sal_Bool*)pSubProps[2].getValue();
                if(pSubProps[3].hasValue())
                    pInsDBColumn->bIsDBFmt = *(sal_Bool*)pSubProps[3].getValue();

                pSubProps[4] >>= pInsDBColumn->sUsrNumFmt;
                OUString sNumberFormatLocale;
                pSubProps[5] >>= sNumberFormatLocale;

                /* XXX Earlier versions wrote a Country-Language string in
                 * SwInsertDBColAutoPilot::Commit() that here was read as
                 * Language-Country with 2 characters copied to language,
                 * 1 character separator and unconditionally 2 characters read
                 * as country. So for 'DE-de' and locales that have similar
                 * case-insensitive equal language/country combos that may have
                 * worked, for all others not. FIXME if you need to read old
                 * data that you were never able to read before. */
                pInsDBColumn->eUsrNumFmtLng = LanguageTag::convertToLanguageType( sNumberFormatLocale );

                pInsDBColumn->nUsrNumFmt = rNFmtr.GetEntryKey( pInsDBColumn->sUsrNumFmt,
                                                        pInsDBColumn->eUsrNumFmtLng );


                pNewData->aDBColumns.insert(pInsDBColumn);
            }
            sal_Int32 n = 0;
            String sTmp( pNewData->sTblList );
            if( sTmp.Len() )
            {
                do {
                    String sEntry( sTmp.GetToken( 0, '\x0a', n ) );
                    //preselect column - if they still exist!
                    if(m_pLbTblDbColumn->GetEntryPos(sEntry) != LISTBOX_ENTRY_NOTFOUND)
                    {
                        m_pLbTableCol->InsertEntry( sEntry );
                        m_pLbTblDbColumn->RemoveEntry( sEntry );
                    }
                } while( n < sTmp.Len() );

                if( !m_pLbTblDbColumn->GetEntryCount() )
                {
                    m_pIbDbcolAllTo->Enable( sal_False );
                    m_pIbDbcolOneTo->Enable( sal_False );
                }
                m_pIbDbcolOneFrom->Enable( sal_True );
                m_pIbDbcolAllFrom->Enable( sal_True );
            }
            m_pEdDbText->SetText( pNewData->sEdit );

            sTmp = pNewData->sTmplNm;
            if( sTmp.Len() )
                m_pLbDbParaColl->SelectEntry( sTmp );
            else
                m_pLbDbParaColl->SelectEntryPos( 0 );

            delete pTAutoFmt, pTAutoFmt = 0;
            sTmp = pNewData->sTAutoFmtNm;
            if( sTmp.Len() )
            {
                // then load the AutoFmt file and look for Autoformat first
                SwTableAutoFmtTbl aAutoFmtTbl;
                aAutoFmtTbl.Load();
                for( sal_uInt16 nAutoFmt = aAutoFmtTbl.size(); nAutoFmt; )
                    if( sTmp == aAutoFmtTbl[ --nAutoFmt ].GetName() )
                    {
                        pTAutoFmt = new SwTableAutoFmt( aAutoFmtTbl[ nAutoFmt ] );
                        break;
                    }
            }

            m_pRbAsTable->Check( pNewData->bIsTable );
            m_pRbAsField->Check( pNewData->bIsField );
            m_pRbAsText->Check( !pNewData->bIsTable && !pNewData->bIsField );

            m_pCbTableHeadon->Check( pNewData->bIsHeadlineOn );
            m_pRbHeadlColnms->Check( !pNewData->bIsEmptyHeadln );
            m_pRbHeadlEmpty->Check( pNewData->bIsEmptyHeadln );
            HeaderHdl(m_pCbTableHeadon);

            // now copy the user defined Numberformat strings to the
            // Shell. Then only these are available as ID
            for( n = 0; n < (sal_Int32) aDBColumns.size() ; ++n )
            {
                SwInsDBColumn& rSet = *aDBColumns[ n ];
                for( sal_uInt16 m = 0; m < pNewData->aDBColumns.size() ; ++m )
                {
                    SwInsDBColumn& rGet = *pNewData->aDBColumns[ m ];
                    if(rGet.sColumn == rSet.sColumn)
                    {
                        if( rGet.bHasFmt && !rGet.bIsDBFmt )
                        {
                            rSet.bIsDBFmt = sal_False;
                            rSet.nUsrNumFmt = rNFmtr.GetEntryKey( rGet.sUsrNumFmt,
                                                                    rGet.eUsrNumFmtLng );
                            if( NUMBERFORMAT_ENTRY_NOT_FOUND == rSet.nUsrNumFmt )
                            {
                                sal_Int32 nCheckPos;
                                short nType;
                                rNFmtr.PutEntry( rGet.sUsrNumFmt, nCheckPos, nType,
                                                rSet.nUsrNumFmt, rGet.eUsrNumFmtLng );
                            }
                        }
                        break;
                    }
                }
            }

            // when the cursor is inside of a table, table must NEVER be selectable
            if( !m_pRbAsTable->IsEnabled() && m_pRbAsTable->IsChecked() )
                m_pRbAsField->Check( sal_True );
            delete pNewData;
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
