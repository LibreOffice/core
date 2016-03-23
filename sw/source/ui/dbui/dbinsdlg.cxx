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
#include <connectivity/dbtools.hxx>

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

#include <o3tl/make_unique.hxx>

#include <boost/noncopyable.hpp>
#include <memory>
#include <swuiexp.hxx>

using namespace ::dbtools;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::uno;

const char cDBFieldStart  = '<';
const char cDBFieldEnd    = '>';

// Helper structure for adding database rows as fields or text
struct DB_Column
{
    enum ColType { DB_FILLTEXT, DB_COL_FIELD, DB_COL_TEXT, DB_SPLITPARA } eColType;

    union {
        OUString* pText;
        SwField* pField;
        sal_uLong nFormat;
    } DB_ColumnData;
    const SwInsDBColumn* pColInfo;

    DB_Column()
    {
        pColInfo = nullptr;
        DB_ColumnData.pText = nullptr;
        eColType = DB_SPLITPARA;
    }

    explicit DB_Column( const OUString& rText )
    {
        pColInfo = nullptr;
        DB_ColumnData.pText = new OUString( rText );
        eColType = DB_FILLTEXT;
    }

    DB_Column( const SwInsDBColumn& rInfo, sal_uLong nFormat )
    {
        pColInfo = &rInfo;
        DB_ColumnData.nFormat = nFormat;
        eColType = DB_COL_TEXT;
    }

    DB_Column( const SwInsDBColumn& rInfo, SwDBField& rField )
    {
        pColInfo = &rInfo;
        DB_ColumnData.pField = &rField;
        eColType = DB_COL_FIELD;
    }

    ~DB_Column()
    {
        if( DB_COL_FIELD == eColType )
            delete DB_ColumnData.pField;
        else if( DB_FILLTEXT == eColType )
            delete DB_ColumnData.pText;
    }
};

struct DB_ColumnConfigData: private boost::noncopyable
{
    SwInsDBColumns aDBColumns;
    OUString sSource;
    OUString sTable;
    OUString sEdit;
    OUString sTableList;
    OUString sTmplNm;
    OUString sTAutoFormatNm;
    bool bIsTable : 1,
         bIsField : 1,
         bIsHeadlineOn : 1,
         bIsEmptyHeadln : 1;

    DB_ColumnConfigData()
    {
        bIsTable = bIsHeadlineOn = true;
        bIsField = bIsEmptyHeadln = false;
    }

    ~DB_ColumnConfigData();
};

bool SwInsDBColumn::operator<( const SwInsDBColumn& rCmp ) const
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
        ConfigItemMode::DelayedUpdate)
    , aDBData(rData)
    , sNoTmpl(SW_RESSTR(SW_STR_NONE))
    , pView(&rView)
    , pTAutoFormat(nullptr)
    , pTableSet(nullptr)
    , pRep(nullptr)
{
    get(m_pRbAsTable, "astable");
    get(m_pRbAsField, "asfields");
    get(m_pRbAsText, "astext");
    get(m_pHeadFrame, "dbframe");
    get(m_pLbTableDbColumn, "tabledbcols");
    get(m_pLbTextDbColumn, "tabletxtcols");
    m_pLbTableDbColumn->SetDropDownLineCount(8);
    m_pLbTextDbColumn->SetDropDownLineCount(8);
    get(m_pFormatFrame, "formatframe");
    get(m_pRbDbFormatFromDb, "fromdatabase");
    get(m_pRbDbFormatFromUsr, "userdefined");
    get(m_pLbDbFormatFromUsr, "numformat");
    aOldNumFormatLnk = m_pLbDbFormatFromUsr->GetSelectHdl();
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
    get(m_pPbTableFormat, "tableformat");
    get(m_pPbTableAutofmt, "autoformat");

    nGBFormatLen = m_pFormatFrame->get_label().getLength();

    if(xColSupp.is())
    {
        SwWrtShell& rSh = pView->GetWrtShell();
        SvNumberFormatter* pNumFormatr = rSh.GetNumberFormatter();
        SvNumberFormatsSupplierObj* pNumFormat = new SvNumberFormatsSupplierObj( pNumFormatr );
        Reference< util::XNumberFormatsSupplier >  xDocNumFormatsSupplier = pNumFormat;
        Reference< util::XNumberFormats > xDocNumberFormats = xDocNumFormatsSupplier->getNumberFormats();
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
        sal_Int32 nCount = aColNames.getLength();
        for (sal_Int32 n = 0; n < nCount; ++n)
        {
            SwInsDBColumn* pNew = new SwInsDBColumn( pColNames[n] );
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
                    pNew->bHasFormat = true;
                    Any aFormat = xCol->getPropertyValue("FormatKey");
                    if(aFormat.hasValue())
                    {
                        sal_Int32 nFormat = 0;
                        aFormat >>= nFormat;
                        if(xNumberFormats.is())
                        {
                            try
                            {
                                Reference<XPropertySet> xNumProps = xNumberFormats->getByKey( nFormat );
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
                                pNew->nDBNumFormat = nKey;
                            }
                            catch (const Exception&)
                            {
                                OSL_FAIL("illegal number format key");
                            }
                        }
                    }
                    else
                    {
                        pNew->nDBNumFormat = getDefaultNumberFormat(xCol,
                                xDocNumberFormatTypes, LanguageTag( rSh.GetCurLang() ).getLocale());
                    }

                }
                break;
            }
            if( !aDBColumns.insert( pNew ).second )
            {
                OSL_ENSURE( false, "Spaltenname mehrfach vergeben?" );
                delete pNew;
            }
        }
    }

    // fill paragraph templates-ListBox
    {
        SfxStyleSheetBasePool* pPool = pView->GetDocShell()->GetStyleSheetPool();
        pPool->SetSearchMask( SFX_STYLE_FAMILY_PARA );
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
    if( pView->GetWrtShell().GetTableFormat() )
    {
        m_pRbAsTable->Enable( false );
        m_pRbAsField->Check();
        m_pRbDbFormatFromDb->Check();
    }
    else
    {
        m_pRbAsTable->Check();
        m_pRbDbFormatFromDb->Check();
        m_pIbDbcolOneFrom->Enable( false );
        m_pIbDbcolAllFrom->Enable( false );
    }

    m_pRbAsTable->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, PageHdl ));
    m_pRbAsField->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, PageHdl ));
    m_pRbAsText->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, PageHdl ));

    m_pRbDbFormatFromDb->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, DBFormatHdl ));
    m_pRbDbFormatFromUsr->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, DBFormatHdl ));

    m_pPbTableFormat->SetClickHdl(LINK(this, SwInsertDBColAutoPilot, TableFormatHdl ));
    m_pPbTableAutofmt->SetClickHdl(LINK(this, SwInsertDBColAutoPilot, AutoFormatHdl ));

    m_pIbDbcolAllTo->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, TableToFromHdl ));
    m_pIbDbcolOneTo->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, TableToFromHdl ));
    m_pIbDbcolOneFrom->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, TableToFromHdl ));
    m_pIbDbcolAllFrom->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, TableToFromHdl ));
    m_pIbDbcolToEdit->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, TableToFromHdl ));

    m_pCbTableHeadon->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, HeaderHdl ));
    m_pRbHeadlColnms->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, HeaderHdl ));
    m_pRbHeadlEmpty->SetClickHdl( LINK(this, SwInsertDBColAutoPilot, HeaderHdl ));

    m_pLbTextDbColumn->SetSelectHdl( LINK( this, SwInsertDBColAutoPilot, SelectHdl ));
    m_pLbTableDbColumn->SetSelectHdl( LINK( this, SwInsertDBColAutoPilot, SelectHdl ));
    m_pLbDbFormatFromUsr->SetSelectHdl( LINK( this, SwInsertDBColAutoPilot, SelectHdl ));
    m_pLbTableCol->SetSelectHdl( LINK( this, SwInsertDBColAutoPilot, SelectHdl ));

    m_pLbTextDbColumn->SetDoubleClickHdl( LINK( this, SwInsertDBColAutoPilot, DblClickHdl ));
    m_pLbTableDbColumn->SetDoubleClickHdl( LINK( this, SwInsertDBColAutoPilot, DblClickHdl ));
    m_pLbTableCol->SetDoubleClickHdl( LINK( this, SwInsertDBColAutoPilot, DblClickHdl ));

    for( size_t n = 0; n < aDBColumns.size(); ++n )
    {
        const OUString& rS = aDBColumns[ n ]->sColumn;
        m_pLbTableDbColumn->InsertEntry( rS, static_cast<sal_Int32>(n) );
        m_pLbTextDbColumn->InsertEntry( rS, static_cast<sal_Int32>(n) );
    }
    m_pLbTextDbColumn->SelectEntryPos( 0 );
    m_pLbTableDbColumn->SelectEntryPos( 0 );

    // read configuration
    Load();

    // lock size to widest config
    m_pHeadFrame->set_width_request(m_pHeadFrame->get_preferred_size().Width());
    // initialise Controls:
    PageHdl( m_pRbAsTable->IsChecked() ? m_pRbAsTable : m_pRbAsField );
}

SwInsertDBColAutoPilot::~SwInsertDBColAutoPilot()
{
    disposeOnce();
}

void SwInsertDBColAutoPilot::dispose()
{
    delete pTableSet;
    delete pRep;

    delete pTAutoFormat;
    m_pRbAsTable.clear();
    m_pRbAsField.clear();
    m_pRbAsText.clear();
    m_pHeadFrame.clear();
    m_pLbTableDbColumn.clear();
    m_pLbTextDbColumn.clear();
    m_pFormatFrame.clear();
    m_pRbDbFormatFromDb.clear();
    m_pRbDbFormatFromUsr.clear();
    m_pLbDbFormatFromUsr.clear();
    m_pIbDbcolToEdit.clear();
    m_pEdDbText.clear();
    m_pFtDbParaColl.clear();
    m_pLbDbParaColl.clear();
    m_pIbDbcolAllTo.clear();
    m_pIbDbcolOneTo.clear();
    m_pIbDbcolOneFrom.clear();
    m_pIbDbcolAllFrom.clear();
    m_pFtTableCol.clear();
    m_pLbTableCol.clear();
    m_pCbTableHeadon.clear();
    m_pRbHeadlColnms.clear();
    m_pRbHeadlEmpty.clear();
    m_pPbTableFormat.clear();
    m_pPbTableAutofmt.clear();
    SfxModalDialog::dispose();
}

IMPL_LINK_TYPED( SwInsertDBColAutoPilot, PageHdl, Button*, pButton, void )
{
    bool bShowTable = pButton == m_pRbAsTable;

    m_pHeadFrame->set_label(MnemonicGenerator::EraseAllMnemonicChars(pButton->GetText()));

    m_pLbTextDbColumn->Show( !bShowTable );
    m_pIbDbcolToEdit->Show( !bShowTable );
    m_pEdDbText->Show( !bShowTable );
    m_pFtDbParaColl->Show( !bShowTable );
    m_pLbDbParaColl->Show( !bShowTable );

    m_pLbTableDbColumn->Show( bShowTable );
    m_pIbDbcolAllTo->Show( bShowTable );
    m_pIbDbcolOneTo->Show( bShowTable );
    m_pIbDbcolOneFrom->Show( bShowTable );
    m_pIbDbcolAllFrom->Show( bShowTable );
    m_pFtTableCol->Show( bShowTable );
    m_pLbTableCol->Show( bShowTable );
    m_pCbTableHeadon->Show( bShowTable );
    m_pRbHeadlColnms->Show( bShowTable );
    m_pRbHeadlEmpty->Show( bShowTable );
    m_pPbTableFormat->Show( bShowTable );
    m_pPbTableAutofmt->Show( bShowTable );

    if( bShowTable )
        m_pPbTableFormat->Enable( 0 != m_pLbTableCol->GetEntryCount() );

    SelectHdl( bShowTable ? *m_pLbTableDbColumn : *m_pLbTextDbColumn );
}

IMPL_LINK_TYPED( SwInsertDBColAutoPilot, DBFormatHdl, Button*, pButton, void )
{
    ListBox& rBox = m_pRbAsTable->IsChecked()
                        ? ( nullptr == m_pLbTableCol->GetEntryData( 0 )
                            ? *m_pLbTableDbColumn
                            : *m_pLbTableCol )
                        : *m_pLbTextDbColumn;

    SwInsDBColumn aSrch( rBox.GetSelectEntry() );
    SwInsDBColumns::const_iterator it = aDBColumns.find( &aSrch );

    bool bFromDB = m_pRbDbFormatFromDb == pButton;
    (*it)->bIsDBFormat = bFromDB;
    m_pLbDbFormatFromUsr->Enable( !bFromDB );
}

IMPL_LINK_TYPED( SwInsertDBColAutoPilot, TableToFromHdl, Button*, pButton, void )
{
    bool bChgEnable = true, bEnableTo = true, bEnableFrom = true;
    m_pLbTableDbColumn->SetUpdateMode( false );
    m_pLbTableCol->SetUpdateMode( false );

    if( pButton == m_pIbDbcolAllTo )
    {
        bEnableTo = false;

        sal_Int32 n, nInsPos = m_pLbTableCol->GetSelectEntryPos(),
               nCnt = m_pLbTableDbColumn->GetEntryCount();
        if( LISTBOX_APPEND == nInsPos )
            for( n = 0; n < nCnt; ++n )
                m_pLbTableCol->InsertEntry( m_pLbTableDbColumn->GetEntry( n ) );
        else
            for( n = 0; n < nCnt; ++n, ++nInsPos )
                m_pLbTableCol->InsertEntry( m_pLbTableDbColumn->GetEntry( n ), nInsPos );
        m_pLbTableDbColumn->Clear();
        m_pLbTableCol->SelectEntryPos( nInsPos );
        m_pLbTableDbColumn->SelectEntryPos( LISTBOX_APPEND );
    }
    else if( pButton == m_pIbDbcolOneTo &&
            LISTBOX_ENTRY_NOTFOUND != m_pLbTableDbColumn->GetSelectEntryPos() )
    {
        sal_Int32 nInsPos = m_pLbTableCol->GetSelectEntryPos(),
               nDelPos = m_pLbTableDbColumn->GetSelectEntryPos(),
               nTopPos = m_pLbTableDbColumn->GetTopEntry();
        m_pLbTableCol->InsertEntry( m_pLbTableDbColumn->GetEntry( nDelPos ), nInsPos );
        m_pLbTableDbColumn->RemoveEntry( nDelPos );

        m_pLbTableCol->SelectEntryPos( nInsPos );
        if( nDelPos >= m_pLbTableDbColumn->GetEntryCount() )
            nDelPos = m_pLbTableDbColumn->GetEntryCount() - 1;
        m_pLbTableDbColumn->SelectEntryPos( nDelPos );
        m_pLbTableDbColumn->SetTopEntry( nTopPos );

        bEnableTo = 0 != m_pLbTableDbColumn->GetEntryCount();
    }
    else if( pButton == m_pIbDbcolOneFrom )
    {
        if( LISTBOX_ENTRY_NOTFOUND != m_pLbTableCol->GetSelectEntryPos() )
        {
            sal_Int32 nInsPos,
                    nDelPos = m_pLbTableCol->GetSelectEntryPos(),
                    nTopPos = m_pLbTableCol->GetTopEntry();

            // look for the right InsertPos!!
            SwInsDBColumn aSrch( m_pLbTableCol->GetEntry( nDelPos ) );
            SwInsDBColumns::const_iterator it = aDBColumns.find( &aSrch );
            if( it == aDBColumns.begin() || (it+1) == aDBColumns.end() )
                nInsPos = it - aDBColumns.begin();
            else
            {
                nInsPos = LISTBOX_ENTRY_NOTFOUND;
                while( ++it != aDBColumns.end() &&
                        LISTBOX_ENTRY_NOTFOUND == (nInsPos = m_pLbTableDbColumn->
                        GetEntryPos( (*it)->sColumn )) )
                    ;
            }

            m_pLbTableDbColumn->InsertEntry( aSrch.sColumn, nInsPos );
            m_pLbTableCol->RemoveEntry( nDelPos );

            if( nInsPos >= m_pLbTableDbColumn->GetEntryCount() )
                nInsPos = m_pLbTableDbColumn->GetEntryCount() - 1;
            m_pLbTableDbColumn->SelectEntryPos( nInsPos );

            if( nDelPos >= m_pLbTableCol->GetEntryCount() )
                nDelPos = m_pLbTableCol->GetEntryCount() - 1;
            m_pLbTableCol->SelectEntryPos( nDelPos );
            m_pLbTableCol->SetTopEntry( nTopPos );
        }
        else
            bEnableTo = 0 != m_pLbTableDbColumn->GetEntryCount();

        bEnableFrom = 0 != m_pLbTableCol->GetEntryCount();
    }
    else if( pButton == m_pIbDbcolAllFrom )
    {
        bEnableFrom = false;

        m_pLbTableDbColumn->Clear();
        m_pLbTableCol->Clear();
        for( size_t n = 0; n < aDBColumns.size(); ++n )
            m_pLbTableDbColumn->InsertEntry( aDBColumns[ n ]->sColumn,
                                           static_cast<sal_Int32>(n) );
        m_pLbTableDbColumn->SelectEntryPos( 0 );
    }
    else if( pButton == m_pIbDbcolToEdit )
    {
        bChgEnable = false;
        // move data to Edit:
        OUString aField( m_pLbTextDbColumn->GetSelectEntry() );
        if( !aField.isEmpty() )
        {
            OUString aStr( m_pEdDbText->GetText() );
            sal_Int32 nPos = static_cast<sal_Int32>(m_pEdDbText->GetSelection().Min());
            const sal_Int32 nSel = static_cast<sal_Int32>(m_pEdDbText->GetSelection().Max()) - nPos;
            if( nSel )
                // first delete the existing selection
                aStr = aStr.replaceAt( nPos, nSel, "" );

            aField = OUStringLiteral1<cDBFieldStart>() + aField + OUStringLiteral1<cDBFieldEnd>();
            if( !aStr.isEmpty() )
            {
                if( nPos )                          // one blank in front
                {
                    sal_Unicode c = aStr[ nPos-1 ];
                    if( '\n' != c && '\r' != c )
                        aField = " " + aField;
                }
                if( nPos < aStr.getLength() )             // one blank behind
                {
                    sal_Unicode c = aStr[ nPos ];
                    if( '\n' != c && '\r' != c )
                        aField += " ";
                }
            }

            m_pEdDbText->SetText( aStr.replaceAt( nPos, 0, aField ) );
            nPos += aField.getLength();
            m_pEdDbText->SetSelection( Selection( nPos ));
        }
    }

    if( bChgEnable )
    {
        m_pIbDbcolOneTo->Enable( bEnableTo );
        m_pIbDbcolAllTo->Enable( bEnableTo );
        m_pIbDbcolOneFrom->Enable( bEnableFrom );
        m_pIbDbcolAllFrom->Enable( bEnableFrom );

        m_pRbDbFormatFromDb->Enable( false );
        m_pRbDbFormatFromUsr->Enable( false );
        m_pLbDbFormatFromUsr->Enable( false );

        m_pPbTableFormat->Enable( bEnableFrom );
    }
    m_pLbTableDbColumn->SetUpdateMode( true );
    m_pLbTableCol->SetUpdateMode( true );
}

IMPL_LINK_TYPED( SwInsertDBColAutoPilot, DblClickHdl, ListBox&, rBox, void )
{
    Button* pButton = nullptr;
    if( &rBox == m_pLbTextDbColumn )
        pButton = m_pIbDbcolToEdit;
    else if( &rBox == m_pLbTableDbColumn && m_pIbDbcolOneTo->IsEnabled() )
        pButton = m_pIbDbcolOneTo;
    else if( &rBox == m_pLbTableCol && m_pIbDbcolOneFrom->IsEnabled() )
        pButton = m_pIbDbcolOneFrom;

    if( pButton )
        TableToFromHdl( pButton );
}

IMPL_LINK_TYPED( SwInsertDBColAutoPilot, TableFormatHdl, Button*, pButton, void )
{
    SwWrtShell& rSh = pView->GetWrtShell();
    bool bNewSet = false;
    if( !pTableSet )
    {
        bNewSet = true;
        pTableSet = new SfxItemSet( rSh.GetAttrPool(), SwuiGetUITableAttrRange() );

        // At first acquire the simple attributes
        pTableSet->Put( SfxStringItem( FN_PARAM_TABLE_NAME, rSh.GetUniqueTableName() ));
        pTableSet->Put( SfxUInt16Item( FN_PARAM_TABLE_HEADLINE, 1 ) );

        pTableSet->Put( SfxUInt16Item( SID_BACKGRND_DESTINATION,
                                    rSh.GetViewOptions()->GetTableDest() ));

        SvxBrushItem aBrush( RES_BACKGROUND );
        pTableSet->Put( aBrush );
        pTableSet->Put( aBrush, SID_ATTR_BRUSH_ROW );
        pTableSet->Put( aBrush, SID_ATTR_BRUSH_TABLE );

        SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
            // table variant, when multiple table cells are selected
        aBoxInfo.SetTable( true );
            // always show gap field
        aBoxInfo.SetDist( true);
            // set minimum size in tables and paragraphs
        aBoxInfo.SetMinDist( false );
            // always set default-gap
        aBoxInfo.SetDefDist( MIN_BORDER_DIST );
            // Single lines can have DontCare-status only in tables
        aBoxInfo.SetValid( SvxBoxInfoItemValidFlags::DISABLE );
        pTableSet->Put( aBoxInfo );

        SwGetCurColNumPara aPara;
        const sal_uInt16 nNum = rSh.GetCurColNum( &aPara );
        long nWidth;

        if( nNum )
        {
            nWidth = aPara.pPrtRect->Width();
            const SwFormatCol& rCol = aPara.pFrameFormat->GetCol();
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
                                FrameTypeFlags::FLY_ANY & rSh.GetFrameType( nullptr, true )
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
        pTableSet->Put( SwPtrItem( FN_TABLE_REP, pRep ));

        pTableSet->Put( SfxUInt16Item( SID_HTML_MODE,
                    ::GetHtmlMode( pView->GetDocShell() )));
    }

    sal_Int32 nCols = m_pLbTableCol->GetEntryCount();
    if (nCols != pRep->GetAllColCount() && nCols > 0)
    {
        // Number of columns has changed: then the TabCols have to be adjusted
        long nWidth = pRep->GetWidth();
        --nCols;
        SwTabCols aTabCols( nCols );
        aTabCols.SetRight( nWidth  );
        aTabCols.SetRightMax( nWidth );
        if( nCols )
        {
            const sal_Int32 nStep = nWidth / (nCols+1);
            for( sal_Int32 n = 0; n < nCols; ++n )
            {
                aTabCols.Insert( nStep*(n+1), false, n );
            }
        }
        delete pRep;
        pRep = new SwTableRep( aTabCols );
        pRep->SetAlign( text::HoriOrientation::NONE );
        pRep->SetSpace( nWidth );
        pRep->SetWidth( nWidth );
        pRep->SetWidthPercent( 100 );
        pTableSet->Put( SwPtrItem( FN_TABLE_REP, pRep ));
    }

    SwAbstractDialogFactory* pFact = swui::GetFactory();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

    std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateSwTableTabDlg(pButton, rSh.GetAttrPool(), pTableSet, &rSh));
    OSL_ENSURE(pDlg, "Dialog creation failed!");
    if( RET_OK == pDlg->Execute() )
        pTableSet->Put( *pDlg->GetOutputItemSet() );
    else if( bNewSet )
    {
        delete pTableSet;
        pTableSet = nullptr;
        delete pRep;
        pRep = nullptr;
    }
}

IMPL_LINK_TYPED( SwInsertDBColAutoPilot, AutoFormatHdl, Button*, pButton, void )
{
    SwAbstractDialogFactory* pFact = swui::GetFactory();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

    std::unique_ptr<AbstractSwAutoFormatDlg> pDlg(pFact->CreateSwAutoFormatDlg(pButton, pView->GetWrtShellPtr(), false, pTAutoFormat));
    OSL_ENSURE(pDlg, "Dialog creation failed!");
    if( RET_OK == pDlg->Execute())
        pDlg->FillAutoFormatOfIndex( pTAutoFormat );
}

IMPL_LINK_TYPED( SwInsertDBColAutoPilot, SelectHdl, ListBox&, rBox, void )
{
    ListBox* pGetBox = &rBox == m_pLbDbFormatFromUsr
                            ? ( m_pRbAsTable->IsChecked()
                                    ? ( nullptr == m_pLbTableCol->GetEntryData( 0 )
                                        ? m_pLbTableDbColumn.get()
                                        : m_pLbTableCol.get() )
                                    : m_pLbTextDbColumn.get() )
                            : &rBox;

    SwInsDBColumn aSrch( pGetBox->GetSelectEntry() );
    SwInsDBColumns::const_iterator it = aDBColumns.find( &aSrch );

    if( &rBox == m_pLbDbFormatFromUsr )
    {
        if( !aSrch.sColumn.isEmpty() )
        {
            aOldNumFormatLnk.Call( rBox );
            (*it)->nUsrNumFormat = m_pLbDbFormatFromUsr->GetFormat();
        }
    }
    else
    {
        // set the selected FieldName at the FormatGroupBox, so that
        // it's clear what field is configured by the format!
        OUString sText( m_pFormatFrame->get_label().copy( 0, nGBFormatLen ));
        if( aSrch.sColumn.isEmpty() )
        {
            m_pRbDbFormatFromDb->Enable( false );
            m_pRbDbFormatFromUsr->Enable( false );
            m_pLbDbFormatFromUsr->Enable( false );
        }
        else
        {
            bool bEnableFormat = (*it)->bHasFormat;
            m_pRbDbFormatFromDb->Enable( bEnableFormat );
            m_pRbDbFormatFromUsr->Enable( bEnableFormat );

            if( bEnableFormat )
            {
                sText += " ("  + aSrch.sColumn + ")";
            }

            bool bIsDBFormat = (*it)->bIsDBFormat;
            m_pRbDbFormatFromDb->Check( bIsDBFormat );
            m_pRbDbFormatFromUsr->Check( !bIsDBFormat );
            m_pLbDbFormatFromUsr->Enable( !bIsDBFormat );
            if( !bIsDBFormat )
                m_pLbDbFormatFromUsr->SetDefFormat( (*it)->nUsrNumFormat );
        }

        m_pFormatFrame->set_label(sText);

        // to know later on, what ListBox was the "active", a Flag
        // is remembered in the 1st entry
        void* pPtr = &rBox == m_pLbTableCol ? m_pLbTableCol.get() : nullptr;
        m_pLbTableCol->SetEntryData( 0, pPtr );
    }
}

IMPL_LINK_TYPED( SwInsertDBColAutoPilot, HeaderHdl, Button*, pButton, void )
{
    if( pButton == m_pCbTableHeadon )
    {
        bool bEnable = m_pCbTableHeadon->IsChecked();

        m_pRbHeadlColnms->Enable( bEnable );
        m_pRbHeadlEmpty->Enable( bEnable );
    }
}

static void lcl_InsTextInArr( const OUString& rText, DB_Columns& rColArr )
{
    sal_Int32 nSttPos = 0, nFndPos;
    while( -1 != ( nFndPos = rText.indexOf( '\x0A', nSttPos )) )
    {
        if( 1 < nFndPos )
        {
            rColArr.push_back(o3tl::make_unique<DB_Column>(rText.copy(nSttPos, nFndPos -1)));
        }
        rColArr.push_back(o3tl::make_unique<DB_Column>());
        nSttPos = nFndPos + 1;
    }
    if( nSttPos < rText.getLength() )
    {
        rColArr.push_back(o3tl::make_unique<DB_Column>(rText.copy(nSttPos)));
    }
}

bool SwInsertDBColAutoPilot::SplitTextToColArr( const OUString& rText,
                                DB_Columns& rColArr,
                                bool bInsField )
{
    // create each of the database columns from the text again
    // and then save in an array
    // database columns are in <> and must be present in the columns' array:
    OUString sText( rText );
    sal_Int32 nFndPos, nEndPos, nSttPos = 0;

    while( -1 != ( nFndPos = sText.indexOf( cDBFieldStart, nSttPos )))
    {
        nSttPos = nFndPos + 1;
        if( -1 != ( nEndPos = sText.indexOf( cDBFieldEnd, nSttPos+1 )))
        {
            // Text in <> brackets found: what is it:
            SwInsDBColumn aSrch( sText.copy( nSttPos, nEndPos - nSttPos ));
            SwInsDBColumns::const_iterator it = aDBColumns.find( &aSrch );
            if( it != aDBColumns.end() )
            {
                // that is a valid field
                // so surely the text "before":
                const SwInsDBColumn& rFndCol = **it;

                DB_Column* pNew;

                if( 1 < nSttPos )
                {
                    ::lcl_InsTextInArr( sText.copy( 0, nSttPos-1 ), rColArr );
                    sText = sText.copy( nSttPos-1 );
                }

                sText = sText.copy( rFndCol.sColumn.getLength() + 2 );
                nSttPos = 0;

                sal_uInt16 nSubType = 0;
                sal_uLong nFormat;
                if( rFndCol.bHasFormat )
                {
                    if( rFndCol.bIsDBFormat )
                        nFormat =  rFndCol.nDBNumFormat;
                    else
                    {
                        nFormat = rFndCol.nUsrNumFormat;
                        nSubType = nsSwExtendedSubType::SUB_OWN_FMT;
                    }
                }
                else
                    nFormat = 0;

                if( bInsField )
                {
                    SwWrtShell& rSh = pView->GetWrtShell();
                    SwDBFieldType aFieldType( rSh.GetDoc(), aSrch.sColumn,
                                            aDBData );
                    pNew = new DB_Column( rFndCol, *new SwDBField(
                            static_cast<SwDBFieldType*>(rSh.InsertFieldType( aFieldType )),
                                                            nFormat ) );
                    if( nSubType )
                        pNew->DB_ColumnData.pField->SetSubType( nSubType );
                }
                else
                    pNew = new DB_Column( rFndCol, nFormat );

                rColArr.push_back( std::unique_ptr<DB_Column>(pNew) );
            }
        }
    }

    // don't forget the last text
    if( !sText.isEmpty() )
        ::lcl_InsTextInArr( sText, rColArr );

    return !rColArr.empty();
}

void SwInsertDBColAutoPilot::DataToDoc( const Sequence<Any>& rSelection,
    Reference< XDataSource> xSource,
    Reference< XConnection> xConnection,
    Reference< sdbc::XResultSet > xResultSet )
{
    const Any* pSelection = rSelection.getLength() ? rSelection.getConstArray() : nullptr;
    SwWrtShell& rSh = pView->GetWrtShell();

    //with the drag and drop interface no result set is initially available
    bool bDisposeResultSet = false;
    // we don't have a cursor, so we have to create our own RowSet
    if ( !xResultSet.is() )
    {
        xResultSet = SwDBManager::createCursor(aDBData.sDataSource,aDBData.sCommand,aDBData.nCommandType,xConnection);
        bDisposeResultSet = xResultSet.is();
    }

    Reference< sdbc::XRow > xRow(xResultSet, UNO_QUERY);
    if ( !xRow.is() )
        return;

    rSh.StartAllAction();
    bool bUndo = rSh.DoesUndo();
    if( bUndo )
        rSh.StartUndo();

    bool bAsTable = m_pRbAsTable->IsChecked();
    SvNumberFormatter& rNumFormatr = *rSh.GetNumberFormatter();

    if( rSh.HasSelection() )
        rSh.DelRight();

    std::unique_ptr<SwWait> pWait;

    Reference< XColumnsSupplier > xColsSupp( xResultSet, UNO_QUERY );
    Reference <XNameAccess> xCols = xColsSupp->getColumns();

    do{                                 // middle checked loop!!
    if( bAsTable )          // Daten als Tabelle einfuegen
    {
        rSh.DoUndo( false );

        sal_Int32 nCols = m_pLbTableCol->GetEntryCount();
        sal_Int32 nRows = 0;
        if( m_pCbTableHeadon->IsChecked() )
            nRows++;

        if( pSelection )
            nRows += rSelection.getLength();
        else
            ++nRows;

        // prepare the array for the selected columns
        std::vector<SwInsDBColumn*> aColFields;
        for( sal_Int32 n = 0; n < nCols; ++n )
        {
            SwInsDBColumn aSrch( m_pLbTableCol->GetEntry( n ) );
            SwInsDBColumns::const_iterator it = aDBColumns.find( &aSrch );
            if (it != aDBColumns.end())
                aColFields.push_back(*it);
            else {
                OSL_ENSURE( false, "database column not found" );
            }
        }

        if( static_cast<size_t>(nCols) != aColFields.size() )
        {
            OSL_ENSURE( false, "not all database columns found" );
            nCols = static_cast<sal_Int32>(aColFields.size());
        }

        if(!nRows || !nCols)
        {
            OSL_ENSURE( false, "wrong parameters" );
            break;
        }

        const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

        bool bHTML = 0 != (::GetHtmlMode( pView->GetDocShell() ) & HTMLMODE_ON);
        rSh.InsertTable(
            pModOpt->GetInsTableFlags(bHTML),
            nRows, nCols, text::HoriOrientation::FULL, (pSelection ? pTAutoFormat : nullptr) );
        rSh.MoveTable( GetfnTablePrev(), GetfnTableStart() );

        if( pSelection && pTableSet )
            SetTabSet();

        SfxItemSet aTableSet( rSh.GetAttrPool(), RES_BOXATR_FORMAT,
                                                RES_BOXATR_VALUE );
        bool bIsAutoUpdateCells = rSh.IsAutoUpdateCells();
        rSh.SetAutoUpdateCells( false );

        if( m_pCbTableHeadon->IsChecked() )
        {
            for( sal_Int32 n = 0; n < nCols; ++n )
            {
                if( m_pRbHeadlColnms->IsChecked() )
                {
                    rSh.SwEditShell::Insert2( aColFields[ n ]->sColumn );
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

            for( sal_Int32 n = 0; n < nCols; ++n )
            {
                // at the very first time, NO GoNextCell, because we're
                // already in it. Also no GoNextCell after the Insert,
                // because an empty line is added at the end.
                if( i || n )
                    rSh.GoNextCell();

                const SwInsDBColumn* pEntry = aColFields[ n ];

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
                    if( pEntry->bHasFormat )
                    {
                        SwTableBoxNumFormat aNumFormat(
                                        pEntry->bIsDBFormat ? pEntry->nDBNumFormat
                                                         : pEntry->nUsrNumFormat );
                        aTableSet.Put(aNumFormat);
                        if( xColumn.is() )
                        {
                            double fVal = xColumn->getDouble();
                            if( xColumn->wasNull() )
                                aTableSet.ClearItem( RES_BOXATR_VALUE );
                            else
                            {
                                if(rNumFormatr.GetType(aNumFormat.GetValue()) & css::util::NumberFormat::DATE)
                                {
                                    ::Date aStandard(1,1,1900);
                                    if (*rNumFormatr.GetNullDate() != aStandard)
                                        fVal += (aStandard - *rNumFormatr.GetNullDate());
                                }
                                aTableSet.Put( SwTableBoxValue( fVal ));
                            }
                        }
                        else
                            aTableSet.ClearItem( RES_BOXATR_VALUE );
                        rSh.SetTableBoxFormulaAttrs( aTableSet );
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
                        const OUString sVal =  xColumn->getString();
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
                pWait.reset(new SwWait( *pView->GetDocShell(), true ));
        }

        rSh.MoveTable( GetfnTableCurr(), GetfnTableStart() );
        if( !pSelection && ( pTableSet || pTAutoFormat ))
        {
            if( pTableSet )
                SetTabSet();

            if( pTAutoFormat )
                rSh.SetTableStyle(*pTAutoFormat);
        }
        rSh.SetAutoUpdateCells( bIsAutoUpdateCells );
    }
    else                            // add data as fields/text
    {
        DB_Columns aColArr;
        if( SplitTextToColArr( m_pEdDbText->GetText(), aColArr, m_pRbAsField->IsChecked() ) )
        {
            // now for each data set, we can iterate over the array
            // and add the data

            if( !rSh.IsSttPara() )
                rSh.SwEditShell::SplitNode();
            if( !rSh.IsEndPara() )
            {
                rSh.SwEditShell::SplitNode();
                rSh.SwCursorShell::Left(1,CRSR_SKIP_CHARS);
            }

            rSh.DoUndo( false );

            SwTextFormatColl* pColl = nullptr;
            {
                const OUString sTmplNm( m_pLbDbParaColl->GetSelectEntry() );
                if( sNoTmpl != sTmplNm )
                {
                    pColl = rSh.FindTextFormatCollByName( sTmplNm );
                    if( !pColl )
                    {
                        const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(
                            sTmplNm, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
                        if( USHRT_MAX != nId )
                            pColl = rSh.GetTextCollFromPool( nId );
                        else
                            pColl = rSh.MakeTextFormatColl( sTmplNm );
                    }
                    rSh.SetTextFormatColl( pColl );
                }
            }

            // for adding as fields -> insert a "NextField" after
            // every data set
            SwDBFormatData aDBFormatData;
            Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
            aDBFormatData.xFormatter.set(util::NumberFormatter::create(xContext), UNO_QUERY_THROW) ;

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
            SwDBNextSetField aNxtDBField( static_cast<SwDBNextSetFieldType*>(rSh.
                                            GetFieldType( 0, RES_DBNEXTSETFLD )),
                                        "1", "", aDBData );

            bool bSetCursor = true;
            const size_t nCols = aColArr.size();
            ::sw::mark::IMark* pMark = nullptr;
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

                for( size_t n = 0; n < nCols; ++n )
                {
                    DB_Column* pDBCol = aColArr[ n ].get();
                    OUString sIns;
                    switch( pDBCol->eColType )
                    {
                    case DB_Column::DB_FILLTEXT:
                        sIns =  *pDBCol->DB_ColumnData.pText;
                        break;

                    case DB_Column::DB_SPLITPARA:
                        rSh.SplitNode();
                        // when the template is not the same as the follow template,
                        // the selected has to be set newly
                        if( pColl && &pColl->GetNextTextFormatColl() != pColl )
                            rSh.SetTextFormatColl( pColl );
                        break;

                    case DB_Column::DB_COL_FIELD:
                        {
                            std::unique_ptr<SwDBField> pField(static_cast<SwDBField *>(
                                pDBCol->DB_ColumnData.pField->CopyField()));
                            double nValue = DBL_MAX;

                            Reference< XPropertySet > xColumnProps;
                            xCols->getByName(pDBCol->pColInfo->sColumn) >>= xColumnProps;

                            pField->SetExpansion( SwDBManager::GetDBField(
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
                                pField->ChgValue( nValue, true );
                            }
                            pField->SetInitialized();

                            rSh.Insert( *pField );
                        }
                        break;

                    case DB_Column::DB_COL_TEXT:
                        {
                            double nValue = DBL_MAX;
                            Reference< XPropertySet > xColumnProps;
                            xCols->getByName(pDBCol->pColInfo->sColumn) >>= xColumnProps;
                            sIns = SwDBManager::GetDBField(
                                                xColumnProps,
                                                aDBFormatData,
                                                &nValue );
                            if( pDBCol->DB_ColumnData.nFormat &&
                                DBL_MAX != nValue )
                            {
                                Color* pCol;
                                if(rNumFormatr.GetType(pDBCol->DB_ColumnData.nFormat) & css::util::NumberFormat::DATE)
                                {
                                    ::Date aStandard(1,1,1900);
                                    if (*rNumFormatr.GetNullDate() != aStandard)
                                        nValue += (aStandard - *rNumFormatr.GetNullDate());
                                }
                                rNumFormatr.GetOutputString( nValue,
                                            pDBCol->DB_ColumnData.nFormat,
                                            sIns, &pCol );
                            }
                        }
                        break;
                    }

                    if( !sIns.isEmpty() )
                    {
                        rSh.Insert( sIns );

                        if( bSetCursor)
                        {
                            // to the beginning and set a mark, so that
                            // the cursor can be set to the initial position
                            // at the end.

                            rSh.SwCursorShell::MovePara(
                                    GetfnParaCurr(), GetfnParaStart() );
                            pMark = rSh.SetBookmark(
                                    vcl::KeyCode(),
                                    OUString(),
                                    OUString(), IDocumentMarkAccess::MarkType::UNO_BOOKMARK );
                            rSh.SwCursorShell::MovePara(
                                    GetfnParaCurr(), GetfnParaEnd() );
                            bSetCursor = false;
                        }
                    }
                }

                if( !pSelection )
                {
                    bool bNext = xResultSet->next();
                    if(!bNext)
                        break;
                }
                else if( i+1 >= rSelection.getLength() )
                    break;

                if( m_pRbAsField->IsChecked() )
                    rSh.Insert( aNxtDBField );

                if( !rSh.IsSttPara() )
                    rSh.SwEditShell::SplitNode();

                if( 10 == i )
                    pWait.reset(new SwWait( *pView->GetDocShell(), true ));
            }

            if( !bSetCursor && pMark != nullptr)
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
        rSh.DoUndo();
        rSh.AppendUndoForInsertFromDB( bAsTable );
        rSh.EndUndo();
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

    if( pTAutoFormat )
    {
        if( pTAutoFormat->IsFrame() )
        {
            // border is from AutoFormat
            pTableSet->ClearItem( RES_BOX );
            pTableSet->ClearItem( SID_ATTR_BORDER_INNER );
        }
        if( pTAutoFormat->IsBackground() )
        {
            pTableSet->ClearItem( RES_BACKGROUND );
            pTableSet->ClearItem( SID_ATTR_BRUSH_ROW );
            pTableSet->ClearItem( SID_ATTR_BRUSH_TABLE );
        }
    }
    else
    {
        // remove the defaults again, it makes no sense to set them
        SvxBrushItem aBrush( RES_BACKGROUND );
        static const sal_uInt16 aIds[3] =
            { RES_BACKGROUND, SID_ATTR_BRUSH_ROW, SID_ATTR_BRUSH_TABLE };
        for( int i = 0; i < 3; ++i )
            if( SfxItemState::SET == pTableSet->GetItemState( aIds[ i ],
                false, &pItem ) && *pItem == aBrush )
                pTableSet->ClearItem( aIds[ i ] );
    }

    if( SfxItemState::SET == pTableSet->GetItemState( FN_PARAM_TABLE_NAME, false,
        &pItem ) && static_cast<const SfxStringItem*>(pItem)->GetValue() ==
                    rSh.GetTableFormat()->GetName() )
        pTableSet->ClearItem( FN_PARAM_TABLE_NAME );

    rSh.MoveTable( GetfnTableCurr(), GetfnTableStart() );
    rSh.SetMark();
    rSh.MoveTable( GetfnTableCurr(), GetfnTableEnd() );

    ItemSetToTableParam( *pTableSet, rSh );

    rSh.ClearMark();
    rSh.MoveTable( GetfnTableCurr(), GetfnTableStart() );
}

DB_ColumnConfigData::~DB_ColumnConfigData() {}

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
    const sal_Int32 nNames = aNames.getLength();
    sal_Int32 nIdx = nNames;
    const OUString* pNames = aNames.getConstArray();
    while(true)
    {
        const OUString sRet = "_" + OUString::number(nIdx++);
        sal_Int32 i = 0;
        while ( i < nNames && pNames[i] != sRet )
        {
            ++i;
        }
        if ( i >= nNames )
            return sRet;    // No match found, return unique name
    }
}

void SwInsertDBColAutoPilot::Notify( const css::uno::Sequence< OUString >&  ) {}

void SwInsertDBColAutoPilot::ImplCommit()
{
    Sequence <OUString> aNames = GetNodeNames(OUString());
    const OUString* pNames = aNames.getArray();
    //remove entries that contain this data source + table at first
    for(sal_Int32 nNode = 0; nNode < aNames.getLength(); nNode++)
    {
        Sequence<OUString> aSourceNames(2);
        OUString* pSourceNames = aSourceNames.getArray();
        pSourceNames[0] = pNames[nNode] + "/DataSource";
        pSourceNames[1] = pNames[nNode] + "/Command";
        Sequence<Any> aSourceProperties = GetProperties(aSourceNames);
        const Any* pSourceProps = aSourceProperties.getArray();
        OUString sSource, sCommand;
        pSourceProps[0] >>= sSource;
        pSourceProps[1] >>= sCommand;
        if(sSource==aDBData.sDataSource && sCommand==aDBData.sCommand)
        {
            Sequence<OUString> aElements { pNames[nNode] };
            ClearNodeElements(OUString(), aElements);
        }
    }

    aNames = GetNodeNames(OUString());
    OUString sNewNode = lcl_CreateUniqueName(aNames);
    Sequence<OUString> aNodeNames = lcl_createSourceNames(sNewNode);
    Sequence<PropertyValue> aValues(aNodeNames.getLength());
    PropertyValue* pValues = aValues.getArray();
    const OUString* pNodeNames = aNodeNames.getConstArray();
    for(sal_Int32 i = 0; i < aNodeNames.getLength(); i++)
    {
        pValues[i].Name = "/" + pNodeNames[i];
    }

    pValues[0].Value <<= aDBData.sDataSource;
    pValues[1].Value <<= aDBData.sCommand;
    pValues[2].Value <<= aDBData.nCommandType;
    pValues[3].Value <<= m_pEdDbText->GetText();

    OUString sTmp;
    const sal_Int32 nCnt = m_pLbTableCol->GetEntryCount();
    for( sal_Int32 n = 0; n < nCnt; ++n )
        sTmp += m_pLbTableCol->GetEntry(n) + "\x0a";

    if (!sTmp.isEmpty())
        pValues[4].Value <<= sTmp;

    if( sNoTmpl != (sTmp = m_pLbDbParaColl->GetSelectEntry()) )
        pValues[5].Value <<= sTmp;

    if( pTAutoFormat )
        pValues[6].Value <<= pTAutoFormat->GetName();

    pValues[7].Value <<= m_pRbAsTable->IsChecked();
    pValues[8].Value <<= m_pRbAsField->IsChecked();
    pValues[9].Value <<= m_pCbTableHeadon->IsChecked();
    pValues[10].Value <<= m_pRbHeadlEmpty->IsChecked();

    SetSetProperties(OUString(), aValues);

    sNewNode += "/ColumnSet";

    LanguageType ePrevLang = (LanguageType)-1;

    SvNumberFormatter& rNFormatr = *pView->GetWrtShell().GetNumberFormatter();
    for(size_t nCol = 0; nCol < aDBColumns.size(); nCol++)
    {
        SwInsDBColumn* pColumn = aDBColumns[nCol];
        OUString sColumnInsertNode(sNewNode + "/__");
        if( nCol < 10 )
            sColumnInsertNode += "00";
        else if( nCol < 100 )
            sColumnInsertNode += "0";
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
        pSubValues[2].Value <<= pColumn->bHasFormat;
        pSubValues[3].Value <<= pColumn->bIsDBFormat;

        SwStyleNameMapper::FillUIName( RES_POOLCOLL_STANDARD, sTmp );
        const SvNumberformat* pNF = rNFormatr.GetEntry( pColumn->nUsrNumFormat );
        LanguageType eLang;
        if( pNF )
        {
            pSubValues[4].Value <<= pNF->GetFormatstring();
            eLang = pNF->GetLanguage();
        }
        else
        {
            pSubValues[4].Value <<= sTmp;
            eLang = GetAppLanguage();
        }

        OUString sPrevLang;
        if( eLang != ePrevLang )
        {
            sPrevLang = LanguageTag::convertToBcp47( eLang );
            ePrevLang = eLang;
        }

        pSubValues[5].Value <<=  sPrevLang;
        SetSetProperties(sNewNode, aSubValues);
    }
}

void SwInsertDBColAutoPilot::Load()
{
    Sequence <OUString> aNames = GetNodeNames(OUString());
    const OUString* pNames = aNames.getArray();
    SvNumberFormatter& rNFormatr = *pView->GetWrtShell().GetNumberFormatter();
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
            std::unique_ptr<DB_ColumnConfigData> pNewData(new DB_ColumnConfigData);
            pNewData->sSource = sSource;
            pNewData->sTable = sCommand;

            pDataSourceProps[3] >>= pNewData->sEdit;
            pDataSourceProps[4] >>= pNewData->sTableList;
            pDataSourceProps[5] >>= pNewData->sTmplNm;
            pDataSourceProps[6] >>= pNewData->sTAutoFormatNm;
            if(pDataSourceProps[7].hasValue())
                pNewData->bIsTable = *static_cast<sal_Bool const *>(pDataSourceProps[7].getValue());
            if(pDataSourceProps[8].hasValue())
                 pNewData->bIsField = *static_cast<sal_Bool const *>(pDataSourceProps[8].getValue());
            if(pDataSourceProps[9].hasValue())
                 pNewData->bIsHeadlineOn = *static_cast<sal_Bool const *>(pDataSourceProps[9].getValue());
            if(pDataSourceProps[10].hasValue())
                 pNewData->bIsEmptyHeadln = *static_cast<sal_Bool const *>(pDataSourceProps[10].getValue());

            const OUString sSubNodeName(pNames[nNode] + "/ColumnSet/");
            Sequence <OUString> aSubNames = GetNodeNames(sSubNodeName);
            const OUString* pSubNames = aSubNames.getConstArray();
            for(sal_Int32 nSub = 0; nSub < aSubNames.getLength(); nSub++)
            {
                Sequence <OUString> aSubNodeNames =
                    lcl_CreateSubNames(sSubNodeName + pSubNames[nSub]);
                Sequence< Any> aSubProps = GetProperties(aSubNodeNames);
                const Any* pSubProps = aSubProps.getConstArray();

                OUString sColumn;
                pSubProps[0] >>= sColumn;
                // check for existence of the loaded column name
                bool bFound = false;
                for(size_t nRealColumn = 0; nRealColumn < aDBColumns.size(); ++nRealColumn)
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
                SwInsDBColumn* pInsDBColumn = new SwInsDBColumn(sColumn);
                if(pSubProps[2].hasValue())
                    pInsDBColumn->bHasFormat = *static_cast<sal_Bool const *>(pSubProps[2].getValue());
                if(pSubProps[3].hasValue())
                    pInsDBColumn->bIsDBFormat = *static_cast<sal_Bool const *>(pSubProps[3].getValue());

                pSubProps[4] >>= pInsDBColumn->sUsrNumFormat;
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
                pInsDBColumn->eUsrNumFormatLng = LanguageTag::convertToLanguageType( sNumberFormatLocale );

                pInsDBColumn->nUsrNumFormat = rNFormatr.GetEntryKey( pInsDBColumn->sUsrNumFormat,
                                                        pInsDBColumn->eUsrNumFormatLng );

                pNewData->aDBColumns.insert(pInsDBColumn);
            }
            OUString sTmp( pNewData->sTableList );
            if( !sTmp.isEmpty() )
            {
                sal_Int32 n = 0;
                do {
                    const OUString sEntry( sTmp.getToken( 0, '\x0a', n ) );
                    //preselect column - if they still exist!
                    if(m_pLbTableDbColumn->GetEntryPos(sEntry) != LISTBOX_ENTRY_NOTFOUND)
                    {
                        m_pLbTableCol->InsertEntry( sEntry );
                        m_pLbTableDbColumn->RemoveEntry( sEntry );
                    }
                } while( n>=0 );

                if( !m_pLbTableDbColumn->GetEntryCount() )
                {
                    m_pIbDbcolAllTo->Enable( false );
                    m_pIbDbcolOneTo->Enable( false );
                }
                m_pIbDbcolOneFrom->Enable();
                m_pIbDbcolAllFrom->Enable();
            }
            m_pEdDbText->SetText( pNewData->sEdit );

            sTmp = pNewData->sTmplNm;
            if( !sTmp.isEmpty() )
                m_pLbDbParaColl->SelectEntry( sTmp );
            else
                m_pLbDbParaColl->SelectEntryPos( 0 );

            delete pTAutoFormat;
            pTAutoFormat = nullptr;
            sTmp = pNewData->sTAutoFormatNm;
            if( !sTmp.isEmpty() )
            {
                // then load the AutoFormat file and look for Autoformat first
                SwTableAutoFormatTable aAutoFormatTable;
                aAutoFormatTable.Load();
                for( size_t nAutoFormat = aAutoFormatTable.size(); nAutoFormat; )
                    if( sTmp == aAutoFormatTable[ --nAutoFormat ].GetName() )
                    {
                        pTAutoFormat = new SwTableAutoFormat( aAutoFormatTable[ nAutoFormat ] );
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
            for( size_t n = 0; n < aDBColumns.size() ; ++n )
            {
                SwInsDBColumn& rSet = *aDBColumns[ n ];
                for( size_t m = 0; m < pNewData->aDBColumns.size() ; ++m )
                {
                    SwInsDBColumn& rGet = *pNewData->aDBColumns[ m ];
                    if(rGet.sColumn == rSet.sColumn)
                    {
                        if( rGet.bHasFormat && !rGet.bIsDBFormat )
                        {
                            rSet.bIsDBFormat = false;
                            rSet.nUsrNumFormat = rNFormatr.GetEntryKey( rGet.sUsrNumFormat,
                                                                    rGet.eUsrNumFormatLng );
                            if( NUMBERFORMAT_ENTRY_NOT_FOUND == rSet.nUsrNumFormat )
                            {
                                sal_Int32 nCheckPos;
                                short nType;
                                rNFormatr.PutEntry( rGet.sUsrNumFormat, nCheckPos, nType,
                                                rSet.nUsrNumFormat, rGet.eUsrNumFormatLng );
                            }
                        }
                        break;
                    }
                }
            }

            // when the cursor is inside of a table, table must NEVER be selectable
            if( !m_pRbAsTable->IsEnabled() && m_pRbAsTable->IsChecked() )
                m_pRbAsField->Check();
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
