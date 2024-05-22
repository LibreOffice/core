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

#include <swtypes.hxx>
#include "mmlayoutpage.hxx"
#include <mailmergewizard.hxx>
#include <mmconfigitem.hxx>
#include <mailmergehelper.hxx>
#include <unotools.hxx>
#include <comphelper/string.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/servicehelper.hxx>
#include <i18nutil/unicode.hxx>
#include <unotools/tempfile.hxx>
#include <uitool.hxx>
#include <view.hxx>
#include <swundo.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <svtools/unitconv.hxx>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/view/DocumentZoomType.hpp>
#include <fldmgr.hxx>
#include <fldbas.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <wrtsh.hxx>
#include <fmtsrnd.hxx>
#include <pagedesc.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <editeng/boxitem.hxx>
#include <osl/file.hxx>
#include <vcl/settings.hxx>
#include <unoprnms.hxx>

#include <dbui.hrc>

using namespace osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::view;

constexpr tools::Long DEFAULT_LEFT_DISTANCE = o3tl::toTwips(25, o3tl::Length::mm); // 2,5 cm
constexpr tools::Long DEFAULT_TOP_DISTANCE  = o3tl::toTwips(55, o3tl::Length::mm); // 5,5 cm
constexpr tools::Long GREETING_TOP_DISTANCE = o3tl::toTwips(125, o3tl::Length::mm); //12,5 cm
constexpr tools::Long DEFAULT_ADDRESS_WIDTH = o3tl::toTwips(75, o3tl::Length::mm); // 7,5 cm
constexpr tools::Long DEFAULT_ADDRESS_HEIGHT = o3tl::toTwips(35, o3tl::Length::mm); // 3,5cm

SwMailMergeLayoutPage::SwMailMergeLayoutPage(weld::Container* pPage, SwMailMergeWizard* pWizard)
    : vcl::OWizardPage(pPage, pWizard, u"modules/swriter/ui/mmlayoutpage.ui"_ustr, u"MMLayoutPage"_ustr)
    , m_pExampleWrtShell(nullptr)
    , m_pAddressBlockFormat(nullptr)
    , m_bIsGreetingInserted(false)
    , m_pWizard(pWizard)
    , m_xPosition(m_xBuilder->weld_container(u"addresspos"_ustr))
    , m_xAlignToBodyCB(m_xBuilder->weld_check_button(u"align"_ustr))
    , m_xLeftFT(m_xBuilder->weld_label(u"leftft"_ustr))
    , m_xLeftMF(m_xBuilder->weld_metric_spin_button(u"left"_ustr, FieldUnit::CM))
    , m_xTopMF(m_xBuilder->weld_metric_spin_button(u"top"_ustr, FieldUnit::CM))
    , m_xGreetingLine(m_xBuilder->weld_container(u"greetingspos"_ustr))
    , m_xUpPB(m_xBuilder->weld_button(u"up"_ustr))
    , m_xDownPB(m_xBuilder->weld_button(u"down"_ustr))
    , m_xZoomLB(m_xBuilder->weld_combo_box(u"zoom"_ustr))
{
    std::shared_ptr<const SfxFilter> pSfxFlt =
            SwDocShell::Factory().GetFilterContainer()->
            GetFilter4FilterName(u"writer8"_ustr, SfxFilterFlags::EXPORT);

    //save the current document into a temporary file
    {
        //temp file needs its own block
        //creating with extension is not supported by a static method :-(
        OUString const sExt(
            comphelper::string::stripStart(pSfxFlt->GetDefaultExtension(),'*'));
        utl::TempFileNamed aTempFile( u"", true, sExt );
        aTempFile.EnableKillingFile();
        m_sExampleURL = aTempFile.GetURL();
    }
    SwView* pView = m_pWizard->GetSwView();
    // Don't save embedded data set! It would steal it from current document.
    uno::Sequence< beans::PropertyValue > aValues =
    {
        comphelper::makePropertyValue(u"FilterName"_ustr, pSfxFlt->GetFilterName()),
        comphelper::makePropertyValue(u"NoEmbDataSet"_ustr, true)
    };

    uno::Reference< frame::XStorable > xStore( pView->GetDocShell()->GetModel(), uno::UNO_QUERY);
    xStore->storeToURL( m_sExampleURL, aValues   );

    Link<SwOneExampleFrame&,void> aLink(LINK(this, SwMailMergeLayoutPage, PreviewLoadedHdl_Impl));
    m_xExampleFrame.reset(new SwOneExampleFrame(EX_SHOW_DEFAULT_PAGE, &aLink, &m_sExampleURL));
    m_xExampleContainerWIN.reset(new weld::CustomWeld(*m_xBuilder, u"example"_ustr, *m_xExampleFrame));

    Size aSize = m_xExampleFrame->GetDrawingArea()->get_ref_device().LogicToPixel(
            Size(124, 159), MapMode(MapUnit::MapAppFont));
    m_xExampleFrame->set_size_request(aSize.Width(), aSize.Height());

    m_xExampleContainerWIN->hide();

    m_xLeftMF->set_value(m_xLeftMF->normalize(DEFAULT_LEFT_DISTANCE), FieldUnit::TWIP);
    m_xTopMF->set_value(m_xTopMF->normalize(DEFAULT_TOP_DISTANCE), FieldUnit::TWIP);

    const LanguageTag& rLang = Application::GetSettings().GetUILanguageTag();
    m_xZoomLB->append_text(unicode::formatPercent(50, rLang));
    m_xZoomLB->append_text(unicode::formatPercent(75, rLang));
    m_xZoomLB->append_text(unicode::formatPercent(100, rLang));
    m_xZoomLB->set_active(0); //page size
    m_xZoomLB->connect_changed(LINK(this, SwMailMergeLayoutPage, ZoomHdl_Impl));

    Link<weld::MetricSpinButton&,void> aFrameHdl = LINK(this, SwMailMergeLayoutPage, ChangeAddressHdl_Impl);
    m_xLeftMF->connect_value_changed(aFrameHdl);
    m_xTopMF->connect_value_changed(aFrameHdl);

    FieldUnit eFieldUnit = ::GetDfltMetric(false);
    ::SetFieldUnit( *m_xLeftMF, eFieldUnit );
    ::SetFieldUnit( *m_xTopMF, eFieldUnit );

    Link<weld::Button&,void> aUpDownHdl = LINK(this, SwMailMergeLayoutPage, GreetingsHdl_Impl );
    m_xUpPB->connect_clicked(aUpDownHdl);
    m_xDownPB->connect_clicked(aUpDownHdl);
    m_xAlignToBodyCB->connect_toggled(LINK(this, SwMailMergeLayoutPage, AlignToTextHdl_Impl));
    m_xAlignToBodyCB->set_active(true);
}

SwMailMergeLayoutPage::~SwMailMergeLayoutPage()
{
    File::remove( m_sExampleURL );
}

void SwMailMergeLayoutPage::Activate()
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    bool bGreetingLine = rConfigItem.IsGreetingLine(false) && !rConfigItem.IsGreetingInserted();
    bool bAddressBlock = rConfigItem.IsAddressBlock() && !rConfigItem.IsAddressInserted();

    m_xPosition->set_sensitive(bAddressBlock);
    AlignToTextHdl_Impl(*m_xAlignToBodyCB);

    m_xGreetingLine->set_sensitive(bGreetingLine);

    //check if greeting and/or address frame have to be inserted/removed
    if(!m_pExampleWrtShell) // initially there's nothing to check
        return;

    if(!rConfigItem.IsGreetingInserted() &&
            m_bIsGreetingInserted != bGreetingLine )
    {
        if( m_bIsGreetingInserted )
        {
            m_pExampleWrtShell->DelFullPara();
            m_bIsGreetingInserted = false;
        }
        else
        {
            InsertGreeting(*m_pExampleWrtShell, m_pWizard->GetConfigItem(), true);
            m_bIsGreetingInserted = true;
        }
    }
    if(!rConfigItem.IsAddressInserted() &&
            rConfigItem.IsAddressBlock() != ( nullptr != m_pAddressBlockFormat ))
    {
        if( m_pAddressBlockFormat )
        {
            m_pExampleWrtShell->Push();
            m_pExampleWrtShell->GotoFly( m_pAddressBlockFormat->GetName() );
            m_pExampleWrtShell->DelRight();
            m_pAddressBlockFormat = nullptr;
            m_pExampleWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
        }
        else
        {
            tools::Long nLeft = static_cast< tools::Long >(m_xLeftMF->denormalize(m_xLeftMF->get_value(FieldUnit::TWIP)));
            tools::Long nTop  = static_cast< tools::Long >(m_xTopMF->denormalize(m_xTopMF->get_value(FieldUnit::TWIP)));
            m_pAddressBlockFormat = InsertAddressFrame(
                    *m_pExampleWrtShell, m_pWizard->GetConfigItem(),
                    Point(nLeft, nTop),
                    m_xAlignToBodyCB->get_active(), true);
        }
    }
    m_xExampleFrame->Invalidate();
}

bool SwMailMergeLayoutPage::commitPage(::vcl::WizardTypes::CommitPageReason eReason)
{
    //now insert the frame and the greeting
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    if (eReason == ::vcl::WizardTypes::eTravelForward || eReason == ::vcl::WizardTypes::eFinish)
    {
        tools::Long nLeft = static_cast< tools::Long >(m_xLeftMF->denormalize(m_xLeftMF->get_value(FieldUnit::TWIP)));
        tools::Long nTop  = static_cast< tools::Long >(m_xTopMF->denormalize(m_xTopMF->get_value(FieldUnit::TWIP)));
        InsertAddressAndGreeting(
                    m_pWizard->GetSwView(),
                    rConfigItem,
                    Point(nLeft, nTop),
                    m_xAlignToBodyCB->get_active());
    }
    return true;
}

SwFrameFormat*  SwMailMergeLayoutPage::InsertAddressAndGreeting(SwView const * pView,
        SwMailMergeConfigItem& rConfigItem,
        const Point& rAddressPosition,
        bool bAlignToBody)
{
    SwFrameFormat* pAddressBlockFormat = nullptr;
    pView->GetWrtShell().StartUndo(SwUndoId::INSERT);
    if(rConfigItem.IsAddressBlock() && !rConfigItem.IsAddressInserted())
    {
        //insert the frame
        Point aAddressPosition(DEFAULT_LEFT_DISTANCE, DEFAULT_TOP_DISTANCE);
        if(rAddressPosition.X() > 0 && rAddressPosition.Y() > 0)
            aAddressPosition = rAddressPosition;
        pAddressBlockFormat = InsertAddressFrame( pView->GetWrtShell(),
                                        rConfigItem,
                                        aAddressPosition, bAlignToBody, false);
        rConfigItem.SetAddressInserted();
    }
    //now the greeting
    if(rConfigItem.IsGreetingLine(false) && !rConfigItem.IsGreetingInserted())
    {
        InsertGreeting( pView->GetWrtShell(), rConfigItem, false);
        rConfigItem.SetGreetingInserted();
    }
    pView->GetWrtShell().EndUndo(SwUndoId::INSERT);
    return pAddressBlockFormat;
}

SwFrameFormat* SwMailMergeLayoutPage::InsertAddressFrame(
        SwWrtShell& rShell,
        SwMailMergeConfigItem const & rConfigItem,
        const Point& rDestination,
        bool bAlignLeft,
        bool bExample)
{
    // insert the address block and the greeting line
    SfxItemSetFixed<
            RES_FRM_SIZE, RES_FRM_SIZE,
            RES_SURROUND, RES_ANCHOR,
            RES_BOX, RES_BOX>  aSet( rShell.GetAttrPool() );
    aSet.Put(SwFormatAnchor(RndStdIds::FLY_AT_PAGE, 1));
    if(bAlignLeft)
        aSet.Put(SwFormatHoriOrient( 0, text::HoriOrientation::NONE, text::RelOrientation::PAGE_PRINT_AREA ));
    else
        aSet.Put(SwFormatHoriOrient( rDestination.X(), text::HoriOrientation::NONE, text::RelOrientation::PAGE_FRAME ));
    aSet.Put(SwFormatVertOrient( rDestination.Y(), text::VertOrientation::NONE, text::RelOrientation::PAGE_FRAME ));
    aSet.Put(SwFormatFrameSize( SwFrameSize::Minimum, DEFAULT_ADDRESS_WIDTH, DEFAULT_ADDRESS_HEIGHT ));
    // the example gets a border around the frame, the real document doesn't get one
    if(!bExample)
        aSet.Put(SvxBoxItem( RES_BOX ));
    aSet.Put(SwFormatSurround( css::text::WrapTextMode_NONE ));

    rShell.NewFlyFrame(aSet, true );
    SwFrameFormat* pRet = rShell.GetFlyFrameFormat();
    OSL_ENSURE( pRet, "Fly not inserted" );

    rShell.UnSelectFrame();
    const Sequence< OUString> aBlocks = rConfigItem.GetAddressBlocks();
    if(bExample)
    {
        rShell.Insert(aBlocks[0]);
    }
    else
    {
        //the placeholders should be replaced by the appropriate fields
        SwFieldMgr aFieldMgr(&rShell);
        //create a database string source.command.commandtype.column
        const SwDBData& rData = rConfigItem.GetCurrentDBData();
        OUString sDBName(rData.sDataSource + OUStringChar(DB_DELIM)
            + rData.sCommand + OUStringChar(DB_DELIM));
        const OUString sDatabaseConditionPrefix(sDBName.replace(DB_DELIM, '.'));
        sDBName += OUString::number(rData.nCommandType) + OUStringChar(DB_DELIM);

        // if only the country is in an address line the
        // paragraph has to be hidden depending on the
        // IsIncludeCountry()/GetExcludeCountry() settings

        bool bIncludeCountry = rConfigItem.IsIncludeCountry();
        bool bHideEmptyParagraphs = rConfigItem.IsHideEmptyParagraphs();
        const OUString rExcludeCountry = rConfigItem.GetExcludeCountry();
        bool bSpecialReplacementForCountry = (!bIncludeCountry || !rExcludeCountry.isEmpty());

        const std::vector<std::pair<OUString, int>>& rHeaders = rConfigItem.GetDefaultAddressHeaders();
        Sequence< OUString> aAssignment =
                        rConfigItem.GetColumnAssignment( rConfigItem.GetCurrentDBData() );
        const OUString* pAssignment = aAssignment.getConstArray();
        const OUString sCountryColumn(
            (aAssignment.getLength() > MM_PART_COUNTRY && !aAssignment[MM_PART_COUNTRY].isEmpty())
            ? aAssignment[MM_PART_COUNTRY]
            : rHeaders[MM_PART_COUNTRY].first);

        OUString sHideParagraphsExpression;
        SwAddressIterator aIter(aBlocks[0]);
        while(aIter.HasMore())
        {
            SwMergeAddressItem aItem = aIter.Next();
            if(aItem.bIsColumn)
            {
                OUString sConvertedColumn = aItem.sText;
                auto nSize = std::min(static_cast<sal_uInt32>(rHeaders.size()),
                                      static_cast<sal_uInt32>(aAssignment.getLength()));
                for(sal_uInt32 nColumn = 0; nColumn < nSize; ++nColumn)
                {
                    if (rHeaders[nColumn].first == aItem.sText &&
                        !pAssignment[nColumn].isEmpty())
                    {
                        sConvertedColumn = pAssignment[nColumn];
                        break;
                    }
                }
                const OUString sDB(sDBName + sConvertedColumn);

                if(!sHideParagraphsExpression.isEmpty())
                   sHideParagraphsExpression += " AND ";
                sHideParagraphsExpression += "![" + sDatabaseConditionPrefix + sConvertedColumn + "]";

                if( bSpecialReplacementForCountry && sCountryColumn == sConvertedColumn )
                {
                    // now insert a hidden paragraph field
                    if( !rExcludeCountry.isEmpty() )
                    {
                        const OUString sExpression("[" + sDatabaseConditionPrefix + sCountryColumn + "]");
                        SwInsertField_Data aData(SwFieldTypesEnum::ConditionalText, 0,
                                               sExpression + " != \"" + rExcludeCountry + "\"",
                                               sExpression,
                                               0, &rShell );
                        aFieldMgr.InsertField( aData );
                    }
                    else
                    {
                        SwInsertField_Data aData(SwFieldTypesEnum::HiddenParagraph, 0, u""_ustr, u""_ustr, 0, &rShell );
                        aFieldMgr.InsertField( aData );
                    }
                }
                else
                {
                    SwInsertField_Data aData(SwFieldTypesEnum::Database, 0, sDB, OUString(), 0, &rShell);
                    aFieldMgr.InsertField( aData );
                }
            }
            else if(!aItem.bIsReturn)
            {
                rShell.Insert(aItem.sText);
            }
            else
            {
                if(bHideEmptyParagraphs)
                {
                    SwInsertField_Data aData(SwFieldTypesEnum::HiddenParagraph, 0, sHideParagraphsExpression, OUString(), 0, &rShell);
                    aFieldMgr.InsertField( aData );
                }
                sHideParagraphsExpression.clear();
                //now add a new paragraph
                rShell.SplitNode();
            }
        }
        if(bHideEmptyParagraphs && !sHideParagraphsExpression.isEmpty())
        {
            SwInsertField_Data aData(SwFieldTypesEnum::HiddenParagraph, 0, sHideParagraphsExpression, OUString(), 0, &rShell);
            aFieldMgr.InsertField( aData );
        }
    }
    return pRet;
}

void SwMailMergeLayoutPage::InsertGreeting(SwWrtShell& rShell, SwMailMergeConfigItem const & rConfigItem, bool bExample)
{
    //set the cursor to the desired position - if no text content is here then
    //new paragraphs are inserted
    const SwRect& rPageRect = rShell.GetAnyCurRect(CurRectType::Page);
    const Point aGreetingPos( DEFAULT_LEFT_DISTANCE + rPageRect.Left(), GREETING_TOP_DISTANCE );

    const bool bRet = rShell.SetShadowCursorPos( aGreetingPos, SwFillMode::TabSpace );

    if(!bRet)
    {
        //there's already text at the desired position
        //go to start of the doc, directly!
        rShell.SttEndDoc(true);
        //and go by paragraph until the position is reached
        tools::Long nYPos = rShell.GetCharRect().Top();
        while(nYPos < GREETING_TOP_DISTANCE)
        {
            if(!rShell.FwdPara())
                break;
            nYPos = rShell.GetCharRect().Top();
        }
        //text needs to be appended
        while(nYPos < GREETING_TOP_DISTANCE)
        {
            if(!rShell.AppendTextNode())
                break;
            nYPos = rShell.GetCharRect().Top();
        }
    }
    else
    {
        //we may end up inside of a paragraph if the left margin is not at DEFAULT_LEFT_DISTANCE
        rShell.MovePara(GoCurrPara, fnParaStart);
    }
    bool bSplitNode = !rShell.IsEndPara();
    SwNodeOffset nMoves(rConfigItem.GetGreetingMoves());
    if( !bExample && SwNodeOffset(0) != nMoves )
    {
        if(nMoves < SwNodeOffset(0))
        {
            rShell.MoveParagraph( nMoves );
        }
        else
            while(nMoves)
            {
                bool bMoved = rShell.MoveParagraph();
                if(!bMoved)
                {
                    //insert a new paragraph before the greeting line
                    rShell.SplitNode();
                }
                --nMoves;
            }
    }
    //now insert the greeting text - if we have any?
    const bool bIndividual = rConfigItem.IsIndividualGreeting(false);
    if(bIndividual)
    {
        //lock expression fields - prevents hiding of the paragraph to insert into
        rShell.LockExpFields();
        if(bExample)
        {
            for(sal_Int8 eGender = SwMailMergeConfigItem::FEMALE;
                eGender <= SwMailMergeConfigItem::NEUTRAL; ++eGender)
            {
                Sequence< OUString > aEntries =
                            rConfigItem.GetGreetings(static_cast<SwMailMergeConfigItem::Gender>(eGender));
                sal_Int32 nCurrent = rConfigItem.GetCurrentGreeting(static_cast<SwMailMergeConfigItem::Gender>(eGender));
                if( nCurrent >= 0 && nCurrent < aEntries.getLength())
                {
                    // Greeting
                    rShell.Insert(aEntries[nCurrent]);
                    break;
                }
            }
        }
        else
        {
            SwFieldMgr aFieldMgr(&rShell);
            //three paragraphs, each with an appropriate hidden paragraph field
            //are to be inserted

            //name of the gender column
            const OUString sGenderColumn = rConfigItem.GetAssignedColumn(MM_PART_GENDER);
            const OUString sNameColumn = rConfigItem.GetAssignedColumn(MM_PART_LASTNAME);

            const OUString& rFemaleGenderValue = rConfigItem.GetFemaleGenderValue();
            bool bHideEmptyParagraphs = rConfigItem.IsHideEmptyParagraphs();
            const SwDBData& rData = rConfigItem.GetCurrentDBData();
            const OUString sCommonBase(rData.sDataSource + "." + rData.sCommand + ".");
            const OUString sConditionBase("[" + sCommonBase + sGenderColumn + "]");
            const OUString sNameColumnBase("[" + sCommonBase + sNameColumn + "]");

            const OUString sDBName(rData.sDataSource + OUStringChar(DB_DELIM)
                + rData.sCommand + OUStringChar(DB_DELIM)
                + OUString::number(rData.nCommandType) + OUStringChar(DB_DELIM));

//          Female:  [database.sGenderColumn] != "rFemaleGenderValue" && [database.NameColumn]
//          Male:    [database.sGenderColumn] == "rFemaleGenderValue" && [database.rGenderColumn]
//          Neutral: [database.sNameColumn]
            OSL_ENSURE(!sGenderColumn.isEmpty() && !rFemaleGenderValue.isEmpty(),
                    "gender settings not available - how to form the condition?");
            //column used as lastname
            for(sal_Int8 eGender = SwMailMergeConfigItem::FEMALE;
                eGender <= SwMailMergeConfigItem::NEUTRAL; ++eGender)
            {
                Sequence< OUString> aEntries = rConfigItem.GetGreetings(static_cast<SwMailMergeConfigItem::Gender>(eGender));
                sal_Int32 nCurrent = rConfigItem.GetCurrentGreeting(static_cast<SwMailMergeConfigItem::Gender>(eGender));
                if( nCurrent >= 0 && nCurrent < aEntries.getLength())
                {
                    const OUString sGreeting = aEntries[nCurrent];
                    OUString sCondition;
                    OUString sHideParagraphsExpression;
                    switch(eGender)
                    {
                        case  SwMailMergeConfigItem::FEMALE:
                            sCondition = sConditionBase + " != \"" + rFemaleGenderValue
                                + "\" OR NOT " + sNameColumnBase;
                            sHideParagraphsExpression = "!" + sNameColumnBase;
                        break;
                        case  SwMailMergeConfigItem::MALE:
                            sCondition = sConditionBase + " == \"" + rFemaleGenderValue
                                + "\" OR NOT " + sNameColumnBase;
                        break;
                        case  SwMailMergeConfigItem::NEUTRAL:
                            sCondition = sNameColumnBase;
                        break;
                    }

                    if(bHideEmptyParagraphs && !sHideParagraphsExpression.isEmpty())
                    {
                        OUString sComplete = "(" + sCondition + ") OR (" + sHideParagraphsExpression + ")";
                        SwInsertField_Data aData(SwFieldTypesEnum::HiddenParagraph, 0, sComplete, OUString(), 0, &rShell);
                        aFieldMgr.InsertField( aData );
                    }
                    else
                    {
                        SwInsertField_Data aData(SwFieldTypesEnum::HiddenParagraph, 0, sCondition, OUString(), 0, &rShell);
                        aFieldMgr.InsertField( aData );
                    }
                    //now the text has to be inserted
                    const std::vector<std::pair<OUString, int>>& rHeaders = rConfigItem.GetDefaultAddressHeaders();
                    Sequence< OUString> aAssignment =
                                    rConfigItem.GetColumnAssignment( rConfigItem.GetCurrentDBData() );
                    const OUString* pAssignment = aAssignment.getConstArray();
                    SwAddressIterator aIter(sGreeting);
                    while(aIter.HasMore())
                    {
                        SwMergeAddressItem aItem = aIter.Next();
                        if(aItem.bIsColumn)
                        {
                            OUString sConvertedColumn = aItem.sText;
                            auto nSize = std::min(static_cast<sal_uInt32>(rHeaders.size()),
                                                  static_cast<sal_uInt32>(aAssignment.getLength()));
                            for(sal_uInt32 nColumn = 0; nColumn < nSize; ++nColumn)
                            {
                                if (rHeaders[nColumn].first == aItem.sText &&
                                    !pAssignment[nColumn].isEmpty())
                                {
                                    sConvertedColumn = pAssignment[nColumn];
                                    break;
                                }
                            }
                            SwInsertField_Data aData(SwFieldTypesEnum::Database, 0,
                                sDBName + sConvertedColumn,
                                OUString(), 0, &rShell);
                            aFieldMgr.InsertField( aData );
                        }
                        else
                        {
                            rShell.Insert(aItem.sText);
                        }
                    }
                    //now add a new paragraph
                    rShell.SplitNode();
                }
            }

        }
        rShell.UnlockExpFields();
    }
    else
    {
        Sequence< OUString> aEntries = rConfigItem.GetGreetings(SwMailMergeConfigItem::NEUTRAL);
        sal_Int32 nCurrent = rConfigItem.GetCurrentGreeting(SwMailMergeConfigItem::NEUTRAL);
        // Greeting
        rShell.Insert(( nCurrent >= 0 && nCurrent < aEntries.getLength() )
            ? aEntries[nCurrent] : OUString());
    }
    // now insert a new paragraph here if necessary
    if(bSplitNode)
    {
        rShell.Push();
        rShell.SplitNode();
        rShell.Pop(SwCursorShell::PopMode::DeleteCurrent);
    }
    //put the cursor to the start of the paragraph
    rShell.SttPara();

    OSL_ENSURE(nullptr == rShell.GetTableFormat(), "What to do with a table here?");
}

IMPL_LINK_NOARG(SwMailMergeLayoutPage, PreviewLoadedHdl_Impl, SwOneExampleFrame&, void)
{
    m_xExampleContainerWIN->show();

    Reference< XModel > & xModel = m_xExampleFrame->GetModel();
    //now the ViewOptions should be set properly
    Reference< XViewSettingsSupplier >  xSettings(xModel->getCurrentController(), UNO_QUERY);
    m_xViewProperties = xSettings->getViewSettings();
    auto pXDoc = comphelper::getFromUnoTunnel<SwXTextDocument>(xModel);
    SwDocShell* pDocShell = pXDoc->GetDocShell();
    m_pExampleWrtShell = pDocShell->GetWrtShell();
    OSL_ENSURE(m_pExampleWrtShell, "No SwWrtShell found!");
    if(!m_pExampleWrtShell)
        return;

    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    if(rConfigItem.IsAddressBlock())
    {
        m_pAddressBlockFormat = InsertAddressFrame(
                *m_pExampleWrtShell, rConfigItem,
                Point(DEFAULT_LEFT_DISTANCE, DEFAULT_TOP_DISTANCE),
                m_xAlignToBodyCB->get_active(), true);
    }
    if(rConfigItem.IsGreetingLine(false))
    {
        InsertGreeting(*m_pExampleWrtShell, rConfigItem, true);
        m_bIsGreetingInserted = true;
    }

    ZoomHdl_Impl(*m_xZoomLB);

    const SwFormatFrameSize& rPageSize = m_pExampleWrtShell->GetPageDesc(
                                     m_pExampleWrtShell->GetCurPageDesc()).GetMaster().GetFrameSize();
    m_xLeftMF->set_max(rPageSize.GetWidth() - DEFAULT_LEFT_DISTANCE, FieldUnit::NONE);
    m_xTopMF->set_max(rPageSize.GetHeight() - DEFAULT_TOP_DISTANCE, FieldUnit::NONE);
}

IMPL_LINK(SwMailMergeLayoutPage, ZoomHdl_Impl, weld::ComboBox&, rBox, void)
{
    if (!m_pExampleWrtShell)
        return;

    sal_Int16 eType = DocumentZoomType::BY_VALUE;
    short nZoom = 50;
    switch (rBox.get_active())
    {
        case 0 : eType = DocumentZoomType::ENTIRE_PAGE; break;
        case 1 : nZoom = 50; break;
        case 2 : nZoom = 75; break;
        case 3 : nZoom = 100; break;
    }
    Any aZoom;
    aZoom <<= eType;
    m_xViewProperties->setPropertyValue(UNO_NAME_ZOOM_TYPE, aZoom);
    aZoom <<= nZoom;
    m_xViewProperties->setPropertyValue(UNO_NAME_ZOOM_VALUE, aZoom);

    m_xExampleFrame->Invalidate();
}

IMPL_LINK_NOARG(SwMailMergeLayoutPage, ChangeAddressHdl_Impl, weld::MetricSpinButton&, void)
{
    if(!(m_pExampleWrtShell && m_pAddressBlockFormat))
        return;

    tools::Long nLeft = static_cast< tools::Long >(m_xLeftMF->denormalize(m_xLeftMF->get_value(FieldUnit::TWIP)));
    tools::Long nTop  = static_cast< tools::Long >(m_xTopMF->denormalize(m_xTopMF->get_value(FieldUnit::TWIP)));

    SfxItemSetFixed<RES_VERT_ORIENT, RES_ANCHOR> aSet(
        m_pExampleWrtShell->GetAttrPool());
    if (m_xAlignToBodyCB->get_active())
        aSet.Put(SwFormatHoriOrient( 0, text::HoriOrientation::NONE, text::RelOrientation::PAGE_PRINT_AREA ));
    else
        aSet.Put(SwFormatHoriOrient( nLeft, text::HoriOrientation::NONE, text::RelOrientation::PAGE_FRAME ));
    aSet.Put(SwFormatVertOrient( nTop, text::VertOrientation::NONE, text::RelOrientation::PAGE_FRAME ));
    m_pExampleWrtShell->GetDoc()->SetFlyFrameAttr( *m_pAddressBlockFormat, aSet );
    m_xExampleFrame->Invalidate();
}

IMPL_LINK(SwMailMergeLayoutPage, GreetingsHdl_Impl, weld::Button&, rButton, void)
{
    bool bDown = &rButton == m_xDownPB.get();
    bool bMoved = m_pExampleWrtShell->MoveParagraph( SwNodeOffset(bDown ? 1 : -1) );
    if (bMoved || bDown)
        m_pWizard->GetConfigItem().MoveGreeting(bDown ? 1 : -1 );
    if(!bMoved && bDown)
    {
        //insert a new paragraph before the greeting line
        m_pExampleWrtShell->SplitNode();
    }
    m_xExampleFrame->Invalidate();
}

IMPL_LINK(SwMailMergeLayoutPage, AlignToTextHdl_Impl, weld::Toggleable&, rBox, void)
{
    bool bCheck = rBox.get_active() && rBox.get_sensitive();
    m_xLeftFT->set_sensitive(!bCheck);
    m_xLeftMF->set_sensitive(!bCheck);
    ChangeAddressHdl_Impl( *m_xLeftMF );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
