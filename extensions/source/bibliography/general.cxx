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
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <vcl/event.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/settings.hxx>
#include "general.hxx"
#include "bibresid.hxx"
#include "datman.hxx"
#include "bibconfig.hxx"
#include <strings.hrc>
#include "bibmod.hxx"
#include <helpids.h>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <algorithm>
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
        for(const auto & aColumnPair : pMapping->aColumnPairs)
        {
            if(aColumnPair.sLogicalColumnName == sRet)
            {
                sRet = aColumnPair.sRealColumnName;
                break;
            }
        }
    return sRet;
}

namespace {

class BibPosListener    :public cppu::WeakImplHelper <sdbc::XRowSetListener>
{
    VclPtr<BibGeneralPage>     pParentPage;
public:
    explicit BibPosListener(BibGeneralPage* pParent);

    //XPositioningListener
    virtual void SAL_CALL cursorMoved(const lang::EventObject& event) override;
    virtual void SAL_CALL rowChanged(const lang::EventObject& /*event*/) override { /* not interested in */ }
    virtual void SAL_CALL rowSetChanged(const lang::EventObject& /*event*/) override { /* not interested in */ }

    //XEventListener
    virtual void SAL_CALL disposing(const lang::EventObject& Source) override;

};

}

BibPosListener::BibPosListener(BibGeneralPage* pParent) :
    pParentPage(pParent)
{
}

void BibPosListener::cursorMoved(const lang::EventObject& /*aEvent*/)
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
                for(const auto & aColumnPair : pMapping->aColumnPairs)
                {
                    if(aColumnPair.sLogicalColumnName == sTypeMapping)
                    {
                        sTypeMapping = aColumnPair.sRealColumnName;
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
                uno::Reference< sdb::XColumn >  xCol(aVal, UNO_QUERY);
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
                aSel <<= aSelSeq;
                xPropSet->setPropertyValue("SelectedItems", aSel);
            }
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("BibPosListener::cursorMoved: something went wrong !");
    }
}

void BibPosListener::disposing(const lang::EventObject& /*Source*/)
{
}

BibGeneralPage::BibGeneralPage(vcl::Window* pParent, BibDataManager* pMan)
    : InterimItemWindow(pParent, "modules/sbibliography/ui/generalpage.ui", "GeneralPage")
    , BibShortCutHandler(this)
//TODO    , xGrid(m_xBuilder->weld_label("grid"))
//TODO    , xScrolledWindow(m_xBuilder->weld_label("scrolledwindow"))
    , xIdentifierFT(m_xBuilder->weld_label("shortname"))
    , xIdentifierED(m_xBuilder->weld_entry("shortnamecontrol"))
    , xAuthTypeFT(m_xBuilder->weld_label("authtype"))
    , xAuthTypeLB(m_xBuilder->weld_combo_box("authtypecontrol"))
    , xYearFT(m_xBuilder->weld_label("year"))
    , xYearED(m_xBuilder->weld_entry("yearcontrol"))
    , xAuthorFT(m_xBuilder->weld_label("authors"))
    , xAuthorED(m_xBuilder->weld_entry("authorscontrol"))
    , xTitleFT(m_xBuilder->weld_label("title"))
    , xTitleED(m_xBuilder->weld_entry("titlecontrol"))
    , xPublisherFT(m_xBuilder->weld_label("publisher"))
    , xPublisherED(m_xBuilder->weld_entry("publishercontrol"))
    , xAddressFT(m_xBuilder->weld_label("address"))
    , xAddressED(m_xBuilder->weld_entry("addresscontrol"))
    , xISBNFT(m_xBuilder->weld_label("isbn"))
    , xISBNED(m_xBuilder->weld_entry("isbncontrol"))
    , xChapterFT(m_xBuilder->weld_label("chapter"))
    , xPagesFT(m_xBuilder->weld_label("pages"))
    , xEditorFT(m_xBuilder->weld_label("editor"))
    , xEditionFT(m_xBuilder->weld_label("edition"))
    , xBooktitleFT(m_xBuilder->weld_label("booktitle"))
    , xVolumeFT(m_xBuilder->weld_label("volume"))
    , xHowpublishedFT(m_xBuilder->weld_label("publicationtype"))
    , xOrganizationsFT(m_xBuilder->weld_label("organization"))
    , xInstitutionFT(m_xBuilder->weld_label("institution"))
    , xSchoolFT(m_xBuilder->weld_label("university"))
    , xReportTypeFT(m_xBuilder->weld_label("reporttype"))
    , xMonthFT(m_xBuilder->weld_label("month"))
    , xJournalFT(m_xBuilder->weld_label("journal"))
    , xNumberFT(m_xBuilder->weld_label("number"))
    , xSeriesFT(m_xBuilder->weld_label("series"))
    , xAnnoteFT(m_xBuilder->weld_label("annotation"))
    , xNoteFT(m_xBuilder->weld_label("note"))
    , xURLFT(m_xBuilder->weld_label("url"))
    , xCustom1FT(m_xBuilder->weld_label("custom1"))
    , xCustom2FT(m_xBuilder->weld_label("custom2"))
    , xCustom3FT(m_xBuilder->weld_label("custom3"))
    , xCustom4FT(m_xBuilder->weld_label("custom4"))
    , xCustom5FT(m_xBuilder->weld_label("custom5"))
    , mxBibGeneralPageFocusListener(new BibGeneralPageFocusListener(this))
    , pDatMan(pMan)
{
#if 0
    set_hexpand(true);
    set_vexpand(true);
    set_expand(true);
#endif
#if 0

    sal_Int16* pMap = nFT2CtrlMap;
    for( sal_uInt16 i = 0 ; i < FIELD_COUNT ; ++i, ++pMap )
    {
        aControls[ i ] = nullptr;
        *pMap = -1;
    }
#endif

    BibConfig* pBibConfig = BibModul::GetConfig();
    BibDBDescriptor aDesc;
    aDesc.sDataSource = pDatMan->getActiveDataSource();
    aDesc.sTableOrQuery = pDatMan->getActiveDataTable();
    aDesc.nCommandType = CommandType::TABLE;
    const Mapping* pMapping = pBibConfig->GetMapping(aDesc);

    AddControlWithError(lcl_GetColumnName(pMapping, IDENTIFIER_POS),
        xIdentifierFT->get_label(), *xIdentifierED,
        sTableErrorString, HID_BIB_IDENTIFIER_POS);

#if 0
    xCtrlContnr = VCLUnoHelper::CreateControlContainer(pGrid);

    std::vector<vcl::Window*> aChildren;

    sTypeColumnName = lcl_GetColumnName(pMapping, AUTHORITYTYPE_POS);
#endif

    AddControlWithError(lcl_GetColumnName(pMapping, AUTHORITYTYPE_POS),
        xAuthTypeFT->get_label(), *xAuthTypeLB,
        sTableErrorString, HID_BIB_AUTHORITYTYPE_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, YEAR_POS),
        xYearFT->get_label(), *xYearED,
        sTableErrorString, HID_BIB_YEAR_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, AUTHOR_POS),
        xAuthorFT->get_label(), *xAuthorED,
        sTableErrorString, HID_BIB_AUTHOR_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, TITLE_POS),
        xTitleFT->get_label(), *xTitleED,
        sTableErrorString, HID_BIB_TITLE_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, PUBLISHER_POS),
        xPublisherFT->get_label(), *xPublisherED,
        sTableErrorString, HID_BIB_PUBLISHER_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, ADDRESS_POS),
        xAddressFT->get_label(), *xAddressED,
        sTableErrorString, HID_BIB_ADDRESS_POS);

    AddControlWithError(lcl_GetColumnName(pMapping, ISBN_POS),
        xISBNFT->get_label(), *xISBNED,
        sTableErrorString, HID_BIB_ISBN_POS);

#if 0
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

    BuilderUtils::reorderWithinParent(aChildren, false);
#endif

    xPosListener = new BibPosListener(this);
    uno::Reference< sdbc::XRowSet >  xRowSet(pDatMan->getForm(), UNO_QUERY);
    if(xRowSet.is())
        xRowSet->addRowSetListener(xPosListener);
#if 0
    uno::Reference< form::runtime::XFormController > xFormCtrl = pDatMan->GetFormController();
    xFormCtrl->setContainer(xCtrlContnr);
    xFormCtrl->activateTabOrder();
#endif

    if(!sTableErrorString.isEmpty())
        sTableErrorString = BibResId(ST_ERROR_PREFIX) + sTableErrorString;

    SetText(BibResId(ST_TYPE_TITLE));

    Size aSize(LogicToPixel(Size(0, 209), MapMode(MapUnit::MapAppFont)));
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
//TODO    xGrid.reset();
//TODO    xScrolledWindow.reset();
    xIdentifierFT.reset();
    xIdentifierED.reset();
    xAuthTypeFT.reset();
    xAuthTypeLB.reset();
    xYearFT.reset();
    xYearED.reset();
    xAuthorFT.reset();
    xAuthorED.reset();
    xTitleFT.reset();
    xTitleED.reset();
    xPublisherFT.reset();
    xPublisherED.reset();
    xAddressFT.reset();
    xAddressED.reset();
    xISBNFT.reset();
    xISBNED.reset();
    xChapterFT.reset();
    xPagesFT.reset();
    xEditorFT.reset();
    xEditionFT.reset();
    xBooktitleFT.reset();
    xVolumeFT.reset();
    xHowpublishedFT.reset();
    xOrganizationsFT.reset();
    xInstitutionFT.reset();
    xSchoolFT.reset();
    xReportTypeFT.reset();
    xMonthFT.reset();
    xJournalFT.reset();
    xNumberFT.reset();
    xSeriesFT.reset();
    xAnnoteFT.reset();
    xNoteFT.reset();
    xURLFT.reset();
    xCustom1FT.reset();
    xCustom2FT.reset();
    xCustom3FT.reset();
    xCustom4FT.reset();
    xCustom5FT.reset();
#if 0
    for (auto & a: aFixedTexts) a.clear();
#endif
    mxBibGeneralPageFocusListener.clear();
    InterimItemWindow::dispose();
}

void BibGeneralPage::RemoveListeners()
{
#if 0
    for(uno::Reference<awt::XWindow> & aControl : aControls)
    {
        if(aControl.is())
        {
            aControl->removeFocusListener( mxBibGeneralPageFocusListener );
            aControl = nullptr;
        }
    }
#endif
}

void BibGeneralPage::CommitActiveControl()
{
#if 0
    uno::Reference< form::runtime::XFormController > xFormCtrl = pDatMan->GetFormController();
    uno::Reference< awt::XControl >  xCurr = xFormCtrl->getCurrentControl();
    if(xCurr.is())
    {
        uno::Reference< awt::XControlModel >  xModel = xCurr->getModel();
        uno::Reference< form::XBoundComponent >  xBound(xModel, UNO_QUERY);
        if(xBound.is())
            xBound->commit();
    }
#endif
}

namespace
{
    class ChangeListener : public cppu::WeakImplHelper<css::beans::XPropertyChangeListener>
    {
    public:
        explicit ChangeListener(weld::Entry& rEntry, css::uno::Reference<css::beans::XPropertySet>& rPropSet)
            : m_rEntry(rEntry)
            , m_xPropSet(rPropSet)
            , m_bSelfChanging(false)
        {
            m_xPropSet->addPropertyChangeListener("Text", this);
            rEntry.connect_focus_out(LINK(this, ChangeListener, LoseFocusHdl));
        }

        virtual ~ChangeListener() override
        {
            fprintf(stderr, "dtor of change listener\n");
            m_xPropSet->removePropertyChangeListener("Text", this);
        }

        virtual void SAL_CALL disposing(lang::EventObject const &) override
        {
            m_xPropSet.clear();
            fprintf(stderr, "disposing\n");
        }

        virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& evt) override
        {
            if (m_bSelfChanging)
                return;
            OUString sNewName;
            evt.NewValue >>= sNewName;
            fprintf(stderr, "change to %s\n", sNewName.toUtf8().getStr());
            m_rEntry.set_text(sNewName);
            m_rEntry.save_value();
        }

    private:
        weld::Entry& m_rEntry;
        css::uno::Reference<css::beans::XPropertySet> m_xPropSet;
        bool m_bSelfChanging;

        DECL_LINK(LoseFocusHdl, weld::Widget&, void);

        void WriteBack()
        {
            if (!m_rEntry.get_value_changed_from_saved())
                return;
            m_bSelfChanging = true;
            m_xPropSet->setPropertyValue("Text", makeAny(m_rEntry.get_text()));

            css::uno::Reference<css::form::XBoundComponent> xBound(m_xPropSet, css::uno::UNO_QUERY);
            if (xBound.is())
                xBound->commit();

            m_bSelfChanging = false;
            m_rEntry.save_value();
        }

    };

    IMPL_LINK_NOARG(ChangeListener, LoseFocusHdl, weld::Widget&, void)
    {
        WriteBack();
    }
}

bool BibGeneralPage::AddXControl(const OUString& rName, weld::Entry& rEntry)
{
    uno::Reference< awt::XControlModel >  xCtrModel;
    try
    {
        xCtrModel = pDatMan->loadControlModel(rName, false);
        if ( xCtrModel.is() )
        {
            uno::Reference< beans::XPropertySet >  xPropSet( xCtrModel, UNO_QUERY );

            if( xPropSet.is())
            {
                uno::Reference< beans::XPropertySetInfo >  xPropInfo = xPropSet->getPropertySetInfo();

                uno::Any aAny = xPropSet->getPropertyValue("Text");
                OUString sText;
                aAny >>= sText;
                rEntry.set_text(sText);
                rEntry.save_value();

                css::uno::Reference<css::beans::XPropertyChangeListener> xListener(new ChangeListener(rEntry, xPropSet));
            }
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("BibGeneralPage::AddXControl: something went wrong!");
    }
    return xCtrModel.is();
}

template<class Target> void BibGeneralPage::AddControlWithError(const OUString& rColumnName, const OUString& rColumnUIName,
    Target& rWidget, OUString& rErrorString, const OString& rHelpId)
{
    rWidget.set_help_id(rHelpId);
    bool bSuccess = AddXControl(rColumnName, rWidget);
    if (!bSuccess)
    {
        if( !rErrorString.isEmpty() )
            rErrorString += "\n";

        rErrorString += MnemonicGenerator::EraseAllMnemonicChars(rColumnUIName);
    }
}

bool BibGeneralPage::AddXControl(const OUString& rName, weld::ComboBox& rList)
{
    uno::Reference< awt::XControlModel >  xCtrModel;
    try
    {
        xCtrModel = pDatMan->loadControlModel(rName, true);
        if ( xCtrModel.is() )
        {
            uno::Reference< beans::XPropertySet >  xPropSet( xCtrModel, UNO_QUERY );

            if( xPropSet.is())
            {
                uno::Reference< beans::XPropertySetInfo >  xPropInfo = xPropSet->getPropertySetInfo();

                xLBModel.set(xCtrModel, UNO_QUERY);

                css::uno::Sequence<OUString> aEntries;
                xPropSet->getPropertyValue("StringItemList") >>= aEntries;
                for (const OUString& rString : std::as_const(aEntries))
                     rList.append_text(rString);

                sal_Int16 nSelection = -1;
                Sequence<sal_Int16> aSelection;
                xPropSet->getPropertyValue("SelectedItems") >>= aSelection;
                if (aSelection.hasElements())
                    nSelection = aSelection[0];

                rList.set_active(nSelection);
                rList.save_value();
#if 0
                css::uno::Reference<css::beans::XPropertyChangeListener> xListener(new ChangeListener(rEntry, xPropSet));
#endif
            }
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("BibGeneralPage::AddXControl: something went wrong!");
    }
    return xCtrModel.is();
}


void BibGeneralPage::focusGained(const awt::FocusEvent& /*rEvent*/)
{
#if 0
    Reference<awt::XWindow> xCtrWin(rEvent.Source, UNO_QUERY );
    if(!xCtrWin.is())
        return;

    ::Size aOutSize = pScrolledWindow->getVisibleChildSize();
    awt::Rectangle aRect = xCtrWin->getPosSize();
    Point aOffset(pGrid->GetPosPixel());
    tools::Long nX = aRect.X + aOffset.X();
    if (nX < 0 || nX > aOutSize.Width())
    {
        pScrolledWindow->getHorzScrollBar().DoScroll(aRect.X);
    }

    tools::Long nY = aRect.Y + aOffset.Y();
    if (nY < 0 || nY > aOutSize.Height())
    {
        pScrolledWindow->getVertScrollBar().DoScroll(aRect.Y);
    }
#endif
}

void BibGeneralPage::focusLost()
{
    CommitActiveControl();
}

void BibGeneralPage::GetFocus()
{
#if 0
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
#endif
}

BibGeneralPageFocusListener::BibGeneralPageFocusListener(BibGeneralPage *pBibGeneralPage): mpBibGeneralPage(pBibGeneralPage)
{}

void BibGeneralPageFocusListener::focusGained( const css::awt::FocusEvent& e )
{
    mpBibGeneralPage->focusGained(e);
}

void BibGeneralPageFocusListener::focusLost( const css::awt::FocusEvent& )
{
    mpBibGeneralPage->focusLost();
}

void BibGeneralPageFocusListener::disposing( const css::lang::EventObject& )
{}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
