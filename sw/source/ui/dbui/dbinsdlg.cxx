/*************************************************************************
 *
 *  $RCSfile: dbinsdlg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: os $ $Date: 2000-10-27 14:29:45 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
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
#ifndef _COM_SUN_STAR_SDB_XDATABASEACCESS_HPP_
#include <com/sun/star/sdb/XDatabaseAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _UTL_UNO3_DB_TOOLS_HXX_
#include <unotools/dbtools.hxx>
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

#include <sdb/tools.hxx>
#ifndef _SDB_SDBCURS_HXX //autogen
#include <sdb/sdbcurs.hxx>
#endif
#include <float.h>


#ifndef _TOOLS_INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _ZFORMAT_HXX //autogen
#include <svtools/zformat.hxx>
#endif
#ifndef _SFXCFGITEM_HXX //autogen
#include <sfx2/cfgitem.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif

#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_RULRITEM_HXX //autogen
#include <svx/rulritem.hxx>
#endif

#ifndef _TABLEDLG_HXX //autogen
#include <tabledlg.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _TABCOL_HXX //autogen
#include <tabcol.hxx>
#endif
#ifndef _UIITEMS_HXX //autogen
#include <uiitems.hxx>
#endif
#ifndef _VIEWOPT_HXX //autogen
#include <viewopt.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _DBINSDLG_HXX
#include <dbinsdlg.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _TAUTOFMT_HXX
#include <tautofmt.hxx>
#endif
#ifndef _TBLAFMT_HXX
#include <tblafmt.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _DBFLD_HXX
#include <dbfld.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _SECTION_HXX //autogen
#include <section.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _MODOPT_HXX //autogen
#include <modcfg.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif

#ifndef _DBINSDLG_HRC
#include <dbinsdlg.hrc>
#endif
#ifndef _DBUI_HRC
#include <dbui.hrc>
#endif
#ifndef _CFGSTR_HRC
#include <cfgstr.hrc>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _CFGID_H
#include <cfgid.h>
#endif


using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdb;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;

const char cDBFldStart  = '<';
const char cDBFldEnd    = '>';
#define C2U(cChar) OUString::createFromAscii(cChar)
#define C2S(cChar) String::CreateFromAscii(cChar)

// Hilfsstruktur fuers einfuegen von Datenbankspalten als Felder oder Text
struct _DB_Column
{
    enum { DB_FILLTEXT, DB_COL_FIELD, DB_COL_TEXT, DB_SPLITPARA } eColType;

    union {
        String* pText;
        SwField* pField;
        ULONG nFormat;
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

    _DB_Column( const SwInsDBColumn& rInfo, ULONG nFormat )
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

typedef _DB_Column* _DB_ColumnPtr;
SV_DECL_PTRARR_DEL( _DB_Columns, _DB_ColumnPtr, 32, 32 )
SV_IMPL_PTRARR( _DB_Columns, _DB_ColumnPtr )

SV_IMPL_OP_PTRARR_SORT( SwInsDBColumns, SwInsDBColumnPtr )

/*  */

#define DBCOLUMN_CONFIG_VERSION1    1
#define DBCOLUMN_CONFIG_VERSION     DBCOLUMN_CONFIG_VERSION1
#define DBCOLUMN_MAXDATA            5

struct _DB_ColumnConfigData
{
    SwInsDBColumns aDBColumns;
    String sDBName, sEdit, sTblList, sTmplNm, sTAutoFmtNm;
    BOOL bIsTable : 1,
         bIsField : 1,
         bIsHeadlineOn : 1,
         bIsEmptyHeadln : 1;

    _DB_ColumnConfigData()
    {
        bIsTable = bIsHeadlineOn = TRUE;
        bIsField = bIsEmptyHeadln = FALSE;
    }

    _DB_ColumnConfigData( SvStream& rStream, UINT16 nVers );

    ~_DB_ColumnConfigData();

    void Save( SvStream& rStream ) const;

    BOOL IsEqualDB( const _DB_ColumnConfigData& ) const;
private:
    _DB_ColumnConfigData( const _DB_ColumnConfigData& );
    _DB_ColumnConfigData& operator =( const _DB_ColumnConfigData& );
};

class _DB_ColumnConfig : public SfxConfigItem
{
    _DB_ColumnConfigData* aData[ DBCOLUMN_MAXDATA ];

protected:
    virtual String GetName() const;
    virtual int Load(SvStream& rStream);
    virtual BOOL Store(SvStream& rStream);

public:
    _DB_ColumnConfig();
    virtual ~_DB_ColumnConfig();

    virtual void UseDefault();

    const _DB_ColumnConfigData* HasData( const _DB_ColumnConfigData& rData ) const;
    void SetData( _DB_ColumnConfigData* pData );
};

/*  */

int SwInsDBColumn::operator<( const SwInsDBColumn& rCmp ) const
{
    return COMPARE_LESS == Application::GetAppInternational().Compare(
                                                sColumn, rCmp.sColumn );
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
SwInsertDBColAutoPilot::SwInsertDBColAutoPilot( SwView& rView,
        Reference<XDataSource> xDataSource,
        Reference<sdbcx::XColumnsSupplier> xColSupp,
        const SwInsDBData& rData )
    : SfxModalDialog( rView.GetWindow(), SW_RES( DLG_AP_INSERT_DB_SEL )),
    aFtInsertData( this, SW_RES( FT_INSERT_DATA )),
    aRbAsTable( this, SW_RES( RB_AS_TABLE )),
    aRbAsField( this, SW_RES( RB_AS_FIELD )),
    aRbAsText( this, SW_RES( RB_AS_TEXT )),
    aBtOk( this, SW_RES( BT_OK )),
    aBtCancel( this, SW_RES( BT_CANCEL )),
    aBtHelp( this, SW_RES( BT_HELP )),
    aGbSelection( this, SW_RES( GB_SELECTION )),
    aFtDbColumn( this, SW_RES( FT_DB_COLUMN )),
    aGbDbFormat( this, SW_RES( GB_DB_FORMAT )),
    aRbDbFmtFromDb( this, SW_RES( RB_DBFMT_FROM_DB )),
    aRbDbFmtFromUsr( this, SW_RES( RB_DBFMT_FROM_USR )),
    aLbDbFmtFromUsr( this, &rView, SW_RES( LB_DBFMT_FROM_USR )),

    /* ----- Page Text/Field ------- */
    aLbTxtDbColumn( this, SW_RES( LB_TXT_DB_COLUMN )),
    aPbDbcolToEdit( this, SW_RES( PB_DBCOL_TOEDIT )),
    aEdDbText( this, SW_RES( ED_DB_TEXT )),
    aFtDbParaColl( this, SW_RES( FT_DB_PARA_COLL )),
    aLbDbParaColl( this, SW_RES( LB_DB_PARA_COLL )),

    /* ----- Page Table ------------ */
    aLbTblDbColumn( this, SW_RES( LB_TBL_DB_COLUMN )),
    aPbDbcolAllTo( this, SW_RES( PB_DBCOL_ALL_TO )),
    aPbDbcolOneTo( this, SW_RES( PB_DBCOL_ONE_TO )),
    aPbDbcolOneFrom( this, SW_RES( PB_DBCOL_ONE_FROM )),
    aPbDbcolAllFrom( this, SW_RES( PB_DBCOL_ALL_FROM )),
    aFtTableCol( this, SW_RES( FT_TABLE_COL )),
    aLbTableCol( this, SW_RES( LB_TABLE_COL )),
    aGbTableHead( this, SW_RES( GB_TABLE_HEAD )),
    aCbTableHeadon( this, SW_RES( CB_TABLE_HEADON )),
    aRbHeadlColnms( this, SW_RES( RB_HEADL_COLNMS )),
    aRbHeadlEmpty( this, SW_RES( RB_HEADL_EMPTY )),
    aPbTblFormat( this, SW_RES( PB_TBL_FORMAT )),
    aPbTblAutofmt( this, SW_RES( PB_TBL_AUTOFMT )),

    sNoTmpl( SW_RES( STR_NOTEMPL )),
    aDBData(rData),
    pView( &rView ),
    pTAutoFmt( 0 ),
    pTblSet( 0 ),
    pRep( 0 ),
    aOldNumFmtLnk( aLbDbFmtFromUsr.GetSelectHdl() ),
    pConfig( 0 )
{
    FreeResource();

    nGBFmtLen = aGbDbFormat.GetText().Len();

    if(xColSupp.is())
    {
        SwWrtShell& rSh = pView->GetWrtShell();
        SvNumberFormatter* pNumFmtr = rSh.GetNumberFormatter();
        SfxItemSet aSet(pView->GetPool(), RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE);
        rSh.GetAttr( aSet );
        Locale aDocLocale;
        if(SFX_ITEM_DEFAULT <= aSet.GetItemState(RES_CHRATR_LANGUAGE, TRUE))
        {
            Any aLoc;
            const SfxPoolItem& rItem = aSet.Get(RES_CHRATR_LANGUAGE);
            rItem.QueryValue(aLoc,  MID_LANG_LOCALE);
            aDocLocale = *(Locale*)aLoc.getValue();
        }

        SvNumberFormatsSupplierObj* pNumFmt = new SvNumberFormatsSupplierObj( pNumFmtr );
        Reference< util::XNumberFormatsSupplier >  xDocNumFmtsSupplier = pNumFmt;
        Reference< XNumberFormats > xDocNumberFormats = xDocNumFmtsSupplier->getNumberFormats();
         Reference< XNumberFormatTypes > xDocNumberFormatTypes(xDocNumberFormats, UNO_QUERY);

        Reference<XPropertySet> xSourceProps(xDataSource, UNO_QUERY);
        Reference< XNumberFormats > xNumberFormats;
        if(xSourceProps.is())
        {
            Any aFormats = xSourceProps->getPropertyValue(C2U("NumberFormatsSupplier"));
            if(aFormats.hasValue())
            {
                Reference<XNumberFormatsSupplier> xSuppl = *(Reference<util::XNumberFormatsSupplier>*) aFormats.getValue();
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
            SwInsDBColumn* pNew = new SwInsDBColumn(
                                            pColNames[n], n );
            Any aCol = xCols->getByName(pColNames[n]);
            Reference <XPropertySet> xCol = *(Reference <XPropertySet>*)aCol.getValue();
            Any aType = xCol->getPropertyValue(C2S("Type"));
            sal_Int32 eDataType;
            aType >>= eDataType;
            switch(eDataType)
            {
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
                    pNew->bHasFmt = TRUE;
                    Any aFormat = xCol->getPropertyValue(C2U("FormatKey"));
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
                                OUString sFormat;
                                aFormat >>= sFormat;
                                com::sun::star::lang::Locale aLoc;
                                aLocale >>= aLoc;
                                nFmt = xDocNumberFormats->addNew( sFormat, aLoc ),
                                pNew->nDBNumFmt = nFmt;
                            }
                            catch(...)
                            {
                                DBG_ERROR("illegal number format key")
                            }
                        }
                    }
                    else
                        pNew->nDBNumFmt = utl::getDefaultNumberFormat(xCol,
                                     xDocNumberFormatTypes,
                                     aDocLocale);

                }
                break;
            }
            if( !aDBColumns.Insert( pNew ))
            {
                ASSERT( !this, "Spaltenname mehrfach vergeben?" );
                delete pNew;
            }
        }
    }

    // Absatzvorlagen-ListBox fuellen
    {
        SfxStyleSheetBasePool* pPool = pView->GetDocShell()->GetStyleSheetPool();
        pPool->SetSearchMask( SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_ALL );
        aLbDbParaColl.InsertEntry( sNoTmpl );

        const SfxStyleSheetBase* pBase = pPool->First();
        while( pBase )
        {
            aLbDbParaColl.InsertEntry( pBase->GetName() );
            pBase = pPool->Next();
        }
        aLbDbParaColl.SelectEntryPos( 0 );
    }

    // steht der Cursor in einer Tabelle, darf NIE Tabelle auswaehlbar sein
    if( pView->GetWrtShell().GetTableFmt() )
    {
        aRbAsTable.Enable( FALSE );
        aRbAsField.Check( TRUE );
        aRbDbFmtFromDb.Check( TRUE );
    }
    else
    {
        aRbAsTable.Check( TRUE );
        aRbDbFmtFromDb.Check( TRUE );
        aPbDbcolOneFrom.Enable( FALSE );
        aPbDbcolAllFrom.Enable( FALSE );
    }

    aRbAsTable.SetClickHdl( LINK(this, SwInsertDBColAutoPilot, PageHdl ));
    aRbAsField.SetClickHdl( LINK(this, SwInsertDBColAutoPilot, PageHdl ));
    aRbAsText.SetClickHdl( LINK(this, SwInsertDBColAutoPilot, PageHdl ));

    aRbDbFmtFromDb.SetClickHdl( LINK(this, SwInsertDBColAutoPilot, DBFormatHdl ));
    aRbDbFmtFromUsr.SetClickHdl( LINK(this, SwInsertDBColAutoPilot, DBFormatHdl ));

    aPbTblFormat.SetClickHdl(LINK(this, SwInsertDBColAutoPilot, TblFmtHdl ));
    aPbTblAutofmt.SetClickHdl(LINK(this, SwInsertDBColAutoPilot, AutoFmtHdl ));

    aPbDbcolAllTo.SetClickHdl( LINK(this, SwInsertDBColAutoPilot, TblToFromHdl ));
    aPbDbcolOneTo.SetClickHdl( LINK(this, SwInsertDBColAutoPilot, TblToFromHdl ));
    aPbDbcolOneFrom.SetClickHdl( LINK(this, SwInsertDBColAutoPilot, TblToFromHdl ));
    aPbDbcolAllFrom.SetClickHdl( LINK(this, SwInsertDBColAutoPilot, TblToFromHdl ));
    aPbDbcolToEdit.SetClickHdl( LINK(this, SwInsertDBColAutoPilot, TblToFromHdl ));

    aCbTableHeadon.SetClickHdl( LINK(this, SwInsertDBColAutoPilot, HeaderHdl ));
    aRbHeadlColnms.SetClickHdl( LINK(this, SwInsertDBColAutoPilot, HeaderHdl ));
    aRbHeadlEmpty.SetClickHdl( LINK(this, SwInsertDBColAutoPilot, HeaderHdl ));

    aLbTxtDbColumn.SetSelectHdl( LINK( this, SwInsertDBColAutoPilot, SelectHdl ));
    aLbTblDbColumn.SetSelectHdl( LINK( this, SwInsertDBColAutoPilot, SelectHdl ));
    aLbDbFmtFromUsr.SetSelectHdl( LINK( this, SwInsertDBColAutoPilot, SelectHdl ));
    aLbTableCol.SetSelectHdl( LINK( this, SwInsertDBColAutoPilot, SelectHdl ));

    aLbTxtDbColumn.SetDoubleClickHdl( LINK( this, SwInsertDBColAutoPilot, DblClickHdl ));
    aLbTblDbColumn.SetDoubleClickHdl( LINK( this, SwInsertDBColAutoPilot, DblClickHdl ));
    aLbTableCol.SetDoubleClickHdl( LINK( this, SwInsertDBColAutoPilot, DblClickHdl ));

    for( USHORT n = 0; n < aDBColumns.Count(); ++n )
    {
        const String& rS = aDBColumns[ n ]->sColumn;
        aLbTblDbColumn.InsertEntry( rS, n );
        aLbTxtDbColumn.InsertEntry( rS, n );
    }
    aLbTxtDbColumn.SelectEntryPos( 0 );
    aLbTblDbColumn.SelectEntryPos( 0 );

    // Daten aus dem INI-File lesen:
    ReadIniToUI();

    // Controls initialisieren:
    PageHdl( aRbAsTable.IsChecked() ? &aRbAsTable : &aRbAsField );
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
SwInsertDBColAutoPilot::~SwInsertDBColAutoPilot()
{
    delete pTblSet;
    delete pRep;

    delete pConfig;
    delete pTAutoFmt;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK( SwInsertDBColAutoPilot, PageHdl, Button*, pButton )
{
    BOOL bShowTbl = pButton == &aRbAsTable;

    String sTxt( pButton->GetText() );
    aGbSelection.SetText( sTxt.EraseAllChars( '~' ) );

    aLbTxtDbColumn.Show( !bShowTbl );
    aPbDbcolToEdit.Show( !bShowTbl );
    aEdDbText.Show( !bShowTbl );
    aFtDbParaColl.Show( !bShowTbl );
    aLbDbParaColl.Show( !bShowTbl );

    aLbTblDbColumn.Show( bShowTbl );
    aPbDbcolAllTo.Show( bShowTbl );
    aPbDbcolOneTo.Show( bShowTbl );
    aPbDbcolOneFrom.Show( bShowTbl );
    aPbDbcolAllFrom.Show( bShowTbl );
    aFtTableCol.Show( bShowTbl );
    aLbTableCol.Show( bShowTbl );
    aGbTableHead.Show( bShowTbl );
    aCbTableHeadon.Show( bShowTbl );
    aRbHeadlColnms.Show( bShowTbl );
    aRbHeadlEmpty.Show( bShowTbl );
    aPbTblFormat.Show( bShowTbl );
    aPbTblAutofmt.Show( bShowTbl );

    if( bShowTbl )
        aPbTblFormat.Enable( 0 != aLbTableCol.GetEntryCount() );

    SelectHdl( bShowTbl ? &aLbTblDbColumn : &aLbTxtDbColumn );

    return 0;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK( SwInsertDBColAutoPilot, DBFormatHdl, Button*, pButton )
{
    USHORT nFndPos;
    ListBox& rBox = aRbAsTable.IsChecked()
                        ? ( 0 == aLbTableCol.GetEntryData( 0 )
                            ? aLbTblDbColumn
                            : aLbTableCol )
                        : aLbTxtDbColumn;

    SwInsDBColumn aSrch( rBox.GetSelectEntry(), 0 );
    aDBColumns.Seek_Entry( &aSrch, &nFndPos );

    BOOL bFromDB = &aRbDbFmtFromDb == pButton;
    aDBColumns[ nFndPos ]->bIsDBFmt = bFromDB;
    aLbDbFmtFromUsr.Enable( !bFromDB );

    return 0;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK( SwInsertDBColAutoPilot, TblToFromHdl, Button*, pButton )
{
    BOOL bChgEnable = TRUE, bEnableTo = TRUE, bEnableFrom = TRUE;
    aLbTblDbColumn.SetUpdateMode( FALSE );
    aLbTableCol.SetUpdateMode( FALSE );

    if( pButton == &aPbDbcolAllTo )
    {
        bEnableTo = FALSE;

        USHORT n, nInsPos = aLbTableCol.GetSelectEntryPos(),
               nCnt = aLbTblDbColumn.GetEntryCount();
        if( LISTBOX_APPEND == nInsPos )
            for( n = 0; n < nCnt; ++n )
                aLbTableCol.InsertEntry( aLbTblDbColumn.GetEntry( n ),
                                            LISTBOX_APPEND );
        else
            for( n = 0; n < nCnt; ++n, ++nInsPos )
                aLbTableCol.InsertEntry( aLbTblDbColumn.GetEntry( n ), nInsPos );
        aLbTblDbColumn.Clear();
        aLbTableCol.SelectEntryPos( nInsPos );
        aLbTblDbColumn.SelectEntryPos( LISTBOX_APPEND );
    }
    else if( pButton == &aPbDbcolOneTo &&
            LISTBOX_ENTRY_NOTFOUND != aLbTblDbColumn.GetSelectEntryPos() )
    {
        USHORT nInsPos = aLbTableCol.GetSelectEntryPos(),
               nDelPos = aLbTblDbColumn.GetSelectEntryPos(),
               nTopPos = aLbTblDbColumn.GetTopEntry();
        aLbTableCol.InsertEntry( aLbTblDbColumn.GetEntry( nDelPos ), nInsPos );
        aLbTblDbColumn.RemoveEntry( nDelPos );

        aLbTableCol.SelectEntryPos( nInsPos );
        if( nDelPos >= aLbTblDbColumn.GetEntryCount() )
            nDelPos = aLbTblDbColumn.GetEntryCount() - 1;
        aLbTblDbColumn.SelectEntryPos( nDelPos );
        aLbTblDbColumn.SetTopEntry( nTopPos );

        bEnableTo = 0 != aLbTblDbColumn.GetEntryCount();
    }
    else if( pButton == &aPbDbcolOneFrom )
    {
        if( LISTBOX_ENTRY_NOTFOUND != aLbTableCol.GetSelectEntryPos() )
        {
            USHORT nFndPos, nInsPos,
                    nDelPos = aLbTableCol.GetSelectEntryPos(),
                    nTopPos = aLbTableCol.GetTopEntry();

            // die richtige InsertPos suchen!!
            SwInsDBColumn aSrch( aLbTableCol.GetEntry( nDelPos ), 0 );
            aDBColumns.Seek_Entry( &aSrch, &nFndPos );
            if( !nFndPos || nFndPos == aDBColumns.Count()-1 )
                nInsPos = nFndPos;
            else
            {
                nInsPos = LISTBOX_ENTRY_NOTFOUND;
                while( ++nFndPos < aDBColumns.Count() &&
                        LISTBOX_ENTRY_NOTFOUND == (nInsPos = aLbTblDbColumn.
                        GetEntryPos( aDBColumns[ nFndPos ]->sColumn )) )
                    ;
            }

            aLbTblDbColumn.InsertEntry( aSrch.sColumn, nInsPos );
            aLbTableCol.RemoveEntry( nDelPos );

            if( nInsPos >= aLbTblDbColumn.GetEntryCount() )
                nInsPos = aLbTblDbColumn.GetEntryCount() - 1;
            aLbTblDbColumn.SelectEntryPos( nInsPos );

            if( nDelPos >= aLbTableCol.GetEntryCount() )
                nDelPos = aLbTableCol.GetEntryCount() - 1;
            aLbTableCol.SelectEntryPos( nDelPos );
            aLbTableCol.SetTopEntry( nTopPos );
        }
        else
            bEnableTo = 0 != aLbTblDbColumn.GetEntryCount();

        bEnableFrom = 0 != aLbTableCol.GetEntryCount();
    }
    else if( pButton == &aPbDbcolAllFrom )
    {
        bEnableFrom = FALSE;

        aLbTblDbColumn.Clear();
        aLbTableCol.Clear();
        for( USHORT n = 0; n < aDBColumns.Count(); ++n )
            aLbTblDbColumn.InsertEntry( aDBColumns[ n ]->sColumn, n );
        aLbTblDbColumn.SelectEntryPos( 0 );
    }
    else if( pButton == &aPbDbcolToEdit )
    {
        bChgEnable = FALSE;
        // Daten ins Edit moven:
        String aFld( aLbTxtDbColumn.GetSelectEntry() );
        if( aFld.Len() )
        {
            String aStr( aEdDbText.GetText() );
            USHORT nPos = (USHORT)aEdDbText.GetSelection().Min();
            USHORT nSel = USHORT(aEdDbText.GetSelection().Max()) - nPos;
            if( nSel )
                // dann loesche erstmal die bestehende Selektion
                aStr.Erase( nPos, nSel );

            aFld.Insert( cDBFldStart, 0 );
            aFld += cDBFldEnd;
            if( aStr.Len() )
            {
                if( nPos )                          // ein Space davor
                {
                    char c = aStr.GetChar( nPos-1 );
                    if( '\n' != c && '\r' != c )
                        aFld.Insert( ' ', 0 );
                }
                if( nPos < aStr.Len() )             // ein Space dahinter
                {
                    char c = aStr.GetChar( nPos );
                    if( '\n' != c && '\r' != c )
                        aFld += ' ';
                }
            }

            aStr.Insert( aFld, nPos );
            aEdDbText.SetText( aStr );
            nPos += aFld.Len();
            aEdDbText.SetSelection( Selection( nPos ));
        }
    }

    if( bChgEnable )
    {
        aPbDbcolOneTo.Enable( bEnableTo );
        aPbDbcolAllTo.Enable( bEnableTo );
        aPbDbcolOneFrom.Enable( bEnableFrom );
        aPbDbcolAllFrom.Enable( bEnableFrom );

        aRbDbFmtFromDb.Enable( FALSE );
        aRbDbFmtFromUsr.Enable( FALSE );
        aLbDbFmtFromUsr.Enable( FALSE );

        aPbTblFormat.Enable( bEnableFrom );
    }
    aLbTblDbColumn.SetUpdateMode( TRUE );
    aLbTableCol.SetUpdateMode( TRUE );

    return 0;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK( SwInsertDBColAutoPilot, DblClickHdl, ListBox*, pBox )
{
    Button* pButton = 0;
    if( pBox == &aLbTxtDbColumn )
        pButton = &aPbDbcolToEdit;
    else if( pBox == &aLbTblDbColumn && aPbDbcolOneTo.IsEnabled() )
        pButton = &aPbDbcolOneTo;
    else if( pBox == &aLbTableCol && aPbDbcolOneFrom.IsEnabled() )
        pButton = &aPbDbcolOneFrom;

    if( pButton )
        TblToFromHdl( pButton );

    return 0;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK( SwInsertDBColAutoPilot, TblFmtHdl, PushButton*, pButton )
{
    SwWrtShell& rSh = pView->GetWrtShell();
    BOOL bNewSet = FALSE;
    if( !pTblSet )
    {
        bNewSet = TRUE;
        pTblSet = new SfxItemSet( rSh.GetAttrPool(), aUITableAttrRange );

        //Ersteinmal die einfachen Attribute besorgen.
        pTblSet->Put( SfxStringItem( FN_PARAM_TABLE_NAME, rSh.GetUniqueTblName() ));
        pTblSet->Put( SfxBoolItem( FN_PARAM_TABLE_HEADLINE, TRUE ) );

        pTblSet->Put( SfxUInt16Item( SID_BACKGRND_DESTINATION,
                                    rSh.GetViewOptions()->GetTblDest() ));

        SvxBrushItem aBrush( RES_BACKGROUND );
        pTblSet->Put( aBrush );
        pTblSet->Put( aBrush, SID_ATTR_BRUSH_ROW );
        pTblSet->Put( aBrush, SID_ATTR_BRUSH_TABLE );

        SvxBoxInfoItem aBoxInfo;
            // Tabellenvariante, wenn mehrere Tabellenzellen selektiert
        aBoxInfo.SetTable( TRUE );
            // Abstandsfeld immer anzeigen
        aBoxInfo.SetDist( TRUE);
            // Minimalgroesse in Tabellen und Absaetzen setzen
        aBoxInfo.SetMinDist( FALSE );
            // Default-Abstand immer setzen
        aBoxInfo.SetDefDist( MIN_BORDER_DIST );
            // Einzelne Linien koennen nur in Tabellen DontCare-Status haben
        aBoxInfo.SetValid( VALID_DISABLE, TRUE );
        pTblSet->Put( aBoxInfo );

        SwGetCurColNumPara aPara;
        const int nNum = rSh.GetCurColNum( &aPara );
        long nWidth;

        if( nNum )
        {
            nWidth = aPara.pPrtRect->Width();
            const SwFmtCol& rCol = aPara.pFrmFmt->GetCol();
            const SwColumns& rCols = rCol.GetColumns();

            //nStart und nEnd initialisieren fuer nNum == 0
            long nWidth1 = 0,
                nStart1 = 0,
                nEnd1 = nWidth;
            for( int i = 0; i < nNum; ++i )
            {
                SwColumn* pCol = rCols[i];
                nStart1 = pCol->GetLeft() + nWidth1;
                nWidth1 += (long)rCol.CalcColWidth( i, (USHORT)nWidth );
                nEnd1 = nWidth1 - pCol->GetRight();
            }
            if(nStart1 || nEnd1 != nWidth)
                nWidth = nEnd1 - nStart1;
        }
        else
            nWidth = rSh.GetAnyCurRect(
                                FRMTYPE_FLY_ANY & rSh.GetFrmType( 0, TRUE )
                                              ? RECT_FLY_PRT_EMBEDDED
                                              : RECT_PAGE_PRT ).Width();

        SwTabCols aTabCols;
        aTabCols.SetRight( nWidth );
        aTabCols.SetRightMax( nWidth );
        pRep = new SwTableRep( aTabCols, FALSE );
        pRep->SetAlign( HORI_NONE );
        pRep->SetSpace( nWidth );
        pRep->SetWidth( nWidth );
        pRep->SetWidthPercent( 100 );
        pTblSet->Put( SwPtrItem( FN_TABLE_REP, pRep ));

        pTblSet->Put( SfxUInt16Item( SID_HTML_MODE,
                    ::GetHtmlMode( pView->GetDocShell() )));
    }

    if( aLbTableCol.GetEntryCount() != pRep->GetAllColCount() )
    {
        // Anzahl der Spalten hat sich geaendert: dann muessen die
        // TabCols angepasst werden
        long nWidth = pRep->GetWidth();
        USHORT nCols = aLbTableCol.GetEntryCount() - 1;
        SwTabCols aTabCols( nCols );
        aTabCols.SetRight( nWidth  );
        aTabCols.SetRightMax( nWidth );
        if( nCols )
            for( USHORT n = 0, nStep = nWidth / (nCols+1), nW = nStep;
                    n < nCols; ++n, nW += nStep )
            {
                aTabCols.Insert( nW, n );
                aTabCols.InsertHidden( n, FALSE );
            }
        delete pRep;
        pRep = new SwTableRep( aTabCols, FALSE );
        pRep->SetAlign( HORI_NONE );
        pRep->SetSpace( nWidth );
        pRep->SetWidth( nWidth );
        pRep->SetWidthPercent( 100 );
        pTblSet->Put( SwPtrItem( FN_TABLE_REP, pRep ));
    }

    SwTableTabDlg* pDlg = new SwTableTabDlg( pButton, rSh.GetAttrPool(),
                                            pTblSet, &rSh );
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
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK( SwInsertDBColAutoPilot, AutoFmtHdl, PushButton*, pButton )
{
    SwAutoFormatDlg aDlg( pButton, pView->GetWrtShellPtr(), FALSE, pTAutoFmt );
    if( RET_OK == aDlg.Execute())
        aDlg.FillAutoFmtOfIndex( pTAutoFmt );
    return 0;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK( SwInsertDBColAutoPilot, SelectHdl, ListBox*, pBox )
{
    ListBox* pGetBox = pBox == &aLbDbFmtFromUsr
                            ? ( aRbAsTable.IsChecked()
                                    ? ( 0 == aLbTableCol.GetEntryData( 0 )
                                        ? &aLbTblDbColumn
                                        : &aLbTableCol )
                                    : &aLbTxtDbColumn )
                            : pBox;

    USHORT nFndPos;
    SwInsDBColumn aSrch( pGetBox->GetSelectEntry(), 0 );
    aDBColumns.Seek_Entry( &aSrch, &nFndPos );

    if( pBox == &aLbDbFmtFromUsr )
    {
        if( aSrch.sColumn.Len() )
        {
            aOldNumFmtLnk.Call( pBox );
            aDBColumns[ nFndPos ]->nUsrNumFmt = aLbDbFmtFromUsr.GetFormat();
        }
    }
    else
    {
        // an der FormatGroupBox den ausgewaehlten FeldNamen setzen, damit
        // klar ist, welches Feld ueber das Format eingestellt wird!
        String sTxt( aGbDbFormat.GetText().Copy( 0, nGBFmtLen ));
        if( !aSrch.sColumn.Len() )
        {
            aRbDbFmtFromDb.Enable( FALSE );
            aRbDbFmtFromUsr.Enable( FALSE );
            aLbDbFmtFromUsr.Enable( FALSE );
        }
        else
        {
            BOOL bEnableFmt = aDBColumns[ nFndPos ]->bHasFmt;
            aRbDbFmtFromDb.Enable( bEnableFmt );
            aRbDbFmtFromUsr.Enable( bEnableFmt );

            if( bEnableFmt )
                (( sTxt += C2S(" (" )) += aSrch.sColumn ) += (sal_Unicode)')';

            BOOL bIsDBFmt = aDBColumns[ nFndPos ]->bIsDBFmt;
            aRbDbFmtFromDb.Check( bIsDBFmt );
            aRbDbFmtFromUsr.Check( !bIsDBFmt );
            aLbDbFmtFromUsr.Enable( !bIsDBFmt );
            if( !bIsDBFmt )
                aLbDbFmtFromUsr.SetDefFormat( aDBColumns[ nFndPos ]->nUsrNumFmt );
        }

        aGbDbFormat.SetText( sTxt );

        // um spaeter zu wissen, welche ListBox die "aktive" war, wird sich
        // im 1. Eintrag ein Flag gemerkt,
        void* pPtr = pBox == &aLbTableCol ? &aLbTableCol : 0;
        aLbTableCol.SetEntryData( 0, pPtr );
    }
    return 0;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK( SwInsertDBColAutoPilot, HeaderHdl, Button*, pButton )
{
    if( pButton == &aCbTableHeadon )
    {
        BOOL bEnable = aCbTableHeadon.IsChecked();

        aRbHeadlColnms.Enable( bEnable );
        aRbHeadlEmpty.Enable( bEnable );
    }
    return 0;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
void lcl_InsTextInArr( const String& rTxt, _DB_Columns& rColArr )
{
    _DB_Column* pNew;
    USHORT nSttPos = 0, nFndPos;
    while( STRING_NOTFOUND != ( nFndPos = rTxt.Search( '\x0A', nSttPos )) )
    {
        if( 1 < nFndPos )
        {
            pNew = new _DB_Column( rTxt.Copy( nSttPos, nFndPos -1 ) );
            rColArr.Insert( pNew, rColArr.Count() );
        }
        pNew = new _DB_Column;
        rColArr.Insert( pNew, rColArr.Count() );
        nSttPos = nFndPos + 1;
    }
    if( nSttPos < rTxt.Len() )
    {
        pNew = new _DB_Column( rTxt.Copy( nSttPos ) );
        rColArr.Insert( pNew, rColArr.Count() );
    }
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
FASTBOOL SwInsertDBColAutoPilot::SplitTextToColArr( const String& rTxt,
                                _DB_Columns& rColArr,
                                BOOL bInsField )
{
    // aus dem Text wieder die einzelnen Datenbank - Spalten erzeugen
    // und dann in einem Array speichern
    // Die Datenbankspalten stehen in <> und muessen im Array der Spalten
    // vorhanden sein:
    String sTxt( rTxt );
    USHORT nFndPos, nEndPos, nSttPos = 0;

    while( STRING_NOTFOUND != ( nFndPos = sTxt.Search( cDBFldStart, nSttPos )))
    {
        nSttPos = nFndPos + 1;
        if( STRING_NOTFOUND != ( nEndPos = sTxt.Search( cDBFldEnd, nSttPos+1 )))
        {
            // Text in <> geklammert gefunden: was ist es denn:
            SwInsDBColumn aSrch( sTxt.Copy( nSttPos, nEndPos - nSttPos ), 0);
            if( aDBColumns.Seek_Entry( &aSrch, &nFndPos ) )
            {
                // das ist ein gueltiges Feld
                // also sicher den Text "davor":
                const SwInsDBColumn& rFndCol = *aDBColumns[ nFndPos ];

                _DB_Column* pNew;

                if( 1 < nSttPos )
                {
                    ::lcl_InsTextInArr( sTxt.Copy( 0, nSttPos-1 ), rColArr );
                    sTxt.Erase( 0, nSttPos-1 );
                }

                sTxt.Erase( 0, rFndCol.sColumn.Len() + 2 );
                nSttPos = 0;

                USHORT nSubType = 0;
                ULONG nFormat;
                if( rFndCol.bHasFmt )
                {
                    if( rFndCol.bIsDBFmt )
                        nFormat =  rFndCol.nDBNumFmt;
                    else
                    {
                        nFormat = rFndCol.nUsrNumFmt;
                        nSubType = SUB_OWN_FMT;
                    }
                }
                else
                    nFormat = 0;

                if( bInsField )
                {
                    SwWrtShell& rSh = pView->GetWrtShell();
                    String sDBContent(aDBData.sDataBaseName);
                    sDBContent += DB_DELIM;
                    sDBContent += aDBData.sDataTableName;
                    SwDBFieldType aFldType( rSh.GetDoc(), aSrch.sColumn,
                                            sDBContent );
                    pNew = new _DB_Column( rFndCol, *new SwDBField(
                            (SwDBFieldType*)rSh.InsertFldType( aFldType ),
                                                            nFormat ) );
                    if( nSubType )
                        pNew->DB_ColumnData.pField->SetSubType( nSubType );
                }
                else
                    pNew = new _DB_Column( rFndCol, nFormat );

                rColArr.Insert( pNew, rColArr.Count() );
            }
        }
    }

    // den letzten Text nicht vergessen
    if( sTxt.Len() )
        ::lcl_InsTextInArr( sTxt, rColArr );

    return 0 != rColArr.Count();
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
void SwInsertDBColAutoPilot::DataToDoc( const SbaSelectionList* pSelList,
    Reference< XDataSource> xSource,
    Reference< XConnection> xConnection )
{
    SwWrtShell& rSh = pView->GetWrtShell();

       Reference< sdbc::XResultSet > xResultSet;
    Reference< sdbc::XRow > xRow;
    Reference< sdbc::XStatement > xStatement;
    BOOL bScrollable;
    try
    {
        bScrollable = xConnection->getMetaData()->supportsResultSetType((sal_Int32)ResultSetType::SCROLL_INSENSITIVE);

        xStatement = xConnection->createStatement();
        Reference< XPropertySet > xStatProp(xStatement, UNO_QUERY);
        if(bScrollable)
        {
            Any aResType;
            aResType <<= (sal_Int32)ResultSetType::SCROLL_INSENSITIVE;
            xStatProp->setPropertyValue(C2U("ResultSetType"), aResType);
        }
        if(xStatement.is())
            xResultSet = xStatement->executeQuery(aDBData.sStatement);
        xRow = Reference< sdbc::XRow >(xResultSet, UNO_QUERY);
    }
    catch(Exception aExcept)
    {
        DBG_ERROR("exception caught")
    }
    if(!xResultSet.is() || !xRow.is())
        return;
    rSh.StartAllAction();
    rSh.StartUndo( 0 );
    BOOL bGroupUndo = rSh.DoesGroupUndo();
    rSh.DoGroupUndo( FALSE );

    SvNumberFormatter& rNumFmtr = *rSh.GetNumberFormatter();

    if( rSh.HasSelection() )
        rSh.DelRight();

    SwWait *pWait = 0;

    if( aRbAsTable.IsChecked() )            // Daten als Tabelle einfuegen
    {
        USHORT n, nRows = 0, nCols = aLbTableCol.GetEntryCount();
        if( aCbTableHeadon.IsChecked() )
            nRows++;

        if( pSelList )
            nRows += (USHORT)pSelList->Count();
        else
            ++nRows;

        // bereite das Array fuer die ausgewaehlten Spalten auf
        SwInsDBColumns_SAR aColFlds( 255 >= nCols ? (BYTE)nCols : 255, 5 );
        for( n = 0; n < nCols; ++n )
        {
            USHORT nFndPos;
            SwInsDBColumn aSrch( aLbTableCol.GetEntry( n ), 0 );
            if( aDBColumns.Seek_Entry( &aSrch, &nFndPos ) )
                aColFlds.Insert( aDBColumns[ nFndPos ], n );
            else
                ASSERT( !this, "Datenbankspalte nicht mehr gefunden" );
        }

        if( nCols != aColFlds.Count() )
        {
            ASSERT( !this, "nicht alle Datenbankspalten gefunden" );
            nCols = aColFlds.Count();
        }

        const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

        BOOL bHTML = 0 != (::GetHtmlMode( pView->GetDocShell() ) & HTMLMODE_ON);
        rSh.InsertTable( nRows, nCols, HORI_FULL,
                         pModOpt->GetInsTblFlags(bHTML),
                        (pSelList ? pTAutoFmt : 0));
        rSh.MoveTable( fnTablePrev, fnTableStart );

        if( pSelList && pTblSet )
            SetTabSet();

        SfxItemSet aTblSet( rSh.GetAttrPool(), RES_BOXATR_FORMAT,
                                                RES_BOXATR_VALUE );
        BOOL bIsAutoUpdateCells = rSh.IsAutoUpdateCells();
        rSh.SetAutoUpdateCells( FALSE );


        if( aCbTableHeadon.IsChecked() )
        {
            for( n = 0; n < nCols; ++n )
            {
                if( aRbHeadlColnms.IsChecked() )
                    rSh.SwEditShell::Insert( aColFlds[ n ]->sColumn );
                rSh.GoNextCell();
            }
        }
        else
            rSh.SetHeadlineRepeat( FALSE );

        for( ULONG i = 0 ; ; ++i )
        {
            BOOL bBreak = FALSE;
            try
            {
                if(bScrollable)
                {
                    if(pSelList)
                        bBreak = !xResultSet->absolute((ULONG)pSelList->GetObject( i ) );
                    else if(!i)
                        bBreak = !xResultSet->first();
                }
                else if(pSelList)
                {
                    ULONG nOldPos = i ? 0 : (ULONG)pSelList->GetObject( i -1 );
                    ULONG nPos = (ULONG)pSelList->GetObject( i );
                    long nDiff = nPos - nOldPos;
                    while(nDiff > 0 && !bBreak)
                    {
                        bBreak = !xResultSet->next();
                        nDiff--;
                    }
                }
                else if(!i)
                    bBreak = !xResultSet->next();

            }
            catch(Exception aExcept)
            {
                bBreak = TRUE;
            }
            if(bBreak)
                break;
            for( n = 0; n < nCols; ++n )
            {
                // beim aller erstenmal KEIN GoNextCell, weil wir schon
                // drin stehen. Auch nicht nach dem Insert das GoNextCell,
                // weil am Ende eine leere Zeile einfuegt wird.
                if( i || n )
                    rSh.GoNextCell();

                const SwInsDBColumn* pEntry = aColFlds[ n ];
                Reference< XColumnsSupplier > xColsSupp( xResultSet, UNO_QUERY );
                Reference <XNameAccess> xCols = xColsSupp->getColumns();
                Any aCol = xCols->getByName(pEntry->sColumn);
                Reference< XPropertySet > xColumnProps;
                if(aCol.hasValue())
                    xColumnProps = *(Reference< XPropertySet >*)aCol.getValue();
                Reference< XColumn > xColumn(xColumnProps, UNO_QUERY);
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
                    else
                    {
                        OUString sVal =  xColumn->getString();
                        if(!xColumn->wasNull())
                            rSh.SwEditShell::Insert( sVal );
                    }
                }
                catch(Exception aExcept)
                {
                    DBG_ERROR(ByteString(String(aExcept.Message), gsl_getSystemTextEncoding()).GetBuffer())
                }
            }

            if( !pSelList )
            {
                BOOL bNext = xResultSet->next();
                if(!bNext)
                    break;
            }
            else if( i+1 >= pSelList->Count() )
                break;

            if( 10 == i )
                pWait = new SwWait( *pView->GetDocShell(), TRUE );
        }

        rSh.MoveTable( fnTableCurr, fnTableStart );
        if( !pSelList && ( pTblSet || pTAutoFmt ))
        {
            if( pTblSet )
                SetTabSet();

            if( pTAutoFmt )
                rSh.SetTableAutoFmt( *pTAutoFmt );
        }
        rSh.SetAutoUpdateCells( bIsAutoUpdateCells );
    }
    else                            // Daten als Felder/Text einfuegen
    {
        _DB_Columns aColArr;
        if( SplitTextToColArr( aEdDbText.GetText(), aColArr, aRbAsField.IsChecked() ) )
        {
            // jetzt kann bei jedem Datensatz einfach ueber das Array iteriert
            // und die Daten eingefuegt werden

            if( !rSh.IsSttPara() )
                rSh.SwEditShell::SplitNode();
            if( !rSh.IsEndPara() )
            {
                rSh.SwEditShell::SplitNode();
                rSh.SwCrsrShell::Left();
            }

            SwTxtFmtColl* pColl = 0;
            {
                String sTmplNm( aLbDbParaColl.GetSelectEntry() );
                if( sNoTmpl != sTmplNm )
                {
                    pColl = rSh.FindTxtFmtCollByName( sTmplNm );
                    if( !pColl )
                    {
                        USHORT nId = rSh.GetPoolId( sTmplNm, GET_POOLID_TXTCOLL );
                        if( USHRT_MAX != nId )
                            pColl = rSh.GetTxtCollFromPool( nId );
                        else
                            pColl = rSh.MakeTxtFmtColl( sTmplNm );
                    }
                    rSh.SetTxtFmtColl( pColl );
                }
            }

            // fuers Einfuegen als Felder -> nach jedem Datensatz ein
            // "NextField" einfuegen
            SwDBFormatData aDBFormatData;
            Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
            if( xMgr.is() )
            {
                Reference<XInterface> xInstance = xMgr->createInstance( C2U( "com.sun.star.util.NumberFormatter" ));
                aDBFormatData.xFormatter = Reference<util::XNumberFormatter>(xInstance, UNO_QUERY) ;
            }

            Reference<XPropertySet> xSourceProps(xSource, UNO_QUERY);
            if(xSourceProps.is())
            {
                Any aFormats = xSourceProps->getPropertyValue(C2U("NumberFormatsSupplier"));
                if(aFormats.hasValue())
                {
                    Reference<XNumberFormatsSupplier> xSuppl = *(Reference<util::XNumberFormatsSupplier>*) aFormats.getValue();
                    if(xSuppl.is())
                    {
                        Reference< XPropertySet > xSettings = xSuppl->getNumberFormatSettings();
                        Any aNull = xSettings->getPropertyValue(C2U("NullDate"));
                        if(aNull.hasValue())
                            aDBFormatData.aNullDate = *(util::Date*)aNull.getValue();
                    }
                }
            }
            if(xSourceProps.is())
            {
                Any aFormats = xSourceProps->getPropertyValue(C2U("NumberFormatsSupplier"));
                if(aFormats.hasValue())
                {
                    Reference<XNumberFormatsSupplier> xSuppl = *(Reference<util::XNumberFormatsSupplier>*) aFormats.getValue();
                    if(xSuppl.is())
                    {
                        Reference< XPropertySet > xSettings = xSuppl->getNumberFormatSettings();
                        Any aNull = xSettings->getPropertyValue(C2U("NullDate"));
                        if(aNull.hasValue())
                            aDBFormatData.aNullDate = *(util::Date*)aNull.getValue();
                    }
                }
            }
            SfxItemSet aSet(pView->GetPool(), RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE);
            rSh.GetAttr( aSet );
            if(SFX_ITEM_DEFAULT <= aSet.GetItemState(RES_CHRATR_LANGUAGE, TRUE))
            {
                Any aLoc;
                const SfxPoolItem& rItem = aSet.Get(RES_CHRATR_LANGUAGE);
                rItem.QueryValue(aLoc,  MID_LANG_LOCALE);
                aDBFormatData.aLocale = *(Locale*)aLoc.getValue();
            }

            String sDBContent(aDBData.sDataBaseName);
            sDBContent += DB_DELIM;
            sDBContent += aDBData.sDataTableName;
            sDBContent += ';';
            sDBContent += aDBData.sStatement;
            SwDBNextSetField aNxtDBFld( (SwDBNextSetFieldType*)rSh.
                                        GetFldType( 0, RES_DBNEXTSETFLD ),
                                        C2S("1"), aEmptyStr, sDBContent );


            BOOL bSetCrsr = TRUE;
            USHORT n, nCols = aColArr.Count();
            for( ULONG i = 0 ; ; ++i )
            {
                BOOL bBreak = FALSE;
                try
                {
                    if(bScrollable)
                    {
                        if(pSelList)
                            bBreak = !xResultSet->absolute((ULONG)pSelList->GetObject( i ) );
                        else if(!i)
                            bBreak = !xResultSet->first();
                    }
                    else if(pSelList)
                    {
                        ULONG nOldPos = i ? 0 : (ULONG)pSelList->GetObject( i -1 );
                        ULONG nPos = (ULONG)pSelList->GetObject( i );
                        long nDiff = nPos - nOldPos;
                        while(nDiff > 0 && !bBreak)
                        {
                            bBreak = !xResultSet->next();
                            nDiff--;
                        }
                    }
                    else if(!i)
                        bBreak = !xResultSet->next();
                }
                catch(...){ bBreak = TRUE; }
                if(bBreak)
                    break;
                for( n = 0; n < nCols; ++n )
                {
                    _DB_Column* pDBCol = aColArr[ n ];
                    String sIns;
                    switch( pDBCol->eColType )
                    {
                    case _DB_Column::DB_FILLTEXT:
                        sIns =  *pDBCol->DB_ColumnData.pText;
                        break;

                    case _DB_Column::DB_SPLITPARA:
                        rSh.SplitNode();
                        // wenn nicht die gleiche Vorlage die Follow Vorlage
                        // ist, dann muss die ausgewaehlte neu gesetzt werden
                        if( pColl && &pColl->GetNextTxtFmtColl() != pColl )
                            rSh.SetTxtFmtColl( pColl );
                        break;

                    case _DB_Column::DB_COL_FIELD:
                        {
                            SwDBField* pFld = (SwDBField*)pDBCol->DB_ColumnData.
                                                pField->Copy();
                            double nValue = DBL_MAX;
                            Reference< XColumnsSupplier > xColsSupp( xResultSet, UNO_QUERY );
                            Reference <XNameAccess> xCols = xColsSupp->getColumns();
                            Any aCol = xCols->getByName(pDBCol->pColInfo->sColumn);
                            Reference< XPropertySet > xColumnProps;
                            if(aCol.hasValue())
                                xColumnProps = *(Reference< XPropertySet >*)aCol.getValue();
                            pFld->SetExpansion( SwNewDBMgr::GetDBField(
                                                xColumnProps,
                                                aDBFormatData,
                                                &nValue ) );
                            if( DBL_MAX != nValue )
                                pFld->ChgValue( nValue, TRUE );
                            pFld->SetInitialized();

                            rSh.Insert( *pFld );
                            delete pFld;
                        }
                        break;

                    case _DB_Column::DB_COL_TEXT:
                        {
                            double nValue = DBL_MAX;
                            Reference< XColumnsSupplier > xColsSupp( xResultSet, UNO_QUERY );
                            Reference <XNameAccess> xCols = xColsSupp->getColumns();
                            Any aCol = xCols->getByName(pDBCol->pColInfo->sColumn);
                            Reference< XPropertySet > xColumnProps;
                            if(aCol.hasValue())
                                xColumnProps = *(Reference< XPropertySet >*)aCol.getValue();
                            sIns = SwNewDBMgr::GetDBField(
                                                xColumnProps,
                                                aDBFormatData,
                                                &nValue );
                            if( pDBCol->DB_ColumnData.nFormat &&
                                DBL_MAX != nValue )
                            {
                                Color* pCol;
                                SvNumberFormatter& rNFmtr =
                                                    *rSh.GetNumberFormatter();
                                rNFmtr.GetOutputString( nValue,
                                            pDBCol->DB_ColumnData.nFormat,
                                            sIns, &pCol );
                            }
                        }
                        break;
                    }

                    if( sIns.Len() )
                        rSh.Insert( sIns );

                    if( bSetCrsr && sIns.Len() )
                    {
                        // zum Anfang und eine Mark setzen, damit der
                        // Cursor am Ende wieder auf Anfangsposition
                        // gesetzt werden kann.
                        BOOL bDoesUndo = rSh.DoesUndo();
                        rSh.DoUndo( FALSE );
                        rSh.SwCrsrShell::MovePara( fnParaCurr, fnParaStart );
                        rSh.SetBookmark( KeyCode(), C2S("DB_Mark"), aEmptyStr, MARK );
                        rSh.SwCrsrShell::MovePara( fnParaCurr, fnParaEnd );
                        bSetCrsr = FALSE;
                        rSh.DoUndo( bDoesUndo );
                    }
                }

                if( !pSelList )
                {
                    BOOL bNext = xResultSet->next();
                    if(!bNext)
                        break;
                }
                else if( i+1 >= pSelList->Count() )
                    break;

                if( aRbAsField.IsChecked() )
                    rSh.Insert( aNxtDBFld );

                if( !rSh.IsSttPara() )
                    rSh.SwEditShell::SplitNode();

                if( 10 == i )
                    pWait = new SwWait( *pView->GetDocShell(), TRUE );
            }

            if( !bSetCrsr && USHRT_MAX != (n = rSh.FindBookmark( C2S("DB_Mark" ))) )
            {
                BOOL bDoesUndo = rSh.DoesUndo();
                rSh.DoUndo( FALSE );
                rSh.GotoBookmark( n );
                rSh.DelBookmark( n );
                rSh.DoUndo( bDoesUndo );
            }
        }
    }

    // Daten ins INI-File schreiben
    WriteUIToIni();

    rSh.DoGroupUndo( bGroupUndo );
    rSh.EndUndo( 0 );
    rSh.EndAllAction();
    delete pWait;
}
void SwInsertDBColAutoPilot::SetTabSet()
{
    SwWrtShell& rSh = pView->GetWrtShell();
    const SfxPoolItem* pItem;

    if( pTAutoFmt )
    {
        if( pTAutoFmt->IsFrame() )
        {
            // Umrandung kommt vom AutoFormat
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
        // die Defaults wieder entfernen, es macht keinen Sinn sie zu setzen
        SvxBrushItem aBrush( RES_BACKGROUND );
        static USHORT __READONLY_DATA aIds[3] =
            { RES_BACKGROUND, SID_ATTR_BRUSH_ROW, SID_ATTR_BRUSH_TABLE };
        for( int i = 0; i < 3; ++i )
            if( SFX_ITEM_SET == pTblSet->GetItemState( aIds[ i ],
                FALSE, &pItem ) && *pItem == aBrush )
                pTblSet->ClearItem( aIds[ i ] );
    }

    if( SFX_ITEM_SET == pTblSet->GetItemState( FN_PARAM_TABLE_NAME, FALSE,
        &pItem ) && ((const SfxStringItem*)pItem)->GetValue() ==
                    rSh.GetTableFmt()->GetName() )
        pTblSet->ClearItem( FN_PARAM_TABLE_NAME );

    rSh.MoveTable( fnTableCurr, fnTableStart );
    rSh.SetMark();
    rSh.MoveTable( fnTableCurr, fnTableEnd );

    ::lcl_ItemSetToTableParam( *pTblSet, rSh );

    rSh.ClearMark();
    rSh.MoveTable( fnTableCurr, fnTableStart );
}

    // Daten ins INI-File schreiben
void SwInsertDBColAutoPilot::WriteUIToIni()
{
    if( !pConfig )
    {
        pConfig = new _DB_ColumnConfig;
        pConfig->Initialize();
    }

    _DB_ColumnConfigData* pNewData = new _DB_ColumnConfigData;

    //die Column-Daten Pointer gehen in den Besitz des pNewData!
    pNewData->aDBColumns.Insert( &aDBColumns );
    aDBColumns.Remove( (USHORT)0, aDBColumns.Count() );

    pNewData->sDBName = pView->GetWrtShell().GetDBName();
    pNewData->sEdit = aEdDbText.GetText();

    String sTmp;
    for( USHORT n = 0, nCnt = aLbTableCol.GetEntryCount(); n < nCnt; ++n )
        ( sTmp += aLbTableCol.GetEntry( n ) ) += '\x0a';

    if( sTmp.Len() )
        pNewData->sTblList = sTmp;

    if( sNoTmpl != (sTmp = aLbDbParaColl.GetSelectEntry()) )
        pNewData->sTmplNm = sTmp;

    if( pTAutoFmt )
        pNewData->sTAutoFmtNm = pTAutoFmt->GetName();

    pNewData->bIsTable = aRbAsTable.IsChecked();
    pNewData->bIsField = aRbAsField.IsChecked();
    pNewData->bIsHeadlineOn = aCbTableHeadon.IsChecked();
    pNewData->bIsEmptyHeadln = aRbHeadlEmpty.IsChecked();


    // jetzt noch die benutzerdefinierten Numberformatstrings erfragen,
    // denn nur  diese lassen sich "unabhaengig" speichern!
    SvNumberFormatter& rNFmtr = *pView->GetWrtShell().GetNumberFormatter();
    GetDocPoolNm( RES_POOLCOLL_STANDARD, sTmp );
    for( n = 0; n < pNewData->aDBColumns.Count(); ++n )
    {
        SwInsDBColumn& rSet = *pNewData->aDBColumns[ n ];
        if( rSet.bHasFmt && !rSet.bIsDBFmt )
        {
            const SvNumberformat* pNF = rNFmtr.GetEntry( rSet.nUsrNumFmt );
            if( pNF )
            {
                rSet.sUsrNumFmt = pNF->GetFormatstring();
                rSet.eUsrNumFmtLng = pNF->GetLanguage();
            }
            else
            {
                rSet.sUsrNumFmt = sTmp;
                rSet.eUsrNumFmtLng = LANGUAGE_SYSTEM;
            }
        }
    }

    pConfig->SetData( pNewData );
    pConfig->StoreConfig();
}

    // Daten aus dem INI-File lesen:
void SwInsertDBColAutoPilot::ReadIniToUI()
{
    if( !pConfig )

    {
        pConfig = new _DB_ColumnConfig;
        pConfig->Initialize();
    }

    _DB_ColumnConfigData aData;

    //die Column-Daten Pointer werden kopiert, bleiben im Besitz vom Dialog!
    aData.aDBColumns.Insert( &aDBColumns );
    aData.sDBName = pView->GetWrtShell().GetDBName();

    const _DB_ColumnConfigData* pFndData = pConfig->HasData( aData );
    if( pFndData )
    {
        USHORT n = 0;
        String sTmp( pFndData->sTblList );
        if( sTmp.Len() )
        {
            do {
                String sEntry( sTmp.GetToken( 0, '\x0a', n ) );
                aLbTableCol.InsertEntry( sEntry );
                aLbTblDbColumn.RemoveEntry( sEntry );
            } while( n < sTmp.Len() );

            if( !aLbTblDbColumn.GetEntryCount() )
            {
                aPbDbcolAllTo.Enable( FALSE );
                aPbDbcolOneTo.Enable( FALSE );
            }
            aPbDbcolOneFrom.Enable( TRUE );
            aPbDbcolAllFrom.Enable( TRUE );
        }
        aEdDbText.SetText( pFndData->sEdit );

        sTmp = pFndData->sTmplNm;
        if( sTmp.Len() )
            aLbDbParaColl.SelectEntry( sTmp );
        else
            aLbDbParaColl.SelectEntryPos( 0 );

        if( pTAutoFmt )
            delete pTAutoFmt, pTAutoFmt = 0;
        sTmp = pFndData->sTAutoFmtNm;
        if( sTmp.Len() )
        {
            // dann erstmal die AutoFmt-Datei laden und das Autoformat suchen
            SwTableAutoFmtTbl aAutoFmtTbl;
            aAutoFmtTbl.Load();
            for( USHORT n = aAutoFmtTbl.Count(); n; )
                if( sTmp == aAutoFmtTbl[ --n ]->GetName() )
                {
                    pTAutoFmt = new SwTableAutoFmt( *aAutoFmtTbl[ n ] );
                    break;
                }
        }

        aRbAsTable.Check( pFndData->bIsTable );
        aRbAsField.Check( pFndData->bIsField );
        aRbAsText.Check( !pFndData->bIsTable && !pFndData->bIsField );

        aCbTableHeadon.Check( pFndData->bIsHeadlineOn );
        aRbHeadlColnms.Check( !pFndData->bIsEmptyHeadln );
        aRbHeadlEmpty.Check( pFndData->bIsEmptyHeadln );

        // jetzt noch die benutzerdefinierten Numberformat Strings in die
        // Shell kopieren. Nur diese sind dann als ID verfuegbar
        SvNumberFormatter& rNFmtr = *pView->GetWrtShell().GetNumberFormatter();
        for( n = 0; n < aDBColumns.Count(); ++n )
        {
            SwInsDBColumn& rSet = *aDBColumns[ n ];
            const SwInsDBColumn& rGet = *pFndData->aDBColumns[ n ];
            if( rGet.bHasFmt && !rGet.bIsDBFmt )
            {
                rSet.bIsDBFmt = FALSE;
                rSet.nUsrNumFmt = rNFmtr.GetEntryKey( rGet.sUsrNumFmt,
                                                        rGet.eUsrNumFmtLng );
                if( NUMBERFORMAT_ENTRY_NOT_FOUND == rSet.nUsrNumFmt )
                {
                    xub_StrLen nCheckPos;
                    short nType;
                    rNFmtr.PutEntry( (String&)rGet.sUsrNumFmt, nCheckPos, nType,
                                    rSet.nUsrNumFmt, rGet.eUsrNumFmtLng );
                }
            }
        }

        // steht der Cursor in einer Tabelle, darf NIE Tabelle auswaehlbar sein
        if( !aRbAsTable.IsEnabled() && aRbAsTable.IsChecked() )
            aRbAsField.Check( TRUE );
    }

    aData.aDBColumns.Remove( (USHORT)0, aDBColumns.Count() );
}

/*  */


_DB_ColumnConfigData::~_DB_ColumnConfigData() {}

BOOL _DB_ColumnConfigData::IsEqualDB( const _DB_ColumnConfigData& rCmp ) const
{
    // teste, ob die Daten von der gleiche Databanktabelle kommen und
    // ob sie noch die gleichen Werte enthaelt.
    BOOL bRet = FALSE;
    if( sDBName == rCmp.sDBName &&
        aDBColumns.Count() == rCmp.aDBColumns.Count() )
    {
        bRet = TRUE;
        for( USHORT n = aDBColumns.Count(); n; )
        {
            --n;
            const SwInsDBColumn& rCmp1 = *aDBColumns[ n ];
            const SwInsDBColumn& rCmp2 = *rCmp.aDBColumns[ n ];
            if( rCmp1.sColumn != rCmp2.sColumn ||
                rCmp1.nCol != rCmp2.nCol ||
                rCmp1.bHasFmt != rCmp2.bHasFmt )
            {
                bRet = FALSE;
                break;
            }
        }
    }
    return bRet;
}

_DB_ColumnConfigData::_DB_ColumnConfigData( SvStream& rStream, UINT16 nVers )
{
    rStream.ReadByteString( sDBName ).ReadByteString(sEdit )
           .ReadByteString( sTblList ).ReadByteString( sTmplNm )
           .ReadByteString( sTAutoFmtNm );

    UINT16 n;
    rStream >> n;
    if( n )
    {
#ifdef WITH_STREAM_CORRECT
        ULONG nSttPos = rStream.Tell();
        UINT16 nOffset;
#endif
        UINT16 nCol, eLng;
        UINT8 nFlags;
        String sNm, sUsrFmt;

        for( UINT16 i = 0; i < n; ++i )
        {
#ifdef WITH_STREAM_CORRECT
            rStream >> nOffset;
#endif
            rStream.ReadByteString( sNm );
            rStream >> nCol
                    >> nFlags;
            rStream.ReadByteString( sUsrFmt );
            rStream >> eLng;

            SwInsDBColumn* pNew = new SwInsDBColumn( sNm, nCol );
            pNew->sUsrNumFmt = sUsrFmt;
            pNew->eUsrNumFmtLng = (LanguageType)eLng;

            pNew->bHasFmt   = 0 != (nFlags & 0x1);
            pNew->bIsDBFmt  = 0 != (nFlags & 0x2);

            aDBColumns.Insert( pNew, i );

#ifdef WITH_STREAM_CORRECT
            // muss etwas uebersprungen werden?
            ULONG nCurPos = rStream.Tell();
            if( nCurPos != ( nSttPos + nOffset ))
                rStream.Seek( nCurPos = ( nSttPos + nOffset ) );
            nSttPos = nCurPos;
#endif
        }
    }

    rStream >> n;
    bIsTable        = 0 != (n & 0x01);
    bIsField        = 0 != (n & 0x02);
    bIsHeadlineOn   = 0 != (n & 0x04);
    bIsEmptyHeadln  = 0 != (n & 0x08);

/*
    if( nVers > DBCOLUMN_CONFIG_VERSION1 )
    {
    }
*/
}

void _DB_ColumnConfigData::Save( SvStream& rStream ) const
{
    rStream.WriteByteString( sDBName ).WriteByteString( sEdit )
           .WriteByteString( sTblList ).WriteByteString( sTmplNm )
           .WriteByteString( sTAutoFmtNm );

    UINT16 n = aDBColumns.Count();
    rStream << n;
    if( n )
    {
#ifdef WITH_STREAM_CORRECT
        ULONG nCurPos, nSttPos = rStream.Tell();
#endif
        for( UINT16 i = 0; i < n; ++i )
        {
#ifdef WITH_STREAM_CORRECT
            rStream << (UINT16)0;
#endif

            const SwInsDBColumn& rCol = *aDBColumns[ i ];
            UINT8 nFlags = 0;
            if( rCol.bHasFmt )      nFlags |= 0x1;
            if( rCol.bIsDBFmt )     nFlags |= 0x2;

            rStream.WriteByteString( rCol.sColumn )
                    << (UINT16)rCol.nCol
                    << nFlags;
            rStream.WriteByteString( rCol.sUsrNumFmt )
                    << (UINT16)rCol.eUsrNumFmtLng
                    ;

#ifdef WITH_STREAM_CORRECT
            nCurPos = rStream.Tell();
            rStream.Seek( nSttPos );
            rStream << (UINT16)(nCurPos - nSttPos);
            rStream.Seek( nCurPos );
            nSttPos = nCurPos;
#endif
        }
    }

    n = 0;
    if( bIsTable )          n |= 0x01;
    if( bIsField )          n |= 0x02;
    if( bIsHeadlineOn )     n |= 0x04;
    if( bIsEmptyHeadln )    n |= 0x08;
    rStream << n;
}




_DB_ColumnConfig::_DB_ColumnConfig()
    : SfxConfigItem( CFG_INSERT_DBCOLUMN_ITEM )
{
    memset( &aData, 0, sizeof( aData ));
}

_DB_ColumnConfig::~_DB_ColumnConfig()
{
    for( USHORT n = DBCOLUMN_MAXDATA; n; )
        delete aData[ --n ];
}

void _DB_ColumnConfig::UseDefault()
{
    for( USHORT n = DBCOLUMN_MAXDATA; n; )
        delete aData[ --n ];
    SfxConfigItem::UseDefault();
}

String _DB_ColumnConfig::GetName() const
{
    return SW_RESSTR( STR_CFG_INSERT_DBCOLUMN );
}

int _DB_ColumnConfig::Load( SvStream& rStream )
{
    int nRet = SfxConfigItem::ERR_OK;

    SetDefault( FALSE );
    UINT16 nVers;
    rStream >> nVers;
    if( DBCOLUMN_CONFIG_VERSION1 >= nVers )
    {
        UINT16 n, nCnt;
        CharSet eOldCharSet = rStream.GetStreamCharSet();
        rStream >> n >> nCnt;

        if( nCnt )
        {
            rStream.SetStreamCharSet( (CharSet)n );

#ifdef WITH_STREAM_CORRECT
            ULONG nSttPos = rStream.Tell();
#endif
            for( UINT16 i = 0; i < nCnt; ++i )
            {
#ifdef WITH_STREAM_CORRECT
                rStream >> n;
#endif
                if( i < DBCOLUMN_MAXDATA )
                    aData[ i ] = new _DB_ColumnConfigData( rStream, nVers );

#ifdef WITH_STREAM_CORRECT
                // muss etwas uebersprungen werden?
                ULONG nCurPos = rStream.Tell();
                if( nCurPos != ( nSttPos + n ))
                    rStream.Seek( nCurPos = ( nSttPos + n ) );
                nSttPos = nCurPos;
#endif
            }
            rStream.SetStreamCharSet( eOldCharSet );
        }
    }
    else
        nRet = SfxConfigItem::WARNING_VERSION;

    return nRet;
}

BOOL _DB_ColumnConfig::Store( SvStream& rStream )
{
    CharSet eOldCharSet = rStream.GetStreamCharSet();
    rtl_TextEncoding eCurCharSet = ::gsl_getSystemTextEncoding();
    rStream.SetStreamCharSet( eCurCharSet );

    rStream << (UINT16)DBCOLUMN_CONFIG_VERSION
            << (UINT16)eCurCharSet;

    USHORT nCnt = DBCOLUMN_MAXDATA;
    for( USHORT n = 0; n < nCnt; ++n )
        if( !aData[ n ] )
        {
            nCnt = n;
            break;
        }

    rStream << (UINT16)nCnt;
    if( nCnt )
    {
#ifdef WITH_STREAM_CORRECT
        ULONG nCurPos, nSttPos = rStream.Tell();
#endif
        for( USHORT n = 0; n < nCnt; ++n )
        {
#ifdef WITH_STREAM_CORRECT
            rStream << (UINT16)0;
#endif

            aData[ n ]->Save( rStream );

#ifdef WITH_STREAM_CORRECT
            nCurPos = rStream.Tell();
            rStream.Seek( nSttPos );
            rStream << (UINT16)(nCurPos - nSttPos);
            rStream.Seek( nCurPos );
            nSttPos = nCurPos;
#endif
        }
    }
    rStream.SetStreamCharSet( eOldCharSet );

    return SfxConfigItem::ERR_OK;
}

const _DB_ColumnConfigData* _DB_ColumnConfig::HasData(
        const _DB_ColumnConfigData& rData ) const
{
    const _DB_ColumnConfigData* pRet = 0;
    for( USHORT n = 0; n < DBCOLUMN_MAXDATA; ++n )
        if( aData[ n ] && aData[ n ]->IsEqualDB( rData ))
        {
            pRet = aData[ n ];
            break;
        }
    return pRet;
}

void _DB_ColumnConfig::SetData( _DB_ColumnConfigData* pData )
{
    ASSERT( pData, "kein pData ans SetData uebergeben" );
    if( pData )
    {
        USHORT n, nDelPos = DBCOLUMN_MAXDATA-1;
        for( n = 0; n < DBCOLUMN_MAXDATA; ++n )
            if( !aData[ n ] ||  aData[ n ]->IsEqualDB( *pData ))
            {
                nDelPos = n;
                break;
            }

        // loesche den Eintrag an der DelPos und verschiebe alle
        // vorhergehenden eine Position runter. Der aktuelle wird
        // dann wieder am Anfang eingefuegt.
        delete aData[ nDelPos ];
        if( nDelPos )
            memmove( aData+1, aData, nDelPos * sizeof( aData[ 0 ] ));
        aData[ 0 ] = pData;

        SetDefault( FALSE );
    }
}


