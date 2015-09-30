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
#include <com/sun/star/awt/XWindow.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <vcl/builder.hxx>
#include <vcl/settings.hxx>
#include "general.hxx"
#include "bibresid.hxx"
#include "datman.hxx"
#include "bibconfig.hxx"
#include "bibprop.hrc"
#include "bib.hrc"
#include "bibmod.hxx"
#include "bibview.hxx"
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

class BibPosListener    :public cppu::WeakImplHelper <sdbc::XRowSetListener>
{
    VclPtr<BibGeneralPage>     pParentPage;
public:
    explicit BibPosListener(BibGeneralPage* pParent);

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
                uno::Reference< uno::XInterface >  xInt = *static_cast<uno::Reference< uno::XInterface > const *>(aVal.getValue());
                uno::Reference< sdb::XColumn >  xCol(xInt, UNO_QUERY);
                DBG_ASSERT(xCol.is(), "BibPosListener::cursorMoved : invalid column (no sdb::XColumn) !");
                if (xCol.is())
                {
                    nTempVal = xCol->getShort();
                    // getShort returns zero if the value is not a number
                    if (!nTempVal || xCol->wasNull())
                    {
                        OUString sTempVal = xCol->getString();
                        if(sTempVal != "0")
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
                aSel.setValue(&aSelSeq, cppu::UnoType<Sequence<sal_Int16>>::get());
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

BibGeneralPage::BibGeneralPage(vcl::Window* pParent, BibDataManager* pMan):
    BibTabPage(pParent, "GeneralPage", "modules/sbibliography/ui/generalpage.ui"),
    sErrorPrefix(BIB_RESSTR(ST_ERROR_PREFIX)),
    mxBibGeneralPageFocusListener(new BibGeneralPageFocusListener(this)),
    pDatMan(pMan)
{
    get(pIdentifierFT, "shortname");
    get(pAuthTypeFT, "authtype");
    get(pGrid, "grid");
    get(pScrolledWindow, "scrolledwindow");
    get(pYearFT, "year");
    get(pAuthorFT, "authors");
    get(pTitleFT, "title");
    get(pPublisherFT, "publisher");
    get(pAddressFT, "address");
    get(pISBNFT, "isbn");
    get(pChapterFT, "chapter");
    get(pPagesFT, "pages");
    get(pEditorFT, "editor");
    get(pEditionFT, "edition");
    get(pBooktitleFT, "booktitle");
    get(pVolumeFT, "volume");
    get(pHowpublishedFT, "publicationtype");
    get(pOrganizationsFT, "organization");
    get(pInstitutionFT, "institution");
    get(pSchoolFT, "university");
    get(pReportTypeFT, "reporttype");
    get(pMonthFT, "month");
    get(pJournalFT, "journal");
    get(pNumberFT, "number");
    get(pSeriesFT, "series");
    get(pAnnoteFT, "annotation");
    get(pNoteFT, "note");
    get(pURLFT, "url");
    get(pCustom1FT, "custom1");
    get(pCustom2FT, "custom2");
    get(pCustom3FT, "custom3");
    get(pCustom4FT, "custom4");
    get(pCustom5FT, "custom5");

    InitFixedTexts();

    sal_Int16* pMap = nFT2CtrlMap;
    for( sal_uInt16 i = 0 ; i < FIELD_COUNT ; ++i, ++pMap )
    {
        aControls[ i ] = 0;
        *pMap = -1;
    }

    BibConfig* pBibConfig = BibModul::GetConfig();
    BibDBDescriptor aDesc;
    aDesc.sDataSource = pDatMan->getActiveDataSource();
    aDesc.sTableOrQuery = pDatMan->getActiveDataTable();
    aDesc.nCommandType = CommandType::TABLE;
    const Mapping* pMapping = pBibConfig->GetMapping(aDesc);

    xCtrlContnr = VCLUnoHelper::CreateControlContainer(pGrid);

    std::vector<vcl::Window*> aChildren;

    AddControlWithError(lcl_GetColumnName(pMapping, IDENTIFIER_POS), *pIdentifierFT,
        sTableErrorString,
        HID_BIB_IDENTIFIER_POS, 0, aChildren);

    sTypeColumnName = lcl_GetColumnName(pMapping, AUTHORITYTYPE_POS);

    AddControlWithError(sTypeColumnName, *pAuthTypeFT, sTableErrorString,
        HID_BIB_AUTHORITYTYPE_POS, 1, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, YEAR_POS), *pYearFT,
        sTableErrorString, HID_BIB_YEAR_POS, 2, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, AUTHOR_POS), *pAuthorFT,
        sTableErrorString, HID_BIB_AUTHOR_POS, 3, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, TITLE_POS), *pTitleFT, sTableErrorString,
        HID_BIB_TITLE_POS, 4, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, PUBLISHER_POS), *pPublisherFT,
        sTableErrorString, HID_BIB_PUBLISHER_POS, 5, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, ADDRESS_POS), *pAddressFT,
        sTableErrorString, HID_BIB_ADDRESS_POS, 6, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, ISBN_POS), *pISBNFT,
        sTableErrorString, HID_BIB_ISBN_POS, 7, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, CHAPTER_POS), *pChapterFT,
        sTableErrorString, HID_BIB_CHAPTER_POS, 8, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, PAGES_POS), *pPagesFT,
        sTableErrorString, HID_BIB_PAGES_POS, 9, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, EDITOR_POS), *pEditorFT,
        sTableErrorString, HID_BIB_EDITOR_POS, 10, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, EDITION_POS), *pEditionFT,
        sTableErrorString, HID_BIB_EDITION_POS, 11, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, BOOKTITLE_POS), *pBooktitleFT,
        sTableErrorString, HID_BIB_BOOKTITLE_POS, 12, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, VOLUME_POS), *pVolumeFT,
        sTableErrorString, HID_BIB_VOLUME_POS, 13, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, HOWPUBLISHED_POS), *pHowpublishedFT,
        sTableErrorString, HID_BIB_HOWPUBLISHED_POS, 14, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, ORGANIZATIONS_POS), *pOrganizationsFT,
        sTableErrorString, HID_BIB_ORGANIZATIONS_POS, 15, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, INSTITUTION_POS), *pInstitutionFT,
        sTableErrorString, HID_BIB_INSTITUTION_POS, 16, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, SCHOOL_POS), *pSchoolFT,
        sTableErrorString, HID_BIB_SCHOOL_POS, 17, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, REPORTTYPE_POS), *pReportTypeFT,
        sTableErrorString, HID_BIB_REPORTTYPE_POS, 18, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, MONTH_POS), *pMonthFT,
        sTableErrorString, HID_BIB_MONTH_POS, 19, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, JOURNAL_POS), *pJournalFT,
        sTableErrorString, HID_BIB_JOURNAL_POS, 20, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, NUMBER_POS), *pNumberFT,
        sTableErrorString, HID_BIB_NUMBER_POS, 21, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, SERIES_POS), *pSeriesFT,
        sTableErrorString, HID_BIB_SERIES_POS, 22, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, ANNOTE_POS), *pAnnoteFT,
        sTableErrorString, HID_BIB_ANNOTE_POS, 23, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, NOTE_POS),*pNoteFT,
        sTableErrorString, HID_BIB_NOTE_POS, 24, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, URL_POS), *pURLFT,
        sTableErrorString, HID_BIB_URL_POS, 25, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, CUSTOM1_POS), *pCustom1FT,
        sTableErrorString, HID_BIB_CUSTOM1_POS, 26, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, CUSTOM2_POS), *pCustom2FT,
        sTableErrorString, HID_BIB_CUSTOM2_POS, 27, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, CUSTOM3_POS), *pCustom3FT,
        sTableErrorString, HID_BIB_CUSTOM3_POS, 28, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, CUSTOM4_POS), *pCustom4FT,
        sTableErrorString, HID_BIB_CUSTOM4_POS, 29, aChildren);

    AddControlWithError(lcl_GetColumnName(pMapping, CUSTOM5_POS), *pCustom5FT,
        sTableErrorString, HID_BIB_CUSTOM5_POS, 30, aChildren);

    VclBuilder::reorderWithinParent(aChildren, false);

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

    Size aSize(LogicToPixel(Size(0, 209), MapMode(MAP_APPFONT)));
    set_height_request(aSize.Height());
}

BibGeneralPage::~BibGeneralPage()
{
    disposeOnce();
}

void BibGeneralPage::dispose()
{
    if (pDatMan && xPosListener.is())
    {
        uno::Reference< sdbc::XRowSet >  xRowSet(pDatMan->getForm(), UNO_QUERY);
        if(xRowSet.is())
            xRowSet->removeRowSetListener(xPosListener);
    }
    pGrid.clear();
    pScrolledWindow.clear();
    pIdentifierFT.clear();
    pAuthTypeFT.clear();
    pYearFT.clear();
    pAuthorFT.clear();
    pTitleFT.clear();
    pPublisherFT.clear();
    pAddressFT.clear();
    pISBNFT.clear();
    pChapterFT.clear();
    pPagesFT.clear();
    pEditorFT.clear();
    pEditionFT.clear();
    pBooktitleFT.clear();
    pVolumeFT.clear();
    pHowpublishedFT.clear();
    pOrganizationsFT.clear();
    pInstitutionFT.clear();
    pSchoolFT.clear();
    pReportTypeFT.clear();
    pMonthFT.clear();
    pJournalFT.clear();
    pNumberFT.clear();
    pSeriesFT.clear();
    pAnnoteFT.clear();
    pNoteFT.clear();
    pURLFT.clear();
    pCustom1FT.clear();
    pCustom2FT.clear();
    pCustom3FT.clear();
    pCustom4FT.clear();
    pCustom5FT.clear();
    for (auto & a: aFixedTexts) a.clear();
    mxBibGeneralPageFocusListener.clear();
    BibTabPage::dispose();
}

void BibGeneralPage::RemoveListeners()
{
    for(sal_uInt16 i = 0; i < FIELD_COUNT; i++)
    {
        if(aControls[i].is())
        {
            uno::Reference< awt::XWindow > xCtrWin(aControls[i], uno::UNO_QUERY );
            xCtrWin->removeFocusListener( mxBibGeneralPageFocusListener.get() );
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

void BibGeneralPage::AddControlWithError( const OUString& rColumnName, FixedText &rLabel,
    OUString& rErrorString, const OString& sHelpId, sal_uInt16 nIndexInFTArray, std::vector<vcl::Window*> &rChildren)
{
    const OUString aColumnUIName(rLabel.GetText());
    // adds also the XControl and creates a map entry in nFT2CtrlMap[] for mapping between control and FT

    sal_Int16                                   nIndex = -1;
    uno::Reference< awt::XControlModel >    xTmp = AddXControl(rColumnName, rLabel, sHelpId, nIndex, rChildren);
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
        FixedText& rLabel, const OString& sHelpId, sal_Int16& rIndex,
        std::vector<vcl::Window*>& rChildren)
{
    uno::Reference< awt::XControlModel >  xCtrModel;
    try
    {
        const bool bTypeListBox = sTypeColumnName == rName;
        xCtrModel = pDatMan->loadControlModel(rName, bTypeListBox);
        if ( xCtrModel.is() )
        {
            uno::Reference< beans::XPropertySet >  xPropSet( xCtrModel, UNO_QUERY );

            if( xPropSet.is())
            {
                uno::Reference< beans::XPropertySetInfo >  xPropInfo = xPropSet->getPropertySetInfo();

                OUString aControlName;
                if (bTypeListBox)
                {
                    aControlName = "com.sun.star.form.control.ListBox";
                    xLBModel = Reference< form::XBoundComponent >(xCtrModel, UNO_QUERY);
                }
                else
                {
                    uno::Any aAny = xPropSet->getPropertyValue( "DefaultControl" );
                    aAny >>= aControlName;
                }

                OUString uProp("HelpURL");
                if(xPropInfo->hasPropertyByName(uProp))
                {
                    OUString sId( INET_HID_SCHEME );
                    DBG_ASSERT( INetURLObject( OStringToOUString( sHelpId, RTL_TEXTENCODING_UTF8 ) ).GetProtocol() == INetProtocol::NotValid, "Wrong HelpId!" );
                    sId += OStringToOUString( sHelpId, RTL_TEXTENCODING_UTF8 );
                    xPropSet->setPropertyValue( uProp, makeAny( sId ) );
                }

                uno::Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
                uno::Reference< awt::XControl > xControl( xContext->getServiceManager()->createInstanceWithContext(aControlName, xContext), UNO_QUERY);
                if ( xControl.is() )
                {
                    xControl->setModel( xCtrModel);

                    // Peer as Child to the FrameWindow
                    xCtrlContnr->addControl(rName, xControl);
                    uno::Reference< awt::XWindow >  xCtrWin(xControl, UNO_QUERY );
                    xCtrWin->addFocusListener( mxBibGeneralPageFocusListener.get() );
                    rIndex = -1;    // -> implies, that not found
                    for(sal_uInt16 i = 0; i < FIELD_COUNT; i++)
                        if(!aControls[i].is())
                        {
                            aControls[i] = xCtrWin;
                            rIndex = sal_Int16( i );
                            break;
                        }
                    // initially switch on the design mode - switch it off _after_ loading the form
                    xCtrWin->setVisible( sal_True );
                    xControl->setDesignMode( sal_True );

                    vcl::Window* pWindow = VCLUnoHelper::GetWindow(xControl->getPeer());
                    pWindow->set_grid_top_attach(rLabel.get_grid_top_attach());
                    pWindow->set_grid_left_attach(rLabel.get_grid_left_attach()+1);
                    pWindow->set_valign(VCL_ALIGN_CENTER);
                    rLabel.set_mnemonic_widget(pWindow);
                    if (&rLabel == pTitleFT)
                        pWindow->set_grid_width(3);
                    else
                        pWindow->set_hexpand(true);
                    rChildren.push_back(&rLabel);
                    rChildren.push_back(pWindow);
                }
            }
        }
    }
    catch(const Exception& rEx)
    {
        (void) rEx; // make compiler happy
        OSL_FAIL("BibGeneralPage::AddXControl: something went wrong!");
    }
    return xCtrModel;
}

void BibGeneralPage::InitFixedTexts()
{
    aFixedTexts[0] = pIdentifierFT;
    aFixedTexts[1] = pAuthTypeFT;
    aFixedTexts[2] = pYearFT;
    aFixedTexts[3] = pAuthorFT;
    aFixedTexts[4] = pTitleFT;
    aFixedTexts[5] = pPublisherFT;
    aFixedTexts[6] = pAddressFT;
    aFixedTexts[7] = pISBNFT;
    aFixedTexts[8] = pChapterFT;
    aFixedTexts[9] = pPagesFT;

    aFixedTexts[10] = pEditorFT;
    aFixedTexts[11] = pEditionFT;
    aFixedTexts[12] = pBooktitleFT;
    aFixedTexts[13] = pVolumeFT;
    aFixedTexts[14] = pHowpublishedFT;
    aFixedTexts[15] = pOrganizationsFT;
    aFixedTexts[16] = pInstitutionFT;
    aFixedTexts[17] = pSchoolFT;
    aFixedTexts[18] = pReportTypeFT;
    aFixedTexts[19] = pMonthFT;

    aFixedTexts[20] = pJournalFT;
    aFixedTexts[21] = pNumberFT;
    aFixedTexts[22] = pSeriesFT;
    aFixedTexts[23] = pAnnoteFT;
    aFixedTexts[24] = pNoteFT;
    aFixedTexts[25] = pURLFT;

    aFixedTexts[26] = pCustom1FT;
    aFixedTexts[27] = pCustom2FT;
    aFixedTexts[28] = pCustom3FT;
    aFixedTexts[29] = pCustom4FT;
    aFixedTexts[30] = pCustom5FT;

    int                 i;

    MnemonicGenerator   aMnemonicGenerator;

    OUString aFixedStrings[ FIELD_COUNT ];
    for( i = 0 ; i < FIELD_COUNT ; ++i )
        aFixedStrings[i] = aFixedTexts[i]->GetText();

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

void BibGeneralPage::focusGained(const awt::FocusEvent& rEvent) throw( uno::RuntimeException, std::exception )
{
    Reference<awt::XWindow> xCtrWin(rEvent.Source, UNO_QUERY );
    if(xCtrWin.is())
    {
        ::Size aOutSize = pScrolledWindow->getVisibleChildSize();
        awt::Rectangle aRect = xCtrWin->getPosSize();
        Point aOffset(pGrid->GetPosPixel());
        long nX = aRect.X + aOffset.X();
        if (nX < 0 || nX > aOutSize.Width())
        {
            pScrolledWindow->getHorzScrollBar().DoScroll(aRect.X);
        }

        long nY = aRect.Y + aOffset.Y();
        if (nY < 0 || nY > aOutSize.Height())
        {
            pScrolledWindow->getVertScrollBar().DoScroll(aRect.Y);
        }
    }
}

void BibGeneralPage::focusLost(const awt::FocusEvent& ) throw( uno::RuntimeException, std::exception )
{
    CommitActiveControl();
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

                vcl::Window*         pWindow = VCLUnoHelper::GetWindow( xControl->getPeer() );

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

BibGeneralPageFocusListener::BibGeneralPageFocusListener(BibGeneralPage *pBibGeneralPage): mpBibGeneralPage(pBibGeneralPage)
{}

void BibGeneralPageFocusListener::focusGained( const css::awt::FocusEvent& e ) throw( css::uno::RuntimeException, std::exception )
{
    mpBibGeneralPage->focusGained(e);
}

void BibGeneralPageFocusListener::focusLost( const css::awt::FocusEvent& e ) throw( css::uno::RuntimeException, std::exception )
{
    mpBibGeneralPage->focusLost(e);
}

void BibGeneralPageFocusListener::disposing( const css::lang::EventObject& ) throw( css::uno::RuntimeException, std::exception )
{}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
