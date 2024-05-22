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

#include <dbinsdlg.hxx>

#include <float.h>

#include <hintids.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <svl/numuno.hxx>
#include <svl/numformat.hxx>
#include <svl/stritem.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <utility>
#include <vcl/mnemonic.hxx>
#include <svl/style.hxx>
#include <svl/zformat.hxx>
#include <sfx2/htmlmode.hxx>
#include <svl/itemset.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/boxitem.hxx>
#include <unotools/collatorwrapper.hxx>
#include <fmtclds.hxx>
#include <tabcol.hxx>
#include <uiitems.hxx>
#include <viewopt.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <dbmgr.hxx>
#include <tblafmt.hxx>
#include <cellatr.hxx>
#include <swtablerep.hxx>
#include <dbfld.hxx>
#include <fmtcol.hxx>
#include <swwait.hxx>
#include <modcfg.hxx>
#include <swmodule.hxx>
#include <poolfmt.hxx>
#include <connectivity/dbtools.hxx>

#include <cmdid.h>
#include <SwStyleNameMapper.hxx>
#include <tabsh.hxx>
#include <swabstdlg.hxx>
#include <strings.hrc>
#include <IDocumentMarkAccess.hxx>

#include <o3tl/any.hxx>

#include <memory>
#include <string_view>

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
    const enum class Type { FILLTEXT, COL_FIELD, COL_TEXT, SPLITPARA } eColType;

    union {
        OUString* pText;
        SwField* pField;
        sal_uInt32 nFormat;
    };
    const SwInsDBColumn* pColInfo;

    DB_Column() : eColType(Type::SPLITPARA),
                  pText(nullptr),
                  pColInfo(nullptr)
    {}

    explicit DB_Column( const OUString& rText )
                        : eColType(Type::FILLTEXT),
                          pText(new OUString(rText)),
                          pColInfo(nullptr)
    {}

    DB_Column( const SwInsDBColumn& rInfo, sal_uInt32 nFormat_ )
                        : eColType(Type::COL_TEXT),
                          nFormat(nFormat_),
                          pColInfo(&rInfo)
    {}

    DB_Column( const SwInsDBColumn& rInfo, SwDBField& rField )
                        : eColType(Type::COL_FIELD),
                          pField(&rField),
                          pColInfo(&rInfo)
    {}

    ~DB_Column()
    {
        if( Type::COL_FIELD == eColType )
            delete pField;
        else if( Type::FILLTEXT == eColType )
            delete pText;
    }
};

namespace {

struct DB_ColumnConfigData
{
    SwInsDBColumns aDBColumns;
    OUString sEdit;
    OUString sTableList;
    OUString sTmplNm;
    OUString sTAutoFormatNm;
    bool bIsTable : 1,
         bIsField : 1,
         bIsHeadlineOn : 1,
         bIsEmptyHeadln : 1;

    DB_ColumnConfigData(DB_ColumnConfigData const&) = delete;
    DB_ColumnConfigData& operator=(DB_ColumnConfigData const&) = delete;

    DB_ColumnConfigData()
    {
        bIsTable = bIsHeadlineOn = true;
        bIsField = bIsEmptyHeadln = false;
    }
};

}

bool SwInsDBColumn::operator<( const SwInsDBColumn& rCmp ) const
{
    return 0 > GetAppCollator().compareString( sColumn, rCmp.sColumn );
}

SwInsertDBColAutoPilot::SwInsertDBColAutoPilot( SwView& rView,
        Reference<XDataSource> const & xDataSource,
        Reference<sdbcx::XColumnsSupplier> const & xColSupp,
        SwDBData aData )
    : SfxDialogController(rView.GetWindow()->GetFrameWeld(), u"modules/swriter/ui/insertdbcolumnsdialog.ui"_ustr, u"InsertDbColumnsDialog"_ustr)
    , ConfigItem(u"Office.Writer/InsertData/DataSet"_ustr, ConfigItemMode::NONE)
    , m_aDBData(std::move(aData))
    , m_sNoTmpl(SwResId(SW_STR_NONE))
    , m_pView(&rView)
    , m_xRbAsTable(m_xBuilder->weld_radio_button(u"astable"_ustr))
    , m_xRbAsField(m_xBuilder->weld_radio_button(u"asfields"_ustr))
    , m_xRbAsText(m_xBuilder->weld_radio_button(u"astext"_ustr))
    , m_xHeadFrame(m_xBuilder->weld_frame(u"dbframe"_ustr))
    , m_xLbTableDbColumn(m_xBuilder->weld_tree_view(u"tabledbcols"_ustr))
    , m_xLbTextDbColumn(m_xBuilder->weld_tree_view(u"tabletxtcols"_ustr))
    , m_xFormatFrame(m_xBuilder->weld_frame(u"formatframe"_ustr))
    , m_xRbDbFormatFromDb(m_xBuilder->weld_radio_button(u"fromdatabase"_ustr))
    , m_xRbDbFormatFromUsr(m_xBuilder->weld_radio_button(u"userdefined"_ustr))
    , m_xLbDbFormatFromUsr(new NumFormatListBox(m_xBuilder->weld_combo_box(u"numformat"_ustr)))
    , m_xIbDbcolToEdit(m_xBuilder->weld_button(u"toedit"_ustr))
    , m_xEdDbText(m_xBuilder->weld_text_view(u"textview"_ustr))
    , m_xFtDbParaColl(m_xBuilder->weld_label(u"parastylelabel"_ustr))
    , m_xLbDbParaColl(m_xBuilder->weld_combo_box(u"parastyle"_ustr))
    , m_xIbDbcolAllTo(m_xBuilder->weld_button(u"oneright"_ustr))
    , m_xIbDbcolOneTo(m_xBuilder->weld_button(u"allright"_ustr))
    , m_xIbDbcolOneFrom(m_xBuilder->weld_button(u"oneleft"_ustr))
    , m_xIbDbcolAllFrom(m_xBuilder->weld_button(u"allleft"_ustr))
    , m_xFtTableCol(m_xBuilder->weld_label(u"tablecolft"_ustr))
    , m_xLbTableCol(m_xBuilder->weld_tree_view(u"tablecols"_ustr))
    , m_xCbTableHeadon(m_xBuilder->weld_check_button(u"tableheading"_ustr))
    , m_xRbHeadlColnms(m_xBuilder->weld_radio_button(u"columnname"_ustr))
    , m_xRbHeadlEmpty(m_xBuilder->weld_radio_button(u"rowonly"_ustr))
    , m_xPbTableFormat(m_xBuilder->weld_button(u"tableformat"_ustr))
    , m_xPbTableAutofmt(m_xBuilder->weld_button(u"autoformat"_ustr))
{
    m_xEdDbText->set_size_request(m_xEdDbText->get_approximate_digit_width() * 40, -1);
    m_xLbDbParaColl->make_sorted();

    m_nGBFormatLen = m_xFormatFrame->get_label().getLength();

    if (xColSupp.is())
    {
        SwWrtShell& rSh = m_pView->GetWrtShell();
        SvNumberFormatter* pNumFormatr = rSh.GetNumberFormatter();
        rtl::Reference<SvNumberFormatsSupplierObj> pNumFormat = new SvNumberFormatsSupplierObj( pNumFormatr );
        Reference< util::XNumberFormats > xDocNumberFormats = pNumFormat->getNumberFormats();
        Reference< util::XNumberFormatTypes > xDocNumberFormatTypes(xDocNumberFormats, UNO_QUERY);

        Reference<XPropertySet> xSourceProps(xDataSource, UNO_QUERY);
        Reference< util::XNumberFormats > xNumberFormats;
        if(xSourceProps.is())
        {
            Any aFormats = xSourceProps->getPropertyValue(u"NumberFormatsSupplier"_ustr);
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
        const Sequence<OUString> aColNames = xCols->getElementNames();
        for (const OUString& rColName : aColNames)
        {
            std::unique_ptr<SwInsDBColumn> pNew(new SwInsDBColumn( rColName ));
            Any aCol = xCols->getByName(rColName);
            Reference <XPropertySet> xCol;
            aCol >>= xCol;
            Any aType = xCol->getPropertyValue(u"Type"_ustr);
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
                    Any aFormat = xCol->getPropertyValue(u"FormatKey"_ustr);
                    if(aFormat.hasValue())
                    {
                        sal_Int32 nFormat = 0;
                        aFormat >>= nFormat;
                        if(xNumberFormats.is())
                        {
                            try
                            {
                                Reference<XPropertySet> xNumProps = xNumberFormats->getByKey( nFormat );
                                Any aFormatVal = xNumProps->getPropertyValue(u"FormatString"_ustr);
                                Any aLocale = xNumProps->getPropertyValue(u"Locale"_ustr);
                                OUString sFormat;
                                aFormatVal >>= sFormat;
                                lang::Locale aLoc;
                                aLocale >>= aLoc;
                                sal_Int32 nKey = xDocNumberFormats->queryKey( sFormat, aLoc, true);
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
            if( !m_aDBColumns.insert( std::move(pNew) ).second )
            {
                OSL_ENSURE( false, "Spaltenname mehrfach vergeben?" );
            }
        }
    }

    // fill paragraph templates-ListBox
    {
        SfxStyleSheetBasePool* pPool = m_pView->GetDocShell()->GetStyleSheetPool();
        m_xLbDbParaColl->append_text( m_sNoTmpl );

        const SfxStyleSheetBase* pBase = pPool->First(SfxStyleFamily::Para);
        while( pBase )
        {
            m_xLbDbParaColl->append_text( pBase->GetName() );
            pBase = pPool->Next();
        }
        m_xLbDbParaColl->set_active( 0 );
    }

    // when the cursor is inside of a table, table must NEVER be selectable
    if( m_pView->GetWrtShell().GetTableFormat() )
    {
        m_xRbAsField->set_active(true);
        m_xRbAsTable->set_sensitive(false);
        m_xRbDbFormatFromDb->set_active(true);
    }
    else
    {
        m_xRbAsTable->set_active(true);
        m_xRbDbFormatFromDb->set_active(true);
        m_xIbDbcolOneFrom->set_sensitive( false );
        m_xIbDbcolAllFrom->set_sensitive( false );
    }

    // by default, select header button
    m_xRbHeadlColnms->set_active(true);
    m_xRbHeadlEmpty->set_active(false);

    m_xRbAsTable->connect_toggled( LINK(this, SwInsertDBColAutoPilot, PageHdl ));
    m_xRbAsField->connect_toggled( LINK(this, SwInsertDBColAutoPilot, PageHdl ));
    m_xRbAsText->connect_toggled( LINK(this, SwInsertDBColAutoPilot, PageHdl ));

    m_xRbDbFormatFromDb->connect_toggled( LINK(this, SwInsertDBColAutoPilot, DBFormatHdl ));
    m_xRbDbFormatFromUsr->connect_toggled( LINK(this, SwInsertDBColAutoPilot, DBFormatHdl ));

    m_xPbTableFormat->connect_clicked(LINK(this, SwInsertDBColAutoPilot, TableFormatHdl ));
    m_xPbTableAutofmt->connect_clicked(LINK(this, SwInsertDBColAutoPilot, AutoFormatHdl ));

    m_xIbDbcolAllTo->connect_clicked( LINK(this, SwInsertDBColAutoPilot, TableToFromHdl ));
    m_xIbDbcolOneTo->connect_clicked( LINK(this, SwInsertDBColAutoPilot, TableToFromHdl ));
    m_xIbDbcolOneFrom->connect_clicked( LINK(this, SwInsertDBColAutoPilot, TableToFromHdl ));
    m_xIbDbcolAllFrom->connect_clicked( LINK(this, SwInsertDBColAutoPilot, TableToFromHdl ));
    m_xIbDbcolToEdit->connect_clicked( LINK(this, SwInsertDBColAutoPilot, TableToFromHdl ));

    m_xCbTableHeadon->connect_toggled( LINK(this, SwInsertDBColAutoPilot, HeaderHdl ));

    m_xLbTextDbColumn->connect_changed( LINK( this, SwInsertDBColAutoPilot, TVSelectHdl ));
    m_xLbTableDbColumn->connect_changed( LINK( this, SwInsertDBColAutoPilot, TVSelectHdl ));
    m_xLbDbFormatFromUsr->connect_changed( LINK( this, SwInsertDBColAutoPilot, CBSelectHdl ));
    m_xLbTableCol->connect_changed( LINK( this, SwInsertDBColAutoPilot, TVSelectHdl ));

    m_xLbTextDbColumn->connect_row_activated( LINK( this, SwInsertDBColAutoPilot, DblClickHdl ));
    m_xLbTableDbColumn->connect_row_activated( LINK( this, SwInsertDBColAutoPilot, DblClickHdl ));
    m_xLbTableCol->connect_row_activated( LINK( this, SwInsertDBColAutoPilot, DblClickHdl ));

    for( size_t n = 0; n < m_aDBColumns.size(); ++n )
    {
        const OUString& rS = m_aDBColumns[ n ]->sColumn;
        m_xLbTableDbColumn->append_text(rS);
        m_xLbTextDbColumn->append_text(rS);
    }
    m_xLbTextDbColumn->select(0);
    m_xLbTableDbColumn->select(0);

    // read configuration
    Load();

    // lock size to widest config
    m_xHeadFrame->set_size_request(m_xHeadFrame->get_preferred_size().Width(), -1);
    // initialise Controls:
    PageHdl(m_xRbAsTable->get_active() ? *m_xRbAsTable : *m_xRbAsField);
}

SwInsertDBColAutoPilot::~SwInsertDBColAutoPilot()
{
}

IMPL_LINK( SwInsertDBColAutoPilot, PageHdl, weld::Toggleable&, rButton, void )
{
    if (!rButton.get_active())
        return;

    bool bShowTable = m_xRbAsTable->get_active();

    weld::RadioButton& rRadio = dynamic_cast<weld::RadioButton&>(rButton);
    m_xHeadFrame->set_label(MnemonicGenerator::EraseAllMnemonicChars(rRadio.get_label().replace('_', '~')));

    m_xLbTextDbColumn->set_visible( !bShowTable );
    m_xIbDbcolToEdit->set_visible( !bShowTable );
    m_xEdDbText->set_visible( !bShowTable );
    m_xFtDbParaColl->set_visible( !bShowTable );
    m_xLbDbParaColl->set_visible( !bShowTable );

    m_xLbTableDbColumn->set_visible( bShowTable );
    m_xIbDbcolAllTo->set_visible( bShowTable );
    m_xIbDbcolOneTo->set_visible( bShowTable );
    m_xIbDbcolOneFrom->set_visible( bShowTable );
    m_xIbDbcolAllFrom->set_visible( bShowTable );
    m_xFtTableCol->set_visible( bShowTable );
    m_xLbTableCol->set_visible( bShowTable );
    m_xCbTableHeadon->set_visible( bShowTable );
    m_xRbHeadlColnms->set_visible( bShowTable );
    m_xRbHeadlEmpty->set_visible( bShowTable );
    m_xPbTableFormat->set_visible( bShowTable );
    m_xPbTableAutofmt->set_visible( bShowTable );

    if( bShowTable )
        m_xPbTableFormat->set_sensitive( 0 != m_xLbTableCol->n_children() );

    TVSelectHdl( bShowTable ? *m_xLbTableDbColumn : *m_xLbTextDbColumn );
}

IMPL_LINK( SwInsertDBColAutoPilot, DBFormatHdl, weld::Toggleable&, rButton, void )
{
    if (!rButton.get_active())
        return;

    weld::TreeView& rBox = m_xRbAsTable->get_active()
                        ? ( m_xLbTableCol->get_id(0).isEmpty()
                            ? *m_xLbTableDbColumn
                            : *m_xLbTableCol )
                        : *m_xLbTextDbColumn;

    SwInsDBColumn aSrch(rBox.get_selected_text());
    SwInsDBColumns::const_iterator it = m_aDBColumns.find( &aSrch );

    bool bFromDB = m_xRbDbFormatFromDb->get_active();
    (*it)->bIsDBFormat = bFromDB;
    m_xLbDbFormatFromUsr->set_sensitive( !bFromDB );
}

IMPL_LINK( SwInsertDBColAutoPilot, TableToFromHdl, weld::Button&, rButton, void )
{
    bool bChgEnable = true, bEnableTo = true, bEnableFrom = true;

    if( &rButton == m_xIbDbcolAllTo.get() )
    {
        bEnableTo = false;

        sal_Int32 n, nInsPos = m_xLbTableCol->get_selected_index(),
               nCnt = m_xLbTableDbColumn->n_children();

        m_xLbTableDbColumn->unselect_all();

        m_xLbTableDbColumn->freeze();
        m_xLbTableCol->freeze();

        if (nInsPos == -1)
            for( n = 0; n < nCnt; ++n )
                m_xLbTableCol->append_text(m_xLbTableDbColumn->get_text(n));
        else
            for( n = 0; n < nCnt; ++n, ++nInsPos )
                m_xLbTableCol->insert_text(nInsPos, m_xLbTableDbColumn->get_text(n));
        m_xLbTableDbColumn->clear();

        m_xLbTableDbColumn->thaw();
        m_xLbTableCol->thaw();

        m_xLbTableCol->select(nInsPos);
    }
    else if( &rButton == m_xIbDbcolOneTo.get() &&
            m_xLbTableDbColumn->get_selected_index() != -1 )
    {
        sal_Int32 nInsPos = m_xLbTableCol->get_selected_index(),
               nDelPos = m_xLbTableDbColumn->get_selected_index();
        m_xLbTableCol->insert_text(nInsPos, m_xLbTableDbColumn->get_text(nDelPos));
        m_xLbTableDbColumn->remove(nDelPos);

        m_xLbTableCol->select(nInsPos);
        if (nDelPos >= m_xLbTableDbColumn->n_children())
            nDelPos = m_xLbTableDbColumn->n_children() - 1;
        m_xLbTableDbColumn->select(nDelPos);

        bEnableTo = 0 != m_xLbTableDbColumn->n_children();
    }
    else if( &rButton == m_xIbDbcolOneFrom.get() )
    {
        if (m_xLbTableCol->get_selected_index() != -1)
        {
            sal_Int32 nInsPos,
                    nDelPos = m_xLbTableCol->get_selected_index();

            // look for the right InsertPos!!
            SwInsDBColumn aSrch(m_xLbTableCol->get_text(nDelPos));
            SwInsDBColumns::const_iterator it = m_aDBColumns.find( &aSrch );
            if( it == m_aDBColumns.begin() || (it+1) == m_aDBColumns.end() )
                nInsPos = it - m_aDBColumns.begin();
            else
            {
                nInsPos = -1;
                while( ++it != m_aDBColumns.end() &&
                        -1 == (nInsPos = m_xLbTableDbColumn->
                        find_text( (*it)->sColumn )) )
                    ;
            }

            m_xLbTableDbColumn->insert_text(nInsPos, aSrch.sColumn);
            m_xLbTableCol->remove( nDelPos );

            if (nInsPos >= m_xLbTableDbColumn->n_children())
                nInsPos = m_xLbTableDbColumn->n_children() - 1;
            m_xLbTableDbColumn->select(nInsPos);

            if (nDelPos >= m_xLbTableCol->n_children())
                nDelPos = m_xLbTableCol->n_children() - 1;
            m_xLbTableCol->select(nDelPos);
        }
        else
            bEnableTo = 0 != m_xLbTableDbColumn->n_children();

        bEnableFrom = 0 != m_xLbTableCol->n_children();
    }
    else if( &rButton == m_xIbDbcolAllFrom.get() )
    {
        bEnableFrom = false;

        m_xLbTableDbColumn->freeze();
        m_xLbTableCol->freeze();

        m_xLbTableDbColumn->clear();
        m_xLbTableCol->clear();
        for (size_t n = 0; n < m_aDBColumns.size(); ++n)
            m_xLbTableDbColumn->append_text(m_aDBColumns[n]->sColumn);

        m_xLbTableDbColumn->thaw();
        m_xLbTableCol->thaw();

        m_xLbTableDbColumn->select(0);
    }
    else if( &rButton == m_xIbDbcolToEdit.get() )
    {
        bChgEnable = false;
        // move data to Edit:
        OUString aField(m_xLbTextDbColumn->get_selected_text());
        if( !aField.isEmpty() )
        {
            OUString aStr( m_xEdDbText->get_text() );
            int nStartPos, nEndPos;
            m_xEdDbText->get_selection_bounds(nStartPos, nEndPos);
            sal_Int32 nPos = std::min(nStartPos, nEndPos);
            sal_Int32 nMax = std::max(nStartPos, nEndPos);
            const sal_Int32 nSel = nMax - nPos;
            if( nSel )
                // first delete the existing selection
                aStr = aStr.replaceAt( nPos, nSel, u"" );

            aField = OUStringChar(cDBFieldStart) + aField + OUStringChar(cDBFieldEnd);
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

            m_xEdDbText->set_text( aStr.replaceAt( nPos, 0, aField ) );
            nPos += aField.getLength();
            m_xEdDbText->select_region(nPos, nPos);
        }
    }

    if( !bChgEnable )
        return;

    m_xIbDbcolOneTo->set_sensitive( bEnableTo );
    m_xIbDbcolAllTo->set_sensitive( bEnableTo );
    m_xIbDbcolOneFrom->set_sensitive( bEnableFrom );
    m_xIbDbcolAllFrom->set_sensitive( bEnableFrom );

    m_xRbDbFormatFromDb->set_sensitive( false );
    m_xRbDbFormatFromUsr->set_sensitive( false );
    m_xLbDbFormatFromUsr->set_sensitive( false );

    m_xPbTableFormat->set_sensitive( bEnableFrom );
}

IMPL_LINK(SwInsertDBColAutoPilot, DblClickHdl, weld::TreeView&, rBox, bool)
{
    weld::Button* pButton = nullptr;
    if( &rBox == m_xLbTextDbColumn.get() )
        pButton = m_xIbDbcolToEdit.get();
    else if( &rBox == m_xLbTableDbColumn.get() && m_xIbDbcolOneTo->get_sensitive() )
        pButton = m_xIbDbcolOneTo.get();
    else if( &rBox == m_xLbTableCol.get() && m_xIbDbcolOneFrom->get_sensitive() )
        pButton = m_xIbDbcolOneFrom.get();

    if (pButton)
        TableToFromHdl(*pButton);

    return true;
}

IMPL_LINK_NOARG(SwInsertDBColAutoPilot, TableFormatHdl, weld::Button&, void)
{
    SwWrtShell& rSh = m_pView->GetWrtShell();
    bool bNewSet = false;
    if( !m_pTableSet )
    {
        bNewSet = true;
        m_pTableSet.reset(new SfxItemSet( rSh.GetAttrPool(), SwuiGetUITableAttrRange() ));

        // At first acquire the simple attributes
        m_pTableSet->Put( SfxStringItem( FN_PARAM_TABLE_NAME, rSh.GetUniqueTableName() ));
        m_pTableSet->Put( SfxUInt16Item( FN_PARAM_TABLE_HEADLINE, 1 ) );

        m_pTableSet->Put( SfxUInt16Item( SID_BACKGRND_DESTINATION,
                                    rSh.GetViewOptions()->GetTableDest() ));

        SvxBrushItem aBrush( RES_BACKGROUND );
        m_pTableSet->Put( aBrush );
        aBrush.SetWhich(SID_ATTR_BRUSH_ROW);
        m_pTableSet->Put( aBrush );
        aBrush.SetWhich(SID_ATTR_BRUSH_TABLE);
        m_pTableSet->Put( aBrush );

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
        m_pTableSet->Put( aBoxInfo );

        SwGetCurColNumPara aPara;
        const sal_uInt16 nNum = rSh.GetCurColNum( &aPara );
        tools::Long nWidth;

        if( nNum )
        {
            nWidth = aPara.pPrtRect->Width();
            const SwFormatCol& rCol = aPara.pFrameFormat->GetCol();
            const SwColumns& rCols = rCol.GetColumns();

            // initialise nStart and nEnd for nNum == 0
            tools::Long nWidth1 = 0,
                nStart1 = 0,
                nEnd1 = nWidth;
            for( sal_uInt16 i = 0; i < nNum; ++i )
            {
                const SwColumn* pCol = &rCols[i];
                nStart1 = pCol->GetLeft() + nWidth1;
                nWidth1 += static_cast<tools::Long>(rCol.CalcColWidth( i, o3tl::narrowing<sal_uInt16>(nWidth) ));
                nEnd1 = nWidth1 - pCol->GetRight();
            }
            if(nStart1 || nEnd1 != nWidth)
                nWidth = nEnd1 - nStart1;
        }
        else
            nWidth = rSh.GetAnyCurRect(
                                (FrameTypeFlags::FLY_ANY & rSh.GetFrameType( nullptr, true ))
                                              ? CurRectType::FlyEmbeddedPrt
                                              : CurRectType::PagePrt ).Width();

        SwTabCols aTabCols;
        aTabCols.SetRight( nWidth );
        aTabCols.SetRightMax( nWidth );
        m_pRep.reset(new SwTableRep( aTabCols ));
        m_pRep->SetAlign( text::HoriOrientation::NONE );
        m_pRep->SetSpace( nWidth );
        m_pRep->SetWidth( nWidth );
        m_pRep->SetWidthPercent( 100 );
        m_pTableSet->Put( SwPtrItem( FN_TABLE_REP, m_pRep.get() ));

        m_pTableSet->Put( SfxUInt16Item( SID_HTML_MODE,
                    ::GetHtmlMode( m_pView->GetDocShell() )));
    }

    sal_Int32 nCols = m_xLbTableCol->n_children();
    if (nCols != m_pRep->GetAllColCount() && nCols > 0)
    {
        // Number of columns has changed: then the TabCols have to be adjusted
        tools::Long nWidth = m_pRep->GetWidth();
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
        m_pRep.reset(new SwTableRep( aTabCols ));
        m_pRep->SetAlign( text::HoriOrientation::NONE );
        m_pRep->SetSpace( nWidth );
        m_pRep->SetWidth( nWidth );
        m_pRep->SetWidthPercent( 100 );
        m_pTableSet->Put( SwPtrItem( FN_TABLE_REP, m_pRep.get() ));
    }

    SwAbstractDialogFactory& rFact = swui::GetFactory();

    VclPtr<SfxAbstractTabDialog> pDlg(rFact.CreateSwTableTabDlg(m_xDialog.get(), m_pTableSet.get(), &rSh));
    pDlg->StartExecuteAsync(
        [this, pDlg, bNewSet] (sal_Int32 nResult)->void
        {
            if( nResult == RET_OK )
                m_pTableSet->Put( *pDlg->GetOutputItemSet() );
            else if( bNewSet )
            {
                m_pTableSet.reset();
                m_pRep.reset();
            }
            pDlg->disposeOnce();
        }
    );
}

IMPL_LINK_NOARG(SwInsertDBColAutoPilot, AutoFormatHdl, weld::Button&, void)
{
    SwAbstractDialogFactory& rFact = swui::GetFactory();

    VclPtr<AbstractSwAutoFormatDlg> pDlg(rFact.CreateSwAutoFormatDlg(m_xDialog.get(), m_pView->GetWrtShellPtr(), false, m_xTAutoFormat.get()));
    pDlg->StartExecuteAsync(
        [this, pDlg] (sal_Int32 nResult)->void
        {
            if (nResult == RET_OK)
            {
                pDlg->Apply();
                m_xTAutoFormat = pDlg->FillAutoFormatOfIndex();
            }
            pDlg->disposeOnce();
        }
    );
}

IMPL_LINK(SwInsertDBColAutoPilot, TVSelectHdl, weld::TreeView&, rBox, void)
{
    weld::TreeView* pGetBox = &rBox;

    SwInsDBColumn aSrch(pGetBox->get_selected_text());
    SwInsDBColumns::const_iterator it = m_aDBColumns.find( &aSrch );

    // set the selected FieldName at the FormatGroupBox, so that
    // it's clear what field is configured by the format!
    OUString sText( m_xFormatFrame->get_label().copy( 0, m_nGBFormatLen ));
    if( aSrch.sColumn.isEmpty() )
    {
        m_xRbDbFormatFromDb->set_sensitive( false );
        m_xRbDbFormatFromUsr->set_sensitive( false );
        m_xLbDbFormatFromUsr->set_sensitive( false );
    }
    else
    {
        bool bEnableFormat = (*it)->bHasFormat;
        m_xRbDbFormatFromDb->set_sensitive( bEnableFormat );
        m_xRbDbFormatFromUsr->set_sensitive( bEnableFormat );

        if( bEnableFormat )
        {
            sText += " ("  + aSrch.sColumn + ")";
        }

        bool bIsDBFormat = (*it)->bIsDBFormat;
        m_xRbDbFormatFromDb->set_active( bIsDBFormat );
        m_xRbDbFormatFromUsr->set_active( !bIsDBFormat );
        m_xLbDbFormatFromUsr->set_sensitive( !bIsDBFormat );
        if( !bIsDBFormat )
            m_xLbDbFormatFromUsr->SetDefFormat( (*it)->nUsrNumFormat );
    }

    m_xFormatFrame->set_label(sText);

    if (m_xLbTableCol->n_children())
    {
        // to know later on, what ListBox was the "active", a Flag
        // is remembered in the 1st entry
        if (&rBox == m_xLbTableCol.get())
            m_xLbTableCol->set_id(0, u"tablecols"_ustr);
        else
            m_xLbTableCol->set_id(0, OUString());
    }
}

IMPL_LINK_NOARG(SwInsertDBColAutoPilot, CBSelectHdl, weld::ComboBox&, void)
{
    weld::TreeView* pGetBox = m_xRbAsTable->get_active()
                                    ? ( m_xLbTableCol->get_id(0).isEmpty()
                                        ? m_xLbTableDbColumn.get()
                                        : m_xLbTableCol.get() )
                                    : m_xLbTextDbColumn.get();

    SwInsDBColumn aSrch(pGetBox->get_selected_text());
    SwInsDBColumns::const_iterator it = m_aDBColumns.find( &aSrch );

    if( !aSrch.sColumn.isEmpty() )
    {
        m_xLbDbFormatFromUsr->CallSelectHdl();
        (*it)->nUsrNumFormat = m_xLbDbFormatFromUsr->GetFormat();
    }
}

IMPL_LINK_NOARG(SwInsertDBColAutoPilot, HeaderHdl, weld::Toggleable&, void)
{
    bool bEnable = m_xCbTableHeadon->get_active();
    m_xRbHeadlColnms->set_sensitive( bEnable );
    m_xRbHeadlEmpty->set_sensitive( bEnable );
}

static void lcl_InsTextInArr( std::u16string_view aText, DB_Columns& rColArr )
{
    size_t nSttPos = 0;
    size_t nFndPos;
    while( std::u16string_view::npos != ( nFndPos = aText.find( '\x0A', nSttPos )) )
    {
        if( 1 < nFndPos )
        {
            rColArr.push_back(std::make_unique<DB_Column>(OUString(aText.substr(nSttPos, nFndPos -1))));
        }
        rColArr.push_back(std::make_unique<DB_Column>());
        nSttPos = nFndPos + 1;
    }
    if( nSttPos < aText.size() )
    {
        rColArr.push_back(std::make_unique<DB_Column>(OUString(aText.substr(nSttPos))));
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
        nEndPos = sText.indexOf( cDBFieldEnd, nSttPos+1 );
        if( -1 != nEndPos )
        {
            // Text in <> brackets found: what is it:
            SwInsDBColumn aSrch( sText.copy( nSttPos, nEndPos - nSttPos ));
            SwInsDBColumns::const_iterator it = m_aDBColumns.find( &aSrch );
            if( it != m_aDBColumns.end() )
            {
                // that is a valid field
                // so surely the text "before":
                const SwInsDBColumn& rFndCol = **it;

                DB_Column* pNew;

                if( 1 < nSttPos )
                {
                    ::lcl_InsTextInArr( sText.subView( 0, nSttPos-1 ), rColArr );
                    sText = sText.copy( nSttPos-1 );
                }

                sText = sText.copy( rFndCol.sColumn.getLength() + 2 );
                nSttPos = 0;

                sal_uInt16 nSubType = 0;
                sal_uInt32 nFormat;
                if( rFndCol.bHasFormat )
                {
                    if( rFndCol.bIsDBFormat )
                        nFormat =  static_cast<sal_uInt32>(rFndCol.nDBNumFormat);
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
                    SwWrtShell& rSh = m_pView->GetWrtShell();
                    SwDBFieldType aFieldType( rSh.GetDoc(), aSrch.sColumn,
                                            m_aDBData );
                    pNew = new DB_Column( rFndCol, *new SwDBField(
                            static_cast<SwDBFieldType*>(rSh.InsertFieldType( aFieldType )),
                                                            nFormat ) );
                    if( nSubType )
                        pNew->pField->SetSubType( nSubType );
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
    Reference< XDataSource> const & xSource,
    Reference< XConnection> const & xConnection,
    Reference< sdbc::XResultSet > const & xResultSet_in )
{
    auto xResultSet = xResultSet_in;

    const Any* pSelection = rSelection.hasElements() ? rSelection.getConstArray() : nullptr;
    SwWrtShell& rSh = m_pView->GetWrtShell();

    //with the drag and drop interface no result set is initially available
    bool bDisposeResultSet = false;
    // we don't have a cursor, so we have to create our own RowSet
    if ( !xResultSet.is() )
    {
        xResultSet = SwDBManager::createCursor(m_aDBData.sDataSource,m_aDBData.sCommand,m_aDBData.nCommandType,xConnection,m_pView);
        bDisposeResultSet = xResultSet.is();
    }

    Reference< sdbc::XRow > xRow(xResultSet, UNO_QUERY);
    if ( !xRow.is() )
        return;

    rSh.StartAllAction();
    bool bUndo = rSh.DoesUndo();
    if( bUndo )
        rSh.StartUndo();

    bool bAsTable = m_xRbAsTable->get_active();
    SvNumberFormatter& rNumFormatr = *rSh.GetNumberFormatter();

    if( rSh.HasSelection() )
        rSh.DelRight();

    std::optional<SwWait> oWait;

    Reference< XColumnsSupplier > xColsSupp( xResultSet, UNO_QUERY );
    Reference <XNameAccess> xCols = xColsSupp->getColumns();

    uno::Reference<sdbcx::XRowLocate> xRowLocate(xResultSet, uno::UNO_QUERY_THROW);

    do{                                 // middle checked loop!!
    if( bAsTable )                      // fill in data as table
    {
        rSh.DoUndo( false );

        sal_Int32 nCols = m_xLbTableCol->n_children();
        sal_Int32 nRows = 0;
        if( m_xCbTableHeadon->get_active() )
            nRows++;

        if( pSelection )
            nRows += rSelection.getLength();
        else
            ++nRows;

        // prepare the array for the selected columns
        std::vector<SwInsDBColumn*> aColFields;
        for( sal_Int32 n = 0; n < nCols; ++n )
        {
            SwInsDBColumn aSrch(m_xLbTableCol->get_text(n));
            SwInsDBColumns::const_iterator it = m_aDBColumns.find( &aSrch );
            if (it != m_aDBColumns.end())
                aColFields.push_back(it->get());
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

        bool bHTML = 0 != (::GetHtmlMode( m_pView->GetDocShell() ) & HTMLMODE_ON);
        rSh.InsertTable(
            pModOpt->GetInsTableFlags(bHTML),
            nRows, nCols, (pSelection ? m_xTAutoFormat.get(): nullptr) );
        rSh.MoveTable( GotoPrevTable, fnTableStart );

        if( pSelection && m_pTableSet )
            SetTabSet();

        SfxItemSetFixed<RES_BOXATR_FORMAT, RES_BOXATR_VALUE> aTableSet( rSh.GetAttrPool() );
        bool bIsAutoUpdateCells = rSh.IsAutoUpdateCells();
        rSh.SetAutoUpdateCells( false );

        if( m_xCbTableHeadon->get_active() )
        {
            for( sal_Int32 n = 0; n < nCols; ++n )
            {
                if( m_xRbHeadlColnms->get_active() )
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
                    bBreak = !xRowLocate->moveToBookmark(pSelection[i]);
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
                    Any aType = xColumnProps->getPropertyValue(u"Type"_ustr);
                    aType >>= eDataType;
                }
                try
                {
                    if( pEntry->bHasFormat )
                    {
                        SwTableBoxNumFormat aNumFormat(
                                        pEntry->bIsDBFormat ? static_cast<sal_uInt32>(pEntry->nDBNumFormat)
                                                         : pEntry->nUsrNumFormat );
                        aTableSet.Put(aNumFormat);
                        if( xColumn.is() )
                        {
                            double fVal = xColumn->getDouble();
                            if( xColumn->wasNull() )
                                aTableSet.ClearItem( RES_BOXATR_VALUE );
                            else
                            {
                                if(rNumFormatr.GetType(aNumFormat.GetValue()) & SvNumFormatType::DATE)
                                {
                                    ::Date aStandard(1,1,1900);
                                    if (rNumFormatr.GetNullDate() != aStandard)
                                        fVal += (aStandard - rNumFormatr.GetNullDate());
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
                catch (const Exception&)
                {
                    TOOLS_WARN_EXCEPTION("sw", "");
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
                oWait.emplace( *m_pView->GetDocShell(), true );
        }

        rSh.MoveTable( GotoCurrTable, fnTableStart );
        if( !pSelection && ( m_pTableSet || m_xTAutoFormat ))
        {
            if( m_pTableSet )
                SetTabSet();

            if (m_xTAutoFormat)
                rSh.SetTableStyle(*m_xTAutoFormat);
        }
        rSh.SetAutoUpdateCells( bIsAutoUpdateCells );
    }
    else                            // add data as fields/text
    {
        DB_Columns aColArr;
        if( SplitTextToColArr( m_xEdDbText->get_text(), aColArr, m_xRbAsField->get_active() ) )
        {
            // now for each data set, we can iterate over the array
            // and add the data

            if( !rSh.IsSttPara() )
                rSh.SwEditShell::SplitNode();
            if( !rSh.IsEndPara() )
            {
                rSh.SwEditShell::SplitNode();
                rSh.SwCursorShell::Left(1,SwCursorSkipMode::Chars);
            }

            rSh.DoUndo( false );

            SwTextFormatColl* pColl = nullptr;
            {
                const OUString sTmplNm(m_xLbDbParaColl->get_active_text());
                if( m_sNoTmpl != sTmplNm )
                {
                    pColl = rSh.FindTextFormatCollByName( sTmplNm );
                    if( !pColl )
                    {
                        const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(
                            sTmplNm, SwGetPoolIdFromName::TxtColl );
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
              Any aFormats = xSourceProps->getPropertyValue(u"NumberFormatsSupplier"_ustr);
              if(aFormats.hasValue())
              {
                  Reference< util::XNumberFormatsSupplier> xSuppl;
                  aFormats >>= xSuppl;
                  if(xSuppl.is())
                  {
                        Reference< XPropertySet > xSettings = xSuppl->getNumberFormatSettings();
                        Any aNull = xSettings->getPropertyValue(u"NullDate"_ustr);
                        aNull >>= aDBFormatData.aNullDate;
                        if(aDBFormatData.xFormatter.is())
                            aDBFormatData.xFormatter->attachNumberFormatsSupplier(xSuppl);
                  }
              }
            }
            aDBFormatData.aLocale = LanguageTag( rSh.GetCurLang() ).getLocale();
            SwDBNextSetField aNxtDBField( static_cast<SwDBNextSetFieldType*>(rSh.
                                            GetFieldType( 0, SwFieldIds::DbNextSet )),
                                        u"1"_ustr, m_aDBData );

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
                        bBreak = !xRowLocate->moveToBookmark(pSelection[i]);
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
                    case DB_Column::Type::FILLTEXT:
                        sIns =  *pDBCol->pText;
                        break;

                    case DB_Column::Type::SPLITPARA:
                        rSh.SplitNode();
                        // when the template is not the same as the follow template,
                        // the selected has to be set newly
                        if( pColl && &pColl->GetNextTextFormatColl() != pColl )
                            rSh.SetTextFormatColl( pColl );
                        break;

                    case DB_Column::Type::COL_FIELD:
                        {
                            std::unique_ptr<SwDBField> pField(static_cast<SwDBField *>(
                                pDBCol->pField->CopyField().release()));
                            double nValue = DBL_MAX;

                            Reference< XPropertySet > xColumnProps;
                            xCols->getByName(pDBCol->pColInfo->sColumn) >>= xColumnProps;

                            pField->SetExpansion( SwDBManager::GetDBField(
                                                xColumnProps,
                                                aDBFormatData,
                                                &nValue ) );
                            if( DBL_MAX != nValue )
                            {
                                Any aType = xColumnProps->getPropertyValue(u"Type"_ustr);
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

                            rSh.InsertField2( *pField );
                        }
                        break;

                    case DB_Column::Type::COL_TEXT:
                        {
                            double nValue = DBL_MAX;
                            Reference< XPropertySet > xColumnProps;
                            xCols->getByName(pDBCol->pColInfo->sColumn) >>= xColumnProps;
                            sIns = SwDBManager::GetDBField(
                                                xColumnProps,
                                                aDBFormatData,
                                                &nValue );
                            if( pDBCol->nFormat &&
                                DBL_MAX != nValue )
                            {
                                const Color* pCol;
                                if(rNumFormatr.GetType(pDBCol->nFormat) & SvNumFormatType::DATE)
                                {
                                    ::Date aStandard(1,1,1900);
                                    if (rNumFormatr.GetNullDate() != aStandard)
                                        nValue += (aStandard - rNumFormatr.GetNullDate());
                                }
                                rNumFormatr.GetOutputString( nValue,
                                            pDBCol->nFormat,
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
                                    GoCurrPara, fnParaStart );
                            pMark = rSh.SetBookmark(
                                    vcl::KeyCode(),
                                    OUString(),
                                    IDocumentMarkAccess::MarkType::UNO_BOOKMARK );
                            rSh.SwCursorShell::MovePara(
                                    GoCurrPara, fnParaEnd );
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

                if( m_xRbAsField->get_active() )
                    rSh.InsertField2( aNxtDBField );

                if( !rSh.IsSttPara() )
                    rSh.SwEditShell::SplitNode();

                if( 10 == i )
                    oWait.emplace( *m_pView->GetDocShell(), true );
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
    SwWrtShell& rSh = m_pView->GetWrtShell();
    const SfxPoolItem* pItem;

    if (m_xTAutoFormat)
    {
        if (m_xTAutoFormat->IsFrame())
        {
            // border is from AutoFormat
            m_pTableSet->ClearItem( RES_BOX );
            m_pTableSet->ClearItem( SID_ATTR_BORDER_INNER );
        }
        if (m_xTAutoFormat->IsBackground())
        {
            m_pTableSet->ClearItem( RES_BACKGROUND );
            m_pTableSet->ClearItem( SID_ATTR_BRUSH_ROW );
            m_pTableSet->ClearItem( SID_ATTR_BRUSH_TABLE );
        }
    }
    else
    {
        // remove the defaults again, it makes no sense to set them
        SvxBrushItem aBrush( RES_BACKGROUND );
        static const sal_uInt16 aIds[3] =
            { RES_BACKGROUND, SID_ATTR_BRUSH_ROW, SID_ATTR_BRUSH_TABLE };
        for(sal_uInt16 i : aIds)
            if( SfxItemState::SET == m_pTableSet->GetItemState( i,
                false, &pItem ) && *pItem == aBrush )
                m_pTableSet->ClearItem( i );
    }

    const SfxStringItem* pTableNameItem = m_pTableSet->GetItemIfSet( FN_PARAM_TABLE_NAME, false);
    if( pTableNameItem && pTableNameItem->GetValue() == rSh.GetTableFormat()->GetName() )
        m_pTableSet->ClearItem( FN_PARAM_TABLE_NAME );

    rSh.MoveTable( GotoCurrTable, fnTableStart );
    rSh.SetMark();
    rSh.MoveTable( GotoCurrTable, fnTableEnd );

    ItemSetToTableParam( *m_pTableSet, rSh );

    rSh.ClearMark();
    rSh.MoveTable( GotoCurrTable, fnTableStart );
}

static Sequence<OUString> lcl_createSourceNames(std::u16string_view rNodeName)
{
    Sequence<OUString> aSourceNames(11);
    OUString* pNames = aSourceNames.getArray();
    pNames[0] = OUString::Concat(rNodeName) + "/DataSource";
    pNames[1] = OUString::Concat(rNodeName) + "/Command";
    pNames[2] = OUString::Concat(rNodeName) + "/CommandType";
    pNames[3] = OUString::Concat(rNodeName) + "/ColumnsToText";
    pNames[4] = OUString::Concat(rNodeName) + "/ColumnsToTable";
    pNames[5] = OUString::Concat(rNodeName) + "/ParaStyle";
    pNames[6] = OUString::Concat(rNodeName) + "/TableAutoFormat";
    pNames[7] = OUString::Concat(rNodeName) + "/IsTable";
    pNames[8] = OUString::Concat(rNodeName) + "/IsField";
    pNames[9] = OUString::Concat(rNodeName) + "/IsHeadlineOn";
    pNames[10] = OUString::Concat(rNodeName) + "/IsEmptyHeadline";
    return aSourceNames;
}

static Sequence<OUString> lcl_CreateSubNames(std::u16string_view rSubNodeName)
{
    return
    {
        OUString::Concat(rSubNodeName) + "/ColumnName",
        OUString::Concat(rSubNodeName) + "/ColumnIndex",
        OUString::Concat(rSubNodeName) + "/IsNumberFormat",
        OUString::Concat(rSubNodeName) + "/IsNumberFormatFromDataBase",
        OUString::Concat(rSubNodeName) + "/NumberFormat",
        OUString::Concat(rSubNodeName) + "/NumberFormatLocale"
    };
}

static OUString lcl_CreateUniqueName(const Sequence<OUString>& aNames)
{
    sal_Int32 nIdx = aNames.getLength();
    while(true)
    {
        const OUString sRet = "_" + OUString::number(nIdx++);
        if ( comphelper::findValue(aNames, sRet) == -1 )
            return sRet;    // No match found, return unique name
    }
}

void SwInsertDBColAutoPilot::Notify( const css::uno::Sequence< OUString >&  ) {}

void SwInsertDBColAutoPilot::ImplCommit()
{
    Sequence <OUString> aNames = GetNodeNames(OUString());
    //remove entries that contain this data source + table at first
    for (OUString const& nodeName : aNames)
    {
        Sequence<Any> aSourceProperties = GetProperties({ nodeName + "/DataSource", nodeName + "/Command" });
        const Any* pSourceProps = aSourceProperties.getArray();
        OUString sSource, sCommand;
        pSourceProps[0] >>= sSource;
        pSourceProps[1] >>= sCommand;
        if(sSource==m_aDBData.sDataSource && sCommand==m_aDBData.sCommand)
        {
            ClearNodeElements(OUString(), { nodeName });
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

    pValues[0].Value <<= m_aDBData.sDataSource;
    pValues[1].Value <<= m_aDBData.sCommand;
    pValues[2].Value <<= m_aDBData.nCommandType;
    pValues[3].Value <<= m_xEdDbText->get_text();

    OUString sTmp;
    const sal_Int32 nCnt = m_xLbTableCol->n_children();
    for( sal_Int32 n = 0; n < nCnt; ++n )
        sTmp += m_xLbTableCol->get_text(n) + "\x0a";

    if (!sTmp.isEmpty())
        pValues[4].Value <<= sTmp;

    if( m_sNoTmpl != (sTmp = m_xLbDbParaColl->get_active_text()) )
        pValues[5].Value <<= sTmp;

    if (m_xTAutoFormat)
        pValues[6].Value <<= m_xTAutoFormat->GetName();

    pValues[7].Value <<= m_xRbAsTable->get_active();
    pValues[8].Value <<= m_xRbAsField->get_active();
    pValues[9].Value <<= m_xCbTableHeadon->get_active();
    pValues[10].Value <<= m_xRbHeadlEmpty->get_active();

    SetSetProperties(OUString(), aValues);

    sNewNode += "/ColumnSet";

    LanguageType ePrevLang(0xffff);

    SvNumberFormatter& rNFormatr = *m_pView->GetWrtShell().GetNumberFormatter();
    for(size_t nCol = 0; nCol < m_aDBColumns.size(); nCol++)
    {
        SwInsDBColumn* pColumn = m_aDBColumns[nCol].get();
        OUString sColumnInsertNode(sNewNode + "/__");
        if( nCol < 10 )
            sColumnInsertNode += "00";
        else if( nCol < 100 )
            sColumnInsertNode += "0";
        sColumnInsertNode += OUString::number(  nCol );

        const Sequence <OUString> aSubNodeNames = lcl_CreateSubNames(sColumnInsertNode);
        Sequence<PropertyValue> aSubValues(aSubNodeNames.getLength());
        PropertyValue* pSubValues = aSubValues.getArray();
        sal_Int32 i = 0;

        for( const OUString& rSubNodeName : aSubNodeNames)
            pSubValues[i++].Name = rSubNodeName;
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
    const Sequence<OUString> aNames = GetNodeNames(OUString());
    SvNumberFormatter& rNFormatr = *m_pView->GetWrtShell().GetNumberFormatter();
    for(OUString const & nodeName : aNames)
    {
        //search for entries with the appropriate data source and table
        Sequence<OUString> aSourceNames = lcl_createSourceNames(nodeName);

        Sequence< Any> aDataSourceProps = GetProperties(aSourceNames);
        const Any* pDataSourceProps = aDataSourceProps.getConstArray();
        OUString sSource, sCommand;
        sal_Int16 nCommandType;
        pDataSourceProps[0] >>= sSource;
        pDataSourceProps[1] >>= sCommand;
        pDataSourceProps[2] >>= nCommandType;
        if(sSource == m_aDBData.sDataSource && sCommand == m_aDBData.sCommand)
        {
            DB_ColumnConfigData aNewData;

            pDataSourceProps[3] >>= aNewData.sEdit;
            pDataSourceProps[4] >>= aNewData.sTableList;
            pDataSourceProps[5] >>= aNewData.sTmplNm;
            pDataSourceProps[6] >>= aNewData.sTAutoFormatNm;
            if(pDataSourceProps[7].hasValue())
                aNewData.bIsTable = *o3tl::doAccess<bool>(pDataSourceProps[7]);
            if(pDataSourceProps[8].hasValue())
                 aNewData.bIsField = *o3tl::doAccess<bool>(pDataSourceProps[8]);
            if(pDataSourceProps[9].hasValue())
                 aNewData.bIsHeadlineOn = *o3tl::doAccess<bool>(pDataSourceProps[9]);
            if(pDataSourceProps[10].hasValue())
                 aNewData.bIsEmptyHeadln = *o3tl::doAccess<bool>(pDataSourceProps[10]);

            const OUString sSubNodeName(nodeName + "/ColumnSet/");
            const Sequence <OUString> aSubNames = GetNodeNames(sSubNodeName);
            for(const OUString& rSubName : aSubNames)
            {
                Sequence <OUString> aSubNodeNames =
                    lcl_CreateSubNames(Concat2View(sSubNodeName + rSubName));
                Sequence< Any> aSubProps = GetProperties(aSubNodeNames);
                const Any* pSubProps = aSubProps.getConstArray();

                OUString sColumn;
                pSubProps[0] >>= sColumn;
                // check for existence of the loaded column name
                bool bFound = false;
                for(size_t nRealColumn = 0; nRealColumn < m_aDBColumns.size(); ++nRealColumn)
                {
                    if(m_aDBColumns[nRealColumn]->sColumn == sColumn)
                    {
                        bFound = true;
                        break;
                    }
                }
                if(!bFound)
                    continue;
                sal_Int16 nIndex = 0;
                pSubProps[1] >>= nIndex;
                std::unique_ptr<SwInsDBColumn> pInsDBColumn(new SwInsDBColumn(sColumn));
                if(pSubProps[2].hasValue())
                    pInsDBColumn->bHasFormat = *o3tl::doAccess<bool>(pSubProps[2]);
                if(pSubProps[3].hasValue())
                    pInsDBColumn->bIsDBFormat = *o3tl::doAccess<bool>(pSubProps[3]);

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

                aNewData.aDBColumns.insert(std::move(pInsDBColumn));
            }
            OUString sTmp( aNewData.sTableList );
            if( !sTmp.isEmpty() )
            {
                sal_Int32 n = 0;
                do {
                    const OUString sEntry( sTmp.getToken( 0, '\x0a', n ) );
                    //preselect column - if they still exist!
                    if (m_xLbTableDbColumn->find_text(sEntry) != -1)
                    {
                        m_xLbTableCol->append_text(sEntry);
                        m_xLbTableDbColumn->remove_text(sEntry);
                    }
                } while( n>=0 );

                if (!m_xLbTableDbColumn->n_children())
                {
                    m_xIbDbcolAllTo->set_sensitive( false );
                    m_xIbDbcolOneTo->set_sensitive( false );
                }
                m_xIbDbcolOneFrom->set_sensitive(true);
                m_xIbDbcolAllFrom->set_sensitive(true);
            }
            m_xEdDbText->set_text( aNewData.sEdit );

            sTmp = aNewData.sTmplNm;
            if( !sTmp.isEmpty() )
                m_xLbDbParaColl->set_active_text(sTmp);
            else
                m_xLbDbParaColl->set_active(0);

            m_xTAutoFormat.reset();
            sTmp = aNewData.sTAutoFormatNm;
            if( !sTmp.isEmpty() )
            {
                // then load the AutoFormat file and look for Autoformat first
                SwTableAutoFormatTable aAutoFormatTable;
                aAutoFormatTable.Load();
                for( size_t nAutoFormat = aAutoFormatTable.size(); nAutoFormat; )
                    if( sTmp == aAutoFormatTable[ --nAutoFormat ].GetName() )
                    {
                        m_xTAutoFormat.reset(new SwTableAutoFormat(aAutoFormatTable[nAutoFormat]));
                        break;
                    }
            }

            m_xRbAsTable->set_active( aNewData.bIsTable );
            m_xRbAsField->set_active( aNewData.bIsField );
            m_xRbAsText->set_active( !aNewData.bIsTable && !aNewData.bIsField );

            m_xCbTableHeadon->set_active( aNewData.bIsHeadlineOn );
            m_xRbHeadlColnms->set_active( !aNewData.bIsEmptyHeadln );
            m_xRbHeadlEmpty->set_active( aNewData.bIsEmptyHeadln );
            HeaderHdl(*m_xCbTableHeadon);

            // now copy the user defined Numberformat strings to the
            // Shell. Then only these are available as ID
            for( size_t n = 0; n < m_aDBColumns.size() ; ++n )
            {
                SwInsDBColumn& rSet = *m_aDBColumns[ n ];
                for( size_t m = 0; m < aNewData.aDBColumns.size() ; ++m )
                {
                    SwInsDBColumn& rGet = *aNewData.aDBColumns[ m ];
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
                                SvNumFormatType nType;
                                rNFormatr.PutEntry( rGet.sUsrNumFormat, nCheckPos, nType,
                                                rSet.nUsrNumFormat, rGet.eUsrNumFormatLng );
                            }
                        }
                        break;
                    }
                }
            }

            // when the cursor is inside of a table, table must NEVER be selectable
            if( !m_xRbAsTable->get_sensitive() && m_xRbAsTable->get_active() )
                m_xRbAsField->set_active(true);
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
