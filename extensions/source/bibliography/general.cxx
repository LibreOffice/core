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

#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <cppuhelper/implbase1.hxx>
#include <vcl/settings.hxx>
#include "general.hxx"
#include "bibresid.hxx"
#include "datman.hxx"
#include "bibconfig.hxx"
#include "bibprop.hrc"
#include "bib.hrc"
#include "bibmod.hxx"
#include "bibtools.hxx"
#include "bibliography.hrc"
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/mnemonic.hxx>
#include <algorithm>
#include <functional>
#include <vector>
#include <tools/urlobj.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::sdb;
using namespace ::rtl;

#define DISTANCE_CONTROL_TO_FIXEDTEXT 5

static ::Point lcl_MovePoint(const FixedText& rFixedText)
{
    ::Point aRet(rFixedText.GetPosPixel());
    aRet.X() += rFixedText.GetSizePixel().Width();
    aRet.X() += DISTANCE_CONTROL_TO_FIXEDTEXT;
    return aRet;
}

static OUString lcl_GetColumnName( const Mapping* pMapping, sal_uInt16 nIndexPos )
{
    BibConfig* pBibConfig = BibModul::GetConfig();
    OUString sRet = pBibConfig->GetDefColumnName(nIndexPos);
    if(pMapping)
        for(sal_uInt16 i = 0; i < COLUMN_COUNT; i++)
        {
            if(pMapping->aColumnPairs[i].sLogicalColumnName == sRet)
            {
                sRet = pMapping->aColumnPairs[i].sRealColumnName;
                break;
            }
        }
    return sRet;
}

class BibPosListener    :public cppu::WeakImplHelper1 <sdbc::XRowSetListener>
{
    BibGeneralPage*     pParentPage;
public:
    BibPosListener(BibGeneralPage* pParent);

    //XPositioningListener
    virtual void SAL_CALL cursorMoved(const lang::EventObject& event) throw( uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL rowChanged(const lang::EventObject& /*event*/) throw( uno::RuntimeException, std::exception ) SAL_OVERRIDE { /* not interested in */ }
    virtual void SAL_CALL rowSetChanged(const lang::EventObject& /*event*/) throw( uno::RuntimeException, std::exception ) SAL_OVERRIDE { /* not interested in */ }

    //XEventListener
    virtual void SAL_CALL disposing(const lang::EventObject& Source) throw( uno::RuntimeException, std::exception ) SAL_OVERRIDE;

};

BibPosListener::BibPosListener(BibGeneralPage* pParent) :
    pParentPage(pParent)
{
}

void BibPosListener::cursorMoved(const lang::EventObject& /*aEvent*/) throw( uno::RuntimeException, std::exception )
{
    try
    {
        uno::Reference< form::XBoundComponent >  xLstBox = pParentPage->GetTypeListBoxModel();
        uno::Reference< beans::XPropertySet >  xPropSet(xLstBox, UNO_QUERY);
        if(xPropSet.is())
        {
            BibConfig* pBibConfig = BibModul::GetConfig();
            BibDataManager* pDatMan = pParentPage->GetDataManager();
            BibDBDescriptor aDesc;
            aDesc.sDataSource = pDatMan->getActiveDataSource();
            aDesc.sTableOrQuery = pDatMan->getActiveDataTable();
            aDesc.nCommandType = CommandType::TABLE;

            const Mapping* pMapping = pBibConfig->GetMapping(aDesc);
            OUString sTypeMapping = pBibConfig->GetDefColumnName(AUTHORITYTYPE_POS);
            if(pMapping)
            {
                for(sal_uInt16 nEntry = 0; nEntry < COLUMN_COUNT; nEntry++)
                {
                    if(pMapping->aColumnPairs[nEntry].sLogicalColumnName == sTypeMapping)
                    {
                        sTypeMapping = pMapping->aColumnPairs[nEntry].sRealColumnName;
                        break;
                    }
                }
            }
            OUString uTypeMapping = sTypeMapping;

            uno::Reference< form::XForm >  xForm = pDatMan->getForm();
            uno::Reference< sdbcx::XColumnsSupplier >  xSupplyCols(xForm, UNO_QUERY);
            uno::Reference< container::XNameAccess >  xValueAcc;
            if (xSupplyCols.is())
                xValueAcc = xSupplyCols->getColumns();

            sal_Int16 nTempVal = -1;
            if(xValueAcc.is() && xValueAcc->hasByName(uTypeMapping))
            {
                uno::Any aVal = xValueAcc->getByName(uTypeMapping);
                uno::Reference< uno::XInterface >  xInt = *(uno::Reference< uno::XInterface > *)aVal.getValue();
                uno::Reference< sdb::XColumn >  xCol(xInt, UNO_QUERY);
                DBG_ASSERT(xCol.is(), "BibPosListener::cursorMoved : invalid column (no sdb::XColumn) !");
                if (xCol.is())
                {
                    nTempVal = xCol->getShort();
                    // getShort returns zero if the value is not a number
                    if (!nTempVal || xCol->wasNull())
                    {
                        OUString sTempVal = xCol->getString();
                        if(sTempVal != OUString('0'))
                            nTempVal = -1;
                    }
                }
            }
            if(nTempVal < 0 || nTempVal >= TYPE_COUNT)
            {
                uno::Any aSel;
                uno::Sequence<sal_Int16> aSelSeq(1);
                sal_Int16* pArr = aSelSeq.getArray();
                pArr[0] = TYPE_COUNT;
                aSel.setValue(&aSelSeq, ::getCppuType((Sequence<sal_Int16>*)0));
                xPropSet->setPropertyValue("SelectedItems", aSel);
            }
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("BibPosListener::cursorMoved: something went wrong !");
    }
}

void BibPosListener::disposing(const lang::EventObject& /*Source*/) throw( uno::RuntimeException, std::exception )
{
}

BibGeneralPage::BibGeneralPage(Window* pParent, BibDataManager* pMan):
    BibTabPage(pParent, "GeneralPage", "modules/sbibliography/ui/generalpage.ui"),
    pIdentifierFT(new FixedText(this)),
    pAuthTypeFT(new FixedText(this)),
    pYearFT(new FixedText(this)),
    pAuthorFT(new FixedText(this)),
    pTitleFT(new FixedText(this)),
    pPublisherFT(new FixedText(this)),
    pAddressFT(new FixedText(this)),
    pISBNFT(new FixedText(this)),
    pChapterFT(new FixedText(this)),
    pPagesFT(new FixedText(this)),
    pEditorFT(new FixedText(this)),
    pEditionFT(new FixedText(this)),
    pBooktitleFT(new FixedText(this)),
    pVolumeFT(new FixedText(this)),
    pHowpublishedFT(new FixedText(this)),
    pOrganizationsFT(new FixedText(this)),
    pInstitutionFT(new FixedText(this)),
    pSchoolFT(new FixedText(this)),
    pReportTypeFT(new FixedText(this)),
    pMonthFT(new FixedText(this)),
    pJournalFT(new FixedText(this)),
    pNumberFT(new FixedText(this)),
    pSeriesFT(new FixedText(this)),
    pAnnoteFT(new FixedText(this)),
    pNoteFT(new FixedText(this)),
    pURLFT(new FixedText(this)),
    pCustom1FT(new FixedText(this)),
    pCustom2FT(new FixedText(this)),
    pCustom3FT(new FixedText(this)),
    pCustom4FT(new FixedText(this)),
    pCustom5FT(new FixedText(this)),
    aHoriScroll(this, WB_HORZ),
    aVertScroll(this, WB_VERT),
    sErrorPrefix(BIB_RESSTR(ST_ERROR_PREFIX)),
    pDatMan(pMan)
{
    aStdSize = GetOutputSizePixel();

    aBibTypeArr[0] = BIB_RESSTR(ST_TYPE_ARTICLE);
    aBibTypeArr[1] = BIB_RESSTR(ST_TYPE_BOOK);
    aBibTypeArr[2] = BIB_RESSTR(ST_TYPE_BOOKLET);
    aBibTypeArr[3] = BIB_RESSTR(ST_TYPE_CONFERENCE);
    aBibTypeArr[4] = BIB_RESSTR(ST_TYPE_INBOOK );
    aBibTypeArr[5] = BIB_RESSTR(ST_TYPE_INCOLLECTION);
    aBibTypeArr[6] = BIB_RESSTR(ST_TYPE_INPROCEEDINGS);
    aBibTypeArr[7] = BIB_RESSTR(ST_TYPE_JOURNAL       );
    aBibTypeArr[8] = BIB_RESSTR(ST_TYPE_MANUAL    );
    aBibTypeArr[9] = BIB_RESSTR(ST_TYPE_MASTERSTHESIS);
    aBibTypeArr[10] = BIB_RESSTR(ST_TYPE_MISC      );
    aBibTypeArr[11] = BIB_RESSTR(ST_TYPE_PHDTHESIS );
    aBibTypeArr[12] = BIB_RESSTR(ST_TYPE_PROCEEDINGS   );
    aBibTypeArr[13] = BIB_RESSTR(ST_TYPE_TECHREPORT    );
    aBibTypeArr[14] = BIB_RESSTR(ST_TYPE_UNPUBLISHED   );
    aBibTypeArr[15] = BIB_RESSTR(ST_TYPE_EMAIL     );
    aBibTypeArr[16] = BIB_RESSTR(ST_TYPE_WWW           );
    aBibTypeArr[17] = BIB_RESSTR(ST_TYPE_CUSTOM1       );
    aBibTypeArr[18] = BIB_RESSTR(ST_TYPE_CUSTOM2       );
    aBibTypeArr[19] = BIB_RESSTR(ST_TYPE_CUSTOM3       );
    aBibTypeArr[20] = BIB_RESSTR(ST_TYPE_CUSTOM4       );
    aBibTypeArr[21] = BIB_RESSTR(ST_TYPE_CUSTOM5       );

    InitFixedTexts();

    aBasePos = pIdentifierFT->GetPosPixel();

    sal_Int16* pMap = nFT2CtrlMap;
    for( sal_uInt16 i = 0 ; i < FIELD_COUNT ; ++i, ++pMap )
    {
        aControls[ i ] = 0;
        *pMap = -1;
    }

    AdjustScrollbars();
    Link aScrollLnk(LINK(this, BibGeneralPage, ScrollHdl));
    aHoriScroll.SetScrollHdl( aScrollLnk );
    aVertScroll.SetScrollHdl( aScrollLnk );
    aHoriScroll.SetLineSize(10);
    aVertScroll.SetLineSize(10);
    aHoriScroll.SetPageSize( pIdentifierFT->GetSizePixel().Width());
    aVertScroll.SetPageSize(
        pPublisherFT->GetPosPixel().Y() - pIdentifierFT->GetPosPixel().Y());
    aHoriScroll.Show();
    aVertScroll.Show();

    BibConfig* pBibConfig = BibModul::GetConfig();
    BibDBDescriptor aDesc;
    aDesc.sDataSource = pDatMan->getActiveDataSource();
    aDesc.sTableOrQuery = pDatMan->getActiveDataTable();
    aDesc.nCommandType = CommandType::TABLE;
    const Mapping* pMapping = pBibConfig->GetMapping(aDesc);

    xCtrlContnr = VCLUnoHelper::CreateControlContainer(this);

    // the control should be a bit smaller than the fixed text
    Size aControlSize(pIdentifierFT->GetSizePixel());
    aControlSize.Width() = aControlSize.Width() * 8 / 10;

    AddControlWithError( lcl_GetColumnName( pMapping, IDENTIFIER_POS ), lcl_MovePoint( *pIdentifierFT ),
        aControlSize, sTableErrorString, pIdentifierFT->GetText(),
        HID_BIB_IDENTIFIER_POS, 0 );

    sTypeColumnName = lcl_GetColumnName(pMapping, AUTHORITYTYPE_POS);

    AddControlWithError( sTypeColumnName, lcl_MovePoint(*pAuthTypeFT ), aControlSize, sTableErrorString,
        pAuthTypeFT->GetText(), HID_BIB_AUTHORITYTYPE_POS, 1 );

    ::Point aYearPos = lcl_MovePoint(*pYearFT);
    AddControlWithError( lcl_GetColumnName( pMapping, YEAR_POS ), aYearPos,
        aControlSize, sTableErrorString, pYearFT->GetText(), HID_BIB_YEAR_POS, 4 );

    AddControlWithError( lcl_GetColumnName(pMapping, AUTHOR_POS), lcl_MovePoint(*pAuthorFT),
        aControlSize, sTableErrorString, pAuthorFT->GetText(), HID_BIB_AUTHOR_POS, 2 );

    ::Point aTitlePos( lcl_MovePoint( *pTitleFT ) );
    ::Size aTitleSize = pTitleFT->GetSizePixel();
    aTitleSize.Width() = aYearPos.X() + aControlSize.Width() - aTitlePos.X();
    AddControlWithError( lcl_GetColumnName(pMapping, TITLE_POS), aTitlePos, aTitleSize, sTableErrorString,
        pTitleFT->GetText(), HID_BIB_TITLE_POS, 22 );

    AddControlWithError( lcl_GetColumnName( pMapping, PUBLISHER_POS ), lcl_MovePoint( *pPublisherFT),
        aControlSize, sTableErrorString, pPublisherFT->GetText(), HID_BIB_PUBLISHER_POS, 5 );

    AddControlWithError( lcl_GetColumnName( pMapping, ADDRESS_POS ), lcl_MovePoint( *pAddressFT ),
        aControlSize, sTableErrorString, pAddressFT->GetText(), HID_BIB_ADDRESS_POS, 7 );

    AddControlWithError( lcl_GetColumnName( pMapping, ISBN_POS ), lcl_MovePoint( *pISBNFT ),
        aControlSize, sTableErrorString, pISBNFT->GetText(), HID_BIB_ISBN_POS, 6 );

    AddControlWithError( lcl_GetColumnName( pMapping, CHAPTER_POS ), lcl_MovePoint(*pChapterFT),
        aControlSize, sTableErrorString, pChapterFT->GetText(), HID_BIB_CHAPTER_POS, 10 );

    AddControlWithError( lcl_GetColumnName( pMapping, PAGES_POS ), lcl_MovePoint( *pPagesFT ),
        aControlSize, sTableErrorString, pPagesFT->GetText(), HID_BIB_PAGES_POS, 19 );

    AddControlWithError( lcl_GetColumnName( pMapping, EDITOR_POS ), lcl_MovePoint( *pEditorFT ),
        aControlSize, sTableErrorString, pEditorFT->GetText(), HID_BIB_EDITOR_POS, 12 );

    AddControlWithError( lcl_GetColumnName( pMapping, EDITION_POS ), lcl_MovePoint(*pEditionFT),
        aControlSize, sTableErrorString, pEditionFT->GetText(), HID_BIB_EDITION_POS, 11 );

    AddControlWithError( lcl_GetColumnName(pMapping, BOOKTITLE_POS), lcl_MovePoint(*pBooktitleFT),
        aControlSize, sTableErrorString, pBooktitleFT->GetText(), HID_BIB_BOOKTITLE_POS, 9 );

    AddControlWithError( lcl_GetColumnName( pMapping, VOLUME_POS ), lcl_MovePoint( *pVolumeFT ),
        aControlSize, sTableErrorString, pVolumeFT->GetText(), HID_BIB_VOLUME_POS, 24 );

    AddControlWithError( lcl_GetColumnName( pMapping, HOWPUBLISHED_POS ), lcl_MovePoint( *pHowpublishedFT ),
        aControlSize, sTableErrorString, pHowpublishedFT->GetText(), HID_BIB_HOWPUBLISHED_POS, 13 );

    AddControlWithError( lcl_GetColumnName( pMapping, ORGANIZATIONS_POS ), lcl_MovePoint( *pOrganizationsFT ),
        aControlSize, sTableErrorString, pOrganizationsFT->GetText(), HID_BIB_ORGANIZATIONS_POS, 18 );

    AddControlWithError( lcl_GetColumnName( pMapping, INSTITUTION_POS ), lcl_MovePoint( *pInstitutionFT ),
        aControlSize, sTableErrorString, pInstitutionFT->GetText(), HID_BIB_INSTITUTION_POS, 14 );

    AddControlWithError( lcl_GetColumnName( pMapping, SCHOOL_POS ), lcl_MovePoint( *pSchoolFT ),
        aControlSize, sTableErrorString, pSchoolFT->GetText(), HID_BIB_SCHOOL_POS, 20 );

    AddControlWithError( lcl_GetColumnName( pMapping, REPORTTYPE_POS ), lcl_MovePoint( *pReportTypeFT ),
        aControlSize, sTableErrorString, pReportTypeFT->GetText(), HID_BIB_REPORTTYPE_POS, 23 );

    AddControlWithError( lcl_GetColumnName( pMapping, MONTH_POS ), lcl_MovePoint( *pMonthFT ),
        aControlSize, sTableErrorString, pMonthFT->GetText(), HID_BIB_MONTH_POS, 3 );

    AddControlWithError( lcl_GetColumnName( pMapping, JOURNAL_POS ), lcl_MovePoint( *pJournalFT ),
        aControlSize, sTableErrorString, pJournalFT->GetText(), HID_BIB_JOURNAL_POS, 15 );

    AddControlWithError( lcl_GetColumnName( pMapping, NUMBER_POS ), lcl_MovePoint( *pNumberFT ),
        aControlSize, sTableErrorString, pNumberFT->GetText(), HID_BIB_NUMBER_POS, 17 );

    AddControlWithError( lcl_GetColumnName( pMapping, SERIES_POS ), lcl_MovePoint( *pSeriesFT ),
        aControlSize, sTableErrorString, pSeriesFT->GetText(), HID_BIB_SERIES_POS, 21 );

    AddControlWithError( lcl_GetColumnName( pMapping, ANNOTE_POS ), lcl_MovePoint( *pAnnoteFT ),
        aControlSize, sTableErrorString, pAnnoteFT->GetText(), HID_BIB_ANNOTE_POS, 8 );

    AddControlWithError( lcl_GetColumnName( pMapping, NOTE_POS ), lcl_MovePoint( *pNoteFT ),
        aControlSize, sTableErrorString, pNoteFT->GetText(), HID_BIB_NOTE_POS, 16 );

    AddControlWithError( lcl_GetColumnName( pMapping, URL_POS ), lcl_MovePoint( *pURLFT ),
        aControlSize, sTableErrorString, pURLFT->GetText(), HID_BIB_URL_POS, 25 );

    AddControlWithError( lcl_GetColumnName( pMapping, CUSTOM1_POS ), lcl_MovePoint( *pCustom1FT ),
        aControlSize, sTableErrorString, pCustom1FT->GetText(), HID_BIB_CUSTOM1_POS, 26 );

    AddControlWithError( lcl_GetColumnName( pMapping, CUSTOM2_POS ), lcl_MovePoint( *pCustom2FT ),
        aControlSize, sTableErrorString, pCustom2FT->GetText(), HID_BIB_CUSTOM2_POS, 27 );

    AddControlWithError( lcl_GetColumnName( pMapping, CUSTOM3_POS ), lcl_MovePoint( *pCustom3FT ),
        aControlSize, sTableErrorString, pCustom3FT->GetText(), HID_BIB_CUSTOM3_POS, 28 );

    AddControlWithError( lcl_GetColumnName( pMapping, CUSTOM4_POS ), lcl_MovePoint( *pCustom4FT ),
        aControlSize, sTableErrorString, pCustom4FT->GetText(), HID_BIB_CUSTOM4_POS, 29 );

    AddControlWithError( lcl_GetColumnName( pMapping, CUSTOM5_POS ), lcl_MovePoint( *pCustom5FT ),
        aControlSize, sTableErrorString, pCustom5FT->GetText(), HID_BIB_CUSTOM5_POS, 30 );

    xPosListener = new BibPosListener(this);
    uno::Reference< sdbc::XRowSet >  xRowSet(pDatMan->getForm(), UNO_QUERY);
    if(xRowSet.is())
        xRowSet->addRowSetListener(xPosListener);
    uno::Reference< form::runtime::XFormController > xFormCtrl = pDatMan->GetFormController();
    xFormCtrl->setContainer(xCtrlContnr);
    xFormCtrl->activateTabOrder();

    if(!sTableErrorString.isEmpty())
        sTableErrorString = sErrorPrefix + sTableErrorString;

    SetText(BIB_RESSTR(ST_TYPE_TITLE));
}

BibGeneralPage::~BibGeneralPage()
{
    if (pDatMan && xPosListener.is())
    {
        uno::Reference< sdbc::XRowSet >  xRowSet(pDatMan->getForm(), UNO_QUERY);
        if(xRowSet.is())
            xRowSet->removeRowSetListener(xPosListener);
    }

    delete pIdentifierFT;
    delete pAuthTypeFT;
    delete pYearFT;
    delete pAuthorFT;
    delete pTitleFT;
    delete pPublisherFT;
    delete pAddressFT;
    delete pISBNFT;
    delete pChapterFT;
    delete pPagesFT;
    delete pEditorFT;
    delete pEditionFT;
    delete pBooktitleFT;
    delete pVolumeFT;
    delete pHowpublishedFT;
    delete pOrganizationsFT;
    delete pInstitutionFT;
    delete pSchoolFT;
    delete pReportTypeFT;
    delete pMonthFT;
    delete pJournalFT;
    delete pNumberFT;
    delete pSeriesFT;
    delete pAnnoteFT;
    delete pNoteFT;
    delete pURLFT;
    delete pCustom1FT;
    delete pCustom2FT;
    delete pCustom3FT;
    delete pCustom4FT;
    delete pCustom5FT;
}

void BibGeneralPage::RemoveListeners()
{
    for(sal_uInt16 i = 0; i < FIELD_COUNT; i++)
    {
        if(aControls[i].is())
        {
            uno::Reference< awt::XWindow > xCtrWin(aControls[i], uno::UNO_QUERY );
            xCtrWin->removeFocusListener( this );
            aControls[i] = 0;
        }
    }
}

void BibGeneralPage::CommitActiveControl()
{
    uno::Reference< form::runtime::XFormController > xFormCtrl = pDatMan->GetFormController();
    uno::Reference< awt::XControl >  xCurr = xFormCtrl->getCurrentControl();
    if(xCurr.is())
    {
        uno::Reference< awt::XControlModel >  xModel = xCurr->getModel();
        uno::Reference< form::XBoundComponent >  xBound(xModel, UNO_QUERY);
        if(xBound.is())
            xBound->commit();
    }
}

void BibGeneralPage::AddControlWithError( const OUString& rColumnName, const ::Point& rPos, const ::Size& rSize,
                                OUString& rErrorString, const OUString& aColumnUIName, const OString& sHelpId, sal_uInt16 nIndexInFTArray )
{
    // adds also the XControl and creates a map entry in nFT2CtrlMap[] for mapping between control and FT

    sal_Int16                                   nIndex = -1;
    uno::Reference< awt::XControlModel >    xTmp = AddXControl(rColumnName, rPos, rSize, sHelpId, nIndex );
    if( xTmp.is() )
    {
        DBG_ASSERT( nIndexInFTArray < FIELD_COUNT, "*BibGeneralPage::AddControlWithError(): wrong array index!" );
        DBG_ASSERT( nFT2CtrlMap[ nIndexInFTArray ] < 0, "+BibGeneralPage::AddControlWithError(): index already in use!" );

        nFT2CtrlMap[ nIndexInFTArray ] = nIndex;
    }
    else
    {
        if( !rErrorString.isEmpty() )
            rErrorString += "\n";

        rErrorString += MnemonicGenerator::EraseAllMnemonicChars( aColumnUIName );
    }
}

uno::Reference< awt::XControlModel >  BibGeneralPage::AddXControl(
        const OUString& rName,
        ::Point rPos, ::Size rSize, const OString& sHelpId, sal_Int16& rIndex )
{
    uno::Reference< awt::XControlModel >  xCtrModel;
    try
    {
        bool bTypeListBox = sTypeColumnName == rName;
        xCtrModel = pDatMan->loadControlModel(rName, bTypeListBox);
        if ( xCtrModel.is() )
        {
            uno::Reference< beans::XPropertySet >  xPropSet( xCtrModel, UNO_QUERY );

            if( xPropSet.is())
            {
                uno::Reference< beans::XPropertySetInfo >  xPropInfo = xPropSet->getPropertySetInfo();

                uno::Any aAny = xPropSet->getPropertyValue( "DefaultControl" );
                OUString aControlName;
                aAny >>= aControlName;

                OUString uProp("HelpURL");
                if(xPropInfo->hasPropertyByName(uProp))
                {
                    OUString sId = OUString::createFromAscii( INET_HID_SCHEME );
                    DBG_ASSERT( INetURLObject( OStringToOUString( sHelpId, RTL_TEXTENCODING_UTF8 ) ).GetProtocol() == INET_PROT_NOT_VALID, "Wrong HelpId!" );
                    sId += OStringToOUString( sHelpId, RTL_TEXTENCODING_UTF8 );
                    xPropSet->setPropertyValue( uProp, makeAny( sId ) );
                }

                if(bTypeListBox)
                {
                    //uno::Reference< beans::XPropertySet >  xPropSet(xControl, UNO_QUERY);
                    aAny <<= (sal_Int16)1;
                    xPropSet->setPropertyValue("BoundColumn", aAny);
                    ListSourceType eSet = ListSourceType_VALUELIST;
                    aAny.setValue( &eSet, ::cppu::UnoType<ListSourceType>::get() );
                    xPropSet->setPropertyValue("ListSourceType", aAny);

                    uno::Sequence<OUString> aListSource(TYPE_COUNT);
                    OUString* pListSourceArr = aListSource.getArray();
                    //pListSourceArr[0] = "select TypeName, TypeIndex from TypeNms";
                    for(sal_Int32 i = 0; i < TYPE_COUNT; ++i)
                        pListSourceArr[i] = OUString::number(i);
                    aAny.setValue(&aListSource, ::getCppuType((uno::Sequence<OUString>*)0));

                    xPropSet->setPropertyValue("ListSource", aAny);

                    uno::Sequence<OUString> aValues(TYPE_COUNT + 1);
                    OUString* pValuesArr = aValues.getArray();
                    for(sal_uInt16 j = 0; j < TYPE_COUNT; j++)
                        pValuesArr[j]  = aBibTypeArr[j];
                    // empty string if an invalid value no values is set
                    pValuesArr[TYPE_COUNT] = OUString();

                    aAny.setValue(&aValues, ::getCppuType((uno::Sequence<OUString>*)0));

                    xPropSet->setPropertyValue("StringItemList", aAny);

                    sal_Bool bTrue = sal_True;
                    aAny.setValue( &bTrue, ::getBooleanCppuType() );
                    xPropSet->setPropertyValue( "Dropdown", aAny );

                    aControlName = "com.sun.star.form.control.ListBox";
                    xLBModel = Reference< form::XBoundComponent >(xCtrModel, UNO_QUERY);

                }

                uno::Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
                uno::Reference< awt::XControl > xControl( xContext->getServiceManager()->createInstanceWithContext(aControlName, xContext), UNO_QUERY);
                if ( xControl.is() )
                {
                    xControl->setModel( xCtrModel);

                    // Peer as Child to the FrameWindow
                    xCtrlContnr->addControl(rName, xControl);
                    uno::Reference< awt::XWindow >  xCtrWin(xControl, UNO_QUERY );
                    xCtrWin->addFocusListener( this );
                    rIndex = -1;    // -> implies, that not found
                    for(sal_uInt16 i = 0; i < FIELD_COUNT; i++)
                        if(!aControls[i].is())
                        {
                            aControls[i] = xCtrWin;
                            rIndex = sal_Int16( i );
                            break;
                        }
                    xCtrWin->setVisible( sal_True );
                    xControl->setDesignMode( sal_True );
                    // initially switch on the design mode - switch it off _after_ loading the form

                    xCtrWin->setPosSize(rPos.X(), rPos.Y(), rSize.Width(),
                        rSize.Height(), awt::PosSize::POSSIZE);
                }
            }
        }
    }
    catch(Exception& rEx)
    {
        (void) rEx; // make compiler happy
        OSL_FAIL("BibGeneralPage::AddXControl: something went wrong!");
    }
    return xCtrModel;
}

void BibGeneralPage::AdjustScrollbars()
{
    long nVertScrollWidth = aVertScroll.GetSizePixel().Width();
    long nHoriScrollHeight = aHoriScroll.GetSizePixel().Height();
    ::Size aOutSize(GetOutputSizePixel());
    bool bHoriVisible = aOutSize.Width() <= aStdSize.Width();
    bool bVertVisible = (aOutSize.Height()-(bHoriVisible ? nHoriScrollHeight : 0)) <= (aStdSize.Height());
    aHoriScroll.Show(bHoriVisible);
    aVertScroll.Show(bVertVisible);

    if(bHoriVisible)
    {
        ::Size aHoriSize(aOutSize.Width() - (bVertVisible ? nVertScrollWidth : 0),
                        nHoriScrollHeight);
        aHoriScroll.SetSizePixel(aHoriSize);
        aHoriScroll.SetRange( Range(0, aStdSize.Width()));
        aHoriScroll.SetVisibleSize( aHoriSize.Width()  - (bVertVisible ? nVertScrollWidth : 0));
    }
    if(bVertVisible)
    {
        ::Size aVertSize(nHoriScrollHeight, aOutSize.Height() -
                                            (bHoriVisible ? nHoriScrollHeight : 0));
        aVertScroll.SetSizePixel(aVertSize);
        aVertScroll.SetRange( Range(0, aStdSize.Height()));
        aVertScroll.SetVisibleSize( aVertSize.Height() );
    }

    ::Size aSize(8, 8);
    aSize = LogicToPixel(aSize, MapMode(MAP_APPFONT));
    ::Size aScrollSize(aOutSize.Width() - aSize.Height(), aSize.Height());
    ::Point aScrollPos(0, aOutSize.Height() - aSize.Height());
    aHoriScroll.SetPosSizePixel(aScrollPos, aScrollSize);

    aScrollPos.X() = aOutSize.Width() - aSize.Width();
    aScrollPos.Y() = 0;
    aScrollSize.Width() = aSize.Width();
    aScrollSize.Height() = aOutSize.Height() - aSize.Height();
    aVertScroll.SetPosSizePixel(aScrollPos, aScrollSize);

    ::Size aControlParentWinSz(aOutSize);
    if(bHoriVisible)
        aControlParentWinSz.Height() -=  aSize.Height();
    if(bVertVisible)
        aControlParentWinSz.Width() -=  aSize.Width();
    SetSizePixel(aControlParentWinSz);
}

void BibGeneralPage::Resize()
{
    AdjustScrollbars();
    ScrollHdl(&aVertScroll);
    ScrollHdl(&aHoriScroll);
    Window::Resize();
}

void BibGeneralPage::InitFixedTexts( void )
{
    OUString      aFixedStrings[ FIELD_COUNT ] =
    {
        BIB_RESSTR( ST_IDENTIFIER     ),
        BIB_RESSTR( ST_AUTHTYPE       ),
        BIB_RESSTR( ST_AUTHOR         ),
        BIB_RESSTR( ST_TITLE          ),
        BIB_RESSTR( ST_MONTH          ),
        BIB_RESSTR( ST_YEAR           ),
        BIB_RESSTR( ST_ISBN           ),
        BIB_RESSTR( ST_BOOKTITLE      ),
        BIB_RESSTR( ST_CHAPTER        ),
        BIB_RESSTR( ST_EDITION        ),
        BIB_RESSTR( ST_EDITOR         ),
        BIB_RESSTR( ST_HOWPUBLISHED   ),
        BIB_RESSTR( ST_INSTITUTION    ),
        BIB_RESSTR( ST_JOURNAL        ),
        BIB_RESSTR( ST_NOTE           ),
        BIB_RESSTR( ST_ANNOTE         ),
        BIB_RESSTR( ST_NUMBER         ),
        BIB_RESSTR( ST_ORGANIZATION   ),
        BIB_RESSTR( ST_PAGE           ),
        BIB_RESSTR( ST_PUBLISHER      ),
        BIB_RESSTR( ST_ADDRESS        ),
        BIB_RESSTR( ST_SCHOOL         ),
        BIB_RESSTR( ST_SERIES         ),
        BIB_RESSTR( ST_REPORT         ),
        BIB_RESSTR( ST_VOLUME         ),
        BIB_RESSTR( ST_URL            ),
        BIB_RESSTR( ST_CUSTOM1        ),
        BIB_RESSTR( ST_CUSTOM2        ),
        BIB_RESSTR( ST_CUSTOM3        ),
        BIB_RESSTR( ST_CUSTOM4        ),
        BIB_RESSTR( ST_CUSTOM5        )
    };

    aFixedTexts[0] = pIdentifierFT;
    aFixedTexts[1] = pAuthTypeFT;
    aFixedTexts[2] = pAuthorFT;
    aFixedTexts[3] = pTitleFT;
    aFixedTexts[4] = pMonthFT;
    aFixedTexts[5] = pYearFT;
    aFixedTexts[6] = pISBNFT;
    aFixedTexts[7] = pBooktitleFT;
    aFixedTexts[8] = pChapterFT;
    aFixedTexts[9] = pEditionFT;
    aFixedTexts[10] = pEditorFT;
    aFixedTexts[11] = pHowpublishedFT;
    aFixedTexts[12] = pInstitutionFT;
    aFixedTexts[13] = pJournalFT;
    aFixedTexts[14] = pNoteFT;
    aFixedTexts[15] = pAnnoteFT;
    aFixedTexts[16] = pNumberFT;
    aFixedTexts[17] = pOrganizationsFT;
    aFixedTexts[18] = pPagesFT;
    aFixedTexts[19] = pPublisherFT;
    aFixedTexts[20] = pAddressFT;
    aFixedTexts[21] = pSchoolFT;
    aFixedTexts[22] = pSeriesFT;
    aFixedTexts[23] = pReportTypeFT;
    aFixedTexts[24] = pVolumeFT;
    aFixedTexts[25] = pURLFT;
    aFixedTexts[26] = pCustom1FT;
    aFixedTexts[27] = pCustom2FT;
    aFixedTexts[28] = pCustom3FT;
    aFixedTexts[29] = pCustom4FT;
    aFixedTexts[30] = pCustom5FT;

    int                 i;

    MnemonicGenerator   aMnemonicGenerator;
    // init mnemonics, first register all strings
    for( i = 0 ; i < FIELD_COUNT ; ++i )
        aMnemonicGenerator.RegisterMnemonic( aFixedStrings[ i ] );

    // ... then get all strings
    for( i = 0 ; i < FIELD_COUNT ; ++i )
        aMnemonicGenerator.CreateMnemonic( aFixedStrings[ i ] );

    // set texts
    for( i = 0 ; i < FIELD_COUNT ; ++i )
        aFixedTexts[ i ]->SetText( aFixedStrings[ i ] );
}

IMPL_LINK(BibGeneralPage, ScrollHdl, ScrollBar*, pScroll)
{
    bool bVertical = &aVertScroll == pScroll;
    long nOffset = 0;
    long nCurrentOffset = 0;
    if(bVertical)
        nCurrentOffset = aFixedTexts[0]->GetPosPixel().Y() - aBasePos.Y();
    else
        nCurrentOffset = aFixedTexts[0]->GetPosPixel().X() - aBasePos.X();
    nOffset = pScroll->IsVisible() ? pScroll->GetThumbPos() + nCurrentOffset : nCurrentOffset;

    for(sal_uInt16 i = 0; i < FIELD_COUNT; i++)
    {
        ::Point aPos = aFixedTexts[i]->GetPosPixel();
        if(bVertical)
            aPos.Y() -= nOffset;
        else
            aPos.X() -= nOffset;
        aFixedTexts[i]->SetPosPixel(aPos);
        if(aControls[i].is())
        {
            awt::Rectangle aRect = aControls[i]->getPosSize();
            long nX = aRect.X;
            long nY = aRect.Y;
            if(bVertical)
                nY -= nOffset;
            else
                nX -= nOffset;
            aControls[i]->setPosSize(nX, nY, 0, 0, awt::PosSize::POS);
        }
    }
    return 0;
}

void BibGeneralPage::focusGained(const awt::FocusEvent& rEvent) throw( uno::RuntimeException, std::exception )
{
    Reference<awt::XWindow> xCtrWin(rEvent.Source, UNO_QUERY );
    if(xCtrWin.is())
    {
        ::Size aOutSize = GetOutputSizePixel();
        awt::Rectangle aRect = xCtrWin->getPosSize();
        long nX = aRect.X;
        if(nX < 0)
        {
            // left of the visible area
            aHoriScroll.SetThumbPos(aHoriScroll.GetThumbPos() + nX);
            ScrollHdl(&aHoriScroll);
        }
        else if(nX > aOutSize.Width())
        {
            // right of the visible area
            aHoriScroll.SetThumbPos(aHoriScroll.GetThumbPos() + nX - aOutSize.Width() + aFixedTexts[0]->GetSizePixel().Width());
            ScrollHdl(&aHoriScroll);
        }
        long nY = aRect.Y;
        if(nY < 0)
        {
            // below the visible area
            aVertScroll.SetThumbPos(aVertScroll.GetThumbPos() + nY);
            ScrollHdl(&aVertScroll);
        }
        else if(nY > aOutSize.Height())
        {
            // over the visible area
            aVertScroll.SetThumbPos(aVertScroll.GetThumbPos() + nY - aOutSize.Height()+ aFixedTexts[0]->GetSizePixel().Height());
            ScrollHdl(&aVertScroll);
        }
    }
}

void BibGeneralPage::focusLost(const awt::FocusEvent& ) throw( uno::RuntimeException, std::exception )
{
    CommitActiveControl();
}

void BibGeneralPage::disposing(const lang::EventObject& /*Source*/) throw( uno::RuntimeException, std::exception )
{
}

void BibGeneralPage::GetFocus()
{
    Reference< awt::XWindow >*  pxControl = aControls;

    for( int i = FIELD_COUNT ; i ; --i, ++pxControl )
    {
        if( pxControl->is() )
        {
            ( *pxControl )->setFocus();
            return;
        }
    }

    // fallback
    GrabFocus();
}

bool BibGeneralPage::HandleShortCutKey( const KeyEvent& rKeyEvent )
{
    DBG_ASSERT( KEY_MOD2 == rKeyEvent.GetKeyCode().GetModifier(), "+BibGeneralPage::HandleShortCutKey(): this is not for me!" );

    const vcl::I18nHelper&      rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();
    const sal_Unicode           c = rKeyEvent.GetCharCode();
    bool                        bHandled = false;

    sal_Int16                   i;

    typedef std::vector< sal_Int16 >    sal_Int16_vector;

    sal_Int16_vector::size_type nFocused = 0xFFFF;  // index of focused in vector, no one focused initial
    DBG_ASSERT( nFocused > 0, "*BibGeneralPage::HandleShortCutKey(): size_type works not as expected!" );

    sal_Int16_vector            aMatchList;

    for( i = 0 ; i < FIELD_COUNT ; ++i )
    {
        if( rI18nHelper.MatchMnemonic( aFixedTexts[ i ]->GetText(), c ) )
        {
            bHandled = true;
            sal_Int16           nCtrlIndex = nFT2CtrlMap[ i ];

            if( nCtrlIndex >= 0 )
            {   // store index of control
                DBG_ASSERT( aControls[ nCtrlIndex ].is(), "-BibGeneralPage::HandleShortCutKey(): valid index and no control?" );

                uno::Reference< awt::XControl >  xControl( aControls[ nCtrlIndex ], UNO_QUERY );
                DBG_ASSERT( xControl.is(), "-BibGeneralPage::HandleShortCutKey(): a control which is not a control!" );

                Window*         pWindow = VCLUnoHelper::GetWindow( xControl->getPeer() );

                if( pWindow )
                {
                    aMatchList.push_back( nCtrlIndex );
                    if( pWindow->HasChildPathFocus() )
                    {   // save focused control
                        DBG_ASSERT( nFocused == 0xFFFF, "+BibGeneralPage::HandleShortCutKey(): more than one with focus?!" );
                        DBG_ASSERT( !aMatchList.empty(), "+BibGeneralPage::HandleShortCutKey(): push_back and no content?!" );
                        nFocused = aMatchList.size() - 1;
                    }
                }
            }
        }
    }

    if( bHandled )
    {
        DBG_ASSERT( !aMatchList.empty(), "*BibGeneralPage::HandleShortCutKey(): be prepared to crash..." );

        if( nFocused >= ( aMatchList.size() - 1 ) )
            // >=... includes 0xFFFF
            // no one or last focused, take first
            nFocused = 0;
        else
            // take next one
            nFocused++;

        aControls[ aMatchList[ nFocused ] ]->setFocus();
    }

    return bHandled;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
