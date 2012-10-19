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

#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include "general.hxx"
#include "sections.hrc"
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

#define C2U(cChar) OUString::createFromAscii(cChar)
#define C2S(cChar) String::CreateFromAscii(cChar)
#define DISTANCE_CONTROL_TO_FIXEDTEXT 5

static ::Point lcl_MovePoint(const FixedText& rFixedText)
{
    ::Point aRet(rFixedText.GetPosPixel());
    aRet.X() += rFixedText.GetSizePixel().Width();
    aRet.X() += DISTANCE_CONTROL_TO_FIXEDTEXT;
    return aRet;
}

//-----------------------------------------------------------------------------
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
    virtual void SAL_CALL cursorMoved(const lang::EventObject& event) throw( uno::RuntimeException );
    virtual void SAL_CALL rowChanged(const lang::EventObject& /*event*/) throw( uno::RuntimeException ){ /* not interested in */ }
    virtual void SAL_CALL rowSetChanged(const lang::EventObject& /*event*/) throw( uno::RuntimeException ){ /* not interested in */ }

    //XEventListener
    virtual void SAL_CALL disposing(const lang::EventObject& Source) throw( uno::RuntimeException );

};

BibPosListener::BibPosListener(BibGeneralPage* pParent) :
    pParentPage(pParent)
{
}

void BibPosListener::cursorMoved(const lang::EventObject& /*aEvent*/) throw( uno::RuntimeException )
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
            rtl::OUString uTypeMapping = sTypeMapping;

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
                        rtl::OUString sTempVal = xCol->getString();
                        if(sTempVal != rtl::OUString('0'))
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
                xPropSet->setPropertyValue(C2U("SelectedItems"), aSel);
            }
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("BibPosListener::cursorMoved: something went wrong !");
    }
}

void BibPosListener::disposing(const lang::EventObject& /*Source*/) throw( uno::RuntimeException )
{
}

BibGeneralPage::BibGeneralPage(Window* pParent, BibDataManager* pMan):
    BibTabPage(pParent,BibResId(RID_TP_GENERAL)),
    aControlParentWin(this, WB_DIALOGCONTROL),
    aIdentifierFT(&aControlParentWin,   BibResId(FT_IDENTIFIER  )),
    aAuthTypeFT(&aControlParentWin,     BibResId(FT_AUTHTYPE        )),
    aYearFT(&aControlParentWin,         BibResId(FT_YEAR            )),
    aAuthorFT(&aControlParentWin,       BibResId(FT_AUTHOR      )),
    aTitleFT(&aControlParentWin,        BibResId(FT_TITLE           )),
    aPublisherFT(&aControlParentWin,    BibResId(FT_PUBLISHER       )),
    aAddressFT(&aControlParentWin,      BibResId(FT_ADDRESS     )),
    aISBNFT(&aControlParentWin,         BibResId(FT_ISBN            )),
    aChapterFT(&aControlParentWin,      BibResId(FT_CHAPTER     )),
    aPagesFT(&aControlParentWin,        BibResId(FT_PAGE            )),
    aFirstFL(&aControlParentWin,        BibResId(FL_1        )),
    aEditorFT(&aControlParentWin,       BibResId(FT_EDITOR      )),
    aEditionFT(&aControlParentWin,      BibResId(FT_EDITION     )),
    aBooktitleFT(&aControlParentWin,    BibResId(FT_BOOKTITLE       )),
    aVolumeFT(&aControlParentWin,       BibResId(FT_VOLUME      )),
    aHowpublishedFT(&aControlParentWin, BibResId(FT_HOWPUBLISHED    )),
    aOrganizationsFT(&aControlParentWin,BibResId(FT_ORGANIZATION    )),
    aInstitutionFT(&aControlParentWin,  BibResId(FT_INSTITUTION )),
    aSchoolFT(&aControlParentWin,       BibResId(FT_SCHOOL      )),
    aReportTypeFT(&aControlParentWin,   BibResId(FT_REPORT      )),
    aMonthFT(&aControlParentWin,        BibResId(FT_MONTH           )),
    aSecondFL(&aControlParentWin,       BibResId(FL_2        )),
    aJournalFT(&aControlParentWin,      BibResId(FT_JOURNAL     )),
    aNumberFT(&aControlParentWin,       BibResId(FT_NUMBER      )),
    aSeriesFT(&aControlParentWin,       BibResId(FT_SERIES      )),
    aAnnoteFT(&aControlParentWin,       BibResId(FT_ANNOTE      )),
    aNoteFT(&aControlParentWin,         BibResId(FT_NOTE            )),
    aURLFT(&aControlParentWin,          BibResId(FT_URL         )),
    aThirdFL(&aControlParentWin,        BibResId(FL_3        )),
    aCustom1FT(&aControlParentWin,      BibResId(FT_CUSTOM1     )),
    aCustom2FT(&aControlParentWin,      BibResId(FT_CUSTOM2     )),
    aCustom3FT(&aControlParentWin,      BibResId(FT_CUSTOM3     )),
    aCustom4FT(&aControlParentWin,      BibResId(FT_CUSTOM4     )),
    aCustom5FT(&aControlParentWin,      BibResId(FT_CUSTOM5     )),
    aHoriScroll(this, WB_HORZ),
    aVertScroll(this, WB_VERT),
    sErrorPrefix(BibResId(ST_ERROR_PREFIX)),
    pDatMan(pMan)
{
    aControlParentWin.Show();
    aControlParentWin.SetHelpId(HID_BIB_CONTROL_PARENT);
    aStdSize = GetOutputSizePixel();

    aBibTypeArr[0] = String(BibResId(ST_TYPE_ARTICLE));
    aBibTypeArr[1] = String(BibResId(ST_TYPE_BOOK));
    aBibTypeArr[2] = String(BibResId(ST_TYPE_BOOKLET));
    aBibTypeArr[3] = String(BibResId(ST_TYPE_CONFERENCE));
    aBibTypeArr[4] = String(BibResId(ST_TYPE_INBOOK ));
    aBibTypeArr[5] = String(BibResId(ST_TYPE_INCOLLECTION));
    aBibTypeArr[6] = String(BibResId(ST_TYPE_INPROCEEDINGS));
    aBibTypeArr[7] = String(BibResId(ST_TYPE_JOURNAL       ));
    aBibTypeArr[8] = String(BibResId(ST_TYPE_MANUAL    ));
    aBibTypeArr[9] = String(BibResId(ST_TYPE_MASTERSTHESIS));
    aBibTypeArr[10] = String(BibResId(ST_TYPE_MISC      ));
    aBibTypeArr[11] = String(BibResId(ST_TYPE_PHDTHESIS ));
    aBibTypeArr[12] = String(BibResId(ST_TYPE_PROCEEDINGS   ));
    aBibTypeArr[13] = String(BibResId(ST_TYPE_TECHREPORT    ));
    aBibTypeArr[14] = String(BibResId(ST_TYPE_UNPUBLISHED   ));
    aBibTypeArr[15] = String(BibResId(ST_TYPE_EMAIL     ));
    aBibTypeArr[16] = String(BibResId(ST_TYPE_WWW           ));
    aBibTypeArr[17] = String(BibResId(ST_TYPE_CUSTOM1       ));
    aBibTypeArr[18] = String(BibResId(ST_TYPE_CUSTOM2       ));
    aBibTypeArr[19] = String(BibResId(ST_TYPE_CUSTOM3       ));
    aBibTypeArr[20] = String(BibResId(ST_TYPE_CUSTOM4       ));
    aBibTypeArr[21] = String(BibResId(ST_TYPE_CUSTOM5       ));

    FreeResource();

    InitFixedTexts();

    aBasePos = aIdentifierFT.GetPosPixel();

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
    aHoriScroll.SetPageSize( aIdentifierFT.GetSizePixel().Width());
    aVertScroll.SetPageSize(
        aPublisherFT.GetPosPixel().Y() - aIdentifierFT.GetPosPixel().Y());
    aHoriScroll.Show();
    aVertScroll.Show();

    BibConfig* pBibConfig = BibModul::GetConfig();
    BibDBDescriptor aDesc;
    aDesc.sDataSource = pDatMan->getActiveDataSource();
    aDesc.sTableOrQuery = pDatMan->getActiveDataTable();
    aDesc.nCommandType = CommandType::TABLE;
    const Mapping* pMapping = pBibConfig->GetMapping(aDesc);

    xCtrlContnr = VCLUnoHelper::CreateControlContainer(&aControlParentWin);

    xMgr = comphelper::getProcessServiceFactory();
    // the control should be a bit smaller than the fixed text
    Size aControlSize(aIdentifierFT.GetSizePixel());
    aControlSize.Width() = aControlSize.Width() * 8 / 10;

    AddControlWithError( lcl_GetColumnName( pMapping, IDENTIFIER_POS ), lcl_MovePoint( aIdentifierFT ),
        aControlSize, sTableErrorString, aIdentifierFT.GetText(),
        HID_BIB_IDENTIFIER_POS, 0 );

    sTypeColumnName = lcl_GetColumnName(pMapping, AUTHORITYTYPE_POS);

    AddControlWithError( sTypeColumnName, lcl_MovePoint(aAuthTypeFT ), aControlSize, sTableErrorString,
        aAuthTypeFT.GetText(), HID_BIB_AUTHORITYTYPE_POS, 1 );

    ::Point aYearPos = lcl_MovePoint(aYearFT);
    AddControlWithError( lcl_GetColumnName( pMapping, YEAR_POS ), aYearPos,
        aControlSize, sTableErrorString, aYearFT.GetText(), HID_BIB_YEAR_POS, 4 );

    AddControlWithError( lcl_GetColumnName(pMapping, AUTHOR_POS), lcl_MovePoint(aAuthorFT),
        aControlSize, sTableErrorString, aAuthorFT.GetText(), HID_BIB_AUTHOR_POS, 2 );

    ::Point aTitlePos( lcl_MovePoint( aTitleFT ) );
    ::Size aTitleSize = aTitleFT.GetSizePixel();
    aTitleSize.Width() = aYearPos.X() + aControlSize.Width() - aTitlePos.X();
    AddControlWithError( lcl_GetColumnName(pMapping, TITLE_POS), aTitlePos, aTitleSize, sTableErrorString,
        aTitleFT.GetText(), HID_BIB_TITLE_POS, 22 );

    AddControlWithError( lcl_GetColumnName( pMapping, PUBLISHER_POS ), lcl_MovePoint( aPublisherFT),
        aControlSize, sTableErrorString, aPublisherFT.GetText(), HID_BIB_PUBLISHER_POS, 5 );

    AddControlWithError( lcl_GetColumnName( pMapping, ADDRESS_POS ), lcl_MovePoint( aAddressFT ),
        aControlSize, sTableErrorString, aAddressFT.GetText(), HID_BIB_ADDRESS_POS, 7 );

    AddControlWithError( lcl_GetColumnName( pMapping, ISBN_POS ), lcl_MovePoint( aISBNFT ),
        aControlSize, sTableErrorString, aISBNFT.GetText(), HID_BIB_ISBN_POS, 6 );

    AddControlWithError( lcl_GetColumnName( pMapping, CHAPTER_POS ), lcl_MovePoint(aChapterFT),
        aControlSize, sTableErrorString, aChapterFT.GetText(), HID_BIB_CHAPTER_POS, 10 );

    AddControlWithError( lcl_GetColumnName( pMapping, PAGES_POS ), lcl_MovePoint( aPagesFT ),
        aControlSize, sTableErrorString, aPagesFT.GetText(), HID_BIB_PAGES_POS, 19 );

    AddControlWithError( lcl_GetColumnName( pMapping, EDITOR_POS ), lcl_MovePoint( aEditorFT ),
        aControlSize, sTableErrorString, aEditorFT.GetText(), HID_BIB_EDITOR_POS, 12 );

    AddControlWithError( lcl_GetColumnName( pMapping, EDITION_POS ), lcl_MovePoint(aEditionFT),
        aControlSize, sTableErrorString, aEditionFT.GetText(), HID_BIB_EDITION_POS, 11 );

    AddControlWithError( lcl_GetColumnName(pMapping, BOOKTITLE_POS), lcl_MovePoint(aBooktitleFT),
        aControlSize, sTableErrorString, aBooktitleFT.GetText(), HID_BIB_BOOKTITLE_POS, 9 );

    AddControlWithError( lcl_GetColumnName( pMapping, VOLUME_POS ), lcl_MovePoint( aVolumeFT ),
        aControlSize, sTableErrorString, aVolumeFT.GetText(), HID_BIB_VOLUME_POS, 24 );

    AddControlWithError( lcl_GetColumnName( pMapping, HOWPUBLISHED_POS ), lcl_MovePoint( aHowpublishedFT ),
        aControlSize, sTableErrorString, aHowpublishedFT.GetText(), HID_BIB_HOWPUBLISHED_POS, 13 );

    AddControlWithError( lcl_GetColumnName( pMapping, ORGANIZATIONS_POS ), lcl_MovePoint( aOrganizationsFT ),
        aControlSize, sTableErrorString, aOrganizationsFT.GetText(), HID_BIB_ORGANIZATIONS_POS, 18 );

    AddControlWithError( lcl_GetColumnName( pMapping, INSTITUTION_POS ), lcl_MovePoint( aInstitutionFT ),
        aControlSize, sTableErrorString, aInstitutionFT.GetText(), HID_BIB_INSTITUTION_POS, 14 );

    AddControlWithError( lcl_GetColumnName( pMapping, SCHOOL_POS ), lcl_MovePoint( aSchoolFT ),
        aControlSize, sTableErrorString, aSchoolFT.GetText(), HID_BIB_SCHOOL_POS, 20 );

    AddControlWithError( lcl_GetColumnName( pMapping, REPORTTYPE_POS ), lcl_MovePoint( aReportTypeFT ),
        aControlSize, sTableErrorString, aReportTypeFT.GetText(), HID_BIB_REPORTTYPE_POS, 23 );

    AddControlWithError( lcl_GetColumnName( pMapping, MONTH_POS ), lcl_MovePoint( aMonthFT ),
        aControlSize, sTableErrorString, aMonthFT.GetText(), HID_BIB_MONTH_POS, 3 );

    AddControlWithError( lcl_GetColumnName( pMapping, JOURNAL_POS ), lcl_MovePoint( aJournalFT ),
        aControlSize, sTableErrorString, aJournalFT.GetText(), HID_BIB_JOURNAL_POS, 15 );

    AddControlWithError( lcl_GetColumnName( pMapping, NUMBER_POS ), lcl_MovePoint( aNumberFT ),
        aControlSize, sTableErrorString, aNumberFT.GetText(), HID_BIB_NUMBER_POS, 17 );

    AddControlWithError( lcl_GetColumnName( pMapping, SERIES_POS ), lcl_MovePoint( aSeriesFT ),
        aControlSize, sTableErrorString, aSeriesFT.GetText(), HID_BIB_SERIES_POS, 21 );

    AddControlWithError( lcl_GetColumnName( pMapping, ANNOTE_POS ), lcl_MovePoint( aAnnoteFT ),
        aControlSize, sTableErrorString, aAnnoteFT.GetText(), HID_BIB_ANNOTE_POS, 8 );

    AddControlWithError( lcl_GetColumnName( pMapping, NOTE_POS ), lcl_MovePoint( aNoteFT ),
        aControlSize, sTableErrorString, aNoteFT.GetText(), HID_BIB_NOTE_POS, 16 );

    AddControlWithError( lcl_GetColumnName( pMapping, URL_POS ), lcl_MovePoint( aURLFT ),
        aControlSize, sTableErrorString, aURLFT.GetText(), HID_BIB_URL_POS, 25 );

    AddControlWithError( lcl_GetColumnName( pMapping, CUSTOM1_POS ), lcl_MovePoint( aCustom1FT ),
        aControlSize, sTableErrorString, aCustom1FT.GetText(), HID_BIB_CUSTOM1_POS, 26 );

    AddControlWithError( lcl_GetColumnName( pMapping, CUSTOM2_POS ), lcl_MovePoint( aCustom2FT ),
        aControlSize, sTableErrorString, aCustom2FT.GetText(), HID_BIB_CUSTOM2_POS, 27 );

    AddControlWithError( lcl_GetColumnName( pMapping, CUSTOM3_POS ), lcl_MovePoint( aCustom3FT ),
        aControlSize, sTableErrorString, aCustom3FT.GetText(), HID_BIB_CUSTOM3_POS, 28 );

    AddControlWithError( lcl_GetColumnName( pMapping, CUSTOM4_POS ), lcl_MovePoint( aCustom4FT ),
        aControlSize, sTableErrorString, aCustom4FT.GetText(), HID_BIB_CUSTOM4_POS, 29 );

    AddControlWithError( lcl_GetColumnName( pMapping, CUSTOM5_POS ), lcl_MovePoint( aCustom5FT ),
        aControlSize, sTableErrorString, aCustom5FT.GetText(), HID_BIB_CUSTOM5_POS, 30 );

    xPosListener = new BibPosListener(this);
    uno::Reference< sdbc::XRowSet >  xRowSet(pDatMan->getForm(), UNO_QUERY);
    if(xRowSet.is())
        xRowSet->addRowSetListener(xPosListener);
    uno::Reference< form::runtime::XFormController > xFormCtrl = pDatMan->GetFormController();
    xFormCtrl->setContainer(xCtrlContnr);
    xFormCtrl->activateTabOrder();

    if(sTableErrorString.Len())
        sTableErrorString.Insert(sErrorPrefix, 0);
}
//-----------------------------------------------------------------------------
BibGeneralPage::~BibGeneralPage()
{
    if (pDatMan && xPosListener.is())
    {
        uno::Reference< sdbc::XRowSet >  xRowSet(pDatMan->getForm(), UNO_QUERY);
        if(xRowSet.is())
            xRowSet->removeRowSetListener(xPosListener);
    }
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
//-----------------------------------------------------------------------------
void BibGeneralPage::AddControlWithError( const OUString& rColumnName, const ::Point& rPos, const ::Size& rSize,
                                String& rErrorString, String aColumnUIName, const rtl::OString& sHelpId, sal_uInt16 nIndexInFTArray )
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
        if( rErrorString.Len() )
            rErrorString += '\n';

        rErrorString += MnemonicGenerator::EraseAllMnemonicChars( aColumnUIName );
    }
}
//-----------------------------------------------------------------------------
uno::Reference< awt::XControlModel >  BibGeneralPage::AddXControl(
        const String& rName,
        ::Point rPos, ::Size rSize, const rtl::OString& sHelpId, sal_Int16& rIndex )
{
    uno::Reference< awt::XControlModel >  xCtrModel;
    try
    {
        sal_Bool bTypeListBox = sTypeColumnName == rName;
        xCtrModel = pDatMan->loadControlModel(rName, bTypeListBox);
        if ( xCtrModel.is() && xMgr.is())
        {
            uno::Reference< beans::XPropertySet >  xPropSet( xCtrModel, UNO_QUERY );

            if( xPropSet.is())
            {
                uno::Reference< beans::XPropertySetInfo >  xPropInfo = xPropSet->getPropertySetInfo();

                uno::Any aAny = xPropSet->getPropertyValue( C2U("DefaultControl") );
                rtl::OUString aControlName;
                aAny >>= aControlName;

                rtl::OUString uProp(C2U("HelpURL"));
                if(xPropInfo->hasPropertyByName(uProp))
                {
                    ::rtl::OUString sId = ::rtl::OUString::createFromAscii( INET_HID_SCHEME );
                    DBG_ASSERT( INetURLObject( rtl::OStringToOUString( sHelpId, RTL_TEXTENCODING_UTF8 ) ).GetProtocol() == INET_PROT_NOT_VALID, "Wrong HelpId!" );
                    sId += ::rtl::OStringToOUString( sHelpId, RTL_TEXTENCODING_UTF8 );
                    xPropSet->setPropertyValue( uProp, makeAny( sId ) );
                }

                if(bTypeListBox)
                {
                    //uno::Reference< beans::XPropertySet >  xPropSet(xControl, UNO_QUERY);
                    aAny <<= (sal_Int16)1;
                    xPropSet->setPropertyValue(C2U("BoundColumn"), aAny);
                    ListSourceType eSet = ListSourceType_VALUELIST;
                    aAny.setValue( &eSet, ::getCppuType((const ListSourceType*)0) );
                    xPropSet->setPropertyValue(C2U("ListSourceType"), aAny);

                    uno::Sequence<rtl::OUString> aListSource(TYPE_COUNT);
                    rtl::OUString* pListSourceArr = aListSource.getArray();
                    //pListSourceArr[0] = C2U("select TypeName, TypeIndex from TypeNms");
                    for(sal_Int32 i = 0; i < TYPE_COUNT; ++i)
                        pListSourceArr[i] = rtl::OUString::valueOf(i);
                    aAny.setValue(&aListSource, ::getCppuType((uno::Sequence<rtl::OUString>*)0));

                    xPropSet->setPropertyValue(C2U("ListSource"), aAny);

                    uno::Sequence<rtl::OUString> aValues(TYPE_COUNT + 1);
                    rtl::OUString* pValuesArr = aValues.getArray();
                    for(sal_uInt16 j = 0; j < TYPE_COUNT; j++)
                        pValuesArr[j]  = aBibTypeArr[j];
                    // empty string if an invalid value no values is set
                    pValuesArr[TYPE_COUNT] = rtl::OUString();

                    aAny.setValue(&aValues, ::getCppuType((uno::Sequence<rtl::OUString>*)0));

                    xPropSet->setPropertyValue(C2U("StringItemList"), aAny);

                    sal_Bool bTrue = sal_True;
                    aAny.setValue( &bTrue, ::getBooleanCppuType() );
                    xPropSet->setPropertyValue( C2U("Dropdown"), aAny );

                    aControlName = C2U("com.sun.star.form.control.ListBox");
                    xLBModel = Reference< form::XBoundComponent >(xCtrModel, UNO_QUERY);

                }

                uno::Reference< awt::XControl >  xControl(xMgr->createInstance( aControlName ), UNO_QUERY );
                if ( xControl.is() )
                {
                    xControl->setModel( xCtrModel);

                    // Peer als Child zu dem FrameWindow
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
                        // initially switch on the desing mode - switch it off _after_ loading the form

                    xCtrWin->setPosSize(rPos.X(), rPos.Y(), rSize.Width(),
                        rSize.Height(), awt::PosSize::POSSIZE);
                }
            }
        }
    }
    catch(Exception& rEx)
    {
        (void) rEx; // make compiler happy
        OSL_FAIL("BibGeneralPage::AddXControl: something went wrong !");
    }
    return xCtrModel;
}

void BibGeneralPage::AdjustScrollbars()
{
    long nVertScrollWidth = aVertScroll.GetSizePixel().Width();
    long nHoriScrollHeight = aHoriScroll.GetSizePixel().Height();
    ::Size aOutSize(GetOutputSizePixel());
    sal_Bool bHoriVisible = aOutSize.Width() <= aStdSize.Width();
    sal_Bool bVertVisible = (aOutSize.Height()-(bHoriVisible ? nHoriScrollHeight : 0)) <= (aStdSize.Height());
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
    aControlParentWin.SetSizePixel(aControlParentWinSz);
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
    String      aFixedStrings[ FIELD_COUNT ] =
    {
        String( BibResId( ST_IDENTIFIER     ) ),
        String( BibResId( ST_AUTHTYPE       ) ),
        String( BibResId( ST_AUTHOR         ) ),
        String( BibResId( ST_TITLE          ) ),
        String( BibResId( ST_MONTH          ) ),
        String( BibResId( ST_YEAR           ) ),
        String( BibResId( ST_ISBN           ) ),
        String( BibResId( ST_BOOKTITLE      ) ),
        String( BibResId( ST_CHAPTER        ) ),
        String( BibResId( ST_EDITION        ) ),
        String( BibResId( ST_EDITOR         ) ),
        String( BibResId( ST_HOWPUBLISHED   ) ),
        String( BibResId( ST_INSTITUTION    ) ),
        String( BibResId( ST_JOURNAL        ) ),
        String( BibResId( ST_NOTE           ) ),
        String( BibResId( ST_ANNOTE         ) ),
        String( BibResId( ST_NUMBER         ) ),
        String( BibResId( ST_ORGANIZATION   ) ),
        String( BibResId( ST_PAGE           ) ),
        String( BibResId( ST_PUBLISHER      ) ),
        String( BibResId( ST_ADDRESS        ) ),
        String( BibResId( ST_SCHOOL         ) ),
        String( BibResId( ST_SERIES         ) ),
        String( BibResId( ST_REPORT         ) ),
        String( BibResId( ST_VOLUME         ) ),
        String( BibResId( ST_URL            ) ),
        String( BibResId( ST_CUSTOM1        ) ),
        String( BibResId( ST_CUSTOM2        ) ),
        String( BibResId( ST_CUSTOM3        ) ),
        String( BibResId( ST_CUSTOM4        ) ),
        String( BibResId( ST_CUSTOM5        ) )
    };

    aFixedTexts[0] = &aIdentifierFT;
    aFixedTexts[1] = &aAuthTypeFT;
    aFixedTexts[2] = &aAuthorFT;
    aFixedTexts[3] = &aTitleFT;
    aFixedTexts[4] = &aMonthFT;
    aFixedTexts[5] = &aYearFT;
    aFixedTexts[6] = &aISBNFT;
    aFixedTexts[7] = &aBooktitleFT;
    aFixedTexts[8] = &aChapterFT;
    aFixedTexts[9] = &aEditionFT;
    aFixedTexts[10] = &aEditorFT;
    aFixedTexts[11] = &aHowpublishedFT;
    aFixedTexts[12] = &aInstitutionFT;
    aFixedTexts[13] = &aJournalFT;
    aFixedTexts[14] = &aNoteFT;
    aFixedTexts[15] = &aAnnoteFT;
    aFixedTexts[16] = &aNumberFT;
    aFixedTexts[17] = &aOrganizationsFT;
    aFixedTexts[18] = &aPagesFT;
    aFixedTexts[19] = &aPublisherFT;
    aFixedTexts[20] = &aAddressFT;
    aFixedTexts[21] = &aSchoolFT;
    aFixedTexts[22] = &aSeriesFT;
    aFixedTexts[23] = &aReportTypeFT;
    aFixedTexts[24] = &aVolumeFT;
    aFixedTexts[25] = &aURLFT;
    aFixedTexts[26] = &aCustom1FT;
    aFixedTexts[27] = &aCustom2FT;
    aFixedTexts[28] = &aCustom3FT;
    aFixedTexts[29] = &aCustom4FT;
    aFixedTexts[30] = &aCustom5FT;

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
    sal_Bool bVertical = &aVertScroll == pScroll;
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

void BibGeneralPage::focusGained(const awt::FocusEvent& rEvent) throw( uno::RuntimeException )
{
    Reference<awt::XWindow> xCtrWin(rEvent.Source, UNO_QUERY );
    if(xCtrWin.is())
    {
        ::Size aOutSize = aControlParentWin.GetOutputSizePixel();
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

void BibGeneralPage::focusLost(const awt::FocusEvent& ) throw( uno::RuntimeException )
{
    CommitActiveControl();
}

void BibGeneralPage::disposing(const lang::EventObject& /*Source*/) throw( uno::RuntimeException )
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
    aControlParentWin.GrabFocus();
}

sal_Bool BibGeneralPage::HandleShortCutKey( const KeyEvent& rKeyEvent )
{
    DBG_ASSERT( KEY_MOD2 == rKeyEvent.GetKeyCode().GetModifier(), "+BibGeneralPage::HandleShortCutKey(): this is not for me!" );

    const vcl::I18nHelper&      rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();
    const sal_Unicode           c = rKeyEvent.GetCharCode();
    sal_Bool                        bHandled = sal_False;

    sal_Int16                   i;

    typedef std::vector< sal_Int16 >    sal_Int16_vector;

    sal_Int16_vector::size_type nFocused = 0xFFFF;  // index of focused in vector, no one focused initial
    DBG_ASSERT( nFocused > 0, "*BibGeneralPage::HandleShortCutKey(): size_type works not as expected!" );

    sal_Int16_vector            aMatchList;

    for( i = 0 ; i < FIELD_COUNT ; ++i )
    {
        if( rI18nHelper.MatchMnemonic( aFixedTexts[ i ]->GetText(), c ) )
        {
            bHandled = sal_True;
            sal_Int16           nCtrlIndex = nFT2CtrlMap[ i ];

            if( nCtrlIndex >= 0 )
            {   // store index of control
                DBG_ASSERT( aControls[ nCtrlIndex ].is(), "-BibGeneralPage::HandleShortCutKey(): valid index and no control?" );

                uno::Reference< awt::XControl >  xControl( aControls[ nCtrlIndex ], UNO_QUERY );
                DBG_ASSERT( xControl.is(), "-BibGeneralPage::HandleShortCutKey(): a control wich is not a control!" );

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
