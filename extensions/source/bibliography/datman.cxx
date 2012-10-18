/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <osl/mutex.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdb/XDatabaseEnvironment.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/io/XDataInputStream.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <comphelper/container.hxx>
#include <svl/urihelper.hxx>
#include <svtools/svtabbx.hxx>
#include <svtools/headbar.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/msgbox.hxx>
#include <tools/debug.hxx>
#include "datman.hxx"
#include "bibresid.hxx"
#include "bibmod.hxx"
#include "bibview.hxx"
// #100312# ---------
#include "bibprop.hrc"
#include "toolbar.hxx"
#include "toolbar.hrc"
#include "bibconfig.hxx"
#include "bibbeam.hxx"
#include "bib.hrc"
#include "datman.hrc"
#include "bibliography.hrc"
#include <connectivity/dbtools.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;

#define C2U(cChar) ::rtl::OUString::createFromAscii(cChar)

Reference< XConnection > getConnection(const ::rtl::OUString& _rURL)
{
    // first get the sdb::DataSource corresponding to the url
    Reference< XDataSource >    xDataSource;
    // is it a favorite title ?
    Reference<XComponentContext>  xContext = comphelper::getProcessComponentContext();
    Reference< XDatabaseContext >  xNamingContext = DatabaseContext::create(xContext);
    if (xNamingContext->hasByName(_rURL))
    {
        DBG_ASSERT(Reference< XNamingService > (xNamingContext, UNO_QUERY).is(), "::getDataSource : no NamingService interface on the sdb::DatabaseAccessContext !");
        try
        {
            xDataSource = Reference< XDataSource > (Reference< XNamingService > (xNamingContext, UNO_QUERY)->getRegisteredObject(_rURL), UNO_QUERY);
        }
        catch (const Exception&)
        {
            OSL_FAIL("Exception caught in ODatabaseContext::getRegisteredObject()");
        }
    }
    // build the connection from the data source
    Reference< XConnection > xConn;
    if (xDataSource.is())
    {
        // need user/pwd for this
        Reference< XPropertySet >  xDataSourceProps(xDataSource, UNO_QUERY);
        Reference< XCompletedConnection > xComplConn(xDataSource, UNO_QUERY);
        try
        {
            Reference<task::XInteractionHandler> xIHdl( task::InteractionHandler::createWithParent(xContext, 0), UNO_QUERY_THROW);
            xConn = xComplConn->connectWithCompletion(xIHdl);
        }
        catch (const SQLException&)
        {
            // TODO : a real error handling
        }
        catch (const Exception&)
        {
        }
    }
    return xConn;
}

Reference< XConnection >    getConnection(const Reference< XInterface > & xRowSet)
{
    Reference< XConnection >    xConn;
    try
    {
        Reference< XPropertySet >  xFormProps(xRowSet, UNO_QUERY);
        if (!xFormProps.is())
            return xConn;

        xConn = Reference< XConnection > (*(Reference< XInterface > *)xFormProps->getPropertyValue(C2U("ActiveConnection")).getValue(), UNO_QUERY);
        if (!xConn.is())
        {
            DBG_WARNING("no active connection");
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("exception in getConnection");
    }

    return xConn;
}

Reference< XNameAccess >  getColumns(const Reference< XForm > & _rxForm)
{
    Reference< XNameAccess >  xReturn;
    // check if the form is alive
    Reference< XColumnsSupplier >  xSupplyCols( _rxForm, UNO_QUERY );
    if (xSupplyCols.is())
        xReturn = xSupplyCols->getColumns();

    if (!xReturn.is() || (xReturn->getElementNames().getLength() == 0))
    {   // no ....
        xReturn = NULL;
        // -> get the table the form is bound to and ask it for their columns
        Reference< XTablesSupplier >  xSupplyTables( getConnection( _rxForm ), UNO_QUERY );
        Reference< XPropertySet >  xFormProps( _rxForm, UNO_QUERY );
        if (xFormProps.is() && xSupplyTables.is())
        {
            try
            {
                DBG_ASSERT((*(sal_Int32*)xFormProps->getPropertyValue(C2U("CommandType")).getValue()) == CommandType::TABLE,
                    "::getColumns : invalid form (has no table as data source) !");
                ::rtl::OUString sTable;
                xFormProps->getPropertyValue(C2U("Command")) >>= sTable;
                Reference< XNameAccess >  xTables = xSupplyTables->getTables();
                if (xTables.is() && xTables->hasByName(sTable))
                    xSupplyCols = Reference< XColumnsSupplier > (
                        *(Reference< XInterface > *)xTables->getByName(sTable).getValue(), UNO_QUERY);
                if (xSupplyCols.is())
                    xReturn = xSupplyCols->getColumns();
            }
            catch (const Exception& e)
            {
#ifdef DBG_UTIL
                String sMsg(rtl::OUString("::getColumns : catched an exception ("));
                sMsg += String(e.Message);
                sMsg.AppendAscii(") ...");
                OSL_FAIL(rtl::OUStringToOString(sMsg, RTL_TEXTENCODING_ASCII_US ).getStr());
#else
                (void)e;
#endif
            }

        }
    }
    return xReturn;
}

class MappingDialog_Impl : public ModalDialog
{
    BibDataManager* pDatMan;
    OKButton        aOKBT;
    CancelButton    aCancelBT;
    HelpButton      aHelpBT;
    FixedLine       aMapGB;
    FixedText       aIdentifierFT;
    ListBox         aIdentifierLB;
    FixedText       aAuthorityTypeFT;
    ListBox         aAuthorityTypeLB;
    FixedText       aAuthorFT;
    ListBox         aAuthorLB;
    FixedText       aTitleFT;
    ListBox         aTitleLB;
    FixedText       aMonthFT;
    ListBox         aMonthLB;
    FixedText       aYearFT;
    ListBox         aYearLB;
    FixedText       aISBNFT;
    ListBox         aISBNLB;
    FixedText       aBooktitleFT;
    ListBox         aBooktitleLB;
    FixedText       aChapterFT;
    ListBox         aChapterLB;
    FixedText       aEditionFT;
    ListBox         aEditionLB;
    FixedText       aEditorFT;
    ListBox         aEditorLB;
    FixedText       aHowpublishedFT;
    ListBox         aHowpublishedLB;
    FixedText       aInstitutionFT;
    ListBox         aInstitutionLB;
    FixedText       aJournalFT;
    ListBox         aJournalLB;
    FixedText       aNoteFT;
    ListBox         aNoteLB;
    FixedText       aAnnoteFT;
    ListBox         aAnnoteLB;
    FixedText       aNumberFT;
    ListBox         aNumberLB;
    FixedText       aOrganizationsFT;
    ListBox         aOrganizationsLB;
    FixedText       aPagesFT;
    ListBox         aPagesLB;
    FixedText       aPublisherFT;
    ListBox         aPublisherLB;
    FixedText       aAddressFT;
    ListBox         aAddressLB;
    FixedText       aSchoolFT;
    ListBox         aSchoolLB;
    FixedText       aSeriesFT;
    ListBox         aSeriesLB;
    FixedText       aReportTypeFT;
    ListBox         aReportTypeLB;
    FixedText       aVolumeFT;
    ListBox         aVolumeLB;
    FixedText       aURLFT;
    ListBox         aURLLB;
    FixedText       aCustom1FT;
    ListBox         aCustom1LB;
    FixedText       aCustom2FT;
    ListBox         aCustom2LB;
    FixedText       aCustom3FT;
    ListBox         aCustom3LB;
    FixedText       aCustom4FT;
    ListBox         aCustom4LB;
    FixedText       aCustom5FT;
    ListBox         aCustom5LB;
    ListBox*        aListBoxes[COLUMN_COUNT];
    String          sNone;

    sal_Bool        bModified;



    DECL_LINK(OkHdl, void *);
    DECL_LINK(ListBoxSelectHdl, ListBox*);

public:
    MappingDialog_Impl(Window* pParent, BibDataManager* pDatMan);
    ~MappingDialog_Impl();

    void    SetModified() {bModified = sal_True;}

};

static sal_uInt16 lcl_FindLogicalName(BibConfig* pConfig ,
                                    const ::rtl::OUString& rLogicalColumnName)
{
    for(sal_uInt16 i = 0; i < COLUMN_COUNT; i++)
    {
        if(rLogicalColumnName == pConfig->GetDefColumnName(i))
            return i;
    }
    return USHRT_MAX;
}
//-----------------------------------------------------------------------------
MappingDialog_Impl::MappingDialog_Impl(Window* pParent, BibDataManager* pMan) :
    ModalDialog(pParent, BibResId(RID_DLG_MAPPING) ),
    pDatMan(pMan),
    aOKBT(this,                 BibResId( BT_OK         )),
    aCancelBT(this,             BibResId( BT_CANCEL     )),
    aHelpBT(this,               BibResId( BT_HELP           )),

    aMapGB(this,                BibResId( GB_MAPPING        )),

    aIdentifierFT(this,         BibResId( FT_IDENTIFIER )),
    aIdentifierLB(this,         BibResId( LB_IDENTIFIER )),
    aAuthorityTypeFT(this,      BibResId( FT_AUTHORITYTYPE )),
    aAuthorityTypeLB(this,      BibResId( LB_AUTHORITYTYPE )),
    aAuthorFT(this,             BibResId( FT_AUTHOR     )),
    aAuthorLB(this,             BibResId( LB_AUTHOR     )),
    aTitleFT(this,              BibResId( FT_TITLE      )),
    aTitleLB(this,              BibResId( LB_TITLE      )),
    aMonthFT(this,              BibResId( FT_MONTH      )),
    aMonthLB(this,              BibResId( LB_MONTH      )),
    aYearFT(this,               BibResId( FT_YEAR           )),
    aYearLB(this,               BibResId( LB_YEAR           )),
    aISBNFT(this,               BibResId( FT_ISBN           )),
    aISBNLB(this,               BibResId( LB_ISBN           )),
    aBooktitleFT(this,          BibResId( FT_BOOKTITLE  )),
    aBooktitleLB(this,          BibResId( LB_BOOKTITLE  )),
    aChapterFT(this,            BibResId( FT_CHAPTER        )),
    aChapterLB(this,            BibResId( LB_CHAPTER        )),
    aEditionFT(this,            BibResId( FT_EDITION        )),
    aEditionLB(this,            BibResId( LB_EDITION        )),
    aEditorFT(this,             BibResId( FT_EDITOR     )),
    aEditorLB(this,             BibResId( LB_EDITOR     )),
    aHowpublishedFT(this,       BibResId( FT_HOWPUBLISHED   )),
    aHowpublishedLB(this,       BibResId( LB_HOWPUBLISHED   )),
    aInstitutionFT(this,        BibResId( FT_INSTITUTION    )),
    aInstitutionLB(this,        BibResId( LB_INSTITUTION    )),
    aJournalFT(this,            BibResId( FT_JOURNAL        )),
    aJournalLB(this,            BibResId( LB_JOURNAL        )),
    aNoteFT(this,               BibResId( FT_NOTE           )),
    aNoteLB(this,               BibResId( LB_NOTE           )),
    aAnnoteFT(this,             BibResId( FT_ANNOTE     )),
    aAnnoteLB(this,             BibResId( LB_ANNOTE     )),
    aNumberFT(this,             BibResId( FT_NUMBER     )),
    aNumberLB(this,             BibResId( LB_NUMBER     )),
    aOrganizationsFT(this,      BibResId( FT_ORGANIZATIONS )),
    aOrganizationsLB(this,      BibResId( LB_ORGANIZATIONS )),
    aPagesFT(this,              BibResId( FT_PAGES      )),
    aPagesLB(this,              BibResId( LB_PAGES      )),
    aPublisherFT(this,          BibResId( FT_PUBLISHER  )),
    aPublisherLB(this,          BibResId( LB_PUBLISHER  )),
    aAddressFT(this,            BibResId( FT_ADDRESS        )),
    aAddressLB(this,            BibResId( LB_ADDRESS        )),
    aSchoolFT(this,             BibResId( FT_SCHOOL     )),
    aSchoolLB(this,             BibResId( LB_SCHOOL     )),
    aSeriesFT(this,             BibResId( FT_SERIES     )),
    aSeriesLB(this,             BibResId( LB_SERIES     )),
    aReportTypeFT(this,         BibResId( FT_REPORTTYPE )),
    aReportTypeLB(this,         BibResId( LB_REPORTTYPE )),
    aVolumeFT(this,             BibResId( FT_VOLUME     )),
    aVolumeLB(this,             BibResId( LB_VOLUME     )),
    aURLFT(this,                BibResId( FT_URL            )),
    aURLLB(this,                BibResId( LB_URL            )),
    aCustom1FT(this,            BibResId( FT_CUSTOM1        )),
    aCustom1LB(this,            BibResId( LB_CUSTOM1        )),
    aCustom2FT(this,            BibResId( FT_CUSTOM2        )),
    aCustom2LB(this,            BibResId( LB_CUSTOM2        )),
    aCustom3FT(this,            BibResId( FT_CUSTOM3        )),
    aCustom3LB(this,            BibResId( LB_CUSTOM3        )),
    aCustom4FT(this,            BibResId( FT_CUSTOM4        )),
    aCustom4LB(this,            BibResId( LB_CUSTOM4        )),
    aCustom5FT(this,            BibResId( FT_CUSTOM5        )),
    aCustom5LB(this,            BibResId( LB_CUSTOM5        )),
    sNone(BibResId(ST_NONE)),
    bModified(sal_False)
{
    FreeResource();

    aIdentifierFT.SetText(String(         BibResId( ST_IDENTIFIER    )));
    aAuthorityTypeFT.SetText(String(      BibResId( ST_AUTHTYPE )));
    aAuthorFT.SetText(String(             BibResId( ST_AUTHOR        )));
    aTitleFT.SetText(String(              BibResId( ST_TITLE         )));
    aMonthFT.SetText(String(              BibResId( ST_MONTH         )));
    aYearFT.SetText(String(               BibResId( ST_YEAR          )));
    aISBNFT.SetText(String(               BibResId( ST_ISBN          )));
    aBooktitleFT.SetText(String(          BibResId( ST_BOOKTITLE     )));
    aChapterFT.SetText(String(            BibResId( ST_CHAPTER       )));
    aEditionFT.SetText(String(            BibResId( ST_EDITION       )));
    aEditorFT.SetText(String(             BibResId( ST_EDITOR        )));
    aHowpublishedFT.SetText(String(       BibResId( ST_HOWPUBLISHED  )));
    aInstitutionFT.SetText(String(        BibResId( ST_INSTITUTION   )));
    aJournalFT.SetText(String(            BibResId( ST_JOURNAL       )));
    aNoteFT.SetText(String(               BibResId( ST_NOTE          )));
    aAnnoteFT.SetText(String(             BibResId( ST_ANNOTE        )));
    aNumberFT.SetText(String(             BibResId( ST_NUMBER        )));
    aOrganizationsFT.SetText(String(      BibResId( ST_ORGANIZATION )));
    aPagesFT.SetText(String(              BibResId( ST_PAGE         )));
    aPublisherFT.SetText(String(          BibResId( ST_PUBLISHER     )));
    aAddressFT.SetText(String(            BibResId( ST_ADDRESS       )));
    aSchoolFT.SetText(String(             BibResId( ST_SCHOOL        )));
    aSeriesFT.SetText(String(             BibResId( ST_SERIES        )));
    aReportTypeFT.SetText(String(         BibResId( ST_REPORT    )));
    aVolumeFT.SetText(String(             BibResId( ST_VOLUME        )));
    aURLFT.SetText(String(                BibResId( ST_URL           )));
    aCustom1FT.SetText(String(            BibResId( ST_CUSTOM1       )));
    aCustom2FT.SetText(String(            BibResId( ST_CUSTOM2       )));
    aCustom3FT.SetText(String(            BibResId( ST_CUSTOM3       )));
    aCustom4FT.SetText(String(            BibResId( ST_CUSTOM4       )));
    aCustom5FT.SetText(String(            BibResId( ST_CUSTOM5       )));

    aOKBT.SetClickHdl(LINK(this, MappingDialog_Impl, OkHdl));
    String sTitle = GetText();
    sTitle.SearchAndReplace(rtl::OUString("%1"), pDatMan->getActiveDataTable(), 0);
    SetText(sTitle);

    aListBoxes[0] = &aIdentifierLB;
    aListBoxes[1] = &aAuthorityTypeLB;
    aListBoxes[2] = &aAuthorLB;
    aListBoxes[3] = &aTitleLB;
    aListBoxes[4] = &aYearLB;
    aListBoxes[5] = &aISBNLB;
    aListBoxes[6] = &aBooktitleLB;
    aListBoxes[7] = &aChapterLB;
    aListBoxes[8] = &aEditionLB;
    aListBoxes[9] = &aEditorLB;
    aListBoxes[10] = &aHowpublishedLB;
    aListBoxes[11] = &aInstitutionLB;
    aListBoxes[12] = &aJournalLB;
    aListBoxes[13] = &aMonthLB;
    aListBoxes[14] = &aNoteLB;
    aListBoxes[15] = &aAnnoteLB;
    aListBoxes[16] = &aNumberLB;
    aListBoxes[17] = &aOrganizationsLB;
    aListBoxes[18] = &aPagesLB;
    aListBoxes[19] = &aPublisherLB;
    aListBoxes[20] = &aAddressLB;
    aListBoxes[21] = &aSchoolLB;
    aListBoxes[22] = &aSeriesLB;
    aListBoxes[23] = &aReportTypeLB;
    aListBoxes[24] = &aVolumeLB;
    aListBoxes[25] = &aURLLB;
    aListBoxes[26] = &aCustom1LB;
    aListBoxes[27] = &aCustom2LB;
    aListBoxes[28] = &aCustom3LB;
    aListBoxes[29] = &aCustom4LB;
    aListBoxes[30] = &aCustom5LB;

    aListBoxes[0]->InsertEntry(sNone);
    Reference< XNameAccess >  xFields = getColumns( pDatMan->getForm() );
    DBG_ASSERT(xFields.is(), "MappingDialog_Impl::MappingDialog_Impl : gave me an invalid form !");
    if(xFields.is())
    {
        Sequence< ::rtl::OUString > aNames = xFields->getElementNames();
        sal_Int32 nFieldsCount = aNames.getLength();
        const ::rtl::OUString* pNames = aNames.getConstArray();

        for(sal_Int32 nField = 0; nField < nFieldsCount; nField++)
            aListBoxes[0]->InsertEntry(pNames[nField]);
    }

    Link aLnk = LINK(this, MappingDialog_Impl, ListBoxSelectHdl);

    aListBoxes[0]->SelectEntryPos(0);
    aListBoxes[0]->SetSelectHdl(aLnk);
    for(sal_uInt16 i = 1; i < COLUMN_COUNT; i++)
    {
        for(sal_uInt16 j = 0; j < aListBoxes[0]->GetEntryCount();j++)
            aListBoxes[i]->InsertEntry(aListBoxes[0]->GetEntry(j));
        aListBoxes[i]->SelectEntryPos(0);
        aListBoxes[i]->SetSelectHdl(aLnk);
    }
    BibConfig* pConfig = BibModul::GetConfig();
    BibDBDescriptor aDesc;
    aDesc.sDataSource = pDatMan->getActiveDataSource();
    aDesc.sTableOrQuery = pDatMan->getActiveDataTable();
    aDesc.nCommandType = CommandType::TABLE;
    const Mapping* pMapping = pConfig->GetMapping(aDesc);
    if(pMapping)
    {
        for(sal_uInt16 nEntry = 0; nEntry < COLUMN_COUNT; nEntry++)
        {
            sal_uInt16 nListBoxIndex = lcl_FindLogicalName( pConfig, pMapping->aColumnPairs[nEntry].sLogicalColumnName);
            if(nListBoxIndex < COLUMN_COUNT)
            {
                aListBoxes[nListBoxIndex]->SelectEntry(pMapping->aColumnPairs[nEntry].sRealColumnName);
            }
        }
    }
}

MappingDialog_Impl::~MappingDialog_Impl()
{}

IMPL_LINK(MappingDialog_Impl, ListBoxSelectHdl, ListBox*, pListBox)
{
    sal_uInt16 nEntryPos = pListBox->GetSelectEntryPos();
    if(0 < nEntryPos)
    {
        for(sal_uInt16 i = 0; i < COLUMN_COUNT; i++)
        {
            if(pListBox != aListBoxes[i] && aListBoxes[i]->GetSelectEntryPos() == nEntryPos)
                aListBoxes[i]->SelectEntryPos(0);
        }
    }
    SetModified();
    return 0;
}

IMPL_LINK_NOARG(MappingDialog_Impl, OkHdl)
{
    if(bModified)
    {
        Mapping aNew;
        aNew.sTableName = String(pDatMan->getActiveDataTable());
        aNew.sURL = String(pDatMan->getActiveDataSource());

        sal_uInt16 nWriteIndex = 0;
        BibConfig* pConfig = BibModul::GetConfig();
        for(sal_uInt16 nEntry = 0; nEntry < COLUMN_COUNT; nEntry++)
        {
            String sSel = aListBoxes[nEntry]->GetSelectEntry();
            if(sSel != sNone)
            {
                aNew.aColumnPairs[nWriteIndex].sRealColumnName = sSel;
                aNew.aColumnPairs[nWriteIndex].sLogicalColumnName = pConfig->GetDefColumnName(nEntry);
                nWriteIndex++;
            }
        }
        BibDBDescriptor aDesc;
        aDesc.sDataSource = pDatMan->getActiveDataSource();
        aDesc.sTableOrQuery = pDatMan->getActiveDataTable();
        aDesc.nCommandType = CommandType::TABLE;
        pDatMan->ResetIdentifierMapping();
        pConfig->SetMapping(aDesc, &aNew);
    }
    EndDialog(bModified ? RET_OK : RET_CANCEL);
    return 0;
}

class DBChangeDialog_Impl : public ModalDialog
{
    OKButton        aOKBT;
    CancelButton    aCancelBT;
    HelpButton      aHelpBT;
    FixedLine       aSelectionGB;
    SvTabListBox    aSelectionLB;
    HeaderBar       aSelectionHB;
    DBChangeDialogConfig_Impl   aConfig;
    String          aEntryST;
    String          aURLST;

    BibDataManager* pDatMan;

    DECL_LINK(DoubleClickHdl, SvTabListBox*);
public:
    DBChangeDialog_Impl(Window* pParent, BibDataManager* pMan );
    ~DBChangeDialog_Impl();

    String      GetCurrentURL()const;
};

DBChangeDialog_Impl::DBChangeDialog_Impl(Window* pParent, BibDataManager* pMan ) :
    ModalDialog(pParent, BibResId(RID_DLG_DBCHANGE) ),
    aOKBT(this,         BibResId( BT_OK     )),
    aCancelBT(this,     BibResId( BT_CANCEL )),
    aHelpBT(this,       BibResId( BT_HELP       )),
    aSelectionGB(this,  BibResId( GB_SELECTION )),
    aSelectionLB(this,  BibResId( LB_SELECTION )),
    aSelectionHB(this,  BibResId( HB_SELECTION )),
    aEntryST(BibResId(ST_ENTRY)),
    aURLST( BibResId(ST_URL)),
    pDatMan(pMan)
{
    FreeResource();
    aSelectionLB.SetDoubleClickHdl( LINK(this, DBChangeDialog_Impl, DoubleClickHdl));
    try
    {
        Reference< XMultiServiceFactory >  xMgr = comphelper::getProcessServiceFactory();

        ::Size aSize = aSelectionHB.GetSizePixel();
        long nTabs[2];
        nTabs[0] = 1;// Number of Tabs
        nTabs[1] = aSize.Width() / 4;

        aSelectionHB.SetStyle(aSelectionHB.GetStyle()|WB_STDHEADERBAR);
        aSelectionHB.InsertItem( 1, aEntryST, aSize.Width());
        aSelectionHB.SetSizePixel(aSelectionHB.CalcWindowSizePixel());
        aSelectionHB.Show();

        aSelectionLB.SetTabs( &nTabs[0], MAP_PIXEL );
        aSelectionLB.SetStyle(aSelectionLB.GetStyle()|WB_CLIPCHILDREN|WB_SORT);
        aSelectionLB.GetModel()->SetSortMode(SortAscending);

        ::rtl::OUString sActiveSource = pDatMan->getActiveDataSource();
        const Sequence< ::rtl::OUString >& rSources = aConfig.GetDataSourceNames();
        const ::rtl::OUString* pSourceNames = rSources.getConstArray();
        for(int i = 0; i < rSources.getLength(); i++)
        {
            SvTreeListEntry* pEntry = aSelectionLB.InsertEntry(pSourceNames[i]);
            if(pSourceNames[i] == sActiveSource)
            {
                aSelectionLB.Select(pEntry);
            }
        }
        aSelectionLB.GetModel()->Resort();
    }
    catch (const Exception&)
    {
        OSL_FAIL("Exception in BibDataManager::DBChangeDialog_Impl::DBChangeDialog_Impl");
    }
}

IMPL_LINK(DBChangeDialog_Impl, DoubleClickHdl, SvTabListBox*, /*pLB*/)
{
    EndDialog(RET_OK);
    return 0;
}

DBChangeDialog_Impl::~DBChangeDialog_Impl()
{
}

String  DBChangeDialog_Impl::GetCurrentURL()const
{
    String sRet;
    SvTreeListEntry* pEntry = aSelectionLB.FirstSelected();
    if(pEntry)
    {
        sRet = aSelectionLB.GetEntryText(pEntry, 0);
    }
    return sRet;
}

// XDispatchProvider
BibInterceptorHelper::BibInterceptorHelper( ::bib::BibBeamer* pBibBeamer, ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > xDispatch)
{
    if( pBibBeamer )
    {
        xInterception = pBibBeamer->getDispatchProviderInterception();
        if( xInterception.is() )
            xInterception->registerDispatchProviderInterceptor( this );
    }
    if( xDispatch.is() )
        xFormDispatch = xDispatch;
}

BibInterceptorHelper::~BibInterceptorHelper( )
{
}

void BibInterceptorHelper::ReleaseInterceptor()
{
    if ( xInterception.is() )
        xInterception->releaseDispatchProviderInterceptor( this );
    xInterception.clear();
}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL
    BibInterceptorHelper::queryDispatch( const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XDispatch > xReturn;

    String aCommand( aURL.Path );
    if ( aCommand.EqualsAscii("FormSlots/ConfirmDeletion") )
        xReturn = xFormDispatch;
    else
        if ( xSlaveDispatchProvider.is() )
            xReturn = xSlaveDispatchProvider->queryDispatch( aURL, aTargetFrameName, nSearchFlags);

    return xReturn;
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL
    BibInterceptorHelper::queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts ) throw (::com::sun::star::uno::RuntimeException)
{
    Sequence< Reference< XDispatch> > aReturn( aDescripts.getLength() );
    Reference< XDispatch >* pReturn = aReturn.getArray();
    const DispatchDescriptor* pDescripts = aDescripts.getConstArray();
    for ( sal_Int16 i=0; i<aDescripts.getLength(); ++i, ++pReturn, ++pDescripts )
    {
        *pReturn = queryDispatch( pDescripts->FeatureURL, pDescripts->FrameName, pDescripts->SearchFlags );
    }
    return aReturn;
}

// XDispatchProviderInterceptor
::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL
    BibInterceptorHelper::getSlaveDispatchProvider(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return xSlaveDispatchProvider;
}

void SAL_CALL BibInterceptorHelper::setSlaveDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewSlaveDispatchProvider ) throw (::com::sun::star::uno::RuntimeException)
{
    xSlaveDispatchProvider = xNewSlaveDispatchProvider;
}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL
    BibInterceptorHelper::getMasterDispatchProvider(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return xMasterDispatchProvider;
}

void SAL_CALL BibInterceptorHelper::setMasterDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewMasterDispatchProvider ) throw (::com::sun::star::uno::RuntimeException)
{
    xMasterDispatchProvider = xNewMasterDispatchProvider;
}

//-----------------------------------------------------------------------------
#define STR_UID "uid"
::rtl::OUString gGridName(C2U("theGrid"));
::rtl::OUString gViewName(C2U("theView"));
::rtl::OUString gGlobalName(C2U("theGlobals"));
::rtl::OUString gBeamerSize(C2U("theBeamerSize"));
::rtl::OUString gViewSize(C2U("theViewSize"));

BibDataManager::BibDataManager()
    :BibDataManager_Base( GetMutex() )
    ,m_pInterceptorHelper( NULL )
    ,m_aLoadListeners(m_aMutex)
    ,pBibView( NULL )
    ,pToolbar(0)
{
}


BibDataManager::~BibDataManager()
{
    Reference< XLoadable >      xLoad( m_xForm, UNO_QUERY );
    Reference< XPropertySet >   xPrSet( m_xForm, UNO_QUERY );
    Reference< XComponent >     xComp( m_xForm, UNO_QUERY );
    if ( m_xForm.is() )
    {
        Reference< XComponent >  xConnection;
        xPrSet->getPropertyValue(C2U("ActiveConnection")) >>= xConnection;
        RemoveMeAsUidListener();
        if (xLoad.is())
            xLoad->unload();
        if (xComp.is())
            xComp->dispose();
        if(xConnection.is())
            xConnection->dispose();
        m_xForm = NULL;
    }
    if( m_pInterceptorHelper )
    {
        m_pInterceptorHelper->ReleaseInterceptor();
        m_pInterceptorHelper->release();
        m_pInterceptorHelper = NULL;
    }
}
//------------------------------------------------------------------------
void BibDataManager::InsertFields(const Reference< XFormComponent > & _rxGrid)
{
    if ( !_rxGrid.is() )
        return;

    try
    {
        Reference< XNameContainer > xColContainer( _rxGrid, UNO_QUERY );
        // remove the old fields
        if ( xColContainer->hasElements() )
        {
            Sequence< ::rtl::OUString > aNames = xColContainer->getElementNames();
            const ::rtl::OUString* pNames = aNames.getConstArray();
            const ::rtl::OUString* pNamesEnd = pNames + aNames.getLength();
            for ( ; pNames != pNamesEnd; ++pNames )
                xColContainer->removeByName( *pNames );
        }

        Reference< XNameAccess >  xFields = getColumns( m_xForm );
        if (!xFields.is())
            return;

        Reference< XGridColumnFactory > xColFactory( _rxGrid, UNO_QUERY );

        Reference< XPropertySet >  xField;

        Sequence< ::rtl::OUString > aFields( xFields->getElementNames() );
        const ::rtl::OUString* pFields = aFields.getConstArray();
        const ::rtl::OUString* pFieldsEnd = pFields + aFields.getLength();

        for ( ; pFields != pFieldsEnd; ++pFields )
        {
            xFields->getByName( *pFields ) >>= xField;

            ::rtl::OUString sCurrentModelType;
            const ::rtl::OUString sType(C2U("Type"));
            sal_Int32 nType = 0;
            sal_Bool bIsFormatted           = sal_False;
            sal_Bool bFormattedIsNumeric    = sal_True;
            xField->getPropertyValue(sType) >>= nType;
            switch(nType)
            {
                case DataType::BIT:
                case DataType::BOOLEAN:
                    sCurrentModelType = C2U("CheckBox");
                    break;

                case DataType::BINARY:
                case DataType::VARBINARY:
                case DataType::LONGVARBINARY:
                case DataType::BLOB:
                    sCurrentModelType = C2U("TextField");
                    break;

                case DataType::VARCHAR:
                case DataType::LONGVARCHAR:
                case DataType::CHAR:
                case DataType::CLOB:
                    bFormattedIsNumeric = sal_False;
                    // _NO_ break !
                default:
                    sCurrentModelType = C2U("FormattedField");
                    bIsFormatted = sal_True;
                    break;
            }

            Reference< XPropertySet >  xCurrentCol = xColFactory->createColumn(sCurrentModelType);
            if (bIsFormatted)
            {
                ::rtl::OUString sFormatKey(C2U("FormatKey"));
                xCurrentCol->setPropertyValue(sFormatKey, xField->getPropertyValue(sFormatKey));
                Any aFormatted(&bFormattedIsNumeric, ::getBooleanCppuType());
                xCurrentCol->setPropertyValue(C2U("TreatAsNumber"), aFormatted);
            }
            Any aColName = makeAny( *pFields );
            xCurrentCol->setPropertyValue(FM_PROP_CONTROLSOURCE,    aColName);
            xCurrentCol->setPropertyValue(FM_PROP_LABEL, aColName);

            xColContainer->insertByName( *pFields, makeAny( xCurrentCol ) );
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("Exception in BibDataManager::InsertFields");
    }
}

Reference< awt::XControlModel > BibDataManager::updateGridModel()
{
    return updateGridModel( m_xForm );
}

Reference< awt::XControlModel > BibDataManager::updateGridModel(const Reference< XForm > & xDbForm)
{
    try
    {
        Reference< XPropertySet >  aFormPropSet( xDbForm, UNO_QUERY );
        ::rtl::OUString sName;
        aFormPropSet->getPropertyValue(C2U("Command")) >>= sName;

        if ( !m_xGridModel.is() )
        {
            m_xGridModel = createGridModel( gGridName );

            Reference< XNameContainer >  xNameCont(xDbForm, UNO_QUERY);
            xNameCont->insertByName( sName, makeAny( m_xGridModel ) );
        }

        // insert the fields
        Reference< XFormComponent > xFormComp( m_xGridModel, UNO_QUERY );
        InsertFields( xFormComp );
    }
    catch (const Exception&)
    {
        OSL_FAIL("::updateGridModel: something went wrong !");
    }

    return m_xGridModel;
}

Reference< XForm >  BibDataManager::createDatabaseForm(BibDBDescriptor& rDesc)
{
    Reference< XForm >  xResult;
    try
    {
        Reference< XMultiServiceFactory >  xMgr = comphelper::getProcessServiceFactory();
        m_xForm = Reference< XForm > ( xMgr->createInstance( C2U("com.sun.star.form.component.Form") ), UNO_QUERY );

        Reference< XPropertySet >  aPropertySet( m_xForm, UNO_QUERY );

        aDataSourceURL = rDesc.sDataSource;
        if(aPropertySet.is())
        {
            Any aVal;
            aVal <<= (sal_Int32)ResultSetType::SCROLL_INSENSITIVE;
            aPropertySet->setPropertyValue(C2U("ResultSetType"),aVal );
            aVal <<= (sal_Int32)ResultSetConcurrency::READ_ONLY;
            aPropertySet->setPropertyValue(C2U("ResultSetConcurrency"), aVal);

            //Caching for Performance
            aVal <<= (sal_Int32)50;
            aPropertySet->setPropertyValue(C2U("FetchSize"), aVal);

            Reference< XConnection >    xConnection = getConnection(rDesc.sDataSource);
            aVal <<= xConnection;
            aPropertySet->setPropertyValue(C2U("ActiveConnection"), aVal);

            Reference< XTablesSupplier >  xSupplyTables(xConnection, UNO_QUERY);
            Reference< XNameAccess >  xTables = xSupplyTables.is() ?
                                xSupplyTables->getTables() : Reference< XNameAccess > ();

            Sequence< ::rtl::OUString > aTableNameSeq;
            if (xTables.is())
                aTableNameSeq = xTables->getElementNames();

            if(aTableNameSeq.getLength() > 0)
            {
                const ::rtl::OUString* pTableNames = aTableNameSeq.getConstArray();
                if(!rDesc.sTableOrQuery.isEmpty())
                    aActiveDataTable = rDesc.sTableOrQuery;
                else
                {
                    rDesc.sTableOrQuery = aActiveDataTable = pTableNames[0];
                    rDesc.nCommandType = CommandType::TABLE;
                }

                aVal <<= aActiveDataTable;
                aPropertySet->setPropertyValue(C2U("Command"), aVal);
                aVal <<= rDesc.nCommandType;
                aPropertySet->setPropertyValue(C2U("CommandType"), aVal);


                Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
                aQuoteChar = xMetaData->getIdentifierQuoteString();

                Reference< XMultiServiceFactory > xFactory(xConnection, UNO_QUERY);
                if ( xFactory.is() )
                    m_xParser.set( xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.SingleSelectQueryComposer" ) ) ), UNO_QUERY );

                ::rtl::OUString aString(C2U("SELECT * FROM "));

                ::rtl::OUString sCatalog, sSchema, sName;
                ::dbtools::qualifiedNameComponents( xMetaData, aActiveDataTable, sCatalog, sSchema, sName, ::dbtools::eInDataManipulation );
                aString += ::dbtools::composeTableNameForSelect( xConnection, sCatalog, sSchema, sName );

                m_xParser->setElementaryQuery(aString);
                BibConfig* pConfig = BibModul::GetConfig();
                pConfig->setQueryField(getQueryField());
                startQueryWith(pConfig->getQueryText());

                xResult = m_xForm;
            }
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("::createDatabaseForm: something went wrong !");
    }

    return xResult;
}
//------------------------------------------------------------------------
Sequence< ::rtl::OUString > BibDataManager::getDataSources()
{
    Sequence< ::rtl::OUString > aTableNameSeq;

    try
    {
        Reference< XTablesSupplier >  xSupplyTables( getConnection( m_xForm ), UNO_QUERY );
        Reference< XNameAccess >  xTables;
        if (xSupplyTables.is())
            xTables = xSupplyTables->getTables();
        if (xTables.is())
            aTableNameSeq = xTables->getElementNames();
    }
    catch (const Exception&)
    {
        OSL_FAIL("::getDataSources: something went wrong !");
    }

    return aTableNameSeq;
}
//------------------------------------------------------------------------
::rtl::OUString BibDataManager::getActiveDataTable()
{
    return aActiveDataTable;
}
//------------------------------------------------------------------------
void BibDataManager::setFilter(const ::rtl::OUString& rQuery)
{
    if(!m_xParser.is())
        return;
    try
    {
        m_xParser->setFilter( rQuery );
        ::rtl::OUString aQuery = m_xParser->getFilter();
        Reference< XPropertySet >  xFormProps( m_xForm, UNO_QUERY_THROW );
        xFormProps->setPropertyValue( C2U( "Filter" ), makeAny( aQuery ) );
        xFormProps->setPropertyValue( C2U( "ApplyFilter" ), makeAny( sal_True ) );
        reload();
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }


}
//------------------------------------------------------------------------
::rtl::OUString BibDataManager::getFilter()
{

    ::rtl::OUString aQueryString;
    try
    {
        Reference< XPropertySet > xFormProps( m_xForm, UNO_QUERY_THROW );
        OSL_VERIFY( xFormProps->getPropertyValue( C2U( "Filter" ) ) >>= aQueryString );
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }


    return aQueryString;

}
//------------------------------------------------------------------------
Sequence< ::rtl::OUString > BibDataManager::getQueryFields()
{
    Sequence< ::rtl::OUString > aFieldSeq;
    Reference< XNameAccess >  xFields = getColumns( m_xForm );
    if (xFields.is())
        aFieldSeq = xFields->getElementNames();
    return aFieldSeq;
}
//------------------------------------------------------------------------
::rtl::OUString BibDataManager::getQueryField()
{
    BibConfig* pConfig = BibModul::GetConfig();
    ::rtl::OUString aFieldString = pConfig->getQueryField();
    if(aFieldString.isEmpty())
    {
        Sequence< ::rtl::OUString > aSeq = getQueryFields();
        const ::rtl::OUString* pFields = aSeq.getConstArray();
        if(aSeq.getLength()>0)
        {
            aFieldString=pFields[0];
        }
    }
    return aFieldString;
}
//------------------------------------------------------------------------
void BibDataManager::startQueryWith(const ::rtl::OUString& rQuery)
{
    BibConfig* pConfig = BibModul::GetConfig();
    pConfig->setQueryText( rQuery );

    ::rtl::OUString aQueryString;
    if(!rQuery.isEmpty())
    {
        aQueryString=aQuoteChar;
        aQueryString+=getQueryField();
        aQueryString+=aQuoteChar;
        aQueryString+=C2U(" like '");
        String sQuery(rQuery);
        sQuery.SearchAndReplaceAll('?','_');
        sQuery.SearchAndReplaceAll('*','%');
        aQueryString += sQuery;
        aQueryString+=C2U("%'");
    }
    setFilter(aQueryString);
}

void BibDataManager::setActiveDataSource(const ::rtl::OUString& rURL)
{
    ::rtl::OUString sTmp(aDataSourceURL);
    aDataSourceURL = rURL;

    Reference< XPropertySet >  aPropertySet( m_xForm, UNO_QUERY );
    if(aPropertySet.is())
    {
        unload();

        Reference< XComponent >  xOldConnection;
        aPropertySet->getPropertyValue(C2U("ActiveConnection")) >>= xOldConnection;

        Reference< XConnection >    xConnection = getConnection(rURL);
        if(!xConnection.is())
        {
            aDataSourceURL = sTmp;
            return;
        }
        Any aVal; aVal <<= xConnection;
        aPropertySet->setPropertyValue(C2U("ActiveConnection"), aVal);
        Reference< XMultiServiceFactory >   xFactory(xConnection, UNO_QUERY);
        if ( xFactory.is() )
            m_xParser.set( xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.SingleSelectQueryComposer" ) ) ), UNO_QUERY );

        if(xOldConnection.is())
            xOldConnection->dispose();

        Sequence< ::rtl::OUString > aTableNameSeq;
        Reference< XTablesSupplier >  xSupplyTables(xConnection, UNO_QUERY);
        if(xSupplyTables.is())
        {
            Reference< XNameAccess >  xAccess = xSupplyTables->getTables();
            aTableNameSeq = xAccess->getElementNames();
        }
        if(aTableNameSeq.getLength() > 0)
        {
            const ::rtl::OUString* pTableNames = aTableNameSeq.getConstArray();
            aActiveDataTable = pTableNames[0];
            aVal <<= aActiveDataTable;
            aPropertySet->setPropertyValue(C2U("Command"), aVal);
            aPropertySet->setPropertyValue(C2U("CommandType"), makeAny(CommandType::TABLE));
            //Caching for Performance
            aVal <<= (sal_Int32)50;
            aPropertySet->setPropertyValue(C2U("FetchSize"), aVal);
            ::rtl::OUString aString(C2U("SELECT * FROM "));
            // quote the table name which may contain catalog.schema.table
            Reference<XDatabaseMetaData> xMetaData(xConnection->getMetaData(),UNO_QUERY);
            aQuoteChar = xMetaData->getIdentifierQuoteString();

            ::rtl::OUString sCatalog, sSchema, sName;
            ::dbtools::qualifiedNameComponents( xMetaData, aActiveDataTable, sCatalog, sSchema, sName, ::dbtools::eInDataManipulation );
            aString += ::dbtools::composeTableNameForSelect( xConnection, sCatalog, sSchema, sName );

            m_xParser->setElementaryQuery(aString);
            BibConfig* pConfig = BibModul::GetConfig();
            pConfig->setQueryField(getQueryField());
            startQueryWith(pConfig->getQueryText());
            setActiveDataTable(aActiveDataTable);
        }
        FeatureStateEvent aEvent;
        util::URL aURL;
        aEvent.IsEnabled  = sal_True;
        aEvent.Requery    = sal_False;
        aEvent.FeatureDescriptor = getActiveDataTable();

        aEvent.State = makeAny( getDataSources() );

        if(pToolbar)
        {
            aURL.Complete =C2U(".uno:Bib/source");
            aEvent.FeatureURL = aURL;
            pToolbar->statusChanged( aEvent );
        }

        updateGridModel();
        load();
    }
}


void BibDataManager::setActiveDataTable(const ::rtl::OUString& rTable)
{
    ResetIdentifierMapping();
    try
    {
        Reference< XPropertySet >  aPropertySet( m_xForm, UNO_QUERY );

        if(aPropertySet.is())
        {
            Reference< XConnection >    xConnection = getConnection( m_xForm );
            Reference< XTablesSupplier >  xSupplyTables(xConnection, UNO_QUERY);
            Reference< XNameAccess > xAccess = xSupplyTables->getTables();
            Sequence< ::rtl::OUString > aTableNameSeq = xAccess->getElementNames();
            sal_uInt32 nCount = aTableNameSeq.getLength();

            const ::rtl::OUString* pTableNames = aTableNameSeq.getConstArray();
            const ::rtl::OUString* pTableNamesEnd = pTableNames + nCount;

            for ( ; pTableNames != pTableNamesEnd; ++pTableNames )
            {
                if ( rTable == *pTableNames )
                {
                    aActiveDataTable = rTable;
                    Any aVal; aVal <<= rTable;
                    aPropertySet->setPropertyValue( C2U("Command"), aVal );
                    break;
                }
            }
            if (pTableNames != pTableNamesEnd)
            {
                Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
                aQuoteChar = xMetaData->getIdentifierQuoteString();

                Reference< XMultiServiceFactory > xFactory(xConnection, UNO_QUERY);
                if ( xFactory.is() )
                    m_xParser.set( xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.SingleSelectQueryComposer" ) ) ), UNO_QUERY );

                ::rtl::OUString aString(C2U("SELECT * FROM "));

                ::rtl::OUString sCatalog, sSchema, sName;
                ::dbtools::qualifiedNameComponents( xMetaData, aActiveDataTable, sCatalog, sSchema, sName, ::dbtools::eInDataManipulation );
                aString += ::dbtools::composeTableNameForSelect( xConnection, sCatalog, sSchema, sName );

                m_xParser->setElementaryQuery(aString);

                BibConfig* pConfig = BibModul::GetConfig();
                pConfig->setQueryField(getQueryField());
                startQueryWith(pConfig->getQueryText());

                BibDBDescriptor aDesc;
                aDesc.sDataSource = aDataSourceURL;
                aDesc.sTableOrQuery = aActiveDataTable;
                aDesc.nCommandType = CommandType::TABLE;
                BibModul::GetConfig()->SetBibliographyURL(aDesc);
            }
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("::setActiveDataTable: something went wrong !");
    }
}

//------------------------------------------------------------------------
void SAL_CALL BibDataManager::load(  ) throw (RuntimeException)
{
    if ( isLoaded() )
        // nothing to do
        return;

    Reference< XLoadable > xFormAsLoadable( m_xForm, UNO_QUERY );
    DBG_ASSERT( xFormAsLoadable.is() || !m_xForm.is(), "BibDataManager::load: invalid form!");
    if ( xFormAsLoadable.is() )
    {
        xFormAsLoadable->load();
        SetMeAsUidListener();

        EventObject aEvt( static_cast< XWeak* >( this ) );
        m_aLoadListeners.notifyEach( &XLoadListener::loaded, aEvt );
    }
}

//------------------------------------------------------------------------
void SAL_CALL BibDataManager::unload(  ) throw (RuntimeException)
{
    if ( !isLoaded() )
        // nothing to do
        return;

    Reference< XLoadable >xFormAsLoadable( m_xForm, UNO_QUERY );
    DBG_ASSERT( xFormAsLoadable.is() || !m_xForm.is(), "BibDataManager::unload: invalid form!");
    if ( xFormAsLoadable.is() )
    {
        EventObject aEvt( static_cast< XWeak* >( this ) );

        {
            m_aLoadListeners.notifyEach( &XLoadListener::unloading, aEvt );
        }

        RemoveMeAsUidListener();
        xFormAsLoadable->unload();

        {
            m_aLoadListeners.notifyEach( &XLoadListener::unloaded, aEvt );
        }
    }
}

//------------------------------------------------------------------------
void SAL_CALL BibDataManager::reload(  ) throw (RuntimeException)
{
    if ( !isLoaded() )
        // nothing to do
        return;

    Reference< XLoadable >xFormAsLoadable( m_xForm, UNO_QUERY );
    DBG_ASSERT( xFormAsLoadable.is() || !m_xForm.is(), "BibDataManager::unload: invalid form!");
    if ( xFormAsLoadable.is() )
    {
        EventObject aEvt( static_cast< XWeak* >( this ) );

        {
            m_aLoadListeners.notifyEach( &XLoadListener::reloading, aEvt );
        }

        xFormAsLoadable->reload();

        {
            m_aLoadListeners.notifyEach( &XLoadListener::reloaded, aEvt );
        }
    }
}

//------------------------------------------------------------------------
sal_Bool SAL_CALL BibDataManager::isLoaded(  ) throw (RuntimeException)
{
    Reference< XLoadable >xFormAsLoadable( m_xForm, UNO_QUERY );
    DBG_ASSERT( xFormAsLoadable.is() || !m_xForm.is(), "BibDataManager::isLoaded: invalid form!");

    sal_Bool bLoaded = sal_False;
    if ( xFormAsLoadable.is() )
        bLoaded = xFormAsLoadable->isLoaded();
    return bLoaded;
}

//------------------------------------------------------------------------
void SAL_CALL BibDataManager::addLoadListener( const Reference< XLoadListener >& aListener ) throw (RuntimeException)
{
    m_aLoadListeners.addInterface( aListener );
}

//------------------------------------------------------------------------
void SAL_CALL BibDataManager::removeLoadListener( const Reference< XLoadListener >& aListener ) throw (RuntimeException)
{
    m_aLoadListeners.removeInterface( aListener );
}

//------------------------------------------------------------------------
Reference< awt::XControlModel > BibDataManager::createGridModel(const ::rtl::OUString& rName)
{
    Reference< awt::XControlModel > xModel;

    try
    {
        // create the control model
        Reference< XMultiServiceFactory >  xMgr = ::comphelper::getProcessServiceFactory();
        Reference< XInterface >  xObject = xMgr->createInstance(C2U("com.sun.star.form.component.GridControl"));
        xModel=Reference< awt::XControlModel > ( xObject, UNO_QUERY );

        // set the
        Reference< XPropertySet > xPropSet( xModel, UNO_QUERY );
        xPropSet->setPropertyValue( C2U("Name"), makeAny( rName ) );

        // set the name of the to-be-created control
        ::rtl::OUString aControlName(C2U("com.sun.star.form.control.InteractionGridControl"));
        Any aAny; aAny <<= aControlName;
        xPropSet->setPropertyValue( C2U("DefaultControl"),aAny );

        // the the helpURL
        ::rtl::OUString uProp(C2U("HelpURL"));
        Reference< XPropertySetInfo > xPropInfo = xPropSet->getPropertySetInfo();
        if (xPropInfo->hasPropertyByName(uProp))
        {
            ::rtl::OUString sId(RTL_CONSTASCII_USTRINGPARAM( INET_HID_SCHEME ));
            sId += ::rtl::OUString::createFromAscii( HID_BIB_DB_GRIDCTRL );
            xPropSet->setPropertyValue( uProp, makeAny( sId ) );
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("::createGridModel: something went wrong !");
    }

    return xModel;
}
//------------------------------------------------------------------------
::rtl::OUString BibDataManager::getControlName(sal_Int32 nFormatKey )
{
    ::rtl::OUString aResStr;
    switch (nFormatKey)
    {
        case DataType::BIT:
        case DataType::BOOLEAN:
            aResStr=C2U("CheckBox");
            break;
        case DataType::TINYINT:
        case DataType::SMALLINT:
        case DataType::INTEGER:
            aResStr=C2U("NumericField");   ;
            break;
        case DataType::REAL:
        case DataType::DOUBLE:
        case DataType::NUMERIC:
        case DataType::DECIMAL:
            aResStr=C2U("FormattedField");
            break;
        case DataType::TIMESTAMP:
            aResStr=C2U("FormattedField");
            break;
        case DataType::DATE:
            aResStr=C2U("DateField");
            break;
        case DataType::TIME:
            aResStr=C2U("TimeField");
            break;
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::LONGVARCHAR:
        default:
            aResStr=C2U("TextField");
            break;
    }
    return aResStr;
}
//------------------------------------------------------------------------
Reference< awt::XControlModel > BibDataManager::loadControlModel(
                    const ::rtl::OUString& rName, sal_Bool bForceListBox)
{
    Reference< awt::XControlModel > xModel;
    ::rtl::OUString aName(C2U("View_"));
    aName += rName;

    try
    {
        Reference< XNameAccess >  xFields = getColumns( m_xForm );
        if (!xFields.is())
            return xModel;
        Reference< XPropertySet >  xField;

        Any aElement;

        if(xFields->hasByName(rName))
        {
            aElement = xFields->getByName(rName);
            aElement >>= xField;
            Reference< XPropertySetInfo >  xInfo = xField.is() ? xField->getPropertySetInfo() : Reference< XPropertySetInfo > ();

            ::rtl::OUString sCurrentModelType;
            const ::rtl::OUString sType(C2U("Type"));
            sal_Int32 nFormatKey = 0;
            xField->getPropertyValue(sType) >>= nFormatKey;

            ::rtl::OUString aInstanceName(C2U("com.sun.star.form.component."));

            if (bForceListBox)
                aInstanceName += C2U("ListBox");
            else
                aInstanceName += getControlName(nFormatKey);

            Reference< XMultiServiceFactory >  xMgr = comphelper::getProcessServiceFactory();
            Reference< XInterface >  xObject = xMgr->createInstance(aInstanceName);
            xModel=Reference< awt::XControlModel > ( xObject, UNO_QUERY );
            Reference< XPropertySet >  xPropSet( xModel, UNO_QUERY );
            Any aFieldName; aFieldName <<= aName;

            xPropSet->setPropertyValue( FM_PROP_NAME,aFieldName);
            xPropSet->setPropertyValue( FM_PROP_CONTROLSOURCE, makeAny( rName ) );
            xPropSet->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "NativeWidgetLook" ) ), makeAny( (sal_Bool)sal_True ) );

            Reference< XFormComponent >  aFormComp(xModel,UNO_QUERY );

            Reference< XNameContainer >  xNameCont( m_xForm, UNO_QUERY );
            xNameCont->insertByName(aName, makeAny( aFormComp ) );

            // now if the form where we inserted the new model is already loaded, notify the model of this
            // Note that this implementation below is a HACK as it relies on the fact that the model adds itself
            // as load listener to it's parent, which is an implementation detail of the model.
            //
            // the better solution would be the following:
            // in the current scenario, we insert a control model into a form. This results in the control model
            // adding itself as load listener to the form. Now, the form should realize that it's already loaded
            // and notify the model (which it knows as XLoadListener only) immediately. This seems to make sense.
            // (as an anologon to the XStatusListener semantics).
            //
            // But this would be way too risky for this last-day fix here.
            Reference< XLoadable > xLoad( m_xForm, UNO_QUERY );
            if ( xLoad.is() && xLoad->isLoaded() )
            {
                Reference< XLoadListener > xListener( aFormComp, UNO_QUERY );
                if ( xListener.is() )
                {
                    EventObject aLoadSource;
                    aLoadSource.Source = xLoad;
                    xListener->loaded( aLoadSource );
                }
            }
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("::loadControlModel: something went wrong !");
    }
    return xModel;
}
//------------------------------------------------------------------------
void SAL_CALL BibDataManager::disposing()
{
    BibDataManager_Base::WeakComponentImplHelperBase::disposing();
}

//------------------------------------------------------------------------
void BibDataManager::disposing( const EventObject& /*Source*/ ) throw( ::com::sun::star::uno::RuntimeException )
{
    // not interested in
}

//------------------------------------------------------------------------
void BibDataManager::propertyChange(const beans::PropertyChangeEvent& evt) throw( RuntimeException )
{
    try
    {
        if(evt.PropertyName == FM_PROP_VALUE)
        {
            if( evt.NewValue.getValueType() == ::getCppuType((Reference<io::XInputStream>*)0) )
            {
                Reference< io::XDataInputStream >  xStream(
                    *(const Reference< io::XInputStream > *)evt.NewValue.getValue(), UNO_QUERY );
                aUID <<= xStream->readUTF();
            }
            else
                aUID = evt.NewValue;

            Reference< XRowLocate > xLocate(xBibCursor, UNO_QUERY);
            DBG_ASSERT(xLocate.is(), "BibDataManager::propertyChange : invalid cursor !");
            xLocate->moveToBookmark(aUID);
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("::propertyChange: something went wrong !");
    }
}

//------------------------------------------------------------------------
void BibDataManager::SetMeAsUidListener()
{
    try
    {
        Reference< XNameAccess >  xFields = getColumns( m_xForm );
        if (!xFields.is())
            return;

        Sequence< ::rtl::OUString > aFields(xFields->getElementNames());
        const ::rtl::OUString* pFields = aFields.getConstArray();
        sal_Int32 nCount=aFields.getLength();
        rtl::OUString StrUID(STR_UID);
        ::rtl::OUString theFieldName;
        for( sal_Int32 i=0; i<nCount; i++ )
        {
            String aName= pFields[i];

            if(aName.EqualsIgnoreCaseAscii(StrUID))
            {
                theFieldName=pFields[i];
                break;
            }
        }

        if(!theFieldName.isEmpty())
        {
            Reference< XPropertySet >  xPropSet;
            Any aElement;

            aElement = xFields->getByName(theFieldName);
            xPropSet = *(Reference< XPropertySet > *)aElement.getValue();

            xPropSet->addPropertyChangeListener(FM_PROP_VALUE, this);
        }

    }
    catch (const Exception&)
    {
        OSL_FAIL("Exception in BibDataManager::SetMeAsUidListener");
    }
}

//------------------------------------------------------------------------
void BibDataManager::RemoveMeAsUidListener()
{
    try
    {
        Reference< XNameAccess >  xFields = getColumns( m_xForm );
        if (!xFields.is())
            return;


        Sequence< ::rtl::OUString > aFields(xFields->getElementNames());
        const ::rtl::OUString* pFields = aFields.getConstArray();
        sal_Int32 nCount=aFields.getLength();
        rtl::OUString StrUID(STR_UID);
        ::rtl::OUString theFieldName;
        for( sal_Int32 i=0; i<nCount; i++ )
        {
            String aName= pFields[i];

            if(aName.EqualsIgnoreCaseAscii(StrUID))
            {
                theFieldName=pFields[i];
                break;
            }
        }

        if(!theFieldName.isEmpty())
        {
            Reference< XPropertySet >  xPropSet;
            Any aElement;

            aElement = xFields->getByName(theFieldName);
            xPropSet = *(Reference< XPropertySet > *)aElement.getValue();

            xPropSet->removePropertyChangeListener(FM_PROP_VALUE, this);
        }

    }
    catch (const Exception&)
    {
        OSL_FAIL("Exception in BibDataManager::RemoveMeAsUidListener");
    }
}

void BibDataManager::CreateMappingDialog(Window* pParent)
{
    MappingDialog_Impl* pDlg = new MappingDialog_Impl(pParent, this);
    if(RET_OK == pDlg->Execute() && pBibView)
    {
        reload();
    }
    delete pDlg;
}

::rtl::OUString BibDataManager::CreateDBChangeDialog(Window* pParent)
{
    ::rtl::OUString uRet;
    DBChangeDialog_Impl * pDlg = new DBChangeDialog_Impl(pParent, this );
    if(RET_OK == pDlg->Execute())
    {
        String sNewURL = pDlg->GetCurrentURL();
        if(sNewURL != String(getActiveDataSource()))
        {
            uRet = sNewURL;
        }
    }
    delete pDlg;
    return uRet;
}

void BibDataManager::DispatchDBChangeDialog()
{
    if(pToolbar)
        pToolbar->SendDispatch(TBC_BT_CHANGESOURCE, Sequence< PropertyValue >());
}

const ::rtl::OUString& BibDataManager::GetIdentifierMapping()
{
    if(sIdentifierMapping.isEmpty())
    {
        BibConfig* pConfig = BibModul::GetConfig();
        BibDBDescriptor aDesc;
        aDesc.sDataSource = getActiveDataSource();
        aDesc.sTableOrQuery = getActiveDataTable();
        aDesc.nCommandType = CommandType::TABLE;
        const Mapping* pMapping = pConfig->GetMapping(aDesc);
        sIdentifierMapping = pConfig->GetDefColumnName(IDENTIFIER_POS);
        if(pMapping)
        {
            for(sal_uInt16 nEntry = 0; nEntry < COLUMN_COUNT; nEntry++)
            {
                if(pMapping->aColumnPairs[nEntry].sLogicalColumnName == sIdentifierMapping)
                {
                    sIdentifierMapping = pMapping->aColumnPairs[nEntry].sRealColumnName;
                    break;
                }
            }
        }
    }
    return sIdentifierMapping;
}

void BibDataManager::SetToolbar(BibToolBar* pSet)
{
    pToolbar = pSet;
    if(pToolbar)
        pToolbar->SetDatMan(*this);
}

uno::Reference< form::runtime::XFormController > BibDataManager::GetFormController()
{
    if(!m_xFormCtrl.is())
    {
        Reference< lang::XMultiServiceFactory > xMgr = comphelper::getProcessServiceFactory();
        m_xFormCtrl = uno::Reference< form::runtime::XFormController > (
            xMgr->createInstance(C2U("com.sun.star.form.runtime.FormController")), UNO_QUERY);
        m_xFormCtrl->setModel(uno::Reference< awt::XTabControllerModel > (getForm(), UNO_QUERY));
        m_xFormDispatch = uno::Reference< frame::XDispatch > ( m_xFormCtrl, UNO_QUERY);
    }
    return m_xFormCtrl;
}

void BibDataManager::RegisterInterceptor( ::bib::BibBeamer* pBibBeamer)
{
    DBG_ASSERT( !m_pInterceptorHelper, "BibDataManager::RegisterInterceptor: called twice!" );

    if( pBibBeamer )
        m_pInterceptorHelper = new BibInterceptorHelper( pBibBeamer, m_xFormDispatch);
    if( m_pInterceptorHelper )
        m_pInterceptorHelper->acquire();
}


sal_Bool BibDataManager::HasActiveConnection()const
{
    sal_Bool bRet = sal_False;
    Reference< XPropertySet >   xPrSet( m_xForm, UNO_QUERY );
    if( xPrSet.is() )
    {
        Reference< XComponent >  xConnection;
        xPrSet->getPropertyValue(C2U("ActiveConnection")) >>= xConnection;
        bRet = xConnection.is();
    }
    return bRet;
}

sal_Bool BibDataManager::HasActiveConnection()
{
    return getConnection( m_xForm ).is();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
